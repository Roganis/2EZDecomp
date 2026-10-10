/* SongSelectDirector's mode-input handlers - the per-mode arms of the tick
   @0x446440's thirteen-way switch (see include/modeinput.h). Each one reads
   the cabinet's slot states through Obj40C180::getState, drives the panel
   lamps through setOutput, and moves the song-select cursor.

   Kept apart from src/songselect.cpp so that m4332c0 / setOptionLamps stay
   real calls here (cl would inline a body it can see). */

#include <stdio.h>
#include <string.h>
#include "ingame.h"      /* InGame::pickDistinct @0x433cf0 */
#include "loader.h"      /* Log */
#include "songselect.h"
#include "bigbase.h"
#include "ez2acapp.h"

/* src/nodelists.cpp - the joined-player list. */
struct ListNode;
extern ListNode *findListA(int key);
extern int countListA();
extern int getListAF8();

/* src/gamestate.cpp - the option counters and their gates. */
extern int g_37dcf08;
extern int g_37dcecc;
extern int g_37dcee8;
extern int g_37dced0;
extern int g_37dcec4;
extern int g_37dcf28;
extern int g_37dce9c;
extern int g_37dced4;
extern int g_37dcea4;
extern int g_37dcec8;
extern int g_37dcee4;
extern int g_37dcee0;
extern int g_1b2e588;
extern int g_1b2efd0;   /* 0x1b2efd0 (src/totalresultctor.cpp) */
extern char g_channelName[0x80];              /* 0x1b2e728 (src/songselect.cpp) */
extern char g_textSlots[32][128];             /* 0x1b2ec18 (src/modename.cpp) */
extern void setTextSlot(int slot, const char *text);  /* 0x414e30 */   /* src/modeselectctor.cpp */
extern int   g_1b2e7e0;                       /* 0x1b2e7e0 (src/globals.cpp) - inputRadioMix's legacy-mode flag */
extern float g_playerScale[2];                /* 0x1b2ef20 (src/gamepanels.cpp) */
extern float g_1b2eaec[2];                    /* 0x1b2eaec (src/globals.cpp) */
extern int   g_37dcef8;                       /* 0x37dcef8 (src/gamestate.cpp) - CV2Mix SPEED */
extern int   g_37dcf00;                       /* 0x37dcf00 (src/gamestate.cpp) */
extern int   g_1b2eb7c;                       /* 0x1b2eb7c (src/gamestate.cpp) - CV2Mix efbp toggle */
extern int   g_mixStyle;                      /* 0x1b2ebc8 (src/panels.cpp) */
extern char  g_songInfoName[64];              /* 0x1b2eb80 (src/panels.cpp) */
extern char  g_songShortName[0x40];           /* 0x1b2f004 (src/catchpanelgds.cpp) */
extern int   g_selectedKey;                   /* 0x1b2efd4 (src/globals.cpp) */
extern int g_1b2e7d0;
extern int g_1b2ea18;
extern int g_37dceb0;
extern int g_37dceac;
extern int g_37dceb4;
extern int g_1b2ef60;   /* src/maingame.cpp */

/* One full turn of the cursor wheel. A file-scope `static const`, NOT a
   literal and NOT an extern: cl loads it from its own symbol either way, but
   the register allocation and schedule of the six step blocks depend on which.
   Written as a literal (a rematerializable pool constant) the LAST step block
   on each path gets the other xmm assignment (K in xmm0, the count in xmm1);
   as an extern const the load is scheduled ahead of the conversion in every
   block; as a static const all six come out as the original has them - five
   `cvtsi2ss xmm0 / movss xmm1,K` and the player-two up block with the load
   first. All forty-six references to it in the binary are in this family. */
// GLOBAL: EZ2AC_UNPACKED 0x004916b4
static const float k_twoPi = 6.2831855f;

/* Mode 1, RubyMix: the song-select cursor.

   The cursor `f84` scrolls through `f7c` entries on either turntable passing a
   threshold or on the up/down buttons, in TWO NEAR-IDENTICAL HALVES split on
   `f7c >= 8` that differ only in their thresholds (0x14 / 0xa on the
   turntable, 0xa / 3 on the hold counters) - written out twice, as CLAUDE.md's
   duplicated-halves rule says. Then the wrap and the eased angle, the confirm
   buttons, the three option counters, the preview reload after thirty idle
   ticks, and the option readout / lamps. */
// FUNCTION: EZ2AC_UNPACKED 0x0043b8d0
void SongSelectDirector::inputRubyMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
        }
    }

    if (f7c != 0)
    {
        f84 = (f84 + f7c) % f7c;
        f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(7) == 2 && f_20c6bc >= 1)
    {
        g_37dcecc++;
        if (g_37dcecc == 4)        g_37dcecc = 5;
        else if (g_37dcecc == 8)   g_37dcecc = 9;
        else if (g_37dcecc == 0xb) g_37dcecc = 0;
    }
    if (g_bigInstance->getState(8) == 2 && f_20c6bc >= 1)
    {
        g_37dcee8++;
        if (g_37dcee8 == 4) g_37dcee8 = 0;
    }
    if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
    {
        g_37dced0++;
        if (g_37dced0 == 1)
        {
            g_37dcec4++;
        }
        else if (g_37dced0 == 3)
        {
            g_37dced0 = 0;
            g_37dcec4 = 0;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            // STRING: EZ2AC_UNPACKED 0x004939f0
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    // STRING: EZ2AC_UNPACKED 0x00491b28
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        m_20cb10.setCell(3, 0, f_bfe58);
        m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(5, 0, f_bff64[g_37dcee8]);
        m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
        {
            int a = g_37dcf28 + g_37dced0;
            int c = g_37dcf28 + g_37dcee8;
            int b = g_37dcf28 + g_37dcecc;
            g_bigInstance->setOutput(0xa, 1);
            g_bigInstance->setOutput(0xb, b != 0);
            g_bigInstance->setOutput(0xc, c != 0);
            g_bigInstance->setOutput(0xd, a != 0);
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 0, 5KeyMix. The same cursor as RubyMix's; what differs is after it:
   the option buttons work on two PAGES switched by button 6 (g_37dced4), the
   first page cycling its 0x37dcecc counter over a different set of stops and
   holding button 8 to step 0x37dcea4, the second page the counters
   0x37dcec8 / 0x37dcee4 / 0x37dcee0 / 0x37dced0 - and each page has its own
   four-row readout. No wrap guard here: 5KeyMix always has songs.

   88.39%, STRUCTURALLY COMPLETE, parked on the constant-register assignment.
   The original hoists the constant 1 into ebp for the WHOLE function (the
   lamp pushes at the top are `push ebp`, it is re-materialized right after
   the halves) and gives 5 to edi, then reuses edi as a zero register from
   the second page on; cl gives us 5 -> ebp, 1 -> edi from the confirm
   section, and immediates at the top - a pure register-name swap plus the
   missing top hoist, and every one of the 777 instructions is otherwise the
   same. The use counts of 1, 5 and 0 (45 / 8 / 48) are IDENTICAL to the
   original's, so it is not a missing use; probes show cl's choice flips on
   any single 5-use (`f_bfdc0 >= 6`, `f_bfdb4 = 6`) but nothing legitimate
   moves it, and the top hoist needs +16 stores of 1 that the original does
   not have. Two forms that ARE load-bearing: the page split is a
   `switch (g_37dced4) { default: ... case 1: ... }` (that is what emits
   `sub eax,1; je` instead of a `cmp`), and the 0x37dcecc stop list is TWO
   ifs, not if/else-if (the `= 7` arm sits inline, the `= 0` tests after). */
// FUNCTION: EZ2AC_UNPACKED 0x0043ff00
void SongSelectDirector::input5KeyMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }

            /* Inside the f_bfdb4 == 4 block: the original's f_20c6bc and
               f_bfdb4 tests both jump past the wrap and the lerp. */
            f84 = (f84 + f7c) % f7c;
            f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 4 || g_37dcecc == 5 || g_37dcecc == 6)
                    g_37dcecc = 7;
                if (g_37dcecc == 8 || g_37dcecc == 9 || g_37dcecc == 0xa || g_37dcecc == 0xb)
                    g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff54);
            {
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 11, ScratchMix. The same wheel half as input5KeyMix - identical
   instruction for instruction up to the option section - and then BOTH option
   pages rewired (twindelta.py: 0.926 semantic, 32 regions):

     - page 0 drops 5KeyMix's held-button repeat entirely. Button 7 steps
       g_37dcec8 (wrap 4), button 8 steps g_37dcecc with a wider stop list
       (3/4/5/6 fold to 7, and the last arm is `>= 0xb` rather than `== 0xb`),
       and button 9 steps g_1b2e7d0 (wrap 2);
     - page 1 steps g_1b2ea18 on button 7 instead of g_37dcec8, and its
       button-8 counter wraps at 2 with no nested g_37dcee0 step;
     - the readouts follow: page 0 shows f_bfed4/f_bfee8/f_bff18 and lights
       lamp 0xd, page 1 shows f_bff20 in cell 4. Page 1's lamp 0xb reads
       g_37dcee0, which nothing on that page steps any more - that asymmetry
       is the original's.

   f_bff18 and f_bff20 are carved out of what was modelled as the tail of
   f_bfee8; the extent is unchanged and g_37dcecc only ever reaches 0xa.

   For the port, not for a match - same family wall. */
// FUNCTION: EZ2AC_UNPACKED 0x0043ad10
void SongSelectDirector::inputScratchMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
        }
    }

    f84 = (f84 + f7c) % f7c;
    f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 3 || g_37dcecc == 4 || g_37dcecc == 5 ||
                    g_37dcecc == 6)
                    g_37dcecc = 7;
                if (g_37dcecc == 8 || g_37dcecc == 9 || g_37dcecc == 0xa ||
                    g_37dcecc >= 0xb)
                    g_37dcecc = 0;
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_1b2e7d0++;
                if (g_1b2e7d0 >= 2) g_1b2e7d0 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_1b2ea18++;
                if (g_1b2ea18 == 2) g_1b2ea18 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee4 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(6, 0, f_bff18[g_1b2e7d0]);
            {
                int d = g_37dcf28 + g_1b2e7d0;
                int c = g_37dcf28 + g_37dcecc;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bff20[g_1b2ea18]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcee0 + g_37dcf28;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 2, StreetMix. input5KeyMix again, with the same fourth first-page
   option 7StreetMix has and four differences of its own (twindelta.py: 0.955
   semantic, 24 regions, most of them the register coin):

     - the option-page toggle is additionally refused while button 4 is held;
     - the first option steps OVER two ranges rather than folding them - 4
       becomes 5 and 8 becomes 9, where 5KeyMix sends 4/5/6 to 7;
     - the second option's own button only counts while the confirm run is up
       (`f_20c6bc >= 1`), though the repeat flag still bypasses that;
     - and a cursor move clears four more counters at +0xb0..+0xbc, which no
       other reconstructed function touches - they are carved out of the pad
       in include/songselect.h.

   For the port, not for a match - same wall as the rest of the family. */
// FUNCTION: EZ2AC_UNPACKED 0x0043c2e0
void SongSelectDirector::inputStreetMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }

            /* Inside the f_bfdb4 == 4 block: the original's f_20c6bc and
               f_bfdb4 tests both jump past the wrap and the lerp. */
            f84 = (f84 + f7c) % f7c;
            f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 4) g_37dcecc = 5;
                else if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            /* The f_20c6bc test covers BOTH arms: the original's
               f_bfdc4 == 1 branch lands on it (0x43cae4), not the body. */
            if ((f_bfdc4 == 1 || g_bigInstance->getState(8) == 2) &&
                f_20c6bc >= 1)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e24 = 0;
        f_c0e20 = 0;
        m4332c0();
        f_b0 = 0;
        f_b4 = 0;
        f_b8 = 0;
        f_bc = 0;
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 5, SpaceMix. inputClubMix with three differences - the two are near
   twins (twindelta.py: 0.957 semantic, 31 regions, and ClubMix only became
   the right template for it once ClubMix itself was written; against
   input5KeyMix it scores 0.815/56):

     - the first option stops over one more range: 4 becomes 5, on top of
       ClubMix's 8-becomes-9;
     - the fourth option wraps on `>= 4` rather than `== 4`;
     - and its readout cell 6 comes from f_bff34[] rather than f_bff64[].

   Its frame is four bytes larger than ClubMix's for no source-level reason:
   the original keeps 0x14 AND 0xa in callee-saved registers here, which
   leaves none for `moved`, so that spills to a slot and pushes the sprintf
   buffer up with it. That is the family's constant-register coin showing up
   as a frame size rather than a register name. */
// FUNCTION: EZ2AC_UNPACKED 0x00441540
void SongSelectDirector::inputSpaceMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if (g_bigInstance->accum[0] > 0x14 || g_bigInstance->accum[1] > 0x14 ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x17) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            if (g_bigInstance->accum[0] < -0x14 || g_bigInstance->accum[1] < -0x14 ||
                g_bigInstance->getState(0x10) == 2 ||
                g_bigInstance->getState(0x18) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            }
            else
            {
            if (g_bigInstance->accum[0] > 0xa || g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x17) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            if (g_bigInstance->accum[0] < -0xa || g_bigInstance->accum[1] < -0xa ||
                g_bigInstance->getState(0x10) == 2 ||
                g_bigInstance->getState(0x18) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            }
        }
    }

    f84 = (f84 + f7c) % f7c;
    f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2 ||
                g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                m_sndMove2->q5();
                m_sndMove2->q4(0);
                f_bfdb4 = 5;
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 4) g_37dcecc = 5;
                else if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 0xa || g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            /* The fourth option, which input5KeyMix does not have and
               inputRubyMix wraps the same way. f_20c6bc is re-tested because
               getState is opaque to cl, not because the source doubted it. */
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 >= 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff34[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 7, RadioMix - an edit of 5RadioMix (twindelta 0.942, 41 semantic
   regions), written 2026-08-26 from the region list plus a direct decode of
   each region:

     - NO f38/f40/f3c head block and no per-chart ranking re-read - one
       loadRankingFile, in the moved block;
     - the mode-change guard tests the FLOAT f_20c6a8 == 0.0f where the
       others test f_20c6bc == 0, and its per-player button chains read the
       middle pair swapped: 0xa/0xe/0xc and 0x12/0x16/0x14;
     - the NoteArrange wrap is only == 0xb -> 0;
     - while the legacy flag g_1b2e7e0 is set, m433980 (STUB) runs every
       tick, and the moved block's preview loads from preview_legacy rather
       than preview_radio - written as if/else sprintf so cl cross-jumps
       them into the original's one call;
     - after the preview-restart block it re-arms g_playerScale[0] = 10.0f
       and g_1b2eaec[0] = 8.0f unconditionally;
     - the picker's sixth arm is 11waterflame (pool 6, table +0x173814);
       random4 and 11waterflame and the default go through setTextSlot, the
       first four arms strcpy inline.

   callcmp: CALL SEQUENCE IDENTICAL (108 calls) on the first build. */
// FUNCTION: EZ2AC_UNPACKED 0x0043cf30
void SongSelectDirector::inputRadioMix()
{
    char buf[260];
    /* FOUR ints, not eight - the same deliberate overrun into buf as the
       hub's; see input14RadioMix. */
    int  pick[4];
    int  n;
    int moved = 0;

    updateStartLamps();

    if (f_20c6bc == 0 && f_dc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4 &&
            g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
        {
            if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
            {
            if (g_bigInstance->accum[0] > 0xa ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 >= 5)
            {
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
                f_c0e20 = 0;
                f_20c0e0 = 1;
            }
            else if (g_bigInstance->accum[0] < -0xa ||
                     g_bigInstance->getState(0x10) == 2 ||
                     g_bigInstance->getState(0x1d) == 2 || f_c0e24 >= 5)
            {
                moved = 1;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                f_c0e24 = 0;
                f_20c0e0 = 0;
            }
            }
            if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
            {
            if (g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x17) == 2)
            {
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
                f_20c0e0 = 1;
            }
            else if (g_bigInstance->accum[1] < -0xa ||
                     g_bigInstance->getState(0x18) == 2)
            {
                moved = 1;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                f_20c0e0 = 0;
            }
            }
        }
    }

    f84 = (f84 + f7c) % f7c;

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6a8 == 0.0f)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0e) == 2 ||
                g_bigInstance->getState(0x0c) == 2)
            {
              if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
              {
                int v;

                m_sndMove2->q4(0);
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
              }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x16) == 2 ||
                g_bigInstance->getState(0x14) == 2)
            {
              if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
              {
                int v;

                m_sndMove2->q4(0);
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
              }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (g_1b2e7e0 != 0)
        m433980();

    if (moved == 1)
    {
        m_sndPreview->q5();
        if (g_1b2e7e0 != 0)
        {
            sprintf(buf, "system\\preview_legacy\\%s.wav", rows[f84].name);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00493310
            sprintf(buf, "system\\preview_radio\\%s.wav", rows[f84].name);
        }
        g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
        if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
        {
            m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                "System\\Preview\\0Blank.wav", 1, 0);
        }
        f_20e894 = 0;
        f_c0e20 = 0;
        f_c0e24 = 0;
        m_20d164.reset();
        m_20d380.reset();
        loadRankingFile(rows[f84].name);
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
            m_sndPreview->q4(1);
    }

    g_playerScale[0] = 10.0f;
    g_1b2eaec[0] = 8.0f;

    strcpy(g_channelName, rows[f84].name);
    if (_stricmp(g_channelName, "random") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 0);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xdb294 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random2") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 1);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xf48d4 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random3") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 2);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x10df14 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "7znpfg") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 3);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x127554 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random4") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 4);
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0x140b94 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x004936d8
    else if (_stricmp(g_channelName, "11waterflame") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 6);
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0x173814 + pick[n] * 260);
    }
    else
    {
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0xc0e44 + (f84 * 4 + n) * 260);
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }

    /* The current channel's info panel into both grids, row 1. */
    m_20d164.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
    m_20d380.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
}

/* Mode 6, 5RadioMix - the hub input14RadioMix with the per-player split put
   back. Eight differences, all read off the original (twindelta: 0.920
   semantic, 30 regions): a latch on f38/f40 that clears f3c, a second guard
   term on f_dc, the player split restored in BOTH the wheel and the mode
   change, a loadRankingFile pair where the other handlers call m4332c0 (the
   second only while f3c is clear), a SIXTH picker arm on "random4" reading
   the table at this+0x140b94, its own preview directory, and readout cell 6
   out of f_bff64 rather than f_bff34.

   Inherits the hub's register wall, so expect the same grade: decoded and
   compiled for the port, not byte-matched. See wip/radiomix-43f390.md. */
// FUNCTION: EZ2AC_UNPACKED 0x0043dc00
void SongSelectDirector::input5RadioMix()
{
    char buf[260];
    /* FOUR ints, not eight - the frame says so (0x118, and buf plus the
       cookie account for 0x108). pickDistinct writes eight, so the upper
       four land in buf; the original overruns this array on purpose or by
       accident, and reproducing the frame means reproducing that. */
    int  pick[4];
    int  n;
    int moved = 0;

    updateStartLamps();

    if (f38 != f40)
        f3c = 0;
    f40 = f38;

    if (f_20c6bc == 0 && f_dc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4 &&
            g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
        {
            if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
            {
            if (g_bigInstance->accum[0] > 0xa ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 >= 5)
            {
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
                f_c0e20 = 0;
                f_20c0e0 = 1;
            }
            else if (g_bigInstance->accum[0] < -0xa ||
                     g_bigInstance->getState(0x10) == 2 ||
                     g_bigInstance->getState(0x1d) == 2 || f_c0e24 >= 5)
            {
                moved = 1;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                f_c0e24 = 0;
                f_20c0e0 = 0;
            }
            }
            if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
            {
            if (g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x17) == 2)
            {
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
                f_20c0e0 = 1;
            }
            else if (g_bigInstance->accum[1] < -0xa ||
                     g_bigInstance->getState(0x18) == 2)
            {
                moved = 1;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                f_20c0e0 = 0;
            }
            }

            /* Inside the whole guard: the f_20c6bc, f_dc, f_bfdb4 AND the two
               getState tests all jump past this store (0x43dea5). */
            f84 = (f84 + f7c) % f7c;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
              if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
              {
                int v;

                m_sndMove2->q4(0);
                /* An int per entry at +0xdb104 - the channel level. Not
                   modelled: the row layout has nothing at this stride. */
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                // STRING: EZ2AC_UNPACKED 0x00493a08
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
              }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
              if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
              {
                int v;

                m_sndMove2->q4(0);
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
              }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 4) g_37dcecc = 5;
                else if (g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            /* The fourth option, which input5KeyMix does not have and
               here the wrap is the EQUALITY test (cmp;jne). f_20c6bc is re-tested because
               getState is opaque to cl, not because the source doubted it. */
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    /* The whole preview switch lives in the MOVED block, and the ranking
       re-read on a chart change (`f3c`, cleared at the head when f38 moved)
       is its OWN block after it - not nested inside. Same first-form defect
       as 14Radio: `moved` was never set, so cl deleted all of this. */
    if (moved == 1)
    {
        m_sndPreview->q5();
        // STRING: EZ2AC_UNPACKED 0x00493a24
        sprintf(buf, "System\\Preview_5radio\\%s.wav", rows[f84].name);
        g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
        if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
        {
            m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                "System\\Preview\\0Blank.wav", 1, 0);
        }
        f_20e894 = 0;
        m_20d164.reset();
        m_20d380.reset();
        f_c0e20 = 0;
        f_c0e24 = 0;
        loadRankingFile(rows[f84].name);
    }
    if (f3c == 0)
    {
        loadRankingFile(rows[f84].name);
        f3c = 1;
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
            m_sndPreview->q4(1);
    }

    /* The course picker. Each arm draws four distinct entries and publishes
       their names into the rolling text slots out of its own 0x19640-strided
       table; the last two go through setTextSlot (src/modename.cpp, a real
       call from here) while the first three strcpy into g_textSlots directly.
       The default arm publishes the CURRENT course's own four. */
    strcpy(g_channelName, rows[f84].name);
    // STRING: EZ2AC_UNPACKED 0x00490730
    if (_stricmp(g_channelName, "random") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 0);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xdb294 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x00490720
    else if (_stricmp(g_channelName, "random2") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 1);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xf48d4 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x00493700
    else if (_stricmp(g_channelName, "random3") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 2);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x10df14 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x004936f8
    else if (_stricmp(g_channelName, "7znpfg") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 3);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x127554 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x004936f0
    else if (_stricmp(g_channelName, "random4") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 4);
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0x140b94 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x004936e8
    else if (_stricmp(g_channelName, "11ptgc") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 5);
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0x15a1d4 + pick[n] * 260);
    }
    else
    {
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0xc0e44 + (f84 * 4 + n) * 260);
    }


    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }

    /* The current channel's info panel into both grids, row 1 - the two
       grids the moved block resets. Same tail as 14Radio. */
    m_20d164.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
    m_20d380.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
}

/* Mode 8, 10RadioMix - an edit of the 14RadioMix hub (twindelta 0.931, 22
   semantic regions), written 2026-08-26 from the region list plus a direct
   decode of each region:

     - the NoteArrange wrap drops the == 4 -> 5 skip (8 -> 9 and 0xa/0xb -> 0
       only), and the AutoAssist option RE-TESTS f_20c6bc and wraps with the
       EQUALITY test (== 4), both 5Radio's shape rather than the hub's;
     - its own preview directory (Preview_10radio);
     - the course picker has FOUR named arms plus the default - random /
       random2 / random3 / 11lab (pickDistinct pool 7, table +0x18ce54) - and
       NO setTextSlot anywhere: every arm, the default included, strcpys into
       g_textSlots directly;
     - the lamp tail has NO logged-out arms at all - no setOptionLamps, no
       final else - so setOutput appears 8 times, not 12; logged out it falls
       straight through to the tail setCells.

   callcmp: CALL SEQUENCE IDENTICAL (81 calls) on the first build. */
// FUNCTION: EZ2AC_UNPACKED 0x0043e8d0
void SongSelectDirector::input10RadioMix()
{
    char buf[260];
    /* FOUR ints, not eight - the same deliberate overrun into buf as the
       hub's; see input14RadioMix. */
    int  pick[4];
    int  n;
    int moved = 0;

    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
            {
                if (g_bigInstance->accum[0] > 0xa ||
                    g_bigInstance->getState(0x0f) == 2 ||
                    g_bigInstance->getState(0x1e) == 2 || f_c0e20 >= 5)
                {
                    f84 -= 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    moved = 1;
                    f_c0e20 = 0;
                    f_20c0e0 = 1;
                }
                else if (g_bigInstance->accum[0] < -0xa ||
                         g_bigInstance->getState(0x10) == 2 ||
                         g_bigInstance->getState(0x1d) == 2 || f_c0e24 >= 5)
                {
                    moved = 1;
                    f84 += 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    f_c0e24 = 0;
                    f_20c0e0 = 0;
                }

                if (g_bigInstance->accum[1] > 0xa ||
                    g_bigInstance->getState(0x17) == 2)
                {
                    f84 -= 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    moved = 1;
                    f_20c0e0 = 1;
                }
                else if (g_bigInstance->accum[1] < -0xa ||
                         g_bigInstance->getState(0x18) == 2)
                {
                    moved = 1;
                    f84 += 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    f_20c0e0 = 0;
                }
            }

            /* Inside the f_bfdb4 == 4 block: the original's f_20c6bc and
               f_bfdb4 tests both jump past this store. */
            f84 = (f84 + f7c) % f7c;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2 ||
                g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                int v;

                m_sndMove2->q4(0);
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 0xa || g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        m_sndPreview->q5();
        // STRING: EZ2AC_UNPACKED 0x00493a44
        sprintf(buf, "System\\Preview_10radio\\%s.wav", rows[f84].name);
        g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
        if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
        {
            m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                "System\\Preview\\0Blank.wav", 1, 0);
        }
        f_20e894 = 0;
        m_20d164.reset();
        m_20d380.reset();
        f_c0e20 = 0;
        f_c0e24 = 0;
        loadRankingFile(rows[f84].name);
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
            m_sndPreview->q4(1);
    }

    strcpy(g_channelName, rows[f84].name);
    if (_stricmp(g_channelName, "random") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 0);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xdb294 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random2") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 1);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xf48d4 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random3") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 2);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x10df14 + pick[n] * 260);
    }
    // STRING: EZ2AC_UNPACKED 0x004936d0
    else if (_stricmp(g_channelName, "11lab") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 7);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x18ce54 + pick[n] * 260);
    }
    else
    {
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xc0e44 + (f84 * 4 + n) * 260);
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }

    /* The current channel's info panel into both grids, row 1. */
    m_20d164.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
    m_20d380.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
}

/* Mode 9, 14RadioMix - THE HUB OF THE RADIOMIX SUB-FAMILY.

   *** 54.52%, AND DECODED IS AS FAR AS THIS GOES FOR NOW. ***
   Every section has been read off the original and checked: the wheel, the
   mode change and the picker directly, the option pages and both readouts by
   confirming they are inputSpaceMix's rather than inputClubMix's (the stop
   list's `== 4 -> 5` arm, the `>= 4` wrap on g_37dcee8, cell 6 out of
   f_bff34 - re-basing on SpaceMix was worth seven points).

   What is left is NOT a reading error, it is the whole-function register
   allocation, and the diff says so in its first six instructions:

       ORIGINAL                    OURS
       mov esi, ecx                mov ebp, ecx
       xor edi, edi                -
       xor ebp, ebp                -
       sub esp, 0x118              sub esp, 0x128

   The original keeps `this` in esi and materializes TWO zero registers at
   entry; ours puts `this` in ebp and materializes none, so all 341 differing
   lines are that one substitution and the 16-byte frame it drags with it.
   THE FRAME GAP IS SOLVED and it was worth chasing: the original's pick array
   is FOUR ints, not eight. buf plus the cookie account for 0x108 of the
   0x118 frame, leaving exactly 16 bytes - and with `int pick[4]` the
   `sub esp,0x118` is byte-exact. InGame::pickDistinct writes EIGHT, so the
   original overruns this array into buf by four ints. Harmless there (buf is
   260 bytes and is only written afterwards) but it is what the frame says,
   and a port that wants the same layout should keep the four. Reusing buf
   for the picks instead gives 0x108 and is wrong.

   What is left after that is only the register allocation: `this` in esi
   against our ebp, and three `xor edi,edi` zero materializations we do not
   have. 54.52% -> 55.21%.

   FOR A PORT IT IS DONE: the logic is right and it compiles. Its real value
   was always as the template for 0x43dc00 / 0x43cf30 / 0x43e8d0, which are
   0.92 / 0.89 / 0.93 against it, and a 54% template with correct logic serves
   that purpose exactly as well as a byte-exact one would.
 See
   wip/radiomix-43f390.md for why this one was written first: the four RadioMix
   handlers score only 0.61-0.65 against every other handler but 0.84-0.94
   against each other, so 0x43dc00 / 0x43cf30 / 0x43e8d0 are edits of THIS.

   Two things no other handler has:

   THE WHEEL IS A PLAIN STEPPER. No easing at all - no f_bfdd8 latch, no
   m_be8d8.reset(), no f_20c0d8 term and no f_20c0d4 follow - and no f7c < 8
   split and no findListA lamp preamble either. It opens on updateStartLamps()
   and steps f84 directly. Note the polarity: the POSITIVE accumulator chain
   steps the cursor DOWN and lights f_20c0e0, and each player side clears only
   its own hold counter (player 1 clears neither).

   THE COURSE PICKER, between the preview block and the readouts. The current
   course's name is published to g_channelName and then compared - with
   _stricmp, a real CRT call, not strcmp - against five channel names; each
   match draws four distinct entries through the matched InGame::pickDistinct
   and publishes their names out of its own table. The five tables are one
   array at this+0xdb294 with stride 0x19640, which is why they are reached
   by offset here rather than modelled - the row layout in
   include/songselect.h has nothing at that stride. pickDistinct's pool
   indexes are 0,1,2,3 then FIVE for 11ptgc (four is some other mode's).
   The first FOUR arms strcpy into g_textSlots directly while 11ptgc and the
   default call setTextSlot, an asymmetry that is the original's (setTextSlot
   lives in another TU, so cl cannot have inlined only some of them).

   CALLCMP IS CLEAN except ONE benign fold: cl serves page 1's
   setOutput(0xd,..) and the logged-out else-arm's with one shared
   `push 0xd; call` (the original's `mov ebx,1` rematerialization between
   call and jmp is what keeps them apart there - the constant-register
   wall's downstream artifact). 11 sites for 12, same behaviour. */
// FUNCTION: EZ2AC_UNPACKED 0x0043f390
void SongSelectDirector::input14RadioMix()
{
    char buf[260];
    /* FOUR ints, not eight - the frame says so (0x118, and buf plus the
       cookie account for 0x108). pickDistinct writes eight, so the upper
       four land in buf; the original overruns this array on purpose or by
       accident, and reproducing the frame means reproducing that. */
    int  pick[4];
    int  n;
    int moved = 0;

    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
            {
                if (g_bigInstance->accum[0] > 0xa ||
                    g_bigInstance->getState(0x0f) == 2 ||
                    g_bigInstance->getState(0x1e) == 2 || f_c0e20 >= 5)
                {
                    f84 -= 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    moved = 1;
                    f_c0e20 = 0;
                    f_20c0e0 = 1;
                }
                else if (g_bigInstance->accum[0] < -0xa ||
                         g_bigInstance->getState(0x10) == 2 ||
                         g_bigInstance->getState(0x1d) == 2 || f_c0e24 >= 5)
                {
                    moved = 1;
                    f84 += 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    f_c0e24 = 0;
                    f_20c0e0 = 0;
                }

                if (g_bigInstance->accum[1] > 0xa ||
                    g_bigInstance->getState(0x17) == 2)
                {
                    f84 -= 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    moved = 1;
                    f_20c0e0 = 1;
                }
                else if (g_bigInstance->accum[1] < -0xa ||
                         g_bigInstance->getState(0x18) == 2)
                {
                    moved = 1;
                    f84 += 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    f_20c0e0 = 0;
                }
            }

            /* Inside the f_bfdb4 == 4 block: the original's f_20c6bc and
               f_bfdb4 tests both jump past the wrap (0x43f5dc); only the
               two getState tests land on it (0x43f5c1). */
            f84 = (f84 + f7c) % f7c;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2 ||
                g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                int v;

                m_sndMove2->q4(0);
                /* An int per entry at +0xdb104 - the channel level. Not
                   modelled: the row layout has nothing at this stride. */
                v = *(int *)((char *)this + 0xdb104 + f84 * 4);
                g_1b2efd0 = v;
                Log("Select Channel Level : %02d", v);
                f_bfdb4 = 7;
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 4) g_37dcecc = 5;
                else if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 0xa || g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            /* The fourth option, which input5KeyMix does not have. NO
               f_20c6bc re-test here - that is the 5/10Radio shape; this
               original tests it only on the page toggle and the switch guard. */
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dcee8++;
                if (g_37dcee8 >= 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    /* The whole preview switch lives in the MOVED block - stop the old
       sample, load the new channel's preview, reset both info grids, and
       read the channel's ranking file. (The first written form had the
       loads in the restart branch below and `moved` never set, so cl
       deleted all of this as dead code - callcmp is what caught it.) */
    if (moved == 1)
    {
        m_sndPreview->q5();
        /* Its own preview directory, not the shared one every other
           handler uses - the original pushes 0x493a64 here. */
        // STRING: EZ2AC_UNPACKED 0x00493a64
        sprintf(buf, "System\\Preview_14radio\\%s.wav", rows[f84].name);
        g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
        if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
        {
            m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                "System\\Preview\\0Blank.wav", 1, 0);
        }
        f_20e894 = 0;
        m_20d164.reset();
        m_20d380.reset();
        f_c0e20 = 0;
        f_c0e24 = 0;
        loadRankingFile(rows[f84].name);
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
            m_sndPreview->q4(1);
    }

    /* The course picker. Each arm draws four distinct entries and publishes
       their names into the rolling text slots out of its own 0x19640-strided
       table; the last two go through setTextSlot (src/modename.cpp, a real
       call from here) while the first three strcpy into g_textSlots directly.
       The default arm publishes the CURRENT course's own four. */
    strcpy(g_channelName, rows[f84].name);
    if (_stricmp(g_channelName, "random") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 0);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xdb294 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random2") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 1);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0xf48d4 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "random3") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 2);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x10df14 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "7znpfg") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 3);
        for (n = 0; n < 4; n++)
            strcpy(g_textSlots[n], (char *)this + 0x127554 + pick[n] * 260);
    }
    else if (_stricmp(g_channelName, "11ptgc") == 0)
    {
        ((InGame *)this)->pickDistinct(pick, 5);
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0x15a1d4 + pick[n] * 260);
    }
    else
    {
        for (n = 0; n < 4; n++)
            setTextSlot(n, (char *)this + 0xc0e44 + (f84 * 4 + n) * 260);
    }


    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff34[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }

    /* The current channel's info panel (system\channel_info, loaded into the
       KTexture* array at +0x20d9d4 by the ctor path in src/songselect.cpp)
       is published into both grids every tick - row 1 of each. These are the
       two grids the moved block resets. */
    m_20d164.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
    m_20d380.setCell(1, 0, *(int *)((char *)this + 0x20d9d4 + f84 * 4));
}

/* Mode 4, ClubMix. input7StreetMix WITHOUT THE PER-PLAYER SPLIT - it is 104
   instructions shorter than its template for that reason alone (twindelta.py:
   0.859 semantic, 33 regions):

     - the wheel tests both players in ONE chain per direction. Where the
       others ask `(getListAF8() == 0 && countListA() == 1) || countListA() == 2`
       and then only player 0's accumulator, this asks both accumulators and
       both sides' buttons together - 0x0f/0x17/0x1e forward, 0x10/0x18/0x1d
       back - with no list query at all;
     - the mode change likewise: one chain of all six buttons, no player gate,
       and no `f_bfdb4 == 4` wheel-at-rest guard either;
     - and the first option steps over one range rather than wrapping - 8
       becomes 9, and 0xa or 0xb reset to 0.

   Everything else, including the fourth first-page option and its readout, is
   7StreetMix unchanged. For the port, not for a match. */
// FUNCTION: EZ2AC_UNPACKED 0x00440af0
void SongSelectDirector::inputClubMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if (g_bigInstance->accum[0] > 0x14 || g_bigInstance->accum[1] > 0x14 ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x17) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            if (g_bigInstance->accum[0] < -0x14 || g_bigInstance->accum[1] < -0x14 ||
                g_bigInstance->getState(0x10) == 2 ||
                g_bigInstance->getState(0x18) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            }
            else
            {
            if (g_bigInstance->accum[0] > 0xa || g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x17) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            if (g_bigInstance->accum[0] < -0xa || g_bigInstance->accum[1] < -0xa ||
                g_bigInstance->getState(0x10) == 2 ||
                g_bigInstance->getState(0x18) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
            }
        }

        /* Still inside the f_20c6bc == 0 block: the original's first test
           jumps past all of this, to the getState(6) toggle below. */
        f84 = (f84 + f7c) % f7c;
        f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;

        if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
            g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
            g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
            f_20c6bc == 0)
        {
            if (g_37dcf08 == 1)
            {
                if (g_bigInstance->getState(0x0a) == 2 ||
                    g_bigInstance->getState(0x0c) == 2 ||
                    g_bigInstance->getState(0x0e) == 2 ||
                    g_bigInstance->getState(0x12) == 2 ||
                    g_bigInstance->getState(0x14) == 2 ||
                    g_bigInstance->getState(0x16) == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 8) g_37dcecc = 9;
                else if (g_37dcecc == 0xa || g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            /* The fourth option, which input5KeyMix does not have and
               inputRubyMix wraps the same way. f_20c6bc is re-tested because
               getState is opaque to cl, not because the source doubted it. */
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 3, 7StreetMix. input5KeyMix's shape with a FOURTH option on the first
   page, which is the one inputRubyMix also has - so this handler is the
   template plus four edits, all of them found with scratchpad/twindelta.py
   (0.959 semantic, 20 regions, and sixteen of those were the constant-register
   coin rather than anything the source says):

     - the mode change on buttons 0x0a/0x0c/0x0e is additionally gated on
       f_bfdb4 == 4, so it is refused unless the wheel is at rest;
     - the first option has no stop list at all - where 5KeyMix folds 4/5/6
       into 7 and wraps 8..0xb to 0, this one just wraps at 0xb;
     - button 9 steps a fourth counter, g_37dcee8, wrapping at 4 exactly as
       inputRubyMix wraps it;
     - and that counter drives readout cell 6 out of f_bff64[] and lamp 0xd,
       where 5KeyMix reads the plain member f_bff54 and leaves 0xd dark.

   WRITTEN FOR THE PORT, NOT FOR A MATCH. The whole family is walled on the
   constant-register assignment (see the note on input5KeyMix above and
   wip/modehandler-43b8d0.md) - the original's siblings each get a different
   assignment of 1/5/0 to the callee-saved registers from what is evidently
   one source text, and cl reproduces only RubyMix's. Do not spend a session
   trying to lift this one; its value is that the logic is right and compiled.
*/
// FUNCTION: EZ2AC_UNPACKED 0x00442770
void SongSelectDirector::input7StreetMix()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }

            /* Inside the f_bfdb4 == 4 block: the original's f_20c6bc and
               f_bfdb4 tests both jump past the wrap and the lerp. */
            f84 = (f84 + f7c) % f7c;
            f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;
        }
    }

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
        g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
        f_20c6bc == 0)
    {
        if (g_37dcf08 == 1 && f_bfdb4 == 4)
        {
            if (g_bigInstance->getState(0x0a) == 2 ||
                g_bigInstance->getState(0x0c) == 2 ||
                g_bigInstance->getState(0x0e) == 2)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 5;
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 && f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 0xb) g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 3 && g_bigInstance->heldFor(8) >= 0x96 &&
                g_37dcea4 != 7)
                f_bfdc0++;
            if (f_bfdc0 >= 5)
            {
                f_bfdc4 = 1;
                f_bfdc8 = 1;
                f_bfdc0 = 0;
            }
            if (f_bfdc4 == 1 || g_bigInstance->getState(8) == 2)
            {
                g_37dcea4++;
                f_bfdc4 = 0;
                if (g_37dcea4 >= 0xc) g_37dcea4 = 0;
            }
            /* The fourth option, which input5KeyMix does not have and
               inputRubyMix wraps the same way. f_20c6bc is re-tested because
               getState is opaque to cl, not because the source doubted it. */
            if (g_bigInstance->getState(9) == 2 && f_20c6bc >= 1)
            {
                g_37dcee8++;
                if (g_37dcee8 == 4) g_37dcee8 = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dcee4++;
                if (g_37dcee4 == 4)
                {
                    g_37dcee4 = 0;
                }
                else if (g_37dcee4 >= 2)
                {
                    g_1b2e588 = 0;
                    g_37dcee0++;
                    if (g_37dcee0 == 3) g_37dcee0 = 0;
                }
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dced0++;
                if (g_37dced0 == 1)
                {
                    g_37dcec4++;
                }
                else if (g_37dced0 == 3)
                {
                    g_37dced0 = 0;
                    g_37dcec4 = 0;
                }
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe9c[g_37dcea4]);
            m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
            {
                int d = g_37dcf28 + g_37dcee8;
                int c = g_37dcf28 + g_37dcea4;
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, d != 0);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe5c[g_37dcee4]);
            m_20cb10.setCell(6, 0, f_bff58[g_37dced0]);
            {
                int a = g_37dcf28 + g_37dced0;
                int c = g_37dcf28 + g_37dcee4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 10, EZ2CATCH. 5KeyMix's shape - no wrap guard, two option pages on
   button 6 - with the page toggle also refusing while buttons 0x11 / 0x19
   are down, its own counters (0x37dceb0 / 0x37dceac on the first page,
   0x37dcec8 / 0x37dceb4 / 0x1b2ef60 on the second), and lamps 0xc / 0xd
   simply on for the first page's readout.

   88.56%, STRUCTURALLY COMPLETE, parked on the same constant-register wall
   as input5KeyMix: the original gives 5 to edi only for the confirm run,
   then 1 to edi and a zero to ebp from the page toggle on; cl gives us
   5 -> ebp and a zero in edi from the confirm run's `f_20c6bc == 0` on, and
   leaves 1 as immediates. Every instruction is otherwise the same. Read the
   note on input5KeyMix before spending time here - the same source in the
   original's three siblings gets three different assignments (RubyMix's
   5 -> ebp / 1 -> edi is the one cl reproduces), so the lever is not in the
   handler's own text. */
// FUNCTION: EZ2AC_UNPACKED 0x0043a150
void SongSelectDirector::inputEZ2Catch()
{
    char buf[260];
    int moved = 0;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    updateStartLamps();

    if (f_20c6bc == 0)
    {
        if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
            f_c0e20++;
        if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
            f_c0e24++;

        if (f_bfdb4 == 4)
        {
            if (f7c < 8)
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0x14 ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0x14 ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 0xa)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
            else
            {
                if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[0] > 0xa ||
                        g_bigInstance->getState(0x0f) == 2 ||
                        g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[0] < -0xa ||
                        g_bigInstance->getState(0x10) == 2 ||
                        g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
                if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                {
                    if (g_bigInstance->accum[1] > 0xa ||
                        g_bigInstance->getState(0x17) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 += k_twoPi / (float)f7c;
                        f84 += 1;
                        moved = 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                    }
                    if (g_bigInstance->accum[1] < -0xa ||
                        g_bigInstance->getState(0x18) == 2)
                    {
                        f_bfdd8 = f84;
                        m_be8d8.reset();
                        f_20c0d8 -= k_twoPi / (float)f7c;
                        f84 -= 1;
                        m_sndMove->q5();
                        m_sndMove->q4(0);
                        moved = 1;
                    }
                }
            }
        }

        /* Still inside the f_20c6bc == 0 block, as in inputClubMix: the
           original's first test jumps past all of this (0x43a83f). */
        f84 = (f84 + f7c) % f7c;
        f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;

        if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3 &&
            g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
            g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
            f_20c6bc == 0)
        {
            if (g_37dcf08 == 1)
            {
                if (g_bigInstance->getState(0x0a) == 2 ||
                    g_bigInstance->getState(0x0c) == 2 ||
                    g_bigInstance->getState(0x0e) == 2)
                {
                    if ((getListAF8() == 0 && countListA() == 1) || countListA() == 2)
                    {
                        m_sndMove2->q5();
                        m_sndMove2->q4(0);
                        f_bfdb4 = 5;
                    }
                }
                if (g_bigInstance->getState(0x12) == 2 ||
                    g_bigInstance->getState(0x14) == 2 ||
                    g_bigInstance->getState(0x16) == 2)
                {
                    if ((getListAF8() == 1 && countListA() == 1) || countListA() == 2)
                    {
                        m_sndMove2->q5();
                        m_sndMove2->q4(0);
                        f_bfdb4 = 5;
                    }
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2 &&
        g_bigInstance->getState(0x11) != 2 && g_bigInstance->getState(0x11) != 3 &&
        g_bigInstance->getState(0x19) != 2 && g_bigInstance->getState(0x19) != 3 &&
        f_20c6bc >= 1)
    {
        g_37dced4++;
        if (g_37dced4 == 2) g_37dced4 = 0;
    }
    if (f_20c6bc >= 1)
    {
        switch (g_37dced4)
        {
        default:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcecc++;
                if (g_37dcecc == 3 || g_37dcecc == 4 || g_37dcecc == 5 || g_37dcecc == 6)
                    g_37dcecc = 7;
                if (g_37dcecc == 8 || g_37dcecc == 9 || g_37dcecc == 0xa || g_37dcecc >= 0xb)
                    g_37dcecc = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dceb0++;
                if (g_37dceb0 == 7) g_37dceb0 = 0;
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_37dceac++;
                if (g_37dceac == 0xa) g_37dceac = 0;
            }
            break;
        case 1:
            if (g_bigInstance->getState(7) == 2)
            {
                g_37dcec8++;
                if (g_37dcec8 == 4) g_37dcec8 = 0;
            }
            if (g_bigInstance->getState(8) == 2)
            {
                g_37dceb4++;
                if (g_37dceb4 == 2) g_37dceb4 = 0;
            }
            if (g_bigInstance->getState(9) == 2)
            {
                g_1b2ef60++;
                if (g_1b2ef60 == 2) g_1b2ef60 = 0;
            }
            break;
        }
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m4332c0();
    }
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }

    if (f_20c6bc >= 1)
    {
        if (g_37dced4 == 0)
        {
            m_20cb10.setCell(3, 0, f_bfe50);
            m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
            m_20cb10.setCell(5, 0, f_bfe18[g_37dceb0]);
            m_20cb10.setCell(6, 0, f_bfdf0[g_37dceac]);
            {
                int b = g_37dcf28 + g_37dcecc;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, 1);
                g_bigInstance->setOutput(0xd, 1);
            }
        }
        if (g_37dced4 == 1)
        {
            m_20cb10.setCell(3, 0, f_bfe54);
            m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
            m_20cb10.setCell(5, 0, f_bfe34[g_37dceb4]);
            m_20cb10.setCell(6, 0, f_bfe3c[g_1b2ef60]);
            {
                int a = g_37dcf28 + g_1b2ef60;
                int c = g_37dcf28 + g_37dceb4;
                int b = g_37dcf28 + g_37dcec8;
                g_bigInstance->setOutput(0xa, 1);
                g_bigInstance->setOutput(0xb, b != 0);
                g_bigInstance->setOutput(0xc, c != 0);
                g_bigInstance->setOutput(0xd, a != 0);
            }
        }
    }
    else if (g_37dce9c == 1)
    {
        setOptionLamps(g_37dcf28, g_37dcf28, g_37dcf28, g_37dcf28);
    }
    else
    {
        g_bigInstance->setOutput(0xa, g_37dcf28 != 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
    }
}

/* Mode 12, CV2Mix - the last of the thirteen mode-input handlers, and NOT an
   edit of any sibling (twindelta tops out at 0.544): decoded from scratch off
   the original's 534 instructions.

   What makes it its own animal:
   - NO course picker, NO preview loads, NO ranking read - the moved block
     only resets the timer, stops the preview, resets m_bf05c and latches the
     selected song's short name into f_bfd64;
   - the wheel is the EASED RubyMix wheel (f_bfdd8 latch + m_be8d8.reset() +
     the k_twoPi / f7c step) but gated per player on findListA, scrolling UP
     on the buttons the others scroll down on, with the hold counters capped
     at > 3 rather than >= 5, never cleared by the arms - and the modulo uses
     f80, not f7c, unguarded;
   - the options are FLAT (no g_37dced4 page switch): button 6 steps the efR
     toggle (g_37dcecc, >= 2 wraps), button 7 the SPEED (g_37dcef8, 0..0x14,
     with the mixStyle-7 ladder 1->2 / 3->6 / 7->0xa and the 11ambit floor),
     button 8 the note toggle (g_37dcea4, disabled while g_selectedKey == 8),
     button 9 the efbp toggle (g_1b2eb7c);
   - a bare _stricmp(f_bfd64, g_songShortName) whose result the original
     DISCARDS (reproduced as a statement), then two clamp blocks: selectedKey
     8 zeroes the note toggle, selectedKey 7 forces SPEED to 0 unless it is
     one of {0, 2, 6, 0xa};
   - the readout writes rows 8/7/6/5 of m_bf278 from the efR/efSpeed/efnote/
     efbp tables, and setOptionLamps runs UNCONDITIONALLY with the option
     VALUES (g_37dcecc, g_37dcef8, g_37dcf00, g_37dcec4) as its arguments. */
// FUNCTION: EZ2AC_UNPACKED 0x00441fa0
void SongSelectDirector::inputCV2Mix()
{
    int moved;

    if (findListA(0))
    {
        if (g_bigInstance->getState(0xb) == 3) g_bigInstance->setOutput(0x11, 1);
        else                                    g_bigInstance->setOutput(0x11, 0);
        if (g_bigInstance->getState(0xd) == 3) g_bigInstance->setOutput(0x13, 1);
        else                                    g_bigInstance->setOutput(0x13, 0);
    }
    if (findListA(1))
    {
        if (g_bigInstance->getState(0x13) == 3) g_bigInstance->setOutput(0x19, 1);
        else                                     g_bigInstance->setOutput(0x19, 0);
        if (g_bigInstance->getState(0x15) == 3) g_bigInstance->setOutput(0x1b, 1);
        else                                     g_bigInstance->setOutput(0x1b, 0);
    }
    /* Initialised HERE, after the lamp block: the original's lamp code
       takes its zero from ebx before ebx becomes `moved`, and an
       initializer at the declaration makes cl spill `moved` instead. */
    moved = 0;
    updateStartLamps();

    if (g_bigInstance->getState(0x1e) == 3 && g_bigInstance->heldFor(0x1e) >= 0x96)
        f_c0e20++;
    if (g_bigInstance->getState(0x1d) == 3 && g_bigInstance->heldFor(0x1d) >= 0x96)
        f_c0e24++;

    if (f_bfdb4 == 4)
    {
        if (findListA(0))
        {
            if (g_bigInstance->accum[0] > 0xa ||
                g_bigInstance->getState(0x0f) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                moved = 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
            }
            if (g_bigInstance->accum[0] < -0xa ||
                g_bigInstance->getState(0x10) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
        }
        if (findListA(1))
        {
            if (g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x17) == 2 ||
                g_bigInstance->getState(0x1e) == 2 || f_c0e20 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 += k_twoPi / (float)f7c;
                f84 += 1;
                moved = 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
            }
            if (g_bigInstance->accum[1] < -0xa ||
                g_bigInstance->getState(0x18) == 2 ||
                g_bigInstance->getState(0x1d) == 2 || f_c0e24 > 3)
            {
                f_bfdd8 = f84;
                m_be8d8.reset();
                f_20c0d8 -= k_twoPi / (float)f7c;
                f84 -= 1;
                m_sndMove->q5();
                m_sndMove->q4(0);
                moved = 1;
            }
        }
    }

    f84 = (f84 + f80) % f80;
    f_20c0d4 += (f_20c0d8 - f_20c0d4) * 0.33333334f;

    if (g_bigInstance->getState(6) != 2 && g_bigInstance->getState(6) != 3)
    {
        if (findListA(0))
        {
            if (g_bigInstance->getState(4) != 2 && g_bigInstance->getState(4) != 3 &&
                f_20c6bc == 0 && g_37dcf08 == 1)
            {
                if (g_bigInstance->getState(0x0a) == 2 ||
                    g_bigInstance->getState(0x0c) == 2 ||
                    g_bigInstance->getState(0x0e) == 2)
                {
                    m_sndMove2->q4(0);
                    f_bfdb4 = 7;
                }
            }
        }
        else if (findListA(1))
        {
            if (g_bigInstance->getState(5) != 2 && g_bigInstance->getState(5) != 3 &&
                f_20c6bc == 0 && g_37dcf08 == 1)
            {
                if (g_bigInstance->getState(0x12) == 2 ||
                    g_bigInstance->getState(0x14) == 2 ||
                    g_bigInstance->getState(0x16) == 2)
                {
                    m_sndMove2->q4(0);
                    f_bfdb4 = 7;
                }
            }
        }
    }

    if (g_bigInstance->getState(6) == 2)
    {
        g_37dcecc++;
        if (g_37dcecc >= 2) g_37dcecc = 0;
    }
    if (g_bigInstance->getState(7) == 2)
    {
        if (_stricmp(g_songInfoName, "11ambit") == 0)
        {
            g_37dcef8++;
            if (g_37dcef8 == 0)
                g_37dcef8 = 2;
            else if (g_37dcef8 >= 0x15)
                g_37dcef8 = 2;
        }
        else
        {
            g_37dcef8++;
            if (g_mixStyle == 7)
            {
                if (g_37dcef8 == 1) g_37dcef8 = 2;
                else if (g_37dcef8 == 3) g_37dcef8 = 6;
                else if (g_37dcef8 == 7) g_37dcef8 = 0xa;
                else if (g_37dcef8 >= 0xb) g_37dcef8 = 0;
            }
            else if (g_37dcef8 >= 0x15)
            {
                g_37dcef8 = 0;
            }
        }
    }
    if (g_selectedKey != 8)
    {
        if (g_bigInstance->getState(8) == 2)
        {
            g_37dcea4++;
            if (g_37dcea4 >= 2) g_37dcea4 = 0;
        }
    }
    if (g_bigInstance->getState(9) == 2)
    {
        g_1b2eb7c++;
        if (g_1b2eb7c == 2) g_1b2eb7c = 0;
    }

    if (moved == 1)
    {
        f_20e894 = 0;
        if (m_sndPreview) m_sndPreview->q5();
        f_c0e20 = 0;
        f_c0e24 = 0;
        m_bf05c.reset();
        strcpy(f_bfd64, g_songShortName);
    }

    if (_stricmp(g_songInfoName, "11ambit") == 0 && g_37dcef8 <= 1)
        g_37dcef8 = 2;
    if (g_selectedKey == 8)
        g_37dcea4 = 0;

    /* The original calls this and DISCARDS the result - a bare statement,
       kept because the call is real. */
    _stricmp(f_bfd64, g_songShortName);

    if (g_selectedKey == 7)
    {
        if (g_37dcef8 != 0xa && g_37dcef8 != 6 && g_37dcef8 != 2 &&
            g_37dcef8 != 0)
        {
            g_37dcef8 = 0;
        }
    }

    m_bf278.setCell(8, 0, f_bef2c[g_37dcecc]);
    m_bf278.setCell(7, 0, f_bef34[g_37dcef8]);
    if (g_selectedKey == 8)
        m_bf278.setCell(6, 0, f_befe4[0]);
    else
        m_bf278.setCell(6, 0, f_befe4[g_37dcea4]);
    m_bf278.setCell(5, 0, f_befec[g_1b2eb7c]);
    setOptionLamps(g_37dcecc, g_37dcef8, g_37dcf00, g_37dcec4);
}
