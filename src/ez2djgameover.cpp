/* EZ2DJMainGameDirector::m4204b0 @0x4204b0 - is the song over for
   everyone?  Only while the fade phase f20 is still below 3, and only
   with UseGameOver on: walk list A counting slots whose run is over
   (PlayerSlot::m42e590 - the gauge at zero, or survival's scale reached),
   and raise GAME OVER - g_1b2ea28 = 1 - when more than one of two players
   is dead, or the single player is. (Earlier comments here called this
   "the win"; it is the opposite. The result screen runs only while the
   latch is clear.)

   Its own TU: the walk callee lives in src/nodelists.cpp, the per-slot
   predicate in src/playerslot.cpp, and both callers (m424830 and the
   function @0x420570) elsewhere again, so nothing inlines. */
#include "ezscreen.h"
#include "players.h"

extern int nextListA(ListNode **cur);       /* 0x4182e0 (src/nodelists.cpp) */
extern int countListA();                    /* 0x418310 (src/nodelists.cpp) */
extern int g_useGameOver;                   /* 0x1b2e560 (src/ez2acapp.cpp) */
extern int g_1b2ea28;                       /* src/moderunner.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x004204b0
int EZ2DJMainGameDirector::m4204b0()
{
    ListNode *cur;
    int       n;

    if (f20 >= 3)
        return 0;
    n = 0;
    if (g_useGameOver != 0)
    {
        cur = 0;
        if (nextListA(&cur) != 0)
        {
            do
            {
                if (((PlayerSlot *)cur)->m42e590() != 0)
                    n++;
            }
            while (nextListA(&cur) != 0);
        }
        if (countListA() > 1)
        {
            if (n > 1)
            {
                g_1b2ea28 = 1;
                return 1;
            }
        }
        else if (n == 1)
        {
            g_1b2ea28 = 1;
            return 1;
        }
    }
    return 0;
}
