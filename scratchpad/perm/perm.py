#!/usr/bin/env python3
# perm.py - a source permuter for the matching decomp (decomp-permuter's idea,
# rebuilt for C++ / VC7.1 / COFF, which the stock one cannot parse or score).
#
#   scratchpad/perm/perm.sh 0x412e90 [-j 8] [--minutes 30] [--sym MANGLED]
#
# Finds the function's `// FUNCTION:` marker in src/, compiles copies of its
# TU with the function body rewritten (mutate.py), scores each against the
# original's bytes (score.py) and hill-climbs. Output, all gitignored, in
# scratchpad/perm/out/<addr>/: best.cpp (the function alone), best.diff,
# log.txt. Score 0 means "every instruction equal modulo addresses" - THEN
# run ./match.sh <addr>; reccmp is still the only ground truth.
import argparse, concurrent.futures as cf, difflib, hashlib, json, os, random, re
import subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import score as S
import mutate as M

ROOT = S.ROOT
TOOLS = ROOT + '/tools'


def towin(p):
    return 'Z:' + p.replace('/', '\\')


ENV = dict(os.environ, WINEPREFIX=TOOLS + '/wineprefix', WINEDEBUG='-all', DISPLAY='',
           WAYLAND_DISPLAY='',
           INCLUDE=';'.join(towin(TOOLS + p) for p in
                            ('/msvc71/include', '/platformsdk/Include', '/dx8sdk/include')))
CFLAGS = ['/nologo', '/c', '/O2', '/MT', '/GS', '/Gy', '/GF', '/Gd', '/G7', '/arch:SSE2',
          '/Iinclude', '/Iinclude/dx', '/Isrc']


def eh_model(srcfile):        # keep in step with match.sh's eh_model
    return '/EHs' if srcfile.endswith('src/kengineapp.cpp') else '/EHsc'

# ------------------------------------------------------------ locate ----

def find_function(addr, alt=None):
    """-> (source path, source bytes, start, end) of the definition under the
    FUNCTION marker for addr. alt: read this file instead of src/ (a scratch
    copy) - the TU keeps its src/ path for the exception model."""
    pat = re.compile(rb'//\s*FUNCTION:\s*EZ2AC_UNPACKED\s+0x0*%x\b' % addr, re.I)
    hits = []
    for d in ('src',):
        for f in sorted(os.listdir(os.path.join(ROOT, d))):
            if f.endswith('.cpp'):
                p = os.path.join(ROOT, d, f)
                b = open(p, 'rb').read()
                for m in pat.finditer(b):
                    hits.append((p, b, m.end()))
    if len(hits) != 1:
        raise SystemExit('expected one FUNCTION marker for 0x%x, found %d' % (addr, len(hits)))
    p, b, at = hits[0]
    if alt:
        b = open(alt, 'rb').read()
        at = pat.search(b).end()
    root = M.parse(b)
    for n in M.walk(root):
        if n.type == 'function_definition' and n.start_byte >= at:
            if b'__asm' in n.text:
                raise SystemExit('function has inline __asm - not mutated in v1')
            return p, b, n.start_byte, n.end_byte
    raise SystemExit('no function definition after the marker')


# ----------------------------------------------------------- compile ----
# The candidate TU is compiled from scratchpad/perm/out/<addr>/w<k>/, so a
# quoted #include resolves through /Isrc and /Iinclude, never the TU's dir.

def compile_score(job):
    """Worker: build one candidate TU and score it. Runs in a subprocess."""
    (wdir, srcfile, prefix, fntext, suffix, sym, target, base_content, allowed) = job
    os.makedirs(wdir, exist_ok=True)
    cpp = os.path.join(wdir, 'cand.cpp')
    obj = os.path.join(wdir, 'cand.obj')
    with open(cpp, 'wb') as f:
        f.write(prefix + fntext + suffix)
    if os.path.exists(obj):
        os.remove(obj)
    r = subprocess.run(['wine', TOOLS + '/msvc71/bin/cl.exe'] + CFLAGS + [eh_model(srcfile),
                       '/Fo' + towin(obj), towin(cpp)], cwd=ROOT, env=ENV,
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if not os.path.exists(obj):
        err = [l for l in r.stdout.decode('latin-1').splitlines() if 'error' in l]
        msg = re.sub(r'^.*?cand\.cpp\(\d+\) : ', '', err[0]) if err else 'no obj'
        return dict(ok=False, why='compile: ' + msg[:100], src=fntext)
    try:
        o = S.Obj(obj)
        code, rel = o.function(sym)
    except Exception as e:
        return dict(ok=False, why='obj: %r' % e)
    if base_content is not None:
        why = S.guard_ok(base_content, S.content(code, rel), allowed)
        if why:
            return dict(ok=False, why='guard: ' + why)
    B = S.decode_obj(code, rel)
    return dict(ok=True, score=S.distance(target, B), pct=S.exact_pct(target, B),
                content=S.content(code, rel) if base_content is None else None)

# ------------------------------------------------------------ driver ----

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('addr')
    ap.add_argument('--name', help='Class::method, if the definition does not say')
    ap.add_argument('--sym', help='mangled COFF symbol, if resolution is ambiguous')
    ap.add_argument('-j', type=int, default=max(1, (os.cpu_count() or 2) - 2))
    ap.add_argument('--minutes', type=float, default=20)
    ap.add_argument('--max', type=int, default=0, help='stop after N candidates')
    ap.add_argument('--pool', type=int, default=16)
    ap.add_argument('--only', help='comma list of mutation kinds')
    ap.add_argument('--seed', type=int, default=None)
    ap.add_argument('--no-guard', action='store_true')
    ap.add_argument('--src', help='mutate a scratch copy of the TU instead of src/')
    ap.add_argument('--start', help='seed the pool with this function text (e.g. an earlier best.cpp)')
    ap.add_argument('--region', help='A-B: source-file LINE range; only mutate inside it '
                    '(regions.py finds the lines of a function\'s reccmp diff hunks)')
    ap.add_argument('--restart', type=int, default=0,
                    help='after N candidates without a new best, reset the pool to the source')
    a = ap.parse_args()
    addr = int(a.addr, 16)
    rng = random.Random(a.seed)
    only = set(a.only.split(',')) if a.only else None

    srcfile, src, s0, s1 = find_function(addr, a.src)
    prefix, base_fn, suffix = src[:s0], src[s0:s1], src[s1:]
    import tyoracle
    tyoracle.set_context(src)     # types for name_subexpr
    region_lines = None
    if a.region:
        l1, l2 = (int(x) for x in a.region.split('-'))
        line0 = src[:s0].count(b'\n') + 1          # source line of the function's first byte
        offs = [0] + [i + 1 for i, ch in enumerate(base_fn) if ch == 0x0a]
        lo = offs[max(0, l1 - line0)] if l1 - line0 < len(offs) else len(base_fn)
        hi = offs[l2 - line0 + 1] if l2 - line0 + 1 < len(offs) else len(base_fn)
        region_lines = (l1, l2)
    out = os.path.join(HERE, 'out', '0x%x' % addr)
    os.makedirs(out, exist_ok=True)
    log = open(os.path.join(out, 'log.txt'), 'a')

    def say(msg):
        print(msg, flush=True)
        log.write(msg + '\n'); log.flush()

    target, tlen = S.decode_orig(addr)
    allowed = S.orig_consts(addr, tlen)

    # baseline: compile once, resolve the symbol, record the content to guard
    fn = M.function_node(M.parse(base_fn))
    decl = fn.child_by_field_name('declarator')
    while decl is not None and decl.type not in ('qualified_identifier', 'identifier',
                                                  'destructor_name', 'field_identifier'):
        decl = decl.child_by_field_name('declarator')
    name = a.name or (M.T(decl) if decl is not None else None)
    wdir0 = os.path.join(out, 'w0')
    os.makedirs(wdir0, exist_ok=True)
    with open(os.path.join(wdir0, 'cand.cpp'), 'wb') as f:
        f.write(src)
    subprocess.run(['wine', TOOLS + '/msvc71/bin/cl.exe'] + CFLAGS + [eh_model(srcfile),
                   '/Fo' + towin(wdir0 + '/cand.obj'), towin(wdir0 + '/cand.cpp')],
                   cwd=ROOT, env=ENV, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    o = S.Obj(wdir0 + '/cand.obj')
    sym = a.sym or S.resolve_symbol(o, name)
    code, rel = o.function(sym)
    B = S.decode_obj(code, rel)
    base_score = S.distance(target, B)
    base_content = None if a.no_guard else S.content(code, rel)
    allowed |= base_content[2] if base_content else set()
    say('== 0x%x %s (%s) in %s%s' % (addr, name, sym, os.path.relpath(srcfile, ROOT),
                                   ' lines %d-%d' % region_lines if region_lines else ''))
    say('   original %d insns / %d bytes, ours %d insns; baseline score %d (%.2f%% exact)'
        % (len(target), tlen, len(B), base_score, S.exact_pct(target, B)))
    if base_score == 0:
        say('   already 0 - nothing to do')
        return

    # The OBJECTIVE is reccmp's measure first - the exact-instruction
    # percentage, which tracks reccmp's line ratio - and the weighted
    # distance only as the tie-break that gives the search a gradient. The
    # distance alone counts a register rename as cheap, and climbed to code
    # reccmp rates LOWER (EZ2DJMainGameDirector::m4221b0: distance 26 -> 17,
    # reccmp 97.06% -> 96.57%).
    def K(score, pct):
        return (int(round(10000 - pct * 100)), score)

    base_pct = S.exact_pct(target, B)
    base_key = K(base_score, base_pct)
    pool = [(base_key, base_fn, [])]               # (key, text, mutation history)
    seen = {hashlib.sha1(base_fn).digest()}
    best = base_key
    best_pct = base_pct
    if a.start:
        st = open(a.start, 'rb').read()
        r = compile_score((os.path.join(out, 'w0s'), srcfile, prefix, st, suffix, sym, target,
                           base_content, allowed))
        if r['ok']:
            pool.insert(0, (K(r['score'], r['pct']), st, ['start']))
            pool.sort(key=lambda x: x[0])
            seen.add(hashlib.sha1(st).digest())
            if K(r['score'], r['pct']) < best:
                best, best_pct = K(r['score'], r['pct']), r['pct']
            say('   start %s: score %d (%.2f%%)' % (os.path.basename(a.start), r['score'], r['pct']))
        else:
            say('   start %s rejected: %s' % (a.start, r['why']))
    stats = dict(n=0, fail=0, guard=0, dup=0)
    last_gain = [0]
    hist_sc = __import__('collections').Counter()
    whys = __import__('collections').Counter()
    t_end = time.time() + a.minutes * 60
    t0 = time.time()

    def pick():
        if rng.random() < 0.5:
            return pool[0]
        return rng.choice(pool)

    def set_region(txt):
        if not region_lines:
            return
        offs = [0] + [i + 1 for i, ch in enumerate(txt) if ch == 0x0a]
        line0 = src[:s0].count(b'\n') + 1
        extra = txt.count(b'\n') - base_fn.count(b'\n')   # lines added by earlier edits
        l1 = region_lines[0] - line0
        l2 = region_lines[1] - line0 + 1 + max(0, extra)
        M.REGION = (offs[min(max(0, l1), len(offs) - 1)], offs[l2] if l2 < len(offs) else len(txt))

    def make():
        for tries in range(200):
            sc, txt, hist = pick()
            set_region(txt)
            # a small function exhausts single edits fast: go deeper on repeats
            new, kinds = M.mutate(txt, rng, n=rng.randint(1, 2 + tries // 10), only=only)
            if not kinds:
                continue
            h = hashlib.sha1(new).digest()
            if h in seen:
                stats['dup'] += 1
                continue
            seen.add(h)
            return new, hist + kinds
        return None

    free = list(range(1, a.j + 1))       # one worker dir per in-flight job

    def submit(ex):
        c = make()
        if c is None:
            return None
        txt, hist = c
        slot = free.pop()
        job = (os.path.join(out, 'w%d' % slot), srcfile, prefix, txt, suffix, sym, target,
               base_content, allowed)
        f = ex.submit(compile_score, job)
        f.meta = (txt, hist, slot)
        return f

    with cf.ProcessPoolExecutor(a.j) as ex:
        live = set()
        for _ in range(a.j):
            f = submit(ex)
            if f: live.add(f)
        while live:
            done, live = cf.wait(live, return_when=cf.FIRST_COMPLETED)
            for f in done:
                stats['n'] += 1
                r = f.result()
                txt, hist, slot = f.meta
                free.append(slot)
                if not r['ok']:
                    stats['guard' if r['why'].startswith('guard') else 'fail'] += 1
                    whys[r['why'][:90]] += 1
                    if r['why'].startswith('compile') and whys[r['why'][:90]] == 1:
                        with open(os.path.join(out, 'compile_fail_%d.cpp' % len(whys)), 'wb') as g:
                            g.write(r.get('src', b''))
                else:
                    sc = r['score']
                    k = K(sc, r['pct'])
                    hist_sc[sc] += 1
                    worst = pool[-1][0] if len(pool) >= a.pool else (1 << 30, 0)
                    if k <= worst:
                        pool.append((k, txt, hist))
                        pool.sort(key=lambda x: x[0])
                        del pool[a.pool:]
                    if k < best:
                        best, best_pct = k, r['pct']
                        last_gain[0] = stats['n']
                        say('[%5ds #%d] score %d (%.2f%%) via %s' % (time.time() - t0, stats['n'], sc,
                                                                    r['pct'], ' > '.join(hist)))
                        with open(os.path.join(out, 'best.cpp'), 'wb') as g:
                            g.write(txt)
                        with open(os.path.join(out, 'best_%d.cpp' % sc), 'wb') as g:
                            g.write(txt)
                        d = difflib.unified_diff(base_fn.decode('latin-1').splitlines(True),
                                                 txt.decode('latin-1').splitlines(True),
                                                 os.path.relpath(srcfile, ROOT) + ' (before)',
                                                 os.path.relpath(srcfile, ROOT) + ' (candidate)')
                        with open(os.path.join(out, 'best.diff'), 'w') as g:
                            g.writelines(d)
                if stats['n'] % 50 == 0:
                    say('   ... %d tried (%d compile-fail, %d guard-reject, %d dup), best %d, %.1f/s'
                        % (stats['n'], stats['fail'], stats['guard'], stats['dup'], best[1],
                           stats['n'] / (time.time() - t0)))
                if a.restart and stats['n'] - last_gain[0] >= a.restart:
                    # a plateau: start again from the source, down another path
                    pool[:] = [(base_key, base_fn, [])]
                    last_gain[0] = stats['n']
                    say('   [%5ds #%d] restart from the source (best so far %d)'
                        % (time.time() - t0, stats['n'], best[1]))
                stop = best[1] == 0 or time.time() > t_end or (a.max and stats['n'] >= a.max)
                if not stop:
                    nf = submit(ex)
                    if nf: live.add(nf)
    say('   score histogram: %s' % ', '.join('%d:%d' % kv for kv in sorted(hist_sc.items())[:12]))
    for w, k in whys.most_common(5):
        say('   reject x%d: %s' % (k, w))
    say('== done: %d candidates in %ds; baseline %d -> best %d; pct %.2f -> %.2f%s'
        % (stats['n'], time.time() - t0, base_score, best[1], base_pct, best_pct,
           '  (MATCH CANDIDATE - run ./match.sh 0x%x)' % addr if best[1] == 0 else ''))


if __name__ == '__main__':
    main()
