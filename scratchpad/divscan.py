import json,sys,re,collections
# usage: python3 scratchpad/divscan.py RECCMP.json  (rc wrapper: reccmp --json FILE)
def sig(lines):
    c=collections.Counter()
    for i,l in enumerate(lines):
        t=l[1].split('\t')[0].strip()
        m=re.match(r'mov e[a-d]x, (0x[0-9a-f]{7,8})$',t)
        if m:
            k=int(m.group(1),16)
            if k in (0,) : continue
            sh=0
            for l2 in lines[i+1:i+6]:
                t2=l2[1].split('\t')[0].strip()
                mm=re.match(r'sar edx, (\S+)$',t2)
                if mm: sh=mm.group(1); break
                if t2.startswith('sar edx') and t2=='sar edx, 1': sh='1'; break
            if any(x[1].split()[0]=='imul' for x in lines[i+1:i+4]): c[(hex(k),sh)]+=1
        m=re.match(r'mov (e[a-z]{2}), (0x[0-9a-f]+|\d+)$',t)
        if m and i+2<len(lines):
            for l2 in lines[i+1:i+4]:
                if re.match(r'i?div '+m.group(1)+'$',l2[1].split('\t')[0].strip()): c[('div',m.group(2))]+=1
    return c
for e in json.load(open(sys.argv[1]))['data']:
    if (e.get('matching') or 1)>=1: continue
    o=[];r=[]
    for h in e['diff'] or []:
        for blk in h[1]:
            if 'both' in blk: o+=blk['both']; r+=blk['both']; continue
            o+=blk.get('orig',[]); r+=blk.get('recomp',[])
    so=sig(o); sr=sig(r)
    if so!=sr: print(e['address'],e['name'],'orig',dict(so-sr),'ours',dict(sr-so))
