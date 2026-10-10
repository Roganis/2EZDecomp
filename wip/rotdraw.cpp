/* WORK IN PROGRESS - NOT IN THE BUILD (45.50% structural,
   scratchpad/rotdiff.py). KGraphics::m404250 @0x00404250, the draw
   family's fifth member and the only one that is not a copy of the others.

   Everything around the middle is the family's shared code and is already
   right: the null check, the w/h defaults, the texture window or 0..1, the
   uv/colour/rhw stores, the half-texel bias and the strip. What needs work
   is the rotation itself - the eight corner expressions below are correct
   arithmetic but the wrong SHAPE: the original computes -hw*c and hh*s once
   each and reuses them across the four corners, where this draft lets cl
   re-derive them, so its register traffic looks nothing like the original's.

   WHAT MOVED IT (38.50% -> 45.50%, and 187 -> 196 of the original's 200
   instructions): the four y products - hw*s, -hw*s, hh*c, -hh*c - belong
   INSIDE each flip arm, not hoisted above the test. Same lever as the hold
   tick's and the keys commit's duplicated tails.

   WHAT DID NOT MOVE IT AT ALL, so do not spend another pass on these:
     * naming the eight products as locals, and mutating the x/y parameters
       instead of adding cx/cy locals - byte-identical output, all four
       spellings (the corner arithmetic is normalised to one IR);
     * ALL 24 PERMUTATIONS of the four uv locals' declaration order -
       byte-identical, so their stack slots are assigned by use order and
       our v0/v1 pair stays swapped against the original's.

   What is left is the family's shared wall: our xmm numbering runs one
   register off the original's throughout (here w/h land in xmm2/xmm7 where
   the original uses xmm0/xmm1, and the 1.0 constant in xmm0 where it uses
   xmm5). Every function in this family has it and ~50 source variants have
   not moved it.

   What is already established, and cost the reading:
     * the angle is in DEGREES - `fld angle; fmul 0.017453292; fld st(0);
       fcos; fstp; fsin; fstp` is cos() and sin() of one argument under /Oi,
       and both results are spilled to argument homes;
     * the quad turns about its own CENTRE: hw = w/2, hh = h/2, cx = x + hw,
       cy = y + hh, corners (-+hw, -+hh) with v0 = (-hw, +hh), v1 = (-hw,
       -hh), v2 = (+hw, +hh), v3 = (+hw, -hh) - the family's usual geometry;
     * the four uv values live in STACK SLOTS (F-0x10, F-0xc, F-0x8, F-0x4),
       not registers, because the rotation needs every xmm;
     * `ret 0x1c` = seven arguments, and the stub's `Sprite *` first
       parameter is really a KTexture * - the body reads +0x80..+0x98 - so
       integrating it means moving the call sites' casts the way m404660's
       moved.
*/

/* KGraphics::m404250 @0x00404250 - the family's rotated draw, and the only
 * one that is not a copy of the others: same head (null check, w/h
 * defaults, the texture's window or 0..1) and same tail (the half-texel
 * bias and the strip), but between them it turns the quad about its own
 * CENTRE. The angle is in DEGREES - the x87 pair is `fld angle;
 * fmul 0.017453292; fld st(0); fcos; ...; fsin`, which is what cos() and
 * sin() of one argument compile to under /Oi - and the four corners are
 * (+-w/2, +-h/2) rotated and offset by the centre.
 *
 * Its four uv values live in STACK SLOTS rather than registers, because
 * the rotation needs every xmm it can get.
 */
// FUNCTION: EZ2AC_UNPACKED 0x00404250
void KGraphics::m404250(KTexture *tex, float x, float y, float w, float h,
                        float angle, unsigned int color)
{
    float u0;
    float v0;
    float u1;
    float v1;
    float hw;
    float hh;
    float cx;
    float cy;
    float c;
    float s;
    float nwc;
    float pwc;
    float phs;
    float nhs;
    float nws;
    float pws;
    float phc;
    float nhc;

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
    hh = h * 0.5f;
    hw = w * 0.5f;
    cy = y + hh;
    cx = x + hw;
    c = (float)cos(angle * 0.017453292f);
    s = (float)sin(angle * 0.017453292f);
    nwc = -hw * c;
    phs = hh * s;
    nhs = -hh * s;
    pwc = hw * c;
    g_quadVerts[0].x = nwc - phs + cx;
    g_quadVerts[1].x = nwc - nhs + cx;
    g_quadVerts[2].x = pwc - phs + cx;
    g_quadVerts[3].x = pwc - nhs + cx;
    if (g_d36cb0 == 0)
    {
        nws = -hw * s;
        pws = hw * s;
        phc = hh * c;
        nhc = -hh * c;
        g_quadVerts[3].y = pws + nhc + cy;
        g_quadVerts[1].y = nws + nhc + cy;
        g_quadVerts[2].y = pws + phc + cy;
        g_quadVerts[0].y = nws + phc + cy;
    }
    else
    {
        nws = -hw * s;
        pws = hw * s;
        phc = hh * c;
        nhc = -hh * c;
        g_quadVerts[3].y = 479.0f - (pws + nhc + cy);
        g_quadVerts[1].y = 479.0f - (nws + nhc + cy);
        g_quadVerts[2].y = 479.0f - (pws + phc + cy);
        g_quadVerts[0].y = 479.0f - (nws + phc + cy);
    }
    g_quadVerts[1].u = u0;
    g_quadVerts[0].u = u0;
    g_quadVerts[3].u = u1;
    g_quadVerts[2].u = u1;
    g_quadVerts[2].v = v1;
    g_quadVerts[0].v = v1;
    g_quadVerts[3].v = v0;
    g_quadVerts[1].v = v0;
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
    m4080c0(0, tex);
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
