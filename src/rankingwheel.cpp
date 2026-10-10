/* RankingDirector::m459bc0 @0x459bc0 - the name-entry overlay. In
   name entry proper (g_1b2f024) and again for a one-player field, the
   eight big glyphs of the name so far draw 64x64 across the top - the
   place clamped to 99 past the wrap - and the glyph WHEEL scrolls
   vertically beside the cursor column, the picked glyph brighter; the
   two copies differ only in their x base (159/34) and how much of the
   wheel is visible (150/120). A two-up field instead draws each
   player's live cursor glyph small and pink at its own column and
   re-arms the m_cbc chart with the nudge and the cursor offset.

   Its own TU: the tween tick and the draws live elsewhere. */
#include "ezscreen.h"
#include "ez2acapp.h"
#include "directors.h"

extern int countListA();            /* 0x418310 (src/nodelists.cpp) */
extern int g_1b2f024;               /* src/moderunner.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x00459bc0
void RankingDirector::m459bc0()
{
    int   i, g, sel;
    float x, pos, y;

    if (g_1b2f024 != 0)
    {
        x = 159.0f;
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        /* The original never initialises g: cl gives it x's stack home, so
           it starts as the BITS of x (`mov ebx,[esp+0x10]`). Spelled out
           here rather than left undefined; any such value is >= 50 and the
           clamp below turns it into 0, which is what an empty slot draws. */
        g = *(int *)&x;
        for (i = 0; i < 8; i++)
        {
            if (f85df8 != 0)
            {
                if (m_names2[f85dfc * 50 + i] != 0)
                {
                    if (f85dfc >= 10000)
                        f85dfc = 99;
                    g = m_glyphMap[m_names2[f85dfc * 50 + i]];
                }
            }
            else
            {
                if (m_names1[f109b48 * 50 + i] != 0)
                {
                    if (f109b48 >= 10000)
                        f109b48 = 99;
                    g = m_glyphMap[m_names1[f109b48 * 50 + i]];
                }
            }
            if (g < 0 || g >= 50)
                g = 0;
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x, 221.0f,
                                                 64.0f, 64.0f, 0xffffffff);
            x += 40.0f;
        }
        x = (float)(f109db0[f109de4] * 40) + 159.0f;
        f109b34 = x - 10.0f;
        f109b38 = 209.0f;
        m_152c.f214 = x - 10.0f;
        m_152c.f218 = 209.0f;
        for (g = 0; g < 50; g++)
        {
            pos = (float)g * 100.0f - f109dc0[f109de4];
            if (pos > 2500.0f)
                pos -= 5000.0f;
            if (-2500.0f > pos)
                pos += 5000.0f;
            if (150.0f > pos && pos > -50.0f)
            {
                if (g == f109da8[f109de4])
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x,
                                                         pos * 0.45f + 221.0f,
                                                         64.0f, 64.0f,
                                                         0xff646464);
                else
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x,
                                                         pos * 0.45f + 221.0f,
                                                         64.0f, 64.0f,
                                                         0xff323232);
            }
        }
        return;
    }
    if (countListA() == 1)
    {
        x = 34.0f;
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        /* The original never initialises g: cl gives it x's stack home, so
           it starts as the BITS of x (`mov ebx,[esp+0x10]`). Spelled out
           here rather than left undefined; any such value is >= 50 and the
           clamp below turns it into 0, which is what an empty slot draws. */
        g = *(int *)&x;
        for (i = 0; i < 8; i++)
        {
            if (f85df8 != 0)
            {
                if (m_names2[f85dfc * 50 + i] != 0)
                {
                    if (f85dfc >= 10000)
                        f85dfc = 99;
                    g = m_glyphMap[m_names2[f85dfc * 50 + i]];
                }
            }
            else
            {
                if (m_names1[f109b48 * 50 + i] != 0)
                {
                    if (f109b48 >= 10000)
                        f109b48 = 99;
                    g = m_glyphMap[m_names1[f109b48 * 50 + i]];
                }
            }
            if (g < 0 || g >= 50)
                g = 0;
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x, 221.0f,
                                                 64.0f, 64.0f, 0xffffffff);
            x += 40.0f;
        }
        x = (float)(f109db0[f109de4] * 40) + 34.0f;
        f109b34 = x - 10.0f;
        f109b38 = 209.0f;
        m_152c.f214 = x - 10.0f;
        m_152c.f218 = 209.0f;
        for (g = 0; g < 50; g++)
        {
            pos = (float)g * 100.0f - f109dc0[f109de4];
            if (pos > 2500.0f)
                pos -= 5000.0f;
            if (-2500.0f > pos)
                pos += 5000.0f;
            if (120.0f > pos && pos > -50.0f)
            {
                if (g == f109da8[f109de4])
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x,
                                                         pos * 0.45f + 221.0f,
                                                         64.0f, 64.0f,
                                                         0xff646464);
                else
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[g], x,
                                                         pos * 0.45f + 221.0f,
                                                         64.0f, 64.0f,
                                                         0xff323232);
            }
        }
        return;
    }
    /* Every per-player array is read as `(&field)[i]` with no `place`
       local: that is what anchors cl's walking pointer at &f85dfc[i] with
       the others as displacements off it, as the original does. What is
       left (91.81%): cl merges the two f85df8 arms right after the first
       load, where the original's arms differ by a temporary register
       (eax/edx) and only meet at the drawTex push - and it anchors the
       m_cbc stores at f214 where the original anchors at f218. */
    if (countListA() == 2)
    {
        int bx;

        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        for (bx = 215; bx < 855; bx += 320)
        {
            if (f85df8 != 0)
            {
                if ((&f85dfc)[i] < 99 && (&f109dd0)[i] == 0)
                {
                    f109db8[i] = 0.0f;
                    if ((&f85dfc)[i] < 3)
                        f109db8[i] = (3.0f - (float)(&f85dfc)[i]) * -46.0f;
                    if ((&f85dfc)[i] > 0x270b)
                        f109db8[i] = ((float)(&f85dfc)[i] - 9995.0f) * 46.0f;
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[f109da8[i]],
                                                         (float)f109db0[i] * 16.0f +
                                                         (float)bx,
                                                         f109db8[i] + 266.0f,
                                                         16.0f, 16.0f,
                                                         0xff9b9bff);
                    m_cbc[i].f218 = f109db8[i];
                    m_cbc[i].f214 = (float)f109db0[i] * 16.0f - (float)i;
                    m_cbc[i].m403940();
                }
            }
            else
            {
                if ((&f109b48)[i] < 99 && (&f109dd0)[i] == 0)
                {
                    f109db8[i] = 0.0f;
                    if ((&f109b48)[i] < 3)
                        f109db8[i] = (3.0f - (float)(&f109b48)[i]) * -46.0f;
                    if ((&f109b48)[i] > 0x270b)
                        f109db8[i] = ((float)(&f109b48)[i] - 9995.0f) * 46.0f;
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_bigGlyphs[f109da8[i]],
                                                         (float)f109db0[i] * 16.0f +
                                                         (float)bx,
                                                         f109db8[i] + 266.0f,
                                                         16.0f, 16.0f,
                                                         0xff9b9bff);
                    m_cbc[i].f218 = f109db8[i];
                    m_cbc[i].f214 = (float)f109db0[i] * 16.0f - (float)i;
                    m_cbc[i].m403940();
                }
            }
            i++;
        }
    }
}
