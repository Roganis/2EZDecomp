/* TotalResultDirector::update2 @0x454800 - the total-result screen's tick, a
   five-arm state machine on +0x142c whose arms are laid out 0, 4, 1, 2, 3.

   The shape: the fader at +0x1190 is ticked through slot 17 every frame
   before the dispatch, and every arm falls into a shared tail that bumps the
   frame counter, hands the caller's first argument to the video member and
   publishes the fade level to a global. Arm 0 ramps the level up to 0xff and
   then hands over to arm 4; arm 4 logs the three stage song names and drops
   to arm 1; arm 1 waits for the cabinet or a fixed frame count before going
   to arm 3; arm 3 ramps the level back down and restarts the video when it
   has stopped. */

#include "directors.h"
#include "bigbase.h"     /* Obj40C180, g_bigInstance */
#include "ez2acapp.h"    /* KGraphics, g_sub1b2f054 */
#include "obj402860.h"

/* The fader, viewed for the two slots this function uses. */
struct FaderSlots
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    /* slot 4 -> +0x10: drive the fade. */
    virtual void drive(int v);
    virtual void q05(); virtual void q06(); virtual void q07();
    virtual void q08(); virtual void q09(); virtual void q10();
    virtual void q11(); virtual void q12(); virtual void q13();
    virtual void q14(); virtual void q15(); virtual void q16();
    /* slot 17 -> +0x44: the per-frame tick. */
    virtual void tick();
};

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct Obj1B2E844 { virtual void q00(int v); };

/* The release-and-reset @0x40b8f0, under the name src/com_wrappers.cpp
   defines it with. */
struct ObjB8F0 { void reset(); };

extern void *g_1b2e844;                 /* src/moderunner.cpp */
extern void Log(const char *fmt, ...);  /* 0x402880 */

extern int g_37dd06c;                   /* 0x037dd06c (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00454800
void TotalResultDirector::update2(int a, int /*b*/)
{
    ((FaderSlots *)fader)->tick();

    switch (f142c)
    {
    case 0:
        ((FaderSlots *)fader)->drive(0);
        f1430 += 10;
        if (f1430 > 0xfe)
        {
            f1430 = 0xff;
            f142c = 4;
        }
        break;

    case 4:
        // STRING: EZ2AC_UNPACKED 0x00496ee4
        Log("SongName : %s", m4544f0(0));
        Log("SongName : %s", m4544f0(1));
        Log("SongName : %s", m4544f0(2));
        f142c = 1;
        break;

    case 1:
        if (g_bigInstance->m4191c0() != 0 || f1434 == 0x438)
        {
            f142c = 3;
            ((FaderSlots *)m_119c)->drive(0);
        }
        break;

    case 2:
        ((Obj1B2E844 *)g_1b2e844)->q00(0);
        break;

    case 3:
        f1430 += -10;
        /* The floor is ONE, not zero - `cmp [f1430],ebx` against the 1 the
           prologue materialized, and the same 1 is stored back. */
        if (f1430 < 1)
        {
            f1430 = 1;
            ((Obj1B2E844 *)g_1b2e844)->q00(0);
        }
        /* And the video is torn down while it IS running. */
        if (m_cb8.isRunning() == 1)
            ((ObjB8F0 *)&m_cb8)->reset();
        break;
    }

    f1434 += 1;
    m_cb8.m40b9f0(a);
    g_37dd06c = f1430;
}


/* TotalResultDirector::update @0x4546e0 - the screen's per-frame draw.

   The video member draws, a banner goes across the top, and four tween
   tables refresh unconditionally. In the four course modes a fifth refreshes
   and restarts itself at frame 0x33 when it runs out; a sixth always does,
   at 0x5a. Finally, in phases 0 and 3 only, the same -5,-5 645x485 overlay
   the other result screens use is blitted with its alpha byte from +0x1430. */
extern int g_modeIndex;    /* 0x1b2e548 (src/modename.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004546e0
void TotalResultDirector::update()
{
    m_cb8.m40b710();

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_1408, -5.0f, -1.0f, 645.0f, 0.0f, -1);

    m_10.m403940();
    m_22c.m403940();
    m_448.m403940();
    m_664.m403940();

    if (g_modeIndex == 6 || g_modeIndex == 7 ||
        g_modeIndex == 8 || g_modeIndex == 9)
    {
        m_a9c.m403940();
        if (m_a9c.isDone())
            m_a9c.seek(0x33);
    }

    m_880.m403940();
    if (m_880.isDone())
        m_880.seek(0x5a);

    if (f142c == 0 || f142c == 3)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_11a8, -5.0f, -5.0f, 645.0f, 485.0f,
                                             (f1430 << 24) | 0xffffff);
    }
}
