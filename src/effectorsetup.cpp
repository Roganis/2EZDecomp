/* GFMainGameDirector::m45de70 @0x45de70 and CatchMainGameDirector::m46c760
   @0x46c760 - build the two effector objects for the running mode.

   Both are the same function four bytes apart in the field pair they store
   into, which is why the original's linker left them unfolded.

   The directory dance is asymmetric and that is in the original: the mode's
   own System\<mode>\Panel is pushed, the plain Effector is constructed
   inside it, and the directory is popped BEFORE Effector6thStyle is built.
   Only the first one's constructor loads out of that path.

   The 6th-style object is then pointed back at the director's +0x18 through
   the one-word setter @0x41d1c0 - and through the pointer the factory just
   returned, not through the field that was written from it, which is why
   `this` can be killed by the `add esi,0x18` that forms the argument.

   In its own TU: src/maingame.cpp calls both, and the two factories live in
   src/panels.cpp.

   Both return 1 - nothing reads it, but the `mov eax,1` in front of the
   epilogue is what says so. */

#include <stdio.h>

#include "ezscreen.h"

/* The two pooled factories (src/panels.cpp). */
extern struct Effector         *createEffector();          /* 0x41d060 */
extern struct Effector6thStyle *createEffector6thStyle();  /* 0x41f860 */

/* src/strutil.cpp. */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */

extern char g_modeName[256];   /* 0x1b2e680 (src/modename.cpp) */

/* The one-word setter at +0x10 of whatever it is called on; src/accessors.cpp
   owns the matched body. */
struct Obj41D1C0 { char _pad[0x10]; int f; void set(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x0045de70
int GFMainGameDirector::m45de70()
{
    Effector6thStyle *e6;
    char path[260];

    /* The marker for this literal lives in src/playersink.cpp - one entity
       per address, and a second marker is dropped as a duplicate. */
    sprintf(path, "System\\%s\\Panel", g_modeName);
    changeDir(path, 1);

    m_effector = createEffector();
    popDirectory();

    e6 = createEffector6thStyle();
    m_effector6th = e6;
    ((Obj41D1C0 *)e6)->set((int)((char *)this + 0x18));
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046c760
int CatchMainGameDirector::m46c760()
{
    Effector6thStyle *e6;
    char path[260];

    sprintf(path, "System\\%s\\Panel", g_modeName);
    changeDir(path, 1);

    m_effector = createEffector();
    popDirectory();

    e6 = createEffector6thStyle();
    m_effector6th = e6;
    ((Obj41D1C0 *)e6)->set((int)((char *)this + 0x18));
    return 1;
}
