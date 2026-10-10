/* EZ2DJMainGameDirector::m41ffa0 @0x41ffa0 - the per-frame LANE INPUT SWEEP.

   The director's f380 block is two 0x1c4-byte lane maps, one per player, and
   this walks both of them: for each live map, each lane from `first` to
   `end`, and each of the lane's TWO key codes, it reads that key's current
   state off the engine object and, when the state has moved since last frame,
   reports it to the map's PlayerSlot through handleEvent @0x430970.

   The two scratch keys 0xf and 0x17 are special, and they are the same pair
   PlayerSlot::m430500 @0x430500 swaps: a scratch is TWO physical directions,
   `key` and `key + 1`, so both are read and reduced to one event. A direction
   that has gone to 2 or 3 wins; if the OTHER direction was at 2 or 3 last
   frame and has now dropped to 1 or below, its release is reported first.
   Failing that a 1 on either direction reports 1, and both at rest report 0.
   Anything else only latches. Both directions' states are latched whatever
   happens, which is why the pair is remembered as a two-entry row.

   Its own TU so handleEvent and getStateB stay calls. */
#include <string.h>
#include "ezscreen.h"
#include "players.h"
#include "bigbase.h"

extern Obj40C180 *g_bigInstance;   /* 0x1b2e710 (src/accessors.cpp) */
extern int        g_modeIndex;     /* 0x1b2e548 (src/modename.cpp) */

struct LaneItem
{
    /* The lane's TWO key codes; -1 for an unused one. */
    int key[2];               /* 0x00 */
    int f8;
    int fc;
};

struct LaneMap
{
    int         live;         /* 0x000 */
    int         f4;
    int         end;          /* 0x008 */
    int         first;        /* 0x00c */
    LaneItem    items[18];    /* 0x010 .. 0x130 */
    PlayerSlot *slot;         /* 0x130 */
    int         prev[18][2];  /* 0x134 .. 0x1c4 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0041ffa0
void EZ2DJMainGameDirector::m41ffa0()
{
    LaneMap *m = (LaneMap *)&f380[0];
    int      n = 2;

    do
    {
        if (m->live != 0)
        {
            int i = m->first;

            if (i < m->end)
            {
                int       *pr = &m->prev[i][1];
                const int *item = &m->items[i].key[0];

                do
                {
                    int        j = 0;
                    const int *k = item;

                    do
                    {
                        int key = *k;

                        if (key != -1)
                        {
                            if (key == 0xf || key == 0x17)
                            {
                                int s1 = g_bigInstance->getStateB(key);
                                int s2 = g_bigInstance->getStateB(key + 1);

                                if (s1 == 2 || s1 == 3)
                                {
                                    if (pr[0] >= 2 && s2 <= 1)
                                        m->slot->handleEvent(i, 1);
                                    m->slot->handleEvent(i, s1);
                                }
                                else if (s2 == 2 || s2 == 3)
                                {
                                    if (pr[-1] >= 2 && s1 <= 1)
                                        m->slot->handleEvent(i, 1);
                                    m->slot->handleEvent(i, s2);
                                }
                                else if (s1 == 1 || s2 == 1)
                                {
                                    m->slot->handleEvent(i, 1);
                                }
                                else if (s1 == 0 && s2 == 0)
                                {
                                    m->slot->handleEvent(i, 0);
                                }

                                pr[-1] = s1;
                                pr[0] = s2;
                            }
                            else
                            {
                                int *cur = &m->prev[i][j];
                                int  s = g_bigInstance->getStateB(key);

                                if (s != *cur)
                                {
                                    m->slot->handleEvent(i, s);
                                    *cur = s;
                                    break;
                                }
                                *cur = s;
                            }
                        }

                        j++;
                        k++;
                    }
                    while (j < 2);

                    pr += 2;
                    i++;
                    item += 4;
                }
                while (i < m->end);
            }
        }

        m++;
    }
    while (--n != 0);
}

/* EZ2DJMainGameDirector::m420130 @0x420130 - the SUSTAINED variant of the
   sweep above, and the one the caller @0x424f6a picks when 0x1b5f2e0 is up.

   Same two maps, same lanes, same two key codes, same scratch pair handled
   the same way. What it adds is the other half of a held key: when a key's
   state has NOT moved since last frame, the lane is still charged for the
   hold through PlayerSlot::m42f4e0 @0x42f4e0 with the player's own +0x210
   rate - so the plain variant reports edges only and this one also pays out
   sustains. The one exception is mode 4's lane 6, where only the SECOND of
   the lane's two key codes pays; the first is silent.

   Written next to its twin because they are one source read twice, but note
   they are NOT the same shape: this one indexes `items[i].key[j]` and
   `prev[i][j]` outright where m41ffa0 walks two pointers, and cl anchors its
   map pointer eight bytes in (at `end`) rather than at the map's base. */
/* The same map, anchored where cl anchors THIS function's walking pointer -
   eight bytes in, at `end`. `live` is then two words below it. The trailing
   pad keeps the stride at 0x1c4 so the pointer still walks with ++. */
struct LaneMap8
{
    int         end;          /* map +0x008 */
    int         first;        /* map +0x00c */
    LaneItem    items[18];    /* map +0x010 */
    PlayerSlot *slot;         /* map +0x130 */
    int         prev[18][2];  /* map +0x134 */
    char        _pad1bc[8];
};

// FUNCTION: EZ2AC_UNPACKED 0x00420130
void EZ2DJMainGameDirector::m420130()
{
    LaneMap8 *m = (LaneMap8 *)&f380[2];
    int       n = 2;

    do
    {
        /* The map from its real base. cl keeps ONE walking pointer - the +8
           one - and reaches the item array through it with a -8
           displacement, which is what rooting the item access here
           produces. */
        const LaneMap *m0 = (const LaneMap *)((const char *)m - 8);

        if (m0->live != 0)
        {
            int i = m->first;

            if (i < m->end)
            {
                do
                {
                    int j = 0;

                    do
                    {
                        int key = m0->items[i].key[j];

                        if (key != -1)
                        {
                            if (key == 0xf || key == 0x17)
                            {
                                int s1 = g_bigInstance->getStateB(key);
                                int s2 = g_bigInstance->getStateB(key + 1);

                                if (s1 == 2 || s1 == 3)
                                {
                                    if (m->prev[i][1] >= 2 && s2 <= 1)
                                        m->slot->handleEvent(i, 1);
                                    m->slot->handleEvent(i, s1);
                                }
                                else if (s2 == 2 || s2 == 3)
                                {
                                    if (m->prev[i][0] >= 2 && s1 <= 1)
                                        m->slot->handleEvent(i, 1);
                                    m->slot->handleEvent(i, s2);
                                }
                                else if (s1 == 1 || s2 == 1)
                                {
                                    m->slot->handleEvent(i, 1);
                                }
                                else if (s1 == 0 && s2 == 0)
                                {
                                    m->slot->handleEvent(i, 0);
                                }

                                m->prev[i][0] = s1;
                                m->prev[i][1] = s2;
                            }
                            else
                            {
                                int *cur = &m->prev[i][j];
                                int  s = g_bigInstance->getStateB(key);

                                if (s != *cur)
                                {
                                    m->slot->handleEvent(i, s);
                                    *cur = s;
                                    break;
                                }

                                /* the plain call written in BOTH arms, as in
                                   m41fb20: the original keeps two copies */
                                if (g_modeIndex != 4)
                                {
                                    m->slot->m42f4e0(i, m->slot->f210);
                                }
                                else if (i == 6)
                                {
                                    if (j == 1)
                                        m->slot->m42f4e0(i, m->slot->f210);
                                }
                                else
                                {
                                    m->slot->m42f4e0(i, m->slot->f210);
                                }

                                *cur = s;
                            }
                        }

                        j++;
                    }
                    while (j < 2);

                    i++;
                }
                while (i < m->end);
            }
        }

        m++;
    }
    while (--n != 0);
}

/* EZ2DJMainGameDirector::m41fb20 @0x41fb20 - the IN-PLAY variant of the
   sweep, the one update2 drives during a run (its call site @0x424f7a).

   On top of the m41ffa0/m420130 machine it adds three things.  The
   autoplay filter: with g_1b5f180 up, the side g_1b2eb60 names is
   skipped outright - keys 6,7,0xa..0x11 belong to player 0 and
   8,9,0x12..0x19 to player 1.  The assist devices: three flag/key-pair
   ladders - g_1b2f050 on 0xf/0x10, g_1b2ef80 on 0x17/0x18, g_1b2efbc
   on 0x11/0x19 - each of which bumps the slot's +0x3dc counter at the
   lane's f8 index and, when g_useAudiencePanel is 1, pays the hold
   instead of ever reporting the key.  (The first ladder entry is
   written twice in the original - once under the all-keys guard and
   once in the ladder proper, where it is unreachable - and cl merged
   the dead copy's block with the third entry's.)  And the sustain rule
   grows: modes 4 and 8, and mode 0xc when the style tag says ClubMix,
   keep mode 4's lane-6 exception where only the second key pays.

   This one reads the RAW key state (getState @0x40c1c0) where both
   twins read the debounced getStateB. */

extern int  g_1b5f180;             /* 0x1b5f180 (src/maingame.cpp) */
extern int  g_1b2eb60;             /* 0x1b2eb60 (src/gamestate.cpp) */
extern int  g_1b2f050;             /* 0x1b2f050 (src/rankingctor.cpp) */
extern int  g_1b2ef80;             /* 0x1b2ef80 (src/rankingctor.cpp) */
extern int  g_1b2efbc;             /* 0x1b2efbc (src/rankingctor.cpp) */
extern int  g_useAudiencePanel;    /* 0x1b2eb48 (src/maingame.cpp) */
extern char g_styleTag[36];        /* 0x1b2e5ac (src/maingame.cpp) */

struct Obj426CA0 { void setF0(int i, int v); };   /* 0x426ca0 */

// FUNCTION: EZ2AC_UNPACKED 0x0041fb20
void EZ2DJMainGameDirector::m41fb20()
{
    int a, b, h;

    for (a = 0; a < 2; a++)
    {
        LaneMap *m = (LaneMap *)&f380[a * 0x71];

        if (m->live == 0)
            continue;

        for (b = m->first; b < m->end; b++)
        {
            for (h = 0; h < 2; h++)
            {
                int key = m->items[b].key[h];

                if (key == -1)
                    continue;

                if (g_1b5f180 != 0)
                {
                    if (g_1b2eb60 == 1)
                    {
                        if (key == 0x19 || key == 0x12 || key == 0x13 ||
                            key == 0x14 || key == 0x15 || key == 0x16 ||
                            key == 0x17 || key == 0x18 || key == 8 || key == 9)
                            continue;
                    }
                    else if (g_1b2eb60 == 0)
                    {
                        if (key == 0x11 || key == 0xa || key == 0xb ||
                            key == 0xc || key == 0xd || key == 0xe ||
                            key == 0xf || key == 0x10 || key == 6 || key == 7)
                            continue;
                    }
                }

                if (key == 0x11 || key == 0xa || key == 0xb || key == 0xc ||
                    key == 0xd || key == 0xe || key == 0xf || key == 0x10 ||
                    key == 6 || key == 7 || key == 0x19 || key == 0x12 ||
                    key == 0x13 || key == 0x14 || key == 0x15 || key == 0x16 ||
                    key == 0x17 || key == 0x18 || key == 8 || key == 9)
                {
                    if (key == 0xf || key == 0x10)
                    {
                        if (g_1b2f050 != 0)
                        {
                            ((Obj426CA0 *)m->slot->f3dc)->setF0(m->items[b].f8, 1);
                            if (g_useAudiencePanel == 1)
                                m->slot->m42f4e0(b, m->slot->f210);
                            continue;
                        }
                    }
                }

                if (key == 0xf || key == 0x10)
                {
                    if (g_1b2f050 != 0)
                    {
                        ((Obj426CA0 *)m->slot->f3dc)->setF0(m->items[b].f8, 1);
                        if (g_useAudiencePanel == 1)
                            m->slot->m42f4e0(b, m->slot->f210);
                        continue;
                    }
                }

                if (key == 0x17 || key == 0x18)
                {
                    if (g_1b2ef80 != 0)
                    {
                        ((Obj426CA0 *)m->slot->f3dc)->setF0(m->items[b].f8, 1);
                        if (g_useAudiencePanel == 1)
                            m->slot->m42f4e0(b, m->slot->f210);
                        continue;
                    }
                }

                if (key == 0x11 || key == 0x19)
                {
                    if (g_1b2efbc != 0)
                    {
                        ((Obj426CA0 *)m->slot->f3dc)->setF0(m->items[b].f8, 1);
                        if (g_useAudiencePanel == 1)
                            m->slot->m42f4e0(b, m->slot->f210);
                        continue;
                    }
                }

                if (key == 0xf || key == 0x17)
                {
                    int s1 = g_bigInstance->getState(key);
                    int s2 = g_bigInstance->getState(key + 1);

                    if (s1 == 2 || s1 == 3)
                    {
                        if (m->prev[b][1] >= 2 && s2 <= 1)
                            m->slot->handleEvent(b, 1);
                        m->slot->handleEvent(b, s1);
                    }
                    else if (s2 == 2 || s2 == 3)
                    {
                        if (m->prev[b][0] >= 2 && s1 <= 1)
                            m->slot->handleEvent(b, 1);
                        m->slot->handleEvent(b, s2);
                    }
                    else if (s1 == 1 || s2 == 1)
                    {
                        m->slot->handleEvent(b, 1);
                    }
                    else if (s1 == 0 && s2 == 0)
                    {
                        m->slot->handleEvent(b, 0);
                    }

                    m->prev[b][0] = s1;
                    m->prev[b][1] = s2;
                }
                else
                {
                    int *cur = &m->prev[b][h];
                    int  s = g_bigInstance->getState(key);
                    int  mode;

                    if (s != *cur)
                    {
                        m->slot->handleEvent(b, s);
                        *cur = s;
                        break;
                    }

                    mode = g_modeIndex;
                    /* The pick is an if/else with the plain call written in
                       BOTH arms, the ClubMix test in the outer condition: the
                       original keeps two copies of `m42f4e0(b, f210); *cur =
                       s;`, where one combined `(...) || b != 6` condition gave
                       cl a single shared copy (82.48% -> 97.55% by the permuter's
                       scorer). What is left is the block order around the
                       _stricmp test, which cl normalises. */
                    if (mode == 4 || mode == 8 ||
                        (mode == 0xc && _stricmp("ClubMix", g_styleTag) == 0))
                    {
                        if (b != 6)
                            m->slot->m42f4e0(b, m->slot->f210);
                        else if (h == 1)
                            m->slot->m42f4e0(6, m->slot->f210);
                    }
                    else
                    {
                        m->slot->m42f4e0(b, m->slot->f210);
                    }
                    *cur = s;
                }
            }
        }
    }
}
