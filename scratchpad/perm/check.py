#!/usr/bin/env python3
# check.py 0xADDR [...] - compile each function's CURRENT src/ TU once and
# print the permuter's score (0 = every instruction equal modulo addresses).
# A seconds-long pre-check while hand-tidying a candidate; ./match.sh decides.
import os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import perm, score as S, mutate as M

DIFF = '--diff' in sys.argv


def fmt(i):
    m, ops, t = i
    def o(x):
        if x[0] == 'r': return x[1]
        if x[0] == 'i': return hex(x[1])
        if x[0] == 'A': return 'ADDR'
        if x[0] == 'T': return '->#%s' % t
        if x[0] == 'CALL': return 'CALL'
        if x[0] == 'm':
            d = x[6] if isinstance(x[6], str) else hex(x[6])
            parts = [p for p in (x[3], ('%s*%d' % (x[4], x[5])) if x[4] else '', d) if p]
            return '%s[%s]' % ({1: 'b', 2: 'w', 4: 'd', 8: 'q', 16: 'x'}.get(x[1], ''), '+'.join(parts))
        return str(x)
    return (m + ' ' + ', '.join(o(x) for x in ops)).strip()


def show_diff(A, B, base):
    import difflib
    ca, cb = S._canon(A), S._canon(B)
    for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, ca, cb, autojunk=False).get_opcodes():
        if tag == 'equal':
            continue
        print('  @%-4d %s' % (i1, tag))
        for k in range(i1, i2):
            print('     - %s' % fmt(A[k]))
        for k in range(j1, j2):
            print('     + %s' % fmt(B[k]))


for arg in [x for x in sys.argv[1:] if not x.startswith('--')]:
    addr = int(arg, 16)
    path, src, s0, s1 = perm.find_function(addr)
    target, n = S.decode_orig(addr)
    wdir = os.path.join(HERE, 'out', 'check-0x%x' % addr)
    fn = src[s0:s1]
    node = M.function_node(M.parse(fn))
    d = node.child_by_field_name('declarator')
    while d is not None and d.type not in ('qualified_identifier', 'identifier',
                                           'destructor_name', 'field_identifier'):
        d = d.child_by_field_name('declarator')
    job = (wdir, path, src[:s0], fn, src[s1:], None, target, None, set())
    os.makedirs(wdir, exist_ok=True)
    with open(os.path.join(wdir, 'cand.cpp'), 'wb') as f:
        f.write(src)
    import subprocess
    subprocess.run(['wine', perm.TOOLS + '/msvc71/bin/cl.exe'] + perm.CFLAGS +
                   [perm.eh_model(path), '/Fo' + perm.towin(wdir + '/cand.obj'),
                    perm.towin(wdir + '/cand.cpp')], cwd=perm.ROOT, env=perm.ENV,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    o = S.Obj(wdir + '/cand.obj')
    sym = S.resolve_symbol(o, M.T(d))
    code, rel = o.function(sym)
    B = S.decode_obj(code, rel)
    print('0x%x %-45s score %d (%.2f%%)' % (addr, M.T(d), S.distance(target, B),
                                           S.exact_pct(target, B)), flush=True)
    if DIFF:
        show_diff(target, B, addr)
