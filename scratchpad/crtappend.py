import sys,re

have=set(int(a,16) for a in re.findall(r'// (?:LIBRARY|GLOBAL|FUNCTION|STUB): EZ2AC_UNPACKED (0x[0-9a-f]+)',open('src/library.cpp').read()))
import glob
for f in glob.glob('src/*.cpp'):
    have|=set(int(a,16) for a in re.findall(r'// (?:LIBRARY|GLOBAL|FUNCTION|STUB): EZ2AC_UNPACKED (0x[0-9a-f]+)',open(f,encoding='latin-1').read()))
names=set(re.findall(r'^// (\S+)$',open('src/library.cpp').read(),re.M))
skip={'LeadUpVec','TrailUpVec','UnwindUpVec','S1'}
out=[]
for l in open(sys.argv[1]).read().split('\n'):
    if not l: continue
    p=l.split(); kind=p[0]; addr=int(p[-1],16)
    name=' '.join(p[1:-1])
    for pre in ('call ','jmp ','push '): 
        if name.startswith(pre): name=name[len(pre):]
    if name.isdigit() or name in skip or addr in have or name in names or ' ' in name or '::' in name: 
        if '::' in name: print('SKIP',name)
        continue
    have.add(addr); names.add(name)
    mk='LIBRARY' if kind=='FUNCTION' else 'GLOBAL'
    out+=['','// %s: EZ2AC_UNPACKED 0x%08x'%(mk,addr),'// '+name]
open('src/library.cpp','a').write('\n'.join(out)+'\n')
print(len(out)//3,'added')
