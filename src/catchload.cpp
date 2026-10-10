/* CatchMainGameDirector::m46dba0 @0x46dba0 - the Ez2Catch chart loader, and
   the twin of EZ2DJMainGameDirector::m423a90 @0x423a90 (73.9% of the same
   instruction sequence, so that one is an edit of this).

   What it does, end to end: start the pump, chdir into the song's SOUND
   directory, build the chart's file name, ask the class registry at
   g_sub1b2ef98 for a KSongEZ over it, read four blocks of header out of the
   chart, bind a KEZPlayer to it, refuse a two-player run the mode forbids,
   re-arm every already-claimed lane, build and arm a Judge, then load the
   matching .ini through loadGds and publish the mode's own value.

   The two registry calls are the interesting part: slot 4 (+0x10) takes the
   file name, the KSongEZ class record @0x4a7370 and the sound subsystem and
   hands back the chart - which is what the director keeps as its `sink` -
   and slot 7 (+0x1c) takes that chart, the KEZPlayer record @0x4a733c and
   the pump, and hands back the player. Everything downstream is driven off
   those two.

   100%* EFFECTIVE. It was parked at 87.14% on a reading that the beat count
   is a `volatile int step` and that one header block therefore has to give
   four bytes back (hdrB[7]). Deriving the frame instead - see the
   arithmetic in src/gfload.cpp, which is the same 0x26c layout to the byte -
   says otherwise: the three blocks are EIGHT dwords each, the two buffers
   are 260, `path` ends exactly on the cookie, and there is NO SLOT for the
   beat count at all. The original parks it in hdrA's third word, which the
   reads that follow write over. Two further corrections came with it: the
   third read is 4 dwords, not 16, and the judge failure is the FALL-THROUGH
   of its test with the success block out of line and the two earlier
   failures jumping into it.

   The residue is the ORDER of the four local slots. The original lays
   them out as read-A at +0x08, `step` at +0x10, read-B at +0x28 and
   read-C at +0x48; cl permutes them. Four declaration orders were swept
   (step first, step second, step last, and a split low buffer) and all
   four compile BYTE-IDENTICALLY, which is the documented stack-packer
   wall - the slot assignment is not reachable from source.

   ONE SEMANTIC UNCERTAINTY, flagged for the port. The second argument of
   the two header readers cannot be a byte count: read-B is given 4 and
   the value used comes back at +8 of its buffer, and read-A is given 0x10
   into a buffer that `step` sits four bytes inside - both of which a byte
   count would make garbage. It is either a dword count or, more likely, a
   record selector with a fixed-size result. The frame layout above is
   evidenced; the READ SIZES are not. Anyone porting this must read slot 8
   and slot 9 of the chart before trusting the buffer sizes here. */

#include <stdio.h>
#include "ezscreen.h"
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
extern int  g_1b2ef44[5];              /* src/modeselectctor.cpp */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListB();                          /* 0x4183e0 */
extern void Log(const char *fmt, ...);             /* 0x402880 */
/* The Judge under the three view names its already-matched methods are
   defined with (src/accessors.cpp, src/misc.cpp); only the mangled names
   have to agree, which is what lets the calls link. */
struct Obj426D80 { int  setup(SlotSampleSink *s, int a, int b); };  /* 0x426390 */
struct Obj4264D0 { void setAllElem(int val); };                     /* 0x426e60 */
struct Obj426CA0 { void setAllF0(int v); };                         /* 0x426cd0 */

extern Judge *createJudge();                       /* 0x426f40 */

// FUNCTION: EZ2AC_UNPACKED 0x0046dba0
int CatchMainGameDirector::m46dba0()
{
    /* The three header blocks the chart is read into. The count argument is
       in DWORDS, not bytes - which is what fixes the 0x68 of locals below
       the two /GS buffers, and why the 16-dword reads run off the end of
       their own block into the next one. */
    int   hdrA[8];
    int   hdrB[8];
    int   hdrC[8];
    char  name[260];
    char  path[260];
    ChartSink   *chart;
    ChartPlayer *player;
    Judge       *j;
    const int   *p;
    int          i;

    pump.start(0, 0);

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048fa14
        sprintf(path, "SOUND\\%s", g_songInfoName);
    }
    else
    {
        sprintf(path, "SOUND\\%s", m46c360());
    }
    changeDir(path, 1);

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049db28
        sprintf(name, "catch%dp-%s.ez", 1, g_textSlots[g_elemCount]);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049db18
        sprintf(name, "CV2Mix1p-%s.ez", g_songShortName);
    }

    chart = ((EzClassRegistry *)g_sub1b2ef98)->create(name, &g_ezChartClassInfo,
                                                      g_sub1b2eae4);
    sink = (SlotSampleSink *)chart;
    if (chart == 0)
        goto loadFailed;
    {
        chart->read(hdrC, 4);
        /* The beat count is parked in the FIRST block's third word rather
           than a local of its own - the derived frame has no room for one,
           and the reads that follow write over it. See the same finding
           worked out in full in src/gfload.cpp. */
        hdrA[2] = hdrC[2] * 4;
        ((ChartSink *)sink)->read2(hdrA, 4);
        ((ChartSink *)sink)->read(hdrA, 4);
        f334 = hdrA[2];
        ((ChartSink *)sink)->read(hdrB, 0x10);
        fca4 = hdrB[4];

        player = ((EzClassRegistry *)g_sub1b2ef98)->bind((ChartSink *)sink,
                                                         &classKEZPlayer, &pump);
        notify = (DirectorNotify *)player;
        if (g_1b2eb6c == 0 && player == 0)
            goto loadFailed;
        {
            player->setMode(2);

            if (countListB() > 1)
            {
                if (f348 == 1)
                {
                    // STRING: EZ2AC_UNPACKED 0x0048f9a8
                    Log("Too Many Player");
                    return 0;
                }

                i = 0;
                if (f34c[1] > 0)
                {
                    p = &f34c[115];
                    do
                    {
                        ((ChartSink *)sink)->rearm(p[0], p[-111]);
                        i++;
                        p += 4;
                    }
                    while (i < f34c[1]);
                }
            }

            j = createJudge();
            judge = j;
            if (j == 0)
            {
            loadFailed:
                // STRING: EZ2AC_UNPACKED 0x0049daf0
                Log("CatchMainGameDirector: Can't Load %s", name);
                f38 = 1;
                popDirectory();
                return 0;
            }
            {
                /* The third argument is this object's OWN secondary base -
                   the conversion is what makes it `lea ecx,[esi+0x14]`. */
                ((Obj426D80 *)j)->setup(sink, (int)notify,
                                        (int)(CatchSink2 *)this);
                ((Obj4264D0 *)judge)->setAllElem(0);
                ((Obj426CA0 *)judge)->setAllF0(0);

                if (g_1b2eb6c != 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x0049dae0
                    sprintf(name, "cv2Mix1p-%s.ini", g_songShortName);
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x0049dad0
                    sprintf(name, "catch%dp-%s.ini", 1, g_textSlots[g_elemCount]);
                }

                loadGds(name);
                g_1b2ef44[g_elemCount] = f2f8;
                popDirectory();
                f38 = 0;
                return 1;
            }
        }
    }
}
