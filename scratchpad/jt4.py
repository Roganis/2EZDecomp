import os, sys
# scratchpad/dis.py would shadow the stdlib `dis` capstone imports
sys.path = [p for p in sys.path if os.path.abspath(p or ".") != os.path.dirname(os.path.abspath(__file__))]
import sys,re
src=open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'jt3.py')).read()
src=src.replace("bad.append((hex(io.address),io.mnemonic,norm(O[oidx[to]]),norm(R[ridx[tr]]),norm(R[mo])))",
"""def rm(t): return re.sub(r'\\b(e[a-d]x|e[sd]i|ebp|xmm\\d|[a-d][lh])\\b','R',re.sub(r'esp \\+ 0x[0-9a-f]+','S',t))
        if rm(norm(O[oidx[to]]))==rm(norm(R[ridx[tr]])): continue
        ctx=lambda L,k,base: ' ; '.join('%x:%s'%(i.address-base,norm(i)) for i in L[max(k-1,0):k+3])
        bad.append(('J %x %s'%(io.address-a,io.mnemonic),'O->'+ctx(O,oidx[to],a),'R->'+ctx(R,ridx[tr],r),'R@aligned->'+ctx(R,mo,r)))""")
src=src.replace("    for x in bad[:3]: print('    ',x)","    for x in bad[:4]:\n        for y in x: print('     ',y)\n        print()")
exec(src)
