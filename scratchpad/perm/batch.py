#!/usr/bin/env python3
# batch.py RC.json [--lo 0.85] [--minutes 10] [-j 14] - run perm.py over every
# function reccmp scores in [lo, 1.0), highest first. Report-only: nothing in
# src/ is touched; a best of 0 is a candidate for ./match.sh, nothing more.
# Summary: scratchpad/perm/out/batch-<stamp>/summary.tsv (gitignored).
import argparse, json, os, re, subprocess, sys, time

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
# the worm static TU matches as a whole or not at all (CLAUDE.md)
SKIP_RANGES = [(0x409200, 0x40a400)]

ap = argparse.ArgumentParser()
ap.add_argument('rcjson')
ap.add_argument('--lo', type=float, default=0.85)
ap.add_argument('--minutes', type=float, default=10)
ap.add_argument('-j', type=int, default=14)
ap.add_argument('--resume', help='existing batch dir: skip addresses already in its summary')
ap.add_argument('--addrs', help='file of addresses (one per line): run only these')
ap.add_argument('--restart', type=int, default=0, help='passed to perm.py')
a = ap.parse_args()

d = json.load(open(a.rcjson))['data']
todo = [e for e in d if e.get('matching') is not None and a.lo <= e['matching'] < 1.0
        and e.get('type', 1) in (1, None)]
if a.addrs:
    want = {int(l.split()[0], 16) for l in open(a.addrs) if l.strip()}
    todo = [e for e in todo if int(e['address'], 16) in want]
todo.sort(key=lambda e: -e['matching'])
outdir = a.resume or os.path.join(HERE, 'out', 'batch-' + time.strftime('%Y%m%d-%H%M'))
os.makedirs(outdir, exist_ok=True)
summ = os.path.join(outdir, 'summary.tsv')
done = set()
if os.path.exists(summ):
    done = {l.split('\t')[0] for l in open(summ)}
else:
    open(summ, 'w').write('addr\tname\treccmp%\tstatus\tbaseline\tbest\tcandidates\torig_insns\tour_insns\n')

env = dict(os.environ, WINEPREFIX=ROOT + '/tools/wineprefix', WINEDEBUG='-all',
           DISPLAY='', WAYLAND_DISPLAY='')
print('%d functions, %.1f h at %g min each -> %s' % (len(todo), len(todo) * a.minutes / 60,
                                                      a.minutes, outdir), flush=True)
for k, e in enumerate(todo):
    addr = int(e['address'], 16)
    if e['address'] in done:
        continue
    if any(lo <= addr < hi for lo, hi in SKIP_RANGES):
        status, base, best, n, oi, ui = 'skip:worm-TU', '', '', '', '', ''
    else:
        t0 = time.time()
        try:
            r = subprocess.run([HERE + '/perm.sh', e['address'], '-j', str(a.j),
                                '--minutes', str(a.minutes), '--seed', str(addr & 0xffff),
                                '--name', e['name']] +
                               (['--restart', str(a.restart)] if a.restart else []),
                               cwd=ROOT, env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                               timeout=a.minutes * 60 + 600)
            txt = r.stdout.decode('latin-1')
        except subprocess.TimeoutExpired as x:
            txt = (x.stdout or b'').decode('latin-1') + '\nTIMEOUT'
        open(os.path.join(outdir, '%s.log' % e['address']), 'w').write(txt)
        m = re.search(r'original (\d+) insns .* ours (\d+) insns; baseline score (\d+)', txt)
        f = re.search(r'== done: (\d+) candidates .* baseline (\d+) -> best (\d+)', txt)
        fp = re.search(r'pct ([\d.]+) -> ([\d.]+)', txt)
        oi, ui = (m.group(1), m.group(2)) if m else ('', '')
        if f:
            n, base, best = f.group(1), f.group(2), f.group(3)
            if best == '0':
                status = 'MATCH-CANDIDATE'
            elif fp:   # the objective: the reccmp-like percentage, then the distance
                pa, pb = float(fp.group(1)), float(fp.group(2))
                gain = pb > pa or (pb == pa and int(best) < int(base))
                status = 'improved' if gain else 'no-gain'
            else:
                status = 'improved' if int(best) < int(base) else 'no-gain'
        elif 'already 0' in txt:
            n, base, best, status = '0', '0', '0', 'already-0'
        else:
            n, base, best = '', (m.group(3) if m else ''), ''
            last = [l for l in txt.strip().splitlines() if l.strip()]
            status = 'error:' + (last[-1][:80] if last else 'no output')
    with open(summ, 'a') as g:
        g.write('\t'.join([e['address'], e['name'], '%.2f' % (100 * e['matching']), status,
                           str(base), str(best), str(n), str(oi), str(ui)]) + '\n')
    print('[%d/%d] %s %s %.2f%% -> %s %s->%s' % (k + 1, len(todo), e['address'], e['name'],
                                                100 * e['matching'], status, base, best), flush=True)
print('batch finished', flush=True)
