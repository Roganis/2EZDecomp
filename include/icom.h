#ifndef ICOM_H
#define ICOM_H

/* The COM-style (__stdcall) interfaces the engine objects hold. Only the slots
   actually called are typed; the rest keep the original slot numbering. Lifted
   out of src/com_wrappers.cpp so the graphics methods can use it too. */
struct IThing
{
    virtual int __stdcall v0();
    virtual int __stdcall v1();
    virtual int __stdcall v2();
    virtual int __stdcall v3();
    /* NB: slot 6 (+0x18) and slot 11 (+0x2c) are used by the wrappers at the
       bottom of this file; the names below keep the original slot numbering. */
    virtual int __stdcall query4(int *out, int flag);   /* slot 4 -> vtable+0x10 */
    virtual int __stdcall v5();
    virtual int __stdcall query6(int *out);             /* slot 6 -> vtable+0x18 */
    virtual int __stdcall query7(int *out);             /* slot 7 -> vtable+0x1c */
    virtual int __stdcall query8(int *out);             /* slot 8 -> vtable+0x20 */
    virtual int __stdcall release();                    /* slot 9 -> vtable+0x24 */
    virtual int __stdcall op10(int a, int b, int c, int d, float e, int f);
                                                        /* slot 10 -> vtable+0x28 */
    virtual int __stdcall sub11(int a, void *b);        /* slot 11 -> vtable+0x2c */
    /* slot 12 -> vtable+0x30. Obj40D4E0::v4 drives it with (0, 0, flag). */
    virtual int __stdcall op12(int a, int b, int c);
    /* slot 13 -> vtable+0x34. Obj40D4E0::v5 drives it with 0. */
    virtual int __stdcall op13(int a);
    virtual int __stdcall v14();
    virtual int __stdcall set15(int v);                 /* slot 15 -> vtable+0x3c */
    virtual int __stdcall set16(int v);                 /* slot 16 -> vtable+0x40 */
    virtual int __stdcall set17(int v);                 /* slot 17 -> vtable+0x44 */
    virtual int __stdcall v18(); virtual int __stdcall v19();
    virtual int __stdcall op20(int kind, int arg);     /* slot 20 -> vtable+0x50 */
};

/* The surface the graphics object keeps at +0x34, on the same kind of evidence:
   the screenshot path @0x40a7b0 asks slot 17 for a DC and gives it back through
   slot 26, which is where IDirectDrawSurface7 keeps GetDC and ReleaseDC. Only
   those two are named. */
struct IDirectDrawSurface7
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    /* slot 3 -> +0xc: the z-buffer is attached to the back buffer(s). */
    virtual int __stdcall AddAttachedSurface(struct IDirectDrawSurface7 *s);
    virtual int __stdcall v4();
    /* slot 5 -> +0x14: the presenting blit @0x405d80 passes DDBLT_WAIT. */
    virtual int __stdcall Blt(void *destRect, IDirectDrawSurface7 *src,
                              void *srcRect, int flags, void *fx);
    virtual int __stdcall v6();
    virtual int __stdcall v7();
    /* slot 8 -> +0x20: the z-buffer pick re-aims the device after the
       attach. */
    virtual int __stdcall SetRenderTarget(struct IDirectDrawSurface7 *s, int flags);
    virtual int __stdcall v9();  virtual int __stdcall v10();
    /* slot 11 -> +0x2c: the other half of @0x405d80. */
    virtual int __stdcall Flip(IDirectDrawSurface7 *target, int flags);
    /* slot 12 -> +0x30: the back buffer(s) off the primary @0x405740. */
    virtual int __stdcall GetAttachedSurface(void *caps, IDirectDrawSurface7 **out);
    virtual int __stdcall v13(); virtual int __stdcall v14();
    virtual int __stdcall v15(); virtual int __stdcall v16();
    virtual int __stdcall GetDC(void **hdc);        /* slot 17 -> +0x44 */
    virtual int __stdcall v18(); virtual int __stdcall v19();
    virtual int __stdcall v20(); virtual int __stdcall v21();
    /* slot 22 -> +0x58 */
    virtual int __stdcall GetSurfaceDesc(struct DDSurfaceDesc2 *desc);
    virtual int __stdcall v23();
    virtual int __stdcall IsLost();                 /* slot 24 -> +0x60 */
    /* slot 25 -> +0x64: lockBack @0x406c10 passes a 0x7c-byte descriptor and
       DDLOCK_WAIT, which is Lock's shape and no other slot's. */
    virtual int __stdcall Lock(void *rect, struct DDSurfaceDesc2 *desc,
                               int flags, void *event);
    virtual int __stdcall ReleaseDC(void *hdc);     /* slot 26 -> +0x68 */
    virtual int __stdcall Restore();                /* slot 27 -> +0x6c */
    /* slot 28 -> +0x70: the windowed setup @0x405900 attaches the clipper it
       built for the game window. */
    virtual int __stdcall SetClipper(struct IDirectDrawClipper *clip);
    /* slot 29 -> +0x74: loadTexture sets it with DDCKEY_SRCBLT and a zero key. */
    virtual int __stdcall SetColorKey(int flags, void *key);
    virtual int __stdcall v30(); virtual int __stdcall v31();
    virtual int __stdcall Unlock(void *rect);      /* slot 32 -> +0x80 */
};

/* DDBLTFX - 100 bytes, of which the colour-fill blit @0x40a6a0 touches only the
   size it must declare and the fill colour at +0x50. */
struct DDBltFx
{
    unsigned int dwSize;          /* 0x00 */
    /* int, not char: VC7.1's /GS treats a char array inside a struct-typed
       LOCAL as a string buffer and adds a cookie the original does not have. */
    int          _pad4[(0x50 - 4) / 4];
    unsigned int dwFillColor;     /* 0x50 */
    int          _pad54[(0x64 - 0x50 - 4) / 4];
};

/* DDSURFACEDESC2 - only the four fields loadTexture @0x408d00 touches, and its
   size (0x7c), which the caller must fill in before asking for it. */
struct DDSurfaceDesc2
{
    unsigned int dwSize;        /* 0x00 */
    unsigned int dwFlags;
    unsigned int dwHeight;      /* 0x08 */
    unsigned int dwWidth;       /* 0x0c */
    int          lPitch;        /* 0x10 - bytes per scanline */
    /* int, not char, for the same reason DDBltFx's padding is: a char array in
       a struct-typed LOCAL is a /GS buffer and gets a cookie the original has
       no room for. */
    int          _pad14[(0x24 - 0x14) / 4];
    void        *lpSurface;     /* 0x24 - only valid between Lock and Unlock */
    int          _pad28[(0x48 - 0x28) / 4];
    unsigned int ddpfSize;      /* 0x48 - ddpfPixelFormat.dwSize */
    int          _pad4c[(0x54 - 0x4c) / 4];
    unsigned int dwRGBBitCount; /* 0x54 - inside ddpfPixelFormat */
    int          _pad58[(0x68 - 0x58) / 4];
    unsigned int dwCaps;        /* 0x68 - ddsCaps.dwCaps */
    unsigned int dwCaps2;       /* 0x6c - ddsCaps.dwCaps2 */
    int          _pad70[(0x7c - 0x70) / 4];
};

struct DDColorKey
{
    int dwColorSpaceLowValue;
    int dwColorSpaceHighValue;
};

/* What the graphics object keeps at +0x28 is IDirectDraw7, on the same evidence
   as the device below: slot 9 takes four arguments and countSurfaces @0x408af0
   calls it with 0x11 and a per-surface callback (EnumSurfaces, and 0x11 is
   DDENUMSURFACES_ALL | DDENUMSURFACES_DOESEXIST exactly), while slot 25 takes
   none and is called on the way out of m408c80 @0x408c80 (RestoreAllSurfaces).
   Only the used slots are named. */
/* Only what the windowed setup @0x405900 uses: the window attach and the
   release that follows it. */
struct IDirectDrawClipper
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetClipList(void *rect, void *list, int *size);
    virtual int __stdcall GetHWnd(void **out);
    virtual int __stdcall Initialize(void *dd, int flags);
    virtual int __stdcall IsClipListChanged(int *out);
    virtual int __stdcall SetClipList(void *list, int flags);
    /* slot 8 -> +0x20 */
    virtual int __stdcall SetHWnd(int flags, void *hwnd);
};

struct IDirectDraw7
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall Compact();
    virtual int __stdcall CreateClipper(int flags, struct IDirectDrawClipper **out, void *unk);
    virtual int __stdcall CreatePalette(int caps, void *entries, void **out, void *unk);
    virtual int __stdcall CreateSurface(void *desc, struct IDirectDrawSurface7 **out, void *unk);
    virtual int __stdcall DuplicateSurface(void *src, void **out);
    virtual int __stdcall EnumDisplayModes(int flags, void *desc, void *ctx, void *cb);
    /* slot 9 -> vtable+0x24 */
    virtual int __stdcall EnumSurfaces(int flags, void *desc, int *ctx,
                                       int (__stdcall *cb)(int, int, int *));
    virtual int __stdcall v10();
    /* slot 11 -> +0x2c: the enumeration callback @0x405310 fills both a
       driver and a HEL DDCAPS through it. */
    virtual int __stdcall GetCaps(void *caps, void *helCaps);
    /* slot 12 -> +0x30: m405680 refuses a palettized desktop through it. */
    virtual int __stdcall GetDisplayMode(struct DDSurfaceDesc2 *desc);
    virtual int __stdcall v13();
    virtual int __stdcall v14(); virtual int __stdcall v15();
    virtual int __stdcall v16(); virtual int __stdcall v17();
    virtual int __stdcall v18(); virtual int __stdcall v19();
    /* slot 20 -> +0x50: m405680 sets NORMAL or EXCLUSIVE|FULLSCREEN(|
       ALLOWREBOOT), plus MULTITHREADED unless the 8-bit of its flags. */
    virtual int __stdcall SetCooperativeLevel(void *hwnd, int level);
    /* slot 21 -> +0x54: the full-screen primary setup @0x405740. */
    virtual int __stdcall SetDisplayMode(int w, int h, int bpp, int refresh, int flags);
    virtual int __stdcall v22();
    /* slot 23 -> +0x5c: createOffscreenSurface asks it with DDSCAPS_TEXTURE. */
    virtual int __stdcall GetAvailableVidMem(void *caps, unsigned int *total, unsigned int *avail);
    virtual int __stdcall v24();
    virtual int __stdcall RestoreAllSurfaces();      /* slot 25 -> +0x64 */
};

/* The render device the graphics singleton holds at +0x30. The slots the game
   actually uses line up exactly with IDirect3DDevice7: slot 5/6 are called with
   no arguments as a frame is opened and closed (beginFrame @0x40a770 /
   endFrame @0x405e30 = BeginScene/EndScene), and slot 20 takes a (state, value)
   pair which setBlend @0x40a490 calls with 0x13 and 0x14 - D3D's SRCBLEND and
   DESTBLEND. That is the evidence CLAUDE.md cites for a D3D7 immediate-mode
   render path, now with the vtable INDICES agreeing too, not just the enums.
   Only the used slots are named. */
/* The eight dwords of DDPIXELFORMAT, modelled here rather than pulled from
   ddraw.h - which collides with this header, the same reason enumerateDevices
   declares its own DirectDrawEnumerateExA. */
struct DdPixelFormat
{
    unsigned int size;        /* 0x00 */
    unsigned int flags;       /* 0x04 - bit 0 is DDPF_ALPHAPIXELS */
    unsigned int fourCC;      /* 0x08 */
    unsigned int bitCount;    /* 0x0c */
    unsigned int rMask;       /* 0x10 */
    unsigned int gMask;       /* 0x14 */
    unsigned int bMask;       /* 0x18 */
    unsigned int aMask;       /* 0x1c */
};

/* The factory KGraphics::m405a90 @0x405a90 queries out of IDirectDraw7 and
   then asks for a device. Only slot 4 is exercised. */
/* The D3DVERTEXBUFFERDESC createFlag fills: size, caps, FVF, vertex count. */
struct D3DVbDesc
{
    unsigned int size;
    unsigned int caps;
    unsigned int fvf;
    unsigned int numVertices;
};

/* IDirect3DVertexBuffer7: only Lock / Unlock (slots 3 and 4) are used. */
struct IDirect3DVertexBuffer7
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall Lock(unsigned int flags, void **data, unsigned int *size);   /* slot 3 -> +0xc */
    virtual int __stdcall Unlock();                                                    /* slot 4 -> +0x10 */
};

struct IDirect3D7
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    /* slot 3 -> +0xc: the device probe @0x405310 walks the devices with it. */
    virtual int __stdcall EnumDevices(void *cb, void *ctx);
    /* slot 4 -> +0x10: CreateDevice(guid, renderTarget, out). */
    virtual int __stdcall CreateDevice(void *guid, struct IDirectDrawSurface7 *rt,
                                       struct ID3DDevice7 **out);
    /* slot 5 -> +0x14: CreateVertexBuffer(desc, out, flags). */
    virtual int __stdcall CreateVertexBuffer(D3DVbDesc *desc, IDirect3DVertexBuffer7 **out, unsigned int flags);
    /* slot 6 -> +0x18: the z-buffer pick @0x405b60 walks the formats. */
    virtual int __stdcall EnumZBufferFormats(void *guid, void *cb, void *ctx);
};

struct ID3DDevice7
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetCaps(void *caps);
    virtual int __stdcall EnumTextureFormats(void *cb, void *ctx);
    virtual int __stdcall BeginScene();                 /* slot 5  -> +0x14 */
    virtual int __stdcall EndScene();                   /* slot 6  -> +0x18 */
    virtual int __stdcall v7();
    /* slot 8 -> +0x20: the z-buffer pick re-aims the device after the
       attach. */
    virtual int __stdcall SetRenderTarget(struct IDirectDrawSurface7 *s, int flags);
    virtual int __stdcall v9();  virtual int __stdcall v10();
    /* slot 11 -> +0x2c: the worm draw @0x409480 hands it D3DTRANSFORMSTATE_VIEW
       (2) and its own 4x4. */
    virtual int __stdcall SetTransform(int state, void *matrix);
    virtual int __stdcall v12();
    /* slot 13 -> +0x34: m405a90 @0x405a90 hands it a six-field viewport. */
    virtual int __stdcall SetViewport(void *vp);
    virtual int __stdcall v14();
    virtual int __stdcall v15();
    /* slot 16 -> +0x40: setMaterial @0x4069d0 hands it a 0x44-byte
       D3DMATERIAL7 whose diffuse and ambient it has just filled. */
    virtual int __stdcall SetMaterial(void *material);
    virtual int __stdcall v17(); virtual int __stdcall v18();
    virtual int __stdcall v19();
    virtual int __stdcall SetRenderState(int state, int value); /* slot 20 */
    virtual int __stdcall v21(); virtual int __stdcall v22();
    virtual int __stdcall v23(); virtual int __stdcall v24();
    /* slot 25 -> +0x64: drawRect @0x40a4c0 sends a 4-vertex D3DPT_TRIANGLESTRIP
       with FVF 0x1c4 = XYZRHW|DIFFUSE|SPECULAR|TEX1, whose 0x20-byte stride is
       exactly the one its vertex array uses. */
    virtual int __stdcall DrawPrimitive(int type, int fvf, void *vertices,
                                        int count, int flags);
    /* slot 26 -> +0x68: the worm draw @0x409480 sends its triangle list with
       user-memory vertices and indices. */
    virtual int __stdcall DrawIndexedPrimitive(int type, int fvf, void *vertices,
                                               int vcount, void *indices,
                                               int icount, int flags);
    virtual int __stdcall v27(); virtual int __stdcall v28();
    virtual int __stdcall v29(); virtual int __stdcall v30();
    virtual int __stdcall v31();
    /* slot 32 -> +0x80: m406850 @0x406850 sends a 121-vertex, 600-index
       D3DPT_TRIANGLESTRIP through it - seven arguments, the vertex-buffer
       form of the call above (its second argument is the buffer, not an FVF). */
    virtual int __stdcall DrawIndexedPrimitiveVB(int type, void *vb,
                                                 int startVertex, int vcount,
                                                 void *indices, int icount,
                                                 int flags);
    virtual int __stdcall v33(); virtual int __stdcall v34();
    /* slot 35 -> +0x8c: m408c80 @0x408c80 unbinds all four stages with it. */
    virtual int __stdcall SetTexture(int stage, void *texture);
    virtual int __stdcall v36();
    /* slot 37 -> +0x94: the whole of m408130 @0x408130, whose 190 calls all
       land on D3DTSS_/D3DTOP_/D3DTA_ values that are legal for D3D7. */
    virtual int __stdcall SetTextureStageState(int stage, int type, int value);
};

/* D3DTEXTURESTAGESTATETYPE, D3DTEXTUREOP and the D3DTA_ argument selectors,
   as far as the blend-mode table @0x408130 uses them. */
enum
{
    D3DTSS_COLOROP        = 1,
    D3DTSS_COLORARG1      = 2,
    D3DTSS_COLORARG2      = 3,
    D3DTSS_ALPHAOP        = 4,
    D3DTSS_ALPHAARG1      = 5,
    D3DTSS_ALPHAARG2      = 6,
    D3DTSS_TEXCOORDINDEX  = 11,

    D3DTOP_DISABLE            = 1,
    D3DTOP_SELECTARG1         = 2,
    D3DTOP_SELECTARG2         = 3,
    D3DTOP_MODULATE           = 4,
    D3DTOP_MODULATE2X         = 5,
    D3DTOP_ADD                = 7,
    D3DTOP_ADDSIGNED          = 8,
    D3DTOP_BLENDDIFFUSEALPHA  = 12,
    D3DTOP_BLENDTEXTUREALPHA  = 13,

    D3DTA_DIFFUSE        = 0,
    D3DTA_CURRENT        = 1,
    D3DTA_TEXTURE        = 2,
    D3DTA_TFACTOR        = 3,
    D3DTA_SPECULAR       = 4,
    D3DTA_COMPLEMENT     = 0x10,
    D3DTA_ALPHAREPLICATE = 0x20
};

/* DirectInput, as createInput @0x40c250 uses it: DirectInputCreateA fills the
   root at +0x420100, which then creates a device per GUID_SysMouse and
   GUID_SysKeyboard. Every slot used lands on IDirectInput(Device)A's own
   numbering - CreateDevice at 3 with three arguments, Acquire at 7 with none,
   Unacquire at 8, SetDataFormat at 11 with one, SetCooperativeLevel at 13 with
   two - and the cooperative level is 6, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND. */
struct IDirectInputDeviceA
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();                    /* slot 2 -> +0x08 */
    virtual int __stdcall GetCapabilities(void *caps);
    virtual int __stdcall EnumObjects(void *cb, void *ctx, int flags);
    virtual int __stdcall GetProperty(void *guid, void *hdr);
    virtual int __stdcall SetProperty(void *guid, void *hdr);
    virtual int __stdcall Acquire();                    /* slot 7 -> +0x1c */
    virtual int __stdcall Unacquire();                  /* slot 8 -> +0x20 */
    virtual int __stdcall GetDeviceState(int size, void *data);
    virtual int __stdcall GetDeviceData(int size, void *data, int *n, int flags);
    virtual int __stdcall SetDataFormat(void *format);   /* slot 11 -> +0x2c */
    virtual int __stdcall SetEventNotification(void *event);
    virtual int __stdcall SetCooperativeLevel(void *hwnd, int flags); /* slot 13 */
};

struct IDirectInputA
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();                    /* slot 2 -> +0x08 */
    /* slot 3 -> +0x0c */
    virtual int __stdcall CreateDevice(void *guid, IDirectInputDeviceA **out,
                                       void *unkOuter);
};

#endif /* ICOM_H */
