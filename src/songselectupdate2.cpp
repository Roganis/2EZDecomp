/* SongSelectDirector::update2 @0x446540 - the song-select input tick,
   10.6 KB / ~2,680 instructions / six jump tables. Written 2026-08-26 from
   the end-to-end decode in wip/songselect-update2-446540.md.

   The shape: the START-lamp blink on g_37dd044 % 60; the preview pump; the
   option-page open and the per-player difficulty-cursor steppers (with the
   EZ2CATCH variant stepping g_37dced0); the cursor wrap and the STYLE name
   publish into 0x1b2e5d0; the coin sink; then the NINE-PHASE machine on
   f_bfdb4 (3 empty) whose phase 4 dispatches the thirteen mode-input
   handlers and whose later phases run the countdown, the eyecatch loads and
   the fade-out; the phase-4/5 ranking-panel ease; the per-family wheel
   chases (banded for the list modes, thirds for the radio modes); and the
   key-range publish tail (set_4184e0) that routes the option counters into
   gameplay, ending in the per-round global broadcast.

   Original quirks kept: the difficulty steppers' DOWN arms latch the INPUT
   SINGLETON POINTER as the previous cursor value (mov ecx with ecx =
   g_bigInstance - down-steps therefore always blip), and EZ2CATCH's mode
   test is an inlined strcmp (memcmp-style repz cmpsb) where every other
   compare calls _stricmp. */

#include <stdio.h>
#include <string.h>
#include "songselect.h"
#include "bigbase.h"
#include "ez2acapp.h"
extern float k_tableOriginX;   /* 0x491b0c, 213.0f (src/songselect.cpp) */

extern int countListA();       /* 0x418310 (src/nodelists.cpp) */
extern int getListAF8();       /* 0x418340 (src/nodelists.cpp) */
extern struct ListNode *findListA(int key);   /* 0x418e30 */
extern void set_4184e0(int idx, int val);     /* 0x4184e0 (src/globals.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */

extern char g_modeName[256];   /* 0x1b2e680 (src/modename.cpp) */
extern char g_channelName[0x80];   /* 0x1b2e728 (src/songselect.cpp) */
extern char g_textSlots[32][128];  /* 0x1b2ec18 (src/modename.cpp) */
extern char g_songInfoName[64];    /* 0x1b2eb80 (src/panels.cpp) */
extern int  g_modeIndex;       /* 0x1b2e548 */
extern int  g_elemCount;       /* 0x1b2e718 */
extern int  g_mixStyle;        /* 0x1b2ebc8 (src/panels.cpp) */
extern int  g_selectedKey;     /* 0x1b2efd4 (src/globals.cpp) */
extern int  g_1b2eb6c;         /* src/ez2acapp.cpp - hit-percent mode */
extern int  g_1b2eb74;
extern int  g_1b2e568;         /* FIX TARGETBAR */
extern int  g_37dcf08;
extern int  g_37dce9c;
extern int  g_37dced4;
extern int  g_37dced0;
extern int  g_37dcecc;
extern int  g_37dcec8;
extern int  g_37dcec4;
extern int  g_37dcec0;
extern int  g_37dcebc;
extern int  g_37dceb4;
extern int  g_37dceb0;
extern int  g_37dceac;
extern int  g_37dcea4;
extern int  g_37dcea0;
extern int  g_37dcee0;
extern int  g_37dcee4;
extern int  g_37dcee8;
extern int  g_37dcef8;

/* update2's own working set. Only what nothing else defines is defined
   here; everything else is extern'd under its established name. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcf20
int g_37dcf20;
// GLOBAL: EZ2AC_UNPACKED 0x037dcf24
int g_37dcf24;
// GLOBAL: EZ2AC_UNPACKED 0x037dcea0
int g_37dcea0;
// GLOBAL: EZ2AC_UNPACKED 0x037dd044
int g_37dd044;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e894
int g_1b2e894;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f080
int g_1b2f080;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e700
int g_1b2e700;

extern int   g_37dcf1c;        /* src/gamestate.cpp */
extern int   g_37dcf28;        /* src/gamestate.cpp */
extern int   g_37dd06c;        /* src/globals.cpp */
extern int   g_1b2eb7c;        /* src/gamestate.cpp */
extern int   g_1b2eb74;        /* src/globals.cpp */
extern int   g_1b2e720;        /* src/modeselectctor.cpp */
extern int   g_37dcf30;        /* src/gamestate.cpp - the disc pulse */
extern int   g_37dcf34;
extern int   g_37dcf38;
extern int   g_1b2ef40;        /* src/playerslot.cpp region */
extern int   g_1b2e588;        /* src/playerslot.cpp region */
extern char  g_stylePrefix[64];    /* 0x1b2e5d0 (src/maingame.cpp) */
extern int   g_1b2e7c8;        /* src/moderunner.cpp */
extern int   g_1b2eba0;        /* src/maingame.cpp */
extern int   g_1b2e704;        /* src/playerslot.cpp */
extern int   g_1b5f19c;        /* src/playerslot.cpp */
extern int   g_1b2ea78;        /* src/systemmenu.cpp */
extern int   g_catchOverlay;   /* 0x1b2e5a8 (src/gamepanels.cpp) */
extern int   g_1b2e91c;        /* src/maingame.cpp */
extern int   g_1b2e914;        /* src/panels.cpp */
extern int   g_1b2ef38;        /* src/modeselectctor.cpp */
extern int   g_1b2ea30;        /* src/modeselectctor.cpp */
extern int   g_1b2efa0;        /* src/modeselectctor.cpp */
extern int   g_1b2ef80;        /* src/rankingctor.cpp */
extern int   g_1b2f050;        /* src/rankingctor.cpp */
extern int   g_1b2efbc;        /* src/rankingctor.cpp */
extern int   g_1b2ea4c;        /* src/modeselectctor.cpp */
extern int   g_1b5f178;        /* src/gamestate.cpp */
extern int   g_1b2efb8;        /* src/globals.cpp */
extern int   g_1b2eb04[8];     /* src/globals.cpp - the confirmed song name */
extern int   g_stageNumber;    /* 0x1b2ec0c (src/directors.cpp) */
extern int   g_stageLevels[5]; /* 0x1b2ea50 (src/totalresultctor.cpp) */
extern int   g_1b2ebbc;        /* src/gamestate.cpp */
extern int   g_1b2ebdc;        /* src/gamestate.cpp */
extern float g_1b2ef8c;        /* src/gamepanels.cpp */
extern float g_1b2ea80;        /* src/gamepanels.cpp */
extern void *g_1b2e844;        /* src/moderunner.cpp - the coin sink */

/* The coin/credit sink's one slot. */
struct CoinSink { virtual void take(int kind); };
/* The entry finder and the confirm-target latch, viewed the way
   src/accessors.cpp declares them. */
struct Obj431040 { int findByName(const char *name); };
struct Obj44FA40 { char _pad[0x14]; int f; void set(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x00446540
void SongSelectDirector::update2(int a, int b)
{
    char buf[260];
    int  i;
    int  v;

    /* The START-lamp blink, on the low half of a 60-tick counter. */
    if (g_37dd044++ % 60 < 30)
    {
        if (_stricmp(g_modeName, "ClubMix") != 0 &&
            _stricmp(g_modeName, "SpaceMix") != 0 &&
            _stricmp(g_modeName, "10RadioMix") != 0 &&
            _stricmp(g_modeName, "14RadioMix") != 0)
        {
            if (findListA(0))
            {
                g_bigInstance->setOutput(0x10, 1);
                g_bigInstance->setOutput(0x12, 1);
                g_bigInstance->setOutput(0x14, 1);
            }
            if (findListA(1))
            {
                g_bigInstance->setOutput(0x18, 1);
                g_bigInstance->setOutput(0x1a, 1);
                g_bigInstance->setOutput(0x1c, 1);
            }
        }
        else
        {
            g_bigInstance->setOutput(0x10, 1);
            g_bigInstance->setOutput(0x12, 1);
            g_bigInstance->setOutput(0x14, 1);
            g_bigInstance->setOutput(0x18, 1);
            g_bigInstance->setOutput(0x1a, 1);
            g_bigInstance->setOutput(0x1c, 1);
        }
    }
    else
    {
        g_bigInstance->setOutput(0x10, 0);
        g_bigInstance->setOutput(0x12, 0);
        g_bigInstance->setOutput(0x14, 0);
        g_bigInstance->setOutput(0x18, 0);
        g_bigInstance->setOutput(0x1a, 0);
        g_bigInstance->setOutput(0x1c, 0);
    }

    /* The preview pump: restart whichever voice this mode owns. */
    if (g_1b2eb6c == 0)
    {
        if (m_sndPreview->q16() & 1)
            m_sndPreview->q17();
    }
    else
    {
        if (m_bfd44->q16() & 1)
            m_bfd44->q17();
    }
    f_bfdb0 += 1;
    if (f_bfdb0 == 1)
    {
        g_1b2e704 = 0;
        g_1b5f19c = 0;
    }
    if (g_1b2eb6c != 0)
        m_bfd44->q4(1);

    if (f_bfdb4 < 7)
    {
        if (g_modeIndex != 0xc)
            m4445c0();

        /* EZ2CATCH's compare is the inlined strcmp; everything else in this
           function goes through _stricmp. */
        if (strcmp(g_modeName, "EZ2CATCH") != 0)
        {
            /* Option-page open per player (the OPTION buttons). */
            if (((getListAF8() == 0 && countListA() == 1) || countListA() == 2 ||
                 f78 == 4 || f78 == 5 || f78 == 8 || f78 == 9) &&
                g_bigInstance->getState(0x11) == 2 &&
                g_bigInstance->getState(6) != 3 && f_20c6bc >= 1)
            {
                m_bfd40->q5();
                m_bfd40->q4(0);
                g_37dcf1c += 1;
                m_c05a8.reset();
            }
            if (((getListAF8() == 1 && countListA() == 1) || countListA() == 2 ||
                 f78 == 4 || f78 == 5 || f78 == 8 || f78 == 9) &&
                g_bigInstance->getState(0x19) == 2 &&
                g_bigInstance->getState(6) != 3 && f_20c6bc >= 1)
            {
                m_bfd40->q5();
                m_bfd40->q4(0);
                g_37dcf1c += 1;
                m_c05a8.reset();
            }

            if (f_20c6bc >= 1)
            {
                if (f78 == 4)
                    goto steppers_both;
                if (f78 == 5 || f78 == 8 || f78 == 9)
                    goto steppers_both;

                if (getListAF8() == 0 && countListA() == 1)
                {
                    if (g_bigInstance->getState(0xb) == 3 &&
                        g_bigInstance->heldFor(0xb) >= 0x96)
                        f_c0e30 += 1;
                    if (g_bigInstance->getState(0xd) == 3 &&
                        g_bigInstance->heldFor(0xd) >= 0x96)
                        f_c0e34 += 1;
                    if ((g_bigInstance->getState(0xb) == 2 || f_c0e30 > 5) &&
                        g_bigInstance->getState(6) != 3)
                    {
                        g_37dcf20 = g_37dcf1c;
                        f_c0e30 = 0;
                        if (g_37dcf1c != 0)
                            g_37dcf1c = g_37dcf1c - 1;
                        if (g_37dcf1c != g_37dcf20)
                        {
                            m_bfd40->q5();
                            m_bfd40->q4(0);
                            m_c05a8.reset();
                        }
                    }
                    if ((g_bigInstance->getState(0xd) == 2 || f_c0e34 > 5) &&
                        g_bigInstance->getState(6) != 3)
                    {
                        g_37dcf20 = g_37dcf1c;
                        f_c0e34 = 0;
                        if (g_37dcf1c != f_c09e0 - 1)
                            g_37dcf1c = g_37dcf1c + 1;
                        if (g_37dcf1c != g_37dcf20)
                        {
                            m_bfd40->q5();
                            m_bfd40->q4(0);
                            m_c05a8.reset();
                        }
                    }
                }
                if (getListAF8() == 1 && countListA() == 1)
                {
                    if (g_bigInstance->getState(0x13) == 3 &&
                        g_bigInstance->heldFor(0x13) >= 0x96)
                        f_c0e38 += 1;
                    if (g_bigInstance->getState(0x15) == 3 &&
                        g_bigInstance->heldFor(0x15) >= 0x96)
                        f_c0e3c += 1;
                    if ((g_bigInstance->getState(0x13) == 2 || f_c0e38 > 5) &&
                        g_bigInstance->getState(6) != 3)
                    {
                        g_37dcf20 = g_37dcf1c;
                        f_c0e38 = 0;
                        if (g_37dcf1c != 0)
                            g_37dcf1c = g_37dcf1c - 1;
                        if (g_37dcf1c != g_37dcf20)
                        {
                            m_bfd40->q5();
                            m_bfd40->q4(0);
                            m_c05a8.reset();
                        }
                    }
                    if ((g_bigInstance->getState(0x15) == 2 || f_c0e3c > 5) &&
                        g_bigInstance->getState(6) != 3)
                    {
                        g_37dcf20 = g_37dcf1c;
                        f_c0e3c = 0;
                        if (g_37dcf1c != f_c09e0 - 1)
                            g_37dcf1c = g_37dcf1c + 1;
                        if (g_37dcf1c != g_37dcf20)
                        {
                            m_bfd40->q5();
                            m_bfd40->q4(0);
                            m_c05a8.reset();
                        }
                    }
                }
                if (countListA() != 2)
                    goto after_steppers;
steppers_both:
                if (g_bigInstance->getState(0xb) == 3 &&
                    g_bigInstance->heldFor(0xb) >= 0x96)
                    f_c0e30 += 1;
                if (g_bigInstance->getState(0xd) == 3 &&
                    g_bigInstance->heldFor(0xd) >= 0x96)
                    f_c0e34 += 1;
                if ((g_bigInstance->getState(0xb) == 2 || f_c0e30 > 5) &&
                    g_bigInstance->getState(6) != 3)
                {
                    g_37dcf20 = g_37dcf1c;
                    f_c0e30 = 0;
                    if (g_37dcf1c != 0)
                        g_37dcf1c = g_37dcf1c - 1;
                    if (g_37dcf1c != g_37dcf20)
                    {
                        m_bfd40->q5();
                        m_bfd40->q4(0);
                        m_c05a8.reset();
                    }
                }
                if ((g_bigInstance->getState(0xd) == 2 || f_c0e34 > 5) &&
                    g_bigInstance->getState(6) != 3)
                {
                    g_37dcf20 = g_37dcf1c;
                    f_c0e34 = 0;
                    if (g_37dcf1c != f_c09e0 - 1)
                        g_37dcf1c = g_37dcf1c + 1;
                    if (g_37dcf1c != g_37dcf20)
                    {
                        m_bfd40->q5();
                        m_bfd40->q4(0);
                        m_c05a8.reset();
                    }
                }
                if (g_bigInstance->getState(0x13) == 3 &&
                    g_bigInstance->heldFor(0x13) >= 0x96)
                    f_c0e38 += 1;
                if (g_bigInstance->getState(0x15) == 3 &&
                    g_bigInstance->heldFor(0x15) >= 0x96)
                    f_c0e3c += 1;
                if ((g_bigInstance->getState(0x13) == 2 || f_c0e38 > 5) &&
                    g_bigInstance->getState(6) != 3)
                {
                    g_37dcf20 = g_37dcf1c;
                    f_c0e38 = 0;
                    if (g_37dcf1c != 0)
                        g_37dcf1c = g_37dcf1c - 1;
                    if (g_37dcf1c != g_37dcf20)
                    {
                        m_bfd40->q5();
                        m_bfd40->q4(0);
                        m_c05a8.reset();
                    }
                }
                if ((g_bigInstance->getState(0x15) == 2 || f_c0e3c > 5) &&
                    g_bigInstance->getState(6) != 3)
                {
                    g_37dcf20 = g_37dcf1c;
                    f_c0e3c = 0;
                    if (g_37dcf1c != f_c09e0 - 1)
                        g_37dcf1c = g_37dcf1c + 1;
                    if (g_37dcf1c != g_37dcf20)
                    {
                        m_bfd40->q5();
                        m_bfd40->q4(0);
                        m_c05a8.reset();
                    }
                }
            }
        }
        else if (strcmp(g_modeName, "EZ2CATCH") == 0)
        {
            /* EZ2CATCH: the OPTION buttons step the catch class instead.
               The original re-runs the inlined compare here (0x446e65). */
            if (((getListAF8() == 0 && countListA() == 1) || countListA() == 2 ||
                 f78 == 4 || f78 == 5 || f78 == 8 || f78 == 9) &&
                g_bigInstance->getState(0x11) == 2 &&
                g_bigInstance->getState(6) != 3 && f_20c6bc >= 1)
            {
                g_37dced0 += 1;
                m_c05a8.reset();
                m_bfd40->q5();
                m_bfd40->q4(0);
                if (g_37dced0 > 2)
                {
                    g_37dced0 = 0;
                    g_1b2ef40 = 1;
                }
                else if (g_37dced0 == 1)
                    g_catchOverlay = 1;
                else if (g_37dced0 == 2)
                    g_1b2ef40 = 0;
                else
                    g_1b2ef40 = 1;
            }
            if (((getListAF8() == 1 && countListA() == 1) || countListA() == 2 ||
                 f78 == 4 || f78 == 5 || f78 == 8 || f78 == 9) &&
                g_bigInstance->getState(0x19) == 2 && f_20c6bc >= 1)
            {
                g_37dced0 += 1;
                m_c05a8.reset();
                m_bfd40->q5();
                m_bfd40->q4(0);
                if (g_37dced0 > 2)
                {
                    g_37dced0 = 0;
                    g_1b2ef40 = 1;
                }
                else if (g_37dced0 == 1)
                    g_catchOverlay = 1;
                else if (g_37dced0 == 2)
                    g_1b2ef40 = 0;
                else
                    g_1b2ef40 = 1;
            }
        }
    }
after_steppers:

    /* The cursor wrap and the STYLE name publish. */
    if (g_modeIndex != 0xc)
        g_37dcf1c = g_37dcf1c % f_c09e0;
    if (g_1b2eb7c == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x00493bc8
        sprintf(buf, "STYLE_%s%d_black_", g_modeName, g_37dcf1c + 1);
        if (g_mixStyle == 6)
            g_1b2f080 = 1;
    }
    else
    {
        if (f78 == 10)
        {
            // STRING: EZ2AC_UNPACKED 0x00493bb4
            sprintf(buf, "STYLE_ez2catch%d_%d", g_37dceac + 1, g_37dceb0);
        }
        else if (g_37dced0 == 1)
            sprintf(buf, "STYLE_%s%d_black_", g_modeName, g_37dcf1c + 1);
        else if (g_1b2ef40 == 0)
            sprintf(buf, "STYLE_%s%d_black_", g_modeName, g_37dcf1c + 1);
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00493ba8
            sprintf(buf, "STYLE_%s%d_", g_modeName, g_37dcf1c + 1);
        }
        g_1b2f080 = 0;
    }
    strcpy(g_stylePrefix, buf);
    g_1b2e7c8 = g_37dcf1c;
    if (f78 == 10)
        m_c05a8.setCell(2, 0, f_c0498[g_37dced0]);
    else
        m_c05a8.setCell(2, 0, f_c04b8[g_37dcf1c * 2]);

    /* The coin sink. */
    if (g_bigInstance->getState(0) == 2)
        ((CoinSink *)g_1b2e844)->take(1);
    if (g_bigInstance->getState(0x1c) == 2 || g_bigInstance->getState(1) == 2)
        ((CoinSink *)g_1b2e844)->take(2);

    f8c = f84;
    f90 = f88;

    switch (f_bfdb4)
    {
    case 1:
        if (m_bfd2c)
            m_bfd2c->q4(0);
        g_1b2ea78 = 0;
        if (m_sndPreview)
            m_sndPreview->q4(1);
        if (g_1b2eb6c != 0)
        {
            f84 = f8c = ((Obj431040 *)this)->findByName((const char *)g_1b2eb04);
            f_cc = f_c4 = (float)f84 * 100.0f;
            f90 = f88;
            f_bfdb4 = 2;
        }
        else
        {
            f84 = ((Obj431040 *)this)->findByName((const char *)g_1b2eb04);
            f_bfdd4 = g_selectedKey;
            f_cc = f_c4 = (float)f84 * 100.0f;
            f8c = f84;
            f90 = f88;
            f_bfdb4 = 2;
        }
        break;

    case 2:
        g_1b2ebdc = 0;
        if (g_1b2eb6c == 0)
            f_bfdb8 += 0x1e;
        else
            f_bfdb8 += 5;
        if (f_bfdb8 > 0xfe)
        {
            f_bfdb8 = 0xff;
            f_bfdb4 = 4;
            f20 = 0;
            if (g_1b2eb6c == 0)
            {
                if (g_bigInstance->getState(6) == 2 &&
                    g_bigInstance->getState(0x11) != 2 &&
                    g_bigInstance->getState(0x11) != 3 &&
                    g_bigInstance->getState(0x19) != 2 &&
                    g_bigInstance->getState(0x19) != 3)
                {
                    f_ec = (f_ec == 0);
                    m_bfd18->q5();
                    m_bfd18->q4(0);
                }
            }
            else
            {
                /* The re-test of g_1b2eb6c here can never pass - it is the
                   original's own dead arm, kept as written. */
                if (g_bigInstance->getState(6) == 2 && g_1b2eb6c == 0 &&
                    g_bigInstance->getState(0x11) != 2 &&
                    g_bigInstance->getState(0x11) != 3 &&
                    g_bigInstance->getState(0x19) != 2 &&
                    g_bigInstance->getState(0x19) != 3)
                {
                    f_ec = (f_ec == 0);
                    m_bfd1c->q5();
                    m_bfd1c->q4(0);
                }
            }
            if (f_20c6bc >= 1 && g_1b2eb6c == 0 &&
                (g_bigInstance->getState(7) == 2 ||
                 g_bigInstance->getState(8) == 2 ||
                 g_bigInstance->getState(9) == 2))
            {
                f_ec = (f_ec == 0);
                m_bfd1c->q5();
                m_bfd1c->q4(0);
            }
        }
        if (f_20c6d0 == 1)
        {
            f_20c6a0 += 16.0f;
            if (f_20c6a0 >= 300.0f)
                f_20c6a0 = 300.0f;
            f_20c698 += 0x19;
            f_20c69c += ((300.0f - f_20c6a0) * 0.02f) * 4.0f;
            if (f_20c69c >= k_tableOriginX)
                f_20c69c = k_tableOriginX;
            if (f_20c698 >= 0xfe)
                f_20c698 = 0xff;
        }
        else
        {
            f_20c69c -= 25.0f;
            f_20c698 += -0x19;
            f_20c6a0 = 0.0f;
            if (1.0f >= f_20c69c)
                f_20c69c = 0.0f;
            if (f_20c698 <= 1)
                f_20c698 = 0;
        }
        break;

    case 3:
        if (f_bfdb0 == 0x50)
            f_bfdb4 = 4;
        break;

    case 4:
        switch (f78)
        {
        case 1:  inputRubyMix();     break;
        case 2:  inputStreetMix();   break;
        case 7:  inputRadioMix();    break;
        case 4:  inputClubMix();     break;
        case 5:  inputSpaceMix();    break;
        case 3:  input7StreetMix();  break;
        case 6:  input5RadioMix();   break;
        case 8:  input10RadioMix();  break;
        case 9:  input14RadioMix();  break;
        case 10: inputEZ2Catch();    break;
        case 11: inputScratchMix();  break;
        case 0:  input5KeyMix();     break;
        case 12: inputCV2Mix();      break;
        }
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9)
            m4458d0(1);

        /* The idle blink. */
        f94 = 0;
        if (f_20c6bc == 0 && g_37dce9c == 0)
        {
            g_37dcf24 += 1;
            if (g_37dcf24 >= 0x1e)
            {
                g_37dcf28 = 1;
                if (g_37dcf24 >= 0x3c)
                {
                    g_37dcf28 = 0;
                    g_37dcf24 = 0;
                }
            }
            else if (g_37dcf28 == 1)
            {
                if (g_37dcf24 >= 0x3c)
                {
                    g_37dcf28 = 0;
                    g_37dcf24 = 0;
                }
            }
        }
        else if (f_20c6bc >= 1 && g_37dce9c == 0)
        {
            g_37dcf24 += 1;
            if (g_37dcf24 >= 0x1e)
            {
                g_37dcf28 = 1;
                if (g_37dcf24 >= 0x3c)
                {
                    g_37dcf28 = 0;
                    g_37dcf24 = 0;
                }
            }
            else if (g_37dcf28 == 1)
            {
                if (g_37dcf24 >= 0x3c)
                {
                    g_37dcf28 = 0;
                    g_37dcf24 = 0;
                }
            }
        }
        else if (g_37dce9c == 1)
        {
            g_37dcea0 += 1;
            if (g_37dcea0 >= 0)
                g_37dcf28 = 1;
            if (g_37dcea0 >= 3)
                g_37dcf28 = 0;
            if (g_37dcea0 >= 6)
                g_37dcf28 = 1;
            if (g_37dcea0 >= 9)
                g_37dcf28 = 0;
            if (g_37dcea0 >= 0xc)
                g_37dcf28 = 1;
            if (g_37dcea0 >= 0xf)
            {
                g_37dcea0 = 0;
                g_37dce9c = 0;
                g_37dcf28 = 0;
                g_37dcf24 = 0;
                g_37dd044 = 0x1e;
            }
        }

        /* Option-page close - the same two-arm shape as phase 2's copy,
           dead re-test of g_1b2eb6c included. */
        if (g_1b2eb6c == 0)
        {
            if (g_bigInstance->getState(6) == 2 &&
                g_bigInstance->getState(0x11) != 2 &&
                g_bigInstance->getState(0x11) != 3 &&
                g_bigInstance->getState(0x19) != 2 &&
                g_bigInstance->getState(0x19) != 3)
            {
                f_ec = (f_ec == 0);
                m_bfd18->q5();
                m_bfd18->q4(0);
            }
        }
        else
        {
            if (g_bigInstance->getState(6) == 2 && g_1b2eb6c == 0 &&
                g_bigInstance->getState(0x11) != 2 &&
                g_bigInstance->getState(0x11) != 3 &&
                g_bigInstance->getState(0x19) != 2 &&
                g_bigInstance->getState(0x19) != 3)
            {
                f_ec = (f_ec == 0);
                m_bfd1c->q5();
                m_bfd1c->q4(0);
            }
        }
        if (f_20c6bc >= 1 && g_1b2eb6c == 0 &&
            (g_bigInstance->getState(7) == 2 ||
             g_bigInstance->getState(8) == 2 ||
             g_bigInstance->getState(9) == 2 || f_bfdc8 != 0))
        {
            f_ec = (f_ec == 0);
            m_bfd1c->q5();
            m_bfd1c->q4(0);
            f_bfdc8 = 0;
        }

        /* Page advance. */
        if (g_modeIndex == 1)
        {
            if (g_bigInstance->getState(6) == 2 &&
                g_bigInstance->getState(0x11) != 3 &&
                g_bigInstance->getState(0x19) != 3 &&
                g_bigInstance->getState(0x11) != 2 &&
                g_bigInstance->getState(0x19) != 2)
            {
                if (f_20c6bc == 1)
                {
                    g_37dce9c = 1;
                    f_20c6bc = 0;
                }
                else
                    f_20c6bc = f_20c6bc + 1;
            }
            if (f_20c6bc != 0)
                goto confirm;
        }
        else
        {
            if (g_1b2eb6c == 0)
            {
                if (g_bigInstance->getState(6) == 2 &&
                    g_bigInstance->getState(0x11) != 3 &&
                    g_bigInstance->getState(0x19) != 3 &&
                    g_bigInstance->getState(0x11) != 2 &&
                    g_bigInstance->getState(0x19) != 2)
                {
                    if (f_20c6bc == 2)
                    {
                        g_37dce9c = 1;
                        f_20c6bc = 0;
                    }
                    else
                        f_20c6bc = f_20c6bc + 1;
                }
            }
            if (f_20c6bc == 0)
                goto slide;
            if (g_modeIndex != 4 && g_modeIndex != 5 &&
                g_modeIndex != 8 && g_modeIndex != 9)
                goto confirm;
            goto confirm2;
        }
confirm:
        if (findListA(0))
        {
            if (g_bigInstance->getState(0xa) == 2 ||
                g_bigInstance->getState(0xc) == 2 ||
                g_bigInstance->getState(0xe) == 2)
            {
                g_37dce9c = 1;
                f_20c6bc = 0;
                g_37dcf08 = 1;
                g_37dced4 = 0;
                m_bfd18->q5();
                m_bfd18->q4(0);
            }
        }
        if (findListA(1))
            goto confirm_p2;
        goto slide;
confirm2:
        if (g_bigInstance->getState(0xa) == 2 ||
            g_bigInstance->getState(0xc) == 2 ||
            g_bigInstance->getState(0xe) == 2)
        {
            g_37dce9c = 1;
            f_20c6bc = 0;
            g_37dcf08 = 1;
            g_37dced4 = 0;
            m_bfd18->q5();
            m_bfd18->q4(0);
            goto slide;
        }
confirm_p2:
        if (g_bigInstance->getState(0x12) == 2 ||
            g_bigInstance->getState(0x14) == 2 ||
            g_bigInstance->getState(0x16) == 2)
        {
            g_37dce9c = 1;
            f_20c6bc = 0;
            g_37dcf08 = 1;
            g_37dced4 = 0;
            m_bfd18->q5();
            m_bfd18->q4(0);
        }
slide:
        /* The option-panel slide. */
        if (f_20c6bc >= 1)
        {
            f_20c6c0 += 50.0f;
            f_20c6c4 = 0;
            if (f_20c6c0 >= 511.0f)
                f_20c6c0 = 511.0f;
            f_20c6c8 += -2;
            if (f_20c6c8 <= 0xe7)
                f_20c6c8 = 0xe6;
            g_37dcf08 = 0;
        }
        else if (f_20c6bc <= 0)
        {
            f_20c6c0 -= 50.0f;
            f_20c6c4 = 0;
            if (0.0f >= f_20c6c0)
            {
                f_20c6c0 = 0.0f;
                g_37dcf08 = 1;
            }
            f_20c6c8 += 2;
            if (f_20c6c8 > 0xfe)
                f_20c6c8 = 0xff;
        }

        /* The TARGETBAR toggle. */
        if (g_bigInstance->getState(9) == 2 &&
            g_bigInstance->getState(0x11) != 3 &&
            g_bigInstance->getState(0x19) != 3 &&
            g_bigInstance->getState(0x11) != 2 &&
            g_bigInstance->getState(0x19) != 2 &&
            f_20c6bc == 0)
        {
            g_1b2eb74 = (g_1b2eb74 == 0);
        }

        m446440();

        /* FIX TARGETBAR brightness. */
        if (g_1b2e568 == 1)
            f_c0488 += 0x37;
        else
            f_c0488 += -0x37;
        if (f_c0488 > 0xfe)
            f_c0488 = 0xff;
        if (f_c0488 <= 0)
            f_c0488 = 0;

        /* The countdown. */
        f_bfd5c += 1;
        if (f_bfd5c == 0x3c)
        {
            f_bfd48 -= 1;
            f_bfd5c = 0;
            if (f_bfd48 < 0xa && f_bfda4 == 0)
            {
                m_bfd34->q5();
                m_bfd34->q4(0);
            }
        }
        if (f_bfd48 <= 0)
        {
            f_bfda4 = 1;
            f_bfd48 = 0;
            f_bfd58 = 0xa;
        }
        f_bfd50 += 1;
        if (f_bfd50 == 0x3c)
            g_1b2eba0 = rows[f84].f264;
        if (f_bfda4 == 1 && f_bfd48 == 0)
        {
            f_20c6bc = 0;
            m_sndMove2->q4(0);
            if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
                g_modeIndex != 9 && g_modeIndex != 0xc)
                f_bfdb4 = 5;
            else
                f_bfdb4 = 7;
        }
        g_1b2eba0 = rows[f84].f264;
        break;

    case 5:
        switch (f78)
        {
        case 0: case 1: case 2: case 3: case 4: case 5: case 10: case 11:
            m444ae0(1);       break;
        case 6:  input5RadioMix();  break;
        case 7:  inputRadioMix();   break;
        case 8:  input10RadioMix(); break;
        case 9:  input14RadioMix(); break;
        case 12: inputCV2Mix();     break;
        }
        f_bfd5c += 1;
        if (f_bfda4 == 0)
        {
            if (f_bfd5c == 0x3c)
            {
                f_bfd48 -= 1;
                f_bfd5c = 0;
                if (f_bfd48 < 6)
                {
                    m_bfd34->q5();
                    m_bfd34->q4(0);
                }
            }
            f_bfd50 += 1;
            if (f_bfd50 == 0x3c)
                g_1b2eba0 = rows[f84].f264;
            if (f_bfd48 <= 0)
            {
                f_bfd48 = 0;
                goto phase5_out;
            }
        }
        else
        {
            if (f_bfd5c == 0x3c)
            {
                f_bfd58 -= 1;
                f_bfd5c = 0;
                if (f_bfd58 < 6)
                {
                    m_bfd34->q5();
                    m_bfd34->q4(0);
                }
            }
            f_bfd50 += 1;
            if (f_bfd50 == 0x3c)
                g_1b2eba0 = rows[f84].f264;
            if (f_bfd58 <= 0)
            {
                f_bfd58 = 0;
phase5_out:
                m_sndMove2->q4(0);
                f_bfdb4 = 7;
            }
        }
        if (f_20c6bc <= 0)
        {
            f_20c6c0 -= 50.0f;
            f_20c6c4 = 0;
            if (0.0f >= f_20c6c0)
            {
                f_20c6c0 = 0.0f;
                g_37dcf08 = 1;
            }
            f_20c6c8 += 2;
            if (f_20c6c8 > 0xfe)
                f_20c6c8 = 0xff;
        }
        g_1b2eba0 = rows[f84].f264;
        break;

    case 6:
        if (m_be4a0.isDone())
        {
            m_sndMove2->q5();
            m_sndMove2->q4(0);
            f_bfdb4 = 7;
        }
        break;

    case 7:
        f_bfdb8 += -5;
        g_37dced4 = 0;
        f94 = 0;
        f_bfd48 = 0;
        if (f_bfdb8 < 2)
        {
            if (g_1b2e568 == 1)
            {
                g_1b2ef8c = -2.0f;
                g_1b2ea80 = -2.0f;
            }
            else
            {
                g_1b2ef8c = 0.0f;
                g_1b2ea80 = 0.0f;
            }
            // STRING: EZ2AC_UNPACKED 0x00493ba4
            changeDir(".\\", 1);
            loadRankFile();
            ((Obj44FA40 *)f_20bdc4)->set(0);
            f_bfdb8 = 1;
            f_bfdb4 = 8;
            if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            {
                if (f78 == 7)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493b44
                    sprintf(buf, "system\\Channel_Eyecatch\\radio\\%s.bmp", g_channelName);
                }
                else if (f78 == 6)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493b1c
                    sprintf(buf, "system\\Channel_Eyecatch\\5radio\\%s.bmp", g_channelName);
                }
                else if (f78 == 8)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493af4
                    sprintf(buf, "system\\Channel_Eyecatch\\10radio\\%s.bmp", g_channelName);
                }
                else if (f78 == 9)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493acc
                    sprintf(buf, "system\\Channel_Eyecatch\\14radio\\%s.bmp", g_channelName);
                }
                ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &f_bfd04, 0);
                /* Stage_Mask's STRING entity is owned by moderunner.cpp. */
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    "system\\channel_eyecatch\\common\\Stage_Mask.bmp", &f_bfd08, 0);
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    // STRING: EZ2AC_UNPACKED 0x00493aa0
                    "system\\channel_eyecatch\\common\\Stage_1.bmp", &f_bfd0c, 0);
            }
            else if (g_1b2eb6c == 1)
            {
                // STRING: EZ2AC_UNPACKED 0x00493b84
                sprintf(buf, "system\\CV2Mix\\eyecatch\\%s.bmp", g_songInfoName);
                goto eyecatch_one;
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x00493b6c
                sprintf(buf, "system\\eyecatch\\%s.bmp", g_textSlots[g_elemCount]);
            }
eyecatch_one:
            /* Every path ends on this reload - the radio family has just
               loaded the same texture once already; the original does too. */
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &f_bfd04, 0);
        }
eyecatch_done:
        f_20c6c0 -= 50.0f;
        if (0.0f >= f_20c6c0)
            f_20c6c0 = 0.0f;
        f_20c69c -= 25.0f;
        f_20c698 += -0x19;
        f_20c6a0 = 0.0f;
        if (1.0f >= f_20c69c)
            f_20c69c = 0.0f;
        if (f_20c698 <= 1)
            f_20c698 = 0;
        if (f_bfdb8 == 1)
        {
            g_1b2efb8 = f_bc57c;
            if (g_elemCount < 2)
                strcpy((char *)g_1b2eb04, rows[f84].name);
        }
        break;

    case 8:
        g_stageNumber = f70;
        g_stageLevels[g_elemCount] = f70;
        f_bfdb8 += 0xa;
        if (f_bfdb8 > 0xfe)
        {
            f_bfdb8 = 0xff;
            f_bfdb4 = 9;
        }
        break;

    case 9:
        f_a0 += 1;
        g_37dcf30 = 0;
        g_37dcf34 = 0;
        g_37dcf38 = 0;
        if (f_a0 >= 0x3c)
        {
            if (!(m_sndMove2->q16() & 1))
                ((CoinSink *)g_1b2e844)->take(0);
        }
        break;
    }

    /* The ranking-panel ease, phases 4 and 5 only. */
    if (f_bfdb4 == 4 || f_bfdb4 == 5)
    {
        if (g_1b2eb74 == 1)
        {
            f_20c6a0 += 16.0f;
            if (f_20c6a0 >= 300.0f)
                f_20c6a0 = 300.0f;
            f_20c69c += ((300.0f - f_20c6a0) * 0.02f) * 4.0f;
            f_20c698 += 0x19;
            if (f_20c69c >= k_tableOriginX)
                f_20c69c = k_tableOriginX;
            if (f_20c698 >= 0xfe)
                f_20c698 = 0xff;
        }
        else
        {
            f_20c69c -= 25.0f;
            f_20c698 += -0x19;
            f_20c6a0 = 0.0f;
            if (1.0f >= f_20c69c)
                f_20c69c = 0.0f;
            if (f_20c698 <= 1)
                f_20c698 = 0;
        }
    }

    /* The wheel chase. */
    if (f78 == 0xc)
    {
        float target = (float)f84 * 100.0f;
        float nf = (float)f80;
        float d = target - f_c4;
        f_cc = target;
        if (d > nf * 50.0f)
        {
            float dd = d - nf * 100.0f;
            if (dd > -50.0f)
                f_c4 -= 5.0f;
            else if (dd > -100.0f)
                f_c4 -= 10.0f;
            else if (dd > -200.0f)
                f_c4 -= 30.0f;
            else
                f_c4 -= 50.0f;
        }
        else if (nf * -50.0f > d)
        {
            float du = d + nf * 100.0f;
            if (50.0f > du)
                f_c4 += 5.0f;
            else if (100.0f > du)
                f_c4 += 10.0f;
            else if (200.0f > du)
                f_c4 += 20.0f;
            else
                f_c4 += 30.0f;
        }
        else if (50.0f > d && d > -50.0f)   /* a FLAT -50 (0x448528 /
                                                   0x4487ac): was nf * -50 */
        {
            if (d > 0.0f)
                f_c4 += 5.0f;
            else
                f_c4 -= 5.0f;
        }
        else if (100.0f > d && d > -100.0f)
        {
            if (d > 0.0f)
                f_c4 += 10.0f;
            else
                f_c4 -= 10.0f;
        }
        else if (200.0f > d && d > -200.0f)
        {
            if (d > 0.0f)
                f_c4 += 20.0f;
            else
                f_c4 -= 20.0f;
        }
        else if (d > 0.0f)
            f_c4 += 30.0f;
        else
            f_c4 -= 30.0f;
        if (10.0f > d && d > -10.0f)
            f_c4 = target;
        if (f_c4 > nf * 100.0f - 10.0f && target == 0.0f)
            f_c4 = target;
        if (f_c4 > nf * 100.0f)
            f_c4 -= nf * 100.0f;
        if (0.0f > f_c4)
            f_c4 += nf * 100.0f;
    }
    else if (f78 != 7 && f78 != 6 && f78 != 8 && f78 != 9)
    {
        float target = (float)f84 * 100.0f;
        float nf = (float)f7c;
        float d = target - f_c4;
        f_cc = target;
        if (d > nf * 50.0f)
        {
            float dd = d - nf * 100.0f;
            if (dd > -50.0f)
                f_c4 -= 5.0f;
            else if (dd > -100.0f)
                f_c4 -= 10.0f;
            else if (dd > -200.0f)
                f_c4 -= 30.0f;
            else
                f_c4 -= 50.0f;
        }
        else if (nf * -50.0f > d)
        {
            float du = d + nf * 100.0f;
            if (50.0f > du)
                f_c4 += 5.0f;
            else if (100.0f > du)
                f_c4 += 10.0f;
            else if (200.0f > du)
                f_c4 += 20.0f;
            else
                f_c4 += 30.0f;
        }
        else if (50.0f > d && d > -50.0f)   /* a FLAT -50 (0x448528 /
                                                   0x4487ac): was nf * -50 */
        {
            if (d > 0.0f)
                f_c4 += 5.0f;
            else
                f_c4 -= 5.0f;
        }
        else if (100.0f > d && d > -100.0f)
        {
            if (d > 0.0f)
                f_c4 += 10.0f;
            else
                f_c4 -= 10.0f;
        }
        else if (200.0f > d && d > -200.0f)
        {
            if (d > 0.0f)
                f_c4 += 20.0f;
            else
                f_c4 -= 20.0f;
        }
        else if (d > 0.0f)
            f_c4 += 30.0f;
        else
            f_c4 -= 30.0f;
        if (10.0f > d && d > -10.0f)
            f_c4 = target;
        if (f_c4 > nf * 100.0f - 10.0f && target == 0.0f)
            f_c4 = target;
        if (f_c4 > nf * 100.0f)
            f_c4 -= nf * 100.0f;
        if (0.0f > f_c4)
            f_c4 += nf * 100.0f;
    }
    else
    {
        float target = (float)f84 * 100.0f;
        float d = target - f_c4;
        f_cc = target;
        if (d > (float)(f7c * 50))
        {
            f_c4 -= ((float)(f7c * 100) - d) * 0.33333334f;
        }
        else
        {
            if ((float)(-f7c * 50) > d)
                d = (float)(f7c * 100) + d;
            f_c4 += d * 0.33333334f;
        }
        if (1.0f > d && d > -1.0f)
            f_c4 = target;
        if ((float)(1 - f7c * 100) > d && d > (float)(-1 - f7c * 100))
            f_c4 = target;
        if (f_c4 > (float)(f7c * 100))
            f_c4 -= (float)(f7c * 100);
        if (0.0f > f_c4)
            f_c4 += (float)(f7c * 100);
    }

    /* The key-range publish - the option counters routed into gameplay. */
    g_1b2e91c = 0x80;
    g_1b2e914 = 7;
    if (g_37dcebc == 1)
        set_4184e0(0xf, 1);
    else
        set_4184e0(0xf, 0);
    if (g_37dcec0 == 1)
        set_4184e0(0x10, 1);
    else
        set_4184e0(0x10, 0);
    if (g_37dcec4 == 1)
        set_4184e0(8, 1);
    else
        set_4184e0(8, 0);
    set_4184e0(0, 0);
    set_4184e0(1, 0);
    set_4184e0(2, 0);
    set_4184e0(0x18, 0);
    switch (g_37dcec8)
    {
    case 1: set_4184e0(0, 1);    break;
    case 2: set_4184e0(1, 1);    break;
    case 3: set_4184e0(2, 1);    break;
    case 4: set_4184e0(0x18, 1); break;
    }
    set_4184e0(3, 0);
    set_4184e0(4, 0);
    set_4184e0(0x1d, 0);
    set_4184e0(5, 0);
    set_4184e0(0x16, 0);
    set_4184e0(0x1f, 0);
    set_4184e0(0x15, 0);
    set_4184e0(0x17, 0);
    set_4184e0(0x1e, 0);
    set_4184e0(0x19, 0);
    set_4184e0(0x1a, 0);
    g_1b2e894 = g_37dceb4;
    g_1b2ef38 = 0;
    g_1b2ebbc = 0;
    switch (g_37dcecc)
    {
    case 2:  set_4184e0(4, 1);    break;
    case 3:  set_4184e0(0x1d, 1); break;
    case 4:
        if (g_modeIndex == 4 || g_modeIndex == 5 ||
            g_modeIndex == 8 || g_modeIndex == 9)
        {
            g_1b2ef38 = 1;
            set_4184e0(3, 1);
        }
        else if (g_modeIndex == 3 || g_modeIndex == 7)
        {
            g_1b2ebbc = 1;
            set_4184e0(3, 1);
        }
        else
            set_4184e0(3, 1);
        break;
    case 1:  set_4184e0(3, 1);    break;
    case 5:  set_4184e0(5, 1);    break;
    case 6:  set_4184e0(0x17, 1); break;
    case 7:  set_4184e0(0x16, 1); break;
    case 8:  set_4184e0(0x1f, 1); break;
    case 9:  set_4184e0(0x19, 1); break;
    case 10: set_4184e0(0x1a, 1); break;
    }
    set_4184e0(6, 0);
    set_4184e0(7, 0);
    g_37dcee0 = g_1b2e720;
    if (g_1b2eb6c != 0)
        g_1b2e914 = g_37dcef8;
    switch (g_37dcee4)
    {
    case 1:
        g_1b2e588 = 1;
        break;
    case 2:
        g_1b2e588 = 0;
        set_4184e0(6, 1);
        break;
    case 3:
        g_1b2e588 = 0;
        set_4184e0(7, 1);
        break;
    case 0:
        set_4184e0(0x20, 0);
        set_4184e0(0x21, 0);
        g_1b2e588 = 0;
        set_4184e0(6, 0);
        set_4184e0(7, 0);
        break;
    }
    set_4184e0(0x11, 0);
    g_1b2ea30 = g_37dcea4;
    g_1b2efa0 = g_37dceb0;
    if (g_37dcee8 == 1)
    {
        if (g_modeIndex != 5 && g_modeIndex != 9)
            g_1b2ef80 = 1;
        g_1b2f050 = 1;
        if (g_modeIndex != 1)
            g_1b2ea4c = 1;
    }
    else if (g_37dcee8 == 2)
    {
        if (g_modeIndex != 5 && g_modeIndex != 9)
        {
            g_1b2ef80 = 0;
            g_1b2efbc = 1;
        }
        else
        {
            g_1b2ef80 = 1;
            g_1b2efbc = 0;
        }
        g_1b2f050 = 0;
        if (g_modeIndex != 1)
            g_1b2ea4c = 1;
    }
    else if (g_37dcee8 == 3)
    {
        g_1b2ef80 = 1;
        g_1b2f050 = 1;
        if (g_modeIndex == 5)
        {
            g_1b2efbc = 0;
            g_1b2ea4c = 1;
        }
        else
        {
            g_1b2efbc = 1;
            if (g_modeIndex != 1)
                g_1b2ea4c = 1;
        }
    }
    else
    {
        g_1b2f050 = 0;
        g_1b2ef80 = 0;
        g_1b2efbc = 0;
        g_1b2ea4c = 0;
    }
    g_1b5f178 = g_37dcee8;
    g_1b2e700 = countListA();
    if (g_modeIndex != 0xa)
    {
        g_1b2ef40 = (g_37dced0 != 2);
        g_37dd06c = f_bfdb8;
    }
    else
    {
        g_1b2ef40 = (g_37dced0 != 2);
        g_37dd06c = f_bfdb8;
    }
}
