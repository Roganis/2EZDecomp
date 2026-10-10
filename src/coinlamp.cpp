/* TitleDirector::tickCoinLamp @0x44e880 - one frame of the title screen's
   coin lamp.

   Slot 17 of the device is ticked unconditionally. Then, if the countdown is
   idle AND the device's slot 16 reports its low bit clear, the lamp fires:
   slot 5 lights it, the attract song STOPS (unlocked), and the countdown is
   armed with the caller's frame count. Every later frame decrements it, and
   the frame it reaches zero puts the device back (slots 12 and 4, both with
   0) and starts the song again.

   The last call is unconditional and rescales the lamp period: slot 8 gets
   (m_30 - 255) * 10000 / 300, a signed divide cl emits as the 0x1b4e81b5/5
   magic pair.

   The countdown is read into a local before the decrement - that is what
   gives the original's `add eax,-1` rather than a `sub` on the member, and
   it is also why the store and the zero test share one instruction. */

#include "ezscreen.h"
#include "ksongplayer.h"

// FUNCTION: EZ2AC_UNPACKED 0x0044e880
void TitleDirector::tickCoinLamp(TitleLampDev *dev, int frames)
{
    int n;

    dev->d17();

    if (m_1620 == 0 && (dev->d16() & 1) == 0)
    {
        dev->d5();
        stop(0);
        m_1620 = frames;
    }

    n = m_1620;
    if (n != 0)
    {
        m_1620 = n - 1;
        if (m_1620 == 0)
        {
            dev->d12(0);
            dev->d4(0);
            start();
        }
    }

    dev->d8(((m_30 - 255) * 10000) / 300);
}
