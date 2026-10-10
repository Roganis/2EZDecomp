import json,re,glob,subprocess,sys
# usage: python3 scratchpad/crtnames.py RECCMP.json > crt.txt; then crtappend.py crt.txt
lib=set()
for f in glob.glob('src/*.cpp'):
    lib|=set(int(m,16) for m in re.findall(r'// LIBRARY: EZ2AC_UNPACKED (0x[0-9a-f]+)',open(f,encoding='latin-1').read()))
def ins(a):
    out=subprocess.run(['objdump','-D','-Mintel','--no-show-raw-insn','-b','binary','-m','i386','--adjust-vma=0x400000','--start-address=%d'%a,'--stop-address=%d'%(a+16),'original/EZ2AC_Unpacked.exe'],capture_output=True,text=True).stdout
    for l in out.split('\n'):
        m=re.match(r'\s*([0-9a-f]+):\s+(.*)',l)
        if m and int(m.group(1),16)==a: return m.group(2)
res={}
for e in json.load(open(sys.argv[1]))['data']:
    if int(e['address'],16) not in lib or (e.get('matching') or 0)>=1: continue
    for h in e['diff'] or []:
        for blk in h[1]:
            for x,y in zip(blk.get('orig',[]),blk.get('recomp',[])):
                m=re.search(r'([\w:~`\' ]+) \((FUNCTION|DATA)\)',y[1])
                if not m or x[1].split()[0]!=y[1].split()[0]: continue
                if '-' in y[1].split('(')[0].split('+')[-1]: continue
                name,kind=m.group(1).strip(),m.group(2)
                i=ins(int(x[0],16))
                t=re.findall(r'0x([0-9a-f]{6,8})',i)
                if not t: continue
                res.setdefault((name,kind),set()).add(int(t[-1],16))
for (n,k),v in sorted(res.items()):
    print(k,n,' '.join(hex(a) for a in v))
