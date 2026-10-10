/* GFMainGameDirector::m45d200 @0x45d200 - the GF input scanner: per side
   and per lane-key, route the effector chord, the strum pair or the
   fret keys. The strum machine keeps five per-side latches at
   0xcac..0xccc - the last two raw states, the up/down alternation
   counters (reset past six), and the effector chord counter (reset
   past two) - and the fret arm resolves each key's state against the
   pick before handing changes to the player. Slots walked as usual;
   note the tracker's setF0 cleanup is 8, not 12.

   In race mode (g_1b2e7d0 > 0) the effector keys run the chord
   counter and the strum pair is handled as plain frets; otherwise the
   strum pair drives the pick and the frets consume it. */

#include "ezscreen.h"
#include "players.h"

extern Obj40C180 *g_bigInstance;   /* 0x1b2e710 */
extern int g_1b2e7d0;              /* the race-mode gate */

/* One side's block at +0x340 / +0x504 - tiles the 0x1c4 stride exactly. */
struct GFSideKey
{
    int code[2];
    int lane;
    int _pad;
};
struct GFSide
{
    int active;          /* +0x00 */
    int _pad4;
    int laneEnd;         /* +0x08 */
    int laneStart;       /* +0x0c */
    GFSideKey keys[18];  /* +0x10 .. +0x130 */
    PlayerRecord *player;/* +0x130 */
    int keyStates[36];   /* +0x134 .. +0x1c4 */
};

/* The judge's bounds-checked lane store (src/accessors.cpp). */
struct Obj426CA0 { void setF0(int i, int v); };

// FUNCTION: EZ2AC_UNPACKED 0x0045d200
void GFMainGameDirector::m45d200()
{
    int side, i, j;
    volatile int pick = 0;

    for (side = 0; side < 2; side++)
    {
        GFSide *s = (GFSide *)((char *)this + 0x340 + side * 0x1c4);
        int *bx = &fcac[4 + side];   /* the 0xcbc anchor the walker keeps */

        if (s->active == 0)
            continue;

        for (i = s->laneStart; i < s->laneEnd; i++)
        {
            for (j = 0; j < 2; j++)
            {
                int code = s->keys[i].code[j];
                int st;

                if (code == -1)
                    continue;

                if ((code == 0x11 || code == 0x19) && g_1b2e7d0 > 0)
                {
                    /* The effector chord, race mode only - OUTSIDE race mode
                       these keys fall through to the fret arm below (the
                       original's 0x45d290 `jle` lands on the strum test and
                       then on the frets), not skipped. */
                    pick = g_bigInstance->getState(code);
                    if (g_bigInstance->getState(code) == 2)
                        bx[4] = 1;
                    else if (g_bigInstance->getState(code) == 3)
                    {
                        if (bx[4] > 0)
                        {
                            bx[4] += 1;
                            if (bx[4] > 2)
                            {
                                pick = 0;
                                bx[4] = 0;
                            }
                        }
                    }
                    else
                    {
                        pick = 0;
                        bx[4] = 0;
                    }
                }
                else if ((code == 0xf || code == 0x17) && g_1b2e7d0 <= 0)
                {
                    /* The strum pair, outside race mode. */
                    int stA, stB;

                    stA = g_bigInstance->getState(code);
                    stB = g_bigInstance->getState(code + 1);
                    ((Obj426CA0 *)s->player->f1ec)->setF0(s->keys[i].lane, 1);

                    if (stA == 2 || stA == 3)
                    {
                        pick = stA;
                        if (stA == 2)
                        {
                            bx[0] = 1;
                            bx[2] = 0;
                        }
                        else
                        {
                            if (stA == 3)
                            {
                                if (bx[0] != 0)
                                    bx[0] += 1;
                            }
                            if (bx[0] > 6)
                            {
                                pick = 0;
                                bx[0] = 0;
                            }
                        }
                    }
                    else if (stB == 2 || stB == 3)
                    {
                        pick = stB;
                        if (stB == 2)
                        {
                            bx[0] = 0;
                            bx[2] = 1;
                        }
                        else if (stB == 3)
                        {
                            if (bx[2] != 0)
                                bx[2] += 1;
                        }
                        if (bx[2] > 6)
                        {
                            pick = 0;
                            bx[2] = 0;
                        }
                    }
                    else if (stA == 1 || stB == 1)
                    {
                        pick = 1;
                    }
                    else if (stA == 0 && stB == 0)
                    {
                        pick = 0;
                        bx[0] = 0;
                        bx[2] = 0;
                    }
                    bx[-4] = stA;
                    bx[-2] = stB;
                }
                else
                {
                    int *ks = &s->keyStates[i * 2 + j];

                    st = g_bigInstance->getState(code);
                    ((GFTrackSink *)s->player->f1dc)->m462760(i, st);
                    if (code != 0x10 && code != 0x18 &&
                        code != 0x11 && code != 0x19)
                    {
                        if (bx[4] > 0)
                        {
                            if (st >= 2 && pick != 3)
                                st = 2;
                        }
                        else
                        {
                            if (st >= 2)
                            {
                                int p = pick;
                                if (p != 3)
                                    st = p;
                            }
                            if (bx[0] == 0 && bx[2] == 0)
                                st = 1;
                        }
                        if (st == *ks)
                        {
                            *ks = st;
                        }
                        else
                        {
                            if (pick == 2 && st == 2 && (bx[0] > 0 || bx[2] > 0))
                                s->player->handleEvent(i, 1);
                            else if (st == 2 && bx[4] > 0)
                                s->player->handleEvent(i, 1);
                            s->player->handleEvent(i, st);
                            *ks = st;
                            break;
                        }
                    }
                }
            }
        }
    }
}
