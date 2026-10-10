#!/usr/bin/env python3
# prof.py ADDR [ADDR...] - size / instructions / branches / calls / push-immediates /
# EH-prologue for each function start, read off scratchpad/text.asm. Rank unmatched
# functions by CALL and push-imm density (the "table of calls" vein), NOT by low
# branch density - that finds float kernels, which are the opposite of cheap.
import os
import re,sys
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pat=re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
insns=[]
for l in open(ROOT+'/scratchpad/text.asm'):
    m=pat.match(l)
    if m: insns.append((int(m.group(1),16),m.group(2),m.group(3).strip()))
idx={a:i for i,(a,_,_) in enumerate(insns)}
BR=set('je jne jz jnz jl jle jg jge jb jbe ja jae js jns jo jno jp jnp jmp'.split())
for arg in sys.argv[1:]:
    lo=int(arg,16); i=idx[lo]; n=0; br=0; calls=[]; pushimm=0; eh=False; strs=0
    while i<len(insns):
        a,mn,rest=insns[i]
        if mn=='int3': break
        n+=1
        if mn in BR: br+=1
        if mn=='call': calls.append(rest)
        if mn=='push' and re.match(r'^0x',rest): pushimm+=1
        if 'fs:0x0' in rest: eh=True
        i+=1
    end=insns[i][0] if i<len(insns) else lo
    print("0x%06x  %5d bytes  %4d insns  %3d br  %.3f br/i  %3d calls  %3d pushimm  EH=%s"%(lo,end-lo,n,br,br/max(n,1),len(calls),pushimm,eh))
