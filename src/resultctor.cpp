/* ResultDirector's constructor @0x451f10 - the result screen's 9.4KB builder,
   the biggest function attempted so far (2,563 instructions, 266 calls).
   Reading: scratchpad/result_ctor.{asm,txt} (regenerate with decode.py).

   MATCH IN PROGRESS - 71.5% structural (scratchpad/zoodiff.py with
   ZOO_SYM='??0ResultDirector' ORIG_LO=0x451f10 ORIG_HI=0x4543f8). The frame
   is within 4 bytes (0x298 vs 0x294: name[68]@0x30, buf[260]@0x74,
   bufPlace[28]@0x178, bufVer[260]@0x194+cookie fit the original exactly).
   Remaining known deltas (67.3% reccmp after the backdrop + named-pair +
   RSongDraw_R passes). USE THE REAL RECCMP DIFF for triage - the masked
   structural differ mis-aligns arms whose only difference is a masked
   literal and reports phantom merges:
   - the stats-record fills (mode-10 and mixStyle-8, identical source) merge
     in our build; the original's copies differ ONLY in scratch rotation
     (mode-10 edx-first after a cur reload in ecx @0x453568, mix-8 eax-first
     after `mov eax,[g_mixStyle]`). A node-assignment hoist above the branch
     was tried and made things worse - revert-tested.
   - `rec` SPILLS to [esp+0x18] in our build where the original keeps it in
     edi for the whole loop; several early reloads follow. Fix before
     chasing smaller rotations - it shifts many operands at once.
   - our res/cur slots are swapped (res@0x10/cur@0x14 vs the original's
     cur@0x10/res@0x14); neither declaration order (either way) nor cur=0
     placement moves them. Our name buffer sits at 0x40 (orig 0x30) with the
     fp/cw temps arranged differently - fix the log block first, it decides
     the cw-temp slots (orig has fnstcw temps at 0x1a/0x22/0x2a; ours nowhere,
     so our (int) casts in the log block are NOT compiling to the inline
     fistp+cw form - check for _ftol calls there).
   - our res/cur/i frame slots sit at 0x14/0x18/0x10; the original has
     cur@0x10, res@0x14, and stores 0 to [esp+0x14] JUST BEFORE the loop
     (0x453420) - possibly a literal `res = 0;` - and spends no slot on `i`.
     Also our g_stageRanks store spills the lea to [esp+0x1c]; the
     original stores `[ebp*4+g_stageRanks]` directly.
   - the CV2 log-score block (fyl2x/fistp x87 @0x453ad1) is a first
     approximation: the fistp target puns the rz slot, Log("Current Score")
     arg unverified against raw.
   - the clip-pick at the loop head (0x453438) uses a goto for the CV2
     no-player fall-through; orig block order differs slightly.
   - register rotation in the per-arm drawLine groups not yet compared.

   Structure: seventeen member constructors (states 0..0x10), the cabinet lamp
   raises, a field-zero block, the version readout (ver_%d digits + ver.str
   with magic-divide digit cells), the result sound set (CV2 variants), the
   song-rank clip pick, level/bonus/song-name/disc/difficulty/channel-tag
   textures, the 29-slot effector icon table, the backdrop pick (course /
   gameover / CV2), panels and battle overlays, the PLACE_%u run, the five
   digit-font families, and THE PER-PLAYER LOOP: per-stage textures, grade
   stamp, score ratios, the CV2 log-score computation (x87 log10), the
   per-player readout cells and nine drawLine readouts per mode arm, capped
   by the AllCombo clip and the deciding-sample vcall. */

#include <stdio.h>
#include <string.h>
#include "directors.h"
#include "ez2acapp.h"   /* KGraphics, EzSubsystem, g_sub1b2f054, g_sub1b2eae4 */
#include "bigbase.h"    /* Obj40C180, g_bigInstance */
#include "players.h"    /* ListNode, PlayerSlot */
#include <math.h>

extern void Log(const char *fmt, ...);             /* 0x402880 (src/applog.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 (src/nodelists.cpp) */
extern ListNode *findListA(int key);               /* 0x418e30 (src/nodelists.cpp) */
extern int  nextListA(ListNode **cur);             /* 0x4182e0 (src/nodelists.cpp) */
extern "C" unsigned long __stdcall timeGetTime(void);

extern char g_modeName[256];       /* 0x1b2e680 (src/modename.cpp) */
extern int  g_modeIndex;           /* 0x1b2e548 (src/modename.cpp) */
extern char g_channelName[0x80];   /* 0x1b2e728 (src/songselect.cpp) */
extern int  g_elemCount;           /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2eb6c;             /* src/ez2acapp.cpp - the CV2 flag */
extern int  g_1b2ea28;             /* src/moderunner.cpp */
extern int  g_1b2e7d8;             /* free-play flag (src/sink.cpp reads it) */
extern int  g_mixStyle;            /* 0x1b2ebc8 (src/panels.cpp) */
extern int  g_eventMode;           /* 0x1696404 (src/settings.cpp) */
extern char g_textSlots[32][128];  /* 0x1b2ec18 (src/modename.cpp) */
extern char g_songInfoName[64];    /* 0x1b2eb80 (src/panels.cpp) */
extern int  g_1b2f058[5];          /* the per-stage nonuplet (src/gamestate.cpp) */
extern int  g_1b2ebe0[5];
extern int  g_1b5f15c[5];
extern int  g_1b2f120[5];
extern int  g_1b2e650[5];
extern int  g_1b2ea38[5];
extern int  g_1b2e9fc[5];
extern int  g_1b2e668[5];

/* Obj40B2E0's +0x43c setter (view cast, as in src/modeselectctor.cpp). */
struct Obj40B9E0 { void setField43c(int v); };

/* New globals evidenced here. */
extern int g_1b2f088;              /* the game version (src/kengineapp.cpp) */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb70
int g_1b2eb70;
extern int g_1b2ebdc;              /* src/gamestate.cpp */
extern int g_stageNumber;          /* 0x1b2ec0c (src/directors.cpp) */
extern int g_1b2ebb8;              /* src/gamestate.cpp */
extern int g_1b2e704;              /* src/playerslot.cpp */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f038
int g_stageRanks[5];               /* each stage's rank grade (record +0x30) */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f02c
int g_1b2f02c[2];
// GLOBAL: EZ2AC_UNPACKED 0x01b2f070
int g_1b2f070[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebf8
int g_1b2ebf8[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebc0
int g_1b2ebc0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2efec
int g_1b2efec;

/* The mode-9 (GF) per-stage stat arrays. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7b0
int g_1b2e7b0[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2efd8
int g_1b2efd8[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb2c
int g_1b2eb2c[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2c4
int g_1b5f2c4[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2eaac
int g_1b2eaac[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea64
int g_1b2ea64[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb4c
int g_1b2eb4c[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b5f188
int g_1b5f188[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2f094
int g_1b2f094[5];
// GLOBAL: EZ2AC_UNPACKED 0x01b2eff0
int g_1b2eff0[5];

/* The per-grade rank art (F..S4, with mask twins; the CV2 set stops at S)
   and the bonus plates, indexed by the stage RANK. Placeholder storage pinned
   at the originals' addresses; contents stay external. */
// GLOBAL: EZ2AC_UNPACKED 0x004ae1f8
char *g_rankArt[11];               /* 0x4ae1f8 - Rank\\rank_f .. rank_s4 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae224
char *g_rankArtCv2[7];             /* 0x4ae224 - CV2\\Font\\rank_f .. rank_s */
// GLOBAL: EZ2AC_UNPACKED 0x004ae240
char *g_rankArtMask[11];           /* 0x4ae240 - the _m twins */
// GLOBAL: EZ2AC_UNPACKED 0x004ae26c
char *g_rankArtCv2Mask[7];         /* 0x4ae26c - the CV2 _mask twins */
// GLOBAL: EZ2AC_UNPACKED 0x004ae288
char *g_bonusArt[5];               /* 0x4ae288 - black, Bonus1..4 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae29c
char *g_bonusArtMask[5];           /* 0x4ae29c - their _mask twins */

/* The .rdata readout tables: per-player {x, y} pairs for the nine drawLine
   readouts (stride 0x50 = ten pairs per player), the two-per-player cell ids
   at 0x495860, the two cell-id pairs at 0x495870/0x495878, and the per-grade
   cell value table at 0x495880. Placeholder storage. */
struct ResPair { int a; int b; };
// GLOBAL: EZ2AC_UNPACKED 0x00495770
ResPair g_readoutPos[2][10];       /* 0x495770 - per-player rows of ten */
// GLOBAL: EZ2AC_UNPACKED 0x00495860
ResPair g_placeCells[2];           /* 0x495860 */
// GLOBAL: EZ2AC_UNPACKED 0x00495870
int g_songCellA[2];                /* 0x495870 */
// GLOBAL: EZ2AC_UNPACKED 0x00495878
int g_songCellB[2];                /* 0x495878 */
// GLOBAL: EZ2AC_UNPACKED 0x00495880
int g_gradeCell[4];                /* 0x495880 */

/* The stage record inside the list-A player: 0x230 + 0x40*stage. The same
   region src/rankingctor.cpp reads at +0x254/+0x258 (= +0x24/+0x28 of this). */
struct ResStage
{
    int notes;                     /* 0x00 - total notes */
    int f4;                        /* 0x04 */
    int f8;                        /* 0x08 */
    int fc;                        /* 0x0c */
    int f10;                       /* 0x10 */
    int f14;                       /* 0x14 */
    int f18;                       /* 0x18 */
    int f1c;                       /* 0x1c */
    int f20;                       /* 0x20 - combo bonus */
    int score;                     /* 0x24 */
    int bonus;                     /* 0x28 */
    float f2c;                     /* 0x2c */
    int rank;                      /* 0x30 - the rank grade, F=0 .. S4=10 */
    int f34;                       /* 0x34 */
    float f38;                     /* 0x38 */
    float f3c;                     /* 0x3c */
};

/* The view over the player node (list A). */
struct ResSlotView
{
    char _pad0[0x1ec];
    int  f1ec;                     /* 0x1ec - the published session score */
    char _pad1f0[0x230 - 0x1f0];
    ResStage stage[5];             /* 0x230 + 0x40*i */
};

/* The deciding sample's vtable as this call site sees it: slot 4, one int. */
struct ResSample
{
    virtual void t0();
    virtual void t1();
    virtual void t2();
    virtual void t3();
    virtual void t4(int a);
};

/* The constructor's view of the object past include/directors.h's model. */
struct ResView
{
    char _pad0[0x10];
    int  f10;                      /* 0x0010 */
    int  f14;                      /* 0x0014 */
    int  f18;                      /* 0x0018 */
    int  f1c;                      /* 0x001c */
    int  f20;                      /* 0x0020 */
    int  f24[2];                   /* 0x0024 - per-player result flag */
    char _pad2c[0x28e0 - 0x2c];
    void *sndBgm;                  /* 0x28e0 - result_bgm */
    void *snd28e4;                 /* 0x28e4 - Rank */
    void *snd28e8;                 /* 0x28e8 - Rank_Fail */
    void *snd28ec;                 /* 0x28ec - bonus */
    void *snd28f0;                 /* 0x28f0 - NextStage */
    void *snd28f4;                 /* 0x28f4 - fNextStage */
    void *snd28f8;                 /* 0x28f8 - count */
    void *snd28fc;                 /* 0x28fc - CV2 nametag */
    void *snd2900;                 /* 0x2900 - SetBonus */
    void *snd2904;                 /* 0x2904 - BattleMessage */
    ResStage statsRec;             /* 0x2908 - the synthetic stage record the
                                      catch / CV2 style modes fill from the
                                      per-stage globals */
    char _pad2948[0x298c - 0x2948];
    struct KTexture *texGrad;      /* 0x298c - result_grad */
    struct KTexture *texExtreme;   /* 0x2990 */
    struct KTexture *texFade;      /* 0x2994 - fadeblack */
    struct KTexture *f2998[2];     /* 0x2998 - per-player song name */
    struct KTexture *f29a0[2];     /* 0x29a0 */
    struct KTexture *f29a8[2];     /* 0x29a8 - per-player rank art */
    struct KTexture *f29b0[2];     /* 0x29b0 - per-player rank art B */
    struct KTexture *f29b8[4];     /* 0x29b8 - CV2 bonus masks */
    struct KTexture *f29c8[4];     /* 0x29c8 - CV2 bonus digits */
    struct KTexture *texTopM;      /* 0x29d8 - TT_Result_Top_m */
    struct KTexture *texTopC;      /* 0x29dc - _c / _course */
    struct KTexture *texTopF;      /* 0x29e0 - _f */
    struct KTexture *fontA[10];    /* 0x29e4 - BigFnt / CN_ */
    struct KTexture *fontB[10];    /* 0x2a0c - SmlFnt / SCF_ / totasf */
    struct KTexture *fontC[10];    /* 0x2a34 - ntf_ */
    struct KTexture *fontD[10];    /* 0x2a5c - TF / BigFnt battle */
    struct KTexture *fontE[10];    /* 0x2a84 - SmlFnt battle */
    struct KTexture *f2aac[10];    /* 0x2aac - the CV2 note-count font */
    struct KTexture *texChanTag;   /* 0x2ad4 */
    struct KTexture *texModeIcon;  /* 0x2ad8 */
    char _pad2adc[0x2ae0 - 0x2adc];
    struct KTexture *effTex[29];   /* 0x2ae0 .. 0x2b54 - the effector icons */
    struct KTexture *texDiffNormal;/* 0x2b54 */
    struct KTexture *texDiffHard;  /* 0x2b58 */
    struct KTexture *texDiffSHard; /* 0x2b5c */
    struct KTexture *texDiffEx;    /* 0x2b60 */
    struct KTexture *texLv;        /* 0x2b64 */
    char _pad2b68[0x2b90 - 0x2b68];
    struct KTexture *texSongName;  /* 0x2b90 */
    char _pad2b94[4];
    struct KTexture *texDisc;      /* 0x2b98 */
    struct KTexture *ttStage[4];   /* 0x2b9c - TT_stage_%d */
    struct KTexture *cv2Stage[4];  /* 0x2bac - stage_%02d */
    struct KTexture *cv2StageMask[3]; /* 0x2bbc */
    char _pad2bc8[0x2c08 - 0x2bc8];
    struct KTexture *verTex[10];   /* 0x2c08 - ver_%d */
    struct KTexture *f2c30[2];     /* 0x2c30 - the per-player grade art cell */
    int  f2c38;                    /* 0x2c38 - the bonus-art index */
    int  f2c3c;                    /* 0x2c3c */
    char _pad2c40[0x2c48 - 0x2c40];
    int  f2c48;                    /* 0x2c48 */
    char _pad2c4c[0x2c54 - 0x2c4c];
    int  f2c54;                    /* 0x2c54 - m_aComboBonusDelta */
    char _pad2c58[4];
    int  f2c5c[2];                 /* 0x2c5c */
    int  f2c64[2];                 /* 0x2c64 - the published stage score */
    char _pad2c6c[0x2c7c - 0x2c6c];
    int  f2c7c[2];                 /* 0x2c7c */
    int  f2c84[2];                 /* 0x2c84 */
    int  f2c8c;                    /* 0x2c8c - the CV2 log score */
    int  f2c90;                    /* 0x2c90 */
    int  f2c94;                    /* 0x2c94 - battle winner, -1 = none */
    int  f2c98;                    /* 0x2c98 */
    char _pad2c9c[0x30dc - 0x2c9c];
    struct KTexture *placeTex[5];  /* 0x30dc - PLACE_%u */
    char _pad30f0[0x30f8 - 0x30f0];
    int  f30f8;                    /* 0x30f8 - 5 */
    int  f30fc;                    /* 0x30fc - 5 */
    int  f3100;                    /* 0x3100 - 155 */
    int  f3104;                    /* 0x3104 - 300 */
    int  f3108;                    /* 0x3108 - 1600 */
    int  f310c;                    /* 0x310c - 25 */
    int  f3110;                    /* 0x3110 - 1800 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00451f10
ResultDirector::ResultDirector()
{
    ResView *v = (ResView *)this;
    Obj402860Head *res;
    ListNode *cur;
    char name[64];
    char buf[260];
    char bufPlace[32];
    char bufVer[260];
    int  i;
    int  vn;

    if (g_modeIndex == 4 || g_modeIndex == 5 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        g_bigInstance->m4189e0(1);
        g_bigInstance->m418a50(1);
        g_bigInstance->m418af0(1);
    }
    else
    {
        g_bigInstance->m4189e0(1);
        if (findListA(0) != 0)
            g_bigInstance->m418a50(1);
        if (findListA(1) != 0)
            g_bigInstance->m418af0(1);
    }

    v->f2c48 = 0;
    v->f10 = 0;
    v->f14 = 0;
    v->f18 = 0;
    v->f2c90 = 0;
    v->f2c8c = 0;
    v->f1c = 0;
    v->f20 = 0;
    v->f24[0] = 0;
    v->f24[1] = 0;
    v->f2c3c = 0;
    v->f3100 = 0x9b;
    v->f3104 = 0x12c;
    v->f3108 = 0x640;
    v->f310c = 0x19;
    v->f3110 = 0x708;

    {
    Obj402860 *ver = &m_21ec;
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00494e88
        sprintf(bufVer, "system\\title\\common\\ver_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(bufVer, &v->verTex[i], 0);
    }
    if (g_1b2eb6c == 0)
    {
        /* The three low digit cells are written in BOTH arms - the
           original cross-jumps them, so the dead `< 1000` miss of the
           second test skips them (0x452242 -> 0x452339). */
        if (g_1b2f088 <= 999)
        {
            // STRING: EZ2AC_UNPACKED 0x00496a9c
            sprintf(bufVer, "system\\title\\common\\ver.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(bufVer, &res);
            ver->attach(res, 1);
            vn = g_1b2f088;
            ver->setCell(2, 0, (int)v->verTex[vn / 100 % 10]);
            vn = g_1b2f088;
            ver->setCell(4, 0, (int)v->verTex[vn / 10 % 10]);
            vn = g_1b2f088;
            ver->setCell(5, 0, (int)v->verTex[vn % 10]);
        }
        else if (g_1b2f088 >= 1000)
        {
            // STRING: EZ2AC_UNPACKED 0x00496a78
            sprintf(bufVer, "system\\title\\common\\ver_over.str");
            ((KGraphics *)g_sub1b2f054)->loadStr(bufVer, &res);
            ver->attach(res, 1);
            vn = g_1b2f088;
            ver->setCell(1, 0, (int)v->verTex[vn / 1000 % 10]);
            vn = g_1b2f088;
            ver->setCell(2, 0, (int)v->verTex[vn / 100 % 10]);
            vn = g_1b2f088;
            ver->setCell(4, 0, (int)v->verTex[vn / 10 % 10]);
            vn = g_1b2f088;
            ver->setCell(5, 0, (int)v->verTex[vn % 10]);
        }
    }
    }

    // STRING: EZ2AC_UNPACKED 0x00496a58
    v->sndBgm = (void *)g_sub1b2eae4->createSample("system\\Result\\result_bgm.wav", 1, 0);
    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00496a40
        v->snd28e4 = (void *)g_sub1b2eae4->createSample("System\\Result\\Rank.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x00496a24
        v->snd28e8 = (void *)g_sub1b2eae4->createSample("System\\Result\\Rank_Fail.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x00496a0c
        v->snd28f8 = (void *)g_sub1b2eae4->createSample("System\\Result\\count.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x004969f4
        v->snd28ec = (void *)g_sub1b2eae4->createSample("System\\Result\\bonus.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x004969d4
        v->snd28f4 = (void *)g_sub1b2eae4->createSample("System\\Result\\fNextStage.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x004969b8
        v->snd28f0 = (void *)g_sub1b2eae4->createSample("System\\Result\\NextStage.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x0049699c
        v->snd2900 = (void *)g_sub1b2eae4->createSample("System\\Result\\SetBonus.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x0049697c
        v->snd2904 = (void *)g_sub1b2eae4->createSample("System\\Result\\BattleMessage.wav", 0, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496960
        v->snd28e4 = (void *)g_sub1b2eae4->createSample("System\\Result\\CV2\\Rank.wav", 0, 0);
        v->snd28e8 = (void *)g_sub1b2eae4->createSample("System\\Result\\Rank.wav", 0, 0);
        v->snd28f8 = (void *)g_sub1b2eae4->createSample("System\\Result\\count.wav", 0, 0);
        v->snd28ec = (void *)g_sub1b2eae4->createSample("System\\Result\\bonus.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x00496940
        v->snd28f4 = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\Common\\decide.wav", 0, 0);
        v->snd28f0 = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\Common\\decide.wav", 0, 0);
        // STRING: EZ2AC_UNPACKED 0x00496924
        v->snd28fc = (void *)g_sub1b2eae4->createSample("system\\result\\cv2\\Rank.wav", 0, 0);
    }

    /* The song-rank clip: which RSongDraw variant, by mode and player count. */
    if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        if (countListA() == 2)
        {
            // STRING: EZ2AC_UNPACKED 0x004968a8
            ((KGraphics *)g_sub1b2f054)->loadStr("system\\RSname\\RSongDraw_B_R.str", &res);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00496888
            ((KGraphics *)g_sub1b2f054)->loadStr("system\\RSname\\RSongDraw_R.str", &res);
        }
    }
    else if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00496904
        ((KGraphics *)g_sub1b2f054)->loadStr("system\\Result\\CV2\\nametag.str", &res);
    }
    else if (countListA() == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x004968e8
        ((KGraphics *)g_sub1b2f054)->loadStr("system\\RSname\\RSongDraw.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x004968c8
        ((KGraphics *)g_sub1b2f054)->loadStr("system\\RSname\\RSongDraw_b.str", &res);
    }
    m_ef0.attach(res, 1);

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00496868
        sprintf(buf, "System\\Result\\lvfont\\LV%02d.bmp", g_stageNumber);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texLv, 0);
    }
    else
    {
        for (i = 0; i < 10; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00492118
            sprintf(buf, "System\\CV2Mix\\modeselect\\Lvfont\\lv_%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_digits[i], 0);
        }
    }
    for (i = 0; i < 4; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00496844
        sprintf(buf, "system\\result\\cv2\\bonus_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->f29c8[i], 0);
        // STRING: EZ2AC_UNPACKED 0x0049681c
        sprintf(buf, "system\\result\\cv2\\bonus_%02d_mask.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->f29b8[i], 0);
    }

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004937a8
        sprintf(buf, "system\\CV2Mix\\SongName\\%s.bmp", g_songInfoName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496804
        sprintf(buf, "system\\SongName\\%s.bmp", m450c00());
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texSongName, 0);
    // STRING: EZ2AC_UNPACKED 0x004967e4
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\result_grad.bmp",
                                             &v->texGrad, 0);
    // STRING: EZ2AC_UNPACKED 0x004967c0
    sprintf(buf, "System\\Result\\ModeIcon\\Icon_%s.bmp", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texModeIcon, 0);

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049679c
        ((KGraphics *)g_sub1b2f054)->loadStr("System\\result\\cv2\\Stage\\stage.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496774
        ((KGraphics *)g_sub1b2f054)->loadStr("System\\Result\\Stage\\Result_Stage.str", &res);
    }
    m_197c.attach(res, 0);

    if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496758
        sprintf(buf, "system\\ResultDisc\\%s.bmp", g_textSlots[g_elemCount]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texDisc, 0);
    }
    else
    {
        sprintf(buf, "system\\ResultDisc\\%s.bmp", m450c00());
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texDisc, 0);
    }

    if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        if (g_modeIndex == 6)
        {
            sprintf(buf, "System\\Channel_Tag\\5Radio\\%s.bmp", g_channelName);
        }
        else if (g_modeIndex == 8)
        {
            // STRING: EZ2AC_UNPACKED 0x00496698
            sprintf(buf, "System\\Channel_Tag\\10Radio\\%s.bmp", g_channelName);
        }
        else if (g_modeIndex == 9)
        {
            // STRING: EZ2AC_UNPACKED 0x00496674
            sprintf(buf, "System\\Channel_Tag\\14Radio\\%s.bmp", g_channelName);
        }
        else if (g_modeIndex == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x00496654
            sprintf(buf, "System\\Channel_Tag\\Radio\\%s.bmp", g_channelName);
        }
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texChanTag, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496738
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Diff_Normal.bmp", &v->texDiffNormal, 0);
        // STRING: EZ2AC_UNPACKED 0x0049671c
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Diff_Hard.bmp", &v->texDiffHard, 0);
        // STRING: EZ2AC_UNPACKED 0x004966fc
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Diff_SHard.bmp", &v->texDiffSHard, 0);
        // STRING: EZ2AC_UNPACKED 0x004966e0
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Diff_Ex.bmp", &v->texDiffEx, 0);
    }

    if (g_1b2eb6c != 0)
    {
        m_ef0.setCell(1, 0, (int)v->texSongName);
    }
    else
    {
        m_ef0.setCell(1, 0, (int)v->texSongName);
        m_ef0.setCell(2, 0, (int)v->texLv);
        m_ef0.setCell(3, 0, (int)v->texDisc);
    }

    if (g_1b2eb6c != 0)
    {
        for (i = 0; i < 3; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x0049662c
            sprintf(buf, "System\\Result\\CV2\\stage\\stage_%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->cv2Stage[i], 0);
            // STRING: EZ2AC_UNPACKED 0x00496600
            sprintf(buf, "System\\Result\\CV2\\stage\\stage_%02d_mask.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->cv2StageMask[i], 0);
        }
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x004965dc
            sprintf(buf, "System\\Result\\Stage\\TT_stage_%d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->ttStage[i], 0);
        }
        if (g_1b2eb70 > 0)
        {
            // STRING: EZ2AC_UNPACKED 0x004965b8
            ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\Text_ExtremeMode.bmp", &v->texExtreme, 0);
        }
    }

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496594
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Result\\TT_Result_Top_m.bmp", &v->texTopM, 0);
        // STRING: EZ2AC_UNPACKED 0x00496548
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Result\\TT_Result_Top_course.bmp", &v->texTopC, 0);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Result\\TT_Result_Top_m.bmp", &v->texTopM, 0);
        // STRING: EZ2AC_UNPACKED 0x00496570
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Result\\TT_Result_Top_c.bmp", &v->texTopC, 0);
    }
    // STRING: EZ2AC_UNPACKED 0x00496524
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Result\\TT_Result_Top_f.bmp", &v->texTopF, 0);

    /* The stage cells. */
    if (g_1b2eb6c != 0)
    {
        m_197c.setCell(1, 0, (int)v->cv2StageMask[g_elemCount]);
        m_197c.setCell(2, 0, (int)v->cv2Stage[g_elemCount]);
    }
    else if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
             g_modeIndex == 9)
    {
        m_ef0.setCell(4, 0, (int)v->texChanTag);
        m_197c.setCell(1, 0, (int)v->ttStage[g_elemCount]);
    }
    else
    {
        m_ef0.setCell(4, 0, (int)(&v->texDiffNormal)[g_1b2ebb8 - 1]);
        if (g_elemCount == 2)
            m_197c.setCell(1, 0, (int)v->ttStage[3]);
        else
            m_197c.setCell(1, 0, (int)v->ttStage[g_elemCount]);
    }

    // STRING: EZ2AC_UNPACKED 0x004964f4
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_RANDOM_OFF.bmp", &v->effTex[0], 0);
    // STRING: EZ2AC_UNPACKED 0x004964c8
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_RANDOM.bmp", &v->effTex[1], 0);
    // STRING: EZ2AC_UNPACKED 0x0049649c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SRANDOM.bmp", &v->effTex[2], 0);
    // STRING: EZ2AC_UNPACKED 0x00496474
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_PS.bmp", &v->effTex[3], 0);
    // STRING: EZ2AC_UNPACKED 0x0049644c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FR.bmp", &v->effTex[4], 0);
    // STRING: EZ2AC_UNPACKED 0x00496420
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MRANDOM.bmp", &v->effTex[5], 0);
    // STRING: EZ2AC_UNPACKED 0x004963f4
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_HRANDOM.bmp", &v->effTex[6], 0);
    // STRING: EZ2AC_UNPACKED 0x004963c8
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MIRROR.bmp", &v->effTex[7], 0);
    // STRING: EZ2AC_UNPACKED 0x00496398
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MIRROR_A.bmp", &v->effTex[8], 0);
    // STRING: EZ2AC_UNPACKED 0x00496370
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_KEY.bmp", &v->effTex[9], 0);
    // STRING: EZ2AC_UNPACKED 0x00496348
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SP.bmp", &v->effTex[10], 0);
    // STRING: EZ2AC_UNPACKED 0x0049631c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_4D_OFF.bmp", &v->effTex[21], 0);
    // STRING: EZ2AC_UNPACKED 0x004962f0
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_4D_on.bmp", &v->effTex[22], 0);
    // STRING: EZ2AC_UNPACKED 0x004962c4
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_5D_on.bmp", &v->effTex[23], 0);
    // STRING: EZ2AC_UNPACKED 0x00496294
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\Effector_Reverse_on.bmp", &v->effTex[24], 0);
    // STRING: EZ2AC_UNPACKED 0x00496260
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SPEDAL_OFF.bmp", &v->effTex[11], 0);
    // STRING: EZ2AC_UNPACKED 0x0049622c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SCRATCH_ON.bmp", &v->effTex[12], 0);
    // STRING: EZ2AC_UNPACKED 0x004961f8
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_PEDAL_ON.bmp", &v->effTex[13], 0);
    // STRING: EZ2AC_UNPACKED 0x004961c4
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SPEDAL_ON.bmp", &v->effTex[14], 0);
    // STRING: EZ2AC_UNPACKED 0x00496190
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_DISABLE.bmp", &v->effTex[20], 0);
    // STRING: EZ2AC_UNPACKED 0x00496160
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_off.bmp", &v->effTex[16], 0);
    // STRING: EZ2AC_UNPACKED 0x00496134
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_L.bmp", &v->effTex[17], 0);
    // STRING: EZ2AC_UNPACKED 0x00496108
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_R.bmp", &v->effTex[18], 0);
    // STRING: EZ2AC_UNPACKED 0x004960d8
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_on.bmp", &v->effTex[19], 0);
    // STRING: EZ2AC_UNPACKED 0x004960a8
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEOUT_OFF.bmp", &v->effTex[25], 0);
    // STRING: EZ2AC_UNPACKED 0x0049607c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEOUT.bmp", &v->effTex[26], 0);
    // STRING: EZ2AC_UNPACKED 0x00496050
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEIN.bmp", &v->effTex[27], 0);
    // STRING: EZ2AC_UNPACKED 0x00496024
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_BLINK.bmp", &v->effTex[28], 0);

    // STRING: EZ2AC_UNPACKED 0x00496014
    changeDir("System\\Result", 1);

    /* The backdrop pick. The 2P non-gameover arms attach the clip TWICE -
       once before the scenery triplet and once after (the second is the
       shared tail all arms fall into); and only the 2P course arm uses
       Course_bg.spv - the 1P radio arm scrolls Result_bg.spv. */
    if (countListA() == 1)
    {
        if (g_1b2ea28 == 0)
        {
            if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
                g_modeIndex == 9)
            {
                // STRING: EZ2AC_UNPACKED 0x00495fd4
                ((KGraphics *)g_sub1b2f054)->loadStr("coursebg.str", &res);
                m_cd4.attach(res, 1);
                // STRING: EZ2AC_UNPACKED 0x00495fe4
                m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
                ((Obj40B9E0 *)&m_2408)->setField43c(1);
                m_2408.play(timeGetTime());
            }
            else if (g_1b2eb6c != 0)
            {
                // STRING: EZ2AC_UNPACKED 0x00496000
                ((KGraphics *)g_sub1b2f054)->loadStr("CV2\\bg\\resultbg.str", &res);
                m_cd4.attach(res, 1);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x00495ff8
                ((KGraphics *)g_sub1b2f054)->loadStr("BG.str", &res);
                m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
                ((Obj40B9E0 *)&m_2408)->setField43c(1);
                m_2408.play(timeGetTime());
                m_cd4.attach(res, 1);
            }
        }
        else
        {
            if (g_1b2eb6c != 0)
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("CV2\\bg\\resultbg.str", &res);
                m_cd4.attach(res, 1);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x00495fc4
                ((KGraphics *)g_sub1b2f054)->loadStr("bg_GameOver.str", &res);
                m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
                ((Obj40B9E0 *)&m_2408)->setField43c(1);
                m_2408.play(timeGetTime());
                m_cd4.attach(res, 1);
            }
        }
    }
    else
    {
        if (g_1b2e7d8 <= 1)
        {
            if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
                g_modeIndex == 9)
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("coursebg.str", &res);
                m_cd4.attach(res, 1);
                // STRING: EZ2AC_UNPACKED 0x00495fb0
                m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Course_bg.spv");
                ((Obj40B9E0 *)&m_2408)->setField43c(1);
                m_2408.play(timeGetTime());
                m_cd4.attach(res, 1);
            }
            else
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("BG.str", &res);
                m_cd4.attach(res, 1);
                m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
                ((Obj40B9E0 *)&m_2408)->setField43c(1);
                m_2408.play(timeGetTime());
                m_cd4.attach(res, 1);
            }
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("bg_GameOver.str", &res);
            // STRING: EZ2AC_UNPACKED 0x00495f98
            m_2408.load((KGraphics *)g_sub1b2f054, "bg\\Result_Gameover.spv");
            ((Obj40B9E0 *)&m_2408)->setField43c(1);
            m_2408.play(timeGetTime());
            m_cd4.attach(res, 1);
        }
    }

    if (countListA() == 2)
    {
        // STRING: EZ2AC_UNPACKED 0x00495f84
        ((KGraphics *)g_sub1b2f054)->loadStr("Result_Eff_B.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00495f74
        ((KGraphics *)g_sub1b2f054)->loadStr("Result_Eff.str", &res);
    }
    m_89c.attach(res, 0);
    if (countListA() == 2)
    {
        // STRING: EZ2AC_UNPACKED 0x00495f5c
        ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\1p_Allcombo.str", &res);
        m_1db4.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x00495f44
        ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\2p_Allcombo.str", &res);
        m_1fd0.attach(res, 0);
    }

    /* The panel. */
    if (countListA() == 1)
    {
        if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
            g_modeIndex != 9)
        {
            if (g_modeIndex == 10)
            {
                // STRING: EZ2AC_UNPACKED 0x00495f2c
                ((KGraphics *)g_sub1b2f054)->loadStr("CatchResult_Panel.str", &res);
            }
            else if (g_1b2eb6c != 0)
            {
                // STRING: EZ2AC_UNPACKED 0x00495f18
                ((KGraphics *)g_sub1b2f054)->loadStr("CV2\\resultUI.str", &res);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x00495f04
                ((KGraphics *)g_sub1b2f054)->loadStr("Result_Panel.str", &res);
            }
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00495ef0
            ((KGraphics *)g_sub1b2f054)->loadStr("Result_PanelR.str", &res);
        }
        m_ab8.attach(res, 0);
        if (g_1b2eb6c == 0)
            m_197c.setCell(2, 0, (int)v->texModeIcon);
    }
    else if (countListA() == 2)
    {
        if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
            g_modeIndex == 9)
        {
            // STRING: EZ2AC_UNPACKED 0x00495ebc
            ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\Battle_PanelR.str", &res);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00495ed8
            ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\Battle_Panel.str", &res);
        }
        m_ab8.attach(res, 0);
        m_197c.setCell(2, 0, (int)v->texModeIcon);
    }

    /* The song-rank charts. */
    if (g_modeIndex == 10)
    {
        // STRING: EZ2AC_UNPACKED 0x00495ea8
        ((KGraphics *)g_sub1b2f054)->loadStr("1psongrank_c.str", &res);
        m_2ca4[0].attach(res, 1);
        // STRING: EZ2AC_UNPACKED 0x00495e94
        ((KGraphics *)g_sub1b2f054)->loadStr("2psongrank_c.str", &res);
        m_2ca4[1].attach(res, 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00495e84
        ((KGraphics *)g_sub1b2f054)->loadStr("1psongrank.str", &res);
        m_2ca4[0].attach(res, 1);
        // STRING: EZ2AC_UNPACKED 0x00495e74
        ((KGraphics *)g_sub1b2f054)->loadStr("2psongrank.str", &res);
        m_2ca4[1].attach(res, 1);
    }

    for (i = 0; i < 4; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00495e64
        sprintf(bufPlace, "PLACE_%u.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(bufPlace, &v->placeTex[i], 0);
    }

    /* The digit-font families. */
    if (countListA() == 1)
    {
        if (g_1b2eb6c != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00495e54
            loadDigits("CV2\\Font\\CN_", v->fontA);
            // STRING: EZ2AC_UNPACKED 0x00495e48
            loadDigits("CV2\\Font\\TF", v->fontD);
            // STRING: EZ2AC_UNPACKED 0x00495e38
            loadDigits("CV2\\Font\\CO_", v->fontB);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00495e28
            loadDigits("Font\\SmlFnt_", v->fontE);
            loadDigits("Font\\SmlFnt_", v->fontB);
        }
        if (g_1b2eb6c != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00495e18
            loadDigits("CV2\\Font\\SCF_", v->fontB);
        }
        else if (g_1b2ebdc == 1)
        {
            // STRING: EZ2AC_UNPACKED 0x00495e0c
            loadDigits("totasf_old_", v->fontB);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00495dfc
            loadDigits("Font\\BigFnt_", v->fontB);
        }
    }
    else if (countListA() == 2)
    {
        // STRING: EZ2AC_UNPACKED 0x00495dec
        loadDigits("Battle\\SmlFnt_", v->fontA);
        loadDigits("Battle\\SmlFnt_", v->fontD);
        // STRING: EZ2AC_UNPACKED 0x00495ddc
        loadDigits("Battle\\BigFnt_", v->fontB);
        // STRING: EZ2AC_UNPACKED 0x00495dd0
        loadDigits("Battle\\ntf_", v->fontC);
        loadDigits("Battle\\BigFnt_", v->fontE);
    }

    v->f2c98 = -1;
    v->f2c94 = -1;
    if (countListA() > 1)
    {
        m451200();
        switch (v->f2c94)
        {
        case 1:
            // STRING: EZ2AC_UNPACKED 0x00495db8
            ((KGraphics *)g_sub1b2f054)->loadStr("battle_result_2p.str", &res);
            break;
        case 0:
            // STRING: EZ2AC_UNPACKED 0x00495da0
            ((KGraphics *)g_sub1b2f054)->loadStr("battle_result_1p.str", &res);
            break;
        case -1:
            // STRING: EZ2AC_UNPACKED 0x00495d88
            ((KGraphics *)g_sub1b2f054)->loadStr("battle_result_draw.str", &res);
            break;
        }
        m_110c.attach(res, 0);
    }

    /* THE PER-PLAYER LOOP. */
    cur = 0;
    while (nextListA(&cur))
    {
        ResStage *rec;
        int stage;
        int total;
        int part;
        int j;
        float rc, rb, ra, ra2;

        /* Which per-player result clip. */
        if (countListA() == 1)
        {
            if (g_modeIndex == 10)
            {
                // STRING: EZ2AC_UNPACKED 0x00495d74
                sprintf(name, "CatchResult_1p.str");
                ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
                            }
            else
            {
                                if (g_1b2eb6c != 0)
                {
                    if (findListA(0) != 0)
                    {
                        // STRING: EZ2AC_UNPACKED 0x00495d5c
                        sprintf(name, "CV2\\font\\result_1p.str");
                    }
                    else if (findListA(1) != 0)
                    {
                        // STRING: EZ2AC_UNPACKED 0x00495d44
                        sprintf(name, "CV2\\font\\result_2p.str");
                    }
                    else
                    {
                        goto after_attach;
                    }
                    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x00495d34
                    sprintf(name, "result_1p.str");
                    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
                }
            }
            m_2c[cur->f8].attach(res, 0);
        }
        else if (countListA() == 2)
        {
            // STRING: EZ2AC_UNPACKED 0x00495d1c
            sprintf(name, "Battle\\Battle_%dp.str", cur->f8 + 1);
            ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
                        m_2c[cur->f8].attach(res, 0);
        }
        else
        {
                    }
after_attach:

        stage = g_elemCount;
        if (g_1b2eb6c == 0)
        {
            if (g_modeIndex == 10)
            {
                /* The synthetic stage record, from the per-stage globals. */
                rec = (ResStage *)&v->statsRec;
                v->statsRec.notes = g_1b2ea38[stage];
                v->statsRec.f18 = g_1b5f15c[stage];
                v->statsRec.f8 = g_1b2f120[stage];
                v->statsRec.f14 = 0;
                v->statsRec.f10 = 0;
                v->statsRec.fc = 0;
                v->statsRec.f1c = g_1b2e668[stage];
                v->statsRec.f20 = g_1b2ebe0[stage];
                v->statsRec.score = g_1b2f058[stage];
                v->statsRec.bonus = 0;
                v->statsRec.f2c = 0.0f;
                v->statsRec.rank = g_1b2e9fc[stage];
                v->statsRec.f34 = 0;
                v->statsRec.f38 = 0.0f;
                v->statsRec.f3c = 0.0f;
            }
            else
            {
                rec = &((ResSlotView *)cur)->stage[stage];
            }
        }
        else
        {
            if (g_mixStyle == 8)
            {
                rec = (ResStage *)&v->statsRec;
                v->statsRec.notes = g_1b2ea38[stage];
                v->statsRec.f18 = g_1b5f15c[stage];
                v->statsRec.f8 = g_1b2f120[stage];
                v->statsRec.f14 = 0;
                v->statsRec.f10 = 0;
                v->statsRec.fc = 0;
                v->statsRec.f1c = g_1b2e668[stage];
                v->statsRec.f20 = g_1b2ebe0[stage];
                v->statsRec.score = g_1b2f058[stage];
                v->statsRec.bonus = 0;
                v->statsRec.f2c = 0.0f;
                v->statsRec.rank = g_1b2e9fc[stage];
                v->statsRec.f34 = 0;
                v->statsRec.f38 = 0.0f;
                v->statsRec.f3c = 0.0f;
            }
            else if (g_mixStyle == 9)
            {
                rec = (ResStage *)&v->statsRec;
                v->statsRec.notes = g_1b2e7b0[stage];
                v->statsRec.f18 = g_1b2efd8[stage];
                v->statsRec.f14 = g_1b2eb2c[stage];
                v->statsRec.f10 = g_1b5f2c4[stage];
                v->statsRec.fc = g_1b2eaac[stage];
                v->statsRec.f8 = g_1b2ea64[stage];
                v->statsRec.f1c = g_1b2eb4c[stage];
                v->statsRec.f20 = g_1b5f188[stage];
                v->statsRec.score = g_1b2f094[stage];
                v->statsRec.bonus = 0;
                v->statsRec.f2c = 0.0f;
                v->statsRec.rank = g_1b2eff0[stage];
                v->statsRec.f34 = 0;
                v->statsRec.f38 = 0.0f;
                v->statsRec.f3c = 0.0f;
            }
            else
            {
                rec = &((ResSlotView *)cur)->stage[stage];
            }
        }

        /* The per-stage song textures + the grade stamp (CV2 only). */
        if (g_1b2eb6c == 0)
        {
            ((KGraphics *)g_sub1b2f054)->loadTexture(g_rankArt[rec->rank],
                                                     &v->f2998[cur->f8], 0);
            ((KGraphics *)g_sub1b2f054)->loadTexture(g_rankArtMask[rec->rank],
                                                     &v->f29a0[cur->f8], 0);
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->loadTexture(g_rankArtCv2[rec->rank],
                                                     &v->f2998[cur->f8], 0);
            ((KGraphics *)g_sub1b2f054)->loadTexture(g_rankArtCv2Mask[rec->rank],
                                                     &v->f29a0[cur->f8], 0);
        }
        g_stageRanks[stage] = rec->rank;
        if (g_1b2eb6c != 0)
        {
            int grade;
            if (rec->f18 == rec->notes)
                grade = 3;
            else if (rec->f14 + rec->f18 == rec->notes)
                grade = 2;
            else
                grade = (rec->f10 + rec->f14 + rec->f18 == rec->notes);
            v->f2c30[cur->f8] = (struct KTexture *)g_gradeCell[grade];
            rec->bonus = (int)v->f2c30[cur->f8];
            g_1b2e650[stage] = (int)v->f2c30[cur->f8];
        }

        /* The session totals. */
        total = g_1b2f058[0] + g_1b2f058[1] + g_1b2f058[3] + g_1b2f058[2];
        part = 0;
        for (j = 0; j < stage; j++)
            part += g_1b2f058[j];
        v->f2c64[cur->f8] = rec->score;
        if (v->f2c94 == -1 && v->f2c98 != -2)
        {
            if (g_modeIndex == 10)
            {
                v->f2c7c[cur->f8] = part;
                v->f2c5c[cur->f8] = total;
                ((ResSlotView *)cur)->f1ec = total;
            }
            else
            {
                int t;
                if (((PlayerSlot *)cur)->m42ef70() > 0)
                    t = ((PlayerSlot *)cur)->m42ef70();
                else
                    t = 0;
                v->f2c7c[cur->f8] = t;
                v->f2c5c[cur->f8] = ((PlayerSlot *)cur)->m42ef30();
                ((ResSlotView *)cur)->f1ec = v->f2c5c[cur->f8];
            }
        }
        else
        {
            v->f2c7c[cur->f8] = ((ResSlotView *)cur)->f1ec;
            v->f2c5c[cur->f8] = ((ResSlotView *)cur)->f1ec;
        }

        /* The combo-bonus deltas. */
        if (g_1b2eb6c != 0)
            v->f2c84[cur->f8] = (rec->bonus + rec->score) / 40;
        else
            v->f2c84[cur->f8] = rec->score / 40;
        v->f2c54 = (rec->f20 + rec->score) / 40;
        // STRING: EZ2AC_UNPACKED 0x00495d04
        Log("m_aComboBonusDelta = %d", v->f2c54);
        // STRING: EZ2AC_UNPACKED 0x00495cf8
        Log("%d Score", rec->score);

        ((KGraphics *)g_sub1b2f054)->loadTexture(g_bonusArt[v->f2c38],
                                                 &v->f29a8[cur->f8], 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(g_bonusArtMask[v->f2c38],
                                                 &v->f29b0[cur->f8], 0);

        /* The score ratios. */
        rc = (float)rec->f18 / (float)rec->notes * 100.0f;
        rb = (float)rec->f14 / (float)rec->notes * 100.0f;
        ra = (float)rec->score / (float)(rec->notes * 300) * 100.0f;
        ra2 = 0.0f;
        g_1b2f02c[cur->f8] = 0;

        if (g_modeIndex == 10)
        {
            int n;
            for (n = 0; n < rec->notes; n++)
            {
                float lg = (float)(log10(n + 1.0f) * 0.2f);
                v->f2c8c += 250 - (int)(float)(lg * -250.0f);
                // STRING: EZ2AC_UNPACKED 0x00495ce8
                Log("MAX Score = %d", v->f2c8c);
                /* (double) keeps this cast in x87 (cw+fistp) and therefore
                   un-hoistable, which is why the original recomputes this
                   loop-invariant every iteration. Residue: our 250.0f
                   promotes to a qword pool constant; the original keeps
                   fmuls with the single. */
                // STRING: EZ2AC_UNPACKED 0x00495cd4
                Log("Log Score = %d", (int)(float)(ra * 250.0f));
            }
            // STRING: EZ2AC_UNPACKED 0x00495cc4
            Log("TOTAL NOTE : %d", rec->notes);
            Log("MAX Score = %d", v->f2c8c);
            ra2 = (float)rec->score / (float)v->f2c8c * 100.0f;
            Log("Current Score = %d", v->f2c8c);
        }
        {
            float r = ra2;

            if (g_modeIndex != 10)
                r = ra;
            g_1b2ebf8[g_elemCount] = (int)(r * 10.0f);
        }

        /* The all-combo / perfect phase flags. */
        if (g_1b2eb6c == 0)
        {
            if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
                g_modeIndex == 9)
            {
                if (countListA() == 1)
                {
                    if (rec->f10 + rec->f14 + rec->f18 == rec->notes)
                    {
                        v->f10 = 1;
                        g_1b2f070[g_elemCount] = 1;
                        // STRING: EZ2AC_UNPACKED 0x00495c9c
                        Log("CourseModeAllCombo");
                    }
                    else
                    {
                        v->f10 = 0;
                        g_1b2f070[g_elemCount] = 0;
                    }
                }
                else
                {
                    if (rec->f10 + rec->f14 + rec->f18 == rec->notes)
                        v->f24[cur->f8] = 1;
                }
            }
            else
            {
                if (countListA() == 1)
                {
                    if (rec->f20 == rec->notes)
                    {
                        v->f10 = 1;
                        g_1b2f070[g_elemCount] = 1;
                    }
                    else
                    {
                        v->f10 = 0;
                        g_1b2f070[g_elemCount] = 0;
                    }
                }
                else
                {
                    if (rec->notes == rec->f20)
                        v->f24[cur->f8] = 1;
                }
            }
        }
        else
        {
            if (rec->f18 == rec->notes)
                v->f18 = 1;
            if (rec->f14 + rec->f18 == rec->notes)
                v->f14 = 1;
            if (rec->f10 + rec->f14 + rec->f18 == rec->notes)
                v->f10 = 1;
        }

        /* The 2P place readout. */
        if (countListA() == 2)
        {
            drawLine(&m_2c[cur->f8], g_placeCells[cur->f8].b,
                     (struct KTexture *)(int)rc,
                     g_placeCells[cur->f8].a, v->fontE);
        }

        /* The per-mode readout rows. Each row takes the pair's ADDRESS in a
           named pointer - that is what materializes the original's
           lea/shl/add per row instead of a folded displacement. */
        {
            int r10 = (int)(ra * 10.0f);
            ResPair *pp;
            if (countListA() == 1)
            {
                if (g_modeIndex == 10)
                {
                    pp = &g_readoutPos[cur->f8][0];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->notes), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][1];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f18), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][2];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f14), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][3];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f10), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][4];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->fc), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][5];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(g_1b2e704), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][6];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f20), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][7];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(int)(ra2 * 10.0f), pp->a, v->fontB);
                    pp = &g_readoutPos[cur->f8][8];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(v->f2c64[cur->f8]), pp->a, v->fontB);
                }
                else
                {
                    pp = &g_readoutPos[cur->f8][0];
                    if (g_1b2eb6c != 0)
                        drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->notes), pp->a, v->f2aac);
                    else
                        drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->notes), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][1];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f18), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][2];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f14), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][3];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f10), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][4];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->fc), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][5];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(g_1b2e704), pp->a, v->fontA);
                    pp = &g_readoutPos[cur->f8][6];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f20), pp->a, v->fontD);
                    pp = &g_readoutPos[cur->f8][7];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(r10), pp->a, v->fontB);
                    pp = &g_readoutPos[cur->f8][8];
                    drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(v->f2c64[cur->f8]), pp->a, v->fontB);
                }
            }
            else if (countListA() == 2)
            {
                pp = &g_readoutPos[cur->f8][0];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->notes), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][1];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f18), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][2];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f14), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][3];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f10), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][4];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->fc), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][5];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f8), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][6];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(rec->f20), pp->a, v->fontA);
                pp = &g_readoutPos[cur->f8][7];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(r10), pp->a, v->fontB);
                pp = &g_readoutPos[cur->f8][8];
                drawLine(&m_2c[cur->f8], pp->b, (struct KTexture *)(v->f2c64[cur->f8]), pp->a, v->fontB);
            }
        }

        /* The song cells and the published counters. */
        m_2c[cur->f8].setCell(g_songCellA[cur->f8], 0, (int)v->f2998[cur->f8]);
        m_2c[cur->f8].setCell(g_songCellB[cur->f8], 0, (int)v->f29a0[cur->f8]);
        {
            int t = rec->f20;
            g_1b2efec = t;
            if (t >= 9999)
                g_1b2efec = 9999;
        }
        g_1b2ebc0 = (int)rb;
        // STRING: EZ2AC_UNPACKED 0x00495c8c
        ((KGraphics *)g_sub1b2f054)->loadStr("AllCombo.str", &res);
        m_1544.attach(res, 1);
    }

    if (g_1b2eb6c != 0)
    {
        /* nothing */
    }
    else if (countListA() == 1)
    {
        if (g_1b2ea28 == 0)
            ((ResSample *)v->sndBgm)->t4(0);
    }
    else if (countListA() >= 2 && g_1b2e7d8 <= 1)
    {
        ((ResSample *)v->sndBgm)->t4(0);
    }
    popDirectory();
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp",
                                             &v->texFade, 0);
    v->f30f8 = 5;
    v->f30fc = 5;
    if (g_eventMode == 0 && g_modeIndex != 12)
        m450690();
}
