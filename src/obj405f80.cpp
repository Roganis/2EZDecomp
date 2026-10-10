/* Obj405F80 - see include/obj405f80.h. Constructor, destructor and vtable. */

#include "kcrt.h"   /* operator delete = free, for the deleting destructor */
#include "obj405f80.h"

extern int g_4a5218;      /* src/globals.cpp */
extern int g_d36cb0;      /* 0xd36cb0 (src/maingame.cpp) */

/* Where the object publishes itself while it exists. */
// GLOBAL: EZ2AC_UNPACKED 0x00d36cac
Obj405F80 *g_d36cac;

// FUNCTION: EZ2AC_UNPACKED 0x00405570
void *Obj405F80::getClassDesc()
{
    return &g_4a5218;
}

// FUNCTION: EZ2AC_UNPACKED 0x00405f80
Obj405F80::Obj405F80()
{
    f04[0] = 0;
    f04[1] = 0;
    f04[2] = 0;
    f04[3] = 0;
    f04[4] = 0;
    f28[3] = 0;
    f28[4] = 0;
    f28[5] = 0;
    f28[6] = 0;
    f28[2] = 0;
    f28[0] = 0;
    f28[1] = 0;
    f28[7] = 0;
    fd0 = 0;
    f1cc = 0;
    fd4 = 0;
    f130 = 0;
    f12c = 0xffffff;
    g_d36cac = this;
    g_d36cb0 = 0;
    f1c8 = 0;
}

/* Zero members with destructors - the EH state around the body is 0, and the
   only call after it is the base's at -1. */
// FUNCTION: EZ2AC_UNPACKED 0x00405ff0
Obj405F80::~Obj405F80()
{
    releaseAll();
    freeTable405530();
}

// FUNCTION: EZ2AC_UNPACKED 0x004065e0
// Obj405F80::`scalar deleting destructor'

// VTABLE: EZ2AC_UNPACKED 0x00487b80
struct Obj405F80;

#include <windows.h>
#include <string.h>
#include "ez2acapp.h"
#include "kengineapp.h"
#include "kprofile.h"
#include "icom.h"

extern int enumerateDevices(void *ctx);              /* 0x4054d0 (src/kgraphics.cpp) */
extern int pickDevice(void **out, int flags);        /* 0x405e50 (src/kgraphics.cpp) */
extern int loadGlyphFont(char *name);                /* 0x407080 (src/kgraphics2.cpp) */
extern int loadAsciiFont(char *name);                /* 0x407120 (src/kgraphics2.cpp) */
struct ObjWithProfile { void setProfile(); };        /* 0x401b70 (src/accessors.cpp) */
extern void getExeDir(char *out);                    /* 0x401080 (src/kengineapp.cpp) */

/* The picked entry, viewed with only what init reads (the full carve lives in
   src/kgraphics.cpp). */
struct PickedDevice
{
    char _pad0[0x28];
    void *guid;               /* 0x028 */
    char _pad2c[0x11c - 0x2c];
    void *adapterGuid;        /* 0x11c */
    char _pad120[0x418 - 0x120];
    struct DDSurfaceDesc2 mode640;   /* 0x418 */
    int  chosen;              /* 0x494 */
    int  f498;                /* 0x498 */
};

/* A D3DMATERIAL7, as the four-colour block init zeroes and half-fills. */
struct GMaterial7
{
    float diffuse[4];         /* 0x00 */
    float ambient[4];         /* 0x10 */
    float rest[9];            /* 0x20 .. 0x44 */
};

/* The graphics bring-up, slot 2 of the vtable: enumerate and pick a device,
   read FullScreen out of the .ini (hiding the cursor when it is on), build
   the surfaces, cache the back buffer and its descriptor, aim the camera at
   -Z and set the 45-degree projection, then the render-state table, a grey
   half-lit material, the two profiles, the texture formats, the Arial GDI
   font and the two bitmap fonts - each font falling back to <appdir>\<name>
   before giving up. Returns 1, or 0 when any step fails - with the device
   table freed, EXCEPT when a bitmap font is missing: those two failures jump
   past the free straight to the `return 0` (0x4064f2 / 0x406552 -> 0x406268). `this` is the graphics singleton, so the body reads it through
   the KGraphics view (the two models are one object). */
// FUNCTION: EZ2AC_UNPACKED 0x00406170
int Obj405F80::v2()
{
    KGraphics *g = (KGraphics *)this;
    int fullscreen;
    GVec3 at;
    GVec3 eye;
    GVec3 up;
    GMaterial7 mat;
    char buf[260];
    int flags;

    if (enumerateDevices(g->m_acceptFilter) < 0)
        return 0;
    if (pickDevice((void **)&g->m_dev, 0) < 0)
        return 0;

    g->m_hwnd = g_app->m_hWnd;
    /* The `| 4` is the original's own (`or edi,4` at 0x406225), so the
       m_fullscreen term before it is dead in the original bytes as well -
       and the field is never written anywhere (include/ez2acapp.h). The
       99.38% residue of this function is elsewhere. */
    flags = (((PickedDevice *)g->m_dev)->f498 != 0 ? 2 : 0) |
            (((PickedDevice *)g->m_dev)->chosen == 0) |
            (g->m_fullscreen != 0 ? 4 : 0) | 4;
    fullscreen = 1;
    /* "FullScreen" carries its STRING marker in src/screenrun.cpp. */
    g_iniProfile->getInt("FullScreen", &fullscreen);
    if (fullscreen != 0)
    {
        ShowCursor(0);
        flags |= 1;
    }

    if (g->m406050(g->m_hwnd, ((PickedDevice *)g->m_dev)->adapterGuid,
                   ((PickedDevice *)g->m_dev)->guid,
                   &((PickedDevice *)g->m_dev)->mode640, flags) < 0)
    {
        freeTable405530();
        return 0;
    }

    g->m_backRef = g->m_back;
    g->m_descSize = 0x7c;
    g->m_back->GetSurfaceDesc((DDSurfaceDesc2 *)&g->m_descSize);

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    g->setCamera(&eye, &at, &up);
    g->setProjection(1.0f, 10000.0f, 0.785398163397448f);

    g->device->SetRenderState(0x8b, -1);
    g->device->SetTextureStageState(0, 0xd, 1);
    g->device->SetTextureStageState(0, 0xe, 1);
    g->device->SetTextureStageState(0, 2, 2);
    g->device->SetTextureStageState(0, 3, 0);
    g->device->SetTextureStageState(0, 1, 4);
    g->device->SetTextureStageState(0, 0x11, 2);
    g->device->SetTextureStageState(0, 0x10, 2);
    g->device->SetRenderState(4, 1);
    g->device->SetRenderState(0x29, 1);
    g->device->SetRenderState(0x1a, 1);
    g->device->SetRenderState(2, 2);
    g->device->SetRenderState(0x1b, 1);
    g->device->SetRenderState(0x1d, 0);
    g->device->SetRenderState(7, 1);
    g->device->SetRenderState(0x89, 0);
    g->device->SetRenderState(9, 2);

    memset(&mat, 0, sizeof(mat));
    mat.diffuse[0] = 0.5f;
    mat.ambient[0] = 0.1f;
    mat.diffuse[1] = 0.5f;
    mat.ambient[1] = 0.1f;
    mat.diffuse[2] = 0.5f;
    mat.ambient[2] = 0.1f;
    mat.diffuse[3] = 0.5f;
    mat.ambient[3] = 0.1f;
    g->device->SetMaterial(&mat);

    g->setF128();
    g->m407360();
    ((ObjWithProfile *)this)->setProfile();
    // STRING: EZ2AC_UNPACKED 0x00487be0
    g->m406bf0("Arial", 0x50, 0x190);

    // STRING: EZ2AC_UNPACKED 0x00487bc4
    if (loadGlyphFont("system\\common\\fontkr.dat") == 0)
    {
        getExeDir(buf);
        // STRING: EZ2AC_UNPACKED 0x00487bb8
        strcat(buf, "\\fontkr.dat");
        if (loadGlyphFont(buf) == 0)
            return 0;
    }
    // STRING: EZ2AC_UNPACKED 0x00487b9c
    if (loadAsciiFont("system\\common\\fontEn.dat") == 0)
    {
        getExeDir(buf);
        // STRING: EZ2AC_UNPACKED 0x00487b90
        strcat(buf, "\\fontEn.dat");
        if (loadAsciiFont(buf) == 0)
            return 0;
    }
    return 1;
}
