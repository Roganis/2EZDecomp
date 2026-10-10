#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/* KGraphics, part 2: the methods that src/kgraphics.cpp CALLS and so cannot
   see. cl inlines a body this small on sight, and setFade @0x40b080 has two
   real `call 0x4080c0`s in it - keeping the definition in its own TU is what
   preserves them. */

#include <windows.h>
#include "kcrt.h"
#include "icom.h"
#include "obj405f80.h"
#include "kprofile.h"
#include "kfile.h"
#include "ez2acapp.h"
#include "loader.h"   /* Log */

/* The texture currently bound to each stage, so a redundant SetTexture can be
   skipped. Only stages 0..3 are ever used (m408c80 @0x408c80 unbinds exactly
   four); the array cannot reach past 0xd49c98, which is the next global. */
// GLOBAL: EZ2AC_UNPACKED 0x00d49c78
static IDirectDrawSurface7 *g_boundTex[8];

/* Bind one texture stage. A null texture always goes through - that is how a
   stage is cleared - while a real one is only sent to the device when its
   surface is not already the bound one. */
/* KGraphics::m4038f0 @0x004038f0 - the video blit's dispatcher: multi-frame
   layers with a stage mode of five and up go to the two-texture blit
   m401d80, everything else to the single-texture m402f70, all five
   arguments untouched. Its body has to be out of both blits' TUs for the
   calls to stay calls. */
// FUNCTION: EZ2AC_UNPACKED 0x004038f0
void KGraphics::m4038f0(KTexture **frames, int count, VideoBlit *d,
                        float xoff, float yoff)
{
    if (count > 1 && (unsigned int)d->stageMode >= 5)
        m401d80(frames, count, d, xoff, yoff);
    else
        m402f70(frames, count, d, xoff, yoff);
}

// FUNCTION: EZ2AC_UNPACKED 0x004080c0
void KGraphics::m4080c0(int stage, KTexture *tex)
{
    if (tex == 0)
    {
        g_boundTex[stage] = 0;
        device->SetTexture(stage, 0);
    }
    else
    {
        IDirectDrawSurface7 *s = tex->surface;
        if (g_boundTex[stage] != s)
        {
            g_boundTex[stage] = s;
            device->SetTexture(stage, s);
        }
    }
}

/* Put the finished frame on screen. Either flip the surface chain - with two
   sets of flags the caller picks between - or, when the chain is not flippable,
   blit the back buffer into the destination rectangle and wait for it. With no
   primary surface there is nothing to present, and the failure code goes back
   to the caller as-is. Kept out of src/kgraphics.cpp because endFrame @0x405e30
   tail-jumps here. */
// FUNCTION: EZ2AC_UNPACKED 0x00405d80
int KGraphics::m405d80()
{
    if (surface == 0)
        return 0x8200000f;

    if (f8 != 0)
    {
        /* DDFLIP_WAIT | DDFLIP_STEREO, or plain DDFLIP_WAIT. */
        if (fc != 0)
            return surface->Flip(0, 0x11);
        return surface->Flip(0, 1);
    }
    /* DDBLT_WAIT */
    return surface->Blt(&m_destRect, m_back, 0, 0x1000000, 0);
}

/* The texture blend-mode table: sixteen recipes, each a run of
   SetTextureStageState calls that programs one or two texture stages. This is
   what pins IDirect3DDevice7 slot 37 - every one of the 190 calls carries a
   (stage, type, value) triple that is legal D3D7 and self-consistent
   (D3DTSS_COLOROP always gets a D3DTOP_, the ARG states always a D3DTA_).

   Modes 1 and 2 program the same thing, which is why the original has one
   block for both. The out-of-range default does nothing.

   Kept out of src/kgraphics.cpp because setFade @0x40b080 calls it. */
// FUNCTION: EZ2AC_UNPACKED 0x00408130
void KGraphics::m408130(int mode)
{
    switch (mode)
    {
    case 0:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 1:
    case 2:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 3:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 4:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        break;

    case 5:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 6:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 7:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 8:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 9:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 10:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDDIFFUSEALPHA);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 11:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 12:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 13:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_ADD);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 14:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;

    case 15:
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
        device->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_ADDSIGNED);
        device->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
        device->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        break;
    }
}


/* Obj405F80's factory. Not pooled - a plain malloc through kcrt.h's operator
   new - and it lives here rather than beside the constructor in
   src/obj405f80.cpp, because the original CALLS that constructor. */
// FUNCTION: EZ2AC_UNPACKED 0x00406580
Obj405F80 *createObj405F80()
{
    return new Obj405F80;
}

/* Point the camera one unit down -Z at the origin, set the standard
   projection, and draw one indexed mesh: a 121-vertex, 600-index triangle
   strip, i.e. an 11x11 grid. The camera preamble is the same three statements
   src/directors.cpp uses twice. */
// FUNCTION: EZ2AC_UNPACKED 0x00406850
void KGraphics::drawMesh(MeshDraw *a)
{
    GVec3 eye;
    GVec3 at;
    GVec3 up;

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    setCamera(&eye, &at, &up);
    setProjection(1.0f, 10000.0f, 0.785398185f);

    device->DrawIndexedPrimitiveVB(4, a->vb, 0, 0x79, a->indices, 0x258, 0);
}

/* Lock the back buffer for direct pixel access and cache where it landed. The
   pitch is halved on the way in because every consumer indexes it in 16-bit
   pixels: the surface is RGB565. */
// FUNCTION: EZ2AC_UNPACKED 0x00406c10
int KGraphics::lockBack()
{
    DDSurfaceDesc2 desc;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    if (m_back->Lock(0, &desc, 1, 0) == 0)
    {
        m_lockBits = (unsigned short *)desc.lpSurface;
        m_lockPitch = desc.lPitch / 2;
        return 1;
    }
    return 0;
}

extern unsigned char g_glyphBits[0x92e * 32];

/* Slurp the whole CJK glyph bitmap in one read. Nothing else in the engine
   opens this file, so its 0x125c0 bytes are what fix the font's shape: 0x92e
   glyphs of 16 rows, two bytes each. */
// FUNCTION: EZ2AC_UNPACKED 0x00407080
int loadGlyphFont(char *name)
{
    KFile f;

    if (f.open(name, 1) == 0)
        return 0;

    f.read(g_glyphBits, sizeof(g_glyphBits));
    f.close();
    return 1;
}

extern int g_asciiWidth[128];
extern unsigned char g_asciiBits[128 * 16];

/* The half-width font, in two reads: the bitmap first, then the widths that
   make it proportional. */
// FUNCTION: EZ2AC_UNPACKED 0x00407120
int loadAsciiFont(char *name)
{
    KFile f;

    if (f.open(name, 1) == 0)
        return 0;

    f.read(g_asciiBits, sizeof(g_asciiBits));
    f.read(g_asciiWidth, sizeof(g_asciiWidth));
    f.close();
    return 1;
}

/* printf into a 4 KB frame buffer and draw the result. The graphics object is
   an ordinary first argument here, not `this` - the function is free, as a
   vararg one has to be. */
// FUNCTION: EZ2AC_UNPACKED 0x004071d0
void drawTextf(KGraphics *g, int x, int y, const char *fmt, ...)
{
    char buf[0x1000];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    g->drawText(x, y, buf);
}

/* KGraphics::m408c40 @0x408c40 - one whole frame of the texture-cache tree:
   open the scene, draw the tree depth-first, optionally clear, close the
   scene. The three main-game directors call it (@0x424651, @0x4606e2,
   @0x46f247).

   The tree walk is written exactly as its sibling in KGraphics::reset
   @0x405cf0 writes the freeing half - `m_profile11c->f(m_profile11c->root)`,
   reloading the pointer for the argument - because that is what the original
   does here too (`mov ecx,[esi+0x11c]; mov eax,[ecx+0x14]`).

   The clear goes through Obj40BB00::doOp10 @0x40a730, and the cast is the
   interesting part: `this` is a KGraphics (the scene calls take it in ecx),
   but @0x40a730 is already matched as a method of Obj40BB00 in
   src/com_wrappers.cpp - which on the evidence IS KGraphics under a second
   name, since its only modelled member is an interface at +0x30 and that is
   where KGraphics keeps its IDirect3DDevice7. Merging the two models would
   touch a struct several matched functions depend on, so this casts instead
   and the observation is recorded rather than acted on. */
struct Obj40BB00;   /* src/com_wrappers.cpp - KGraphics under another name */
struct Obj40BB00 { void doOp10(unsigned int v); };   /* 0x40a730 */

// FUNCTION: EZ2AC_UNPACKED 0x00408c40
void KGraphics::m408c40(int clear)
{
    m405e10();
    m_profile11c->drawTree408bf0((KTextureNode *)m_profile11c->root);

    if (clear != 0)
    {
        ((Obj40BB00 *)this)->doOp10(0);
    }

    endFrame();
}

/* What @0x407360 hands the enumeration as its context. */
struct TexFmtPick
{
    int             wantAlpha;   /* 0x00 */
    unsigned int    bitCount;    /* 0x04 */
    int             found;       /* 0x08 */
    DdPixelFormat  *out;         /* 0x0c */
};

/* Reconstructed but not matched - wip/texfmtpick.cpp, 67.83%, and being in
   this TU beside its caller did NOT settle it (tried 2026-08-18). Stubbed in
   src/stubs.cpp so the address the caller pushes resolves. */
extern int __stdcall pickTextureFormat(DdPixelFormat *pf, TexFmtPick *pick);

/* The device caps block GetCaps fills. Only the field at +0x5c is read. */
struct D3DDeviceCaps
{
    char         _pad0[0x5c];
    unsigned int textureCaps;    /* 0x5c */
    char         _pad60[0xec - 0x60];
};

extern int   g_profileD49C98[];      /* src/globals.cpp - 0xd49c98 */
extern void *g_graphicsSingleton;    /* src/globals.cpp - 0xd49cb0 */

/* KGraphics::m407360 - pick the two texture formats the engine will use, then
   read the device caps. Two enumeration passes over the same context block,
   one asking for an opaque 32-bit format and one for a 32-bit format with
   alpha, each writing into its own slot on this object. */
// FUNCTION: EZ2AC_UNPACKED 0x00407360
void KGraphics::m407360()
{
    TexFmtPick    pick;
    D3DDeviceCaps caps;

    m_profile11c = (KProfile *)g_profileD49C98;
    g_graphicsSingleton = this;

    fmtOpaque.size = 0x20;
    pick.out = &fmtOpaque;
    pick.bitCount = 0x20;
    pick.wantAlpha = 0;
    pick.found = 0;
    device->EnumTextureFormats((void *)pickTextureFormat, &pick);

    fmtAlpha.size = 0x20;
    pick.out = &fmtAlpha;
    pick.bitCount = 0x20;
    pick.wantAlpha = 1;
    pick.found = 0;
    device->EnumTextureFormats((void *)pickTextureFormat, &pick);

    m_texBytes = 0;
    m_texCount = 0;

    if (device->GetCaps(&caps) >= 0)
    {
        unsigned int tc = caps.textureCaps;

        m_textureCaps = tc;
        // STRING: EZ2AC_UNPACKED 0x00487cb0
        Log("m_dwTextureCaps: %x", tc);
        fd4 = 0;
    }
}

/* @0x00407b60 - paint a texture's own name across it, the placeholder a
   missing texture gets. Called only by @0x407edf, which the log string names
   CreateDummyTexture2.

   A member that never touches `this` - reloadBmpTexture @0x407e50 calls it
   with ecx loaded - so its bytes are those of a free __stdcall function.

   The six GDI calls all go through IAT slots the packer left unrebuilt - the
   name RVAs stored there (0x33e380e and friends) fall outside the 0xb7000
   file, the same breakage the subsystem lock hit. They resolve anyway because
   the name table itself survived and kept its internal offsets: the slots'
   low twelve bits are 0x80e, 0x81a, 0x92a, 0x93e, 0x94e and 0x96a, and the
   file has FillRect, DrawTextA, CreateSolidBrush, SetTextColor, SetBkMode and
   DeleteObject at exactly 0xb180e, 0xb181a, 0xb192a, 0xb193e, 0xb194e and
   0xb196a. Every one agrees with what the call site does with it.

   0x25 is DT_CENTER|DT_VCENTER|DT_SINGLELINE. */
// FUNCTION: EZ2AC_UNPACKED 0x00407b60
int KGraphics::drawDummyTextureLabel(const char *name, KTexture *tex)
{
    RECT r;
    void *dc;

    if (tex->surface == 0)
        return 0;

    /* top before left: the original stores [esp+0x14] then [esp+0x10], and
       cl follows source order for the pair. */
    r.top = 0;
    r.left = 0;
    r.right = (int)tex->w;
    r.bottom = (int)tex->h;

    // STRING: EZ2AC_UNPACKED 0x00487f1c
    Log("KGraphics::CreateDummyTexture2CreatedTexture(%s)", name);

    /* The hot path is the if-body and the failure is the trailing return, so
       cl lays the `return 0` out of line past the epilogue - which is where
       the original keeps it. Written the other way round (an early
       `if (GetDC(...) < 0) return 0;`) the two tails merge and the function
       stops at 91.93%. */
    if (tex->surface->GetDC(&dc) >= 0)
    {
        HBRUSH b = CreateSolidBrush(0x7f0000);

        FillRect((HDC)dc, &r, b);
        DeleteObject(b);

        SetTextColor((HDC)dc, 0xffffff);
        SetBkMode((HDC)dc, 1);
        DrawTextA((HDC)dc, name, strlen(name), &r, 0x25);
        tex->surface->ReleaseDC(dc);
        return 1;
    }

    return 0;
}

/* KGraphics::m405a90 @0x405a90 - bring the 3D device up. Three steps, each
   with its own log line and its own custom HRESULT: query IDirect3D7 out of
   the DirectDraw object, create the device on the back buffer with the
   caller's device GUID, and give it a viewport covering the whole screen.

   The viewport is the six fields of D3DVIEWPORT7 - x, y, width, height, then
   the two z bounds at 0.0 and 1.0 - which is exactly the 0x18 bytes the
   prologue reserves. Each failure block is laid out INLINE after its test,
   which is what plain early returns give. */
extern int g_iidDirect3D7[4];   /* src/globals.cpp - 0x4a01b0 */

struct D3DViewport7
{
    int   x;
    int   y;
    int   width;
    int   height;
    float minZ;
    float maxZ;
};

// FUNCTION: EZ2AC_UNPACKED 0x00405a90
int KGraphics::m405a90(void *deviceGuid)
{
    D3DViewport7 vp;

    if (ddraw->QueryInterface(g_iidDirect3D7, (void **)&d3d) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487a18
        Log("KGraphics: Couldn't get the Direct3D interface");
        return 0x82000003;
    }

    if (d3d->CreateDevice(deviceGuid, m_back, &device) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004879ec
        Log("KGraphics: Couldn't create the D3DDevice");
        return 0x82000004;
    }

    vp.x = 0;
    vp.y = 0;
    vp.width = m_width;
    vp.height = m_height;
    vp.minZ = 0.0f;
    vp.maxZ = 1.0f;

    if (device->SetViewport(&vp) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004879b0
        Log("KGraphics: Error: Couldn't set current viewport to device");
        return 0x82000007;
    }

    return 0;
}

/* The 0x7c-byte block createOffscreenSurface fills - the frame says so
   exactly: 0x90 = 0x7c here + 0x10 of RECT + the four-byte DC. That size is
   DDSURFACEDESC2's, which is what an off-screen surface call takes. */

/* @0x00407a50 - build the placeholder texture itself: a 256x256 off-screen
   surface with the texture's name painted across it.

   It does NOT call drawDummyTextureLabel @0x407b60 even though the two share
   their whole GDI tail - the block is written out again here, which is what
   the original's duplicate call sites say. */
// FUNCTION: EZ2AC_UNPACKED 0x00407a50
int KGraphics::createDummyTexture(const char *name, float w, float h, KTexture *tex)
{
    RECT  r;
    void *dc;
    DdSurfaceDesc2 desc;

    r.top = 0;
    r.left = 0;
    r.right = 0x100;
    r.bottom = 0x100;

    tex->w = 256.0f;
    tex->h = 256.0f;
    tex->surface = createOffscreenSurface(&desc, 0x100, 0x100, 0);

    // STRING: EZ2AC_UNPACKED 0x00487ef4
    Log("KGraphics::CreateDummyTexture(%s)", name);

    if (tex->surface->GetDC(&dc) >= 0)
    {
        HBRUSH br = CreateSolidBrush(0x7f0000);

        FillRect((HDC)dc, &r, br);
        DeleteObject(br);

        SetTextColor((HDC)dc, 0xffffff);
        SetBkMode((HDC)dc, 1);
        DrawTextA((HDC)dc, name, strlen(name), &r, 0x25);
        tex->surface->ReleaseDC(dc);
        return 1;
    }

    return 0;
}

/* Load a texture out of a .bmp: read the whole file into g_bmpBuffer, undo the
   header scramble when fd4 says the assets are encrypted (bfOffBits ^ 0x109a,
   biBitCount ^ 0xb18f, biWidth ^ 0xcfa1, biHeight ^ 0x51ae), size the
   texture, make its off-screen surface and StretchDIBits the pixels into it
   through a DC - upside down (source height negated, origin one past the
   bottom) when g_d36cb0 says so. A file that will not open gets the 128x128
   placeholder under its base name instead. StretchDIBits is called from an
   IAT slot the packer left unrebuilt, resolved by its name-table offset. */
extern unsigned char g_bmpFileHeader[14];     /* 0x530c48 (src/globals.cpp) */
extern unsigned char g_bmpInfo[];             /* 0x530c56 - the info header + pixels */
extern int  g_d36cb0;                         /* src/maingame.cpp */
extern void extractBaseName(char *name, char *out);   /* 0x412f00 */
#define BMP_OFFBITS   (*(int *)&g_bmpFileHeader[0xa])
#define BMP_WIDTH     (*(int *)&g_bmpInfo[0x4])
#define BMP_HEIGHT    (*(int *)&g_bmpInfo[0x8])
#define BMP_BITCOUNT  (*(unsigned short *)&g_bmpInfo[0xe])

// FUNCTION: EZ2AC_UNPACKED 0x00407c60
int KGraphics::loadBmpTexture(char *name, KTexture *dst)
{
    void *dc;
    KFile f;
    DdSurfaceDesc2 desc;
    char base[0x104];
    unsigned char *bits;

    if (f.open(name, 1) == 0)
    {
        extractBaseName(name, base);
        return createDummyTexture(base, 128.0f, 128.0f, dst);
    }
    f.read(g_bmpFileHeader, f.size());
    f.close();
    if (fd4 != 0)
    {
        BMP_OFFBITS ^= 0x109a;
        BMP_BITCOUNT ^= 0xb18f;
        BMP_WIDTH ^= 0xcfa1;
        BMP_HEIGHT ^= 0x51ae;
    }
    bits = &g_bmpFileHeader[BMP_OFFBITS];
    dst->w = (float)BMP_WIDTH;
    dst->h = (float)BMP_HEIGHT;
    dst->surface = createOffscreenSurface(&desc, BMP_WIDTH, BMP_HEIGHT, 0);
    if (dst->surface->GetDC(&dc) >= 0)
    {
        if (g_d36cb0 == 0)
            StretchDIBits((HDC)dc, 0, 0, desc.raw[3], desc.raw[2], 0, 0, BMP_WIDTH, BMP_HEIGHT, bits, (BITMAPINFO *)g_bmpInfo, DIB_RGB_COLORS, SRCCOPY);
        else
            StretchDIBits((HDC)dc, 0, 0, desc.raw[3], desc.raw[2], 0, BMP_HEIGHT + 1, BMP_WIDTH, -BMP_HEIGHT, bits, (BITMAPINFO *)g_bmpInfo, DIB_RGB_COLORS, SRCCOPY);
        dst->surface->ReleaseDC(dc);
    }
    return 1;
}

/* An off-screen surface for a texture: a DDSURFACEDESC2 zeroed into the
   caller's block, sized w x h, with the device's alpha (`flag`) or opaque
   pixel format copied in, and CreateSurface. On failure the DDERR is named
   in the log (the cases are in the original's arm order, which is not
   numeric), the free video and physical memory measured, and 0 returned.
   GlobalMemoryStatus comes through an IAT slot the packer left unrebuilt. */
struct DdCaps2 { int caps; int caps2; int caps3; int caps4; };

// FUNCTION: EZ2AC_UNPACKED 0x00407450
IDirectDrawSurface7 *KGraphics::createOffscreenSurface(DdSurfaceDesc2 *desc, int w, int h, int flag)
{
    IDirectDrawSurface7 *surf;
    int hr;

    memset(desc, 0, sizeof(*desc));
    desc->raw[0] = 0x7c;
    if (flag != 0)
        memcpy(&desc->raw[0x48 / 4], &fmtAlpha, sizeof(DdPixelFormat));
    else
        memcpy(&desc->raw[0x48 / 4], &fmtOpaque, sizeof(DdPixelFormat));
    desc->raw[1] = 0x11007;
    desc->raw[0x68 / 4] = 0x10005000;
    desc->raw[0x6c / 4] = 0;
    desc->raw[3] = w;
    desc->raw[2] = h;
    hr = ddraw->CreateSurface(desc, &surf, 0);
    if (hr != 0)
    {
        DdCaps2 caps;
        MEMORYSTATUS ms;
        unsigned int total;
        unsigned int avail;
        char buf[0x100];

        memset(&ms.dwMemoryLoad, 0, sizeof(ms) - 4);
        memset(&caps, 0, sizeof(caps));
        ms.dwLength = sizeof(ms);
        caps.caps = 0x4000;
        ddraw->GetAvailableVidMem(&caps, &total, &avail);
        GlobalMemoryStatus(&ms);
        switch (hr)
        {
        case 0x80070057:
            // STRING: EZ2AC_UNPACKED 0x00487e90
            Log("DDERR_INVALIDPARAMS");
            break;
        case 0x8007000e:
            // STRING: EZ2AC_UNPACKED 0x00487e7c
            Log("DDERR_OUTOFMEMORY");
            break;
        case 0x8876005f:
            // STRING: EZ2AC_UNPACKED 0x00487e60
            Log("DDERR_INCOMPATIBLEPRIMARY");
            break;
        case 0x88760064:
            // STRING: EZ2AC_UNPACKED 0x00487e4c
            Log("DDERR_INVALIDCAPS");
            break;
        case 0x88760082:
            // STRING: EZ2AC_UNPACKED 0x00487e38
            Log("DDERR_INVALIDOBJECT");
            break;
        case 0x88760091:
            // STRING: EZ2AC_UNPACKED 0x00487e1c
            Log("DDERR_INVALIDPIXELFORMAT");
            break;
        case 0x887600b4:
            // STRING: EZ2AC_UNPACKED 0x00487e0c
            Log("DDERR_NOALPHAHW");
            break;
        case 0x887600d4:
            // STRING: EZ2AC_UNPACKED 0x00487df0
            Log("DDERR_NOCOOPERATIVELEVELSET");
            break;
        case 0x887600e1:
            // STRING: EZ2AC_UNPACKED 0x00487dd8
            Log("DDERR_NOEXCLUSIVEMODE");
            break;
        case 0x887600e6:
            // STRING: EZ2AC_UNPACKED 0x00487dc8
            Log("DDERR_NOFLIPHW");
            break;
        case 0x88760154:
            // STRING: EZ2AC_UNPACKED 0x00487db4
            Log("DDERR_NOZBUFFERHW");
            break;
        case 0x88760104:
            // STRING: EZ2AC_UNPACKED 0x00487da0
            Log("DDERR_NOOVERLAYHW");
            break;
        case 0x8876017c:
            // STRING: EZ2AC_UNPACKED 0x00487d88
            Log("DDERR_OUTOFVIDEOMEMORY");
            break;
        case 0x88760233:
            // STRING: EZ2AC_UNPACKED 0x00487d70
            Log("DDERR_NODIRECTDRAWHW");
            break;
        case 0x88760234:
            // STRING: EZ2AC_UNPACKED 0x00487d4c
            Log("DDERR_PRIMARYSURFACEALREADYEXISTS");
            break;
        case 0x8876024f:
            // STRING: EZ2AC_UNPACKED 0x00487d38
            Log("DDERR_NOMIPMAPHW");
            break;
        case 0x8876024e:
            // STRING: EZ2AC_UNPACKED 0x00487d20
            Log("DDERR_UNSUPPORTEDMODE");
            break;
        case 0x88760235:
            // STRING: EZ2AC_UNPACKED 0x00487d0c
            Log("DDERR_NOEMULATION");
            break;
        }
        // STRING: EZ2AC_UNPACKED 0x00487cc8
        sprintf(buf, "KGraphics::CreateTextureSurface() Failed (AvailPhy:%d, AvailVid:%d)", ms.dwAvailPhys, avail);
        Log(buf);
        return 0;
    }
    return surf;
}

/* KGraphics::createSystemTextureSurface @0x407750 - createOffscreenSurface
   again for a system-memory texture: DDSD_TEXTURESTAGE in the flags and
   DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY for the caps; one caller, the
   texture finder @0x409200. The DDERR strings are the ones above. */
// FUNCTION: EZ2AC_UNPACKED 0x00407750
IDirectDrawSurface7 *KGraphics::createSystemTextureSurface(DdSurfaceDesc2 *desc, int w, int h, int flag)
{
    IDirectDrawSurface7 *surf;
    int hr;

    memset(desc, 0, sizeof(*desc));
    desc->raw[0] = 0x7c;
    if (flag != 0)
        memcpy(&desc->raw[0x48 / 4], &fmtAlpha, sizeof(DdPixelFormat));
    else
        memcpy(&desc->raw[0x48 / 4], &fmtOpaque, sizeof(DdPixelFormat));
    desc->raw[1] = 0x101007;        /* ... | DDSD_TEXTURESTAGE */
    desc->raw[0x68 / 4] = 0x1800;   /* DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY */
    desc->raw[0x6c / 4] = 0;
    desc->raw[3] = w;
    desc->raw[2] = h;
    hr = ddraw->CreateSurface(desc, &surf, 0);
    if (hr != 0)
    {
        DdCaps2 caps;
        MEMORYSTATUS ms;
        unsigned int total;
        unsigned int avail;
        char buf[0x100];

        memset(&ms.dwMemoryLoad, 0, sizeof(ms) - 4);
        memset(&caps, 0, sizeof(caps));
        ms.dwLength = sizeof(ms);
        caps.caps = 0x4000;
        ddraw->GetAvailableVidMem(&caps, &total, &avail);
        GlobalMemoryStatus(&ms);
        switch (hr)
        {
        case 0x80070057:
            Log("DDERR_INVALIDPARAMS");
            break;
        case 0x8007000e:
            Log("DDERR_OUTOFMEMORY");
            break;
        case 0x8876005f:
            Log("DDERR_INCOMPATIBLEPRIMARY");
            break;
        case 0x88760064:
            Log("DDERR_INVALIDCAPS");
            break;
        case 0x88760082:
            Log("DDERR_INVALIDOBJECT");
            break;
        case 0x88760091:
            Log("DDERR_INVALIDPIXELFORMAT");
            break;
        case 0x887600b4:
            Log("DDERR_NOALPHAHW");
            break;
        case 0x887600d4:
            Log("DDERR_NOCOOPERATIVELEVELSET");
            break;
        case 0x887600e1:
            Log("DDERR_NOEXCLUSIVEMODE");
            break;
        case 0x887600e6:
            Log("DDERR_NOFLIPHW");
            break;
        case 0x88760154:
            Log("DDERR_NOZBUFFERHW");
            break;
        case 0x88760104:
            Log("DDERR_NOOVERLAYHW");
            break;
        case 0x8876017c:
            Log("DDERR_OUTOFVIDEOMEMORY");
            break;
        case 0x88760233:
            Log("DDERR_NODIRECTDRAWHW");
            break;
        case 0x88760234:
            Log("DDERR_PRIMARYSURFACEALREADYEXISTS");
            break;
        case 0x8876024f:
            Log("DDERR_NOMIPMAPHW");
            break;
        case 0x8876024e:
            Log("DDERR_UNSUPPORTEDMODE");
            break;
        case 0x88760235:
            Log("DDERR_NOEMULATION");
            break;
        }
        // STRING: EZ2AC_UNPACKED 0x00487ea8
        sprintf(buf, "KGraphics::CreateSystemTextureSurface() Failed (AvailPhy:%d, AvailVid:%d)", ms.dwAvailPhys, avail);
        Log(buf);
        return 0;
    }
    return surf;
}

/* Build a flag: zero the block, make a 121-vertex XYZ|NORMAL|TEX1 (0x112)
   vertex buffer, and fill it as an 11x11 grid - x/y = 0.2 + 3.31 * (i, j)/10,
   z 0, the normal (0, 0, -1), u = i/10, v = 1 - j/10 - vertex (i, j) at
   index j*11 + i. Then 600 indices, two triangles per cell, and the two wave
   parameters stored last. Returns 1.

   Every loop shape here is load-bearing: both fill loops carry an EXPLICIT
   down-counter beside the index (i and j survive only for the float
   conversions), the column is a BYTE offset `col` the vertex stores add to
   the re-read `verts` (an int-index or a struct index either loses the
   outer induction register or anchors it twelve bytes in), the index loop's
   vertex number is carried (`v += 11`) inside a down-counted inner loop, and
   the outer row `r` is an int compared as `(unsigned short)r < 10` in a
   do-while - a plain `for` on a short reverses into a counter. */
#define VTX(o) (*(float *)((char *)verts + col + j * 352 + (o)))
// FUNCTION: EZ2AC_UNPACKED 0x00406620
int KGraphics::createFlag(GfxFlag *f, float a, float b)
{
    D3DVbDesc desc;
    float *verts;
    int i;
    int j;
    int ni;
    int nj;
    int col;
    unsigned short n;
    int r;

    memset(f, 0, sizeof(*f));
    memset(&desc, 0, sizeof(desc));
    desc.size = 0x10;
    desc.caps = 0;
    desc.fvf = 0x112;
    desc.numVertices = 121;
    d3d->CreateVertexBuffer(&desc, &f->vb, 0);
    f->vb->Lock(1, (void **)&verts, 0);
    col = 0;
    for (i = 0, ni = 11; ni != 0; ni--, i++, col += 32)
    {
        float fx = (float)i * 0.1f;
        float x = fx * 3.31f + 0.2f;
        for (j = 0, nj = 11; nj != 0; nj--, j++)
        {
            float fy = (float)j * 0.1f;
            VTX(0) = x;
            VTX(4) = fy * 3.31f + 0.2f;
            VTX(8) = 0.0f;
            VTX(12) = 0.0f;
            VTX(16) = 0.0f;
            VTX(20) = -1.0f;
            VTX(24) = fx;
            VTX(28) = 1.0f - fy;
        }
    }
    f->vb->Unlock();
    n = 0;
    r = 0;
    do
    {
        int nc;
        int v = r + 11;
        for (nc = 10; nc != 0; nc--)
        {
            f->indices[n++] = v;
            f->indices[n++] = v - 10;
            f->indices[n++] = v - 11;
            f->indices[n++] = v;
            f->indices[n++] = v + 1;
            f->indices[n++] = v - 10;
            v += 11;
        }
        r++;
    }
    while ((unsigned short)r < 10);
    f->a = a;
    f->b = b;
    return 1;
}

/* Tear a flag down: log it, Release the vertex buffer if there is one, and
   zero the block. */
// FUNCTION: EZ2AC_UNPACKED 0x00406820
void KGraphics::destroyFlag(GfxFlag *f)
{
    // STRING: EZ2AC_UNPACKED 0x00487bf8
    Log("Destroying Flag at %x", f);
    if (f->vb != 0)
        f->vb->Release();
    memset(f, 0, sizeof(*f));
}

/* KGraphics::createPrimary @0x405740 - the full-screen path of the graphics
   init: the destination rect is the settings' size, the mode is forced to
   640x480x32 at the settings' refresh, the primary is created as a flipping
   complex 3D surface with one back buffer (in video memory with a stereo
   left buffer when fc asks), and the back buffer - and the left one - are
   taken off it and held. Each failure logs and returns its own HRESULT,
   DDERR_OUTOFVIDEOMEMORY passed straight through.
   75.16%: cl keeps &surface in edi across CreateSurface and spills hr, where
   the original reloads [this+0x34] and keeps hr in edi; the two DDSCAPS2
   blocks also come out in the other slot order. Typed out-params, an hr
   assigned before the first call, scoping the second block and swapping the
   declarations all leave it. */

// FUNCTION: EZ2AC_UNPACKED 0x00405740
int KGraphics::createPrimary(KDisplaySettings *p)
{
    DdSurfaceDesc2 sd;
    int caps[4];
    int hr;

    SetRect((RECT *)&m_destRect, 0, 0, p->width, p->height);
    m_width = 640;
    m_height = 480;
    p->bpp = 32;
    hr = ddraw->SetDisplayMode(m_width, m_height, 32, p->refresh, 0);
    if (hr < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487934
        Log("KGraphics: Can't set display mode");
        return 0x8200000a;
    }

    memset(&sd, 0, sizeof(sd));
    sd.raw[0] = sizeof(sd);
    sd.raw[1] = 0x21;                 /* DDSD_CAPS | DDSD_BACKBUFFERCOUNT */
    sd.raw[0x68 / 4] = 0x2218;        /* PRIMARYSURFACE | FLIP | COMPLEX | 3DDEVICE */
    sd.raw[0x14 / 4] = 1;
    if (fc != 0)
    {
        *((unsigned char *)&sd + 0x6e) |= 8;   /* DDSCAPS2_STEREOSURFACELEFT */
        sd.raw[0x68 / 4] = 0x6218;             /* ... | VIDEOMEMORY */
    }
    hr = ddraw->CreateSurface(&sd, &surface, 0);
    if (hr < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487904
        Log("KGraphics: Error: Can't create primary surface");
        if (hr == (int)0x8876017c)
        {
            // STRING: EZ2AC_UNPACKED 0x004878dc
            Log("KGraphics: Error: Out of video memory");
            return (int)0x8876017c;
        }
        return 0x82000008;
    }

    caps[0] = 4;                      /* DDSCAPS_BACKBUFFER */
    caps[1] = 0;
    caps[2] = 0;
    caps[3] = 0;
    if (surface->GetAttachedSurface(caps, &m_back) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004878b4
        Log("KGraphics:  Can't get the backbuffer");
        return 0x8200000b;
    }
    m_back->AddRef();

    if (fc != 0)
    {
        int capsLeft[4];
        capsLeft[0] = 0;
        capsLeft[1] = 0x80000;        /* DDSCAPS2_STEREOSURFACELEFT */
        capsLeft[2] = 0;
        capsLeft[3] = 0;
        if (m_back->GetAttachedSurface(capsLeft, &m_backLeft) < 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00487888
            Log("KGraphics:  Can't get the left backbuffer");
            return 0x8200000b;
        }
        m_backLeft->AddRef();
    }
    return 0;
}

/* KGraphics::createWindowed @0x405900 - the windowed path of the graphics
   init, and createPrimary's sibling.

   The destination rect is the WINDOW's client area in screen coordinates -
   GetClientRect then ClientToScreen on each corner - and the render size is
   whatever that measures. The primary is a plain one (no flipping, no back
   buffer chain, since a window cannot flip), clipped to the window through a
   clipper that is attached and then released - the surface holds its own
   reference. The back buffer is a separate offscreen 3D render target at the
   measured size, which is what the presenting blit later stretches.

   Each failure logs and returns its own HRESULT, DDERR_OUTOFVIDEOMEMORY
   passed straight through from either CreateSurface. The clipper failure has
   no out-of-memory path of its own. */

// FUNCTION: EZ2AC_UNPACKED 0x00405900
int KGraphics::createWindowed()
{
    DdSurfaceDesc2 sd;
    IDirectDrawClipper *clip;
    int hr;

    GetClientRect((HWND)m_hwnd, (RECT *)&m_destRect);
    ClientToScreen((HWND)m_hwnd, (POINT *)&m_destRect);
    ClientToScreen((HWND)m_hwnd, (POINT *)&m_destRect.right);
    m_width = m_destRect.right - m_destRect.left;
    m_height = m_destRect.bottom - m_destRect.top;

    memset(&sd, 0, sizeof(sd));
    sd.raw[0] = sizeof(sd);
    sd.raw[1] = 1;                    /* DDSD_CAPS */
    sd.raw[0x68 / 4] = 0x200;         /* DDSCAPS_PRIMARYSURFACE */
    hr = ddraw->CreateSurface(&sd, &surface, 0);
    if (hr < 0)
    {
        /* The literal is already pinned by createPrimary above - one STRING
           marker per address, and a second is dropped with an error. */
        Log("KGraphics: Error: Can't create primary surface");
        if (hr == (int)0x8876017c)
            goto outOfMemory;
        return 0x82000008;
    }

    if (ddraw->CreateClipper(0, &clip, 0) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487984
        Log("KGraphics: Error: Couldn't create clipper");
        return 0x82000009;
    }

    clip->SetHWnd(0, m_hwnd);
    surface->SetClipper(clip);
    if (clip != 0)
    {
        clip->Release();
        clip = 0;
    }

    sd.raw[0xc / 4] = m_width;
    sd.raw[8 / 4] = m_height;
    sd.raw[1] = 7;                    /* DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH */
    sd.raw[0x68 / 4] = 0x2040;        /* DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE */
    hr = ddraw->CreateSurface(&sd, &m_back, 0);
    if (hr < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487958
        Log("KGraphics:  Couldn't create the backbuffer");
        if (hr != (int)0x8876017c)
            return 0x8200000b;
        goto outOfMemory;
    }

    return 0;

outOfMemory:
    Log("KGraphics: Error: Out of video memory");
    return (int)0x8876017c;
}

/* KGraphics::reloadBmpTexture @0x407e50 - loadBmpTexture's twin for a texture
   that already has its surface: the .bmp is read into the shared header
   buffer (decrypted when fd4 asks) and stretched onto the texture's own
   surface at the texture's own size; a missing file paints the label instead.
   The log string names it CreateDummyTexture2. */
// FUNCTION: EZ2AC_UNPACKED 0x00407e50
int KGraphics::reloadBmpTexture(char *name, KTexture *tex)
{
    void *dc;
    KFile f;
    char base[0x104];
    unsigned char *bits;

    if (tex == 0)
        return 0;
    if (f.open(name, 1) == 0)
    {
        extractBaseName(name, base);
        return drawDummyTextureLabel(base, tex);
    }
    f.read(g_bmpFileHeader, f.size());
    f.close();
    if (fd4 != 0)
    {
        BMP_OFFBITS ^= 0x109a;
        BMP_BITCOUNT ^= 0xb18f;
        BMP_WIDTH ^= 0xcfa1;
        BMP_HEIGHT ^= 0x51ae;
    }
    bits = &g_bmpFileHeader[BMP_OFFBITS];
    if (tex->surface->GetDC(&dc) >= 0)
    {
        if (g_d36cb0 == 0)
            StretchDIBits((HDC)dc, 0, 0, (int)tex->w, (int)tex->h, 0, 0, BMP_WIDTH, BMP_HEIGHT, bits, (BITMAPINFO *)g_bmpInfo, DIB_RGB_COLORS, SRCCOPY);
        else
            StretchDIBits((HDC)dc, 0, 0, (int)tex->w, (int)tex->h, 0, BMP_HEIGHT + 1, BMP_WIDTH, -BMP_HEIGHT, bits, (BITMAPINFO *)g_bmpInfo, DIB_RGB_COLORS, SRCCOPY);
        tex->surface->ReleaseDC(dc);
    }
    return 1;
}

/* Which bit of a glyph row byte each column uses, LSB first - so column c is
   1 << (c & 7). Derived from the shift/and pair both glyph loops index it
   with; the CJK draw below shares it. */
// GLOBAL: EZ2AC_UNPACKED 0x00487c64
const unsigned char g_bitMask[8] = { 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80 };

/* KGraphics::drawAsciiGlyphWide @0x406ea0 - the half-width companion of the CJK
   drawGlyph @0x406c80 (parked, wip/kgraphics-drawglyph.cpp), and built to
   the same pattern: one 1bpp glyph straight into the locked back buffer,
   rows that fall off the bottom ending the draw, and the row byte reached by
   an INDEX rather than a walking pointer.

   Three things differ from the CJK one, and all three are visible in the
   codegen. The font is PROPORTIONAL, so the column loop runs to
   g_asciiWidth[glyph] - re-read every iteration, because the pixel store
   could alias it - instead of a fixed sixteen. The row is ONE byte, not two,
   so the glyph base is a bare glyph * 16. And the pixel is written 32 bits
   at a time with the destination stepping four bytes over a pitch counted in
   16-bit units, i.e. the glyph is drawn at DOUBLE horizontal scale, two
   identical 16-bit pixels per column; the colour therefore arrives already
   packed as a pair and is stored through unchanged, where the CJK draw packs
   0x00RRGGBB down to one RGB565 pixel up front.

   The column's divide is SIGNED here (`cltd; and 7; add; sar 3`) where the
   CJK one shifts unsigned - so `col / 8`, not `(unsigned)col >> 3`. */

// FUNCTION: EZ2AC_UNPACKED 0x00406ea0
void KGraphics::drawAsciiGlyphWide(int x, int y, int glyph, int color)
{
    int row;
    int col;

    if (glyph < 0 || glyph >= 0x80)
        return;

    for (row = 0; row < 16; row++)
    {
        if (y + row >= m_height)
            return;

        int *dst = (int *)(m_lockBits + (m_lockPitch * (y + row) + 2 * x));

        for (col = 0; col < g_asciiWidth[glyph]; col++)
        {
            if (g_asciiBits[glyph * 16 + row + col / 8] & g_bitMask[col & 7])
                *dst = color;
            dst++;
        }
    }
}

/* A D3DMATERIAL7: four colour quadruples and a power, 0x44 bytes, which is
   exactly the 17 dwords setMaterial's memset clears. */
struct D3DColorValue7 { float r, g, b, a; };
struct D3DMaterial7
{
    D3DColorValue7 diffuse;    /* 0x00 */
    D3DColorValue7 ambient;    /* 0x10 */
    D3DColorValue7 specular;   /* 0x20 */
    D3DColorValue7 emissive;   /* 0x30 */
    float          power;      /* 0x40 */
};

/* KGraphics::setMaterial @0x4069d0 - publish one packed ARGB twice over.

   It goes straight out as render state 0x8b (D3DRS_AMBIENT) in its packed
   form, and is then unpacked into a material whose DIFFUSE and AMBIENT are
   both set to it, everything else left zero. So a background item's colour
   word drives the scene ambient and the mesh material together.

   Each channel is converted through an UNSIGNED int, which is what the
   `test`/`jge`/`fadd 4294967296.0` fixup around every `fild` is - cl has no
   unsigned x87 convert, so it adds 2^32 back when the value reads negative.
   The fixup is dead here, since every channel is masked to 0..255 first, but
   the original emits it four times and so the source really did convert
   through unsigned. The scale is 1/255, and the parameter itself is
   unsigned - written `int` the two high shifts come out `sar`, not `shr`.

   The eight stores go CHANNEL BY CHANNEL, each value written to diffuse and
   ambient back to back, not diffuse's four then ambient's four. All four
   conversions still happen first, so this is not about evaluation order -
   it is about how long each value stays live. Grouped by channel, cl spills
   blue and alpha to scratch and copies their BITS into both structs with
   integer moves, keeping only red and green on the x87 stack; grouped by
   struct it keeps all four there and duplicates with `fld st(n)`. The
   original does the former. Three other orders were measured: diffuse then
   ambient 78.95%, ambient then diffuse 76.32%, ambient read back out of
   diffuse 58.99%, and a chained `a = b = v` 60.15%. */
// FUNCTION: EZ2AC_UNPACKED 0x004069d0
void KGraphics::setMaterial(unsigned int color)
{
    D3DMaterial7 mat;
    float r;
    float g;
    float b;
    float a;

    device->SetRenderState(0x8b, (int)color);

    memset(&mat, 0, sizeof(mat));

    r = (float)((color >> 16) & 0xff) * (1.0f / 255.0f);
    g = (float)((color >> 8) & 0xff) * (1.0f / 255.0f);
    b = (float)(color & 0xff) * (1.0f / 255.0f);
    a = (float)(color >> 24) * (1.0f / 255.0f);

    mat.diffuse.r = r;
    mat.ambient.r = r;
    mat.diffuse.g = g;
    mat.ambient.g = g;
    mat.diffuse.b = b;
    mat.ambient.b = b;
    mat.diffuse.a = a;
    mat.ambient.a = a;

    device->SetMaterial(&mat);
}

/* KGraphics::drawAsciiGlyph @0x406dd0 - the plain half-width glyph draw, and
   the third member of the family: it shares drawAsciiGlyphWide's proportional
   font and signed column divide, and drawGlyph's RGB565 pack.

   What is its own is that it writes ONE 16-bit pixel per column at the plain
   x, where the wide one writes a 32-bit pair over a pitch counted in 16-bit
   units. So this is the ordinary text draw and that one is the doubled
   variant.

   The packed colour lands in the COLOUR PARAMETER'S OWN home slot, which is
   dead by then, and is read back sixteen bits at a time for each pixel. */
// FUNCTION: EZ2AC_UNPACKED 0x00406dd0
void KGraphics::drawAsciiGlyph(int x, int y, int glyph, unsigned int color)
{
    unsigned int packed;
    int row;
    int col;

    if (glyph < 0 || glyph >= 0x80)
        return;

    /* The cast is load-bearing, exactly as it is in drawGlyph: without it cl
       factors the common >> 3 out of all three terms and packs in five
       instructions instead of the original's ten. */
    packed = (unsigned short)(((color >> 8) & 0xf800) | ((color >> 5) & 0x7e0) |
                              ((color >> 3) & 0x1f));

    for (row = 0; row < 16; row++)
    {
        if (y + row >= m_height)
            return;

        unsigned short *dst = m_lockBits + (m_lockPitch * (y + row) + x);

        for (col = 0; col < g_asciiWidth[glyph]; col++)
        {
            if (g_asciiBits[glyph * 16 + row + col / 8] & g_bitMask[col & 7])
                *dst = (unsigned short)packed;
            dst++;
        }
    }
}

/* KGraphics::drawGlyph @0x406c80 - the CJK glyph draw, integrated at 70.97%
   with EVERY instruction present and in the original's order. The whole
   remaining difference is which register holds the `glyph` parameter and the
   two scheduling effects that follow: the original keeps it in ebx, a
   callee-saved register pushed at entry ahead of ebp.

   IT LIVES HERE, not in its own TU, because it shares g_bitMask with the two
   half-width glyph draws above - and that table is a file-scope `const`, which
   in C++ has INTERNAL linkage. Adding `extern` to reach it from another TU
   compiles, but changes how cl references it from THIS one and cost
   drawAsciiGlyph and drawAsciiGlyphWide their 100%. Same TU is the fix that
   costs nothing. */
// FUNCTION: EZ2AC_UNPACKED 0x00406c80
void KGraphics::drawGlyph(int x, int y, int glyph, unsigned int color)
{
    int row;
    int col;
    unsigned int packed;

    if (glyph < 0 || glyph >= 0x92e)
        return;

    /* The cast is load-bearing: without it cl factors the common >> 3 out of
       all three terms and packs in five instructions instead of eight. */
    packed = (unsigned short)(((color >> 8) & 0xf800) | ((color >> 5) & 0x7e0) |
                              ((color >> 3) & 0x1f));

    for (row = 0; row < 16; row++)
    {
        if (y + row >= m_height)
            return;

        unsigned short *dst = m_lockBits + (m_lockPitch * (y + row) + x);
        /* An INDEX, not a pointer: the original keeps the glyph's base in a
           register and the array in the displacement. */
        int base = (glyph * 16 + row) * 2;

        for (col = 0; col < 16; col++)
        {
            /* The column counter is signed (the loop test is `jl`) but its
               shift is not (`shr`), so the cast is the original's own. */
            if (g_glyphBits[base + ((unsigned int)col >> 3)] &
                g_bitMask[col & 7])
                *dst = (unsigned short)packed;
            dst++;
        }
    }
}

/* KGraphics::m406d40 @0x406d40 - blit one 16x16 glyph of the big font
   into the locked surface at (x, y) in the given colour: two bytes per
   row, a set bit paints a 32-bit pixel, rows past the surface height
   stop the blit. */
// FUNCTION: EZ2AC_UNPACKED 0x00406d40
void KGraphics::m406d40(int x, int y, int c, int color)
{
    int r, i;

    if (c < 0 || c >= 0x92e)
        return;
    for (r = 0; r < 16; r++)
    {
        int *dst;
        int  bits;

        if (y + r >= m_height)
            return;
        dst = (int *)(m_lockBits + m_lockPitch * (y + r) + x * 2);
        bits = c * 16 + r;
        i = 0;
        bits = bits + bits;
        for (; i < 16; i++)
        {
            if (g_glyphBits[bits + ((unsigned int)i >> 3)] & g_bitMask[i & 7])
                *dst = color;
            dst++;
        }
    }
}

/* The 0x28-byte block the video-blit setter takes by value: a
   BITMAPINFOHEADER, of which only biWidth (v[1]) and biHeight (v[2]) are
   read here - the whole block is handed on to StretchDIBits as the
   BITMAPINFO. */
struct GfxBlob28 { int v[10]; };

/* Hand one frame of bits to a texture: GetDC on its surface (the HDC lands
   in the first parameter's own slot - the pointer is cached in a register
   and its home is reused), a write-only Lock through the texture's own
   descriptor, StretchDIBits the frame in 1:1 at the header's size, then
   ReleaseDC and Unlock. Always returns 1. */
// FUNCTION: EZ2AC_UNPACKED 0x00408040
int KGraphics::m408040(KTexture *tex, void *p, GfxBlob28 s)
{
    KTexture *t = tex;

    t->surface->GetDC((void **)&tex);
    t->surface->Lock(0, &t->desc, 0x20, 0);
    StretchDIBits((HDC)tex, 0, 0, s.v[1], s.v[2], 0, 0, s.v[1], s.v[2],
                  p, (BITMAPINFO *)&s, 0, SRCCOPY);
    t->surface->ReleaseDC(tex);
    t->surface->Unlock(0);
    return 1;
}

extern int  hasExtension(const char *ext, char *name);   /* src/strutil.cpp */
extern void replaceExtension(char *name, char *ext);     /* 0x413000 */

/* Fetch a sprite by its profile NAME, reloading its pixels from `path`: a
   cache hit reloads the .bmp onto the cached texture in place (through the
   .abm swap when fd4 says the assets are scrambled, and through the "TGA"
   placeholder for any other extension) and hands that texture back; a miss
   falls through to loadTexture with the name as the cache key. */
// FUNCTION: EZ2AC_UNPACKED 0x00408e80
void KGraphics::loadSprite(const char *name, char *path, Sprite **out)
{
    char buf[260];
    KProfileNode *n;
    KTexture *t;

    /* The original lays this arm OUT of line (je forward past the whole hit
       path, its own epilogue) with the hit path falling through - but every
       spelling that reproduces that layout (if/else both ways, the *out
       store shared or per-arm, the condition materialized into a flag) makes
       cl DUPLICATE the shared reloadBmpTexture tail into all three arms
       instead of cross-jumping them the way the original does. This
       early-return form is the only one whose arms merge into the original's
       single push/push/jmp tail; the cost is the inverted branch and this
       block's position, a layout coin. 70 of 71 instructions, calls exact. */
    n = m_profile11c->find((char *)name);
    if (n == 0)
    {
        loadTexture(path, (KTexture **)out, (char *)name);
        return;
    }
    t = (KTexture *)&n->value;
    if (hasExtension(".bmp", path))
    {
        if (fd4 != 0)
        {
            strcpy(buf, path);
            replaceExtension(buf, ".abm");
            reloadBmpTexture(buf, t);
        }
        else
        {
            reloadBmpTexture(path, t);
        }
    }
    else
    {
        reloadBmpTexture((char *)"TGA", t);
    }
    *out = (Sprite *)t;
}
