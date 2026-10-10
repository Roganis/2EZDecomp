#!/usr/bin/env python3
import os
"""sizesweep.py [--all] - find globals the ORIGINAL's code reaches past the
size our declaration gives them.

For every `// GLOBAL:` in src/, collect every absolute address the original's
.text names inside that global's span (up to the next annotated global): memory
operands, indexed or not, and immediates. Our size comes from the declaration
when the type is plain (char/int/float/pointer arrays), else from the gap to the
next symbol in build/EZ2AC_Unpacked.map (an upper bound). An access ending past
our size is reported; `cmp`/`lea`/immediate one-past-the-end (a loop bound) is
not. In .rdata only INDEXED accesses are kept unless --all - float constants and
strings sit right after many tables and would swamp the list.

Needs scratchpad/text.asm (objdump -D of the original's .text; never commit it).

Triage of every hit on 2026-10-05 (after g_parserTable and the four speed
ladders were fixed - those came from the data-continuation sweep):
  * x[len-3]-shaped biased reads of the NEXT global, landing a few bytes below
    it (the video-name buffers, g_37dcf38, g_gfPanelSwayPhase)  - false positive
  * an unrolled loop's biased displacements (SongTable key tables) - fine
  * strcpy from an unannotated literal, a byte store sized as a dword,
    a pushed address of an unannotated neighbour, vftable pointers stored
    next to g_menu_*                                              - false positive
  * the five subsystem singletons in src/ez2acapp.cpp (g_obj1b2e320 etc.) are
    1-byte placeholders for whole static objects - DELIBERATE (never
    constructed in the matching build; a running build needs real storage).
"""
import re,glob,bisect,collections,sys
ALL='--all' in sys.argv   # also report non-indexed .rdata hits (mostly neighbours)
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
prim={'char':1,'unsigned char':1,'signed char':1,'bool':1,'short':2,'unsigned short':2,'int':4,'unsigned int':4,'unsigned':4,'long':4,'unsigned long':4,'float':4,'double':8,'BYTE':1,'WORD':2,'DWORD':4}
# ---- annotated globals: original address -> (name, file, decl, declared size or None)
G={}
for f in glob.glob(ROOT+'/src/*.cpp'):
    L=open(f,encoding='latin-1').read().split('\n')
    for k,l in enumerate(L):
        m=re.match(r'\s*//\s*GLOBAL:\s*EZ2AC_UNPACKED\s+(0x[0-9a-fA-F]+)',l)
        if not m or k+1>=len(L): continue
        a=int(m.group(1),16); d=L[k+1].strip()
        mm=re.match(r'(?:static\s+)?(?:const\s+)?(?:volatile\s+)?((?:unsigned\s+|signed\s+)?[A-Za-z_]\w*)\s*(\*?)\s*(?:const\s+)?([A-Za-z_]\w*)\s*((?:\[[^\]]+\])*)\s*[;=]',d)
        size=None; name=None
        if mm:
            name=mm.group(3)
            el=4 if mm.group(2) else prim.get(mm.group(1))
            if el:
                try:
                    n=1
                    for x in re.findall(r'\[([^\]]+)\]',mm.group(4)): n*=int(eval(x))
                    size=el*n
                except Exception: size=None
        G.setdefault(a,(name,f.split('/')[-1],d,size))
# ---- our build: symbol sizes from the map (gap to the next symbol in the same section)
syms=[]
for l in open(ROOT+'/build/EZ2AC_Unpacked.map',encoding='latin-1'):
    m=re.match(r'\s*(000[2-9]):([0-9a-f]+)\s+(\S+)\s+([0-9a-f]{8})\s',l)
    if m: syms.append((m.group(1),int(m.group(2),16),m.group(3)))
syms.sort()
mapsize={}
for i,(sec,off,nm) in enumerate(syms):
    nxt=syms[i+1] if i+1<len(syms) and syms[i+1][0]==sec else None
    base=re.match(r'\?(\w+)@@',nm) or re.match(r'_(\w+)$',nm)
    if base and nxt: mapsize.setdefault(base.group(1),nxt[1]-off)
# ---- original code: every absolute address a memory operand or immediate names
W={'BYTE':1,'WORD':2,'DWORD':4,'QWORD':8,'TBYTE':10,'XMMWORD':16}
acc=collections.defaultdict(list)
keys=sorted(G)
LO,HI=0x487000,0x37e3000
pat=re.compile(r'^\s*([0-9a-f]+):\t(\S+)\s*(.*)$')
for l in open(ROOT+'/scratchpad/text.asm'):
    m=pat.match(l)
    if not m: continue
    ia=int(m.group(1),16); mn=m.group(2); ops=m.group(3)
    if not (0x401000<=ia<0x478000): continue
    for mm in re.finditer(r'(?:(BYTE|WORD|DWORD|QWORD|TBYTE|XMMWORD) PTR )?(?:ds:)?\[?([^\],]*?)?(0x[0-9a-f]{6,8})\]?',ops):
        a=int(mm.group(3),16)
        if not (LO<=a<HI): continue
        inmem='[' in mm.group(0) or 'ds:' in mm.group(0)
        idx=bool(mm.group(2) and re.search(r'e[a-z]{2}',mm.group(2)))
        w=W.get(mm.group(1),4) if inmem else 0
        k=bisect.bisect_right(keys,a)-1
        if k<0: continue
        g=keys[k]
        nxt=keys[k+1] if k+1<len(keys) else HI
        if a>=nxt: continue
        acc[g].append((a-g,w,mn,idx,ia,inmem))
# ---- report: accesses past our size
rows=[]
for g,lst in acc.items():
    name,f,d,decl=G[g]
    ours=decl if decl is not None else mapsize.get(name)
    if ours is None: continue
    bad=[x for x in lst if x[0]+max(x[1],1)>ours and not (x[0]==ours and (x[2] in ('cmp','lea','mov','push') and not x[5] or x[1]==0))
         and (g>=0x4a5000 or x[3] or ALL)]
    if bad:
        mx=max(x[0]+max(x[1],1) for x in bad)
        rows.append((g,name,f,'decl' if decl is not None else 'map',ours,mx,len(bad),sorted(set((hex(x[4]),x[2],hex(x[0]),x[1],'idx' if x[3] else '') for x in bad))[:4]))
rows.sort()
for r in rows: print('%#x %-26s %-22s %-4s ours=%-6d maxEnd=%-6d n=%-3d %s'%r)
print(len(rows),'globals with accesses past their size;',len(acc),'globals referenced')
