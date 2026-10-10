/* Obj401780::run @0x401930 - THE SCREEN DRIVER every screen goes through.
   EZ2AC::runGame @0x417e90 calls it on each screen it builds, and so does
   every mode runner in src/moderunner.cpp. Integrated from wip/screenrun.cpp
   at 88.50%.

   LOGIC AND CODEGEN ARE FULLY SOLVED. Every instruction matches - the field
   init, the profile fallback, the slot 2/4/5 setup calls, timeGetTime through
   the import thunk, the two pump loops, the `call ebx` for a Sleep whose IAT
   slot cl hoisted out of the loop, the unsigned `div` for (slot + 2) % 3, and
   the teardown through slots 6 and 3 with the VirtualFree - EXCEPT the STACK
   SLOT ASSIGNMENT of three locals, which is a 3-cycle permutation:

       local        original      ours
       fullScreen   esp+0x0c      esp+0x10
       elapsed      esp+0x10      esp+0x24   (the dead `profile` param slot)
       slot         esp+0x24      esp+0x0c
       frameMs[3]   esp+0x14..    esp+0x14.. (matches)

   THE COUPLING: the two loop locals have to be MEMORY-resident and reloaded
   per use, which is what `volatile` gives - and is also why cl hoists Sleep's
   IAT slot into ebx, the register a non-volatile local would have taken. But
   with TWO volatile locals cl's stack packer stops using declaration order and
   produces the permutation above. With only ONE the packer DOES keep
   declaration order and fullScreen lands correctly at 0x0c, but then cl gives
   the other loop local a register and stops hoisting Sleep, so the loop body
   diverges instead. Swept: all 24 declaration orders, per-local scoping, the
   array volatile, fullScreen volatile, and elapsed declared inside the loop.

   A 100% BYTE MATCH IS AVAILABLE AND IS DELIBERATELY NOT TAKEN. Declaring the
   two scalars as one `volatile unsigned int pair[2]` reaches it, because
   arrays ARE packed in declaration order. That would misrepresent the
   original's variables - a screen-mode flag and an elapsed time are not one
   array - and CLAUDE.md's rule is to park the analysis rather than ship a hack
   that scores. The honest form is what is here; the trick is recorded so
   nobody rediscovers it and assumes it was missed. */

#include <windows.h>
#include "ezscreen.h"

extern KProfile *g_iniProfile;
extern "C" DWORD __stdcall timeGetTime(void);

/* The screen driver: initialize the screen, let its slots 2/4/5 set it up, then
   pump slot 1 until f8 goes to zero, and tear it down through slots 6 and 3.
   Windowed mode ("FullScreen" 0 in the profile) paces the pump at 60fps with a
   17/17/16 ms budget table walked BACKWARDS - (slot + 2) % 3 - sleeping off
   whatever is left of each frame; fullscreen mode pumps flat out. */
// FUNCTION: EZ2AC_UNPACKED 0x00401930
int Obj401780::run(KProfile *profile)
{
    /* Declaration order is load-bearing: cl assigns these stack slots in a
       right-rotation of the declaration order (and hands the highest one the dead
       `profile` parameter slot), so this order is what puts fullScreen, elapsed
       and slot where the original has them. */
    volatile unsigned int elapsed;
    volatile unsigned int slot;
    unsigned int frameMs[3];
    int fullScreen;

    f8 = 3;
    fc = 0;
    f10 = 0;
    f14 = 0;
    f18 = 0;
    f1c = 0;
    f20 = 0;
    f24 = 0;
    f28 = 0;
    f2c = 0;

    if (profile == 0)
    {
        f4 = g_iniProfile;
    }
    else
    {
        f4 = profile;
    }

    s2();
    s4();
    m4017a0();
    f30 = timeGetTime();
    f8 = 1;
    s5();

    if (profile != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487584
        profile->getInt("FullScreen", &fullScreen);
    }
    else
    {
        fullScreen = 1;
    }

    if (fullScreen != 0)
    {
        while (f8 != 0)
        {
            s1();
        }
    }
    else
    {
        frameMs[0] = 17;
        frameMs[1] = 17;
        frameMs[2] = 16;
        slot = 0;

        while (f8 != 0)
        {
            elapsed = timeGetTime();
            s1();
            elapsed = timeGetTime() - elapsed;

            if (elapsed < frameMs[slot])
            {
                Sleep(frameMs[slot] - elapsed);
            }

            slot = (slot + 2) % 3;
        }
    }

    s6();
    m4018c0();

    if (f28 != 0)
    {
        VirtualFree(f28, 0, MEM_RELEASE);
    }

    s3();
    return fc;
}
