/* RankingDirector::m4574c0 @0x4574c0 - draw one ranking row: the
   four-digit place number (c+1, leading zeros suppressed but the ones
   digit always drawn, 20x37 cells), the dot glyph tucked 5 back, the
   eight-character name (16x16 glyphs through the ASCII map, entries
   >= 50 draw nothing), and the seven-digit score (19x18 cells) on a
   second line 18 below.  f85df8 selects the CHANNEL name/score tables
   over the LOCAL pair.  The row is white when `white` is 1 and
   0x646464 grey otherwise, under the caller's alpha.  Both callers
   live in the row-walker m459870. */

#include "ezscreen.h"
#include "ez2acapp.h"
#include "directors.h"

// FUNCTION: EZ2AC_UNPACKED 0x004574c0
void RankingDirector::m4574c0(float x, float y, int c, int white, int alpha)
{
    const char *row;
    float cx, gx;
    int color, place, i, q, digit, score;

    color = alpha << 24;
    if (white == 1)
        color |= 0xffffff;
    else
        color |= 0x646464;
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    cx = x;
    place = 1000;
    white = 4;
    do {
        q = (c + 1) / place;
        digit = q;
        while (digit % 10 != digit)
            digit = digit % 10;
        if (q > 0 || place == 1)
            ((KGraphics *)g_sub1b2f054)->drawTex(m_placeDigits[digit], cx, y,
                                                 20.0f, 37.0f, color);
        cx += 20.0f;
        place /= 10;
    } while (--white != 0);

    cx -= 5.0f;
    ((KGraphics *)g_sub1b2f054)->drawTex(m_placeDigits[10], cx, y,
                                         20.0f, 37.0f, color);
    cx += 12.0f;
    row = (const char *)this + 50 * c;
    gx = cx;
    for (i = 0; i < 8; i++) {
        if (f85df8 != 0) {
            char ch = row[0x85e04 + i];
            if (ch != 0) {
                int gi = m_glyphMap[ch];
                if (gi < 50)
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_glyphTex[gi],
                                                         gx + 10.0f, y,
                                                         16.0f, 16.0f, color);
            }
        } else {
            char ch = row[0x20cc + i];
            if (ch != 0) {
                int gi = m_glyphMap[ch];
                if (gi < 50)
                    ((KGraphics *)g_sub1b2f054)->drawTex(m_glyphTex[gi],
                                                         gx + 10.0f, y,
                                                         16.0f, 16.0f, color);
            }
        }
        gx += 16.0f;
    }

    y += 18.0f;
    gx = cx;
    place = 1000000;
    if (f85df8 != 0)
        score = m_scores2[c];
    else
        score = m_scores1[c];
    c = 7;
    do {
        digit = score / place;
        if (digit > 10)
            digit = 0;
        ((KGraphics *)g_sub1b2f054)->drawTex(m_scoreDigits[digit],
                                             gx + 10.0f, y,
                                             19.0f, 18.0f, color);
        score -= digit * place;
        gx += 18.0f;
        place /= 10;
    } while (--c != 0);
}
