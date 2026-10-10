/* InGameSongSelect::update @0x4261d0 - the in-game song-select tick.

   While there is another element to come and this slot's list is non-empty,
   the overlay is opened, the selection advanced, and one of the two
   difficulty tables refreshed - the first player's when the list is his,
   the second's otherwise. Then the two other tables are stepped, or reset,
   according to g_1b2ead8.

   NOTE the asymmetry, which is in the original: the first-player arm sets
   the FIRST table's +0x214 to 5 and its +0x218 to zero, and the other arm
   refreshes the SECOND table but zeroes the FIRST one's same two fields.
   Both write m_39130's pair; only the refreshed table differs. */

#include "directors.h"
#include "obj402860.h"
#include "ez2acapp.h"
#include "players.h"

/* The graphics singleton under the name its matched wrapper uses. */
struct Obj40BB00 { void doOp7(int v); };   /* 0x40a470 */

/* The director that built this overlay, viewed for the one count it holds. */
struct IgsOwner { char _pad0[0x7c8]; int total; };   /* +0x7c8 */

extern int g_elemCount;      /* 0x1b2e718 (src/loops.cpp) */

// GLOBAL: EZ2AC_UNPACKED 0x01b2ead8
int g_1b2ead8;

extern int getListAF8();     /* 0x418340 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004261d0
void InGameSongSelect::update()
{
    int n;

    n = g_elemCount;
    if (n + 1 < ((IgsOwner *)f14)->total && m_count[n] != 0)
    {
        ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
        m425b40();

        if (getListAF8() == 0)
        {
            /* Held in a local across the refresh - the original keeps
               &m_39130 in a saved register and addresses both stores off
               it, where the other arm addresses off `this`. */
            Obj402860 *t = &m_39130;
            t->m403940();
            t->f214 = 5.0f;
            t->f218 = 0.0f;
        }
        else
        {
            m_3934c.m403940();
            m_39130.f214 = 0.0f;
            m_39130.f218 = 0.0f;
        }

        ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
    }

    if (g_1b2ead8 == 1)
    {
        m_39568.m403940();
        m_39784.m403940();
    }
    else if (g_1b2ead8 == 0)
    {
        m_39568.reset();
        m_39784.reset();
    }
}


/* InGameSongSelect::update2 @0x4262a0 - the overlay's tick.

   The list-A player is looked up TWICE, which is in the original: once to
   read its progress and once to run the course result on it. Then the same
   guard update() uses drives this screen own advance. */


extern ListNode *findListA(int key);   /* 0x418e30 (src/nodelists.cpp) */
extern int g_1b2eb28;                  /* 0x1b2eb28 (src/gamestate.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004262a0
void InGameSongSelect::update2(int /*a*/, int /*b*/)
{
    int key = getListAF8();
    int n;

    if (((PlayerSlot *)findListA(key))->f1e0 >= 0x3e8
        && g_elemCount >= 2
        && g_1b2eb28 <= 0)
    {
        ((PlayerSlot *)findListA(key))->m42e2e0();
    }

    n = g_elemCount;

    if (n + 1 < ((IgsOwner *)f14)->total && m_count[n] != 0)
    {
        m425ea0();
    }
}
