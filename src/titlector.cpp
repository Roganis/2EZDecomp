/* TitleDirector::TitleDirector @0x44e9b0 - the title screen's constructor, and
   the cheapest of the port queue's big holes: 2,352 bytes over 549
   instructions with only twenty branches, i.e. the "table of calls" shape.

   What it does, in order: build the nine clip players and the pump, put a
   run's worth of global state back to zero, read the cabinet, load the
   version readout and the eight title animations, and start the attract song.

   Three things in it are worth knowing for a port:

     - THE VERSION READOUT. g_1b2f088 is stamped into the four digit cells of
       the +0x828 chart, and which chart script it uses depends on the number:
       up to 999 it is `common\ver.str` with no digits at all, from 1000 it is
       `common\ver_over.str` and the thousands digit is written to cell 1. The
       original spells the /1000, /100 and /10 divides as magic multiplies;
       they are plain divisions and cl regenerates the same constants.
     - THE COIN CHART is gated on the cabinet twice, on both +0x420284 (coins
       in) and +0x420290 (credits), and the two tests are not the same:
       m_1d0 is `coins >= credits` while m_1648 is set when there are coins at
       all, or when there are no credits.
     - THE ATTRACT SONG runs off the class's own KSongPlayerBase base at +0x14
       through the pump at +0x14f4, at a rate of 405.34.

   Written for the PORT rather than for the byte match - the goal here is that
   the title screen's behaviour exists in the tree at all, since an empty stub
   gives a recompilation nothing. Every offset, global and callee in it was
   pinned first (see the inventory this file replaced in src/stubs.cpp); the
   twenty-five fields it writes were carved out of TitleDirector's pads and
   the ten globals it needed were added, both verified against the full match
   on their own. */

#include <stdio.h>
#include "ezscreen.h"
#include "ez2acapp.h"
#include "bigbase.h"
#include "obj402860.h"
#include "kprofile.h"

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern void set_4184e0(int idx, int val);          /* 0x4184e0 (src/globals.cpp) */
extern void Log(const char *fmt, ...);             /* 0x402880 */
extern char  g_modeName[256];                      /* 0x1b2e680 (src/modename.cpp) */

/* WINMM's timeGetTime declared WITHOUT __declspec(dllimport), so the call goes
   through the linker's thunk @0x4145f0 - which is the form this ctor uses. The
   windows.h declaration is the dllimport one and would compile to a six-byte
   indirect call instead; the original mixes both forms across TUs, so it is a
   per-call-site decision. */
extern "C" DWORD __stdcall timeGetTime();

/* The graphics singleton's frame op, as src/titledirector.cpp views it. */
struct Obj40BB00
{
    void doOp7(int v);            /* 0x40a470 */
};

/* src/globals.cpp - the ten this ctor was the first to need. */
extern int   g_1b2ea94;   /* src/titledirector.cpp */
extern int   g_16963c4;
extern int   g_1b2e55c;
extern int   g_1b2e7e0;
extern int   g_1b2ea9c;
extern int   g_1b2ead4;
extern int   g_1b2eb04[8];
extern int   g_1b2eb74;
extern float g_1b2ebc4;
extern int   g_1b2efb8;
extern int   g_1b2f034;

/* Already modelled elsewhere; declared here rather than pulled in wholesale. */
extern int   g_bonusScore[4];      /* src/rankingctor.cpp   0x1b2e54c */
extern int   g_stageLevels[5];     /* src/totalresultctor.cpp 0x1b2ea50 */
extern int   g_1b2efc0[4];         /* src/totalresultctor.cpp */
extern int   g_1b2e568, g_1b2e584, g_1b2e588, g_1b2e7a8, g_1b2e7cc;
extern int   g_1b2e89c, g_1b2ea18, g_1b2ea20, g_1b2ea28, g_1b2ea30;
extern int   g_1b2eadc, g_1b2eb70, g_1b2ebdc, g_1b2ec14, g_1b2ef40;
extern int   g_1b2ef60, g_1b2ef80, g_1b2efb4, g_1b2efbc, g_1b2f050;
extern int   g_1b2f06c, g_1b2f084, g_1b2f088, g_1b5f184, g_37dd06c;
extern int   g_advSound, g_autoScratchL, g_autoScratchR, g_autoPedal;
extern int   g_catchOverlay, g_coinInserted, g_coinShown, g_mixStyle;
extern int   g_panelDrawn, g_testModeFlag, g_useAudiencePanel, g_1b2eb6c;
extern float g_rate1p, g_rate2p;

// FUNCTION: EZ2AC_UNPACKED 0x0044e9b0
TitleDirector::TitleDirector()
{
    char path[260];
    int  i;

    /* The nine clip players, the song base at +0x14 and the pump at +0x14f4
       are members; C++ runs their constructors here, in declaration order,
       which is the order the original's EH states 1..0xa record. */

    // STRING: EZ2AC_UNPACKED 0x00494ea8
    Log("TitleDirector::Create at 0x%p", this);
    m_1624 = 3;

    for (i = 0; i < 40; i++)
        set_4184e0(i, 0);

    g_1b2e7cc = 0;
    g_1b2ef40 = 1;
    g_coinInserted = 0;
    g_coinShown = 0;
    g_1b2ea94 = 0;
    g_1b2ea20 = 0;
    g_useAudiencePanel = 0;
    g_rate1p = 0.0f;
    g_rate2p = 0.0f;
    g_1b5f184 = 0;
    g_1b2ead4 = 0;
    m_10c = 0;
    g_1b2ebc4 = 0.0f;
    g_testModeFlag = 0;
    g_1b2efb4 = 0;
    g_1b2e89c = 0;
    g_1b2e7e0 = 0;
    g_1b2eb70 = 0;

    /* The coin chart runs when there are coins in the slot, or - with none -
       when there are no credits either. */
    if (g_bigInstance->have != 0)
        m_1648 = 1;
    else
        m_1648 = (g_bigInstance->need == 0);
    if (g_bigInstance->have >= 0x80)
        g_bigInstance->have = 0;

    g_1b2ec14 = 0;
    g_1b2e568 = 0;
    m_1c8 = 0;
    m_38 = 0;
    m_1644 = 0;
    g_1b2f034 = 1;
    g_panelDrawn = 0;
    g_catchOverlay = 0;
    m_30 = 0;
    m_34 = 0xff;
    m_114 = 0;
    m_1630 = 0;
    m_1634 = 0;
    m_163c = 0;
    m_1640 = 0;
    g_1b2eb6c = 0;

    for (i = 0; i < 10; i++)
    {
        sprintf(path, "system\\title\\common\\ver_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &m_1a0[i], 0);
    }

    g_mixStyle = 0;
    // STRING: EZ2AC_UNPACKED 0x00494e70
    m_14e0 = (TitleStoppable *)g_sub1b2eae4->createSample("System\\Title\\Title.wav", 1, 0);
    // STRING: EZ2AC_UNPACKED 0x00494e54
    m_14e4 = (TitleStoppable *)g_sub1b2eae4->createSample("System\\Title\\Title_coin.wav", 1, 0);
    m_14f0 = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\hiddencmd.wav", 0, 0);

    // STRING: EZ2AC_UNPACKED 0x00494e44
    changeDir("System\\Title", 1);

    /* The version readout, on the +0xa48 chart. `common\\ver.str` is loaded
       and attached UNCONDITIONALLY, and then again in the under-a-thousand
       arm - the original really does load it twice, which is what an
       if/else edited from a common ancestor looks like. Over a thousand it
       is `common\\ver_over.str` instead, plus the thousands digit in cell 1.
       The head pointer goes to +0x828 and is read straight back. */
    // STRING: EZ2AC_UNPACKED 0x00494e34
    sprintf(path, "common\\ver.str", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadStr(path, &m_828);
    m_a48.attach(m_828, 1);

    if (g_1b2f088 <= 999)
    {
        sprintf(path, "common\\ver.str", g_modeName);
        ((KGraphics *)g_sub1b2f054)->loadStr(path, &m_828);
        m_a48.attach(m_828, 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00494e20
        sprintf(path, "common\\ver_over.str", g_modeName);
        ((KGraphics *)g_sub1b2f054)->loadStr(path, &m_828);
        m_a48.attach(m_828, 1);
        m_a48.setCell(1, 0, (int)m_1a0[g_1b2f088 / 1000 % 10]);
    }
    m_a48.setCell(2, 0, (int)m_1a0[g_1b2f088 / 100 % 10]);
    m_a48.setCell(4, 0, (int)m_1a0[g_1b2f088 / 10 % 10]);
    m_a48.setCell(5, 0, (int)m_1a0[g_1b2f088 % 10]);

    // STRING: EZ2AC_UNPACKED 0x00494e10
    ((KGraphics *)g_sub1b2f054)->loadTexture("fadeblack.bmp", &m_198, 0);
    popDirectory();

    m_attract = 0;
    m_restart = 0;
    m_108 = 0;
    m_118 = 0;
    m_11c = 0;
    m_120 = 0;
    m_12c = 0;
    m_1638 = 0;
    m_110 = 0;

    g_1b2efc0[0] = 0;
    g_1b2efc0[1] = 0;
    g_1b2efc0[2] = 0;
    g_1b2efc0[3] = 0;
    g_stageLevels[0] = 0;
    g_stageLevels[1] = 0;
    g_stageLevels[2] = 0;
    g_stageLevels[3] = 0;
    g_stageLevels[4] = 0;
    g_bonusScore[0] = 0;
    g_bonusScore[1] = 0;
    g_bonusScore[2] = 0;
    g_bonusScore[3] = 0;
    g_1b2e55c = 0;
    g_1b2f06c = 1;
    g_1b2ea28 = 0;
    g_1b2ebdc = 0;

    g_iniProfile->setInt("UseBackground", 1);

    g_1b2f050 = g_autoScratchL;
    for (i = 0; i < 8; i++)
        g_1b2eb04[i] = 0;
    g_1b2e584 = 0;
    g_1b2e7a8 = 0;
    g_1b2e588 = 0;
    g_1b2eadc = 0;
    g_1b2ea9c = 0;
    g_1b2eb74 = 0;
    g_1b2ef80 = g_autoScratchR;
    g_1b2efbc = g_autoPedal;
    g_1b2f084 = 0;
    g_1b2ea30 = 0;
    g_1b2ef60 = 0;
    g_1b2ea18 = 0;
    g_1b2efb8 = 0;

    g_songTable.clearAll();
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);

    changeDir("System\\Title", 1);

    // STRING: EZ2AC_UNPACKED 0x00494dfc
    ((KGraphics *)g_sub1b2f054)->loadStr("TitleLoop\\Title.str", &m_c64);
    m_1d4.attach(m_c64, 1);
    // STRING: EZ2AC_UNPACKED 0x00494de4
    ((KGraphics *)g_sub1b2f054)->loadStr("Title_Intro\\Coin.str", &m_c64);
    m_3f0.attach(m_c64, 0);
    // STRING: EZ2AC_UNPACKED 0x00494dcc
    ((KGraphics *)g_sub1b2f054)->loadStr("Title_Outro\\Intro.str", &m_c64);
    m_82c.attach(m_c64, 0);
    // STRING: EZ2AC_UNPACKED 0x00494dbc
    ((KGraphics *)g_sub1b2f054)->loadStr("Logo\\Logo.str", &m_c64);
    m_12c0.attach(m_c64, 1);
    // STRING: EZ2AC_UNPACKED 0x00494da8
    ((KGraphics *)g_sub1b2f054)->loadStr("Common\\Scanline.str", &m_c64);
    m_10a4.attach(m_c64, 1);
    // STRING: EZ2AC_UNPACKED 0x00494d98
    ((KGraphics *)g_sub1b2f054)->loadStr("Common\\Coin.str", &m_c64);
    m_60c.attach(m_c64, 0);

    /* Both coin scripts are loaded and the cabinet picks between them. */
    m_1d0 = (g_bigInstance->have >= g_bigInstance->need);
    // STRING: EZ2AC_UNPACKED 0x00494d80
    ((KGraphics *)g_sub1b2f054)->loadStr("Common\\InsertCoin.str", &m_c64);
    // STRING: EZ2AC_UNPACKED 0x00494d68
    ((KGraphics *)g_sub1b2f054)->loadStr("Common\\Pressstart.str", &m_c68);
    /* ONE attach on the selected head, not an attach per arm - the original
       picks the pointer and then calls once. */
    m_c6c.attach(m_1d0 != 0 ? m_c68 : m_c64, 1);

    // STRING: EZ2AC_UNPACKED 0x00494d5c
    m_14e8 = (TitleStoppable *)g_sub1b2eae4->createSample("start.wav", 0, 0);
    popDirectory();

    m_1d0 = (g_bigInstance->have >= g_bigInstance->need);
    if (g_bigInstance->need == 0)
        m_1638 = 1;

    if (g_advSound == 1)
    {
        m_1644 = 1;
        if (m_14e0 != 0)
        {
            m_14e0->s4(0);
            if (g_bigInstance->need == 0)
                m_1638 = 1;
        }
    }
    else if (g_bigInstance->have != 0 && m_1630 == 0 && m_1638 == 0 &&
             m_14e0 != 0)
    {
        m_14e0->s4(0);
        m_1630 = 1;
        m_1c8 = 1;
    }

    m_1cc = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    g_bigInstance->setOutput(8, 1);
    g_bigInstance->setOutput(9, 1);

    m_strobe = 0;
    m_strobeLeft = 0;
    m_1620 = 0;
    m_164c = 0;
    m_1650 = 0;
    m_1654 = 0;
    m_1660 = 0;
    m_pump.start(0, 0);
    m_pump.addPlayer(this);
    /* One lamp beat every 405.4054 ms = 60000 / 148, i.e. 148 BPM - the
       original pushes 0x43cab3e4. TitleSong::step @0x44deb0 is the tick; it
       raises m_strobe, and tickLamps advances the walk on it. */
    setRate(405.4054f);
    m_pump.restart();
    start();

    g_37dd06c = 0xff;
    m_1628 = (int)timeGetTime();
    m_162c = 0;
    m_step = 0;
    m_165c = 0;
    m_100 = 0;

    // STRING: EZ2AC_UNPACKED 0x00494d40
    Log("Current SystemPoint : %d", g_16963c4);
}

ScrFader *m450570();    /* 0x450570 (src/scrfader.cpp) */

/* The one-shot handoff out of the title screen, run by update2 once the
   overlay's own tick is taken: order the 2->2 fade (unless the attract loop
   already owns the lamps), stop whichever of the two title samples is still
   playing, and latch m_1cc so it never fires twice. Returns 1 on the run
   that did the work, 0 after. */
// FUNCTION: EZ2AC_UNPACKED 0x0044dec0
int TitleDirector::m44dec0()
{
    if (m_1cc == 1)
        return 0;
    if (m_attract != 1)
    {
        ScrFader *fader = m450570();
        fader->m450420(2, 2, (TitleSink *)this);
    }
    if (m_attract == 0)
    {
        m_attract = 1;
        if (((TitleLampDev *)m_14e0)->d16() & 1)
            ((TitleLampDev *)m_14e0)->d5();
        if (((TitleLampDev *)m_14e4)->d16() & 1)
            ((TitleLampDev *)m_14e4)->d5();
    }
    m_1cc = 1;
    return 1;
}
