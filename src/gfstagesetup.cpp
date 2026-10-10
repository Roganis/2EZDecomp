#include <stdlib.h>
#include <string.h>
#include "players.h"

/* PlayerRecord::beginStage @0x466700 - the scratch-mode player's stage
   binding, the third dialect of PlayerSlot::beginStage (src/stagesetup.cpp)
   and CatchPlayer::beginStage (src/catchstagesetup.cpp); its own TU so that
   sumTracks (src/playerslot.cpp) stays out of sight.

   The keys shape - options block at +0x284, the owner's sink/clock/judge,
   the summary's third word into +0x1f0, the second player panned, every
   sound kinded (2 under the g_1b5f2e0 option), marked and flagged, its hits
   reset - with the judge windows widened by 6/8/12/22 over the owner's four,
   the link loop on the slot (0 when a single player is listed), the effect
   sink built if missing and its +0x34 published, the elemCount'th 64-byte
   block cleared; then, unless the owner holds the state over with the
   element count up, the counters are cleared and the gauge reset - to 100
   under mode 0, to 0 with the second gauge at 5 under mode 1 on a fresh
   run or with no owner at all; mode 1 also copies the second gauge's level
   out; the sound table set to -1; the note count accumulated. */

struct Pair4184 { int a; int b; };
extern Pair4184 g_table4184[];   /* 0x1b2e8a8 (src/globals.cpp) */
extern int g_1b5f2e0;            /* src/holdtick.cpp */
extern int g_elemCount;          /* 0x1b2e718 */
extern int countListC();         /* 0x4184a0 */

struct Obj4264D0 { void setElem(int idx, int val); };                /* 0x4264d0 */
struct Obj426CA0 { void setF0(int i, int v); };                      /* 0x426ca0 */
struct Obj426D80
{
    char _pad0[0x10];
    int  f10;
    int  claim(int lane, int id);                                    /* 0x426d80 */
    int  openLane();                                                 /* 0x426d40 */
    void setWindows(int w5, int w4, int w3, int w2);                 /* 0x426ea0 */
};
struct LinkSlot { int f0; int n; int v[16]; };

/* The owner's fields this needs. */
struct GFStageOwner
{
    char     _pad0[0x150];
    Obj426D80 *judge;                  /* 0x150 */
    char     _pad154[0x2ec - 0x154];
    int      f2ec, f2f0, f2f4, f2f8;   /* the four judgement windows */
    char     _pad2fc[0x750 - 0x2fc];
    int      mode;                     /* 0x750 */
    char     _pad754[0x768 - 0x754];
    int      holdOver;                 /* 0x768 */
    char     _pad76c[0x788 - 0x76c];
    int      linkCount[2];             /* 0x788 */
    LinkSlot links[2][8];              /* 0x790 .. 0xc10 */
};

/* The player's own fields. */
struct GFStagePlayer
{
    char     _pad0[0xc];
    int      fc;
    int      f10[6];                   /* 0x10 .. 0x28 - the per-judge counters */
    int      f28, f2c, f30, f34;
    float    gauge;                    /* 0x38 */
    char     _pad3c[0x50 - 0x3c];
    float    f50;
    float    gauge2;                   /* 0x54 */
    char     _pad58[0x1f0 - 0x58];
    int      f1f0;                     /* 0x1f0 - the summary's third word */
    char     _pad1f4[0x284 - 0x1f4];
    Pair4184 opts[40];                 /* 0x284 .. 0x3c4 */
    char     _pad3c4[0x530 - 0x3c4];
    int      noteCount;                /* 0x530 */
    int      f534;
};

// FUNCTION: EZ2AC_UNPACKED 0x00466700
void PlayerRecord::beginStage(GFSlotOwner *owner)
{
    GFStagePlayer *me = (GFStagePlayer *)this;
    int i;
    TrackList *tl;
    SlotState st;
    int slot;
    int j;
    const int *hp;

    memcpy(me->opts, g_table4184, sizeof(me->opts));
    f1e0 = owner;
    f1e4 = (int)owner->sink;
    f1e8 = (int)owner->clock;
    f1ec = (int)((GFStageOwner *)owner)->judge;
    ((SlotSampleSink *)f1e4)->query((int *)&st, 4);
    me->f1f0 = *(int *)&st.f8;
    tl = &f1e0->tracks[id];
    if (countListC() > 1)
    {
        i = 0;
        if (i < tl->count)
        {
            hp = &tl->items[0].handle;
            do
            {
                ((SlotClock *)f1e8)->setPan(*hp, id != 0 ? 0x7f : 0);
                i++;
                hp += 4;
            }
            while (i < tl->count);
        }
    }
    j = 0;
    if (tl->count > 0)
    {
    SlotSound sn;
    hp = &tl->items[0].handle;
    do
    {
        int h = *hp;
        ((SlotClock *)f1e8)->setKind(h, (g_1b5f2e0 != 0) + 1);
        ((Obj4264D0 *)f1ec)->setElem(h, 1);
        ((Obj426CA0 *)f1ec)->setF0(h, g_1b5f2e0 != 0);
        if (((SlotSampleSink *)f1e4)->begin(h, 0, 0, 1, &st, &sn) != 0)
        {
            do
            {
                st.rate = 1.0f;
                st.offset = 0;
                ((SlotSampleSink *)f1e4)->setState(&sn, &st);
            }
            while (((SlotSampleSink *)f1e4)->next(1, &st, &sn) != 0);
        }
        j++;
        hp += 4;
    }
    while (j < tl->count);
    }
    {
        GFStageOwner *d = (GFStageOwner *)f1e0;
        ((Obj426D80 *)f1ec)->setWindows(d->f2f8 + 6, d->f2f4 + 8, d->f2f0 + 12, d->f2ec + 22);
    }
    if (countListC() > 1)
        slot = id;
    else
        slot = 0;
    for (i = 0; i < ((GFStageOwner *)f1e0)->linkCount[slot]; i++)
    {
        int lane = ((Obj426D80 *)f1ec)->openLane();
        LinkSlot *e = &((GFStageOwner *)f1e0)->links[slot][i];
        int k;
        for (k = 0; k < e->n; k++)
            ((Obj426D80 *)f1ec)->claim(lane, e->v[k]);
    }
    if (f1dc == 0)
    {
        m465d90();
        ((Obj426D80 *)f1ec)->f10 = f1dc->f34;
    }
    memset(&block[g_elemCount * 16], 0, 64);
    if (((GFStageOwner *)f1e0)->holdOver == 0 || g_elemCount == 0)
    {
        memset(me->f10, 0, sizeof(me->f10));
        me->noteCount = 0;
        if (f1e0 != 0)
        {
            if (((GFStageOwner *)f1e0)->mode == 0)
            {
                me->gauge = 100.0f;
                me->f28 = 0;
                me->fc = 0;
                me->f2c = 0;
                me->f30 = 0;
            }
            else if (((GFStageOwner *)f1e0)->mode == 1)
            {
                if (g_elemCount == 0)
                {
                    me->gauge = 0.0f;
                    me->gauge2 = 5.0f;
                    me->f28 = 0;
                    me->fc = 0;
                }
                me->f2c = 0;
                me->f30 = 0;
            }
        }
        else
        {
            me->gauge = 0.0f;
            me->gauge2 = 5.0f;
            me->fc = 0;
            me->f28 = 0;
            me->f2c = 0;
            me->f30 = 0;
        }
    }
    if (((GFStageOwner *)f1e0)->mode == 1)
        me->f50 = me->gauge2;
    for (i = 0; i < 18; i++)
    {
        snd[i].f0 = -1;
        snd[i].id = -1;
    }
    me->f534 = me->opts[13].b;
    me->noteCount += sumTracks();
}

/* PlayerRecord::m4659f0 @0x4659f0 - the tail of beginStage as its own
   method: the per-judge counters, the note count, and the gauges by the
   owner's mode - the same block, so the same view. */
// FUNCTION: EZ2AC_UNPACKED 0x004659f0
void PlayerRecord::m4659f0()
{
    GFStagePlayer *me = (GFStagePlayer *)this;

    memset(me->f10, 0, sizeof(me->f10));
    me->noteCount = 0;
    if (f1e0 != 0)
    {
        if (((GFStageOwner *)f1e0)->mode == 0)
        {
            me->gauge = 100.0f;
            me->fc = 0;
            me->f28 = 0;
            me->f2c = 0;
            me->f30 = 0;
        }
        else if (((GFStageOwner *)f1e0)->mode == 1)
        {
            if (g_elemCount == 0)
            {
                me->gauge = 0.0f;
                me->gauge2 = 5.0f;
                me->f28 = 0;
                me->fc = 0;
            }
            me->f2c = 0;
            me->f30 = 0;
        }
    }
    else
    {
        me->gauge = 0.0f;
        me->gauge2 = 5.0f;
        me->fc = 0;
        me->f28 = 0;
        me->f2c = 0;
        me->f30 = 0;
    }
}

extern int g_1b5f180;            /* 0x1b5f180 (src/maingame.cpp) */
/* The mode-9 per-stage stat arrays (src/resultctor.cpp). */
extern int g_1b2f094[5];
extern int g_1b5f188[5];
extern int g_1b2efd8[5];
extern int g_1b2eb2c[5];
extern int g_1b5f2c4[5];
extern int g_1b2eaac[5];
extern int g_1b2ea64[5];
extern int g_1b2e7b0[5];
extern int g_1b2eff0[5];
extern int g_1b2eb4c[5];

/* One 0x40-byte row of the per-stage accumulator block at +0x5c (the region
   beginStage's memset clears, seen typed). */
struct GFStageAcc
{
    int   r00;   /* += the stage's note count */
    int   r04;
    int   r08, r0c, r10, r14, r18;   /* += the five judge counters */
    int   r1c, r20, r24;             /* += f28 / f2c / f30 */
    int   r28;
    float r2c;   /* += the gauge */
    int   r30;   /* the grade ladder, run after the clear */
    int   r34;   /* = g_1b5f180 */
    float r38;   /* += the second gauge */
    int   r3c;
};

/* The GF stage-end fold, reached through the director's slot-release pass
   (m45def0 @0x45def0): add this stage's counters into the elemCount'th
   accumulator row, publish them into the per-stage stat arrays the result
   screen reads, run the grade ladder twice (once into the array before the
   counters are cleared, once into the row after), and drop the combo -
   unconditionally once, and once more if the owner is not holding the run
   over (a faithful dead second store). */
// FUNCTION: EZ2AC_UNPACKED 0x00465c70
void PlayerRecord::m465c70()
{
    GFStagePlayer *me = (GFStagePlayer *)this;
    GFStageAcc *a = (GFStageAcc *)&block[g_elemCount * 16];
    int i;

    a->r00 += me->noteCount;
    a->r18 += me->f10[5];
    a->r14 += me->f10[4];
    a->r10 += me->f10[3];
    a->r0c += me->f10[2];
    a->r08 += me->f10[1];
    a->r1c += me->f28;
    a->r20 += me->f2c;
    a->r24 += me->f30;
    a->r2c += me->gauge;
    a->r34 = g_1b5f180;

    i = g_elemCount;
    g_1b2f094[i] = me->f30;
    g_1b5f188[i] = me->f2c;
    g_1b2efd8[i] = me->f10[5];
    g_1b2eb2c[i] = me->f10[4];
    g_1b5f2c4[i] = me->f10[3];
    g_1b2eaac[i] = me->f10[2];
    g_1b2ea64[i] = me->f10[1];
    g_1b2e7b0[i] = me->noteCount;
    g_1b2eff0[i] = m465b30();
    g_1b2eb4c[i] = me->f28;

    me->f30 = 0;
    me->f2c = 0;
    f1dc = 0;
    a->r38 += me->gauge2;
    a->r30 = m465b30();

    if (((GFStageOwner *)f1e0)->holdOver == 0)
        f1dc = 0;
}
