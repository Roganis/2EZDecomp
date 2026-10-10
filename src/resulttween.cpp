/* ResultDirector::m450b50 @0x450b50 and GFResultDirector::m4672f0 @0x4672f0 -
   tick the two result charts, by phase.

   Byte-identical twins; every member offset they touch sits 0x348 apart
   between the two classes, the same shift as the score transfer pair.

   Before a player reaches phase 5 its chart is ticked from the loop, and
   g_1b2eb60 naming that player excludes it. From phase 5 on the tick moves
   out of the loop and behind a list-A membership test, and the two players
   are not symmetric there: player 0 keeps ticking for every phase at or past
   5, player 1 only while it is exactly 5. Both post-phase ticks are skipped
   The counter is REUSED after the loop rather than compared against a
   literal 2: the two post-phase tests are written `i != g_1b2eb60` as well,
   which is what puts the global on the right of the compare (`cmp esi,[g]`,
   six bytes) instead of an immediate compare against 2 (seven). Written the
   other way round every branch displacement in the function shifts by one. */

#include "directors.h"
#include "obj402860.h"

/* Find the list-A node with this id, or 0 (src/nodelists.cpp). */
struct ListNode *findListA(int key);

extern int g_1b2eb60;   /* 0x1b2eb60 (src/gamestate.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00450b50
void ResultDirector::m450b50()
{
    int i;

    for (i = 0; i < 2; i++)
    {
        if (m_state[i] < 5 && i != g_1b2eb60)
            m_2ca4[i].m403940();
    }

    if (findListA(0) != 0 && m_state[0] >= 5 && i != g_1b2eb60)
    {
        m_2ca4[0].m403940();
        return;
    }

    if (findListA(1) != 0 && m_state[1] == 5 && i != g_1b2eb60)
        m_2ca4[1].m403940();
}

// FUNCTION: EZ2AC_UNPACKED 0x004672f0
void GFResultDirector::m4672f0()
{
    int i;

    for (i = 0; i < 2; i++)
    {
        if (m_state[i] < 5 && i != g_1b2eb60)
            m_295c[i].m403940();
    }

    if (findListA(0) != 0 && m_state[0] >= 5 && i != g_1b2eb60)
    {
        m_295c[0].m403940();
        return;
    }

    if (findListA(1) != 0 && m_state[1] == 5 && i != g_1b2eb60)
        m_295c[1].m403940();
}
