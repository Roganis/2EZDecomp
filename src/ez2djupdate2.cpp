/* EZ2DJMainGameDirector::update2 @0x4249e0 - the keys game's per-frame
   tick. In order: wait for the song-select overlay's go-ahead and run
   start() once; under the pause latch g_1b2ec14 walk the CV2 lamp wave;
   route the key channels for the mode (radio modes light everything,
   mode 0 and ClubMix-less mode 0xc split by player, 5KeyMix keeps the
   split of its own); pump the clap/boo keys (0x24/0x25 - a press or
   three long holds steps the audience track and logs it); publish the
   latch and pump the sampler; leave on the service press (screen 2
   with g_1b2eb44 up, else 0) or five seconds of both start keys; step
   past the song when nothing is held (m424830, or screen 3 mid-
   teardown); publish tracksIdle, count the overlay cooldown down,
   rebase list A; run the assist committer under any device; blink
   output 4 on the beat (two-thirds duty); run the right input sweep -
   demo play pays sustains and leaves after 30 seconds or when the big
   base runs dry - and ramp the end-of-round fade with its volume,
   the CV2 arm handing a second element straight on; latch the
   audience-panel verdict; force the leave after 120 ramp ticks; hand
   the live scores to the overlay; count the pause menu and release
   notify slot 8 when its budget lapses; and stamp the round clock. */
#include <string.h>
#include "ezscreen.h"
#include "players.h"
#include "bigbase.h"

extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */
extern void      *g_1b2e844;        /* src/moderunner.cpp */
extern int   g_1b2ea28;             /* src/moderunner.cpp */
extern int   g_1b2e898;             /* src/maingame.cpp */
extern int   g_1b2ec14;             /* src/gfupdate2.cpp */
extern int   g_mixStyle;            /* 0x1b2ebc8 (src/panels.cpp) */
extern int   g_modeIndex;           /* 0x1b2e548 (src/modename.cpp) */
extern int   g_demoPlay;            /* 0x1b2ef78 (src/panels.cpp) */
extern char  g_styleTag[36];        /* 0x1b2e5ac (src/maingame.cpp) */
extern int   g_1b2eb44;             /* src/gamestate.cpp */
extern int   g_1b5f2e0;             /* src/holdtick.cpp */
extern int   g_1b2f050;             /* 0x1b2f050 (src/rankingctor.cpp) */
extern int   g_1b2ef80;             /* 0x1b2ef80 (src/rankingctor.cpp) */
extern int   g_1b2efbc;             /* 0x1b2efbc (src/rankingctor.cpp) */
extern int   g_1b2eb6c;             /* src/ez2acapp.cpp */
extern int   g_1b2eba4;             /* 0x01b2eba4 (src/maingame.cpp) */
extern int   g_elemCount;           /* 0x1b2e718 */
extern int   g_useAudiencePanel;    /* 0x1b2eb48 (src/maingame.cpp) */
extern int   g_1b5f184;             /* src/gamepanels.cpp */
extern int   g_1b2ef88;             /* src/globals.cpp */
extern int   g_1b2ea10;             /* src/globals.cpp */
extern int   g_1b2f08c;             /* src/gamestate.cpp */
extern int          countListA();           /* 0x418310 (src/nodelists.cpp) */
extern ListNode    *findListA(int key);     /* 0x418e30 (src/nodelists.cpp) */
extern void         rebaseListA();          /* 0x41fae0 (src/accessors.cpp) */
extern unsigned int SatSub(unsigned int a, unsigned int b);  /* 0x4145d0 */
extern void         Log(const char *fmt, ...);   /* 0x402880 (folded) */

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct ObjDj1B2E844 { virtual void q00(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x004249e0
void EZ2DJMainGameDirector::update2(int a, int b)
{
    int mode;

    if (pending != 0)
    {
        if (pending->ready == 0)
            return;
        if (ready == 0)
        {
            g_1b2ea28 = 0;
            g_1b2e898 = 0;
            ready = 1;
            start();
        }
    }
    if (g_1b2ec14 == 1)
    {
        if (f358 < 0x3f)
            f35c = 1;
        else
            f35c = 0;
        if (g_mixStyle == 7 && f35c != 0)
        {
            f354 = 0;
            do
            {
                notify->v13(f354, 0x76);
                // STRING: EZ2AC_UNPACKED 0x0048fafc
                Log("Set Track : %d", f354);
                f358 += 1;
                f354 += 1;
            }
            while (f354 < 0x40);
        }
        mode = g_modeIndex;
        if (mode == 5 || mode == 4 || mode == 8 || mode == 9)
        {
            g_bigInstance->m4189e0(g_demoPlay == 0);
            g_bigInstance->m418a50(g_demoPlay == 0);
            g_bigInstance->m418af0(g_demoPlay == 0);
        }
        else
        {
            g_bigInstance->m4189e0(g_demoPlay == 0);
            mode = g_modeIndex;
            if (mode == 0)
            {
                if (findListA(0) != 0)
                    g_bigInstance->m418b90(g_demoPlay == 0);
                if (findListA(1) != 0)
                    g_bigInstance->routeOutputs(g_demoPlay == 0);
            }
            else if (mode == 0xc)
            {
                // STRING: EZ2AC_UNPACKED 0x0048faf4
                if (_stricmp("5KeyMix", g_styleTag) == 0)
                {
                    if (findListA(0) != 0)
                        g_bigInstance->m418b90(g_demoPlay == 0);
                    if (findListA(1) != 0)
                        g_bigInstance->routeOutputs(g_demoPlay == 0);
                }
                else
                {
                    if (findListA(0) != 0)
                        g_bigInstance->m418a50(g_demoPlay == 0);
                    if (findListA(1) != 0)
                        g_bigInstance->m418af0(g_demoPlay == 0);
                }
            }
            else
            {
                if (findListA(0) != 0)
                    g_bigInstance->m418a50(g_demoPlay == 0);
                if (findListA(1) != 0)
                    g_bigInstance->m418af0(g_demoPlay == 0);
            }
        }
        if (g_bigInstance->getState(0x24) == 3)
        {
            if (g_bigInstance->heldFor(0x24) >= 150)
                f7a4 += 1;
        }
        if (g_bigInstance->getState(0x25) == 3)
        {
            if (g_bigInstance->heldFor(0x25) >= 150)
                f7a8 += 1;
        }
        if (g_bigInstance->getState(0x24) == 2 || f7a4 >= 3)
        {
            if (f7a0 <= 0x7e)
                f7a0 = f7a0 + 1;
            else
                f7a0 = 0x7f;
            f7a4 = 0;
            notify->v11(f7a0);
            // STRING: EZ2AC_UNPACKED 0x0048fae4
            Log("VOLUME UP : %d", f7a0);
        }
        if (g_bigInstance->getState(0x25) == 2 || f7a8 >= 3)
        {
            if (f7a0 >= 1)
                f7a0 = f7a0 - 1;
            else
                f7a0 = 0;
            f7a8 = 0;
            notify->v11(f7a0);
            // STRING: EZ2AC_UNPACKED 0x0048fad0
            Log("VOLUME DOWN : %d", f7a0);
        }
    }
    g_1b2ec14 = ready;
    pump.pump();
    if (g_1b2eb44 != 0)
    {
        if (g_bigInstance->getState(0) == 2)
            ((ObjDj1B2E844 *)g_1b2e844)->q00(2);
        if (g_bigInstance->getState(4) == 3 &&
            g_bigInstance->heldFor(4) >= 5000)
            ((ObjDj1B2E844 *)g_1b2e844)->q00(2);
        else if (g_bigInstance->getState(5) == 3 &&
                 g_bigInstance->heldFor(5) >= 5000)
            ((ObjDj1B2E844 *)g_1b2e844)->q00(2);
    }
    else if (g_bigInstance->getState(0) == 2)
        ((ObjDj1B2E844 *)g_1b2e844)->q00(0);
    if (notify->playState() == 0)
    {
        if (f350 != 1)
            m424830();
        else
            ((ObjDj1B2E844 *)g_1b2e844)->q00(3);
    }
    f7f0 = ((SlotOwner *)this)->tracksIdle();
    if (fc7c > 0)
        fc7c = fc7c - 1;
    rebaseListA();
    if (g_1b5f2e0 == 1 || g_1b2f050 != 0 || g_1b2ef80 != 0 ||
        g_1b2efbc != 0)
        m422d30();
    /* Lit for the first THIRD of the measure, as in the GF copy
       (0x424ecc: f34c * 0xaaaaaaab >> 33). */
    {
        unsigned int now = (unsigned int)notify->v22() % (unsigned int)f34c;

        if ((unsigned int)f34c / 3 > now)
            g_bigInstance->setOutput(4, 1);
        else
            g_bigInstance->setOutput(4, 0);
    }
    if (g_demoPlay != 0)
    {
        m420130();
        if (g_bigInstance->have != 0)
        {
            fd10 = 4;
            if (f20 == 0)
            {
                f15c->m450420(2, 10, (EZ2DJSink1 *)this);
                f20 = 1;
            }
        }
        if (SatSub(a, started) >= 30000 && f20 == 0)
        {
            f15c->m450420(2, 10, (EZ2DJSink1 *)this);
            f20 = 1;
        }
    }
    else if (g_1b5f2e0 != 0)
        m420130();
    else
    {
        m41fb20();
        m41ffa0();
    }
    if (g_1b2eb6c == 0)
    {
        if (g_1b2eba4 == 1 && countListA() != 2)
        {
ramp:
            g_1b2e898 += 1;
            f7d4 += 0x32;
            notify->v11((f7d4 - 255) * 10000 / 300);
            notify->v10();
        }
    }
    else if (g_1b2eba4 == 1)
    {
        if (g_elemCount > 1)
        {
            notify->v10();
            ((ObjDj1B2E844 *)g_1b2e844)->q00(0);
        }
        else
            goto ramp;
    }
    if (f7d4 >= 255)
        f7d4 = 255;
    if (g_useAudiencePanel == 1)
        g_1b2ea28 = (g_1b5f184 == 1);
    if (g_1b2e898 >= 0x78 && countListA() != 2)
    {
        g_1b2ea28 = 1;
        if (f20 == 0)
        {
            f15c->m450420(2, 10, (EZ2DJSink1 *)this);
            f20 = 1;
        }
        g_1b2eba4 = 0;
    }
    if (pending != 0 && countListA() != 1 && g_modeIndex != 1)
    {
        pending->f14 = ((PlayerSlot *)f380[(0x4b0 - 0x380) / 4])->f1e4;
        pending->f18 = ((PlayerSlot *)f380[(0x674 - 0x380) / 4])->f1e4;
    }
    if (g_1b2ef88 == 1)
    {
        f79c += 1;
        notify->v9();
    }
    if (f79c >= g_1b2ea10)
    {
        notify->v8();
        f79c = 0;
        g_1b2ea10 = 0;
    }
    g_1b2f08c = SatSub(a, started);
}
