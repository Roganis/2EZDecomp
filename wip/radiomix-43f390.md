# The RadioMix sub-family, and the hub decode of `0x43f390`

**Status 2026-08-23: analysis banked, function NOT yet written.** This file is
the decode of the two parts of `SongSelectDirector::input14RadioMix` @0x43f390
that no written handler contains. The rest of it is close enough to
`inputClubMix` @0x440af0 to be an edit, and `scratchpad/twindelta.py` names
those edits.

## Why this one first

`0x43cf30` / `0x43dc00` / `0x43e8d0` / `0x43f390` (RadioMix, 5RadioMix,
10RadioMix, 14RadioMix) score only **0.61-0.65** against every one of the six
handlers written so far, and the five written on 2026-08-23 barely moved them -
they are the family's second shape. But they are **0.84-0.94 similar to each
other**, so the payoff order is hub-first. By mean similarity to the other
three the hub is `0x43f390` (0.914), then `0x43dc00` (0.910); `0x43e8d0` is
the worst pick (0.879).

Budget it as a real decode: 779 instructions, ~100 more than any template.

## Part 1 - the wheel is a plain stepper, no easing

Where every other handler eases the wheel (`f_bfdd8 = f84`, `m_be8d8.reset()`,
`f_20c0d8 += k_twoPi / f7c`), this one just steps the cursor and plays the
sound. There is also **no `f7c < 8` split** and **no `findListA` lamp preamble
at all** - it opens straight on `updateStartLamps()`.

    updateStartLamps();
    if (f_20c6bc == 0)
    {
        if (getState(0x1e) == 3 && heldFor(0x1e) >= 0x96) f_c0e20++;
        if (getState(0x1d) == 3 && heldFor(0x1d) >= 0x96) f_c0e24++;

        if (f_bfdb4 == 4 && getState(4) != 3 && getState(5) != 3)
        {
            /* player 0 - note which way round: the POSITIVE chain steps the
               cursor DOWN and lights f_20c0e0, and each side clears only its
               own hold counter. */
            if (accum[0] > 0xa || getState(0x0f) == 2 ||
                getState(0x1e) == 2 || f_c0e20 >= 5)
            { f84 -= 1; sndMove.q5(); q4(0); f_c0e20 = 0; f_20c0e0 = 1; }
            else if (accum[0] < -0xa || getState(0x10) == 2 ||
                     getState(0x1d) == 2 || f_c0e24 >= 5)
            { f84 += 1; sndMove.q5(); q4(0); f_c0e24 = 0; f_20c0e0 = 0; }

            /* player 1 - two tests only, and neither clears a counter */
            if (accum[1] > 0xa || getState(0x17) == 2)
            { f84 -= 1; sndMove.q5(); q4(0); f_20c0e0 = 1; }
            else if (accum[1] < -0xa || getState(0x18) == 2)
            { f84 += 1; sndMove.q5(); q4(0); f_20c0e0 = 0; }
        }
    }
    f84 = (f84 + f7c) % f7c;

`f_20c0e0` is a new field - nothing else reconstructed touches it.

## Part 2 - the course picker (twindelta region 39, 136 instructions)

This is the only genuinely new logic in the family and the reason the function
is 100 instructions longer than its template. Five blocks of the same shape,
each gated on the CHANNEL name (`g_channelName` @0x1b2e728, already modelled),
each picking distinct entries and copying their names into the rolling text
slots:

    if (strcmp(g_channelName, "<name>") == 0)
    {
        pickDistinct(&idx, N);              /* 0x433cf0, matched, src/ingame.cpp */
        for (k = 0; k < 4; k++)
            strcpy(g_textSlots[k], <table>[idx[k]].name);
        ok = 1;
    }

with, in order:

| gate | pickDistinct arg | name table (this + ...) |
| --- | ---: | --- |
| (none - the first block) | 0 | `0xdb294` |
| `"random2"` @0x490720 | 1 | `0xf48d4` |
| `"random3"` @0x493700 | 2 | `0x10df14` |
| `"7znpfg"` @0x4936f8 | 3 | `0x127554` |
| `"11ptgc"` @0x4936e8 | 4 | `0x15a1d4` |

Read off the encoding: the row stride is 260 (`shl $6` + `add` = *65, scaled
by 4 in the `lea`), the slot walk is `0x1b2ec18` to `0x1b2ee18` step 0x80 -
i.e. `g_textSlots[0..3]`, which is already `extern char g_textSlots[32][128]`
in src/songselectinput.cpp - and the copy is cl's /Oi `strcpy` expansion
(`mov (%eax),%cl` / `mov %cl,(%eax,%edx,1)` / `add $1,%eax` / `test` / `jne`
with `edx = dst - src`), so the source says `strcpy`, not a hand-written loop.
The last block is followed by `call 0x414e30` (src/modename.cpp).

**Those five table bases are five more name fields to carve**, and none of them
is modelled yet. They are 260-strided, so they are fields of the song row, not
separate arrays - check `include/songselect.h`'s row layout before adding them.

## What is left to do

1. Write the wheel above and the picker above.
2. Take everything else from `inputClubMix`, then run
   `twindelta.py 0x440af0 0x43f390` again and reconcile the ~30 remaining
   regions - they are the same kinds of edit as the five handlers already
   done (stop-list arms, wrap limits, which counter feeds which lamp).
3. Then `0x43dc00`, `0x43cf30`, `0x43e8d0` become edits of THIS one at
   0.92 / 0.89 / 0.93.

---

## `0x43dc00` (5RadioMix) - deltas against the hub, ready to write

twindelta 0x43f390 -> 0x43dc00: **0.920 semantic, 30 regions**. Everything
below is read off the original; the rest is the hub unchanged.

1. **A latch at the top**, right after `updateStartLamps()`:

       if (f38 != f40) { f3c = 0; f40 = f38; }

   `f38`, `f3c` and `f40` are NOT modelled - they fall in
   `_pad38[0x7c - 0x38]` in include/songselect.h and need carving.

2. **The guard gains a second term**: `if (f_20c6bc == 0 && f_dc == 0)`.

3. **The per-player split is BACK.** The hub tests both players' accumulators
   in one chain; this one wraps player 0's chains in
   `(getListAF8() == 0 && countListA() == 1) || countListA() == 2` and
   player 1's in the `== 1` form, exactly as input7StreetMix does. The step
   bodies themselves are the hub's (no easing).

4. **The mode change likewise splits** into the two button groups with the
   same gates, each running the hub's body (q4(0), publish the level, log,
   park at 7 - here through `edi`, which holds 7 from just after the modulo).

5. **A ranking load on the move**, where the others call `m4332c0()`:

       f_c0e20 = 0; f_c0e24 = 0;
       loadRankingFile(rows[f84].name);          /* 0x431420, matched */
       if (f3c == 0) { loadRankingFile(rows[f84].name); f3c = 1; }

6. **A SIXTH picker arm**, `"random4"` @0x4936f0, between `"7znpfg"` and
   `"11ptgc"`: `pickDistinct(pick, 4)` then `setTextSlot(k, table + idx*260)`
   with the table at `this + 0x140b94`.

7. **Its own preview directory**: `"System\Preview_5radio\%s.wav"` @0x493a24,
   where the hub uses `Preview_14radio` @0x493a64.

8. **Readout cell 6 reads `f_bff64`**, not the hub's `f_bff34`.

### Cost note, recorded honestly

The first five handlers of this family took ~20 minutes each because they had
templates at 0.95+. These four are 0.92 against a hub that is itself only
55% accurate, so an edit of it inherits that error: expect ~50%, decoded but
not matched, at roughly a tick each. That is real port value - 3 KB of correct
logic apiece - but it is an order of magnitude worse per tick than the earlier
work, and the alternative (the big spine holes: SongSelectDirector::update2
@0x446540 at 10.6 KB, ModeSelectDirector::update @0x4491b0 at 8.4 KB) should
be weighed before finishing the set.
