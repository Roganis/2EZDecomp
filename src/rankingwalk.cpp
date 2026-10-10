/* RankingDirector::m459870 @0x459870 - draw one column of the ranking
   wheel. The scroll t is in pixels, 46 to a row: the first visible row
   and its y fall out of t/46, the top row pins to the list head (t
   under two/three rows) or to place 9992 past the wrap at 459770, and
   rows walk down 46 apart until the bottom edge, the player's own
   place drawn white. The two halves - one field, two - are one source
   written twice with their own bases (189/129), bottom edges (465/451)
   and leftovers; the two-up copy also masks the strips above 129 and
   below 444 and re-arms the wheel chart differently.

   Its own TU: the row drawer lives in src/rankingrow.cpp. */
#include "ezscreen.h"
#include "ez2acapp.h"
#include "directors.h"

extern int countListA();            /* 0x418310 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00459870
void RankingDirector::m459870(float x, float t, int which)
{
    int   sel, row;
    float y, left;

    if (f85df8 != 0)
        sel = (&f85dfc)[which];
    else
        sel = (&f109b48)[which];
    if (countListA() == 1)
    {
        row = (int)t / 46 - 2;
        y = (float)((int)(t * 0.0217391308f) * 46) + (189.0f - t);
        left = 0.0f;
        if (92.0f > t)
        {
            y = 189.0f;
            left = t - 92.0f;
            row = 0;
        }
        if (t > 459770.0f)
        {
            y = 189.0f;
            left = t - 459770.0f;
            row = 0x2708;
        }
        else if (!(465.0f > y))
            goto tail1;
        do
        {
            if (row >= 0 && row < 99)
            {
                if (row == sel)
                    m4574c0(x, y, row, 1, f109b44);
                else
                    m4574c0(x, y, row, 0, f109b44);
            }
            y += 46.0f;
            row++;
        }
        while (465.0f > y);
tail1:
        {
            Obj402860 *ch = &m_884[which];

            ch->f214 = 6.0f;
            ch->f218 = left + 23.0f;
        }
        return;
    }
    if (countListA() == 2)
    {
        row = (int)t / 46 - 3;
        y = (float)((int)(t * 0.0217391308f) * 46) + (129.0f - t);
        left = 0.0f;
        if (138.0f > t)
        {
            y = 129.0f;
            left = t - 138.0f;
            row = 0;
        }
        if (t > 459770.0f)
        {
            y = 129.0f;
            left = t - 459770.0f;
            row = 0x2708;
        }
        else if (!(451.0f > y))
            goto tail2;
        do
        {
            if (row >= 0 && row < 99)
            {
                if (row == sel)
                    m4574c0(x, y, row, 1, f109b44);
                else
                    m4574c0(x, y, row, 0, f109b44);
            }
            y += 46.0f;
            row++;
        }
        while (451.0f > y);
tail2:
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 1);
        ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
        ((KGraphics *)g_sub1b2f054)->drawRect(0.0f, 0.0f, 640.0f, 129.0f, 0);
        ((KGraphics *)g_sub1b2f054)->drawRect(0.0f, 444.0f, 640.0f, 480.0f, 0);
        {
            Obj402860 *ch = &m_884[which];

            ch->f214 = 0.0f;
            ch->f218 = left + 4.0f;
        }
    }
}
