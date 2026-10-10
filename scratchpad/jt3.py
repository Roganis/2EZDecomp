import os, sys
# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
# jt3.py JSON [addr] : whole-function jump-target comparison (capstone + difflib alignment)
import json,sys,re,struct,difflib,bisect
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
import os; S=os.environ.get('RCJSON_DIR', 'scratchpad')
md=Cs(CS_ARCH_X86,CS_MODE_32)
d=json.load(open(S+'/'+sys.argv[1]))['data']
only=int(sys.argv[2],16) if len(sys.argv)>2 else None
def load(path):
    b=open(path,'rb').read(); pe=struct.unpack('<I',b[0x3c:0x40])[0]
    base=struct.unpack('<I',b[pe+0x34:pe+0x38])[0]
    n=struct.unpack('<H',b[pe+6:pe+8])[0]; so=pe+24+struct.unpack('<H',b[pe+20:pe+22])[0]
    secs=[]
    for i in range(n):
        s=b[so+40*i:so+40*i+40]; vs,va,rs,ro=struct.unpack('<IIII',s[8:24]); secs.append((base+va,max(vs,rs),ro))
    def rd(a,n):
        for va,vs,ro in secs:
            if va<=a<va+vs: return b[a-va+ro:a-va+ro+n]
    return rd
ro=load('original/EZ2AC_Unpacked.exe'); rr=load('build/EZ2AC_Unpacked.exe')
# recomp function sizes from the map
syms=[]
for l in open('build/EZ2AC_Unpacked.map',encoding='latin-1'):
    m=re.match(r'\s*0001:[0-9a-f]+\s+\S+\s+([0-9a-f]{8})\s',l)
    if m: syms.append(int(m.group(1),16))
syms=sorted(set(syms))
def dis(rd,a,n):
    code=rd(a,n) or b''
    out=[]
    for i in md.disasm(code,a):
        out.append(i)
    return out
JM=re.compile(r'^j')
def norm(i):
    if JM.match(i.mnemonic) or i.mnemonic=='call': return i.mnemonic
    return i.mnemonic+' '+re.sub(r'0x[0-9a-f]{5,}','A',i.op_str)
def trim(L):
    # cut at the last ret before a run of int3 / end, keeping all jump targets
    maxt=0; end=len(L)
    for k,i in enumerate(L):
        if i.mnemonic=='int3' and k>0 and L[k-1].mnemonic in ('ret','jmp') :
            # stop if no earlier jump targets beyond here
            if maxt<=i.address: end=k; break
        if JM.match(i.mnemonic):
            try:
                t=int(i.op_str,16)
                if t>i.address: maxt=max(maxt,t)
            except: pass
    return L[:end]
res=[]
for e in d:
    m=e.get('matching')
    if m is None or not e.get('recomp'): continue
    if only is None and m>=1: continue
    a=int(e['address'],16); r=int(e['recomp'],16)
    if only and a!=only: continue
    if a>=0x4784e0: continue
    k=bisect.bisect_right(syms,r); size=(syms[k]-r) if k<len(syms) else 0x400
    R=dis(rr,r,size)
    while R and R[-1].mnemonic=='int3': R.pop()
    O=trim(dis(ro,a,size+0x400))
    on=[norm(i) for i in O]; rn=[norm(i) for i in R]
    sm=difflib.SequenceMatcher(None,on,rn,autojunk=False)
    o2r={}
    for tag,i1,i2,j1,j2 in sm.get_opcodes():
        if tag=='equal' or (tag=='replace' and i2-i1==j2-j1):
            for t in range(i2-i1): o2r[i1+t]=j1+t
    oidx={i.address:k for k,i in enumerate(O)}; ridx={i.address:k for k,i in enumerate(R)}
    bad=[]
    for oi,ri in o2r.items():
        io=O[oi]; ir=R[ri]
        if not (JM.match(io.mnemonic) and io.mnemonic==ir.mnemonic): continue
        try: to=int(io.op_str,16); tr=int(ir.op_str,16)
        except: continue
        if to not in oidx or tr not in ridx: continue
        mo=o2r.get(oidx[to])
        if mo is None or mo==ridx[tr]: continue
        # tolerate when both targets begin an epilogue-ish run (pop/ret/add esp/mov ecx,[esp..] cookie)
        def epi(L,k):
            for i in L[k:k+4]:
                if i.mnemonic in ('pop','ret','leave') : continue
                if i.mnemonic=='add' and i.op_str.startswith('esp'): continue
                return False
            return True
        if epi(O,oidx[to]) and epi(R,ridx[tr]): continue
        bad.append((hex(io.address),io.mnemonic,norm(O[oidx[to]]),norm(R[ridx[tr]]),norm(R[mo])))
    if bad:
        res.append((e['address'],round(m*100,2),e['name'],bad))
for a,m,n,bad in res:
    print(a,m,n,len(bad))
    for x in bad[:3]: print('    ',x)
