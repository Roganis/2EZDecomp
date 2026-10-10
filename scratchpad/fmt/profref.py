import sys,re,struct,subprocess,os
D=sys.argv[1]
S=os.path.dirname(os.path.abspath(__file__))
tok=re.compile(r'"([^"]*)"|([-+]?(?:\d+\.\d*|\.\d+|\d+))|(=)|(\S)')
L=open(D+'/list.txt').read().split('\n')[:-1]
same=0
for i,p in enumerate(L):
    txt=subprocess.run(['luajit',S+'/decini.lua',p],capture_output=True).stdout.decode('latin-1')
    toks=[m for m in tok.finditer(txt)]
    prof={}
    k=0
    while k+2<len(toks):
        key,eq,val=toks[k],toks[k+1],toks[k+2]
        if key.group(1) is None or eq.group(3) is None: break
        if val.group(1) is not None: prof[key.group(1)]=('S',val.group(1))
        elif val.group(2) is not None:
            v=val.group(2)
            if '.' in v: prof[key.group(1)]=('F','%.6g'%struct.unpack('<f',struct.pack('<f',float(v)))[0])
            else: prof[key.group(1)]=('I',str(int(v)))
        k+=3
    ref=sorted('%s\t%s\t%s'%(a,b,c) for a,(b,c) in prof.items())
    d=open(D+'/%d.dump'%i,encoding='latin-1').read().split('\n')
    hdr=d[0]; ours=sorted(x for x in d[1:] if x)
    if ours==ref: same+=1; print('OK  ',hdr,len(ref),p.split('/game/')[1])
    else:
        a,b=set(ours),set(ref)
        print('DIFF',hdr,'ours',len(ours),'ref',len(ref),p.split('/game/')[1])
        for x in sorted(a-b)[:4]: print('   ours-only',repr(x[:120]))
        for x in sorted(b-a)[:4]: print('   ref-only ',repr(x[:120]))
print('%d/%d identical'%(same,len(L)))
