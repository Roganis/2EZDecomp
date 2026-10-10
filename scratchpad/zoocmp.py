#!/usr/bin/env python3
# zoocmp.py ZOO.asm LO HI — masked diff of a zoo listing (one function, objdump
# -d --no-show-raw-insn, sliced) against original bytes LO..HI. Masks branch
# and call targets, absolute ds: constants and address-sized immediates.
import re, subprocess, difflib, sys
zoo, lo, hi = sys.argv[1], int(sys.argv[2],16), int(sys.argv[3],16)
def norm(t):
    t = re.sub(r'\b(call|j\w+)\s+\S.*', r'\1 X', t)
    t = re.sub(r'ds:0x[0-9a-f]+', 'ds:K', t)
    t = re.sub(r'0x4[0-9a-f]{5}\b', 'ADDR', t)
    t = re.sub(r'\s+', ' ', t).strip()
    return t
def parse_zoo(path):
    out=[]
    for l in open(path):
        m=re.match(r'^\s*([0-9a-f]+):\t(.*)$', l)
        if m: out.append(norm(m.group(2)))
    return out
def parse_orig():
    out=[]
    p=subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel','--adjust-vma=0x400000',
        '--start-address=0x%x'%lo,'--stop-address=0x%x'%hi,'original/EZ2AC_Unpacked.exe'],
        capture_output=True,text=True)
    for l in p.stdout.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s*(?:[0-9a-f]{2} )+\s*(.+)$', l)
        if m: out.append(norm(m.group(2)))
    return out
a=parse_orig(); b=parse_zoo(zoo)
sm=difflib.SequenceMatcher(None,a,b,autojunk=False)
same=sum(bl.size for bl in sm.get_matching_blocks())
print("orig %d insns, zoo %d insns, matched %d (%.2f%%)"%(len(a),len(b),same,100.0*same/max(len(a),len(b))))
for op,i1,i2,j1,j2 in sm.get_opcodes():
    if op=='equal': continue
    print('@@ orig %d..%d  zoo %d..%d'%(i1,i2,j1,j2))
    for x in a[i1:i2]: print('  - '+x)
    for x in b[j1:j2]: print('  + '+x)
