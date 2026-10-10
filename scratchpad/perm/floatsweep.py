#!/usr/bin/env python3
# floatsweep.py RC.json - every non-100% function: aligned instructions whose
# FLOAT POOL CONSTANT differs from the original's (score.py's F:<bits>
# tokens). Reads the current build's objects in build/obj/ - run ./match.sh
# first - so nothing is compiled. A hit is a candidate BEHAVIOUR bug (a wrong
# literal), the kind ModeSelectDirector::update's -40 was; read each by hand.
import difflib, glob, json, os, re, struct, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import score as S

ROOT = S.ROOT
d = json.load(open(sys.argv[1]))['data']
todo = {int(e['address'], 16): e['name'] for e in d
        if e.get('type') == 1 and e.get('matching') is not None and e['matching'] < 1.0}


def plain(mangled):
    """?meth@Class@Outer@@... -> Outer::Class::meth (ctor ??0, dtor ??1)."""
    if not mangled.startswith('?'):
        return mangled.lstrip('_')
    m = re.match(r'\?(\?[01])?([^@]*)@((?:[^@]+@)*)@', mangled)
    if not m:
        return None
    special, meth, scope = m.groups()
    if special:                          # ??0Class@Outer@@ : the class is first
        scope = meth + '@' + scope
    parts = [p for p in scope.split('@') if p][::-1]
    if special and not parts:
        return None
    if special == '?0':
        meth = parts[-1]
    elif special == '?1':
        meth = '~' + parts[-1]
    return '::'.join(parts + [meth])


objs = {}
for f in sorted(glob.glob(os.path.join(ROOT, 'build', 'obj', '*.obj'))):
    try:
        o = S.Obj(f)
    except Exception:
        continue
    for sy in o.functions():
        p = plain(sy['name'])
        if p:
            objs.setdefault(p, []).append((o, sy['name']))


def fval(tok):
    h = tok[2:]
    if len(h) == 8:
        return struct.unpack('<f', int(h, 16).to_bytes(4, 'little'))[0]
    return struct.unpack('<d', int(h, 16).to_bytes(8, 'little'))[0]


hits = 0
for addr, name in sorted(todo.items()):
    cands = objs.get(name, [])
    if len(cands) != 1:
        continue
    o, sym = cands[0]
    try:
        code, rel = o.function(sym)
        B = S.decode_obj(code, rel)
        A, n = S.decode_orig(addr)
    except Exception:
        continue
    ca, cb = S._canon(S._notgt(A)), S._canon(S._notgt(B))
    # align with the float constants BLANKED so a wrong one still aligns
    def blank(L):
        return [(m, tuple(o[:6] + ('F',) if o[0] == 'm' and isinstance(o[6], str)
                          and o[6].startswith('F:') else o for o in ops), t) for m, ops, t in L]
    sm = difflib.SequenceMatcher(None, blank(ca), blank(cb), autojunk=False)
    out = []
    for blk in sm.get_matching_blocks():
        for k in range(blk.size):
            ia, ib = blk.a + k, blk.b + k
            for x, y in zip(A[ia][1], B[ib][1]):
                if x[0] == y[0] == 'm' and isinstance(x[6], str) and isinstance(y[6], str) \
                        and x[6].startswith('F:') and y[6].startswith('F:') and x[6] != y[6]:
                    out.append('    #%d %s: original %g, ours %g' % (ia, A[ia][0], fval(x[6]), fval(y[6])))
    # and the MULTISET: a constant used more or fewer times than the
    # original (the -40 bug: ours 40.0 x2, the original 40.0 x1 + an xorps)
    import collections
    def fset(L):
        return collections.Counter(o[6] for _, ops, _ in L for o in ops
                                   if o[0] == 'm' and isinstance(o[6], str) and o[6].startswith('F:'))
    fa, fb = fset(A), fset(B)
    if fa != fb and fb:                  # fb empty: our constants did not resolve
        extra = fb - fa
        miss = fa - fb
        out.append('    multiset: ours extra %s, missing %s' % (
            {round(fval(k), 6): v for k, v in extra.items()},
            {round(fval(k), 6): v for k, v in miss.items()}))
    if out:
        hits += 1
        print('0x%x %s' % (addr, name))
        print('\n'.join(out[:8]))
print('%d functions with a differing float constant (of %d checked)' % (hits, len(todo)))
