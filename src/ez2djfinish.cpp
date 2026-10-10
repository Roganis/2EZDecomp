/* EZ2DJMainGameDirector::m424830 - step past a finished song.  Its own
   TU, away from what it calls: ScrFader::m450420 in src/directors.cpp,
   the all-finished check in src/ez2djgameover.cpp, and the stage
   starter in src/ez2djstage.cpp. */
#include "ezscreen.h"
#include "sink-view.h"
#include <stdio.h>

extern char g_modeName[256];        /* 0x1b2e680 (src/modename.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern struct Effector *createEffector();                    /* 0x41d060 */
extern struct AudienceRatePanel *createAudienceRatePanel();  /* 0x41ad50 */
extern struct Effector6thStyle *createEffector6thStyle();    /* 0x41f860 */

struct Obj41D1C0 { char _pad[0x10]; int f; void set(int v); };

extern int   g_elemCount;                   /* 0x1b2e718 */
extern void *g_1b2e844;                     /* src/moderunner.cpp */
extern int   g_1b2ea28;                     /* src/moderunner.cpp */
extern int   g_1b2eb6c;                     /* src/ez2acapp.cpp - the CV2 flag */
extern int   g_modeIndex;                   /* 0x1b2e548 (src/modename.cpp) */
extern int   g_1b5f19c;                     /* src/playerslot.cpp */
extern int   g_useAudiencePanel;            /* 0x1b2eb48 (src/maingame.cpp) */
extern int   g_1b2e7d8;
extern int   countListA();                  /* 0x418310 (src/nodelists.cpp) */
extern int   countListC();                  /* 0x4184a0 (src/nodelists.cpp) */

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct Obj1B2E844f
{
    virtual void q00(int a);
};

/* Step past the finished song.  Nothing to do mid-fade (f20 1 or 2).
   In survival gauge mode a finished field jumps straight to screen 3.
   With stages left (f7c4 up and the counter short of f7c8) begin the
   next one; otherwise notify slot 9 and start the exit fade once.

   Left: the original holds the constant 1 in edi (pushed after esi) for
   its three uses - `f20 == 1`, `f794 == 1` and `f20 = 1` - where cl uses
   immediates. Early-return and `&&` spellings of the head do not move it. */
// FUNCTION: EZ2AC_UNPACKED 0x00424830
void EZ2DJMainGameDirector::m424830()
{
    int a = f20;

    if (a != 1 && a != 2)
    {
        if (f794 == 1)
        {
            if (m4204b0() != 0)
            {
                ((Obj1B2E844f *)g_1b2e844)->q00(3);
                return;
            }
        }
        /* The conjunction materialized (cookbook, success-block-after-
           the-return): the original lays the notify arm inline and the
           next-stage arm last, falling into the shared pop/ret. */
        int more = f7c4 != 0 && g_elemCount + 1 < f7c8;
        if (more == 0)
        {
            notify->v9();
            if (f20 == 0)
            {
                f15c->m450420(2, 10, (EZ2DJSink1 *)this);
                f20 = 1;
            }
            return;
        }
        m4246d0();
        f20 = 0;
    }
}

/* Decide whether a DEAD GAUGE ends the session. g_1b2ea28 is the GAME-OVER
   latch, not a win: m4204b0 (src/ez2djgameover.cpp) answers 1 only when a
   player's gauge has hit zero (PlayerSlot::m42e590), and every raise below
   sits behind that test. The runners read it after the main game - 1 sends
   the session to the game-over screen once the round threshold is met, 0
   lets the result screen run (EZ2AC::runClubMix @0x417670) - and song
   select, the stage start, the ranking and the ending clear it. Nothing
   while f350 is latched or nobody is dead; under CV2 always game over.
   Mode 1 ends only past fifty g_1b5f19c; the audience panel, the first
   element's radio modes, and a two-up field all just unlatch (the stage
   plays on with the gauge at zero). What is left ends the session on a
   later element - the two-up arm behind the dead `== 2` test (unreachable
   after the `> 1` bail) additionally wants g_1b2e7d8 at 2. */
// FUNCTION: EZ2AC_UNPACKED 0x00420570
void SinkDirector::m420570(SinkSlot *p)
{
    EZ2DJMainGameDirector *d = (EZ2DJMainGameDirector *)this;
    int mode;

    if (d->f350 != 0)
        return;
    if (d->m4204b0() == 0)
        return;
    if (g_1b2eb6c != 0)
    {
        g_1b2ea28 = 1;
        return;
    }
    mode = g_modeIndex;
    if (mode == 1)
    {
        if (g_1b5f19c < 0x32)
        {
            d->f350 = 0;
            return;
        }
        g_1b2ea28 = 1;
        return;
    }
    if (g_useAudiencePanel == 1)
    {
        d->f350 = 0;
        return;
    }
    if (g_elemCount == 0)
    {
        if (mode == 7 || mode == 6 || mode == 8 || mode == 9)
        {
            d->f350 = 0;
            return;
        }
    }
    if (countListA() > 1)
    {
        d->f350 = 0;
        return;
    }
    if (countListA() != 2)
    {
        if (g_elemCount == 0)
        {
            d->f350 = 0;
            return;
        }
        g_1b2ea28 = 1;
        return;
    }
    if (countListA() < 2)
        return;
    if (g_elemCount == 0)
    {
        d->f350 = 0;
        return;
    }
    if (g_1b2e7d8 < 2)
        return;
    g_1b2ea28 = 1;
}

/* The scratch director's game-over raise - the loose twin of m420570
   above: no mode-1, audience-panel or radio-mode arms, and the f31c
   latch instead of f350.  countListC really is read twice, and the
   unlatch store is redundant (the entry guard proved it zero) but the
   original makes it. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d860
void GFSinkDirector::m45d860(GFSinkSlot *p)
{
    GFMainGameDirector *d = (GFMainGameDirector *)this;

    if (d->f31c != 0)
        return;
    if (d->m45d740() == 0)
        return;
    if (g_1b2eb6c != 0)
    {
        g_1b2ea28 = 1;
        return;
    }
    if (countListC() != 2)
    {
        if (g_elemCount != 0)
        {
            g_1b2ea28 = 1;
            return;
        }
        d->f31c = 0;
        return;
    }
    if (countListC() < 2)
        return;
    if (g_elemCount == 0)
    {
        d->f31c = 0;
        return;
    }
    if (g_1b2e7d8 < 2)
        return;
    g_1b2ea28 = 1;
}

/* EZ2DJMainGameDirector::m420e30 @0x420e30 - build the three overlays
   out of the mode's Panel directory: the effector inside it, then the
   audience-rate panel (audience mode only) and the 6th-style effector
   outside, the latter handed the address of this director's f18. */
// FUNCTION: EZ2AC_UNPACKED 0x00420e30
int EZ2DJMainGameDirector::m420e30()
{
    char buf[0x104];

    /* the literal is STRING-pinned by src/playersink.cpp */
    sprintf(buf, "System\\%s\\Panel", g_modeName);
    changeDir(buf, 1);
    f788 = createEffector();
    popDirectory();
    if (g_useAudiencePanel == 1)
        f790 = createAudienceRatePanel();
    f78c = createEffector6thStyle();
    ((Obj41D1C0 *)f78c)->set((int)((char *)this + 0x18));
    return 1;
}
