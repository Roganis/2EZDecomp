/* InGameSongSelect::m425ea0 @0x425ea0 - advance the wheel: the dial or
   the turntable keys step the selection (1P forward on the left, 2P
   mirrored), the selection wraps modulo the count, the picked entry's
   name goes to the text slot above the caller's own (and "7aura-hd",
   verbatim, to slot 3 - the original hardcodes it), and the scroll
   eases a fifth of the gap toward selection*100 each tick, taking the
   short way around the ring, snapping inside three pixels, and
   wrapping back into [0, count*100). The two difficulty panels then
   show the picked entry's level, and the tail arms f39a44 (-1, or 0
   in mode 4 with a slot that is not 2). */

#include "ezscreen.h"
#include "directors.h"

extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 */
extern int getListAF8();            /* 0x418340 */
extern int g_demoPlay;              /* 0x1b2ef78 */
extern int g_elemCount;             /* 0x1b2e718 */
extern int g_modeIndex;             /* 0x1b2e548 */
extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00425ea0
void InGameSongSelect::m425ea0()
{
    int   e;
    float d, target;

    if (g_demoPlay == 0)
    {
        if (getListAF8() == 1)
        {
            if (g_bigInstance->accum[0] > 10 ||
                g_bigInstance->getState(0xf) == 2)
                f2c[g_elemCount] += 1;
            if (g_bigInstance->accum[0] < -10 ||
                g_bigInstance->getState(0x10) == 2)
                f2c[g_elemCount] -= 1;
        }
        if (getListAF8() == 0)
        {
            if (g_bigInstance->accum[1] > 10 ||
                g_bigInstance->getState(0x17) == 2)
                f2c[g_elemCount] -= 1;
            if (g_bigInstance->accum[1] < -10 ||
                g_bigInstance->getState(0x18) == 2)
                f2c[g_elemCount] += 1;
        }
    }

    e = g_elemCount;
    f2c[e] = (f2c[e] + m_count[e]) % m_count[e];

    /* g_elemCount re-read after the store, and the default name copied
       through a pointer: the original copies it with a byte loop, which
       is cl's strcpy when it cannot see the literal's length. */
    e = g_elemCount;
    strcpy(g_textSlots[e + 1], m_entry[e][f2c[e]].name);
    {
        // STRING: EZ2AC_UNPACKED 0x0048fde0
        const char *def = "7aura-hd";
        strcpy(g_textSlots[3], def);
    }

    f44[e] = (float)f2c[e] * 100.0f;

    e = g_elemCount;
    d = f44[e] - f3c[e];
    if (d > (float)m_count[e] * 50.0f)
        f3c[e] -= ((float)m_count[e] * 100.0f - d) * 0.2f;
    else if ((float)m_count[e] * -50.0f > d)
        f3c[e] += ((float)m_count[e] * 100.0f + d) * 0.2f;
    else
        f3c[e] += d * 0.2f;

    if (3.0f > d && d > -3.0f)
        f3c[g_elemCount] = f44[g_elemCount];

    e = g_elemCount;
    if (f3c[e] > (float)m_count[e] * 100.0f - 3.0f)
    {
        if (f44[e] == 0.0f)
        {
            f3c[e] = f44[e];
            e = g_elemCount;
        }
    }
    if ((float)m_count[e] * 100.0f < f3c[e])
    {
        f3c[e] -= (float)m_count[e] * 100.0f;
        e = g_elemCount;
    }
    if (0.0f > f3c[e])
    {
        f3c[e] += (float)m_count[e] * 100.0f;
        e = g_elemCount;
    }

    m_39130.setCell(5, 0, (int)m_diff[m_entry[e][f2c[e]].level]);
    e = g_elemCount;
    m_3934c.setCell(5, 0, (int)m_diff[m_entry[e][f2c[e]].level]);

    if (g_modeIndex == 4 && g_elemCount != 2)
        f39a44 = 0;
    else
        f39a44 = -1;
}
