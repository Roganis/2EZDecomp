/* KGraphics::m402f70 @0x402f70 - the single-texture video blit, the
   two-texture m401d80's sibling (src/kgraphicsblit.cpp documents the
   family's idioms: the corner spill-and-read-back that anchors the
   rotation, the mirrored arm baking the 479-line flip into the y).
   This one keeps its OWN four-vertex block (one uv set, FVF 0x1c4),
   lazily built on first use, remaps the uvs into the texture's
   sub-rect when the atlas carve is armed (tex->f88), and draws with
   the blend pair and a half-pixel bias. */

#include "ezscreen.h"

extern float g_sinTable[1024];   /* 0x4a5228 (src/globals.cpp) */
extern float g_cosTable[1024];   /* 0x4a6228 (src/globals.cpp) */
extern int g_d36cb0;             /* 0x00d36cb0 (src/maingame.cpp) */

/* The single-uv vertex and this blit's own quad + init flag. */
struct Blit2Vert
{
    float x;
    float y;
    float z;
    float rhw;
    int   color;
    int   spec;
    float u0;
    float v0;
};

// GLOBAL: EZ2AC_UNPACKED 0x00530ba8
Blit2Vert g_blit2Verts[4];
// GLOBAL: EZ2AC_UNPACKED 0x00530c28
int g_blit2Init;

// FUNCTION: EZ2AC_UNPACKED 0x00402f70
void KGraphics::m402f70(KTexture **frames, int count, VideoBlit *d,
                        float xoff, float yoff)
{
    KTexture *tex;
    unsigned int color;
    float u0, v0, uw, vh;
    float s, c;
    int a;

    if (!(g_blit2Init & 1))
    {
        g_blit2Init |= 1;
        g_blit2Verts[0].x = 0.0f;
        g_blit2Verts[0].y = 0.0f;
        g_blit2Verts[0].z = 1.0f;
        g_blit2Verts[0].rhw = 0.5f;
        g_blit2Verts[0].color = -1;
        g_blit2Verts[0].spec = 0;
        g_blit2Verts[0].u0 = 0.0f;
        g_blit2Verts[0].v0 = 1.0f;
        g_blit2Verts[1].x = 0.0f;
        g_blit2Verts[1].y = 0.0f;
        g_blit2Verts[1].z = 1.0f;
        g_blit2Verts[1].rhw = 0.5f;
        g_blit2Verts[1].color = -1;
        g_blit2Verts[1].spec = 0;
        g_blit2Verts[1].u0 = 0.0f;
        g_blit2Verts[1].v0 = 0.0f;
        g_blit2Verts[2].x = 0.0f;
        g_blit2Verts[2].y = 0.0f;
        g_blit2Verts[2].z = 1.0f;
        g_blit2Verts[2].rhw = 0.5f;
        g_blit2Verts[2].color = -1;
        g_blit2Verts[2].spec = 0;
        g_blit2Verts[2].u0 = 1.0f;
        g_blit2Verts[2].v0 = 1.0f;
        g_blit2Verts[3].x = 0.0f;
        g_blit2Verts[3].y = 0.0f;
        g_blit2Verts[3].z = 1.0f;
        g_blit2Verts[3].rhw = 0.5f;
        g_blit2Verts[3].color = -1;
        g_blit2Verts[3].spec = 0;
        g_blit2Verts[3].u0 = 1.0f;
        g_blit2Verts[3].v0 = 0.0f;
    }

    tex = frames[(int)d->frame];
    if (tex == 0)
        return;

    m4080c0(0, tex);
    m4080c0(1, 0);
    m408130(d->stageMode);

    color = (int)(double)d->ca;
    color = color << 8 | (int)(double)d->cr;
    color = color << 8 | (int)(double)d->cg;
    color = color << 8 | (int)(double)d->cb;

    u0 = d->u0;
    v0 = d->v0;
    uw = d->uw0;
    vh = d->vh0;
    if (tex->f88 != 0)
    {
        float du = tex->u1 - tex->u0;
        float dv = tex->v1 - tex->v0;

        u0 = u0 * du + tex->u0;
        v0 = v0 * dv + tex->v0;
        uw = uw * du;
        vh = vh * dv;
    }

    if (g_d36cb0 == 0)
    {
        if (d->xb > d->xa)
        {
            g_blit2Verts[1].x = d->xa;
            g_blit2Verts[1].u0 = u0;
            g_blit2Verts[3].x = d->xb;
            g_blit2Verts[3].u0 = u0 + uw;
            g_blit2Verts[0].x = d->xd;
            g_blit2Verts[0].u0 = u0;
            g_blit2Verts[2].x = d->xc;
            g_blit2Verts[2].u0 = u0 + uw;
        }
        else
        {
            g_blit2Verts[1].x = d->xb;
            g_blit2Verts[1].u0 = u0 + uw;
            g_blit2Verts[3].x = d->xa;
            g_blit2Verts[3].u0 = u0;
            g_blit2Verts[0].x = d->xc;
            g_blit2Verts[0].u0 = u0 + uw;
            g_blit2Verts[2].x = d->xd;
            g_blit2Verts[2].u0 = u0;
        }
        if (d->yc > d->yb)
        {
            g_blit2Verts[3].y = d->yb;
            g_blit2Verts[3].v0 = v0;
            g_blit2Verts[2].y = d->yc;
            g_blit2Verts[2].v0 = v0 + vh;
            g_blit2Verts[1].y = d->ya;
            g_blit2Verts[1].v0 = v0;
            g_blit2Verts[0].y = d->yd;
            g_blit2Verts[0].v0 = v0 + vh;
        }
        else
        {
            g_blit2Verts[3].y = d->yc;
            g_blit2Verts[3].v0 = v0 + vh;
            g_blit2Verts[2].y = d->yb;
            g_blit2Verts[2].v0 = v0;
            g_blit2Verts[1].y = d->yd;
            g_blit2Verts[1].v0 = v0 + vh;
            g_blit2Verts[0].y = d->ya;
            g_blit2Verts[0].v0 = v0;
        }
        g_blit2Verts[0].color = color;
        g_blit2Verts[1].color = color;
        g_blit2Verts[2].color = color;
        g_blit2Verts[3].color = color;
        a = (int)d->angle;
        if (a < 0)
            s = 0.0f - g_sinTable[-a & 0x3ff];
        else
            s = g_sinTable[a & 0x3ff];
        if (a < 0)
            a = -a;
        c = g_cosTable[a & 0x3ff];
        {
            float x0 = g_blit2Verts[0].x;

            g_blit2Verts[0].x = x0 * c - g_blit2Verts[0].y * s + d->x + xoff;
            g_blit2Verts[0].y = x0 * s + g_blit2Verts[0].y * c + d->y + yoff;
        }
        {
            float x1 = g_blit2Verts[1].x;

            g_blit2Verts[1].x = x1 * c - g_blit2Verts[1].y * s + d->x + xoff;
            g_blit2Verts[1].y = x1 * s + g_blit2Verts[1].y * c + d->y + yoff;
        }
        {
            float x2 = g_blit2Verts[2].x;

            g_blit2Verts[2].x = x2 * c - g_blit2Verts[2].y * s + d->x + xoff;
            g_blit2Verts[2].y = x2 * s + g_blit2Verts[2].y * c + d->y + yoff;
        }
        {
            float x3 = g_blit2Verts[3].x;

            g_blit2Verts[3].x = x3 * c - g_blit2Verts[3].y * s + d->x + xoff;
            g_blit2Verts[3].y = x3 * s + g_blit2Verts[3].y * c + d->y + yoff;
        }
    }
    else
    {
        if (d->xb > d->xa)
        {
            g_blit2Verts[1].x = d->xa;
            g_blit2Verts[1].u0 = u0;
            g_blit2Verts[3].x = d->xb;
            g_blit2Verts[3].u0 = u0 + uw;
            g_blit2Verts[0].x = d->xd;
            g_blit2Verts[0].u0 = u0;
            g_blit2Verts[2].x = d->xc;
            g_blit2Verts[2].u0 = u0 + uw;
        }
        else
        {
            g_blit2Verts[1].x = d->xb;
            g_blit2Verts[1].u0 = u0 + uw;
            g_blit2Verts[3].x = d->xa;
            g_blit2Verts[3].u0 = u0;
            g_blit2Verts[0].x = d->xc;
            g_blit2Verts[0].u0 = u0 + uw;
            g_blit2Verts[2].x = d->xd;
            g_blit2Verts[2].u0 = u0;
        }
        if (d->yc > d->yb)
        {
            g_blit2Verts[3].y = d->yb;
            g_blit2Verts[3].v0 = v0;
            g_blit2Verts[2].y = d->yc;
            g_blit2Verts[2].v0 = v0 + vh;
            g_blit2Verts[1].y = d->ya;
            g_blit2Verts[1].v0 = v0;
            g_blit2Verts[0].y = d->yd;
            g_blit2Verts[0].v0 = v0 + vh;
        }
        else
        {
            g_blit2Verts[3].y = d->yc;
            g_blit2Verts[3].v0 = v0 + vh;
            g_blit2Verts[2].y = d->yb;
            g_blit2Verts[2].v0 = v0;
            g_blit2Verts[1].y = d->yd;
            g_blit2Verts[1].v0 = v0 + vh;
            g_blit2Verts[0].y = d->ya;
            g_blit2Verts[0].v0 = v0;
        }
        g_blit2Verts[0].color = color;
        g_blit2Verts[1].color = color;
        g_blit2Verts[2].color = color;
        g_blit2Verts[3].color = color;
        a = (int)d->angle;
        if (a < 0)
            s = 0.0f - g_sinTable[-a & 0x3ff];
        else
            s = g_sinTable[a & 0x3ff];
        if (a < 0)
            a = -a;
        c = g_cosTable[a & 0x3ff];
        {
            float x0 = g_blit2Verts[0].x;

            g_blit2Verts[0].x = x0 * c - g_blit2Verts[0].y * s + d->x + xoff;
            g_blit2Verts[0].y = 479.0f - (x0 * s + g_blit2Verts[0].y * c + d->y + yoff);
        }
        {
            float x1 = g_blit2Verts[1].x;

            g_blit2Verts[1].x = x1 * c - g_blit2Verts[1].y * s + d->x + xoff;
            g_blit2Verts[1].y = 479.0f - (x1 * s + g_blit2Verts[1].y * c + d->y + yoff);
        }
        {
            float x2 = g_blit2Verts[2].x;

            g_blit2Verts[2].x = x2 * c - g_blit2Verts[2].y * s + d->x + xoff;
            g_blit2Verts[2].y = 479.0f - (x2 * s + g_blit2Verts[2].y * c + d->y + yoff);
        }
        {
            float x3 = g_blit2Verts[3].x;

            g_blit2Verts[3].x = x3 * c - g_blit2Verts[3].y * s + d->x + xoff;
            g_blit2Verts[3].y = 479.0f - (x3 * s + g_blit2Verts[3].y * c + d->y + yoff);
        }
    }

    device->SetRenderState(0x13, d->blendSrc);
    device->SetRenderState(0x14, d->blendDst);
    g_blit2Verts[0].x = g_blit2Verts[0].x - 0.5f;
    g_blit2Verts[0].y = g_blit2Verts[0].y - 0.5f;
    g_blit2Verts[1].x = g_blit2Verts[1].x - 0.5f;
    g_blit2Verts[1].y = g_blit2Verts[1].y - 0.5f;
    g_blit2Verts[2].x = g_blit2Verts[2].x - 0.5f;
    g_blit2Verts[2].y = g_blit2Verts[2].y - 0.5f;
    g_blit2Verts[3].x = g_blit2Verts[3].x - 0.5f;
    g_blit2Verts[3].y = g_blit2Verts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, g_blit2Verts, 4, 1);
}
