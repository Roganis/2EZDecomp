#!/usr/bin/env python3
"""strcheck.py - verify every `// STRING:` marker in src/ against the original.

A marker that names the WRONG address is close to invisible: reccmp creates the
entity anyway, so nothing errors, and the only symptom is the losing operand
quietly failing to resolve in whichever function shares the address. That cost
EZ2DJMainGameDirector::m423a90 five points until the build log's "Dropped
duplicate address" line was read (its failure log was annotated with the CATCH
director's copy of the same sentence, 0x49daf0 instead of 0x48f980).

This checks all of them at once: read the bytes at the marked address and
compare against the literal on the following line. Run it after any batch that
copies a function from a sibling, which is when the error class appears.

Known false positive: a WIDE literal (L"...") is UTF-16 in the binary and reads
as a one-character byte string here. There is one such marker (0x488170).
"""
import re, glob, sys

BIN = sys.argv[1] if len(sys.argv) > 1 else 'original/EZ2AC_Unpacked.exe'
d = open(BIN, 'rb').read()


def at(a):
    o = a - 0x400000
    return None if o < 0 or o >= len(d) else d[o:o + 400].split(b'\0')[0]


def unesc(lit):
    out = bytearray(); i = 0
    while i < len(lit):
        c = lit[i]
        if c == '\\':
            n = lit[i + 1]
            if n in '01234567':
                j = i + 1; k = ''
                while j < len(lit) and lit[j] in '01234567' and len(k) < 3:
                    k += lit[j]; j += 1
                out.append(int(k, 8)); i = j; continue
            out.append({'n': 10, 't': 9, 'r': 13, '\\': 92, '"': 34, '0': 0}
                       .get(n, ord(n)))
            i += 2; continue
        out.append(ord(c)); i += 1
    return bytes(out)


bad = []; seen = {}
for f in sorted(glob.glob('src/*.cpp')):
    L = open(f).read().split('\n')
    for i, l in enumerate(L):
        m = re.search(r'// STRING: EZ2AC_UNPACKED (0x[0-9a-fA-F]+)', l)
        if not m:
            continue
        a = int(m.group(1), 16)
        seen.setdefault(a, []).append(f)
        lit = None
        for j in range(i + 1, min(i + 3, len(L))):
            q = re.search(r'"((?:[^"\\]|\\.)*)"', L[j])
            if q:
                lit = q.group(1); break
        if lit is None:
            bad.append((f, i + 1, hex(a), 'no literal on the following line'))
            continue
        want, got = unesc(lit), at(a)
        if got != want:
            bad.append((f, i + 1, hex(a),
                        'literal %d bytes, address holds %s'
                        % (len(want), len(got) if got is not None else '?')))

print("markers: %d  distinct: %d  shared: %d"
      % (sum(len(v) for v in seen.values()), len(seen),
         sum(len(v) - 1 for v in seen.values() if len(v) > 1)))
print("mismatches: %d" % len(bad))
for b in bad:
    print("  %s:%d %s  %s" % b)
