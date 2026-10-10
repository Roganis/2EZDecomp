"""gdsref.py DIR : a key-based reading of every mode .gds in DIR/list.txt, dumped in
ftgdsmode's format and compared with its dumps. The file is parsed into its
brace structure and values are looked up BY NAME (TrackK / GroupK by number);
the game reads every block positionally, so a difference means a file whose
key order matters."""
import sys,re,struct,collections
D=sys.argv[1]
f32=lambda v: '%.6g'%struct.unpack('<f',struct.pack('<f',float(v)))[0]
def parse(text):
    text=re.sub(r"'[^\n]*",'',text)   # ' starts a comment to end of line (KParser's quote char)
    toks=re.findall(r'\[[^\]]*\]|[{}=,]|[^\s{}=,\[\]]+',text)
    root={}; sec=None; i=0
    def block(i):
        d={}
        while i<len(toks) and toks[i]!='}' and not toks[i].startswith('['):
            k=toks[i]; i+=1
            if i<len(toks) and toks[i]=='=':
                i+=1
                if i<len(toks) and toks[i]=='{':
                    sub,i=block(i+1); d[k.lower()]=sub; i+=1
                else:
                    vals=[]
                    while i<len(toks) and toks[i] not in ('{','}',',') and not toks[i].startswith('['):
                        vals.append(toks[i].strip('"')); i+=1
                        if i<len(toks) and toks[i]==',': i+=1; continue
                        break
                    d[k.lower()]=vals
            elif k=='{':
                sub,i=block(i); i+=1
        return d,i
    while i<len(toks):
        t=toks[i]
        if t.startswith('['):
            sec=t[1:-1].strip().lower(); body,i=block(i+1); root[sec]=body
        else: i+=1
    return root
def first(d,k,default='0'):
    v=d.get(k); return v[0] if isinstance(v,list) and v else default
L=open(D+'/list.txt',encoding='utf-8').read().split('\n')[:-1]
same=0; diffs=[]
for i,p in enumerate(L):
    g=parse(open(p,encoding='latin-1').read())
    G=g.get('general',{})
    ref=['load=1',
         'General=%s,%s,%s,%s'%tuple(first(G,k) for k in ('numberofslot','usechainplay','maxbasestage','maxbonusstage')),
         'Background=%s Gauge=%s InGameSongSelect=%s Panel=%s'%(first(g.get('background',{}),'useforcebackground'),
              next(iter([v[0] for v in g.get('gauge',{}).values() if isinstance(v,list) and v]),'0'),
              next(iter([v[0] for v in g.get('ingamesongselect',{}).values() if isinstance(v,list) and v]),'0'),
              next(iter([v[0] for v in g.get('panel',{}).values() if isinstance(v,list) and v]),''))]
    sc=g.get('showcredit',{}); c1=sc.get('coord1',['0','0']); c2=sc.get('coord2',['0','0'])
    ref.append('ShowCredit='+','.join(f32(x) for x in (c1+['0','0'])[:2]+(c2+['0','0'])[:2]))
    for s in (1,2):
        S=g.get('slot%d'%s,{}); n=int(first(S,'numberoftrack'))
        parts=[]
        for t in range(1,min(n,27)+1):
            T=S.get('track%d'%t,{}); k=T.get('key',['0','0']); parts.append(' %s,%s:%s'%(k[0],(k+['0'])[1],first(T,'songtrack')))
        ref.append('Slot%d n=%d%s'%(s,n,''.join(parts)))
    JL=g.get('judgmentlink',{})
    for s in (1,2):
        S=JL.get('slot%d'%s,{}); n=int(first(S,'numberofgroup'))
        parts=[]
        for k in range(1,min(n,8 if s==1 else 7)+1):
            Gr=S.get('group%d'%k,{}); m=int(first(Gr,'numberoftrack',first(Gr,'numberoflink')))
            parts.append(' ['+','.join(Gr.get('songtracks',[])[:min(m,0x11)])+']')
        ref.append('Link%d groups=%d%s'%(s,n,''.join(parts)))
    ours=open(D+'/%d.dump'%i,encoding='latin-1').read().split('\n')[:len(ref)]
    if ours==ref: same+=1
    else:
        for a,b in zip(ours,ref):
            if a!=b: diffs.append((p.split('/game/')[-1],a[:110],b[:110])); break
print('%d identical, %d differ'%(same,len(diffs)))
for d in diffs: print(' ',d)
