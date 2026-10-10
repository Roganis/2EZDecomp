#include <stdlib.h>
#include <string.h>
#include "players.h"

/* PlayerSlot::beginStage @0x4305a0 - bind a keys-mode player to its director
   for a stage. THIS TU EXISTS SO THAT NOTHING IT CALLS IS VISIBLE: sumTracks,
   m42f080 and m42f1a0 live in src/playerslot.cpp and cl would inline them
   here (a stage's callee inlined = the call disappears).

   What it does, in order: copy the 40-pair options block @0x1b2e8a8 into
   the slot (+0x474..0x5b4 - the f4f4/f51c option words are inside it), clear
   g_1b2ea2c, take the director's sink / clock / judge, ask the sink for the
   run summary (its third word lands in +0x3e0), pan every sound of a second
   player to 0x7f when two players are on, then for every sound: set its
   kind (2 under the g_1b5f2e0 option), mark it in the judge, and walk its
   hits resetting rate to 1 and offset to 0. Options: shuffle hits, claim
   hold-note handles (two flavours). Judge windows +3 (or +1 under g_1b2eb6c)
   from the director's four; JudgmentLink lanes opened and claimed for this
   slot (slot 0 when a single player); the effect sink built if missing and
   its +0x38 published to the judge; the elemCount'th 64-byte block cleared;
   the sound table set to -1; the note count accumulated and x300 into +0x3c8.

   100% (2026-08-19). The "zero register" that held it at 95.60% for a day was
   a NESTING shape: `if (countListA() > 1) { i = 0; if (i < tl->count) {...} }`
   - the zero is materialized BETWEEN the two tests, which is where the
   original keeps edi alive from. Written as one `&&` guard the zero folds
   into immediates everywhere after it. Also load-bearing:
   loop 1's counter is `i` (a register), loop 2's is a SEPARATE `j` (memory,
   sharing i's slot), the link loop is `i` again - that is what puts tl below
   i in the frame; the sink summary is read INTO st (`(int *)&st`) so the
   frame is 0x44 not 0x64; the later director reads go through f3d0, not the
   parameter; the judge is re-read from f3dc at every use; the four windows
   go smallest-first (f304 is w5). */

struct Pair4184 { int a; int b; };
extern Pair4184 g_table4184[];   /* 0x1b2e8a8 (src/globals.cpp) */
extern int g_1b2ea2c;            /* src/globals.cpp */
extern int g_1b5f2e0;            /* src/holdtick.cpp */
extern int g_1b2eb6c;            /* src/ez2acapp.cpp */
extern int g_elemCount;          /* 0x1b2e718 */
extern int countListA();         /* 0x418310 */

/* The judge's three views, as src/accessors.cpp and src/misc.cpp define them. */
struct Obj4264D0 { void setElem(int idx, int val); };                /* 0x4264d0 */
struct Obj426CA0 { void setF0(int i, int v); };                      /* 0x426ca0 */
struct Obj426D80
{
    char _pad0[0x10];
    int  f10;                                                        /* 0x10 <- sink->f38 */
    int  claim(int lane, int id);                                    /* 0x426d80 */
    int  openLane();                                                 /* 0x426d40 */
    void setWindows(int w5, int w4, int w3, int w2);                 /* 0x426ea0 */
};

/* One JudgmentLink slot: 0x48 bytes, the count at +4 and the ids after it. */
struct LinkSlot { int f0; int n; int v[16]; };

/* The director fields this needs, through the slot's f3d0. */
struct StageDirector
{
    char     _pad0[0x154];
    Obj426D80 *judge;                  /* 0x154 */
    char     _pad158[0x2f8 - 0x158];
    int      f2f8, f2fc, f300, f304;   /* the four judgement windows */
    char     _pad308[0x794 - 0x308];
    int      f794;
    char     _pad798[0x7c4 - 0x798];
    int      f7c4;
    char     _pad7c8[0x7f4 - 0x7c8];
    int      linkCount[2];             /* 0x7f4 */
    LinkSlot links[2][8];              /* 0x7fc .. 0xc7c */
    char     _padc7c[0xd14 - 0xc7c];
    int      fd14;
};

/* The slot's own far fields. */
struct StageSlot
{
    char     _pad0[0x218];
    int      f218;
    int      f21c;
    char     _pad220[0x474 - 0x220];
    Pair4184 opts[40];                 /* 0x474 .. 0x5b4 - the options block */
    char     _pad5b4[0xe105bc - 0x5b4];
    int      noteCount;                /* 0xe105bc */
    int      f_e105c0;
};

// FUNCTION: EZ2AC_UNPACKED 0x004305a0
void PlayerSlot::beginStage(SlotOwner *owner)
{
    StageSlot *me = (StageSlot *)this;
    int i;
    TrackList *tl;
    SlotState st;
    int slot;
    int j;
    const int *hp;

    memcpy(me->opts, g_table4184, sizeof(me->opts));
    g_1b2ea2c = 0;
    f3d0 = owner;
    f3d4 = (int)owner->sink;
    f3d8 = (int)owner->clock;
    f3dc = (int)((StageDirector *)owner)->judge;
    ((SlotSampleSink *)f3d4)->query((int *)&st, 4);
    f3e0 = *(int *)&st.f8;
    tl = &f3d0->tracks[id];
    if (countListA() > 1)
    {
        i = 0;
        if (i < tl->count)
        {
            hp = &tl->items[0].handle;
            do
            {
                ((SlotClock *)f3d8)->setPan(*hp, id != 0 ? 0x7f : 0);
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
        ((SlotClock *)f3d8)->setKind(h, (g_1b5f2e0 != 0) + 1);
        ((Obj4264D0 *)f3dc)->setElem(h, 1);
        ((Obj426CA0 *)f3dc)->setF0(h, g_1b5f2e0 != 0);
        if (((SlotSampleSink *)f3d4)->begin(h, 0, 0, 1, &st, &sn) != 0)
        {
            do
            {
                st.rate = 1.0f;
                st.offset = 0;
                ((SlotSampleSink *)f3d4)->setState(&sn, &st);
            }
            while (((SlotSampleSink *)f3d4)->next(1, &st, &sn) != 0);
        }
        j++;
        hp += 4;
    }
    while (j < tl->count);
    }
    if (me->opts[7].a != 0)
        f3d0->randomizeHits((ListNode *)this, 0, ((StageDirector *)f3d0)->fd14);
    if (f4f4 != 0)
        m42f080();
    if (f51c != 0)
        m42f1a0();
    {
        StageDirector *d = (StageDirector *)f3d0;
        if (g_1b2eb6c == 0)
            ((Obj426D80 *)f3dc)->setWindows(d->f304 + 3, d->f300 + 3, d->f2fc + 3, d->f2f8 + 3);
        else
            ((Obj426D80 *)f3dc)->setWindows(d->f304 + 1, d->f300 + 1, d->f2fc + 1, d->f2f8 + 1);
    }
    if (countListA() > 1)
        slot = id;
    else
        slot = 0;
    for (i = 0; i < ((StageDirector *)f3d0)->linkCount[slot]; i++)
    {
        int lane = ((Obj426D80 *)f3dc)->openLane();
        LinkSlot *e = &((StageDirector *)f3d0)->links[slot][i];
        int k;
        for (k = 0; k < e->n; k++)
            ((Obj426D80 *)f3dc)->claim(lane, e->v[k]);
    }
    if (f3cc == 0)
    {
        m42e470();
        ((Obj426D80 *)f3dc)->f10 = f3cc->f38;
    }
    memset(&block[g_elemCount * 16], 0, 64);
    if (((StageDirector *)f3d0)->f7c4 == 0 || g_elemCount == 0)
        m42e000();
    if (((StageDirector *)f3d0)->f794 == 1)
        me->f218 = me->f21c;
    for (i = 0; i < 18; i++)
    {
        snd[i].f0 = -1;
        snd[i].id = -1;
    }
    me->f_e105c0 = me->opts[13].b;
    me->noteCount += sumTracks();
    f3c8 = me->noteCount * 300;
}

extern int  g_modeIndex;          /* 0x1b2e548 (src/modename.cpp) */
extern char g_modeName[256];      /* 0x1b2e680 */
extern int  g_1b2ea28;            /* src/moderunner.cpp */
extern int  g_1b2f028;            /* src/rankingctor.cpp */
extern int  g_useAudiencePanel;   /* 0x1b2eb48 (src/maingame.cpp) */
struct ChordSlot { int start; int prev; int grade; };
extern ChordSlot g_chord[128];    /* 0x37dcdc0 (src/holdtick.cpp) */
extern int  g_chordScore;         /* 0x37dce98 */
struct StageSlot2
{
    char     _pad0[0xe105b8];
    char     koolAll;                  /* 0xe105b8 - every grade a KOOL (RadioMix with the audience) */
    char     _pad3[3];
    int      noteCount;                /* 0xe105bc */
};

/* PlayerSlot::m42e000 @0x42e000 - reset the player's stage state: the chord
   bookkeeping on for a two-player 7-key mode (its table wiped), the
   per-judge counters and the note count cleared; then by the director's
   mode: mode 0 fills the life (100) on a fresh run or outside the four
   radio modes - inside them only when the option says - and clears the
   combo; mode 1 on a fresh run zeroes the life, sets the second gauge to 5
   and clears the combo; no director does the same; f1e4 takes the global.
   RadioMix with the audience panel forces every grade to KOOL.
   45.56%: structurally exact; cl gives the constant 1 the callee-saved ebx
   (five uses) and spells the zeros as eax, where the original keeps ZERO in
   ebx and 1 as immediates - the constant-register coin (a named zero and a
   byte store through the flag both leave it). */
/* 45.56%, and it is the constant-register coin INVERTED: cl materializes 1
   in ebx and spells `cmp [g_modeIndex],ebx` and `[esi+0x214] = ebx` against
   it, where the original uses immediates for both and gives ebx to ZERO.
   88 instructions against 92.

   Checked before believing the coin, per the CLAUDE.md rule that a constant
   register cl declines is more often a semantic error in the constants: the
   original really does `call countListA; cmp eax,2` and `cmpl $1,g_modeIndex`
   and `movl $1,[esi+0x214]`, so every constant here is read correctly. It is
   not a misreading.

   No frame on either side, so the frame-derivation method that fixed
   m425250 and the chart loaders has nothing to work with. */
// FUNCTION: EZ2AC_UNPACKED 0x0042e000
void PlayerSlot::m42e000()
{
    StageSlot2 *me = (StageSlot2 *)this;

    if (countListA() == 2 && g_modeIndex == 1)
    {
        f214 = 1;
        memset(g_chord, 0, 0x36 * 4);
        g_chordScore = 0;
    }
    else
    {
        f214 = 0;
    }
    memset(f1c4, 0, sizeof(f1c4));
    me->noteCount = 0;
    if (f3d0 != 0)
    {
        if (((StageDirector *)f3d0)->f794 == 0)
        {
            int mode = g_modeIndex;

            if (g_elemCount == 0)
            {
                gauge = 100.0f;
                f1c0 = 0;
                f1dc = 0;
                f1e0 = 0;
            }
            else if (mode == 7 || mode == 6 || mode == 8 || mode == 9)
            {
                if (g_1b2ea28 != 0)
                    gauge = 100.0f;
                f1e4 = 0;
            }
            else
            {
                gauge = 100.0f;
                f1e4 = 0;
                f1c0 = 0;
                f1dc = 0;
                f1e0 = 0;
            }
        }
        else if (((StageDirector *)f3d0)->f794 == 1)
        {
            if (g_elemCount == 0)
            {
                gauge = 0.0f;
                gauge2 = 5.0f;
                f1c0 = 0;
                f1dc = 0;
            }
            f1e4 = 0;
            f1e0 = 0;
        }
    }
    else
    {
        gauge = 0.0f;
        gauge2 = 5.0f;
        f1e4 = 0;
        f1c0 = 0;
        f1dc = 0;
        f1e0 = 0;
    }
    f1e4 = g_1b2f028;
    if (_stricmp(g_modeName, "RadioMix") == 0 && g_useAudiencePanel == 1)
        me->koolAll = 1;
    else
        me->koolAll = 0;
}
