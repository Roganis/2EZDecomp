import re,struct,os
S=os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
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
ann={}
for f in sorted(os.listdir(ROOT+'/src')):
    if f.endswith('.cpp'):
        txt=open(ROOT+'/src/'+f).read()
        for m in re.finditer(r'(?:FUNCTION|STUB|LIBRARY):\s+EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)', txt):
            ann[int(m.group(1),16)]=f
starts |= set(ann)
starts=sorted(a for a in starts if 0x401000<=a<0x478000 and a in addr2i)
info={}
for i,s in enumerate(starts):
    nxt=starts[i+1] if i+1<len(starts) else 0x478000
    j=addr2i[s]; calls=[]; n=0
    while j<len(ins) and ins[j][0]<nxt:
        a,mn,rest=ins[j]
        if mn in ('int3','(bad)'): break
        n+=1
        if mn=='call' and re.match(r'^0x[0-9a-f]+$',rest): calls.append(int(rest,16))
        j+=1
    end=ins[j][0] if j<len(ins) else nxt
    info[s]=(end-s, n, calls)
return_ = None
def show(title, pred):
    rows=[(s,)+info[s] for s in starts if s not in ann and pred(s, info[s])]
    rows.sort(key=lambda r:-r[1])
    print("\n== %s == (%d)" % (title, len(rows)))
    for s,sz,n,c in rows[:30]:
        print("  0x%06x  %5d bytes %4d insns" % (s,sz,n))
show("unmatched fns calling EzObject::EzObject 0x401690 (ctors)", lambda s,v: 0x401690 in v[2])
show("unmatched fns calling ehvec dtor iterator 0x47a12b (dtors w/ arrays)", lambda s,v: 0x47a12b in v[2])
show("unmatched fns calling ehvec ctor iterator 0x47a06b", lambda s,v: 0x47a06b in v[2])
