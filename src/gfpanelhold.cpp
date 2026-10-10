/* GFPanel::m463210 @0x463210 - draw one hold note: catch the slot's row up
   to the panel's cursor, place the bar, then the edge lines (flat pair or
   the beam variant by this->f18), and the three-part bar - head cap,
   stretched body, tail cap - mirrored by g_d36cb0. The first of the four
   families' hold drawers; Panel::m42af30 / GFPanel::m4637f0 / CatchPanel's
   pair are edits of this.

   Every argument list below was walked out of the original's frame slots
   (scratchpad slot tracker over 0x463210..0x4637f0); the original reuses
   all four parameter homes as float locals once the parameters die.

   Returns 2 above the window, 0 fully judged or clipped out, 1 drawn. */

#include "ezscreen.h"
#include "gamepanels.h"

extern int g_d36cb0;    /* src/kgraphicsquad.cpp - the quad drawer's mirror */

/* The beam (src/gfpaneltap.cpp has the same helper): inlined, so its
   arguments are evaluated before the m4080c0 call. */
static __forceinline void drawBeam(float x, float y, float w, float h,
                                   unsigned int c0, unsigned int c1)
{
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->m40a5a0(x, y, w, h, c0, c1);
}

// FUNCTION: EZ2AC_UNPACKED 0x00463210
int GFPanel::m463210(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y, h, yTop, halfvh, capY, capEnd, q3gate, bodyH, xq;
    unsigned int cEnd, cStart;

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    /* Catch the row up: a judged advance moves start and shortens the
       width; otherwise trail the panel cursor by at most 12 rows. The
       comparisons are UNSIGNED - the original's jbe/jb. */
    if (st->offset != 0)
    {
        int w = st->width;

        st->start += st->offset;
        w -= st->offset;
        if (w < 0)
            w = 0;
        st->width = (unsigned short)w;
    }
    else if (f30 == 1)
    {
        if ((unsigned int)st->start > f24 + 12)
            st->start -= 12;
        else if ((unsigned int)st->start >= f24 - 12)
            st->start = f24;
        else
            st->start += 12;
    }

    y = t - (48.0f / (float)(int)f28) * (float)(st->start - (int)f24) *
            e->fe8 * st->rate + e->f08;
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= 5 && st->f8 >= 5)
        return 0;
    if (st->f7 >= f34 && st->f8 == 0)
        e->ffc = 1;

    h = (48.0f / (float)(int)f28) * (float)st->width * e->fe8 * st->rate;
    yTop = y - h - vh;
    halfvh = vh * 0.5f;
    capY = halfvh + yTop;
    q3gate = halfvh;
    if (h < 0.0f)
        h = 0.0f;
    capEnd = h + capY;

    {
        int s0 = st->start;
        int cur = f24;

        cEnd = m4607c0(cur, s0 + st->width);
        cStart = m4607c0(cur, s0);
    }

    /* The lower clip when the window ends mid-bar. */
    if (f2c == 1)
    {
        float bottom = e->f10 + e->f08;

        if (yTop >= bottom)
            return 0;
        if (capEnd >= bottom)
            h = bottom - capY;
        if (capEnd >= bottom)
            q3gate = 0.0f;
    }

    xq = e->ff0 - voice->w * 0.5f;
    if (e->f1c != 0 || e->f20 != 0)
    {
        bodyH = e->f10 * e->fe8 * 0.1f;
        if (f18 == 0)
        {
            float y1 = yTop + halfvh * 0.5f;

            ((Panel *)this)->m427bd0(e->ff0 - e->f0c * 0.5f + fe0,
                                     y1, e->f0c, -bodyH, e->f1c, e->f20);
            ((Panel *)this)->m427bd0(e->ff0 - e->f0c * 0.5f + fe0,
                                     y1 + 1.0f, e->f0c, capEnd - yTop,
                                     e->f1c, e->f1c);
            {
                float over = bodyH + capEnd + 1.0f;

                if (over > f1870)
                {
                    float v = bodyH - over + f1870;

                    bodyH = 0.0f;
                    if (v > 0.0f)
                        bodyH = v;
                }
                ((Panel *)this)->m427bd0(e->ff0 - e->f0c * 0.5f + fe0,
                                         capEnd + 1.0f, e->f0c, bodyH,
                                         e->f1c, e->f20);
            }
        }
        else if (f18 == 1)
        {
            /* The beam: unbind stage 0 and one gradient quad, colours
                mirrored with the drawer. */
            if (g_d36cb0 == 0)
                drawBeam(e->ff0 - e->f0c * 0.5f + fe0, yTop - bodyH,
                         e->f0c, bodyH, e->f20, e->f1c);
            else
                drawBeam(e->ff0 - e->f0c * 0.5f + fe0, yTop - bodyH,
                         e->f0c, bodyH, e->f1c, e->f20);
        }
    }

    /* The three-part bar off the voice texture: head cap at yTop, the
       stretched body from capY, the tail cap at capEnd - the mirror
       swaps which half of the cap texture each end samples. */
    if (g_d36cb0 == 0)
    {
        ((KGraphics *)g_sub1b2f054)->m404660(
            voice, xq + fe0, yTop, 0.0f, halfvh,
            0.0f, 0.0f, 1.0f, 0.5f, cEnd);
        if (h > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404950(
                voice, xq + fe0, capY, 0.0f, h,
                0.0f, 0.5f, 1.0f, 0.0f, cEnd, cStart);
        if (q3gate > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe0, capEnd, 0.0f, halfvh,
                0.0f, 0.5f, 1.0f, 0.5f, cStart);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->m404660(
            voice, xq + fe0, yTop, 0.0f, halfvh,
            0.0f, 0.5f, 1.0f, 0.5f, cEnd);
        if (h > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404950(
                voice, xq + fe0, capY, 0.0f, h,
                0.0f, 0.5f, 1.0f, 0.0f, cEnd, cStart);
        if (q3gate > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, xq + fe0, capEnd, 0.0f, halfvh,
                0.0f, 0.0f, 1.0f, 0.5f, cStart);
    }

    return 1;
}
