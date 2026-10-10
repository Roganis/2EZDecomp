/* Panel::m42af30 @0x42af30 - the EZ2DJ panel's tap-note drawer, the
   Panel-family cousin of GFPanel::m4637f0 (src/gfpaneltap.cpp): the same
   unsigned +/-12 catch-up, placement and clip, the edge pair or beam -
   with the body factor 1/14 rather than 1/10, the whole edge block
   skipped in mix style 7, and the clamp against f18d4 - then one plain
   drawTex whose colour comes from a KIND ladder: 0xd the biased fade,
   0xe the plain fade, 0xf a period step off 0xff, 0x10 the fixed
   0xffff6464, and everything else the m4270a0 ramp. Args slot-verified
   the same way as the GFPanel pair.

   Returns 2 above the window, 0 judged out or clipped, 1 drawn. */

#include "ezscreen.h"
#include "gamepanels.h"

extern int g_d36cb0;    /* src/kgraphicsquad.cpp - the quad drawer's mirror */
extern int g_mixStyle;  /* 0x1b2ebc8 */
extern int fadeAlphaBiased(int elapsed, int period);          /* 0x426ff0 */
extern int fadeAlpha(int elapsed, int period);                /* 0x427050 */
extern int periodStep(unsigned int pos, unsigned int steps,
                      unsigned int period);                   /* 0x426fc0 */

/* The beam (as in src/gfpaneltap.cpp): inlined, so its arguments are
   evaluated before the m4080c0 call. */
static __forceinline void drawBeam(float x, float y, float w, float h,
                                   unsigned int c0, unsigned int c1)
{
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->m40a5a0(x, y, w, h, c0, c1);
}

// FUNCTION: EZ2AC_UNPACKED 0x0042af30
int Panel::m42af30(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y, bodyH, yMid, x;
    unsigned int c;

    if (f34 == 1)
    {
        if ((unsigned int)st->start > f28 + 12)
            st->start -= 12;
        else if ((unsigned int)st->start >= f28 - 12)
            st->start = f28;
        else
            st->start += 12;
    }

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    y = t - (48.0f / (float)(int)f2c) * (float)(st->start - (int)f28) *
            st->rate * e->fe8 + e->f08;
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= f38)
        return 0;

    t = y - vh;
    if (f30 == 1)
    {
        if (t >= e->f10 + e->f08)
            return 0;
    }

    /* Mix style 7 draws no edges at all. */
    if (g_mixStyle != 7)
    {
        if (e->f1c != 0 || e->f20 != 0)
        {
            bodyH = e->f10 * e->fe8 * (1.0f / 14.0f);
            yMid = t + vh * 0.5f;
            x = e->ff0 - e->f0c * 0.5f + fe4;
            if (f1c == 0)
            {
                float h3;

                m427bd0(x, yMid, e->f0c, -bodyH, e->f1c, e->f20);
                h3 = bodyH;
                if (yMid + bodyH + 1.0f > f18d4)
                {
                    h3 = f18d4 - (yMid + 1.0f) + 1.0f;
                    if (h3 <= 0.0f)
                        h3 = 0.0f;
                }
                m427bd0(x, yMid + 1.0f, e->f0c, h3, e->f1c, e->f20);
            }
            else if (f1c == 1)
            {
                if (g_d36cb0 == 0)
                    drawBeam(x, t - bodyH, e->f0c, bodyH, e->f20, e->f1c);
                else
                    drawBeam(x, t - bodyH, e->f0c, bodyH, e->f1c, e->f20);
            }
        }
    }

    if (st->kind == 0xd)
        c = (fadeAlphaBiased(st->start - (int)f28, f2c) << 24) | 0xffffff;
    else if (st->kind == 0xe)
        c = (fadeAlpha(st->start - (int)f28, f2c) << 24) | 0xffffff;
    else if (st->kind == 0xf)
    {
        int a = 0xff - periodStep(f28, 0x1fe, f2c);

        if (a < 0)
            a = 0;
        c = (a << 24) | 0xffffff;
    }
    else if (st->kind == 0x10)
        c = 0xffff6464;
    else
        c = m4270a0(f28, st->start);

    ((KGraphics *)g_sub1b2f054)->drawTex(voice,
                                         e->ff0 - voice->w * 0.5f + fe4,
                                         t, 0.0f, vh, c);
    return 1;
}
