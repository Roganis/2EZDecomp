#!/usr/bin/env python3
# regions.py 0xADDR [--gap 4] [--pad 0] - which SOURCE LINES does a function's reccmp
# diff fall on? Runs reccmp --verbose on the current build (run ./match.sh
# first if src/ changed), attributes every changed instruction to the nearest
# preceding `(file.cpp:NNN)` annotation in its hunk, and merges the lines into
# spans. Feed a span to `perm.sh 0xADDR --region A-B` so a 2,000-instruction
# function is searched where it differs, not everywhere.
import os, re, subprocess, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
args = [a for a in sys.argv[1:] if not a.startswith('--')]
gap = int(sys.argv[sys.argv.index('--gap') + 1]) if '--gap' in sys.argv else 4
pad = int(sys.argv[sys.argv.index('--pad') + 1]) if '--pad' in sys.argv else 0
args = [a for a in args if not a.isdigit()]
addr = args[0]
env = dict(os.environ, WINEPREFIX=ROOT + '/tools/wineprefix', WINEDEBUG='-all',
           DISPLAY='', WAYLAND_DISPLAY='')
r = subprocess.run([ROOT + '/.venv/bin/reccmp-reccmp', '--target', 'EZ2AC_UNPACKED', '--verbose',
                    addr], cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
txt = re.sub(r'\x1b\[[0-9;]*m', '', r.stdout.decode('utf-8', 'replace'))

lines, cur, fname = [], None, None
for l in txt.splitlines():
    if l.startswith('@@'):
        cur = None                       # a new hunk: no attribution carried over
        continue
    m = re.search(r'\((\w+\.cpp):(\d+)\)', l)
    if m:
        fname, cur = m.group(1), int(m.group(2))
    body = l.split(':', 1)[1].strip() if ':' in l else ''
    if body.startswith(('-', '+')) and cur is not None:
        lines.append(cur)

if not lines:
    print('no attributable changed lines (100%, or no line annotations)')
    sys.exit(0)
lines = sorted(set(lines))
spans = [[lines[0], lines[0]]]
for x in lines[1:]:
    if x - spans[-1][1] <= gap:
        spans[-1][1] = x
    else:
        spans.append([x, x])
print('%s: %d changed source lines in %d spans' % (fname, len(lines), len(spans)))
# pad each span so the statements around a changed line fit inside it (a
# mutation is only kept when ALL its edits lie in the region)
for a, b in spans:
    print('  --region %d-%d' % (a - pad, b + pad))
