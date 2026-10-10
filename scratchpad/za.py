#!/usr/bin/env python3
"""za.py 0xADDR [--raw] -- zdiff one annotated function, found by address.

Resolves the `// FUNCTION: EZ2AC_UNPACKED 0xADDR` marker to its source file and
definition, builds the decorated-name prefix cl uses (`Class::method` becomes
`?method@Class`), finds the function's end in the original by scanning to the
first int3, copies the TU into scratchpad/ and runs zdiff on it.
"""
import re, os, subprocess, sys, shutil

addr = int(sys.argv[1], 16)
raw = ['--raw'] if '--raw' in sys.argv else []

pat = re.compile(r'//\s*FUNCTION:\s*EZ2AC_UNPACKED\s+0x0*%x\b' % addr, re.I)
hit = None
for fn in sorted(os.listdir('src')):
    if not fn.endswith('.cpp'):
        continue
    lines = open('src/' + fn).read().split('\n')
    for i, l in enumerate(lines):
        if pat.search(l):
            # the definition is the next non-comment, non-blank line
            for j in range(i + 1, min(i + 6, len(lines))):
                t = lines[j].strip()
                if t and not t.startswith('//'):
                    hit = ('src/' + fn, t)
                    break
            break
    if hit:
        break
if not hit:
    sys.exit("no // FUNCTION marker for %#x in src/" % addr)
path, defn = hit

m = re.search(r'(?:(\w+)::)?(~?\w+)\s*\(', defn)
if not m:
    sys.exit("cannot read a name out of: %s" % defn)
cls, meth = m.group(1), m.group(2)
# cl decorates a ctor as ??0Class@@..., a dtor as ??1Class@@... and an
# ordinary method as ?method@Class@@... - three different prefixes, and
# guessing the method form for a constructor finds nothing.
if meth.startswith('~'):
    sym = '??1%s@' % cls
elif cls and meth == cls:
    sym = '??0%s@' % cls
elif cls:
    sym = '?%s@%s' % (meth, cls)
else:
    sym = '?%s@' % meth

# end of the function in the original = first int3 at or after the entry
d = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386', '-Mintel',
    '--no-show-raw-insn', '--adjust-vma=0x400000', '--start-address=%#x' % addr,
    '--stop-address=%#x' % (addr + 0x4000), 'original/EZ2AC_Unpacked.exe'],
    capture_output=True, text=True)
end = None
table = None
for l in d.stdout.splitlines():
    mm = re.match(r'^\s*([0-9a-f]+):\t(.*)$', l)
    if not mm:
        continue
    at, text = int(mm.group(1), 16), mm.group(2).strip()
    if text == 'int3':
        end = at
        break
    # A switch's JUMP TABLE lives in .text right after the function and
    # disassembles as plausible garbage, so the first int3 can be well past
    # the real end - m420a60 @0x420a60 read as 12 instructions longer than it
    # is, which looked like missing logic in the reconstruction. The indirect
    # jump names the table's address; if it is ahead of us, that IS the end.
    t = re.search(r'jmp\s+DWORD PTR \[[^\]]*\*4\+(0x[0-9a-f]+)\]', text)
    if t:
        cand = int(t.group(1), 16)
        if addr < cand and (table is None or cand < table):
            table = cand
if table is not None and (end is None or table < end):
    end = table
if end is None:
    sys.exit("no int3 terminator within 16 KB of %#x" % addr)

zoo = 'scratchpad/za_%x.cpp' % addr
shutil.copy(path, zoo)
print("%s  %s  [%s]  %#x..%#x" % (path, defn, sym, addr, end))
sys.stdout.flush()
os.execv(sys.executable, [sys.executable, 'scratchpad/zdiff.py', zoo, sym,
                          hex(addr), hex(end)] + raw)
