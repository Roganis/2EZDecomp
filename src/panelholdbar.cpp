/* Panel::m42a630 @0x42a630 - the EZ2DJ panel's hold-note drawer: the
   catch-up (unconditional advance, with the width debited 24 or 12 rows
   by the slot's +0x494 flag), the note kind latched into this->f18, the
   same placement/clip/edge skeleton as the GFPanel hold drawer on
   Panel's field map (body factor 1/14, mix style 7 draws no edges), and
   the three-part bar whose EVERY segment's colour re-runs the kind
   ladder - 0xd the biased fade, 0xe the plain fade, 0xf a period step
   off 0xff, 0x10 the fixed 0xffff6464, else the two m4270a0 ramps with
   cEnd on top. Args slot-verified like the rest of the family.

   Returns 2 above the window, 0 fully judged or clipped out, 1 drawn.

   2026-10-03 (27.04% -> 42.85%): the MIRRORED bar (g_d36cb0 != 0) is
   not the plain bar's twin - it runs the ladder for the head only, for
   kinds 0xd..0xf, drawing that head twice (ladder colour, then the
   ramp); body and tail take the ramps. The C had the full ladder on all
   three, a behaviour difference (docs/PORT-DELTAS.md). Also: the beam
   helper, 48/f2c parenthesised, the width debit as an if/else, and the
   tail y as its own variable (the original keeps h + capY twice). */

#include "ezscreen.h"
#include "gamepanels.h"

extern int g_d36cb0;    /* src/kgraphicsquad.cpp - the quad drawer's mirror */
extern int g_mixStyle;  /* 0x1b2ebc8 */
extern int fadeAlphaBiased(int elapsed, int period);          /* 0x426ff0 */
extern int fadeAlpha(int elapsed, int period);                /* 0x427050 */
extern int periodStep(unsigned int pos, unsigned int steps,
                      unsigned int period);                   /* 0x426fc0 */

/* One ladder pick, repeated per segment as the original lays it. The
   two-colour variant feeds m404950: the ladder kinds use their alpha for
   both ends, the default keeps the two ramps. */
static __forceinline void holdColor2(Panel *p, SlotState *st,
                                     unsigned int rampTop,
                                     unsigned int rampBot,
                                     unsigned int *cT, unsigned int *cB)
{
    if (st->kind == 0xd)
        *cT = *cB = (fadeAlphaBiased(st->start - (int)p->f28, p->f2c) << 24) |
                    0xffffff;
    else if (st->kind == 0xe)
        *cT = *cB = (fadeAlpha(st->start - (int)p->f28, p->f2c) << 24) |
                    0xffffff;
    else if (st->kind == 0xf)
    {
        int a = 0xff - periodStep(p->f28, 0x1fe, p->f2c);

        if (a < 0)
            a = 0;
        *cT = *cB = (a << 24) | 0xffffff;
    }
    else if (st->kind == 0x10)
        *cT = *cB = 0xffff6464;
    else
    {
        *cT = rampTop;
        *cB = rampBot;
    }
}


static __forceinline unsigned int holdColor(Panel *p, SlotState *st, unsigned int ramp)
{
    if (st->kind == 0xd)
        return (fadeAlphaBiased(st->start - (int)p->f28, p->f2c) << 24) |
               0xffffff;
    if (st->kind == 0xe)
        return (fadeAlpha(st->start - (int)p->f28, p->f2c) << 24) | 0xffffff;
    if (st->kind == 0xf)
    {
        int a = 0xff - periodStep(p->f28, 0x1fe, p->f2c);

        if (a < 0)
            a = 0;
        return (a << 24) | 0xffffff;
    }
    if (st->kind == 0x10)
        return 0xffff6464;
    return ramp;
}

/* The beam (as in src/gfpaneltap.cpp): inlined, so its arguments are
   evaluated before the m4080c0 call. */
static __forceinline void drawBeam(float x, float y, float w, float h,
                                   unsigned int c0, unsigned int c1)
{
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->m40a5a0(x, y, w, h, c0, c1);
}

// FUNCTION: EZ2AC_UNPACKED 0x0042a630
int Panel::m42a630(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y, h, yTop, halfvh, capY, capEnd, q3gate, bodyH, xq, tailY, x;
    unsigned int cEnd, cStart;
    int w;

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    /* The catch-up: the judged advance is unconditional here, and the
       width is debited by 24 or 12 rows by the slot's own flag. */
    st->start += st->offset;
    if (slot->f494 != 0)
        w = st->width - st->offset - 0x18;
    else
        w = st->width - st->offset - 0xc;
    if (w < 0)
        w = 0;
    st->width = (unsigned short)w;

    /* The note kind drives the edge style for the rest of the frame. */
    f18 = st->kind;

    y = t - (48.0f / (float)(int)f2c) * (float)(st->start - (int)f28) *
            e->fe8 * st->rate + e->f08;
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= 5 && st->f8 >= 5)
        return 0;
    if (st->f7 >= f38 && st->f8 == 0)
        e->ffc = 1;

    h = (48.0f / (float)(int)f2c) * (float)st->width * e->fe8 * st->rate;
    yTop = y - h - vh;
    halfvh = vh * 0.5f;
    capY = halfvh + yTop;
    q3gate = halfvh;
    if (h < 0.0f)
        h = 0.0f;
    capEnd = h + capY;
    tailY = h + capY;

    cEnd = m4270a0(f28, st->start + st->width);
    cStart = m4270a0(f28, st->start);

    if (f30 == 1)
    {
        float bottom = e->f10 + e->f08;

        if (yTop >= bottom)
            return 0;
        if (capEnd >= bottom)
            h = bottom - capY;
        if (tailY >= bottom)
            q3gate = 0.0f;
    }

    xq = e->ff0 - voice->w * 0.5f;

    if (g_mixStyle != 7)
    {
        if (e->f1c != 0 || e->f20 != 0)
        {
            bodyH = e->f10 * e->fe8 * (1.0f / 14.0f);
            x = e->ff0 - e->f0c * 0.5f + fe4;
            if (f18 == 0)
            {
                float y1 = yTop + halfvh * 0.5f;
                float over;

                m427bd0(x, y1, e->f0c, -bodyH, e->f1c, e->f20);
                m427bd0(x, y1 + 1.0f, e->f0c, capEnd - yTop,
                        e->f1c, e->f1c);
                over = bodyH + capEnd + 1.0f;
                if (over > f18d4)
                {
                    float v = bodyH - over + f18d4;

                    bodyH = 0.0f;
                    if (v > 0.0f)
                        bodyH = v;
                }
                m427bd0(x, capEnd + 1.0f, e->f0c, bodyH, e->f1c, e->f20);
            }
            else if (f18 == 1)
            {
                if (g_d36cb0 == 0)
                    drawBeam(x, yTop - bodyH, e->f0c, bodyH, e->f20, e->f1c);
                else
                    drawBeam(x, yTop - bodyH, e->f0c, bodyH, e->f1c, e->f20);
            }
        }
    }

    /* The bar, each segment's colour through the ladder. */
    if (g_d36cb0 == 0)
    {
        ((KGraphics *)g_sub1b2f054)->m404660(
            voice, xq + fe4, yTop, 0.0f, halfvh,
            0.0f, 0.0f, 1.0f, 0.5f, holdColor(this, st, cEnd));
        if (h > 0.0f)
        {
            unsigned int cT, cB;

            holdColor2(this, st, cEnd, cStart, &cT, &cB);
            ((KGraphics *)g_sub1b2f054)->m404950(
                voice, xq + fe4, capY, 0.0f, h,
                0.0f, 0.5f, 1.0f, 0.0f, cT, cB);
        }
        if (q3gate > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe4, tailY, 0.0f, q3gate,
                0.0f, 0.5f, 1.0f, 0.5f, holdColor(this, st, cStart));
    }
    else
    {
        /* The mirrored bar runs the ladder for the HEAD only, and only for
           kinds 0xd..0xf - which then draw the head cap twice, ladder
           colour first and the plain ramp over it. 0x10 gets no fixed
           colour here, and the body and tail always take the ramps. (The
           C ran the full ladder on all three until 2026-10-03:
           docs/PORT-DELTAS.md.) */
        if (st->kind == 0xd)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe4, yTop, 0.0f, halfvh, 0.0f, 0.5f, 1.0f, 0.5f,
                (fadeAlphaBiased(st->start - (int)f28, f2c) << 24) | 0xffffff);
        else if (st->kind == 0xe)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe4, yTop, 0.0f, halfvh, 0.0f, 0.5f, 1.0f, 0.5f,
                (fadeAlpha(st->start - (int)f28, f2c) << 24) | 0xffffff);
        else if (st->kind == 0xf)
        {
            int a = 0xff - periodStep(f28, 0x1fe, f2c);

            if (a < 0)
                a = 0;
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe4, yTop, 0.0f, halfvh, 0.0f, 0.5f, 1.0f, 0.5f,
                (a << 24) | 0xffffff);
        }
        ((KGraphics *)g_sub1b2f054)->m404660(
            voice, xq + fe4, yTop, 0.0f, halfvh,
            0.0f, 0.5f, 1.0f, 0.5f, cEnd);
        if (h > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404950(
                voice, xq + fe4, capY, 0.0f, h,
                0.0f, 0.5f, 1.0f, 0.0f, cEnd, cStart);
        if (q3gate > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe4, tailY, 0.0f, q3gate,
                0.0f, 0.0f, 1.0f, 0.5f, cStart);
    }

    return 1;
}
