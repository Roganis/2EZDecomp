/* EZ2AC::runRankingScreen / runModeIntro / runGameOver / runResult /
   runGFResult, in their own TU. Beside the mode runners in
   src/moderunner.cpp, cl inlined them at EVERY call site; the original's
   compiler inlined only some sites and CALLED the rest (0x4160e0 and its
   siblings `call 0x415fe0` / `call 0x415d70` at some sites and carry the
   new/run/delete block at others). So the helpers live out of sight here,
   and each runner writes the three statements out at exactly the sites
   the original inlined - that took the four RadioMix runners to 100%. */
#include <stdio.h>
#include <string.h>
#include "kcrt.h" /* operator new/delete == an inlined malloc/free */
#include "ezscreen.h"
#include "ez2acapp.h" /* g_iniProfile, g_1b2eb6c */
#include "kparser.h"

// FUNCTION: EZ2AC_UNPACKED 0x00415fe0
int EZ2AC::runRankingScreen()
{
    Screen48D3F0 *ranking = new Screen48D3F0;
    ranking->run(g_iniProfile);
    delete ranking;
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00416020
int EZ2AC::runModeIntro()
{
    DrawModeIntro *intro = new DrawModeIntro;
    int code = intro->run(g_iniProfile);

    delete intro;
    return code;
}

/* Game over - five runners, gated by runGame's "UseGameOver" setting.
   A MEMBER, despite never touching `this`: all five of the original's call
   sites load ecx first (0x416b26, 0x416d16, 0x417a21, 0x417d0c, 0x417e45), and
   a __thiscall member that ignores `this` is byte-identical to a free
   function, so only a reconstructed call site can tell. runStreetMix
   @0x4168c0 is the first one, and it wanted the `mov ecx,ebp`. */
// FUNCTION: EZ2AC_UNPACKED 0x00415d70
int EZ2AC::runGameOver()
{
    Screen48D32C *gameOver = new Screen48D32C;
    int code = gameOver->run(g_iniProfile);
    delete gameOver;
    return code;
}

/* The result screen - six runners (Ruby, Radio, 5Radio, 10Radio, 14Radio, Space). */
// FUNCTION: EZ2AC_UNPACKED 0x00415ec0
int EZ2AC::runResult()
{
    Screen48D380 *result = new Screen48D380;
    int code = result->run(g_iniProfile);
    delete result;
    return code;
}

/* The GF result screen - ScratchMix only. */
// FUNCTION: EZ2AC_UNPACKED 0x004162e0
int EZ2AC::runGFResult()
{
    Screen48D498 *result = new Screen48D498;
    int code = result->run(g_iniProfile);
    delete result;
    return code;
}
