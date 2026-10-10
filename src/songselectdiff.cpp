/* SongSelectDirector::drawDifficulty @0x432930 - one difficulty readout.

   The plate is drawn half a pixel down and right of the given position and
   the level bar squarely on it, both 64x32. Levels below 0x15 index the two
   twenty-one entry runs at +0xc0348 and +0xc03a0; anything at or past 0x15
   uses the single pair at +0xc0400. `lit` picks the bar's tint - bright
   blue when set, grey when not.

   Seven arguments, of which only the level, the two coordinates and `lit`
   are read; the first, fifth and sixth are ignored, which is what the
   original's `ret 0x1c` against four loads says.

   cl reproduces the original's cross-jumping for free: the two arms that
   share a tint share the second draw's argument set-up, and the other two
   build theirs inline. */

#include "songselect.h"
#include "ez2acapp.h"

// FUNCTION: EZ2AC_UNPACKED 0x00432930
void SongSelectDirector::drawDifficulty(int /*a1*/, int level, float x, float y,
                                        int /*a5*/, int /*a6*/, char lit)
{
    /* Read the level BEFORE the blend call - the original loads it into a
       saved register in the prologue, which only happens when the source
       names it ahead of the call. */
    int lv = level;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);

    if (lv >= 0x15)
    {
        if (lit != 0)
        {
            ((KGraphics *)g_sub1b2f054)->drawTex(f_c0404, x + 0.5f, y + 0.5f,
                                                 64.0f, 32.0f, -1);
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(f_c0400, x, y,
                                                 64.0f, 32.0f, 0xff46e1ff);
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->drawTex(f_c0404, x + 0.5f, y + 0.5f,
                                                 64.0f, 32.0f, -1);
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(f_c0400, x, y,
                                                 64.0f, 32.0f, 0xdcdcdcdc);
        }
    }
    else if (lit != 0)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex(t_c03a0[lv], x + 0.5f, y + 0.5f,
                                             64.0f, 32.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(t_c0348[lv], x, y,
                                             64.0f, 32.0f, 0xff46e1ff);
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->drawTex(t_c03a0[lv], x + 0.5f, y + 0.5f,
                                             64.0f, 32.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(t_c0348[lv], x, y,
                                             64.0f, 32.0f, 0xdcdcdcdc);
    }
}
