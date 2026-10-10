#include <math.h>
#include "ez2acapp.h"
#include "icom.h"

struct QuadVert
{
    float x;
    float y;
    float z;
    float rhw;
    unsigned int diffuse;
    unsigned int specular;
    float u;
    float v;
};

// GLOBAL: EZ2AC_UNPACKED 0x004a5198
QuadVert g_quadVerts[4];

extern int g_d36cb0;

/* KGraphics::drawTex @0x00403f80 - the family's plainest member and its
 * most-called: no crop fractions, no gradient, one colour, and it builds
 * the quad straight in the shared global rather than a stack copy. The
 * texture's own window is used as-is when it has one and 0..1 when it does
 * not, which is the whole difference from the three above.
 *
 * It ends in a TAIL CALL - cl overwrites the six incoming argument slots
 * with DrawPrimitive's own and jumps, which is what a call in return
 * position with a matching stack size compiles to.
 */
// FUNCTION: EZ2AC_UNPACKED 0x00403f80
void KGraphics::drawTex(KTexture *tex, float x, float y, float w, float h,
                        int color)
{
    float u0;
    float v0;
    float u1;
    float v1;
    volatile float b;

    if (tex == 0)
        return;
    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        u0 = tex->u0;
        v0 = tex->v0;
        u1 = tex->u1;
        v1 = tex->v1;
    }
    else
    {
        u0 = 0.0f;
        v0 = 0.0f;
        u1 = 1.0f;
        v1 = 1.0f;
    }
    g_quadVerts[0].x = x;
    g_quadVerts[1].x = x;
    g_quadVerts[2].x = x + w;
    g_quadVerts[3].x = x + w;
    g_quadVerts[2].u = u1;
    g_quadVerts[3].u = u1;
    g_quadVerts[0].u = u0;
    g_quadVerts[1].u = u0;
    g_quadVerts[0].rhw = 1.0f;
    g_quadVerts[1].rhw = 1.0f;
    g_quadVerts[2].rhw = 1.0f;
    g_quadVerts[3].rhw = 1.0f;
    g_quadVerts[0].diffuse = color;
    g_quadVerts[1].diffuse = color;
    g_quadVerts[2].diffuse = color;
    g_quadVerts[3].diffuse = color;
    g_quadVerts[0].specular = color;
    g_quadVerts[1].specular = color;
    g_quadVerts[2].specular = color;
    g_quadVerts[3].specular = color;
    if (g_d36cb0 == 0)
    {
        g_quadVerts[2].y = y + h;
        g_quadVerts[0].y = y + h;
        g_quadVerts[3].y = y;
        g_quadVerts[1].y = y;
        g_quadVerts[2].v = v1;
        g_quadVerts[0].v = v1;
        g_quadVerts[3].v = v0;
        g_quadVerts[1].v = v0;
    }
    else
    {
        b = y + h;
        g_quadVerts[2].y = 479.0f - b;
        g_quadVerts[0].y = 479.0f - b;
        g_quadVerts[3].y = 479.0f - y;
        g_quadVerts[1].y = 479.0f - y;
        g_quadVerts[2].v = v0;
        g_quadVerts[0].v = v0;
        g_quadVerts[3].v = v1;
        g_quadVerts[1].v = v1;
    }
    m4080c0(0, tex);
    m4080c0(1, 0);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, g_quadVerts, 4, 0);
}

/* KGraphics::m404250 @0x00404250 - the family's ROTATED member: the same
 * quad as drawTex, spun by `angle` degrees around the sprite's centre.
 * The half sizes rotate through one cos/sin pair (computed off a single
 * x87 chain), the x corners land before the flip branch, and the flipped
 * arm mirrors around 480 rather than drawTex's 479. The uv writes are
 * duplicated into both arms as the original has them; colour goes to
 * diffuse and specular alike, and stage 1 is explicitly unbound where
 * drawTex leaves it alone. */
// FUNCTION: EZ2AC_UNPACKED 0x00404250
void KGraphics::m404250(Sprite *s, float x, float y, float w, float h,
                        float angle, unsigned int color)
{
    KTexture *tex = (KTexture *)s;
    float u0;
    float v0;
    float u1;
    float v1;
    float rad, c, sn, hw, hh, negw, negh;

    if (tex == 0)
        return;
    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        u0 = tex->u0;
        v0 = tex->v0;
        u1 = tex->u1;
        v1 = tex->v1;
    }
    else
    {
        v0 = 0.0f;
        u0 = 0.0f;
        v1 = 1.0f;
        u1 = 1.0f;
    }
    rad = angle * 0.0174532925f;   /* float(pi/180): 0x3c8efa35 */
    hh = h * 0.5f;
    hw = w * 0.5f;
    c = (float)cos(rad);
    negw = 0.0f - hw;
    negh = 0.0f - hh;
    y = y + hh;
    x = x + hw;
    sn = (float)sin(rad);
    g_quadVerts[0].x = negw * c - hh * sn + x;
    g_quadVerts[1].x = negw * c - negh * sn + x;
    g_quadVerts[2].x = hw * c - hh * sn + x;
    g_quadVerts[3].x = hw * c - negh * sn + x;
    if (g_d36cb0 == 0)
    {
        g_quadVerts[3].y = hw * sn + negh * c + y;
        g_quadVerts[1].u = u0;
        g_quadVerts[0].u = u0;
        g_quadVerts[3].u = u1;
        g_quadVerts[2].u = u1;
        g_quadVerts[2].y = hw * sn + hh * c + y;
        g_quadVerts[2].v = v1;
        g_quadVerts[0].v = v1;
        g_quadVerts[0].y = hh * c + negw * sn + y;
        g_quadVerts[1].y = negh * c + negw * sn + y;
    }
    else
    {
        float base = 479.0f - y;

        g_quadVerts[2].y = base - hw * sn - hh * c;
        g_quadVerts[1].u = u0;
        g_quadVerts[0].u = u0;
        g_quadVerts[3].u = u1;
        g_quadVerts[2].u = u1;
        g_quadVerts[2].v = v1;
        g_quadVerts[0].v = v1;
        g_quadVerts[3].y = base - hw * sn - negh * c;
        g_quadVerts[0].y = base - negw * sn - hh * c;
        g_quadVerts[1].y = base - negw * sn - negh * c;
    }
    g_quadVerts[3].v = v0;
    g_quadVerts[1].v = v0;
    g_quadVerts[3].diffuse = color;
    g_quadVerts[2].diffuse = color;
    g_quadVerts[1].diffuse = color;
    g_quadVerts[0].diffuse = color;
    g_quadVerts[3].specular = color;
    g_quadVerts[2].specular = color;
    g_quadVerts[1].specular = color;
    g_quadVerts[0].specular = color;
    g_quadVerts[3].rhw = 1.0f;
    g_quadVerts[2].rhw = 1.0f;
    g_quadVerts[1].rhw = 1.0f;
    g_quadVerts[0].rhw = 1.0f;
    m4080c0(0, tex);
    m4080c0(1, 0);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, g_quadVerts, 4, 0);
}

/* KGraphics::m404660 @0x00404660 - the SAME quad as m404950 below with ONE
 * colour instead of two: 146 of the two functions' 155 instructions are
 * identical, and the whole difference is that this one writes its single
 * colour into all four vertices' diffuse and specular where the other takes
 * a bottom and a top. Everything else - the w/h defaults, the crop-window
 * fractions, the 479.0f flip, the half-texel bias applied to the shared
 * global quad, the DrawPrimitive - is the same code.
 *
 * That is also what fixes its signature. It was stubbed as
 * `(Sprite *, float, float, int, float, int, float, float, float, uint)`
 * from its call sites, where the two ints are always a literal zero; the
 * body reads them as the width and the u fraction, so they are floats, and
 * `push 0` is what a 0.0f argument compiles to either way.
 */
// FUNCTION: EZ2AC_UNPACKED 0x00404660
void KGraphics::m404660(KTexture *tex, float x, float y, float w, float h,
                        float u, float v, float uw, float vh,
                        unsigned int color)
{
    QuadVert q[4];
    float su;
    float sv;
    float du;
    float dv;
    float ue;
    float ve;
    volatile float b;

    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        float tu = tex->u1 - tex->u0;
        float tv = tex->v1 - tex->v0;

        su = tu * u;
        sv = tv * v;
        su = su + tex->u0;
        sv = sv + tex->v0;
        du = tu * uw;
        dv = tv * vh;
    }
    else
    {
        dv = vh;
        du = uw;
        sv = v;
        su = u;
    }
    q[0].x = x;
    q[1].x = x;
    q[2].x = x + w;
    q[3].x = x + w;
    ue = su + du;
    ve = sv + dv;
    q[0].u = su;
    q[1].u = su;
    q[2].u = ue;
    q[3].u = ue;
    q[0].diffuse = color;
    q[0].specular = color;
    q[2].diffuse = color;
    q[2].specular = color;
    q[1].diffuse = color;
    q[1].specular = color;
    q[3].diffuse = color;
    q[3].specular = color;
    if (g_d36cb0 == 0)
    {
        q[2].y = y + h;
        q[0].y = y + h;
        q[3].y = y;
        q[1].y = y;
        q[2].v = ve;
        q[0].v = ve;
        q[3].v = sv;
        q[1].v = sv;
    }
    else
    {
        b = y + h;
        q[2].y = 479.0f - b;
        q[0].y = 479.0f - b;
        q[3].y = 479.0f - y;
        q[1].y = 479.0f - y;
        q[2].v = sv;
        q[0].v = sv;
        q[3].v = ve;
        q[1].v = ve;
    }
    q[3].rhw = 1.0f;
    q[2].rhw = 1.0f;
    q[1].rhw = 1.0f;
    q[0].rhw = 1.0f;
    m4080c0(0, tex);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, q, 4, 0);
}

/* WIP 62.86% structural (scratchpad/quaddiff.py) - the reading is complete,
   the residue is the xmm numbering: the original keeps w in xmm5 and h in
   xmm7 and copies du/dv into xmm6/xmm4 at the crop branch's exit, where cl
   gives us a tighter assignment with no phi copies at all. 27 orderings of
   the crop branch, the defaults, the common stores, the arms and the rhw
   run were swept (scratchpad/gen_quad.py + sweep.py); this is the best of
   them, and the register-blind score is 84.29%, so what is left really is
   naming rather than shape.

   KGraphics::m404950 @0x404950 - the engine's textured quad with a per-edge
 * colour, and the most-called of the untouched 0x404xxx draw family (15 call
 * sites). It takes a position and a size - either half defaulting to the
 * texture's own when passed zero - plus the sub-rectangle as fractions of
 * whatever crop window the texture carries (tex->f88 set means it is a .txg
 * sub-sprite, so the fractions are mapped into u0..u1/v0..v1), and two
 * colours: the bottom pair of vertices takes one, the top pair the other,
 * each written to BOTH the diffuse and the specular field of FVF 0x1c4
 * (XYZRHW | DIFFUSE | SPECULAR | TEX1, 0x20 bytes a vertex). z is never
 * written - the original leaves it whatever the frame held.
 *
 * The portrait cabinet's flip mirrors every y through 479.0f and swaps the
 * two v fractions, exactly as the video blit @0x401d80 does.
 *
 * THE HALF-TEXEL BIAS IS APPLIED TO THE WRONG QUAD, and that is the
 * original's own bug, not a misreading: the vertices it draws are the local
 * `q`, but the eight `- 0.5f` stores go to the SHARED global quad at
 * 0x4a5198 that the rest of the family draws. Reproduced verbatim.
 *
 * Its own TU: the texture bind m4080c0 is called with `this` reloaded into
 * ecx, so its body must stay invisible here. */
// FUNCTION: EZ2AC_UNPACKED 0x00404950
void KGraphics::m404950(KTexture *tex, float x, float y, float w, float h,
                        float u, float v, float uw, float vh,
                        unsigned int cTop, unsigned int cBottom)
{
    QuadVert q[4];
    float su;
    float sv;
    float du;
    float dv;
    float ue;
    float ve;
    volatile float b;

    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        float tu = tex->u1 - tex->u0;
        float tv = tex->v1 - tex->v0;

        su = tu * u;
        sv = tv * v;
        su = su + tex->u0;
        sv = sv + tex->v0;
        du = tu * uw;
        dv = tv * vh;
    }
    else
    {
        dv = vh;
        du = uw;
        sv = v;
        su = u;
    }
    q[0].x = x;
    q[1].x = x;
    q[2].x = x + w;
    q[3].x = x + w;
    ue = su + du;
    ve = sv + dv;
    q[0].u = su;
    q[1].u = su;
    q[2].u = ue;
    q[3].u = ue;
    q[0].diffuse = cBottom;
    q[0].specular = cBottom;
    q[2].diffuse = cBottom;
    q[2].specular = cBottom;
    q[1].diffuse = cTop;
    q[1].specular = cTop;
    q[3].diffuse = cTop;
    q[3].specular = cTop;
    if (g_d36cb0 == 0)
    {
        q[2].y = y + h;
        q[0].y = y + h;
        q[3].y = y;
        q[1].y = y;
        q[2].v = ve;
        q[0].v = ve;
        q[3].v = sv;
        q[1].v = sv;
    }
    else
    {
        b = y + h;
        q[2].y = 479.0f - b;
        q[0].y = 479.0f - b;
        q[3].y = 479.0f - y;
        q[1].y = 479.0f - y;
        q[2].v = sv;
        q[0].v = sv;
        q[3].v = ve;
        q[1].v = ve;
    }
    q[3].rhw = 1.0f;
    q[2].rhw = 1.0f;
    q[1].rhw = 1.0f;
    q[0].rhw = 1.0f;
    m4080c0(0, tex);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, q, 4, 0);
}

/* KGraphics::m404c50 @0x00404c50 - the THIRD of the family, 136 of its 150
 * instructions shared with the two above. Two colours again, but where
 * m404950 splits them bottom against top in its common block, this one
 * assigns the pair INSIDE the flip halves: v0 always takes the last
 * argument and v3 the other, while v1 and v2 swap which is which when the
 * screen is flipped - which is the only reason its colour stores are not
 * where its twin's are.
 *
 * (Its one caller @0x4323c0 pushes every argument on the stack. The note in
 * the project memory that it takes an xmm2 argument, and so belonged to the
 * static-register-convention cluster, does not survive reading that site.)
 */
// FUNCTION: EZ2AC_UNPACKED 0x00404c50
void KGraphics::m404c50(KTexture *tex, float x, float y, float w, float h,
                        float u, float v, float uw, float vh,
                        unsigned int cB, unsigned int cA)
{
    QuadVert q[4];
    float su;
    float sv;
    float du;
    float dv;
    float ue;
    float ve;
    volatile float b;

    if (w == 0.0f)
        w = tex->w;
    if (h == 0.0f)
        h = tex->h;
    if (tex->f88 != 0)
    {
        float tu = tex->u1 - tex->u0;
        float tv = tex->v1 - tex->v0;

        su = tu * u;
        sv = tv * v;
        su = su + tex->u0;
        sv = sv + tex->v0;
        du = tu * uw;
        dv = tv * vh;
    }
    else
    {
        dv = vh;
        du = uw;
        sv = v;
        su = u;
    }
    q[0].x = x;
    q[1].x = x;
    q[2].x = x + w;
    q[3].x = x + w;
    ue = su + du;
    ve = sv + dv;
    q[0].u = su;
    q[1].u = su;
    q[2].u = ue;
    q[3].u = ue;
    q[0].diffuse = cA;
    q[0].specular = cA;
    if (g_d36cb0 == 0)
    {
        q[2].y = y + h;
        q[0].y = y + h;
        q[3].y = y;
        q[1].y = y;
        q[2].v = ve;
        q[0].v = ve;
        q[3].v = sv;
        q[1].v = sv;
        q[1].diffuse = cA;
        q[1].specular = cA;
        q[2].diffuse = cB;
        q[2].specular = cB;
    }
    else
    {
        b = y + h;
        q[2].y = 479.0f - b;
        q[0].y = 479.0f - b;
        q[3].y = 479.0f - y;
        q[1].y = 479.0f - y;
        q[2].v = sv;
        q[0].v = sv;
        q[3].v = ve;
        q[1].v = ve;
        q[2].diffuse = cA;
        q[2].specular = cA;
        q[1].diffuse = cB;
        q[1].specular = cB;
    }
    q[3].diffuse = cB;
    q[3].specular = cB;
    q[3].rhw = 1.0f;
    q[2].rhw = 1.0f;
    q[1].rhw = 1.0f;
    q[0].rhw = 1.0f;
    m4080c0(0, tex);
    g_quadVerts[0].x = g_quadVerts[0].x - 0.5f;
    g_quadVerts[0].y = g_quadVerts[0].y - 0.5f;
    g_quadVerts[1].x = g_quadVerts[1].x - 0.5f;
    g_quadVerts[1].y = g_quadVerts[1].y - 0.5f;
    g_quadVerts[2].x = g_quadVerts[2].x - 0.5f;
    g_quadVerts[2].y = g_quadVerts[2].y - 0.5f;
    g_quadVerts[3].x = g_quadVerts[3].x - 0.5f;
    g_quadVerts[3].y = g_quadVerts[3].y - 0.5f;
    device->DrawPrimitive(5, 0x1c4, q, 4, 0);
}

/* @0x00432ac0 - draw one entry's picture twice, once through the shared
   texture at this+0xbc44c under an additive blend and once through the
   entry's own under the normal one. The vertical position is a SINE of the
   caller's angle: index the engine's table with `angle * 1.28`, mirrored and
   negated below zero exactly as angleToY @0x434d70 does, then 330s + 230 -
   100. Everything else is fixed - x 49, 202 by 200, no tint.

   THE ELEMENT ARRAY IS AT this+0x12c, not 0x130. The 0x374 stride is the
   same one the parked 0x4323c0 walks, and that function reads +0x130 - which
   is this array's SECOND word. So the element leads with two texture
   pointers, and the two functions read one each. */
extern float g_sinTable[1024];   /* 0x4a5228 (src/globals.cpp) */

struct Quad432AC0Elem
{
    struct KTexture *tex0;   /* 0x00 - this function's */
    struct KTexture *tex1;   /* 0x04 - 0x4323c0's */
    char             _pad8[0x374 - 8];
};

struct Obj432AC0
{
    char            _pad0[0x12c];
    Quad432AC0Elem  elems[1];      /* 0x12c, stride 0x374 */
    char            _pad4a0[0xbc44c - 0x12c - 0x374];
    struct KTexture *shared;       /* 0xbc44c */

    void drawEntry(int i, float angle);
};

// FUNCTION: EZ2AC_UNPACKED 0x00432ac0
void Obj432AC0::drawEntry(int i, float angle)
{
    float s;
    float y;

    if ((int)(angle * 1.28f) < 0)
        s = -g_sinTable[(int)(angle * -1.28f) & 0x3ff];
    else
        s = g_sinTable[(int)(angle * 1.28f) & 0x3ff];

    y = s * 330.0f + 230.0f - 100.0f;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(shared, 49.0f, y, 202.0f, 200.0f, -1);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->drawTex(elems[i].tex0, 49.0f, y, 202.0f,
                                         200.0f, -1);
}

/* The SECOND quad global. drawRect @0x40a4c0 and this one share it, where
   the four texture draws above share the one at 0x4a5198. */
// GLOBAL: EZ2AC_UNPACKED 0x004a7228
QuadVert g_quadVerts2[4];

/* KGraphics::m40a5a0 @0x40a5a0 - an untextured quad with a VERTICAL COLOUR
 * GRADIENT, and the plainest member of the family: no texture, no crop, no
 * rhw or specular or uv stores at all - only the four x's, the four y's and
 * the four diffuse colours, straight into the shared global and out through
 * the same DrawPrimitive tail call.
 *
 * The two colours split by ROW, not by corner: the second goes to vertices 0
 * and 2 (the y + h edge) and the first to 1 and 3 (the y edge), so the
 * gradient runs top to bottom. Like drawTex above it takes an origin and a
 * size rather than drawRect's absolute edges, and it flips through the same
 * g_d36cb0 against the same 479.0f last scanline. Note the pixel bias is
 * 1.0f here, not the 0.5f the four texture draws use.
 *
 * 51.16% (from a STUB), and it is the SAME WALL as its sibling drawRect
 * @0x40a4c0, parked in wip/drawrect.cpp: the original evaluates `y - 1.0f`
 * TWICE per y value - `movaps xmm5,xmm2` then a `subss` on each copy - and
 * cl common-subexpression-eliminates it to once. Every store, every
 * constant, the branch layout and the DrawPrimitive tail call are right; the
 * residue is that CSE and the xmm renumbering downstream of it. Two things
 * that DID move it, from 44.19%: computing the two x edges into named
 * locals BEFORE the flip branch, which is where the original forms them, and
 * the 1.0f bias.
 */
// FUNCTION: EZ2AC_UNPACKED 0x0040a5a0
void KGraphics::m40a5a0(float x, float y, float w, float h, unsigned int c1,
                        unsigned int c2)
{
    float t, b, l, r;

    /* The two x edges are computed BEFORE the flip branch - the original
       loads x and forms x + w above its `jne`. */
    l = x;
    r = x + w;

    g_quadVerts2[0].diffuse = c2;
    g_quadVerts2[2].diffuse = c2;
    g_quadVerts2[1].diffuse = c1;
    g_quadVerts2[3].diffuse = c1;

    if (g_d36cb0 == 0)
    {
        b = y + h;
        t = y;
    }
    else
    {
        b = 479.0f - y;
        t = 479.0f - (y + h);
    }

    g_quadVerts2[0].x = l - 1.0f;
    g_quadVerts2[0].y = b - 1.0f;
    g_quadVerts2[1].x = l - 1.0f;
    g_quadVerts2[1].y = t - 1.0f;
    g_quadVerts2[2].x = r - 1.0f;
    g_quadVerts2[2].y = b - 1.0f;
    g_quadVerts2[3].x = r - 1.0f;
    g_quadVerts2[3].y = t - 1.0f;

    device->DrawPrimitive(5, 0x1c4, g_quadVerts2, 4, 0);
}

/* KGraphics::drawRect @0x40a4c0 - the sibling of m40a5a0 above, and the only
 * user of IDirect3DDevice7 slot 25 that pins it three ways at once (five
 * arguments, type 5 = D3DPT_TRIANGLESTRIP with count 4, and fvf 0x1c4 whose
 * 0x20 stride is exactly the vertex spacing). Arguments are absolute EDGES,
 * not an origin and a size: setFade @0x40b080 calls it with (-5, -5, 650,
 * 490), an over-scan quad. Every coordinate is written one pixel low and the
 * vertical axis flips through the same g_d36cb0 against the same 479.0f last
 * scanline. The strip is (left,b), (left,t), (right,b), (right,t).
 *
 * Integrated from wip/drawrect.cpp. SAME WALL as m40a5a0: the original emits
 * EIGHT subss because it evaluates `b - 1.0f` and `t - 1.0f` twice each -
 * `movaps xmm4,xmm0` / `movaps xmm5,xmm2` make copies AFTER the flip merge
 * and both copies are live at once - where cl common-subexpression-eliminates
 * each to one and emits six. The x subtracts ARE shared on both sides, which
 * is the odd part: the x values come from parameters and the y values out of
 * a branch, so the original's compiler simply did not value-number across
 * that phi and ours does.
 *
 * The park lists ~20 forms already swept (statement orders, per-vertex and
 * per-edge inlined helpers, the flip inside the helper, four separate y
 * variables - which DOES give eight subss but puts the copies inside both
 * arms - four separate globals, loops, /Op, /fp:precise, /Za, and both
 * alternate cl builds). Three more on integration:
 *
 *   - plain copies `b2 = b; t2 = t;` after the merge, the one shape the park
 *     did not have. It is the closest yet - 43 instructions against 44, and
 *     cl DOES emit the two movaps - but it still loses, because cl sinks each
 *     copy to its own use and reuses ONE scratch register, so xmm2 is written
 *     four times where the original keeps xmm3/xmm4/xmm5 live together. Same
 *     failure as the park's out-parameter helper, reached a shorter way. It
 *     is also a hack rather than a reading of the original's source, so the
 *     natural form is what is kept;
 *   - the same through a volatile alias: worse (a memory reload per use where
 *     the original has a register copy);
 *   - duplicating all four y assignments inside both flip arms: 47
 *     instructions, worst of the three.
 *
 * One correction to the park while integrating: it claimed the total length
 * matched on both sides. It does not - the CSE costs FIVE instructions (two
 * movaps and two subss, plus one more in the flip arm), so this is not a pure
 * register permutation. Retry only with a NEW idea about making cl evaluate a
 * shared float expression twice, and note that getting the movaps back is no
 * longer the hard part - keeping both copies live across a use is.
 */
// FUNCTION: EZ2AC_UNPACKED 0x0040a4c0
void KGraphics::drawRect(float left, float top, float right, float bottom,
                         int color)
{
    float t, b;

    g_quadVerts2[0].diffuse = color;
    g_quadVerts2[1].diffuse = color;
    g_quadVerts2[2].diffuse = color;
    g_quadVerts2[3].diffuse = color;

    if (g_d36cb0 == 0)
    {
        b = bottom;
        t = top;
    }
    else
    {
        b = 479.0f - top;
        t = 479.0f - bottom;
    }

    g_quadVerts2[0].x = left - 1.0f;
    g_quadVerts2[1].x = left - 1.0f;
    g_quadVerts2[0].y = b - 1.0f;
    g_quadVerts2[1].y = t - 1.0f;
    g_quadVerts2[2].x = right - 1.0f;
    g_quadVerts2[2].y = b - 1.0f;
    g_quadVerts2[3].x = right - 1.0f;
    g_quadVerts2[3].y = t - 1.0f;

    device->DrawPrimitive(5, 0x1c4, g_quadVerts2, 4, 0);
}

/* KGraphics::m40ac80 @0x40ac80 - the LINE drawer, into the second quad
 * global. Width 1.0 fills only the first two vertices and draws a
 * two-vertex D3DPT_LINELIST; any other width builds the four-vertex
 * strip between the x and x+w edges. Both paths run the same -1.0 bias
 * over ALL FOUR vertices - which in the thin path decrements the two
 * STALE vertices in place, and in the thick path leaves the one
 * raw-then-biased double store on the first y. The colour argument is
 * declared float so no call site moves (every caller bit-casts), and
 * read back as the int it is. The flip mirrors around 480 where the
 * texture family uses 479. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ac80
void KGraphics::m40ac80(float x0, float y0, float x1, float y1, float w,
                        float f)
{
    int   color = *(int *)&f;
    float ey0, ey1;

    g_quadVerts2[0].diffuse = color;
    g_quadVerts2[1].diffuse = color;
    if (w == 1.0f)
    {
        if (g_d36cb0 == 0)
        {
            ey0 = y0;
            ey1 = y1;
        }
        else
        {
            ey0 = 480.0f - y0;
            ey1 = 480.0f - y1;
        }
        g_quadVerts2[0].y = ey0 - 1.0f;
        g_quadVerts2[1].x = x1 - 1.0f;
        g_quadVerts2[2].x = g_quadVerts2[2].x - 1.0f;
        g_quadVerts2[2].y = g_quadVerts2[2].y - 1.0f;
        g_quadVerts2[3].x = g_quadVerts2[3].x - 1.0f;
        g_quadVerts2[0].x = x0 - 1.0f;
        g_quadVerts2[1].y = ey1 - 1.0f;
        g_quadVerts2[3].y = g_quadVerts2[3].y - 1.0f;
        device->DrawPrimitive(2, 0x1c4, g_quadVerts2, 2, 0);
        return;
    }
    {
        float xr0 = x0 + w;
        float xr1 = x1 + w;

        g_quadVerts2[2].diffuse = color;
        g_quadVerts2[3].diffuse = color;
        if (g_d36cb0 == 0)
        {
            ey1 = y1;
            ey0 = y0;
        }
        else
        {
            ey1 = 480.0f - y1;
            ey0 = 480.0f - y0;
        }
        g_quadVerts2[0].x = x1 - 1.0f;
        g_quadVerts2[0].y = ey1;
        g_quadVerts2[0].y = g_quadVerts2[0].y - 1.0f;
        g_quadVerts2[1].x = x0 - 1.0f;
        g_quadVerts2[1].y = ey0 - 1.0f;
        g_quadVerts2[2].x = xr1 - 1.0f;
        g_quadVerts2[2].y = ey1 - 1.0f;
        g_quadVerts2[3].x = xr0 - 1.0f;
        g_quadVerts2[3].y = ey0 - 1.0f;
        device->DrawPrimitive(5, 0x1c4, g_quadVerts2, 4, 0);
    }
}

/* KGraphics::drawLine @0x40ae90 - m40ac80's GRADIENT twin: the same
 * thin/thick split, the same -1.0 bias over all four vertices (stale
 * ones included in the thin path, the raw-then-biased first y in the
 * thick one), but a colour PER END - c0 into the left/first vertex
 * pair, c1 into the right/second - and a plain call out rather than
 * the tail jump (seven arguments do not fit DrawPrimitive's five). */
// FUNCTION: EZ2AC_UNPACKED 0x0040ae90
void KGraphics::drawLine(float x0, float y0, float x1, float y1, float w,
                         int c0, int c1)
{
    float ey0, ey1;

    g_quadVerts2[0].diffuse = c0;
    if (w == 1.0f)
    {
        g_quadVerts2[1].diffuse = c1;
        if (g_d36cb0 == 0)
        {
            ey0 = y0;
            ey1 = y1;
        }
        else
        {
            ey0 = 480.0f - y0;
            ey1 = 480.0f - y1;
        }
        g_quadVerts2[0].y = ey0 - 1.0f;
        g_quadVerts2[1].x = x1 - 1.0f;
        g_quadVerts2[2].x = g_quadVerts2[2].x - 1.0f;
        g_quadVerts2[2].y = g_quadVerts2[2].y - 1.0f;
        g_quadVerts2[3].x = g_quadVerts2[3].x - 1.0f;
        g_quadVerts2[0].x = x0 - 1.0f;
        g_quadVerts2[1].y = ey1 - 1.0f;
        g_quadVerts2[3].y = g_quadVerts2[3].y - 1.0f;
        device->DrawPrimitive(2, 0x1c4, g_quadVerts2, 2, 0);
        return;
    }
    {
        float xr0 = x0 + w;
        float xr1 = x1 + w;

        g_quadVerts2[2].diffuse = c0;
        g_quadVerts2[1].diffuse = c1;
        g_quadVerts2[3].diffuse = c1;
        if (g_d36cb0 == 0)
        {
            ey1 = y1;
            ey0 = y0;
        }
        else
        {
            ey1 = 480.0f - y1;
            ey0 = 480.0f - y0;
        }
        g_quadVerts2[0].x = x1 - 1.0f;
        g_quadVerts2[0].y = ey1;
        g_quadVerts2[0].y = g_quadVerts2[0].y - 1.0f;
        g_quadVerts2[1].x = x0 - 1.0f;
        g_quadVerts2[1].y = ey0 - 1.0f;
        g_quadVerts2[2].x = xr1 - 1.0f;
        g_quadVerts2[2].y = ey1 - 1.0f;
        g_quadVerts2[3].x = xr0 - 1.0f;
        g_quadVerts2[3].y = ey0 - 1.0f;
        device->DrawPrimitive(5, 0x1c4, g_quadVerts2, 4, 0);
    }
}
