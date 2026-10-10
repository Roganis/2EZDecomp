# zd2.py OBJ SYMBOL-SUBSTR ADDR [N]: capstone diff of one COMDAT function in a
# zoo .obj against the original at ADDR. Relocated operands on our side and
# image addresses on the original's are masked; jump/call targets compared
# as function-relative offsets (calls masked).
import sys,struct,re,difflib
import os
sys.path=[p for p in sys.path if os.path.abspath(p or '.') != os.path.dirname(os.path.abspath(__file__))]
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
md=Cs(CS_ARCH_X86,CS_MODE_32)
obj=open(sys.argv[1],'rb').read(); want=sys.argv[2]; addr=int(sys.argv[3],16)
nshow=int(sys.argv[4]) if len(sys.argv)>4 else 40
mach,nsec,ts,symptr,nsym,opt,ch=struct.unpack('<HHIIIHH',obj[:20])
secs=[]
for i in range(nsec):
    h=obj[20+opt+40*i:20+opt+40*i+40]
    name=h[:8];vs,va,rs,rp,relp,lnp,nrel,nln,chs=struct.unpack('<IIIIIIHHI',h[8:])
    secs.append((rs,rp,relp,nrel))
strtab=symptr+18*nsym
def symname(i):
    e=obj[symptr+18*i:symptr+18*i+18]
    if e[:4]==b'\0\0\0\0':
        off=struct.unpack('<I',e[4:8])[0]; s=obj[strtab+off:obj.index(b'\0',strtab+off)]
    else: s=e[:8].rstrip(b'\0')
    return s.decode('latin1')
sec=None;i=0
while i<nsym:
    e=obj[symptr+18*i:symptr+18*i+18]
    val,sn,typ,cls,naux=struct.unpack('<IhHBB',e[8:])
    n=symname(i)
    if want in n and sn>0 and cls==2 and (typ>>4)==2:
        sec=sn-1; fval=val; break
    i+=1+naux
rs,rp,relp,nrel=secs[sec]
code=obj[rp:rp+rs]
rel=set()
for k in range(nrel):
    va,si,t=struct.unpack('<IIH',obj[relp+10*k:relp+10*k+10])
    rel.add(va)
def norm(ins,base,relocs):
    m=ins.mnemonic; o=ins.op_str
    if m=='call': return 'call X'
    if m.startswith('j') or m=='loop':
        try: return '%s +%x'%(m,int(o,16)-base)
        except: return m+' '+o
    if relocs is not None:
        if any(ins.address<=r<ins.address+ins.size for r in relocs):
            o=re.sub(r'0x[0-9a-f]+|\b\d+\b(?![\]x])', lambda mm: 'X' if (int(mm.group(0),0)==0 or True) else mm.group(0), o) if False else re.sub(r'(?<![+\-*])\b0x0\b|\b0\b','X',o)
    else:
        o=re.sub(r'0x([0-9a-f]+)',lambda mm:'X' if 0x401000<=int(mm.group(1),16)<0x4000000 else mm.group(0),o)
    return m+' '+o
R=[norm(i,fval,rel) for i in md.disasm(code[fval:],fval)]
# original
b=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','original','EZ2AC_Unpacked.exe'),'rb').read()
O=[]
for i in md.disasm(b[addr-0x400000:addr-0x400000+len(code)+0x40],addr):
    O.append(norm(i,addr,None))
    if len(O)>=len(R): break
d=[l for l in difflib.unified_diff(O,R,lineterm='',n=0) if not l.startswith(('---','+++'))]
print('mine=%d diff lines: %d'%(len(R),len([l for l in d if l[:1] in '+-'])))
for l in d[:nshow]: print(l)
