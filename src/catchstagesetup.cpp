#include <stdlib.h>
#include <string.h>
#include "players.h"

/* CatchPlayer::beginStage @0x473fd0 - bind a catch-mode player to its owner
   for a stage: the keys PlayerSlot::beginStage (src/stagesetup.cpp) in the
   catch dialect. Its own TU for the same reason: sumTracks lives in
   src/playerslot.cpp and must not be visible here.

   Copy the 40-pair options block @0x1b2e8a8 into +0x408, take the owner's
   sink / clock / judge, ask the sink for the run summary (third word into
   +0x374), pan every sound of a second player to 0x7f when two players are
   listed, then for every sound: kind 1, mark it in the judge, unflag it, and
   walk its hits resetting rate and offset. Judge windows straight from the
   owner's four; JudgmentLink lanes opened and claimed for this slot; the
   effect sink built if missing and 5 published to the judge; the
   elemCount'th 60-byte block cleared; the per-judge counters, the combo,
   the life (100 with an owner, else 0) and the totals zeroed; the sound
   table set to -1; the note count accumulated and kept. */

struct Pair4184 { int a; int b; };
extern Pair4184 g_table4184[];   /* 0x1b2e8a8 (src/globals.cpp) */
extern int g_elemCount;          /* 0x1b2e718 */
extern int countListB();         /* 0x4183e0 */

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
struct CatchStageOwner
{
    char     _pad0[0x304];
    int      f304, f308, f30c, f310;   /* the four judgement windows */
    char     _pad314[0x784 - 0x314];
    int      linkCount[2];             /* 0x784 */
    LinkSlot links[2][8];              /* 0x78c .. 0xc0c */
};

/* The player's own fields. */
struct CatchStagePlayer
{
    char     _pad0[0x1c0];
    int      f1c0;                     /* 0x1c0 - the combo latch */
    int      f1c4[6];                  /* 0x1c4 .. 0x1dc - the per-judge counters */
    int      f1dc;                     /* 0x1dc - the running combo */
    int      f1e0, f1e4, f1e8;
    float    life;                     /* 0x1ec */
    char     _pad1f0[0x1f4 - 0x1f0];
    int      blk[4][15];               /* 0x1f4 - the per-element block */
    char     _pad2e4[0x35c - 0x2e4];
    struct CatchEffectSink *f35c;      /* 0x35c */
    CatchSlotOwner *owner;             /* 0x360 */
    SlotSampleSink *sink;              /* 0x364 */
    SlotClock      *clock;             /* 0x368 */
    void           *judge;             /* 0x36c */
    int      f370;
    int      f374;                     /* 0x374 - the summary's third word */
    char     _pad378[0x408 - 0x378];   /* the sound table */
    Pair4184 opts[40];                 /* 0x408 .. 0x548 */
    int      f548, f54c, f550, f554;
    int      noteCount;                /* 0x558 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00473fd0
void CatchPlayer::beginStage(CatchSlotOwner *owner)
{
    CatchStagePlayer *me = (CatchStagePlayer *)this;
    int i;
    CatchTrackList *tl;
    SlotState st;
    int j;
    const int *hp;

    memcpy(me->opts, g_table4184, sizeof(me->opts));
    me->owner = owner;
    me->sink = owner->sink;
    me->clock = owner->clock;
    me->judge = owner->judge;
    me->sink->query((int *)&st, 4);
    me->f374 = *(int *)&st.f8;
    tl = &me->owner->tracks[id];
    if (countListB() > 1)
    {
        i = 0;
        if (i < tl->count)
        {
            hp = &tl->items[0].key;
            do
            {
                me->clock->setPan(*hp, id != 0 ? 0x7f : 0);
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
    hp = &tl->items[0].key;
    do
    {
        int h = *hp;
        me->clock->setKind(h, 1);
        ((Obj4264D0 *)me->judge)->setElem(h, 1);
        ((Obj426CA0 *)me->judge)->setF0(h, 0);
        if (me->sink->begin(h, 0, 0, 1, &st, &sn) != 0)
        {
            do
            {
                st.rate = 1.0f;
                st.offset = 0;
                me->sink->setState(&sn, &st);
            }
            while (me->sink->next(1, &st, &sn) != 0);
        }
        j++;
        hp += 4;
    }
    while (j < tl->count);
    }
    {
        CatchStageOwner *d = (CatchStageOwner *)me->owner;
        ((Obj426D80 *)me->judge)->setWindows(d->f310, d->f30c, d->f308, d->f304);
    }
    for (i = 0; i < ((CatchStageOwner *)me->owner)->linkCount[id]; i++)
    {
        int lane = ((Obj426D80 *)me->judge)->openLane();
        LinkSlot *e = &((CatchStageOwner *)me->owner)->links[id][i];
        int k;
        for (k = 0; k < e->n; k++)
            ((Obj426D80 *)me->judge)->claim(lane, e->v[k]);
    }
    if (me->f35c == 0)
    {
        m472e60();
        ((Obj426D80 *)me->judge)->f10 = 5;
    }
    memset(me->blk[g_elemCount], 0, 60);
    memset(me->f1c4, 0, sizeof(me->f1c4));
    me->noteCount = 0;
    me->f54c = 0;
    me->f550 = 0;
    me->f548 = 0;
    me->f1e8 = 0;
    me->f1e4 = 0;
    me->f1e0 = 0;
    me->f1dc = 0;
    me->f1c0 = 0;
    if (me->owner != 0)
        me->life = 100.0f;
    else
        me->life = 0.0f;
    for (i = 0; i < 18; i++)
    {
        snd[i].f0 = -1;
        snd[i].id = -1;
    }
    me->f554 = me->opts[13].b;
    me->noteCount += sumTracks();
}
