/* GFResultDirector::GFResultDirector @0x4682e0
   TRANSCRIPTION (5,776 bytes). The class model in include/directors.h
   matches the EH member sequence 1:1, so the member construction is
   implicit; this is the body. All 95 string literals are resolved (see
   the git history of this file). Regenerate the listing with objdump
   0x4682e0..0x4699a0 + decode.py.

   DONE: the head through the difficulty icons (decode lines 1-190).
   TODO: the effector-icon run (the ~30 EFFECTOR_*.bmp loads), the
   panel picks (Battle/CatchResult/Result by mode), the songrank pair
   (1p/2p, _c variants), the stage/channel tags, the samples
   (result_bgm, count, bonus, NextStage...), and the trailing
   defaults. */


#include <stdio.h>
#include "directors.h"
#include "ez2acapp.h"
#include "bigbase.h"
#include "players.h"     /* ListNode, nextListC, findPlayer */

extern EzSubsystem *g_sub1b2f054;   /* 0x1b2f054 */
extern EzSubsystem *g_sub1b2eae4;   /* 0x1b2eae4 */
extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 */
extern int countListC();            /* 0x4184a0 */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
/* Redeclared minus dllimport so the call is the original's 5-byte thunk. */
extern "C" unsigned long __stdcall timeGetTime();

extern int  g_modeIndex;            /* 0x1b2e548 (src/modename.cpp) */
extern char g_modeName[256];        /* 0x1b2e680 (src/modename.cpp) */
extern char g_channelName[0x80];    /* 0x1b2e728 (src/songselect.cpp) */
extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */
extern int  g_stageNumber;          /* 0x1b2ec0c (src/directors.cpp) */
extern int  g_elemCount;            /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2ebb8;              /* the published difficulty */
extern int  g_1b2ea28;              /* the game-over flag */
extern int  g_1b2e7d8;              /* src/gamestate.cpp */
extern int  g_1b2ebdc;              /* src/gamestate.cpp */
extern int  g_1b2e704;              /* the 1P layout's pair-5 stat */
extern int  g_1b2ebc0;              /* the hit percent */
extern int  g_stageRanks[5];      /* 0x1b2f038 (src/resultctor.cpp) */
extern int  g_1b2f070[];            /* per-count full-combo */
extern int  g_1b2ebf8[];            /* per-count score permille */
extern int  g_eventMode;            /* 0x1696404 (src/settings.cpp) */

/* The .data tables beside the position pairs (src/directors.cpp). */
extern GfrLine g_gfrPos[2][10];     /* 0x49b0e8 */
extern int   g_rankRow[2];          /* 0x49b1d8 */
extern int   g_rankRowSmall[2];     /* 0x49b1e0 */
extern char *g_clearMark[5];        /* 0x49b1e8 */
/* The rank/clear texture-path pointer tables (src/globals.cpp). */
extern char *g_rankTexBig[11];      /* 0x4ae7c0 */
extern char *g_rankTexSmall[11];    /* 0x4ae7ec */
extern char *g_clearPlate[5];       /* 0x4ae818 */
extern char *g_clearPlateM[5];      /* 0x4ae82c */

/* The sample-object view (same shape as modeselectupdate2's). */
struct MsSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
    virtual void stop();             /* +0x14 */
};

/* File-local views defined elsewhere; redeclared for the calls. */
struct Obj467930 { void rankPair(); };            /* src/accessors.cpp */
struct Obj466660 { int sumElems(); };             /* src/loops.cpp */
struct Obj4665F0 { int totalWidth(); };           /* src/accessors.cpp */
struct Obj40B9E0 { void setField43c(int v); };    /* src/com_wrappers.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x004682e0
GFResultDirector::GFResultDirector()
{
    char buf2[0x40];     /* base+0x20 - the verdict / panel-script names */
    char name[0x20];     /* base+0x60 - the PLACE loop */
    char buf[260];       /* base+0x80 - the main asset path */
    char intro[260];     /* base+0x184 - the ModeIntro pair */
    Obj402860Head *res;
    ListNode *cur;
    Obj40B2E0 *vid;
    KTexture **t;
    int  i;

    vid = &m_2438;
    g_bigInstance->m4189e0(1);
    if (findPlayer(0) != 0)
        g_bigInstance->m418a50(1);
    if (findPlayer(1) != 0)
        g_bigInstance->m418af0(1);

    f2918 = 0;
    f10 = 0;
    f2928 = 0;
    f14 = 0;
    f18 = 0;
    f2db8 = 0x9b;
    f2dbc = 0x12c;
    f2dc0 = 0x640;
    f2dc4 = 0x19;
    f2dc8 = 0x708;

    /* The song-name draw script: _R for the keys modes, _C for
       RadioMix, _b for a single course list, plain otherwise. */
    if (g_modeIndex == 7 || g_modeIndex == 6 || g_modeIndex == 8 ||
        g_modeIndex == 9)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            "system\\RSname\\RSongDraw_R.str", &res);
    }
    else if (g_modeIndex == 0xa)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            // STRING: EZ2AC_UNPACKED 0x0049b374
            "system\\RSname\\RSongDraw_C.str", &res);
    }
    else if (countListC() == 1)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            "system\\RSname\\RSongDraw.str", &res);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadStr(
            "system\\RSname\\RSongDraw_b.str", &res);
    }
    m_ee0.attach(res, 1);

    sprintf(buf, "System\\SongSelect\\lvfont\\LV%02d.bmp", g_stageNumber);
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_23e0, 0);
    sprintf(buf, "system\\SongName\\%s.bmp", m4673a0());
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_23e4, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b348
    sprintf(intro, "System\\ModeSelect\\inout\\ModeIntro_%s.bmp", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(intro, &m_233c, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b318
    sprintf(intro, "System\\ModeSelect\\inout\\ModeIntro_%s_Mask.bmp",
            g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(intro, &m_2340, 0);
    sprintf(buf, "System\\Result\\ModeIcon\\Icon_%s.bmp", g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_2344, 0);
    ((KGraphics *)g_sub1b2f054)->loadStr(
        "System\\Result\\Stage\\Result_Stage.str", &res);
    m_1da4.attach(res, 0);
    sprintf(buf, "system\\ResultDisc\\%s.bmp", g_textSlots[g_elemCount]);
    t = &m_23ec;
    ((KGraphics *)g_sub1b2f054)->loadTexture(buf, t, 0);

    if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            "System\\Result\\Diff_Normal.bmp", &m_23c0, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            "System\\Result\\Diff_Hard.bmp", &m_23c4, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            "System\\Result\\Diff_SHard.bmp", &m_23c8, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture(
            "System\\Result\\Diff_Ex.bmp", &m_23cc, 0);
    }
    else
    {
        /* The keys modes load the channel tag instead - one directory
           per radio flavour, 5Radio for mode 6. */
        if (g_modeIndex == 6)
        {
            // STRING: EZ2AC_UNPACKED 0x004966bc
            sprintf(buf, "System\\Channel_Tag\\5Radio\\%s.bmp",
                    g_channelName);
        }
        else if (g_modeIndex == 8)
        {
            sprintf(buf, "System\\Channel_Tag\\10Radio\\%s.bmp",
                    g_channelName);
        }
        else if (g_modeIndex == 9)
        {
            sprintf(buf, "System\\Channel_Tag\\14Radio\\%s.bmp",
                    g_channelName);
        }
        else if (g_modeIndex == 7)
        {
            sprintf(buf, "System\\Channel_Tag\\Radio\\%s.bmp",
                    g_channelName);
        }
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_2338, 0);
    }

    m_ee0.setCell(1, 0, (int)m_23e4);
    m_ee0.setCell(2, 0, (int)m_23e0);
    m_ee0.setCell(3, 0, (int)*t);

    for (i = 0; i < 4; i++)
    {
        sprintf(buf, "System\\Result\\Stage\\TT_stage_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &f23d0[i], 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "system\\Result\\TT_Result_Top_m.bmp", &m_232c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "system\\Result\\TT_Result_Top_c.bmp", (KTexture **)&f2330, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "system\\Result\\TT_Result_Top_f.bmp", (KTexture **)&f2334, 0);

    /* Row 4 = the difficulty icon (indexed by the published
       difficulty) or the channel tag; the stage chart's row 1 by the
       player count, with the dedicated 2P plate. */
    if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        m_ee0.setCell(4, 0, (int)(&m_23bc)[g_1b2ebb8]);
        if (g_elemCount == 2)
            m_1da4.setCell(1, 0, (int)f23d0[3]);
        else
            m_1da4.setCell(1, 0, (int)f23d0[g_elemCount]);
    }
    else
    {
        m_ee0.setCell(4, 0, (int)m_2338);
        m_1da4.setCell(1, 0, (int)f23d0[g_elemCount]);
    }

    /* The thirty effector icons, in the original's load order. */
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_RANDOM_OFF.bmp", &m_234c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_RANDOM.bmp", &m_2350, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SRANDOM.bmp", &m_2354, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_PS.bmp", &m_2358, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FR.bmp", &m_235c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MRANDOM.bmp", &m_2360, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_HRANDOM.bmp", &m_2364, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MIRROR.bmp", &m_2368, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_MIRROR_A.bmp", &m_236c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_KEY.bmp", &m_2370, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SP.bmp", &m_2374, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_4D_OFF.bmp", &m_2398, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_4D_on.bmp", &m_239c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_5D_on.bmp", &m_23a0, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\Effector_Reverse_on.bmp", &m_23bc, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SPEDAL_OFF.bmp", &m_2378, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SCRATCH_ON.bmp", &m_237c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_PEDAL_ON.bmp", &m_2380, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTO_SPEDAL_ON.bmp", &m_2384, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SWITCH_OFF.bmp", &m_23b4, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_SWITCH_ON.bmp", &m_23b8, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_off.bmp", &m_2388, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_L.bmp", &m_238c, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_R.bmp", &m_2390, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_AUTOS_on.bmp", &m_2394, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEOUT_OFF.bmp", &m_23a4, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEOUT.bmp", &m_23a8, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_FADEIN.bmp", &m_23ac, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Result\\Effector\\EFFECTOR_BLINK.bmp", &m_23b0, 0);

    changeDir("System\\Result", 1);

    /* The background pick: BG/coursebg with the matching .spv into the
       video player while the game is alive, bg_GameOver after. The
       alive paths attach the script AGAIN through the shared tail -
       the original's bytes jump the video quad back into the game-over
       path's attach. */
    if (countListC() == 1)
    {
        if (g_1b2ea28 == 0)
        {
            if (g_modeIndex != 6 && g_modeIndex != 7 &&
                g_modeIndex != 8 && g_modeIndex != 9)
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("BG.str", &res);
                m_cc4.attach(res, 1);
                vid->load((KGraphics *)g_sub1b2f054,
                            "bg\\Result_bg.spv");
                ((Obj40B9E0 *)vid)->setField43c(1);
                vid->play(timeGetTime());
            }
            else
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("coursebg.str",
                                                     &res);
                m_cc4.attach(res, 1);
                vid->load((KGraphics *)g_sub1b2f054,
                            // STRING: EZ2AC_UNPACKED 0x0049b2a4
                            "bg\\Course_BG.spv");
                ((Obj40B9E0 *)vid)->setField43c(1);
                vid->play(timeGetTime());
            }
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("bg_GameOver.str",
                                                 &res);
        }
        m_cc4.attach(res, 1);
    }
    else
    {
        /* Two players: the game-over gate first, then the same BG /
           coursebg pick, with the matching .spv into the video. */
        if (g_1b2e7d8 >= 2)
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("bg_GameOver.str",
                                                 &res);
            m_cc4.attach(res, 1);
            vid->load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
        }
        else if (g_modeIndex != 6 && g_modeIndex != 7 &&
                 g_modeIndex != 8 && g_modeIndex != 9)
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("BG.str", &res);
            m_cc4.attach(res, 1);
            vid->load((KGraphics *)g_sub1b2f054, "bg\\Result_bg.spv");
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("coursebg.str", &res);
            m_cc4.attach(res, 1);
            vid->load((KGraphics *)g_sub1b2f054, "bg\\Course_BG.spv");
        }
        ((Obj40B9E0 *)vid)->setField43c(1);
        vid->play(timeGetTime());
    }

    /* The effector strip: the battle variant with two players. */
    if (countListC() == 2)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("Result_Eff_B.str", &res);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("Result_Eff.str", &res);
    }
    m_88c.attach(res, 0);

    if (countListC() == 2)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\1p_Allcombo.str",
                                             &res);
        m_196c.attach(res, 0);
        ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\2p_Allcombo.str",
                                             &res);
        m_1b88.attach(res, 0);
    }

    /* The result panel: catch / R (keys) / plain, or the battle panel
       with two players; the mode icon lands in its row 2. */
    if (countListC() == 1)
    {
        if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
            g_modeIndex != 9)
        {
            if (g_modeIndex == 0xa)
            {
                ((KGraphics *)g_sub1b2f054)->loadStr(
                    "CatchResult_Panel.str", &res);
            }
            else
            {
                ((KGraphics *)g_sub1b2f054)->loadStr("Result_Panel.str",
                                                     &res);
            }
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("Result_PanelR.str",
                                                 &res);
        }
        m_aa8.attach(res, 0);
        m_ee0.setCell(2, 0, (int)m_2344);
    }
    else if (countListC() == 2)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("Battle\\Battle_Panel.str",
                                             &res);
        m_aa8.attach(res, 0);
        m_ee0.setCell(2, 0, (int)m_2344);
    }

    /* The two songrank charts (the _c course variants in RadioMix). */
    if (g_modeIndex == 0xa)
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("1psongrank_c.str", &res);
        m_295c[0].attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2psongrank_c.str", &res);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->loadStr("1psongrank.str", &res);
        m_295c[0].attach(res, 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("2psongrank.str", &res);
    }
    m_295c[1].attach(res, 1);

    /* The five PLACE plates. */
    for (i = 0; i < 5; i++)
    {
        sprintf(name, "PLACE_%u.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &f2d94[i], 0);
    }

    /* The digit fonts, through TotalResultDirector's loader: the
       Battle set with two players; the third slot's face flips to the
       old total font under g_1b2ebdc. */
    if (countListC() == 1)
    {
        loadDigits("Font\\SmlFnt_", m_2264);
        loadDigits("Font\\SmlFnt_", m_22dc);
        if (g_1b2ebdc == 1)
        {
            loadDigits("totasf_old_", m_228c);
        }
        else
        {
            loadDigits("Font\\BigFnt_", m_228c);
        }
    }
    else if (countListC() == 2)
    {
        loadDigits("Battle\\SmlFnt_", m_2264);
        loadDigits("Battle\\SmlFnt_", m_22dc);
        loadDigits("Battle\\BigFnt_", m_228c);
        loadDigits("Battle\\ntf_", m_22b4);
        loadDigits("Battle\\BigFnt_", m_2304);
    }

    /* The battle verdict: rankPair fills winner/loser; the message
       script by who won. */
    loser = -1;
    winner = -1;
    if (countListC() > 1)
    {
        ((Obj467930 *)this)->rankPair();
        if (winner == 1)
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("battle_result_2p.str",
                                                 &res);
        }
        else if (winner == 0)
        {
            ((KGraphics *)g_sub1b2f054)->loadStr("battle_result_1p.str",
                                                 &res);
        }
        else if (winner == -1)
        {
            ((KGraphics *)g_sub1b2f054)->loadStr(
                "battle_result_draw.str", &res);
        }
        m_10fc.attach(res, 0);
    }

    /* The per-player pass: the side's panel script, the rank plates,
       the clear marker, the score sums, and the stat readout lines. */
    cur = 0;
    if (nextListC(&cur) != 0)
    {
        do
        {
            int *st;
            int  pct10;

            if (countListC() == 1)
            {
                sprintf(buf2, "result_1p.str");
                ((KGraphics *)g_sub1b2f054)->loadStr(buf2, &res);
                m_1c[((int *)cur)[2]].attach(res, 0);
            }
            else if (countListC() == 2)
            {
                sprintf(buf2, "Battle\\Battle_%dp.str", ((int *)cur)[2] + 1);
                ((KGraphics *)g_sub1b2f054)->loadStr(buf2, &res);
                m_1c[((int *)cur)[2]].attach(res, 0);
            }

            /* The per-elemCount stat block inside the node. */
            st = (int *)((char *)cur + g_elemCount * 0x40 + 0x5c);

            ((KGraphics *)g_sub1b2f054)->loadTexture(
                g_rankTexBig[st[12]], &f2244[((int *)cur)[2]], 0);
            ((KGraphics *)g_sub1b2f054)->loadTexture(
                g_rankTexSmall[st[12]], &f224c[((int *)cur)[2]], 0);
            g_stageRanks[g_elemCount] = st[12];

            /* The clear marker: every note judged means a full pass. */
            {
                int miss;

                miss = st[5] + st[4] + st[6] - st[0];
                miss = (miss != 0) ? -2 : 0;
                f242c[((int *)cur)[2]] = (int)g_clearMark[miss + 2];
                st[10] = f242c[((int *)cur)[2]];
                m_2934[((int *)cur)[2]] = (KTexture *)st[9];
                miss += 2;

                if (winner == -1 && loser != -2)
                {
                    int v = ((Obj466660 *)cur)->sumElems();
                    m_scoreLatch[((int *)cur)[2]] = (v > 0) ? ((Obj466660 *)cur)->sumElems() : 0;
                    m_score[((int *)cur)[2]] = ((Obj4665F0 *)cur)->totalWidth();
                    ((int *)cur)[0xd] = m_score[((int *)cur)[2]];
                }
                else
                {
                    m_scoreLatch[((int *)cur)[2]] = ((int *)cur)[0xd];
                    m_score[((int *)cur)[2]] = ((int *)cur)[0xd];
                }
                m_scoreStep[((int *)cur)[2]] = st[9] / 40;
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    g_clearPlate[miss], &f2254[((int *)cur)[2]], 0);
                ((KGraphics *)g_sub1b2f054)->loadTexture(
                    g_clearPlateM[miss], &f225c[((int *)cur)[2]], 0);
            }

            /* The percents: the hit rate spilled for the tail, and the
               score rate published per side. */
            {
                float hitPct;

                hitPct = (float)st[5] / (float)st[0] * 100.0f;
                pct10 = (int)(((float)st[9] / (float)(st[0] * 300) *
                               100.0f) * 10.0f);
                g_1b2ebf8[g_elemCount] = pct10;

                if (countListC() == 1)
                {
                    if (st[8] == st[0])
                    {
                        f10 = 1;
                        g_1b2f070[g_elemCount] = 1;
                    }
                    else
                    {
                        f10 = 0;
                        g_1b2f070[g_elemCount] = 0;
                    }
                }
                else if (st[0] == st[8])
                    (&f14)[((int *)cur)[2]] = 1;

                /* The stat readout lines: pairs 0..7 of the position
                   table, then the two big-font lines. Pair 5 reads the
                   global 0x1b2e704 in the 1P layout and the node's own
                   +0x8 word in battle. */
                                if (countListC() == 1)
                {
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][0].f4, (KTexture *)st[0],
                             g_gfrPos[((int *)cur)[2]][0].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][1].f4, (KTexture *)st[6],
                             g_gfrPos[((int *)cur)[2]][1].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][2].f4, (KTexture *)st[5],
                             g_gfrPos[((int *)cur)[2]][2].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][3].f4, (KTexture *)st[4],
                             g_gfrPos[((int *)cur)[2]][3].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][4].f4, (KTexture *)st[3],
                             g_gfrPos[((int *)cur)[2]][4].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][5].f4,
                             (KTexture *)g_1b2e704, g_gfrPos[((int *)cur)[2]][5].f0, m_2264);
                }
                else if (countListC() == 2)
                {
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][0].f4, (KTexture *)st[0],
                             g_gfrPos[((int *)cur)[2]][0].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][1].f4, (KTexture *)st[6],
                             g_gfrPos[((int *)cur)[2]][1].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][2].f4, (KTexture *)st[5],
                             g_gfrPos[((int *)cur)[2]][2].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][3].f4, (KTexture *)st[4],
                             g_gfrPos[((int *)cur)[2]][3].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][4].f4, (KTexture *)st[3],
                             g_gfrPos[((int *)cur)[2]][4].f0, m_2264);
                    drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][5].f4, (KTexture *)st[2],
                             g_gfrPos[((int *)cur)[2]][5].f0, m_2264);
                }
                else
                    goto skipLines;
                drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][6].f4, (KTexture *)st[7],
                         g_gfrPos[((int *)cur)[2]][6].f0, m_2264);
                drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][7].f4, (KTexture *)pct10,
                         g_gfrPos[((int *)cur)[2]][7].f0, m_228c);
                drawLine(&m_1c[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][8].f4,
                         (KTexture *)m_2934[((int *)cur)[2]], g_gfrPos[((int *)cur)[2]][8].f0, m_228c);
skipLines:
                m_1c[((int *)cur)[2]].setCell(g_rankRow[((int *)cur)[2]], 0, (int)f2244[((int *)cur)[2]]);
                m_1c[((int *)cur)[2]].setCell(g_rankRowSmall[((int *)cur)[2]], 0, (int)f224c[((int *)cur)[2]]);
                g_1b2ebc0 = (int)hitPct;
            }

            ((KGraphics *)g_sub1b2f054)->loadStr("AllCombo.str", &res);
            m_1534.attach(res, 1);
        }
        while (nextListC(&cur) != 0);
    }

    /* The samples. */
    m_21dc = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x0049b294
        "result_bgm.wav", 1, 0);
    if (countListC() == 1 && g_1b2ea28 == 0)
        ((MsSnd *)m_21dc)->play(0);
    if (countListC() == 2 && g_1b2e7d8 != 1)
        ((MsSnd *)m_21dc)->play(0);
    m_21e0 = (EzSoundObject *)g_sub1b2eae4->createSample("Rank.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b284
    m_21e4 = (EzSoundObject *)g_sub1b2eae4->createSample("Rank_Fail.wav",
                                                         0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b278
    m_21f4 = (EzSoundObject *)g_sub1b2eae4->createSample("count.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b26c
    m_21e8 = (EzSoundObject *)g_sub1b2eae4->createSample("bonus.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b25c
    m_21f0 = (EzSoundObject *)g_sub1b2eae4->createSample("fNextStage.wav",
                                                         0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049b24c
    m_21ec = (EzSoundObject *)g_sub1b2eae4->createSample("NextStage.wav",
                                                         0, 0);
    m_21f8 = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x0049b23c
        "SetBonus.wav", 0, 0);
    m_21fc = (EzSoundObject *)g_sub1b2eae4->createSample(
        // STRING: EZ2AC_UNPACKED 0x0049b228
        "BattleMessage.wav", 0, 0);
    popDirectory();
    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "system\\modeselect\\fadeblack.bmp", &m_2240, 0);
    m_state[0] = 5;
    m_state[1] = 5;
    if (g_eventMode == 0)
        m466ee0();
}



