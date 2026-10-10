import os, sys
# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
# jctx.py JSON ADDR : full side-by-side listing (orig | recomp) of a function, aligned
import json,sys,re,struct,difflib,bisect
exec(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'jt3.py')).read().split('res=[]')[0])
a=int(sys.argv[2],16)
e=[x for x in d if int(x['address'],16)==a][0]; r=int(e['recomp'],16)
k=bisect.bisect_right(syms,r); size=(syms[k]-r) if k<len(syms) else 0x400
R=dis(rr,r,size)
while R and R[-1].mnemonic=='int3': R.pop()
O=trim(dis(ro,a,size+0x400))
on=[norm(i) for i in O]; rn=[norm(i) for i in R]
sm=difflib.SequenceMatcher(None,on,rn,autojunk=False)
def f(i,base): 
    t=i.mnemonic+' '+i.op_str
    if JM.match(i.mnemonic):
        try: t=i.mnemonic+' +%x'%(int(i.op_str,16)-base)
        except: pass
    return '%4x %s'%(i.address-base,t)
for tag,i1,i2,j1,j2 in sm.get_opcodes():
    n=max(i2-i1,j2-j1)
    for t in range(n):
        x=f(O[i1+t],a) if i1+t<i2 else ''
        y=f(R[j1+t],r) if j1+t<j2 else ''
        print(('  ' if tag=='equal' else '* ')+x.ljust(55)+'| '+y)
