/* PlayerSlot::m42e190 @0x42e190 - THE KEYS MODES' GRADE LADDER. Integrated
 * from wip/keysrank.cpp, where it was parked at 56.67% on a cross-jump coin.
 * IT NOW SCORES 72.15% with no change to the source - the park's number
 * predates some of the reccmp operand patches.
 *
 * DECODED AND SEMANTICALLY EXACT. 67
 * instructions, no calls. The reconstruction below produces the original's
 * instruction COUNT exactly (79 against 79 on the normalised stream) and its
 * every instruction, in the same basic blocks, with the same operands and the
 * same registers. What differs is a single binary choice described at the
 * bottom, and eight source forms give two outcomes that are exact mirror
 * images of each other.
 *
 * For the port this file is FINISHED - the reading is the whole content, and
 * port/ez2/score.c already implements both ladders from it.
 *
 * ---- what it computes ----------------------------------------------------
 *
 * The sink's companion, and the last of the four things CV2Mix does
 * differently: it does not grade on the score at all. The switch is the same
 * `g_1b2eb6c` (see src/sink.cpp's header), and the two ladders do not even
 * read the same quantity.
 *
 *   normal   score as a percentage of the maximum, on ELEVEN steps
 *            100 / 98 / 95 / 93 / 90 / 85 / 80 / 70 / 60 / 50  ->  10..0
 *   CV2Mix   (KOOL + COOL) / notes as a percentage, on SEVEN
 *            95 / 90 / 80 / 70 / 60 / 50                       ->  6..0
 *
 * The two agree on 80/70/60/50 -> 4/3/2/1 and differ only in the top three
 * steps, which is why the tails cross-jump at all.
 *
 * THE MAXIMUM IS `notes * 300`, computed as `(n << 4) - n` then doubled twice,
 * so the ladder's denominator hard-codes the NORMAL value table's KOOL - see
 * the consequence for the Radio modes below.
 *
 * The per-player summary block is `PlayerSlot::block` @+0x230 at a 0x40 stride,
 * indexed by `g_elemCount`: [0] is the note count and [5] / [6] are the top two
 * tiers. Both percentages are computed BEFORE the mode test - the original
 * divides and scales twice whichever ladder it then walks - and 100.0f is
 * loaded once into xmm2, serving both scalings AND the normal ladder's first
 * compare.
 *
 * ---- AND IT EXPOSES A FOURTH SCORING CASE --------------------------------
 *
 * The sink gives the ALTERNATE value table (40/100/170) to CV2Mix *and* to
 * mode 6 / mode 7 when `g_1b2ea20` is set, but it applies the combo multiplier
 * only when `g_1b2eb6c` is set - and THIS function grades on `g_1b2eb6c` too.
 * So a Radio run under that flag scores with the alternate values, gets no
 * multiplier, and is then graded on the eleven-step ladder against a
 * `notes * 300` denominator. A flawless play tops out at 170/300 = 56.67%,
 * which on that ladder is a 1.
 *
 * That reading follows from two functions that are each byte-verified in their
 * own right, so the arithmetic is not in doubt - and the gate is live.
 * `g_1b2ea20` is set to 1 in exactly one place, @0x44d802, from the
 * mode-select director's +0x95f4, which is a PLAYER-TOGGLEABLE OPTION: zeroed
 * in that director's init @0x44bb58 and flipped by five button handlers
 * (@0x44d3bb, @0x44d3cb, @0x44d46b, @0x44d4ca, @0x44d4e4), each behind a
 * `call 0x40c1c0` input query. It only ever holds 0 or 1, and the mode-select
 * screen slides by 320.0 px per unit (@0x44af25) - a full screen width, so it
 * is a two-page selector. Confirming page 1 sets g_1b2e89c as well, and THAT
 * is what says what the option is: the chart section parser DOUBLES every
 * gauge rate when it is set (@0x4207af, scale 2.0 @0x487f68). Faster gauge in
 * both directions plus lower note values is a hard/expert option, whatever its
 * on-screen label.
 *
 * ---- AND THIS GRADES A COURSE, NOT A STAGE -------------------------------
 *
 * The only caller is @0x42e360, which ACCUMULATES a finished stage into the
 * very +0x230 block this function reads - notes, all five per-grade counters,
 * combo, max combo, score and gauge each `+=` - and then calls this on the
 * running total, storing the result at +0x30. The Radio modes are multi-stage
 * courses, so the percentage the flag above distorts is the one shown at the
 * END of a course. Both @0x42e360 and the course result @0x42e2e0 are now
 * MATCHED in src/sink.cpp and name the whole block; @0x42e2e0 reads the stored
 * +0x30 rather than re-grading, so this ladder's output is what a course
 * result screen shows.
 *
 * ---- WHAT THE 50% IS -----------------------------------------------------
 *
 * The two ladders end in six identical `mov eax,K; ret` blocks (K = 6..1) plus
 * a shared `>= 50 -> 1` / `xor eax,eax` tail. cl emits ONE copy of each and
 * redirects the other arm into it. The original keeps CV2's copies - the CV2
 * arm is laid out first, and the normal arm's `>= 90`.. tests jump BACKWARDS
 * into it at 0x42e1fa / 0x42e209 / 0x42e218 / 0x42e227 / 0x42e236.
 *
 * cl 13.10 always keeps the copies belonging to the arm written SECOND, and
 * lays the arms out in source order. So the two available outcomes are:
 *
 *   CV2 arm first in the source  ->  CV2 laid out first, NORMAL owns the
 *                                    copies, CV2 jumps forward  (this file)
 *   CV2 arm second in the source ->  CV2 owns the copies, but NORMAL is laid
 *                                    out first and jumps forward
 *
 * The original wants CV2 laid out first AND owning, which is neither. Eight
 * forms were compiled - both source orders, each as early-returns and as a
 * symmetric if/else with one shared `return 0`, the CV2 ladder as a `static`
 * helper cl inlines, both ladders as helpers, and the mode test materialised
 * into a flag local in both polarities (CLAUDE.md's
 * success-block-after-the-return lever). They collapse to exactly the two
 * outcomes above; the symmetric-if/else pair also costs two extra
 * instructions. Nothing at source level chooses the direction, which puts this
 * in the same family as the zero-register coin flip in CLAUDE.md rather than
 * in the family of things worth another afternoon.
 *
 * CONFIRMED ON A SECOND FUNCTION, 2026-08-22. The scratch game's copy of this
 * ladder, PlayerRecord::m465b30 @0x465b30, is parked beside it in
 * wip/gfrank-465b30.cpp at 71.79% with the identical defect and nothing else
 * wrong. Two forms this list does not cover were tried there and are also
 * negative: `goto` into the first arm's blocks (cl tail-duplicates the target
 * and merges the other way, byte-identical output), and the NORMAL ladder as
 * a `static` helper (cl declines to inline it, which makes the CV2 arm own
 * its copies - but only because there is then no duplicate left to merge
 * with; `__forceinline` returns the exact original score).
 *
 * Everything upstream of the ladder - all 24 instructions of the head,
 * including the `shl eax,0x6` indexing, the `(n << 4) - n` doubled twice, the
 * register and xmm assignment and the scheduling of the mode compare between
 * the two divides - is byte-identical on the first build.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "players.h"
#include "sink-view.h"

extern int g_1b2eb6c;   /* 0x1b2eb6c - 1 = CV2Mix */
extern int g_elemCount; /* 0x1b2e718 - the player index this run is for */

/* The marker was deliberately omitted while the tree was at Accuracy 100.00%,
   on the rule that a function is not done until it is. That rationale is gone:
   the port track carries functions far below it, and a decoded function
   sitting in wip/ is invisible to a build. */
// FUNCTION: EZ2AC_UNPACKED 0x0042e190
int PlayerSlot::m42e190()
{
    const int *b = &this->block[g_elemCount * 16];
    int notes = b[0];
    int top = b[6] + b[5];
    float hitPct = (float)top / (float)notes * 100.0f;
    float scorePct = (float)((SinkSlot *)this)->score / (float)(notes * 300) * 100.0f;

    if (g_1b2eb6c != 0)
    {
        if (hitPct >= 95.0f)  return 6;
        if (hitPct >= 90.0f)  return 5;
        if (hitPct >= 80.0f)  return 4;
        if (hitPct >= 70.0f)  return 3;
        if (hitPct >= 60.0f)  return 2;
        if (hitPct >= 50.0f)  return 1;
        return 0;
    }

    if (scorePct >= 100.0f) return 10;
    if (scorePct >= 98.0f)  return 9;
    if (scorePct >= 95.0f)  return 8;
    if (scorePct >= 93.0f)  return 7;
    if (scorePct >= 90.0f)  return 6;
    if (scorePct >= 85.0f)  return 5;
    if (scorePct >= 80.0f)  return 4;
    if (scorePct >= 70.0f)  return 3;
    if (scorePct >= 60.0f)  return 2;
    if (scorePct >= 50.0f)  return 1;
    return 0;
}
