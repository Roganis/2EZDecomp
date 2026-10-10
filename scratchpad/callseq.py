import os, sys
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
# callseq.py JSON ADDR : the ordered call-target names (plus pushed strings) of
# one function on both sides, aligned - where a call is missing or extra.
import difflib,re
sys.argv=[sys.argv[0],sys.argv[1],sys.argv[2]]
src=open(os.path.join(os.path.dirname(os.path.abspath(__file__)),'semscan.py')).read()
exec(src[:src.index('out=[]')])
from capstone.x86 import X86_OP_IMM
def seq(rd,a,size,fn,gt,gk,lo,hi):
    out=[]
    for i in md.disasm(rd(a,size) or b'',a):
        if i.mnemonic=='call' and i.operands[0].type==X86_OP_IMM:
            out.append(('%x '%(i.address-a))+'call '+fn.get(i.operands[0].imm,'%x'%i.operands[0].imm))
        elif i.mnemonic=='push' and i.operands[0].type==X86_OP_IMM:
            v=i.operands[0].imm&0xffffffff
            if lo<=v<hi:
                t=cstr(rd,v)
                if t: out.append(('%x '%(i.address-a))+'"%s"'%t)
    return out
a=only; e=[x for x in d if int(x['address'],16)==a][0]; r=int(e['recomp'],16)
k=bisect.bisect_right(syms,r); rs=(syms[k]-r) if k<len(syms) else 0x400
k=bisect.bisect_right(oa,a); osz=(oa[k]-a) if k<len(oa) else 0x400
O=seq(ro,a,osz,fn_o,glob_o,go,0x401000,0x4000000); R=seq(rr,r,rs,fn_r,glob_r,gr,0x11000000,0x13000000)
on=[x.split(' ',1)[1] for x in O]; rn=[x.split(' ',1)[1] for x in R]
rn=[re.sub(r'^call [0-9a-f]+$','call ?',x) for x in rn]
for tag,i1,i2,j1,j2 in difflib.SequenceMatcher(None,on,rn,autojunk=False).get_opcodes():
    if tag=='equal': continue
    print(tag); [print('  < '+x) for x in O[i1:i2]]; [print('  > '+x) for x in R[j1:j2]]
