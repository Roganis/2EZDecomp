# SongSelectDirector — the remaining draw/loader functions (surveyed 2026-08-18)

UPDATE 2026-08-18 (later): DONE at 100% - drawRankTable @0x432bb0,
drawCourseRankTable @0x433d50, drawWheel @0x433180, drawWheelWithList
@0x433030, drawListPanel @0x432f00, drawGenreStrip @0x432200 (all in
src/songselect.cpp), plus Obj402860::seek @0x402d60. PARKED:
wip/songselect-banner-4324b0.cpp (the entry banner + level digits, arm-merge
wall). Still open in the class: 0x431ae0 (1.8 KB /GS draw with rotdraw +
sprintf, x87/SSE mix - a dedicated sitting), 0x4394e0 (2.2 KB), 0x435070
(out-of-image globals), 0x430970 (needs InGameSongSelect), the ctor 0x435390
(14.7 KB), update/update2 0x443380/0x446540. Also 0x4305a0 is
InGameSongSelect (a 0x3d0-layout class), not this one.

Done this session: drawLevels @0x4343b0, drawCounter @0x434a30 (91%),
loadRankFile @0x434b80, loadRankingFile @0x431420. What is left in the class,
profiled from scan.py, with why each is NOT a quick loop-tick win:

## 0x432bb0 — the ranking-table draw (two floats, thiscall, ret 8)  [FIELDS CARVED]

FIELDS ARE NOW IN include/songselect.h and the full match still passes, so the
carve is safe: f_20be8c[] (name glyphs), f_20bef4[] (score digits), f_20c240[]
(char->glyph map), f_20c698 (brightness byte), f_20c69c (y origin). The two
header textures 0xbfecc / 0xbfed0 ALIAS f_bfe9c[12] / f_bfe9c[13] - do NOT
re-carve, reference `(KTexture *)f_bfe9c[12]` and `[13]`.

drawTex is `drawTex(tex, x, y, w, h, color)`, thiscall (this = g_sub1b2f054),
all five args on the stack. The colour argument is the brightness byte
replicated to all four bytes: `c = b; c = c<<8 | b; ...` (three shl/or). The
header draws two textures at (x-derived, y-21) with w=h=0. The name loop is 8
names x 7 chars stepping the x DOWN by 8.0 per char and the row DOWN by 37.0;
a char of 0 or space is skipped, else glyph = f_20be8c[f_20c240[(signed)ch]].
The score loop is 8 scores x up-to-7 digits, MSD first, split by the
magic-divide-by-10 (0x66666667) idiom, digit = f_20bef4[digit]. Constants
seen: 24, 21, 72, 213, 41, 17, 37, 31, 8, 55, 1000000 (0xf4240 modulus). It is
the register-wall draw class - budget several iterations and expect a
100%*-or-park outcome. Twin 0x433d50 follows as an edit.

## (original 0x432bb0 notes)
Self-contained, no EH, ~110 instructions. A header row of two textures
(fields 0xbfecc / 0xbfed0), then 8 name rows x 7 glyph chars, then 8 score
rows x 5 digits, all through drawTex @0x403f80 + setBlend @0x40a490. Reads:
- 0x20c698 a brightness byte (built into 0xRRGGBBAA by shl/or x3),
- 0x20c69c a float y-origin,
- 0x20c1cc m_rankingNames, 0x20c220 m_rankingScores (already modelled),
- 0x20c240 a char->glyph-index map (indexed by the signed name byte),
- 0x20be8c the glyph-texture array (name chars),
- 0x20bef4 the digit-texture array (scores; the score is split to digits by
  the repeated `idiv 10` / magic-divide-by-10 loop, MSD first, 7 wide).
CAUTION: 0xbfecc / 0xbfed0 fall INSIDE f_bfe9c[] as carved for the mode
handlers - reconcile that overlap before carving (either the handler table is
shorter, or these two are separate fields the pad must be re-split around).
0x20be8c / 0x20bef4 live inside `_padc0e28`; 0x20c240 / 0x20c698 / 0x20c69c
inside `_pad20c240`. Its twin is 0x433d50 (same shape, one extra highlighted
column - do them together). Draw-register (xmm accumulator) wall risk is real
but it is small; worth an attempt in a focused sitting.

## 0x433d50 — the same draw with a highlight column
Twin of 0x432bb0 (146+ shared instructions by eye). Reconstruct 0x432bb0
first, then this is an edit.

## 0x433380 — the Stage_%d.ini course loader (EH, ~430 insns)
`system\radiomix\Stage_%d.ini` -> a KProfile on the stack (ctor 0x413dc0,
Load 0x413fe0, getStr 0x413f70, dtor 0x413e20), then a run of sprintf
(0x4788cf) building keys "%s_level" / "%s_goal" / "%s_grow" and `;`-split
token loops (the `cmp 0x3b`/`mov 0` inline strtok), atof (0x479ffa) and atol
(0x4788c5) into the row table at 0x238 (stride 0x374). Genuine spine
game-logic and the repetitive-parse kind that lands well - but it is EH with a
KProfile local, so the frame is exacting. A dedicated tick, not a loop slice.
Strings: 0x491c18 Stage_%d.ini, 0x491c08 channel_normal, 0x491c00 %s_grow,
0x491bec %s_goal, 0x491bd4 %s_level, 0x491b88/0x491b64 legacy bmp paths,
0x491b44 preview_legacy wav, 0x491bc0 "Course Name = %s", 0x491bac "Course
Level = %02d". The `8;8;9;10` / `10;17;26;35` defaults are literals.

## 0x435070 — a row search/copy over OUT-OF-IMAGE globals
References 0x3789bd4 / 0x3789c90 (packer-dumped, outside the loaded image) as
big arrays, plus a 0x374-stride row copy (rep movs 0xdd). Needs those globals
modelled as placeholder storage first; skip until the rest is done.

## Recommendation
Next focused sitting: reconcile the 0xbfecc overlap, carve the 0x432bb0
fields, reconstruct 0x432bb0 + 0x433d50 as a pair. Then the course loader
0x433380 as its own tick. Leave 0x435070 for last.
