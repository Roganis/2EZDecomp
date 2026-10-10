/* The eight unreconstructed slots of KEZPlayer's 35-slot vftable @0x4887d0 -
   the last referenced-but-unannotated functions the census found in the
   sound player. The vftable model itself stays the truncated prefix
   (extending it means virtualizing methods scattered across the placeholder
   views - see docs/DONE-CRITERIA notes); these bodies close the addresses.

   Their own TU: setEntryVolume/-Pan tail-jmp into setVolume/setPan
   (src/sound.cpp), and a visible callee would be inlined into the tail. */

#include <windows.h>
#include "sound.h"
#include "ksongplayer.h"
#include "ezchart.h"

/* The slots the master-volume push and the reseed reach through the table -
   a view, because the model does not declare them virtual. */
struct KEZPlayerSlots
{
    virtual void q0();  virtual void q1();  virtual void q2();
    virtual void q3();  virtual void q4();  virtual void q5();
    virtual void q6();  virtual void q7();  virtual void q8();
    virtual void q9();  virtual void q10(); virtual void q11();
    virtual void q12();
    virtual void setEntryVolume(int i, int v);   /* slot 13 -> +0x34 */
    virtual int  getEntryVolume(int i);          /* slot 14 -> +0x38 */
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21();
    virtual int  getClock();                     /* slot 22 -> +0x58 */
};

/* What an entry's `src` points into: only the group's record total. */
struct LaneGroupView
{
    char _pad0[0x48];
    int  f48;          /* 0x48 - the lane's own record count */
};

/* The sample as the player's root f04 sees it here: only the entry count. */
struct BoundSample
{
    char _pad0[0x98];
    int  count;        /* 0x98 */
};

/* Re-push every entry's stored volume through the per-entry setter after the
   master level at +0x726c changes (clamped to 0..0x7f); each setter call
   rescales against the new master. Slot 11. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f3b0
void KEZPlayer::setMasterVolume(int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7f)
        v = 0x7f;
    if (f726c == v)
        return;
    f726c = v;
    if (f04 != 0)
    {
        int i;
        for (i = 0; i < ((BoundSample *)f04)->count; i++)
        {
            ((KEZPlayerSlots *)this)->setEntryVolume(
                i, ((KEZPlayerSlots *)this)->getEntryVolume(i));
        }
    }
}

/* Slots 15, 19, 21 and 23 of the original table are all this compiled-out
   one-argument body - ICF folded them onto one address. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f430
void KEZPlayer::m40f430(int)
{
}

/* The other folded slots: each an empty or constant body the linker ICF'd
   onto one address with its twins elsewhere - slots 19/21/23 onto m40f430
   above, 12/16 onto findSoundObject (src/constants.cpp), 20/24 onto
   getZero (src/accessors.cpp) and 26 onto ret1_arg3 (src/constants.cpp). */
// FUNCTION: EZ2AC_UNPACKED 0x0040f430 folded
void KEZPlayer::v19(int)
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f430 folded
void KEZPlayer::v21(int)
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f430 folded
void KEZPlayer::v23(int)
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f420 folded
int KEZPlayer::v12(int, int)
{
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f420 folded
int KEZPlayer::v16(int, int)
{
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f440 folded
float KEZPlayer::v20()
{
    return 0.0f;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f440 folded
float KEZPlayer::v24()
{
    return 0.0f;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f480 folded
int KEZPlayer::v26(int, int, int)
{
    return 1;
}

/* The clock the reseed compares record times against: five ticks behind the
   queue clock, but never before tick 1. Slot 22. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f450
int KEZPlayer::getClock()
{
    int v = f7258 - 5;
    if (v < 0)
        v = 1;
    return v;
}

/* Store the entry's own level (clamped) and re-run the channel volume with
   the entry's bound id - the tail jump into setVolume is the return. Slot
   13. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fb10
void KEZPlayer::setEntryVolume(int i, int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7f)
        v = 0x7f;
    KEZPlayer::Entry *e = &entries[i];
    e->f1c = v;
    setVolume((SoundParam *)e, e->f10);
}

/* The pan twin, slot 17. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fb70
void KEZPlayer::setEntryPan(int i, int v)
{
    if (v < 0)
        v = 0;
    else if (v > 0x7f)
        v = 0x7f;
    KEZPlayer::Entry *e = &entries[i];
    e->f20 = v;
    setPan((SoundParam *)e, e->f14);
}

/* Force one entry's state word (0, 1 or 2 - anything else writes 0), with
   the index checked against the bound sample's count. Slot 30. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ff00
void KEZPlayer::setEntryState(int i, int mode)
{
    if (i < 0)
        return;
    if (i >= ((BoundSample *)f04)->count)
        return;
    /* The entry pointer is formed BEFORE the dispatch, and case 0 is its
       own arm (identical to the default) so the sub-0 test survives. */
    KEZPlayer::Entry *e = &entries[i];
    switch (mode)
    {
    case 0:
        e->f00 = 0;
        break;
    case 1:
        e->f00 = 1;
        break;
    case 2:
        e->f00 = 2;
        break;
    default:
        e->f00 = 0;
        break;
    }
}

/* Re-seed one lane's channel from the chart around the current clock: find
   the nearest event before and after now (kind-1 records may sit exactly on
   the cursor), pick whichever is closer, and re-run its bind, volume, pan
   and level through the ordinary appliers. Slot 32. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ffa0
void KEZPlayer::reseedLane(int i)
{
    KEZPlayer::Entry *e = &entries[i];

    if (e->f00 == 0)
        return;
    if (e->f2c < 0)
        return;
    if (e->f2c >= ((EzChart *)f04)->i94)
        return;

    int now = ((KEZPlayerSlots *)this)->getClock();
    int after = ((EzChart *)f04)->findAfter(i, e->f2c, 1);
    int before;
    Record28 *r;

    /* The backward search starts from the forward hit when there is one;
       only with no forward hit does a kind-1 record sitting exactly on the
       cursor stand in for the backward result. */
    if (after != -1)
    {
        before = ((EzChart *)f04)->findPrev(i, after, 1);
    }
    else if ((r = &((EzChart *)f04)->records[e->f2c])->byte4 == 1 &&
             r->byte5 == i)
    {
        before = e->f2c;
    }
    else
    {
        before = ((EzChart *)f04)->findPrev(i, e->f2c, 1);
    }

    if (before == -1)
    {
        if (after == -1)
            return;
        r = &((EzChart *)f04)->records[after];
    }
    else if (after == -1)
    {
        r = &((EzChart *)f04)->records[before];
    }
    else if (e->f00 != 1)
    {
        r = &((EzChart *)f04)->records[before];
    }
    else
    {
        /* The NEARER of the two: `after` only once the clock is past the
           midpoint (0x410090 `ja` keeps it), otherwise `before`. The C had
           this inverted until 2026-09-28 (port REQUESTS.md). */
        r = &((EzChart *)f04)->records[after];
        Record28 *rb = &((EzChart *)f04)->records[before];
        if ((unsigned int)now <= (r->d0 + rb->d0) / 2u)
            r = rb;
    }

    SoundEvent *ev = (SoundEvent *)r;
    m40fbf0((SoundParam *)e, ev->v.n.w0e, ev->v.n.b12);
    setVolume((SoundParam *)e, ev->v.n.b10);
    setPan((SoundParam *)e, ev->v.n.b11);
    m40f520((SoundParam *)e);
}

/* The walk is a `while (r != 0)` with the due-time test at its top, and
   the song-end exit jumps straight to the clock advance - the shape that
   lays the song-end block out last, as the original does. And the entry
   records the cursor BEFORE bumping its count: cl only hoists a load over
   a store off the same base, so the order is visible (40.3% -> zoo-exact,
   2026-10-03). */
/* One turn of the event queue, slot 34: under the idle lock, and only while
   the gate is up, walk the due records - each one goes through applyEvent
   on its lane's entry, bumps the entry's play count (the count against the
   lane group's own total is what keeps bit 0 of the flags meaning "more to
   come") - park the next due time in +0x725c, and advance the clock. When
   the walk runs off the end of the chart the run is over: drop the playing
   flag and the gate, and stop every live target. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fd70
void KEZPlayer::drainQueue()
{
    WaitForSingleObject(ev0.m_h, -1);
    if (WaitForSingleObject(ev1.m_h, 0) == 0)
    {
        if (f7258 >= (unsigned int)f725c)
        {
            Record28 *r;

            /* An exhausted queue skips even the clock advance. */
            if (f7278 >= ((EzChart *)f04)->i94)
                goto out;
            r = &((EzChart *)f04)->records[f7278];
            while (r != 0)
            {
                if (r->d0 > (unsigned int)f7258)
                    break;
                {
                    int lane = r->byte5;
                    KEZPlayer::Entry *e = &entries[lane];

                    if (e->f00 != 0)
                    {
                        applyEvent((SoundParam *)e, (SoundEvent *)r);
                        e->f2c = f7278;
                        e->f08 += 1;
                        if ((unsigned int)e->f08 >=
                            (unsigned int)((LaneGroupView *)e->src)->f48)
                            e->flags &= ~1;
                        else
                            e->flags |= 1;
                    }
                }
                f7278 += 1;
                if (f7278 >= ((EzChart *)f04)->i94)
                {
                    /* The run is over: no next due time is parked. */
                    f7274 = 0;
                    ResetEvent(ev1.m_h);
                    m40fd20();
                    goto advance;
                }
                r = &((EzChart *)f04)->records[f7278];
            }
            f725c = ((EzChart *)f04)->records[f7278].d0;
        }
advance:
        f7258 += 1;
    }
out:
    SetEvent(ev0.m_h);
}
