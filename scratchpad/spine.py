"""Which STUBs are on the runtime spine?

Builds the call graph from call-rel32 plus vtable slots, walks it from the
application entry points, and reports every STUB the walk reaches - i.e. the
functions a RECOMPILATION actually needs, as opposed to whatever the
branch-density scan happens to rank highly.

Virtual dispatch is handled by over-approximating: once any vtable's address is
STORED by reachable code (a `mov [reg], 0x4xxxxx` vptr store), every slot of
that vtable becomes reachable too.
"""
import re, struct, os, sys, collections

S = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASE = 0x400000
TEXT_LO, TEXT_HI = 0x401000, 0x478000
RD_LO, RD_HI = 0x478000, 0x4b1000

pat = re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
insns = []
for l in open(S + '/text.asm'):
    m = pat.match(l)
    if m:
        insns.append((int(m.group(1), 16), m.group(2), m.group(3).strip()))

d = open(ROOT + '/original/EZ2AC_Unpacked.exe', 'rb').read()

# ---- function starts -------------------------------------------------------
# A `call rel32` target is always a function. A `jmp rel32` target USUALLY IS
# NOT: cl compiles every loop tail, every `else` and every cross-jumped block
# inside one function to a `jmp`, so admitting them wholesale invents thousands
# of phantom "functions" in the middle of code that is already reconstructed -
# which is how this script came to report 988 unreconstructed spine functions
# and 154 KB when the true figures are 70 and 70 KB. The tail-call forwarders
# ARE real, though (`void T::s5() { doS5(); }` compiles to a bare jmp), so the
# targets are admitted in a second pass below, filtered against what we know.
starts = set()
jmp_targets = set()
for a, mn, rest in insns:
    if mn in ('call', 'jmp'):
        m = re.match(r'^0x([0-9a-f]+)$', rest)
        if m and TEXT_LO <= int(m.group(1), 16) < TEXT_HI:
            (starts if mn == 'call' else jmp_targets).add(int(m.group(1), 16))

vtslot = collections.defaultdict(list)      # vtable va -> [slot targets]
for va in range(RD_LO, RD_HI, 4):
    v = struct.unpack('<I', d[va - BASE:va - BASE + 4])[0]
    if TEXT_LO <= v < TEXT_HI:
        starts.add(v)

# ---- annotations -----------------------------------------------------------
ann = {}
amark = re.compile(r'//\s*(FUNCTION|STUB):\s*EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)')
for root, _, fs in os.walk(ROOT + '/src'):
    for f in fs:
        if not f.endswith(('.cpp', '.h')):
            continue
        for line in open(os.path.join(root, f), errors='ignore'):
            m = amark.search(line)
            if m:
                a = int(m.group(2), 16)
                # FUNCTION wins over STUB if both ever appear
                if ann.get(a) != 'FUNCTION':
                    ann[a] = m.group(1)
starts |= set(ann)

# Second pass for the jmp targets: admit one only when it does NOT fall inside
# a function we already know the extent of. A jmp landing in the interior of an
# ANNOTATED function is a branch, never an entry point; one landing in a region
# we have no annotation for may well be a tail-called function, and splitting
# there costs nothing because those bytes count as unreconstructed either way.
# The reliable tell is cl's own padding: it aligns each function and fills the
# gap with int3, so a REAL entry point reached only by a tail `jmp` has int3
# immediately before it, while an interior block has a live instruction there.
# Without this the count is meaningless even when the byte totals are right -
# interior blocks of UNANNOTATED functions split them into ~46-byte fragments,
# which is what made "the biggest unreconstructed function" list print
# mid-function addresses.
_prev = {}
for _i in range(1, len(insns)):
    _prev[insns[_i][0]] = insns[_i - 1][1]
for _t in sorted(jmp_targets):
    if _t not in starts and _prev.get(_t) == 'int3':
        starts.add(_t)

sorted_starts = sorted(s for s in starts if TEXT_LO <= s < TEXT_HI)
def owner(a):
    i = 0
    lo, hi = 0, len(sorted_starts) - 1
    while lo <= hi:
        mid = (lo + hi) // 2
        if sorted_starts[mid] <= a:
            i = mid; lo = mid + 1
        else:
            hi = mid - 1
    return sorted_starts[i]

# ---- per-function body + edges --------------------------------------------
end_of = {}
for i, s in enumerate(sorted_starts):
    end_of[s] = sorted_starts[i + 1] if i + 1 < len(sorted_starts) else TEXT_HI

edges = collections.defaultdict(set)
vt_stored = collections.defaultdict(set)    # func -> vtables it stores
imm = re.compile(r'0x([0-9a-f]{6,8})')
for a, mn, rest in insns:
    f = owner(a)
    if mn in ('call', 'jmp'):
        m = re.match(r'^0x([0-9a-f]+)$', rest)
        if m:
            t = int(m.group(1), 16)
            if TEXT_LO <= t < TEXT_HI and t != f:
                edges[f].add(t)
    if mn == 'mov' and 'DWORD PTR [' in rest:
        m = imm.search(rest.split(',')[-1])
        if m:
            v = int(m.group(1), 16)
            if RD_LO <= v < RD_HI:
                vt_stored[f].add(v)

# vtable slot groups: a run of consecutive text pointers starting at the stored va
def slots_at(va):
    out = []
    p = va
    while p < RD_HI:
        v = struct.unpack('<I', d[p - BASE:p - BASE + 4])[0]
        if not (TEXT_LO <= v < TEXT_HI):
            break
        out.append(v)
        p += 4
        if len(out) > 40:
            break
    return out

# ---- walk ------------------------------------------------------------------
ROOTS = [0x401450, 0x4015e0, 0x417e90, 0x401930, 0x4181b0, 0x415cb0]
seen, work = set(), list(ROOTS)
while work:
    f = work.pop()
    # Map EVERY reached address to the function that contains it. Guarding this
    # with `if f in starts` let vtable slots and tail targets that are not
    # themselves starts enter `seen` raw, where they have no size and no
    # annotation - so they counted as zero-byte "unreconstructed functions" and
    # inflated that column into the thousands while the byte total stayed right.
    f = owner(f)
    if f in seen or not (TEXT_LO <= f < TEXT_HI):
        continue
    seen.add(f)
    for t in edges.get(f, ()):
        work.append(t)
    for vt in vt_stored.get(f, ()):
        for t in slots_at(vt):
            work.append(t)

# ---- report ----------------------------------------------------------------
stubs = [(a, end_of.get(a, a) - a) for a in sorted(seen) if ann.get(a) == 'STUB']
stubs.sort(key=lambda x: -x[1])
tot = sum(s for _, s in stubs)
print('spine reached %d functions; %d of them are STUBs, %d bytes' %
      (len(seen), len(stubs), tot))
print('%-10s %7s' % ('addr', 'bytes'))
for a, s in stubs[:int(sys.argv[1]) if len(sys.argv) > 1 else 40]:
    print('0x%06x %7d' % (a, s))
allstubs = [a for a in ann if ann[a] == 'STUB']
print('\n(total STUBs annotated: %d; off-spine: %d)' %
      (len(allstubs), len(allstubs) - len(stubs)))

# ---- spine coverage --------------------------------------------------------
matched = [a for a in seen if ann.get(a) == 'FUNCTION']
stubbed = [a for a in seen if ann.get(a) == 'STUB']
missing = [a for a in seen if a not in ann]
def bytes_of(l): return sum(end_of.get(a, a) - a for a in l)
print('\n== SPINE COVERAGE (functions reachable from the app entry points) ==')
for name, l in (('matched', matched), ('stubbed', stubbed), ('unreconstructed', missing)):
    print('  %-16s %5d funcs %8d bytes' % (name, len(l), bytes_of(l)))
sp = bytes_of(matched) + bytes_of(stubbed) + bytes_of(missing)
print('  spine total      %5d funcs %8d bytes  (matched = %.1f%%)' %
      (len(seen), sp, 100.0 * bytes_of(matched) / sp))
big = sorted(missing, key=lambda a: -(end_of.get(a, a) - a))[:15]
print('\n== BIGGEST UNRECONSTRUCTED FUNCTIONS ON THE SPINE ==')
for a in big:
    print('  0x%06x %7d' % (a, end_of.get(a, a) - a))
