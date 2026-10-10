#include <stdlib.h>
#include <time.h>
#include "kcrt.h" /* operator delete == an inlined free */
#include "ez2acapp.h"
#include "bigbase.h" /* g_bigInstance @0x1b2e710 */
#include "ezscreen.h"
#include "settings.h"

/* setBasePath @0x412db0 lives in src/strutil.cpp — a separate TU, so cl cannot
   inline its strcpy into the ctor below and drop the call the original makes. */
extern void setBasePath(const char *path);

/* WINMM's timeGetTime, declared WITHOUT dllimport: the original reaches it
   through the import library's thunk (`call 0x4145f0` -> `jmp
   [__imp__timeGetTime]`), the same shape as BlockInput above. mmsystem.h (pulled
   in by windows.h) declares the dllimport form, which would compile to a 6-byte
   indirect call; timeBeginPeriod/timeEndPeriod in runApp DO use that form, so the
   original mixed the two — nothing stops that, they are different declarations.
   The result is discarded at both call sites. */
extern "C" DWORD __stdcall timeGetTime(void);

/* src/stubs.cpp — the engine-wide per-round helpers runGame drives. */
extern void saveSettings();   /* 0x415310, src/moderunner.cpp */
extern void engineTick418e60();
extern void engineTick418ee0();
extern void engineTick418f60();

/* src/modename.cpp — the mode index runGame dispatches on. */
extern int g_modeIndex;

/* src/loops.cpp — the element/slot cursor runGame clears each round. */
extern int g_elemCount;

/* EZ2AC — the game's application class (see include/ez2acapp.h). Its member
   sub-object's ctor/dtor live in src/subobj402860.cpp so cl cannot inline them
   into the ctor/dtor here, which is what keeps the original's two calls. */

/* Every slot is accounted for (0/1 matched here, 2-4 mapped by the STUBs in
   src/stubs.cpp), so the vtable can be pinned: without it the dtor's vptr store
   has nothing to resolve 0x48cf34 to on the original side. */
// VTABLE: EZ2AC_UNPACKED 0x0048cf34
struct EZ2AC;

/* Beyond the compiler-generated base ctor, vptr store and member construction,
   the body publishes the process's current directory as the engine's base path
   and seeds the LCG. */
// FUNCTION: EZ2AC_UNPACKED 0x00414ea0
EZ2AC::EZ2AC()
{
    char dir[260];

    GetCurrentDirectoryA(0x104, dir);
    setBasePath(dir);
    srand((unsigned int)time(0));
}

/* USER32's BlockInput, deliberately declared WITHOUT dllimport: the original
   calls it through the import library's thunk (`call 0x478522` -> `jmp
   [__imp__BlockInput]`), which is what you get when the declaration is not
   __declspec(dllimport). windows.h's declaration is the dllimport one and would
   compile to a 6-byte indirect call instead. */
extern "C" BOOL __stdcall BlockInput(BOOL fBlockIt);

/* The 0x1b2e320 singleton's class, as far as it is known: src/misc.cpp matched
   its `this+0xd4 = v` setter @0x407350, which v2 calls. Repeated here (rather
   than shared through a header) so the two definitions stay where their owners
   are; v3 reaches the same object through EzSubsystem's slot 3. */
struct Obj407350 { char _pad[0xd4]; int field_0xd4; void setFieldD4(int v); };

/* The subsystem singletons themselves — static objects whose addresses v2
   publishes into the globals below. Placeholder storage (declaring them with
   their real types would force cl to emit vftables for classes we have only
   modeled, so v2 casts the storage instead). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e290
static char g_obj1b2e290[1];
// GLOBAL: EZ2AC_UNPACKED 0x01b2e320
static char g_obj1b2e320[1];
// GLOBAL: EZ2AC_UNPACKED 0x01b2e4f8
static char g_obj1b2e4f8[1];
// GLOBAL: EZ2AC_UNPACKED 0x01696430
static char g_obj1696430[1];
// GLOBAL: EZ2AC_UNPACKED 0x0170dff0
static char g_obj170dff0[1];

/* Subsystem singletons, torn down by v3 below. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e70c
EzSubsystem *g_sub1b2e70c;
// GLOBAL: EZ2AC_UNPACKED 0x01b2eae4
EzSubsystem *g_sub1b2eae4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef98
EzSubsystem *g_sub1b2ef98;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f054
EzSubsystem *g_sub1b2f054;

/* The "UseGameOver" setting, published for the rest of the game. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e560
int g_useGameOver;

/* Cleared once per runGame; the rest is unreconstructed. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef7c
int g_1b2ef7c;

/* Round counter, bumped every dispatch round. */
// GLOBAL: EZ2AC_UNPACKED 0x016963cc
int g_totalPlay;

/* The three flags that gate the mode dispatch. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb6c
int g_1b2eb6c;
// GLOBAL: EZ2AC_UNPACKED 0x01b2eadc
int g_1b2eadc;
// GLOBAL: EZ2AC_UNPACKED 0x01696400
int g_1696400;

/* Set to 1 to ask for a machine reboot on the way out. */
// GLOBAL: EZ2AC_UNPACKED 0x0169641c
int g_169641c;

/* The five screens runGame creates. Their vftables have to be pinned for the
   inlined ctors' vptr stores to resolve; every slot is a STUB in src/stubs.cpp,
   which is enough for the vtable data to compare equal. */
// VTABLE: EZ2AC_UNPACKED 0x0048d310
struct Screen48D310;
// VTABLE: EZ2AC_UNPACKED 0x0048d348
struct Screen48D348;
// VTABLE: EZ2AC_UNPACKED 0x0048d3d4
struct Screen48D3D4;
// VTABLE: EZ2AC_UNPACKED 0x0048d40c
struct Screen48D40C;
// VTABLE: EZ2AC_UNPACKED 0x0048d428
struct Screen48D428;

/* The game proper: a round loop that creates a screen, runs it and dispatches on
   its return code — 0 quits, 2 and 3 run another screen, anything else advances
   the game and runs the current mode (g_modeIndex, i.e. kModeNames). When m_f1c
   drops the loop runs one last screen and, if g_169641c asks for it, reboots the
   machine. */
// FUNCTION: EZ2AC_UNPACKED 0x00417e90
int EZ2AC::runGame()
{
    int useGameOver;
    HANDLE token;
    TOKEN_PRIVILEGES priv;

    useGameOver = 1;
    // STRING: EZ2AC_UNPACKED 0x0048d658
    g_iniProfile->getInt("UseGameOver", &useGameOver);
    g_useGameOver = useGameOver;

    m_f18 = 1;
    m_f14 = 0;
    g_1b2ef7c = 0;
    m_f6d830 = 0;

    for (;;)
    {
        m_f78 = 1;

        if (m_f1c != 0)
        {
            int code;

            if (m_f18 == 1)
            {
                Screen48D40C *first = new Screen48D40C;
                first->run(g_iniProfile);
                delete first;
                m_f18 = 0;
            }

            Screen48D348 *round = new Screen48D348;
            code = round->run(g_iniProfile);
            delete round;

            if (code == 0)
            {
                engineTick418e60();
                engineTick418ee0();
                engineTick418f60();
                return 0;
            }

            if (code == 2)
            {
                Screen48D310 *two = new Screen48D310;
                two->run(g_iniProfile);
                delete two;
            }
            else if (code == 3)
            {
                Screen48D428 *three = new Screen48D428;
                three->run(g_iniProfile);
                delete three;
                continue;
            }
            else
            {
                g_elemCount = 0;
                timeGetTime();
                g_totalPlay++;

                /* The case order is the original's, not numeric: cl emits switch
                   arms in source order, and the jump table's entries are
                   compared as offsets into the function. */
                if (g_1b2eb6c == 0 && (g_1b2eadc == 0 || g_1696400 != 0) &&
                    m415f00() == 0)
                {
                    switch (g_modeIndex)
                    {
                    case 0:
                        run5KeyMix();
                        break;
                    case 1:
                        runRubyMix();
                        break;
                    case 2:
                        runStreetMix();
                        break;
                    case 3:
                        run7StreetMix();
                        break;
                    case 6:
                        run5RadioMix();
                        break;
                    case 8:
                        run10RadioMix();
                        break;
                    case 9:
                        run14RadioMix();
                        break;
                    case 7:
                        runRadioMix();
                        break;
                    case 4:
                        runClubMix();
                        break;
                    case 5:
                        runSpaceMix();
                        break;
                    case 10:
                        runEz2Catch();
                        break;
                    case 11:
                        runScratchMix();
                        break;
                    case 12:
                        runCv2Mix();
                        break;
                    }
                }
            }

            engineTick418e60();
            engineTick418ee0();
            engineTick418f60();
            timeGetTime();
            saveSettings();
            continue;
        }

        Screen48D3D4 *last = new Screen48D3D4;
        last->run(g_iniProfile);
        delete last;

        if (g_169641c == 1)
        {
            if (OpenProcessToken(GetCurrentProcess(),
                                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                 &token) == 0)
            {
                // STRING: EZ2AC_UNPACKED 0x0048d644
                LookupPrivilegeValueA(0, "SeShutdownPrivilege",
                                      &priv.Privileges[0].Luid);
            }

            priv.PrivilegeCount = 1;
            priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            AdjustTokenPrivileges(token, FALSE, &priv, 0, 0, 0);
            ExitWindowsEx(EWX_REBOOT, 0);
            return 0;
        }
    }
}

/* Slot 2: bring the engine up — publish each subsystem singleton, initialize
   them in order, wire the sound owner and block user input; m_f10 records that
   it succeeded (v3 and v4 both key off it). */
// FUNCTION: EZ2AC_UNPACKED 0x00415cb0
void EZ2AC::v2()
{
    m_f10 = 0;

    g_sub1b2f054 = (EzSubsystem *)g_obj1b2e320;
    g_sub1b2eae4 = (EzSubsystem *)g_obj1696430;
    g_bigInstance = (Obj40C180 *)g_obj170dff0;
    g_sub1b2e70c = (EzSubsystem *)g_obj1b2e290;
    g_sub1b2ef98 = (EzSubsystem *)g_obj1b2e4f8;

    g_sub1b2f054->init();
    g_sub1b2eae4->init();
    g_bigInstance->init();
    g_sub1b2e70c->init();
    g_sub1b2ef98->init();

    ((Obj407350 *)g_sub1b2f054)->setFieldD4(1);
    g_sub1b2eae4->bind(&g_4a72b8);
    g_sub1b2eae4->f48 = &g_4a72b8;
    g_sub1b2eae4->f4c = 1;
    BlockInput(TRUE);

    m_f10 = 1;
}

/* Slot 3: shut the engine down — nothing to do unless m_f10 says the subsystems
   came up, then each singleton is shut down and cleared. */
// FUNCTION: EZ2AC_UNPACKED 0x00414ff0
void EZ2AC::v3()
{
    if (m_f10 != 0)
    {
        if (g_sub1b2ef98 != 0)
        {
            g_sub1b2ef98->shutdown();
            g_sub1b2ef98 = 0;
        }

        if (g_sub1b2e70c != 0)
        {
            g_sub1b2e70c->shutdown();
            g_sub1b2e70c = 0;
        }

        if (g_bigInstance != 0)
        {
            g_bigInstance->shutdown();
            g_bigInstance = 0;
        }

        if (g_sub1b2eae4 != 0)
        {
            g_sub1b2eae4->shutdown();
            g_sub1b2eae4 = 0;
        }

        if (g_sub1b2f054 != 0)
        {
            g_sub1b2f054->shutdown();
            g_sub1b2f054 = 0;
        }
    }
}

/* Slot 4: the app's main entry. Does nothing unless the subsystems came up, and
   bails out while Tab is held; otherwise the app .ini picks between the
   two test harnesses and the game proper. */
// FUNCTION: EZ2AC_UNPACKED 0x004181b0
int EZ2AC::v4()
{
    int testSong;
    int testGame;

    if (m_f10 == 0)
    {
        return 0;
    }

    if ((GetAsyncKeyState(VK_TAB) & 0x8000) != 0)
    {
        return 0;
    }

    readSettings();

    testSong = 0;
    // STRING: EZ2AC_UNPACKED 0x0048d670
    g_iniProfile->getInt("TestSong", &testSong);
    if (testSong != 0)
    {
        runTestSong();
        return 0;
    }

    testGame = 0;
    // STRING: EZ2AC_UNPACKED 0x0048d664
    g_iniProfile->getInt("TestGame", &testGame);
    if (testGame != 0)
    {
        runTestGame();
        return 0;
    }

    runGame();
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00415a20
// EZ2AC::`scalar deleting destructor'

/* The body is empty: everything the original does is compiler-generated — the
   vptr store, the member sub-object's destructor, then the base dtor (and the
   EH frame that guarantees the base dtor still runs if the member's throws). */
// FUNCTION: EZ2AC_UNPACKED 0x00414f50
EZ2AC::~EZ2AC()
{
}

/* Slot 1. Was a free function in src/classnames.cpp until the class was
   modeled. */
// FUNCTION: EZ2AC_UNPACKED 0x00414fb0
const char *EZ2AC::className()
{
    // STRING: EZ2AC_UNPACKED 0x0048cf48
    return "EZ2AC";
}

/* The two readers readSettings @0x415a40 finishes with. They live here, not in
   src/settings.cpp, because it calls the first and TAIL-JUMPS the second. */

/* Re-read the maintenance switches - the same thirteen patch.ini keys
   readSettings already read once inline. No STRING markers: src/settings.cpp
   annotates all thirteen addresses. */
// FUNCTION: EZ2AC_UNPACKED 0x00415900
void EZ2AC::readPatchSettings()
{
    g_patchProfile->getInt("RestartSystem", &g_restartSystem);
    g_patchProfile->getInt("ResetTime", &g_resetTime);
    g_patchProfile->getInt("RemainPatch", &g_remainPatch);
    g_patchProfile->getInt("InitializePatch", &g_initializePatch);
    g_patchProfile->getInt("AllRankingFileDelete", &g_allRankingFileDelete);
    g_patchProfile->getInt("DelTurntableRanking", &g_delTurntableRanking);
    g_patchProfile->getInt("DelStreetMixRanking", &g_delStreetMixRanking);
    g_patchProfile->getInt("DelSpaceMixRanking", &g_delSpaceMixRanking);
    g_patchProfile->getInt("DelRubyMixRanking", &g_delRubyMixRanking);
    g_patchProfile->getInt("DelEz2CatchRanking", &g_delEz2CatchRanking);
    g_patchProfile->getInt("DelClubMixRanking", &g_delClubMixRanking);
    g_patchProfile->getInt("Del7StreetMixRanking", &g_del7StreetMixRanking);
    g_patchProfile->getInt("Del5keymixRanking", &g_del5KeyMixRanking);
}

/* Fold the operator's bookkeeping file into the machine's counters: the coin
   slot's own reading becomes the pending-coin count, and the running totals are
   ADDED to what the .ini already had. Win32's own profile reader this time, not
   a KProfile - and the IAT slot is loaded once into a register because all three
   calls go through the same import. */
// FUNCTION: EZ2AC_UNPACKED 0x00415070
int EZ2AC::readBookkeeping()
{
    // STRING: EZ2AC_UNPACKED 0x0048cf70
    g_bigInstance->have = GetPrivateProfileIntA("GAMEASSIGNMENTS",
    // STRING: EZ2AC_UNPACKED 0x0048cf68
                                                "Coins", 0,
    // STRING: EZ2AC_UNPACKED 0x004a8dd0
                                                ".\\EZ2DJ1st\\bookkeeping.ini");
    // STRING: EZ2AC_UNPACKED 0x0048cf8c
    g_totalCoin += GetPrivateProfileIntA("STATISTICS",
    // STRING: EZ2AC_UNPACKED 0x0048cf98
                                         "TOTALCOIN", 0,
                                         ".\\EZ2DJ1st\\bookkeeping.ini");
    g_serviceCoin += GetPrivateProfileIntA("STATISTICS",
    // STRING: EZ2AC_UNPACKED 0x0048cf80
                                           "SERVICECOIN", 0,
                                           ".\\EZ2DJ1st\\bookkeeping.ini");
    return 1;
}
