import os, sys
# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
# semscan.py JSON [addr] : register-free content comparison of every sub-100
# function - call targets BY NAME, global references BY NAME, member
# displacements and small immediates - as multisets, so register choice,
# scheduling and layout drop out and what is left is a wrong constant, a
# wrong field, a missing/extra call or a wrong global. Ranked by size.
import json,re,struct,bisect,collections,glob
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
from capstone.x86 import X86_OP_IMM,X86_OP_MEM
S=os.environ.get('RCJSON_DIR','scratchpad')
md=Cs(CS_ARCH_X86,CS_MODE_32); md.detail=True
d=json.load(open(S+'/'+sys.argv[1]))['data']
only=int(sys.argv[2],16) if len(sys.argv)>2 else None
src=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'jt3.py')).read()
exec(src[src.index('def load'):src.index('def dis')])
fn_o={int(e['address'],16):e['name'] for e in d}
fn_r={int(e['recomp'],16):e['name'] for e in d if e.get('recomp')}
# globals: original address -> name from the GLOBAL markers; recomp -> name from the map
glob_o={}
for f in glob.glob('src/*.cpp')+glob.glob('include/*.h'):
    L=open(f,encoding='latin-1').read().split('\n')
    for k,l in enumerate(L):
        m=re.match(r'\s*//\s*GLOBAL:\s*EZ2AC_UNPACKED\s+(0x[0-9a-fA-F]+)',l)
        if m and k+1<len(L):
            n=re.findall(r'([A-Za-z_]\w*)\s*(?:\[|=|;|\()',L[k+1])
            if n: glob_o[int(m.group(1),16)]=n[0]
glob_r={}
for l in open('build/EZ2AC_Unpacked.map',encoding='latin-1'):
    m=re.match(r'\s*000[2-9]:[0-9a-f]+\s+(\S+)\s+([0-9a-f]{8})\s',l)
    if m:
        nm=m.group(1); mm=re.match(r'\?(\w+)@@',nm) or re.match(r'_(\w+)$',nm)
        glob_r[int(m.group(2),16)]=mm.group(1) if mm else nm
go=sorted(glob_o); gr=sorted(glob_r)
def gname(a,tab,keys):
    if a in tab: return tab[a]
    k=bisect.bisect_right(keys,a)-1
    if k>=0 and a-keys[k]<0x1000: return '%s+%x'%(tab[keys[k]],a-keys[k])
    return None
FL=re.compile(r'^(movss|movsd|comiss|ucomiss|comisd|addss|subss|mulss|divss|addsd|subsd|mulsd|divsd|minss|maxss|cvtss2sd|cvtsd2ss|f\w+)$')
def cstr(rd,v):
    b=rd(v,256) or b''
    z=b.find(b'\0')
    if z>=2 and all(32<=x<127 or x in (9,10,13) for x in b[:z]): return b[:z].decode().replace('\n','\\n')
    return None
def toks(rd,a,size,fn,gt,gk,lo,hi,blo,side):
    c=collections.Counter()
    code=rd(a,size) or b''
    for i in md.disasm(code,a):
        if i.mnemonic=='int3': continue
        if i.mnemonic=='call':
            op=i.operands[0]
            if op.type==X86_OP_IMM:
                if op.imm in fn: c['call '+fn[op.imm]]+=1
            else:
                c['call*']+=1
                if op.type==X86_OP_MEM and op.mem.base!=0 and op.mem.index==0:
                    c['vcall %#x'%(op.mem.disp&0xffffffff)]+=1
            continue
        if i.mnemonic.startswith('j'): continue
        for op in i.operands:
            if op.type==X86_OP_IMM:
                v=op.imm & 0xffffffff
                if lo<=v<hi:
                    if v in fn: c['&'+fn[v]]+=1
                    else:
                        n=gname(v,gt,gk)
                        if n is None or n.startswith('??_C@') or '+' in n:
                            t=cstr(rd,v); n='"%s"'%t if t else (n if n and not n.startswith('??_C@') else 'ADDR')
                        c['imm '+n]+=1
                elif v<0x10000 or v>0xffff0000: c['imm %#x'%v]+=1
            elif op.type==X86_OP_MEM:
                disp=op.mem.disp & 0xffffffff
                if lo<=disp<hi and (op.mem.base==0 or disp>=blo):
                    n=gname(disp,gt,gk)
                    if n is None or n.startswith('__real') or '+' in n:
                        if FL.match(i.mnemonic) and op.mem.index==0 and RD[side][0]<=disp<RD[side][1]:
                            sz=op.size or 4
                            raw=rd(disp,sz) or b''
                            if len(raw)==8: n='%r'%struct.unpack('<d',raw)[0]
                            elif len(raw)==4: n='%r'%struct.unpack('<f',raw)[0]
                            c['f '+n]+=1; continue
                    if n and n.startswith('??_C@'):
                        t=cstr(rd,disp); n='"%s"'%t if t else n
                    c['g '+(n or 'ADDR')]+=1
                elif op.mem.base!=0 and md.reg_name(op.mem.base) not in ('esp',) and disp:
                    c['d %#x'%disp]+=1
    return c
def rdata(path):
    b=open(path,'rb').read(); pe=struct.unpack('<I',b[0x3c:0x40])[0]
    base=struct.unpack('<I',b[pe+0x34:pe+0x38])[0]
    n=struct.unpack('<H',b[pe+6:pe+8])[0]; so=pe+24+struct.unpack('<H',b[pe+20:pe+22])[0]
    for i in range(n):
        h=b[so+40*i:so+40*i+40]
        if h[:6]==b'.rdata': vs,va=struct.unpack('<II',h[8:16]); return (base+va,base+va+vs)
RD={}
RD['o']=rdata('original/EZ2AC_Unpacked.exe'); RD['r']=rdata('build/EZ2AC_Unpacked.exe')
syms=[]
for l in open('build/EZ2AC_Unpacked.map',encoding='latin-1'):
    m=re.match(r'\s*0001:[0-9a-f]+\s+\S+\s+([0-9a-f]{8})\s',l)
    if m: syms.append(int(m.group(1),16))
syms=sorted(set(syms))
oa=sorted(fn_o)
out=[]
for e in d:
    m=e.get('matching')
    if m is None or not e.get('recomp'): continue
    a=int(e['address'],16); r=int(e['recomp'],16)
    if only is not None and a!=only: continue
    if only is None and m>=1: continue
    if a>=0x4784e0: continue
    k=bisect.bisect_right(syms,r); rs=(syms[k]-r) if k<len(syms) else 0x400
    k=bisect.bisect_right(oa,a); osz=(oa[k]-a) if k<len(oa) else 0x400
    O=toks(ro,a,osz,fn_o,glob_o,go,0x401000,0x4000000,0x480000,'o')
    R=toks(rr,r,rs,fn_r,glob_r,gr,0x11000000,0x13000000,0x11000000,'r')
    # the original's span can run into unannotated neighbours: only report tokens
    # whose counts differ when both sides have them, or that recomp has and orig lacks
    diff=[]
    tokonly=os.environ.get('ONLY')   # e.g. ONLY=vcall: report just that token kind
    for t in sorted(set(O)|set(R)):
        if tokonly and not t.startswith(tokonly): continue
        if O[t]!=R[t]: diff.append((t,O[t],R[t]))
    NOISE=('imm ADDR','g ADDR','&Log','call KBase::KBase','g __security_cookie','g g_info_4afc34+6c','call*')
    score=sum(abs(x-y) for _,x,y in diff if not _.startswith(NOISE))
    out.append((score,a,m,e['name'],diff))
out.sort(key=lambda t:-t[0])
for score,a,m,n,diff in out[: (None if only else 60)]:
    print('%4d %x %6.2f %s'%(score,a,m*100,n))
    if only or score:
        for t,x,y in diff[:int(os.environ.get("NDIFF",40)) if only else 8]: print('      %-40s %3d %3d'%(t,x,y))
