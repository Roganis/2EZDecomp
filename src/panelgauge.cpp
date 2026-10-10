/* Panel::m429170 @0x429170 - draw the shared life gauge. Sums every
   joined player's life, splits the full span (count*100) into f1c48
   segments, fills as many as the summed life covers, and draws them left to right
   with the last one scaled by the remainder and re-centred. The
   segment texture is picked from the f1c50 run by the song's progress
   (f28/f2c through the f1c4c-entry ramp), the filled-segment count is
   published to g_1b5f2ac, and an empty gauge latches b118 - which,
   fifty ticks of g_1b5f19c later, raises the gauge-over flag
   g_1b2eba4. Skipped entirely in a two-up game with f24 set. */

#include "ezscreen.h"
#include "gamepanels.h"
#include "players.h"

extern int countListA();                    /* 0x418310 */
extern int nextListA(struct ListNode **cur);/* 0x4182e0 */
extern int g_1b5f2ac;                       /* the filled-segment count */
extern int g_1b5f19c;                       /* src/playerslot.cpp */
extern int g_1b2eba4;                       /* the gauge-over flag */

// FUNCTION: EZ2AC_UNPACKED 0x00429170
void Panel::m429170()
{
    ListNode *cur;
    float     span, sum, stepw, dxp, dyp;
    int       seg, idx, count;

    if (f1c28 == 0)
        return;
    if (countListA() > 1 && f24 != 0)
        return;

    cur = 0;
    span = 0.0f;
    sum = 0.0f;
    while (nextListA(&cur) != 0)
    {
        span += 100.0f;
        sum += ((PlayerSlot *)cur)->gauge;
    }

    f1cd0->m403940();
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);

    stepw = span / (float)f1c48;

    /* The texture ramp rides the song's progress. */
    idx = (int)((f28 % f2c) * f1c4c / f2c) - 1;
    if (idx < 0)
        idx = 0;

    count = (int)sum / (int)stepw + 1;
    {
        int   part = (int)sum % (int)stepw;
        float fpart;

        if ((float)(count - 1) * stepw == sum)
            count -= 1;
        if (count > f1c48)
            count = f1c48;
        if ((float)part == 0.0f)
        {
            dxp = f1c40;
            dyp = f1c44;
        }
        else
        {
            fpart = (float)part / stepw + 1.0f;
            dxp = f1c40 * 0.5f * fpart;
            dyp = f1c44 * 0.5f * fpart;
        }
    }

    g_1b5f2ac = count;
    if (count <= 0)
        b118 = 1;
    if (b118 == 1 && g_1b5f19c >= 50)
        g_1b2eba4 = 1;

    if (count > 0)
    {
        float x = f1c38;

        for (seg = count - 1; seg > 0; seg--)
        {
            ((KGraphics *)g_sub1b2f054)->drawTex(f1c50[idx], x, f1c3c,
                                                 f1c40, f1c44, -1);
            x += f1c40;
        }
        ((KGraphics *)g_sub1b2f054)->drawTex(
            f1c50[idx], (f1c40 - dxp) * 0.5f + x,
            (f1c44 - dyp) * 0.5f + f1c3c, dxp, dyp, -1);
    }
}
