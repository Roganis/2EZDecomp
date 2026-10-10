import json,sys,re,struct,bisect,collections
import os, sys
# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
# mnscan.py JSON : per sub-100 function, compare the COUNTS of rarely-noisy
# mnemonics (movsx/movzx, div/idiv/mul, shifts, float ops, calls) between the
# original and our build. A count that differs is a cheap pointer at a wrong
# operation: a signed divide that should be unsigned, a loop that swaps three
# dwords too many, a draw call through the wrong primitive.
# Usage: RCJSON_DIR=<dir of the reccmp --json> .venv/bin/python scratchpad/mnscan.py b.json
exec(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'jt3.py')).read().split('res=[]')[0])
KEY={'and','or','xor','lea','sbb','adc','movsx','movzx','imul','idiv','div','mul','sar','shr','shl','neg','not','cdq','sete','setne','setg','setl','cvttss2si','cvtsi2ss','sqrtss','divss','mulss','addss','subss','fld','fstp','fistp','call','ret'}
oa=sorted(int(x['address'],16) for x in d)
for e in d:
    m=e.get('matching')
    if m is None or m>=1 or not e.get('recomp'): continue
    a=int(e['address'],16); r=int(e['recomp'],16)
    if a>=0x4784e0: continue
    k=bisect.bisect_right(syms,r); size=(syms[k]-r) if k<len(syms) else 0x400
    R=dis(rr,r,size)
    while R and R[-1].mnemonic=='int3': R.pop()
    nx=oa[bisect.bisect_right(oa,a)] if bisect.bisect_right(oa,a)<len(oa) else a+size+0x400
    O=trim(dis(ro,a,min(size+0x400,nx-a)))
    co=collections.Counter(i.mnemonic for i in O if i.mnemonic in KEY)
    cr=collections.Counter(i.mnemonic for i in R if i.mnemonic in KEY)
    diff={x:(co[x],cr[x]) for x in KEY if co[x]!=cr[x] and x not in ('ret',)}
    if diff: print(e['address'],round(m*100,2),e['name'],diff)
