#include <stdio.h>
#include <string.h>
#include "players.h"
#include "gamepanels.h"
#include "loader.h"      /* Log */

/* PlayerSlot::m42e470 @0x42e470 - build the keys player's panel (the
   TrackSink the rest of the player talks to): change into the mode's Panel
   directory (the CV2 one under the CV2 flag), pick the panel slot (0 when the
   owner's +0x37c says every player shares one, else the player's id), create
   a Panel, point it back at this slot, load "<stylePrefix><slot>.pvi" (logged
   either way), init it on the slot, mark it the first player's when the first
   list-A node is this player, and pop the directory. Its own TU: createPanel
   and LoadPVI must not be inlined. */
extern int  g_1b2eb6c;             /* src/ez2acapp.cpp - the CV2 flag */
extern char g_styleTag[36];        /* 0x1b2e5ac (src/maingame.cpp) */
extern char g_stylePrefix[64];     /* 0x1b2e5d0 */
extern char g_modeName[256];       /* 0x1b2e680 (src/modename.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                         /* 0x412e60 */
extern Panel *createPanel();                        /* 0x42d560 */
extern int nextListA(ListNode **cur);               /* 0x4182e0 */

// FUNCTION: EZ2AC_UNPACKED 0x0042e470
int PlayerSlot::m42e470()
{
    char dir[0x104];
    ListNode *p;
    int slot;

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049121c
        sprintf(dir, "System\\CV2Mix\\%s\\Panel", g_styleTag);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048f840
        sprintf(dir, "System\\%s\\Panel", g_modeName);
    }
    changeDir(dir, 1);
    if (f3d0->f37c == 1)
        slot = 0;
    else
        slot = id;
    f3cc = (TrackSink *)createPanel();
    ((Panel *)f3cc)->slot = (PanelSlot *)this;
    // STRING: EZ2AC_UNPACKED 0x00491210
    sprintf(dir, "%s%d.pvi", g_stylePrefix, slot);
    // STRING: EZ2AC_UNPACKED 0x004911fc
    Log("Player Slot = %d", slot);
    if (((Panel *)f3cc)->LoadPVI(dir) == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004911e4
        Log("Player: Can't PVI '%s'", dir);
    }
    ((Panel *)f3cc)->init((PanelSlot *)this);
    p = 0;
    nextListA(&p);
    if (p->f8 == id)
        ((Panel *)f3cc)->f24 = 1;
    popDirectory();
    return 1;
}

/* CatchPlayer::m472e60 @0x472e60 - the catch player's panel: the CV2 panel
   directory or the mode's, a CatchPanel pointed back at this player, the
   .pvi by name - under CV2 one of two fixed files, the second for the two
   special songs - else "<stylePrefix>.pvi"; loaded (logged on failure),
   initialised on the player, marked first when the first list-B node is
   this player. */
extern CatchPanel *createCatchPanel();             /* 0x472490 */
extern int nextListB(ListNode **cur);              /* 0x4183b0 */
extern char g_songShortName[0x40];                 /* 0x1b2f004 (src/catchpanelgds.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00472e60
int CatchPlayer::m472e60()
{
    char dir[0x104];
    ListNode *p;

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049e480
        sprintf(dir, "System\\CV2MIx\\EZ2Catch\\Panel");
    }
    else
    {
        sprintf(dir, "System\\%s\\Panel", g_modeName);
    }
    changeDir(dir, 1);
    f35c = createCatchPanel();
    f35c->slot = (PanelSlot *)this;
    if (g_1b2eb6c != 0)
    {
        /* both titles are STRING-pinned by src/catchpanelgds.cpp */
        if (_stricmp(g_songShortName, "8ashtray-ca1") != 0 &&
            _stricmp(g_songShortName, "therhythm-ca1") != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0049e468
            sprintf(dir, "STYLE_EZ2Catch1_0.pvi");
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0049e450
            sprintf(dir, "STYLE_EZ2Catch3_0.pvi");
        }
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049e448
        sprintf(dir, "%s.pvi", g_stylePrefix);
    }
    if (f35c->LoadPVI(dir) == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049e42c
        Log("CatchPlayer: Can't PVI '%s'", dir);
    }
    f35c->init((PanelSlot *)this);
    p = 0;
    nextListB(&p);
    if (p->f8 == id)
        f35c->f24c = 1;
    popDirectory();
    return 1;
}

/* PlayerRecord::m465d90 @0x465d90 - the scratch player's panel, the keys
   shape with the file name in its own buffer: the CV2 directory or the
   mode's, a GFPanel pointed back at this player, "<stylePrefix><slot>.pvi"
   or the fixed CV2 name with the slot, loaded (logged on failure),
   initialised, marked first when the first list-C node is this player. */
extern GFPanel *createGFPanel();                   /* 0x465320 */
extern int nextListC(ListNode **cur);              /* 0x418470 */

// FUNCTION: EZ2AC_UNPACKED 0x00465d90
int PlayerRecord::m465d90()
{
    char name[0x104];
    char dir[0x104];
    ListNode *p;
    int slot;

    if (g_1b2eb6c == 0)
    {
        sprintf(dir, "System\\%s\\Panel", g_modeName);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049a6bc
        sprintf(dir, "System\\CV2Mix\\ScratchMix\\Panel");
    }
    changeDir(dir, 1);
    if (f1e0->f33c == 1)
        slot = 0;
    else
        slot = id;
    f1dc = (GFTrackSink *)createGFPanel();
    ((GFPanel *)f1dc)->slot = (PanelSlot *)this;
    if (g_1b2eb6c == 0)
    {
        sprintf(name, "%s%d.pvi", g_stylePrefix, slot);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049a6a0
        sprintf(name, "STYLE_ScratchMix1_%d.pvi", slot);
    }
    if (((GFPanel *)f1dc)->LoadPVI(name) == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049a684
        Log("GFPlayer: Can't PVI '%s'", name);
    }
    ((GFPanel *)f1dc)->init((PanelSlot *)this);
    p = 0;
    nextListC(&p);
    if (p->f8 == id)
        ((GFPanel *)f1dc)->f20 = 1;
    popDirectory();
    return 1;
}

extern int   countListC();                  /* 0x4184a0 (src/nodelists.cpp) */
extern float g_playerScale[2];              /* 0x1b2ef20 (src/gamepanels.cpp) */
extern int   g_elemCount;                   /* 0x1b2e718 */
extern int   g_1b2ea28;                     /* src/moderunner.cpp */

/* Is this player's run over - the GF twin of PlayerSlot::m42e590, with
   the win published to g_1b2ea28 here rather than in the caller. */
// FUNCTION: EZ2AC_UNPACKED 0x00465ed0
int PlayerRecord::m465ed0()
{
    float v;

    if (f1e0->f750 == 0)
    {
        if (countListC() >= 2)
        {
            if (findPlayer(0)->f38 == 0.0f)
                goto win;
            v = findPlayer(1)->f38;
        }
        else
            v = f38;
        if (v != 0.0f)
            return 0;
win:
        g_1b2ea28 = 1;
win2:
        return 1;
    }
    if (f1e0->f750 != 1)
        return 0;
    /* Negated: the original jumps to win2 on `jae` and falls into its own
       `return 0` (unordered counts as not-won). */
    if (!(g_playerScale[g_elemCount] >= f54))
        return 0;
    goto win2;
}
