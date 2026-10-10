#!/usr/bin/env python3
"""twins.py [minratio] - find UNMATCHED functions that are near-copies of
MATCHED ones. A copy costs a rename, so this is the cheapest work in the
tree; it is how the draw family, the fade ramps and the catch player's
walks were written. Compares mnemonic+operand-shape sequences (addresses
masked) and only pairs functions of similar length."""
import re, os, sys, difflib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LO, HI = 0x401000, 0x478000
MINR = float(sys.argv[1]) if len(sys.argv)>1 else 0.85

pat = re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
insns=[]
for l in open(ROOT+'/scratchpad/text.asm'):
    m=pat.match(l)
    if m: insns.append((int(m.group(1),16), m.group(2), m.group(3).strip()))

starts=set()
for a,mn,rest in insns:
    if mn=='call':
        m=re.match(r'^0x([0-9a-f]+)$', rest)
        if m and LO<=int(m.group(1),16)<HI: starts.add(int(m.group(1),16))
ann_fn=set()
for root,_,files in os.walk(ROOT+'/src'):
    for f in files:
        if not f.endswith(('.cpp','.h')): continue
        for line in open(os.path.join(root,f), errors='ignore'):
            m=re.search(r'//\s*FUNCTION:\s*EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)', line)
            if m and LO<=int(m.group(1),16)<HI: ann_fn.add(int(m.group(1),16))
starts |= ann_fn

ordered=sorted(starts)
idx={a:i for i,a in enumerate(insns and [x[0] for x in insns])}
body={}
for i,a in enumerate(ordered):
    end = ordered[i+1] if i+1<len(ordered) else HI
    seq=[]
    for addr,mn,rest in insns:
        if addr<a: continue
        if addr>=end: break
        if mn=='int3': continue
        seq.append(mn+' '+re.sub(r'0x[0-9a-f]{4,8}','A',rest))
    if 8 <= len(seq) <= 400: body[a]=seq

matched=[a for a in body if a in ann_fn]
unmatched=[a for a in body if a not in ann_fn]
print("comparing %d unmatched against %d matched" % (len(unmatched), len(matched)))
hits=[]
for u in unmatched:
    nu=len(body[u]); best=(0,None)
    for m in matched:
        if abs(len(body[m])-nu) > max(3, nu//10): continue
        r=difflib.SequenceMatcher(None, body[u], body[m], autojunk=False).ratio()
        if r>best[0]: best=(r,m)
    if best[0]>=MINR: hits.append((best[0], u, best[1], nu))
hits.sort(reverse=True)
for r,u,m,n in hits[:40]:
    print("  0x%06x (%3d insns)  ~= matched 0x%06x   %.0f%%" % (u,n,m,100*r))
print("%d candidate twins at >=%.0f%%" % (len(hits), 100*MINR))
