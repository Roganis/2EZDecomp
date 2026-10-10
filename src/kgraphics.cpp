#include <windows.h>
#include "kcrt.h"      /* the array operator new is a real call */
#include "obj405f80.h" /* releaseAll - the same object under its other name */
#include "ez2acapp.h"
#include <string.h>
#include <stdlib.h>
#include "kfile.h"

/* src/strutil.cpp - both already matched. */
extern int  hasExtension(const char *ext, char *name);
extern void replaceExtension(char *name, char *ext);

/* WINMM's timeGetTime, declared WITHOUT dllimport so the call goes through the
   linker's thunk @0x4145f0 - which is the form resetFrameTimer uses. (The
   original mixes the two forms; the players in src/obj414750.cpp take
   mmsystem.h's dllimport declaration and the indirect call.) */
extern "C" DWORD __stdcall timeGetTime(void);

/* Methods of the graphics singleton (include/ez2acapp.h). Their callers live in
   src/ezscreens.cpp and src/moderunner.cpp, so these must stay here. */

/* The counting callback countSurfaces enumerates with: bump the caller's counter
   and keep going. __stdcall with three arguments (`ret 0xc`); only the third is
   used. */
// FUNCTION: EZ2AC_UNPACKED 0x00408ae0
int __stdcall countSurfaceCallback(int /*a*/, int /*b*/, int *counter)
{
    (*counter)++;
    return 1;
}

/* How many surfaces the container holds. */
// FUNCTION: EZ2AC_UNPACKED 0x00408af0
int KGraphics::countSurfaces()
{
    int count = 0;

    ddraw->EnumSurfaces(0x11, 0, &count, countSurfaceCallback);
    return count;
}

/* One frame's worth of window servicing, then let the device begin the scene. */
// FUNCTION: EZ2AC_UNPACKED 0x00405e10
void KGraphics::m405e10()
{
    g_app->pumpMessages();
    device->BeginScene();
}

/* The blend mode: D3D's SRCBLEND (0x13) and DESTBLEND (0x14) render states,
   through the device's slot 20 - which is where IDirect3DDevice7 keeps
   SetRenderState (see include/icom.h). */
// FUNCTION: EZ2AC_UNPACKED 0x0040a490
void KGraphics::setBlend(int src, int dst)
{
    device->SetRenderState(0x13, src);
    device->SetRenderState(0x14, dst);
}

/* Restart the frame counter. Obj401780::s2 @0x4184f0 tail-jumps here once it has
   forced the mixer volumes, so a screen starts its timing from zero. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a750
void KGraphics::resetFrameTimer()
{
    m_frames = 0;
    m_frameBase = timeGetTime();
}

/* Count a frame, and once a second has gone by publish the count as the frame
   rate and start the next second. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a770
void KGraphics::beginFrame()
{
    m_frames++;
    if (timeGetTime() - m_frameBase >= 1000)
    {
        m_fps = m_frames;
        m_frames = 0;
        m_frameBase = timeGetTime();
    }
}

/* Throw the graphics profile away and start it over: hand its whole node tree
   back, then re-initialise it. The profile pointer is re-read after the call
   because freeNodes may have written it. */
// FUNCTION: EZ2AC_UNPACKED 0x00403f60
void KGraphics::m403f60()
{
    m_profile->freeNodes(m_profile->root);
    m_profile->init();
}

/* Close the frame the device has open, then let the rest of the graphics object
   finish up (a tail call - m405d80 has to stay in another TU). */
// FUNCTION: EZ2AC_UNPACKED 0x00405e30
void KGraphics::endFrame()
{
    device->EndScene();
    m405d80();
}

/* Let go of everything the device and DirectDraw are holding: unbind all four
   texture stages, put every surface back, and reset the other profile. */
// FUNCTION: EZ2AC_UNPACKED 0x00408c80
void KGraphics::m408c80()
{
    if (device != 0)
    {
        device->SetTexture(0, 0);
        device->SetTexture(1, 0);
        device->SetTexture(2, 0);
        device->SetTexture(3, 0);
    }

    if (ddraw != 0)
    {
        ddraw->RestoreAllSurfaces();
    }

    /* The texture cache stores KTextureNodes in the generic tree field. */
    m_profile11c->freeNodes408b40((KTextureNode *)m_profile11c->root);
    m_profile11c->init();
}

/* Fade the screen by drawing one alpha-only quad over all of it: the blend is
   set to ZERO/SRCALPHA so the framebuffer is scaled by the quad's alpha, and the
   level - clamped to a byte - goes into the colour's alpha channel. The quad is
   5 px oversized on every side (-5,-5 by 650x490 over a 640x480 screen).

   The clamp has to be written as two assignments to ONE variable, not as a
   three-way pick: `else if (level < 0) a = 0; else a = level;` makes cl select
   branchlessly (xor/setl/sub/and), where overwriting a value that is already
   there keeps the original's `test/jge` branch. */
// FUNCTION: EZ2AC_UNPACKED 0x0040b080
void KGraphics::setFade(int level)
{
    m4080c0(0, 0);
    m4080c0(1, 0);
    m408130(0);

    device->SetRenderState(0x13, 1);
    device->SetRenderState(0x14, 5);

    if (level >= 255)
    {
        level = 255;
    }
    else if (level < 0)
    {
        level = 0;
    }

    drawRect(-5.0f, -5.0f, 650.0f, 490.0f, level << 24);
}

/* Save the back buffer to ScreenShot\\shot%04d.bmp. A 640x480 24-bit DIB section
   is created, the surface's own DC is blitted into it, and the two bitmap headers
   and the pixels go out through a stack KFile - whose destructor is what gives
   this function its EH frame. The file header claims a size of just the headers,
   which is what the original writes. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a7b0
void KGraphics::screenShot()
{
    BITMAPINFOHEADER bmih;
    BITMAPFILEHEADER bfh;
    char name[260];
    void *bits;
    HDC dc;
    HBITMAP bmp;
    HDC memDC;

    bmih.biSize = 0x28;
    bmih.biWidth = 640;
    bmih.biHeight = 480;
    bmih.biPlanes = 1;
    bmih.biBitCount = 24;
    bmih.biCompression = 0;
    bmih.biSizeImage = 0xe1000;
    bmih.biXPelsPerMeter = 0;
    bmih.biYPelsPerMeter = 0;
    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

    bmp = CreateDIBSection(0, (BITMAPINFO *)&bmih, 0, &bits, 0, 0);
    surface->GetDC((void **)&dc);
    memDC = CreateCompatibleDC(dc);
    SelectObject(memDC, bmp);
    BitBlt(memDC, 0, 0, 640, 480, dc, 0, 0, SRCCOPY);

    bfh.bfType = 0x4d42;
    bfh.bfSize = 0x36;
    bfh.bfReserved2 = 0;
    bfh.bfReserved1 = 0;
    bfh.bfOffBits = 0x36;

    // STRING: EZ2AC_UNPACKED 0x00487fac
    wsprintfA(name, "ScreenShot\\shot%04d.bmp", m_shotCount);

    /* Declared here, not at the top: the original constructs the file object at
       this point and destroys it at the end of the function. */
    KFile file;

    file.open(name, 2);
    file.write(&bfh, 14);
    file.write(&bmih, 40);
    file.write(bits, bmih.biSizeImage);
    file.close();

    DeleteDC(memDC);
    surface->ReleaseDC(dc);
    DeleteObject(bmp);
    m_shotCount++;
}

/* KGraphics's device enumeration entry @0x004054d0: stash the caller's
   context where the enumeration callback @0x405310 can find it, enumerate
   every DirectDraw device including the attached and non-display ones
   (DDENUM_ATTACHEDSECONDARYDEVICES | DETACHEDSECONDARYDEVICES |
   NONDISPLAYDEVICES = 7), and turn the two counters the callback keeps into
   the two failure codes. A FREE function, not a KGraphics member: it ends
   in a plain `ret` with the argument left on the stack, which is __cdecl.

   The call is a five-byte thunk, which is what the linker builds when the
   API is declared WITHOUT __declspec(dllimport) - ddraw.h's own declaration
   would give the six-byte indirect form - so it is redeclared here.
   ddraw.h cannot be included here - it redefines the COM interfaces
   include/icom.h models - so the prototype is spelled out with generic
   pointer arguments. The decorated name only depends on the twelve bytes of
   arguments, so it still resolves to _DirectDrawEnumerateExA@12. */
extern "C" long __stdcall DirectDrawEnumerateExA(void *cb, void *ctx,
                                                 unsigned long flags);
extern void Log(const char *fmt, ...);

/* The callback's three globals: the context it reads, and the two counts it
   raises. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x00d36ca0
void *g_enumContext;
// GLOBAL: EZ2AC_UNPACKED 0x00d36ca4
int g_enumDevices;
// GLOBAL: EZ2AC_UNPACKED 0x00d36ca8
int g_enumAccepted;

int __stdcall enumDevice(void *, char *, char *, void *, void *);  /* 0x405310 (below) */

// FUNCTION: EZ2AC_UNPACKED 0x004054d0
int enumerateDevices(void *ctx)
{
    g_enumContext = ctx;
    DirectDrawEnumerateExA((void *)enumDevice, 0, 7);

    if (g_enumDevices == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048779c
        Log("KGraphics: No devices and/or modes were enumerated!");
        return 0x81000002;
    }

    if (g_enumAccepted == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048776c
        Log("KGraphics: No enumerated devices were accepted!");
        // STRING: EZ2AC_UNPACKED 0x00487734
        Log("KGraphics: Try enabling the D3D Reference Rasterizer.");
        return 0x81000003;
    }

    return 0;
}

/* A 16-byte GUID as the raw block the probe copies it as, and the two big
   capability blocks as sized blobs - ddraw.h cannot be included here (it
   redefines what include/icom.h models), so only the fields the code reads
   are named inside them. */
struct GuidBlob  { int d[4]; };
struct Desc7Blob                  /* D3DDEVICEDESC7, 0xec bytes */
{
    unsigned int devCaps;         /* 0x00 - bit 19 is HWTRANSFORMANDLIGHT */
    int v[(0x74 - 4) / 4];
    unsigned int renderBitDepth;  /* 0x74 - DDBD_16/24/32 bits */
    int w[(0xc4 - 0x78) / 4];
    GuidBlob deviceGuid;          /* 0xc4 */
    int x[(0xec - 0xd4) / 4];
};
struct CapsBlob                   /* DDCAPS, 0x17c bytes */
{
    unsigned int size;            /* 0x00 - always written 0x17c */
    unsigned int caps;            /* 0x04 */
    unsigned int caps2;           /* 0x08 - bit 19 gates `valid` */
    int v[(0x17c - 0xc) / 4];
};

/* One accepted device, as the enumeration callback @0x405310 leaves it - and
   ALSO the callback's own working context: @0x405310 builds one of these on
   its stack per adapter and the device callback @0x405060 copies out of it,
   which is why both zero exactly 0x134 dwords. The entries are 0x4d0 bytes
   and twenty of them fill 0xd30c60 .. 0xd36ca0, which is where the counts
   start. */
struct D3DDeviceEntry
{
    char name[0x28];              /* 0x000 - lstrcpynA'd driver text, 0x27 max */
    GuidBlob *guid;               /* 0x028 - points at guidData below */
    Desc7Blob desc7;              /* 0x02c - the D3DDEVICEDESC7 copy */
    int  isHardware;              /* 0x118 - devCaps & 0x80000 */
    GuidBlob *adapterGuid;        /* 0x11c - 0 on the primary display */
    CapsBlob caps;                /* 0x120 - the driver DDCAPS */
    CapsBlob helCaps;             /* 0x29c - the HEL DDCAPS */
    DDSurfaceDesc2 mode640;       /* 0x418 - the 640x480x32 mode, if found */
    int  chosen;                  /* 0x494 - seeded from `valid`, set on the winner */
    int  _pad498;                 /* 0x498 */
    GuidBlob guidData;            /* 0x49c - the device GUID */
    GuidBlob adapterGuidData;     /* 0x4ac - the adapter GUID */
    /* 0x4bc - a heap block the entry owns (its accepted-mode list);
       freeTable405530 @0x405530 frees and nulls it for every accepted
       entry. */
    DDSurfaceDesc2 *owned;
    int  modeCount;               /* 0x4c0 */
    int  mode640Index;            /* 0x4c4 */
    int  valid;                   /* 0x4c8 - primary adapter with caps2 bit 19 */
    int  hasStereoMode;           /* 0x4cc - an accepted mode had ddsCaps2 bit 19 */
};

// GLOBAL: EZ2AC_UNPACKED 0x00d30c60
D3DDeviceEntry g_enumTable[20];

extern int g_iidDirect3D7[4];     /* 0x4a01b0 (src/globals.cpp) */
extern int g_iidDirectDraw7[4];   /* 0x4a01c0 (src/globals.cpp) */

/* The same five-byte-thunk treatment as DirectDrawEnumerateExA above. */
extern "C" long __stdcall DirectDrawCreateEx(void *guid, void **out,
                                             void *iid, void *unk);

/* qsort's comparator over the per-adapter mode list: ascending by width,
   then height, then bit depth (the last spelled branchless by cl). */
// FUNCTION: EZ2AC_UNPACKED 0x00404f70
int __cdecl compareModes(const void *pa, const void *pb)
{
    DDSurfaceDesc2 *a = (DDSurfaceDesc2 *)pa;
    DDSurfaceDesc2 *b = (DDSurfaceDesc2 *)pb;

    if (a->dwWidth < b->dwWidth)
        return -1;
    else if (a->dwWidth > b->dwWidth)
        return 1;
    else if (a->dwHeight < b->dwHeight)
        return -1;
    else if (a->dwHeight > b->dwHeight)
        return 1;
    else
    {
        /* Loaded a-first: the original reads a's depth, then b's, and
           compares b against a - the named pair is what pins that order. */
        unsigned int da = a->dwRGBBitCount;
        unsigned int db = b->dwRGBBitCount;
        if (db > da)
            return -1;
        return db < da;
    }
}

/* The EnumDisplayModes callback: append this mode to the context's list by
   reallocating it one element bigger each time. The list pointer is
   published before the new mode is copied in, as the original stores it. */
// FUNCTION: EZ2AC_UNPACKED 0x00404fb0
long __stdcall enumMode(DDSurfaceDesc2 *mode, void *pctx)
{
    D3DDeviceEntry *ctx = (D3DDeviceEntry *)pctx;
    DDSurfaceDesc2 *nw;

    nw = new DDSurfaceDesc2[ctx->modeCount + 1];   /* the ARRAY new - a real CRT call (kcrt.h) */
    if (ctx->owned != 0)
    {
        memcpy(nw, ctx->owned, ctx->modeCount * 0x7c);
        if (ctx->owned != 0)
        {
            delete ctx->owned;
            ctx->owned = 0;
        }
    }
    ctx->owned = nw;
    nw[ctx->modeCount] = *mode;
    ctx->modeCount += 1;
    return 1;
}

/* The EnumDevices callback: claim the next g_enumTable entry and fill it
   from the device description and the per-adapter context @0x405310 built -
   caps, HEL caps, the device GUID (pointed at through +0x28), the adapter
   GUID when there is one, and the adapter's driver text (the D3D device
   NAME on the primary). A secondary adapter without hardware T&L is
   rejected; an acceptance filter installed in g_enumContext can veto too
   (that path leaks the freshly allocated mode list, as the original does).
   The context's modes are then filtered against the device's render bit
   depths into the entry's own list, the 640x480x32 mode is looked up and
   copied aside, and the accepted count advances. */
// FUNCTION: EZ2AC_UNPACKED 0x00405060
long __stdcall enumD3DDevice(char *desc, char *name, void *pdd, void *pctx)
{
    Desc7Blob *dd = (Desc7Blob *)pdd;
    D3DDeviceEntry *ctx = (D3DDeviceEntry *)pctx;
    D3DDeviceEntry *e = &g_enumTable[g_enumAccepted];
    DDSurfaceDesc2 m;
    unsigned int i;

    memset(e, 0, sizeof(D3DDeviceEntry));
    e->isHardware = dd->devCaps & 0x80000;
    e->desc7 = *dd;
    e->valid = ctx->valid;
    e->caps = ctx->caps;
    g_enumDevices += 1;
    e->helCaps = ctx->helCaps;
    e->guidData = dd->deviceGuid;
    e->guid = &e->guidData;
    e->owned = new DDSurfaceDesc2[ctx->modeCount];   /* the ARRAY new again */
    if (ctx->adapterGuid != 0)
    {
        e->adapterGuidData = ctx->adapterGuidData;
        e->adapterGuid = &e->adapterGuidData;
        lstrcpynA(e->name, ctx->name, 0x27);
    }
    else
    {
        e->adapterGuid = 0;
        lstrcpynA(e->name, name, 0x27);
    }

    if (e->adapterGuid != 0 && e->isHardware == 0)
    {
        if (e->owned != 0)
        {
            delete e->owned;
            e->owned = 0;
        }
        return 1;
    }

    if (g_enumContext != 0)
    {
        if (((int (__cdecl *)(void *, void *))g_enumContext)(&e->caps,
                                                             &e->desc7) < 0)
            return 1;
    }

    for (i = 0; i < (unsigned int)ctx->modeCount; i++)
    {
        unsigned int depth = e->desc7.renderBitDepth;

        m = ctx->owned[i];
        /* A bpp DISPATCH, not a flat || - a mode whose depth bit is off
           skips the remaining arms outright (each arm's failed test jumps
           to the loop tail, its passed test threads to the shared append
           after the chain). */
        if (m.dwRGBBitCount == 0x20)
        {
            if ((depth & 0x100) == 0)
                continue;
        }
        else if (m.dwRGBBitCount == 0x18)
        {
            if ((depth & 0x200) == 0)
                continue;
        }
        else if (m.dwRGBBitCount == 0x10)
        {
            if ((depth & 0x400) == 0)
                continue;
        }
        else
            continue;

        e->owned[e->modeCount] = m;
        e->modeCount += 1;
        if ((m.dwCaps2 & 0x80000) != 0)
            e->hasStereoMode = 1;
    }

    if (e->modeCount == 0)
        return 1;

    for (i = 0; i < (unsigned int)e->modeCount; i++)
    {
        if (e->owned[i].dwWidth == 0x280 && e->owned[i].dwHeight == 0x1e0 &&
            e->owned[i].dwRGBBitCount == 0x20)
        {
            e->mode640 = e->owned[i];
            e->mode640Index = i;
            break;
        }
    }

    e->chosen = e->valid;
    g_enumAccepted += 1;
    return 1;
}

/* The per-adapter probe DirectDrawEnumerateExA calls: create this adapter's
   own DirectDraw, query its Direct3D, and build a D3DDeviceEntry-shaped
   context on the stack - driver text, both DDCAPS, the adapter GUID when
   one was given, `valid` up on a primary whose driver caps2 has bit 19 -
   then collect and sort its display modes and walk its devices through the
   callback above. Always returns 1 to keep enumerating. */
// FUNCTION: EZ2AC_UNPACKED 0x00405310
int __stdcall enumDevice(void *guid, char *desc, char *name, void *unused,
                         void *monitor)
{
    IDirectDraw7 *dd;
    IDirect3D7 *d3d;
    D3DDeviceEntry ctx;

    if (DirectDrawCreateEx(guid, (void **)&dd, g_iidDirectDraw7, 0) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487700
        Log("KGraphics: Can't create DDraw during enumeration!");
        return 1;
    }
    if (dd->QueryInterface(g_iidDirect3D7, (void **)&d3d) < 0)
    {
        dd->Release();
        // STRING: EZ2AC_UNPACKED 0x004876c8
        Log("KGraphics: Can't query IDirect3D7 during enumeration!");
        return 1;
    }

    memset(&ctx, 0, sizeof(ctx));
    lstrcpynA(ctx.name, desc, 0x27);
    ctx.caps.size = 0x17c;
    ctx.helCaps.size = 0x17c;
    dd->GetCaps(&ctx.caps, &ctx.helCaps);

    if (guid != 0)
    {
        ctx.adapterGuidData = *(GuidBlob *)guid;
        ctx.adapterGuid = &ctx.adapterGuidData;
    }
    if ((ctx.caps.caps2 & 0x80000) != 0 && ctx.adapterGuid == 0)
        ctx.valid = 1;

    dd->EnumDisplayModes(0, 0, &ctx, (void *)enumMode);
    qsort(ctx.owned, ctx.modeCount, 0x7c, compareModes);
    d3d->EnumDevices((void *)enumD3DDevice, &ctx);

    if (ctx.owned != 0)
    {
        delete ctx.owned;
        ctx.owned = 0;
    }
    d3d->Release();
    dd->Release();
    return 1;
}

extern int g_iidTnLHalDevice[4];  /* 0x4a0180 (src/globals.cpp) */
extern int g_iidRefDevice[4];     /* 0x4a0190 (src/globals.cpp) */

/* The D3DDEVICEDESC7 as the z-buffer pick reads it - the char pad is
   load-bearing: it is what gives the function its /GS cookie. */
struct D3DCapsBlob
{
    unsigned int devCaps;         /* 0x00 */
    int  v[(0x44 - 4) / 4];
    unsigned int f44;             /* 0x44 - bit 15 skips the z-buffer */
    char _pad48[0xec - 0x48];
};

/* The EnumZBufferFormats callback is copyIfSameTag @0x405650 - already
   reconstructed in src/accessors.cpp as the generic tagged-record copy: the
   "tag" at +0xc is the pixel format's depth, the 32-byte copy is the whole
   DDPIXELFORMAT, and returning 0 on the copy is D3DENUMRET_CANCEL. */
extern int __stdcall copyIfSameTag(struct TaggedRec *src, struct TaggedRec *dst);

/* Step one of the bring-up: this adapter's DirectDraw into +0x28, the
   cooperative level (NORMAL windowed, EXCLUSIVE|FULLSCREEN|ALLOWREBOOT
   fullscreen, MULTITHREADED unless bit 3 of the flags), and a sanity check
   that the desktop is deeper than 8 bits. */
// FUNCTION: EZ2AC_UNPACKED 0x00405680
int KGraphics::m405680(void *adapterGuid, int flags)
{
    DDSurfaceDesc2 desc;
    int level;

    if (DirectDrawCreateEx(adapterGuid, (void **)&ddraw, g_iidDirectDraw7,
                           0) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487860
        Log("KGraphics: Could not create DirectDraw");
        return (int)0x82000001;
    }

    /* An overwrite, not a select - the ternary goes branchless. */
    level = 0x8;
    if (f8 != 0)
        level = 0x13;
    /* Constant FIRST: `(flags & 8) == 0` gets the test narrowed to the low
       byte where the original tests the dword (97.52%); `0 == (flags & 8)`
       keeps it wide. Found by scratchpad/perm. */
    if (0 == (flags & 8))
        level |= 0x800;
    if (ddraw->SetCooperativeLevel(m_hwnd, level) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048783c
        Log("KGraphics: Couldn't set coop level");
        return (int)0x82000002;
    }

    desc.dwSize = 0x7c;
    ddraw->GetDisplayMode(&desc);
    return desc.dwRGBBitCount > 8 ? 0 : (int)0x8200000e;
}

/* The z-buffer: skip it when the device says so, else walk the device's
   z formats for the back buffer's own depth (then for 32-bit), create the
   surface in the memory class the vertex flags picked, attach it to the
   back buffer - and to the left one when stereo is on - and point the
   device at the back buffer. */
// FUNCTION: EZ2AC_UNPACKED 0x00405b60
int KGraphics::m405b60(void *deviceGuid)
{
    DDSurfaceDesc2 desc;
    D3DCapsBlob caps;
    int hr;

    device->GetCaps(&caps);
    if ((caps.f44 & 0x8000) != 0)
        return 0;

    desc.dwSize = 0x7c;
    m_back->GetSurfaceDesc(&desc);
    desc.dwCaps = m_vbFlags | 0x20000;
    desc.dwFlags = 0x1007;
    desc.ddpfSize = 0;
    d3d->EnumZBufferFormats(deviceGuid, (void *)copyIfSameTag,
                            (char *)&desc + 0x48);
    if (desc.ddpfSize == 0)
    {
        desc.dwRGBBitCount = 0x20;
        d3d->EnumZBufferFormats(deviceGuid, (void *)copyIfSameTag,
                                (char *)&desc + 0x48);
        if (desc.ddpfSize == 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00487b44
            Log("KGraphics: Device doesn't support requested zbuffer format");
            return (int)0x82000005;
        }
    }

    hr = ddraw->CreateSurface(&desc, &m_zbuf, 0);
    if (hr < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487b10
        Log("KGraphics: Error: Couldn't create a ZBuffer surface");
        if (hr == (int)0x8876017c)
        {
            /* The literal's STRING marker lives in src/kgraphics2.cpp. */
            Log("KGraphics: Error: Out of video memory");
            return hr;
        }
        return (int)0x82000005;
    }

    if (m_back->AddAttachedSurface(m_zbuf) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487ad4
        Log("KGraphics: Error: Couldn't attach zbuffer to render surface");
        return (int)0x82000005;
    }
    if (fc != 0)
    {
        if (m_backLeft->AddAttachedSurface(m_zbuf) < 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00487a90
            Log("KGraphics: Error: Couldn't attach zbuffer to left render surface");
            return (int)0x82000005;
        }
    }
    if (device->SetRenderTarget(m_back, 0) < 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487a48
        Log("KGraphics: Error: SetRenderTarget() failed after attaching zbuffer!");
        return (int)0x82000005;
    }
    return 0;
}

extern int g_iidHALDevice[4];     /* 0x4a01a0 (src/globals.cpp) */

/* The surface bring-up init hands the picked device to: validate the
   arguments (a fullscreen run must have found the 640x480x32 mode), record
   the window, the fullscreen bit and - when the entry advertised it and the
   mode carries the stereo caps bit - the stereo flag, choose hardware or
   software vertex processing by which device GUID this is, then run the
   chain: DirectDraw itself, the primary (fullscreen) or the windowed target,
   the device, and optionally the second device; any failure tears everything
   down and hands the HRESULT up. */
// FUNCTION: EZ2AC_UNPACKED 0x00406050
int KGraphics::m406050(void *hwnd, void *adapterGuid, void *deviceGuid,
                       DDSurfaceDesc2 *mode, int flags)
{
    int hr;
    int fs;

    if (hwnd == 0 || deviceGuid == 0 || (mode == 0 && (flags & 1) != 0))
        return (int)0x80070057;

    fs = flags & 1;
    m_hwnd = hwnd;
    fc = 0;
    f8 = fs;
    if ((flags & 2) != 0 && fs != 0 && (mode->dwCaps2 & 0x80000) != 0)
        fc = 1;

    if (memcmp(deviceGuid, g_iidHALDevice, 16) == 0)
        m_vbFlags = 0x4000;
    else
        m_vbFlags = memcmp(deviceGuid, g_iidTnLHalDevice, 16) == 0 ? 0x4000
                                                                   : 0x800;

    /* Nested successes over one teardown tail, with the success return
       MATERIALIZED through a flag so it trails the teardown - the
       success-block-after-the-return layout steer. */
    int ok = 0;
    hr = m405680(adapterGuid, flags);
    if (hr >= 0)
    {
        if (fs != 0)
            hr = createPrimary((KDisplaySettings *)mode);
        else
            hr = createWindowed();
        if (hr >= 0)
        {
            hr = m405a90(deviceGuid);
            if (hr >= 0)
            {
                if ((flags & 4) != 0)
                    hr = m405b60(deviceGuid);
                if (hr >= 0)
                    ok = 1;
            }
        }
    }
    if (ok == 0)
    {
        ((Obj405F80 *)this)->releaseAll();
        return hr;
    }
    return 0;
}

/* The GDI text font build init asks for ("Arial", 80, 400): compiled out -
   the original is a bare `ret 0xc`. */
// FUNCTION: EZ2AC_UNPACKED 0x00406bf0
void KGraphics::m406bf0(char *, int, int)
{
}

/* pickDevice @0x405e50 - choose which enumerated device to render with.

   The accepted devices are sorted into four buckets in one pass, by whether
   the callback marked them hardware and by which GUID they carry:

     hardware + IID_IDirect3DTnLHalDevice   the best - hardware T&L
     hardware, any other GUID               plain HAL
     software, any other GUID               RGB or MMX
     software + IID_IDirect3DRefDevice      the reference rasteriser, worst

   Bit 0 of `flags` means "software only": with it set the two hardware
   buckets are skipped entirely, so the search starts at RGB. Without it the
   order is exactly the list above. Whichever wins is marked at +0x494 and
   handed back through the out-pointer; a null out-pointer is 0x80070057 and
   finding nothing at all is 0x81000004.

   Both GUID compares share one `mov ecx,4` and one load of the entry's GUID
   pointer, hoisted above the hardware test - which is what the original has,
   and what writing them as two memcmps of the same size gives.

   68.27%, and it is one allocator decision with a knock-on. The original
   spills `hwOther` to a frame slot and keeps the loop counter in edx; cl
   does the reverse, and the register that frees then goes to the constant 1,
   so the tail spells `mov ecx,1` / `and eax,ecx` where the original has
   immediates. Every instruction of the scan and all four buckets are right.
   Tried and worse: one counter variable instead of two (65.07%), and
   `volatile` on hwOther to pin it to memory, which is the documented lever
   for exactly this and costs thirteen points (55.40%) because it also stops
   cl folding the pointer into the stores. */
// FUNCTION: EZ2AC_UNPACKED 0x00405e50
int pickDevice(void **out, int flags)
{
    D3DDeviceEntry *hwTnL;
    D3DDeviceEntry *hwOther;
    D3DDeviceEntry *swOther;
    D3DDeviceEntry *swRef;
    unsigned int n;

    if (out == 0)
        return 0x80070057;

    n = (unsigned int)g_enumAccepted;
    swRef = 0;
    swOther = 0;
    hwOther = 0;
    hwTnL = 0;

    if (n > 0)
    {
        D3DDeviceEntry *e = g_enumTable;
        unsigned int k = n;

        do
        {
            if (e->valid != 0)
            {
                if (e->isHardware != 0)
                {
                    if (memcmp(e->guid, g_iidTnLHalDevice, 16) == 0)
                        hwTnL = e;
                    else
                        hwOther = e;
                }
                else
                {
                    if (memcmp(e->guid, g_iidRefDevice, 16) == 0)
                        swRef = e;
                    else
                        swOther = e;
                }
            }

            e++;
        }
        while (--k != 0);
    }

    if ((flags & 1) == 0 && hwTnL != 0)
    {
        *out = hwTnL;
        hwTnL->chosen = 1;
        return 0;
    }

    if ((flags & 1) == 0 && hwOther != 0)
    {
        *out = hwOther;
        hwOther->chosen = 1;
        return 0;
    }

    if (swOther != 0)
    {
        *out = swOther;
        swOther->chosen = 1;
        return 0;
    }

    if (swRef != 0)
    {
        *out = swRef;
        swRef->chosen = 1;
        return 0;
    }

    return 0x81000004;
}

/* freeTable405530 @0x405530 - free every accepted device entry's owned block.

   Free, not a member: Obj405F80::v3 @0x405dd0 pops its saved esi and tail-jumps
   here with nothing in ecx. The walk is anchored at the FIELD rather than the
   entry (the original starts at 0xd3111c, which is g_enumTable + 0x4bc, and
   strides a whole 0x4d0 element), which is the documented induction anchoring,
   and the count is compared UNSIGNED - `jbe` past the guard and `jb` at the
   bottom. */
// FUNCTION: EZ2AC_UNPACKED 0x00405530
void freeTable405530()
{
    unsigned int i;

    /* The counter is zeroed BEFORE the guard so the guard compares the count
       against that register rather than an immediate - the documented
       record-loop shape, and worth 20 points here on 20 instructions. */
    i = 0;
    if (i < (unsigned int)g_enumAccepted)
    {
        D3DDeviceEntry *e = g_enumTable;

        do
        {
            if (e->owned != 0)
            {
                delete e->owned;
                e->owned = 0;
            }
            i++;
            e++;
        }
        while (i < (unsigned int)g_enumAccepted);
    }
}
