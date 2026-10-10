#!/usr/bin/env python3
"""How much of the binary's own code is reconstructed, by BYTES.

Function starts = call-rel32 targets in .text + every annotated address.
A function's size = distance to the next start (int3 padding excluded by
walking back over trailing 0xcc).  Matched = a FUNCTION marker in src/,
stubbed = a STUB marker (address mapped, body not written)."""
import re, os, subprocess, collections

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LO, HI = 0x401000, 0x478000

text = open(ROOT+'/scratchpad/text.asm').read().split('\n')
pat = re.compile(r'^\s*([0-9a-f]{6}):\t(\S+)\s*(.*)$')
insns = []
for l in text:
    m = pat.match(l)
    if m: insns.append((int(m.group(1),16), m.group(2), m.group(3).strip()))

starts = set()
for a, mn, rest in insns:
    if mn == 'call':
        m = re.match(r'^0x([0-9a-f]+)$', rest)
        if m:
            t = int(m.group(1),16)
            if LO <= t < HI: starts.add(t)

ann_fn, ann_stub = set(), set()
for root, _, files in os.walk(ROOT+'/src'):
    for f in files:
        if not f.endswith(('.cpp','.h')): continue
        for line in open(os.path.join(root,f), errors='ignore'):
            m = re.search(r'//\s*(FUNCTION|STUB):\s*EZ2AC_UNPACKED\s+0x([0-9a-fA-F]+)', line)
            if m:
                a = int(m.group(2),16)
                if LO <= a < HI:
                    (ann_fn if m.group(1)=='FUNCTION' else ann_stub).add(a)
starts |= ann_fn | ann_stub

# size of each function = to the next start, minus trailing int3 padding
byaddr = {a:(mn,rest) for a,mn,rest in insns}
ordered = sorted(starts)
size = {}
for i,a in enumerate(ordered):
    end = ordered[i+1] if i+1 < len(ordered) else HI
    e = end
    while e > a and byaddr.get(e-1, ('',''))[0] == 'int3': e -= 1
    # walk back over the whole padding run
    while e > a and byaddr.get(e-1, ('',''))[0] == 'int3': e -= 1
    size[a] = max(0, e - a)

tot   = sum(size.values())
match = sum(size[a] for a in ann_fn if a in size)
stub  = sum(size[a] for a in ann_stub if a in size)
print("functions found            %5d" % len(ordered))
print("  reconstructed (FUNCTION) %5d   %8d bytes  %5.1f%%" % (len(ann_fn), match, 100.0*match/tot))
print("  stubbed (address only)   %5d   %8d bytes  %5.1f%%" % (len(ann_stub), stub, 100.0*stub/tot))
print("  untouched                %5d   %8d bytes  %5.1f%%" % (len(ordered)-len(ann_fn)-len(ann_stub),
                                                               tot-match-stub, 100.0*(tot-match-stub)/tot))
print("total code in functions          %8d bytes" % tot)
