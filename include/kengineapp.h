#ifndef KENGINEAPP_H
#define KENGINEAPP_H

#include <windows.h>
#include "kprofile.h"

/* The engine's application base class (vtable @0x487418). Its ctor @0x401000
   and dtor @0x401020 are called from the EZ2AC app class's ctor @0x414ea0 /
   dtor @0x414f90, so EZ2AC derives from this; only the base is modeled here.

   vtable @0x487418 slots:
     0  `scalar deleting destructor'   @0x4011e0  (~KEngineApp @0x401020)
     1  className                      @0x401120  -> "KEngineApp"
     2  v2                             @0x402880  (empty; ICF-folded)
     3  v3                             @0x402880  (empty; ICF-folded)
     4  v4                             @0x401070  (`or eax,-1; ret`)

   The ctor publishes the instance to the singleton pointer @0x530b60 (read by
   the window proc @0x401130 as `g_app->m_wndProcHook`). */

/* The optional per-message hook the window proc @0x401130 calls before its own
   handling; the return value is discarded. */
typedef LRESULT(CALLBACK *KWndProcHook)(HWND, UINT, WPARAM, LPARAM);

struct KEngineApp
{
    KEngineApp();                         /* 0x401000 */
    virtual ~KEngineApp();                /* 0x401020; deleting dtor 0x4011e0 */
    virtual const char *className();      /* 0x401120 */
    virtual void v2();                    /* 0x402880 (folded empty) */
    virtual void v3();                    /* 0x402880 (folded empty) */
    virtual int v4();                     /* 0x401070 */

    HWND m_hWnd;                /* +0x04 */
    HINSTANCE m_hInst;          /* +0x08 */
    KWndProcHook m_wndProcHook; /* +0x0c, zeroed by the ctor */

    void pumpMessages();             /* 0x4011a0 */
    int loadProfiles(char *iniPath); /* 0x401240 */
    int create(HINSTANCE hInst, HINSTANCE prev, char *cmdLine, int show); /* 0x401450 */
};

extern KEngineApp *g_app; /* 0x530b60 */

/* The three KProfiles the app creates (loadProfiles @0x401240) and destroys
   (~KEngineApp @0x401020): the app's own .ini, system\title\common\version.ini
   and system\common\patch.ini. */
extern KProfile *g_iniProfile;     /* 0x530b64 */
extern KProfile *g_versionProfile; /* 0x530b6c */
extern KProfile *g_patchProfile;   /* 0x530b68 */

#endif /* KENGINEAPP_H */
