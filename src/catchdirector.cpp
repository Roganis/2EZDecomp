#include "ezscreen.h"

/* timeGetTime WITHOUT dllimport, so the call goes through the thunk @0x4145f0 -
   the form this function uses. countListB is the list-B walker in
   src/nodelists.cpp. */
extern "C" DWORD __stdcall timeGetTime(void);
extern int countListA();
extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */
extern int g_elemCount;  /* 0x1b2e718 (src/loops.cpp) */
extern int countListB();
extern int countListC();   /* 0x4184a0 - the scratch director's counter */

/* Bring Ez2Catch's director up. Anything pending at +0xca8 is told to stop and
   forgotten first, and both times its ready byte is clear the whole start is
   abandoned - including the second look, which only happens with more than one
   Catch player on the list. Then the per-frame consumer is given the current
   time, the notifiable is poked, the start time is stamped, and the embedded song
   pump is restarted (a tail call, so Obj414750::restart has to stay in its own
   TU).

   Note the second `pending->ready` test can only be reached with pending null if
   the first block ran, which would fault; the original is written that way and
   evidently relies on the two paths being exclusive in practice. */
// FUNCTION: EZ2AC_UNPACKED 0x0046bfa0
void CatchMainGameDirector::start()
{
    if (pending != 0)
    {
        if (pending->ready == 0)
        {
            return;
        }

        pending->v1(0);
        pending = 0;
    }

    if (countListB() > 1)
    {
        if (pending->ready == 0)
        {
            return;
        }
    }

    if (timed != 0)
    {
        timed->m46b760(timeGetTime());
    }

    if (notify != 0)
    {
        notify->v8();
    }

    started = timeGetTime();
    pump.restart();
}

/* The main game's director start-up: the same skeleton as CatchMainGameDirector's, plus
   the mode. The pending object is only stopped when the mode is 1 or there is a
   single player on the list, and the field at +0xda4 is cleared only in mode 4
   with an element count other than two - everything else leaves it at -1. */
// FUNCTION: EZ2AC_UNPACKED 0x0041fa10
void EZ2DJMainGameDirector::start()
{
    if (pending != 0)
    {
        if (pending->ready == 0)
        {
            return;
        }

        if (g_modeIndex == 1 || countListA() == 1)
        {
            pending->v1(0);
            pending = 0;
        }
    }

    if (countListA() > 1 && g_modeIndex != 1)
    {
        if (pending->ready == 0)
        {
            return;
        }
    }

    if (timed != 0)
    {
        timed->m41b460(timeGetTime());
    }

    if (notify != 0)
    {
        notify->v8();
    }

    if (g_modeIndex == 4 && g_elemCount != 2)
    {
        fda4 = 0;
    }
    else
    {
        fda4 = -1;
    }

    ready = 1;
    started = timeGetTime();
    pump.restart();
}

/* The scratch director's copy, over its own members - twins.py had it at 76%
   against the one above. Two REAL differences beyond the offsets: the first
   guard drops the `g_modeIndex == 1` alternative, and the second counts
   list B rather than list A and does not re-test the mode. Its `pending`
   shares the slot the constructor fills with createBattleMode(), so it is
   reached through a cast rather than by retyping the member. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d100
void GFMainGameDirector::start()
{
    if (f32c != 0)
    {
        if (((DirectorPending *)f32c)->ready == 0)
        {
            return;
        }

        if (countListA() == 1)
        {
            ((DirectorPending *)f32c)->v1(0);
            f32c = 0;
        }
    }

    if (countListC() > 1)
    {
        if (((DirectorPending *)f32c)->ready == 0)
        {
            return;
        }
    }

    if (timed != 0)
    {
        ((MainTimed *)timed)->m41b460(timeGetTime());
    }

    if (notify != 0)
    {
        notify->v8();
    }

    if (g_modeIndex == 4 && g_elemCount != 2)
    {
        fd38 = 0;
    }
    else
    {
        fd38 = -1;
    }

    b330 = 1;
    started = timeGetTime();
    pump.restart();
}

extern EzObjectInfo g_info_4aeb88;   /* 0x4aeb88 (src/classinfo.cpp) */

/* CatchMainGameDirector's new-plus-ctor factory. */
// FUNCTION: EZ2AC_UNPACKED 0x0046f290
CatchMainGameDirector *createCatchDirector()
{
    g_objectRecord = &g_info_4aeb88;
    return new CatchMainGameDirector;
}


#include "players.h"   /* ListNode, SlotSampleSink */

/* CatchMainGameDirector::m46cf00 @0x46cf00 - mirror the player's lane list
   over a time range.

   Each pair of lanes is exchanged the only way the note mover allows: three
   calls through SCRATCH LANE 0x15, since m46cd30 @0x46cd30 moves notes from
   one lane to another and would otherwise overwrite. Low goes to the scratch
   lane, high goes to low, and the scratch lane goes to high. Once every pair
   has moved, the sink's slot 23 commits the batch.

   The list lives inside f34c and is INDEXED rather than carved out, which is
   the convention m46ca30 @0x46ca30 already set - splitting the array would
   cost the constructor's memset its `sizeof`. Each player's record is 111
   ints; within it index 1 is the count and the lanes start at index 4 with a
   four-int stride. Index 1 is the lane count the .gds declares (13), and the
   mirror deliberately leaves the LAST entry where it is - which is why the
   mirror of lane i is at count-i-2 and the loop runs count/2 times. (An
   earlier reading here called the count "one more than the number of
   lanes"; the port checked it against the .gds, CATCH-REVIEW.md S5.)

   Both the count and the record base are re-read every iteration - the note
   mover can touch them - so neither is hoisted; only the low lane's walking
   pointer is explicit, set up inside the guard the way the original's
   `lea edx,[ebp+0x10]` past the `jle` is. */
// FUNCTION: EZ2AC_UNPACKED 0x0046cf00
void CatchMainGameDirector::m46cf00(ListNode *n, int a, int b)
{
    int *rec = &f34c[n->f8 * 111];
    int *p;
    int  i;

    i = 0;
    if (rec[1] / 2 > 0)
    {
        p = &rec[4];

        do
        {
            int lo = *p;
            int hi = rec[(rec[1] - i - 1) * 4];

            m46cd30(lo, 0x15, a, b);
            m46cd30(hi, lo, a, b);
            m46cd30(0x15, hi, a, b);

            i++;
            p += 4;
        }
        while (i < rec[1] / 2);
    }

    sink->commit();
}

/* CatchMainGameDirector::m46cfc0 @0x46cfc0 - mirror only the PLAYABLE lanes.

   m46cf00 @0x46cf00 mirrors the lane list as it stands; this one filters
   first. It walks the same records and copies out the lane of every entry
   whose gds KEY id is one of fourteen playable ones, then mirrors THAT list end
   for end through scratch lane 0x15, three moves per pair, exactly as its
   sibling does. So a chart with unplayable lanes mixed in still mirrors
   sensibly instead of shuffling them into play.

   The two together settle the record layout: the entries start at index 2 of
   each 111-int record with a four-int stride and hold the gds key id at +0
   (the input code the lane is bound to) and the lane at +2; index 1 is the
   declared count and the walk stops one entry short of it (`sub eax,1`), the
   same last entry m46cf00 leaves alone.

   The fourteen keys are a single `||` chain, which is what puts every test
   as a forward `je` to one block; separate `if`s would inline the first
   one's body. Their order is the original's - 0xa..0xe, then 0x12..0x16,
   then 6..9 last. */
// FUNCTION: EZ2AC_UNPACKED 0x0046cfc0
void CatchMainGameDirector::m46cfc0(ListNode *n, int a, int b)
{
    int  lanes[16];
    int *rec = &f34c[n->f8 * 111];
    int  count = rec[1] - 1;
    int  out = 0;
    int  j;

    if (count > 0)
    {
        int *p = &rec[2];
        int  k = count;

        do
        {
            int key = *p;

            if (key == 0xa || key == 0xb || key == 0xc || key == 0xd ||
                key == 0xe || key == 0x12 || key == 0x13 || key == 0x14 ||
                key == 0x15 || key == 0x16 || key == 6 || key == 7 ||
                key == 8 || key == 9)
            {
                lanes[out] = p[2];
                out++;
            }

            p += 4;
        }
        while (--k != 0);
    }

    j = 0;
    if (out / 2 > 0)
    {
        int *hi = &lanes[out - 1];

        do
        {
            /* Declared HIGH first: cl hands the two lanes ebx and edi in
               declaration order, and with `lo` first the pair comes out
               swapped and nothing else changes (91.59%). Writing lanes[j]
               inline at both use sites instead is equally exact - the
               indexed-load naming rule, seen from the other end. */
            int h = *hi;
            int lo = lanes[j];

            m46cd30(lo, 0x15, a, b);
            m46cd30(h, lo, a, b);
            m46cd30(0x15, h, a, b);

            j++;
            hi--;
        }
        while (j < out / 2);
    }

    sink->commit();
}

/* CatchMainGameDirector::m46d0f0 @0x46d0f0 - m46cfc0's twin over the OTHER
   keys. Same collect-then-mirror, same three moves through scratch
   lane 0x15; it differs in exactly two things, and a masked diff of the two
   listings shows nothing else.

   The key list is the six the other one does NOT take - 0xf, 0x10, 0x17,
   0x18, 0x11, 0x19, in that order - so between them the two functions
   partition the lanes and mirror each group among itself. And the entry
   count is rec[1] - 2 rather than - 1, so this pass stops one entry short
   of the other's.
*/
// FUNCTION: EZ2AC_UNPACKED 0x0046d0f0
void CatchMainGameDirector::m46d0f0(ListNode *n, int a, int b)
{
    int  lanes[16];
    int *rec = &f34c[n->f8 * 111];
    int  count = rec[1] - 2;
    int  out = 0;
    int  j;

    if (count > 0)
    {
        int *p = &rec[2];
        int  k = count;

        do
        {
            int key = *p;

            if (key == 0xf || key == 0x10 || key == 0x17 ||
                key == 0x18 || key == 0x11 || key == 0x19)
            {
                lanes[out] = p[2];
                out++;
            }

            p += 4;
        }
        while (--k != 0);
    }

    j = 0;
    if (out / 2 > 0)
    {
        int *hi = &lanes[out - 1];

        do
        {
            /* Declared HIGH first: cl hands the two lanes ebx and edi in
               declaration order, and with `lo` first the pair comes out
               swapped and nothing else changes (91.59%). Writing lanes[j]
               inline at both use sites instead is equally exact - the
               indexed-load naming rule, seen from the other end. */
            int h = *hi;
            int lo = lanes[j];

            m46cd30(lo, 0x15, a, b);
            m46cd30(h, lo, a, b);
            m46cd30(0x15, h, a, b);

            j++;
            hi--;
        }
        while (j < out / 2);
    }

    sink->commit();
}
