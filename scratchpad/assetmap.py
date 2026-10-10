#!/usr/bin/env python3
"""assetmap.py LO HI - the asset-to-member table of a big constructor.

The engine's screen constructors are mostly `loadTexture(path, &member, 0)` and
`loadStr(path, &head)` / `attach(head, flag)` repeated a hundred times, and the
thing a PORT needs out of them is which asset lands on which screen element.
Reading that off a raw listing is error-prone because both operands arrive in
registers: the destination is a `lea reg,[esi+off]` some lines earlier and the
path is either a string address or a buffer an earlier sprintf filled.

So: walk the instructions keeping a tiny register file of `esi+off`
expressions and the last sprintf format seen, and print one row per call.
"""
import re, subprocess, sys

lo, hi = int(sys.argv[1], 16), int(sys.argv[2], 16)
img = open('original/EZ2AC_Unpacked.exe','rb').read()

def cstr(a):
    off = a - 0x400000
    if off < 0 or off >= len(img): return None
    e = img.index(b'\0', off)
    s = img[off:e]
    if len(s) > 200 or not s: return None
    try: t = s.decode('latin-1')
    except: return None
    return t if all(32 <= ord(c) < 127 or c in '\t' for c in t) else None

NAMES = {}
import os
for fn in sorted(os.listdir('src')):
    if not fn.endswith('.cpp'): continue
    L = open('src/'+fn, errors='replace').read().split('\n')
    for i, l in enumerate(L):
        m = re.search(r'//\s*(?:FUNCTION|STUB):\s*EZ2AC_UNPACKED\s+(0x[0-9a-fA-F]+)', l)
        if not m: continue
        for j in range(i+1, min(i+6, len(L))):
            t = L[j].strip()
            if t and not t.startswith('//') and not t.startswith('/*'):
                nm = re.search(r'(?:(\w+)::)?(~?\w+)\s*\(', t)
                if nm: NAMES.setdefault(int(m.group(1),16),
                    (nm.group(1)+"::" if nm.group(1) else "")+nm.group(2))
                break

d = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
    '--no-show-raw-insn','--adjust-vma=0x%x'%0x400000,'--start-address=%#x'%lo,
    '--stop-address=%#x'%hi,'original/EZ2AC_Unpacked.exe'],
    capture_output=True, text=True).stdout

reg = {}          # register -> a description of what it holds
fmt = None        # the last format string an sprintf was given
pend = []         # pending pushes, newest first
rows = []
for l in d.splitlines():
    m = re.match(r'^\s*([0-9a-f]+):\t(\S+)\s*(.*)$', l)
    if not m: continue
    mn, op = m.group(2), m.group(3).split('#')[0].strip()

    t = re.match(r'^(e[a-z][a-z]),\[esi\+(0x[0-9a-f]+)\]$', op)
    if mn == 'lea' and t:
        reg[t.group(1)] = 'this+' + t.group(2); continue
    t = re.match(r'^(e[a-z][a-z]),\[esp\+(0x[0-9a-f]+)\]$', op)
    if mn == 'lea' and t:
        reg[t.group(1)] = 'buf'; continue
    t = re.match(r'^(e[a-z][a-z]),(0x[0-9a-f]+)$', op)
    if mn == 'mov' and t:
        s = cstr(int(t.group(2), 16))
        reg[t.group(1)] = ('"%s"' % s) if s else t.group(2); continue

    if mn == 'push':
        s = None
        if re.match(r'^0x[0-9a-f]+$', op): s = cstr(int(op, 16))
        pend.insert(0, ('"%s"' % s) if s else reg.get(op, op)); continue

    if mn == 'call':
        t = re.search(r'0x([0-9a-f]+)', op)
        nm = NAMES.get(int(t.group(1),16), 'sub_'+t.group(1)) if t else '<indirect>'
        if nm == 'sub_4788cf' or nm.endswith('sprintf'):
            fmt = next((p for p in pend if p.startswith('"')), None)
            rows.append(('sprintf', fmt or '?', 'buf'))
        elif nm.endswith('loadTexture') or nm.endswith('loadStr'):
            path = next((p for p in pend if p.startswith('"')), None)
            dst  = next((p for p in pend if p.startswith('this+')), None)
            # A load with no literal path is reading the buffer the LAST
            # sprintf filled - but only the next one, so consume `fmt` here.
            # Without that a format leaks down the file and four unrelated
            # loads all claim to be loading "Battle\\Battle_%dp.str".
            if path is None and fmt is not None:
                path, fmt = fmt + '  (via sprintf)', None
            rows.append((nm.split('::')[-1], path or 'buf', dst or '?'))
        elif nm.endswith('attach') or nm.endswith('setCell'):
            rows.append((nm.split('::')[-1], ', '.join(pend[:3]), reg.get('ecx','?')))
        pend = []; continue
    if mn.startswith('j') or mn == 'ret':
        pend = []

w = max(len(r[1]) for r in rows) if rows else 10
for k, a, b in rows:
    print("%-12s %-*s -> %s" % (k, min(w, 46), a[:46], b))
