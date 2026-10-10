import json,sys,re
# usage: python3 scratchpad/cmpscan.py RECCMP.json  (rc wrapper: reccmp --json FILE)
ORD={'jl','jle','jg','jge','jb','jbe','ja','jae'}
MIR={'jl':'jg','jle':'jge','jg':'jl','jge':'jle','jb':'ja','jbe':'jae','ja':'jb','jae':'jbe'}
def seq(lines):
    out=[]
    for i,l in enumerate(lines):
        t=l[1].split('\t')[0].strip()
        m=re.match(r'cmp (.+?), (.+)$',t)
        if m and i+1<len(lines):
            j=lines[i+1][1].split()[0]
            if j in ORD: out.append((m.group(1).strip(),m.group(2).strip(),j,l[0]))
    return out
def norm(x): return re.sub(r'<OFFSET\d+>|\(.*?\)','',x).strip()
for e in json.load(open(sys.argv[1]))['data']:
    if (e.get('matching') or 1)>=1: continue
    orig=[];rec=[]
    for h in e['diff'] or []:
        for blk in h[1]:
            if 'both' in blk: orig+=blk['both']; rec+=[(x[2],x[1]) for x in blk['both']]; continue
            orig+=blk.get('orig',[]); rec+=blk.get('recomp',[])
    so=seq(orig); sr=seq(rec)
    for a,b,j,ad in so:
        for c,d,k,ad2 in sr:
            if norm(a)==norm(d) and norm(b)==norm(c) and norm(a)!=norm(b) and j==k:
                print(e['address'],e['name'],'orig',ad,'cmp',a,',',b,j,'| ours cmp',c,',',d,k)
