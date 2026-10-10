/* EZ2DJMainGameDirector::m420a60 @0x420a60 - build the keys game's
   background, and CatchMainGameDirector::m46c4d0 @0x46c4d0's twin
   (src/catchbgload.cpp). Read that function's header for the shape; the
   deltas are:

     - there is one MORE fallback: when g_1b2ef40 is down the per-song
       directory is abandoned and BG\BlackBG is loaded instead, which the
       catch copy simply skips.

   The first load attempt passes the name buffer UNINITIALIZED here too.

   93.70%, and the residue is one merge choice. The original SHARES the
   final load attempt between the shared-background arm and the 7Loki arm -
   one `mov ecx,[esi+0x374]; call; test` block that the first jumps to and
   the second falls into - while giving the third arm its own copy. cl
   merges all three instead, which costs four instructions and, because the
   function then ends two bytes short, an alignment nop before the jump
   table that unaligns every table entry in the diff. Writing the call as a
   statement in the first two arms and as the third arm's `else if`
   CONDITION - which is what the catch twin does - does not separate them.
   The branch displacements are all downstream of the same four bytes. */

#include <stdio.h>
#include "ezscreen.h"
#include "panels.h"
#include "backgrounds.h"
#include "kprofile.h"

/* The background under the one field this function writes. */
struct BgOwnerDj { char _pad0[0x18]; void *owner; };   /* +0x18 - the catch
                                                          copy's is at +0x10 */

extern KProfile *g_iniProfile;      /* 0x530b64 (src/kengineapp.cpp) */
extern char  g_songInfoName[64];    /* 0x1b2eb80 (src/panels.cpp) */
extern int   g_1b2eb6c;             /* src/ez2acapp.cpp */
extern int   g_1b2ef40;             /* src/moderunner.cpp */
extern int   g_1b2eba0;             /* src/maingame.cpp */
extern int   g_1b2ea14;             /* src/panels.cpp */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern void Log(const char *fmt, ...);             /* 0x402880 */
extern BackGround *createBackGround();             /* 0x41bb30 */
extern GFBackGround *createGFBackGround();         /* 0x45cf70 */
extern char  g_textSlots[32][128];  /* 0x1b2ec18 (src/modename.cpp) */
extern int   g_elemCount;           /* 0x1b2e718 (src/loops.cpp) */

/* 93.70%, and THREE instructions short - not the fifteen it appears to be if
   the function's end is taken as the first int3, because this one's switch
   puts its jump TABLE in .text right after the `ret` and the table
   disassembles as plausible garbage. scratchpad/za.py now reads the table's
   address out of the indirect jump and stops there.

   What is actually missing is one shared `m41b900(name)` call. The original
   merges the commonbg arm's and the 7Loki arm's - the commonbg arm pushes
   &name and JUMPS into the 7Loki arm's copy - and keeps the third arm's
   separate; cl merges all three into one. Four shapes of that third arm were
   swept (the `else if` on the call as written, a named result, a plain else
   block, and a named BackGround pointer, the last of which is 68.8%) and
   none of them separates the merge. It is the shared-block attachment the
   cookbook calls unsteerable, at three instructions. */
// FUNCTION: EZ2AC_UNPACKED 0x00420a60
int EZ2DJMainGameDirector::m420a60()
{
    int  use;
    char path[260];
    char name[260];
    BackGround *bg;

    bg = createBackGround();
    timed = (MainTimed *)bg;
    ((BgOwnerDj *)bg)->owner = this;

    g_iniProfile->getInt("UseBackground", &use);
    if (use == 0)
        goto done;

    if (g_1b2eb6c == 0)
    {
        sprintf(path, "BG\\%s", m4208f0());
    }
    else
    {
        sprintf(path, "BG\\%s", g_songInfoName);
    }
    changeDir(path, 1);

    if (g_1b2ef40 != 0)
    {
    if (((BackGround *)timed)->m41b900(name) != 0)
        goto popdone;

    if (g_1b2eb6c != 0)
    {
        sprintf(name, "%s.scr", g_songInfoName);
    }
    else
    {
        sprintf(name, "%s.scr", m4208f0());
    }

    if (((BackGround *)timed)->m41b900(name) != 0)
        goto popdone;

    popDirectory();

    if (g_1b2eb6c == 0)
    {
        switch (g_1b2eba0)
        {
        case 7:
            sprintf(path, "BG\\commonbg7");
            break;
        case 8:
            sprintf(path, "BG\\commonbg8");
            break;
        case 9:
            break;
        case 10:
            sprintf(path, "BG\\commonbg9");
            break;
        case 11:
            sprintf(path, "BG\\commonbg9");
            break;
        case 12:
            sprintf(path, "BG\\commonbg11");
            break;
        case 13:
            sprintf(path, "BG\\commonbg12");
            break;
        case 14:
            sprintf(path, "BG\\commonbg13");
            break;
        }

        changeDir(path, 1);
        sprintf(name, "%s.scr", "commonbg");
        if (((BackGround *)timed)->m41b900(name) != 0)
            goto popdone;
    }
    else if (g_1b2ea14 == 1)
    {
        sprintf(path, "BG\\7Loki");
        changeDir(path, 1);
        sprintf(name, "%s.scr", "7Loki");
        /* The opposite polarity from the other arms (`je <fail>; jmp
           <done>` at 0x420c5b) - which is also what keeps cl from merging
           this call with the next arm's identical one. */
        if (((BackGround *)timed)->m41b900(name) == 0)
            goto fail;
        goto popdone;
    }
    else if (((BackGround *)timed)->m41b900(name) != 0)
    {
        goto popdone;
    }

fail:
    Log("Backgournd: Load Failure: %s", name);
    popDirectory();
    return 0;
    }

    /* No per-song background: fall back on the black one. The catch copy has
       no equivalent - it simply gives up here. Written as the tail of the
       function rather than a `goto` target, so cl lays the main path inline
       and this out of line, which is where the original has it. */
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x0048f774
    sprintf(path, "BG\\BlackBG");
    changeDir(path, 1);
    sprintf(name, "%s.scr",
            // STRING: EZ2AC_UNPACKED 0x0048f76c
            "BlackBG");
    ((BackGround *)timed)->m41b900(name);
popdone:
    popDirectory();
done:
    return 1;
}

/* GFMainGameDirector::m45db70 @0x45db70 - the scratch game's copy of the
   loader above, and the third of the family. Every difference is an address
   except one: the scratch copy SEEDS the name buffer from the current text
   slot before the first attempt, where the keys and catch copies pass it
   uninitialized. The style numbers and the BlackBG fallback are the keys
   copy's, not the catch copy's.

   The literals are all pinned by the two functions above - one STRING marker
   per address, and a second is dropped with an error. */
// FUNCTION: EZ2AC_UNPACKED 0x0045db70
int GFMainGameDirector::m45db70()
{
    int  use;
    char path[260];
    char name[260];
    GFBackGround *bg;

    bg = createGFBackGround();
    timed = bg;
    ((BgOwnerDj *)bg)->owner = this;

    g_iniProfile->getInt("UseBackground", &use);
    if (use == 0)
        goto done;

    if (g_1b2eb6c == 0)
    {
        sprintf(path, "BG\\%s", m45da00());
    }
    else
    {
        sprintf(path, "BG\\%s", g_songInfoName);
    }
    changeDir(path, 1);

    /* The keys and catch copies leave `name` uninitialized for the first
       attempt; the scratch copy seeds it from the current text slot. */
    sprintf(name, "%s.scr", g_textSlots[g_elemCount]);

    if (g_1b2ef40 != 0)
    {
    if (((GFBackGround *)timed)->m45cd40(name) != 0)
        goto popdone;

    if (g_1b2eb6c != 0)
    {
        sprintf(name, "%s.scr", g_songInfoName);
    }
    else
    {
        sprintf(name, "%s.scr", m45da00());
    }

    if (((GFBackGround *)timed)->m45cd40(name) != 0)
        goto popdone;

    popDirectory();

    if (g_1b2eb6c == 0)
    {
        switch (g_1b2eba0)
        {
        case 7:
            sprintf(path, "BG\\commonbg7");
            break;
        case 8:
            sprintf(path, "BG\\commonbg8");
            break;
        case 9:
            break;
        case 10:
            sprintf(path, "BG\\commonbg9");
            break;
        case 11:
            sprintf(path, "BG\\commonbg9");
            break;
        case 12:
            sprintf(path, "BG\\commonbg11");
            break;
        case 13:
            sprintf(path, "BG\\commonbg12");
            break;
        case 14:
            sprintf(path, "BG\\commonbg13");
            break;
        }

        changeDir(path, 1);
        sprintf(name, "%s.scr", "commonbg");
        if (((GFBackGround *)timed)->m45cd40(name) != 0)
            goto popdone;
    }
    else if (g_1b2ea14 == 1)
    {
        sprintf(path, "BG\\7Loki");
        changeDir(path, 1);
        sprintf(name, "%s.scr", "7Loki");
        /* Opposite polarity from the other arms, as in m420a60. */
        if (((GFBackGround *)timed)->m45cd40(name) == 0)
            goto fail;
        goto popdone;
    }
    else if (((GFBackGround *)timed)->m45cd40(name) != 0)
    {
        goto popdone;
    }

fail:
    Log("Backgournd: Load Failure: %s", name);
    popDirectory();
    return 0;
    }

    /* No per-song background: fall back on the black one. The catch copy has
       no equivalent - it simply gives up here. Written as the tail of the
       function rather than a `goto` target, so cl lays the main path inline
       and this out of line, which is where the original has it. */
    popDirectory();
    sprintf(path, "BG\\BlackBG");
    changeDir(path, 1);
    sprintf(name, "%s.scr",
            "BlackBG");
    ((GFBackGround *)timed)->m45cd40(name);
popdone:
    popDirectory();
done:
    return 1;
}
