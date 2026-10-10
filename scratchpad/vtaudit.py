#!/usr/bin/env python3
"""vtaudit.py -- for every `// VTABLE: EZ2AC_UNPACKED 0xADDR` pin, count the
original table's slots (consecutive .text pointers in .rdata) and report the
slots whose target has no FUNCTION/STUB annotation. Truncated models score
100% on the compared prefix, so this is the honesty check."""
import re, subprocess, struct

d = open('original/EZ2AC_Unpacked.exe', 'rb').read()
IMG = 0x400000
TEXT_LO, TEXT_HI = 0x401000, 0x47a200   # include CRT thunks as valid targets

out = subprocess.run(['grep', '-rhoE', 'VTABLE: EZ2AC_UNPACKED 0x[0-9a-fA-F]+',
                      'src'], capture_output=True, text=True).stdout
pins = sorted(set(int(m, 16) for m in re.findall(r'0x[0-9a-fA-F]+', out)))

src = subprocess.run(['grep', '-rhoE',
                      '(FUNCTION|STUB): EZ2AC_UNPACKED 0x[0-9a-fA-F]+', 'src'],
                     capture_output=True, text=True).stdout
annotated = set(int(m, 16) for m in re.findall(r'0x[0-9a-fA-F]+', src))
pinset = set(pins)

bad = 0
for p in pins:
    slots = []
    a = p
    while True:
        v = struct.unpack('<I', d[a - IMG:a - IMG + 4])[0]
        if not (TEXT_LO <= v < TEXT_HI):
            break
        if a != p and a in pinset:   # next pinned table starts here
            break
        slots.append(v)
        a += 4
    # _purecall (CRT @0x479a20) resolves by name from our libcmt.
    missing = [(i, v) for i, v in enumerate(slots)
               if v not in annotated and v != 0x479a20]
    if missing:
        bad += 1
        print("0x%06x: %d slots, %d unannotated:" % (p, len(slots), len(missing)),
              " ".join("[%d]=0x%x" % (i, v) for i, v in missing))
print("%d pinned tables, %d with unannotated slots" % (len(pins), bad))
