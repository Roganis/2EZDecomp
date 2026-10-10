/* EZ2DJSink2::v0 @0x4202f0 - the lane-expiry sink.

   The director hands over the SlotSound whose countdown has run out. Its
   +0x00 is that sound's own slot index, which selects a pair out of the
   table at director +0x16c: the pair's first word is the player the sound
   belongs to, the second is the track-list entry that put it there. The
   player's 452-byte run record is checked for being live, and if it is,
   the commit runs on the PlayerSlot the record holds and the BACKGROUND is
   handed the grade so it can flash its judgement notice.

   THE RECORD'S +0x130 IS ONLY EVIDENCED AS AN OFFSET. include/players.h
   models the same 452-byte region as TrackList, whose items[27] would put
   this inside items[18] - so either that array is shorter than modelled or
   the record is not a TrackList. Nothing else in the tree reads the
   address, so it is left as a local view rather than carved into either.

   In its own TU: src/holdtick.cpp defines the commit, and this must keep
   its `call`. */

#include "players.h"
#include "ezscreen.h"
#include "backgrounds.h"
#include "bigbase.h"   /* Obj401780::armAndSetC */

extern void *g_1b2e844;   /* src/moderunner.cpp */
extern int   g_panelDrawn;/* 0x1b2f090 (src/gamepanels.cpp) */

/* The 452-byte per-player run record at director +0x380 = sink +0x36c. */
struct SinkRecord
{
    int         live;                    /* +0x000 */
    char        _pad4[0x130 - 4];
    PlayerSlot *slot;                    /* +0x130 */
    char        _pad134[452 - 0x134];
};

/* The sink's own view of the director it is a base of. */
struct Ez2djSinkView
{
    char       _pad0[0x158];
    SlotSound  pairs[48];                /* 0x158 - director +0x16c */
    char       _pad2d8[0x360 - 0x2d8];
    /* The BACKGROUND, not the timed object: the director's field is typed
       MainTimed * but src/bgload.cpp assigns it the background, and this
       call lands on a GFBackGround method. */
    GFBackGround *bg;                    /* 0x360 - director +0x374 */
    char       _pad364[0x36c - 0x364];
    SinkRecord records[2];               /* 0x36c - director +0x380 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004202f0
void EZ2DJSink2::v0(SlotSound *s, int a, int b)
{
    Ez2djSinkView *v = (Ez2djSinkView *)this;
    int            k = s->f0;
    SinkRecord    *rec = &v->records[v->pairs[k].f0];

    if (rec->live != 0)
    {
        rec->slot->m42fd90(v->pairs[k].id, s, a, b);
        v->bg->m45cb20(a, b);
    }
}


/* GFSink2::v0 @0x45d580 - the GF twin, the same function with the three
   member offsets 0x14 lower and PlayerRecord's commit in place of
   PlayerSlot's. The background it notifies is reached the same way, through
   the same method. */

/* The GF record: same 452-byte stride, same +0x130. */
struct GfSinkRecord
{
    int           live;                  /* +0x000 */
    char          _pad4[0x130 - 4];
    PlayerRecord *rec;                   /* +0x130 */
    char          _pad134[452 - 0x134];
};

struct GfSinkView
{
    char          _pad0[0x14c];
    SlotSound     pairs[48];             /* 0x14c - director +0x160 */
    char          _pad2cc[0x324 - 0x2cc];
    GFBackGround *bg;                    /* 0x324 - director +0x338 */
    char          _pad328[0x32c - 0x328];
    GfSinkRecord  records[2];            /* 0x32c - director +0x340 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0045d580
void GFSink2::v0(SlotSound *s, int a, int b)
{
    GfSinkView   *v = (GfSinkView *)this;
    int           k = s->f0;
    GfSinkRecord *rec = &v->records[v->pairs[k].f0];

    if (rec->live != 0)
    {
        rec->rec->m466ac0(v->pairs[k].id, s, a, b);
        v->bg->m45cb20(a, b);
    }
}


/* CatchSink2::v0 @0x46c060 - the catch member of the family, and the one
   that is really different.

   Its run record is 444 bytes rather than 452 and keeps the player at
   +0x128 rather than +0x130. It also carries a step the other two do not:
   a per-lane flag byte at director +0x1c, thirteen per player, and when the
   expiring lane's flag is up the player's +0x548 counter is bumped before
   the commit runs.

   The entry index is read out of the pair table TWICE - once for the flag
   and once for the commit's first argument - which is in the original. */

/* The 444-byte per-player run record at director +0x34c = sink +0x338. */
struct CatchSinkRecord
{
    int          live;                   /* +0x000 */
    char         _pad4[0x128 - 4];
    CatchPlayer *player;                 /* +0x128 */
    char         _pad12c[444 - 0x12c];
};

struct CatchSinkView
{
    char             _pad0[8];
    char             laneFlags[26];      /* 0x008 - director +0x1c, 13/player */
    char             _pad22[0x164 - 0x22];
    SlotSound        pairs[48];          /* 0x164 - director +0x178 */
    char             _pad2e4[0x330 - 0x2e4];
    CatchBackGround *bg;                 /* 0x330 - director +0x344 */
    char             _pad334[0x338 - 0x334];
    CatchSinkRecord  records[2];         /* 0x338 - director +0x34c */
};

// FUNCTION: EZ2AC_UNPACKED 0x0046c060
void CatchSink2::v0(SlotSound *s, int a, int b)
{
    CatchSinkView   *v = (CatchSinkView *)this;
    int              k = s->f0;
    int              p = v->pairs[k].f0;
    CatchSinkRecord *rec = &v->records[p];

    if (rec->live != 0)
    {
        if (v->laneFlags[p * 13 + v->pairs[k].id] != 0)
        {
            rec->player->f548 += 1;
        }

        rec->player->m473c60(v->pairs[k].id, s, a, b);
        v->bg->m46b7a0(a, b);
    }
}


/* EZ2DJSink1::v0 @0x4248c0 - report the run's outcome to the screen driver.
   The elaborate member of its family; the GF and catch copies of this one
   are much shorter and live in src/directors.cpp.

   Only kind 2 and kind 4 mean anything. Kind 4 just latches run state 3.
   Kind 2 branches on the run state:

     1 - hand the driver 3 if the flag at sink+0x340 is up, otherwise the
         stored code at sink+0xd00, and clear the panel-drawn latch either
         way (the original stores it twice, once per arm);
     2 - hand the driver 3 if that same flag is up, otherwise run the stage
         teardown pair on the director and reset the run state to 0;
     4 - hand the driver 5.

   `this` is the first sink, 0x10 into the director. */
struct Ez2djSink1Fields
{
    char _pad0[0x10];
    int  f10;                  /* sink+0x010 - the run state */
    char _pad14[0x340 - 0x14];
    int  f340;                 /* sink+0x340 */
    char _pad344[0xd00 - 0x344];
    int  fd00;                 /* sink+0xd00 - the exit code */
};

// FUNCTION: EZ2AC_UNPACKED 0x004248c0
void EZ2DJSink1::v0(int, int what)
{
    Ez2djSink1Fields     *f = (Ez2djSink1Fields *)this;
    EZ2DJMainGameDirector *d;

    if (what == 2)
    {
        if (f->f10 == 1)
        {
            if (f->f340 == 1)
            {
                ((Obj401780 *)g_1b2e844)->armAndSetC(3);
                g_panelDrawn = 0;
                return;
            }

            ((Obj401780 *)g_1b2e844)->armAndSetC(f->fd00);
            g_panelDrawn = 0;
            return;
        }

        if (f->f10 == 2)
        {
            if (f->f340 == 1)
            {
                ((Obj401780 *)g_1b2e844)->armAndSetC(3);
                return;
            }

            d = (EZ2DJMainGameDirector *)((char *)this - 0x10);
            d->m420530();
            d->m4246d0();
            f->f10 = 0;
            return;
        }

        /* DEAD: the run state (director+0x20) is only ever stored 0, 1
           (the ctor, update2, m424830) or 3 (just below), never 2 or 4 -
           and nothing in the binary starts a ScrFader in mode 3 (all 17
           m450420 calls pass 1 or 2), so even the 3 never happens. Exit
           code 5 is unreachable, as is the `== 2` arm above. (Byte scan,
           2026-10-01.) */
        if (f->f10 == 4)
        {
            ((Obj401780 *)g_1b2e844)->armAndSetC(5);
        }
    }
    else if (what == 4)
    {
        f->f10 = 3;
    }
}
