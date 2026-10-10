"""scan.py - rank unmatched functions by BRANCH DENSITY, ascending: the
straight-line ones land first try (see CLAUDE.md's big-repetitive-functions
section).

CAVEAT, measured 2026-08-17: the top of this ranking can be DATA. 0x4774a0
led the list at 2912 bytes / 1060 instructions / 16 branches - a perfect
straight-line score - and disassembles as a repeating record with an
incrementing index field, i.e. a table. Nothing calls it; its single
reference is a table pointer at 0x4a0104. Before spending a tick on a
candidate, check it has a `call` site:

    python3 - <<'X'
    import struct
    d=open('original/EZ2AC_Unpacked.exe','rb').read()
    t=0xADDR
    print([hex(0x400000+i) for i in range(0x1000,0x78000)
           if d[i]==0xe8 and 0x400000+i+5+struct.unpack_from('<i',d,i+1)[0]==t])
    X

A real function with no call site is possible (a vtable slot), but then the
address appears in .rdata - if NEITHER holds, it is data.

SECOND CAVEAT, measured 2026-08-17: a ZERO-BRANCH score can also mean the
function is a `static` with a non-standard register convention, which CLAUDE.md
says never to attempt in isolation. Running this at (40, 200) put 0x409980 and
0x409ce0 at the top on 0.0000 branches each, and both are in the known-hard
0x409660 / 0x409a30 cluster: 0x409ce0 CALLS both of them, passing arguments in
edx/eax/esi and xmm1/edi, and 0x409980 is the same shape one level down - it
takes five floats in xmm0..xmm4, an out-pointer in eax, and two more arguments
on the stack, then cleans 0x1c bytes itself. Before spending a tick on a
straight-line candidate, check its prologue takes its arguments where a
__thiscall or __cdecl function would."""
import re, struct, subprocess, os, sys
S=os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
lines=open(S+'/text.asm').read().split('\n')
insns=[]   # (addr, mnemonic, rest)
pat=re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
for l in lines:
    m=pat.match(l)
    if m: insns.append((int(m.group(1),16), m.group(2), m.group(3).strip()))
addr2i={a:i for i,(a,_,_) in enumerate(insns)}

# function starts: call rel32 targets
starts=set()
for a,mn,rest in insns:
    if mn=='call':
        m=re.match(r'^0x([0-9a-f]+)$', rest)
        if m: starts.add(int(m.group(1),16))
# vtable slots + any 4-byte in .data/.rdata pointing into text region, from data sections
d=open(ROOT+'/original/EZ2AC_Unpacked.exe','rb').read()
# scan whole file for LE dwords in [0x401000,0x478000) that are 16-byte-ish starts
for off in range(0x88000, min(len(d),0x1000000)-4):
    pass
# too slow; instead scan .rdata/.data range 0x478000..0x600000 file offsets
base=0x400000
for va in range(0x478000, 0x4b1000, 4):
    v=struct.unpack('<I', d[va-base:va-base+4])[0]
    if 0x401000<=v<0x478000: starts.add(v)
# annotated addresses
ann=set()
for root,_,fs in os.walk(ROOT+'/src'):
    for f in fs:
        if f.endswith('.cpp'):
            for m in re.finditer(r'(?:FUNCTION|STUB|LIBRARY):\s+EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)', open(os.path.join(root,f)).read()):
                ann.add(int(m.group(1),16))
starts |= ann
starts = sorted(a for a in starts if 0x401000<=a<0x478000 and a in addr2i)

# size: to next start, minus trailing int3
res=[]
for i,s in enumerate(starts):
    nxt = starts[i+1] if i+1<len(starts) else 0x478000
    j=addr2i[s]; br=0; n=0; end=s
    while j<len(insns) and insns[j][0]<nxt:
        a,mn,rest=insns[j]
        if mn=='(bad)': break
        if mn=='int3':
            # trailing padding: stop
            k=j
            while k<len(insns) and insns[k][1]=='int3': k+=1
            if k>=len(insns) or insns[k][0]>=nxt: break
            # int3 in middle -> also treat as end
            break
        n+=1
        if mn[0]=='j' or mn.startswith('set') or mn.startswith('cmov') or mn.startswith('loop'): br+=1
        j+=1
        end=insns[j][0] if j<len(insns) else nxt
    res.append((s, end-s, n, br))
# size window: scan.py [min] [max]  (default 400 .. inf)
LO = int(sys.argv[1]) if len(sys.argv)>1 else 400
HI = int(sys.argv[2]) if len(sys.argv)>2 else 1<<30
un=[r for r in res if r[0] not in ann and LO<=r[1]<HI]
un.sort(key=lambda r: (r[3]/max(r[2],1), -r[1]))
print("addr      size  insns  br   br/insn")
for s,sz,n,br in un[:60]:
    print("0x%06x  %5d  %5d  %3d  %.4f" % (s,sz,n,br,br/max(n,1)))
print("\ntotal unmatched in [%d,%d):" % (LO,HI), len(un), "bytes:", sum(r[1] for r in un))
