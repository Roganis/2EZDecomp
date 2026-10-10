#include <stdio.h>
#include <string.h>
#include "kcrt.h" /* operator delete == an inlined free */
#include "kengineapp.h"
#include "loader.h" /* Log */

/* wip/extractdirname.cpp; stubbed in src/stubs.cpp until it matches. */
extern void extractDirName(char *src, char *out);

void getExeDir(char *out); /* below */

/* KEngineApp — the application base class (see include/kengineapp.h). */

// GLOBAL: EZ2AC_UNPACKED 0x00530b60
KEngineApp *g_app;

// GLOBAL: EZ2AC_UNPACKED 0x00530b64
KProfile *g_iniProfile;
// GLOBAL: EZ2AC_UNPACKED 0x00530b68
KProfile *g_patchProfile;
// GLOBAL: EZ2AC_UNPACKED 0x00530b6c
KProfile *g_versionProfile;

// GLOBAL: EZ2AC_UNPACKED 0x004a50d0
int g_4a50d0;

/* Passed to all three KProfile ctors as their (unused) second argument, and
   referenced nowhere else in the original. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x004b0b60
char g_4b0b60[1];

/* Cleared when system\common\patch.ini is missing; read all over the game's
   screens (0x415c71, 0x44ed04, 0x4521f5...). Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f088
int g_1b2f088;

/* The .ini's "Wide" mode, published for the rest of the game. Placeholder. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb64
int g_1b2eb64;

// VTABLE: EZ2AC_UNPACKED 0x00487418
struct KEngineApp;

// FUNCTION: EZ2AC_UNPACKED 0x00401000
KEngineApp::KEngineApp()
{
    g_app = this;
    m_wndProcHook = 0;
    g_4a50d0 = -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004011e0
// KEngineApp::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401020
KEngineApp::~KEngineApp()
{
    // STRING: EZ2AC_UNPACKED 0x0048742c
    Log("KApplication::Delete");
    delete g_iniProfile;
    delete g_versionProfile;
    delete g_patchProfile;
}

/* Slot 1: the class-name getter (was a free function in src/classnames.cpp
   until the vtable pin needed it as a real virtual member). */
// FUNCTION: EZ2AC_UNPACKED 0x00401120
const char *KEngineApp::className()
{
    // STRING: EZ2AC_UNPACKED 0x00487444
    return "KEngineApp";
}

/* Slots 2 and 3 are both the shared bare `ret` @0x402880 — two compiled-out
   virtuals that the original's linker ICF-folded onto each other (and onto
   Log). `folded` maps both onto that one original address so the vtable slots
   symbolize identically on either side; only the first annotation there is
   scored. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void KEngineApp::v2()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void KEngineApp::v3()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00401070
int KEngineApp::v4()
{
    return -1;
}

/* The window class's message handler: it forwards every message to the app's
   optional hook (discarding the result), then handles WM_DESTROY / WM_CLOSE and
   sends the rest to DefWindowProc. A real WNDPROC, so a free __stdcall function
   rather than a member. */
// FUNCTION: EZ2AC_UNPACKED 0x00401130
LRESULT CALLBACK appWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    KWndProcHook hook = g_app->m_wndProcHook;

    if (hook != 0)
    {
        hook(hwnd, msg, wp, lp);
    }

    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcA(hwnd, msg, wp, lp);
}

/* Creates the app's three KProfiles and loads them. `iniPath` overrides the
   app's own .ini path; when it is empty the three paths are built from the
   directory the exe lives in, the app .ini being named after the (virtual)
   class name — "EZ2AC.ini" for the real app. A missing patch.ini only clears
   the g_1b2f088 flag; either of the other two failing is fatal.

   Each `new KProfile` is what puts the EH frame on this function: the ctor can
   throw (it calls Log), so cl tracks a state per new-expression so the unwinder
   can hand the memory back. */
// FUNCTION: EZ2AC_UNPACKED 0x00401240
int KEngineApp::loadProfiles(char *iniPath)
{
    char ini[260];
    char dir[260];
    char patchIni[260];
    char versionIni[260];

    g_iniProfile = new KProfile(0x80000, g_4b0b60);
    g_patchProfile = new KProfile(0x80000, g_4b0b60);
    g_versionProfile = new KProfile(0x80000, g_4b0b60);

    if (*iniPath != 0)
    {
        strncpy(ini, iniPath, 0x104);
    }
    else
    {
        getExeDir(dir);
        // STRING: EZ2AC_UNPACKED 0x004874a4
        sprintf(ini, "%s\\%s.ini", dir, className());
        // STRING: EZ2AC_UNPACKED 0x00487488
        sprintf(patchIni, "%s\\system\\common\\patch.ini", dir);
        // STRING: EZ2AC_UNPACKED 0x00487464
        sprintf(versionIni, "%s\\system\\title\\common\\version.ini", dir);
    }

    if (g_iniProfile->Load(ini) == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487450
        Log("Error on loading %s", ini);
        return 0;
    }

    if (g_versionProfile->Load(versionIni) == 0)
    {
        g_1b2f088 = 0;
    }

    if (g_patchProfile->Load(patchIni) == 0)
    {
        Log("Error on loading %s", patchIni);
        return 0;
    }

    return 1;
}

/* Registers the window class and creates the game window. The window position
   comes from the app .ini ("Window X"/"Window Y") and its size from the "Wide"
   mode: 1 = 854x480, 2 = 1440x1080, anything else = 640x480. Both the class name
   and the window title are the (virtual) class name, and the icon is the
   resource of that name. */
// FUNCTION: EZ2AC_UNPACKED 0x00401450
int KEngineApp::create(HINSTANCE hInst, HINSTANCE prev, char *cmdLine, int show)
{
    WNDCLASSA wc;
    int x;
    int y;
    int wide;
    int width;
    int height;

    if (loadProfiles(cmdLine) == 0)
    {
        return 0;
    }

    // STRING: EZ2AC_UNPACKED 0x004874e0
    g_iniProfile->setStr("CommandLine", cmdLine);

    wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wc.lpfnWndProc = appWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = 0;
    wc.hIcon = LoadIconA(0, className());
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = className();

    if (RegisterClassA(&wc) == 0)
    {
        return 0;
    }

    x = 0;
    y = 0;
    // STRING: EZ2AC_UNPACKED 0x004874d4
    g_iniProfile->getInt("Window X", &x);
    // STRING: EZ2AC_UNPACKED 0x004874c8
    g_iniProfile->getInt("Window Y", &y);
    // STRING: EZ2AC_UNPACKED 0x004874c0
    g_iniProfile->getInt("Wide", &wide);

    if (wide == 1)
    {
        width = 854;
        height = 480;
    }
    else if (wide == 2)
    {
        width = 1440;
        height = 1080;
    }
    else
    {
        width = 640;
        height = 480;
    }

    g_1b2eb64 = wide;
    // STRING: EZ2AC_UNPACKED 0x004874b0
    Log("Wide Num = %d", wide);

    m_hWnd = CreateWindowExA(WS_EX_APPWINDOW, className(), className(),
                             WS_POPUP | WS_VISIBLE, x, y, width, height,
                             0, 0, hInst, 0);
    if (m_hWnd == 0)
    {
        return 0;
    }

    m_hInst = hInst;
    UpdateWindow(m_hWnd);
    m_wndProcHook = 0;
    return 1;
}

/* The init / main-loop / shutdown trio, taken as a parameter rather than read
   from g_app three times: passing the pointer in is what makes cl materialize it
   in ecx for the first call and copy it to esi for the other two (a local read
   from the global goes straight into esi, and reading the global at each call
   reloads it three times — a virtual call may write it). */
static void driveApp(KEngineApp *app)
{
    app->v2();
    app->v4();
    app->v3();
}

/* The app's entry point (called with WinMain's arguments): bump the multimedia
   timer resolution, create the window, then drive the app through vtable slots
   2 (init), 4 (the main loop) and 3 (shutdown). The exit code is the global the
   ctor seeds with -1. */
// FUNCTION: EZ2AC_UNPACKED 0x004015e0
int __stdcall runApp(HINSTANCE hInst, HINSTANCE prev, char *cmdLine, int show)
{
    timeBeginPeriod(1);

    if (g_app == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004874ec
        MessageBoxA(0, "Critical Error: No KApplication Object.",
                    // STRING: EZ2AC_UNPACKED 0x00487514
                    "KEngine", MB_ICONHAND);
        return 0;
    }

    if (g_app->create(hInst, prev, cmdLine, show) == 0)
    {
        return 0;
    }

    driveApp(g_app);
    timeEndPeriod(1);
    return g_4a50d0;
}

/* Hands the directory the executable was started from to `out`: pull argv[0] out
   of the raw command line (the quoted form runs to the first '"' after the
   opening one, otherwise to the first ' '), then let extractDirName strip the file name off it. */
// FUNCTION: EZ2AC_UNPACKED 0x00401080
void getExeDir(char *out)
{
    char path[260];
    char *cmd;
    char *start;
    char *end;

    cmd = GetCommandLineA();

    if (*cmd == '"')
    {
        start = cmd + 1;
        end = strchr(start, '"');
    }
    else
    {
        start = cmd;
        end = strchr(cmd, ' ');
    }

    if (end == 0)
    {
        end = cmd + strlen(cmd);
    }

    path[0] = 0;
    strncpy(path, start, end - start);
    path[end - start] = 0;
    extractDirName(path, out);
}

/* One non-blocking step of the message loop. A member: KGraphics::m405e10
   @0x405e10 loads g_app into ecx before calling it, though `this` is unused. */
// FUNCTION: EZ2AC_UNPACKED 0x004011a0
void KEngineApp::pumpMessages()
{
    MSG msg;

    if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}
