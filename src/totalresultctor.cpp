/* TotalResultDirector's constructor @0x454990 - the course-total screen's
   4.4KB builder (reading: wip/totalresult-ctor.cpp; raw windows in
   scratchpad/tres_ctor.{asm,txt}). 55 setCells + 17 drawLine readouts over
   a per-stage texture set - the mechanical class of ctor.

   The drawLine readouts take a FORMAT STRING ("%04d"/"%07d") loaded from a
   stride-8 pointer table in .rdata @0x496e60 - modeled as one placeholder
   array indexed at the even slots. The per-stage rank values come from
   g_1b2ebf8[] (published by ResultDirector's ctor). */

#include <stdio.h>
#include <string.h>
#include "directors.h"
#include "ez2acapp.h"   /* KGraphics, EzSubsystem, g_sub1b2f054, g_sub1b2eae4 */
#include "bigbase.h"    /* Obj40C180, g_bigInstance */
#include "players.h"    /* ListNode, PlayerSlot */

extern void Log(const char *fmt, ...);             /* 0x402880 (src/applog.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 (src/nodelists.cpp) */
extern int  countListC();                          /* 0x4184a0 (src/nodelists.cpp) */
extern ListNode *findListA(int key);               /* 0x418e30 (src/nodelists.cpp) */
extern int  nextListA(ListNode **cur);             /* 0x4182e0 (src/nodelists.cpp) */
extern "C" unsigned long __stdcall timeGetTime(void);

extern char g_modeName[256];       /* 0x1b2e680 (src/modename.cpp) */
extern int  g_modeIndex;           /* 0x1b2e548 (src/modename.cpp) */
extern char g_channelName[0x80];   /* 0x1b2e728 (src/songselect.cpp) */
extern int  g_elemCount;           /* 0x1b2e718 (src/loops.cpp) */
extern void *g_1b2e844;            /* src/moderunner.cpp */
extern char g_textSlots[32][128];  /* 0x1b2ec18 (src/modename.cpp) */
extern int  g_1b2ebf8[5];          /* src/resultctor.cpp - per-stage rank */
extern int  g_1b2f070[5];          /* src/resultctor.cpp - all-combo flags */
extern int  g_1b2f058[5];          /* src/gamestate.cpp */
extern int  g_1b2f094[5];          /* src/resultctor.cpp */
extern int  g_stageRanks[5];     /* 0x1b2f038 (src/resultctor.cpp) */
extern int  g_1b2efec;             /* src/resultctor.cpp */

/* Obj40B2E0's +0x43c setter (view cast, as in src/modeselectctor.cpp). */
struct Obj40B9E0 { void setField43c(int v); };

/* New globals evidenced here. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea50
int g_stageLevels[5];              /* the LV font each stage is stamped with */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efc0
int g_1b2efc0[4];                  /* per-stage new-record flags */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efd0
int g_1b2efd0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e590
int g_1b2e590[5];                  /* what PlayerSlot::m42ef30 publishes */

/* The rank-art name tables and the readout format-pointer table. Placeholder
   storage pinned at the originals' addresses. */
// GLOBAL: EZ2AC_UNPACKED 0x004ae2ec
char *g_tresRankArt[11];
// GLOBAL: EZ2AC_UNPACKED 0x004ae318
char *g_tresStageArtB[11];
// GLOBAL: EZ2AC_UNPACKED 0x004ae344
char *g_tresStageArtA[11];
// GLOBAL: EZ2AC_UNPACKED 0x00496e60
char *g_tresFmt[22];               /* stride-8 slots; even indices hold fmts */

/* The constructor's view past include/directors.h's model. */
struct TResV
{
    char _pad0[0x1190];
    void *sndBgm;                  /* 0x1190 */
    int  f1194;                    /* 0x1194 */
    void *sndRank;                 /* 0x1198 */
    void *sndDecide;               /* 0x119c */
    struct KTexture *texNot;       /* 0x11a0 */
    struct KTexture *texNotM;      /* 0x11a4 */
    struct KTexture *texFade;      /* 0x11a8 */
    struct KTexture *texRankArt;   /* 0x11ac */
    int  f11b0;                    /* 0x11b0 */
    struct KTexture *stageArt[4];  /* 0x11b4 */
    int  f11c4;                    /* 0x11c4 */
    char _pad11c8[0x1204 - 0x11c8];
    struct KTexture *texSong[5];   /* 0x1204 */
    struct KTexture *texLv[5];     /* 0x1218 */
    struct KTexture *fontA[10];    /* 0x122c */
    struct KTexture *fontB[10];    /* 0x1254 */
    char _pad127c[0x1344 - 0x127c];
    struct KTexture *texChanTag;   /* 0x1344 */
    struct KTexture *lvFont[48];   /* 0x1348 */
    struct KTexture *texGrad;      /* 0x1408 */
    struct KTexture *texDisc[4];   /* 0x140c - [4] can index to the icon slot */
    struct KTexture *texIcon;      /* 0x141c */
    int  f1420;                    /* 0x1420 */
    int  total;                    /* 0x1424 */
    char _pad1428[0x1434 - 0x1428];
    int  f1434;                    /* 0x1434 */
};

/* The deciding sample's slot-4 vcall, as in src/resultctor.cpp. */
struct TResSample
{
    virtual void t0();
    virtual void t1();
    virtual void t2();
    virtual void t3();
    virtual void t4(int a);
};

/* The stage record (src/resultctor.cpp's ResStage): +0x30 holds the GRADE
   this ctor computes. */
struct TResStage
{
    char _pad0[0x30];
    int grade;                     /* 0x30 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00454990
TotalResultDirector::TotalResultDirector()
{
    TResV *v = (TResV *)this;
    Obj402860Head *res;
    char name[64];
    char buf[256];
    int  i;
    int  j;
    int  k;
    int  l;
    char (*slot)[128];
    struct KTexture **lw;
    struct KTexture **pnot;
    Obj402860 *sc;
    Obj402860 *sn;

    g_bigInstance->m4189e0(1);
    if (findListA(0) != 0)
        g_bigInstance->m418a50(1);
    if (findListA(1) != 0)
        g_bigInstance->m418af0(1);

    v->f1434 = 0;
    if (countListA() == 2 || countListC() == 2)
        ((SoundBank *)g_1b2e844)->v0(0);

    for (i = 0, slot = g_textSlots; i <= g_elemCount; i++)
    {
        if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
            g_modeIndex == 9)
        {
            sprintf(name, "system\\disc\\%s.bmp", m4544f0(i));
        }
        else
        {
            sprintf(name, "system\\disc\\%s.bmp", slot);
        }
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &v->texDisc[i], 0);
        sprintf(name, "system\\SongName\\%s.bmp", m4544f0(i));
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &v->texSong[i], 0);
        // STRING: EZ2AC_UNPACKED 0x004971b8
        sprintf(name, "system\\SongSelect\\LvFont\\LV%02d.bmp", g_stageLevels[i]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &v->texLv[i], 0);
        slot++;
    }

    for (j = 0; j < 3; j++)
    {
        // STRING: EZ2AC_UNPACKED 0x004971a0
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\discsmall\\x.bmp",
                                                 &v->texDisc[j], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x0049717c
    sprintf(buf, "System\\result\\ModeIcon\\Icon_%s.bmp", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texIcon, 0);
    pnot = &v->texNot;
    // STRING: EZ2AC_UNPACKED 0x00497158
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\TotalResult\\AllCombo_not.bmp",
                                             pnot, 0);
    // STRING: EZ2AC_UNPACKED 0x00497130
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\TotalResult\\AllCombo_not_m.bmp",
                                             &v->texNotM, 0);
    lw = v->lvFont;
    {
        int n48;
        for (n48 = 0; n48 < 48; n48++)
        {
            // STRING: EZ2AC_UNPACKED 0x00497108
            sprintf(buf, "System\\TotalResult\\LvFont\\lv%02d.bmp", n48);
            ((KGraphics *)g_sub1b2f054)->loadTexture(buf, lw, 0);
            lw++;
        }
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp",
                                             &v->texFade, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\result_grad.bmp",
                                             &v->texGrad, 0);

    if (g_modeIndex == 6)
    {
        // STRING: EZ2AC_UNPACKED 0x004970e4
        sprintf(buf, "System\\Channel_Tag\\5radio\\%s.bmp", g_channelName);
    }
    else if (g_modeIndex == 7)
    {
        // STRING: EZ2AC_UNPACKED 0x004970c4
        sprintf(buf, "System\\Channel_Tag\\radio\\%s.bmp", g_channelName);
    }
    else if (g_modeIndex == 8)
    {
        // STRING: EZ2AC_UNPACKED 0x004970a0
        sprintf(buf, "System\\Channel_Tag\\10radio\\%s.bmp", g_channelName);
    }
    else if (g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x0049707c
        sprintf(buf, "System\\Channel_Tag\\14radio\\%s.bmp", g_channelName);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &v->texChanTag, 0);
    Log("Channel Name : %s", buf);

    // STRING: EZ2AC_UNPACKED 0x00497068
    changeDir("System\\TotalResult", 1);
    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00497040
        sprintf(name, "bg_r.str");
        // STRING: EZ2AC_UNPACKED 0x0049702c
        m_cb8.load((KGraphics *)g_sub1b2f054, "bg\\TRCourse_bg.spv");
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00497060
        sprintf(name, "bg.str");
        // STRING: EZ2AC_UNPACKED 0x0049704c
        m_cb8.load((KGraphics *)g_sub1b2f054, "bg\\TotalResult.spv");
    }
    ((Obj40B9E0 *)&m_cb8)->setField43c(1);
    m_cb8.play(timeGetTime());
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_10.attach(res, 1);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00497008
        sprintf(name, "TotalResult_R.str", g_modeName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049701c
        sprintf(name, "TotalResult.str", g_modeName);
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_22c.attach(res, 0);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496ff4
        sprintf(name, "TotalR_csName_R.str");
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_a9c.attach(res, 0);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496fcc
        sprintf(name, "totalresult_scR.str", g_modeName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496fe0
        sprintf(name, "TotalResult_sc.str", g_modeName);
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_448.attach(res, 0);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496fa0
        sprintf(name, "TotalR_SongName_R.str", g_modeName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496fb8
        sprintf(name, "TotalR_Songname.str", g_modeName);
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_664.attach(res, 0);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        // STRING: EZ2AC_UNPACKED 0x00496f74
        sprintf(name, "TotalR_Allcombo_R.str");
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00496f8c
        sprintf(name, "Total_Allcombo.str");
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_880.attach(res, 0);

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        m_448.f214 = -18.0f;
        m_448.f218 = 0.0f;
        m_664.f214 = -18.0f;
        m_664.f218 = 0.0f;
        m_880.f214 = -18.0f;
        m_880.f218 = 0.0f;
        m_a9c.f214 = -18.0f;
        m_a9c.f218 = 0.0f;
    }

    /* The all-combo cells on the Allcombo clip. */
    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        if (g_1b2f070[0] == 0)
        {
            m_880.setCell(1, 0, (int)v->texNotM);
            m_880.setCell(2, 0, (int)*pnot);
            m_880.setCell(3, 0, (int)*pnot);
        }
        if (g_1b2f070[1] == 0)
        {
            m_880.setCell(4, 0, (int)v->texNotM);
            m_880.setCell(5, 0, (int)*pnot);
            m_880.setCell(6, 0, (int)*pnot);
        }
        if (g_1b2f070[2] == 0)
        {
            m_880.setCell(7, 0, (int)v->texNotM);
            m_880.setCell(8, 0, (int)*pnot);
            m_880.setCell(9, 0, (int)*pnot);
        }
        if (g_1b2f070[3] == 0)
        {
            m_880.setCell(10, 0, (int)v->texNotM);
            m_880.setCell(11, 0, (int)*pnot);
            m_880.setCell(12, 0, (int)*pnot);
        }
    }
    else if (g_modeIndex == 11 || g_modeIndex == 10)
    {
        m_880.setCell(1, 0, (int)v->texNotM);
        m_880.setCell(2, 0, (int)*pnot);
        m_880.setCell(3, 0, (int)*pnot);
        m_880.setCell(4, 0, (int)v->texNotM);
        m_880.setCell(5, 0, (int)*pnot);
        m_880.setCell(6, 0, (int)*pnot);
        m_880.setCell(7, 0, (int)v->texNotM);
        m_880.setCell(8, 0, (int)*pnot);
        m_880.setCell(12, 0, (int)*pnot);
    }
    else
    {
        if (g_1b2f070[0] == 0)
        {
            m_880.setCell(1, 0, (int)v->texNotM);
            m_880.setCell(2, 0, (int)*pnot);
            m_880.setCell(3, 0, (int)*pnot);
        }
        if (g_1b2f070[1] == 0)
        {
            m_880.setCell(4, 0, (int)v->texNotM);
            m_880.setCell(5, 0, (int)*pnot);
            m_880.setCell(6, 0, (int)*pnot);
        }
        if (g_1b2f070[2] == 0)
        {
            m_880.setCell(7, 0, (int)v->texNotM);
            m_880.setCell(8, 0, (int)*pnot);
            m_880.setCell(12, 0, (int)*pnot);
        }
    }

    if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        m_22c.setCell(7, 0, (int)v->texIcon);
        m_a9c.setCell(1, 0, (int)v->texChanTag);
        m_a9c.setCell(2, 0, (int)v->lvFont[g_1b2efd0]);
    }

    // STRING: EZ2AC_UNPACKED 0x00496f60
    loadDigits("font\\BigFnt_", v->fontA);
    // STRING: EZ2AC_UNPACKED 0x00496f50
    loadDigits("font\\SmlFnt_", v->fontB);

    /* The grade. */
    {
        TResStage *rec;
        float pct;
        int r10;
        int w0 = 0;
        ListNode *cur;

        cur = 0;
        nextListA(&cur);
        v->total = ((PlayerSlot *)cur)->m42ef30();
        rec = (TResStage *)((char *)cur + 0x230 + (g_elemCount << 6));
        if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
            g_modeIndex == 9)
        {
            pct = (float)(g_1b2ebf8[0] + g_1b2ebf8[1] + g_1b2ebf8[2] + g_1b2ebf8[3]) * 0.025f;
        }
        else
        {
            pct = (float)(g_1b2ebf8[0] + g_1b2ebf8[1] + g_1b2ebf8[2]) * 0.0333333351f;
        }
        r10 = (int)(pct * 10.0f);

        if (pct >= 100.0f)
            rec->grade = 10;
        else if (pct >= 98.0f)
            rec->grade = 9;
        else if (pct >= 95.0f)
            rec->grade = 8;
        else if (pct >= 93.0f)
            rec->grade = 7;
        else if (pct >= 90.0f)
            rec->grade = 6;
        else if (pct >= 85.0f)
            rec->grade = 5;
        else if (pct >= 80.0f)
            rec->grade = 4;
        else if (pct >= 70.0f)
            rec->grade = 3;
        else if (pct >= 60.0f)
            rec->grade = 2;
        else if (pct >= 50.0f)
            rec->grade = 1;
        else
            rec->grade = 0;

        ((KGraphics *)g_sub1b2f054)->loadTexture(g_tresRankArt[rec->grade],
                                                 &v->texRankArt, 0);

        /* The per-stage result art. */
        if (g_modeIndex == 11 || g_modeIndex == 10)
        {
            for (l = 0; l < 3; l++)
            {
                ((KGraphics *)g_sub1b2f054)->loadTexture(g_tresStageArtB[g_stageRanks[l]],
                                                         &v->stageArt[l], 0);
            }
        }
        else if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
                 g_modeIndex == 9)
        {
            for (i = 0; i < 4; i++)
            {
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    g_1b2efc0[i] == 1 ? g_tresStageArtA[g_stageRanks[i]]
                                      : g_tresStageArtB[g_stageRanks[i]],
                    &v->stageArt[i], 0);
                // STRING: EZ2AC_UNPACKED 0x00496f34
                Log("m_nRank Value = %d", g_stageRanks[i]);
            }
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                if (g_1b2efc0[i] == 1)
                    ((KGraphics *)g_sub1b2f054)->loadTexture(g_tresStageArtA[g_stageRanks[i]],
                                                             &v->stageArt[i], 0);
                else
                    ((KGraphics *)g_sub1b2f054)->loadTexture(g_tresStageArtB[g_stageRanks[i]],
                                                             &v->stageArt[i], 0);
            }
        }

        /* The seventeen readouts - the score clip held in a named pointer
           (edi), which also carries into the final cell runs. */
        sc = &m_448;
        drawLine(sc, 9, (struct KTexture *)g_1b2ebf8[0], (int)g_tresFmt[8], v->fontB);
        drawLine(sc, 0xd, (struct KTexture *)g_1b2ebf8[1], (int)g_tresFmt[10], v->fontB);
        drawLine(sc, 0x11, (struct KTexture *)g_1b2ebf8[2], (int)g_tresFmt[12], v->fontB);
        if (g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 ||
            g_modeIndex == 9)
        {
            drawLine(sc, 0x3b, (struct KTexture *)g_1b2ebf8[3], (int)g_tresFmt[14], v->fontB);
        }
        if (g_modeIndex == 10)
        {
            drawLine(sc, 0x15, (struct KTexture *)g_1b2f058[0], (int)g_tresFmt[0], v->fontB);
            drawLine(sc, 0x1c, (struct KTexture *)g_1b2f058[1], (int)g_tresFmt[2], v->fontB);
            drawLine(sc, 0x23, (struct KTexture *)g_1b2f058[2], (int)g_tresFmt[4], v->fontB);
        }
        else if (g_modeIndex == 11)
        {
            drawLine(sc, 0x15, (struct KTexture *)g_1b2f094[0], (int)g_tresFmt[0], v->fontB);
            drawLine(sc, 0x1c, (struct KTexture *)g_1b2f094[1], (int)g_tresFmt[2], v->fontB);
            drawLine(sc, 0x23, (struct KTexture *)g_1b2f094[2], (int)g_tresFmt[4], v->fontB);
        }
        else if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
                 g_modeIndex != 9)
        {
            drawLine(sc, 0x15, (struct KTexture *)g_1b2e590[0], (int)g_tresFmt[0], v->fontB);
            drawLine(sc, 0x1c, (struct KTexture *)g_1b2e590[1], (int)g_tresFmt[2], v->fontB);
            drawLine(sc, 0x23, (struct KTexture *)g_1b2e590[2], (int)g_tresFmt[4], v->fontB);
        }
        else
        {
            drawLine(sc, 0x15, (struct KTexture *)g_1b2e590[0], (int)g_tresFmt[0], v->fontB);
            drawLine(sc, 0x1c, (struct KTexture *)g_1b2e590[1], (int)g_tresFmt[2], v->fontB);
            drawLine(sc, 0x23, (struct KTexture *)g_1b2e590[2], (int)g_tresFmt[4], v->fontB);
            drawLine(sc, 0x3f, (struct KTexture *)g_1b2e590[3], (int)g_tresFmt[6], v->fontB);
        }
        drawLine(sc, 0x2a, (struct KTexture *)r10, (int)g_tresFmt[16], v->fontA);
        if (g_modeIndex == 10)
        {
            drawLine(sc, 0x2e,
                     (struct KTexture *)(g_1b2f058[2] + g_1b2f058[1] + g_1b2f058[0]),
                     (int)g_tresFmt[18], v->fontA);
        }
        else if (g_modeIndex == 11)
        {
            drawLine(sc, 0x2e,
                     (struct KTexture *)(g_1b2f094[1] + g_1b2f094[2] + g_1b2f094[0]),
                     (int)g_tresFmt[18], v->fontA);
        }
        else
        {
            drawLine(sc, 0x2e, (struct KTexture *)v->total, (int)g_tresFmt[18], v->fontA);
        }
    }

    /* The final cell runs - the plain arm is the fall-through (the negated
       test), the radio arm out of line with its 9999-clamped bonus row. */
    if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        sc->setCell(0x38, 0, (int)v->texRankArt);
        sc->setCell(0x35, 0, (int)v->stageArt[0]);
        sc->setCell(0x36, 0, (int)v->stageArt[1]);
        sc->setCell(0x37, 0, (int)v->stageArt[2]);
        sn = &m_664;
        sn->setCell(1, 0, (int)v->texSong[0]);
        sn->setCell(2, 0, (int)v->texSong[1]);
        sn->setCell(3, 0, (int)v->texSong[2]);
        sn->setCell(4, 0, (int)v->texDisc[0]);
        sn->setCell(5, 0, (int)v->texDisc[1]);
        sn->setCell(6, 0, (int)v->texDisc[2]);
        sn->setCell(7, 0, (int)v->texLv[0]);
        sn->setCell(8, 0, (int)v->texLv[1]);
        sn->setCell(9, 0, (int)v->texLv[2]);
    }
    else
    {
        sc->setCell(0x38, 0, (int)v->texRankArt);
        sc->setCell(0x35, 0, (int)v->stageArt[0]);
        sc->setCell(0x36, 0, (int)v->stageArt[1]);
        sc->setCell(0x37, 0, (int)v->stageArt[2]);
        sc->setCell(0x46, 0, (int)v->stageArt[3]);
        sn = &m_664;
        sn->setCell(1, 0, (int)v->texSong[0]);
        sn->setCell(2, 0, (int)v->texSong[1]);
        sn->setCell(3, 0, (int)v->texSong[2]);
        sn->setCell(4, 0, (int)v->texSong[3]);
        sn->setCell(5, 0, (int)v->texDisc[0]);
        sn->setCell(6, 0, (int)v->texDisc[1]);
        sn->setCell(7, 0, (int)v->texDisc[2]);
        sn->setCell(8, 0, (int)v->texDisc[3]);
        {
            int t = g_1b2efec;
            if (t >= 9999)
                g_1b2efec = 9999;
            drawLine(sc, 0x47, (struct KTexture *)g_1b2efec, (int)g_tresFmt[20], v->fontB);
        }
    }

    // STRING: EZ2AC_UNPACKED 0x00496f20
    v->sndBgm = (void *)g_sub1b2eae4->createSample("TotalResult_bgm.wav", 1, 0);
    ((TResSample *)v->sndBgm)->t4(0);
    // STRING: EZ2AC_UNPACKED 0x00496f14
    v->sndRank = (void *)g_sub1b2eae4->createSample("Rank.wav", 0, 0);
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x00496ef4
    v->sndDecide = (void *)g_sub1b2eae4->createSample("system\\modeselect\\decide.wav", 0, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp",
                                             &v->texFade, 0);
}
