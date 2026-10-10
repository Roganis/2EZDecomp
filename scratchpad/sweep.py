import subprocess, os, sys
def score(src, tag):
    f='scratchpad/zoo/quad_p.cpp'; open(f,'w').write(src)
    r=subprocess.run(['./scratchpad/zoo.sh',f],stdout=open('scratchpad/quad_p.asm','w'),stderr=subprocess.PIPE)
    if r.returncode: print(tag,'BUILD FAIL'); return 0,0
    e=dict(os.environ,ZOO_ASM='scratchpad/quad_p.asm')
    a=subprocess.run(['python3','scratchpad/quaddiff.py','0'],capture_output=True,text=True,env=e).stdout.split('\n')[0]
    b=subprocess.run(['python3','scratchpad/quaddiff_nx.py','0'],capture_output=True,text=True,env=e).stdout.split('\n')[0]
    pa=float(a.split('(')[1].split('%')[0]); pb=float(b.split('(')[1].split('%')[0])
    print("%-28s %6.2f  blind %6.2f"%(tag,pa,pb)); return pa,pb
