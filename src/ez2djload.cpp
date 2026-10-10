/* EZ2DJMainGameDirector::m423a90 @0x423a90 - the EZ2DJ chart loader, written
   as an EDIT of its twin src/catchload.cpp (@0x46dba0, 87.14%), which shares
   73.9% of this instruction sequence. Read that file's header first: it
   explains the class-registry calls, why `step` is volatile, and the frame.
   The delta map this was written from is wip/ez2dj-chartload-423a90.md.

   What is different here, beyond the field and callee substitutions:

   * a prologue gate that hands the course result to the first player slot
     when the mode is 5, two elements are up and only one player is listed;
   * the chart name is built from the MODE NAME rather than a fixed prefix,
     with a second arm under mix style 6, and the song is then compared
     against "7bc-fm1" to set g_1b2eac8 - the Blood Castle perc-mode check;
   * the ticks-per-measure value is logged as it is stored;
   * after the lane loop, three key ranges are chained on the first listed
     node under two mode tests, and the chart is committed;
   * the published value goes into BOTH listed nodes at +0x3b0, not into a
     global array.

   85.25%. The original holds 0 in ebx and 1 in ebp for the whole function
   (`cmp [g],ebx`, `push ebp`, `add ebx,ebp`, `mov eax,ebp` for the return)
   and cl spells most of those as immediates - that is the constant-register
   wall in CLAUDE.md, and it is most of the residue. The rest is the same
   stack-packer permutation of the four header slots that parks the twin.

   Both are structural, so do not spend a session here; what IS worth having
   is that this file and src/catchload.cpp now pin the whole chart-load path
   for two of the three modes. */

#include <stdio.h>
#include "ezscreen.h"
#include "judge.h"
#include "players.h"
#include "ezchart.h"
#include "obj414750.h"

/* The chart the registry hands back, under the slots this function uses.
   It is the same object the director keeps as `sink`; only the slot shape
   matters here, so it is viewed rather than re-declared. */
struct ChartSink
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    /* slot 6 -> +0x18: re-arm one already-claimed lane. */
    virtual void rearm(int a, int b);
    virtual void q07();
    /* slot 8 -> +0x20: read a block of the chart header. */
    virtual void read(void *out, int n);
    /* slot 9 -> +0x24: and its sibling, which reads the other one. */
    virtual void read2(void *out, int n);
    virtual void q10(); virtual void q11(); virtual void q12();
    virtual void q13(); virtual void q14(); virtual void q15();
    virtual void q16(); virtual void q17(); virtual void q18();
    virtual void q19(); virtual void q20(); virtual void q21();
    virtual void q22();
    /* slot 23 -> +0x5c: run once the batch of lane work is finished. */
    virtual void commit();
};

/* The player the registry binds to the chart - only its slot 29 is used,
   so the dummies before it are what place it at +0x74. */
struct ChartPlayer
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09(); virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21(); virtual void q22(); virtual void q23();
    virtual void q24(); virtual void q25(); virtual void q26();
    virtual void q27(); virtual void q28();
    virtual void setMode(int m);   /* slot 29 -> +0x74 */
};

/* The class registry: given a name and a class record it constructs the
   object. Slot 7 additionally binds the result to an owner. */
struct EzClassRegistry
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual ChartSink   *create(char *name, void *cls, void *owner);
    virtual void q05(); virtual void q06();
    virtual ChartPlayer *bind(ChartSink *chart, void *cls, void *owner);
};

extern EzSubsystem *g_sub1b2eae4;      /* src/ez2acapp.cpp */
extern EzSubsystem *g_sub1b2ef98;      /* src/ez2acapp.cpp - the registry */
extern EzClassInfo  g_ezChartClassInfo;/* 0x4a7370 (src/ezchart.cpp) */
extern KRuntimeClass classKEZPlayer;   /* 0x4a733c (src/globals.cpp) */

extern char g_textSlots[32][128];      /* 0x1b2ec18 (src/modename.cpp) */
extern char g_songInfoName[64];        /* 0x1b2eb80 (src/panels.cpp) */
extern char g_songShortName[0x40];     /* 0x1b2f004 (src/catchpanelgds.cpp) */
extern int  g_elemCount;               /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2eb6c;                 /* src/ez2acapp.cpp */
/* The listed node, viewed only for the per-element block the loader
   publishes into. */
struct NodeSlots { char _pad0[0x3b0]; int f3b0[5]; };

extern char g_modeName[256];           /* 0x1b2e680 (src/modename.cpp) */
extern int  g_modeIndex;               /* 0x1b2e548 (src/modename.cpp) */
extern int  g_mixStyle;                /* 0x1b2ebc8 (src/panels.cpp) */
extern int  g_1b2eac8;                 /* src/gamestate.cpp */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 */
extern int  getListAF8();                          /* 0x418340 */
extern struct ListNode *findListA(int key);         /* 0x418e30 */
extern void Log(const char *fmt, ...);             /* 0x402880 */
/* The Judge under the three view names its already-matched methods are
   defined with (src/accessors.cpp, src/misc.cpp); only the mangled names
   have to agree, which is what lets the calls link. */
struct Obj426D80 { int  setup(SlotSampleSink *s, int a, int b); };  /* 0x426390 */
struct Obj4264D0 { void setAllElem(int val); };                     /* 0x426e60 */
struct Obj426CA0 { void setAllF0(int v); };                         /* 0x426cd0 */

extern Judge *createJudge();                       /* 0x426f40 */

// FUNCTION: EZ2AC_UNPACKED 0x00423a90
int EZ2DJMainGameDirector::m423a90()
{
    /* The three header blocks the chart is read into. The count argument is
       in DWORDS, not bytes - which is what fixes the 0x68 of locals below
       the two /GS buffers, and why the 16-dword reads run off the end of
       their own block into the next one. */
    int   hdrA[8];
    int   tpb;
    int   hdrB[8];
    int   hdrC[8];
    char  name[260];
    char  path[260];
    ChartSink   *chart;
    ChartPlayer *player;
    Judge       *j;
    const int   *p;
    struct ListNode *n;
    int          i;

    pump.start(0, 0);

    /* Mode 5 with two elements up and only one player listed: hand the
       course result to the first listed slot before anything is loaded. */
    if (g_modeIndex == 5 && g_elemCount >= 2 && countListA() <= 1)
        ((PlayerSlot *)findListA(getListAF8()))->m42e2e0();

    if (g_1b2eb6c != 0)
    {
        sprintf(path, "SOUND\\%s", g_songInfoName);
    }
    else
    {
        sprintf(path, "SOUND\\%s", m4208f0());
    }
    changeDir(path, 1);

    if (g_1b2eb6c == 1)
    {
        if (g_mixStyle == 6)
        {
            // STRING: EZ2AC_UNPACKED 0x0048fa08
            sprintf(name, "%s%dp-%s.ez", g_modeName, 2, g_songShortName);
            // STRING: EZ2AC_UNPACKED 0x0048f9f4
            Log("Loaded File : %s", name);
        }
        else
        {
            sprintf(name, "%s%dp-%s.ez", g_modeName, 1, g_songShortName);
        }

        /* Blood Castle runs in percussion mode; nothing else does. */
        // STRING: EZ2AC_UNPACKED 0x0048f9ec
        if (_stricmp(g_songShortName, "7bc-fm1") == 0)
        {
            g_1b2eac8 = 1;
            // STRING: EZ2AC_UNPACKED 0x0048f9c4
            Log("Check Song : Blood Castle // Perc Mode");
        }
        else
        {
            g_1b2eac8 = 0;
        }
    }
    else
    {
        sprintf(name, "%s%dp-%s.ez", g_modeName, 1, g_textSlots[g_elemCount]);
    }

    chart = ((EzClassRegistry *)g_sub1b2ef98)->create(name, &g_ezChartClassInfo,
                                                      g_sub1b2eae4);
    sink = (SlotSampleSink *)chart;
    if (chart == 0)
        goto loadFailed;
    {
        chart->read(hdrC, 4);
        /* The beat count is parked in the FIRST block's third word rather
           than a local of its own - the derived frame has no room for one.
           See src/gfload.cpp for the arithmetic. */
        hdrA[2] = hdrC[2] * 4;
        ((ChartSink *)sink)->read2(hdrA, 4);
        ((ChartSink *)sink)->read(hdrA, 4);
        tpb = hdrA[2];
        f34c = tpb;
        // STRING: EZ2AC_UNPACKED 0x0048f9b8
        Log("m_nTPB = %d", tpb);
        ((ChartSink *)sink)->read(hdrB, 0x10);
        fd14 = hdrB[4];

        player = ((EzClassRegistry *)g_sub1b2ef98)->bind((ChartSink *)sink,
                                                         &classKEZPlayer, &pump);
        notify = (DirectorNotify *)player;
        if (player == 0)
            goto loadFailed;
        {
            player->setMode(2);

            if (countListA() > 1)
            {
                if (f37c == 1)
                {
                    Log("Too Many Player");
                    return 0;
                }

                i = 0;
                if (f380[1] > 0)
                {
                    p = &f380[119];
                    do
                    {
                        ((ChartSink *)sink)->rearm(p[0], p[-113]);
                        i++;
                        p += 4;
                    }
                    while (i < f380[1]);
                }
            }

            /* Chain three key ranges on the first listed node - in modes 7
               and 3, and in any mode under mix style 2 - and only then commit
               the sink. Both the calls and the commit are skipped otherwise:
               every test's miss lands on createJudge (0x423d92). */
            n = findListA(1);
            if (n != 0 && (g_modeIndex == 7 || g_modeIndex == 3 || g_mixStyle == 2))
            {
                m421600(n, 7, 8, 0, fd14);
                m421600(n, 1, 5, 0, fd14);
                m421600(n, 2, 4, 0, fd14);
                ((ChartSink *)sink)->commit();
            }

            j = createJudge();
            judge = j;
            if (j == 0)
            {
            loadFailed:
                /* 0x49daf0 is the CATCH director's copy of this line - the
                   EZ2DJ one is its own string at 0x48f980. */
                // STRING: EZ2AC_UNPACKED 0x0048f980
                Log("EZ2DJMainGameDirector: Can't Load %s", name);
                f1c = 1;
                popDirectory();
                return 0;
            }
            {
                /* The third argument is this object's OWN secondary base -
                   the conversion is what makes it `lea edx,[esi+0x14]`.
                   It must name THIS class's second sink: casting to the
                   catch one is not a base of this class, so the conversion
                   is a no-op and the argument comes out as a bare `this`. */
                ((Obj426D80 *)j)->setup(sink, (int)notify,
                                        (int)(EZ2DJSink2 *)this);
                ((Obj4264D0 *)judge)->setAllElem(0);
                ((Obj426CA0 *)judge)->setAllF0(0);

                if (g_1b2eb6c != 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x0048f970
                    sprintf(name, "%s%dp-%s.ini", g_modeName, 1, g_songShortName);
                }
                else
                {
                    sprintf(name, "%s%dp-%s.ini", g_modeName, 1,
                            g_textSlots[g_elemCount]);
                }

                loadGds(name);

                /* Published into BOTH listed nodes, not into a global. */
                n = findListA(0);
                if (n != 0)
                    ((NodeSlots *)n)->f3b0[g_elemCount] = f2ec;
                n = findListA(1);
                if (n != 0)
                    ((NodeSlots *)n)->f3b0[g_elemCount] = f2ec;

                popDirectory();
                f1c = 0;
                return 1;
            }
        }
    }

}
