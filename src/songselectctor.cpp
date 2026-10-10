/* SongSelectDirector's constructor @0x435390 - 14.7 KB, the biggest function
   in the class and the whole song-select data pipeline in one place. Written
   from the raw listing (scratchpad/ssdir.asm via decode.py + a register-exact
   argument tracker); the reading in wip/songselect-ctor.cpp is the map.

   In order: the forty Obj402860 players (EH states 0..0x27), the camera, a
   long zero block, the fresh-credit entry-name reset, the field defaults, the
   rank/UseKey panels, then the CHANNEL SYSTEM - system\%s\Stage.ini through
   KProfile (state 0x29): the hidden-song table (HID-HD%d), the flattened
   ALLSONG list, and the unlock-gated HID%d appends for the normal modes; the
   carry-over toggles and the radiomix Stage.ini variant (state 0x2a) with
   channel_normal, the three channel_secret%d banks, the eight special-channel
   draw pools (random-star, 7znpfg, 11ptgc, 11waterflame, 11lab) and the
   per-channel stage/level fill for the radio modes. Then the per-song walk (sound\%s\
   resolution, tier probes, disc/title art), eleven samples, the per-mode
   preview voice, and the long asset run into the forty players, ending with
   the style-select block, the rank-font glyph maps and the trailing defaults.

   Port notes live in wip/songselect-ctor.cpp - the pools, the hidden table
   and the preview directories are semantics nothing in ez2play models yet. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "songselect.h"
#include "ez2acapp.h"    /* KGraphics, EzSubsystem, GVec3, g_sub1b2f054 */
#include "kprofile.h"
#include "players.h"     /* ListNode */
#include "bigbase.h"     /* Obj40C180 */
#include "loader.h"      /* KTexture */

/* The song-info reader (bodies in src/songinfo.cpp - the declarations are
   repeated here so the call compiles without exposing the body to cl). */
struct SongInfoOut
{
    char  _pad0[8];
    char  name[0x20c];          /* 0x008 */
    float levels[4][3];         /* 0x214 .. 0x244 */
    char  _pad244[0x370 - 0x244];
    int   version;              /* 0x370 - the record's +0x30, see src/songinfo.cpp */
};
struct Obj431660
{
    char _pad0[0x30];
    int  f30;                   /* 0x30 */

    void loadSongInfo(char *name, SongInfoOut *out);   /* 0x431660 */
};

extern void LogAlt(const char *fmt, ...);  /* 0x402880 (src/applog.cpp, folded) */
extern void Log(const char *fmt, ...);             /* 0x402880 (src/applog.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 (src/nodelists.cpp) */
extern int  getListAF8();                          /* 0x418340 (src/nodelists.cpp) */
extern struct ListNode *findListA(int key);        /* 0x418e30 */

extern EzSubsystem *g_sub1b2f054;   /* 0x1b2f054 (src/ez2acapp.cpp) */
extern EzSubsystem *g_sub1b2eae4;   /* 0x1b2eae4 (src/ez2acapp.cpp) */
extern KProfile *g_iniProfile;      /* 0x530b64 (src/kengineapp.cpp) */
extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */

extern int  g_modeIndex;            /* 0x1b2e548 (src/modename.cpp) */
extern char g_modeName[256];        /* 0x1b2e680 (src/modename.cpp) */
extern char g_channelName[0x80];    /* 0x1b2e728 (src/songselect.cpp) */
extern int  g_elemCount;            /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2eb6c;              /* src/ez2acapp.cpp - hit-percent (CV2) */
extern int  g_1b2e568;              /* src/songselecttoggle.cpp */
extern int  g_catchJudging;         /* 0x1b2e58c (src/playerslot.cpp) */
extern int  g_1b2e714;              /* src/globals.cpp */
extern int  g_1b2e7c8;              /* src/moderunner.cpp */
extern int  g_1b2e7d8;              /* src/globals.cpp */
extern int  g_1b2e7e0;              /* src/globals.cpp - the RadioMix legacy variant */
extern int  g_hidCounts[22];        /* 0x1b2e848 (src/globals.cpp) - the unlock flags */
extern int  g_speedPercent;         /* 0x1b2e890 (src/panels.cpp) */
extern int  g_1b2e89c;              /* src/gamestate.cpp - the gauge-doubling half
                                       of the HARD option */
extern int  g_1b2ea34;              /* src/moderunner.cpp */
extern int  g_fastCount;            /* 0x1b2eaa8 (src/judgegrade.cpp) */
extern int  g_1b2ead4;              /* src/globals.cpp */
extern int  g_1b2eb04[8];           /* src/globals.cpp - the confirmed song name */
extern int  g_1b2eb60;              /* src/gamestate.cpp */
extern int  g_1b2eb74;              /* src/globals.cpp */
extern int  g_1b2ebac;              /* src/gamepanels.cpp */
extern int  g_1b2ebdc;              /* src/gamestate.cpp */
extern int  g_1b2ebf4;              /* src/gamestate.cpp - the song/channel count
                                       mirror */
extern int  g_player0Down;          /* 0x1b2ef90 (src/globals.cpp) */
extern int  g_player1Down;          /* 0x1b2ef64 (src/globals.cpp) */
extern int  g_1b2efa4;              /* src/holdtick.cpp */
extern int  g_1b2efb8;              /* src/globals.cpp */
extern int  g_selectedKey;          /* 0x1b2efd4 (src/globals.cpp) */
extern int  g_1b2f06c;              /* src/endingupdate2.cpp */
extern int  g_1b2f138;              /* src/directors.cpp */
extern int  g_1b5f180;              /* src/maingame.cpp */
extern int  g_1b5f19c;              /* src/playerslot.cpp */
extern int  g_slowCount;            /* 0x1b5f1a0 (src/judgegrade.cpp) */
extern int  g_1b2e7d0;              /* src/gamestate.cpp */
extern int  g_1b2eb7c;              /* src/gamestate.cpp */
extern int  g_37dceac;              /* src/gamestate.cpp */
extern int  g_37dceb0;              /* src/gamestate.cpp */
extern int  g_37dcec8;              /* src/gamestate.cpp */
extern int  g_37dcecc;              /* src/gamestate.cpp */
extern int  g_37dcee8;              /* src/gamestate.cpp */
extern int  g_37dcf00;              /* src/gamestate.cpp */
extern int  g_37dcef8;              /* src/gamestate.cpp */
extern int  g_37dcf08;              /* src/gamestate.cpp */
extern int  g_37dcf0c;              /* src/gamestate.cpp - the carry-over toggles */
extern int  g_37dcf14;              /* src/gamestate.cpp */
extern int  g_37dcf18;              /* src/gamestate.cpp */
extern int  g_37dcf1c;              /* src/gamestate.cpp - the difficulty cursor */
extern char g_stylePrefix[64];      /* 0x1b2e5d0 (src/maingame.cpp) */

/* The five per-element global arrays reset per credit. */
extern int g_1b2f058[5], g_1b2ebe0[5], g_1b5f15c[5], g_1b2f120[5];
extern int g_1b2e650[5], g_1b2ea38[5], g_1b2e9fc[5], g_1b2e668[5];

struct GfxBlob28
{
    int d[10];
};
extern GfxBlob28 g_1b2e868;         /* src/directors.cpp */

/* The rank-font glyph order ("A".."9" then the punctuation set) - a .data
   table; placeholder storage, contents derived from the font sheet. */
// GLOBAL: EZ2AC_UNPACKED 0x004abd18
char g_rankFontChars[0x30];

/* The coin/credit sink's slot pair, as update2 models it. */
struct SsCtorSnd
{
    /* play(loop) is slot 4 (+0x10) and stop() slot 5 (+0x14), as in every
       other sample view; this was one slot short until 2026-10-03, so the
       constructor called slots 3/4 instead (docs/PORT-DELTAS.md). */
    virtual void q0();
    virtual void q1();
    virtual void q2();
    virtual void q3();
    virtual void q4(int v);
    virtual void q5();
};

// FUNCTION: EZ2AC_UNPACKED 0x00435390
SongSelectDirector::SongSelectDirector()
{
    char  val[0x2000];
    char  tok[0x100];
    char  styleName[0x80];
    char  key[0x114];
    char  key2[0x1c];
    char  path[260];
    char  spath[260];
    Obj402860Head *res;
    GVec3 eye;
    GVec3 at;
    GVec3 up;
    int   i;
    int   j;
    int   n;
    int   d;
    char *s;
    char *p;
    char *e;

    // STRING: EZ2AC_UNPACKED 0x0049396c
    Log("SongSelectDirector::Create");

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -600.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    ((KGraphics *)g_sub1b2f054)->setCamera(&eye, &at, &up);
    ((KGraphics *)g_sub1b2f054)->setProjection(0.785398163397f, 10000.0f, 1.0f);
    ((KGraphics *)g_sub1b2f054)->setMaterial(-1);

    f_a0 = 0;
    f94 = 0;
    if (g_elemCount == 0)
        f_bc57c = 0;
    f_c0 = 0;
    f_bc57c = g_1b2efb8;
    f_bc580 = 0;
    f_bc584 = 0.0f;
    f_c0e40 = 0.0f;
    g_1b2eb74 = 0;
    g_slowCount = 0;
    g_fastCount = 0;
    f38 = 0;
    f3c = 0;
    g_catchJudging = 0;
    f_bc578 = 0;
    f_b0 = 0;
    f_b4 = 0;
    f_b8 = 0;
    f_bc = 0;
    f_d4 = 0;
    f_d8 = 0;
    f_dc = 0;
    f_e0 = 0;
    f_e4 = 0;
    f_c0470 = 0.0f;
    f_c0478 = 0;
    f_a4 = 0;
    f64 = 0;
    f_20c0e0 = 0;
    f24 = 0;
    f_c0488 = (g_1b2e568 == 1) ? 0xff : 0x100;
    g_speedPercent = (g_modeIndex == 10) ? 0xc8 : 0xfa;
    g_1b2efa4 = 0;
    g_1b2ebac = 0;
    g_37dcef8 = g_1b2f06c;
    g_1b2f058[g_elemCount] = 0;
    g_1b2ebe0[g_elemCount] = 0;
    g_1b5f15c[g_elemCount] = 0;
    g_1b2f120[g_elemCount] = 0;
    g_1b2e650[g_elemCount] = 0;
    g_1b2ea38[g_elemCount] = 0;
    g_1b2e9fc[g_elemCount] = 0;
    g_1b2e668[g_elemCount] = 0;
    g_player0Down = 0;
    g_player1Down = 0;
    g_1b2ea34 = 0;
    g_1b2e7d8 = 0;
    g_1b2e714 = 0;
    f_bfdcc = 0;
    f98 = 0;
    g_1b2ebdc = 0;
    g_1b2ead4 = 0;
    g_iniProfile->setInt("UseBackground", 1);

    f_bfde8 = 0;
    f_bfde4 = 0;
    f_20c6d4 = 0;
    f6c = 0;
    f20eb0c[0] = 0;
    f20eb0c[1] = 0;
    f20eb0c[2] = 0;
    memset(m_seq, 0, 0x50);
    seqLen[0] = 0;
    seqLen[1] = 0;
    f_c0e30 = 0;
    f_c0e34 = 0;
    f_c0e38 = 0;
    f_c0e3c = 0;
    f_c0e28 = 0;
    f_c0e2c = 0;
    f_c04b4 = 0;
    f_bfdc0 = 0;
    f_bfdc4 = 0;
    f_bfdc8 = 0;
    f_bfdd4 = g_selectedKey;

    /* A fresh credit blanks the entry names - both players' when two are
       in, else just the one getListAF8 names. Every field goes through the
       accessor again, as the original does. */
    if (g_elemCount == 0)
    {
        if (countListA() >= 2)
        {
            for (i = 0; i < 2; i++)
            {
                findListA(i)->f22c = 0;
                if (findListA(i)->f228 > 0)
                {
                    findListA(i)->f228 = 0;
                    findListA(i)->name[0] = ' ';
                    findListA(i)->name[1] = ' ';
                    findListA(i)->name[2] = ' ';
                    findListA(i)->name[3] = ' ';
                    findListA(i)->name[4] = ' ';
                    findListA(i)->name[5] = ' ';
                    findListA(i)->name[6] = ' ';
                    findListA(i)->name[7] = ' ';
                }
            }
        }
        else if (countListA() >= 1)
        {
            i = getListAF8();
            findListA(i)->f22c = 0;
            if (findListA(i)->f228 > 0)
            {
                findListA(i)->f228 = 0;
                findListA(i)->name[0] = ' ';
                findListA(i)->name[1] = ' ';
                findListA(i)->name[2] = ' ';
                findListA(i)->name[3] = ' ';
                findListA(i)->name[4] = ' ';
                findListA(i)->name[5] = ' ';
                findListA(i)->name[6] = ' ';
                findListA(i)->name[7] = ' ';
            }
        }
    }

    f78 = g_modeIndex;
    f74 = 1;
    f_c8 = 0.0f;
    f_c4 = 0.0f;
    f_d0 = 0.0f;
    f_cc = 0.0f;
    f_f0 = -1;
    f_f8 = 0;
    f_f4 = 0;
    f_fc = 0.0f;
    f_100 = -30.0f;
    f_104[0] = 0;
    f_104[1] = 0;
    f_104[2] = 0;
    f_104[3] = 0;
    f_104[4] = 0;
    f_20da3c = 0;
    f_20da40 = 0;
    f_20e894 = 0x1e;
    f_20c698 = 0;
    f_20c69c = 0.0f;
    f_20c6a0 = 0.0f;
    f_bfdec = 0x64;
    f_20c6a4 = 0.0f;
    f_20c6a8 = 0.0f;
    f_20c6b0 = 0xff;
    f_20c6b8 = 0.0f;
    f_20c6ac = 0.0f;
    f_20c6cc = 0;
    f_20c6bc = 0;
    f_20c6c0 = 0.0f;
    f_20c6c4 = 0;
    f_20c6c8 = 0xff;
    f_ec = 0;

    if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00493938
            "System\\SongSelect\\Rank\\p_Rank_panel_mask.bmp",
            (KTexture **)&f_bfecc, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00493910
            "System\\SongSelect\\Rank\\p_Rank_panel.bmp",
            (KTexture **)&f_bfed0, 0);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004938dc
            "System\\SongSelect\\radio_rank\\p_Rank_panel_mask.bmp",
            (KTexture **)&f_bfecc, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004938ac
            "System\\SongSelect\\radio_rank\\p_Rank_panel.bmp",
            (KTexture **)&f_bfed0, 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(
    // STRING: EZ2AC_UNPACKED 0x00493884
        "System\\SongSelect\\TT_UseKey2_panel.bmp", (KTexture **)&f_c04a4, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
    // STRING: EZ2AC_UNPACKED 0x00493858
        "System\\SongSelect\\TT_UseKey2_panel_m.bmp", (KTexture **)&f_c04a8, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
    // STRING: EZ2AC_UNPACKED 0x0049382c
        "System\\SongSelect\\TT_UseKey_catch_panel.bmp", (KTexture **)&f_c04ac, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
    // STRING: EZ2AC_UNPACKED 0x00493800
        "System\\SongSelect\\TT_UseKey_ruby_panel.bmp", (KTexture **)&f_c04b0, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "System\\Common\\Video_Blank_256.bmp", (KTexture **)&f_bc670, 0);
    ((KGraphics *)g_sub1b2f054)->m408040((KTexture *)f_bc670, &g_1b2f138,
                                         g_1b2e868);

    f_20e890 = 0;
    f80 = 0;
    f88 = 0;
    f90 = 0;
    i = 0;
    n = 0;
    memset(val, 0, 0x2000);

    if (f78 != 7 && f78 != 6 && f78 != 8 &&
        f78 != 9)
    {
        sprintf(path, "system\\%s\\Stage.ini", g_modeName);
        {
            KProfile prof(0xc0000, 0);
            prof.Load(path);

            /* 4a. The hidden-song table - HID-HD%d, d = 1 up, giving up
               after thirty missing keys or at a hundred entries. */
            while (f_20e890 < 100)
            {
                d = i + 1;
                // STRING: EZ2AC_UNPACKED 0x004937d0
                sprintf(key, "HID-HD%d", d);
                if (!prof.getStr(key, val))
                {
                    i = d;
                    if (d >= 0x1e)
                        break;
                    continue;
                }
                val[strlen(val) + 1] = 0;
                n = 0;
                memset(tok, 0, 0x100);
                if (val[0] != 0)
                {
                    for (s = val; *s != 0; )
                    {
                        if (*s == ';')
                        {
                            *s = 0;
                            strcpy(m_hidden[f_20e890].name, tok);
                            m_hidden[f_20e890].group = i;
                            f_20e890 += 1;
                            s++;
                            n = 0;
                            memset(tok, 0, 0x100);
                        }
                        else
                        {
                            tok[n++] = *s;
                            s++;
                        }
                    }
                }
                i = d;
            }

            /* 4b. The flattened song list - one ';'-separated ALLSONG value,
               capped at 436 songs, count mirrored after every append. */
            sprintf(path, "ALLSONG");
            prof.getStr(path, val);
            val[strlen(val) + 1] = 0;
            if (val[0] != 0)
            {
                for (p = val; *p != 0; p++)
                {
                    if (*p == ';')
                        *p = 0;
                }
            }
            if (val[0] != 0)
            {
                s = val;
                do
                {
                    n = f80;
                    f80 = n + 1;
                    strcpy((char *)this + 0x5e3c8 + n * 0x374, s);
                    s += strlen(s) + 1;
                    g_1b2ebf4 = f80;
                    if (f80 == 0x1b4)
                        break;
                } while (*s != 0);
            }

            /* 4c. HID%d appends, one per raised unlock flag. */
            n = 0;
            i = 1;
            for (j = 0; ; )
            {
                if (&g_hidCounts[j] >= &g_hidCounts[22])
                    break;
                if (g_hidCounts[j] > 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x004937c8
                    sprintf(key, "HID%d", i);
                    if (!prof.getStr(key, val))
                    {
                        i += 1;
                        j += 1;
                        if (&g_hidCounts[j] >= &g_hidCounts[30])
                            break;
                        if (n >= 0x64)
                            break;
                        continue;
                    }
                    val[0x8] = 0;
                    n = 0;
                    prof.getStr(key, val);
                    val[strlen(val) + 1] = 0;
                    if (val[0] != 0)
                    {
                        for (p = val; *p != 0; p++)
                        {
                            if (*p == ';')
                                *p = 0;
                        }
                    }
                    if (val[0] != 0)
                    {
                        s = val;
                        do
                        {
                            strcpy((char *)this + 0x5e3c8 + (f80 + n) * 0x374, s);
                            s += strlen(s) + 1;
                            n += 1;
                        } while (*s != 0);
                    }
                    f80 += n;
                    g_1b2ebf4 = f80;
                }
                j += 1;
                i += 1;
                if (n >= 0x64)
                    break;
            }
        }
    }

    if (f78 == 7 || f78 == 6 || f78 == 8 ||
        f78 == 9)
    {
        /* 4d. The carry-over toggles from mode select, each behind its own
           once-lock; modes 7/8/9 route theirs through toggleOption. */
        if (g_modeIndex == 6)
        {
            if (g_37dcf0c != 0 && f20eb0c[0] == 0)
            {
                ((SsCtorSnd *)m_sndToggle)->q5();
                ((SsCtorSnd *)m_sndToggle)->q4(0);
                g_1b2e89c = (g_1b2e89c == 0);
            }
            if (g_37dcf14 != 0 && f20eb0c[1] == 0)
            {
                ((SsCtorSnd *)m_sndToggle)->q5();
                ((SsCtorSnd *)m_sndToggle)->q4(0);
                if (g_1b2e568 != 0)
                    goto disableTargetBar;
                g_1b2e568 = 1;
                Log("Enable FIX TARGETBAR");
            }
        }
        else if (g_modeIndex == 8 || g_modeIndex == 9)
        {
            if (g_37dcf0c != 0)
                toggleOption(0);
            if (g_37dcf14 != 0 && f20eb0c[1] == 0)
            {
                ((SsCtorSnd *)m_sndToggle)->q5();
                ((SsCtorSnd *)m_sndToggle)->q4(0);
                if (g_1b2e568 != 0)
                    goto disableTargetBar;
                g_1b2e568 = 1;
                LogAlt("Enable FIX TARGETBAR");
            }
        }
        else if (g_modeIndex == 7)
        {
            if (g_37dcf0c != 0)
                toggleOption(0);
            if (g_37dcf14 != 0)
                toggleOption(1);
        }
        goto thirdToggle;
disableTargetBar:
        g_1b2e568 = 0;
        Log("Disable FIX TARGETBAR");
thirdToggle:
        if (g_37dcf18 != 0 && f20eb0c[2] == 0)
        {
            ((SsCtorSnd *)m_sndToggle)->q5();
            ((SsCtorSnd *)m_sndToggle)->q4(0);
            g_catchJudging = (g_catchJudging == 0);
        }

        if (g_modeIndex == 7 && g_1b2e7e0 != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00493738
            sprintf(path, "system\\radiomix\\Stage_0.ini");
        }
        else if (g_modeIndex == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0049371c
            sprintf(path, "system\\radiomix\\Stage.ini");
        }
        else
        {
            sprintf(path, "system\\%s\\Stage.ini", g_modeName);
        }

        {
            KProfile prof(0xc0000, 0);
            prof.Load(path);

            /* 4e. The channel list proper. */
            // STRING: EZ2AC_UNPACKED 0x00491c08
            prof.getStr("channel_normal", val);
            val[strlen(val) + 1] = 0;
            if (val[0] != 0)
            {
                for (p = val; *p != 0; p++)
                {
                    if (*p == ';')
                        *p = 0;
                }
            }
            if (val[0] != 0)
            {
                s = val;
                do
                {
                    n = f7c;
                    f7c = n + 1;
                    strcpy(rows[n].name, s);
                    s += strlen(s) + 1;
                } while (*s != 0);
            }

            /* The three secret banks, each gated on its unlock lock. */
            for (i = 0; i < 3; i++)
            {
                if (f20eb0c[i] > 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493708
                    sprintf(key2, "channel_secret%d", i + 1);
                    val[8] = 0;
                    prof.getStr(key2, val);
                    val[strlen(val) + 1] = 0;
                    if (val[0] != 0)
                    {
                        for (p = val; *p != 0; p++)
                        {
                            if (*p == ';')
                                *p = 0;
                        }
                    }
                    (&f_20da3c)[i] = 0;
                    if (val[0] != 0)
                    {
                        s = val;
                        do
                        {
                            strcpy(rows[f7c + (&f_20da3c)[i]].name, s);
                            s += strlen(s) + 1;
                            (&f_20da3c)[i] += 1;
                        } while (*s != 0);
                    }
                    f7c += (&f_20da3c)[i];
                }
            }

            /* 4f/4g. Per channel: its name read back as a key gives the
               stage list - eight special names route to the draw pools,
               everything else fills the five-stage table - then %s_level. */
            for (i = 0; i < f7c; i++)
            {
                prof.getStr(rows[i].name, val);
                val[strlen(val) + 1] = 0;
                if (val[0] != 0)
                {
                    for (p = val; *p != 0; p++)
                    {
                        if (*p == ';')
                            *p = 0;
                    }
                }
                g_1b2ebf4 = f7c;
                s = val + 8;
                if (_stricmp(rows[i].name, "random") == 0)
                {
                    f_20bd94 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_poolRandom[j], s);
                        s += strlen(s) + 1;
                        f_20bd94 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "random2") == 0)
                {
                    f_20bd98 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_poolRandom2[j], s);
                        s += strlen(s) + 1;
                        f_20bd98 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "random3") == 0)
                {
                    f_20bd9c = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_poolRandom3[j], s);
                        s += strlen(s) + 1;
                        f_20bd9c += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "7znpfg") == 0)
                {
                    f_20bda0 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_pool7znpfg[j], s);
                        s += strlen(s) + 1;
                        f_20bda0 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "random4") == 0)
                {
                    f_20bda4 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_poolRandom4[j], s);
                        s += strlen(s) + 1;
                        f_20bda4 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "11ptgc") == 0)
                {
                    f_20bda8 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_pool11ptgc[j], s);
                        s += strlen(s) + 1;
                        f_20bda8 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "11waterflame") == 0)
                {
                    f_20bdac = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_pool11waterflame[j], s);
                        s += strlen(s) + 1;
                        f_20bdac += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else if (_stricmp(rows[i].name, "11lab") == 0)
                {
                    f_20bdb0 = 0;
                    for (j = 0; j < 400; j++)
                    {
                        strcpy(m_pool11lab[j], s);
                        s += strlen(s) + 1;
                        f_20bdb0 += 1;
                        if (*s == 0)
                            break;
                    }
                }
                else
                {
                    /* An ordinary channel: up to five stages. */
                    e = m_stageTable[i];
                    for (j = 5; j != 0; j--)
                    {
                        strcpy(e, s);
                        s += strlen(s) + 1;
                        e += 0x104;
                        if (*s == 0)
                            break;
                    }
                }
                // STRING: EZ2AC_UNPACKED 0x00491bd4
                sprintf(path, "%s_level", rows[i].name);
                prof.getStr(path, val);
                val[strlen(val) + 1] = 0;
                m_courseLevels[i] = atoi(val);
                Log("Course Name = %s", path);
                Log("Course Level = %02d", m_courseLevels[i]);
            }

            /* The channel wheel's disc + info art, per mode family. */
            for (i = 0; i < f7c; i++)
            {
                if (f78 == 7)
                {
                    // STRING: EZ2AC_UNPACKED 0x004936ac
                    sprintf(path, "system\\channel_disc\\radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)((char *)this + 0x12c + i * 0x374), 0);
                    // STRING: EZ2AC_UNPACKED 0x00493688
                    sprintf(path, "system\\channel_info\\radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20d9d4[i], 0);
                }
                if (f78 == 6)
                {
                    // STRING: EZ2AC_UNPACKED 0x00493664
                    sprintf(path, "system\\channel_disc\\5radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)((char *)this + 0x12c + i * 0x374), 0);
                    // STRING: EZ2AC_UNPACKED 0x00493640
                    sprintf(path, "system\\channel_info\\5radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20d9d4[i], 0);
                }
                if (f78 == 8)
                {
                    // STRING: EZ2AC_UNPACKED 0x0049361c
                    sprintf(path, "system\\channel_disc\\10radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)((char *)this + 0x12c + i * 0x374), 0);
                    // STRING: EZ2AC_UNPACKED 0x004935f8
                    sprintf(path, "system\\channel_info\\10radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20d9d4[i], 0);
                }
                if (f78 == 9)
                {
                    // STRING: EZ2AC_UNPACKED 0x004935d4
                    sprintf(path, "system\\channel_disc\\14radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)((char *)this + 0x12c + i * 0x374), 0);
                    // STRING: EZ2AC_UNPACKED 0x004935b0
                    sprintf(path, "system\\channel_info\\14radio\\%s.bmp", rows[i].name);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20d9d4[i], 0);
                }
            }
            ((KGraphics *)g_sub1b2f054)->loadTexture(
                // STRING: EZ2AC_UNPACKED 0x00493584
                "system\\channel_disc\\common\\radiomask.bmp", (KTexture **)&f_bc44c, 0);
        }
    }
    else
    {
        /* 5. The per-song walk: resolve sound\%s\, probe the tiers, load the
           disc and title art. */
        if (f80 > 0)
        {
            for (i = 0; i < f80; i++)
            {
                e = (char *)this + 0x5e3c8 + i * 0x374;
                sprintf(path, "sound\\%s\\", e);
                changeDir(path, 1);
                ((struct Obj431660 *)this)->loadSongInfo(e, (SongInfoOut *)(e - 0x10c));
                popDirectory();
                for (j = 0; j <= 4; j++)
                    m430cf0(e, e + 0x138 + j * 0x28, e + 0x200 + j * 0x14, j);
                if (m431810(e) == 1)
                    *(float *)(e + 0x114) = 0.0f;
                if (m431900(e) == 2)
                    *(float *)(e + 0x120) = 0.0f;
                if (m4319f0(e) == 3)
                    *(float *)(e + 0x12c) = 0.0f;
                if (g_1b2eb6c == 0)
                {
                    sprintf(spath, "system\\discsmall\\%s.bmp", e);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(spath,
                        (KTexture **)(e - 0x10c), 0);
                }
                else
                {
                    sprintf(spath, "system\\CV2Mix\\Disc\\cv2disc_%02d.bmp", i + 1);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(spath,
                        (KTexture **)(e - 0x10c), 0);
                }
                if (g_1b2eb6c == 1)
                {
                    m430f50(i);
                    sprintf(path, "system\\CV2Mix\\SongName\\%s.bmp", path + 4);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)(e - 0x108), 0);
                    // STRING: EZ2AC_UNPACKED 0x004937a0
                    Log("%d, %s", i, spath);
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x0048cfb8
                    sprintf(path, "system\\songname\\%s.bmp", e);
                    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                        (KTexture **)(e - 0x108), 0);
                }
            }
        }
        // STRING: EZ2AC_UNPACKED 0x00493784
        sprintf(spath, "system\\disc\\initdisc.bmp");
        // STRING: EZ2AC_UNPACKED 0x00493770
        ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDiscNormal", spath,
                                                (Sprite **)&f_104[0]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            "system\\disc\\disc-mask.bmp", (KTexture **)&f_bc44c, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            // STRING: EZ2AC_UNPACKED 0x00493754
            "system\\disc\\shape_mask.bmp", (KTexture **)&f_bc450, 0);
        f_c8 = 0.0f;
        f_d0 = 0.0f;
    }

    if (f78 != 6 && f78 != 7 && f78 != 8 && f78 != 9 && f78 != 0xc)
    {
        m435070(g_1b2efb8, 1, 1);
        f84 = f2c;
        f_cc = f_c4 = (float)f2c * 100.0f;
    }

    /* 6. The select audio. */
    m_sndToggle = (EzSoundObject *)g_sub1b2eae4->createSample(
        "system\\ModeSelect\\hiddencmd.wav", 0, 0);
    m_bfd2c = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x00493544
        "system\\modeselect\\modeoutro.wav", 0, 0);
    if (g_1b2eb6c != 0)
    {
        m_bfd34 = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x00493504
            "system\\CV2Mix\\Common\\count.wav", 0, 0);
    }
    else
    {
        m_bfd34 = (EzSoundObject *)g_sub1b2eae4->createSample(
            "system\\ModeSelect\\timecount.wav", 0, 0);
    }
    m_bfd44 = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x004934dc
        "System\\CV2Mix\\ModeSelect\\Modeselect.wav", 1, 0);
    if (g_1b2eb6c == 1)
    {
        m_sndMove = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x004934bc
            "system\\CV2Mix\\Common\\select.wav", 0, 0);
    }
    else if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
    {
        m_sndMove = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x0049349c
            "system\\SongSelect\\select.wav", 0, 0);
    }
    else
    {
        m_sndMove = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x0049347c
            "system\\SongSelect\\select2.wav", 0, 0);
    }
    if (g_1b2eb6c != 0)
    {
        m_sndMove2 = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x0049343c
            "system\\CV2Mix\\Common\\decide.wav", 0, 0);
    }
    else
    {
        m_sndMove2 = (EzSoundObject *)g_sub1b2eae4->createSample(
            // STRING: EZ2AC_UNPACKED 0x0049345c
            "system\\SongSelect\\decide.wav", 0, 0);
    }
    m_bfd38 = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x00493418
        "system\\SongSelect\\LevelChange.wav", 0, 0);
    m_bfd3c = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x004933f4
        "system\\SongSelect\\NoLevelChange.wav", 0, 0);
    m_bfd18 = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x004933d4
        "system\\SongSelect\\EffectTab.wav", 0, 0);
    m_bfd1c = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x004933b0
        "system\\SongSelect\\EffectSound.wav", 0, 0);
    m_bfd40 = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x0049338c
        "system\\SongSelect\\SkinChange.wav", 0, 0);

    /* The preview voice, per mode family. */
    if (f78 == 6)
    {
        // STRING: EZ2AC_UNPACKED 0x0049336c
        sprintf(spath, "system\\preview_5radio\\%s.wav", rows[f84].name);
    }
    else if (f78 == 8)
    {
        // STRING: EZ2AC_UNPACKED 0x0049334c
        sprintf(spath, "system\\preview_10radio\\%s.wav", rows[f84].name);
    }
    else if (f78 == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x0049332c
        sprintf(spath, "system\\preview_14radio\\%s.wav", rows[f84].name);
    }
    else if (f78 == 7)
    {
        if (g_1b2e7e0 != 0)
            sprintf(spath, "system\\preview_legacy\\%s.wav", rows[f84].name);
        else
            sprintf(spath, "system\\preview_radio\\%s.wav", rows[f84].name);
    }
    else
    {
        n = 0;
        if (f7c > 0)
        {
            for (n = 0; n < f7c; n++)
            {
                if (_stricmp((const char *)g_1b2eb04, rows[n].name) == 0)
                    break;
            }
            if (n >= f7c)
                n = 0;
        }
        // STRING: EZ2AC_UNPACKED 0x004932f8
        sprintf(spath, "system\\Preview\\%s.wav", rows[n].name);
    }
    m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(spath, 1, 0);
    if (!g_sub1b2eae4->loadSample((struct EzSample *)m_sndPreview, spath))
    {
        m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
            "System\\Preview\\0Blank.wav", 1, 0);
    }

    /* 7. The texture and clip runs. */
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "system\\modeselect\\fadeblack.bmp", (KTexture **)&f_bc454, 0);
    if (g_1b2eb6c != 0)
    {
        for (i = 0; i < 10; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x004932ac
            sprintf(path, "System\\CV2Mix\\ModeSelect\\Time\\time%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&m_bd31c[i], 0);
            // STRING: EZ2AC_UNPACKED 0x0049327c
            sprintf(path, "System\\CV2Mix\\ModeSelect\\Time\\time%02d_mask.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&m_bd344[i], 0);
        }
    }
    else
    {
        for (i = 0; i < 10; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00493250
            sprintf(path, "system\\SongSelect\\VF\\TimeFont\\Time_%d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&m_bd31c[i], 0);
        }
    }
    for (i = 0; i < 0x2f; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00493220
        sprintf(path, "System\\SongSelect\\Sortimage\\category_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bc4b8[i], 0);
    }
    for (i = 0; i < 0xf; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004931ec
        sprintf(path, "system\\songselect\\channellistnum\\channel_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bc58c[i], 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004931b4
        "system\\songselect\\channellistnum\\channel_list_bg.bmp",
        (KTexture **)&f_bc5c8, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00493178
        "system\\songselect\\channellistnum\\channel_list_cursor.bmp",
        (KTexture **)&f_bc5cc, 0);

    if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x0049315c
        changeDir("System\\Channel_Info\\common\\", 1);
        // STRING: EZ2AC_UNPACKED 0x0049314c
        ((KGraphics *)g_sub1b2f054)->loadStr("info_Panel.str", &res);
        m_20d7b8.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x00493140
        ((KGraphics *)g_sub1b2f054)->loadStr("info_u.str", &res);
        m_20d164.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x00493134
        ((KGraphics *)g_sub1b2f054)->loadStr("info_d.str", &res);
        m_20d380.attach(res, 0);
        popDirectory();
    }

    if (g_1b2eb6c != 0)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            // STRING: EZ2AC_UNPACKED 0x00493104
            "system\\cv2mix\\modeselect\\effector\\effector.str", &res);
        m_bf278.attach(res, 1);
    }
    /* The attach is written into BOTH arms - cl cross-jumps the two copies
       back into one (the original's first arm jumps into the second's), and
       the per-arm sprintf+loadStr copies survive. One shared attach after
       the if/else lets cl merge the loadStr too, which is a call short. */
    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004930dc
        sprintf(spath, "system\\CV2Mix\\ModeSelect\\BG\\tgbg.str");
        ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
        m_bc890.attach(res, 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x004930ac
        sprintf(spath, "system\\SongSelect\\bg\\default\\songselect_bg.str");
        ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
        m_bc890.attach(res, 1);
    }
    // STRING: EZ2AC_UNPACKED 0x0049308c
    ((KGraphics *)g_sub1b2f054)->loadStr("System\\Disc\\TT_DiscCloseUp.str", &res);
    m_c09e8.attach(res, 0);
    // STRING: EZ2AC_UNPACKED 0x00493064
    ((KGraphics *)g_sub1b2f054)->loadStr("system\\SongSelect\\caution\\caution.str", &res);
    m_c0c04.attach(res, 1);

    if (f78 != 7 && f78 != 6 && f78 != 8 && f78 != 9)
    {
        if (g_1b2eb6c != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0049303c
            sprintf(spath, "System\\CV2Mix\\ModeSelect\\tgselect.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
            m_bccc8.attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x00493018
            sprintf(spath, "System\\CV2Mix\\disc\\tglevelicon.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
            m_bf05c.attach(res, 1);
            m_bf05c.setCell(4, 0, *(int *)((char *)this + 0x5e2bc));
            ((KGraphics *)g_sub1b2f054)->loadTexture(
                // STRING: EZ2AC_UNPACKED 0x00492fec
                "system\\CV2Mix\\Targetname\\0-BackPanel.bmp", (KTexture **)&f_bc5d0, 0);
            ((KGraphics *)g_sub1b2f054)->loadTexture(
                // STRING: EZ2AC_UNPACKED 0x00492fbc
                "system\\CV2Mix\\Targetname\\0-BackPanel_mask.bmp",
                (KTexture **)&f_bc5d4, 0);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00492f90
            sprintf(spath, "System\\SongSelect\\VF\\songselect_layout.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
            m_bccc8.attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x00492f68
            sprintf(spath, "System\\SongSelect\\VF\\Selectcursor.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
            m_bcee4.attach(res, 1);
        }
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00492f44
        sprintf(spath, "System\\SongSelect\\CourseLayout.str");
        ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
        m_bcaac.attach(res, 1);
    }

    changeDir("System\\ModeSelect\\", 1);
    for (i = 1; i <= 12; i++)
    {
        sprintf(path, "VF_ModeInOut\\modetext%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bc458[i - 1], 0);
        sprintf(path, "VF_ModeInOut\\modetext%02d_m.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bc488[i - 1], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x00492ed4
    sprintf(spath, "VF_ModeInout\\Mode_Outro.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
    m_bc674.attach(res, 0);
    m_bc674.setCell(4, 0, f_bc488[g_modeIndex]);
    m_bc674.setCell(5, 0, f_bc458[g_modeIndex]);
    popDirectory();

    for (i = 0; i < 0x13; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492ea8
        sprintf(path, "System\\SongSelect\\Version\\Version_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bff64[4 + i], 0);
        // STRING: EZ2AC_UNPACKED 0x00492e74
        sprintf(path, "System\\SongSelect\\V_View\\version\\Version_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)(&f_bffc0[0] + i), 0);
    }
    for (i = 0; i < 0x1e; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492e3c
        sprintf(path, "System\\SongSelect\\V_View\\sortstyle\\sortstyle_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_c000c[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x00492e10
    ((KGraphics *)g_sub1b2f054)->loadStr("System\\SongSelect\\Version\\ani_Version.str", &res);
    m_be8d8.attach(res, 0);
    // STRING: EZ2AC_UNPACKED 0x00492de4
    ((KGraphics *)g_sub1b2f054)->loadStr("System\\SongSelect\\V_View\\Version_View.str", &res);
    m_beaf4.attach(res, 0);
    // STRING: EZ2AC_UNPACKED 0x00492db8
    ((KGraphics *)g_sub1b2f054)->loadStr("System\\SongSelect\\V_View\\Sortstyle_View.str", &res);
    m_bed10.attach(res, 0);
    m_bed10.setCell(2, 0, f_c000c[0]);
    m_beaf4.setCell(2, 0,
        (&f_bffc0[0])[*(int *)((char *)this + 0x49c + f84 * 0x374)]);

    if (g_1b2eb6c == 0)
    {
        changeDir("System\\common\\", 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYER.str", &res);
        m_bd3c0.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYER.str", &res);
        m_bd5dc.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERInsertCoin.str", &res);
        m_bd7f8.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERInsertCoin.str", &res);
        m_bda14.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERPressStart.str", &res);
        m_bdc30.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERPressStart.str", &res);
        m_bde4c.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERWait.str", &res);
        m_be068.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERWait.str", &res);
        m_be284.attach(res, 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00492cf8
        changeDir("System\\CV2Mix\\common\\", 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYER.str", &res);
        m_bd3c0.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYER.str", &res);
        m_bd5dc.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERWait.str", &res);
        m_be068.attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERWait.str", &res);
        m_be284.attach(res, 1);
    }
    popDirectory();

    if (f78 == 0xa)
    {
        for (i = 0; i < 0xa; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00492cc0
            sprintf(path, "System\\SongSelect\\Effector\\Effector_catch_skin%d.bmp", i + 1);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bfdf0[i], 0);
        }
        for (i = 0; i < 7; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00492c88
            sprintf(path, "System\\SongSelect\\Effector\\Effector_catch_player%d.bmp", i + 1);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bfe18[i], 0);
        }
        // STRING: EZ2AC_UNPACKED 0x00492c50
        sprintf(spath, "System\\SongSelect\\Effector\\Effector_catch_turn_on.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe34[1], 0);
        // STRING: EZ2AC_UNPACKED 0x00492c18
        sprintf(spath, "System\\SongSelect\\Effector\\Effector_catch_turn_off.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe34[0], 0);
        // STRING: EZ2AC_UNPACKED 0x00492be8
        sprintf(spath, "System\\SongSelect\\Effector\\EFFECTOR_4D_on.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe3c[1], 0);
        // STRING: EZ2AC_UNPACKED 0x00492bb8
        sprintf(spath, "System\\SongSelect\\Effector\\EFFECTOR_4D_off.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe3c[0], 0);
        // STRING: EZ2AC_UNPACKED 0x00492b80
        sprintf(spath, "System\\SongSelect\\Effector\\Effector_catch_2_3_on.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe3c[3], 0);
        // STRING: EZ2AC_UNPACKED 0x00492b48
        sprintf(spath, "System\\SongSelect\\Effector\\Effector_catch_2_3_off.bmp");
        ((KGraphics *)g_sub1b2f054)->loadTexture(spath, (KTexture **)&f_bfe3c[2], 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492b18
        "System\\SongSelect\\Effector\\EFFECTOR_CATE_1.bmp", (KTexture **)&f_bfe50, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492ae8
        "System\\SongSelect\\Effector\\EFFECTOR_CATE_2.bmp", (KTexture **)&f_bfe54, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492ab8
        "System\\SongSelect\\Effector\\EFFECTOR_CATE_x.bmp", (KTexture **)&f_bfe58, 0);
    for (i = 0; i < 2; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492a88
        sprintf(path, "System\\cv2mix\\modeselect\\effector\\efR_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bef2c[i], 0);
    }
    for (i = 0; i < 0x15; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492a54
        sprintf(path, "System\\cv2mix\\modeselect\\effector\\efSpeed_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bef34[i], 0);
    }
    for (i = 0; i < 2; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492a20
        sprintf(path, "System\\cv2mix\\modeselect\\effector\\efnote_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_befe4[i], 0);
    }
    for (i = 0; i < 2; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004929f0
        sprintf(path, "System\\cv2mix\\modeselect\\effector\\efbp_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_befec[i], 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004929bc
        "System\\SongSelect\\Effector\\Effector_Reverse_off.bmp", (KTexture **)&f_bfe5c[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492988
        "System\\SongSelect\\Effector\\Effector_Reverse_on.bmp", (KTexture **)&f_bfe5c[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "System\\SongSelect\\Effector\\EFFECTOR_4D_on.bmp", (KTexture **)&f_bfe5c[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492958
        "System\\SongSelect\\Effector\\EFFECTOR_5D_on.bmp", (KTexture **)&f_bfe5c[3], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492924
        "System\\SongSelect\\Effector\\Effector_Scan_Off.bmp", (KTexture **)&f_bfe5c[9], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004928f4
        "System\\SongSelect\\Effector\\Effector_Scan_On.bmp", (KTexture **)&f_bfe5c[10], 0);
    for (i = 0; i < 0xc; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004928d0
        sprintf(path, "System\\SongSelect\\bpm\\bpm_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bc638[i], 0);
    }
    for (i = 0; i < 0xc; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0049289c
        sprintf(path, "System\\SongSelect\\Effector\\EFFECTOR_NOTE_%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_bfe9c[i], 0);
    }
    if (g_modeIndex == 1)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            // STRING: EZ2AC_UNPACKED 0x0049286c
            "System\\SongSelect\\Effector\\EffectPanel_Ruby.str", &res);
    }
    else if (g_modeIndex == 0xa)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            // STRING: EZ2AC_UNPACKED 0x00492838
            "System\\SongSelect\\Effector\\EffectPanel_catch.str", &res);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            // STRING: EZ2AC_UNPACKED 0x0049280c
            "System\\SongSelect\\Effector\\EffectPanel.str", &res);
    }
    m_20cb10.attach(res, 1);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004927d8
        "System\\SongSelect\\Effector\\EFFECTOR_FADEOUT_OFF.bmp", (KTexture **)&f_bfed4[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004927a8
        "System\\SongSelect\\Effector\\EFFECTOR_FADEOUT.bmp", (KTexture **)&f_bfed4[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492778
        "System\\SongSelect\\Effector\\EFFECTOR_FADEIN.bmp", (KTexture **)&f_bfed4[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492748
        "System\\SongSelect\\Effector\\EFFECTOR_BLINK.bmp", (KTexture **)&f_bfed4[3], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492714
        "System\\SongSelect\\Effector\\EFFECTOR_SWITCH_OFF.bmp", (KTexture **)&f_bff18[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004926e4
        "System\\SongSelect\\Effector\\EFFECTOR_SWITCH.bmp", (KTexture **)&f_bff18[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004926b0
        "System\\SongSelect\\Effector\\EFFECTOR_RANDOM_OFF.bmp", (KTexture **)&f_bfee8[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492680
        "System\\SongSelect\\Effector\\EFFECTOR_RANDOM.bmp", (KTexture **)&f_bfee8[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492650
        "System\\SongSelect\\Effector\\EFFECTOR_SRANDOM.bmp", (KTexture **)&f_bfee8[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492624
        "System\\SongSelect\\Effector\\EFFECTOR_PS.bmp", (KTexture **)&f_bfee8[3], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004925f8
        "System\\SongSelect\\Effector\\EFFECTOR_FR.bmp", (KTexture **)&f_bfee8[4], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004925c8
        "System\\SongSelect\\Effector\\EFFECTOR_MRANDOM.bmp", (KTexture **)&f_bfee8[5], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492598
        "System\\SongSelect\\Effector\\EFFECTOR_HRANDOM.bmp", (KTexture **)&f_bfee8[6], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492568
        "System\\SongSelect\\Effector\\EFFECTOR_MIRROR.bmp", (KTexture **)&f_bfee8[7], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492534
        "System\\SongSelect\\Effector\\EFFECTOR_MIRROR_A.bmp", (KTexture **)&f_bfee8[8], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492508
        "System\\SongSelect\\Effector\\EFFECTOR_KEY.bmp", (KTexture **)&f_bfee8[9], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004924dc
        "System\\SongSelect\\Effector\\EFFECTOR_SP.bmp", (KTexture **)&f_bfee8[10], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004924ac
        "System\\SongSelect\\Effector\\EFFECTOR_4D_OFF.bmp", (KTexture **)&f_bff20[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "System\\SongSelect\\Effector\\EFFECTOR_4D_on.bmp", (KTexture **)&f_bff20[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "System\\SongSelect\\Effector\\EFFECTOR_5D_on.bmp", (KTexture **)&f_bff20[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492478
        "System\\SongSelect\\Effector\\EFFECTOR_AUTOS_off.bmp", (KTexture **)&f_bff34[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492448
        "System\\SongSelect\\Effector\\EFFECTOR_AUTOS_L.bmp", (KTexture **)&f_bff34[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492418
        "System\\SongSelect\\Effector\\EFFECTOR_AUTOS_R.bmp", (KTexture **)&f_bff34[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004923e4
        "System\\SongSelect\\Effector\\EFFECTOR_AUTOS_on.bmp", (KTexture **)&f_bff34[3], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004923ac
        "System\\SongSelect\\Effector\\EFFECTOR_AUTO_SPEDAL_OFF.bmp", (KTexture **)&f_bff64[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492374
        "System\\SongSelect\\Effector\\EFFECTOR_AUTO_SCRATCH_ON.bmp", (KTexture **)&f_bff64[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x0049233c
        "System\\SongSelect\\Effector\\EFFECTOR_AUTO_PEDAL_ON.bmp", (KTexture **)&f_bff64[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492304
        "System\\SongSelect\\Effector\\EFFECTOR_AUTO_SPEDAL_ON.bmp", (KTexture **)&f_bff64[3], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004922cc
        "System\\SongSelect\\Effector\\EFFECTOR_AUTO_DISABLE.bmp", (KTexture **)&f_bff54, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x0049229c
        "System\\SongSelect\\Effector\\EFFECTOR_BG_ON.bmp", (KTexture **)&f_bff58[0], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x0049226c
        "System\\SongSelect\\Effector\\EFFECTOR_BLACK.bmp", (KTexture **)&f_bff58[1], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x0049223c
        "System\\SongSelect\\Effector\\EFFECTOR_BG_OFF.bmp", (KTexture **)&f_bff58[2], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00492208
        "System\\SongSelect\\Effector\\effector_judgechange.bmp",
        (KTexture **)&f_c0480, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x004921d0
        "System\\SongSelect\\Effector\\effector_judgechange_m.bmp",
        (KTexture **)&f_c0484, 0);
    // STRING: EZ2AC_UNPACKED 0x004921a8
    sprintf(path, "System\\SongSelect\\Stage\\TT_Stage_%d.bmp", g_elemCount);
    ((KGraphics *)g_sub1b2f054)->loadTexture(path,
        (KTexture **)&f_c0084[g_elemCount], 0);
    for (i = 0; i < 3; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492178
        sprintf(path, "System\\CV2Mix\\modeselect\\Stage\\stage_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path,
            (KTexture **)&f_c0084[4 + i], 0);
        // STRING: EZ2AC_UNPACKED 0x00492144
        sprintf(path, "System\\CV2Mix\\modeselect\\Stage\\stage_%02d_mask.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path,
            (KTexture **)&f_c0084[7 + i], 0);
    }
    if (g_1b2eb6c != 0)
    {
        for (i = 0; i < 0xa; i++)
        {
            sprintf(path, "System\\CV2Mix\\modeselect\\Lvfont\\lv_%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&f_c0320[i], 0);
        }
    }
    else
    {
        for (i = 0; i < 0x16; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x004920f0
            sprintf(path, "System\\SongSelect\\lvfont\\ALV_%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&t_c0348[0] + i, 0);
            // STRING: EZ2AC_UNPACKED 0x004920c8
            sprintf(path, "System\\SongSelect\\lvfont\\ALV_%02d_m.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&t_c03a0[0] + i, 0);
            // STRING: EZ2AC_UNPACKED 0x004920a4
            sprintf(path, "System\\SongSelect\\lvfont\\LV%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&f_c02c8[0] + i, 0);
            // STRING: EZ2AC_UNPACKED 0x00492074
            sprintf(path, "System\\SongSelect\\LargeLvFont\\Level_%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path,
                (KTexture **)&f_c040c[0] + i, 0);
        }
    }
    // STRING: EZ2AC_UNPACKED 0x00492044
    sprintf(spath, "system\\songselect\\LargeLvFont\\levelcursor.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
    m_20cd2c.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492018
    sprintf(spath, "system\\songselect\\Lvfont\\levelcursor2.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(spath, &res);
    m_20cf48.attach(res, 1);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491ff4
        "system\\songselect\\lvfont\\alv_23.bmp", (KTexture **)&f_c0400, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491fcc
        "system\\songselect\\lvfont\\alv_23_m.bmp", (KTexture **)&f_c0404, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491fa0
        "system\\songselect\\LargeLvFont\\Level_23.bmp",
        (KTexture **)&f_c040c[21], 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491f6c
        "system\\songselect\\largeLvFont\\level_sel_Grad.bmp",
        (KTexture **)&f_c0468, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491f38
        "system\\songselect\\largeLvFont\\Level_top_font.bmp",
        (KTexture **)&f_c0464, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491f14
        "system\\SongSelect\\lvfont\\alv_na.bmp", (KTexture **)&f_c03f8, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491eec
        "system\\SongSelect\\lvfont\\alv_na_m.bmp", (KTexture **)&f_c03fc, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491ec8
        "System\\SongSelect\\vf\\b_mask_2.bmp", (KTexture **)&f_c046c, 0);

    /* The per-mode option-readout seed - the four rows of m_20cb10 (or
       CV2's m_bf278), in the original's block order. */
    switch (g_modeIndex)
    {
    case 0:
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(5, 0, f_bfe9c[0]);
        m_20cb10.setCell(6, 0, f_bff54);
        break;
    case 1:
        m_20cb10.setCell(3, 0, f_bfe58);
        m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
        m_20cb10.setCell(5, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
        break;
    case 2:
    case 3:
    case 4:
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(5, 0, f_bfe9c[0]);
        m_20cb10.setCell(6, 0, f_bff64[g_37dcee8]);
        break;
    case 6:
    case 7:
    case 8:
        /* The radio modes stack all four values into row 3, as the
           original does. */
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(3, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(3, 0, f_bfe9c[0]);
        m_20cb10.setCell(3, 0, f_bff64[g_37dcee8]);
        break;
    case 5:
    case 9:
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(5, 0, f_bfe9c[0]);
        m_20cb10.setCell(6, 0, f_bff34[g_37dcee8]);
        break;
    case 10:
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(4, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(5, 0, f_bfe18[g_37dceb0]);
        m_20cb10.setCell(6, 0, f_bfdf0[g_37dceac]);
        break;
    case 11:
        m_20cb10.setCell(3, 0, f_bfe50);
        m_20cb10.setCell(4, 0, f_bfed4[g_37dcec8]);
        m_20cb10.setCell(5, 0, f_bfee8[g_37dcecc]);
        m_20cb10.setCell(6, 0, f_bff18[g_1b2e7d0]);
        break;
    case 12:
        m_bf278.setCell(8, 0, f_bef2c[g_37dcecc]);
        m_bf278.setCell(7, 0, f_bef34[g_37dcef8]);
        m_bf278.setCell(6, 0, f_befe4[g_37dcf00]);
        m_bf278.setCell(5, 0, f_befec[g_1b2eb7c]);
        break;
    }

    // STRING: EZ2AC_UNPACKED 0x00491ea8
    changeDir("System\\SongSelect\\StyleSelect\\", 1);
    // STRING: EZ2AC_UNPACKED 0x00491e94
    ((KGraphics *)g_sub1b2f054)->loadStr("STYLE_SELECT.str", &res);
    m_c05a8.attach(res, 0);
    // STRING: EZ2AC_UNPACKED 0x00491e7c
    ((KGraphics *)g_sub1b2f054)->loadStr("STYLE_SELECT_nend.str", &res);
    m_c07c4.attach(res, 1);
    popDirectory();
    *(float *)((char *)&m_c05a8 + 0x214) = 322.0f;
    *(float *)((char *)&m_c05a8 + 0x218) = 316.0f;
    *(float *)((char *)&m_c07c4 + 0x214) = 322.0f;
    *(float *)((char *)&m_c07c4 + 0x218) = 316.0f;
    for (i = 0; i < 3; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00491e40
        sprintf(path, "system\\SongSelect\\StyleSelect\\STYLE_EZ2CATCH%d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_c0498[i], 0);
    }
    switch (g_modeIndex)
    {
    case 0:
        f_c09e0 = 0x15;
        break;
    case 1:
        f_c09e0 = 0xc;
        break;
    case 2:
    case 6:
        f_c09e0 = 0x16;
        break;
    case 3:
    case 7:
        f_c09e0 = 0x12;
        break;
    case 4:
    case 8:
        f_c09e0 = 0xf;
        break;
    case 5:
    case 9:
        f_c09e0 = 0xe;
        break;
    case 10:
        f_c09e0 = 0xa;
        break;
    case 11:
        f_c09e0 = 0x9;
        break;
    case 12:
        f_c09e0 = 0x2;
        break;
    }
    if (f_c09e0 > 0)
    {
        for (i = 0; i < f_c09e0; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00491e10
            sprintf(path, "system\\SongSelect\\StyleSelect\\STYLE_%s%d.bmp",
                    g_modeName, i + 1);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_c04b8[i * 2], 0);
        }
    }
    m_c05a8.setCell(2, 0, f_c04b8[g_37dcf1c * 2]);
    m_c07c4.setCell(2, 0, f_c04b8[g_37dcf1c * 2]);
    sprintf(styleName, "%s_", g_modeName);
    strcpy(g_stylePrefix, styleName);
    g_1b2e7c8 = g_37dcf1c;
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        // STRING: EZ2AC_UNPACKED 0x00491df0
        "system\\songrank\\songrank.bmp", (KTexture **)&f_20bdc8, 0);
    if (g_1b2eb6c == 0)
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            // STRING: EZ2AC_UNPACKED 0x00491dc8
            "system\\SongSelect\\VF\\o_scrollcusor.bmp", (KTexture **)&f_20c0cc, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            // STRING: EZ2AC_UNPACKED 0x00491d9c
            "system\\SongSelect\\VF\\o_scrollcusor_m.bmp", (KTexture **)&f_20c0d0, 0);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            // STRING: EZ2AC_UNPACKED 0x00491d70
            "system\\CV2Mix\\modeselect\\o_ListCursor.bmp", (KTexture **)&f_20c0cc, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            // STRING: EZ2AC_UNPACKED 0x00491d48
            "system\\CV2Mix\\modeselect\\o_ListBar.bmp", (KTexture **)&f_bf058, 0);
    }

    /* The rank-font glyph maps: clear both halves to -1, then fill the
       inverse char map twice - once per glyph sheet, as the original does. */
    for (i = 0; i < 0x80; i++)
    {
        f_20c240[i] = -1;
        f_20c240[i + 0x80] = -1;
    }
    for (i = 0; i < 0x30; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00491d28
        changeDir("System\\SongSelect\\rank\\font\\", 1);
        f_20c240[g_rankFontChars[i]] = i;
        // STRING: EZ2AC_UNPACKED 0x00491d18
        sprintf(path, "srf_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20be8c[i], 0);
        popDirectory();
    }
    for (i = 0; i < 0x30; i++)
    {
        f_20c240[g_rankFontChars[i]] = i;
        // STRING: EZ2AC_UNPACKED 0x00491cf8
        sprintf(path, "system\\SongRank\\schar_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)&f_20c0e4[i], 0);
    }

    /* 8. Trailing defaults. */
    {
        Obj40C180 *g = g_bigInstance;
        g->accum[0] = 0;
        g->accum[1] = 0;
        g->limit = 10;
    }
    f14 = 0;
    f_bfd48 = 0x50;
    f_bfd4c = 10;
    f_bfd60 = 10;
    f_bfd58 = 10;
    f_bfda8 = 0;
    f_bfda4 = 0;
    f_bfdac = 0;
    f_bfd50 = 0;
    f_bfd54 = 0;
    f_bfdb0 = 0;
    f_bfdb8 = 1;
    g_1b5f180 = 0;
    g_1b2eb60 = -1;
    if (g_elemCount == 0)
        f_bfdb4 = 1;
    else
        f_bfdb4 = 2;
    if (g_elemCount <= 2)
        f_bfdb4 = 1;
    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
        loadRankingFile(rows[f84].name);
}
