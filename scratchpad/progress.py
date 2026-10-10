#!/usr/bin/env python3
# progress.py RECCMP.json [OUTDIR] - the README's progress figures.
#
# Draws the classic decomp progress MAP: the game's code in address order,
# one cell per 64 bytes, each cell coloured by how closely the function that
# owns it matches (byte-identical, 95-99.9%, 85-95%, below 85%) - so a
# function's area is its size. Writes progress-light.svg / progress-dark.svg
# (for a <picture> switch), two badges, and progress.json.
#
# Function sizes are measured on YOUR copy of the original executable
# (original/EZ2AC_Unpacked.exe) - only sizes, addresses and percentages
# reach the output, never bytes. Needs a reccmp --json from a current build:
#   ./match.sh && .venv/bin/reccmp-reccmp --target EZ2AC_UNPACKED --nolib --json rc.json
#   .venv/bin/python scratchpad/progress.py rc.json docs/progress
import json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, 'perm'))
import score as S

CELL_BYTES = 64          # one cell = 64 bytes of original code
COLS = 128
PITCH = 7                # px per cell (6 px mark + 1 px surface gap)

# Ordinal blue ramp from the dataviz reference palette, validated with
# scripts/validate_palette.js --ordinal (light: light->dark = worse->better;
# dark: reversed, so a better match is always the stronger mark).
TIERS = [  # (label, test)
    ('byte-identical', lambda m: m >= 1.0),
    ('95 - 99.9%', lambda m: m >= 0.95),
    ('85 - 95%', lambda m: m >= 0.85),
    ('below 85%', lambda m: True),
]
THEME = {
    'light': dict(surface='#fcfcfb', ink='#0b0b0b', ink2='#52514e', empty='#ecebe7',
                  tiers=['#104281', '#2a78d6', '#5598e7', '#86b6ef']),
    'dark': dict(surface='#1a1a19', ink='#ffffff', ink2='#c3c2b7', empty='#2a2a28',
                 tiers=['#cde2fb', '#6da7ec', '#2a78d6', '#184f95']),
}


def tier(m):
    for i, (_, t) in enumerate(TIERS):
        if t(m):
            return i


def load(rcjson):
    d = json.load(open(rcjson))['data']
    fns = []
    for e in d:
        if e.get('type') != 1 or e.get('matching') is None:
            continue
        a = int(e['address'], 16)
        if not (0x401000 <= a < 0x478000):       # game code only, not the CRT
            continue
        n = S.function_extent(S.orig_bytes(a), a)
        fns.append(dict(addr=a, name=e['name'], size=n, match=float(e['matching'])))
    fns.sort(key=lambda f: f['addr'])
    return fns


def esc(s):
    return s.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')


def svg(fns, mode, stats):
    t = THEME[mode]
    # cells in address order; consecutive cells of one function in one row
    # become one rect, its last pixel left as the surface gap
    runs, cell = [], 0
    for f in fns:
        n = max(1, round(f['size'] / CELL_BYTES))
        ti = tier(f['match'])
        while n > 0:
            row, col = divmod(cell, COLS)
            k = min(n, COLS - col)
            runs.append((row, col, k, ti, f))
            cell += k
            n -= k
    rows = (cell + COLS - 1) // COLS
    W = COLS * PITCH
    top, left = 84, 16
    H = top + rows * PITCH + 44
    out = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d" '
           'role="img" aria-labelledby="t d">' % (W + 2 * left, H, W + 2 * left, H),
           '<title id="t">2EZDecomp progress</title>',
           '<desc id="d">%s of the code is byte-identical to the original (%d of %d functions); '
           'every function has decompiled C.</desc>' % (stats['code_pct_s'], stats['n_matched'], stats['n']),
           '<rect width="100%%" height="100%%" fill="%s"/>' % t['surface'],
           '<style>text{font-family:-apple-system,"Segoe UI",Helvetica,Arial,sans-serif}</style>',
           '<text x="%d" y="34" font-size="22" font-weight="600" fill="%s">%s byte-identical</text>'
           % (left, t['ink'], stats['code_pct_s']),
           '<text x="%d" y="58" font-size="13" fill="%s">%d of %d functions match exactly - '
           'all %d decompiled - average similarity %s - each cell is %d bytes of code, '
           'in address order</text>'
           % (left, t['ink2'], stats['n_matched'], stats['n'], stats['n'], stats['avg_s'], CELL_BYTES)]
    for row, col, k, ti, f in runs:
        out.append('<rect x="%d" y="%d" width="%d" height="%d" rx="1" fill="%s"><title>%s %s - %.2f%%'
                   '</title></rect>' % (left + col * PITCH, top + row * PITCH, k * PITCH - 1, PITCH - 1,
                                        t['tiers'][ti], hex(f['addr']), esc(f['name']), 100 * f['match']))
    # legend: swatch + label + count, text in ink (never the series colour)
    y, x = top + rows * PITCH + 24, left
    for i, (label, _) in enumerate(TIERS):
        out.append('<rect x="%d" y="%d" width="12" height="12" rx="2" fill="%s"/>' % (x, y - 10, t['tiers'][i]))
        txt = '%s (%d)' % (label, stats['tier_counts'][i])
        out.append('<text x="%d" y="%d" font-size="12" fill="%s">%s</text>' % (x + 18, y, t['ink2'], txt))
        x += 18 + 7 * len(txt) + 22
    out.append('</svg>')
    return '\n'.join(out)


def badge(label, value, color):
    lw, vw = 6 * len(label) + 12, 7 * len(value) + 12
    return ('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="20" role="img" aria-label="%s: %s">'
            '<rect width="%d" height="20" rx="3" fill="#555"/><rect x="%d" width="%d" height="20" rx="3" fill="%s"/>'
            '<rect x="%d" width="4" height="20" fill="%s"/>'
            '<g fill="#fff" font-family="Verdana,DejaVu Sans,sans-serif" font-size="11">'
            '<text x="%d" y="14">%s</text><text x="%d" y="14">%s</text></g></svg>'
            % (lw + vw, label, value, lw + vw, lw, vw, color, lw, color, 6, label, lw + 6, value))


def main():
    fns = load(sys.argv[1])
    outdir = sys.argv[2] if len(sys.argv) > 2 else os.path.join(HERE, '..', 'docs', 'progress')
    os.makedirs(outdir, exist_ok=True)
    total = sum(f['size'] for f in fns)
    matched = [f for f in fns if f['match'] >= 1.0]
    mbytes = sum(f['size'] for f in matched)
    counts = [0] * len(TIERS)
    for f in fns:
        counts[tier(f['match'])] += 1
    avg = sum(f['match'] for f in fns) / len(fns)
    stats = dict(n=len(fns), n_matched=len(matched), code_bytes=total, matched_bytes=mbytes,
                 code_pct=100.0 * mbytes / total, avg_similarity=100.0 * avg, tier_counts=counts,
                 code_pct_s='%.1f%%' % (100.0 * mbytes / total), avg_s='%.2f%%' % (100.0 * avg))
    for mode in THEME:
        open(os.path.join(outdir, 'progress-%s.svg' % mode), 'w').write(svg(fns, mode, stats))
    open(os.path.join(outdir, 'badge-matched.svg'), 'w').write(
        badge('code matched', stats['code_pct_s'], '#2a78d6'))
    open(os.path.join(outdir, 'badge-decompiled.svg'), 'w').write(
        badge('functions decompiled', '100%', '#104281'))
    json.dump({k: v for k, v in stats.items() if not k.endswith('_s')},
              open(os.path.join(outdir, 'progress.json'), 'w'), indent=1)
    print('%d functions, %d bytes of game code: %s byte-identical (%d functions), avg similarity %s'
          % (len(fns), total, stats['code_pct_s'], len(matched), stats['avg_s']))
    print('tiers:', dict(zip([t[0] for t in TIERS], counts)))


if __name__ == '__main__':
    main()
