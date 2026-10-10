#include <stdio.h>
#include <windows.h>
#include "kcrt.h"
#include "ksound.h"   /* KRuntimeClass */
#include "bigbase.h"  /* EzObjectInfo */
#include "ezscreen.h"
#include "kprofile.h"
#include "kparser.h"
#include "kengineapp.h"   /* g_patchProfile */
#include "bigbase.h"      /* g_bigInstance */
#include "directors.h"    /* SoundBank */

/* The .ini's "ResetTime" (readSettings @0x415a40 names it), which the
   constructor seeds its +0x10 from. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01696320
int g_resetTime;

/* Storage for the class's own runtime-class descriptor (contents
   unreconstructed; only the address matters). */
// GLOBAL: EZ2AC_UNPACKED 0x004aea58
KRuntimeClass classResetScreen;

/* src/classinfo.cpp - the object record the factory allocates from. */
extern EzObjectInfo g_info_4aea68;

// FUNCTION: EZ2AC_UNPACKED 0x0046a4e0
KRuntimeClass *Obj46AA80::getRuntimeClass()
{
    return &classResetScreen;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a4f0
void *Obj46AA80::getObjectInfo()
{
    return &g_info_4aea68;
}

/* Empty - the screen owns nothing. cl still emits the vptr store and the
   tail-jump to the folded base destructor. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a500
Obj46AA80::~Obj46AA80()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ab00
// Obj46AA80::`scalar deleting destructor'

/* src/strutil.cpp - the game's chdir pair. */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
/* Declared here rather than taken from windows.h: the original calls the
   5-byte non-dllimport thunk @0x4145f0, not the indirect IAT form. */
extern "C" unsigned long __stdcall timeGetTime(void);

/* The reset screen's director. Built out of the pool by createObj46AA80
   @0x46ab20, so it must stay out of that TU or cl inlines the whole body into
   the factory. The EH frame is the base class construction's, and the directory
   is pushed and immediately popped - the visit itself is the point. */
// FUNCTION: EZ2AC_UNPACKED 0x0046aa80
Obj46AA80::Obj46AA80()
{
    f10 = g_resetTime;
    f14 = 0;
    f18 = 0x78;
    f20 = 0;

    // STRING: EZ2AC_UNPACKED 0x0049d1ec
    changeDir("System\\ResetScreen", 1);
    popDirectory();

    f24 = timeGetTime();
}

// VTABLE: EZ2AC_UNPACKED 0x0049cfd0
struct Obj46AA80;


/* Vtable slot 5 is the shared `xor eax,eax; ret 0xc` the original's linker
   folded - the same body as ret0_arg3 in src/constants.cpp, which owns the
   address (its file sorts first). */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int Obj46AA80::v5(int, int, int)
{
    return 0;
}

/* The reset screen's draw @0x46a6d0: clear, then the "applying update
   packages / do not turn off / remaining packages / remaining time or please
   wait" lines in Korean (g_language 0) or English, white with the warning in
   red. The Korean lines are EUC-KR literals in .rdata; they are pinned as
   placeholder storage rather than spelled out here. Two halves written out
   in full, per CLAUDE.md's rule - cl cross-jumps only the last call. */
extern int g_language;          /* 0x16963f4 (src/testinput.cpp) */
extern int g_remainPatch;       /* 0x16963f0 (src/settings.cpp) */
extern int g_initializePatch;   /* 0x16963d8 (src/settings.cpp) */
extern void drawTextf(struct KGraphics *g, int x, int y, const char *fmt, ...);   /* 0x4071d0 */
struct Obj406C00 { void setField12c(int v); };            /* 0x406c00 - the text colour */
struct Obj40BB00 { void doOp10(unsigned int color); };    /* 0x40a730 - device Clear */
struct KGraphics;
extern struct EzSubsystem *g_sub1b2f054;

// GLOBAL: EZ2AC_UNPACKED 0x0049d1b8
char g_strUpdateKor[0x20];
// GLOBAL: EZ2AC_UNPACKED 0x0049d198
char g_strRebootKor[0x20];
// GLOBAL: EZ2AC_UNPACKED 0x0049d17c
char g_strNoPowerKor[0x1c];
// GLOBAL: EZ2AC_UNPACKED 0x0049d164
char g_strRemainPkgKor[0x18];
// GLOBAL: EZ2AC_UNPACKED 0x0049d150
char g_strRemainTimeKor[0x14];
// GLOBAL: EZ2AC_UNPACKED 0x0049d138
char g_strWaitKor[0x18];

// FUNCTION: EZ2AC_UNPACKED 0x0046a6d0
void Obj46AA80::v3()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    if (g_language == 0)
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, g_strUpdateKor);
        drawTextf((KGraphics *)g_sub1b2f054, 20, 40, g_strRebootKor);
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff0a0a);
        drawTextf((KGraphics *)g_sub1b2f054, 20, 60, g_strNoPowerKor);
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        drawTextf((KGraphics *)g_sub1b2f054, 20, 80, g_strRemainPkgKor, g_remainPatch);
        if (g_initializePatch == 0)
            drawTextf((KGraphics *)g_sub1b2f054, 20, 100, g_strRemainTimeKor, f10);
        else
            drawTextf((KGraphics *)g_sub1b2f054, 20, 100, g_strWaitKor, f10);
    }
    else
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x0049d118
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, "Applying Game Update Packages");
        // STRING: EZ2AC_UNPACKED 0x0049d0d8
        drawTextf((KGraphics *)g_sub1b2f054, 20, 40, "The system will automatically reboot after remaining time over.");
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff0a0a);
        // STRING: EZ2AC_UNPACKED 0x0049d0b8
        drawTextf((KGraphics *)g_sub1b2f054, 20, 60, "Do not turn off your machine.");
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x0049d098
        drawTextf((KGraphics *)g_sub1b2f054, 20, 80, "Remaining Update Packages : %d", g_remainPatch);
        if (g_initializePatch == 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0049d07c
            drawTextf((KGraphics *)g_sub1b2f054, 20, 100, "Remaining Time : %02d secs", f10);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0049d064
            drawTextf((KGraphics *)g_sub1b2f054, 20, 100, "Please Wait a Moment.", f10);
        }
    }
}

/* The update screen's tick @0x46a880 (vtable slot 4). Count a second off
   f10 every sixty ticks (f14); when it runs out, count f18 down instead and,
   at zero, ask for the reboot (g_169641c) and tell the sound bank. With the
   ranking wipe requested, delete the ranking files. Once no patches remain,
   clear RestartSystem (and InitializePatch / the nine ranking-delete keys
   when they are off) in the version profile, write it back to
   system\common\patch.ini and re-encrypt it through a KParser. Slot 0 of
   the engine's first table reading 2 tells the sound bank as well. */
extern int g_169641c;           /* src/ez2acapp.cpp */
extern int g_allRankingFileDelete;   /* 0x16963d0 (src/settings.cpp) */
extern void *g_1b2e844;         /* src/moderunner.cpp - the sound bank */

// FUNCTION: EZ2AC_UNPACKED 0x0046a880
void Obj46AA80::v4(int, int)
{
    char path[0x104];

    // STRING: EZ2AC_UNPACKED 0x0049d1d4
    sprintf(path, "system\\common\\patch.ini");
    f14++;
    if (f10 <= 0)
    {
        f18--;
        f10 = 0;
    }
    else if (f14 >= 60)
    {
        f14 = 0;
        f10--;
    }
    if (f18 <= 0)
    {
        f18 = 0;
        g_169641c = 1;
        ((SoundBank *)g_1b2e844)->v0(0);
    }
    if (g_allRankingFileDelete >= 1)
        deleteRankings();
    if (g_remainPatch == 0)
    {
        g_patchProfile->setInt("RestartSystem", 0);
        if (g_initializePatch == 0)
            g_patchProfile->setInt("InitializePatch", 0);
        if (g_allRankingFileDelete == 0)
        {
            g_patchProfile->setInt("AllRankingFileDelete", 0);
            g_patchProfile->setInt("DelTurntableRanking", 0);
            g_patchProfile->setInt("DelStreetMixRanking", 0);
            g_patchProfile->setInt("DelSpaceMixRanking", 0);
            g_patchProfile->setInt("DelRubyMixRanking", 0);
            g_patchProfile->setInt("DelEz2CatchRanking", 0);
            g_patchProfile->setInt("DelClubMixRanking", 0);
            g_patchProfile->setInt("Del7StreetMixRanking", 0);
            g_patchProfile->setInt("Del5keymixRanking", 0);
        }
        g_patchProfile->writeFile(path);
        {
            KParser p;
            p.m4133d0(path);
        }
    }
    if (g_bigInstance->getState(0) == 2)
        ((SoundBank *)g_1b2e844)->v0(0);
}

/* Wipe the rankings the patch asked for: the five per-mode rank files under
   sound\, then everything under system\ranking\ - TestBookkeeping's
   deleteRankingFiles @0x474a40 with the five patterns spelled out. Clears the
   two request flags at the end. */

// FUNCTION: EZ2AC_UNPACKED 0x0046a510
void Obj46AA80::deleteRankings()
{
    WIN32_FIND_DATAA fd;
    HANDLE h;

    changeDir("sound\\", 1);
    // STRING: EZ2AC_UNPACKED 0x0049d048
    h = FindFirstFileA("rank_5keymix_*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    // STRING: EZ2AC_UNPACKED 0x0049d030
    h = FindFirstFileA("rank_streetmix_*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    // STRING: EZ2AC_UNPACKED 0x0049d018
    h = FindFirstFileA("rank_7streetmix_*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    // STRING: EZ2AC_UNPACKED 0x0049d004
    h = FindFirstFileA("rank_clubmix_*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    // STRING: EZ2AC_UNPACKED 0x0049cff0
    h = FindFirstFileA("rank_spacemix_*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    popDirectory();

    changeDir("system\\ranking\\", 1);
    h = FindFirstFileA("*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
                DeleteFileA(fd.cFileName);
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    popDirectory();
    g_initializePatch = 0;
    g_allRankingFileDelete = 0;
}
