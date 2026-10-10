/* KGraphics::drawText @0x406f50 - lock the back buffer and write a whole
   string into it. A negative lead byte up to 0xca starts a two-byte CJK
   glyph - `second + 94 * lead - 0x4141`, the Wansung page - advancing a
   fixed fifteen; ASCII uses the proportional half-width font. The
   screen's bytes-per-pixel (m_f64 / m_f60) picks the 32-bit family
   (m406d40 / drawAsciiGlyphWide) over the 16-bit one (drawGlyph /
   drawAsciiGlyph); the two halves are the same loop written twice, as
   the original duplicates it.

   Its own TU: the four glyph draws and lockBack live in
   src/kgraphics2.cpp, and drawTextf - drawText's own caller - lives
   there too and must keep its call. */
#include "ez2acapp.h"

extern int g_asciiWidth[128];   /* 0xd36cb8 (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00406f50
void KGraphics::drawText(int x, int y, char *s)
{
    if (lockBack() != 1)
        return;
    if (m_f64 / m_f60 == 4)
    {
        int cx;

        if (*s != 0)
        {
            cx = x;
            do
            {
                char c = *s;

                s++;
                if (c < 0)
                {
                    if ((unsigned char)c <= 0xca)
                    {
                        int c2 = *(unsigned char *)s;
                        int g = c2 + 94 * (unsigned char)c - 0x4141;

                        m406d40(cx, y, g, m_textColor);
                        s++;
                    }
                    cx += 15;
                }
                else
                {
                    drawAsciiGlyphWide(cx, y, (unsigned char)c, m_textColor);
                    cx += g_asciiWidth[(unsigned char)c];
                }
            }
            while (*s != 0);
        }
    }
    else
    {
        int cx;

        if (*s != 0)
        {
            cx = x;
            do
            {
                char c = *s;

                s++;
                if (c < 0)
                {
                    if ((unsigned char)c <= 0xca)
                    {
                        drawGlyph(cx, y,
                                  *(unsigned char *)s + 94 * (unsigned char)c - 0x4141,
                                  m_textColor);
                        s++;
                    }
                    cx += 15;
                }
                else
                {
                    drawAsciiGlyph(cx, y, (unsigned char)c, m_textColor);
                    cx += g_asciiWidth[(unsigned char)c];
                }
            }
            while (*s != 0);
        }
    }
    m_back->Unlock(0);
    m_lockBits = 0;
}
