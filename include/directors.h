#ifndef DIRECTORS_H
#define DIRECTORS_H

#include "panels.h"      /* EzObject, Obj40B2E0, EzSoundObject */
#include "ezscreen.h"    /* EzPooled, ScrFader, g_objectRecord */

/* Three more directors whose destructors are their member maps. Sizes come from
   their object records; every member offset and count below is read straight
   off the destructor's EH states.

     vtable    name              size      dtor      deleting ~
     0x498680  RankingDirector   0x109de8  0x456750  0x459850
     0x495c5c  ResultDirector    0x3118    0x450ce0  0x451ae0
     0x49b210  GFResultDirector  0x2dd0    0x467480  0x468030

   The two result directors are the same class twice over: release a fixed set
   of sound objects through slot 5, put four pieces of cabinet state back, then
   let the members go. */

/* Eleven members, states 0..10. */
struct RankingDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x456730 */
    virtual DrawGroup     *getObjectInfo();    /* 0x456740 */
    virtual void           update();           /* 0x45a290 (src/rankingupdate.cpp) */
    virtual void           update2(int a, int b); /* 0x45bc30 (src/rankingtick.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    /* The name-entry stepper (wip/ranking-nameentry.cpp) and its commit. */
    void m45aaf0(int a, int b);   /* 0x45aaf0 (src/rankingname.cpp) */
    void m4577b0();               /* 0x4577b0 (src/rankingdefault.cpp) */
    void m456b20();               /* 0x456b20 (src/rankinginsert.cpp) */
    void m4570a0();               /* 0x4570a0 (src/rankingchannel.cpp) */
    void m4574c0(float x, float y, int c, int white, int alpha); /* 0x4574c0 (src/rankingrow.cpp) */
    ~RankingDirector();                        /* 0x456750 */
    RankingDirector();                         /* 0x457a90 (src/rankingctor.cpp) */
    /* The go-ahead the constructor asks for before entering name entry. */
    int m456870();                             /* 0x456870 (STUB) */
    /* The two helpers update() drives. m459870 walks the visible ranking
       rows for one player column: x (63.0f / 383.0f for two players,
       400.0f for one), the scroll position in 46-pixel rows, and which
       column (src/rankingwalk.cpp). */
    void m459870(float x, float t, int which); /* 0x459870 */
    /* The name-entry overlay: the eight big glyphs of the name so far,
       the glyph wheel, and - two-up - each player's live cursor glyph
       (src/rankingwheel.cpp). */
    void m459bc0();                            /* 0x459bc0 */

    /* The mode this director is in - update() @0x45a290 dispatches on it
       through a seven-entry jump table, and skips the video draw at 5. */
    int   f10;                 /* 0x0010 */
    Obj402860 m_14;            /* 0x0014 */
    Obj402860 m_230;           /* 0x0230 */
    Obj402860 m_44c;           /* 0x044c */
    Obj402860 m_668;           /* 0x0668 */
    Obj402860 m_884[2];        /* 0x0884 */
    Obj402860 m_cbc[2];        /* 0x0cbc */
    Obj402860 m_10f4;          /* 0x10f4 */
    Obj402860 m_1310;          /* 0x1310 */
    Obj402860 m_152c;          /* 0x152c */
    Obj40B2E0 m_1748;          /* 0x1748 */
    /* Everything below is read off update() @0x45a290 - see src/rankingupdate.cpp.
       The offsets are exact and the total size is unchanged at 0x109de8. */
    /* The fifty BIG glyphs the name-entry overlay draws (m459bc0). */
    struct KTexture *m_bigGlyphs[50];    /* 0x1c20 */
    /* The row drawer's texture tables (m4574c0): fifty name glyphs, the
       small score digits, and the big place digits with the dot at [10]. */
    struct KTexture *m_glyphTex[50];     /* 0x1ce8 */
    struct KTexture *m_scoreDigits[20];  /* 0x1db0 */
    struct KTexture *m_placeDigits[11];  /* 0x1e00 */
    /* The ten digit glyphs the two-digit readout indexes with `v % 10`. */
    struct KTexture *m_digits[10];  /* 0x1e2c */
    char _pad1e54[0x1ec0 - 0x1e54];
    /* The value both cell tables get written into row 8, column 0. */
    int   f1ec0;               /* 0x1ec0 */
    char _pad1ec4[0x1ecc - 0x1ec4];
    /* ASCII -> glyph index; entries >= 50 draw nothing (m4574c0). */
    int   m_glyphMap[128];               /* 0x1ecc */
    /* The LOCAL ranking tables: 50-byte name records and their scores.
       The CHANNEL pair sits below the f85df8 selector. */
    char  m_names1[0x7c1bc - 0x20cc];    /* 0x20cc */
    int   m_scores1[(0x85df8 - 0x7c1bc) / 4];  /* 0x7c1bc */
    /* Which pair of per-player counters the two-player lamp test reads. */
    char  f85df8;              /* 0x85df8 */
    char _pad85df9[0x85dfc - 0x85df9];
    int   f85dfc;              /* 0x85dfc */
    int   f85e00;              /* 0x85e00 */
    char  m_names2[0xffef4 - 0x85e04];         /* 0x85e04 */
    int   m_scores2[(0x109b30 - 0xffef4) / 4]; /* 0xffef4 */
    /* The fade level, stepped by 9 a tick and clamped to 0xff, splatted into
       all three colour channels. */
    int   f109b30;             /* 0x109b30 */
    float f109b34;             /* 0x109b34 */
    float f109b38;             /* 0x109b38 */
    char _pad109b3c[0x109b44 - 0x109b3c];
    int   f109b44;             /* 0x109b44 */
    int   f109b48;             /* 0x109b48 */
    int   f109b4c;             /* 0x109b4c */
    char _pad109b50[0x109b58 - 0x109b50];
    /* FLOATS, not ints - update2 @0x45bc30 ramps them with movss/comiss.
       The pair at +0x109b58 is the displayed score counter and the pair at
       +0x109b60 its target; the readout divides by 46. */
    float f109b58;             /* 0x109b58 */
    float f109b5c;             /* 0x109b5c */
    float f109b60;             /* 0x109b60 */
    float f109b64;             /* 0x109b64 */
    Obj402860 m_109b68;        /* 0x109b68 */
    int   f109d84;             /* 0x109d84 */
    int   f109d88;             /* 0x109d88 */
    /* The three name-entry click samples: wheel, type, confirm. */
    void *f109d8c;             /* 0x109d8c */
    void *f109d90;             /* 0x109d90 */
    int   f109d94;             /* 0x109d94 */
    int   f109d98;             /* 0x109d98 */
    int   f109d9c;             /* 0x109d9c */
    struct KTexture *m_109da0; /* 0x109da0 */
    /* The alpha byte of the overlay's colour: (a << 24) | 0xffffff. */
    int   f109da4;             /* 0x109da4 */
    /* The name-entry per-slot state: wheel row, position, and the
       eased wheel scroll with its target (m45aaf0). */
    int   f109da8[2];          /* 0x109da8 */
    int   f109db0[2];          /* 0x109db0 */
    /* The two-up live readout's vertical nudge, per player (m459bc0). */
    float f109db8[2];          /* 0x109db8 */
    float f109dc0[2];          /* 0x109dc0 */
    float f109dc8[2];          /* 0x109dc8 */
    int   f109dd0;             /* 0x109dd0 */
    int   f109dd4;             /* 0x109dd4 */
    char _pad109dd8[0x109ddc - 0x109dd8];
    /* The two-digit number the readout draws. */
    int   f109ddc;             /* 0x109ddc */
    int   f109de0;             /* 0x109de0 */
    int   f109de4;             /* 0x109de4 */
};

/* Seventeen members, states 0..16, and seven sound objects at +0x28e4. The one
   at +0x28fc is the only one released behind a null check. */
struct ResultDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x450670 */
    virtual DrawGroup     *getObjectInfo();    /* 0x450680 */
    virtual void           update();           /* 0x451b80 (src/resultdraw.cpp) */
    virtual void           update2(int, int);  /* 0x451550 (src/resultupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~ResultDirector();                         /* 0x450ce0 */
    ResultDirector();                          /* 0x451f10 (src/resultctor.cpp) */
    /* The song-name trim - the videoName family's sibling. */
    char *m450c00();                           /* 0x450c00 (STUB) */
    /* Decide the battle winner into +0x2c94/+0x2c98. */
    void m451200();                            /* 0x451200 (STUB) */
    void m4512b0();                            /* 0x4512b0 (STUB) */
    /* Shared with GFResultDirector - the original's linker folded it, and
       this class calls it directly at 0x467900 rather than having its own. */
    void m467900();                            /* 0x467900 (STUB) */
    /* The end-of-ctor score file write. */
    void m450690();                            /* 0x450690 (STUB) */
    /* The shared ten-digit-run loader - the same address as
       TotalResultDirector::loadDigits, which owns it. */
    int loadDigits(char *base, struct KTexture **dst);  /* 0x454480 (folded) */

    /* Three per-player flags, as GFResultDirector keeps at its own +0x10. */
    int  f10;                  /* 0x0010 */
    int  f14;                  /* 0x0014 */
    int  f18;                  /* 0x0018 */
    char _pad1c[0x24 - 0x1c];
    /* Two more per-player flags update @0x451b80 gates the +0x1db4 and
       +0x1fd0 charts on, in the two-player case. */
    int  f24;                  /* 0x0024 */
    int  f28;                  /* 0x0028 */
    Obj402860 m_2c[2];         /* 0x002c */
    Obj402860 m_464[2];        /* 0x0464 */
    Obj402860 m_89c;           /* 0x089c */
    Obj402860 m_ab8;           /* 0x0ab8 */
    Obj402860 m_cd4;           /* 0x0cd4 */
    Obj402860 m_ef0;           /* 0x0ef0 */
    Obj402860 m_110c;          /* 0x110c */
    Obj402860 m_1328;          /* 0x1328 */
    Obj402860 m_1544;          /* 0x1544 */
    Obj402860 m_1760;          /* 0x1760 */
    Obj402860 m_197c;          /* 0x197c */
    Obj402860 m_1b98;          /* 0x1b98 */
    Obj402860 m_1db4;          /* 0x1db4 */
    Obj402860 m_1fd0;          /* 0x1fd0 */
    Obj402860 m_21ec;          /* 0x21ec */
    Obj40B2E0 m_2408;          /* 0x2408 */
    /* The fader update2 @0x451550 ticks through slot 17 every frame - the
       twin of GFResultDirector's m_21dc. */
    EzSoundObject *m_28e0;     /* 0x28e0 */
    EzSoundObject *m_28e4;     /* 0x28e4 */
    EzSoundObject *m_28e8;     /* 0x28e8 */
    EzSoundObject *m_28ec;     /* 0x28ec */
    EzSoundObject *m_28f0;     /* 0x28f0 */
    EzSoundObject *m_28f4;     /* 0x28f4 */
    EzSoundObject *m_28f8;     /* 0x28f8 */
    EzSoundObject *m_28fc;     /* 0x28fc */
    char _pad2900[0x2904 - 0x2900];
    int  m_2904;               /* 0x2904 */
    char _pad2908[0x298c - 0x2908];
    /* The screen background the tick blits first, and the fade overlay it
       blits last while the phase is 0 or 7. */
    struct KTexture *m_298c;   /* 0x298c */
    char _pad2990[0x2994 - 0x2990];
    struct KTexture *m_2994;   /* 0x2994 */
    char _pad2998[0x29b8 - 0x2998];
    /* The rank plates, four tiers by the f10/f14/f18 flags: cell 0x2c takes
       the first four, cell 0x2d the second four. */
    struct KTexture *m_rankA[4];  /* 0x29b8 .. 0x29c8 */
    struct KTexture *m_rankB[4];  /* 0x29c8 .. 0x29d8 */
    char _pad29d8[0x29dc - 0x29d8];
    /* The two-player header plate, picked on g_1b2ea28 and g_1b2e7d8. */
    struct KTexture *m_29dc;   /* 0x29dc */
    struct KTexture *m_29e0;   /* 0x29e0 */
    char _pad29e4[0x2a0c - 0x29e4];
    int  m_2a0c;               /* 0x2a0c - passed by address to every draw */
    char _pad2a10[0x2ae0 - 0x2a10];
    /* The option-icon textures m4512b0 @0x4512b0 stamps into m_89c's cells 1,
       2 and 3 - one per option flag, indexed here by (offset - 0x2ae0) / 4 so
       the ladder reads as the table it is. 0x2b34 is unused. */
    struct KTexture *m_optTex[(0x2b54 - 0x2ae0) / 4];  /* 0x2ae0 .. 0x2b54 */
    char _pad2b54[0x2b68 - 0x2b54];
    /* The ten digit textures the stage number is drawn from (m450ef0). */
    struct KTexture *m_digits[10];  /* 0x2b68 */
    char _pad2b90[0x2c30 - 0x2b90];
    /* The one-off bonus each player is owed, and the flag that says it has
       not been paid yet - m451180 @0x451180 pays it once for the whole walk
       and clears the flag. */
    int  m_2c30[2];            /* 0x2c30 */
    char _pad2c38[0x2c3c - 0x2c38];
    int  m_2c3c;               /* 0x2c3c */
    /* The screen's phase, the overlay's alpha and the phase's frame counter -
       GFResultDirector's f2910/f2914/f2918 at this class's own offsets.
       f2c40 only ever holds 0, 1, 4 or 7: update2 @0x451550 is the only
       writer in the binary (1, 4, 7); phases 2/3/5 are unreachable. */
    int  f2c40;                /* 0x2c40 */
    int  f2c44;                /* 0x2c44 */
    int  f2c48;                /* 0x2c48 */
    char _pad2c4c[0x2c5c - 0x2c4c];
    /* Each player's score, and where m4510a0 latches it. */
    int  m_score[2];           /* 0x2c5c */
    struct KTexture *m_2c64[2];/* 0x2c64 - one per player */
    char _pad2c6c[0x2c7c - 0x2c6c];
    int  m_scoreLatch[2];      /* 0x2c7c */
    /* How much of the loser's latch moves to the winner's each tick
       (m450fa0 @0x450fa0). */
    int  m_scoreStep[2];       /* 0x2c84 */
    char _pad2c8c[0x2c90 - 0x2c8c];
    int  m_2c90;       /* 0x2c90 */
    int  winner;       /* 0x2c94 */
    int  loser;        /* 0x2c98 */
    char _pad2c9c[0x2ca4 - 0x2c9c];
    Obj402860 m_2ca4[2];       /* 0x2ca4 */
    /* The five place-number plates, and the two shared ones m450690 loads:
       PLACE_f.bmp for the single-chart cases, PLACE_c.bmp for a player who
       ranked nothing or when g_1b5f178 is up. */
    struct KTexture *m_rankPlate[5];  /* 0x30dc */
    struct KTexture *m_placeF;        /* 0x30f0 */
    struct KTexture *m_placeC;        /* 0x30f4 */
    /* Per-player phase of the result sequence; 5 is the last one, and the
       two charts are ticked before it and after it by different rules
       (m450b50 @0x450b50). */
    int  m_state[2];           /* 0x30f8 */
    /* The frame counts update2 compares f2c48 against - the twins of
       GFResultDirector's f2db8/f2dbc/f2dc0/f2dc8. */
    int  f3100;                /* 0x3100 */
    int  f3104;                /* 0x3104 */
    int  f3108;                /* 0x3108 */
    char _pad310c[0x3110 - 0x310c];
    int  f3110;                /* 0x3110 */
    char _pad3114[0x3118 - 0x3114];

    /* Draw the stage number: one digit centred at x=372, or two at 367 and
       377, all on the y=373 baseline. */
    /* Move one tick of the loser's latched score across to the winner
       (src/scoretransfer.cpp). */
    void m450fa0();            /* 0x450fa0 */
    /* Tick the two result charts, by phase (src/resulttween.cpp). */
    void m450b50();            /* 0x450b50 */
    void drawStageNumber();    /* 0x450ef0 */
    /* Walk list A and draw one line per player. */
    void drawPlayers();        /* 0x451b00 */
    /* Latch each player's score, then hand the battle pair their combined
       total. */
    /* Advance every present player's latched score by its step
       (src/scoretransfer.cpp). */
    void m451040();            /* 0x451040 */
    /* Pay the pending bonus once, restarting the sound at +0x28ec
       (src/scoretransfer.cpp). */
    void m451180();            /* 0x451180 */
    void m4510a0();            /* 0x4510a0 */
};

/* Sixteen members, states 0..15, and six sound objects at +0x21e0 - the same
   shape as ResultDirector with one chart and one sound object fewer. */
/* One (x, y) of the result readout's position table @0x49b0e8 - ten
   pairs per side; pair 8 is what drawPlayers reads. */
struct GfrLine
{
    int f0;
    int f4;
};

struct GFResultDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x466ec0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x466ed0 */
    virtual void           update();           /* 0x4680d0 (src/gfresultupdate.cpp) */
    virtual void           update2(int a, int b); /* 0x467b90 (src/gfresultupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    /* The class's own copy of the video-name trim. */
    char *m4673a0();                           /* 0x4673a0 */
    ~GFResultDirector();                       /* 0x467480 */
    GFResultDirector();                        /* 0x4682e0 (src/gfresultctor.cpp) */
    void m466ee0();                            /* 0x466ee0 (STUB) */
    /* The shared ten-digit-run loader (see ResultDirector's copy). */
    int loadDigits(char *base, struct KTexture **dst);  /* 0x454480 (folded) */

    /* Three per-player flags update() @0x4680d0 gates three of its tween
       refreshes on. */
    int  f10;                  /* 0x0010 */
    int  f14;                  /* 0x0014 */
    int  f18;                  /* 0x0018 */
    /* Indexed by the list node's id, not just a pair. */
    Obj402860 m_1c[2];         /* 0x001c */
    Obj402860 m_454[2];        /* 0x0454 */
    Obj402860 m_88c;           /* 0x088c */
    Obj402860 m_aa8;           /* 0x0aa8 */
    Obj402860 m_cc4;           /* 0x0cc4 */
    Obj402860 m_ee0;           /* 0x0ee0 */
    Obj402860 m_10fc;          /* 0x10fc */
    Obj402860 m_1318;          /* 0x1318 */
    Obj402860 m_1534;          /* 0x1534 */
    Obj402860 m_1750;          /* 0x1750 */
    Obj402860 m_196c;          /* 0x196c */
    Obj402860 m_1b88;          /* 0x1b88 */
    Obj402860 m_1da4;          /* 0x1da4 */
    Obj402860 m_1fc0;          /* 0x1fc0 */
    /* The fader update2 @0x467b90 ticks through slot 17 every frame. */
    EzSoundObject *m_21dc;     /* 0x21dc */
    EzSoundObject *m_21e0;     /* 0x21e0 */
    EzSoundObject *m_21e4;     /* 0x21e4 */
    EzSoundObject *m_21e8;     /* 0x21e8 */
    EzSoundObject *m_21ec;     /* 0x21ec */
    EzSoundObject *m_21f0;     /* 0x21f0 */
    EzSoundObject *m_21f4;     /* 0x21f4 */
    EzSoundObject *m_21f8;     /* 0x21f8 - SetBonus.wav */
    /* The one phase 4 restarts when the tween's frame reaches 0x19. */
    EzSoundObject *m_21fc;     /* 0x21fc */
    char _pad2200[0x2240 - 0x2200];
    /* The overlay update() blits at the end. */
    struct KTexture *m_2240;   /* 0x2240 */
    /* Per-side rank plates, clear plates, and the first small digit
       font block loadDigits fills. */
    struct KTexture *f2244[2]; /* 0x2244 */
    struct KTexture *f224c[2]; /* 0x224c */
    struct KTexture *f2254[2]; /* 0x2254 */
    struct KTexture *f225c[2]; /* 0x225c */
    struct KTexture *m_2264[10];  /* 0x2264 - a loadDigits run */
    struct KTexture *m_228c[10];  /* 0x228c - a loadDigits run */
    struct KTexture *m_22b4[10];  /* 0x22b4 - a loadDigits run */
    struct KTexture *m_22dc[10];  /* 0x22dc - a loadDigits run */
    struct KTexture *m_2304[10];  /* 0x2304 - a loadDigits run */
    struct KTexture *m_232c;   /* 0x232c - TT_Result_Top_m */
    /* The two title textures update() writes into row 6 of the header
       table (TT_Result_Top_c / _f). */
    int  f2330;                /* 0x2330 */
    int  f2334;                /* 0x2334 */
    /* The result screen's texture set, carved out of the pad from the
       constructor @0x4682e0's own loads (scratchpad/assetmap.py). The long
       run from +0x234c is the option-icon strip the readout draws. */
    struct KTexture *m_2338;  /* System\Channel_Tag\Radio\%s.bmp */
    struct KTexture *m_233c;  /* System\ModeSelect\inout\ModeIntro_%s.bmp"  (v */
    struct KTexture *m_2340;  /* System\ModeSelect\inout\ModeIntro_%s_Mask.bmp */
    struct KTexture *m_2344;  /* System\Result\ModeIcon\Icon_%s.bmp */
    char _pad2348[0x234c - 0x2348];
    struct KTexture *m_234c;  /* System\Result\Effector\EFFECTOR_RANDOM_OFF.bm */
    struct KTexture *m_2350;  /* System\Result\Effector\EFFECTOR_RANDOM.bmp */
    struct KTexture *m_2354;  /* System\Result\Effector\EFFECTOR_SRANDOM.bmp */
    struct KTexture *m_2358;  /* System\Result\Effector\EFFECTOR_PS.bmp */
    struct KTexture *m_235c;  /* System\Result\Effector\EFFECTOR_FR.bmp */
    struct KTexture *m_2360;  /* System\Result\Effector\EFFECTOR_MRANDOM.bmp */
    struct KTexture *m_2364;  /* System\Result\Effector\EFFECTOR_HRANDOM.bmp */
    struct KTexture *m_2368;  /* System\Result\Effector\EFFECTOR_MIRROR.bmp */
    struct KTexture *m_236c;  /* System\Result\Effector\EFFECTOR_MIRROR_A.bmp */
    struct KTexture *m_2370;  /* System\Result\Effector\EFFECTOR_KEY.bmp */
    struct KTexture *m_2374;  /* System\Result\Effector\EFFECTOR_SP.bmp */
    struct KTexture *m_2378;  /* System\Result\Effector\EFFECTOR_AUTO_SPEDAL_O */
    struct KTexture *m_237c;  /* System\Result\Effector\EFFECTOR_AUTO_SCRATCH_ */
    struct KTexture *m_2380;  /* System\Result\Effector\EFFECTOR_AUTO_PEDAL_ON */
    struct KTexture *m_2384;  /* System\Result\Effector\EFFECTOR_AUTO_SPEDAL_O */
    struct KTexture *m_2388;  /* System\Result\Effector\EFFECTOR_AUTOS_off.bmp */
    struct KTexture *m_238c;  /* System\Result\Effector\EFFECTOR_AUTOS_L.bmp */
    struct KTexture *m_2390;  /* System\Result\Effector\EFFECTOR_AUTOS_R.bmp */
    struct KTexture *m_2394;  /* System\Result\Effector\EFFECTOR_AUTOS_on.bmp */
    struct KTexture *m_2398;  /* System\Result\Effector\EFFECTOR_4D_OFF.bmp */
    struct KTexture *m_239c;  /* System\Result\Effector\EFFECTOR_4D_on.bmp */
    struct KTexture *m_23a0;  /* System\Result\Effector\EFFECTOR_5D_on.bmp */
    struct KTexture *m_23a4;  /* System\Result\Effector\EFFECTOR_FADEOUT_OFF.b */
    struct KTexture *m_23a8;  /* System\Result\Effector\EFFECTOR_FADEOUT.bmp */
    struct KTexture *m_23ac;  /* System\Result\Effector\EFFECTOR_FADEIN.bmp */
    struct KTexture *m_23b0;  /* System\Result\Effector\EFFECTOR_BLINK.bmp */
    struct KTexture *m_23b4;  /* System\Result\Effector\EFFECTOR_SWITCH_OFF.bm */
    struct KTexture *m_23b8;  /* System\Result\Effector\EFFECTOR_SWITCH_ON.bmp */
    struct KTexture *m_23bc;  /* System\Result\Effector\Effector_Reverse_on.bm */
    struct KTexture *m_23c0;  /* System\Result\Diff_Normal.bmp */
    struct KTexture *m_23c4;  /* System\Result\Diff_Hard.bmp */
    struct KTexture *m_23c8;  /* System\Result\Diff_SHard.bmp */
    struct KTexture *m_23cc;   /* System\Result\Diff_Ex.bmp */
    /* The four stage plates; [3] doubles as the 2P plate. */
    struct KTexture *f23d0[4]; /* 0x23d0 */
    struct KTexture *m_23e0;  /* System\SongSelect\lvfont\LV%02d.bmp */
    struct KTexture *m_23e4;  /* system\SongName\%s.bmp */
    char _pad23e8[0x23ec - 0x23e8];
    struct KTexture *m_23ec;  /* system\ResultDisc\%s.bmp */
    char _pad23f0[0x242c - 0x23f0];
    /* The per-side clear marker (a pointer from the 0x49b1e8 pair). */
    int  f242c[2];             /* 0x242c */
    char _pad2434[0x2438 - 0x2434];
    Obj40B2E0 m_2438;          /* 0x2438 */
    /* The screen's phase, and the overlay's alpha byte. */
    int  f2910;                /* 0x2910 */
    int  f2914;                /* 0x2914 */
    /* The phase's frame counter, bumped once per tick by update2
       @0x467b90, and a latch it sets when it starts the outro sound. */
    int  f2918;                /* 0x2918 */
    char _pad291c[0x2928 - 0x291c];
    int  f2928;                /* 0x2928 */
    /* The same score/latch/winner group ResultDirector keeps at 0x2c5c, but
       NOT at a uniform offset from it: the scores are 0x330 lower, the latch
       0x340 and the winner pair 0x348, so the padding between the groups
       differs between the two classes. */
    int  m_score[2];           /* 0x292c */
    struct KTexture *m_2934[2];/* 0x2934 */
    int  m_scoreLatch[2];      /* 0x293c */
    /* The GF copy of the per-tick transfer step (m467720 @0x467720). */
    int  m_scoreStep[2];       /* 0x2944 */
    int  winner;               /* 0x294c */
    int  loser;                /* 0x2950 */
    char _pad2954[0x295c - 0x2954];
    Obj402860 m_295c[2];       /* 0x295c */
    struct KTexture *f2d94[5]; /* 0x2d94 - PLACE_%u */
    struct KTexture *m_placeF; /* 0x2da8 - PLACE_f */
    struct KTexture *m_placeC; /* 0x2dac - PLACE_c */
    /* The GF copy of the per-player result phase (m4672f0 @0x4672f0). */
    int  m_state[2];           /* 0x2db0 */
    /* The four frame counts update2 @0x467b90 compares f2918 against. */
    int  f2db8;                /* 0x2db8 */
    int  f2dbc;                /* 0x2dbc */
    int  f2dc0;                /* 0x2dc0 */
    int  f2dc4;                /* 0x2dc4 */
    int  f2dc8;                /* 0x2dc8 */
    char _pad2dcc[0x2dd0 - 0x2dcc];

    void drawPlayers();        /* 0x468050 */
    /* ResultDirector::m4510a0's twin over this class's own group. */
    void m467820();            /* 0x467820 */
    /* Tick the two result charts, by phase (src/resulttween.cpp). */
    void m4672f0();            /* 0x4672f0 */
    /* The four helpers update2 @0x467b90 drives; m467900 is its workhorse,
       called eight times. */
    void m467900();            /* 0x467900 (STUB) */
    void m4679e0();            /* 0x4679e0 (STUB) */
    /* Advance every present player's latched score by its step
       (src/scoretransfer.cpp). */
    void m4677c0();            /* 0x4677c0 */
    /* Move one tick of the loser's latched score across to the winner
       (src/scoretransfer.cpp). */
    void m467720();            /* 0x467720 */
};

/* The object at 0x1b2e844, as its callers see it: only slot 0 is ever used, and
   always with one int argument. GameOverSink's override @0x44da10 forwards
   straight to it, and EndingDirector's constructor calls it twice. */
struct SoundBank
{
    virtual void v0(int a);
};

/* THE SECOND BASE. Three of the four classes below store a SECOND vtable
   pointer at +0x10 - right after the root's own 0x10 bytes - which is multiple
   inheritance: an interface with a single virtual taking one int. The override
   sits in the secondary vtable directly, with no adjustor thunk, so it is
   written on the second base itself (a thunk is what cl would emit if the
   override were declared on the derived class). Each class gets its own sink
   type for that reason; what the notification means is not evidenced beyond
   WarningDirector's, which latches two fields when the second argument is 2. */

/* One member, an Obj402860 at +0x10, and NO second base - the only one of the
   four warning/result screens without one. Size 0x230. */
struct WarningShortDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x469a00 */
    virtual DrawGroup     *getObjectInfo();    /* 0x469a10 */
    virtual void           update();              /* 0x469a80 */
    /* `ret 8` - two arguments, both unused. */
    virtual void           update2(int a, int b); /* 0x469ac0 */
    virtual int            v5(int, int, int);     /* 0x401770 (folded) */

    WarningShortDirector();                    /* 0x469b00 */
    ~WarningShortDirector();                   /* 0x469a20 */

    Obj402860 m_10;            /* 0x10 .. 0x22c */
    int  m_22c;                /* 0x22c - timeGetTime at construction */
};

/* `ret 8`, so the virtual takes TWO ints and the first one is unused at both
   call sites we can see. novtable like every other secondary base here: without
   it cl stores the sink's OWN vftable into +0x10 before the derived one, and
   the constructor comes out with an extra store the original does not have. */
struct __declspec(novtable) GameOverSink
{
    virtual void setMode(int unused, int mode);   /* 0x44da10 (STUB) */
};

/* Size 0x23c: root 0x10, the sink's vptr at 0x10, one word at 0x14, the chart
   at 0x18. */
struct GameOverDirector : EzObject, GameOverSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x44d940 */
    virtual DrawGroup     *getObjectInfo();    /* 0x44d950 */
    virtual void           update();           /* 0x44d9c0 */
    virtual void           update2(int, int);  /* 0x44db70 (src/endingupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    /* Hand the sink to the shared director object with mode 7 - update2
       calls it once the counter at +0x14 reaches 200. */
    int  m44d9d0();                            /* 0x44d9d0 */
    ~GameOverDirector();                       /* 0x44d960 */
    GameOverDirector();                        /* 0x44da30 */

    int  m_14;                 /* 0x14 */
    Obj402860 m_18;            /* 0x18 */
    int  m_sample;             /* 0x234 */
    int  m_started;            /* 0x238 */
};

/* novtable: as a SECONDARY base it must not store a vptr of its own, or
   WarningDirector's constructor gains an extra store before the derived
   two. */
struct __declspec(novtable) WarningSink
{
    virtual void setMode(int unused, int mode);   /* 0x45c300 */
};

/* What WarningSink::setMode writes through its own `this`: because the override
   is compiled for the SECOND base's pointer (no adjustor thunk), the two
   derived fields it touches - the object's +0x14 and +0x238 - appear at sink+4
   and sink+0x228. The sink itself is only its vptr, so this is a view, not the
   base's layout. */
struct WarningSinkFields
{
    char _pad0[4];
    int  f4;                   /* sink+0x004 = WarningDirector+0x014 */
    char _pad8[0x228 - 8];
    int  f228;                 /* sink+0x228 = WarningDirector+0x238 */
};

/* The same shape as GameOverDirector, size 0x23c - which its factory
   createWarningDirector @0x45c3f0 confirms by pushing exactly that. */
struct WarningDirector : EzObject, WarningSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x45c1c0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x45c1d0 */
    virtual void           update();           /* 0x45c240 (src/endingupdate2.cpp) */
    virtual void           update2(int, int);  /* 0x45c2b0 (STUB) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    WarningDirector();                         /* 0x45c320 */
    ~WarningDirector();                        /* 0x45c1e0 */

    int  m_14;                 /* 0x14 */
    Obj402860 m_18;            /* 0x18 .. 0x234 */
    struct KTexture *m_tex;    /* 0x234 - System\\warning.bmp */
    int  m_238;                /* 0x238 - the sink's +0x228 */
};

/* Two Obj402860 at +0x10 and +0x22c, nothing else. Size 0x460. */
struct EndingDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x455ba0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x455bb0 */
    virtual void           update();           /* 0x455c30 (src/endingupdate2.cpp) */
    virtual void           update2(int a, int b); /* 0x455ce0 (src/endingupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~EndingDirector();                         /* 0x455bc0 */

    Obj402860 m_10;            /* 0x010 */
    Obj402860 m_22c;           /* 0x22c */
    void     *m_sample;        /* 0x448 - "ending.wav" */
    int       f44c;            /* 0x44c - 1 */
    int       f450;            /* 0x450 - 0 */
    int       f454;            /* 0x454 - 0xff, a fade level */
    struct KTexture *m_fade;   /* 0x458 - the black fade bitmap */
    unsigned int m_started;    /* 0x45c - timeGetTime() at construction */

    EndingDirector();                          /* 0x455df0 */
};

/* One entry of the in-game song select's list: the song's art and its name.
   The name buffer's extent is exact - m425250 @0x425250 strcpy's the entry name
   straight into +0x4 of the record, and the record is 0x10c bytes. */
struct InGameSongEntry
{
    struct KTexture *tex;      /* 0x000 - system\discsmall\<name>.bmp */
    char             name[0x104];  /* 0x004 */
    int              level;    /* 0x108 - m425140's read of "<mode>1p-<name>.ini" */
};

/* Four Obj402860 at the very end of a 0x39a58-byte object. */
struct InGameSongSelect : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x425120 */
    virtual DrawGroup     *getObjectInfo();    /* 0x425130 */
    virtual void           update();           /* 0x4261d0 (src/ingameupdate.cpp) */
    virtual void           update2(int, int);  /* 0x4262a0 (src/ingameupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~InGameSongSelect();                       /* 0x425830 */

    /* Loads the song table off disk - a 0x225c-byte frame with a KFile and a
       KProfile on it. The constructor's last act. */
    void m425250();                            /* 0x425250 - loads the two song lists */
    /* Draw one list entry: the frame behind it, then the entry's own art, both
       centred on (x, y) and tinted by one byte replicated four times. */
    void m4258d0(int n, float x, float y, float w, float h, int a);  /* 0x4258d0 */
    /* Read one int out of "<mode>1p-<name>.ini". */
    int  m425140(char *name);                  /* 0x425140 */
    /* Whatever the tick does to the selection before the two lists are
       refreshed - update() @0x4261d0 calls it and nothing else does. */
    void m425b40();                            /* 0x425b40 (STUB) */
    /* What update2 advances once its own tick has been taken. */
    void m425ea0();                            /* 0x425ea0 (STUB) */

    int  f10;                  /* 0x010 - zeroed just before m425250 */
    void *f14;                 /* 0x014 - the director that built it */
    char _pad18[0x24 - 0x18];
    int  m_count[2];           /* 0x024 - entries in each slot's list */
    int  f2c[2];               /* 0x02c - zeroed per slot by the loader */
    int  f34[2];               /* 0x034 */
    float f3c[2];              /* 0x03c - m_count * 50 */
    /* The ease target - the selection times 100. */
    float f44[2];              /* 0x044 */
    float f4c;                 /* 0x04c - 90 with a list-A first player, 550 without */
    /* Two slots' worth of entries; the row stride 0x1c870 and the element
       0x10c both come out of m4258d0's address arithmetic. */
    InGameSongEntry m_entry[2][436];   /* 0x050 .. 0x39130 exactly */
    Obj402860 m_39130;         /* 0x39130 - difficulty_1p.str */
    Obj402860 m_3934c;         /* 0x3934c - difficulty_2p.str */
    Obj402860 m_39568;         /* 0x39568 - StartSkip.str */
    Obj402860 m_39784;         /* 0x39784 - TR.str */
    struct KTexture *m_399a0;  /* 0x399a0 - the frame behind every entry (disc-mask) */
    struct KTexture *m_diff[19]; /* 0x399a4 - clubmix\panel\difficulty_%d.bmp */
    char _pad399f0[0x39a44 - 0x399f0];
    /* Armed by m425ea0's tail: -1, or 0 in mode 4 off slot 2. */
    int  f39a44;               /* 0x39a44 */
    int  f39a48;               /* 0x39a48 - taken off slot 1's count at the end */
    int  f39a4c;
    int  f39a50;               /* 0x39a50 - m425140 takes f39a54 off it */
    int  f39a54;               /* 0x39a54 - the object's last dword */

    InGameSongSelect();                        /* 0x4259c0 */
};

InGameSongSelect *createInGameSongSelect();    /* 0x426310 */

/* TestModeDirector - 0x24 bytes, the smallest class in the family, and the door
   into the operator test menu: its teardown latches two flags on the engine
   object and a global, then logs "Go Test Mode TRUE".

   It has NO member with a destructor: the EH state around the Log is 0, and
   the state is always the index of the next member to destroy, so zero members
   are alive. State 0 exists at all only because the body can throw - the
   region between it and the root's own destructor at state -1. */
/* `ret 0xc`, so THREE arguments. Every page that leaves itself calls it as
   v0(2, page, 0) after finding the live director with
   Obj401780::nextInstance, and arm 1 of @0x4763e0 publishes the menu's
   language row into g_language. */
/* The abstract sink every test-menu page keeps at +0x10 (the pages' own
   derived sinks are in include/systemmenu.h). Deliberately NOT novtable:
   its vftable @0x49ec9c holds `_purecall`, and every page constructor
   stores it before overwriting it with the page's own - which is exactly
   what an ordinary base with a pure virtual does. `ret 0xc`; the SECOND
   argument is the command the menu is reporting. */
struct TestPageSink
{
    virtual void onMenu(int a, int what, int c) = 0;  /* pure; vftable 0x49ec9c */
};

struct __declspec(novtable) TestModeSink : TestPageSink
{
    virtual void onMenu(int what, int item, int c);  /* 0x4763e0 - the menu event sink */
};

struct TestModeDirector : EzObject, TestModeSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x4761c0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x4761d0 */
    virtual void           update();           /* 0x476640 */
    virtual void           update2(int a, int b); /* 0x4765e0 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~TestModeDirector();                       /* 0x4761e0 */
    TestModeDirector();                        /* 0x4766e0 (src/systemmenu.cpp) */

    /* The test menu's OWN page - the seven-entry list of pages plus the
       language row. */
    void buildMenu();                          /* 0x476260 */

    int  f14;                  /* 0x14 - 1 until the menu has been built */
    int  f18;                  /* 0x18 */
    struct SystemMenu *m_menu; /* 0x1c */
    int  f20;                  /* 0x20 - the row the menu opens on */
};

/* Seven members and two sound objects; the same teardown as the result
   directors. Size 0x1438. */
struct TotalResultDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x454460 */
    virtual DrawGroup     *getObjectInfo();    /* 0x454470 */
    virtual void           update();           /* 0x4546e0 (src/totalresultupdate.cpp) */
    virtual void           update2(int a, int b); /* 0x454800 (src/totalresultupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~TotalResultDirector();                    /* 0x4545d0 */

    TotalResultDirector();                     /* 0x454990 (src/totalresultctor.cpp) */
    /* The per-stage song-name trim (the m450c00 family, indexed). */
    char *m4544f0(int stage);                  /* 0x4544f0 (STUB) */
    /* Load a numbered run of ten "<base>N.bmp" into `dst`. */
    int loadDigits(char *base, struct KTexture **dst);  /* 0x454480 */

    Obj402860 m_10;            /* 0x0010 */
    Obj402860 m_22c;           /* 0x022c */
    Obj402860 m_448;           /* 0x0448 */
    Obj402860 m_664;           /* 0x0664 */
    Obj402860 m_880;           /* 0x0880 */
    Obj402860 m_a9c;           /* 0x0a9c */
    Obj40B2E0 m_cb8;           /* 0x0cb8 */
    /* The fader update2 @0x454800 ticks through slot 17 every frame and
       drives through slot 4 in two of its arms. */
    struct ScrFader *fader;    /* 0x1190 */
    char _pad1194[0x1198 - 0x1194];
    EzSoundObject *m_1198;     /* 0x1198 */
    EzSoundObject *m_119c;     /* 0x119c */
    char _pad11a0[0x11a8 - 0x11a0];
    /* The overlay update() @0x4546e0 blits when the phase is 0 or 3. */
    struct KTexture *m_11a8;   /* 0x11a8 */
    char _pad11ac[0x1408 - 0x11ac];
    /* And the banner it draws across the top every frame. */
    struct KTexture *m_1408;   /* 0x1408 */
    char _pad140c[0x142c - 0x140c];
    /* The five-arm state, the fade level it ramps, and the frame counter
       arm 1 waits on - all read off update2 @0x454800. */
    int  f142c;                /* 0x142c */
    int  f1430;                /* 0x1430 */
    int  f1434;                /* 0x1434 */
};

/* One Obj402860 at +0x10, and the same camera teardown as ModeSelectDirector -
   down to logging the SAME line, "ModeSelectDirector::Delete" @0x493fe8.
   Size 0x29c. */
struct DrawModeIntroDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x46a070 */
    virtual DrawGroup     *getObjectInfo();    /* 0x46a080 */
    virtual void           update();           /* 0x46a200 (src/endingupdate2.cpp) */
    virtual void           update2(int, int);  /* 0x46a1a0 (src/endingupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~DrawModeIntroDirector();                  /* 0x46a090 */

    Obj402860 m_10;            /* 0x010 */
    void     *m_sample;        /* 0x22c - "ModeIntro.wav" */
    int       f230;            /* 0x230 */
    int       f234;            /* 0x234 - the ctor leaves it alone */
    struct KTexture *m_video;  /* 0x238 - "Video_Blank_256.bmp" */
    struct KTexture *m_maskTex[12]; /* 0x23c - modetext%02d_m.bmp */
    struct KTexture *m_tex[12];     /* 0x26c - modetext%02d.bmp */

    DrawModeIntroDirector();                   /* 0x46a260 */
};

#endif /* DIRECTORS_H */
