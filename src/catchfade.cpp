/* CatchPanel::m46f3d0 @0x46f3d0 - the three-mode fade colour, the catch twin
   of Panel::m4270a0 @0x4270a0 (src/gamepanels.cpp).

   Same three mutually-exclusive modes in the same order, but read from a
   DIFFERENT set of slot flags - 0x408/0x410/0x418 where Panel reads
   0x474/0x47c/0x484 - and with the biased ramp CALLED rather than written
   out: the catch copies @0x46f310 and @0x46f370 are real calls here where
   Panel has its biased one inlined, which is why this cannot live in
   src/gamepanels.cpp beside their definitions.

   Mode three is the same 510-steps-per-period saw Panel's third mode uses,
   counted down from 255. Off all three modes the colour is -1, not white.

   100%* EFFECTIVE: three renamed registers in the two call arms - the
   original carries the elapsed difference in edx in both, cl picks ecx in
   the first and swaps eax/edx in the second. Every instruction and every
   encoding length is the same, and mode three is byte-exact. Naming the two
   call arguments in locals - the usual lever - is much WORSE here (69%): it
   hoists the member reads and reshapes both arms. */

#include "gamepanels.h"

/* src/gamepanels.cpp - the two catch fade ramps, both __stdcall. */
int __stdcall catchFadeAlphaBiased(int elapsed, int period);   /* 0x46f310 */
int __stdcall catchFadeAlpha(int elapsed, int period);         /* 0x46f370 */

static unsigned int whiteWithAlpha(int alpha)
{
    return (alpha << 24) | 0xffffff;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046f3d0
unsigned int CatchPanel::m46f3d0(unsigned int from, unsigned int to)
{
    if (slot->f408 != 0)
        return whiteWithAlpha(catchFadeAlphaBiased(to - from, f254));

    if (slot->f410 != 0)
        return whiteWithAlpha(catchFadeAlpha(to - from, f254));

    if (slot->f418 != 0)
    {
        unsigned int period = f254;
        unsigned int pos = from % period;
        int v = (int)(pos * 255 * 2 / period) - 1;
        int alpha;

        v = (v < 0) ? 0 : v;
        alpha = 255 - v;
        if (alpha < 0)
            alpha = 0;
        return whiteWithAlpha(alpha);
    }

    return 0xffffffff;
}
