/* TrackSink::m4299e0 @0x4299e0 - retire one lane's effect state.

   The keys commit calls it with 2 and with 1 around a judged press. What it
   does depends on whether the lane's hit is one the counter object still
   recognises: it asks the owner slot's +0x3dc for the entry named by the
   lane's own word, and if that answers 1 AND the option at 0x1b5f2e0 is off,
   only the lane's on/off moves. Otherwise the effect is (re)armed as well -
   kind 1 with a 0.0001 timer on the way in, kind 2 on the way out.

   The lane's word is read out of a table the owner points at with +0x3d0,
   with a 452-byte per-player stride and a 16-byte per-lane one at a 0x398
   bias. The structure that lives in is not modelled; the arithmetic is what
   the original computes and is written out directly. */

#include "players.h"

extern int g_1b5f2e0;   /* 0x1b5f2e0 (src/holdtick.cpp) */

/* src/accessors.cpp - the counter object at the owner's +0x3dc. */
struct Obj426CA0 { int getF0(int i); };   /* 0x426d10 */

/* The owning PlayerSlot, seen through the three fields this needs. */
struct SinkOwner
{
    char       _pad0[8];
    int        id;                       /* 0x008 */
    char       _pad0c[0x3d0 - 0xc];
    char      *f3d0;                     /* 0x3d0 */
    char       _pad3d4[0x3dc - 0x3d4];
    Obj426CA0 *f3dc;                     /* 0x3dc */
};

// FUNCTION: EZ2AC_UNPACKED 0x004299e0
void TrackSink::m4299e0(int lane, int what)
{
    LaneFx    *r = &lanes[lane];
    SinkOwner *o = (SinkOwner *)owner;
    int        v = *(int *)(o->f3d0 + o->id * 452 + 0x398 + lane * 16);

    if (o->f3dc->getF0(v) == 1 && g_1b5f2e0 == 0)
    {
        if (what == 2)
            r->f100 = 1;
        else if (what == 1)
            r->f100 = 0;
    }
    else
    {
        if (what == 2)
        {
            r->ff8 = 1;
            r->ff4 = 0.0001f;
            r->f100 = 1;
        }
        else if (what == 1)
        {
            r->ff8 = 2;
            r->f100 = 0;
        }
    }
}


/* GFTrackSink::m462760 @0x462760 - the same notice on the GF sink. It is the
   ELSE arm of the one above and nothing else: the GF run has no autoplay
   gate, so the lane's word is never read and the owner is never touched. */
// FUNCTION: EZ2AC_UNPACKED 0x00462760
void GFTrackSink::m462760(int lane, int what)
{
    GFLaneFx *r = &lanes[lane];

    if (what == 2)
    {
        r->ff8 = 1;
        r->ff4 = 0.0001f;
        r->f100 = 1;
    }
    else if (what == 1)
    {
        r->ff8 = 2;
        r->f100 = 0;
    }
}
