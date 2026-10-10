/* GfxText::m40aaa0 @0x40aaa0 - the sprite-font number drawer the whole
   family funnels into: sprintf the float through the caller's format,
   right-shift short strings one cell, then walk the text - digits index
   the caller's texture table, a '.' takes slot ten and pulls the cursor
   in by `step` on both sides, anything else just advances - drawing
   each glyph, or, when the texture has no window yet, unbinding stage
   0 and filling the cell with a solid rect instead.

   Its own TU: gfxtext.cpp's forwarders tail-call this and would inline
   it; the draw callees live in src/kgraphicsquad.cpp. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ez2acapp.h"
#include "gfxtext.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040aaa0
void GfxText::m40aaa0(float x, float y, float w, float h, float adv,
                      float step, float value, const char *fmt, int color,
                      void *tab)
{
    char        buf[128];
    const char *p;
    int         dot;

    sprintf(buf, fmt, (double)value);
    if ((int)strlen(buf) <= 3)
        x += w;
    dot = 0;
    for (p = buf; *p != 0; x += adv, p++)
    {
        int       idx;
        KTexture *tex;

        if (dot != 0)
        {
            x -= step;
            dot = 0;
        }
        if (isdigit(*p) == 0)
        {
            if (*p != '.')
                continue;
            x -= step;
            idx = 10;
            dot = 1;
        }
        else
            idx = *p - '0';
        tex = ((KTexture **)tab)[idx];
        if (tex->f88 != 0)
            ((KGraphics *)this)->drawTex(tex, x, y, w, h, color);
        else
        {
            ((KGraphics *)this)->m4080c0(0, tex);
            ((KGraphics *)this)->drawRect(x, y, x + w, y + h, color);
        }
    }
}

/* GfxText::m40a970 @0x40a970 - the INTEGER flavour: the same walk with
   no short-string shift and no dot handling - non-digits just advance. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a970
void GfxText::m40a970(float x, float y, float w, float h, float adv,
                      int value, const char *fmt, int color, void *tab)
{
    char        buf[128];
    const char *p;

    sprintf(buf, fmt, value);
    for (p = buf; *p != 0; x += adv, p++)
    {
        KTexture *tex;

        if (isdigit(*p) == 0)
            continue;
        tex = ((KTexture **)tab)[*p - '0'];
        if (tex->f88 != 0)
            ((KGraphics *)this)->drawTex(tex, x, y, w, h, color);
        else
        {
            ((KGraphics *)this)->m4080c0(0, tex);
            ((KGraphics *)this)->drawRect(x, y, x + w, y + h, color);
        }
    }
}
