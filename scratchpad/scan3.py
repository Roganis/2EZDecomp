import re,struct,os
S=os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
pat=re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
ins=[]
for l in open(S+'/text.asm'):
    m=pat.match(l)
    if m: ins.append((int(m.group(1),16), m.group(2), m.group(3).strip()))
addr2i={a:i for i,(a,_,_) in enumerate(ins)}
d=open(ROOT+'/original/EZ2AC_Unpacked.exe','rb').read()
starts=set()
for a,mn,rest in ins:
    if mn=='call' and re.match(r'^0x[0-9a-f]+$',rest): starts.add(int(rest,16))
for va in range(0x478000,0x4b1000,4):
    v=struct.unpack('<I', d[va-0x400000:va-0x400000+4])[0]
    if 0x401000<=v<0x478000: starts.add(v)
ann=set()
for f in os.listdir(ROOT+'/src'):
    if f.endswith('.cpp'):
        for m in re.finditer(r'(?:FUNCTION|STUB|LIBRARY|VTABLE|GLOBAL):\s+EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)', open(ROOT+'/src/'+f).read()):
            ann.add(int(m.group(1),16))
starts |= ann
starts=sorted(a for a in starts if 0x401000<=a<0x478000 and a in addr2i)
res=[]
for i,s in enumerate(starts):
    nxt=starts[i+1] if i+1<len(starts) else 0x478000
    j=addr2i[s]; body=[]
    while j<len(ins) and ins[j][0]<nxt:
        if ins[j][1] in ('int3','(bad)'): break
        body.append(ins[j]); j+=1
    if not body: continue
    end=ins[j][0] if j<len(ins) else nxt
    # destructor tell: stores a vtable constant into [reg] near the top,
    # and the LAST call is 0x402880 or 0x426e40
    vt=None
    for a,mn,r in body[:12]:
        m=re.match(r'^DWORD PTR \[e[a-z][a-z]\],(0x4[0-9a-f]{5})$', r)
        if mn=='mov' and m: vt=int(m.group(1),16)
    calls=[int(r,16) for a,mn,r in body if mn=='call' and re.match(r'^0x[0-9a-f]+$',r)]
    if vt and calls and calls[-1] in (0x402880,0x426e40):
        res.append((s, end-s, vt, len(body), s in ann))
res.sort(key=lambda r:-r[1])
print("%-10s %6s %-10s %5s  state" % ("dtor","bytes","vtable","insns"))
for s,sz,vt,n,done in res:
    print("0x%06x %6d 0x%06x %5d  %s" % (s,sz,vt,n,"DONE" if done else ""))
