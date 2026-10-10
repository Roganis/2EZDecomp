#!/usr/bin/env python3
"""scan4.py - EH state-store sequences per function.

Collects `mov <size> ptr [esp+X], imm` stores whose value is small or -1 (cl's
EH state slot) and prints the sequence per function, flagging the ones that
ASCEND before the final -1. Ascending is the shape ~Obj40EAC0 @0x40eae0 needs
and that no ordinary member destruction produces."""
import re, os, struct
S=os.path.dirname(os.path.abspath(__file__)); ROOT=os.path.dirname(S)
pat=re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
ins=[]
for l in open(S+'/text.asm'):
    m=pat.match(l)
    if m: ins.append((int(m.group(1),16), m.group(2), m.group(3).strip()))
addr2i={a:i for i,(a,_,_) in enumerate(ins)}
d=open(ROOT+'/original/EZ2AC_Unpacked.exe','rb').read()
starts=set()
for a,mn,r in ins:
    if mn=='call' and re.match(r'^0x[0-9a-f]+$',r): starts.add(int(r,16))
for va in range(0x478000,0x4b1000,4):
    v=struct.unpack('<I', d[va-0x400000:va-0x400000+4])[0]
    if 0x401000<=v<0x478000: starts.add(v)
starts=sorted(a for a in starts if 0x401000<=a<0x478000 and a in addr2i)
STORE=re.compile(r'^(?:DWORD|BYTE) PTR \[esp(?:\+0x[0-9a-f]+)?\],(0x[0-9a-f]+|0xffffffff)$')
out=[]
for i,s in enumerate(starts):
    nxt=starts[i+1] if i+1<len(starts) else 0x478000
    j=addr2i[s]; vals=[]
    while j<len(ins) and ins[j][0]<nxt:
        a,mn,r=ins[j]
        if mn in ('int3','(bad)'): break
        if mn=='mov':
            m=STORE.match(r)
            if m:
                v=int(m.group(1),16)
                if v==0xffffffff: v=-1
                if -1<=v<=0x40: vals.append(v)
        j+=1
    if len(vals)>=2 and vals[-1]==-1:
        body=vals[:-1]
        if len(body)>=2 and all(body[k]<body[k+1] for k in range(len(body)-1)):
            out.append((s,vals))
print("ASCENDING EH-state functions (%d):" % len(out))
for s,v in out: print("  0x%06x  %s" % (s,v))
