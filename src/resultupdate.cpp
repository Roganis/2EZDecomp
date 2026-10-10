/* ResultDirector::update2 @0x451550 - the keys result screen's tick, and the
   sibling of GFResultDirector::update2 @0x467b90 (src/gfresultupdate.cpp).

   Same spine - an eight-arm state machine on the phase at +0x2c40, with case 6
   empty and a shared tail that bumps the counter and publishes the fade - but
   it is NOT a copy, and the differences are the interesting part:

     - a PROLOGUE the GF copy has no equivalent of: cabinet input 0x26 forces
       the phase to 1 outright, and while g_1b2eb44 is up the screen pokes
       g_1b2e844's slot 0 every frame;
     - a THIRD sound object at +0x28fc, used only on the g_1b2eb6c-gated path
       of case 1, where the GF copy has just the two;
     - its own helper m451180 @0x451180 in place of m467900 - but it ALSO
       calls m467900 itself at 0x467900, so the two classes really do share
       that one, ICF-folded;
     - the all-combo check at +0x2c3c/+0x10..0x18, which the GF copy does not
       have at all;
     - and case 4 gates the outro on m_2904's frame reaching 0x19 through
       +0x1114, where GF reads its own chart.

   The field offsets are this class's own: +0x2c40/+0x2c44/+0x2c48 for the
   phase, fade and counter against GF's +0x2910/+0x2914/+0x2918, and the frame
   marks at +0x3100/+0x3104/+0x3108/+0x3110 against GF's +0x2db8 family.

   TWO ARGUMENTS, NOT ONE. The stub declared `update2(int)` and the original
   ends `ret 8`, so it takes the same (int, int) as every other screen's - the
   declaration in include/directors.h is corrected with it. */

#include "ezscreen.h"
#include "directors.h"
#include "bigbase.h"
#include "obj402860.h"

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern int  countListA();                         /* 0x4184a0 */

/* src/com_wrappers.cpp's view of the video object, for its release-and-reset
   @0x40b8f0 - the same storage as m_2408, a different set of members. */
struct ObjB8F0 { void reset(); };
extern void *g_1b2e844;                           /* src/moderunner.cpp */
extern int  g_1b2ea78, g_1b2ea28, g_1b2eb6c, g_1b2e7d8;
extern int  g_modeIndex;                          /* 0x1b2e548 (src/modename.cpp) */
extern int  g_1b2eb44, g_1b5f19c, g_1b2e704, g_37dd06c;

/* The screen's sound objects, viewed for the slots this tick uses - the same
   shape GFResultDirector's tick views its own through. */
struct ResSound
{
    virtual void q00(); virtual void q01(); virtual void q02(); virtual void q03();
    virtual void s4(int a);   /* slot 4  -> +0x10 */
    virtual void s5();        /* slot 5  -> +0x14 */
    virtual void q06(); virtual void q07(); virtual void q08(); virtual void q09();
    virtual void q10(); virtual void q11(); virtual void q12(); virtual void q13();
    virtual void q14(); virtual void q15();
    virtual int  busy();      /* slot 16 -> +0x40 */
    virtual void tick();      /* slot 17 -> +0x44 */
};

/* Whatever g_1b2e844 points at; only slot 0 is reached, always with one int. */
struct ResNotify { virtual void q00(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x00451550
void ResultDirector::update2(int, int)
{
    ((ResSound *)m_28e0)->tick();

    if (m_ef0.isDone())
        m_ef0.seek(0x5f);

    m4512b0();

    /* The cabinet can force the screen straight into phase 1. */
    if (g_bigInstance->getState(0x26) == 2)
        f2c40 = 1;
    if (g_1b2eb44 == 1)
        ((ResNotify *)g_1b2e844)->q00(0);

    switch (f2c40)
    {
    case 0:
        g_1b2ea78 += 1;
        f2c44 += 10;
        if (f2c44 > 0xfe)
        {
            f2c44 = 0xff;
            f2c40 = 1;
        }
        break;

    case 1:
        if (f2c48 == f3100)
        {
            if (countListA() == 1)
            {
                if (g_1b2ea28 != 1)
                {
                    /* The third sound object is the CV2 one. */
                    if (g_1b2eb6c != 0)
                    {
                        ((ResSound *)m_28fc)->s4(0);
                        m451180();
                    }
                    else
                    {
                        ((ResSound *)m_28e4)->s4(0);
                        m451180();
                    }
                }
                else
                {
                    ((ResSound *)m_28e8)->s4(0);
                }
            }
            else
            {
                if (g_1b2e7d8 != 2)
                    ((ResSound *)m_28e4)->s4(0);
                else
                    ((ResSound *)m_28e8)->s4(0);
                m451180();
                Log("Set Bonus Battle Play");
            }
        }

        /* The all-combo announce, which the GF copy has no equivalent of. */
        if (g_1b2eb6c != 0 && (f10 == 1 || f14 == 1 || f18 == 1) &&
            f2c48 == 0xaa)
        {
            m_2c3c = 1;
            m451180();
        }

        if (countListA() == 1)
        {
            /* NO break here: the original falls through into the next
               test (0x451717), so a skip press also runs the m419780 check
               below - which, with f2c48 just moved to the end, fires too. */
            if (g_bigInstance->getState(0x26) == 2)
            {
                m467900();
                f2c48 = f3108 - 1;
                m467900();
                if (g_1b2eb6c == 0 || m_2c3c == 0)
                    f2c40 = 7;
            }
            if (g_bigInstance->m419780() != 0 && f2c48 >= 0x9e)
            {
                m467900();
                f2c48 = f3108 - 1;
                m467900();
                if (g_1b2eb6c == 0 || m_2c3c == 0)
                    f2c40 = 7;
                break;
            }
            if (f2c48 >= f3108 - 1)
            {
                f2c48 = f3104 - 1;
                f2c40 = 7;
            }
            break;
        }
        if (countListA() == 2 && f2c48 >= 0x9e)
        {
            if (f2c48 < 0x258 && g_bigInstance->m419780() == 0)
                break;
            f2c48 = 0;
            f2c40 = 4;
        }
        break;

    case 2:
        m451180();
        if (g_bigInstance->m419780() == 0 && f2c48 < f3108 - 1)
            break;
        m467900();
        f2c48 = f3108 - 1;
        f2c40 = 7;
        break;

    case 3:
        m451040();
        if (g_bigInstance->m419780() == 0 && f2c48 < f3110)
            break;
        m467900();
        m4510a0();
        /* ONE copy of the `f2c40 = 7; s4(0)` exit, store first: the
           original reaches it from both the mode-1 two-player test and the
           fall-through (writing it in both places kept two copies, 7
           instructions longer than the original). */
        if (g_modeIndex != 1 || countListA() != 2)
        {
            if (winner != -1 || loser == -2)
            {
                f2c48 = 0;
                f2c40 = 4;
                ((ResSound *)m_28f0)->s4(0);
                break;
            }
        }
        f2c40 = 7;
        ((ResSound *)m_28f0)->s4(0);
        break;

    case 4:
        if (m_89c.isDone())
            m_89c.seek(0x78);
        if (m_110c.f8 == 0x19)
        {
            ((ResSound *)m_2904)->s5();
            ((ResSound *)m_2904)->s4(0);
        }
        if (g_bigInstance->m419780() == 0 || f2c48 < 0x3c)
        {
            if (f2c48 >= 0x1e0)
                f2c40 = 7;
        }
        else
        {
            f2c40 = 7;
        }
        break;

    case 5:
        m450fa0();
        if (m_89c.isDone())
            m_89c.seek(0x78);
        if (g_bigInstance->m419780() != 0)
        {
            m467900();
            f2c40 = 7;
            ((ResSound *)m_28f0)->s4(0);
            break;
        }
        if (f2c48 >= 0x30c)
        {
            f2c40 = 7;
            ((ResSound *)m_28f0)->s4(0);
        }
        break;

    case 6:
        break;

    case 7:
        if (countListA() == 1)
        {
            if ((((ResSound *)m_28f0)->busy() & 1) == 0 &&
                (((ResSound *)m_28f4)->busy() & 1) == 0 &&
                m_2c90 == 0)
            {
                if (g_1b2ea28 == 0)
                    ((ResSound *)m_28f0)->s4(0);
                else
                    ((ResSound *)m_28f4)->s4(0);
                m_2c90 = 1;
            }
        }
        else if ((((ResSound *)m_28f0)->busy() & 1) == 0 &&
                 (((ResSound *)m_28f4)->busy() & 1) == 0 &&
                 m_2c90 == 0)
        {
            if (g_1b2e7d8 <= 1)
                ((ResSound *)m_28f0)->s4(0);
            else
                ((ResSound *)m_28f4)->s4(0);
            m_2c90 = 1;
        }

        ((ResSound *)m_28e0)->s5();
        f2c44 += -10;
        if (f2c44 < 1)
        {
            f2c44 = 1;
            g_1b5f19c = 0;
            g_1b2e704 = 0;
            if ((((ResSound *)m_28f4)->busy() & 1) == 0 &&
                (((ResSound *)m_28f0)->busy() & 1) == 0)
                ((ResNotify *)g_1b2e844)->q00(0);
        }
        /* The release-and-reset at 0x40b8f0 - a different view of the same
           object, so it is reached by cast the way com_wrappers.cpp models it. */
        if (m_2408.isRunning() == 1)
            ((ObjB8F0 *)&m_2408)->reset();
        break;
    }

    f2c48 += 1;
    g_37dd06c = f2c44;
}
