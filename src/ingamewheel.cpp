/* InGameSongSelect::m425b40 @0x425b40 - lay the current slot's song wheel
   out: for every entry, wrap its 100px-spaced position around the ring
   (the ring is count*100 long, re-centred into [-50n, +50n)), cull
   outside +/-450, then place it on the ellipse - sine gives the y, cosine
   the x, both scaled by -250 and biased 240, the x mirrored for the
   second player - shrinking (128 - |pos|/2, floor 10) and dimming
   (0xff - |pos| inside +/-100, 0x9b beyond) with distance, and hand it
   to the entry drawer m4258d0. Slots and args walked as usual.

   Matched 2026-10-01 (from 76.75%) by four source facts: the products
   are PARENTHESISED - `(s * -250.0f) + 240.0f` - which is the only
   spelling that keeps cl from turning a multiply by a negative constant
   into `240 - s * 250` (and from folding 240 + 15 into 255); the sine
   pick tests j < 0 first; each arm declares its own locals (the
   original's 0x1c frame gives the two arms separate slots); and the loop
   is `i = 0; guard; k = 0; do..while`, which zeroes k's register inside
   the guard. */

#include "ezscreen.h"
#include "directors.h"

extern int getListAF8();     /* 0x418340 (src/nodelists.cpp) */
extern int g_elemCount;      /* 0x1b2e718 */
extern float g_sinTable[1024];   /* 0x4a5228 (src/globals.cpp) */
extern float g_cosTable[1024];   /* 0x4a6228 (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00425b40
void InGameSongSelect::m425b40()
{
    int i, k;

    i = 0;
    if (m_count[g_elemCount] <= 0)
        return;
    k = 0;
    do
    {
        float pos = (float)k - f3c[g_elemCount];
        int   n = m_count[g_elemCount];

        if (pos > (float)(50 * n))
            pos -= (float)(100 * n);
        if ((float)(-50 * n) > pos)
            pos += (float)(100 * n);
        if (450.0f > pos && pos > -450.0f)
        {
            if (getListAF8() == 0)
            {
                float s, c, size, curve, x, y;
                int   a, j;
                if (100.0f > pos && pos > -100.0f)
                    a = 0xff - abs((int)pos);
                else
                    a = 0x9b;
                size = 128.0f - (float)abs((int)pos / 2);
                if (10.0f > size)
                    size = 10.0f;
                curve = (pos * pos * (-1.0f / 600.0f)) + 128.0f;
                j = (int)(pos * 0.8f);
                if (j < 0)
                    s = -g_sinTable[(int)(pos * -0.8f) & 0x3ff];
                else
                    s = g_sinTable[j & 0x3ff];
                y = (s * -250.0f) + 240.0f;
                j = (int)(pos * 0.6f);
                if (j < 0)
                    j = (int)(pos * -0.6f);
                c = g_cosTable[j & 0x3ff];
                x = ((c * -250.0f) + 240.0f) + f4c + 15.0f;
                m4258d0(i, x, y, curve, size, a);
            }
            else
            {
                float s, c, size, curve, x, y;
                int   a, j;
                if (100.0f > pos && pos > -100.0f)
                    a = 0xff - abs((int)pos);
                else
                    a = 0x9b;
                size = 128.0f - (float)abs((int)pos / 2);
                if (10.0f > size)
                    size = 10.0f;
                curve = (pos * pos * (-1.0f / 600.0f)) + 128.0f;
                j = (int)(pos * 0.8f);
                if (j < 0)
                    s = -g_sinTable[(int)(pos * -0.8f) & 0x3ff];
                else
                    s = g_sinTable[j & 0x3ff];
                y = (s * -250.0f) + 240.0f;
                j = (int)(pos * 0.6f);
                if (j < 0)
                    j = (int)(pos * -0.6f);
                c = g_cosTable[j & 0x3ff];
                x = f4c - ((c * -250.0f) + 240.0f) - 11.0f;
                m4258d0(i, x, y, curve, size, a);
            }
        }
        i++;
        k += 100;
    } while (i < m_count[g_elemCount]);
}
