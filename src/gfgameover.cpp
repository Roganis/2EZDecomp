/* GFMainGameDirector::m45d740 @0x45d740 - is the song over for
   everyone, the GF twin of EZ2DJMainGameDirector::m4204b0: under
   UseGameOver, walk list C counting players whose run is over
   (PlayerRecord::m465ed0); more than one of two publishes g_1b2ea28,
   a lone finished player just answers yes (m465ed0 published for it).

   Its own TU, same reasoning as src/ez2djgameover.cpp. */
#include "ezscreen.h"
#include "players.h"

extern int nextListC(ListNode **cur);       /* 0x418470 (src/nodelists.cpp) */
extern int countListC();                    /* 0x4184a0 (src/nodelists.cpp) */
extern int g_useGameOver;                   /* 0x1b2e560 (src/ez2acapp.cpp) */
extern int g_1b2ea28;                       /* src/moderunner.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0045d740
int GFMainGameDirector::m45d740()
{
    ListNode *cur;
    int       n;

    if (f20 >= 3)
        return 0;
    n = 0;
    if (g_useGameOver != 0)
    {
        cur = 0;
        if (nextListC(&cur) != 0)
        {
            do
            {
                if (((PlayerRecord *)cur)->m465ed0() != 0)
                    n++;
            }
            while (nextListC(&cur) != 0);
        }
        if (countListC() > 1)
        {
            if (n > 1)
            {
                g_1b2ea28 = 1;
                return 1;
            }
        }
        else if (n == 1)
            return 1;
    }
    return 0;
}
