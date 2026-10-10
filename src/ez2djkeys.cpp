/* EZ2DJMainGameDirector::m422f20 @0x422f20 - the EZ2DJ game screen's
   service-key handler, and the last of the family of three (the Ez2Catch one
   is m46e750 @0x46e750 in src/catchkeys.cpp, the GF one m45f640 @0x45f640 in
   src/gfkeys.cpp).

   Same source as its two siblings for the speed and boost halves - read
   src/catchkeys.cpp's header for those. This is the LONGEST dialect: list A
   (nextListA @0x4182e0), fd14 as the shared argument, and twenty-two keys
   against catch's seventeen and GF's fifteen. What only this one has:

     - KEY 9's SECOND CLAMP. When g_mixStyle is 7 the option index tops out at
       3 rather than 0x14, so the two arms share one store.
     - KEYS 0x20 and 0x21, each a full walk followed by a Log that runs even
       when the list is empty - the "FLIP ENABLE" line is the join the while
       loop's failing first test falls into, not part of the body.
     - KEY 0x16's MODE LADDER. Modes 5 and 9 take m4226f0; 7 and 3 take
       m422410 followed by m422870; everything else takes reverseLanes. The
       tests come out 5, 9, 7, 3 with the else-of-the-inner-test laid inline
       first, which is what the plain nested form gives.
     - KEY 0x19's ladder, whose two arms call the SAME function with the SAME
       arguments. cl merges the call and keeps both push sequences, so it is
       written here as the if/else the bytes describe rather than collapsed.
     - KEYS 0x26 and 0x27, the two lamp sweeps - 0x13..0x40 and 0x00..0x12 -
       driven through the notify object's slot 13, with 0x7f for on and 0x76
       for off.

   Written for the port; 1,995 bytes, 645 instructions, 58 calls. */

#include "ezscreen.h"
#include "directors.h"
#include "players.h"
#include <string.h>

/* Defined in src/mixer.cpp; the same local view src/testinput.cpp uses. */
struct MixerControl { void setValue(unsigned long value); };   /* 0x412a10 */

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern int  nextListA(ListNode **cur);            /* 0x4182e0 */

extern int  g_speedPercent;                       /* 0x1b2e890 (src/panels.cpp) */
extern int  g_1b2e914;                            /* src/panels.cpp */
extern int  g_1b2eb6c;                            /* src/ez2acapp.cpp */
extern int  g_1b2f06c;
extern int  g_mixStyle;                           /* 0x1b2ebc8 (src/panels.cpp) */
extern int  g_modeIndex;                          /* 0x1b2e548 (src/modename.cpp) */
extern char g_songInfoName[64];                   /* 0x1b2eb80 (src/panels.cpp) */
extern int  g_bassBoostOn;                        /* 0x1b2e980 */
extern int  g_trebleBoostOn;                      /* 0x1b2e988 */
extern int  g_bgBoostOn;                          /* 0x1b2e9d8 */
extern int  g_playBoostOn;                        /* 0x1b2e9e0 */

// FUNCTION: EZ2AC_UNPACKED 0x00422f20
void EZ2DJMainGameDirector::m422f20(int key, int on)
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
        if (g_mixStyle != 7)
        {
            if (g_1b2e914 >= 0x15)
                g_1b2e914 = 0x14;
        }
        else
        {
            if (g_1b2e914 >= 4)
                g_1b2e914 = 3;
        }
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

    if (key == 0x20)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m421ed0(n, 0, fd14);
        // STRING: EZ2AC_UNPACKED 0x0048f8c8
        Log("FLIP ENABLE");
        return;
    }
    if (key == 0x21)
    {
        n = 0;
        while (nextListA(&n) != 0)
            reverseLanes(n, 0, fd14);
        Log("FLIP ENABLE");
        return;
    }

    if (key == 4)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m4219c0(n, 0, fd14, 1);
        return;
    }
    if (key == 0x1d)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m421750(n, 0, fd14, 1);
        return;
    }

    if (key == 0x16)
    {
        n = 0;
        while (nextListA(&n) != 0)
        {
            if (g_modeIndex != 5 && g_modeIndex != 9)
            {
                if (g_modeIndex != 7 && g_modeIndex != 3)
                {
                    reverseLanes(n, 0, fd14);
                }
                else
                {
                    m422410(n, 0, fd14);
                    m422870(n, 0, fd14);
                }
            }
            else
            {
                ((SlotOwner *)this)->m4226f0(n, 0, fd14);
            }
        }
        return;
    }

    if (key == 0x1f)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m4221b0(n, 0, fd14);
        return;
    }
    if (key == 0x17)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m4219c0(n, 0, fd14, 0);
        return;
    }
    if (key == 0x1e)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m421750(n, 0, fd14, 0);
        return;
    }

    if (key == 0x19)
    {
        n = 0;
        while (nextListA(&n) != 0)
        {
            /* Both arms are the same call; cl merges it and keeps the two
               push sequences, which is what the original has. */
            if (g_modeIndex == 5 || g_modeIndex == 9)
                m422410(n, 0, fd14);
            else
                m422410(n, 0, fd14);
        }
        return;
    }
    if (key == 0x1a)
    {
        n = 0;
        while (nextListA(&n) != 0)
            m422870(n, 0, fd14);
        return;
    }

    if (key == 3 || key == 5)
    {
        /* No return here: the original falls through into the 0x1b/0x1c/
           0x26/0x27 tests, none of which can match. */
        n = 0;
        while (nextListA(&n) != 0)
            m421c30(n, key == 3, 0, fd14);
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
        return;
    }

    if (key == 0x26)
    {
        f354 = 0x13;
        if (g_bgBoostOn != 0)
        {
            do
            {
                notify->v13(f354, 0x7f);
                f354 += 1;
            } while (f354 < 0x40);
            // STRING: EZ2AC_UNPACKED 0x0048f6a4
            Log("BG BOOST: %s\n", "ON");
            return;
        }
        do
        {
            notify->v13(f354, 0x76);
            f354 += 1;
        } while (f354 < 0x40);
        Log("BG BOOST: %s\n", "OFF");
        return;
    }

    if (key == 0x27)
    {
        f354 = 0;
        if (g_playBoostOn != 0)
        {
            do
            {
                notify->v13(f354, 0x7f);
                f354 += 1;
            } while (f354 < 0x12);
            // STRING: EZ2AC_UNPACKED 0x0048f6b4
            Log("PLAY BOOST: %s\n", "ON");
            return;
        }
        do
        {
            notify->v13(f354, 0x76);
            f354 += 1;
        } while (f354 < 0x12);
        Log("PLAY BOOST: %s\n", "OFF");
    }
}
