import json,sys,re,collections
# usage: python3 scratchpad/fltscan.py RECCMP.json  (rc wrapper: reccmp --json FILE)
R=re.compile(r'\[([-0-9.e+]+) \((FLOAT|DOUBLE)\)\]|push (0x[0-9a-f]{8})')
def fl(lines):
    c=collections.Counter()
    for l in lines:
        t=l[1].split('\t')[0]
        for m in R.finditer(t):
            if m.group(1): c[m.group(1)+m.group(2)[0]]+=1
            else:
                v=int(m.group(3),16)
                if 0x3c000000<=v<=0x4a000000 or 0xbc000000<=v<=0xca000000:
                    import struct; c['p%.4g'%struct.unpack('<f',struct.pack('<I',v))[0]]+=1
    return c
for e in json.load(open(sys.argv[1]))['data']:
    if (e.get('matching') or 1)>=1: continue
    o=[];r=[]
    for h in e['diff'] or []:
        for blk in h[1]:
            if 'both' in blk: continue
            o+=blk.get('orig',[]); r+=blk.get('recomp',[])
    a=fl(o); b=fl(r)
    if a!=b: print(e['address'],e['name'],'orig',dict(a-b),'ours',dict(b-a))
