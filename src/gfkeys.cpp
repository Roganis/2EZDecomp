/* GFMainGameDirector::m45f640 @0x45f640 - the GF game screen's service-key
   handler, and the second of the family of three (the Ez2Catch one is
   m46e750 @0x46e750 in src/catchkeys.cpp, written first; the EZ2DJ one is
   m422f20 @0x422f20 and is still a stub).

   The speed and boost halves are the SAME SOURCE as the catch copy, down to
   the instruction - the whole 0x1b2e890 clamp, the g_1b2eb6c split onto
   g_1b2e914, the "11ambit" floor of 2, and both mixer arms. Read
   src/catchkeys.cpp's header for what those do; only the dialect differs:

     - the list is C, not B (nextListC @0x418470), and the shared argument is
       fca8 rather than fca4;
     - the per-note keys are a SHORTER set - this one has no 0x19 or 0x1a, so
       fifteen keys against the catch copy's seventeen;
     - the two four-argument shuffles pass 1 as their last argument where the
       catch copy passes 0, and key 0x1d's uses m45e770 (eight measures per
       slice) against key 4's and 0x17's m45e9f0;
     - bass is mixerB and treble mixerA in both, but GF's class puts them at
       +0x778 / +0x774 where catch has +0x774 / +0x770.

   Written for the port; 1,036 bytes, 326 instructions, 27 calls. */

#include "ezscreen.h"
#include "directors.h"
#include "players.h"
#include <string.h>

/* Defined in src/mixer.cpp; the same local view src/testinput.cpp uses. */
struct MixerControl { void setValue(unsigned long value); };   /* 0x412a10 */

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern int  nextListC(ListNode **cur);            /* 0x418470 */

extern int  g_speedPercent;                       /* 0x1b2e890 (src/panels.cpp) */
extern int  g_1b2e914;                            /* src/panels.cpp */
extern int  g_1b2eb6c;                            /* src/ez2acapp.cpp */
extern int  g_1b2f06c;
extern char g_songInfoName[64];                   /* 0x1b2eb80 (src/panels.cpp) */
extern int  g_bassBoostOn;                        /* 0x1b2e980 */
extern int  g_trebleBoostOn;                      /* 0x1b2e988 */

// FUNCTION: EZ2AC_UNPACKED 0x0045f640
void GFMainGameDirector::m45f640(int key, int on)
{
    ListNode *n;

    if (on == 0)
        return;

    if (key == 9)
    {
        if (g_1b2eb6c == 0)
        {
            g_speedPercent += 0x19;
            if (g_speedPercent >= 0x3e7)
                g_speedPercent = 0x3e7;
            return;
        }
        g_1b2e914 += 1;
        if (g_1b2e914 >= 0x15)
            g_1b2e914 = 0x14;
        if (g_1b2eb6c == 1)
        {
            g_1b2f06c = g_1b2e914;
            Log(" SPEED = %d", g_1b2e914);
        }
        return;
    }

    if (key == 0xa)
    {
        if (g_1b2eb6c == 0)
        {
            if (g_speedPercent >= 0x3e7)
                g_speedPercent -= 0x18;
            else
                g_speedPercent -= 0x19;
            if (g_speedPercent <= 0x32)
                g_speedPercent = 0x32;
            return;
        }
        g_1b2e914 -= 1;
        if (_stricmp(g_songInfoName, "11ambit") == 0)
        {
            if (g_1b2e914 < 2)
                g_1b2e914 = 2;
        }
        else if (g_1b2e914 < 0)
        {
            g_1b2e914 = 0;
        }
        if (g_1b2eb6c == 1)
        {
            g_1b2f06c = g_1b2e914;
            Log(" SPEED = %d", g_1b2e914);
        }
        return;
    }

    if (key == 0x22)
    {
        g_speedPercent += 1;
        if (g_speedPercent >= 0x3e7)
            g_speedPercent = 0x3e7;
        return;
    }
    if (key == 0x23)
    {
        g_speedPercent -= 1;
        if (g_speedPercent <= 0x32)
            g_speedPercent = 0x32;
        return;
    }
    if (key == 0x24)
    {
        g_speedPercent += 0xa;
        if (g_speedPercent >= 0x3e7)
            g_speedPercent = 0x3e7;
        return;
    }
    if (key == 0x25)
    {
        if (g_speedPercent >= 0x3e7)
            g_speedPercent -= 9;
        else
            g_speedPercent -= 0xa;
        if (g_speedPercent <= 0x32)
            g_speedPercent = 0x32;
        return;
    }

    if (key == 0xb)
    {
        stepPan(1);
        return;
    }
    if (key == 0xc)
    {
        stepPan(0);
        return;
    }

    if (key == 3 || key == 5)
    {
        n = 0;
        while (nextListC(&n) != 0)
            m45ec70(n, key == 3, 0, fca8);
        return;
    }

    if (key == 4)
    {
        n = 0;
        while (nextListC(&n) != 0)
            m45e9f0(n, 0, fca8, 1);
        return;
    }
    if (key == 0x1d)
    {
        n = 0;
        while (nextListC(&n) != 0)
            m45e770(n, 0, fca8, 1);
        return;
    }
    if (key == 0x16)
    {
        n = 0;
        while (nextListC(&n) != 0)
            m45ed90(n, 0, fca8);
        return;
    }
    if (key == 0x17)
    {
        n = 0;
        while (nextListC(&n) != 0)
            m45e9f0(n, 0, fca8, 0);
        return;
    }

    if (key == 0x1b)
    {
        if (g_bassBoostOn != 0)
        {
            if (mixerB == 0)
                return;
            mixerB->setValue(0x85d4);
            Log("BASS BOOST: %s\n", "ON");
            return;
        }
        if (mixerB == 0)
            return;
        mixerB->setValue(0x5938);
        Log("BASS BOOST: %s\n", "OFF");
        return;
    }

    if (key == 0x1c)
    {
        if (g_trebleBoostOn != 0)
        {
            if (mixerA == 0)
                return;
            mixerA->setValue(0xc293);
            Log("TREBLE BOOST: %s\n", "ON");
            return;
        }
        if (mixerA == 0)
            return;
        mixerA->setValue(0xaaaa);
        Log("TREBLE BOOST: %s\n", "OFF");
    }
}
