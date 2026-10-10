#!/usr/bin/env python3
"""census.py -- every REAL function start in the game's .text that has no
FUNCTION/STUB annotation in src/.

A start is real when it is referenced: a `call rel32` target from .text, or a
slot value in any vtable-looking run of .text pointers in .rdata/.data.
(The int3-scan approach over-counts by every jump table glued to a function.)
Each hit is sized by scanning to the next referenced-or-annotated start.
"""
import re, subprocess, struct, bisect, sys

d = open('original/EZ2AC_Unpacked.exe', 'rb').read()
TEXT_LO, TEXT_HI = 0x401000, 0x477a00     # game code; CRT after
IMG = 0x400000

src = subprocess.run(['grep', '-rhoE',
                      '(FUNCTION|STUB): EZ2AC_UNPACKED 0x[0-9a-fA-F]+', 'src'],
                     capture_output=True, text=True).stdout
annotated = set(int(m, 16) for m in re.findall(r'0x[0-9a-fA-F]+', src))

targets = set()
# call rel32 across the whole image's code (CRT calls into game code too)
for off in range(0x1000, 0x80000):
    if d[off] == 0xe8:
        t = IMG + off + 5 + struct.unpack('<i', d[off+1:off+5])[0]
        if TEXT_LO <= t < TEXT_HI:
            targets.add(t)
# data-section pointer words into .text (vtable slots, callbacks, tables)
for off in range(0x80000, 0xb0000, 4):
    v = struct.unpack('<I', d[off:off+4])[0]
    if TEXT_LO <= v < TEXT_HI:
        targets.add(v)

# a referenced address only counts as a function start if it begins after
# int3 padding or at a 16-byte boundary right after another function's tail;
# cheap filter: previous byte is int3, or the target is itself annotated-like.
def plausible(t):
    b = d[t - IMG - 1]
    return b == 0xcc or t % 16 == 0 or d[t - IMG] in (0x55, 0x56, 0x53, 0x83, 0x81, 0x8b, 0x6a, 0xa1, 0x51, 0x57, 0xb8, 0x33, 0xe9, 0xff)

starts = sorted(t for t in targets if plausible(t))
allstarts = sorted(set(starts) | annotated)

# Resolved by hand 2026-08-30 (commit e2472bf): interior jump-table targets
# inside reconstructed functions, one function TAIL, data-in-text, and the
# two import thunks. A clean census prints an empty list.
ARTIFACTS = {
    0x4774a0,   # data table at the end of .text
    0x441060, 0x440fd7, 0x441033,   # inside inputClubMix @0x440af0
    0x41786f,   # inside runClubMix @0x417670
    0x4610d4,   # inside a GF director function
    0x42b7f0,   # inside a panel function @0x42b6xx
    0x470020,   # inside CatchPanel @0x46ffxx
    0x46cc30,   # inside CatchMainGameDirector::m46ca30
    0x409980,   # the TAIL of the worm TU's @0x409830
    0x4145f0,   # WINMM timeGetTime import thunk
    0x47748e,   # DirectSoundCreate linker thunk
}

rows = []
for t in starts:
    if t in annotated or t in ARTIFACTS:
        continue
    j = bisect.bisect_right(allstarts, t)
    nxt = allstarts[j] if j < len(allstarts) else TEXT_HI
    rows.append((t, min(nxt, TEXT_HI) - t))

rows.sort(key=lambda r: -r[1])
tot = sum(sz for _, sz in rows)
print("unannotated referenced functions: %d, %d bytes" % (len(rows), tot))
lim = int(sys.argv[1]) if len(sys.argv) > 1 else 40
for t, sz in rows[:lim]:
    print("0x%06x  %5d" % (t, sz))
