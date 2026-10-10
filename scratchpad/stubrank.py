import re,subprocess,os,sys
# collect STUB addresses from src/
stubs={}
for f in os.listdir('src'):
    if not f.endswith('.cpp'): continue
    L=open('src/'+f).read().split('\n')
    for i,l in enumerate(L):
        m=re.search(r'//\s*STUB:\s*EZ2AC_UNPACKED\s+(0x[0-9a-fA-F]+)',l)
        if m:
            nm=''
            for j in range(i+1,min(i+4,len(L))):
                if L[j].strip() and not L[j].strip().startswith('//'):
                    nm=L[j].strip(); break
            stubs[int(m.group(1),16)]=(f,nm)
# disassemble the whole text once
out=subprocess.run(['objdump','-D','-b','binary','-m','i386','--adjust-vma=0x400000',
  '--start-address=0x401000','--stop-address=0x478000','original/EZ2AC_Unpacked.exe'],
  capture_output=True,text=True).stdout
ins={}
order=[]
for l in out.split('\n'):
    m=re.match(r'\s*([0-9a-f]+):\t([0-9a-f ]+)\t(.*)',l)
    if m:
        a=int(m.group(1),16); ins[a]=m.group(3).strip(); order.append(a)
order.sort()
import bisect
res=[]
for a,(f,nm) in stubs.items():
    i=bisect.bisect_left(order,a)
    if i>=len(order) or order[i]!=a: continue
    n=0; eh=False; txt=[]
    j=i
    while j<len(order) and n<200:
        t=ins[order[j]]
        if t.startswith('int3'): break
        txt.append(t); n+=1
        if 'fs:0x0' in t or '%fs:0x0' in t: eh=True
        j+=1
    res.append((n,eh,a,f,nm))
res.sort()
for n,eh,a,f,nm in res:
    if n<=45 and not eh:
        print('%3d  0x%06x  %-28s %s'%(n,a,f,nm[:60]))
print('total small no-EH:',sum(1 for n,eh,a,f,nm in res if n<=45 and not eh))
