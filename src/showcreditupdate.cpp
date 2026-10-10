/* ShowCredit::update @0x450040 - draw the credit overlay: the mask
   additively, the plate over it, then the three readouts - credits
   (two digits over ten, capped 99), the coin remainder and the coin
   need (both clamped to the ten-step series) - all from the CV2 or the
   classic skin by g_1b2eb6c, at the skin's own pixel biases. The tail
   pumps the m_37c script by g_1b2eadc (0 pumps, 2 pumps in tail
   position). Publishes the credit count to g_1b2e7e4.

   The remainder digit sits at x150 - 4 on the CV2 skin and x150 - 2 on
   the classic one; the C had 1.5 and 2.5 until 2026-10-01
   (docs/PORT-DELTAS.md). Left: the original loads `need` once into a
   register for both the zero test and the divides, where cl tests it in
   memory and reloads it (a `need` local fixes the load and then swaps the
   nd/rem registers - worse). */

#include "ezscreen.h"

extern Obj40C180 *g_bigInstance;   /* 0x1b2e710 */

/* The texture-stage reset, on the graphics singleton's other view (see
   src/backgrounds.cpp). */
struct Obj40BB00 { void doOp7(int v); };
extern int g_1b2eb6c;              /* the CV2-mix flag */
extern int g_1b2eadc;
extern int g_1b2e7e4;              /* the published credit count */

// FUNCTION: EZ2AC_UNPACKED 0x00450040
void ShowCredit::update()
{
    /* The graphics global is re-read for every call, as the original
       does - a cached local costs a callee-saved register and spills rem. */
#define g ((KGraphics *)g_sub1b2f054)

    if (f14 == 0)
        return;

    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    if (g_1b2eb6c != 0)
    {
        g->setBlend(1, 3);
        g->drawTex((KTexture *)cvMaskTex, x, y, 0.0f, 0.0f, -1);
        g->setBlend(2, 2);
        g->drawTex((KTexture *)cvSprite, x, y, 0.0f, 0.0f, -1);
    }
    else
    {
        g->setBlend(1, 3);
        g->drawTex((KTexture *)maskTex, x, y, 0.0f, 0.0f, -1);
        g->setBlend(2, 2);
        g->drawTex((KTexture *)sprite, x, y, 0.0f, 0.0f, -1);
    }

    if (g_bigInstance->need != 0)
    {
        /* the divisor read into its own local first: cl then re-reads
           need for the remainder and for nd, as the original does
           (reusing nd as the divisor is far worse) */
        int d   = g_bigInstance->need;
        int cr  = g_bigInstance->have / d;
        int rem = g_bigInstance->have % g_bigInstance->need;
        int nd  = g_bigInstance->need;

        g_1b2e7e4 = cr;
        if (nd > 10)
            nd = 10;
        if (rem > 10)
            rem = 10;
        if (cr >= 100)
            cr = 99;

        g->setBlend(2, 2);
        if (g_1b2eb6c != 0)
        {
            /* The CV2 skin sits its digits a pixel high. */
            if (cr >= 10)
            {
                g->drawTex((KTexture *)cvDigits[cr % 10], x130 + 3.0f,
                           y8 - 1.0f, 0.0f, 0.0f, -1);
                cr /= 10;
                g->drawTex((KTexture *)cvDigits[cr % 10], x130 - 6.0f,
                           y8 - 1.0f, 0.0f, 0.0f, -1);
            }
            else
            {
                g->drawTex((KTexture *)cvDigits[cr % 10], x130,
                           y8 - 1.0f, 0.0f, 0.0f, -1);
            }
            g->drawTex((KTexture *)cvSeriesS[rem], x150 - 4.0f,
                       y9 - 1.0f, 0.0f, 0.0f, -1);
            g->drawTex((KTexture *)cvSeriesSb[nd], x168 - 1.0f,
                       y9b - 1.0f, 0.0f, 0.0f, -1);
        }
        else
        {
            if (cr >= 10)
            {
                g->drawTex((KTexture *)digits[cr % 10], x130 + 3.0f,
                           y8, 0.0f, 0.0f, -1);
                cr /= 10;
                g->drawTex((KTexture *)digits[cr % 10], x130 - 6.0f,
                           y8, 0.0f, 0.0f, -1);
            }
            else
            {
                g->drawTex((KTexture *)digits[cr % 10], x130, y8,
                           0.0f, 0.0f, -1);
            }
            g->drawTex((KTexture *)seriesS[rem], x150 - 2.0f, y9,
                       0.0f, 0.0f, -1);
            g->drawTex((KTexture *)seriesSb[nd], x168 - 1.0f, y9b,
                       0.0f, 0.0f, -1);
        }
    }

    if (b10 != 0)
    {
        if (g_1b2eadc == 0)
            m_37c.m403940();
        if (g_1b2eadc == 2)
            m_37c.m403940();
    }
#undef g
}
