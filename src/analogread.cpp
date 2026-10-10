/* Obj40C180::m40c4b0 @0x40c4b0 - promoted from wip/analogread.cpp at its
   parked 42.9%: FULLY DECODED, call-exact, and the residue is a
   whole-function register permutation with no source-level lever (the
   full analysis - the volatile-counter probe, the two-pointer walk, and
   the diagnostic that freeing a register does not snap the allocation -
   is in the wip file's git history). The layout it established lives in
   include/bigbase.h: delta[2] @0x420230, accum[2] @0x420238 (adjacent,
   hence d[2]), hist[2][5] @0x420240, the limit @0x420268. Same
   permutation class as wip/runclubmix.cpp; retry only if that one ever
   falls. */
#include <stdlib.h>
#include <conio.h>
#include "bigbase.h"

/* Read the two analog encoders. Both are 8-bit counters that wrap, so this tick's
   movement is the difference from the last reading folded back into
   [-0x80, 0x80]; a running total that has grown past the limit is thrown away
   before this tick's movement is added to it.

   Then, per side, a direction indicator with hysteresis: while the encoder is
   moving, the direction bit follows the movement and the history is wiped; when
   it stops, the direction is held for as long as any of the last five movements
   was non-zero (the most recent non-zero one wins), and only once all five are
   zero do both bits clear and the running total reset.

   The per-side loop walks three POINTERS rather than indexing four arrays: the
   history one anchored one entry in (h[-1]..h[3]), and the running total reached
   off the movement pointer as d[2], which is what the original's `[edi+8]`
   says the source did - the two arrays are adjacent for that reason. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c4b0
void Obj40C180::m40c4b0()
{
    int a;
    int b;
    int side;
    int *d;
    int *h;
    unsigned char *bits;

    if (f4200f4 == 0)
    {
        return;
    }

    port103b = port103;
    port104b = port104;
    tail = head;

    keyState[4] = (unsigned char)~_inp(0x101);
    keyState[5] = (unsigned char)~_inp(0x102);
    keyState[6] = (unsigned char)~_inp(0x106);
    head = _inp(0x105);

    a = _inp(0x103);
    b = _inp(0x104);

    delta[0] = a - port103b;
    port104 = b;
    delta[1] = b - port104b;
    port103 = a;

    if (delta[0] > 0x80)
    {
        delta[0] -= 0x100;
    }

    if (delta[0] < -0x80)
    {
        delta[0] += 0x100;
    }

    if (delta[1] > 0x80)
    {
        delta[1] -= 0x100;
    }

    if (delta[1] < -0x80)
    {
        delta[1] += 0x100;
    }

    if (abs(accum[0]) > limit)
    {
        accum[0] = 0;
    }

    if (abs(accum[1]) > limit)
    {
        accum[1] = 0;
    }

    accum[0] += delta[0];
    accum[1] += delta[1];

    d = delta;
    bits = &keyState[5];
    h = &hist[0][1];

    for (side = 0; side < 2; side++)
    {
        if (*d != 0)
        {
            if (*d > 0)
            {
                *bits = (unsigned char)((*bits & 0xbf) | 0x20);
            }
            else
            {
                *bits = (unsigned char)((*bits & 0xdf) | 0x40);
            }

            h[3] = 0;
            h[2] = 0;
            h[1] = 0;
            h[0] = 0;
            h[-1] = 0;
        }
        else if (h[-1] == 0 && h[0] == 0 && h[1] == 0 && h[2] == 0 && h[3] == 0)
        {
            *bits &= 0x9f;
            d[2] = 0;
        }
        else
        {
            int dir = 0;

            if (h[-1] != 0)
            {
                dir = (h[-1] > 0) ? 1 : -1;
            }

            if (h[0] != 0)
            {
                dir = (h[0] > 0) ? 1 : -1;
            }

            if (h[1] != 0)
            {
                dir = (h[1] > 0) ? 1 : -1;
            }

            if (h[2] != 0)
            {
                dir = (h[2] > 0) ? 1 : -1;
            }

            if (h[3] != 0)
            {
                if (h[3] > 0)
                {
                    *bits = (unsigned char)((*bits & 0xbf) | 0x20);
                }
                else
                {
                    *bits = (unsigned char)((*bits & 0xdf) | 0x40);
                }
            }
            else if (dir > 0)
            {
                *bits = (unsigned char)((*bits & 0xbf) | 0x20);
            }
            else if (dir < 0)
            {
                *bits = (unsigned char)((*bits & 0xdf) | 0x40);
            }
        }

        h[-1] = h[0];
        h[0] = h[1];
        h[1] = h[2];
        h[2] = h[3];
        h[3] = *d;

        d++;
        bits++;
        h += 5;
    }
}
