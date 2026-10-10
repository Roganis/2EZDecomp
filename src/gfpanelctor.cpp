/* GFPanel::GFPanel @0x462870 - the scratch modes' panel constructor, and the
   sibling of Panel::Panel @0x429b70 (src/gamepanels.cpp), which matches.

   Clear the counters, park the two chart members, set the screen size, fill
   the forty item handles with -1, zero the four blocks, and load the mode's
   "newdj.wav" out of System\BattleMode\ with the working directory saved and
   restored around it.

   96.18%, and every instruction is right except TWO, both in the float block:

       ORIGINAL                       OURS
       or   eax,0xffffffff            xorps xmm0,xmm0
       xorps xmm0,xmm0                4 zero stores
       4 zero stores                  or   eax,0xffffffff
       movss xmm1,[640]               movss xmm0,[640]
       movss [esi+0xe8],xmm1          movss [esi+0xe8],xmm0
       movss xmm1,[480]               movss xmm0,[480]
       movss [esi+0xec],xmm1          movss [esi+0xec],xmm0

   i.e. `or eax,-1` is scheduled one slot late and the two constants reuse
   xmm0 where the original takes xmm1. Both are allocator choices: xmm0 is
   provably dead after the fourth zero store (the whole function has no other
   xmm use), so cl coalesces and the original did not.

   WHAT IS LOAD-BEARING AND MUST NOT BE UNDONE:
   - The three float PAIRS go through an inlined two-argument helper. Written
     as six plain assignments cl hoists the whole `xorps` block to the top of
     the function - worth ~7%, and it is the difference between the float
     block sitting after the integer stores (right) and before them (wrong).
   - buf is char[256], not 260: the frame is 0x108 and `this` plus the cookie
     take eight of it. The original still passes 260 to GetCurrentDirectoryA.
   - The zeros come before the constants in SOURCE order, which is what the
     float-register rule in CLAUDE.md says decides xmm0/xmm1 - it is just not
     sufficient here.

   SWEPT, ALL BYTE-IDENTICAL TO THE FORM BELOW (six forms, 2026-08-23, after
   ten earlier ones): the zeros as plain assignments with the constants still
   paired; the constants plain with the zeros paired; a named `int minus = -1`
   declared before the float block, at the top of the function, or both;
   and those combined with the plain zeros. cl folds the named constant back
   every time. Earlier sweeps also ruled out constants-before-zeros, moving
   the -1 loop above the float block, a named float zero, one-value helpers,
   and down-counting loop forms.

   The lever, if there is one, is whatever keeps xmm0 reserved across the two
   constant loads. In Panel::Panel the same two constants DO land in xmm1,
   there because that function's zero is genuinely still live past them. */
#include <windows.h>
#include <string.h>
#include "gamepanels.h"
#include "ez2acapp.h"

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 (src/strutil.cpp) */
extern void popDirectory();                        /* 0x412e60 (src/strutil.cpp) */

static __forceinline void setPair(float *p, float a, float b)
{
    p[0] = a;
    p[1] = b;
}

// FUNCTION: EZ2AC_UNPACKED 0x00462870
GFPanel::GFPanel()
{
    char buf[256];
    int i;

    ff8 = 0;
    f1acc = 0;
    be4 = 0;
    f18 = 0;
    f34 = 4;
    f2c = 1;
    f30 = 0;
    setPair(&f1ac4, 0.0f, 0.0f);
    setPair(&ff0, 0.0f, 0.0f);
    setPair(&fe8, 640.0f, 480.0f);
    for (i = 0; i < 40; i++)
    {
        f38[i] = -1;
    }
    memset(items, 0, sizeof(items));
    memset(s1af4, 0, sizeof(s1af4));
    memset(f3030, 0, sizeof(f3030));
    memset(f3048, 0, sizeof(f3048));
    GetCurrentDirectoryA(260, buf);
    popDirectory();
    /* The literal is already annotated by its other user in src/panels.cpp;
       the entity is keyed by address, so one marker is enough. */
    changeDir("System\\BattleMode\\", 1);
    f10 = 0;
    b34d4 = 0;
    // STRING: EZ2AC_UNPACKED 0x00490188
    m_sample = (void *)g_sub1b2eae4->createSample("newdj.wav", 0, 0);
    popDirectory();
    changeDir(buf, 1);
}
