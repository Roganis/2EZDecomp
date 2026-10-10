"""iniref.py DIR : key-based reading of every chart .ini in DIR/list.txt (decrypted
with decini.lua), dumped in ftini's format, then compared with ftini's dumps.
Sections and keys are matched case-insensitively, like ../EZ2REWRITE's ini.lua;
a missing key or section reads as 0 (the harness zeroes the director)."""
import sys,os,re,struct,subprocess,collections
D=sys.argv[1]; HERE=os.path.dirname(os.path.abspath(__file__))
f32=lambda v: '%.6g'%struct.unpack('<f',struct.pack('<f',float(v)))[0]
def num(s):
    m=re.match(r'\s*([-+]?\d+(?:\.\d*)?|[-+]?\.\d+)',s or '')
    return m.group(1) if m else None
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
same=0; diffs=[]; kinds=collections.Counter()
for i,p in enumerate(L):
    txt=subprocess.run(['luajit',HERE+'/decini.lua',p],capture_output=True).stdout.decode('latin-1')
    sec=None; S={}; order=collections.defaultdict(list)
    for line in txt.replace('\r','').split('\n'):
        line=line.split(';')[0]
        m=re.match(r'\s*\[([^\]]+)\]',line)
        if m: sec=m.group(1).strip().lower(); S.setdefault(sec,{}); continue
        if sec and '=' in line:
            k,v=line.split('=',1); k=k.strip().lower(); v=v.strip()
            S[sec][k]=v; order[sec].append(v)
    g=lambda s,k: num(S.get(s,{}).get(k)) or '0'
    lvl=g('general','level'); ms=g('general','measurescale')
    jd=[str(int(float(g('judgmentdelta',k)))) for k in ('kool','cool','good','miss')]
    ga=[f32(g('gaugeupdownrate',k)) for k in ('cool','good','miss','fail')]
    au=[num(x) or '0' for x in order.get('audiencerate',[])]+['0']*9
    ref=['load=1','Level=%d MeasureScale=%s'%(int(float(lvl)),f32(ms)),'Judgment='+','.join(jd),'Gauge='+','.join(ga),
         'Audience=%d,%d,'%(int(float(au[0])),int(float(au[1])))+','.join(f32(x) for x in au[2:9])]
    ours=open(D+'/%d.dump'%i,encoding='latin-1').read().split('\n')[:5]
    if ours==ref: same+=1; continue
    for a,b in zip(ours,ref):
        if a!=b: kinds[a.split('=')[0]]+=1; diffs.append((p.split('/game/')[-1],a,b)); break
print('%d identical, %d differ'%(same,len(diffs)))
print(kinds.most_common())
for d in diffs[:12]: print(' ',d)
