#!/usr/bin/env python3
"""ecxsweep.py - which reconstructed FREE functions are really MEMBERS?

A __thiscall member that never touches `this` is byte-identical to a
__stdcall free function of the same arity: both take their arguments at
[esp+4]... and both end `ret 4N`. So the callee matches at 100% either way and
ONLY A CALL SITE CAN TELL - the tell being a `mov ecx,<object>` immediately
before the `call` that our reconstruction does not emit.

This walks the original's .text, finds every call to an address we have
annotated as a free function, and reports how many of those sites load ecx
immediately beforehand.

READ THE RESULT WITH TWO FILTERS, or it lies:

  * A `static` helper legitimately takes arguments in ecx (cl gives a static
    that is not address-taken its own register convention), so the worm mesh
    TU's helpers show up and are NOT defects.
  * A caller that is ALREADY AT 100% would clear the site - the ecx load then
    belongs to that caller's own neighbouring code and our build emits it too.
    BUT FIND THE ENCLOSING FUNCTION BY ITS REAL END, NOT BY THE NEXT
    ANNOTATION. Bisecting the annotation list put setVec2's site inside
    GFBackGround::~GFBackGround @0x45c550 and angleToY's inside
    SongSelectDirector::m439dc0 @0x439dc0, and both are 100%, which read as two
    clean false positives. Both are wrong: ~GFBackGround ends at 0x45c5ea and
    the site is at 0x45c642, m439dc0 ends at 0x439efa and the site is at
    0x43a0aa. The sites are in unannotated neighbours, so nothing clears them.
    Resolve the end with objdump before believing a caller owns a site.

Confirmed by this method: runGameOver @0x415d70, which src/moderunner.cpp had
declared free on the grounds that its call sites do not set ecx. All five do.
"""
import subprocess, re, glob, sys

free = {}
for f in glob.glob('src/*.cpp'):
    L = open(f, errors='replace').read().split('\n')
    for i, l in enumerate(L):
        m = re.match(r'\s*//\s*FUNCTION:\s*EZ2AC_UNPACKED\s+0x0*([0-9a-fA-F]+)\s*(folded)?\s*$', l)
        if not m:
            continue
        for j in range(i + 1, min(i + 6, len(L))):
            t = L[j].strip()
            if not t or t.startswith('//') or t.startswith('/*') or t.startswith('*'):
                continue
            if '(' in t and '::' not in t.split('(')[0] and not t.startswith('#'):
                free[int(m.group(1), 16)] = (f, t)
            break

o = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386', '-Mintel',
                    '--no-show-raw-insn', '--adjust-vma=0x400000',
                    '--start-address=0x401000', '--stop-address=0x478000',
                    'original/EZ2AC_Unpacked.exe'],
                   capture_output=True, text=True).stdout.split('\n')
ins = []
for l in o:
    m = re.match(r'\s*([0-9a-f]+):\t(.*)', l)
    if m:
        ins.append((int(m.group(1), 16), re.sub(r'\s+', ' ', m.group(2).strip())))

sites = {}
for i, (pc, t) in enumerate(ins):
    m = re.match(r'call 0x([0-9a-f]+)$', t)
    if not m:
        continue
    tgt = int(m.group(1), 16)
    if tgt not in free:
        continue
    prev = ins[i - 1][1] if i else ''
    ecx = bool(re.match(r'mov ecx,', prev))     # IMMEDIATELY before, nothing else
    e = sites.setdefault(tgt, [0, 0, []])
    e[0 if ecx else 1] += 1
    if ecx:
        e[2].append('%06x <- %s' % (pc, prev))

bad = sorted([(t, v) for t, v in sites.items() if v[0] and not v[1]], key=lambda x: -x[1][0])
print('%d free functions are called; %d have EVERY call site loading ecx\n' % (len(sites), len(bad)))
for t, v in bad:
    print('0x%06x  %d site(s)  %s  %s' % (t, v[0], free[t][0], free[t][1][:52]))
    for s in v[2]:
        print('        %s' % s)
