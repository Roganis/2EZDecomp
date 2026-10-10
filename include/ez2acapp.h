#ifndef EZ2ACAPP_H
#define EZ2ACAPP_H

#include "kengineapp.h"
#include "kbase.h"
#include "obj402860.h"
#include "icom.h"

/* The game's own application class: KEngineApp's only derived class (its ctor
   @0x414ea0 calls the base ctor @0x401000, its dtor @0x414f50 the base dtor
   @0x401020), and the object runApp @0x4015e0 drives through the vtable.

   vtable @0x48cf34 overrides every slot of the base:
     0  `scalar deleting destructor'   @0x415a20  (~EZ2AC @0x414f50)
     1  className                      @0x414fb0  -> "EZ2AC"
     2  v2  (init)                     @0x415cb0
     3  v3  (shutdown)                 @0x414ff0
     4  v4  (main loop)                @0x4181b0
   Slots 2-4 are STUBs in src/stubs.cpp for now. */

/* One of the engine subsystems the app shuts down: only vtable slot 3 (its
   shutdown) is exercised, so model just enough slots to place it. The four
   globals v3 tears down may not all be this class — a `call [vptr+0xc]` has no
   target to symbolize, so the slot shape is all that matters. */
struct __declspec(novtable) EzSubsystem : KBase
{
    /* Slots 0 and 1 are KBase's: the runtime-class getter and the virtual
       destructor. Every subsystem's vtable has its own pair there. */
    EzSubsystem();             /* 0x40e670 (src/ksoundds.cpp) */
    virtual int  init();       /* slot 2 */
    virtual void shutdown();   /* slot 3 */
    /* slot 4: hand the resolved name and the opened stream to the backend and
       get the sample back (loadSample @0x40e590). */
    /* Load a sound target from an opened stream. Obj40EAC0's override
       @0x40eda0 says what the three arguments are; the base's own slot 4 is
       not reconstructed. */
    virtual int s4(struct Obj40D230 *t, unsigned int bytes,
                   struct SoundFeed *in);
    /* slot 5 -> +0x14. Obj40EAC0 overrides it as `void s5()`, but m40e520
       @0x40e520 calls it with the owner and uses the result, so the real
       signature is `EzSample *(EzSample *)`. Left as-is here and reached
       through a mirror struct in src/subsystemload.cpp rather than retyping
       the override - the slot INDEX is all a call needs. */
    virtual struct EzSample *s5(struct EzSample *src);
    /* slot 6 -> +0x18: make a sample from scratch (KSoundDS::CreateSample,
       src/ksoundds.cpp). */
    virtual struct EzSample *s6(int bytes, void *wfx, int flags);
    virtual int  s7(void *sample);  /* slot 7 -> +0x1c, freeSample's own hook */

    void *slots[16];           /* 0x04..0x44 - what bind fills */
    int   count;               /* 0x44 */
    void *f48;                 /* 0x48 */
    /* Resolve the handle once: hand back +0x48 if it is already set, else
       derive the dot-name and look it up. */
    void *m40e200(char *name);      /* 0x40e200 */
    void *m40e1c0(char *name);      /* 0x40e1c0 (STUB) */
    int   f4c;                 /* 0x4c */
    /* Which of two backends is in use; every dispatch below switches on it, and
       backend 1's work goes to the sub-object that follows. */
    int   kind;                /* 0x50 */

    /* The backend at +0x54, and the INLINE destructor that owns it.

       Both details are load-bearing. `backend` really is EzSubsystem's member,
       not the derived class's - and because this destructor is inline it gets
       INLINED into every derived destructor, so the `lea ecx,[this+0x54];
       call ~SubsystemBackend` there is a BASE region, not a member one. That
       is what numbers the EH states 0 -> 1 -> -1 (ascending) instead of cl's
       usual descending form. ~Obj40EAC0 @0x40eae0 is the binary's only
       ascending destructor and this is why. */
    ~EzSubsystem() {}

    struct SubsystemBackend
    {
        /* The constructor @0x40da50 writes the whole layout down: a 0xa000-byte
           scratch buffer, two counters, and a KLock whose inlined ctor runs
           first (it is the member, so it is constructed before the body). */
        SubsystemBackend();                /* 0x40da50 */
        ~SubsystemBackend();               /* 0x40daa0 (STUB) */
        /* Release every claimed slot in the table (src/subsystembase.cpp). */
        void m40dd10();                    /* 0x40dd10 */
        /* Tear one slot's object down. Reconstructed in src/accessors.cpp
           and it never touches `this` - but it IS a member: the call site
           in m40e070 @0x40e070 sets ecx, which a free __stdcall would not
           need. With one stack argument the two conventions give identical
           BODIES, so only the caller tells them apart. */
        int release(struct Rec40DCA0 *r);  /* 0x40dca0 */

        /* Where in the table `sample` sits, or -1/-1 and 0 if nowhere
           (src/subsystembase.cpp). */
        int  findSlotOf(void *sample, int *entryOut, int *slotOut); /* 0x40e000 */
        /* Wind the sample's record down where it stands. */
        int  m40e070(void *sample);        /* 0x40e070 */
        /* Forget the sample's record outright, decrementing both counts. */
        int  m40e0f0(void *sample);        /* 0x40e0f0 */
        /* Two guarded scans over the 0x800-entry table that starts at +4 -
           see src/subsystembase.cpp. Both take the cursor by pointer, advance
           it past the slot they return, and hand back 0 when nothing is left. */
        void *nextUsed(int *cur);          /* 0x40df20 */
        void *nextUsedIn(int idx, int *cur); /* 0x40df90 */
        /* Hand out the owner's SECOND slot if it holds an object nobody has
           taken, marking it busy (src/subsystembase.cpp). */
        struct EzSample *m40db00(struct EzSample *owner);  /* 0x40db00 */
        /* Claim the best free slot: with bit 0 of `flags` the first gated
           one, otherwise the lowest +0x10 at or above `floor` whose kind
           matches (src/subsystembase.cpp). */
        struct EzSample *m40dbc0(unsigned int floor, short kind, int arg,
                                 int flags);                  /* 0x40dbc0 */
        /* The worker the table lookup below hands its hit to. */
        int  m40ddd0(int entry, int a);     /* 0x40ddd0 (STUB) */
        /* Find `owner`'s entry under the lock and hand it to the worker;
           0 when it is not in the table (src/subsystembase.cpp). */
        int  m40de30(void *owner, int a);   /* 0x40de30 */
        /* Park `p` in the first FREE entry under the lock, bumping that
           entry's own counter and the live count (src/subsystembase.cpp). */
        int  m40dea0(void *p);              /* 0x40dea0 */

        int         buf[0x2800];           /* 0x0000 .. 0xa000 */
        int         fa000;                 /* 0xa000 */
        int         fa004;                 /* 0xa004 */
        /* A KLock. Held as raw storage so this header need not pull in
           include/ksongplayer.h; the constructor writes its inlined body out
           by hand, which is also the order the original has (the member's
           ctor runs before the enclosing body). */
        void       *lockHandle;            /* 0xa008 */
    } backend;                 /* 0x54 */

    /* Load one sample for `owner` through whichever backend `kind` selects,
       then copy the owner's name into it (src/subsystemload.cpp). */
    struct EzSample *m40e520(struct EzSample *owner);   /* 0x40e520 */
    void  m40e770();           /* 0x40e770 */
    int   freeSample(void *sample);               /* 0x40e620 */
    /* 0x40e590 - decoded but parked, see wip/loadsample.cpp. */
    void *loadSample(struct EzSample *dst, char *name);

    int   bind(void *p);       /* 0x40e180, on the 0x1696430 singleton */
    /* Hand every live sample to slot 7 and then wipe the backend's table
       (src/subsystemreset.cpp). */
    void  m40e6d0();           /* 0x40e6d0 */
    int   createSample(char *name, int a, int b); /* 0x40e3c0 (src/createsample.cpp) */
    /* Resolve `name` (it may REWRITE the caller's pointer - it takes its
       address) and open the stream. 0x40e2a0 (STUB). */
    int   openSample(char *name, int a, char **out, int b, struct EzStream **in);
};

/* The graphics subsystem singleton - the object at 0x1b2e320 that v2 publishes
   into g_sub1b2f054. Named from its own error strings ("KGraphics::
   CreateTextureSurface() Failed" @0x487cc8, pushed @0x40769a, and two siblings),
   which sit in the same region as KTextureContainer. Only what the texture
   report @0x455b70 evidences is modeled. The `this+0xd4` setter modeled as
   Obj407350 (src/misc.cpp) is a method of THIS class; merging the two models is
   a later cleanup. */
/* One loaded texture: the 0xa0-byte payload loadTexture @0x408d00 carves out of
   the graphics profile, keyed by file name. */
/* A loaded sample: only the name it remembers is modeled. */
struct EzSample
{
    char _pad0[0x26];
    char name[0x80];   /* 0x26 - strncpy'd 0x7f then terminated at 0xa5 */
};

/* The opened stream openSample hands back: the release on slot 1 and
   the open itself on slot 3. */
struct EzStream
{
    virtual void v0();
    virtual void release(int flag);   /* slot 1 -> +0x04 */
    virtual void v2();
    virtual int  open(char *name, char **out, int b);   /* slot 3 -> +0x0c */
};

/* The 0x7c-byte block createOffscreenSurface fills - a DDSURFACEDESC2. int,
   not char: a char array here is a /GS buffer and the frame carries no cookie. */
struct DdSurfaceDesc2 { int raw[0x7c / 4]; };
/* What the graphics init hands createPrimary: only the four fields it reads
   (width, height, refresh) and writes (bpp, forced to 32) are named. */
struct KDisplaySettings
{
    char _pad0[8];
    int  height;     /* 0x08 */
    int  width;      /* 0x0c */
    char _pad10[0x18 - 0x10];
    int  refresh;    /* 0x18 */
    char _pad1c[0x54 - 0x1c];
    int  bpp;        /* 0x54 */
};

struct KTexture
{
    IDirectDrawSurface7 *surface; /* 0x00 */
    DDSurfaceDesc2 desc;          /* 0x04 - the video player locks the surface through it */
    /* The source rectangle, in pixels and again normalized to the surface -
       loadTXG @0x408f70 carves a sub-sprite out of one atlas by writing exactly
       these six and the surface pointer, leaving everything else zero. */
    float w;                      /* 0x80 */
    float h;                      /* 0x84 */
    int  f88;                     /* 0x88 - cleared once loaded */
    float u0;                     /* 0x8c */
    float v0;                     /* 0x90 */
    float u1;                     /* 0x94 */
    float v1;                     /* 0x98 */
    int  bytes;                   /* 0x9c - width * height * bytes per pixel */
};

/* A node of the texture cache: KProfile's key tree with a whole KTexture as
   the payload the generic node keeps at +0x50 (KProfile::create takes the node
   size, so a node is exactly as big as what it has to hold). The offsets
   freeNodes408b40 @0x408b40 touches - 0xd8 and 0xec - are the texture's f88
   and bytes seen through the node. */
struct KTextureNode
{
    KTextureNode *left;   /* 0x00 */
    KTextureNode *right;  /* 0x04 */
    char          key[0x40];  /* 0x08 */
    int           type;   /* 0x48 */
    int           f4c;    /* 0x4c */
    KTexture      tex;    /* 0x50 */
};

/* The 0x4bc-byte "Flag" block the background classes embed - a waving-banner
   mesh, named by the line its release logs ("Destroying Flag at %x" @0x487bf8).
   KGraphics::destroyFlag @0x406820 Releases the COM object at +8 and then
   zeroes all 0x4bc bytes (`mov ecx,0x12f; rep stosd`), which is what fixes the
   size. */
/* The waving-banner mesh: an 11x11 grid of XYZ|NORMAL|TEX1 vertices in a
   D3D vertex buffer and 600 indices (100 cells x 2 triangles), built by
   KGraphics::createFlag @0x406620 with the two wave parameters. */
struct FlagVertex
{
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

struct GfxFlag
{
    float a;                                /* 0x00 - createFlag's second argument */
    float b;                                /* 0x04 - and its third */
    struct IDirect3DVertexBuffer7 *vb;      /* 0x08 */
    unsigned short indices[600];            /* 0x0c .. 0x4bc */
};

/* Only the extent the two measurements @0x408110/0x408120 read. */
struct Sprite
{
    char  _pad0[0x80];
    float width;    /* 0x80 */
    float height;   /* 0x84 */
};

/* The descriptor a video layer carries: position, the two stages' texture
   windows, the four corner coordinates, the frame cursor, the rotation,
   the four color channels, and the blend/stage modes. */
struct VideoBlit
{
    float x;          /* 0x00 */
    float y;          /* 0x04 */
    float u0;         /* 0x08 */
    float v0;         /* 0x0c */
    float uw0;        /* 0x10 */
    float vh0;        /* 0x14 */
    float u1;         /* 0x18 */
    float v1;         /* 0x1c */
    float uw1;        /* 0x20 */
    float vh1;        /* 0x24 */
    float xa;         /* 0x28 */
    float xb;         /* 0x2c */
    float xc;         /* 0x30 */
    float xd;         /* 0x34 */
    float ya;         /* 0x38 */
    float yb;         /* 0x3c */
    float yc;         /* 0x40 */
    float yd;         /* 0x44 */
    float frame;      /* 0x48 */
    /* How the cell table's refresh @0x403940 cycles `frame` per tick: 1 add,
       2 add and clamp to count-1, 3 wrap, 4 subtract and wrap, 5 ping-pong
       over count-1; anything else leaves it. */
    int   frameMode;  /* 0x4c */
    float frameSpeed; /* 0x50 */
    float angle;      /* 0x54 */
    float cr;         /* 0x58 */
    float cg;         /* 0x5c */
    float cb;         /* 0x60 */
    float ca;         /* 0x64 */
    int   blendSrc;   /* 0x68 */
    int   blendDst;   /* 0x6c */
    int   stageMode;  /* 0x70 */
};

/* A Win32 RECT, spelled out because not every TU that sees KGraphics includes
   windows.h. */
struct GRect
{
    int left, top, right, bottom;
};

/* What KGraphics::drawMesh @0x406850 is handed: a vertex format and the index
   block that follows it. */
struct MeshDraw
{
    char _pad0[8];
    void *vb;        /* 0x08 - the IDirect3DVertexBuffer7 the strip is drawn from */
    char indices[4]; /* 0x0c - the index block starts here */
};

/* Three floats, the argument shape setCamera @0x40bd40 reads: it loads +0x00,
   +0x04 and +0x08 of each of its three pointers and subtracts them pairwise to
   build the view basis. */
struct GVec3
{
    float x, y, z;
};

struct KGraphics
{

    char _pad0[0x04];
    /* The game window. The windowed setup @0x405900 measures its client area
       for the destination rect and hands it to the clipper. */
    void *m_hwnd;                 /* 0x04 */
    /* The two present-mode flags @0x405d80 reads: f8 picks flipping over
       blitting, and fc picks which flip flags. */
    int  f8;                      /* 0x08 */
    int  fc;                      /* 0x0c */
    /* The screen size the colour-fill helper defaults a zero width/height to. */
    int  m_width;                 /* 0x10 */
    int  m_height;                /* 0x14 */
    GRect m_destRect;             /* 0x18 - where the back buffer lands */
    IDirectDraw7 *ddraw;          /* 0x28 - what countSurfaces enumerates */
    IDirect3D7 *d3d;              /* 0x2c - queried out of ddraw */
    ID3DDevice7 *device;          /* 0x30 - the render device */
    IDirectDrawSurface7 *surface; /* 0x34 - what the screenshot path grabs */
    IDirectDrawSurface7 *m_back;  /* 0x38 - the source of the presenting blit */
    IDirectDrawSurface7 *m_backLeft; /* 0x3c - the stereo left back buffer, when fc asks */
    /* Query the Direct3D interface out of ddraw, create the device on the back
       buffer, and give it a full-screen viewport. */
    int  m405a90(void *deviceGuid);   /* 0x405a90 */
    IDirectDrawSurface7 *m_zbuf;  /* 0x40 - the z-buffer m405b60 creates */
    /* 0x44 - hardware (0x4000) or software (0x800) vertex processing, from
       which device GUID m406050 was handed. */
    int  m_vbFlags;
    /* What init @0x406170 leaves behind: the device the probe picked, (NOT
       the .ini's FullScreen answer - see m_fullscreen), its own copy of the
       back buffer, and the
       back buffer's DDSURFACEDESC2 (0x54..0xd0 - m_f60/m_f64 below are that
       desc's dwWidth and lPitch, named separately because drawText reads
       them on their own). */
    struct D3DDeviceEntry *m_dev; /* 0x48 */
    /* 0x4c - NEVER WRITTEN. No store to +0x4c of this object exists in
       .text (the ctor @0x405f80 leaves it alone, and neither the pool nor
       the CRT allocator zeroes), and its one read is v2 @0x406170's flags
       term `(m_fullscreen != 0 ? 4 : 0) | 4` - the original ORs the
       constant 4 in unconditionally at 0x406225, so the term is dead in the
       original bytes too. The .ini FullScreen answer goes to a LOCAL there
       and to flags bit 0; it never lands here. The name is a guess at the
       author's intent, not a description of what the field holds. */
    int  m_fullscreen;            /* 0x4c */
    IDirectDrawSurface7 *m_backRef; /* 0x50 */
    unsigned int m_descSize;      /* 0x54 - the desc's dwSize, written 0x7c */
    char _pad58[0x60 - 0x58];
    /* The primary's cached width and pitch-in-bytes: their quotient is the
       screen's bytes per pixel, which is how drawText picks its 16- or
       32-bit glyph family. */
    unsigned int m_f60;           /* 0x60 */
    unsigned int m_f64;           /* 0x64 */
    char _pad68[0xd0 - 0x68];
    /* The acceptance filter init hands to enumerateDevices (it lands in
       g_enumContext and vetoes devices from the enumeration callback).
       Nothing in the tree installs one. */
    int (__cdecl *m_acceptFilter)(void *caps, void *desc7);  /* 0xd0 */
    /* Full-screen setup @0x405740: set the display mode from the settings,
       create the flipping primary and take its back buffer(s). */
    int  createPrimary(struct KDisplaySettings *p);
    /* Windowed setup @0x405900: measure the window, create a plain primary
       and clip it to the window, then a 3D render target of that size. */
    int  createWindowed();                          /* 0x405900 */
    /* Bring the surfaces up for init @0x406170: the window, the adapter and
       device GUIDs, the 640x480x32 mode the probe found, and the flag word
       init assembled. */
    int  m406050(void *hwnd, void *adapterGuid, void *deviceGuid,
                 struct DDSurfaceDesc2 *mode, int flags);  /* 0x406050 (src/kgraphics.cpp) */
    /* The GDI text font build ("Arial", 80 by 400) - compiled out to a bare
       `ret 0xc` in the original. */
    void m406bf0(char *face, int h, int weight);    /* 0x406bf0 */
    /* m406050's first step: DirectDraw on the given adapter and the
       cooperative level (src/kgraphics.cpp). */
    int  m405680(void *adapterGuid, int flags);     /* 0x405680 */
    /* The z-buffer, picked by depth from the device's own format walk and
       attached to the back buffer(s) (src/kgraphics.cpp). */
    int  m405b60(void *deviceGuid);                 /* 0x405b60 */
    int  fd4;                     /* 0xd4 - loadTexture consults it */
    /* The two texture formats the device enumeration picks - one without an
       alpha channel and one with, both 32-bit. m407360 fills them. */
    struct DdPixelFormat fmtOpaque;   /* 0x0d8 */
    struct DdPixelFormat fmtAlpha;    /* 0x0f8 */
    int  m_textureCaps;               /* 0x118 - the device's dwTextureCaps */
    /* A second KProfile, emptied the same way as the one at +0x128 but through
       its own node-releasing method. */
    KProfile *m_profile11c;   /* 0x11c */
    /* Enumerate the device's texture formats into the two slots above, then
       read and log its caps. */
    void m407360();               /* 0x407360 */
    int  m_texBytes;          /* 0x120: byte total */
    int  m_texCount;          /* 0x124: texture count */
    /* The static KProfile @0xd30c48 that setF128 @0x403ef0 points here and
       m403f60 @0x403f60 empties. */
    KProfile *m_profile;      /* 0x128 */
    int  m_textColor;             /* 0x12c - what drawText paints with */
    char _pad130[0x134 - 0x130];
    /* Where the back buffer lands while it is locked, and its pitch in
       16-BIT PIXELS - the surface is RGB565, and the putpixel @0x406c80 packs
       into it with (r >> 8) & 0xf800, (g >> 5) & 0x7e0. */
    unsigned short *m_lockBits;   /* 0x134 */
    int  m_lockPitch;             /* 0x138 */
    char _pad13c[0x1c0 - 0x13c];
    int  m_frames;            /* 0x1c0: frames since the timer was reset */
    unsigned int m_frameBase; /* 0x1c4: timeGetTime() at that reset */
    int  m_fps;               /* 0x1c8: frames counted in the last second */
    int  m_shotCount;         /* 0x1cc: how many screenshots have been taken */
    /* The .str clip cache - a KProfile keyed by resource name, whose nodes
       carry the parsed cell table and the raw file in one allocation
       (KGraphics::loadStr @0x402de0). */
    struct KProfile *m_clips; /* 0x1d0 */

    int countSurfaces();      /* 0x408af0 */
    void resetFrameTimer();   /* 0x40a750 */
    void beginFrame();        /* 0x40a770 */
    void m405e10();           /* 0x405e10 */
    void screenShot();        /* 0x40a7b0 */
    void endFrame();          /* 0x405e30 */
    int  m405d80();           /* 0x405d80 - present; what endFrame tail-jumps */
    void m403f60();           /* 0x403f60 */
    void setF128();           /* 0x403ef0 */
    void m408c80();           /* 0x408c80 */
    IDirectDrawSurface7 *createSystemTextureSurface(DdSurfaceDesc2 *desc, int w, int h, int flag); /* 0x407750 */
    int  findOrCreateTexture(char *name, struct KTexture **out, char *key, int flag); /* 0x409200 (src/loadtexture.cpp) */
    int  loadTexture(char *name, struct KTexture **out, char *key); /* 0x408d00
                                        (STUB; wip/loadtexture.cpp) */
    /* Read a .txg atlas description: one texture file plus a list of named
       sub-rectangles, each registered in the texture profile as a KTextureNode
       sharing the atlas surface. (wip/loadtxg.cpp - 78.71%.) */
    int  loadTXG(char *path);                        /* 0x408f70 */
    /* Load a .str resource and hand back the cell table it describes; the
       result is what Obj402860::attach @0x4028b0 is given. Reads its own
       KResourceContainer at +0x1d0 and builds a KFile on the stack. */
    int  loadStr(char *name, struct Obj402860Head **out);  /* 0x402de0 */
    int  m401b80(char *clip, struct Obj402860Head **out, char *dir); /* 0x401b80 (STUB) */
    void setBlend(int src, int dst);                 /* 0x40a490 */
    /* An off-screen surface of w x h, described into `desc` (a 0x7c-byte
       DDSURFACEDESC2 the caller provides), on the primary's pixel format
       (or the video one's when `flag`). Logs the DDraw error and returns 0
       on failure. */
    IDirectDrawSurface7 *createOffscreenSurface(struct DdSurfaceDesc2 *desc, int w, int h, int flag); /* 0x407450 */
    /* The placeholder texture: a 256x256 off-screen surface with the
       texture's name painted across it. `this` is unused. */
    int  createDummyTexture(const char *name, float w, float h, KTexture *tex); /* 0x407a50 */
    /* Load a texture from a .bmp file into `dst`, decrypting the header when
       fd4 says the assets are scrambled; falls back to the placeholder. */
    int  loadBmpTexture(char *name, KTexture *dst);  /* 0x407c60 */
    int  reloadBmpTexture(char *name, KTexture *tex); /* 0x407e50 - onto the texture's own surface */
    int  drawDummyTextureLabel(const char *name, KTexture *tex); /* 0x407b60 */
    /* An untextured line/quad: the x pair, the y pair (both mirrored about
       480 when the screen is flipped), a width whose 1.0 case is special, and
       two colours. */
    void drawLine(float x0, float y0, float x1, float y1, float w,
                  int c0, int c1);                   /* 0x40ae90 (STUB) */
    float widthOf(Sprite *s);                        /* 0x408110 */
    float heightOf(Sprite *s);                       /* 0x408120 */
    void drawTex(KTexture *tex, float x, float y, float w, float h, int color); /* 0x403f80 */
    void m40a5a0(float x, float y, float w, float h, unsigned int c1,
                 unsigned int c2);                   /* 0x40a5a0 (STUB) */
    void m40ac80(float x, float y, float w, float h, float e, float f);
                                                     /* 0x40ac80 (src/kgraphicsquad.cpp) */
    /* Draw a vertical span of a sprite: position, the span's rounded size,
       and the top/bottom texture fractions. */
    /* The one-colour twin of m404950: the same quad, the same defaults and
       crop fractions, one colour into every vertex. */
    void m404660(struct KTexture *tex, float x, float y, float w, float h,
                 float u, float v, float uw, float vh,
                 unsigned int color);                /* 0x404660 */
    /* Blit one 16x16 glyph of the big font into the locked surface
       (src/kgraphics2.cpp). */
    void m406d40(int x, int y, int c, int color);    /* 0x406d40 */
    /* Draw a sprite rotated around its position. */
    void m404250(Sprite *s, float x, float y, float w, float h,
                 float angle, unsigned int color);   /* 0x404250 (src/kgraphicsquad.cpp) */
    /* Load `path` into the named sprite slot and hand the sprite back. */
    void loadSprite(const char *name, char *path,
                    struct Sprite **out);            /* 0x408e80 (src/kgraphics2.cpp) */
    /* The video-frame blit: the rotated two-stage quad for the current and
       next frame of a video layer (src/kgraphicsblit.cpp). */
    void m401d80(struct KTexture **frames, int count, struct VideoBlit *d,
                 float xoff, float yoff);            /* 0x401d80 */
    /* The guard in front of it: nothing is drawn for a single-frame layer or
       a stage mode below five. */
    void m402f70(struct KTexture **frames, int count, struct VideoBlit *d,
                 float xoff, float yoff);           /* 0x402f70 (src/kgraphicsblit2.cpp) */
    void m4038f0(struct KTexture **frames, int count, struct VideoBlit *d,
                 float xoff, float yoff);            /* 0x4038f0 */
    void setFade(int level);                         /* 0x40b080 */
    /* Bind (or unbind) one texture stage, skipping the call when the stage
       already holds that surface. */
    void m4080c0(int stage, KTexture *tex);          /* 0x4080c0 */
    /* One of sixteen texture blend-mode recipes. */
    /* A textured quad with a per-edge colour: the position and size (either
       zero taking the texture's own), the sub-rectangle as fractions of the
       texture's crop window, and the bottom/top colours. */
    void m404950(struct KTexture *tex, float x, float y, float w, float h,
                 float u, float v, float uw, float vh,
                 unsigned int cTop, unsigned int cBottom);  /* 0x404950 */
    /* The third of the family: the colour pair is assigned inside the flip
       halves rather than bottom-against-top. */
    void m404c50(struct KTexture *tex, float x, float y, float w, float h,
                 float u, float v, float uw, float vh,
                 unsigned int cB, unsigned int cA);        /* 0x404c50 */
    void m408130(int mode);                          /* 0x408130 */
    /* Absolute edges, not an origin and a size (wip/drawrect.cpp). */
    void drawRect(float left, float top, float right, float bottom,
                  int color);                    /* 0x40a4c0 (STUB) */
    /* The caption bar the operator test pages draw behind their text: an
       origin and a size (either half defaulting to the screen's when passed
       zero) plus a flags word, blitted through the surface at +0x38. */
    void m40a6a0(int x, int y, int w, int h, int color); /* 0x40a6a0 (src/colorfill.cpp) */
    /* Hand a frame to the video-blit path: GetDC + write-only Lock on the
       texture's surface, StretchDIBits the bits in 1:1, ReleaseDC, Unlock.
       The third argument is a BITMAPINFOHEADER passed BY VALUE (0x28 bytes);
       always returns 1. */
    int  m408040(KTexture *tex, void *p, struct GfxBlob28 s);  /* 0x408040 (src/kgraphics2.cpp) */
    /* Publish one packed ARGB as the scene's ambient render state AND as a
       material whose diffuse and ambient are both that colour
       (src/kgraphics2.cpp). */
    void setMaterial(unsigned int color);             /* 0x4069d0 */
    /* Step a mesh's own animation, then draw it. The backgrounds call the
       two back to back on their single mesh. */
    void m40a300(void *mesh);                         /* 0x40a300 */
    void m409480(void *mesh);                         /* 0x409480 */
    /* Build the spline tube the backgrounds fly through (src/kgraphicsworm.cpp):
       `rings` vertices per ring, the eye's speed along the curve, the tube's
       radius, and mode 1 for the helix path. Returns 0 if a buffer failed. */
    void setVec2(struct Vec2 *v, float x, float y);  /* 0x406600 - a member; the
                                                       background pump's call site
                                                       loads the singleton */
    int  createWorm(void **out, int rings, float speed, float radius, int mode); /* 0x409d90 */
    /* One whole frame of the texture-cache tree: open the scene, draw the
       tree depth-first, optionally clear, close the scene. */
    void m408c40(int clear);                          /* 0x408c40 */
    /* Tear down an embedded flag block, and free a mesh's three buffers (it
       nulls +0x00 and `free`s +0x04/+0x08/+0x0c). Both take their operand as a
       stack argument even though they are called with `this` in ecx. */
    void destroyFlag(GfxFlag *f);                     /* 0x406820 */
    void freeBuffers(void *mesh);                     /* 0x409420 */
    /* destroyFlag's counterpart: it zeroes all 0x4bc bytes of the block (the
       same `mov ecx,0x12f; rep stosd` that fixes GfxFlag's size) and then
       builds the banner's vertex buffer through the COM object at +0x2c. The
       two floats are pushed as raw dwords, so they are `float`, not `double`;
       the three background constructors all pass 0.3f and 0.00012f. */
    int  createFlag(GfxFlag *f, float a, float b);    /* 0x406620 */
    /* The view transform: eye, look-at and up, each a GVec3 by pointer. */
    void setCamera(GVec3 *eye, GVec3 *at, GVec3 *up); /* 0x40bd40 (src/setcamera.cpp) */
    /* The projection: near plane, far plane and the field of view in radians
       (SongSelectDirector's destructor restores 1.0f / 10000.0f / PI/4). It
       divides fov's cosine and sine out into the matrix it builds at +0x30. */
    void setProjection(float nearZ, float farZ, float fov); /* 0x40bb20 (src/colorfill.cpp) */
    /* Point the camera down -Z, set the standard projection and draw one
       indexed mesh. */
    void drawMesh(struct MeshDraw *a);               /* 0x406850 */
    void m406900(struct GfxFlag *f);                 /* 0x406900 (src/flagwave.cpp) */
    /* Lock the back buffer and cache where it landed. */
    int  lockBack();                                 /* 0x406c10 */
    /* Draw one 16x16 glyph of the CJK font straight into the locked back
       buffer. Decoded and structurally exact, parked on one register:
       wip/kgraphics-drawglyph.cpp. */
    void drawGlyph(int x, int y, int glyph, unsigned int color); /* 0x406c80 */
    /* Its half-width companions, both 8x16 proportional ASCII glyphs
       (src/kgraphics2.cpp). The plain one packs an 0x00RRGGBB colour down to
       RGB565 and writes one pixel per column; the WIDE one takes a colour
       already packed as a PAIR and writes two at a time. */
    void drawAsciiGlyph(int x, int y, int glyph, unsigned int color);
                                                              /* 0x406dd0 */
    void drawAsciiGlyphWide(int x, int y, int glyph, int color); /* 0x406ea0 */
    /* Lock the back buffer and write a whole string into it, half-width and
       CJK glyphs both. */
    void drawText(int x, int y, char *s);            /* 0x406f50 (src/drawtext.cpp) */
};

struct LoaderInfo;
extern LoaderInfo g_4a72b8;        /* 0x4a72b8 - KEzwLoader's descriptor
                                      (src/loaders.cpp); what v2 registers */

/* The subsystem singletons (defined in src/ez2acapp.cpp). */
extern EzSubsystem *g_sub1b2e70c;
extern EzSubsystem *g_sub1b2eae4;
extern EzSubsystem *g_sub1b2ef98;
extern EzSubsystem *g_sub1b2f054;

struct EZ2AC : KEngineApp
{
    int       m_f10;           /* 0x10: nonzero once the subsystems are up */
    int       m_f14;           /* 0x14 */
    int       m_f18;           /* 0x18: runGame's "still need the first screen" */
    int       m_f1c;           /* 0x1c: nonzero while the game loop should run */
    char      _pad20[0x78 - 0x20];
    int       m_f78;           /* 0x78: re-armed every round */
    char      _pad7c[0x6d830 - 0x7c];
    int       m_f6d830;        /* 0x6d830 */
    char      _pad6d834[0x6d93c - 0x6d834];
    Obj402860 m_sub;           /* 0x6d93c (its dtor is the ICF-folded ret) */

    EZ2AC();                         /* 0x414ea0 */
    void loadCounters();     /* 0x4156c0 */
    void readSettings();             /* 0x415a40 (src/settings.cpp) */
    void readPatchSettings();        /* 0x415900 */
    int  readBookkeeping();          /* 0x415070 */
    int  runTestSong();              /* 0x4163a0 */
    int  runTestGame();              /* 0x4164f0 */
    int  runGame();                  /* 0x417e90 */
    int  m415f00();                  /* 0x415f00 (STUB) */

    /* The screen helpers the mode runners share (src/moderunner.cpp). Each one
       news a screen, drives it through Obj401780::run and hands its code back. */
    int  loadModeGds();              /* 0x4150d0 - every runner starts here */
    int  runEnding();                /* 0x415db0 - every runner ends here */
    int  runResult();                /* 0x415ec0 */
    int  runSongSelect();            /* 0x415f70 */
    int  runRankingScreen();         /* 0x415fe0 */
    int  runModeIntro();             /* 0x416020 */
    int  runCatchGame();             /* 0x416060 */
    int  runGFResult();              /* 0x4162e0 */
    int  runMainGame();              /* 0x416320 */

    /* One runner per game mode, dispatched on g_modeIndex - the index into
       kModeNames (src/modename.cpp), which is what names them. */
    int  run5KeyMix();               /* 0x416b90 */
    int  run7StreetMix();            /* 0x416b90 too - ICF-folded onto it */
    int  runRubyMix();               /* 0x416600 */
    int  runStreetMix();             /* 0x4168c0 */
    int  runGameOver();              /* 0x415d70 - a member; every call site loads ecx */
    void crossFade(char *first, char *second, char *third, int step); /* 0x415200 - a member; all four call sites load ecx */
    int  runClubMix();               /* 0x417670 */
    int  runSpaceMix();              /* 0x417850 (src/runspacemix.cpp) */
    int  run5RadioMix();             /* 0x416fc0 */
    int  runRadioMix();              /* 0x416d60 */
    int  run10RadioMix();            /* 0x417230 */
    int  run14RadioMix();            /* 0x417450 */
    int  runEz2Catch();              /* 0x4160e0 */
    int  runScratchMix();            /* 0x417c70 */
    int  runCv2Mix();                /* 0x417a30 (src/moderunner.cpp) */
    virtual ~EZ2AC();                /* 0x414f50; deleting dtor 0x415a20 */
    virtual const char *className(); /* 0x414fb0 */
    virtual void v2();               /* 0x415cb0 */
    virtual void v3();               /* 0x414ff0 */
    virtual int v4();                /* 0x4181b0 */
};

#endif /* EZ2ACAPP_H */
