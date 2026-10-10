#!/usr/bin/env python3
"""wipaudit.py - what is actually IN wip/, as opposed to described there.

Two checks, both of which caught real defects on 2026-08-25:

  (a) comment lexing, properly - `/*` does not nest and a `*/` inside a
      comment ENDS it, so a header that mentions `p_*/q_*` closes itself
      early and leaves stray `*/` behind (loadez-411460.cpp);
  (b) whether the file contains a function BODY at all, counting the ones
      inside `#if 0` separately - six wip/ files are analysis notes with no
      implementation, and PORT-WORKQUEUE.md used to list four of them as
      "done for a port", including the 14.7 KB songselect ctor.

A file that lexes clean and has zero bodies is a NOTE, not a park.
"""
import os, re, sys

def lex(s):
    i, n, incom, start, ev = 0, len(s), False, 0, []
    while i < n:
        if incom:
            if s.startswith('*/', i): incom = False; i += 2; continue
            i += 1; continue
        if s.startswith('/*', i): incom = True; start = i; i += 2; continue
        if s.startswith('//', i):
            j = s.find('\n', i); i = n if j < 0 else j + 1; continue
        if s.startswith('*/', i):
            ev.append(('stray */', s.count('\n', 0, i) + 1)); i += 2; continue
        if s[i] in '"\'':
            q = s[i]; i += 1
            while i < n and s[i] != q: i += 2 if s[i] == '\\' else 1
            i += 1; continue
        i += 1
    if incom: ev.append(('unterminated /*', s.count('\n', 0, start) + 1))
    return ev

d = sys.argv[1] if len(sys.argv) > 1 else 'wip'
print("%-34s %6s %7s %8s  %s" % ("file", "lines", "bodies", "in #if0", "defects"))
notes = bad = 0
for fn in sorted(os.listdir(d)):
    if not fn.endswith('.cpp'): continue
    src = open(os.path.join(d, fn), errors='replace').read()
    ev = lex(src)
    nocom = re.sub(r'/\*.*?\*/', '', src, flags=re.S)
    nocom = re.sub(r'//[^\n]*', '', nocom)
    live = re.sub(r'#if\s+0\b.*?#endif', '', nocom, flags=re.S)
    b = len(re.findall(r'\)\s*(?:const\s*)?\{', live))
    i0 = len(re.findall(r'\)\s*(?:const\s*)?\{', nocom)) - b
    note = "NOTE ONLY" if b == 0 and i0 == 0 else ""
    if note: notes += 1
    if ev: bad += 1
    print("%-34s %6d %7d %8d  %s%s" % (fn, src.count('\n') + 1, b, i0,
          "; ".join("%s line %d" % e for e in ev), (" " + note) if note else ""))
print("\n%d analysis-only file(s), %d with comment defects" % (notes, bad))
