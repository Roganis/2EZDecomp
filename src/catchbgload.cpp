/* CatchMainGameDirector::m46c4d0 @0x46c4d0 - build the Ez2Catch background.

   Make the CatchBackGround, point it back at this director, and give up
   early unless the .ini says UseBackground. Then chdir into BG\<song> and
   try three names in turn through the background's own loader: whatever the
   buffer already holds, then "<song>.scr", then a fallback. Outside a CV2
   run the fallback is one of six shared backgrounds chosen by the style at
   g_1b2eba0; inside one it is 7Loki when g_1b2ea14 says so, and otherwise
   the same buffer again. Anything that loads returns 1; falling off the end
   logs the failure (with the original's spelling) and returns 0.

   The FIRST load attempt passes the name buffer UNINITIALIZED - the
   original does `lea ecx,[esp+0x10c]; call m46bc30` before anything has
   written there. That is in the bytes, not a misreading, and it is
   reproduced rather than tidied. */

#include <stdio.h>
#include "ezscreen.h"
#include "backgrounds.h"
#include "kprofile.h"

/* The background under the one field this function writes. */
struct BgOwner { char _pad0[0x10]; void *owner; };   /* +0x10 */

extern KProfile *g_iniProfile;      /* 0x530b64 (src/kengineapp.cpp) */
extern char  g_songInfoName[64];    /* 0x1b2eb80 (src/panels.cpp) */
extern int   g_1b2eb6c;             /* src/ez2acapp.cpp */
extern int   g_1b2ef40;             /* src/... */
extern int   g_1b2eba0;             /* src/... */
extern int   g_1b2ea14;             /* src/panels.cpp */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern void Log(const char *fmt, ...);             /* 0x402880 */
extern CatchBackGround *createCatchBackGround();   /* 0x46be20 */

// FUNCTION: EZ2AC_UNPACKED 0x0046c4d0
int CatchMainGameDirector::m46c4d0()
{
    int  use;
    char path[260];
    char name[260];
    CatchBackGround *bg;

    bg = createCatchBackGround();
    timed = (CatchTimed *)bg;
    ((BgOwner *)bg)->owner = this;

    // STRING: EZ2AC_UNPACKED 0x0048f830
    g_iniProfile->getInt("UseBackground", &use);
    if (use == 0)
        goto done;

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048f828
        sprintf(path, "BG\\%s", m46c360());
    }
    else
    {
        sprintf(path, "BG\\%s", g_songInfoName);
    }
    changeDir(path, 1);

    if (g_1b2ef40 == 0)
        goto popdone;
    if (((CatchBackGround *)timed)->m46bc30(name) != 0)
        goto popdone;

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048f820
        sprintf(name, "%s.scr", g_songInfoName);
    }
    else
    {
        sprintf(name, "%s.scr", m46c360());
    }

    if (((CatchBackGround *)timed)->m46bc30(name) != 0)
        goto popdone;

    popDirectory();

    if (g_1b2eb6c == 0)
    {
        switch (g_1b2eba0)
        {
        case 7:
            // STRING: EZ2AC_UNPACKED 0x0048f810
            sprintf(path, "BG\\commonbg7");
            break;
        case 8:
            // STRING: EZ2AC_UNPACKED 0x0048f800
            sprintf(path, "BG\\commonbg8");
            break;
        case 9:
            break;
        case 10:
            // STRING: EZ2AC_UNPACKED 0x0048f7f0
            sprintf(path, "BG\\commonbg9");
            break;
        case 11:
            sprintf(path, "BG\\commonbg9");
            break;
        case 12:
            // STRING: EZ2AC_UNPACKED 0x0048f7e0
            sprintf(path, "BG\\commonbg11");
            break;
        case 13:
            // STRING: EZ2AC_UNPACKED 0x0048f7d0
            sprintf(path, "BG\\commonbg12");
            break;
        case 14:
            // STRING: EZ2AC_UNPACKED 0x0048f7c0
            sprintf(path, "BG\\commonbg13");
            break;
        }

        changeDir(path, 1);
        /* Two literals, so the marker has to sit on the line that begins
           with the one it names - reccmp reads the string off the FOLLOWING
           line and would otherwise test "%s.scr" against these bytes. */
        sprintf(name, "%s.scr",
                // STRING: EZ2AC_UNPACKED 0x0048f7b4
                "commonbg");
        if (((CatchBackGround *)timed)->m46bc30(name) != 0)
            goto popdone;
    }
    else if (g_1b2ea14 == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x0048f7a8
        sprintf(path, "BG\\7Loki");
        changeDir(path, 1);
        sprintf(name, "%s.scr",
                // STRING: EZ2AC_UNPACKED 0x0048f7a0
                "7Loki");
        if (((CatchBackGround *)timed)->m46bc30(name) != 0)
            goto popdone;
    }
    else if (((CatchBackGround *)timed)->m46bc30(name) != 0)
    {
        goto popdone;
    }

    // STRING: EZ2AC_UNPACKED 0x0048f780
    Log("Backgournd: Load Failure: %s", name);
    popDirectory();
    return 0;

popdone:
    popDirectory();
done:
    return 1;
}
