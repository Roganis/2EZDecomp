/* EZ2DJMainGameDirector::m420d90 @0x420d90 - build the credit readout, place
   it by mode, and mark it for the single-player modes.

   The y it is placed at is 735 for the course modes and 435 otherwise, but
   WHICH list of modes decides that depends on g_1b2eb6c: with it at 1 the
   test is against the mix style (3, 4, 5 or 7), otherwise against the mode
   index (4, 5, 8 or 9). And g_1b2efa4 short-circuits the whole thing to 435.

   The placement is written as a CALL PER ARM, not as a y computed into a
   local: the original pushes the constant inside each arm and jumps to a
   shared `push -1.0f; call`, which is what cl's cross-jumping gives that
   shape - the 435 arm is the fall-through and the two 735 arms jump to it.
   A single call fed by a variable would push a register instead.

   The `this` for setPos is the factory's return value still in ecx, so it
   is a named local; the b10 store afterwards reads the MEMBER back, which
   is what the reload past countListA says. */

#include "ezscreen.h"

extern int g_1b2efa4;    /* 0x1b2efa4 (src/holdtick.cpp) */
extern int g_mixStyle;   /* 0x1b2ebc8 (src/panels.cpp) */
extern int g_1b2eb6c;    /* 0x1b2eb6c (src/ez2acapp.cpp) */
extern int g_modeIndex;  /* 0x1b2e548 (src/modename.cpp) */

ShowCredit *createShowCredit();   /* 0x450390 (src/misc.cpp) */
int         countListA();         /* 0x418310 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00420d90
int EZ2DJMainGameDirector::m420d90()
{
    ShowCredit *c = createShowCredit();

    credit = c;

    if (g_1b2efa4 == 0)
    {
        if (g_1b2eb6c == 1)
        {
            if (g_mixStyle == 3 || g_mixStyle == 4 || g_mixStyle == 5 ||
                g_mixStyle == 7)
                c->setPos(-1.0f, 735.0f);
            else
                c->setPos(-1.0f, 435.0f);
        }
        else
        {
            if (g_modeIndex == 4 || g_modeIndex == 5 || g_modeIndex == 8 ||
                g_modeIndex == 9)
                c->setPos(-1.0f, 735.0f);
            else
                c->setPos(-1.0f, 435.0f);
        }
    }
    else
    {
        c->setPos(-1.0f, 435.0f);
    }

    if (countListA() < 2 &&
        (g_modeIndex == 2 || g_modeIndex == 3 || g_modeIndex == 1))
        credit->b10 = 1;

    return 1;
}
