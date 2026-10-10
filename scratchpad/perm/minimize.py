#!/usr/bin/env python3
# minimize.py 0xADDR [best.cpp] - shrink a permuter candidate to the edits
# that matter. Splits candidate-vs-source into line hunks and greedily
# reverts each one, keeping the revert whenever the score stays at the
# candidate's (exact %, distance). Prints the minimal function and its diff;
# src/ is always restored. The result still needs a person: rename _pN
# locals, fix the indentation, update comments, then ./match.sh.
import difflib, os, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import perm, score as S

addr = int(sys.argv[1], 16)
best = open(sys.argv[2] if len(sys.argv) > 2 else
            os.path.join(HERE, 'out', '0x%x' % addr, 'best.cpp'), 'rb').read()
path, src, s0, s1 = perm.find_function(addr)
base = src[s0:s1]
target, _ = S.decode_orig(addr)
wdir = os.path.join(HERE, 'out', 'min-0x%x' % addr)


BASE_CONTENT = None
ALLOWED = set()


def key(fn):
    r = perm.compile_score((wdir, path, src[:s0], fn, src[s1:], SYM, target, BASE_CONTENT, ALLOWED))
    if not r['ok']:
        return None
    return (round(-r['pct'], 2), r['score'])


# the symbol, from the baseline compile perm.py left behind
import re
log = open(os.path.join(HERE, 'out', '0x%x' % addr, 'log.txt')).read()
SYM = re.findall(r'\((\?[^)]*|_\w+)\) in ', log)[-1]

a = base.decode('latin-1').splitlines(True)
b = best.decode('latin-1').splitlines(True)
ops = [o for o in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes() if o[0] != 'equal']
# the behaviour guard perm.py applies: callees (multiset), globals and
# constants of every trial must equal the SOURCE's
r0 = perm.compile_score((wdir, path, src[:s0], base, src[s1:], SYM, target, None, set()))
o0 = S.Obj(os.path.join(wdir, 'cand.obj'))
c0, rel0 = o0.function(SYM)
BASE_CONTENT = S.content(c0, rel0)
ALLOWED = S.orig_consts(addr, len(c0) + 0x1000) | BASE_CONTENT[2]
goal = key(best)
print('candidate key %s, %d hunks' % (goal, len(ops)))


def build(keep):
    """apply only the hunks in `keep` to the base text."""
    out, ia = [], 0
    for idx, (tag, i1, i2, j1, j2) in enumerate(ops):
        out.extend(a[ia:i1])
        out.extend(b[j1:j2] if idx in keep else a[i1:i2])
        ia = i2
    out.extend(a[ia:])
    return ''.join(out).encode('latin-1')


import collections, mutate as M


def stmts(t):
    """what the side-effecting statements DO, not how they are spelled: each
    call by its callee, each store by its target. Full statement text missed
    a rewritten call that was then dropped (`drawTex(f_bc664, wx - 3.0f, ..)`
    became `drawTex(f_bc664, _p1, ..)`; reverting the new line and keeping
    the deletion left neither, and both texts' counts looked consistent)."""
    c = collections.Counter()
    for n in M.walk(M.parse(t)):
        if n.type == 'call_expression':
            f = n.child_by_field_name('function')
            c[b'call:' + re.sub(rb'\s+', b'', f.text if f is not None else b'?')] += 1
        elif n.type in ('assignment_expression', 'update_expression'):
            lhs = n.child_by_field_name('left') or n.child_by_field_name('argument')
            if lhs is not None and not re.match(rb'^_p\d+$', lhs.text):
                c[b'set:' + re.sub(rb'\s+', b'', lhs.text)] += 1
    return c


SB, SC = stmts(base), stmts(best)


def sane(t):
    """A MOVE is two hunks (delete here, insert there); reverting only one
    of them duplicates or drops the statement, and the score may not notice
    (a duplicated `miss += 2;` slipped through on GFResultDirector's ctor).
    Every statement's count must stay between its source and candidate
    counts."""
    st = stmts(t)
    for k in set(st) | set(SB) | set(SC):
        if not (min(SB[k], SC[k]) <= st[k] <= max(SB[k], SC[k])):
            return False
    return True


keep = set(range(len(ops)))
for idx in range(len(ops)):
    trial = keep - {idx}
    if not sane(build(trial)):
        print('  hunk %d kept (reverting it alone would duplicate or drop a statement)' % idx)
        continue
    k = key(build(trial))
    if k is not None and k <= goal:
        keep = trial
        print('  hunk %d not needed' % idx)
    else:
        print('  hunk %d NEEDED (%s)' % (idx, k))
final = build(keep)
print('minimal key %s with %d of %d hunks' % (key(final), len(keep), len(ops)))
open(os.path.join(HERE, 'out', '0x%x' % addr, 'minimal.cpp'), 'wb').write(final)
sys.stdout.writelines(difflib.unified_diff(a, final.decode('latin-1').splitlines(True),
                                           'before', 'minimal'))
