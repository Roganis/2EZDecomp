/* GFPanel::m4637f0 @0x4637f0 - draw one tap note: the same row catch-up
   and placement as the hold drawer beside it (src/gfpanelhold.cpp), then
   the edge pair or the beam, and one plain drawTex of the voice texture
   at its faded colour. Argument lists walked from the frame slots the
   same way as m463210's.

   Returns 2 above the window, 0 judged past the panel grade or clipped
   out, 1 drawn.

   2026-10-03, 50.12% -> 73.68%: the beam is an INLINED helper (its
   arguments are evaluated before the m4080c0 call and ride across it in
   the frame), `48.0f / f28` is parenthesised against cl's reassociation,
   the catch-up's `= f24` arm comes second, the clamp stores 0 first and
   overwrites, and `t` is reused for y - vh (the original stores it in
   t's home). Left: cl still shares the two m40a5a0 call instructions,
   and the packer's slot choice for vh / yMid / bodyH. */

#include "ezscreen.h"
#include "gamepanels.h"

extern int g_d36cb0;    /* src/kgraphicsquad.cpp - the quad drawer's mirror */

/* The beam: untextured, then the two-colour quad. Inlined, so its
   arguments are evaluated BEFORE the m4080c0 call and ride across it in
   the frame - which is what the original does. */
static __forceinline void drawBeam(float x, float y, float w, float h,
                                   unsigned int c0, unsigned int c1)
{
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->m40a5a0(x, y, w, h, c0, c1);
}

// FUNCTION: EZ2AC_UNPACKED 0x004637f0
int GFPanel::m4637f0(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y, bodyH, yMid;
    unsigned int c;

    /* The catch-up trails the panel cursor by at most 12 rows - the
       comparisons unsigned, as in the hold drawer - but a tap has no
       judged-advance arm. */
    if (f30 == 1)
    {
        if ((unsigned int)st->start > f24 + 12)
            st->start -= 12;
        else if ((unsigned int)st->start >= f24 - 12)
            st->start = f24;
        else
            st->start += 12;
    }

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    /* NOTE the factor order: rate before the item's rate - the reverse
       of the hold drawer's spelling, kept as the bytes have it. */
    y = t - (48.0f / (float)(int)f28) * (float)(st->start - (int)f24) *
            st->rate * e->fe8 + e->f08;
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= f34)
        return 0;

    t = y - vh;
    if (f2c == 1)
    {
        if (t >= e->f10 + e->f08)
            return 0;
    }

    if (e->f1c != 0 || e->f20 != 0)
    {
        bodyH = e->f10 * e->fe8 * 0.1f;
        yMid = t + vh * 0.5f;
        if (f18 == 0)
        {
            float h3;

            ((Panel *)this)->m427bd0(e->ff0 - e->f0c * 0.5f + fe0,
                                     yMid, e->f0c, -bodyH,
                                     e->f1c, e->f20);
            h3 = bodyH;
            if (yMid + bodyH + 1.0f > f1870)
            {
                float v = f1870 - (yMid + 1.0f) + 1.0f;

                h3 = 0.0f;
                if (v > 0.0f)
                    h3 = v;
            }
            ((Panel *)this)->m427bd0(e->ff0 - e->f0c * 0.5f + fe0,
                                     yMid + 1.0f, e->f0c, h3,
                                     e->f1c, e->f20);
        }
        else if (f18 == 1)
        {
            if (g_d36cb0 == 0)
                drawBeam(e->ff0 - e->f0c * 0.5f + fe0, t - bodyH,
                         e->f0c, bodyH, e->f20, e->f1c);
            else
                drawBeam(e->ff0 - e->f0c * 0.5f + fe0, t - bodyH,
                         e->f0c, bodyH, e->f1c, e->f20);
        }
    }

    c = m4607c0(f24, st->start);
    ((KGraphics *)g_sub1b2f054)->drawTex(voice,
                                         e->ff0 - voice->w * 0.5f + fe0,
                                         t, 0.0f, vh, c);
    return 1;
}
