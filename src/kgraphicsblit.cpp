/* WIP: the corners are COMPILER TEMPORARIES, not named locals - the whole
   register cascade turned on that. A named local has a home stack slot, so
   cl spills it in preference to a temporary that would need a new slot:
   with `float y0..y3` declared, two of the four y's went to [esp+0x14] /
   [esp+0x18] and every xmm number downstream shifted. Storing the corner
   into the vertex and reading it back (`g_blitVerts[i].y`, forwarded by cl)
   makes all four temporaries, and they land in the original's xmm1..xmm4;
   the same change on x0 gives the original's shared [esp+0x18] x-slot.
   70.34% -> 88.62% structural (scratchpad/blitdiff.py).
   Residue (86.49% reccmp): the original loads xoff into xmm7 at vertex 0
   and keeps it; we fold it out of [esp+0x1c] there and only registerize it
   at vertex 1 - one vertex late. It is a chicken-and-egg in the allocator:
   with xoff in xmm7 the original has to spill each x to its one [esp+0x18]
   slot, while we have xmm7 free for the x copy and so never need the slot.
   yoff behaves identically on both sides (folded at v0/v1, registerized at
   v2). Tried and rejected: commuting the y expression to `y*c + x*s`
   (identical codegen), computing both coordinates into nx/ny temporaries
   before storing (81.16%), volatile x locals (70.15%), /Op (no change).
   Store maps: scratchpad/fn401d80_stores.txt.

   The video-frame blit @0x401d80 - the engine's biggest leaf: build the
 * four-vertex two-stage quad for the current and next video frame, rotate
 * it by the descriptor's angle through the 1024-entry sine/cosine tables,
 * set the blend pair, bias by the D3D half texel, and draw the strip.
 *
 * Its own TU so KGraphics::m4080c0's body stays out of sight - the
 * original calls it, and a visible body would invite the inliner. */

#include "ez2acapp.h"
#include "icom.h"

/* The blit's transformed-and-lit vertex, FVF 0x244: position, rhw, one
   diffuse color, and two texture coordinate pairs. */
struct BlitVert
{
    float x;
    float y;
    float z;
    float rhw;
    int   color;      /* 0x10 */
    float u0;         /* 0x14 */
    float v0;         /* 0x18 */
    float u1;         /* 0x1c */
    float v1;         /* 0x20 */
};

// GLOBAL: EZ2AC_UNPACKED 0x004a5108
BlitVert g_blitVerts[4];

/* The quarter-degree sine/cosine ladders the blit rotates with - the same
   pair Obj40BBB0::rotateZ uses, so the storage and the GLOBAL pins live in
   src/globals.cpp and this TU only refers to them. Declaring them a second
   time here made TWO entities at 0x4a5228: reccmp dropped one, our operand
   pointed at our own copy, and the blit's four table reads mismatched with
   every byte right. */
extern float g_sinTable[1024];   /* 0x4a5228 (src/globals.cpp) */
extern float g_cosTable[1024];   /* 0x4a6228 (src/globals.cpp) */

extern int g_d36cb0;   /* 0x00d36cb0 (src/maingame.cpp) */


// FUNCTION: EZ2AC_UNPACKED 0x00401d80
void KGraphics::m401d80(KTexture **frames, int count, VideoBlit *d,
                        float xoff, float yoff)
{
    int cur = (int)d->frame;
    int next = cur + 1;
    int a;
    unsigned int color;
    float s;
    float c;

    if (next >= count)
        next = 0;
    {
        KTexture *ta = frames[cur];
        KTexture *tb = frames[next];

        if (ta != 0 && tb != 0)
        {
        m4080c0(0, ta);
        m4080c0(1, tb);
        m408130(d->stageMode);
        color = (int)(double)d->ca;
        color = color << 8 | (int)(double)d->cr;
        color = color << 8 | (int)(double)d->cg;
        color = color << 8 | (int)(double)d->cb;
        if (g_d36cb0 == 0)
        {
            if (d->xb > d->xa)
            {
                g_blitVerts[1].x = d->xa;
                g_blitVerts[1].u0 = d->u0;
                g_blitVerts[3].x = d->xb;
                g_blitVerts[3].u0 = d->u0 + d->uw0;
                g_blitVerts[0].x = d->xd;
                g_blitVerts[0].u0 = d->u0;
                g_blitVerts[2].x = d->xc;
                g_blitVerts[2].u0 = d->u0 + d->uw0;
                g_blitVerts[1].u1 = d->u1;
                g_blitVerts[3].u1 = d->u1 + d->uw1;
                g_blitVerts[0].u1 = d->u1;
                g_blitVerts[2].u1 = d->u1 + d->uw1;
            }
            else
            {
                g_blitVerts[1].x = d->xb;
                g_blitVerts[1].u0 = d->u0 + d->uw0;
                g_blitVerts[3].x = d->xa;
                g_blitVerts[3].u0 = d->u0;
                g_blitVerts[0].x = d->xc;
                g_blitVerts[0].u0 = d->u0 + d->uw0;
                g_blitVerts[2].x = d->xd;
                g_blitVerts[2].u0 = d->u0;
                g_blitVerts[1].u1 = d->u1 + d->uw1;
                g_blitVerts[3].u1 = d->u1;
                g_blitVerts[0].u1 = d->u1 + d->uw1;
                g_blitVerts[2].u1 = d->u1;
            }
            if (d->yc > d->yb)
            {
                g_blitVerts[3].y = d->yb;
                g_blitVerts[3].v0 = d->v0;
                g_blitVerts[2].y = d->yc;
                g_blitVerts[2].v0 = d->v0 + d->vh0;
                g_blitVerts[1].y = d->ya;
                g_blitVerts[1].v0 = d->v0;
                g_blitVerts[0].y = d->yd;
                g_blitVerts[0].v0 = d->v0 + d->vh0;
                g_blitVerts[3].v1 = d->v1;
                g_blitVerts[2].v1 = d->v1 + d->vh1;
                g_blitVerts[1].v1 = d->v1;
                g_blitVerts[0].v1 = d->v1 + d->vh1;
            }
            else
            {
                g_blitVerts[3].y = d->yc;
                g_blitVerts[3].v0 = d->v0 + d->vh0;
                g_blitVerts[2].y = d->yb;
                g_blitVerts[2].v0 = d->v0;
                g_blitVerts[1].y = d->yd;
                g_blitVerts[1].v0 = d->v0 + d->vh0;
                g_blitVerts[0].y = d->ya;
                g_blitVerts[0].v0 = d->v0;
                g_blitVerts[3].v1 = d->v1 + d->vh1;
                g_blitVerts[2].v1 = d->v1;
                g_blitVerts[1].v1 = d->v1 + d->vh1;
                g_blitVerts[0].v1 = d->v1;
            }
        g_blitVerts[0].color = color;
            g_blitVerts[1].color = color;
            g_blitVerts[2].color = color;
            g_blitVerts[3].color = color;
            a = (int)d->angle;
            if (a < 0)
                s = 0.0f - g_sinTable[-a & 0x3ff];
            else
                s = g_sinTable[a & 0x3ff];
            if (a < 0)
                a = -a;
            c = g_cosTable[a & 0x3ff];
            {
                float x0 = g_blitVerts[0].x;

                g_blitVerts[0].x = x0 * c - g_blitVerts[0].y * s + d->x + xoff;
                g_blitVerts[0].y = x0 * s + g_blitVerts[0].y * c + d->y + yoff;
            }
            {
                float x1 = g_blitVerts[1].x;

                g_blitVerts[1].x = x1 * c - g_blitVerts[1].y * s + d->x + xoff;
                g_blitVerts[1].y = x1 * s + g_blitVerts[1].y * c + d->y + yoff;
            }
            {
                float x2 = g_blitVerts[2].x;

                g_blitVerts[2].x = x2 * c - g_blitVerts[2].y * s + d->x + xoff;
                g_blitVerts[2].y = x2 * s + g_blitVerts[2].y * c + d->y + yoff;
            }
            {
                float x3 = g_blitVerts[3].x;

                g_blitVerts[3].x = x3 * c - g_blitVerts[3].y * s + d->x + xoff;
                g_blitVerts[3].y = x3 * s + g_blitVerts[3].y * c + d->y + yoff;
            }
        }
        else
        {
            if (d->xb > d->xa)
            {
                g_blitVerts[1].x = d->xa;
                g_blitVerts[1].u0 = d->u0;
                g_blitVerts[3].x = d->xb;
                g_blitVerts[3].u0 = d->u0 + d->uw0;
                g_blitVerts[0].x = d->xd;
                g_blitVerts[0].u0 = d->u0;
                g_blitVerts[2].x = d->xc;
                g_blitVerts[2].u0 = d->u0 + d->uw0;
                g_blitVerts[1].u1 = d->u1;
                g_blitVerts[3].u1 = d->u1 + d->uw1;
                g_blitVerts[0].u1 = d->u1;
                g_blitVerts[2].u1 = d->u1 + d->uw1;
            }
            else
            {
                g_blitVerts[1].x = d->xb;
                g_blitVerts[1].u0 = d->u0 + d->uw0;
                g_blitVerts[3].x = d->xa;
                g_blitVerts[3].u0 = d->u0;
                g_blitVerts[0].x = d->xc;
                g_blitVerts[0].u0 = d->u0 + d->uw0;
                g_blitVerts[2].x = d->xd;
                g_blitVerts[2].u0 = d->u0;
                g_blitVerts[1].u1 = d->u1 + d->uw1;
                g_blitVerts[3].u1 = d->u1;
                g_blitVerts[0].u1 = d->u1 + d->uw1;
                g_blitVerts[2].u1 = d->u1;
            }
            if (d->yc > d->yb)
            {
                g_blitVerts[3].y = d->yb;
                g_blitVerts[3].v0 = d->v0 + d->vh0;
                g_blitVerts[2].y = d->yc;
                g_blitVerts[2].v0 = d->v0;
                g_blitVerts[1].y = d->ya;
                g_blitVerts[1].v0 = d->v0 + d->vh0;
                g_blitVerts[0].y = d->yd;
                g_blitVerts[0].v0 = d->v0;
                g_blitVerts[3].v1 = d->v1 + d->vh1;
                g_blitVerts[2].v1 = d->v1;
                g_blitVerts[1].v1 = d->v1 + d->vh1;
                g_blitVerts[0].v1 = d->v1;
            }
            else
            {
                g_blitVerts[3].y = d->yc;
                g_blitVerts[3].v0 = d->v0;
                g_blitVerts[2].y = d->yb;
                g_blitVerts[2].v0 = d->v0 + d->vh0;
                g_blitVerts[1].y = d->yd;
                g_blitVerts[1].v0 = d->v0;
                g_blitVerts[0].y = d->ya;
                g_blitVerts[0].v0 = d->v0 + d->vh0;
                g_blitVerts[3].v1 = d->v1;
                g_blitVerts[2].v1 = d->v1 + d->vh1;
                g_blitVerts[1].v1 = d->v1;
                g_blitVerts[0].v1 = d->v1 + d->vh1;
            }
        g_blitVerts[0].color = color;
            g_blitVerts[1].color = color;
            g_blitVerts[2].color = color;
            g_blitVerts[3].color = color;
            a = (int)d->angle;
            if (a < 0)
                s = 0.0f - g_sinTable[-a & 0x3ff];
            else
                s = g_sinTable[a & 0x3ff];
            if (a < 0)
                a = -a;
            c = g_cosTable[a & 0x3ff];
            {
                float x0 = g_blitVerts[0].x;

                g_blitVerts[0].x = x0 * c - g_blitVerts[0].y * s + d->x + xoff;
                g_blitVerts[0].y = 479.0f - (x0 * s + g_blitVerts[0].y * c + d->y + yoff);
            }
            {
                float x1 = g_blitVerts[1].x;

                g_blitVerts[1].x = x1 * c - g_blitVerts[1].y * s + d->x + xoff;
                g_blitVerts[1].y = 479.0f - (x1 * s + g_blitVerts[1].y * c + d->y + yoff);
            }
            {
                float x2 = g_blitVerts[2].x;

                g_blitVerts[2].x = x2 * c - g_blitVerts[2].y * s + d->x + xoff;
                g_blitVerts[2].y = 479.0f - (x2 * s + g_blitVerts[2].y * c + d->y + yoff);
            }
            {
                float x3 = g_blitVerts[3].x;

                g_blitVerts[3].x = x3 * c - g_blitVerts[3].y * s + d->x + xoff;
                g_blitVerts[3].y = 479.0f - (x3 * s + g_blitVerts[3].y * c + d->y + yoff);
            }
        }
        device->SetRenderState(0x13, d->blendSrc);
        device->SetRenderState(0x14, d->blendDst);
        g_blitVerts[0].x = g_blitVerts[0].x - 0.5f;
        g_blitVerts[0].y = g_blitVerts[0].y - 0.5f;
        g_blitVerts[1].x = g_blitVerts[1].x - 0.5f;
        g_blitVerts[1].y = g_blitVerts[1].y - 0.5f;
        g_blitVerts[2].x = g_blitVerts[2].x - 0.5f;
        g_blitVerts[2].y = g_blitVerts[2].y - 0.5f;
        g_blitVerts[3].x = g_blitVerts[3].x - 0.5f;
        g_blitVerts[3].y = g_blitVerts[3].y - 0.5f;
        device->DrawPrimitive(5, 0x244, g_blitVerts, 4, 0);
        }
    }
}
