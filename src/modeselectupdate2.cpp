/* ModeSelectDirector::update2 @0x44c860 - the mode wheel's input/state
   tick, transcribed end to end (the draw half is
   src/modeselectupdate.cpp). Head: the BGM gate, the page-cycle chord
   (2P skips the CV2 page; the casual page seeds 0xb since catch is
   1P-only), side claims through m44b290, the service hook on key 0.
   States: 0 fade-in; 1 the input state (page-hold keys and four
   wheel/key step blocks walking f18 through per-page caps, the option
   counters cleared each step - g_37dd048[1] never - the selection blip
   on f18 != f1c, the m44b4b0/m44b750 steppers); 5 the diff-style pick
   (its choice publishes the HARD flags in the exit); 2 the fade-out
   with the attract video handed off through reset/submit/copyOut and
   the Video_Blank_256 blit; 3 the confirmed handoff (the intro voice -
   the start sting in mode 12 - pumping f9434, one loadSongBin of
   StreetMix / CV2Mix / g_modeName, the notify-hook exit past 200/420).
   The case order 1,5,2,3 is the original's block order. */

#include "modeselect.h"
#include "ezscreen.h"     /* SongTable */
#include "songselect.h"   /* updateStartLamps */
#include "ez2acapp.h"
#include "bigbase.h"

extern EzSubsystem *g_sub1b2f054;   /* 0x1b2f054 */
extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */
struct ListNode;
extern ListNode *findListA(int key);   /* 0x418e30 */
extern int countListA();               /* 0x418310 */
extern int getListAF8();               /* 0x418340 */
extern void blinkAlternateLamps();     /* 0x44b330 (src/lamps.cpp) */
extern void *g_1b2e844;
extern int g_1b2ea78;
extern int g_37dd048[8];    /* 0x37dd048 (src/modeselectctor.cpp) - the
                               option counters; [1] is never cleared */
extern int g_1b2ea20;       /* src/modeselectctor.cpp */
extern int g_1b2e89c;       /* src/gamestate.cpp */
extern int g_1b2e584;       /* src/modeselectctor.cpp */
extern int g_1b2eba8;       /* src/globals.cpp */
extern int g_37dd06c;       /* src/gfresultupdate.cpp */
extern int g_modeIndex;     /* 0x1b2e548 (src/modename.cpp) */
extern char g_modeName[256];/* 0x1b2e680 (src/modename.cpp) */
struct GfxBlob28
{
    int d[10];
};
extern GfxBlob28 g_1b2e868;         /* src/directors.cpp */
extern int g_1b2f138;       /* src/directors.cpp */
extern struct SongTable g_songTable;   /* src/globals.cpp */

struct ObjB8F0 { void reset(); };                 /* src/com_wrappers.cpp */
struct Obj40B980 { void submit(int arg); };       /* src/com_wrappers.cpp */
struct Obj40B9A0 { void copyOut(void *dst); };    /* src/accessors.cpp */

/* g_1b2e844's slot 0 (the titleupdate pattern). */
struct ModeNotify { virtual void q00(int v); };

/* The sound slots this tick uses. */
struct MsSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
    virtual void stop();             /* +0x14 */
    virtual void q6(); virtual void q7(); virtual void q8(); virtual void q9();
    virtual void q10(); virtual void q11(); virtual void q12();
    virtual void q13(); virtual void q14(); virtual void q15();
    virtual int  playing();          /* +0x40 */
    virtual void restart();          /* +0x44 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0044c860
void ModeSelectDirector::update2(int a, int)
{
    if (f95ec < 0x1e)
        f95ec += 1;
    blinkAlternateLamps();

    /* Start the BGM once anything holds it, except in the confirmed
       state; remember that it started for the coin lamp below. */
    if (sndBgm != 0 && f95e4 != 3)
    {
        f961c = 1;
        ((MsSnd *)sndBgm)->play(0);
    }
    ((MsSnd *)sndModeIntro)->restart();
    if (f93e0 == 1)
    {
        f9628 = 0;
        f18 = 0;
        ((MsSnd *)sndHidden)->stop();
        ((MsSnd *)sndHidden)->play(0);
        f93e0 = 0;
    }

    /* The page-cycle chord: key 6 with neither join key down. */
    if (g_bigInstance->getState(6) == 2 &&
        g_bigInstance->getState(4) != 2 &&
        g_bigInstance->getState(4) != 3 &&
        g_bigInstance->getState(5) != 2 &&
        g_bigInstance->getState(5) != 3 &&
        fe0 == 0 && f95e4 == 1)
    {
        f9628 += 1;
        f9458 = 0;
        ((MsSnd *)sndModeTap)->stop();
        ((MsSnd *)sndModeTap)->play(0);
        if (f9454 >= 10)
        {
            if (f9628 > 4)
                f9628 = 0;
            if (countListA() == 2)
            {
                if (f9628 > 3)
                    f9628 = 0;
                switch (f9628 - 1)
                {
                case 0: f18 = 2;   break;
                case 1: f18 = 6;   break;
                case 2: f18 = 0xb; break;
                case 3:            break;
                default: f18 = 0;  break;
                }
            }
            else
            {
                if (f9628 > 4)
                    f9628 = 0;
                switch (f9628 - 1)
                {
                case 0: f18 = 2;   break;
                case 1: f18 = 6;   break;
                case 2: f18 = 0xa; break;
                case 3: f18 = 0xc; break;
                default: f18 = 0;  break;
                }
            }
        }
        else
        {
            if (countListA() == 2)
            {
                if (f9628 > 3)
                    f9628 = 0;
            }
            else
            {
                if (f9628 > 4)
                    f9628 = 0;
            }
            switch (f9628 - 1)
            {
            case 0: f18 = 2;   break;
            case 1: f18 = 6;   break;
            case 2: f18 = countListA() == 2 ? 0xb : 0xa; break;
            case 3: f18 = 0xc; break;
            default: f18 = 0;  break;
            }
        }
    }

    g_1b2ea78 = 0;
    if (sndBgm != 0 && f961c != 0)
        tickCoinLamp(sndBgm, 0x1e0);

    /* The join keys: a side can be claimed while it is free. */
    if (countListA() != 2 && fe0 == 0 && f95e4 == 1)
    {
        if ((getListAF8() == 1 && countListA() == 1) || countListA() == 0)
        {
            if (g_bigInstance->getState(4) == 2)
            {
                m44b290(0);
                f9628 = 0;
                f18 = 0;
            }
        }
        if ((getListAF8() == 0 && countListA() == 1) || countListA() == 0)
        {
            if (g_bigInstance->getState(5) == 2)
            {
                m44b290(1);
                f9628 = 0;
                f18 = 0;
            }
        }
    }
    if (g_bigInstance->getState(0) == 2)
        ((ModeNotify *)g_1b2e844)->q00(1);

    switch (f95e4)
    {
    case 0:
        /* Fade in, then hand to the input state. */
        f95e8 += 10;
        if (f95e8 > 0xfe)
        {
            f95e8 = 0xff;
            f95e4 = 1;
        }
        break;
    case 1:
    {
        /* The input state. The long-hold page keys (0x1e up, 0x1d
           down, 150 ticks then every 5) and the four step blocks
           (wheel past +-20 or the step keys, each clearing the option
           counters) all run the same per-page f18 walk; the selection
           blip fires when f18 moved; the white keys confirm to the
           fade-out, keys b/d (P2 13/15) back out of the diff pick.
           NOTE: slots 0x1d/0x1e are NEVER bound in this build -
           bindButtons @0x418c90 (the only addButton caller, reached
           once from init as vtable slot 1) fills 0x00..0x19, tick's
           loop stops at f4 + 1 = 27, and no code in the binary
           writes slotsA[0x1d]/[0x1e] (+0x254/+0x268). Their state
           is always 0, so both long-hold arms are dead here and in
           the song select. (Byte scan, 2026-10-01.) */
        clearCommand();
        if (g_bigInstance->getState(0x1e) == 3 &&
            g_bigInstance->heldFor(0x1e) >= 0x96 && fe0 == 0)
        {
            fcc += 1;
            if (fcc >= 5)
            {
                switch (f9628 - 1)
                {
                case 0:
                    if (countListA() >= 2)
                    {
                        if (f18 < 3) f18 += 1;
                        if (f18 >= 3) f18 = 3;
                    }
                    else
                    {
                        if (f18 < 5) f18 += 1;
                        if (f18 >= 5) f18 = 5;
                    }
                    break;
                case 1:
                    if (countListA() >= 2)
                    {
                        if (f18 < 7) f18 += 1;
                        if (f18 >= 7) f18 = 7;
                    }
                    else
                    {
                        if (f18 < 9) f18 += 1;
                        if (f18 >= 9) f18 = 9;
                    }
                    break;
                case 2:
                    if (countListA() != 2)
                    {
                        if (f18 < 0xb) f18 += 1;
                        if (f18 < 0xb) break;
                    }
                    f18 = 0xb;
                    break;
                case 3:
                    f18 = 0xc;
                    break;
                default:
                    if (f18 < 3) f18 += 1;
                    if (f18 >= 2) f18 = 2;
                    break;
                }
                fcc = 0;
            }
        }
        if (g_bigInstance->getState(0x1d) == 3 &&
            g_bigInstance->heldFor(0x1d) >= 0x96 && fe0 == 0)
        {
            fcc += 1;
            if (fcc >= 5)
            {
                switch (f9628 - 1)
                {
                case 0:
                    if (f18 > 2) f18 -= 1; else f18 = 2;
                    if (f18 <= 2) f18 = 2;
                    break;
                case 1:
                    if (f18 > 6) f18 -= 1; else f18 = 6;
                    if (f18 <= 6) f18 = 6;
                    break;
                case 2:
                    if (countListA() == 2)
                        f18 = 0xb;
                    else
                    {
                        if (f18 > 0xa) f18 -= 1; else f18 = 0xa;
                        if (f18 <= 0xa) f18 = 0xa;
                    }
                    break;
                case 3:
                    f18 = 0xc;
                    break;
                default:
                    if (f18 > 0) f18 -= 1; else f18 = 0;
                    if (f18 <= 0) f18 = 0;
                    break;
                }
                fcc = 0;
            }
        }
        /* The four step blocks: P1/P2 x up/down. Each is the same
           option-counter clear + per-page walk; P1 acts when side 0 is
           joined or both are, P2 the mirror. */
        if (g_bigInstance->getState(4) != 3 &&
            g_bigInstance->getState(5) != 3 && fe0 == 0)
        {
            if (getListAF8() == 0 || countListA() == 2)
            {
                if (g_bigInstance->accum[0] > 0x14 ||
                    g_bigInstance->getState(0xf) == 2 ||
                    g_bigInstance->getState(0x1e) == 2)
                {
g_37dd048[4] = 0; g_37dd048[5] = 0; g_37dd048[6] = 0;
                    g_37dd048[7] = 0; g_37dd048[0] = 0; g_37dd048[2] = 0;
                    g_37dd048[3] = 0;
                    f9614 = 0;
                    f95d4 = 0;
                    switch (f9628 - 1)
                    {
                    case 0:
                        if (countListA() >= 2)
                        {
                            if (f18 < 3) f18 += 1;
                            if (f18 >= 3) f18 = 3;
                        }
                        else
                        {
                            if (f18 < 5) f18 += 1;
                            if (f18 >= 5) f18 = 5;
                        }
                        break;
                    case 1:
                        if (countListA() >= 2)
                        {
                            if (f18 < 7) f18 += 1;
                            if (f18 >= 7) f18 = 7;
                        }
                        else
                        {
                            if (f18 < 9) f18 += 1;
                            if (f18 >= 9) f18 = 9;
                        }
                        break;
                    case 2:
                        if (countListA() != 2)
                        {
                            if (f18 < 0xb) f18 += 1;
                            if (f18 < 0xb) break;
                        }
                        f18 = 0xb;
                        break;
                    case 3:
                        f18 = 0xc;
                        break;
                    default:
                        if (f18 < 3) f18 += 1;
                        if (f18 >= 2) f18 = 2;
                        break;
                    }
                }
                if (g_bigInstance->accum[0] < -0x14 ||
                    g_bigInstance->getState(0x10) == 2 ||
                    g_bigInstance->getState(0x1d) == 2)
                {
g_37dd048[4] = 0; g_37dd048[5] = 0; g_37dd048[6] = 0;
                    g_37dd048[7] = 0; g_37dd048[0] = 0; g_37dd048[2] = 0;
                    g_37dd048[3] = 0;
                    f9614 = 0;
                    f95d4 = 0;
                    switch (f9628 - 1)
                    {
                    case 0:
                        if (f18 > 2) f18 -= 1; else f18 = 2;
                        if (f18 <= 2) f18 = 2;
                        break;
                    case 1:
                        if (f18 > 6) f18 -= 1; else f18 = 6;
                        if (f18 <= 6) f18 = 6;
                        break;
                    case 2:
                        if (countListA() == 2)
                            f18 = 0xb;
                        else
                        {
                            if (f18 > 0xa) f18 -= 1; else f18 = 0xa;
                            if (f18 <= 0xa) f18 = 0xa;
                        }
                        break;
                    case 3:
                        f18 = 0xc;
                        break;
                    default:
                        if (f18 > 0) f18 -= 1; else f18 = 0;
                        if (f18 <= 0) f18 = 0;
                        break;
                    }
                }
            }
            if (getListAF8() == 1 || countListA() == 2)
            {
                if (g_bigInstance->getState(5) != 3)
                {
                    if (g_bigInstance->accum[1] > 0x14 ||
                        g_bigInstance->getState(0x17) == 2)
                    {
g_37dd048[4] = 0; g_37dd048[5] = 0; g_37dd048[6] = 0;
                    g_37dd048[7] = 0; g_37dd048[0] = 0; g_37dd048[2] = 0;
                    g_37dd048[3] = 0;
                    f9614 = 0;
                    f95d4 = 0;
                    switch (f9628 - 1)
                    {
                    case 0:
                        if (countListA() >= 2)
                        {
                            if (f18 < 3) f18 += 1;
                            if (f18 >= 3) f18 = 3;
                        }
                        else
                        {
                            if (f18 < 5) f18 += 1;
                            if (f18 >= 5) f18 = 5;
                        }
                        break;
                    case 1:
                        if (countListA() >= 2)
                        {
                            if (f18 < 7) f18 += 1;
                            if (f18 >= 7) f18 = 7;
                        }
                        else
                        {
                            if (f18 < 9) f18 += 1;
                            if (f18 >= 9) f18 = 9;
                        }
                        break;
                    case 2:
                        if (countListA() != 2)
                        {
                            if (f18 < 0xb) f18 += 1;
                            if (f18 < 0xb) break;
                        }
                        f18 = 0xb;
                        break;
                    case 3:
                        f18 = 0xc;
                        break;
                    default:
                        if (f18 < 3) f18 += 1;
                        if (f18 >= 2) f18 = 2;
                        break;
                    }
                    }
                    if (g_bigInstance->accum[1] < -0x14 ||
                        g_bigInstance->getState(0x18) == 2)
                    {
g_37dd048[4] = 0; g_37dd048[5] = 0; g_37dd048[6] = 0;
                    g_37dd048[7] = 0; g_37dd048[0] = 0; g_37dd048[2] = 0;
                    g_37dd048[3] = 0;
                    f9614 = 0;
                    f95d4 = 0;
                    switch (f9628 - 1)
                    {
                    case 0:
                        if (f18 > 2) f18 -= 1; else f18 = 2;
                        if (f18 <= 2) f18 = 2;
                        break;
                    case 1:
                        if (f18 > 6) f18 -= 1; else f18 = 6;
                        if (f18 <= 6) f18 = 6;
                        break;
                    case 2:
                        if (countListA() == 2)
                            f18 = 0xb;
                        else
                        {
                            if (f18 > 0xa) f18 -= 1; else f18 = 0xa;
                            if (f18 <= 0xa) f18 = 0xa;
                        }
                        break;
                    case 3:
                        f18 = 0xc;
                        break;
                    default:
                        if (f18 > 0) f18 -= 1; else f18 = 0;
                        if (f18 <= 0) f18 = 0;
                        break;
                    }
                    }
                }
            }
        }
        /* The selection blip. */
        if (f18 != f1c)
        {
            if (g_bigInstance->getState(6) != 2)
            {
                ((MsSnd *)sndSelect)->stop();
                ((MsSnd *)sndSelect)->play(0);
            }
            f1c = f18;
        }
        if (countListA() == 1)
        {
            m44b4b0(getListAF8() + 1);
            m44b750();
        }
        else
        {
            m44b4b0(1);
            m44b4b0(2);
            m44b750();
        }
        break;
    }
    case 5:
        /* The diff-style pick: wheel/keys move f95f4 between the two
           panels with the blip, white keys confirm to the fade, the
           black keys back out. */
        f95f0 += 1;
        if (findListA(0) != 0 && f95f0 >= 0xa)
        {
            if (g_bigInstance->accum[0] > 0xa ||
                g_bigInstance->getState(0xf) == 2)
            {
                if (f95f4 != 1)
                {
                    f95f4 = 1;
                    ((MsSnd *)sndSelect)->stop();
                    ((MsSnd *)sndSelect)->play(0);
                }
            }
            else if (g_bigInstance->accum[0] < -0xa ||
                     g_bigInstance->getState(0x10) == 2)
            {
                if (f95f4 != 0)
                {
                    f95f4 = 0;
                    ((MsSnd *)sndSelect)->stop();
                    ((MsSnd *)sndSelect)->play(0);
                }
            }
            if (g_bigInstance->getState(0xa) == 2 ||
                g_bigInstance->getState(0xc) == 2 ||
                g_bigInstance->getState(0xe) == 2)
            {
                ((MsSnd *)sndDecide)->stop();
                ((MsSnd *)sndDecide)->play(0);
                f95e4 = 2;
            }
            if (g_bigInstance->getState(0xb) == 2 ||
                g_bigInstance->getState(0xd) == 2)
            {
                f95f4 = 0;
                f95e4 = 1;
            }
        }
        if (findListA(1) != 0 && f95f0 >= 0xa)
        {
            if (g_bigInstance->accum[1] > 0xa ||
                g_bigInstance->getState(0x17) == 2)
            {
                if (f95f4 != 1)
                {
                    f95f4 = 1;
                    ((MsSnd *)sndSelect)->stop();
                    ((MsSnd *)sndSelect)->play(0);
                }
            }
            else if (g_bigInstance->accum[1] < -0xa ||
                     g_bigInstance->getState(0x18) == 2)
            {
                if (f95f4 != 0)
                {
                    f95f4 = 0;
                    ((MsSnd *)sndSelect)->stop();
                    ((MsSnd *)sndSelect)->play(0);
                }
            }
            if (g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                ((MsSnd *)sndDecide)->stop();
                ((MsSnd *)sndDecide)->play(0);
                f95e4 = 2;
            }
            if (g_bigInstance->getState(0x13) == 2 ||
                g_bigInstance->getState(0x15) == 2)
            {
                f95f4 = 0;
                f95e4 = 1;
            }
        }
        break;
    case 2:
        /* Fade out toward the confirmed state; the mode text lands in
           the hold chart, the BGM stops, and (except mode 12) the
           attract video hands its last frame off through the blit. */
        m_1a40.setCell(1, 0, (int)modeTextM[g_modeIndex]);
        m_1a40.setCell(5, 0, (int)modeText[g_modeIndex]);
        if (sndBgm != 0)
            ((MsSnd *)sndBgm)->stop();
        f95e8 += -0x1e;
        if (f18 != 0xc)
        {
            if (f95e8 < 2)
            {
                f95e8 = 1;
                fd8 = 1;
            }
            if (m_83b8.isRunning() != 0)
            {
                ((ObjB8F0 *)&m_83b8)->reset();
                ((Obj40B980 *)&m_83b8)->submit((int)&g_1b2f138);
                ((Obj40B9A0 *)&m_83b8)->copyOut(&g_1b2e868);
                /* the Video_Blank_256 literal is annotated in
                   src/songselectctor.cpp */
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    "System\\Common\\Video_Blank_256.bmp", &f88c0, 0);
                ((KGraphics *)g_sub1b2f054)->m408040(f88c0, &g_1b2f138,
                                                     g_1b2e868);
            }
            g_1b2e584 = g_37dd048[2] > 0;
        }
        else if (f95e8 < 2)
            f95e8 = 1;
        if (f95e8 == 1)
            f95e4 = 3;
        g_1b2eba8 += 1;
        break;
    case 3:
        /* Confirmed: stop the BGM, run the intro voice (the start
           sting in mode 12), pump f9434 while it plays, load the
           mode's song table once, and leave through the notify hook
           when the hold chart is done and the voice has run. */
        if (((MsSnd *)sndBgm)->playing() & 1)
            ((MsSnd *)sndBgm)->stop();
        if (g_modeIndex != 0xc)
        {
            if (!(((MsSnd *)sndModeIntro)->playing() & 1))
                ((MsSnd *)sndModeIntro)->play(0);
            if (((MsSnd *)sndModeIntro)->playing() & 1)
                f9434 += 0x14;
        }
        else
        {
            if (!(((MsSnd *)sndStart)->playing() & 1))
                ((MsSnd *)sndStart)->play(0);
            if (((MsSnd *)sndStart)->playing() & 1)
                f9434 += 0x14;
        }
        if (fe4 == 0 && fec != 0x1e)
        {
            if (f18 == 6 || f18 == 7 || f18 == 8 || f18 == 9)
                g_songTable.loadSongBin("StreetMix");
            else if (g_modeIndex == 0xc)
                g_songTable.loadSongBin("CV2Mix");
            else
                g_songTable.loadSongBin(g_modeName);
            fe4 = 0x14;
        }
        fec += 0x14;
        if (g_modeIndex == 0xc)
        {
            if (m_2d3c.isDone() != 0 && f9434 >= 0xc8)
                ((ModeNotify *)g_1b2e844)->q00(0);
        }
        if (m_1a40.isDone() != 0 && f9434 >= 0x1a4)
            ((ModeNotify *)g_1b2e844)->q00(0);
        break;
    }
    /* The exit: the diff pick publishes the HARD flags, the video is
       pumped with update2's first argument, the fade level lands in
       g_37dd06c, and the start lamps run. */
    if (f95f4 != 0)
    {
        g_1b2ea20 = 1;
        g_1b2e89c = 1;
    }
    m_83b8.m40b9f0(a);
    g_37dd06c = f95e8;
    ((SongSelectDirector *)this)->updateStartLamps();
}
