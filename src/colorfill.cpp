/* Two small KGraphics leaves, promoted and derived together.

   m40a6a0 @0x40a6a0 comes from wip/kgraphics-colorfill.cpp at its
   parked 58.54%: a DirectDraw colour fill on the back buffer - zero
   width or height means the whole screen, the flip flag mirrors about
   y = 480, DDBLT_COLORFILL with the colour in the DDBLTFX at +0x50.
   The residue is the rect-store schedule (nine forms swept in the
   park; the asymmetric w-local/h-parameter lever is load-bearing and
   kept).

   setProjection @0x40bb20 builds the perspective matrix in place:
   cos(fov/2) on the two diagonal cells, sin(fov/2) in the w column,
   Q = sin / (1 - near/far) with -Q*near in the translation cell, and
   SetTransform(3) hands it to the device. */
#include "ez2acapp.h"
#include "icom.h"
#include <math.h>
#include <string.h>

extern int g_d36cb0;   /* 0xd36cb0 (src/maingame.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0040a6a0
void KGraphics::m40a6a0(int x, int y, int w, int h, int color)
{
    GRect r;
    DDBltFx fx;
    int cw = w;

    if (cw == 0)
    {
        cw = m_width;
    }
    if (h == 0)
    {
        h = m_height;
    }

    r.left = x;
    r.right = x + cw;
    if (g_d36cb0 == 0)
    {
        r.top = y;
        r.bottom = y + h;
    }
    else
    {
        r.top = 480 - y;
        r.bottom = (480 - y) - h;
    }

    fx.dwFillColor = color;
    fx.dwSize = sizeof(fx);
    m_back->Blt(&r, 0, 0, 0x400, &fx);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040bb20
void KGraphics::setProjection(float nearZ, float farZ, float fov)
{
    float m[16];
    float c, s, q;

    c = (float)cos(fov * 0.5);
    s = (float)sin(fov * 0.5);
    /* q BEFORE the memset (95.00% computed beside its store): the original
       finishes the float divide ahead of the zero fill. scratchpad/perm. */
    q = s / (1.0f - nearZ / farZ);
    memset(m, 0, sizeof(m));
    m[0] = c;
    m[5] = c;
    m[11] = s;
    m[10] = q;
    m[14] = 0.0f - q * nearZ;
    device->SetTransform(3, m);
}
