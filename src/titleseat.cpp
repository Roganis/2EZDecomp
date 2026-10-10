/* TitleDirector::m44e680 @0x44e680 - seat the player in `slot`.

   Two paths. With m_1654 at 2 the screen is taking a SECOND player: mark
   the busy flag, refuse if that player is already seated or the coin
   mechanism has nothing to consume, seat him, and - if the follow-up check
   agrees and the third stoppable exists - hand it a zero in TAIL position,
   which is why the original overwrites its own argument slot and jumps
   through the vtable rather than calling.

   Otherwise it is the FIRST player: the same refusals against list A,
   then the attract loop is handed over - note the first stoppable is
   poked WITHOUT a null check here and re-poked WITH one three
   instructions later, which is in the original and is left as it is. */

#include "ezscreen.h"
#include "bigbase.h"     /* Obj40C180, g_bigInstance */

extern int g_1b2eadc;        /* src/ez2acapp.cpp */

extern struct PlayerRecord *findPlayer(int id);   /* 0x418f30 */
extern struct PlayerRecord *addPlayer(int id);    /* 0x418410 */
extern struct ListNode *findListA(int key);       /* 0x418e30 */
extern struct ListNode *addToListA(int id);       /* 0x418260 */

// FUNCTION: EZ2AC_UNPACKED 0x0044e680
void TitleDirector::m44e680(int slot)
{
    if (m_1654 == 2)
    {
        g_1b2eadc = 2;
        if (findPlayer(slot) != 0)
            return;
        if (g_bigInstance->consume() == 0)
            return;

        addPlayer(slot);
        m_1624 = 1;
        if (m44dec0() != 1)
            return;
        if (m_14e8 == 0)
            return;

        /* TAIL position - the original overwrites its own argument slot
           and jumps through the vtable rather than calling, which only
           happens when this function returns void as the callee does. */
        m_14e8->s4(0);
        return;
    }

    g_1b2eadc = 0;
    if (findListA(slot) != 0)
        return;
    if (g_bigInstance->consume() == 0)
        return;

    m_attract = 1;
    m_14e0->s5();
    addToListA(slot);

    if (m_14e0 != 0)
        m_14e0->s5();
    if (m_14e4 != 0)
        m_14e4->s5();
    m_14e8->s4(0);
    m_1624 = 1;
    return;
}
