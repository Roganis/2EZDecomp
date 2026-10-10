import json,sys,re,collections
# usage: python3 scratchpad/immscan.py RECCMP.json  (rc wrapper: reccmp --json FILE)
IM=re.compile(r'(?<![\w+\-*\[])(0x[0-9a-f]+|\d+)(?![\w\]])')
def key(t):
    t=t.split('\t')[0].strip()
    t=re.sub(r'<OFFSET\d+>','OFF',t)
    ims=[x for x in IM.findall(t)]
    sk=IM.sub('#',t)
    return sk,ims
def cnt(lines):
    c=collections.Counter()
    for l in lines:
        t=l[1].split('\t')[0].strip()
        if t.startswith(('j','call','push 0x4','push 0x1','lea esp')): continue
        c[t]+=1
    return c
for e in json.load(open(sys.argv[1]))['data']:
    if (e.get('matching') or 1)>=1: continue
    if e['name'].startswith(('_','?')) : continue
    o=[];r=[]
    for h in e['diff'] or []:
        for blk in h[1]:
            if 'both' in blk: continue
            o+=blk.get('orig',[]); r+=blk.get('recomp',[])
    co=cnt(o); cr=cnt(r)
    oo=co-cr; rr=cr-co
    # immediates-only differences: same skeleton
    so=collections.defaultdict(list); sr=collections.defaultdict(list)
    for t,n in oo.items():
        sk,ims=key(t)
        if ims: so[sk]+= [tuple(ims)]*n
    for t,n in rr.items():
        sk,ims=key(t)
        if ims: sr[sk]+= [tuple(ims)]*n
    for sk in so:
        if sk in sr:
            a=collections.Counter(so[sk]); b=collections.Counter(sr[sk])
            # only report small immediates (not addresses / float bits)
            def small(v): 
                try: return all(int(x,0)<0x10000 for x in v)
                except: return False
            A=[v for v in (a-b) if small(v)]; B=[v for v in (b-a) if small(v)]
            if A and B: print(e['address'],e['name'],'|',sk,'| orig',A,'ours',B)
