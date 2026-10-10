# 0x43b8d0 — the smallest mode-input handler — MATCHED 2026-08-18

**Done: `SongSelectDirector::inputRubyMix` @0x43b8d0 is 100% (byte-exact) in
src/songselectinput.cpp**, together with `setOptionLamps` @0x434b20 (the one
callee that was unidentified - a four-flag lamp writer, `__thiscall` because
every caller loads ecx). Everything below the rule is the decode as it was
banked before the match; it was right. What it took beyond the decode:

* `updateStartLamps` @0x44b2c0 became a `SongSelectDirector` member (see the
  section below); src/lamps.cpp still holds the body.
* fifteen members carved into include/songselect.h, three globals added to
  src/gamestate.cpp (0x37dcf08 / 0x37dce9c / 0x37dcf28), `EzSoundObject`
  extended to slot 16 (0x40: bit 0 = playing).
* **the 2*pi constant @0x4916b4 is a file-scope `static const float`**, not
  a literal. Written as a literal it is a rematerializable pool constant and
  cl gives the LAST step block on each path the other xmm assignment (K in
  xmm0, the count in xmm1) - one block at 99.08% and nothing in the block's
  own text moves it (forty forms swept). As `extern const` it is a real load
  scheduled ahead of the conversion in every block (a 100%* effective match,
  not byte-exact). As `static const` all six blocks come out as the original
  has them. Cookbook rule added to CLAUDE.md.
* the three option-lamp sums are declared in the order 0x37dced0, 0x37dcee8,
  0x37dcecc (that decides which lea lands in which register).
* the wrap IS guarded here (`if (f7c != 0)`); 5KeyMix's and EZ2CATCH's are
  not.

## The siblings: two written, both parked on ONE register wall

`input5KeyMix` @0x43ff00 (88.39%) and `inputEZ2Catch` @0x43a150 (88.56%)
are in src/songselectinput.cpp, structurally complete: every instruction is
present and in place, and every remaining defect is which callee-saved
register holds the constants 1, 5 and 0. The original's THREE siblings get
THREE different assignments from what is the same source text (RubyMix
5->ebp / 1->edi late; 5KeyMix 1->ebp for the whole function including the
lamp pushes at the top, 5->edi, then edi as a zero; EZ2CATCH 5->edi briefly,
1->edi, 0->ebp), and cl reproduces only RubyMix's. Use counts of 1/5/0 in
our 5KeyMix are IDENTICAL to the original's (45/8/48), so it is not a
missing use; the choice flips on any single 5-use, and the top hoist of 1
needs +16 stores that the original does not have. Two structural rules that
DID matter for the two-page handlers: the page split is a
`switch (g_37dced4) { default: A; break; case 1: B; break; }` (that emits
`sub eax,1; je`, where any `if` form emits `cmp`), and the 0x37dcecc stop
list is two `if`s, not `if / else if` (the `= 7` arm sits inline). Both
handlers use those.

Do NOT write the other four (0x43c2e0 StreetMix, 0x442770 7StreetMix,
0x43ad10 ScratchMix, 0x440af0 ClubMix, 0x441540 SpaceMix, 0x441fa0 CV2Mix)
until the constant-allocation rule is understood - StreetMix's listing puts
`moved` in ebp, 1 in edi and a zero in ebx from the very top, a fourth
distinct assignment. The lever, whatever it is, is not in the handler's own
statements; it may be a whole-TU or source-order effect (the originals were
one TU - all forty-six references to the 2*pi constant are in this family).

---


Decoded 2026-08-18. 650 instructions, 100 branches, 57 basic blocks, 2565
bytes, no EH prologue, frame `sub esp,0x108` with a /GS cookie. First of the
seven-strong family (see include/modeinput.h); the other six are 0.4-0.7
self-similar, so this one is the reconstruction and they should be edits.

## It is a SongSelectDirector method

Not stated anywhere before. Every member offset it touches lands in that
class: 0x7c, 0x84, 0xbe8d8, 0xbfd10/14/20, 0xbfdb4, 0xbfdd8, 0xc0e20/24,
0x20c0d8, 0x20c6bc, 0x20e894. It also calls `SongSelectDirector::m4332c0`
@0x4332c0 with its own `this`, which settles it.

`[ecx+0x420238]` and `[ecx+0x42023c]` are NOT this class - ecx there is
`g_bigInstance`, whose offsets run to ~0x420290.

## Callees, all identified

| addr | what | count |
| --- | --- | --- |
| 0x40c1c0 | `Obj40C180::getState(i)` | 31 |
| 0x40c780 | `Obj40C180::setOutput(ch, v)` | 11 |
| 0x40c1d0 | `Obj40C180::heldFor(i)` | 2 |
| 0x418310 | `countListA()` | 12 |
| 0x418340 | `getListAF8()` | 6 |
| 0x418e30 | `findListA(key)` | 2 |
| 0x402990 | `Obj402860::reset()` | 6 |
| 0x402910 | `Obj402860::setCell(r,c,v)` | 4 |
| 0x40e590 | `EzSubsystem::loadSample` (wip/loadsample.cpp) | 2 |
| 0x40e3c0 | `EzSubsystem::createSample` | 1 |
| 0x44b2c0 | `updateStartLamps()` | 1 |
| 0x4332c0 | `SongSelectDirector::m4332c0()` | 1 |
| 0x434b20 | still unidentified | 1 |
| 0x47882f, 0x4788cf | CRT | 3 |

Plus indirect calls through `[edx+0x14]` (x9), `[eax+0x10]` (x8),
`[edx+0x10]`, `[eax+0x40]`.

## Shape

1. Two lamp blocks - `findListA(0)` then `findListA(1)`, each testing
   `getState(k) == 3` twice and driving two outputs.
2. `updateStartLamps()`.
3. `if (f20c6bc == 0)` gates everything down to L43bdff.
4. Two `heldFor(k) >= 0x96` checks (150 ticks).
5. `if (fbfdb4 == 4)` then a split on `f7c >= 8` into TWO NEAR-IDENTICAL
   HALVES - L43ba47..L43bb9b against L43bbc9..L43bd16 - each containing a
   player-0 and a player-1 sub-half. **The halves differ only in two
   constants**: the `[g+0x420238]` threshold is 0x14 in the low half and 0xa
   in the high one, and the `c0e20`/`c0e24` threshold is 0xa against 3. This
   is the duplicated-near-identical-halves pattern in CLAUDE.md - write both
   out, including their shared tail, rather than sharing through a goto.
6. A long run of `getState(k) == 2 || == 3` tests all jumping to one skip
   label (L43bfc6), then two more list-gated blocks.
7. Three `getState(k) == 2 && f20c6bc >= 0` blocks with inner value tests.
8. `if (f20e894 > 0x1e)` and a virtual `[eax+0x40] & 1` guard around the two
   `loadSample` calls and the `createSample` fallback.
9. Tail: four `setCell` calls and four `setOutput`s, or the `0x434b20` path.

## Estimate

Three to four ticks. It is branch-dense rather than a table of calls, so
CLAUDE.md's "inline, serially, one to four iterations" applies; the payoff is
that six siblings should follow as edits.

## updateStartLamps @0x44b2c0 is a THISCALL MEMBER, not a free function

Found while reading the prologue, and it has to be fixed before any handler
can match. `src/lamps.cpp` declares it `void updateStartLamps()`, and it sits
at 100% - but only because its body never touches `this`. **All fourteen call
sites load ecx first**: the eleven handlers do `mov ecx,esi` immediately
before the call (esi being their own `this`), the four-family ones inherit ecx
unchanged from entry, and @0x44d82f uses `mov ecx,ebp` from a different class
entirely (its own field is at ebp+0x95e8).

Declared free, cl emits a bare `call` and every handler loses that
instruction plus the displacement shift behind it. It must be declared as a
member. The class is not obvious - at least two unrelated classes call it - so
follow the tree's existing idiom: declare it on one and cast at the other site,
the way the scratch shuffles call EZ2DJ's m421510.

Changing the declaration is safe for its own match (the body ignores ecx, and
reccmp maps by the marker address, not the mangled name) but there is nothing
to VERIFY it against until a caller exists, so do it as part of writing the
first handler rather than on its own.

## Prologue, decoded exactly

    xor ebx,ebx            ; ebx = 0, live to the end
    push ebx               ; doubles as findListA(0)'s argument
    if (findListA(0)) {
        setOutput(0x11, getState(0x0b) == 3);
        setOutput(0x13, getState(0x0d) == 3);
    }
    if (findListA(1)) {
        setOutput(0x19, getState(0x13) == 3);
        setOutput(0x1b, getState(0x15) == 3);
    }
    updateStartLamps();                       // thiscall, see above
    if (f20c6bc != 0) goto L43bdff;
    if (getState(0x1e) == 3 && heldFor(0x1e) >= 0x96) fc0e20++;
    if (getState(0x1d) == 3 && heldFor(0x1d) >= 0x96) fc0e24++;
    if (fbfdb4 != 4) goto L43bdff;

Note the four lamp writes are NOT ternaries in the source: each is a
`push 1` / `jmp` / `push 0` pair feeding one `setOutput`, which is the shape
CLAUDE.md records for two separate calls cross-jumped back together - so write
`if (c) setOutput(k, 1); else setOutput(k, 0);` rather than
`setOutput(k, c == 3)`.

## Fully decoded 2026-08-18 — and NOT a fit for 20-minute ticks

~590 of the 650 instructions are read and understood. What it is: **the
song-select cursor**. `f84` is the entry index, `f7c` the entry count, and the
core scrolls `f84` up or down on either the analog stick passing ±0x14 (or ±0xa
in the high half) or one of two buttons, wrapping through
`f84 = (f84 + f7c) % f7c` and easing `f20c0d4` toward `f20c0d8`.

### The parts

* **Prologue** — written out above.
* **The two halves** — split on `f7c >= 8`. Each half has a player-0 and a
  player-1 sub-half. One sub-half reads:

      if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2) {
          if (g->f420238 > 0x14 || getState(0x0f) == 2 ||
              getState(0x1e) == 2 || fc0e20 > 0xa) {
              fbfdd8 = f84;
              m_be8d8.reset();
              f20c0d8 += <0x4916b4> / (float)f7c;
              f84 += 1;  ebx = 1;
              fbfd10->v14();  fbfd10->v10(0);
          }
          ... the mirror, with f420238 < -0x14, getState(0x10), getState(0x1d),
              fc0e24
      }

  The high half is the same with 0x14 -> 0xa and 0xa -> 3, and the down
  thresholds negated to match.
* **The wrap** — `if (f7c) { f84 = (f84 + f7c) % f7c; f20c0d4 += (f20c0d8 -
  f20c0d4) * <0x4904f8>; }`
* **The confirm run** — six `getState(k) == 2 || == 3` over channels 6, 4 and
  5, all skipping to L43bfc6; then two list-gated blocks driving `fbfd14`.
* **Three wrapping option counters** in globals 0x37dcecc (mod 4 -> 5, 8 -> 9,
  0xb -> 0), 0x37dcee8 (mod 4) and 0x37dced0 (1 -> bumps 0x37dcec4, 3 -> both
  zeroed).
* **The preview load** — after 0x1e frames idle, `sprintf(buf, <0x4939f0>,
  &rows[f84])` then two `loadSample` calls on `g_1b2eae4`, a `createSample`
  fallback with the literal at 0x491b28, and a play through `[edx+0x10]`.
* **The readout** — four `setCell` calls into `m_20cb10` indexed by the three
  option globals, then four `setOutput`s.

### Why this is being handed off rather than finished in the loop

Four ticks in. The reading is done and correct, but WRITING it needs, before a
single line of body: eight or so new members carved (0xbfd10, 0xbfd14,
0xbfd20, 0xbfdb4, 0xbfe58, 0xbfee8, 0xbff58, 0xbff64, 0x20c0d4, 0x20c0d8,
0x20cb10, 0x20e894, 0xc0e20, 0xc0e24, 0x20c6bc), ten new globals with GLOBAL
pins, two STRING markers, two float pool constants, `updateStartLamps`
converted to a member (see above), and a vtable shape for whatever `fbfd10`
and `fbfd20` point at - four virtuals deep, slots 0x10, 0x14 and 0x40.

That is a session's work with one measurement at the end, which is the
opposite of what a 20-minute tick is good for. **Do this in a dedicated
sitting**, with this file open; the loop should spend its ticks on things that
can be finished and verified within one.

## Correction banked 2026-08-18

The 0x374-stride table's base is **0x238**, not the 0x49c recorded when
m4332c0 was matched. This function builds `esi + i*0x374 + 0x238` and hands it
to sprintf as a string, which only makes sense at an element's start, so
m4332c0's `+0x49c` is that element's field at +0x264. include/songselect.h now
models it as `SongSelectRow { char name[0x264]; int f264; ... } rows[]`, and
m4332c0 re-verified at 100% after the change.
