/* RankingDirector::update2 @0x45bc30 - the ranking screen's state
   machine, dispatching f10 through a seven-entry table:
     0 wait for the intro chart, 1 fade the rows in (f109b44 up to 255,
     then 2 - or 3 straight into name entry), 2 ramp both score
     readouts toward place*46 with the row-tick sample on every row
     crossed, 3 count the name-entry timer down (a minute per unit),
     4 fade out - and on the fade's end write the rankings back (the
     channel table only outside name entry, never with an assist device
     on), hand the radio modes and a second element to the next screen,
     stop the wrap video - 5/6 idle on the outro sample with a
     420-tick timeout (6 also drops the g_1b2ea28 latch).
   Above the switch: the outro forces 6, the radio modes and mode 0xb
   pick 5/6 off the field, a fresh service press leaves for screen 1,
   and a wrapped place pins both readouts to 92. After it: the
   name-entry input runs in state 3 (per player two-up, off the list
   head one-up), a finished timer forces the fade, the wrap video
   draws, and the fade level is published to g_37dd06c. */
#include "ezscreen.h"
#include "directors.h"
#include "bigbase.h"

/* The slots this tick uses of the four samples. */
struct RankSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
    virtual void stop();             /* +0x14 */
    virtual void q6(); virtual void q7();
    virtual void setVolume(int v);   /* +0x20 */
    virtual void q9(); virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15();
    virtual int  playing();          /* +0x40 - bit 0 while playing */
    virtual void tick();             /* +0x44 */
};

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct ObjRank1B2E844 { virtual void q00(int v); };

/* The video player's release-and-reset, modelled in src/com_wrappers.cpp. */
struct ObjB8F0 { void reset(); };

extern void *g_1b2e844;             /* src/moderunner.cpp */
extern int   g_1b2ea78;             /* src/systemmenu.cpp */
extern int   g_1b2ea28;             /* src/moderunner.cpp */
extern int   g_1b2ebdc;             /* src/gamestate.cpp */
extern int   g_37dd06c;             /* 0x037dd06c (src/globals.cpp) */
extern int   g_modeIndex;           /* 0x1b2e548 (src/modename.cpp) */
extern int   g_elemCount;           /* 0x1b2e718 */
extern int   g_1b2f024;             /* src/moderunner.cpp */
extern int   g_1b2f050;             /* 0x1b2f050 (src/rankingctor.cpp) */
extern int   g_1b2ef80;             /* 0x1b2ef80 (src/rankingctor.cpp) */
extern int   g_1b2efbc;             /* 0x1b2efbc (src/rankingctor.cpp) */
extern int   g_1b2f0a8;             /* 0x1b2f0a8 (src/rankingctor.cpp) */
extern int   countListA();          /* 0x418310 (src/nodelists.cpp) */
extern int   countListC();          /* 0x4184a0 (src/nodelists.cpp) */
extern int   getListAF8();          /* 0x418340 (src/nodelists.cpp) */
extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0045bc30
void RankingDirector::update2(int a, int b)
{
    int mode, i;

    g_1b2ea78 = 0;
    ((RankSnd *)f109d98)->tick();
    if (g_1b2ea28 == 1 || g_1b2ebdc == 1)
        f10 = 6;
    mode = g_modeIndex;
    if (mode == 6 || mode == 7 || mode == 8 || mode == 9)
    {
        if (countListA() == 2)
            f10 = 6;
    }
    else if (mode != 0xb)
    {
        /* The countListA arm first: the original falls through into it and
           sends mode 0xb's countListC out of line, back into this tail. */
        if (countListA() == 2)
        {
            if (g_elemCount > 1)
            {
                if (g_elemCount == 2)
                    f10 = 6;
            }
            else
                f10 = 5;
        }
    }
    else if (countListC() == 2)
    {
        if (g_elemCount > 1)
        {
            if (g_elemCount == 2)
                f10 = 6;
        }
        else
            f10 = 5;
    }
    if (f10 < 5)
    {
        if (countListA() == 1)
        {
            if (g_1b2ea28 == 0 || g_1b2ebdc == 0)
                ((RankSnd *)f109d84)->play(1);
        }
        if (countListA() > 0)
            ((RankSnd *)f109d84)->tick();
    }
    if (g_bigInstance->getState(0) == 2)
        ((ObjRank1B2E844 *)g_1b2e844)->q00(1);
    if (f10 < 5)
    {
        if (f85df8 != 0)
        {
            if (f85dfc >= 99)
            {
                f109b58 = 92.0f;
                f109b60 = 92.0f;
            }
        }
        else if (f109b48 >= 99)
        {
            f109b58 = 92.0f;
            f109b60 = 92.0f;
        }
    }
    switch (f10)
    {
    case 0:
        if (m_14.isDone() != 0)
            f10 = 1;
        break;
    case 1:
        f109b44 += 10;
        if (f109b44 > 255)
        {
            f109b44 = 255;
            f10 = (g_1b2f024 != 0) + 2;
        }
        break;
    case 2:
        for (i = 0; i < 2; i++)
        {
            int cur10 = (int)(&f109b58)[i] / 46;
            int place;

            if (f85df8 != 0)
                place = (&f85dfc)[i];
            else
                place = (&f109b48)[i];
            if (place < 99 && f109dd0 == 0)
            {
                float target = (float)place * 46.0f;

                (&f109b60)[i] = target;
                if (target * 0.5f > (&f109b58)[i])
                    (&f109b58)[i] = (&f109b58)[i] * 1.5f + 0.2f;
                else
                    (&f109b58)[i] = (target - (&f109b58)[i]) * 0.5f +
                                    (&f109b58)[i];
            }
            if ((double)((&f109b60)[i] - (&f109b58)[i]) < 0.5)
                (&f109b58)[i] = (&f109b60)[i];
            if (cur10 != (int)(&f109b58)[i] / 46)
            {
                ((RankSnd *)f109d88)->stop();
                ((RankSnd *)f109d88)->play(0);
            }
        }
        if (f109b58 == f109b60 && f109b5c == f109b64)
            f10 = 3;
        break;
    case 3:
        f109de0++;
        if (f109de0 == 60)
        {
            f109ddc--;
            f109de0 = 0;
            if (f109ddc == 0)
            {
                ((RankSnd *)f109d94)->stop();
                ((RankSnd *)f109d94)->play(0);
                f109dd4 = 1;
                f109dd0 = 1;
            }
        }
        break;
    case 4:
        f109da4 -= 10;
        if (f109da4 < 0)
        {
            f109da4 = 1;
            if (g_1b2f050 == 0 && g_1b2ef80 == 0 && g_1b2efbc == 0)
            {
                if (g_1b2f024 == 0)
                    m4570a0();
                m456b20();
            }
            f10 = 5;
            mode = g_modeIndex;
            if (mode == 7 || mode == 6 || mode == 8 || mode == 9)
            {
                if ((((RankSnd *)f109d94)->playing() & 1) == 0)
                    ((ObjRank1B2E844 *)g_1b2e844)->q00(0);
                f10 = 4;
            }
            if (g_elemCount >= 2)
            {
                if ((((RankSnd *)f109d94)->playing() & 1) == 0)
                    ((ObjRank1B2E844 *)g_1b2e844)->q00(0);
                f10 = 4;
            }
            if (m_1748.isRunning() == 1)
                ((ObjB8F0 *)&m_1748)->reset();
        }
        if (countListA() > 0)
            ((RankSnd *)f109d84)->setVolume((f109da4 - 255) * 10000 / 300);
        break;
    case 5:
        f109da4 = 255;
        f109b44 = 255;
        ((RankSnd *)f109d98)->play(0);
        if ((((RankSnd *)f109d98)->playing() & 1) != 0)
            f109d9c++;
        if (f109d9c >= 420)
            ((ObjRank1B2E844 *)g_1b2e844)->q00(0);
        break;
    case 6:
        g_1b2ea28 = 0;
        ((RankSnd *)f109d98)->play(0);
        if ((((RankSnd *)f109d98)->playing() & 1) != 0)
            f109d9c++;
        if (f109d9c >= 420)
            ((ObjRank1B2E844 *)g_1b2e844)->q00(0);
        break;
    }
    if (countListA() == 0)
        ((ObjRank1B2E844 *)g_1b2e844)->q00(1);
    if (f10 == 3)
    {
        if (g_1b2f024 != 0)
            m45aaf0(f109de4, f109de4);
        else if (countListA() == 2)
        {
            m45aaf0(0, 0);
            m45aaf0(1, 1);
        }
        else if (countListA() == 1)
            m45aaf0(0, getListAF8());
    }
    if (f10 < 5 && f109dd0 == 1 && f109dd4 == 1)
    {
        f10 = 4;
        g_1b2f0a8 = 1;
    }
    m_1748.m40b9f0(a);
    g_37dd06c = f109da4;
}
