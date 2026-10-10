#!/usr/bin/env python3
"""Render an objdump listing as pseudo-source: one line per call, with the
pending pushes as arguments and the last ecx load as `this`."""
import sys, re, collections

NAMES = {}
for l in open(sys.argv[2]) if len(sys.argv) > 2 else []:
    a, n = l.split(None, 1)
    NAMES[int(a, 16)] = n.strip()

lines = open(sys.argv[1]).read().splitlines()
I = []
pat = re.compile(r'^([0-9a-f]+):\t([0-9a-f ]+)\t(\S+)\s*(.*)$')
for l in lines:
    m = pat.match(l.strip())
    if m:
        I.append((int(m.group(1), 16), m.group(3), m.group(4).split('#')[0].strip()))

targets = set()
for a, mn, op in I:
    if mn[0] == 'j':
        m = re.match(r'^0x([0-9a-f]+)', op)
        if m:
            targets.add(int(m.group(1), 16))

def nm(v):
    return NAMES.get(v, '0x%x' % v)

pending = []
ecx = None
out = []
for a, mn, op in I:
    if a in targets:
        out.append('L_%x:' % a)
    if mn == 'push':
        pending.append(op.replace('$', ''))
        continue
    if mn == 'lea' and op.endswith('%ecx'):
        ecx = op.split(',')[0]
        continue
    if mn == 'mov' and op.endswith(',%ecx'):
        ecx = op.split(',')[0]
        continue
    if mn == 'call':
        m = re.match(r'^0x([0-9a-f]+)', op)
        t = nm(int(m.group(1), 16)) if m else op
        args = ', '.join(reversed(pending))
        out.append('  %-28s (%s)   [ecx=%s]' % (t, args, ecx))
        pending = []
        ecx = None
        continue
    if mn == 'add' and op.startswith('$0x') and '%esp' in op:
        continue
    out.append('  ;%-8s %s%s' % (mn, op, ('   <<pend ' + ','.join(pending) + '>>') if pending else ''))
    if pending and mn not in ('mov', 'movl', 'movb', 'lea', 'movzbl', 'movss'):
        pass
print('\n'.join(out))
