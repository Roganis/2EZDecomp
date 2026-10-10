/* GFMainGameDirector::m45fd50 @0x45fd50 - the scratch chart loader, and the
   twin of CatchMainGameDirector::m46dba0 @0x46dba0 (src/catchload.cpp).
   Read that function's header for the shape and for why `step` is volatile;
   the deltas are:

     - the file names are built from the MODE NAME rather than a literal
       prefix, so the format is "%s%dp-%s.ez" / "%s%dp-%s.ini" with
       g_modeName first and either the current text slot or - under CV2 -
       the short name, the CV2 arm passing the literal "CV2Mix" for the .ini
       where the .ez arm passes the mode name;
     - the header reads are 4/4/4/16 dwords rather than 4/4/16/16, the beat
       count lands at +0x318 and the ladder value at +0xca8;
     - the two-player refusal is gated on +0x33c and the re-arm loop walks
       +0x51c against the count at +0x344;
     - and there is NO judge setup between the player and the .ini. Instead
       the FIRST list player is looked up and handed three lane
       arrangements through m45e630 - (7,8), (1,5) and (2,4), each with the
       ladder value - and the chart is committed; the judge is built after
       that and only wired to the chart and the notify.

   The failure log names this director, and the failure path leaves +0x1c
   set where success clears it.

   100%* EFFECTIVE, and the whole frame was DERIVED rather than guessed -
   which is what got it there from 82.50%. Three size splits had been tried
   by then and none worked, because the question is not how big each block
   is but where the frame's edges are. Resolve every `lea` and every spill
   to an absolute offset from the entry esp and the layout falls out with no
   slack at all:

       hdrA  S-0x26c   8 dwords     the 4/4 read pair's target
       hdrB  S-0x24c   8 dwords     the 16-dword read's
       hdrC  S-0x22c   8 dwords     the first 4-dword read's
       name  S-0x20c   260
       path  S-0x108   260          ends exactly at the cookie
       cookie S-0x4
                                    total 0x268 + 4 = the 0x26c frame

   TWO THINGS THAT FALL OUT OF IT, and neither was reachable by guessing.
   The buffers are 260, not 256 - `path` ending exactly on the cookie fixes
   that. And there is NO SLOT FOR THE BEAT COUNT: the frame is full, and the
   original parks it in hdrA's THIRD WORD, which the two reads that follow
   then write over. The catch twin's `volatile int step` has nowhere to live
   here, and writing `hdrA[2] = hdrC[2] * 4` instead is what the bytes say.

   The last edit was the branch layout: the judge failure is the FALL-THROUGH
   of its test, with the success block out of line, and the two earlier
   failures jump into it. Written as `if (j != 0) { success }` with a
   trailing Log, cl lays the success inline and every displacement after the
   chart test shifts.

   The residue is one instruction's SCHEDULE - the spill of the beat count
   sits one instruction earlier than the original's - so it is a 100%
   effective match rather than byte-identical. */

#include <stdio.h>
#include "ezscreen.h"
#include "players.h"
#include "ezchart.h"   /* EzClassInfo */

/* The chart the registry builds, viewed for the four slots this uses. */
struct GFChartSink
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    /* slot 6 -> +0x18: re-arm one already-claimed lane. */
    virtual void rearm(int a, int b);
    virtual void q07();
    /* slot 8 -> +0x20: read a block of the chart header. */
    virtual void read(void *out, int n);
    /* slot 9 -> +0x24: and its sibling. */
    virtual void read2(void *out, int n);
    virtual void q10(); virtual void q11(); virtual void q12();
    virtual void q13(); virtual void q14(); virtual void q15();
    virtual void q16(); virtual void q17(); virtual void q18();
    virtual void q19(); virtual void q20(); virtual void q21();
    virtual void q22();
    /* slot 23 -> +0x5c: commit the arrangement. */
    virtual void commit();
};

struct GFChartPlayer
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

struct GFRegistry
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual GFChartSink   *create(char *name, void *cls, void *owner);
    virtual void q05(); virtual void q06();
    virtual GFChartPlayer *bind(GFChartSink *chart, void *cls, void *owner);
};

/* The Judge under the three view names its matched methods are defined with. */
struct Obj426D80 { int  setup(struct SlotSampleSink *s, int a, int b); };  /* 0x426390 */
struct Obj4264D0 { void setAllElem(int val); };           /* 0x426e60 */
struct Obj426CA0 { void setAllF0(int v); };               /* 0x426cd0 */

extern EzSubsystem *g_sub1b2eae4;      /* src/ez2acapp.cpp */
extern EzSubsystem *g_sub1b2ef98;      /* src/ez2acapp.cpp - the registry */
extern KRuntimeClass classKEZPlayer;   /* 0x4a733c (src/globals.cpp) */

extern char g_textSlots[32][128];      /* 0x1b2ec18 (src/modename.cpp) */
extern char g_songInfoName[64];        /* 0x1b2eb80 (src/panels.cpp) */
extern char g_songShortName[0x40];     /* 0x1b2f004 (src/catchpanelgds.cpp) */
extern char g_modeName[256];           /* 0x1b2e680 (src/modename.cpp) */
extern int  g_elemCount;               /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2eb6c;                 /* src/ez2acapp.cpp */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListC();                          /* 0x4184a0 */
extern void Log(const char *fmt, ...);             /* 0x402880 */
extern Judge *createJudge();                       /* 0x426f40 */
extern PlayerRecord *findPlayer(int id);           /* 0x418f30 */

/* The per-element ladder value this loader publishes. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e9e8
int g_1b2e9e8[5];

// FUNCTION: EZ2AC_UNPACKED 0x0045fd50
int GFMainGameDirector::m45fd50()
{
    int   hdrA[8];
    int   hdrB[8];
    int   hdrC[8];
    char  name[260];
    char  path[260];
    GFChartSink   *chart;
    GFChartPlayer *player;
    PlayerRecord  *first;
    Judge         *j;
    const int     *p;
    int            i;

    pump.start(0, 0);

    if (g_1b2eb6c != 0)
    {
        sprintf(path, "SOUND\\%s", g_songInfoName);
    }
    else
    {
        sprintf(path, "SOUND\\%s", m45da00());
    }
    changeDir(path, 1);

    if (g_1b2eb6c != 0)
    {
        sprintf(name, "%s%dp-%s.ez", g_modeName, 1, g_songShortName);
    }
    else
    {
        sprintf(name, "%s%dp-%s.ez", g_modeName, 1, g_textSlots[g_elemCount]);
    }

    chart = ((GFRegistry *)g_sub1b2ef98)->create(name, &g_ezChartClassInfo,
                                                 g_sub1b2eae4);
    sink = (SlotSampleSink *)chart;
    if (chart == 0)
        goto loadFailed;
    {
        chart->read(hdrC, 4);
        /* The beat count is parked in the FIRST block's third word rather
           than a local of its own - the frame has no room for one, and the
           two reads that follow write over it. */
        hdrA[2] = hdrC[2] * 4;
        ((GFChartSink *)sink)->read2(hdrA, 4);
        ((GFChartSink *)sink)->read(hdrA, 4);
        f318 = hdrA[2];
        ((GFChartSink *)sink)->read(hdrB, 0x10);
        fca8 = hdrB[4];

        player = ((GFRegistry *)g_sub1b2ef98)->bind((GFChartSink *)sink,
                                                    &classKEZPlayer, &pump);
        notify = (DirectorNotify *)player;
        if (player == 0)
            goto loadFailed;
        {
            player->setMode(2);

            if (countListC() > 1)
            {
                if (f33c == 1)
                {
                    Log("Too Many Player");
                    return 0;
                }

                i = 0;
                if (f340[1] > 0)
                {
                    p = &f340[119];
                    do
                    {
                        ((GFChartSink *)sink)->rearm(p[0], p[-113]);
                        i++;
                        p += 4;
                    }
                    while (i < f340[1]);
                }
            }

            first = findPlayer(1);
            if (first != 0)
            {
                m45e630((ListNode *)first, 7, 8, 0, fca8);
                m45e630((ListNode *)first, 1, 5, 0, fca8);
                m45e630((ListNode *)first, 2, 4, 0, fca8);
                ((GFChartSink *)sink)->commit();
            }

            j = createJudge();
            judge = j;
            if (j == 0)
            {
            loadFailed:
                // STRING: EZ2AC_UNPACKED 0x0049a184
                Log("GFMainGameDirector: Can't Load %s", name);
                f1c = 1;
                popDirectory();
                return 0;
            }
            {
                /* This class's OWN second sink, not a raw offset - the
                   conversion is what makes it `lea edx,[esi+0x14]`. */
                ((Obj426D80 *)j)->setup(sink, (int)notify,
                                        (int)(GFSink2 *)this);
                ((Obj4264D0 *)judge)->setAllElem(0);
                ((Obj426CA0 *)judge)->setAllF0(0);

                if (g_1b2eb6c != 0)
                {
                    sprintf(name, "%s%dp-%s.ini",
                            // STRING: EZ2AC_UNPACKED 0x0048d63c
                            "CV2Mix", 1, g_songShortName);
                }
                else
                {
                    sprintf(name, "%s%dp-%s.ini", g_modeName, 1,
                            g_textSlots[g_elemCount]);
                }

                loadGds(name);
                g_1b2e9e8[g_elemCount] = f2e0;
                popDirectory();
                f1c = 0;
                return 1;
            }
        }
    }
}
