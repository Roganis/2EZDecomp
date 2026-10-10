/* CatchMainGameDirector::m46e750 @0x46e750 - the Ez2Catch game screen's
   service-key handler, and the smallest of the family of three (the EZ2DJ one
   is m422f20 @0x422f20, the GF one m45f640 @0x45f640, both still stubs).

   Everything is gated on the second argument: with it zero the function does
   nothing at all. Past that it is one long `else if` ladder on the key, and
   the arms fall into four kinds.

     THE SPEED KEYS (9, 10, 0x22..0x25) move g_speedPercent and clamp it to
     [0x32, 0x3e7]. The step depends on the key - 0x19 for the coarse pair,
     1 for 0x22/0x23, 0xa for 0x24/0x25 - and every DOWNWARD arm takes one
     LESS than its step when the value is already at or above 0x3e7, so 999
     falls to 975 rather than 974. That asymmetry is in all three down arms.
     When g_1b2eb6c is non-zero those same keys move g_1b2e914 (an option
     index clamped to [0, 0x14]) instead, and key 10 additionally consults
     the cabinet name: on "11ambit" the floor is 2 rather than 0.

     THE PER-NOTE KEYS (3, 4, 5, 0x16, 0x17, 0x19, 0x1a, 0x1d) each walk list
     B and hand every node to one of the already-reconstructed per-note
     helpers with fca4 as the last argument. Keys 3 and 5 share one arm and
     differ only in the flag they pass - `key == 3` - which the original
     computes with a `sete`, so it is written here as the comparison it is.

     THE PAN KEYS (0xb, 0xc) are one call to stepPan with 1 or 0.

     THE BOOST KEYS (0x1b, 0x1c) set a mixer line from a member and log it.
     Both are gated twice: the enable global must be up, and the member itself
     must be non-zero, or nothing happens. The four magic mixer values are
     the original's own.

   Written for the port. 1,196 bytes, 379 instructions, 33 calls - it passed
   the float/ladder screen as a table of calls, and every callee it needs was
   already reconstructed. */

#include "ezscreen.h"
#include "directors.h"
#include "players.h"
#include <string.h>

/* Defined in src/mixer.cpp; the same local view src/testinput.cpp uses. */
struct MixerControl { void setValue(unsigned long value); };   /* 0x412a10 */

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern int  nextListB(ListNode **cur);            /* 0x4183b0 */

extern int  g_speedPercent;                       /* 0x1b2e890 (src/panels.cpp) */
extern int  g_1b2e914;                            /* src/panels.cpp */
extern int  g_1b2eb6c;                            /* src/ez2acapp.cpp */
extern int  g_1b2f06c;
extern char g_songInfoName[64];                   /* 0x1b2eb80 (src/panels.cpp) */
extern int  g_bassBoostOn;                        /* 0x1b2e980 */
extern int  g_trebleBoostOn;                      /* 0x1b2e988 */

// FUNCTION: EZ2AC_UNPACKED 0x0046e750
void CatchMainGameDirector::m46e750(int key, int on)
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
            // STRING: EZ2AC_UNPACKED 0x0048f8dc
            Log(" SPEED = %d", g_1b2e914);
        }
        return;
    }

    if (key == 0xa)
    {
        if (g_1b2eb6c == 0)
        {
            /* One less than the step when it is already pinned at the top. */
            if (g_speedPercent >= 0x3e7)
                g_speedPercent -= 0x18;
            else
                g_speedPercent -= 0x19;
            if (g_speedPercent <= 0x32)
                g_speedPercent = 0x32;
            return;
        }
        g_1b2e914 -= 1;
        // STRING: EZ2AC_UNPACKED 0x0048f8d4
        if (_stricmp(g_songInfoName, "11ambit") == 0)
        {
            /* This cabinet's floor is 2, not 0. */
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
        while (nextListB(&n) != 0)
            m46e3e0(n, key == 3, 0, fca4);
        return;
    }

    if (key == 4)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46e170(n, 0, fca4, 0);
        return;
    }
    if (key == 0x1d)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46df00(n, 0, fca4, 0);
        return;
    }
    if (key == 0x16)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46cf00(n, 0, fca4);
        return;
    }
    if (key == 0x17)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46e170(n, 0, fca4, 0);
        return;
    }
    if (key == 0x19)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46cfc0(n, 0, fca4);
        return;
    }
    if (key == 0x1a)
    {
        n = 0;
        while (nextListB(&n) != 0)
            m46d0f0(n, 0, fca4);
        return;
    }

    if (key == 0x1b)
    {
        /* The ON arm is the one the original lays inline, so it is the
           if-body and the OFF arm is the trailing block. */
        if (g_bassBoostOn != 0)
        {
            if (mixerB == 0)
                return;
            mixerB->setValue(0x85d4);
            // STRING: EZ2AC_UNPACKED 0x0048f678
            Log("BASS BOOST: %s\n",
                // STRING: EZ2AC_UNPACKED 0x0048f68c
                "ON");
            return;
        }
        if (mixerB == 0)
            return;
        mixerB->setValue(0x5938);
        Log("BASS BOOST: %s\n",
            // STRING: EZ2AC_UNPACKED 0x0048f688
            "OFF");
        return;
    }

    if (key == 0x1c)
    {
        if (g_trebleBoostOn != 0)
        {
            if (mixerA == 0)
                return;
            mixerA->setValue(0xc293);
            // STRING: EZ2AC_UNPACKED 0x0048f690
            Log("TREBLE BOOST: %s\n", "ON");
            return;
        }
        if (mixerA == 0)
            return;
        mixerA->setValue(0xaaaa);
        Log("TREBLE BOOST: %s\n", "OFF");
    }
}
