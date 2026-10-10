/* The field views PlayerSlot::commit @0x42e620 needs - see wip/sink.cpp.
 *
 * SEPARATE FROM include/players.h ON PURPOSE. Every offset below sits inside a
 * `_pad` run of `PlayerSlot` or `SlotOwner`, and CLAUDE.md's "adding fields to
 * a verified struct breaks matched functions" rule cost five matches the last
 * time it was ignored. Casting through these instead costs no code and cannot
 * disturb anything reccmp has already scored.
 *
 * It is a header rather than a block inside the .cpp only so that
 * port/oracle/oracle_vc.cpp can build a slot and a director to drive the sink
 * with. Nothing in src/ includes it.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef WIP_SINK_VIEW_H
#define WIP_SINK_VIEW_H

/* ---- the two views ------------------------------------------------------- *
 *
 * The offsets this function needs and include/players.h deliberately does not
 * carve out. `SlotOwner` and the EZ2DJMainGameDirector are the same object -
 * `f380`/`f340` is the same track table - so the director fields are reached
 * through the slot's `f3d0` by casting. */

struct SinkDirector
{
    char  _pad0[0x308];
    float gaugeRate[6];        /* 0x308 + grade*4 - see the header on order */
    char  _pad320[0x32c - 0x320];
    float audienceRate[6];     /* 0x32c + grade*4 */
    char  _pad344[0x794 - 0x344];
    int   f794;                /* 0x794 - which audience model */
    char  _pad798[0x7c4 - 0x798];
    /* 0x7c4 - zero means the stage teardown @0x42e360 drops the slot's track
       sink. Carved out of _pad798 WITHOUT moving anything: the pad is split,
       not resized. */
    int   f7c4;
    char  _pad7c8[0xd18 - 0x7c8];
    int   fd18;                /* 0xd18 - "a GOOD breaks the combo" (CV2) */

    void m420570(struct SinkSlot *p);   /* 0x420570 */
};

struct SinkSlot
{
    char  _pad0[0x1c0];
    int   f1c0;                /* 0x1c0 - CV2's "the combo has started" latch */
    int   counts[6];           /* 0x1c4 .. 0x1d8, indexed by the grade 1..5 */
    int   combo;               /* 0x1dc */
    int   maxCombo;            /* 0x1e0 */
    int   score;               /* 0x1e4 */
    char  _pad1e8[0x1f4 - 0x1e8];
    float gauge;               /* 0x1f4 */
    char  _pad1f8[0x21c - 0x1f8];
    float audience;            /* 0x21c */
    char  _pad220[0x3d0 - 0x220];
    SinkDirector *dir;         /* 0x3d0 */
    char  _pad3d4[0xe105bc - 0x3d4];
    int   noteCount;           /* 0xe105bc */
};

#endif /* WIP_SINK_VIEW_H */

/* ---- the scratch game's pair, for PlayerRecord::m465f60 @0x465f60 -------- */
struct GFSinkDirector
{
    char  _pad0[0x2fc];
    float gaugeRate[6];        /* 0x2fc + grade*4 */
    char  _pad314[0x750 - 0x314];
    int   mode;                /* 0x750 - 0 the gauge game, 1 the audience one */

    void m45d860(struct GFSinkSlot *p);   /* 0x45d860 (STUB) */
};

struct GFSinkSlot
{
    char  _pad0[0xc];
    int   fc;                  /* 0x0c - the combo latch */
    int   counts[6];           /* 0x10 .. 0x24, indexed by the grade 1..5 */
    int   combo;               /* 0x28 */
    int   maxCombo;            /* 0x2c */
    int   score;               /* 0x30 */
    char  _pad34[0x38 - 0x34];
    float gauge;               /* 0x38 */
    char  _pad3c[0x54 - 0x3c];
    float audience;            /* 0x54 */
    char  _pad58[0x1e0 - 0x58];
    GFSinkDirector *dir;       /* 0x1e0 */
    char  _pad1e4[0x530 - 0x1e4];
    int   noteCount;           /* 0x530 */
};
