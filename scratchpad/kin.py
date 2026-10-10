#!/usr/bin/env python3
"""kin.py [minratio] - find families among the UNMATCHED functions themselves.

scratchpad/twins.py compares every unmatched function against every MATCHED
one, which is the right question when a template already exists. It is blind to
the other case: a family where NO member has been written yet. The three
background script loaders - BackGround::m41b900 @0x41b900,
GFBackGround::m45cd40 @0x45cd40 and CatchBackGround::m46bc30 @0x46bc30 - were
exactly that, and twins.py could not have found them at any threshold. Writing
one of them made the other two cost a copy and a rename; the first came out
100% and the second 100% on its first build.

So run this when twins.py comes back empty, which it now does at 0.50. Pick the
BIGGEST member of a high-scoring group to write first: it carries the whole
shape, and the shorter ones are edits of it.

Reads the STUB annotations in src/ for the address list and sizes each one by
the distance to the next annotated address, trimming int3 padding.
"""
import re, os, sys, glob, bisect, difflib, subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MINR = float(sys.argv[1]) if len(sys.argv) > 1 else 0.55

ann, stubs = set(), {}
for f in glob.glob(ROOT + '/src/*.cpp'):
    L = open(f, errors='replace').read().split('\n')
    for i, l in enumerate(L):
        m = re.search(r'//\s*(FUNCTION|STUB):\s*EZ2AC_UNPACKED\s+0x0*([0-9a-fA-F]+)', l)
        if not m:
            continue
        a = int(m.group(2), 16)
        ann.add(a)
        if m.group(1) == 'STUB':
            for j in range(i + 1, min(i + 4, len(L))):
                t = L[j].strip()
                if t and not t.startswith('//'):
                    stubs[a] = t[:64]
                    break

data = open(ROOT + '/original/EZ2AC_Unpacked.exe', 'rb').read()
srt = sorted(ann)

def size(a):
    i = bisect.bisect_right(srt, a)
    end = min(srt[i] if i < len(srt) else a + 4096, a + 8192)
    off = end - 0x400000
    while off > a - 0x400000 and data[off - 1] == 0xcc:
        off -= 1
    return off + 0x400000 - a

def dis(a, n):
    o = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386', '-Mintel',
                        '--no-show-raw-insn', '--adjust-vma=0x400000',
                        '--start-address=%d' % a, '--stop-address=%d' % (a + n),
                        ROOT + '/original/EZ2AC_Unpacked.exe'],
                       capture_output=True, text=True).stdout
    out = []
    for l in o.split('\n'):
        m = re.match(r'\s*[0-9a-f]+:\t(.*)', l)
        if m:
            t = re.sub(r'0x[0-9a-f]{5,8}', 'A', m.group(1)).split()
            if t:
                out.append(' '.join(t))
    return out

seqs = {}
for a in stubs:
    n = size(a)
    if 60 <= n <= 8000:
        seqs[a] = dis(a, n)

keys = sorted(seqs)
pairs = []
for i, x in enumerate(keys):
    for y in keys[i + 1:]:
        lx, ly = len(seqs[x]), len(seqs[y])
        if not lx or not ly or min(lx, ly) / float(max(lx, ly)) < 0.55:
            continue
        r = difflib.SequenceMatcher(None, seqs[x], seqs[y]).ratio()
        if r >= MINR:
            pairs.append((r, x, y))

# group into families by transitive closure
parent = {}
def find(a):
    while parent.get(a, a) != a:
        a = parent[a]
    return a
for r, x, y in pairs:
    parent.setdefault(x, x); parent.setdefault(y, y)
    rx, ry = find(x), find(y)
    if rx != ry:
        parent[rx] = ry

fams = {}
for a in parent:
    fams.setdefault(find(a), []).append(a)

best = {}
for r, x, y in pairs:
    best[(x, y)] = r
print('%d stubs sized; %d pairs at >=%.2f; %d families\n' % (len(seqs), len(pairs), MINR, len(fams)))
for root in sorted(fams, key=lambda k: -len(fams[k])):
    mem = sorted(fams[root], key=lambda a: -len(seqs[a]))
    print('family of %d:' % len(mem))
    for a in mem:
        print('  0x%06x  %4d insns  %s' % (a, len(seqs[a]), stubs[a]))
    for (x, y), r in sorted(best.items(), key=lambda kv: -kv[1]):
        if x in mem and y in mem:
            print('      0x%06x ~ 0x%06x  %.3f' % (x, y, r))
    print()
