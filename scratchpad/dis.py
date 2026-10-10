import sys,re
import os
S=os.path.dirname(os.path.abspath(__file__))
lo=int(sys.argv[1],16); hi=int(sys.argv[2],16)
pat=re.compile(r'^\s*([0-9a-f]{6}):\t')
for l in open(S+'/text.asm'):
    m=pat.match(l)
    if m:
        a=int(m.group(1),16)
        if lo<=a<hi: sys.stdout.write(l)
