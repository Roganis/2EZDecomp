/* ResultDirector::update @0x451b80 - the keys result screen's per-frame draw.

   Order: pick the header plate, run the background video, blit the screen
   background, tick the always-on charts, draw the players and the stage
   number, then walk list A stamping each present player's rank plates into
   its own chart, then the conditional charts, and finally the fade overlay
   while the phase is 0 or 7.

   Two things a port needs out of it:

     - THE RANK PLATES COME IN TWO SETS OF FOUR, chosen by the same f10/f14/f18
       flags the tick uses, and BOTH are stamped into the same chart - cell
       0x2c from the first set, cell 0x2d from the second. The flags are tested
       f18, f14, f10 in that order with a fourth (neither) default, so they are
       a priority ladder rather than independent.
     - THE PER-PLAYER CHART IS INDEXED BY THE LIST NODE'S OWN id, not by a
       slot: `this + id * 0x21c + 0x2c`, i.e. m_2c[node->f08]. The original
       spells that as `(id << 4) - id` scaled by nine, which is the 0x21c
       stride; write it as the subscript and cl re-derives it.

   The mode-index tests (6, 7, 8, 9 and 12 all skipping) are the course modes:
   the +0x2ca4 chart and the m450b50 helper are for the non-course ones only.

   Written for the port. 912 bytes, 272 instructions, 36 calls. */

#include "ezscreen.h"
#include "directors.h"
#include "bigbase.h"
#include "obj402860.h"
#include "players.h"

extern int   countListA();                        /* 0x4184a0 */
extern int   nextListA(ListNode **cur);           /* 0x4182e0 */
extern ListNode *findListA(int key);              /* 0x418e30 */
extern int   g_1b2ea28, g_1b2eb6c, g_1b2e7d8;
extern int   g_modeIndex;                         /* 0x1b2e548 (src/modename.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00451b80
void ResultDirector::update()
{
    ListNode *cur;

    if (g_1b2eb6c == 0)
    {
        /* The plate call is written in every arm; cl cross-jumps them into
           one call, each arm pushing its own plate. */
        if (g_1b2ea28 == 1)
        {
            if (countListA() == 2)
            {
                if (g_1b2e7d8 >= 2)
                    m_ab8.setCell(6, 0, (int)m_29e0);
                else
                    m_ab8.setCell(6, 0, (int)m_29dc);
            }
            else
            {
                m_ab8.setCell(6, 0, (int)m_29e0);
            }
        }
        else
        {
            /* Both arms take the same plate; the count is still asked for. */
            if (countListA() == 2)
                m_ab8.setCell(6, 0, (int)m_29dc);
            else
                m_ab8.setCell(6, 0, (int)m_29dc);
        }
    }

    m_2408.m40b710();

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_298c, -5.0f, -1.0f, 645.0f, 0.0f, -1);

    m_cd4.m403940();
    m_ab8.m403940();
    m_ef0.m403940();
    if (g_1b2eb6c == 0)
        m_21ec.m403940();

    drawPlayers();
    drawStageNumber();

    cur = 0;
    while (nextListA(&cur) != 0)
    {
        if (g_1b2eb6c == 0)
        {
            m_2c[cur->f8].m403940();
        }
        else
        {
            if (g_1b2eb6c == 1)
            {
                /* The rank ladder: first flag that is up wins, and the two
                   sets are stamped into cells 0x2c and 0x2d of the player's
                   own chart. */
                if (f18 == 1)
                {
                    m_2c[cur->f8].setCell(0x2c, 0, (int)m_rankA[3]);
                    m_2c[cur->f8].setCell(0x2d, 0, (int)m_rankB[3]);
                }
                else if (f14 == 1)
                {
                    m_2c[cur->f8].setCell(0x2c, 0, (int)m_rankA[2]);
                    m_2c[cur->f8].setCell(0x2d, 0, (int)m_rankB[2]);
                }
                else if (f10 == 1)
                {
                    m_2c[cur->f8].setCell(0x2c, 0, (int)m_rankA[1]);
                    m_2c[cur->f8].setCell(0x2d, 0, (int)m_rankB[1]);
                }
                else
                {
                    m_2c[cur->f8].setCell(0x2c, 0, (int)m_rankA[0]);
                    m_2c[cur->f8].setCell(0x2d, 0, (int)m_rankB[0]);
                }
            }

            if (findListA(0) != 0)
                m_2c[0].m403940();
            else if (findListA(1) != 0)
                m_2c[1].m403940();
        }
    }

    if (countListA() == 1)
    {
        if (g_1b2ea28 == 1)
        {
            if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
                g_modeIndex != 9 && g_modeIndex != 12)
                m_2ca4[0].m403940();
        }
        else if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
                 g_modeIndex != 9 && g_modeIndex != 12)
        {
            if (countListA() == 1)
                m450b50();
        }
    }

    if (countListA() == 2)
    {
        if (f24 == 1)
            m_1db4.m403940();
        if (f28 == 1)
            m_1fd0.m403940();
    }

    if (f2c40 >= 4 && g_modeIndex != 1)
        m_110c.m403940();

    if (g_1b2eb6c == 0)
        m_89c.m403940();

    if (countListA() != 2 && g_1b2eb6c == 0 && f10 == 1)
        m_1544.m403940();

    m_197c.m403940();

    if (f2c40 == 0 || f2c40 == 7)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_2994, -5.0f, -5.0f, 645.0f,
                                             485.0f, (f2c44 << 24) | 0xffffff);
    }
}
