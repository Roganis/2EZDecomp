/* PlayerSlot::m42ee40 @0x42ee40, PlayerRecord::m466550 @0x466550 and
   CatchPlayer::m473260 @0x473260 - what the third sink forwards to every
   node of the run's player list.

   Keys 9 and 10 are the two speed keys, and only while `what` is 2 or more:
   each latches its own flag on the player and, WHEN g_1b2eb6c IS 1, steps
   the speed index at the third offset - up to 20 for key 9, down to 0 for
   key 10 - then tells the panel the key is down. Every other key, and every
   value of `what`, still reaches the panel's second hook at the end.

   The three differ only in the three member offsets and which panel class
   the pointer is. The catch copy uses a different pair of flags again
   (+0x450 / +0x458 against the other two's +0x4bc / +0x4c4 and
   +0x2cc / +0x2d4), so all nine offsets are per-class.

   The two panel calls in the speed arms are written out per arm; cl
   cross-jumps them back into one call fed by two pushes, which is what the
   original has.

   In its own TU: the six panel methods are all defined in
   src/gamepanels.cpp and src/catchpanelgds.cpp, and these must keep their
   calls. */

#include "players.h"
#include "gamepanels.h"

extern int g_1b2eb6c;   /* src/ez2acapp.cpp */

/* The three players' own fields, all inside a modelled _pad run. */
struct SlotSpeedFields
{
    char _pad0[0x4bc];
    int  fUp;                  /* 0x4bc */
    char _pad4c0[4];
    int  fDown;                /* 0x4c4 */
    char _pad4c8[0x4e0 - 0x4c8];
    int  index;                /* 0x4e0 */
};

struct RecordSpeedFields
{
    char _pad0[0x2cc];
    int  fUp;                  /* 0x2cc */
    char _pad2d0[4];
    int  fDown;                /* 0x2d4 */
    char _pad2d8[0x2f0 - 0x2d8];
    int  index;                /* 0x2f0 */
};

struct CatchSpeedFields
{
    char _pad0[0x450];
    int  fUp;                  /* 0x450 */
    char _pad454[4];
    int  fDown;                /* 0x458 */
    char _pad45c[0x474 - 0x45c];
    int  index;                /* 0x474 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0042ee40
void PlayerSlot::m42ee40(int key, int what)
{
    SlotSpeedFields *f = (SlotSpeedFields *)this;

    if (what >= 2)
    {
        if (key == 9)
        {
            f->fUp = 1;

            if (g_1b2eb6c == 1)
            {
                f->index += 1;

                if (f->index >= 0x15)
                {
                    f->index = 0x14;
                }
            }

            ((Panel *)f3cc)->m429910(9, 1);
        }
        else if (key == 10)
        {
            f->fDown = 1;

            if (g_1b2eb6c == 1)
            {
                f->index -= 1;

                if (f->index < 0)
                {
                    f->index = 0;
                }
            }

            ((Panel *)f3cc)->m429910(10, 1);
        }
    }

    ((Panel *)f3cc)->m429aa0(key, what);
}

// FUNCTION: EZ2AC_UNPACKED 0x00466550
void PlayerRecord::m466550(int key, int what)
{
    RecordSpeedFields *f = (RecordSpeedFields *)this;

    if (what >= 2)
    {
        if (key == 9)
        {
            f->fUp = 1;

            if (g_1b2eb6c == 1)
            {
                f->index += 1;

                if (f->index >= 0x15)
                {
                    f->index = 0x14;
                }
            }

            ((GFPanel *)f1dc)->m462720(9, 1);
        }
        else if (key == 10)
        {
            f->fDown = 1;

            if (g_1b2eb6c == 1)
            {
                f->index -= 1;

                if (f->index < 0)
                {
                    f->index = 0;
                }
            }

            ((GFPanel *)f1dc)->m462720(10, 1);
        }
    }

    ((GFPanel *)f1dc)->m4627c0(key, what);
}

// FUNCTION: EZ2AC_UNPACKED 0x00473260
void CatchPlayer::m473260(int key, int what)
{
    CatchSpeedFields *f = (CatchSpeedFields *)this;

    if (what >= 2)
    {
        if (key == 9)
        {
            f->fUp = 1;

            if (g_1b2eb6c == 1)
            {
                f->index += 1;

                if (f->index >= 0x15)
                {
                    f->index = 0x14;
                }
            }

            f35c->m471290(9, 1);
        }
        else if (key == 10)
        {
            f->fDown = 1;

            if (g_1b2eb6c == 1)
            {
                f->index -= 1;

                if (f->index < 0)
                {
                    f->index = 0;
                }
            }

            f35c->m471290(10, 1);
        }
    }

    f35c->m4712d0(key, what);
}
