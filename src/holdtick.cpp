#include <stdlib.h>

#include "players.h"
#include "gamepanels.h"

/* THE KEYS MODES' HOLD STATE MACHINE - three of the four functions the
 * original lays out back to back at 0x42f4e0, 0x42f880, 0x42fbf0 and
 * 0x42fd90, in that order, which is why they share this one TU: the fourth,
 * the keys commit @0x42fd90, calls m42fbf0 ten times and has to see its body.
 *
 * PlayerSlot::m42f4e0 @0x0042f4e0 - THE HOLD-SUSTAIN TICK, and the last of
 * the judgement sink's four callers to be read (five of the sink's twelve
 * call sites are in here; docs/oracle-trace.md called it "unread").
 *
 * One call pays one lane's next instalment of a long note. The state lives
 * in the per-lane tables PlayerSlot keeps in a row from +0x10:
 *
 *     t58[lane]   the sustain is running
 *     ta0[lane]   how many instalments are still owed
 *     te8[lane]   the song time the next instalment falls due
 *     t130[lane]  the display lane the judgement is drawn on
 *     t178[lane]  the grade this instalment will award
 *     t10[lane]   a per-lane flag that forces KOOL (with the +0xe105b8 byte)
 *
 * `kind` is the value the caller reads out of the slot's own +0x210, which
 * the keys commit's predecessor latches; 7..12 pay nothing at all, and it
 * picks the beat division the instalments fall on: 1 = half, 2 = an eighth,
 * 3 = a sixteenth, anything else a quarter of whatever the chart object at
 * +0x3d4 reports as the measure. The song time comes from slot 22 of the
 * player at +0x3d8 - the same "clock - 5" tick judgement uses.
 *
 * Every instalment ends the same way: the due time moves on by one
 * division, a grade above GOOD is knocked down to FAIL, the KOOL override
 * is applied, the sink is paid one note at that grade, the effect is drawn
 * through the track sink at +0x3cc, and one instalment comes off the debt.
 *
 * `kind` 6 is the exception - the last two instalments of its holds are
 * GRADED rather than paid flat, on how far past the due time the tick
 * lands (a sixty-fourth of a measure is the window), and paying them ends
 * the hold outright rather than counting down.
 *
 * WIP: not matched yet - this is the reading, committed so the port has it.
 *
 * The tail duplication has been swept and 75.41% is the best of it: sharing
 * the tail between all four ladder arms is 67.40%, the three ladder arms
 * copied (as below) is 75.41%. The re-read discriminator that unparked
 * ScrFader::update @0x450450 does NOT apply here - it was tried and cost
 * eight points - because the original really does keep ONE shared tail at
 * 0x42f620 reached by three jumps plus a second copy in the kind-6 path, and
 * cl will not reproduce that split from either source shape. Do not re-sweep
 * the copy count.
 */

/* The chart object at +0x3d4: the same class src/accessors.cpp models for
   its @0x422dd0 accessor, redeclared here rather than moved to a header so
   that TU is not touched. */
struct Obj422DD0
{
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void query8(int *buf, int n);   /* slot 8 -> vtable+0x20 */
    int get();
};

/* The song player at +0x3d8; only slot 22, the song tick, is called here. */
struct HoldClock
{
    virtual void s0();  virtual void s1();  virtual void s2();  virtual void s3();
    virtual void s4();  virtual void s5();  virtual void s6();  virtual void s7();
    virtual void s8();  virtual void s9();  virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21();
    virtual unsigned int tick();            /* slot 22 -> vtable+0x58 */
    virtual void s23(); virtual void s24(); virtual void s25();
    virtual void s26(); virtual void s27(); virtual void s28();
    virtual void s29(); virtual void s30(); virtual void s31();
    /* slot 32 -> vtable+0x80: the keys commit hands it a sound handle. */
    virtual void s32(int handle);
};

/* Two accessors src/accessors.cpp owns, redeclared for the same reason the
   two above are: the commit reaches both through PlayerSlot's own pointers.
   @0x422d10 is the owner seen as a 2-D setter, @0x426d10 a bounds-checked
   getter on the object at +0x3dc. */
struct Obj422D10 { char _pad[0xc80]; int arr[1][18]; void set(int a, int b, int c); };
/* The GF owner's twin of the above - src/accessors.cpp owns the definition,
   this is the view PlayerRecord::m466ac0 @0x466ac0 reaches it through. */
struct Obj45F180 { char _pad[0xc14]; int arr[1][18]; void set(int a, int b, int c); };

struct Rec426CA0 { int f0; int rest[22]; };   /* 0x5c */

struct Obj426CA0
{
    char      _pad[0x14];
    Rec426CA0 arr[96];   /* 0x0014 .. 0x2294 */
    int       count;     /* 0x2294 */
    int  getF0(int i);          /* 0x426d10 */
};

/* Set while the stage is in the state that pays every instalment KOOL. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2e0
int g_1b5f2e0;

/* The same override, as an option rather than a state - only the quarter
   division consults it. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efa4
int g_1b2efa4;

extern int g_elemCount;      /* 0x1b2e718 (src/loops.cpp) - the player cursor */
extern int g_1b5f300[5];     /* 0x1b5f300 (src/modeselectctor.cpp) */
extern int g_catchJudging;   /* 0x1b2e58c (src/playerslot.cpp) */

extern int g_mixStyle;       /* 0x1b2ebc8 (src/panels.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0042f4e0
void PlayerSlot::m42f4e0(int lane, int kind)
{
    unsigned int now;
    int measure;

    if (t58[lane] == 0)
        return;
    if (ta0[lane] <= 0)
    {
        t58[lane] = 0;
        ta0[lane] = 0;
        return;
    }
    now = ((HoldClock *)f3d8)->tick();
    if (kind >= 7 && kind <= 12)
        return;
    if (kind == 1)
    {
        int buf[8];

        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        if (now <= (unsigned int)(buf[2] / 2 + te8[lane]))
            return;
        if (g_1b5f2e0 != 0)
            t178[lane] = 5;
        te8[lane] += ((Obj422DD0 *)f3d4)->get() / 2;
        if (t178[lane] >= 3)
            t178[lane] = 2;
        if (g_1b5f2e0 != 0)
            t178[lane] = 5;
        else if (((char *)this)[0xe105b8] != 0 && t10[lane] != 0)
            t178[lane] = 5;
        m42e620(t178[lane], 1, 1);
        f3cc->m42d370(t130[lane], t178[lane], 0);
        ta0[lane] -= 1;
        return;
    }
    else
    {
        measure = ((Obj422DD0 *)f3d4)->get();
        if (kind == 2)
        {
            if (now <= (unsigned int)(measure / 8 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 8;
        if (t178[lane] >= 3)
            t178[lane] = 2;
        if (g_1b5f2e0 != 0)
            t178[lane] = 5;
        else if (((char *)this)[0xe105b8] != 0 && t10[lane] != 0)
            t178[lane] = 5;
        m42e620(t178[lane], 1, 1);
        f3cc->m42d370(t130[lane], t178[lane], 0);
        ta0[lane] -= 1;
            return;

        }
        else if (kind == 3)
        {
            if (now <= (unsigned int)(measure / 16 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 16;
        if (t178[lane] >= 3)
            t178[lane] = 2;
        if (g_1b5f2e0 != 0)
            t178[lane] = 5;
        else if (((char *)this)[0xe105b8] != 0 && t10[lane] != 0)
            t178[lane] = 5;
        m42e620(t178[lane], 1, 1);
        f3cc->m42d370(t130[lane], t178[lane], 0);
        ta0[lane] -= 1;
            return;

        }
        else
        {
            if (now <= (unsigned int)(measure / 4 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            if (g_1b2efa4 == 1)
                t178[lane] = 5;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 4;
            if (kind >= 7 && kind <= 12)
                goto dec6;
            if (kind == 6)
            {
                if (ta0[lane] == 1)
                {
                    if (g_1b5f2e0 != 0)
                    {
                        t178[lane] = 5;
                        m42e620(5, 1, 1);
                        f3cc->m42d370(t130[lane], t178[lane], 0);
                        ta0[lane] -= 1;
                        return;
                    }
                }
                else if (g_1b5f2e0 != 0)
                {
                    goto dec6;
                }
                if (ta0[lane] >= 1 && ta0[lane] <= 2)
                {
                    if (t178[lane] == 5)
                    {
                        unsigned int late = now - te8[lane];
                        int v = ((Obj422DD0 *)f3d4)->get();

                        if (ta0[lane] == 1)
                            t178[lane] = late < (unsigned int)v >> 6 ? 3 : 4;
                        else if (ta0[lane] == 2)
                            t178[lane] = late < (unsigned int)v >> 6 ? 2 : 3;
                        m42e620(t178[lane], 1, 1);
                        f3cc->m42d370(t130[lane], t178[lane], 0);
                    }
                    else
                    {
                        t178[lane] = 3;
                        m42e620(t178[lane], 1, 1);
                        f3cc->m42d370(t130[lane], t178[lane], 0);
                    }
                }
                else
                {
                    t178[lane] = 2;
                    m42e620(t178[lane], 1, 1);
                    f3cc->m42d370(t130[lane], t178[lane], 0);
                }
                ta0[lane] = 1;
            dec6:
                ta0[lane] -= 1;
                return;
            }
        }
    }
    if (t178[lane] >= 3)
        t178[lane] = 2;
    if (g_1b5f2e0 != 0)
        t178[lane] = 5;
    else if (((char *)this)[0xe105b8] != 0 && t10[lane] != 0)
        t178[lane] = 5;
    m42e620(t178[lane], 1, 1);
    f3cc->m42d370(t130[lane], t178[lane], 0);
    ta0[lane] -= 1;
}

/* PlayerSlot::m42f880 @0x0042f880 - THE HOLD PUMP, the sustain tick's twin:
 * four more of the judgement sink's twelve call sites, and with
 * src/holdtick.cpp done it leaves only the keys commit @0x42fd90.
 *
 * Same skeleton as the tick @0x42f4e0 - same five per-lane tables, same
 * "is the sustain running / are instalments still owed" head, same
 * measure-division ladder off the slot's +0x210 latch - but this is the one
 * that runs while the key is HELD, and it pays differently:
 *
 *   - A lane whose grade has fallen BELOW GOOD is not paid at all. It is
 *     RESTORED to GOOD instead, and only when this player's entry in the
 *     option array @0x1b5f300 is not negative (ModeSelectDirector's ctor
 *     seeds all five to 7). That is the only thing this function does on
 *     that path - no sink, no effect, not even a decrement.
 *   - COOL and KOOL (grade > 3) additionally bump the counter at +0xc;
 *     GOOD pays the sink and draws the effect but does not. Both arms push
 *     their three arguments before the split, which is why the original has
 *     two sink call sites here rather than one.
 *   - The effect is drawn with a trailing 1 where the tick passes 0.
 *   - `kind` 6 pays NOTHING until the last instalment, and even then only
 *     if the grade is still KOOL; every earlier tick just advances the due
 *     time and counts down.
 *   - `kind` 0 and 13+ share the quarter division with 6, but reach it
 *     through the chart query written out inline rather than through the
 *     @0x422dd0 accessor - the same split the tick has.
 *
 * The divisions: 1 = half, 2 = an eighth, 3 = a sixteenth, 6/0/13+ = a
 * quarter, 7..12 nothing, and 4/5 fall out without touching anything.
 *
 * STRUCTURE (2026-10-03): the pay-out is written out in EVERY timing arm,
 * as `if (grade <= 3) {...} else {... +0xc}`, and cl cross-jumps the
 * copies into the kind-1 arm's and the kind-6 arm's calls - one pay-out
 * after the ladder attaches the shared tail to the wrong arm. The test is
 * still ONE expression, `tick() <= get()/n + te8`, and the kind-0/13+ arm
 * reads buf[2] into a named local before the clock call: that puts it in
 * ebx, which makes the ebx save precede the kind-0 branch, which is what
 * lets that arm's tails merge (without it the arm has no `pop ebx` and its
 * copies stay separate).
 */

// FUNCTION: EZ2AC_UNPACKED 0x0042f880
void PlayerSlot::m42f880(int lane, int kind)
{
    if (t58[lane] == 0)
        return;
    if (ta0[lane] <= 0)
    {
        t58[lane] = 0;
        ta0[lane] = 0;
        return;
    }
    if (kind >= 7 && kind <= 12)
        return;
    if (t178[lane] >= 3)
    {
        if (kind == 0 || kind > 12)
        {
            int buf[8];

            int   len;

            ((Obj422DD0 *)f3d4)->query8(buf, 4);
            len = buf[2];
            if (((HoldClock *)f3d8)->tick() <= (unsigned int)(len / 4 + te8[lane]))
                return;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 4;
            if (t178[lane] <= 3)
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
            }
            else
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
                fc += 1;
            }
            ta0[lane] -= 1;
        }
        else if (kind == 1)
        {
            if (((HoldClock *)f3d8)->tick() <=
                (unsigned int)(((Obj422DD0 *)f3d4)->get() / 2 + te8[lane]))
                return;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 2;
            if (t178[lane] <= 3)
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
            }
            else
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
                fc += 1;
            }
            ta0[lane] -= 1;
        }
        else if (kind == 2)
        {
            if (((HoldClock *)f3d8)->tick() <=
                (unsigned int)(((Obj422DD0 *)f3d4)->get() / 8 + te8[lane]))
                return;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 8;
            if (t178[lane] <= 3)
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
            }
            else
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
                fc += 1;
            }
            ta0[lane] -= 1;
        }
        else if (kind == 3)
        {
            if (((HoldClock *)f3d8)->tick() <=
                (unsigned int)(((Obj422DD0 *)f3d4)->get() / 16 + te8[lane]))
                return;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 16;
            if (t178[lane] <= 3)
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
            }
            else
            {
                m42e620(t178[lane], 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
                fc += 1;
            }
            ta0[lane] -= 1;
        }
        else if (kind == 6)
        {
            if (((HoldClock *)f3d8)->tick() <=
                (unsigned int)(((Obj422DD0 *)f3d4)->get() / 4 + te8[lane]))
                return;
            te8[lane] += ((Obj422DD0 *)f3d4)->get() / 4;
            if (ta0[lane] == 1 && t178[lane] == 5)
            {
                m42e620(5, 1, 1);
                f3cc->m42d370(t130[lane], t178[lane], 1);
            }
            ta0[lane] -= 1;
            return;
        }
        else
            return;
    }
    else if (g_1b5f300[g_elemCount] >= 0)
    {
        t178[lane] = 3;
    }
}

/* PlayerSlot::m42fbf0 @0x0042fbf0 - START A HOLD, the state the tick and the
 * pump then spend. Five arguments: the lane, the grade the head press
 * earned, the note's length in ticks, the time it starts at, and its KIND -
 * which is a BYTE OF THE NOTE RECORD, not the game mode (the commit
 * @0x42fd90 reads it out of the record at +0x32 and passes it here), and
 * which this function latches into the slot's +0x210 for the tick and the
 * pump to read back.
 *
 * The kind picks the instalment size the same way they do - 1 a half of the
 * measure, 2 an eighth, 3 a sixteenth, 0/6/13+ a quarter - except that
 * kinds 4 and 5 fall through to a step of the note's own length, i.e. one
 * instalment. 7..12 latch the kind and do nothing else at all.
 *
 * The count is `length / step`, MINUS ONE unless the note is kind 6 or this
 * player's option @0x1b5f300 is negative, and clamped up to 1. Under
 * MIX STYLE 7 none of that applies: the count is the plain quotient.
 *
 * NOT A COIN AFTER ALL. This was parked at 66.67% on the reading that the
 * original materializes the CONSTANT 1 in ebp while cl caches `length`
 * there instead - "the zero-register-vs-counter coin flip one level along,
 * and nothing at source level exposes the ranking". That was wrong. The
 * two are not competing for one register by chance: the original's
 * `length` is MEMORY-RESIDENT, reloaded from its home slot at each of its
 * four uses, which is what leaves ebp free for the constant. Reading it
 * through a volatile alias at each use - the documented lever for a
 * parameter the original re-reads - frees ebp, and `mov ebp,1` /
 * `cmp ebx,ebp` / `sub eax,ebp` all come out. 66.67% -> 78.05%.
 *
 * What is left is two layout choices, not registers. cl inverts the last
 * ladder test (`kind == 6 || kind >= 13`) and lays its six-byte else arm
 * inline where the original lays the body inline and the else past it; and
 * it saves edi ahead of the mix-style test where the original saves it
 * inside the non-style-7 arm. Both are the same size, so everything after
 * them is displacement noise.
 *
 * Already tried and NOT the answer, from the original park: a named local
 * for the quotient (`int n = length / step;`), `+ -1` for the subtraction,
 * and the two halves in either layout order - though that last one is
 * load-bearing and stays, since `if (g_mixStyle != 7) { normal } else
 * { style7 }` is what puts the style-7 half out of line.
 */
// FUNCTION: EZ2AC_UNPACKED 0x0042fbf0
void PlayerSlot::m42fbf0(int lane, int grade, int length, int start, int kind)
{
    int step;
    int buf[8];

    if (kind == 0)
    {
        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        step = buf[2] / 4;
    }
    else if (kind == 1)
    {
        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        step = buf[2] / 2;
    }
    else if (kind == 2)
    {
        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        step = buf[2] / 8;
    }
    else if (kind == 3)
    {
        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        step = buf[2] / 16;
    }
    else if (kind == 6 || kind >= 13)
    {
        ((Obj422DD0 *)f3d4)->query8(buf, 4);
        step = buf[2] / 4;
    }
    else
    {
        step = length;
    }
    f210 = kind;
    if (kind >= 7 && kind <= 12)
        return;
    if (g_mixStyle != 7)
    {
        if (g_1b5f300[g_elemCount] >= 0)
        {
            if (kind != 6)
                ta0[lane] = *(volatile int *)&length / step - 1;
            else
                ta0[lane] = *(volatile int *)&length / step;
        }
        else if (kind == 6)
        {
            ta0[lane] = *(volatile int *)&length / step;
        }
        if (ta0[lane] <= 0)
            ta0[lane] = 1;
        te8[lane] = start;
        t58[lane] = 1;
        t130[lane] = lane;
        t178[lane] = grade;
    }
    else
    {
        te8[lane] = start;
        ta0[lane] = *(volatile int *)&length / step;
        if (ta0[lane] <= 0)
            ta0[lane] = 1;
        t58[lane] = 1;
        t130[lane] = lane;
        t178[lane] = grade;
    }
}

/* CatchPlayer::m473a60 @0x473a60 - START A HOLD, the catch twin of
 * PlayerSlot::m42fbf0 above, and the same five arguments. Read that
 * function's header for what they mean; everything it says holds here.
 *
 * The one real difference is the subdivision table. The keys modes offer
 * four steps - a half, an eighth, a sixteenth, and a quarter for 0/6/13+ -
 * and the catch game offers SIX, adding a sixth (kind 2) and a
 * twenty-fourth (kind 5) and shifting the rest along, so its kinds are
 * 1 -> 1/2, 2 -> 1/6, 3 -> 1/8, 4 -> 1/16, 5 -> 1/24, 0/6/13+ -> 1/4.
 * Kinds 7..12 still latch and do nothing, and the latch is +0x370 here
 * rather than +0x210.
 *
 * The tempo summary comes off the sample sink at +0x364 rather than the
 * keys game's +0x3d4, and it is queried afresh inside EVERY arm - six call
 * sites for one logical call, which is what writing the arms out separately
 * gives. `length` is read through a volatile alias for the same reason it
 * is in the twin: the original reloads it from its home slot at each of its
 * three uses.
 *
 * 83.77%, and every remaining defect is cl's own block placement - the same
 * family as the twin's, and better than the twin's 78.05%. Two things are
 * wrong and neither is steerable from here:
 *
 *   The last two arms are laid out the other way round: the original falls
 *   through into the quarter-step body and jumps forward to `step = length`,
 *   ours does the reverse. Writing the condition inverted, so that the arm
 *   cl inlines is the one the original inlines, changes NOTHING - the output
 *   is byte-identical either way.
 *
 *   And cl CROSS-JUMPS the two mix-style arms' trailing `t58`/`t130` stores,
 *   which the original keeps separate. The original can keep them separate
 *   because its two arms hold the lane in different registers - edi on the
 *   ordinary path (pushed only there) and a freshly reloaded ecx under mix
 *   style 7 - so there is nothing to merge. Reading `lane` through a
 *   volatile alias in the second arm to force that reload also changes
 *   nothing. All three source forms compile to the same 83.77%.
 */
// FUNCTION: EZ2AC_UNPACKED 0x00473a60
void CatchPlayer::m473a60(int lane, int grade, int length, int start, int kind)
{
    int step;
    int buf[8];

    if (kind == 0)
    {
        f364->query(buf, 4);
        step = buf[2] / 4;
    }
    else if (kind == 1)
    {
        f364->query(buf, 4);
        step = buf[2] / 2;
    }
    else if (kind == 2)
    {
        f364->query(buf, 4);
        step = buf[2] / 6;
    }
    else if (kind == 3)
    {
        f364->query(buf, 4);
        step = buf[2] / 8;
    }
    else if (kind == 4)
    {
        f364->query(buf, 4);
        step = buf[2] / 16;
    }
    else if (kind == 5)
    {
        f364->query(buf, 4);
        step = buf[2] / 24;
    }
    else if (kind == 6 || kind >= 13)
    {
        f364->query(buf, 4);
        step = buf[2] / 4;
    }
    else
    {
        step = length;
    }
    f370 = kind;
    if (kind >= 7 && kind <= 12)
        return;
    if (g_mixStyle != 7)
    {
        if (g_1b5f300[g_elemCount] >= 0)
        {
            if (kind != 6)
                ta0[lane] = *(volatile int *)&length / step - 1;
            else
                ta0[lane] = *(volatile int *)&length / step;
        }
        else if (kind == 6)
        {
            ta0[lane] = *(volatile int *)&length / step;
        }
        te8[lane] = start;
        if (ta0[lane] <= 0)
            ta0[lane] = 1;
        t58[lane] = 1;
        t130[lane] = lane;
        t178[lane] = grade;
    }
    else
    {
        te8[lane] = start;
        ta0[lane] = *(volatile int *)&length / step;
        if (ta0[lane] <= 0)
            ta0[lane] = 1;
        t58[lane] = 1;
        t130[lane] = lane;
        t178[lane] = grade;
    }
}

/* The chord table: three dwords an entry - the addressing is
   `[ecx+ecx*1+base]` off a 6k index, so the stride is 12, not 24. */
struct ChordSlot
{
    int start;      /* 0x00 */
    int prev;       /* 0x04 */
    int grade;      /* 0x08 */
};

// GLOBAL: EZ2AC_UNPACKED 0x037dcdc0
ChordSlot g_chord[128];
// GLOBAL: EZ2AC_UNPACKED 0x037dce98
int g_chordScore;


/* PlayerSlot::m42fd90 @0x0042fd90 - THE KEYS COMMIT, the fourth and last of
   the judgement sink's callers: 1,900 bytes, ~450 instructions, no EH,
   frame 0x60, `ret 0x10`. This is the reading turned into code; it belongs
   in src/holdtick.cpp's TU when it builds and scores, because it calls
   m42fbf0 ten times and the original lays all four out together
   (0x42f4e0, 0x42f880, 0x42fbf0, 0x42fd90, in that order).

   ---- what it is ---------------------------------------------------------

       void PlayerSlot::m42fd90(int lane, SlotSound *s, int grade, int flag)

   `flag` 1 is a PRESS, anything else a RELEASE, and it is also the third
   argument the effect is drawn with. Everything else comes out of the
   sink: `getState` (slot 17) fills a SlotState for this hit and returns a
   status, and the clock's slot 22 gives the time the press landed.

   THE WHOLE BODY EXISTS TWICE - `g_mixStyle == 7` picks between two nearly
   identical halves, exactly as the sink @0x42e620 is two functions in one.
   The style-7 half differs in three places only: it demands BOTH a nonzero
   status AND a hold (`width > 6`) before it re-queries, it knocks a KOOL
   down to COOL after the skin retune, and it skips the +0xe105b8 test.

   ---- the press ----------------------------------------------------------

   A press with `width > 6` (i.e. a long note) STARTS A HOLD through
   m42fbf0, with the grade promoted three different ways depending on this
   player's option @0x1b5f300 and on @0x1b2efa4 - and the arm that promotes
   nothing is also the arm that starts no hold and leaves `pay` set. Above
   the sink's threshold at +0x38 it additionally writes the resync
   correction into the state (`offset = now - start`), hands it back with
   `setState` (slot 18), and remembers the sound in `snd[lane]`.

   ---- the release --------------------------------------------------------

   If `snd[lane].id` is not -1 the hold is closed: getState fills a second
   state out of `snd[lane]`, and the grade becomes `(that state's f7 +
   grade + 1) / 2` - the head's grade averaged with the tail's, which is
   docs/judge-timing.md's "a hold is graded twice but TALLIED ONCE" seen
   from this side. Both halves of `snd[lane]` go back to -1 and `pay` is
   cleared, so the average is drawn but never paid twice.

   ---- the chord table @0x37dcdc0 -----------------------------------------

   Gated on this+0x214. Six dwords per entry, indexed by the state's f6:
   +0x00 the start time currently held, +0x04 the previous one, +0x08 the
   grade it was hit with. A second hit on the same start time adds
   `4 - |thatGrade - grade|` to the counter at 0x37dce98 and sets this+0x5b4
   when both grades are at least GOOD and equal - the same-frame chord
   bookkeeping.

   ---- the tail -----------------------------------------------------------

       m42e620(grade, 1, pay2);              // the sink, the only site in
       f3cc->m42d370(lane, shown, flag);     // the binary with a variable
                                             // third argument
*/
// FUNCTION: EZ2AC_UNPACKED 0x0042fd90
void PlayerSlot::m42fd90(int lane, SlotSound *s, int grade, int flag)
{
    SlotState st;
    SlotState tail;
    int status;
    int pay;
    int shown;
    int handle;
    int now;
    char drawShown;

    pay = 1;
    status = f3d0->sink->getState(s, &st);
    drawShown = 0;
    shown = 0;
    now = f3d0->clock->now();
    handle = f3d0->tracks[id].items[lane].handle;
    if (g_mixStyle != 7)
    {
        if (st.kind >= 7 && st.kind <= 12)
            return;
        if (flag == 1)
        {
            if (status == 1 && st.width > 6)
            {
                if (grade >= f3cc->f38)
                {
                    pay = 0;
                    if (g_1b5f300[g_elemCount] >= 0 && st.kind != 6)
                    {
                        if (grade == 4)
                            grade = 5;
                        m42fbf0(lane, grade, st.width, st.start, st.kind);
                    }
                    else if (g_1b2efa4 == 1)
                    {
                        if (grade == 4)
                            grade = 5;
                        m42fbf0(lane, grade, st.width, st.start, st.kind);
                    }
                    else
                    {
                        if (grade == 4)
                            grade = 5;
                        if (grade == 5)
                            m42fbf0(lane, 5, st.width, st.start, st.kind);
                        else
                            pay = 1;
                    }
                    st.offset = now - st.start;
                    f3d0->sink->setState(s, &st);
                    snd[lane].f0 = s->f0;
                    snd[lane].id = s->id;
                    if (g_1b5f300[g_elemCount] >= 0)
                        pay = 1;
                }
                else
                {
                    /* NO `pay = 0` here: the below-threshold press branch at
                       0x42ff5a opens directly on the g_1b5f300 test. Only the
                       ABOVE-threshold branch clears pay. Reported from the port
                       side (port/REQUESTS.md, 2026-08-16) and verified in the
                       disassembly: a hold head under Panel+0x38 commits like a tap. */
                    if (g_1b5f300[g_elemCount] >= 0 && st.kind != 6)
                    {
                        if (grade == 4)
                            grade = 5;
                        m42fbf0(lane, grade, st.width, st.start, st.kind);
                    }
                    else if (g_1b2efa4 == 1)
                    {
                        if (grade == 4)
                            grade = 5;
                        m42fbf0(lane, grade, st.width, st.start, st.kind);
                    }
                    else
                    {
                        if (grade == 4)
                            grade = 5;
                        if (grade == 5)
                            m42fbf0(lane, 5, st.width, st.start, st.kind);
                        else
                            pay = 1;
                    }
                }
            }
            if (g_1b5f2e0 != 0)
            {
                f3cc->m4299e0(lane, 2);
                if (status == 0 || st.width <= 6)
                    ((Obj422D10 *)f3d0)->set(id, lane, 10);
            }
            if (((Obj426CA0 *)f3dc)->getF0(handle) == 1 && g_1b5f2e0 == 0)
            {
                drawShown = 1;
                ((HoldClock *)f3d8)->s32(handle);
                f3cc->m4299e0(lane, 2);
                if (status == 0 || st.width <= 6)
                    ((Obj422D10 *)f3d0)->set(id, lane, 10);
                t10[lane] = 1;
            }
            else
            {
                t10[lane] = 0;
                goto pay_a;
            }
        }
        else
        {
            if (snd[lane].id != -1)
            {
                if (grade >= f3cc->f38)
                {
                    ((SlotSampleSink *)f3d4)->getState(&snd[lane], &tail);
                    grade = (tail.f7 + grade + 1) / 2;
                }
                snd[lane].f0 = -1;
                snd[lane].id = -1;
                pay = 0;
            }
            if (g_1b5f2e0 != 0)
                f3cc->m4299e0(lane, 1);
            if (((Obj426CA0 *)f3dc)->getF0(handle) == 1 && g_1b5f2e0 == 0)
            {
                drawShown = 1;
                f3cc->m4299e0(lane, 1);
            }
            else
            {
                /* Like the press: an undrawn release goes straight to the
                   pay test (0x430123 / 0x43012c -> 0x43014e), past `shown`
                   and the +0xe105b8 zeroing. */
                goto pay_a;
            }
        }
        shown = grade;
        if (((char *)this)[0xe105b8] == 0)
            grade = 0;
    pay_a:
        if (pay == 0)
            return;
        {
            int pay2 = 1;

            if (grade >= 3)
            {
                if (status == 1 && (f4f4 != 0 || f51c != 0) && st.skin != -1)
                    m42ebf0(st.skin, 1);
            }
            else if ((grade == 2 || grade == 1) && s->id == -1)
            {
                pay2 = 0;
            }
            if (f214 != 0)
            {
                int k = st.f6;
                int v;

                ((SlotSampleSink *)f3d4)->getState(s, &st);
                v = st.start;
                if (g_chord[k].start == 0)
                {
                    if (g_chord[k].prev != v)
                    {
                        g_chord[k].start = v;
                        g_chord[k].grade = grade;
                    }
                }
                else if (g_chord[k].start == v && g_chord[k].prev != v)
                {
                    int was = g_chord[k].grade;
                    int d = was - grade;

                    if (d < 0)
                        d = -d;
                    g_chordScore += 4 - d;
                    g_chord[k].prev = v;
                    g_chord[k].start = 0;
                    if (grade >= 3 && was == grade)
                        f5b4 = 1;
                    else
                        f5b4 = 0;
                }
                else
                {
                    g_chord[k].start = 0;
                }
            }
            m42e620(grade, 1, pay2);
            if (drawShown != 0)
                f3cc->m42d370(lane, shown, flag);
            else
                f3cc->m42d370(lane, grade, flag);
        }
    }
    else
    {
        /* the style-7 half - written second because that is where the
           original puts it, out of line past the other's epilogue */
        if (flag == 1)
        {
            if (status == 1 && st.width > 6 && grade >= f3cc->f38)
            {
                st.offset = now - st.start;
                f3d0->sink->setState(s, &st);
                snd[lane].f0 = s->f0;
                snd[lane].id = s->id;
                pay = 0;
            }
            if (g_1b5f2e0 != 0)
            {
                f3cc->m4299e0(lane, 2);
                if (status == 0 || st.width <= 6)
                    ((Obj422D10 *)f3d0)->set(id, lane, 10);
            }
        }
        else
        {
            if (snd[lane].id != -1)
            {
                if (grade >= f3cc->f38)
                {
                    ((SlotSampleSink *)f3d4)->getState(&snd[lane], &tail);
                    grade = (tail.f7 + grade + 1) / 2;
                }
                snd[lane].f0 = -1;
                snd[lane].id = -1;
            }
            if (g_1b5f2e0 != 0)
                f3cc->m4299e0(lane, 1);
        }
            if (pay == 0)
            return;
        {
            int pay2 = 1;

            if (grade >= 3)
            {
                if (status == 1 && (f4f4 != 0 || f51c != 0) && st.skin != -1)
                    m42ebf0(st.skin, 1);
                if (grade == 5)
                    grade = 4;
            }
            else if ((grade == 2 || grade == 1) && s->id == -1)
            {
                pay2 = 0;
            }
            if (f214 != 0)
            {
                int k = st.f6;
                int v;

                ((SlotSampleSink *)f3d4)->getState(s, &st);
                v = st.start;
                if (g_chord[k].start == 0)
                {
                    if (g_chord[k].prev != v)
                    {
                        g_chord[k].start = v;
                        g_chord[k].grade = grade;
                    }
                }
                else if (g_chord[k].start == v && g_chord[k].prev != v)
                {
                    int was = g_chord[k].grade;
                    int d = was - grade;

                    if (d < 0)
                        d = -d;
                    g_chordScore += 4 - d;
                    g_chord[k].prev = v;
                    g_chord[k].start = 0;
                    if (grade >= 3 && was == grade)
                        f5b4 = 1;
                    else
                        f5b4 = 0;
                }
                else
                {
                    g_chord[k].start = 0;
                }
            }
            m42e620(grade, 1, pay2);
            if (drawShown != 0)
                f3cc->m42d370(lane, shown, flag);
            else
                f3cc->m42d370(lane, grade, flag);
        }
    }
}


/* CatchPlayer::m473660 @0x473660 - ONE INSTALMENT of a running hold, the
 * catch twin of PlayerSlot::m42f4e0 above. Same guard (a dead lane returns,
 * an exhausted one clears itself), same per-kind instalment size - the
 * catch's six steps, not the keys' four - and the same "pay when the clock
 * has passed the due time, then push the due time on by one more step".
 *
 * Two things it does that the keys tick does not:
 *
 *   the quarter-step arm (kinds 0, 6 and 13+) also forces the grade to 5 when
 *   g_1b2efa4 is up, beside the g_1b5f2e0 test every arm has;
 *
 *   and kind 6 - the hold's TAIL - grades on the OVERSHOOT rather than paying
 *   a flat instalment. With one instalment left and the option down, the
 *   grade comes from how far past the due time the clock has gone, measured
 *   against a sixty-fourth of the measure: 4 or 3 on the last instalment, 3
 *   or 2 on the second last, and a flat 2 outside that range. A grade that
 *   was not already 5 is knocked to 3. All of those set the count to 1 and
 *   then fall into the same decrement, so the hold ends there.
 *
 * The tail - commit, show, decrement - is SHARED by every timing arm here.
 * Written once for that reason. The kind-6 grading is NOT: each grade arm
 * carries its own show + commit pair, as in the keys copy - the original
 * keeps three m472fc0 call sites there (77.22% -> 82.95%, 2026-10-03).
 *
 * Residues:
 *
 *   `measure` lives in ecx for us and in eax for the original, so the two
 *   power-of-two arms pay a `mov eax,ecx` before their shift. The original
 *   keeps the one get() result in eax across all six kind tests.
 *
 *   and our early returns share an epilogue laid out in the MIDDLE of the
 *   function where the original's share the one at the end - which is what
 *   makes every exit displacement differ by a constant. Same shared-block
 *   placement family as the three background loaders.
 */
// FUNCTION: EZ2AC_UNPACKED 0x00473660
void CatchPlayer::m473660(int lane, int kind)
{
    unsigned int now;
    int measure;
    int step;

    if (t58[lane] == 0)
        return;
    if (ta0[lane] <= 0)
    {
        t58[lane] = 0;
        ta0[lane] = 0;
        return;
    }

    now = (unsigned int)f368->now();
    if (kind >= 7 && kind <= 12)
        return;

    if (kind == 1)
    {
        int buf[8];

        f364->query(buf, 4);
        if (now <= (unsigned int)(buf[2] / 2 + te8[lane]))
            return;
        if (g_1b5f2e0 != 0)
            t178[lane] = 5;
        step = ((Obj422DD0 *)f364)->get() / 2;
    }
    else
    {
        measure = ((Obj422DD0 *)f364)->get();
        if (kind == 2)
        {
            if (now <= (unsigned int)(measure / 6 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            step = ((Obj422DD0 *)f364)->get() / 6;
        }
        else if (kind == 3)
        {
            if (now <= (unsigned int)(measure / 8 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            step = ((Obj422DD0 *)f364)->get() / 8;
        }
        else if (kind == 4)
        {
            if (now <= (unsigned int)(measure / 16 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            step = ((Obj422DD0 *)f364)->get() / 16;
        }
        else if (kind == 5)
        {
            if (now <= (unsigned int)(measure / 24 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = kind;
            step = ((Obj422DD0 *)f364)->get() / 24;
        }
        else
        {
            if (now <= (unsigned int)(measure / 4 + te8[lane]))
                return;
            if (g_1b5f2e0 != 0)
                t178[lane] = 5;
            if (g_1b2efa4 == 1)
                t178[lane] = 5;
            te8[lane] += ((Obj422DD0 *)f364)->get() / 4;

            if (kind >= 7 && kind <= 12)
                goto dec;
            if (kind == 6)
            {
                unsigned int over;
                int          sixty4th;

                if (ta0[lane] == 1)
                {
                    if (g_1b5f2e0 == 0)
                        goto grade;
                    t178[lane] = 5;
                    ((PanelSlot *)this)->m472fc0(5, 1, 1);
                    f35c->m472300(t130[lane], t178[lane], 0);
                    goto dec;
                }
                if (g_1b5f2e0 != 0)
                    goto dec;

            grade:
                if (ta0[lane] >= 1 && ta0[lane] <= 2)
                {
                    if (t178[lane] == 5)
                    {
                        over = now - (unsigned int)te8[lane];
                        sixty4th = ((Obj422DD0 *)f364)->get();
                        if (ta0[lane] == 1)
                            t178[lane] = (over < (unsigned int)sixty4th >> 6) ? 3 : 4;
                        else if (ta0[lane] == 2)
                            t178[lane] = (over < (unsigned int)sixty4th >> 6) ? 2 : 3;
                        ((PanelSlot *)this)->m472fc0(t178[lane], 1, 1);
                        f35c->m472300(t130[lane], t178[lane], 0);
                    }
                    else
                    {
                        t178[lane] = 3;
                        ((PanelSlot *)this)->m472fc0(t178[lane], 1, 1);
                        f35c->m472300(t130[lane], t178[lane], 0);
                    }
                }
                else
                {
                    t178[lane] = 2;
                    ((PanelSlot *)this)->m472fc0(t178[lane], 1, 1);
                    f35c->m472300(t130[lane], t178[lane], 0);
                }
                ta0[lane] = 1;
                goto dec;
            }
            goto tail;
        }
    }

    te8[lane] += step;

tail:
    ((PanelSlot *)this)->m472fc0(t178[lane], 1, 1);
    f35c->m472300(t130[lane], t178[lane], 0);
dec:
    ta0[lane] -= 1;
}

/* CatchPlayer::m473c60 @0x473c60 - THE CATCH COMMIT, the twin of
   PlayerSlot::m42fd90 @0x42fd90 above and the last of the four the catch
   lane-expiry sink drives. 868 bytes, 251 instructions, frame 0x54,
   `ret 0x10`. It belongs in this TU: m473a60 @0x473a60 sits directly below it
   in the original and is already here.

   Same skeleton as the keys commit - getState fills a SlotState and returns a
   status, the clock's slot 22 gives the time, `flag` 1 is a press and anything
   else a release - but it is SHORTER, and the differences are the point.

     - IT HAS NO g_mixStyle SPLIT. Where the keys commit is two near-identical
       halves, this one is a single body and consults g_mixStyle at exactly one
       place: the KOOL promotion is `grade += (g_mixStyle != 7)`, so a 4 goes to
       5 in every style but 7, where it stays 4. The keys copy spells the same
       thing as a knock-down after the fact.
     - A SECOND MODE FLAG, g_catchJudging, splits both the press and the release,
       and in three of the four combinations the function RETURNS rather than
       falling into the shared tail. Only the press with the flag up and the
       release with it down reach the tail.
     - THE PRESS WITH THE FLAG DOWN needs only `grade >= 3` - no hold, no
       threshold test - and it re-reads the clock for its own offset rather
       than reusing the one taken at entry. That second `now()` is why the two
       calls are not CSE'd.
     - THE THRESHOLD IS THE PANEL'S +0x244, not the keys copy's +0x38.

   `pay` and `pay2` share the dead `grade` parameter's home in the original -
   cl overlaps them once grade has been copied to a register, the same thing
   the crossFade comment notes about its texture handles. They are separate
   locals here. */
// FUNCTION: EZ2AC_UNPACKED 0x00473c60
void CatchPlayer::m473c60(int lane, SlotSound *s, int grade, int flag)
{
    SlotState st;
    SlotState tail;
    int  status;
    int  pay;
    int  pay2;
    int  now;
    char drawShown;

    status = f360->sink->getState(s, &st);
    drawShown = 0;
    now = f360->clock->now();

    if (flag == 1)
    {
        if (status == 1 && st.width > 6)
        {
            if (g_catchJudging != 0)
            {
                if (grade < f35c->f244)
                    goto tail_;
                pay = 0;
                if (g_1b5f300[g_elemCount] >= 0 && st.kind != 6)
                {
                    if (grade == 4)
                        grade += (g_mixStyle != 7);
                    m473a60(lane, grade, st.width, st.start, st.kind);
                }
                else if (g_1b2efa4 == 1)
                {
                    if (grade == 4)
                        grade += (g_mixStyle != 7);
                    m473a60(lane, grade, st.width, st.start, st.kind);
                }
                else
                {
                    if (grade == 4)
                        grade = 5;
                    if (grade == 5)
                        m473a60(lane, 5, st.width, st.start, st.kind);
                    else
                        pay = 1;
                }
                st.offset = now - st.start;
                f360->sink->setState(s, &st);
                snd[lane].f0 = s->f0;
                snd[lane].id = s->id;
                if (g_1b5f300[g_elemCount] < 0 && pay == 0)
                    return;
                goto tail_;
            }

            /* The flag-down press: no threshold, no hold, and its own clock
               read rather than the one taken at entry. */
            if (grade < 3)
                goto tail_;
            f368->reseedLane(f360->tracks[id].items[lane].key);
            st.offset = f360->clock->now() - st.start;
            f360->sink->setState(s, &st);
            snd[lane].f0 = s->f0;
            snd[lane].id = s->id;
            return;
        }
        goto tail_;
    }

    if (g_catchJudging != 0)
    {
        if (snd[lane].id == -1)
            goto tail_;
        if (grade >= f35c->f244)
            ((SlotSampleSink *)f364)->getState(&snd[lane], &tail);
        snd[lane].f0 = -1;
        snd[lane].id = -1;
        return;
    }

    if (snd[lane].id != -1)
    {
        if (grade >= 3)
        {
            ((SlotSampleSink *)f364)->getState(&snd[lane], &tail);
            grade = (tail.f7 + grade + 1) / 2;
            drawShown = 1;
        }
        snd[lane].f0 = -1;
        snd[lane].id = -1;
    }

tail_:
    pay2 = 1;
    if ((grade == 2 || grade == 1) && s->id == -1)
        pay2 = 0;
    if (drawShown == 0 && grade == 5)
        f368->reseedLane(f360->tracks[id].items[lane].key);
    ((PanelSlot *)this)->m472fc0(grade, 1, pay2);
    f35c->m472300(lane, grade, flag);
}

/* PlayerRecord::m466ac0 @0x466ac0 - THE GF COMMIT, third of the family after
   PlayerSlot::m42fd90 and CatchPlayer::m473c60 above. 728 bytes, 202
   instructions, frame 0x58, `ret 0x10`.

   The simplest of the three. No g_mixStyle split, no second mode flag, and no
   grade promotion at all - a press does not start a hold through anything, it
   retimes the note and remembers the sound. What it has instead is a HANDLE
   GATE: both the press and the release ask the lane's own object (f1ec, slot
   getF0) whether the handle is live before touching it, and only the press
   releases it.

   Three things worth carrying to a port:

     - `pay` is only meaningful on the PRESS. The release path jumps past the
       `if (pay == 0) return;` entirely and reuses that register for the grade,
       so the early return belongs inside the press branch, not after the join.
     - THE HOLD AVERAGE IS THE SAME AS THE KEYS COPY - `(tail.f7 + grade + 1)
       / 2`, the head's grade averaged with the tail's - and it lands back in
       the `grade` parameter's own home, which the shared tail then reads.
     - THE TAIL'S grade == 5 ARM RELEASES THE HANDLE, but only when the release
       path did not already set drawShown.

   THE TAIL IS SHARED, NOT DUPLICATED - and that is measured, not assumed. The
   original's release arm jumps BACKWARD into it (0x466d07 and 0x466d1b both
   target 0x466bff), which is normally the signature of a tail written out per
   arm and merged by cl into the copy written first. Here it is not: writing it
   twice gives 286 instructions against the original's 202 because cl declines
   the merge outright. What the original actually has is the tail SPLIT around
   the release block - 0x466bff..0x466c65 before it and 0x466d20 onward after -
   which is cl's own block ordering and not something the source states.

   It reads `items[lane].handle` where the catch copy reads `items[lane].key` -
   the two owners' track entries are consulted for different fields. */
// FUNCTION: EZ2AC_UNPACKED 0x00466ac0
void PlayerRecord::m466ac0(int lane, SlotSound *s, int grade, int flag)
{
    SlotState st;
    SlotState tail;
    int  status;
    int  pay;
    int  pay2;
    int  now;
    int  h;
    char drawShown;

    pay = 1;
    status = f1e0->sink->getState(s, &st);
    drawShown = 0;
    now = f1e0->clock->now();

    if (flag == 1)
    {
        if (status == 1 && st.width > 6 && grade >= f1dc->f34)
        {
            h = f1e0->tracks[id].items[lane].handle;
            if (((Obj426CA0 *)f1ec)->getF0(h) == 1)
            {
                ((SlotClock *)f1e8)->reseedLane(h);
                drawShown = 1;
            }
            st.offset = now - st.start;
            f1e0->sink->setState(s, &st);
            snd[lane].f0 = s->f0;
            snd[lane].id = s->id;
            pay = 0;
        }
        if (g_1b5f2e0 != 0)
        {
            f1dc->m462760(lane, 2);
            if (status == 0 || st.width <= 6)
                ((Obj45F180 *)f1e0)->set(id, lane, 10);
        }
        if (pay == 0)
            return;
    }
    else
    {
    if (snd[lane].id != -1)
    {
        if (grade >= f1dc->f34)
        {
            h = f1e0->tracks[id].items[lane].handle;
            if (((Obj426CA0 *)f1ec)->getF0(h) == 1)
                drawShown = 1;
            ((SlotSampleSink *)f1e4)->getState(&snd[lane], &tail);
            grade = (tail.f7 + grade + 1) / 2;
        }
        snd[lane].f0 = -1;
        snd[lane].id = -1;
    }
    if (g_1b5f2e0 != 0)
        f1dc->m462760(lane, 1);
    }

    h = f1e0->tracks[id].items[lane].handle;
    pay2 = 1;
    if (drawShown == 0 && grade == 5)
    {
        if (((Obj426CA0 *)f1ec)->getF0(h) == 1)
            ((SlotClock *)f1e8)->reseedLane(h);
    }
    else if (grade < 3)
    {
        if ((grade == 2 || grade == 1) && s->id == -1)
            pay2 = 0;
        goto pay_;
    }
    /* ONE copy, reached from both arms - the original's single block at
       0x466d25. Written per-arm instead it is emitted twice, which the call
       multiset catches immediately (two m4662d0 against the original's one). */
    if (status == 1 && (f304 != 0 || f32c != 0) && st.skin != -1)
        m4662d0(st.skin, 1);
pay_:
    m465f60(grade, 1, pay2);
    f1dc->m4651a0(lane, grade, flag);
}
