/* The two main-game directors' slot-release pass, @0x420ec0 and @0x45def0 -
 * twins with different bases.
 *
 * Each walks TWO 0x1c4-byte slot entries and releases the object each holds,
 * if the entry's flag is set. The 0x1c4 stride is the same slot-block size the
 * .gds loader uses (wip/maingame-gds.cpp), so these are the same entries.
 *
 * The fields are reached through a LOCAL VIEW rather than by adding them to
 * EZ2DJMainGameDirector / GFMainGameDirector: carving evidenced fields out of
 * that class's `_pad` broke four already-matched methods of it once already
 * (see CLAUDE.md), and a view costs no code.
 *
 * Their own TU because both callers are matched functions in src/maingame.cpp -
 * a visible definition risks cl inlining it and deleting the original's `call`.
 */

#include "ez2acapp.h"
#include "ezscreen.h"
#include "players.h"   /* SlotSampleSink, SlotSound, SlotState */
#include "scorekeeper.h"
#include "ez2ac.h"   /* Obj40B970::getField438 - the same class as Obj40B2E0 */

/* src/mixer.cpp - one mixer line's level (setValue @0x412a10). */
struct MixerControl { void setValue(unsigned long value); };

/* The Catch director's variant: a 0x1bc stride and the flag 0x128 back. */
struct CatchSlotEntry
{
    int   flag;                    /* +0x000 */
    char  _pad4[0x128 - 4];
    void *obj;                     /* +0x128 */
    char  _pad12c[0x1bc - 0x12c];
};

struct CatchSlotView
{
    char           _pad0[0x38];
    int            guard;          /* 0x038 */
    char           _pad3c[0x34c - 0x3c];
    CatchSlotEntry slots[2];       /* 0x34c - the OBJ of entry 0 lands at 0x474,
                                      which is what the original anchors on */
};

/* One slot entry as this pass sees it: a flag, and 0x130 later the object. */
struct GameSlotEntry
{
    int   flag;                    /* +0x000 */
    char  _pad4[0x130 - 4];
    void *obj;                     /* +0x130 */
    char  _pad134[0x1c4 - 0x134];
};

/* The two directors seen from here: the guard at +0x1c and the slot pair. */
struct EZ2DJSlotView
{
    char          _pad0[0x1c];
    int           guard;           /* 0x01c */
    char          _pad20[0x380 - 0x20];
    GameSlotEntry slots[2];        /* 0x380 */
};

struct GFSlotView
{
    char          _pad0[0x1c];
    int           guard;           /* 0x01c */
    char          _pad20[0x340 - 0x20];
    GameSlotEntry slots[2];        /* 0x340 */
};

/* Each director's slots hold a different object type, so each has its own
   release entry point (src/stubs.cpp). */
struct EZ2DJSlotObject { void release(); };   /* 0x42e360 */
/* The GF slot object is a PlayerRecord; its release is the stage-end fold
   PlayerRecord::m465c70 (src/gfstagesetup.cpp). */

// FUNCTION: EZ2AC_UNPACKED 0x00420ec0
void EZ2DJMainGameDirector::m420ec0()
{
    EZ2DJSlotView *v = (EZ2DJSlotView *)this;
    int i;

    if (v->guard != 0)
    {
        return;
    }

    for (i = 0; i < 2; i++)
    {
        if (v->slots[i].flag != 0)
        {
            ((EZ2DJSlotObject *)v->slots[i].obj)->release();
            v->slots[i].obj = 0;
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0045def0
void GFMainGameDirector::m45def0()
{
    GFSlotView *v = (GFSlotView *)this;
    int i;

    if (v->guard != 0)
    {
        return;
    }

    for (i = 0; i < 2; i++)
    {
        if (v->slots[i].flag != 0)
        {
            ((PlayerRecord *)v->slots[i].obj)->m465c70();
            v->slots[i].obj = 0;
        }
    }
}

/* The two directors' timed-start hooks, @0x41b460 and @0x46b760 - twins again,
 * different offsets. Forward the current time to the Obj40B2E0 they own if it
 * reports ready, then latch the time and mark themselves started. */
// FUNCTION: EZ2AC_UNPACKED 0x0041b460
void MainTimed::m41b460(unsigned int now)
{
    if (m_guard != 0)
    {
        if (((Obj40B970 *)&m_sub)->getField438() != 0)
        {
            m_sub.play(now);
        }
        m_start = now;
        m_started = 1;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b760
void CatchTimed::m46b760(unsigned int now)
{
    if (m_guard != 0)
    {
        if (((Obj40B970 *)&m_sub)->getField438() != 0)
        {
            m_sub.play(now);
        }
        m_start = now;
        m_started = 1;
    }
}

/* The main game's mixer curve @0x420360: one 0..0x100 slider drives two mixer
 * lines in opposite directions. Below the midpoint each line sits at its floor;
 * above it, it climbs linearly - and the two slopes differ, which is why one
 * multiply comes out as a lea chain (11420) and the other as a real `imul`
 * (0x2b71). Writing both as plain `x * K` lets cl pick, per the cookbook. */
// FUNCTION: EZ2AC_UNPACKED 0x00420360
void EZ2DJMainGameDirector::m420360(int level)
{
    int inv = 0x100 - level;
    int a;
    int b;

    if (inv <= 0x80)
    {
        a = 0x5938;
    }
    else
    {
        a = ((inv - 0x80) * 11420) / 128 + 0x5938;
    }

    if (level <= 0x80)
    {
        b = 0xaaaa;
    }
    else
    {
        b = ((level - 0x80) * 0x2b71) / 128 + 0xaaaa;
    }

    if (mixerB != 0)
    {
        mixerB->setValue(a);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(b);
    }
}

/* The Catch director's third variant of the same pass @0x46c7e0. Same two-entry
   walk, different stride (0x1bc) and offsets - and what it calls on each object
   is ScoreKeeper::commit @0x472d10, so these slots hold the per-player score
   keepers and this is the round's commit rather than a release. */
// FUNCTION: EZ2AC_UNPACKED 0x0046c7e0
void CatchMainGameDirector::m46c7e0()
{
    CatchSlotView *v = (CatchSlotView *)this;
    int i;

    if (v->guard != 0)
    {
        return;
    }

    for (i = 0; i < 2; i++)
    {
        if (v->slots[i].flag != 0)
        {
            ((ScoreKeeper *)v->slots[i].obj)->commit();
            v->slots[i].obj = 0;
        }
    }
}

/* Only the slot index the node carries. */
struct SlotEntryOwner
{
    char _pad0[8];
    int  f8;                   /* 0x08 */
};

/* EZ2DJMainGameDirector::m421600 @0x421600 - chain three key ranges together
   across one slot's entry table.

   The node names the slot, the slot's 0x1c4-byte entry block is indexed twice
   at a 0x10 stride from its +0x18, and the two values that come back are
   strung into three calls - (x, 21), (y, x), (21, y) - so 21 is a sentinel at
   both ends of the run. */
// FUNCTION: EZ2AC_UNPACKED 0x00421600
void EZ2DJMainGameDirector::m421600(ListNode *n, int a, int b, int c, int d)
{
    /* A named base for the slot's entry block: the original builds it with one
       `lea` and indexes off it twice, where the folded form adds `this` into
       each displacement instead. */
    int *p = &f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int  x = p[6 + a * 4];
    int  y = p[6 + b * 4];

    m421510(x, 21, c, d);
    m421510(y, x, c, d);
    m421510(21, y, c, d);
}

/* GFMainGameDirector::m45e630 @0x45e630 - m421600's twin on the GF director's
   own slot table at +0x340.

   It calls @0x421510 with its own `this`, so that one function serves both
   directors; the cast is what says so rather than pretending the method
   belongs to this class as well. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e630
void GFMainGameDirector::m45e630(ListNode *n, int a, int b, int c, int d)
{
    int *p = &f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int  x = p[6 + a * 4];
    int  y = p[6 + b * 4];

    ((EZ2DJMainGameDirector *)this)->m421510(x, 21, c, d);
    ((EZ2DJMainGameDirector *)this)->m421510(y, x, c, d);
    ((EZ2DJMainGameDirector *)this)->m421510(21, y, c, d);
}

/* Re-stamp every hit of one lane with another lane's id, from wherever the
   sink starts it up to the time limit. A no-op when the two are the same, or
   when the lane has nothing playing.

   Like laneBusy below, this one is in this TU because its callers depend on
   what it leaves alone. */
// FUNCTION: EZ2AC_UNPACKED 0x00421510
void EZ2DJMainGameDirector::m421510(int from, int to, int c, int d)
{
    SlotSound s;
    SlotState st;

    if (from == to)
        return;

    if (sink->begin(from, c, 0, 1, &st, &s) != 1)
        return;

    /* A do-while with a break in it, not a while: the head test is an EXIT
       and the bottom test is the one that jumps back, so cl must not be
       allowed to rotate a condition into the tail. */
    do
    {
        if ((unsigned int)st.start > (unsigned int)d)
            break;

        st.lane = (unsigned char)to;
        sink->setState(&s, &st);
    }
    while (sink->next(1, &st, &s) == 1);
}

/* The catch director's copy of it, on its own sink at +0x164. */
// FUNCTION: EZ2AC_UNPACKED 0x0046cd30
void CatchMainGameDirector::m46cd30(int from, int to, int c, int d)
{
    SlotSound s;
    SlotState st;

    if (from == to)
        return;

    if (sink->begin(from, c, 0, 1, &st, &s) != 1)
        return;

    /* A do-while with a break in it, not a while: the head test is an EXIT
       and the bottom test is the one that jumps back, so cl must not be
       allowed to rotate a condition into the tail. */
    do
    {
        if ((unsigned int)st.start > (unsigned int)d)
            break;

        st.lane = (unsigned char)to;
        sink->setState(&s, &st);
    }
    while (sink->next(1, &st, &s) == 1);
}

/* Is this lane free? Keys 15..25 are the ones the mode can use, and of those
   only four answer no; everything else answers no as well.

   The gate picks between two IDENTICAL switches - the original compiled the
   same eleven-key table twice, once per arm, and only cross-jumped the two
   return blocks. Written as one switch after the gate cl folds the arms
   together and emits a single table, so both copies have to be spelled out.

   It lives in this TU, ahead of its caller, ON PURPOSE: it touches only eax
   and ecx, and pickTwoLanes below keeps its second draw in edx across two
   calls to it. cl will only believe that of a callee whose body it can see.
*/
// FUNCTION: EZ2AC_UNPACKED 0x00421490
int EZ2DJMainGameDirector::laneBusy(int key)
{
    if (f7c4 != 0 && f164 == 0)
    {
        switch (key)
        {
        case 15: case 17: case 23: case 25:
            return 1;
        default:
            return 0;
        }
    }

    switch (key)
    {
    case 15: case 17: case 23: case 25:
        return 1;
    default:
        return 0;
    }
}

/* Pick two DIFFERENT lanes at random, both below `range` and neither already
   busy, and give up after 32 tries with `base` for both. `check` of zero takes
   the first pair that passes the three cheap tests.

   The scratch director's @0x45e6b0 is the same idea with the two range tests
   dropped (wip/gf-picktwolanes.cpp), and that difference is what flips cl's
   register budget between the two: with `range` read four times it stays in a
   register and the table is spilled, which is what the original does here. */
// FUNCTION: EZ2AC_UNPACKED 0x00421680
void EZ2DJMainGameDirector::pickTwoLanes(ListNode *n, int base, int range,
                                         int check, int *outA, int *outB)
{
    int *p = &f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int  k;

    for (k = 0; k < 32; k++)
    {
        int a = base + rand() % range;
        int b = base + rand() % range;

        if (a < range && b < range && a != b)
        {
            if (check == 0)
            {
                *outA = a;
                *outB = b;
                return;
            }

            if (laneBusy(p[(b + 1) * 4]) == 0 && laneBusy(p[(a + 1) * 4]) == 0)
            {
                *outA = a;
                *outB = b;
                return;
            }
        }
    }

    *outA = base;
    *outB = base;
}

/* The scratch director's own lane test. Same shape as the EZ2DJ one next door,
   and the same reason for living in this TU - but with an extra bank of four
   keys when the mode is on, and only one gate rather than two, which is why cl
   hoists the argument load above the branch here and duplicates it there. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e460
int GFMainGameDirector::laneBusy(int key)
{
    if (f768 != 0)
    {
        switch (key)
        {
        case 6: case 7: case 8: case 9:
        case 15: case 17: case 23: case 25:
            return 1;
        default:
            return 0;
        }
    }

    switch (key)
    {
    case 15: case 17: case 23: case 25:
        return 1;
    default:
        return 0;
    }
}

/* Pick two DIFFERENT lanes at random out of [base, base+range), rejecting a
   pair whose entries are already busy, and give up after 32 tries with base
   for both. `check` of zero takes the first distinct pair it draws. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e6b0
void GFMainGameDirector::pickTwoLanes(ListNode *n, int base, int range,
                                      int check, int *outA, int *outB)
{
    int *p = &f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int  k;

    for (k = 0; k < 32; k++)
    {
        int a = base + rand() % range;
        int b = base + rand() % range;

        if (a != b)
        {
            if (check == 0)
            {
                *outA = a;
                *outB = b;
                return;
            }

            if (laneBusy(p[(b + 1) * 4]) == 0 && laneBusy(p[(a + 1) * 4]) == 0)
            {
                *outA = a;
                *outB = b;
                return;
            }
        }
    }

    *outA = base;
    *outB = base;
}

/* Reverse the run's lane order: swap each lane with its mirror, working from
   the list's first entry to the middle. Every swap goes through lane 21 as the
   temporary, which is the same three-call idiom m421600 uses.

   The half-way bound is recomputed at the bottom of every iteration because
   the source spells it out in the loop condition, and m421510 can move the
   entries under it. */
// FUNCTION: EZ2AC_UNPACKED 0x00421df0
void EZ2DJMainGameDirector::reverseLanes(ListNode *n, int c, int d)
{
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int i;

    for (i = t->first; i < t->first + (t->end - t->first + 1) / 2; i++)
    {
        int h1 = t->items[i].handle;
        int h2 = t->items[t->first + t->end - i - 1].handle;

        m421510(h1, 21, c, d);
        m421510(h2, h1, c, d);
        m421510(21, h2, c, d);
    }

    sink->commit();
}

/* The scratch director's copy of reverseLanes @0x421df0 above - same body,
   same shared swap helper (it calls EZ2DJ's m421510 with its own `this`),
   over its own list at +0x340. */
// FUNCTION: EZ2AC_UNPACKED 0x0045ed90
void GFMainGameDirector::m45ed90(ListNode *n, int c, int d)
{
    TrackList *t = (TrackList *)&f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int i;

    for (i = t->first; i < t->first + (t->end - t->first + 1) / 2; i++)
    {
        int h1 = t->items[i].handle;
        int h2 = t->items[t->first + t->end - i - 1].handle;

        ((EZ2DJMainGameDirector *)this)->m421510(h1, 21, c, d);
        ((EZ2DJMainGameDirector *)this)->m421510(h2, h1, c, d);
        ((EZ2DJMainGameDirector *)this)->m421510(21, h2, c, d);
    }

    sink->commit();
}

/* Rotate the run's playable lanes by one: collect the entries whose key is one
   of the ten the mode uses, then walk the collection backwards swapping each
   neighbouring pair, which carries the last lane to the front. The swap is the
   same three-call idiom through lane 21. */
// FUNCTION: EZ2AC_UNPACKED 0x00422970
void EZ2DJMainGameDirector::rotateLanes(ListNode *n, int c, int d)
{
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int buf[10];
    int cnt = 0;
    int i;
    int m;

    for (i = t->first; i < t->end; i++)
    {
        int k = t->items[i].key;

        if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
            k == 18 || k == 19 || k == 20 || k == 21 || k == 22)
        {
            buf[cnt] = t->items[i].handle;
            cnt++;
        }
    }

    for (m = cnt - 2; m >= 0; m--)
    {
        int y = buf[m + 1];
        int x = buf[m];

        m421510(x, 21, c, d);
        m421510(y, x, c, d);
        m421510(21, y, c, d);
    }

    sink->commit();
}

/* The scratch director's copy, over its own slot table - and like its two
   neighbours it borrows the EZ2DJ director's @0x421510 with its own `this`. */
/* Rotate the run's playable lanes by one: collect the entries whose key is one
   of the ten the mode uses, then walk the collection backwards swapping each
   neighbouring pair, which carries the last lane to the front. The swap is the
   same three-call idiom through lane 21. */
// FUNCTION: EZ2AC_UNPACKED 0x0045ee70
void GFMainGameDirector::rotateLanes(ListNode *n, int c, int d)
{
    TrackList *t = (TrackList *)&f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int buf[10];
    int cnt = 0;
    int i;
    int m;

    for (i = t->first; i < t->end; i++)
    {
        int k = t->items[i].key;

        if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
            k == 18 || k == 19 || k == 20 || k == 21 || k == 22)
        {
            buf[cnt] = t->items[i].handle;
            cnt++;
        }
    }

    for (m = cnt - 2; m >= 0; m--)
    {
        int y = buf[m + 1];
        int x = buf[m];

        ((EZ2DJMainGameDirector *)this)->m421510(x, 21, c, d);
        ((EZ2DJMainGameDirector *)this)->m421510(y, x, c, d);
        ((EZ2DJMainGameDirector *)this)->m421510(21, y, c, d);
    }

    sink->commit();
}

/* Rotate the pickable lanes of one slot through the spare 21: the key banks
   are collected in one pass (the 10..14/6/7 and 15/16 keys into the first,
   18..22/8/9/23/24 into the second, the 23/24 pair twice), then paired first
   bank forward against second bank backward, each pair spun with the same
   (x,21)(y,x)(21,y) chain m421600 uses. The sink's slot 23 is kicked once at
   the end whether anything moved or not.

   85.38% (was 65.13% before the m422410 lessons: kb declared before ka,
   the counter zeroed ahead of the guard, guarded do-while) stopped on one
   register rotation inside the collect loop - the original walks bank B
   through ebp with the down-counter in edx and the handle temp in esi.
   Neither a block-scope walking pointer nor naming the handle in each arm
   moved it. What does is ORDER: `pb = keysB;` set FIRST in the guarded
   block, ahead of the `e` / `left` declarations rather than after them.
   Found by scratchpad/perm (2026-10-08). */
// FUNCTION: EZ2AC_UNPACKED 0x004226f0
void SlotOwner::m4226f0(ListNode *n, int a, int b)
{
    int keysA[16];
    int keysB[16];
    int *pb;
    TrackList *t = &tracks[n->f8];
    int i = t->first;
    int nA = 0;
    int j;

    if (i < t->end)
    {
        pb = keysB;
        TrackEntry *e = &t->items[i];
        int left = t->end - i;

        do
        {
            int k = e->key;

            if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
                k == 6 || k == 7)
            {
                keysA[nA] = e->handle;
                nA++;
            }
            if (k == 15 || k == 16)
            {
                keysA[nA] = e->handle;
                nA++;
            }
            if (k == 18 || k == 19 || k == 20 || k == 21 || k == 22 ||
                k == 8 || k == 9 || k == 23 || k == 24)
            {
                *pb = e->handle;
                pb++;
            }
            if (k == 23 || k == 24)
            {
                *pb = e->handle;
                pb++;
            }
            e++;
            left--;
        }
        while (left != 0);
    }

    j = 0;
    if (nA > 0)
    {
        pb = &keysB[nA - 1];

        do
        {
            int kb = *pb;
            int ka = keysA[j];

            ((EZ2DJMainGameDirector *)this)->m421510(ka, 21, a, b);
            ((EZ2DJMainGameDirector *)this)->m421510(kb, ka, a, b);
            ((EZ2DJMainGameDirector *)this)->m421510(21, kb, a, b);
            pb--;
            j++;
        }
        while (j < nA);
    }
    sink->commit();
}

/* Find the first hold on lane h inside [a, limit]: open the lane's state
   with the sink, walk hits while they start in range, and hand back a copy
   of the first one wider than 6. Lives beside the GF director's helpers but
   serves both shuffles - the EZ2DJ time-slicer borrows it with a cast, the
   same arrangement as m421510 in the other direction. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e5a0
int GFMainGameDirector::m45e5a0(int h, unsigned int a, unsigned int limit,
                                SlotState *out)
{
    SlotSound snd;
    SlotState st;
    int r = sink->begin(h, a, 0, 1, &st, &snd);

    while (r != 0)
    {
        if ((unsigned int)st.start > limit)
            break;
        if (st.width > 6)
        {
            if (out != 0)
                *out = st;
            return 1;
        }
        r = sink->next(1, &st, &snd);
    }
    return 0;
}

/* And the catch director's copy of it, on its own sink at +0x164: the same
   fifty instructions, differing only in that member offset. */
// FUNCTION: EZ2AC_UNPACKED 0x0046ce70
int CatchMainGameDirector::m46ce70(int h, unsigned int a, unsigned int limit,
                                SlotState *out)
{
    SlotSound snd;
    SlotState st;
    int r = sink->begin(h, a, 0, 1, &st, &snd);

    while (r != 0)
    {
        if ((unsigned int)st.start > limit)
            break;
        if (st.width > 6)
        {
            if (out != 0)
                *out = st;
            return 1;
        }
        r = sink->next(1, &st, &snd);
    }
    return 0;
}

/* The lane-bank reversal @0x4221b0: collect the two key banks (one test
   apiece here, unlike m4226f0's doubled entries), mirror-reverse bank A
   through the spare 21, then bank B the same way - unless bank B holds nine
   or more lanes, which get a fixed scramble driven by two index tables.

   97.06%, parked on one scheduling coin: in each mirror loop's SECOND call
   the original evaluates the descending-side load at its push site through
   eax alone, where our build hoists the two-instruction chain above the
   pushes into ecx. Same instruction count, pure placement. Volatile pins
   reproduce the placement but force the deref into a fresh register (never
   the original's eax->eax) and re-roll the fixed-arm's registers, netting
   worse; a named descending index, a named x, and an explicit pointer all
   give the identical hoisted schedule. Likely the program-global allocator
   seed - re-score after new TUs land. */
// FUNCTION: EZ2AC_UNPACKED 0x004221b0
void EZ2DJMainGameDirector::m4221b0(ListNode *n, int c, int d)
{
    int keysB[16];
    int keysA[16];
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int i = t->first;
    int nA = 0;
    int nB = 0;
    int m;

    if (i < t->end)
    {
        TrackEntry *e = &t->items[i];
        int left = t->end - i;

        do
        {
            int k = e->key;

            if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
                k == 6 || k == 7 || k == 15 || k == 17)
            {
                keysA[nA] = e->handle;
                nA++;
            }
            if (k == 18 || k == 19 || k == 20 || k == 21 || k == 22 ||
                k == 8 || k == 9 || k == 23 || k == 25)
            {
                keysB[nB] = e->handle;
                nB++;
            }
            e++;
            left--;
        }
        while (left != 0);
    }

    m = 0;
    {
        int half = nA / 2;

        if (half > 0)
        {
        do
        {
            m421510(keysA[m], 21, c, d);
            m421510(keysA[nA - 1 - m], keysA[m], c, d);
            m421510(21, keysA[nA - 1 - m], c, d);
            m++;
        }
        while (m < half);
        }
    }

    m = 0;
    {
        int idx2[4] = { 3, 4, 5, 0 };
        int idx1[4] = { 1, 6, 8, 7 };

        if (nB >= 9)
        {
            do
            {
                int y = keysB[idx1[m]];
                int x = keysB[idx2[m]];

                m421510(x, 21, c, d);
                m421510(y, x, c, d);
                m421510(21, y, c, d);
                m++;
            }
            while (m < 4);
        }
        else
        {
            int half = nB / 2;

            if (half > 0)
            {
                do
                {
                    m421510(keysB[m], 21, c, d);
                    m421510(keysB[nB - 1 - m], keysB[m], c, d);
                    m421510(21, keysB[nB - 1 - m], c, d);
                    m++;
                }
                while (m < half);
            }
        }
    }
    sink->commit();
}

extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */
extern int g_1b2ef38;     /* (src/modeselectctor.cpp) */
extern int g_1b2ebbc;     /* (src/gamestate.cpp) */

/* The slice length the time-sliced shuffle steps by, at 0x34c of the EZ2DJ
   director - a view for the same reason as the guard below. */
struct ShuffleStepView
{
    char _pad0[0x34c];
    int  measure;              /* 0x34c */
};

/* The two fields the shuffle respects: an armed flag and the one lane index
   that must not move. A view - the class's own header stays untouched. */
struct ShuffleGuardView
{
    char _pad0[0x7bc];
    int  armed;                /* 0x7bc */
    int  keep;                 /* 0x7c0 */
};

/* Sixteen successful random pair-swaps of one slot's lanes through the spare
   21. A draw is retried - without counting - when it lands on one lane, on
   the guarded lane, on a busy lane (when the caller asks for the check), or
   on a pair sharing one handle. Modes 4/8 under their flag draw from the
   whole list but keep the second draw in the first draw's half; the other
   modes draw end-2 wide when their flag is up. */
// FUNCTION: EZ2AC_UNPACKED 0x00421c30
void EZ2DJMainGameDirector::m421c30(ListNode *n, int check, int c, int d)
{
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int end = t->end;
    int first = t->first;
    int attempt;

    for (attempt = 0; attempt < 16; attempt++)
    {
        int a;
        int b;

        if (g_modeIndex != 4 && g_modeIndex != 8)
        {
            if (g_1b2ebbc != 0)
            {
                int r = end - 2;

                a = rand() % r + first;
                b = rand() % r + first;
            }
            else
            {
                a = rand() % end + first;
                b = rand() % end + first;
            }
        }
        else if (g_1b2ef38 == 0)
        {
            a = rand() % end + first;
            b = rand() % end + first;
        }
        else
        {
            int half;

            first = 0;
            a = rand() % end;
            half = end / 2;
            b = rand() % end / 2;
            if (a >= half)
                b += half;
        }

        if (a == b)
        {
            attempt--;
            continue;
        }
        if (((ShuffleGuardView *)this)->armed != 0 &&
            (((ShuffleGuardView *)this)->keep == a ||
             ((ShuffleGuardView *)this)->keep == b))
        {
            attempt--;
            continue;
        }
        if (check != 0)
        {
            if (laneBusy(t->items[b].key) != 0 ||
                laneBusy(t->items[a].key) != 0)
            {
                attempt--;
                continue;
            }
        }
        {
            int x = t->items[a].handle;
            int y = t->items[b].handle;

            if (x == y)
            {
                attempt--;
                continue;
            }
            m421510(x, 21, c, d);
            m421510(y, x, c, d);
            m421510(21, y, c, d);
        }
    }
    sink->commit();
}

/* The time-sliced shuffle: in every eight-measure slice of [start, limit),
   six attempts at a random pair-swap through the spare 21. A pair is refused
   when the two lanes are one, when either lane is inside a hold that covers
   the slice's start, or (on request) when either lane is busy - and when a
   chosen lane carries a hold inside the slice, the slice's window is
   stretched to that hold's last tick so the swap never cuts one in half.

   81.56%, parked on a single frame-slot inversion: the original packs r1 at
   [esp+0x14] (below the track-list pointer at 0x20), ours puts the pointer
   at 0x14 and r1 at 0x2c, and every other defect follows - the shifted
   t/B/end/first quartet, the width spill the original's low r1 home forces
   (mov [esp+0x14],ecx / lea ecx / reload), and the rotated call-argument
   registers. Declaration order (five permutations), scoping r1 inner, and
   reusing r1/r2 for the widths all leave the packer's choice unchanged;
   assigning cur after the declarations made it worse (71%). The packer
   appears to rank by access count, which is circular with the spill it
   should produce - the seed family. Re-score after new TUs land. */
// FUNCTION: EZ2AC_UNPACKED 0x00421750
void EZ2DJMainGameDirector::m421750(ListNode *n, unsigned int start,
                                    unsigned int limit, int check)
{
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int step = ((ShuffleStepView *)this)->measure * 8;
    int first = t->first;
    int end = t->end;
    unsigned int cur = start + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (start < limit)
    {
        do
        {
            tries = 6;
            do
            {
                pickTwoLanes(n, first, end, check, &A, &B);
                {
                    int x = t->items[A].handle;
                    int y = t->items[B].handle;

                    if (x != y &&
                        !(sink->begin(x, start, 3, 1, &stA, &sndA) != 0 &&
                          stA.width >= 6 &&
                          start >= (unsigned int)stA.start &&
                          start <= (unsigned int)stA.start + stA.width + 8) &&
                        !(sink->begin(y, start, 3, 1, &stB, &sndB) != 0 &&
                          stB.width >= 6 &&
                          start >= (unsigned int)stB.start &&
                          start <= (unsigned int)stB.start + stB.width + 8))
                    {
                        unsigned int r2;

                        r1 = ((GFMainGameDirector *)this)->m45e5a0(x, start, cur, &st3);
                        r2 = ((GFMainGameDirector *)this)->m45e5a0(y, start, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto reject;
                            cur = stop;
                        }
                        m421510(x, 21, start, cur);
                        m421510(y, x, start, cur);
                        m421510(21, y, start, cur);
                    }
                }
reject:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            start += step;
            if (cur > limit)
                cur = limit;
        }
        while (start < limit);
    }
    sink->commit();
}

/* The time-sliced shuffle: in every quarter-measure slice of [start, limit),
   six attempts at a random pair-swap through the spare 21. A pair is refused
   when the two lanes are one, when either lane is inside a hold that covers
   the slice's start, or (on request) when either lane is busy - and when a
   chosen lane carries a hold inside the slice, the slice's window is
   stretched to that hold's last tick so the swap never cuts one in half.

   The byte-for-byte twin of m421750 above from the step computation on -
   including the same r1-at-[esp+0x14] frame packing ours inverts, so it
   parks at the same score for the same reason. One source, two constants. */
// FUNCTION: EZ2AC_UNPACKED 0x004219c0
void EZ2DJMainGameDirector::m4219c0(ListNode *n, unsigned int start,
                                    unsigned int limit, int check)
{
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int step = ((ShuffleStepView *)this)->measure / 4;
    int first = t->first;
    int end = t->end;
    unsigned int cur = start + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (start < limit)
    {
        do
        {
            tries = 6;
            do
            {
                pickTwoLanes(n, first, end, check, &A, &B);
                {
                    int x = t->items[A].handle;
                    int y = t->items[B].handle;

                    if (x != y &&
                        !(sink->begin(x, start, 3, 1, &stA, &sndA) != 0 &&
                          stA.width >= 6 &&
                          start >= (unsigned int)stA.start &&
                          start <= (unsigned int)stA.start + stA.width + 8) &&
                        !(sink->begin(y, start, 3, 1, &stB, &sndB) != 0 &&
                          stB.width >= 6 &&
                          start >= (unsigned int)stB.start &&
                          start <= (unsigned int)stB.start + stB.width + 8))
                    {
                        unsigned int r2;

                        r1 = ((GFMainGameDirector *)this)->m45e5a0(x, start, cur, &st3);
                        r2 = ((GFMainGameDirector *)this)->m45e5a0(y, start, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto reject2;
                            cur = stop;
                        }
                        m421510(x, 21, start, cur);
                        m421510(y, x, start, cur);
                        m421510(21, y, start, cur);
                    }
                }
reject2:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            start += step;
            if (cur > limit)
                cur = limit;
        }
        while (start < limit);
    }
    sink->commit();
}

/* GFMainGameDirector::m45ec70 @0x45ec70 - the scratch director's plain
   shuffle, the one that is not time-sliced: six successful random pair-swaps
   of one slot's lanes through the spare 21, over the whole run at once.

   EZ2DJMainGameDirector::m421c30 @0x421c30 below is the template, and this is
   the simpler half of it: sixteen attempts there against six here, and where
   that one picks its draw width from the mode and the two scroll flags - and
   additionally refuses the guarded lane - this draws the full [first, end)
   twice with no guard at all. The retry idiom is the same and so is the
   three-call swap; only the callee is EZ2DJ's m421510, with this director's
   own `this`, as everywhere else in this family.

   A draw is retried WITHOUT counting - `attempt--; continue;` - when it lands
   on one lane, on a busy lane (only when the caller asks for the check), or
   on a pair sharing one handle. */
// FUNCTION: EZ2AC_UNPACKED 0x0045ec70
void GFMainGameDirector::m45ec70(ListNode *n, int check, int c, int d)
{
    TrackList *t = (TrackList *)&f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int first = t->first;
    int end = t->end;
    int attempt;

    for (attempt = 0; attempt < 6; attempt++)
    {
        int a = rand() % end + first;
        int b = rand() % end + first;

        if (a == b)
        {
            attempt--;
            continue;
        }
        if (check != 0)
        {
            if (laneBusy(t->items[b].key) != 0 ||
                laneBusy(t->items[a].key) != 0)
            {
                attempt--;
                continue;
            }
        }
        {
            int x = t->items[a].handle;
            int y = t->items[b].handle;

            if (x == y)
            {
                attempt--;
                continue;
            }
            ((EZ2DJMainGameDirector *)this)->m421510(x, 21, c, d);
            ((EZ2DJMainGameDirector *)this)->m421510(y, x, c, d);
            ((EZ2DJMainGameDirector *)this)->m421510(21, y, c, d);
        }
    }
    sink->commit();
}

/* CatchMainGameDirector::m46df00 @0x46df00 and ::m46e170 @0x46e170 - the
   Ez2Catch copies of the two time-sliced shuffles, eight measures and a
   quarter-measure per slice. Found by scratchpad/twins.py at 54% against the
   scratch pair below; a heavier edit than that ratio's usual, because the
   Catch list is shaped differently:

     - the per-slot stride is 0x6f ints, not 0x71, which is exactly
       sizeof(CatchTrackList) - the array is at +0x34c, the measure at +0x334
       and the sink at +0x164;
     - the list has NO `first`. The walk is 0 .. count-1, so the picker gets a
       base of zero and a span of `count - 1` where the other two directors
       pass first and end;
     - the entry value handed to the sink is `key` at +0, where the keys and
       scratch entries use `handle` at +8;
     - the swap goes through this class's own m46cd30 rather than EZ2DJ's
       m421510, and the hold-extender is m46ce70.

   The cast on the picker's first argument is not decorative: m46dea0
   @0x46dea0 is already matched in src/maingame.cpp, where its first parameter
   is typed `int` because the callee ignores it. The original passes this
   function's own node pointer there, so the call site casts rather than
   changing a matched function's signature.

   AND THE REJECTION RULE IS THE THIRD DISTINCT ONE. EZ2DJ refuses a lane on
   `w >= 6 && start >= s && start <= s + w + 8`, the scratch director on the
   same without the `+ 8`, and this one on `w > 6 && start < s + w` - a
   strictly-greater width threshold AND no lower bound at all. Written out as
   the original tests it; see finding 14 in docs/PORT-DELTAS.md, which also
   notes the lower bound's absence may make the position test vacuous. */
// FUNCTION: EZ2AC_UNPACKED 0x0046df00
void CatchMainGameDirector::m46df00(ListNode *n, unsigned int start,
                                unsigned int limit, int check)
{
    CatchTrackList *t =
        (CatchTrackList *)&f34c[0x6f * ((SlotEntryOwner *)n)->f8];
    int step = f334 * 8;
    int span = t->count - 1;
    unsigned int pos = start;
    unsigned int cur = pos + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (pos < limit)
    {
        do
        {
            tries = 6;
            do
            {
                m46dea0((int)n, 0, span, check, &A, &B);
                {
                    int x = t->items[A].key;
                    int y = t->items[B].key;

                    if (x != y &&
                        !(sink->begin(x, pos, 3, 1, &stA, &sndA) != 0 &&
                          stA.width > 6 &&
                          pos < (unsigned int)stA.start + stA.width) &&
                        !(sink->begin(y, pos, 3, 1, &stB, &sndB) != 0 &&
                          stB.width > 6 &&
                          pos < (unsigned int)stB.start + stB.width))
                    {
                        unsigned int r2;

                        r1 = m46ce70(x, pos, cur, &st3);
                        r2 = m46ce70(y, pos, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto rejectc1;
                            cur = stop;
                        }
                        m46cd30(x, 21, pos, cur);
                        m46cd30(y, x, pos, cur);
                        m46cd30(21, y, pos, cur);
                    }
                }
rejectc1:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            pos += step;
            if (cur > limit)
                cur = limit;
        }
        while (pos < limit);
    }
    ((CatchSlotOwner *)this)->m46cdc0(t, start, limit);
    sink->commit();
}

/* m46df00's twin from the step computation on - a quarter of a measure per
   slice. One source, two constants. */
// FUNCTION: EZ2AC_UNPACKED 0x0046e170
void CatchMainGameDirector::m46e170(ListNode *n, unsigned int start,
                                unsigned int limit, int check)
{
    CatchTrackList *t =
        (CatchTrackList *)&f34c[0x6f * ((SlotEntryOwner *)n)->f8];
    int step = f334 / 4;
    int span = t->count - 1;
    unsigned int pos = start;
    unsigned int cur = pos + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (pos < limit)
    {
        do
        {
            tries = 6;
            do
            {
                m46dea0((int)n, 0, span, check, &A, &B);
                {
                    int x = t->items[A].key;
                    int y = t->items[B].key;

                    if (x != y &&
                        !(sink->begin(x, pos, 3, 1, &stA, &sndA) != 0 &&
                          stA.width > 6 &&
                          pos < (unsigned int)stA.start + stA.width) &&
                        !(sink->begin(y, pos, 3, 1, &stB, &sndB) != 0 &&
                          stB.width > 6 &&
                          pos < (unsigned int)stB.start + stB.width))
                    {
                        unsigned int r2;

                        r1 = m46ce70(x, pos, cur, &st3);
                        r2 = m46ce70(y, pos, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto rejectc2;
                            cur = stop;
                        }
                        m46cd30(x, 21, pos, cur);
                        m46cd30(y, x, pos, cur);
                        m46cd30(21, y, pos, cur);
                    }
                }
rejectc2:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            pos += step;
            if (cur > limit)
                cur = limit;
        }
        while (pos < limit);
    }
    ((CatchSlotOwner *)this)->m46cdc0(t, start, limit);
    sink->commit();
}

/* GFMainGameDirector::m45e4e0 @0x45e4e0 - the tail call the two shuffles below
   make once their slicing is done: walk the track list and shorten every hit
   wider than 0x1e by 0x12, stopping each lane's walk once the hits pass the
   window's end.

   CatchSlotOwner::m46cdc0 @0x46cdc0 in src/playerslot.cpp is the same function
   on the catch owner - found by scratchpad/twins.py at 65%, and the whole of
   the difference is that this one walks a keys-shaped TrackList from `first`
   to `end` where that one walks a catch list from 0 to `count`.

   The list is re-read from its parameter home at the bottom of the loop
   (`mov ecx,[ebp+0x8]; cmp ebx,[ecx+0x8]`) rather than kept in a register,
   because setState and next can both move the entries under it - so `t->end`
   belongs in the loop condition and must not be hoisted.

   67.16% -> 100% on one change, and it is the induction-anchoring rule: written
   as `const TrackEntry *e = &t->items[i]` with `e->handle`, cl builds the
   address as `(i + 1) << 4` plus the list plus 8, because items starts at 0x10
   and the field adds another 8. The original folds both into one displacement
   (`mov ecx,ebx; shl ecx,4; lea edi,[ecx+eax+0x18]`), which is what an explicit
   `const int *h = &t->items[i].handle` walked by `h += 4` produces. The
   register assignment came along with it: with the shift in ecx the list stays
   in eax and `this` survives in ecx until after the pushes, where our version
   had copied it to esi up front and every vtable-call register followed. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e4e0
void GFMainGameDirector::m45e4e0(TrackList *t, unsigned int start,
                                 unsigned int limit)
{
    SlotSound snd;
    SlotState st;
    int i;

    i = t->first;
    if (i < t->end)
    {
        const int *h = &t->items[i].handle;

        do
        {
            int r = sink->begin(*h, start, 0, 1, &st, &snd);

            while (r == 1)
            {
                if ((unsigned int)st.start > limit)
                    break;

                if (st.width >= 0x1e)
                {
                    st.width = (unsigned short)(st.width - 0x12);
                    sink->setState(&snd, &st);
                }

                r = sink->next(1, &st, &snd);
            }

            i++;
            h += 4;
        }
        while (i < t->end);
    }
}

/* GFMainGameDirector::m45e770 @0x45e770 and ::m45e9f0 @0x45e9f0 - the scratch
   director's copies of the two time-sliced shuffles above, eight measures per
   slice and a quarter-measure per slice respectively. Found by
   scratchpad/twins.py against the EZ2DJ pair (67%, the whole remainder being
   register renames), and they are an EDIT of that source rather than a
   reconstruction - but three of the differences are real, and one of them is a
   behaviour delta:

     - the track list is at +0x340 and the measure at +0x318, against the
       EZ2DJ director's +0x380 and +0x34c;
     - the hold-covers-the-slice-start test has NO `+ 8` slack. EZ2DJ refuses a
       lane when the slice start falls within eight ticks PAST the hold's end
       (`lea edx,[ecx+eax*1+0x8]`); the scratch director refuses it only while
       the start is still inside the hold (`add ecx,eax`). So a scratch lane
       whose hold ended in the last eight ticks is available to be swapped and
       an EZ2DJ one is not - one more of the family in docs/PORT-DELTAS.md;
     - the tail calls m45e4e0 with the track list and the window, before the
       commit both versions end on. Note the window it passes is the ORIGINAL
       one: the original reloads both parameter homes (`mov ecx,[esp+0xf4]` /
       `mov edi,[esp+0xf0]`) rather than using the values the loop advanced,
       which is why the loop walks a separate `pos` here where the EZ2DJ
       template advances `start` itself. The early bail jumps straight into
       that call, so it runs even when the window was empty.

   82.12%, and the whole defect is the same packer permutation the EZ2DJ pair
   is parked on - which makes FOUR independent functions on it. It is a
   permutation, not a size error: the /GS aggregates above the scalars land at
   identical offsets on both sides (0x38, 0x40, 0x48, 0x98, 0xac, 0xc0, 0xd4),
   so both frames reserve the same ten-slot block and only assign it
   differently. The original's is cur 0x10, list 0x14, HOLE 0x18, stop 0x1c,
   tries 0x20, B 0x24, end 0x28, first 0x2c, A 0x30, step 0x34 - note B nine
   slots below A with end and first interleaved between them, and one slot
   never touched at all. Ours fills 0x18 and shifts stop/tries/end/first/A/B
   down by four, which costs the register rotation around the three m421510
   pushes and nothing else; every instruction is present and in order.
   Declaring r2 at function scope (to reserve a tenth slot) and swapping A/B's
   declaration order both leave it unchanged, as the five permutations tried on
   the EZ2DJ pair did. Nor did reconstructing m45e4e0 above, which these two
   call and which was a STUB when they were first measured: the cookbook's
   "reconstruct the CALLEE first" rule predicts a stub's fictional clobber set
   distorts its caller, and here it did not move either function by a hundredth
   of a percent. That rule is about register allocation and scheduling AROUND
   the call; the stack packer's slot assignment is decided independently of it. An array for A/B is ruled out by the original itself -
   the two `lea`s are 0xc apart, so they are separate locals.

   Worth noting what this pair adds to the EZ2DJ analysis: the scratch original
   packs the list at 0x14 where the EZ2DJ original homes r1 there, so the
   ORIGINAL compiler produced two different layouts from what is essentially
   one source. That points at register pressure - the `+ 8` and the extra tail
   call - rather than at anything the source shape says, which is why no
   declaration order was ever going to fix either one. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e770
void GFMainGameDirector::m45e770(ListNode *n, unsigned int start,
                                 unsigned int limit, int check)
{
    TrackList *t = (TrackList *)&f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int step = f318 * 8;
    int first = t->first;
    int end = t->end;
    unsigned int pos = start;
    unsigned int cur = pos + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (pos < limit)
    {
        do
        {
            tries = 6;
            do
            {
                pickTwoLanes(n, first, end, check, &A, &B);
                {
                    int x = t->items[A].handle;
                    int y = t->items[B].handle;

                    if (x != y &&
                        !(sink->begin(x, pos, 3, 1, &stA, &sndA) != 0 &&
                          stA.width >= 6 &&
                          pos >= (unsigned int)stA.start &&
                          pos <= (unsigned int)stA.start + stA.width) &&
                        !(sink->begin(y, pos, 3, 1, &stB, &sndB) != 0 &&
                          stB.width >= 6 &&
                          pos >= (unsigned int)stB.start &&
                          pos <= (unsigned int)stB.start + stB.width))
                    {
                        unsigned int r2;

                        r1 = m45e5a0(x, pos, cur, &st3);
                        r2 = m45e5a0(y, pos, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto rejectg1;
                            cur = stop;
                        }
                        ((EZ2DJMainGameDirector *)this)->m421510(x, 21, pos, cur);
                        ((EZ2DJMainGameDirector *)this)->m421510(y, x, pos, cur);
                        ((EZ2DJMainGameDirector *)this)->m421510(21, y, pos, cur);
                    }
                }
rejectg1:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            pos += step;
            if (cur > limit)
                cur = limit;
        }
        while (pos < limit);
    }
    m45e4e0(t, start, limit);
    sink->commit();
}

/* m45e770's twin from the step computation on - a quarter of a measure per
   slice instead of eight measures. One source, two constants; see the note
   above for everything else. */
// FUNCTION: EZ2AC_UNPACKED 0x0045e9f0
void GFMainGameDirector::m45e9f0(ListNode *n, unsigned int start,
                                 unsigned int limit, int check)
{
    TrackList *t = (TrackList *)&f340[0x71 * ((SlotEntryOwner *)n)->f8];
    int step = f318 / 4;
    int first = t->first;
    int end = t->end;
    unsigned int pos = start;
    unsigned int cur = pos + step;
    int tries;
    int A;
    int B;
    unsigned int r1;
    unsigned int stop;
    SlotSound sndA;
    SlotSound sndB;
    SlotState stB;
    SlotState st3;
    SlotState stA;
    SlotState st4;

    if (cur > limit)
        cur = limit;
    if (pos < limit)
    {
        do
        {
            tries = 6;
            do
            {
                pickTwoLanes(n, first, end, check, &A, &B);
                {
                    int x = t->items[A].handle;
                    int y = t->items[B].handle;

                    if (x != y &&
                        !(sink->begin(x, pos, 3, 1, &stA, &sndA) != 0 &&
                          stA.width >= 6 &&
                          pos >= (unsigned int)stA.start &&
                          pos <= (unsigned int)stA.start + stA.width) &&
                        !(sink->begin(y, pos, 3, 1, &stB, &sndB) != 0 &&
                          stB.width >= 6 &&
                          pos >= (unsigned int)stB.start &&
                          pos <= (unsigned int)stB.start + stB.width))
                    {
                        unsigned int r2;

                        r1 = m45e5a0(x, pos, cur, &st3);
                        r2 = m45e5a0(y, pos, cur, &st4);
                        if (r1 != 0 || r2 != 0)
                        {
                            unsigned int wa = 0;

                            if (r1 != 0)
                            {
                                r1 = st3.width;
                                if (r1 > 0)
                                {
                                    stop = r1 + st3.start - 1;
                                    wa = r1;
                                }
                            }
                            if (r2 != 0)
                            {
                                r2 = st4.width;
                                if (r2 > wa)
                                    stop = r2 + st4.start - 1;
                            }
                            if (stop > limit)
                                goto rejectg2;
                            cur = stop;
                        }
                        ((EZ2DJMainGameDirector *)this)->m421510(x, 21, pos, cur);
                        ((EZ2DJMainGameDirector *)this)->m421510(y, x, pos, cur);
                        ((EZ2DJMainGameDirector *)this)->m421510(21, y, pos, cur);
                    }
                }
rejectg2:
                tries--;
            }
            while (tries != 0);
            cur = cur + step;
            pos += step;
            if (cur > limit)
                cur = limit;
        }
        while (pos < limit);
    }
    m45e4e0(t, start, limit);
    sink->commit();
}

/* The full-mirror variant @0x421ed0: reverse the whole run in place (the
   reverseLanes loop, restated), then collect the five main keys of each
   side and reverse the banks - bank B by the fixed (0,2)(6,3)(5,4)
   scramble when it holds seven or more lanes, mirrored otherwise.

   98.40%: the only residue is m4221b0's mirror-call2 coin at both mirror
   loops - the original derefs the walking pointer at the push site into
   eax, ours hoists the deref above the pushes into ecx. Same instruction
   count; the seed family. */
// FUNCTION: EZ2AC_UNPACKED 0x00421ed0
void EZ2DJMainGameDirector::m421ed0(ListNode *n, int c, int d)
{
    int keysB[16];
    int keysA[16];
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int i = t->first;
    int nA;
    int nB;
    int m;

    for (; i < t->first + (t->end - t->first + 1) / 2; i++)
    {
        int h1 = t->items[i].handle;
        int h2 = t->items[t->first + t->end - i - 1].handle;

        m421510(h1, 21, c, d);
        m421510(h2, h1, c, d);
        m421510(21, h2, c, d);
    }

    i = t->first;
    nA = 0;
    nB = 0;
    if (i < t->end)
    {
        TrackEntry *e = &t->items[i];
        int left = t->end - i;

        do
        {
            int k = e->key;

            if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14)
            {
                keysA[nA] = e->handle;
                nA++;
            }
            if (k == 18 || k == 19 || k == 20 || k == 21 || k == 22)
            {
                keysB[nB] = e->handle;
                nB++;
            }
            e++;
            left--;
        }
        while (left != 0);
    }

    m = 0;
    {
        int half = nA / 2;

        if (half > 0)
        {
        do
        {
            m421510(keysA[m], 21, c, d);
            m421510(keysA[nA - 1 - m], keysA[m], c, d);
            m421510(21, keysA[nA - 1 - m], c, d);
            m++;
        }
        while (m < half);
        }
    }

    m = 0;
    if (nB < 7)
    {
        int half = nB / 2;

        if (half > 0)
        {
            do
            {
                m421510(keysB[m], 21, c, d);
                m421510(keysB[nB - 1 - m], keysB[m], c, d);
                m421510(21, keysB[nB - 1 - m], c, d);
                m++;
            }
            while (m < half);
        }
    }
    else
    {
        int idx2[3] = { 2, 3, 4 };
        int idx1[3] = { 0, 6, 5 };

        do
        {
            int y = keysB[idx1[m]];
            int x = keysB[idx2[m]];

            m421510(x, 21, c, d);
            m421510(y, x, c, d);
            m421510(21, y, c, d);
            m++;
        }
        while (m < 3);
    }
    sink->commit();
}

/* The mode-aware bank swap @0x422410: collect the seven keys of each side,
   then either pair bank A against bank B in opposite order (the radio
   family, 4/5/8/9 - the m4226f0 pairing) or reverse each bank in place
   (m421ed0's mirror-or-scramble tail, same seven-lane threshold).

   97.55%: the residue is the same two mirror-call2 deref coins as m421ed0
   (the original derefs the walking pointer at the push site, ours hoists
   it); every descending-side spelling gives the hoisted schedule. */
// FUNCTION: EZ2AC_UNPACKED 0x00422410
void EZ2DJMainGameDirector::m422410(ListNode *n, int c, int d)
{
    int keysB[16];
    int keysA[16];
    TrackList *t = (TrackList *)&f380[0x71 * ((SlotEntryOwner *)n)->f8];
    int i = t->first;
    int nA = 0;
    int nB = 0;
    int m;
    int *pb;

    if (i < t->end)
    {
        TrackEntry *e = &t->items[i];
        int left = t->end - i;

        do
        {
            int k = e->key;

            if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
                k == 6 || k == 7)
            {
                keysA[nA] = e->handle;
                nA++;
            }
            if (k == 18 || k == 19 || k == 20 || k == 21 || k == 22 ||
                k == 8 || k == 9)
            {
                keysB[nB] = e->handle;
                nB++;
            }
            e++;
            left--;
        }
        while (left != 0);
    }

    if (g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 8 && g_modeIndex != 9)
    {
        m = 0;
        {
            int half = nA / 2;

            if (half > 0)
            {
            do
            {
                m421510(keysA[m], 21, c, d);
                m421510(keysA[nA - 1 - m], keysA[m], c, d);
                m421510(21, keysA[nA - 1 - m], c, d);
                m++;
            }
            while (m < half);
            }
        }

        m = 0;
        if (nB < 7)
        {
            int half = nB / 2;

            if (half > 0)
            {
                do
                {
                    m421510(keysB[m], 21, c, d);
                    m421510(keysB[nB - 1 - m], keysB[m], c, d);
                    m421510(21, keysB[nB - 1 - m], c, d);
                    m++;
                }
                while (m < half);
            }
        }
        else
        {
            int idx2[3] = { 2, 3, 4 };
            int idx1[3] = { 0, 6, 5 };

            do
            {
                int y = keysB[idx1[m]];
                int x = keysB[idx2[m]];

                m421510(x, 21, c, d);
                m421510(y, x, c, d);
                m421510(21, y, c, d);
                m++;
            }
            while (m < 3);
        }
    }
    else
    {
        m = 0;
        if (nA > 0)
        {
            pb = &keysB[nA - 1];

            do
            {
                int kb = *pb;
                int ka = keysA[m];

                m421510(ka, 21, c, d);
                m421510(kb, ka, c, d);
                m421510(21, kb, c, d);
                pb--;
                m++;
            }
            while (m < nA);
        }
    }
    sink->commit();
}

/* CatchMainGameDirector::m46e3e0 @0x46e3e0 - the catch game's copy of
   GFMainGameDirector::m45ec70 above: six attempted swaps of two randomly
   picked entries in one slot's table, each strung through the sentinel lane
   21 as (x,21)(21 is free)(y,x)(21,y), and one commit at the end.

   Three differences, all of them the catch list's own:

     - the stride is 0x1bc rather than 0x1c4, so the dword step is 0x6f;
     - the two values come off the entry's FIRST word rather than its handle
       at +8, which is what include/players.h already records about the catch
       walkers;
     - and there is NO busy check. The keys and scratch copies skip an
       attempt when either picked lane is still sounding; this one takes its
       second argument and never reads it.

   The random range is `count - 1` with no `first` added, where the other two
   pick within [first, end). */
// FUNCTION: EZ2AC_UNPACKED 0x0046e3e0
void CatchMainGameDirector::m46e3e0(ListNode *n, int a, int b, int c)
{
    int *p = &f34c[0x6f * ((SlotEntryOwner *)n)->f8];
    int  limit = p[1] - 1;
    int  i;

    for (i = 0; i < 6; i++)
    {
        int x = rand() % limit;
        int y = rand() % limit;

        if (x == y)
        {
            i--;
            continue;
        }

        {
            int u = p[4 + x * 4];
            int v = p[4 + y * 4];

            if (u == v)
            {
                i--;
                continue;
            }

            m46cd30(u, 0x15, b, c);
            m46cd30(v, u, b, c);
            m46cd30(0x15, v, b, c);
        }
    }

    sink->commit();
}
