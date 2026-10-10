#!/usr/bin/env python3
"""twindelta.py TEMPLATE_VA NEW_VA — what a sibling function actually says.

`twins2.py` finds that two functions are near-copies; this says WHICH parts
differ and, more usefully, which of the differences are real.

The engine's families come in threes (keys / scratch / catch) and their members
are edits of one source. Diffing their listings raw is useless: the two sit at
different addresses so every branch target differs, and cl resolves the
constant-register coin differently in each, so `push $0x1` against `push %ebp`
appears dozens of times. Both are noise for a reconstruction.

So it masks in three passes and reports the ratio at each:

  structural  registers kept, addresses and immediates blanked
              -> how similar the CODE SHAPE is
  value       registers and branch targets blanked, data addresses and
              immediates kept -> the globals and constants each one uses
  semantic    the above plus the 0/1 constant-register coin folded away
              -> what is LEFT is the real difference between the two sources

Only the third listing is worth reading line by line. On 0x43ff00 vs 0x43ad10
it turns 66 differing regions into a handful of genuine ones (an extra option
counter, a different wrap limit, one comparison changed from != to <).
"""
import subprocess, difflib, re, sys, os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BIN  = ROOT + '/original/EZ2AC_Unpacked.exe'

def listing(va, window=0x4000):
    out = subprocess.run(
        ['objdump', '-D', '-b', 'binary', '-m', 'i386', '--adjust-vma=0x400000',
         '--start-address=0x%x' % va, '--stop-address=0x%x' % (va + window), BIN],
        capture_output=True, text=True).stdout.split('\n')[7:]
    ins = []
    for l in out:
        m = re.match(r'^\s*([0-9a-f]{6}):\t(.*)$', l)
        if not m:
            continue
        t = re.sub(r'\s+', ' ', m.group(2)).strip()
        # the encoding column bleeds into the text for long instructions; the
        # mnemonic is the first token that is not a pair of hex digits
        t = re.sub(r'^(?:[0-9a-f]{2} )+', '', t)
        # objdump wraps a long encoding onto a second line that holds nothing
        # but the trailing bytes; those are not instructions and they showed up
        # as bare `00` deltas.
        if re.fullmatch(r'[0-9a-f]{2}(?: [0-9a-f]{2})*', t):
            continue
        if t.startswith('int3'):
            if len(ins) and ins[-1].split()[0] in ('ret', 'jmp'):
                break
            continue
        if t:
            ins.append(t)
    return ins

def m_struct(l):
    l = re.sub(r'0x[0-9a-f]{5,8}', 'A', l)
    return re.sub(r'\$0x[0-9a-f]+', 'I', l)

def m_value(l):
    l = re.sub(r'%e[a-z]{2}', 'R', l)
    return re.sub(r'^((?:call|j[a-z]{1,3})) .*', r'\1 T', l)

def m_sem(l):
    l = re.sub(r'\$0x[01]\b', 'K', m_value(l))
    l = re.sub(r'\bR\b', 'K', l)
    # AT&T writes the operand-size suffix only when no register fixes it, so
    # `add K,mem` and `addl K,mem` are the same instruction seen through the
    # coin - fold the suffix away or every one of them reads as a difference.
    return re.sub(r'^(add|cmp|mov|sub|or|and|test|xor)l\b', r'\1', l)

a, b = listing(int(sys.argv[1], 16)), listing(int(sys.argv[2], 16))
print('%d instructions in the template, %d in the sibling' % (len(a), len(b)))
for name, fn in (('structural', m_struct), ('value', m_value), ('semantic', m_sem)):
    sm = difflib.SequenceMatcher(None, [fn(x) for x in a], [fn(x) for x in b],
                                 autojunk=False)
    ops = [o for o in sm.get_opcodes() if o[0] != 'equal']
    print('  %-11s %.3f   %d differing regions' % (name, sm.ratio(), len(ops)))

sm = difflib.SequenceMatcher(None, [m_sem(x) for x in a], [m_sem(x) for x in b],
                             autojunk=False)
print('\n== SEMANTIC DELTAS (everything else is the register coin) ==')
n = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        continue
    n += 1
    print('--- %d %s  template[%d:%d] sibling[%d:%d]' % (n, tag, i1, i2, j1, j2))
    for x in a[i1:i2][:8]:
        print('  T| ' + x)
    for x in b[j1:j2][:8]:
        print('  N| ' + x)
print('\n%d semantic regions' % n)
