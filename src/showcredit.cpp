#include <stdio.h>
#include "ezscreen.h"
#include "ez2acapp.h"
#include "bigbase.h"   /* g_bigInstance */
#include "loader.h"    /* KTexture */ /* KGraphics, g_sub1b2f054 */

/* ShowCredit - the credits/announcement overlay (KRuntimeClass "ShowCredit"
   @0x4ae108). Only its placement is reconstructed. */

/* The two -1.0f sentinels mean "work it out from the sprite": x centres the
   sprite in a 640-wide screen, y sits it 23 px above the bottom of a 480-tall
   one. Everything after that is fixed offsets off the resulting corner, so the
   overlay's parts follow it.

   The y bias is NEGATIVE five and it is SUBTRACTED. cl does not fold the double
   negative away, so the pool records which form the source used - and the
   constant has to stay an anonymous literal: giving it a name puts a symbol on
   it and the operand then prints as data rather than as the float. */

// FUNCTION: EZ2AC_UNPACKED 0x0044fa50
void ShowCredit::setPos(float px, float py)
{
    if (px == -1.0f)
    {
        x = (640.0f - ((KGraphics *)g_sub1b2f054)->widthOf(sprite)) * 0.5f;
    }
    else
    {
        x = px;
    }

    if (py == -1.0f)
    {
        y = 480.0f - (((KGraphics *)g_sub1b2f054)->heightOf(sprite) + 23.0f);
    }
    else
    {
        y = py;
    }

    y = y - -5.0f;
    x130 = x + 130.0f;
    y8 = y + 8.0f;
    x150 = x + 150.0f;
    y9 = y + 9.0f;
    x168 = x + 168.0f;
    y9b = y + 9.0f;
    f38 = 64.0f;
}

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classShowCredit;   /* 0x4ae108 */
extern EzObjectInfo  g_info_4ae118;     /* 0x4ae118 */

/* Which side of a two-player game is being credited (src/ez2acapp.cpp). */
extern int g_1b2eadc;
extern int countListA();                /* 0x418310 (src/nodelists.cpp) */
extern int countListC();                /* 0x4184a0 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0044f930
KRuntimeClass *ShowCredit::getRuntimeClass()
{
    return &classShowCredit;
}

// FUNCTION: EZ2AC_UNPACKED 0x0044f940
DrawGroup *ShowCredit::getObjectInfo()
{
    return &g_info_4ae118;
}

/* Hand back whichever of the two samples was loaded, then blank the charts that
   belong to a side that is not playing on - the first branch clears both, the
   second only the middle one. */
// FUNCTION: EZ2AC_UNPACKED 0x0044f950
ShowCredit::~ShowCredit()
{
    if (sampleA != 0)
    {
        g_sub1b2eae4->freeSample(sampleA);
    }
    if (sampleB != 0)
    {
        g_sub1b2eae4->freeSample(sampleB);
    }
    if (g_1b2eadc == 0 && countListA() < 2)
    {
        m_160.clear();
        m_37c.clear();
    }
    if (g_1b2eadc == 2 && countListC() < 2)
    {
        m_37c.clear();
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00450020
// ShowCredit::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int ShowCredit::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00495068
struct ShowCredit;

extern int  getListAF8();     /* 0x418340 (src/nodelists.cpp) */
extern int  getListCF8();     /* 0x4184d0 (src/nodelists.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */

/* Two identical halves - System\Common and System\CV2Mix\Common - each loading
   the same five things: the freeplay-or-credits plate (which one depends on
   whether the cabinet needs coins), two eleven-texture series, ten digits and a
   mask, plus two coin samples. Between them sit the three "N PLAYER WAIT"
   charts, each behind its own side-and-count guard. */
// FUNCTION: EZ2AC_UNPACKED 0x0044fc30
ShowCredit::ShowCredit()
{
    Obj402860Head *res;
    char path[260];
    char name[260];
    int i;

    // STRING: EZ2AC_UNPACKED 0x00490178
    changeDir("System\\Common", 1);
    if (g_bigInstance->need == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00495238
        ((KGraphics *)g_sub1b2f054)->loadTexture("Freeplay.bmp",
                                                 (KTexture **)&sprite, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049522c
        ((KGraphics *)g_sub1b2f054)->loadTexture("Credits.bmp",
                                                 (KTexture **)&sprite, 0);
        // STRING: EZ2AC_UNPACKED 0x00495220
        loadSeries(seriesS, "Credits_S", 11, (KGraphics *)g_sub1b2f054);
        // STRING: EZ2AC_UNPACKED 0x00495214
        loadSeries(seriesSb, "Credits_Sb", 11, (KGraphics *)g_sub1b2f054);
    }
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00495204
        sprintf(name, "Credits_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, (KTexture **)&digits[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004951f0
    ((KGraphics *)g_sub1b2f054)->loadTexture("Credits_mask.bmp",
                                             (KTexture **)&maskTex, 0);
    // STRING: EZ2AC_UNPACKED 0x004951e4
    sampleA = (void *)g_sub1b2eae4->createSample("coin0.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x004951d8
    sampleB = (void *)g_sub1b2eae4->createSample("coin1.wav", 0, 0);
    b10 = 0;
    if (g_1b2eadc == 0 && countListA() < 2)
    {
        // STRING: EZ2AC_UNPACKED 0x004951c4
        sprintf(path, "%dPLAYERWAIT.str", getListAF8() != 0 ? 1 : 2);
        ((KGraphics *)g_sub1b2f054)->loadStr(path, &res);
        m_37c.attach(res, 1);
    }
    if (g_1b2eadc == 2 && countListC() < 2)
    {
        sprintf(path, "%dPLAYERWAIT.str", getListCF8() != 0 ? 1 : 2);
        ((KGraphics *)g_sub1b2f054)->loadStr(path, &res);
        m_37c.attach(res, 1);
    }
    popDirectory();
    if (g_1b2eadc == 0 && countListA() < 2)
    {
        // STRING: EZ2AC_UNPACKED 0x0049519c
        sprintf(path, "system\\cv2mix\\common\\%dPLAYERWAIT.str",
                getListAF8() != 0 ? 1 : 2);
        ((KGraphics *)g_sub1b2f054)->loadStr(path, &res);
        m_598.attach(res, 1);
    }
    if (g_bigInstance->need == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00495178
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\cv2mix\\common\\Freeplay.bmp",
                                                 (KTexture **)&cvSprite, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00495154
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\cv2mix\\common\\Credits.bmp",
                                                 (KTexture **)&cvSprite, 0);
        // STRING: EZ2AC_UNPACKED 0x00495134
        loadSeries(cvSeriesS, "system\\cv2mix\\common\\Credits_S", 11,
                   (KGraphics *)g_sub1b2f054);
        // STRING: EZ2AC_UNPACKED 0x00495114
        loadSeries(cvSeriesSb, "system\\cv2mix\\common\\Credits_Sb", 11,
                   (KGraphics *)g_sub1b2f054);
    }
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004950f0
        sprintf(name, "system\\cv2mix\\common\\Credits_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, (KTexture **)&cvDigits[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004950c8
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\cv2mix\\common\\Credits_mask.bmp",
                                             (KTexture **)&cvMaskTex, 0);
    // STRING: EZ2AC_UNPACKED 0x004950a8
    cvSampleA = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\Common\\coin0.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00495088
    cvSampleB = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\Common\\coin1.wav", 0, 0);
    credits = g_bigInstance->have;
    setPos(-1.0f, -1.0f);
    f14 = 1;
}

extern int g_1b2eb6c;      /* 0x1b2eb6c (src/ez2acapp.cpp) - the CV2 skin gate */
extern int g_shownCredits; /* 0x1b5f2e8 (src/globals.cpp) */

/* The credit sounds, seen through the two slots this reaches. */
struct CreditSound
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);   /* slot 4 -> +0x10 */
    virtual void stop();           /* slot 5 -> +0x14 */
};

/* ShowCredit::update2 @0x44fb40 - play the coin or credit sound when the
   counter moves, then publish it.

   Nothing happens at all if the coins-per-credit setting is zero, or if the
   engine's coin count has not risen since the last tick. When it has, the
   sound depends on whether this coin COMPLETED a credit: an exact multiple
   of the setting plays the credit sound, anything else the coin sound. The
   CV2 skin has its own pair, chosen by g_1b2eb6c - and note the null guard
   is on the DEFAULT sound of the pair either way, so a missing CV2 sample
   would be played rather than skipped.

   Each of the four arms stops its own object and then plays it, and cl
   cross-jumps the four plays into the one the original has, each arm
   re-reading its member before jumping.

   Twenty credits is the ceiling: at that point both the published count and
   the engine's own are zeroed rather than left to climb. The published copy
   is what everything else reads. */
// FUNCTION: EZ2AC_UNPACKED 0x0044fb40
void ShowCredit::update2(int, int)
{
    if (g_bigInstance->need == 0)
        return;

    if (credits < g_bigInstance->have)
    {
        if (g_bigInstance->have % g_bigInstance->need == 0)
        {
            if (sampleB != 0)
            {
                if (g_1b2eb6c == 0)
                {
                    ((CreditSound *)sampleB)->stop();
                    ((CreditSound *)sampleB)->play(0);
                }
                else
                {
                    ((CreditSound *)cvSampleB)->stop();
                    ((CreditSound *)cvSampleB)->play(0);
                }
            }
        }
        else if (sampleA != 0)
        {
            if (g_1b2eb6c == 0)
            {
                ((CreditSound *)sampleA)->stop();
                ((CreditSound *)sampleA)->play(0);
            }
            else
            {
                ((CreditSound *)cvSampleA)->stop();
                ((CreditSound *)cvSampleA)->play(0);
            }
        }
    }

    if (g_bigInstance->have / g_bigInstance->need >= 19)
    {
        g_shownCredits = 0;
        g_bigInstance->have = 0;
    }
    else
    {
        g_shownCredits = g_bigInstance->have;
    }

    credits = g_bigInstance->have;
}
