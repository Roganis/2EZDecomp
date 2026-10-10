/* PlayerSlot::commit @0x42e620 - THE KEYS MODES' JUDGEMENT SINK.
 *
 * MATCHED, 100% - 1,376 bytes, 313 instructions. The reading below was written
 * first, from the disassembly alone, and it survived the match unchanged in
 * every semantic respect: what closed the last eleven instructions was block
 * placement and inlining shape, not a correction. See "WHAT THE MATCH ADDED"
 * at the bottom for the four levers.
 *
 * Every judged event in the twelve keys modes ends here. Its twelve call
 * sites sit in FOUR functions, ALL FOUR NOW RECONSTRUCTED (attribution
 * corrected 2026-08-10 against the disassembly): the hold-sustain tick @0x42f4e0 (five sites - 0x42f66e, 0x42f74c,
 * 0x42f7e1, 0x42f808, 0x42f834; src/holdtick.cpp), the hold pump @0x42f880
 * (four - 0x42f94c,
 * 0x42fae7, 0x42fb85, 0x42fba1), the keys commit @0x42fd90 (two - 0x43026c
 * normal, 0x4304d8 on the lane-swap variant) and `SlotOwner::applyTracks`
 * @0x422c65 - in src/playerslot.cpp; the other three are src/holdtick.cpp,
 * and it calls this with grade 5 and the summed count.
 *
 *     void PlayerSlot::commit(int grade, int count, int flag)
 *
 * `grade` is the GAME's numbering, 1..5 = MISS, FAIL, GOOD, COOL, KOOL - the
 * same index the per-grade counters use. `count` is how many notes this event
 * is worth (a hold pays its ticks through here, which is why the combo advances
 * by `count` and not by one). `flag` gates every combo and tally effect; a zero
 * flag still scores and still moves the gauge.
 *
 * ---- IT IS TWO FUNCTIONS IN ONE, AND THE SWITCH IS CV2MIX -----------------
 *
 * `g_1b2eb6c` is set to 1 by `runCv2Mix` @0x417a3f and by nothing else, so
 * every "alternate" branch below is CV2Mix and every other branch is the
 * remaining twelve modes. That single fact is what makes the rest legible:
 * the two note-value tables, the combo latch, the multiplier and the grade
 * ladder are all CV2 against not-CV2.
 *
 * ---- 1. what a note is worth ---------------------------------------------
 *
 *     normal      GOOD 41   COOL 150   KOOL 300
 *     alternate   GOOD 40   COOL 100   KOOL 170
 *
 * and the alternate table is used by CV2Mix AND by 5RadioMix/RadioMix when
 * `g_1b2ea20` is set - two modes that otherwise score normally. The table is
 * six ints on the stack indexed by the grade, so MISS and FAIL are worth zero
 * by construction rather than by a test.
 *
 * ---- 2. the combo, and CV2's off-by-one ----------------------------------
 *
 * Normal: grades 5, 4 and 3 add `count`; grades 2 and 1 zero it. So a FAIL
 * breaks the combo, which src/../port/ez2/score.c had wrong until this was
 * read (it had been taken from EZ2CATCH's commit @0x472fc0, a different
 * class).
 *
 * CV2: only 5 and 4 add. GOOD does nothing at all - unless the director's
 * `fd18` is set, in which case a GOOD BREAKS the combo. And the first scoring
 * hit after a break sets `combo = count - 1` rather than `+= count`, latching
 * `f1c0`; every later hit adds normally. With count == 1 that means CV2's
 * combo reads one behind: 0 on the first note, 1 on the second.
 *
 * ---- 3. the score ---------------------------------------------------------
 *
 * Normal:    score += value[grade]                       (flat, no combo)
 * Alternate: score += (int)((float)value[grade] * mult[min(combo, 1499)])
 *
 * where `combo` is the value AFTER this event's update, and `mult` is a
 * 1,500-entry float table @0x4a9160. THE TABLE IS `1 + log10(c) / 3`, exact to
 * one ulp of float32 across all 1,500 entries (mult[0] = mult[1] = 1.0), which
 * matters for the port: the table is the game's data and may not be copied
 * out, so it has to be computed.
 *
 * ---- 4. the gauge ---------------------------------------------------------
 *
 *     rate = dir->gaugeRate[grade]          dir + 0x308 + grade*4
 *
 * and the section parser @0x4206e0 fills that block POSITIONALLY from the
 * chart `.ini`'s `[GaugeUpDownRate]` in file order - Cool, Good, Miss, Fail ->
 * +0x318, +0x314, +0x310, +0x30c. So grade 1 (a note never pressed) takes the
 * file's `Fail` and grade 2 (a press in the outermost band) takes its `Miss`,
 * which is the opposite of what their names suggest and is exactly what the
 * port had backwards. Grades 3 and 4 landing on `Good` and `Cool` is what
 * pins the alignment.
 *
 * The add is guarded by `gauge > 0`, so a gauge that reaches zero never
 * recovers. Ceiling 100.0; the 1P path clamps to [0, 100] and raises game over
 * at <= 0, and the 2P path does the same for both players and only calls game
 * over when BOTH are down.
 *
 * ---- 5. the audience rate -------------------------------------------------
 *
 * A second accumulator at +0x21c, only when `g_useAudiencePanel` is 1:
 *
 *   dir->f794 == 1   only KOOL/COOL move it, by (g_1b2eaec[player] + 10) /
 *                    noteCount, clamped to [0, 100]
 *   level 0 or 2     only KOOL/COOL, by (g_1b2f04c + 10) / noteCount,
 *                    clamped to [0, 99.99], and it LOGS both the tier name and
 *                    the delta
 *   otherwise        every grade, by dir->audienceRate[grade]
 *                    (dir + 0x32c + grade*4), clamped to [0, 99.99]
 *
 * `noteCount` is at +0xe105bc - the 14 MB per-player element CLAUDE.md warns
 * about, and the reason this class cannot be modelled with a plain `_pad`.
 *
 * ---- what this cost the port ----------------------------------------------
 *
 * Three defects already fixed from it (the two crossed gauge rates, FAIL
 * breaking the combo, the frozen gauge) and one still open: CV2Mix is scored
 * as a keys mode, which is wrong in the value table, the multiplier, the combo
 * rule AND the grade ladder - the keys rank @0x42e190 grades CV2 on
 * (KOOL+COOL)/notes over a SEVEN-step ladder instead of on the score
 * percentage over eleven.
 *
 * ---- WHAT THE MATCH ADDED -------------------------------------------------
 *
 * The decoded draft was 307 instructions against 313 and structurally right
 * everywhere except four places. All four are placement, and three of them are
 * new levers worth remembering:
 *
 * 1. THE TWO-PLAYER CHECK IS WRITTEN OUT TWICE, NOT LOOPED. The original has
 *    no loop counter, no `if (i == 0)` and two direct stores to the two down
 *    flags. Keeping the `for (i = 0; i < 2; i++)` cost far more than the loop
 *    itself: with the counter alive, cl had no register for the constant 1, so
 *    every `= 1` store became a ten-byte immediate and the WHOLE FUNCTION's
 *    callee-saved assignment permuted (ebx/ebp swapped roles against the
 *    original throughout). Unrolling it fixed the register allocation of code
 *    two hundred instructions away. The slot is also re-fetched at every touch
 *    - three `findListA` calls on the low path - which is what says the source
 *    never held it in a local.
 * 2. THE AUDIENCE CLAMP IS ONE INLINED HELPER AT THREE CALL SITES. All three
 *    sites compute their delta into xmm0 and then `addss xmm0,[+0x21c]`, and
 *    all three load the CEILING INTO A REGISTER instead of folding it into the
 *    compare - the fingerprint of an inlined helper's float parameter. Writing
 *    the three clamps out separately puts their stores in different xmm
 *    registers, and two sequences that differ by a register cannot cross-jump:
 *    the shared store @0x42eb64, which serves the zero clamp AND both ceiling
 *    clamps, only exists because one helper produced all three.
 * 3. THE NOTE-VALUE TABLE'S CONDITION DECIDES WHICH TABLE IS LAID DOWN FIRST.
 *    `if (A || ((m == 6 || m == 7) && C))` emits the tests in exactly the
 *    original's order and then puts the blocks the WRONG WAY ROUND - normal
 *    falling out of the last test, alternate jumped to. Nine permutations were
 *    compiled (De Morgan, an explicit `!`, a duplicated alternate arm, a
 *    `static __forceinline` predicate, flag locals); only a flag local built in
 *    two statements gives the original's `je NORMAL`. The predicate form gives
 *    it too - but ADDING A FIFTH forceinline HELPER shifted the xmm allocation
 *    in the log arm and cost the eleven instructions rule 2 had just bought.
 *    Two fixes can be individually right and jointly wrong; check the whole
 *    function after each.
 * 4. `self->dir` IS RE-READ AT EVERY USE. A `SinkDirector *dir` local at the
 *    top hoists the load into the prologue and holds a register across the
 *    combo section.
 *
 * Two shapes the draft already had right, recorded because they read as
 * compiler artifacts and are not: the two epilogues (the `count <= 0` exit
 * leaves before `ebp`/`edi` are pushed, so it pops two registers where the
 * main exit pops four - a source-level early `return`), and the per-grade
 * counter bump appearing twice (@0x42eafd and @0x42eb6c), which is cl's own
 * duplication because the audience-log arm leaves the `if` chain elsewhere.
 *
 * ---- THE CLASS FIELDS ARE REACHED THROUGH VIEW STRUCTS --------------------
 *
 * Every offset this needs sits inside a `_pad` run of `PlayerSlot` or
 * `SlotOwner` in include/players.h, and they are DELIBERATELY not carved out
 * there: CLAUDE.md's "adding fields to a verified struct breaks matched
 * functions" rule cost five matches the last time it was ignored. The casts in
 * include/sink-view.h cost no code and cannot disturb anything reccmp has
 * already scored.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "players.h"

struct EZ2DJSlotObject { void release(); };   /* 0x42e360 */

/* ---- the globals ---------------------------------------------------------
 *
 * All plain externs: every one of these is DEFINED somewhere else in src/ (the
 * five this function was the first to need are in src/globals.cpp), and the
 * GLOBAL marker belongs at the definition. A second marker here would create a
 * second entity at the same address - CLAUDE.md's "grep for an address both
 * ways" trap, which silently costs an unrelated caller its operand. */

extern int g_1b2eb6c;        /* 0x1b2eb6c - 1 = CV2Mix (runCv2Mix @0x417a3f) */
extern int g_modeIndex;      /* 0x1b2e548 (src/modename.cpp) */
extern int g_1b2ea20;        /* 0x1b2ea20 - the Radio modes' alternate-value
                                flag (src/modeselectctor.cpp) */
extern int g_elemCount;      /* 0x1b2e718 - the player index this run is for
                                (src/loops.cpp) */
extern int g_useAudiencePanel;  /* 0x1b2eb48 (src/maingame.cpp) */
extern int g_1b2e7d4;        /* 0x1b2e7d4 - 0, 1 or 2, which audience model
                                applies (src/maingame.cpp) */
extern int g_1b2ea28;        /* 0x1b2ea28 - game over has been raised
                                (src/moderunner.cpp) */
extern int g_1b2ea34;        /* 0x1b2ea34 (src/moderunner.cpp) */
extern int g_1b2e704;        /* 0x1b2e704 - the global miss tally
                                (src/playerslot.cpp) */
extern int g_1b5f19c;        /* 0x1b5f19c - consecutive misses; a hit zeroes it
                                (src/playerslot.cpp) */
extern int g_1b2efb0;        /* 0x1b2efb0 - the last grade, for the panel
                                (src/gamepanels.cpp) */
extern float g_1b2f04c;      /* 0x1b2f04c (src/maingame.cpp) */

/* First needed here - defined in src/globals.cpp. */
extern int g_player0Down;    /* 0x1b2ef90 */
extern int g_player1Down;    /* 0x1b2ef64 */
extern int g_1b2e7d8;        /* 0x1b2e7d8 */
extern float g_1b2eaec[2];   /* 0x1b2eaec - per-player audience base */
/* THE COMBO MULTIPLIER TABLE, and it is game data - see the header. Referenced,
   never read from the repo, and reproduced by `1 + log10(c)/3` in the port. */
extern float g_comboMult[1500];   /* 0x4a9160 */

extern int g_1b2eb28;        /* 0x1b2eb28 (src/gamestate.cpp) */
extern int g_1b2ea4c;        /* 0x1b2ea4c (src/modeselectctor.cpp) */
extern int g_1b2ea2c;        /* 0x1b2ea2c - forces a perfect stage; see
                                EZ2DJSlotObject::release below */
extern int g_1b5f180;        /* 0x1b5f180 (src/maingame.cpp) */

extern void Log(const char *fmt, ...);      /* 0x402880 (src/applog.cpp) */
extern int countListA();                    /* 0x418310 (src/nodelists.cpp) */
extern ListNode *findListA(int index);      /* 0x418e30 - by INDEX here */

#include "sink-view.h"

/* ---- the pieces the original inlines ------------------------------------- *
 *
 * Written out separately here for legibility; the original has one body with
 * cross-jumps into it, and a match attempt will have to inline them back. Each
 * is annotated with the address its code sits at. */

/* @0x42e6fa - the shared combo reset both grade arms jump into. */
static __forceinline void breakCombo(SinkSlot *self, int grade)
{
    self->combo = 0;
    self->f1c0 = 0;
    if (grade == 1)
        g_1b2e704++;
    /* RubyMix under a flag zeroes the consecutive-miss run instead of
       advancing it - the only mode that does. */
    if (g_modeIndex == 1 && g_1b2ea28 == 0)
        g_1b5f19c = 0;
    else
        g_1b5f19c++;
}

/* @0x42ea05 - one player: clamp to [0, 100] and raise game over at zero. */
static __forceinline void checkThisPlayer(SinkSlot *self)
{
    if (g_useAudiencePanel != 0 && g_1b2e7d4 != 2)
        return;

    if (0.0f >= self->gauge)
    {
        self->gauge = 0.0f;
        g_1b2ea28 = 1;
        if (g_elemCount == 0)
            g_1b2ea34 = 1;
        self->dir->m420570(self);
    }
    else if (self->gauge > 100.0f)
    {
        self->gauge = 100.0f;
    }
}

/* @0x42e853 / @0x42e8e1 - ONE player's clamp. The original has this body
   TWICE, written out for player 0 and player 1: there is no loop, no runtime
   index, and the two down-flags are two direct stores. The slot is re-fetched
   at EVERY touch (three `findListA` calls on the low path), which is what says
   the source never held it in a local. */
static __forceinline void checkOnePlayer(SinkSlot *self, int index, int *down)
{
    if (0.0f >= ((SinkSlot *)findListA(index))->gauge)
    {
        if (g_useAudiencePanel == 0)
            ((SinkSlot *)findListA(index))->gauge = 0.0f;
        if (countListA() != 2)
        {
            g_1b2ea28 = 1;
            self->dir->m420570(self);
        }
        *down = 1;
        g_1b2ea34 = 1;
    }
    else if (((SinkSlot *)findListA(index))->gauge > 100.0f)
    {
        ((SinkSlot *)findListA(index))->gauge = 100.0f;
    }
}

/* @0x42e83e - two players: each is clamped independently, and game over is
   only raised once BOTH are down. The `countListA() != 2` test inside is what
   lets a single-player-in-a-two-slot-cabinet run end immediately. */
static __forceinline void checkBothPlayers(SinkSlot *self)
{
    if (g_useAudiencePanel != 0 && g_1b2e7d4 != 2)
        return;

    checkOnePlayer(self, 0, &g_player0Down);
    checkOnePlayer(self, 1, &g_player1Down);

    if (g_player0Down == 1 && g_player1Down == 1)
    {
        g_1b2ea28 = 1;
        g_1b2ea34 = (g_elemCount < 1);
        self->dir->m420570(self);
        g_1b2e7d8 = 2;
    }
}

/* @0x42e9e7 / @0x42eaad / @0x42eb47 - THE AUDIENCE ACCUMULATOR, one helper at
 * three call sites.
 *
 * All three sites compute their delta into xmm0 and then `addss xmm0,[+0x21c]`,
 * and all three load the ceiling into a REGISTER rather than folding it into
 * the compare - which is what an inlined helper's float parameter looks like
 * (CLAUDE.md: a constant passed through an inlined helper's parameter is not
 * folded). It is also what lets the rate site's clamp cross-jump into the
 * f794==1 site's tail @0x42eb5f, which is where the original's shared store
 * @0x42eb64 comes from: written out three times the stores land in different
 * registers and cannot merge. */
static __forceinline void addAudienceValue(SinkSlot *self, float d, float hi)
{
    float a = d + self->audience;

    self->audience = a;
    if (0.0f > a)
        self->audience = 0.0f;
    else if (a > hi)
        self->audience = hi;
}

/* @0x42e9ab - the audience meter. */
static __forceinline void addAudience(SinkSlot *self, int grade)
{
    if (g_useAudiencePanel != 1)
        return;

    if (g_1b2e7d4 != 0 && g_1b2e7d4 != 2)
    {
        addAudienceValue(self, self->dir->audienceRate[grade], 99.99f);
        return;
    }

    if (grade != 5 && grade != 4)
        return;

    {
        float d = (g_1b2f04c + 10.0f) / (float)self->noteCount;

        addAudienceValue(self, d, 99.99f);

        /* One marker per literal, each DIRECTLY above the line its literal
           begins - three literals cannot share two lines (CLAUDE.md). */
        // STRING: EZ2AC_UNPACKED 0x00491248
        Log("JUDGMENT %s",
            // STRING: EZ2AC_UNPACKED 0x0049125c
            grade == 5 ? "KOOL"
            // STRING: EZ2AC_UNPACKED 0x00491254
                       : "COOL");
        // STRING: EZ2AC_UNPACKED 0x00491234
        Log("Audience += %3.5f", (double)d);
    }
}


// FUNCTION: EZ2AC_UNPACKED 0x0042e620
void PlayerSlot::m42e620(int grade, int count, int flag)
{
    SinkSlot *self = (SinkSlot *)this;
    int value[6];
    int alt;
    int c;

    /* 1. the note-value table */
    alt = g_1b2eb6c;
    if (alt == 0 && (g_modeIndex == 6 || g_modeIndex == 7))
        alt = g_1b2ea20;

    if (alt != 0)
    {
        value[3] = 40;
        value[4] = 100;
        value[5] = 170;
    }
    else
    {
        value[3] = 41;
        value[4] = 150;
        value[5] = 300;
    }
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;

    if (count <= 0)
        return;

    /* 2. the combo */
    if (g_1b2eb6c == 1)
    {
        /* CV2Mix. Only the top two tiers build a combo. */
        if (grade == 5 || grade == 4)
        {
            if (flag != 0)
            {
                if (self->f1c0 == 1)
                {
                    self->combo += count;
                }
                else
                {
                    self->combo = count - 1;
                    self->f1c0 = 1;
                }
                if (self->combo > self->maxCombo)
                    self->maxCombo = self->combo;
                g_1b5f19c = 0;
            }
        }
        else if (grade == 2 || grade == 1)
        {
            if (flag != 0)
                breakCombo(self, grade);
        }
        else if (grade == 3 && self->dir->fd18 != 0 && flag != 0)
        {
            self->combo = 0;
            self->f1c0 = 0;
        }
    }
    else
    {
        g_1b2efb0 = grade;

        if (grade == 5 || grade == 4 || grade == 3)
        {
            if (flag != 0)
            {
                self->combo += count;
                if (self->combo > self->maxCombo)
                    self->maxCombo = self->combo;
                g_1b5f19c = 0;
            }
        }
        else if (grade == 2 || grade == 1)
        {
            if (flag != 0)
                breakCombo(self, grade);
        }
    }

    /* 3. the score */
    if (g_1b2eb6c != 0)
    {
        c = self->combo;
        if (c >= 1500)
            c = 1499;
        self->score += (int)((float)value[grade] * g_comboMult[c]);
    }
    else
    {
        self->score += value[grade];
    }

    /* 4. the gauge, and 5. the audience */
    if (self->dir->f794 == 0)
    {
        if (self->gauge > 0.0f)
            self->gauge = self->dir->gaugeRate[grade] + self->gauge;
        if (g_1b2e7d4 <= 1 && g_useAudiencePanel == 1)
            self->gauge = 100.0f;

        if (countListA() >= 2)
            checkBothPlayers(self);
        else
            checkThisPlayer(self);

        addAudience(self, grade);
    }
    else if (self->dir->f794 == 1 && (grade == 5 || grade == 4))
    {
        float d = (g_1b2eaec[g_elemCount] + 10.0f) / (float)self->noteCount;

        addAudienceValue(self, d, 100.0f);
    }

    self->counts[grade]++;
}

/* EZ2DJSlotObject::release @0x42e360 - THE STAGE TEARDOWN, and it is where a
 * COURSE is totalled.
 *
 * Called from @0x420ec0, which walks the two slots and drops each object's
 * pointer immediately after - so this is the last thing a finished stage does.
 * What it does is accumulate that stage into the per-player summary block, and
 * the accumulation is what makes the Radio modes' multi-stage courses work:
 * every field is `+=`, and the grade is then taken over the RUNNING TOTAL.
 *
 * ---- the summary block, +0x230 at a 0x40 stride per player ----------------
 *
 * Fifteen fields, and this function names all of them:
 *
 *     [0]  +0x00  notes            += slot's noteCount (+0xe105bc)
 *     [1]  +0x04  (untouched here)
 *     [2]  +0x08  MISS             += counts[1]  (+0x1c8)
 *     [3]  +0x0c  FAIL             += counts[2]  (+0x1cc)
 *     [4]  +0x10  GOOD             += counts[3]  (+0x1d0)
 *     [5]  +0x14  COOL             += counts[4]  (+0x1d4)
 *     [6]  +0x18  KOOL             += counts[5]  (+0x1d8)
 *     [7]  +0x1c  combo            += combo      (+0x1dc)
 *     [8]  +0x20  maxCombo         += maxCombo   (+0x1e0)
 *     [9]  +0x24  score            += score      (+0x1e4)
 *     [11] +0x2c  gauge  (float)   += gauge      (+0x1f4)
 *     [12] +0x30  the GRADE, from the ladder @0x42e190 over the total
 *     [13] +0x34  = g_1b5f180
 *     [14] +0x38  = 100.0f
 *
 * The counter order is what pins the grade numbering independently of the
 * sink: block[k + 1] takes counts[k] for k = 5..1, so the slot's counter block
 * really does run MISS, FAIL, GOOD, COOL, KOOL from +0x1c8, and the ladder's
 * `b[6] + b[5]` really is KOOL + COOL.
 *
 * ---- g_1b2ea2c FORCES A PERFECT STAGE -------------------------------------
 *
 * When it is set - and the mode is not CV2Mix - the block is topped up as if
 * every note had been a KOOL: notes added to KOOL, combo and maxCombo, and
 * `notes * 300` (the same `(n << 4) - n` doubled twice the ladder uses for its
 * denominator) added to the score, with the grade then FORCED to 10 rather
 * than computed. It is set in one place, @0x429f2a, immediately after an
 * audience-meter bump, and cleared during slot setup @0x4305c5. What selects
 * it is not read; the effect is.
 *
 * The four re-reads of the note count are real - `add [mem]` can alias the
 * member, so cl reloads it at every use, and the source reads it each time.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// FUNCTION: EZ2AC_UNPACKED 0x0042e360
void EZ2DJSlotObject::release()
{
    PlayerSlot *self = (PlayerSlot *)this;
    SinkSlot *s = (SinkSlot *)this;
    int *b;

    b = &self->block[g_elemCount * 16];
    b[0] += s->noteCount;
    b[6] += s->counts[5];
    b[5] += s->counts[4];
    b[4] += s->counts[3];
    b[3] += s->counts[2];
    b[2] += s->counts[1];
    b[7] += s->combo;
    b[8] += s->maxCombo;
    b[9] += s->score;
    *(float *)&b[11] += s->gauge;
    b[13] = g_1b5f180;
    *(float *)&b[14] = 100.0f;
    b[12] = self->m42e190();

    if (g_1b2ea2c != 0 && g_1b2eb6c == 0)
    {
        b[6] += s->noteCount;
        b[7] += s->noteCount;
        b[8] += s->noteCount;
        b[9] += s->noteCount * 300;
        b[12] = 10;
    }

    if (s->dir->f7c4 == 0)
        self->f3cc = 0;
}

/* PlayerSlot::m42e2e0 @0x42e2e0 - THE COURSE RESULT: the players' grades,
 * averaged.
 *
 * This is the other half of the answer the stage teardown @0x42e360 sets up.
 * It walks every player's summary block at a 0x40 stride, reads the GRADE the
 * teardown stored at block[12], and returns the mean over `g_elemCount + 1`
 * players. So the result screen does NOT re-grade - it reads the stored
 * course grade, which is the one computed from the accumulated total.
 *
 * Four call sites: @0x423ae6, @0x4262dd, @0x4587ad and @0x458823, each
 * reaching it on a list-A node straight out of findListA.
 *
 * THE PER-PLAYER ARRAY IS DEAD. The loop stores each grade into a 10-int stack
 * array that nothing ever reads - the frame is exactly `sub esp,0x28` and the
 * base is the bottom of it. Only the sum is used. Written out because the
 * stores are in the original and a shorter array changes the frame.
 *
 * The two `g_1b2eb28 = 0` stores are NOT merged in the original, so they are
 * two statements: one when a second player is present (or mode 5 with at least
 * one) and the mode is 2 or 3, and one whenever g_1b2ea4c is set.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// FUNCTION: EZ2AC_UNPACKED 0x0042e2e0
int PlayerSlot::m42e2e0()
{
    int grades[10];
    int sum = 0;
    int i = 0;

    if (g_elemCount >= 0)
    {
        const int *p = &this->block[12];

        do
        {
            int g = *p;

            grades[i] = g;
            i++;
            sum += g;
            p += 16;
        } while (i <= g_elemCount);
    }

    if (g_elemCount >= 2 || (g_modeIndex == 5 && g_elemCount >= 1))
    {
        switch (g_modeIndex)
        {
        case 2:
            g_1b2eb28 = 0;
            break;
        case 3:
            g_1b2eb28 = 0;
            break;
        }
    }
    if (g_1b2ea4c != 0)
        g_1b2eb28 = 0;

    return sum / (g_elemCount + 1);
}
