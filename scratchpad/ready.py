import re,os
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LO,HI=0x401000,0x478000
pat=re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
insns=[]
for l in open(ROOT+'/scratchpad/text.asm'):
    m=pat.match(l)
    if m: insns.append((int(m.group(1),16),m.group(2),m.group(3).strip()))
idx={a:i for i,(a,_,_) in enumerate(insns)}
starts=set()
for a,mn,rest in insns:
    if mn=='call':
        m=re.match(r'^0x([0-9a-f]+)$',rest)
        if m and LO<=int(m.group(1),16)<HI: starts.add(int(m.group(1),16))
ann=set(); stub=set()
for root,_,files in os.walk(ROOT+'/src'):
    for f in files:
        if not f.endswith(('.cpp','.h')): continue
        for line in open(os.path.join(root,f),errors='ignore'):
            m=re.search(r'//\s*(FUNCTION|STUB):\s*EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)',line)
            if m: (ann if m.group(1)=='FUNCTION' else stub).add(int(m.group(2),16))
starts |= ann | stub
rows=[]
for lo in sorted(starts):
    if lo not in idx or lo in ann: continue
    i=idx[lo]; n=0; eh=False; callees=[]
    while i<len(insns):
        a,mn,rest=insns[i]
        if mn=='int3': break
        if a!=lo and a in starts: break
        n+=1
        if mn=='call':
            m=re.match(r'^0x([0-9a-f]+)$',rest)
            if m: callees.append(int(m.group(1),16))
        if 'fs:0x0' in rest: eh=True
        i+=1
    end=insns[i][0] if i<len(insns) else lo
    size=end-lo
    if eh or not (150<=size<=700) or n<20: continue
    uniq=set(callees)
    known=sum(1 for c in uniq if c in ann)
    unknown=[c for c in uniq if c not in ann and LO<=c<HI]
    ready = known/len(uniq) if uniq else 1.0
    rows.append((len(unknown), -ready, size, n, len(uniq), known, lo))
rows.sort()
print("%-10s %5s %5s %6s %6s %8s"%("addr","bytes","insns","callees","known","unknown"))
for unk,nr,size,n,tot,known,lo in rows[:20]:
    print("0x%06x %5d %5d %6d %6d %8d"%(lo,size,n,tot,known,unk))
