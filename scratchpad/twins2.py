#!/usr/bin/env python3
"""Rank every UNMATCHED function against every MATCHED one.

One whole-.text objdump, split at function boundaries, so it runs in seconds
where the per-function version shelled out thousands of times.  Operands are
masked down to their SHAPE (registers kept, addresses and displacements
blanked), which is what lets a sibling on another class score high.

Boundaries come from call-rel32 targets plus every address annotated in src/,
so a function reached only through a vtable is bounded by its neighbours.
"""
import subprocess, re, struct, difflib, glob, sys

BIN = 'original/EZ2AC_Unpacked.exe'
LO, HI = 0x401000, 0x478000

real, stub = set(), set()
for f in glob.glob('src/*.cpp'):
    for m in re.finditer(r'// (FUNCTION|STUB): EZ2AC_UNPACKED 0x([0-9a-fA-F]+)',
                         open(f).read()):
        (real if m.group(1) == 'FUNCTION' else stub).add(int(m.group(2), 16))
stub -= real
done = real | stub

out = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386',
                      '--adjust-vma=0x400000', '--start-address=%s' % hex(LO),
                      '--stop-address=%s' % hex(HI), BIN],
                     capture_output=True, text=True).stdout

insns = []
for l in out.splitlines():
    m = re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{2} )+\s*(\S.*)', l)
    if m:
        insns.append((int(m.group(1), 16), m.group(2).strip()))

# Boundaries come from the DISASSEMBLY, not from a byte scan for 0xe8.  A
# naive byte scan finds `e8` inside displacements and immediates and invents
# function starts mid-instruction: 0x1de8 in `cmpl $0,0x1de8(%esi)` produced a
# phantom at 0x42d2ba that ranked 0.87 and cost a decode before it was caught.
starts = set()
for a, t in insns:
    m = re.match(r'call\s+0x([0-9a-f]+)$', t)
    if m:
        v = int(m.group(1), 16)
        if LO <= v < HI:
            starts.add(v)
starts |= {a for a in done if LO <= a < HI}
order = sorted(starts)

bodies = {}
idx = 0
for i, a in enumerate(order):
    end = order[i + 1] if i + 1 < len(order) else HI
    body = []
    while idx < len(insns) and insns[idx][0] < a:
        idx += 1
    j = idx
    while j < len(insns) and insns[j][0] < end:
        t = insns[j][1]
        if t.startswith('int3'):
            break
        body.append(re.sub(r'0x[0-9a-f]{3,8}', 'A', t))
        j += 1
    if len(body) >= 20:
        bodies[a] = body

# Only a RECONSTRUCTED function is a template. A STUB is annotated but has
# no source, so a twin of one is two decodes, not a copy-edit - it is still
# worth knowing about, but it must not be mistaken for a free match.
matched = {a: b for a, b in bodies.items() if a in real}
unmatched = {a: b for a, b in bodies.items() if a not in real}
print('%d reconstructed, %d not (of which %d are stubs)'
      % (len(matched), len(unmatched),
         len([a for a in unmatched if a in stub])))

thresh = float(sys.argv[1]) if len(sys.argv) > 1 else 0.55
res = []
for a, va in unmatched.items():
    best = (0.0, None)
    for b, vb in matched.items():
        if not (0.6 <= len(vb) / float(len(va)) <= 1.7):
            continue
        r = difflib.SequenceMatcher(None, va, vb).quick_ratio()
        if r <= best[0]:
            continue
        r = difflib.SequenceMatcher(None, va, vb).ratio()
        if r > best[0]:
            best = (r, b)
    if best[0] >= thresh:
        res.append((round(best[0], 3), hex(a), 'STUB' if a in stub else '',
                    len(va), hex(best[1])))

# The flag belongs to the LEFT address (the one with no source), never to the
# template - printing it last once read as if the template were the stub.
print('%-6s %-10s %-5s %-10s  %s'
      % ('ratio', 'todo', 'kind', 'insns', 'template (reconstructed)'))
for r in sorted(res, reverse=True):
    print('%-6s %-10s %-5s %-10s  %s' % (r[0], r[1], r[2], r[3], r[4]))
