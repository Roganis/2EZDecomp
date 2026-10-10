/* CatchPanel's note drawers @0x46fbb0 / @0x46fe30 - the family's simplest
   pair: Ez2Catch has no sway offset, no edge lines, no mirror, and its
   clip runs unconditionally. The colour ramp is the catch fade
   m46f3d0, the cursor/period live at +0x250/+0x254, and the grade
   limit is the literal 3. Args slot-verified like the rest
   (src/gfpanelhold.cpp has the method notes). */

#include "ezscreen.h"
#include "gamepanels.h"

/* The hold bar. Returns 2 above the window, 0 judged or clipped, 1 drawn. */
// FUNCTION: EZ2AC_UNPACKED 0x0046fbb0
int CatchPanel::m46fbb0(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y, h, yTop, halfvh, capY, capEnd, q3gate;
    unsigned int c1, c2;

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    /* Only the judged-advance arm - no cursor trailing. */
    if (st->offset != 0)
    {
        int w = st->width;

        st->start += st->offset;
        w -= st->offset;
        if (w < 0)
            w = 0;
        st->width = (unsigned short)w;
    }

    {
        int s0 = st->start;
        int cur = f250;
        int per = f254;

        y = t - (48.0f / (float)per) * (float)(s0 - cur) *
                (e->fe8 * st->rate) + e->f08;
    }
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= 5 && st->f8 >= 5)
        return 0;
    if (st->f7 >= 3 && st->f8 == 0)
        e->ffc = 1;

    h = (48.0f / (float)(int)f254) * (float)st->width * e->fe8 * st->rate;
    yTop = y - h - vh;
    halfvh = vh * 0.5f;
    capY = halfvh + yTop;
    q3gate = halfvh;
    if (h < 0.0f)
        h = 0.0f;
    capEnd = h + capY;

    {
        int s0 = st->start;
        int cur = f250;

        c1 = m46f3d0(cur, s0 + st->width);
        c2 = m46f3d0(cur, s0);
    }

    /* The clip is unconditional. */
    {
        float bottom = e->f10 + e->f08;

        if (yTop >= bottom)
            return 0;
        if (capEnd >= bottom)
            h = bottom - capY;
        if (capEnd >= bottom)
            q3gate = 0.0f;
    }

    {
        float x = e->ff0 - voice->w * 0.5f;

        ((KGraphics *)g_sub1b2f054)->m404660(
            voice, x, yTop, 0.0f, halfvh,
            0.0f, 0.0f, 1.0f, 0.5f, c1);
        if (h > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404950(
                voice, x, capY, 0.0f, h,
                0.0f, 0.5f, 1.0f, 0.0f, c1, c2);
        if (q3gate > 0.0f)
            ((KGraphics *)g_sub1b2f054)->m404660(
                voice, x, capEnd, 0.0f, q3gate,
                0.0f, 0.5f, 1.0f, 0.5f, c2);
    }

    return 1;
}

/* The falling tap. */
// FUNCTION: EZ2AC_UNPACKED 0x0046fe30
int CatchPanel::m46fe30(GdsItem *e, SlotState *st, KTexture *voice, float t)
{
    float vh, y;
    unsigned int c;

    vh = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)voice);

    {
        int s0 = st->start;
        int cur = f250;
        int per = f254;

        y = t - (48.0f / (float)per) * (float)(s0 - cur) *
                (e->fe8 * st->rate) + e->f08;
    }
    if (e->f08 - vh > y)
        return 2;

    if (st->f7 >= 3)
        return 0;

    t = y - vh;
    if (t >= e->f10 + e->f08)
        return 0;

    c = m46f3d0(f250, st->start);
    ((KGraphics *)g_sub1b2f054)->drawTex(voice,
                                         e->ff0 - voice->w * 0.5f,
                                         t, 0.0f, vh, c);
    return 1;
}
