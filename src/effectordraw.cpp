/* The effector readout's draw @0x41cb90: three sprite layers, then - for the
   two readout kinds that show a number - one more sprite and a formatted value
   drawn through the sprite-font routines.

   It never touches `this`, but its call site @0x41ce40 does `mov ecx,ebx`
   first, so it IS a member (the same shape as Obj401780::nextInstance and
   TestBookkeeping::deleteRankingFiles). */

#include "ez2acapp.h"
#include "gfxtext.h"
#include "panels.h"   /* Effector */
#include "bigbase.h"  /* Obj40C180, g_bigInstance */

struct KTexture;

/* The block the caller hands over. Offsets are exactly what the function
   reads; the two holes at +0x10 and +0x24 it never touches. */
struct EffectorReadout
{
    int       kind;      /* 0x00 - 0xd draws a float, 0xe an int */
    float     x0;        /* 0x04 */
    float     y0;        /* 0x08 */
    KTexture *tex0;      /* 0x0c */
    int       slot1;     /* 0x10 - the first input slot index */
    int       on1;       /* 0x14 - the layer is drawn only at 2 or more */
    float     x1;        /* 0x18 */
    float     y1;        /* 0x1c */
    KTexture *tex1;      /* 0x20 */
    int       slot2;     /* 0x24 - the second input slot index */
    int       on2;       /* 0x28 */
    float     x2;        /* 0x2c */
    float     y2;        /* 0x30 */
    KTexture *tex2;      /* 0x34 */
    float     baseX;     /* 0x38 - the moving sprite's origin */
    float     y;         /* 0x3c */
    float     stepX;     /* 0x40 - times the level, so a slider */
    KTexture *tex;       /* 0x44 */
    float     nx;        /* 0x48 - where the number goes */
    float     ny;        /* 0x4c */
    float     nw;        /* 0x50 */
    float     nh;        /* 0x54 */
    int       out;       /* 0x58 - the caller's scratch, by address */
};

/* src/panels.cpp / src/maingame.cpp - the two effector levels. */
extern int g_1b2e914;
extern int g_1b2e91c;

/* A global X offset every layer is shifted by. */
// GLOBAL: EZ2AC_UNPACKED 0x037da9d4
float g_drawOriginX;

/* The quarter-step scale ladder the float readout picks from - 0.25, 0.5,
   0.75, 1.0 and on up. It was read as a table of font pointers until
   CatchPanel's readout @0x471170 turned up indexing an identical copy at
   0x49deb8; the values settle it. All four copies are 34 floats (0x88
   bytes: quarter steps to 7.0, then 7.5..9.0, 10, 99) and are indexed by the
   0..0x14 speed options - they were declared [16] until 2026-10-05. */
// GLOBAL: EZ2AC_UNPACKED 0x0048e6c8
float g_scaleLadder[34];

struct EffectorDraw
{
    void drawReadout(EffectorReadout *e);   /* 0x41cb90 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0041cb90
void EffectorDraw::drawReadout(EffectorReadout *e)
{
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)e->tex0,
                                         e->x0 + g_drawOriginX, e->y0,
                                         0.0f, 0.0f, -1);

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    if (e->on1 >= 2)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)e->tex1,
                                             e->x1 + g_drawOriginX, e->y1,
                                             0.0f, 0.0f, -1);
    }
    if (e->on2 >= 2)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)e->tex2,
                                             e->x2 + g_drawOriginX, e->y2,
                                             0.0f, 0.0f, -1);
    }

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);

    if (e->kind == 0xd)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)e->tex,
            ((float)g_1b2e914 * 0.029411765f) * e->stepX + e->baseX + g_drawOriginX,
            e->y, 0.0f, 0.0f, -1);
        ((GfxText *)g_sub1b2f054)->drawNumF(e->nx + g_drawOriginX, e->ny,
                                            e->nw, e->nh, e->nw * 0.2f,
                                            g_scaleLadder[g_1b2e914],
        // STRING: EZ2AC_UNPACKED 0x0048e910
                                            "%02.1f", -1, &e->out);
        return;
    }

    if (e->kind == 0xe)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)e->tex,
            ((float)g_1b2e91c * 0.00390625f) * e->stepX + e->baseX + g_drawOriginX,
            e->y, 0.0f, 0.0f, -1);
        ((GfxText *)g_sub1b2f054)->drawNumI(e->nx + g_drawOriginX, e->ny,
            e->nw, e->nh,
            (int)(((float)g_1b2e91c * 0.00390625f) * 100.0f),
        // STRING: EZ2AC_UNPACKED 0x0048e908
            "%3d", -1, &e->out);
    }
}

/* What the effector reports its readouts' input transitions to; only slot 0
   is ever reached, and it takes a code and the new state. */
struct EffectorReport { virtual void report(int code, int state); };

/* Effector::m41cac0 @0x41cac0 - poll one readout's two input slots and report
   every transition worth reporting.

   Two identical halves, one per slot, differing only in which pair of codes
   they use: the first reports 9 (float readout) or 0xb (int readout), the
   second 10 and 0xc. A float readout only reports on a press (state 2) or on
   a hold that has already run half a second (state 3 and heldFor >= 0x1f4);
   an int readout reports every state unconditionally. Either way the state
   is latched into the readout afterwards, which is what the NEXT frame
   compares against.

   The trailing latch is written once - cl duplicates it into the reporting
   arm by itself, which is where the original's two copies come from.

   100%* EFFECTIVE: cl hoists the second slot index's load ABOVE the first
   half's latch store, where the original loads it after. The two members
   cannot alias so nothing at source level forbids the move; per-half scoping
   of the state local, the documented lever for exactly this, changes
   nothing. One three-byte instruction, three bytes earlier. */
/* One input readout: poll the slot, report the transition with the code for
   this layer, and latch the state through the layer's own pointer - it is
   stored before the next layer's slot is read. */
static __forceinline void pollLayer(Effector *fx, EffectorReadout *e, int *slot,
                                    int *on, int held, int plain)
{
    int v = g_bigInstance->getState(*slot);

    if (e->kind == 0xd)
    {
        if (v == 2 || (v == 3 && g_bigInstance->heldFor(*slot) >= 0x1f4))
            fx->f10->report(held, v);
    }
    else if (e->kind == 0xe)
    {
        fx->f10->report(plain, v);
    }

    *on = v;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041cac0
void Effector::m41cac0(EffectorReadout *e)
{
    pollLayer(this, e, &e->slot1, &e->on1, 9, 0xb);
    pollLayer(this, e, &e->slot2, &e->on2, 0xa, 0xc);
}
