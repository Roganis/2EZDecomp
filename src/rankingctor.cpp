/* RankingDirector's constructor @0x457a90 - the ranking / name-entry screen's
   7.6KB builder. The class's teardown is matched (src/directors.cpp); the
   reading behind this reconstruction is wip/rankingdir-ctor.cpp and
   scratchpad/rank_ctor.{asm,txt}.

   Structure: eleven member constructors (states 0..10), the radio-family test,
   a coin-counter early out, the screen's textures / six sounds / the mode-plate
   composite, the phase seeding (with the original's own dead phase-5 stores -
   an unconditional phase=0 follows them in the binary), the ascii->glyph map
   and the name-entry fonts, the two mode-wide ranking tables (create-if-absent
   with the fwrite-8-from-stride-50 quirk, then read), the score computation
   per session shape, the rank insertion into both tables, and three big arms
   (name entry / 2P / solo) that stamp the entered or saved names in. The
   repeated result-less findListA calls are a compiled-out debug macro's
   argument evaluations - the calls survive, the loads died. */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "directors.h"
#include "ez2acapp.h"   /* KGraphics, EzSubsystem, g_sub1b2f054, g_sub1b2eae4 */
#include "bigbase.h"    /* Obj40C180, g_bigInstance */
#include "players.h"    /* ListNode, PlayerSlot */

extern void Log(const char *fmt, ...);             /* 0x402880 (src/applog.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 (src/nodelists.cpp) */
extern int  countListC();                          /* 0x4184a0 (src/nodelists.cpp) */
extern int  getListAF8();                          /* 0x418340 (src/nodelists.cpp) */
extern ListNode *findListA(int key);               /* 0x418e30 (src/nodelists.cpp) */
extern ListNode *removeFromListA(ListNode *p);     /* 0x4182b0 (src/nodelists.cpp) */
extern "C" unsigned long __stdcall timeGetTime(void);

extern char g_modeName[256];       /* 0x1b2e680 (src/modename.cpp) */
extern int  g_modeIndex;           /* 0x1b2e548 (src/modename.cpp) */
extern char g_channelName[0x80];   /* 0x1b2e728 (src/songselect.cpp) */
extern int  g_elemCount;           /* 0x1b2e718 (src/loops.cpp) */
extern void *g_1b2e844;            /* src/moderunner.cpp */
extern int  g_1b2eb44;             /* src/gamestate.cpp */
extern int  g_1b2ea28;             /* src/moderunner.cpp */
extern int  g_1b2f024;             /* src/moderunner.cpp */

/* The graphics blob the video-blit setter takes by value (src/directors.cpp). */
struct GfxBlob28
{
    int v[10];
};
extern GfxBlob28 g_1b2e868;        /* src/directors.cpp */
extern int g_1b2f138;              /* src/directors.cpp */

/* Obj40B2E0's +0x43c setter, matched under a placeholder class name in
   src/misc.cpp - the same view cast src/modeselectctor.cpp uses. */
struct Obj40B9E0 { void setField43c(int v); };

/* New globals evidenced only here. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7ac
int g_playCounter;                 /* clamped to 0x8235 on the way in */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e54c
int g_bonusScore[4];               /* per-player, logged as "Current Bonus Score" */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f050
int g_1b2f050;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef80
int g_1b2ef80;
// GLOBAL: EZ2AC_UNPACKED 0x01b2efbc
int g_1b2efbc;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f028
int g_1b2f028;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f0a8
int g_1b2f0a8;
/* The replay flag at 0x37dd27c lives in src/backgrounds.cpp, pinned directly
   below g_gfVideoName whose biased operand lands on it. */
extern char g_37dd27c[4];
/* The last entered player name, kept across screens beside the root's object
   record @0x4a8e60. */
// GLOBAL: EZ2AC_UNPACKED 0x004a8e8c
char g_savedName[8];

/* The name-entry charset - 50 characters, one per r_fon_%02d glyph. It lives
   in .data right after the class's object record, i.e. a file-scope array of
   this TU, not a pooled literal. */
// GLOBAL: EZ2AC_UNPACKED 0x004ae454
char kEntryChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>()*^_/-;. eb";

/* The constructor's own view of the object past the modeled members
   (include/directors.h stays untouched; the same convention as the sink
   views). Offsets are this-relative. */
struct RankView
{
    char _pad0[0x10];
    int  phase;                    /* 0x0010 - 5 skips the screen, 4 on radio */
    char _pad14[0x1c20 - 0x14];
    struct KTexture *rfon[50];     /* 0x1c20 - r_fon_%02d */
    struct KTexture *nrf[50];      /* 0x1ce8 - nrf_%02d */
    struct KTexture *scfont[10];   /* 0x1db0 - scfont_%1d */
    char _pad1dd8[0x1e00 - 0x1dd8];
    struct KTexture *whiteEntry[10];  /* 0x1e00 - white_entry_%1d */
    struct KTexture *whiteEntry10;    /* 0x1e28 */
    struct KTexture *timeFont[10];    /* 0x1e2c - time_%d */
    char _pad1e54[0x1e58 - 0x1e54];
    struct KTexture *tex[12];      /* 0x1e58 - modetext%02d */
    char _pad1e88[4];
    struct KTexture *maskTex[12];  /* 0x1e8c - modetext%02d_m */
    char _pad1ebc[0x1ec0 - 0x1ebc];
    struct KTexture *modeIcon;     /* 0x1ec0 */
    struct KTexture *videoBlank;   /* 0x1ec4 */
    char _pad1ec8[0x1ecc - 0x1ec8];
    int  glyphMap[0x80];           /* 0x1ecc - ascii -> glyph index, -1 empty */
    char names[99][0x32];          /* 0x20cc - the mode-wide table's names */
    char _pad3422[0x7c1bc - 0x3422];
    int  scores99[99];             /* 0x7c1bc */
    char _pad7c348[0x85df8 - 0x7c348];
    char isRadioFamily;            /* 0x85df8 */
    char _pad85df9[3];
    int  radioRank[2];             /* 0x85dfc */
    char radioNames[99][0x32];     /* 0x85e04 */
    char _pad8715a[0xffef4 - 0x8715a];
    int  radioScores99[99];        /* 0xffef4 */
    char _pad100080[0x109b30 - 0x100080];
    int  f109b30;                  /* 0x109b30 */
    float f109b34;                 /* 0x109b34 */
    float f109b38;                 /* 0x109b38 */
    char _pad109b3c[0x109b40 - 0x109b3c];
    int  f109b40;                  /* 0x109b40 */
    int  f109b44;                  /* 0x109b44 */
    int  rank[2];                  /* 0x109b48 - each player's slot, 99 = none */
    int  score[2];                 /* 0x109b50 */
    int  f109b58;                  /* 0x109b58 - logged as "Current Score" */
    char _pad109b5c[0x109d84 - 0x109b5c];
    void *sample[6];               /* 0x109d84 - the five ranking sounds + intro */
    int  f109d9c;                  /* 0x109d9c */
    struct KTexture *fadeTex;      /* 0x109da0 - fadeblack.bmp */
    int  f109da4;                  /* 0x109da4 - 0xff */
    int  f109da8[2];               /* 0x109da8 - 0x30 while entering */
    int  f109db0[2];               /* 0x109db0 - 8 while entering */
    char _pad109db8[0x109dc0 - 0x109db8];
    float f109dc0;                 /* 0x109dc0 */
    float f109dc4;                 /* 0x109dc4 */
    float f109dc8;                 /* 0x109dc8 */
    float f109dcc;                 /* 0x109dcc */
    int  editFlag[2];              /* 0x109dd0 */
    char _pad109dd8[4];
    int  f109ddc;                  /* 0x109ddc - 0x28 */
    int  f109de0;                  /* 0x109de0 */
    int  winner;                   /* 0x109de4 - which player enters a name */
};

/* The fields of the list-A player this screen reads - a view over PlayerSlot
   (include/players.h models the slot's own extent; these sit in its pads). */
struct RankStage
{
    int  s254;                     /* +0x254 + i*0x40 */
    int  s258;                     /* +0x258 + i*0x40 */
    char _pad[0x40 - 8];
};

struct RankSlotView
{
    char _pad0[0x1e4];
    int  f1e4;                     /* 0x1e4 - one mode family's session score */
    char _pad1e8[4];
    int  f1ec;                     /* 0x1ec - the other family's */
    int  f1f0;                     /* 0x1f0 - the total this ctor publishes */
    char _pad1f4[0x220 - 0x1f4];
    char name[8];                  /* 0x220 - the player's entered name */
    int  f228;                     /* 0x228 - nonzero while the name is valid */
    char _pad22c[0x254 - 0x22c];
    RankStage stage[8];            /* 0x254 - per-stage score pairs */
};

// FUNCTION: EZ2AC_UNPACKED 0x00457a90
RankingDirector::RankingDirector()
{
    RankView *v = (RankView *)this;
    Obj402860Head *res;
    char path[260];
    char name[260];
    char radioPath[260];
    char iconPath[260];
    int  i;
    int  j;
    int  sc;
    FILE *f;
    Obj40C180 *g;

    if (strcmp(g_modeName, "RadioMix") == 0 ||
        strcmp(g_modeName, "5RadioMix") == 0 ||
        // STRING: EZ2AC_UNPACKED 0x00493bf4
        strcmp(g_modeName, "10RadioMix") == 0 ||
        // STRING: EZ2AC_UNPACKED 0x00493be8
        strcmp(g_modeName, "14RadioMix") == 0)
    {
        v->isRadioFamily = 1;
    }
    else
    {
        v->isRadioFamily = 0;
    }

    if (g_playCounter >= 0x8235)
    {
        g_playCounter = 0x8235;
        popDirectory();
        ((SoundBank *)g_1b2e844)->v0(0);
        return;
    }

    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Common\\Video_Blank_256.bmp",
                                             &v->videoBlank, 0);
    ((KGraphics *)g_sub1b2f054)->m408040(v->videoBlank, &g_1b2f138, g_1b2e868);
    v->f109d9c = 0;
    // STRING: EZ2AC_UNPACKED 0x00498ab8
    sprintf(iconPath, "System\\Ranking\\VRanking\\Modeicon\\icon_%s.bmp", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(iconPath, &v->modeIcon, 0);

    // STRING: EZ2AC_UNPACKED 0x00498a98
    v->sample[0] = (void *)g_sub1b2eae4->createSample("system\\ranking\\ranking_bgm.wav", 1, 0);
    // STRING: EZ2AC_UNPACKED 0x00498a78
    v->sample[1] = (void *)g_sub1b2eae4->createSample("system\\ranking\\rankingfall.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00498a58
    v->sample[2] = (void *)g_sub1b2eae4->createSample("system\\ranking\\rankingmove.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00498a38
    v->sample[3] = (void *)g_sub1b2eae4->createSample("system\\ranking\\rankingnext.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00498a20
    v->sample[4] = (void *)g_sub1b2eae4->createSample("system\\ranking\\rank.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00498a00
    v->sample[5] = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\ModeIntro.wav", 1, 0);

    // STRING: EZ2AC_UNPACKED 0x004989e0
    changeDir("system\\modeselect\\VF_ModeInOut\\", 1);
    for (i = 0; i < 12; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004989cc
        sprintf(name, "modetext%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &v->tex[i], 0);
        // STRING: EZ2AC_UNPACKED 0x004989b8
        sprintf(name, "modetext%02d_m.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &v->maskTex[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004989a8
    sprintf(name, "Mode_Intro.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_109b68.attach(res, 0);
    m_109b68.setCell(4, 0, (int)v->maskTex[g_modeIndex]);
    m_109b68.setCell(5, 0, (int)v->tex[g_modeIndex]);
    popDirectory();

    /* The original's own dead stores: four conditions raise phase 5, an
       unconditional zero follows, and the tests below re-raise it. */
    if (g_1b2eb44 == 1)
    {
        v->phase = 5;
        g_1b2f028 = 0;
    }
    if (g_1b2ea28 == 1)
        v->phase = 5;
    if (countListA() >= 2)
        v->phase = 5;
    if (countListC() >= 2)
        v->phase = 5;
    v->phase = 0;
    if (g_1b2f024 != 0)
    {
        if (countListA() == 1)
            v->winner = getListAF8();
        if (m456870() == 0)
            v->phase = 5;
    }
    if (!(g_1b2f050 == 0 && g_1b2ef80 == 0 && g_1b2efbc == 0))
        v->phase = 5;
    if (v->phase == 5)
    {
        if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
            g_modeIndex == 9)
        {
            ((SoundBank *)g_1b2e844)->v0(0);
            v->phase = 4;
        }
        if (g_elemCount >= 2)
        {
            ((SoundBank *)g_1b2e844)->v0(0);
            v->phase = 4;
        }
        return;
    }

    for (i = 0; i < 0x80; i++)
        v->glyphMap[i] = -1;
    for (i = 0; i < 50; i++)
    {
        v->glyphMap[kEntryChars[i]] = i;
        // STRING: EZ2AC_UNPACKED 0x0049897c
        sprintf(path, "system\\ranking\\VRanking\\Font\\r_fon_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->rfon[i], 0);
        v->glyphMap[kEntryChars[i]] = i;
        // STRING: EZ2AC_UNPACKED 0x00498958
        sprintf(path, "system\\ranking\\font\\nrf_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->nrf[i], 0);
    }
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00498934
        sprintf(path, "system\\ranking\\font\\scfont_%1d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->scfont[i], 0);
        // STRING: EZ2AC_UNPACKED 0x00498910
        sprintf(path, "system\\ranking\\white_entry_%1d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->whiteEntry[i], 0);
    }
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004988e4
        sprintf(path, "system\\Ranking\\VRanking\\Font\\time_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->timeFont[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004988c0
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\ranking\\white_entry_10.bmp",
                                             &v->whiteEntry10, 0);

    if (countListA() > 1)
    {
        // STRING: EZ2AC_UNPACKED 0x004986e0
        sprintf(path, "system\\ranking\\ranking_%s_battle.bin", g_modeName);
        if (v->isRadioFamily != 0)
            // STRING: EZ2AC_UNPACKED 0x004986b8
            sprintf(radioPath, "system\\ranking\\ranking_%s_%s_battle.bin", g_modeName, g_channelName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00498698
        sprintf(path, "system\\ranking\\ranking_%s.bin", g_modeName);
        if (v->isRadioFamily != 0)
            // STRING: EZ2AC_UNPACKED 0x00491984
            sprintf(radioPath, "system\\ranking\\ranking_%s_%s.bin", g_modeName, g_channelName);
    }

    if (_access(path, 0) == -1)
    {
        for (j = 0; j < 99; j++)
        {
            v->names[j][0] = 'E';
            v->names[j][1] = 'Z';
            v->names[j][2] = '2';
            v->names[j][3] = 'A';
            v->names[j][4] = 'C';
            v->names[j][5] = '_';
            v->names[j][6] = 'F';
            v->names[j][7] = 'N';
            v->scores99[j] = 0;
        }
        // STRING: EZ2AC_UNPACKED 0x00488294
        f = fopen(path, "wb");
        fwrite(v->names, 8, 99, f);
        fwrite(v->scores99, 4, 99, f);
        fclose(f);
    }
    f = fopen(path, "rb");
    fread(v->names, 8, 99, f);
    fread(v->scores99, 4, 99, f);
    fclose(f);

    if (v->isRadioFamily != 0)
    {
        if (_access(radioPath, 0) == -1)
        {
            for (j = 0; j < 99; j++)
            {
                v->radioNames[j][0] = 'E';
                v->radioNames[j][1] = 'Z';
                v->radioNames[j][2] = '2';
                v->radioNames[j][3] = 'A';
                v->radioNames[j][4] = 'C';
                v->radioNames[j][5] = '_';
                v->radioNames[j][6] = 'F';
                v->radioNames[j][7] = 'N';
                v->radioScores99[j] = 0;
            }
            f = fopen(radioPath, "wb");
            fwrite(v->radioNames, 8, 99, f);
            fwrite(v->radioScores99, 4, 99, f);
            fclose(f);
        }
        f = fopen(radioPath, "rb");
        fread(v->radioNames, 8, 99, f);
        fread(v->radioScores99, 4, 99, f);
        fclose(f);
    }

    v->rank[1] = 99;
    v->rank[0] = 99;
    v->radioRank[1] = 99;
    v->radioRank[0] = 99;
    v->score[1] = 0;
    v->score[0] = 0;

    if (countListA() == 1)
    {
        for (i = 0; i <= g_elemCount; i++)
        {
            v->score[0] += ((RankSlotView *)findListA(getListAF8()))->stage[i].s258
                         + ((RankSlotView *)findListA(getListAF8()))->stage[i].s254
                         + g_bonusScore[i];
            findListA(getListAF8());
            findListA(getListAF8());
        }
        sc = v->score[0];
        ((RankSlotView *)findListA(getListAF8()))->f1f0 = sc;
    }
    else if (countListA() == 2)
    {
        if (g_modeIndex != 1)
        {
            if (g_modeIndex > 3 && g_modeIndex <= 5)
            {
                sc = ((RankSlotView *)findListA(0))->f1e4;
                v->score[0] = sc;
                ((RankSlotView *)findListA(0))->f1f0 = sc;
                sc = ((RankSlotView *)findListA(1))->f1e4;
                v->score[1] = sc;
                ((RankSlotView *)findListA(1))->f1f0 = sc;
            }
            else
            {
                sc = ((RankSlotView *)findListA(0))->f1ec;
                v->score[0] = sc;
                ((RankSlotView *)findListA(0))->f1f0 = sc;
                sc = ((RankSlotView *)findListA(1))->f1ec;
                v->score[1] = sc;
                ((RankSlotView *)findListA(1))->f1f0 = sc;
            }
        }
        else
        {
            for (i = 0; i <= g_elemCount; i++)
            {
                v->score[0] += ((RankSlotView *)findListA(0))->stage[i].s258
                             + ((RankSlotView *)findListA(0))->stage[i].s254;
                v->score[1] += ((RankSlotView *)findListA(1))->stage[i].s258
                             + ((RankSlotView *)findListA(1))->stage[i].s254;
            }
            sc = v->score[0];
            ((RankSlotView *)findListA(0))->f1f0 = sc;
            sc = v->score[1];
            ((RankSlotView *)findListA(1))->f1f0 = sc;
        }
    }

    v->f109b40 = 0;
    for (i = 0; i < 2; i++)
    {
        int sc = v->score[i];

        for (j = 0; j < 99; j++)
        {
            if (v->scores99[j] < sc)
            {
                v->rank[i] = j;
                break;
            }
        }
        if (v->rank[i] < 99)
        {
            for (j = 98; j > v->rank[i]; j--)
            {
                v->scores99[j] = v->scores99[j - 1];
                v->names[j][0] = v->names[j - 1][0];
                v->names[j][1] = v->names[j - 1][1];
                v->names[j][2] = v->names[j - 1][2];
                v->names[j][3] = v->names[j - 1][3];
                v->names[j][4] = v->names[j - 1][4];
                v->names[j][5] = v->names[j - 1][5];
                v->names[j][6] = v->names[j - 1][6];
                v->names[j][7] = v->names[j - 1][7];
            }
            v->scores99[v->rank[i]] = v->score[i];
            v->names[v->rank[i]][0] = ' ';
            v->names[v->rank[i]][1] = ' ';
            v->names[v->rank[i]][2] = ' ';
            v->names[v->rank[i]][3] = ' ';
            v->names[v->rank[i]][4] = ' ';
            v->names[v->rank[i]][5] = ' ';
            v->names[v->rank[i]][6] = ' ';
            v->names[v->rank[i]][7] = ' ';
        }
    }
    if (g_1b2f024 == 0 && countListA() >= 2 && v->rank[1] <= v->rank[0])
        v->rank[0] = v->rank[0] + 1;

    if (v->isRadioFamily != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00495cb0
        Log("Current Score = %d", v->f109b58);
        /* Plain ascending. The accumulator's register is decided by an
           allocator seed that shifts with the REST OF THE PROGRAM: at first
           reconstruction no permutation of this chain reached the original's
           bytes (a0+a1+a3+a2 got closest, one add direction off, parked as
           an effective match); after two more TUs were added the seed
           re-rolled and the natural order became byte-exact; after two MORE
           it re-rolled back; after two more it flipped a FOURTH time, to a
           state where the natural order was byte-exact; and one TU later it
           flipped a FIFTH, back to a0+a1+a3+a2; and a SIXTH, back to the
           natural order.

           The fifth flip took one build and no sweep, which is the point of
           the note below - read the diff, pick the other form, done.

           A SEVENTH flip on 2026-08-25 (adding src/resultupdate.cpp) landed
           on the REGISTER side, so it stands as an effective match at 99.89%
           and there is nothing to do. READ IT WITH `scratchpad/za.py
           0x457a90`, NOT WITH reccmp: two seconds against ten minutes, and
           the answer is one hunk - `add eax,edx` / `add eax,[..]` against our
           `add edx,eax` / `add edx,[..]` is the register case, while
           `[3]` loading before `[2]` is the steerable one. Everything else
           zdiff prints for this function is relocation masking (`fs:0x0`
           against `fs:A`), not a difference.

           So this line is a standing coin, not a one-off, and it has now
           been observed in three distinct states. Do NOT sweep the
           permutations when it drops - it is a ONE-LINE reorder between two
           known forms, and the diff says which:

             only the LOAD ORDER wrong ([3] before [2])  -> flip to the other
               of a0+a1+a2+a3 / a0+a1+a3+a2; that half IS steerable.
             the ACCUMULATOR REGISTER wrong (add into the [2] register where
               the original uses the [3] one) -> not steerable at all; eight
               shapes were tried (four permutations, a named total, a
               parenthesised pair sum, two combinations) and the best any
               reaches is the effective match. Take it and move on.

               IT RE-ROLLS whenever a TU is added, so do not re-sweep it -
               read the diff instead. If only the LOAD ORDER is wrong, flip
               the last two terms between [2]+[3] and [3]+[2] and rebuild
               once; if the ACCUMULATOR REGISTER is the only difference, it
               is already the effective match and there is nothing to do.
               (Re-rolled nine times so far; the term order in this line is
               therefore meaningless as documentation of the original.) */
        // STRING: EZ2AC_UNPACKED 0x004988a4
        Log("Current Bonus Score = %d", g_bonusScore[0] + g_bonusScore[1] + g_bonusScore[2] + g_bonusScore[3]);
        for (i = 0; i < 2; i++)
        {
            int sc = v->score[i];

            for (j = 0; j < 99; j++)
            {
                if (v->radioScores99[j] < sc)
                {
                    v->radioRank[i] = j;
                    break;
                }
            }
            if (v->radioRank[i] < 99)
            {
                for (j = 98; j > v->radioRank[i]; j--)
                {
                    v->radioScores99[j] = v->radioScores99[j - 1];
                    v->radioNames[j][0] = v->radioNames[j - 1][0];
                    v->radioNames[j][1] = v->radioNames[j - 1][1];
                    v->radioNames[j][2] = v->radioNames[j - 1][2];
                    v->radioNames[j][3] = v->radioNames[j - 1][3];
                    v->radioNames[j][4] = v->radioNames[j - 1][4];
                    v->radioNames[j][5] = v->radioNames[j - 1][5];
                    v->radioNames[j][6] = v->radioNames[j - 1][6];
                    v->radioNames[j][7] = v->radioNames[j - 1][7];
                }
                v->radioScores99[v->radioRank[i]] = v->score[i];
                v->radioNames[v->radioRank[i]][0] = ' ';
                v->radioNames[v->radioRank[i]][1] = ' ';
                v->radioNames[v->radioRank[i]][2] = ' ';
                v->radioNames[v->radioRank[i]][3] = ' ';
                v->radioNames[v->radioRank[i]][4] = ' ';
                v->radioNames[v->radioRank[i]][5] = ' ';
                v->radioNames[v->radioRank[i]][6] = ' ';
                v->radioNames[v->radioRank[i]][7] = ' ';
            }
        }
        if (v->radioRank[1] <= v->radioRank[0])
            v->radioRank[0] = v->radioRank[0] + 1;
    }

    v->editFlag[1] = 0;
    v->editFlag[0] = 0;
    if (countListA() >= 2)
    {
        for (i = 0; i < 2; i++)
        {
            ((PlayerSlot *)findListA(i))->m42e2e0();
            for (j = 0; j < 3; j++)
            {
                findListA(i);
                findListA(i);
            }
            for (j = 0; j < 3; j++)
            {
                findListA(i);
                findListA(i);
            }
        }
    }
    else
    {
        for (j = 0; j < 3; j++)
        {
            findListA(getListAF8());
            findListA(getListAF8());
        }
        ((PlayerSlot *)findListA(getListAF8()))->m42e2e0();
        for (j = 0; j < 3; j++)
        {
            findListA(getListAF8());
            findListA(getListAF8());
        }
    }

    v->f109da4 = 0xff;
    v->f109b44 = 0;
    v->f109db0[0] = 0;
    v->f109db0[1] = 0;
    v->f109da8[0] = 0;
    v->f109da8[1] = 0;
    v->f109dc8 = 0.0f;
    v->f109dc0 = 0.0f;
    v->f109dcc = 0.0f;
    v->f109dc4 = 0.0f;
    v->f109b34 = 0.0f;
    v->f109b38 = 0.0f;
    v->f109ddc = 0x28;

    if (g_1b2f024 != 0)
    {
        g_37dd27c[0] = 0;
        v->rank[0] = 0;
        v->rank[1] = 0;
        v->editFlag[1] = 1;
        v->editFlag[0] = 1;
        v->editFlag[v->winner] = 0;
        v->scores99[v->rank[0]] = v->score[0];
        v->names[v->rank[0]][0] = ' ';
        v->names[v->rank[0]][1] = ' ';
        v->names[v->rank[0]][2] = ' ';
        v->names[v->rank[0]][3] = ' ';
        v->names[v->rank[0]][4] = ' ';
        v->names[v->rank[0]][5] = ' ';
        v->names[v->rank[0]][6] = ' ';
        v->names[v->rank[0]][7] = ' ';
        v->scores99[v->rank[1]] = v->score[1];
        v->names[v->rank[1]][0] = ' ';
        v->names[v->rank[1]][1] = ' ';
        v->names[v->rank[1]][2] = ' ';
        v->names[v->rank[1]][3] = ' ';
        v->names[v->rank[1]][4] = ' ';
        v->names[v->rank[1]][5] = ' ';
        v->names[v->rank[1]][6] = ' ';
        v->names[v->rank[1]][7] = ' ';
        if (((RankSlotView *)findListA(v->winner))->f228 > 0)
        {
            v->names[v->rank[v->winner]][0] = ((RankSlotView *)findListA(v->winner))->name[0];
            v->names[v->rank[v->winner]][1] = ((RankSlotView *)findListA(v->winner))->name[1];
            v->names[v->rank[v->winner]][2] = ((RankSlotView *)findListA(v->winner))->name[2];
            v->names[v->rank[v->winner]][3] = ((RankSlotView *)findListA(v->winner))->name[3];
            v->names[v->rank[v->winner]][4] = ((RankSlotView *)findListA(v->winner))->name[4];
            v->names[v->rank[v->winner]][5] = ((RankSlotView *)findListA(v->winner))->name[5];
            v->names[v->rank[v->winner]][6] = ((RankSlotView *)findListA(v->winner))->name[6];
            v->names[v->rank[v->winner]][7] = ((RankSlotView *)findListA(v->winner))->name[7];
            v->f109da8[v->winner] = 0x30;
            v->f109db0[v->winner] = 8;
            g_savedName[0] = ((RankSlotView *)findListA(v->winner))->name[0];
            g_savedName[1] = ((RankSlotView *)findListA(v->winner))->name[1];
            g_savedName[2] = ((RankSlotView *)findListA(v->winner))->name[2];
            g_savedName[3] = ((RankSlotView *)findListA(v->winner))->name[3];
            g_savedName[4] = ((RankSlotView *)findListA(v->winner))->name[4];
            g_savedName[5] = ((RankSlotView *)findListA(v->winner))->name[5];
            g_savedName[6] = ((RankSlotView *)findListA(v->winner))->name[6];
            g_savedName[7] = ((RankSlotView *)findListA(v->winner))->name[7];
            g_1b2f0a8 = 1;
        }
        v->f109ddc = 0x28;
    }
    else if (countListA() == 2)
    {
        for (i = 0; i < 2; i++)
        {
            if (((RankSlotView *)findListA(i))->f228 > 0)
            {
                if (v->isRadioFamily != 0)
                {
                    v->radioNames[v->radioRank[i]][0] = ((RankSlotView *)findListA(i))->name[0];
                    v->radioNames[v->radioRank[i]][1] = ((RankSlotView *)findListA(i))->name[1];
                    v->radioNames[v->radioRank[i]][2] = ((RankSlotView *)findListA(i))->name[2];
                    v->radioNames[v->radioRank[i]][3] = ((RankSlotView *)findListA(i))->name[3];
                    v->radioNames[v->radioRank[i]][4] = ((RankSlotView *)findListA(i))->name[4];
                    v->radioNames[v->radioRank[i]][5] = ((RankSlotView *)findListA(i))->name[5];
                    v->radioNames[v->radioRank[i]][6] = ((RankSlotView *)findListA(i))->name[6];
                    v->radioNames[v->radioRank[i]][7] = ((RankSlotView *)findListA(i))->name[7];
                }
                v->names[v->rank[i]][0] = ((RankSlotView *)findListA(i))->name[0];
                v->names[v->rank[i]][1] = ((RankSlotView *)findListA(i))->name[1];
                v->names[v->rank[i]][2] = ((RankSlotView *)findListA(i))->name[2];
                v->names[v->rank[i]][3] = ((RankSlotView *)findListA(i))->name[3];
                v->names[v->rank[i]][4] = ((RankSlotView *)findListA(i))->name[4];
                v->names[v->rank[i]][5] = ((RankSlotView *)findListA(i))->name[5];
                v->names[v->rank[i]][6] = ((RankSlotView *)findListA(i))->name[6];
                v->names[v->rank[i]][7] = ((RankSlotView *)findListA(i))->name[7];
                v->f109da8[i] = 0x30;
                v->f109db0[i] = 8;
            }
            else if (g_playCounter > 0)
            {
                if (v->isRadioFamily != 0)
                {
                    v->radioNames[v->radioRank[i]][0] = g_savedName[0];
                    v->radioNames[v->radioRank[i]][1] = g_savedName[1];
                    v->radioNames[v->radioRank[i]][2] = g_savedName[2];
                    v->radioNames[v->radioRank[i]][3] = g_savedName[3];
                    v->radioNames[v->radioRank[i]][4] = g_savedName[4];
                    v->radioNames[v->radioRank[i]][5] = g_savedName[5];
                    v->radioNames[v->radioRank[i]][6] = g_savedName[6];
                    v->radioNames[v->radioRank[i]][7] = g_savedName[7];
                }
                v->names[v->rank[i]][0] = g_savedName[0];
                v->names[v->rank[i]][1] = g_savedName[1];
                v->names[v->rank[i]][2] = g_savedName[2];
                v->names[v->rank[i]][3] = g_savedName[3];
                v->names[v->rank[i]][4] = g_savedName[4];
                v->names[v->rank[i]][5] = g_savedName[5];
                v->names[v->rank[i]][6] = g_savedName[6];
                v->names[v->rank[i]][7] = g_savedName[7];
                v->f109da8[i] = 0x30;
                v->f109db0[i] = 8;
            }
        }
        g_37dd27c[0] = 1;
        if (v->isRadioFamily != 0)
        {
            if (v->radioRank[0] > 98)
            {
                v->radioRank[0] = v->radioRank[1];
                v->score[0] = v->score[1];
                removeFromListA(findListA(0));
                v->editFlag[1] = 1;
                if (v->radioRank[0] > 98)
                {
                    removeFromListA(findListA(1));
                    v->editFlag[0] = 1;
                }
            }
            else if (v->radioRank[1] > 98)
            {
                removeFromListA(findListA(1));
                v->editFlag[1] = 1;
            }
        }
        else
        {
            if (v->rank[0] > 98)
            {
                v->rank[0] = v->rank[1];
                v->score[0] = v->score[1];
                removeFromListA(findListA(0));
                v->editFlag[1] = 1;
                if (v->rank[0] > 98)
                {
                    removeFromListA(findListA(1));
                    v->editFlag[0] = 1;
                }
            }
            else if (v->rank[1] > 98)
            {
                removeFromListA(findListA(1));
                v->editFlag[1] = 1;
            }
        }
    }
    else if (countListA() == 1)
    {
        if (((RankSlotView *)findListA(getListAF8()))->f228 > 0)
        {
            if (v->isRadioFamily != 0)
            {
                v->radioNames[v->radioRank[0]][0] = ((RankSlotView *)findListA(getListAF8()))->name[0];
                v->radioNames[v->radioRank[0]][1] = ((RankSlotView *)findListA(getListAF8()))->name[1];
                v->radioNames[v->radioRank[0]][2] = ((RankSlotView *)findListA(getListAF8()))->name[2];
                v->radioNames[v->radioRank[0]][3] = ((RankSlotView *)findListA(getListAF8()))->name[3];
                v->radioNames[v->radioRank[0]][4] = ((RankSlotView *)findListA(getListAF8()))->name[4];
                v->radioNames[v->radioRank[0]][5] = ((RankSlotView *)findListA(getListAF8()))->name[5];
                v->radioNames[v->radioRank[0]][6] = ((RankSlotView *)findListA(getListAF8()))->name[6];
                v->radioNames[v->radioRank[0]][7] = ((RankSlotView *)findListA(getListAF8()))->name[7];
            }
            v->names[v->rank[0]][0] = ((RankSlotView *)findListA(getListAF8()))->name[0];
            v->names[v->rank[0]][1] = ((RankSlotView *)findListA(getListAF8()))->name[1];
            v->names[v->rank[0]][2] = ((RankSlotView *)findListA(getListAF8()))->name[2];
            v->names[v->rank[0]][3] = ((RankSlotView *)findListA(getListAF8()))->name[3];
            v->names[v->rank[0]][4] = ((RankSlotView *)findListA(getListAF8()))->name[4];
            v->names[v->rank[0]][5] = ((RankSlotView *)findListA(getListAF8()))->name[5];
            v->names[v->rank[0]][6] = ((RankSlotView *)findListA(getListAF8()))->name[6];
            v->names[v->rank[0]][7] = ((RankSlotView *)findListA(getListAF8()))->name[7];
            v->f109da8[0] = 0x30;
            v->f109db0[0] = 8;
        }
        g_37dd27c[0] = 0;
        if (v->isRadioFamily != 0)
        {
            if (v->radioRank[1] > 98)
                v->editFlag[1] = 1;
            if (v->radioRank[0] > 98)
            {
                removeFromListA(findListA(getListAF8()));
                v->editFlag[0] = 1;
            }
        }
        else
        {
            if (v->rank[1] > 98)
                v->editFlag[1] = 1;
            if (v->rank[0] > 98)
            {
                removeFromListA(findListA(getListAF8()));
                v->editFlag[0] = 1;
            }
        }
    }

    changeDir("system\\ranking\\", 1);
    // STRING: EZ2AC_UNPACKED 0x00498880
    ((KGraphics *)g_sub1b2f054)->loadStr("BG\\rankingbg.str", &res);
    m_668.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x0049886c
    ((KGraphics *)g_sub1b2f054)->loadStr("TT_RankCusor.str", &res);
    m_152c.attach(res, 1);
    if (g_1b2f024 != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00498858
        m_1748.load((KGraphics *)g_sub1b2f054, "Bg\\Vranking_bg.spv");
        ((Obj40B9E0 *)&m_1748)->setField43c(1);
        m_1748.play(timeGetTime());
        // STRING: EZ2AC_UNPACKED 0x0049883c
        ((KGraphics *)g_sub1b2f054)->loadStr("VRanking\\VrankingBG_1.str", &res);
        m_10f4.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x00498820
        ((KGraphics *)g_sub1b2f054)->loadStr("Vranking\\VrankingBG_2.str", &res);
        m_1310.attach(res, 1);
        // STRING: EZ2AC_UNPACKED 0x00498804
        ((KGraphics *)g_sub1b2f054)->loadStr("VRanking\\vranking_int.str", &res);
        m_14.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x004987e8
        ((KGraphics *)g_sub1b2f054)->loadStr("VRanking\\vranking_loop.str", &res);
        m_44c.attach(res, 1);
        // STRING: EZ2AC_UNPACKED 0x004987dc
        ((KGraphics *)g_sub1b2f054)->loadStr("1p_name.str", &res);
        m_884[0].attach(res, 1);
        // STRING: EZ2AC_UNPACKED 0x004987cc
        ((KGraphics *)g_sub1b2f054)->loadStr("1p_arrow.str", &res);
        m_cbc[0].attach(res, 1);
        /* The same arrow again into the second cursor - what the original
           does (0x459624 pushes 0x4987cc, not the 2p2 arrow). */
        ((KGraphics *)g_sub1b2f054)->loadStr("1p_arrow.str", &res);
        m_cbc[1].attach(res, 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x004987b8
        m_1748.load((KGraphics *)g_sub1b2f054, "Bg\\ranking_bg.spv");
        ((Obj40B9E0 *)&m_1748)->setField43c(1);
        m_1748.play(timeGetTime());
        if (countListA() == 1)
        {
            // STRING: EZ2AC_UNPACKED 0x004987a8
            ((KGraphics *)g_sub1b2f054)->loadStr("TotalUp_int.str", &res);
            m_14.attach(res, 0);
            // STRING: EZ2AC_UNPACKED 0x00498794
            ((KGraphics *)g_sub1b2f054)->loadStr("TotalUp_Loop.str", &res);
            m_44c.attach(res, 1);
            ((KGraphics *)g_sub1b2f054)->loadStr("1p_name.str", &res);
            m_884[0].attach(res, 1);
            ((KGraphics *)g_sub1b2f054)->loadStr("1p_arrow.str", &res);
            m_cbc[0].attach(res, 1);
        }
        else if (countListA() == 2)
        {
            // STRING: EZ2AC_UNPACKED 0x00498788
            ((KGraphics *)g_sub1b2f054)->loadStr("2p_int.str", &res);
            m_14.attach(res, 0);
            // STRING: EZ2AC_UNPACKED 0x0049877c
            ((KGraphics *)g_sub1b2f054)->loadStr("2p_loop.str", &res);
            m_44c.attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x0049876c
            ((KGraphics *)g_sub1b2f054)->loadStr("2p1_name.str", &res);
            m_884[0].attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x0049875c
            ((KGraphics *)g_sub1b2f054)->loadStr("2p2_name.str", &res);
            m_884[1].attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x0049874c
            ((KGraphics *)g_sub1b2f054)->loadStr("2p1_arrow.str", &res);
            m_cbc[0].attach(res, 1);
            // STRING: EZ2AC_UNPACKED 0x0049873c
            ((KGraphics *)g_sub1b2f054)->loadStr("2p2_arrow.str", &res);
            m_cbc[1].attach(res, 1);
        }
    }
    popDirectory();
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp",
                                             &v->fadeTex, 0);
    v->f109de0 = 0;
    v->f109b30 = 0;
    g = g_bigInstance;
    g->accum[0] = 0;
    g->accum[1] = 0;
    g->limit = 5;
}
