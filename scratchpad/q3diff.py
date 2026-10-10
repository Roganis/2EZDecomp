#!/usr/bin/env python3
# rankdiff.py [maxregions] — structural diff of the zoo-compiled
# RankingDirector ctor vs the original @0x457a90. Masks call/branch targets,
# address-sized displacements and relocation zeros.
import re, subprocess, difflib, sys, os

ZOO = os.environ['ZOO_ASM']
SYM = os.environ.get('ZOO_SYM', '?m404c50@KGraphics')
LO  = int(os.environ.get('ORIG_LO', '0x404c50'), 16)
HI  = int(os.environ.get('ORIG_HI', '0x404f61'), 16)
MAXR = int(sys.argv[1]) if len(sys.argv) > 1 else 30

def parse_zoo(path):
    out=[]; started=False
    for l in open(path):
        if l.startswith('00000000 <'+SYM): started=True; continue
        if started:
            if re.match(r'^[0-9a-f]{8} <', l): break
            m=re.match(r'^([0-9a-f]+):\t(.*)$', l)
            if m: out.append((int(m.group(1),16), m.group(2).strip()))
    return out

def parse_orig():
    out=[]
    p=subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel','--adjust-vma=0x400000',
        '--start-address=0x%x'%LO,'--stop-address=0x%x'%HI,'original/EZ2AC_Unpacked.exe'],
        capture_output=True,text=True)
    for l in p.stdout.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s*(?:[0-9a-f]{2} )+\s*(.+)$', l)
        if m: out.append((int(m.group(1),16), m.group(2).strip()))
    return out

def norm(ins):
    ins=re.sub(r'\s+',' ',ins)
    ins=re.sub(r'<[^>]*>','',ins).strip()
    if re.match(r'^(call|jmp|j[a-z]{1,2}) ',ins):
        return re.sub(r' .*',' X',ins,count=1)
    ins=re.sub(r'0x[0-9a-f]{5,8}','W',ins)
    ins=re.sub(r"\b0x0\b","W",ins); ins=re.sub(r"ds:0x[0-9a-f]{1,2}\b","ds:W",ins)
    return ins

zoo=parse_zoo(ZOO); orig=[o for o in parse_orig() if not re.fullmatch(r'(00 ?)+', o[1])]
zn=[norm(i[1]) for i in zoo]; on=[norm(i[1]) for i in orig]
sm=difflib.SequenceMatcher(None,on,zn,autojunk=False)
same=sum(b.size for b in sm.get_matching_blocks())
print(f"orig {len(on)} zoo {len(zn)} matching {same} ({100.0*same/len(on):.2f}%)")
ops=[o for o in sm.get_opcodes() if o[0]!='equal']
print(f"{len(ops)} divergent regions")
for tag,i1,i2,j1,j2 in ops[:MAXR]:
    print(f"--- {tag} @0x{orig[min(i1,len(orig)-1)][0]:x} (o{i2-i1}/z{j2-j1})")
    for k in range(i1,min(i2,i1+8)): print(f"  O {orig[k][0]:x}: {orig[k][1]}")
    for k in range(j1,min(j2,j1+8)): print(f"  Z {zoo[k][0]:x}: {zoo[k][1]}")
