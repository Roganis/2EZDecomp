#include <conio.h>
#include "bigbase.h"

/* One engine tick. Three sub-updates first, then every live entry of the first
   slot table is advanced by whichever source drives it:

     source 1  the hardware tracker - watch bit 7 of the entry's keyState byte
               and run the four-state press machine (0 idle, 2 just pressed and
               time-stamped, 3 held, 1 just released);
     source 0  and source 3, two handlers that get the entry and the time.

   With bit 1 of the mode word set, every entry also reports (index, state, time).
   Afterwards: an optional whole-table pass with bit 2 set, resolve the states onto
   the output bits, push those bits out to the I/O board, and finish through vtable
   slot 2. Kept out of src/engine.cpp so applyOutputs stays a call. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cc90
void Obj40C180::tick(unsigned int now)
{
    int i;

    m40c390();
    m40c430();
    m40c4b0();

    for (i = 0; i < f4 + 1; i++)
    {
        IoSlot *e = &slotsA[i];

        if (e->source == 1)
        {
            if ((keyState[e->bit] & 0x80) != 0)
            {
                if (e->state == 0 || e->state == 1)
                {
                    e->state = 2;
                    e->since = now;
                }
                else
                {
                    e->state = 3;
                }
            }
            else
            {
                if (e->state == 2 || e->state == 3)
                {
                    e->state = 1;
                }
                else
                {
                    e->state = 0;
                }
            }
        }
        else if (e->source == 0)
        {
            m40c3d0(e, now);
        }
        else if (e->source == 3)
        {
            m40c6f0(e, now);
        }

        if ((f4200ec & 2) != 0)
        {
            m40ca90(i, (char)e->state, now);
        }
    }

    if ((f4200ec & 4) != 0)
    {
        m40cb20(now);
    }

    applyOutputs();

    if (f4200f4 != 0)
    {
        _outp(0x100, (unsigned char)flags[0]);
        _outp(0x101, (unsigned char)flags[1]);
        _outp(0x102, (unsigned char)flags[2]);
        _outp(0x103, (unsigned char)flags[3]);
    }

    serviceCoins();
}
