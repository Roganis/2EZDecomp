#!/usr/bin/env python3
"""deadscan.py - UNREFERENCED function starts in the game's .text that no
annotation covers. census.py only sees referenced starts (call targets and
vtable slots); a function the linker kept but nothing calls - an out-of-line
copy of an inline ctor/dtor, say - is invisible to it. This walks the
decoded instruction stream (scratchpad/text.asm) for a 16-aligned instruction
that follows int3 padding after a ret/jmp, and reports the ones with no
FUNCTION/STUB/LIBRARY marker, with any absolute references to them.
(The raw-byte form of this scan lies: an operand byte 0xcc before an aligned
address looks like padding.)  2026-10-01: 9 found, 8 reconstructed; the
ninth (0x4774a0) is dinput.lib's c_dfDIKeyboard object table, not code."""
import re, subprocess, struct, bisect
d = open('original/EZ2AC_Unpacked.exe', 'rb').read(); IMG = 0x400000
LO, HI = 0x401000, 0x477a00
src = subprocess.run(['grep', '-rhoE', '(FUNCTION|STUB|LIBRARY): EZ2AC_UNPACKED 0x[0-9a-fA-F]+', 'src'],
                     capture_output=True, text=True).stdout
ann = sorted(set(int(m, 16) for m in re.findall(r'0x[0-9a-fA-F]+', src)))
pat = re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
ins = []
for l in open('scratchpad/text.asm'):
    m = pat.match(l)
    if m:
        a = int(m.group(1), 16)
        if LO <= a < HI: ins.append((a, m.group(2)))
annset = set(ann); n = 0
for i in range(1, len(ins)):
    a, mn = ins[i]
    if mn == 'int3' or ins[i-1][1] != 'int3' or a % 16: continue
    j = i - 1
    while j > 0 and ins[j][1] == 'int3': j -= 1
    if ins[j][1] not in ('ret', 'jmp') or a in annset: continue
    refs = [hex(m.start() + IMG) for m in re.finditer(re.escape(struct.pack('<I', a)), d)]
    k = bisect.bisect_right(ann, a)
    print(hex(a), 'after', hex(ann[k-1]), 'abs refs', refs); n += 1
print(n, 'unannotated unreferenced starts')
