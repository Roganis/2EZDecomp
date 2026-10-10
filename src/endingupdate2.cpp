/* EndingDirector::update2 @0x455ce0 - the ending screen's tick.

   Every frame the sample is driven and stepped, the frame counter bumped,
   three globals cleared or set, and then one of three phases runs: phase 1
   waits 0x78 frames - or a held play button, or its first chart running out
   - before handing over to phase 2; phase 2 waits 0x32 on the second chart
   the same way before phase 3; phase 3 ramps the fade down by 10 a frame
   and, once it goes negative, clamps it to zero and hands 0 to g_1b2e844.
   The fade level is published to g_37dd06c on the way out of every path. */

#include "directors.h"
#include "obj402860.h"
#include "bigbase.h"     /* Obj40C180, g_bigInstance */

/* The sample, viewed for the two slots this drives. */
struct EndSample
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual void s4(int a);   /* slot 4  -> +0x10 */
    virtual void q05(); virtual void q06(); virtual void q07();
    virtual void q08(); virtual void q09(); virtual void q10();
    virtual void q11(); virtual void q12(); virtual void q13();
    virtual void q14(); virtual void q15(); virtual void q16();
    virtual void tick();      /* slot 17 -> +0x44 */
};

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct ObjEnd1B2E844 { virtual void q00(int v); };

extern void *g_1b2e844;      /* src/moderunner.cpp */
extern int   g_1b2ea78;      /* src/systemmenu.cpp */
extern int   g_1b2ea28;      /* src/moderunner.cpp */
extern int   g_1b2ebdc;      /* src/gamestate.cpp */
extern int   g_37dd06c;      /* 0x037dd06c (src/globals.cpp) */

// GLOBAL: EZ2AC_UNPACKED 0x01b2f06c
int g_1b2f06c;

// FUNCTION: EZ2AC_UNPACKED 0x00455ce0
void EndingDirector::update2(int /*a*/, int /*b*/)
{
    g_1b2ea78 = 0;
    ((EndSample *)m_sample)->s4(0);
    ((EndSample *)m_sample)->tick();

    f450 += 1;
    g_1b2ea28 = 0;
    g_1b2ebdc = 0;
    g_1b2f06c = 1;

    switch (f44c)
    {
    case 1:
        if ((g_bigInstance->anyPlayHeld() != 0 || m_10.isDone()) && f450 > 0x78)
        {
            f44c = 2;
            f450 = 0;
        }
        break;

    case 2:
        if ((g_bigInstance->anyPlayHeld() != 0 || m_22c.isDone()) && f450 > 0x32)
        {
            f44c = 3;
        }
        break;

    case 3:
        f454 += -10;
        if (f454 < 0)
        {
            f454 = 0;
            ((ObjEnd1B2E844 *)g_1b2e844)->q00(0);
        }
        break;
    }

    g_37dd06c = f454;
}

/* EndingDirector::update @0x455c30 - the ending screen's draw.

   The frame is cleared to black and render state 7 turned off, then one of
   three phases draws: phase 1 the first chart, phase 2 the second, phase 3
   the second again PLUS the black fade quad over the whole screen (five
   pixels of bleed on every side) at the current level, in additive blend.
   Phase 3 draws nothing at all once the level has reached zero, which is
   what hands the screen over.

   The colour is the level in the alpha byte with white underneath -
   `(level << 24) | 0xffffff`, not a table lookup.

   The three phases are a SWITCH: the original's three `sub eax,1; je` tests
   are what a switch on 1/2/3 lowers to, where any if-chain would compare. */

/* The graphics singleton, viewed for the two device wrappers it forwards. */
struct Obj40BB00
{
    void doOp7(int v);               /* 0x40a470 */
    void doOp10(unsigned int color); /* 0x40a730 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00455c30
void EndingDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);

    switch (f44c)
    {
    case 1:
        m_10.m403940();
        break;

    case 2:
        m_22c.m403940();
        break;

    case 3:
        if (f454 > 0)
        {
            m_22c.m403940();
            ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_fade, -5.0f, -5.0f,
                                                 645.0f, 485.0f,
                                                 (f454 << 24) | 0xffffff);
        }
        break;
    }
}


/* WarningDirector::update @0x45c240 - the warning screen's draw. The same
   shape as the ending's, one phase shorter: clear, force additive blend,
   black the frame, then tick the one chart. Phase 0 additionally hands the
   screen over as soon as that chart has run out; phase 1 only ticks, which
   is why it comes out as a tail jump. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c240
void WarningDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);

    switch (m_238)
    {
    case 0:
        m_18.m403940();

        if (m_18.isDone() != 0)
        {
            ((ObjEnd1B2E844 *)g_1b2e844)->q00(0);
        }
        break;

    case 1:
        m_18.m403940();
        break;
    }
}

/* DrawModeIntroDirector::update @0x46a200 - the mode-intro draw. Black the
   frame, tick the chart, and when it has run out seek back to its last row
   and tick again, so the table holds on its final frame instead of
   restarting. Render state 7 goes off around the whole thing. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a200
void DrawModeIntroDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);

    m_10.m403940();

    if (m_10.isDone() != 0)
    {
        m_10.seek(m_10.f8 - 1);
        m_10.m403940();
    }

    ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
}

/* The mode-intro sample, viewed for the three slots its tick drives. Slot 16
   reports whether the sample crossed a frame boundary; only bit 0 is read. */
struct IntroSample
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual void s4(int a);   /* slot 4  -> +0x10 */
    virtual void q05(); virtual void q06(); virtual void q07();
    virtual void q08(); virtual void q09(); virtual void q10();
    virtual void q11(); virtual void q12(); virtual void q13();
    virtual void q14(); virtual void q15();
    virtual int  stepped();   /* slot 16 -> +0x40 */
    virtual void tick();      /* slot 17 -> +0x44 */
};

/* DrawModeIntroDirector::update2 @0x46a1a0 - the mode-intro tick. Drive the
   sample, count the frames it reports, and hand the screen over once the
   chart has run out AND 460 frames have gone by. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a1a0
void DrawModeIntroDirector::update2(int /*a*/, int /*b*/)
{
    ((IntroSample *)m_sample)->tick();
    ((IntroSample *)m_sample)->s4(0);

    if ((((IntroSample *)m_sample)->stepped() & 1) != 0)
    {
        f230 += 1;
    }

    if (m_10.isDone() != 0 && f230 >= 0x1cc)
    {
        ((ObjEnd1B2E844 *)g_1b2e844)->q00(0);
    }
}


/* GameOverDirector::update2 @0x44db70 - the game-over screen's tick, the
   ending's without the fade. The sample is driven and stepped, the frame
   counter bumped, two globals cleared, and past frame 0x3c a held play
   button hands the screen straight over; failing that, frame 0xc8 exactly
   is when the shared director object is handed the sink. */
// FUNCTION: EZ2AC_UNPACKED 0x0044db70
void GameOverDirector::update2(int /*a*/, int /*b*/)
{
    g_1b2ea78 = 0;
    ((EndSample *)m_sample)->s4(0);
    ((EndSample *)m_sample)->tick();

    m_14 += 1;
    g_1b2ea28 = 0;
    g_1b2ebdc = 0;

    if (m_14 > 0x3c)
    {
        if (g_bigInstance->anyPlayHeld() != 0)
        {
            ((ObjEnd1B2E844 *)g_1b2e844)->q00(0);
            return;
        }

        if (m_14 == 0xc8)
        {
            m44d9d0();
        }
    }
}
