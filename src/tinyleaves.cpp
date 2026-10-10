/* Small leaves whose callers are already reconstructed.

   They live in their own TU rather than beside their callers: an empty STUB
   that cl can see gets inlined and deletes the surrounding code (CLAUDE.md),
   and the same is true of a real body small enough for the inliner - the
   call sites in src/kezplayer.cpp and src/gfresultupdate.cpp must keep their
   `call`. */

#include <windows.h>
#include <string.h>

#include "players.h"      /* ListNode, nextListA */
#include "directors.h"    /* GFResultDirector */
#include "ksongplayer.h"  /* KEZPlayer */
#include "sound.h"        /* SoundParam, IVolTarget */
#include "obj40d4e0.h"
#include "obj40eac0.h"
#include "obj405f80.h"
#include "ksound.h"
#include "ezchart.h"      /* g_ezChartClassInfo */
#include "obj414750.h"    /* Obj414750::removePlayer, addPlayer */

extern const float k120;   /* 0x4887c4 (src/ezchart.cpp) - 120 BPM */
#include "ez2acapp.h"
#include "icom.h"
#include "obj402860.h"

/* Drain list A. The result is discarded - the walk itself is the point,
   since nextListA @0x4182e0 advances the caller's cursor and each visited
   node's own refresh has already happened by the time this runs. */
// FUNCTION: EZ2AC_UNPACKED 0x00467900 folded
void GFResultDirector::m467900()
{
    ListNode *cur = 0;

    while (nextListA(&cur) != 0)
        ;
}

/* ResultDirector's own copy - the two classes' drains are identical and the
   original's linker folded them onto this one address, which is why the keys
   tick @0x451550 calls 0x467900 directly rather than having its own. Both
   copies carry the body and a `folded` annotation, since the winner here is a
   real function rather than a stub. */
// FUNCTION: EZ2AC_UNPACKED 0x00467900 folded
void ResultDirector::m467900()
{
    ListNode *cur = 0;

    while (nextListA(&cur) != 0)
        ;
}

/* RETRIGGER the channel's target: slot 5 then slot 4 with a zero. The
   target is a KSample (Obj40D4E0, vtable @0x488320 - slot 4 is v4 @0x40d640,
   slot 5 is v5 @0x40d6a0, src/tinyleaves.cpp below): slot 5 is STOP - it
   tells the DirectSound buffer Stop (buffer slot 18) then
   SetCurrentPosition(0) (slot 13); slot 4 is PLAY - it latches the loop bit
   from its argument and calls the buffer's Play(0, 0, flags) (slot 12), so
   a zero argument is "play once from the start", NOT a mute (the level and
   pan live in slots 8 and 6, which this never touches). applyEvent's note
   arm binds, sets volume and pan, then calls this: that is a keysound
   trigger, and reseedLane ends the same way to sound the record nearest the
   clock.
   The second call is in tail position and the function returns nothing, so
   cl overwrites its own argument slot and jumps (CLAUDE.md's tail-jmp rule)
   rather than pushing. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f520
void KEZPlayer::m40f520(SoundParam *a)
{
    if (a->field_0x24 != 0)
    {
        a->field_0x24->vf05();
        a->field_0x24->vf04(0);
    }
}

/* Slot 3: run slot 5's teardown, then drop the COM object at +0xb8. The
   release goes through slot 2 with `this` pushed - the __stdcall COM shape,
   not a thiscall. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d140
void Obj40D4E0::v3()
{
    v5();

    if (fb8 != 0)
    {
        fb8->v2();
        fb8 = 0;
    }
}

/* Point the record at a table and start it over: the head and its flag, the
   run flag at +0x210, the cursor and the whole 0x200-byte per-row index. The
   last two lines are the table's own bookkeeping - `fc` counts the rows still
   to come, and a row 0 with no tween keys does not count as one.

   A null head is ignored, which is what makes GFBackGround::m45c840's
   attach(0, 0) a no-op. */
// FUNCTION: EZ2AC_UNPACKED 0x004028b0
void Obj402860::attach(Obj402860Head *h, int flag)
{
    if (h != 0)
    {
        head = h;
        f4 = flag;
        f210 = 1;
        f8 = 0;
        f20c = 0;
        memset(cursor, 0, sizeof(cursor));

        head->fc = head->rowCount;

        if (head->rows[0].keyCount == 0)
        {
            head->fc--;
        }
    }
}

/* The game-over screen's per-frame update is nothing but its table's tween
   tick, so cl tail-jumps into it with the adjusted `this`. */
// FUNCTION: EZ2AC_UNPACKED 0x0044d9c0
void GameOverDirector::update()
{
    m_18.m403940();
}

/* The sample seen through the two slots s7 uses. A pointer of this type is
   what keeps the calls dynamic. */
struct FreeTarget
{
    virtual void q0();
    virtual void q1(int flag);   /* slot 1 -> +0x04 */
    virtual void q2();
    virtual void q3();           /* slot 3 -> +0x0c */

    char        _pad4[4];
    FreeTarget *f8;              /* +0x08 - the device, quieted the same way */
};

/* freeSample's hook: quiet the device the sample holds, run the sample's own
   slot 3 teardown, quiet the sample, and report that it was handled. A null
   sample is not. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ea80
int Obj40EAC0::s7(void *sample)
{
    FreeTarget *s = (FreeTarget *)sample;
    FreeTarget *d;

    if (s == 0)
    {
        return 0;
    }

    d = s->f8;

    if (d != 0)
    {
        d->q1(1);
    }

    s->q3();
    s->q1(1);
    return 1;
}

/* Start the run-out fade, once. Phases 1 and 2 mean it is already running.
   The fader gets this director's first sink base, which is what makes the
   argument a `lea` off `this` rather than a plain push. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d8d0
void GFMainGameDirector::m45d8d0()
{
    if (f20 != 1 && f20 != 2)
    {
        m45d7c0();
        f158->m450420(2, 10, (char *)this + 0x10);
        f20 = 1;
    }
}

/* The catch twin. Same shape, except that it releases the cursor clip where
   the GF one runs a helper - and its guard tests 0 rather than 2, so only
   phase 0 starts the fade at all. */
// FUNCTION: EZ2AC_UNPACKED 0x0046c240
void CatchMainGameDirector::m46c240()
{
    if (f3c != 1 && f3c == 0)
    {
        ClipCursor(0);
        f174->m450420(2, 10, (char *)this + 0x10);
        f3c = 1;
    }
}

/* The fade IN, the mirror of m45d8d0 above. Phase 0 goes through the first
   fader; phases 2 and 3 both go through the second, and the original keeps
   the two blocks apart rather than testing them together - so they are two
   arms here too. Any other phase does nothing. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d810
void GFMainGameDirector::m45d810()
{
    if (f20 == 0)
    {
        f158->m450420(1, 10, (char *)this + 0x10);
    }
    else if (f20 == 2)
    {
        f15c->m450420(1, 10, (char *)this + 0x10);
    }
    else if (f20 == 3)
    {
        f15c->m450420(1, 10, (char *)this + 0x10);
    }
}

/* Stop every live tick target and drop bit 0 of its entry. The live count is
   re-read from the bound sample on every iteration, exactly as tick() does -
   a slot 5 may change it - and the counter is zeroed ahead of the guard so
   the guard compares the count against that register. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fd20
void KEZPlayer::m40fd20()
{
    int i;

    if (f04 != 0)
    {
        i = 0;

        if (i < ((Counted *)f04)->count)
        {
            /* Anchored at `obj`, not at the entry: the flag word is eight
               dwords below it, which is where the original's displacements
               put the walk. */
            TickTarget **p = &entries[0].obj;

            do
            {
                TickTarget *t = *p;

                if (t != 0)
                {
                    t->v5();
                }

                ((int *)p)[-8] &= ~1;
                i++;
                p = (TickTarget **)((char *)p + sizeof(Entry));
            }
            while (i < ((Counted *)f04)->count);
        }
    }
}

/* The object at the graphics singleton's +0x1d0 - src/accessors.cpp owns
   the two matched accessors; only the second is needed here. */
struct ObjWithProfile
{
    char _pad[0x1d0];
    void *profile;   /* 0x1d0 */
    void initProfile();
};

/* Obj405F80::v3 @0x405dd0 - the graphics object's teardown. The GDI object
   at +0x130 goes first (the IAT slot resolved to DeleteObject through the
   surviving import name table), then both KProfiles are emptied, the seven
   interfaces released, and the global table freed - the last of those in
   tail position, which is why it is a jmp. */
// FUNCTION: EZ2AC_UNPACKED 0x00405dd0
void Obj405F80::v3()
{
    if (f130 != 0)
    {
        DeleteObject((HGDIOBJ)f130);
        f130 = 0;
    }

    ((ObjWithProfile *)this)->initProfile();
    m409320();
    ((KGraphics *)this)->m403f60();
    releaseAll();
    freeTable405530();
}

/* Obj40D4E0::v4 @0x40d640 - latch the run flag. Bit 1 of f04 follows the
   argument, f0ac is marked dirty, and the COM object is told to run when
   EITHER bit 0 of f04 is already up or the argument asked for it. Waking
   the gate is gated on bit 0 alone. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d640
void Obj40D4E0::v4(int on)
{
    if (fb8 != 0)
    {
        if (on != 0)
        {
            f04 |= 2;
        }
        else
        {
            f04 &= ~2;
        }

        fac |= 1;

        if ((f04 & 1) != 0 || on != 0)
        {
            on = 1;
        }

        fb8->op12(0, 0, on);

        if ((f04 & 1) != 0)
        {
            SetEvent(ev1.m_h);
        }
    }
}

/* Obj40EAC0::shutdown @0x40e8c0 - run the base's two teardowns, then drop
   the two COM objects in the reverse of the order they were made. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e8c0
void Obj40EAC0::shutdown()
{
    m40e770();
    m40e6d0();

    if (fa064 != 0)
    {
        fa064->v2();
        fa064 = 0;
    }

    if (fa060 != 0)
    {
        fa060->v2();
        fa060 = 0;
    }
}

/* Obj40D4E0::v5 @0x40d6a0 - stop. While bit 0 of f04 is up the pair of
   events is a real lock, so it is taken here and released at the end; the
   dirty bit goes down in between, and the COM object is told to stop (slot
   18) and rewind (slot 13 with 0). */
// FUNCTION: EZ2AC_UNPACKED 0x0040d6a0
void Obj40D4E0::v5()
{
    if ((f04 & 1) != 0)
    {
        WaitForSingleObject(ev0.m_h, INFINITE);
        ResetEvent(ev1.m_h);
    }

    fac &= ~1;

    if (fb8 != 0)
    {
        fb8->v18();
        fb8->op13(0);
    }

    if ((f04 & 1) != 0)
    {
        SetEvent(ev0.m_h);
    }
}

/* What KEZPlayer::v3 asks its candidate for - the class descriptor, so it
   can be tested against KSound's. */
struct KSoundLike
{
    virtual KRuntimeClass *getRuntimeClass();
};

/* KEZPlayer::v3 @0x40f340 - bind a KSound-derived object as the source, but
   only while nothing is playing. The `push` of KSound's descriptor happens
   BEFORE the virtual call that produces the object it is tested against,
   because it is the argument of the OUTER call - which is why one push
   serves a call two levels down. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f340
int KEZPlayer::v3(void *obj)
{
    if (obj != 0 && f7274 == 0)
    {
        if (((KSoundLike *)obj)->getRuntimeClass()->isDerivedFrom(&classKSound) != 0)
        {
            f08 = (int)obj;
            return 1;
        }
    }

    return 0;
}

/* KEZPlayer's own vtable, viewed for the one slot m4101c0 drives. The class
   models eight slots; slot 10 is past them, and a mirror keeps the call
   indirect the way the original has it. */
struct KEZPlayerSlots
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4(); virtual void q5();
    virtual void q6(); virtual void q7(); virtual void q8();
    virtual void q9();
    virtual void q10();               /* slot 10 -> +0x28 */
};

/* KEZPlayer::v2 @0x40f8f0 - bind an EzChart-derived object as the chart,
   but only while nothing is playing. The descriptor is asked for TWICE: once
   to test it against EzChart's outright, and again for the base-chain walk
   when that fails - which is why the second call carries the push the first
   does not. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f8f0
int KEZPlayer::v2(void *obj)
{
    if (f7274 != 0)
    {
        return 0;
    }

    if (((KSoundLike *)obj)->getRuntimeClass() == (KRuntimeClass *)&g_ezChartClassInfo
        || ((KSoundLike *)obj)->getRuntimeClass()->isDerivedFrom(
               (KRuntimeClass *)&g_ezChartClassInfo) != 0)
    {
        f04 = (int)obj;
        m40f880();
        createSamples();
        m40f690();
        return 1;
    }

    return 0;
}

/* KEZPlayer::m4101c0 @0x4101c0 - full stop. Slot 10 first, then both
   embedded players come off the pump and the pump pointer is dropped; the
   tick table is rebuilt from scratch, both bindings cleared, and the gate
   re-armed (set the idle event, reset the busy one). */
// FUNCTION: EZ2AC_UNPACKED 0x004101c0
void KEZPlayer::m4101c0()
{
    ((KEZPlayerSlots *)this)->q10();

    if (f0c != 0)
    {
        ((Obj414750 *)f0c)->removePlayer((KSongPlayerBase *)&player0);
        ((Obj414750 *)f0c)->removePlayer((KSongPlayerBase *)&player1);
        f0c = 0;
    }

    m40f880();

    f04 = 0;
    f08 = 0;
    SetEvent(ev0.m_h);
    ResetEvent(ev1.m_h);
}

/* One entry of the bound sample's own table, at Counted +0x9c with a 0x4c
   stride. Only the word m40f730 tests is evidenced. */
struct SampleTableEntry
{
    char _pad0[0x48];
    int  f48;                         /* +0x48 - non-zero means the entry is live */
};

/* KEZPlayer::m40f730 @0x40f730 - blank the tick table and re-seed it from
   the bound sample.

   The blank pass touches nine of the entry's twelve words and leaves +0x00,
   +0x1c and +0x20 alone, so it is a re-seed rather than a clear. The second
   pass runs only while a sample is bound and gives each live entry its source
   pointer, marking the entry live when that source's +0x48 is set. The count
   is re-read every iteration, as everywhere else in this class. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f730
void KEZPlayer::m40f730()
{
    Entry *e = entries;
    int    n = 96;
    int    i;

    do
    {
        e->flags = 0;
        e->f08 = 0;
        e->f0c = -1;
        e->f10 = -1;
        e->f14 = -1;
        e->f18 = 0x7f;
        e->obj = 0;
        e->src = 0;
        e->f2c = 0;
        e++;
    }
    while (--n != 0);

    if (f04 != 0)
    {
        i = 0;

        if (i < ((Counted *)f04)->count)
        {
            do
            {
                Entry            *e2 = &entries[i];
                SampleTableEntry *s =
                    (SampleTableEntry *)((char *)f04 + 0x9c + i * 0x4c);

                e2->src = s;

                if (s->f48 != 0)
                {
                    e2->flags = 1;
                }

                i++;
            }
            while (i < ((Counted *)f04)->count);
        }
    }
}

/* KEZPlayer::v4 @0x40f960 - bind the pump and start both embedded players,
   but only while nothing is playing.

   The pump takes both players; the second is set to a flat 15.625, and the
   first to 60000 divided by (frames/4 * 120) - the sample's own frame size
   at Counted +0x88 turned into a playback rate against the 120 BPM base,
   which is also what the speed member is reset to.

   The 120 is loaded ONCE and serves both the member store and the divisor -
   hence the named local, without which cl re-reads it as a memory operand. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f960
int KEZPlayer::v4(void *pump)
{
    if (f7274 != 0)
    {
        return 0;
    }

    f0c = (int)pump;

    if (pump != 0)
    {
        ((Obj414750 *)pump)->addPlayer((KSongPlayerBase *)&player0);
        ((Obj414750 *)f0c)->addPlayer((KSongPlayerBase *)&player1);

        ((KSongPlayerBase *)&player1)->setRate(15.625f);

        float base = k120;

        f7260 = base;
        ((KSongPlayerBase *)&player0)->setRate(
            60000.0f / ((float)(((Counted *)f04)->f88 / 4) * base));
    }

    return 1;
}
