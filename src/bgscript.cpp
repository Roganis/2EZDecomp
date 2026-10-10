/* BackGround::m41b900 @0x41b900 - read a background .scr script off disk and
   hand it to the parser. 553 bytes, 156 instructions, an EH function because
   of the KFile local.

   It is the first of a family of three: GFBackGround::m45cd40 @0x45cd40 is
   96.8% the same instruction sequence, and CatchBackGround::m46bc30 @0x46bc30
   a shorter variant at ~57%.

   Two things a port needs out of it:

     - THE VIDEO SHORT-CIRCUIT. If the object already has video running and the
       name it is handed is "commonbg.scr", the load is skipped entirely: f698
       is set and it returns 1 without opening anything. That is what stops the
       common background replacing a song's own video.
     - g_d36cb0 IS SUPPRESSED FOR THE WHOLE LOAD. It is saved on entry, zeroed,
       and put back at every exit - and once EARLY, in the branch that starts
       the video, so drawing resumes as soon as the surface exists rather than
       waiting for the script to be read. g_d36cb0 is the REVERSE (upside-down)
       flag: src/kgraphicsquad.cpp flips every quad to 479 - y and swaps v0/v1
       while it is set, so the load draws unflipped. (Corrected 2026-10-03; an
       earlier reading called a zero "do not draw yet".) The commonbg.scr
       short-circuit is the one exit that does NOT restore it.

   The buffer is a megabyte and the size check is against 0x100000 exactly, so
   the "Background Buf Over" line is logged for a script that would not fit -
   and then it reads it anyway, which is in the original. */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "ez2ac.h"
#include "panels.h"
#include "backgrounds.h"
#include "kfile.h"
#include "ez2acapp.h"   /* g_sub1b2f054 */

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern unsigned int SatSub(unsigned int a, unsigned int b);   /* 0x4145d0 */
/* The thunk form, not windows.h's dllimport one. */
extern "C" DWORD __stdcall timeGetTime();         /* 0x4145f0 */

extern int   g_d36cb0;                            /* 0x00d36cb0 (src/maingame.cpp) */
extern int   g_1b2ef40;                           /* src/moderunner.cpp */
extern int   g_1b2ea14;                           /* src/panels.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0041b900
int BackGround::m41b900(char *name)
{
    unsigned int t0;
    int          saved;
    int          one;
    unsigned int len;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    /* Declared HERE, not at the top: a local with a constructor runs it at its
       declaration point, and the original saves and clears g_d36cb0 before the
       KFile is built. */
    KFile        f;
    t0 = timeGetTime();

    if (g_1b2ef40 != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e2dc
        if (((Obj40B970 *)&m_1c0)->getField438() != 0 && _stricmp(name, "commonbg.scr") == 0)
        {
            f698 = 1;
            g_d36cb0 = saved;
            return 1;
        }
        if (((Obj40B970 *)&m_1c0)->getField438() == 0 && _access(m_videoPath, 0) != -1)
        {
            m_1c0.load((KGraphics *)g_sub1b2f054, m_videoPath);
            one = 1;
            f10 = 0;
            fa8 = one;
            /* Put it back as soon as the surface exists, not at the end. */
            g_d36cb0 = saved;
        }
        else
        {
            one = 1;
        }
    }
    else
    {
        one = 1;
    }

    if (((Obj40B970 *)&m_1c0)->getField438() == 0)
        f10 = one;
    if (f14 == one && f10 == one)
        g_1b2ea14 = one;

    if (f.open(name, one) == 0)
    {
        f14 = one;
        g_d36cb0 = saved;
        return 0;
    }

    len = f.size();
    if (len > 0x100000)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e2c8
        Log("Background Buf Over");
    }
    f.read(m_scrText, len);
    f.close();

    if (m41b680() == 0)
    {
        g_d36cb0 = saved;
        return 0;
    }

    // STRING: EZ2AC_UNPACKED 0x0048e2c4
    sprintf(f28, "%d,", SatSub(timeGetTime(), t0));
    fa8 = one;
    g_d36cb0 = saved;
    return one;
}

/* GFBackGround::m45cd40 @0x45cd40 - the scratch copy, 96.8% the same
   instruction sequence as m41b900 above and the same layout at every offset it
   touches. It even calls the SAME parser hook at 0x41b680 with its own `this`,
   which is why that one is reached by cast here.

   ONE structural difference, and it is the only one: the g_1b2ea14 latch runs
   BEFORE the `f10 = 1` assignment rather than after. Everything else that
   differs between the two listings is a branch displacement or the EH
   handler's address. */
// FUNCTION: EZ2AC_UNPACKED 0x0045cd40
int GFBackGround::m45cd40(char *name)
{
    unsigned int t0;
    int          saved;
    int          one;
    unsigned int len;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    KFile        f;
    t0 = timeGetTime();

    if (g_1b2ef40 != 0)
    {
        if (((Obj40B970 *)&m_1c0)->getField438() != 0 &&
            _stricmp(name, "commonbg.scr") == 0)
        {
            f698 = 1;
            g_d36cb0 = saved;
            return 1;
        }
        if (((Obj40B970 *)&m_1c0)->getField438() == 0 &&
            _access(m_videoPath, 0) != -1)
        {
            m_1c0.load((KGraphics *)g_sub1b2f054, m_videoPath);
            one = 1;
            f10 = 0;
            fa8 = one;
            g_d36cb0 = saved;
        }
        else
        {
            one = 1;
        }
    }
    else
    {
        one = 1;
    }

    /* The latch first here, where BackGround's copy assigns f10 first. */
    if (f14 == one && f10 == one)
        g_1b2ea14 = one;
    if (((Obj40B970 *)&m_1c0)->getField438() == 0)
        f10 = one;

    if (f.open(name, one) == 0)
    {
        f14 = one;
        g_d36cb0 = saved;
        return 0;
    }

    len = f.size();
    if (len > 0x100000)
        Log("Background Buf Over");
    f.read(m_scrText, len);
    f.close();

    if (((BackGround *)this)->m41b680() == 0)
    {
        g_d36cb0 = saved;
        return 0;
    }

    sprintf(f28, "%d,", SatSub(timeGetTime(), t0));
    fa8 = one;
    g_d36cb0 = saved;
    return one;
}

/* CatchBackGround::m46bc30 @0x46bc30 - the Ez2Catch copy, the shortest of the
   three at 140 instructions. Its class puts the same fields at its own
   offsets: the two flags BackGround keeps at +0x10/+0x14 are at +0xac/+0xb0
   here, the video object at +0x1b8, the script at +0x694 and the readout at
   +0x20. It has its own parser hook, m46b9b0, rather than sharing 0x41b680.

   Three real differences from the other two, not just offsets:

     - NO g_1b2ef40 GATE. The other two skip the whole video section unless
       that flag is up; this one always runs it.
     - IT NEVER CLEARS THE FIRST FLAG. Where BackGround's video branch sets
       f10 = 0 alongside fa8 = 1, this one only sets fa0.
     - THE SAVED g_d36cb0 LIVES IN A FRAME SLOT, not a register, and is
       RELOADED after the video branch rather than restored early - so the
       suppression holds across the whole load here where BackGround lifts it
       as soon as the surface exists.

   The early "commonbg.scr" return and the success return SHARE one exit, both
   handing back the same 1. */
// FUNCTION: EZ2AC_UNPACKED 0x0046bc30
int CatchBackGround::m46bc30(char *name)
{
    unsigned int t0;
    volatile int saved;
    int          one;
    unsigned int len;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    KFile        f;
    t0 = timeGetTime();

    if (((Obj40B970 *)&m_1b8)->getField438() != 0 &&
        _stricmp(name, "commonbg.scr") == 0)
    {
        one = 1;
        f690 = one;
        /* The original SHARES the success path's exit here and SKIPS the
           restore (0x46bca8 -> 0x46bdf0): g_d36cb0 stays 0 after this
           return. Written without the restore - a restore here scored 2
           points higher but is behaviour the binary does not have (found
           by the port, CATCH-REVIEW.md S6). */
        return one;
    }

    one = 1;
    if (((Obj40B970 *)&m_1b8)->getField438() == 0 &&
        _access(m_videoPath, 0) != -1)
    {
        m_1b8.load((KGraphics *)g_sub1b2f054, m_videoPath);
        fa0 = one;
    }

    if (((Obj40B970 *)&m_1b8)->getField438() == 0)
        fac = one;
    if (fb0 == one && fac == one)
        g_1b2ea14 = one;

    if (f.open(name, one) == 0)
    {
        g_d36cb0 = saved;
        return 0;
    }

    len = f.size();
    if (len > 0x100000)
        Log("Background Buf Over");
    f.read(m_scrText, len);
    f.close();

    if (m46b9b0() == 0)
    {
        g_d36cb0 = saved;
        return 0;
    }

    sprintf(f20, "%d,", SatSub(timeGetTime(), t0));
    fa0 = one;
    g_d36cb0 = saved;
    return one;
}
