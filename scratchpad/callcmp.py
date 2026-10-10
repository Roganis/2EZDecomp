#!/usr/bin/env python3
"""callcmp.py SRC.cpp SYMBOL LO HI - compare the CALL MULTISET of a
zoo-compiled function against the original's.

This is the correctness oracle the port track otherwise gives up. A
reconstruction can compile clean, look right and still be calling the wrong
member or missing a load: on TitleDirector's ctor the tell was eleven loadStr
in the original against ten in mine, which turned out to be the version chart
being read off the wrong offset. Instruction diffs bury that in codegen noise;
a call count does not.

Our side reads the COMDAT's relocation table (a call to another function is a
REL32 against a symbol) rather than the disassembly, because in a .obj the
call targets are all zero.
"""
import re, subprocess, sys, os

def orig_calls(lo, hi, names):
    p = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
        '--no-show-raw-insn','--adjust-vma=0x400000','--start-address=%#x'%lo,
        '--stop-address=%#x'%hi,'original/EZ2AC_Unpacked.exe'],
        capture_output=True, text=True)
    out=[]
    for l in p.stdout.splitlines():
        m = re.match(r'^\s*[0-9a-f]+:\t(call\s+.*)$', l)
        if not m: continue
        t = re.search(r'call\s+0x([0-9a-f]+)', m.group(1))
        out.append(names.get(int(t.group(1),16), 'sub_'+t.group(1)) if t
                   else '<indirect>')
    return out

def annotations():
    names={}
    for fn in sorted(os.listdir('src')):
        if not fn.endswith('.cpp'): continue
        L=open('src/'+fn,errors='replace').read().split('\n')
        for i,l in enumerate(L):
            m=re.search(r'//\s*(?:FUNCTION|STUB|LIBRARY):\s*EZ2AC_UNPACKED\s+(0x[0-9a-fA-F]+)',l)
            if not m: continue
            for j in range(i+1,min(i+6,len(L))):
                t=L[j].strip()
                if t and not t.startswith('/*'):
                    if t.startswith('//'):
                        names.setdefault(int(m.group(1),16), t.lstrip('/ ').strip())
                        break
                    nm=re.search(r'(?:(\w+)::)?(~?\w+)\s*\(',t)
                    if nm:
                        names.setdefault(int(m.group(1),16),
                            (nm.group(1)+"::" if nm.group(1) else "")+nm.group(2))
                    break
    return names

def zoo_calls(path, sym):
    """Our function's calls, IN ORDER, mixing relocated and indirect ones.

    A call to another function is a REL32 relocation (the target is zero in the
    .obj, so the disassembly cannot name it); an indirect call through a vtable
    has no relocation at all. Both have to appear in ONE ordered list or the
    sequence check is meaningless - counting the indirect ones separately and
    appending them, which is what this did first, makes every function with a
    virtual call look reordered.
    """
    r = subprocess.run(['./scratchpad/zoo.sh', path], capture_output=True, text=True)
    if r.returncode: sys.exit("cl failed:\n"+r.stdout[:1500])
    obj = 'scratchpad/obj/%s.obj' % path.split('/')[-1][:-4]

    sec = val = None
    for l in subprocess.run(['objdump','-t',obj],capture_output=True,text=True).stdout.splitlines():
        m = re.match(r'^\[\s*\d+\]\(sec\s+(\d+)\).*?\s(0x[0-9a-f]+)\s+(\S+)\s*$', l)
        if m and m.group(3).startswith(sym):
            sec, val = int(m.group(1)), int(m.group(2), 16); break
    if sec is None: sys.exit("symbol %r not found" % sym)

    b = open(obj,'rb').read()
    h = 20 + (sec-1)*40
    size = int.from_bytes(b[h+16:h+20],'little'); off = int.from_bytes(b[h+20:h+24],'little')
    ptr  = int.from_bytes(b[h+24:h+28],'little'); n = int.from_bytes(b[h+32:h+34],'little')
    symtab = int.from_bytes(b[8:12],'little'); nsyms = int.from_bytes(b[12:16],'little')
    strtab = symtab + nsyms*18

    rel = {}
    for i in range(n):
        rec = b[ptr+i*10:ptr+i*10+10]
        if int.from_bytes(rec[8:10],'little') != 0x14: continue   # REL32
        idx = int.from_bytes(rec[4:8],'little')
        e = b[symtab+idx*18:symtab+idx*18+18]
        if e[0:4] == b'\0\0\0\0':
            o2 = int.from_bytes(e[4:8],'little')
            nm = b[strtab+o2:b.index(b'\0', strtab+o2)].decode('latin-1')
        else:
            nm = e[0:8].rstrip(b'\0').decode('latin-1')
        rel[int.from_bytes(rec[0:4],'little')] = nm

    d = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
        '--no-show-raw-insn','--start-address=%#x'%(off+val),
        '--stop-address=%#x'%(off+size), obj], capture_output=True, text=True).stdout
    ins = []
    for l in d.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\t(.*)$', l)
        if m: ins.append((int(m.group(1),16) - off, m.group(2).strip()))
    out = []
    for k, (a, t) in enumerate(ins):
        if not t.startswith('call'): continue
        nxt = ins[k+1][0] if k+1 < len(ins) else a + 8
        hit = [nm for r, nm in rel.items() if a <= r < nxt]
        out.append(hit[0] if hit else '<indirect>')
    return out


def zoo_indirect(path, sym):
    """Indirect calls (`call [reg+off]`) carry no relocation, so the table
    above cannot see them - count them out of the listing instead, or a
    virtual call the reconstruction dropped reads as no difference at all."""
    from subprocess import run
    obj = 'scratchpad/obj/%s.obj' % path.split('/')[-1][:-4]
    sec = val = None
    for l in run(['objdump','-t',obj],capture_output=True,text=True).stdout.splitlines():
        m = re.match(r'^\[\s*\d+\]\(sec\s+(\d+)\).*?\s(0x[0-9a-f]+)\s+(\S+)\s*$', l)
        if m and m.group(3).startswith(sym):
            sec, val = int(m.group(1)), int(m.group(2),16); break
    if sec is None: return 0
    b = open(obj,'rb').read(); h = 20 + (sec-1)*40
    size = int.from_bytes(b[h+16:h+20],'little'); off = int.from_bytes(b[h+20:h+24],'little')
    d = run(['objdump','-D','-b','binary','-m','i386','-Mintel','--no-show-raw-insn',
             '--start-address=%#x'%(off+val),'--stop-address=%#x'%(off+size),obj],
            capture_output=True,text=True).stdout
    return len(re.findall(r'\tcall\s+DWORD PTR', d))

names = annotations()
o = orig_calls(int(sys.argv[3],16), int(sys.argv[4],16), names)
z = zoo_calls(sys.argv[1], sys.argv[2])
from collections import Counter
co, cz = Counter(o), Counter(z)
print("original %d calls (%d distinct), ours %d (%d distinct)\n" % (len(o),len(co),len(z),len(cz)))
def matcher(s):
    """Return a predicate over a DECORATED name for the original's callee `s`.

    cl mangles a member as ?method@Class@@, i.e. method FIRST, so testing for
    "Class::method" never hits; and a bare substring test lets `restart` match
    `start`. Anchor on the '?method@' and '@Class@' forms instead. A plain C
    function is either _name (extern "C") or ?name@@ (C++ linkage).
    """
    s = s.strip()
    if s == '<indirect>':
        return lambda k: k == '<indirect>'
    if '::' in s:
        c, m = s.split('::', 1)
        if m == c:      return lambda k: k.startswith('??0' + c + '@')
        if m == '~' + c: return lambda k: k.startswith('??1' + c + '@')
        return lambda k: ('?' + m + '@') in k and ('@' + c + '@') in k
    base = s.lstrip('_').split('@')[0]
    # our side may spell the same function four ways: _name (extern "C"),
    # ?name@@... (C++ linkage), __imp__name@N (dllimport) and @name@N
    # (fastcall). Strip all of that before comparing.
    def bare(k):
        return k.lstrip('?@_').replace('imp__', '').split('@')[0]
    return lambda k: bare(k) == base

print("%-34s %5s %5s" % ("original callee","orig","ours"))
bad = 0
for k, v in sorted(co.items(), key=lambda kv: -kv[1]):
    pred = matcher(k)
    got = sum(n for kk, n in cz.items() if pred(kk))
    flag = ""
    if got != v:
        flag = "   <-- %+d" % (got - v); bad += 1
    print("  %-32s %5d %5d%s" % (k, v, got, flag))
print()
print("%d callee(s) with a count mismatch" % bad)

# The multiset says the right calls happen; the SEQUENCE says they happen in
# the right order and under the right conditions, which is the stronger claim
# and the one that matters when a reconstruction scores low. ResultDirector::
# update @0x451b80 came out at 73% with an exact multiset AND an exact
# sequence, which is how it was settled that the residue is codegen rather
# than logic.
def short(k):
    m = re.match(r'\?(\w+)@', k)
    return m.group(1) if m else k.lstrip('?_@').replace('imp__', '').split('@')[0]
import difflib
sa = [short(x.split('::')[-1]) for x in o]
sb = [short(x.split('::')[-1]) for x in z]
d = [l for l in difflib.unified_diff(sa, sb, 'orig', 'ours', lineterm='', n=1)]
print()
if not d:
    print("CALL SEQUENCE IDENTICAL (%d calls) - the residue is codegen, not logic" % len(sa))
else:
    print("call sequence differs:")
    for l in d[:60]: print("  " + l)
