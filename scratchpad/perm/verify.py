#!/usr/bin/env python3
# verify.py 0xADDR [best.cpp] - splice a permuter candidate into its src/ TU,
# run ./match.sh 0xADDR, print reccmp's verdict, and ALWAYS restore the file.
# --keep leaves the candidate in place when reccmp says 100%.
import os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import perm

args = [x for x in sys.argv[1:] if not x.startswith('--')]
keep = '--keep' in sys.argv
addr = int(args[0], 16)
cand = open(args[1] if len(args) > 1 else os.path.join(HERE, 'out', '0x%x' % addr, 'best.cpp'), 'rb').read()
path, src, s0, s1 = perm.find_function(addr)
new = src[:s0] + cand + src[s1:]
ok = False
try:
    open(path, 'wb').write(new)
    r = subprocess.run(['./match.sh', '0x%x' % addr], cwd=perm.ROOT, stdout=subprocess.PIPE,
                       stderr=subprocess.STDOUT)
    out = r.stdout.decode('latin-1')
    out = re.sub(r'\x1b\[[0-9;]*m', '', out)
    lines = [l for l in out.splitlines() if 'WARNING' not in l and 'INFO' not in l
             and not l.startswith('wine:') and not l.startswith('compiling')]
    tail = '\n'.join(lines[-60:])
    ok = bool(re.search(r'is 100(\.00)?% similar|100% match', out)) and 'effective' not in out
    print(tail)
    print('VERDICT 0x%x: %s' % (addr, '100% MATCH' if ok else 'not 100%'))
finally:
    if not (keep and ok):
        open(path, 'wb').write(src)
