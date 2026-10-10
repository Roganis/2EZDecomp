/* CatchMainGameDirector::m46d360 @0x46d360 - move each side's paddle and
   open the lanes under it. Args and slots walked as with the drawer
   family (scratchpad tracker).

   The paddle half-width defaults to 65px; extreme mode logs itself and
   narrows it to 47; RaceMode (g_1b2ebac) widens the window to the full
   65 unless the paddle is hidden (g_1b5f2e0), which otherwise pins it
   at 430. Per side: the dial delta or the held keys drive the speed -
   the boost keys at 0xa/0xc/0xe (0x12/0x14/0x16 for 2P) scale it by
   0.8/1.2/1.5 while held - and the curve is squared near zero (x6, x-6
   leftward) and linear (x30) beyond +/-5. The reverse modifier
   g_1b2e894 negates it. The paddle x clamps to [0, (lanes-1)*width -
   halfwidth]. Then every lane opens or closes by whether its column is
   under the paddle - inverted whole-sale by g_1b2ef1c, with a grace
   timer (g_1b2ebb4) that zeroes the half-width while it runs - and the
   judging hook m473660 fires on each change when g_catchJudging is on. */

#include "ezscreen.h"
#include "gamepanels.h"
#include "players.h"

extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 */
extern struct ListNode *findListA(int key);        /* 0x418e30 */
extern void Log(const char *fmt, ...);             /* 0x402880 (folded) */
/* The lane table's bounds-checked store - the real definition lives in
   src/accessors.cpp; this TU sees only the slot it calls. */
struct Obj426CA0 { void setF0(int i, int v); };

extern int g_1b2eb70;      /* the extreme-mode flag */
extern int g_1b2ebac;      /* RaceMode, published from f330 */
extern int g_1b5f2e0;      /* the hidden-paddle flag */
extern int g_1b2e894;      /* the mode-10 chord toggle - reverse */
extern int g_1b2ebb4;      /* the lane grace timer */
extern int g_1b2ef1c;      /* the invert modifier */
extern int g_catchJudging; /* 0x1b2e58c */

/* One side's paddle block at +0x34c + side * 0x1bc - a view into the
   director's f34c run, matching the walker's own anchors. */
struct CatchSideLane
{
    int id;
    int _pad[3];
};
struct CatchSide
{
    int active;               /* +0x34c */
    int laneCount;            /* +0x350 */
    int _pad354[2];
    CatchSideLane lanes[13];  /* +0x35c */
    char _pad42c[0x474 - 0x42c];
    CatchPlayer *player;      /* +0x474 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0046d360
void CatchMainGameDirector::m46d360()
{
    float half;
    int side;

    half = 65.0f;
    if (g_1b2eb70 > 0)
    {
        half = 47.0f;
        // STRING: EZ2AC_UNPACKED 0x0049daa8
        Log("Use Extreme Mode");
    }
    if (g_1b2ebac != 0)
        half = g_1b5f2e0 != 0 ? 0.0f : 65.0f;
    else if (g_1b5f2e0 != 0)
        half = 430.0f;

    for (side = 0; side < 2; side++)
    {
        CatchSide *s = (CatchSide *)((char *)this + 0x34c + side * 0x1bc);
        char *flags = (char *)this + 0x1c + side * 0xd;
        CatchPanel *panel;
        float width, v, x, hi;
        int stL, stR, i;

        if (s->active == 0)
            continue;

        width = s->player->f35c->f20;
        stL = g_bigInstance->getState(side * 8 + 0xf);
        stR = g_bigInstance->getState(side * 8 + 0x10);

        v = 0.0f;
        if (stL != 2 && stL != 3)
        {
            if (stR == 2 || stR == 3)
            {
                float t = (float)g_bigInstance->delta[side];

                if (findListA(0) != 0)
                {
                    if (g_bigInstance->getState(0xa) == 3)
                        t *= 0.8f;
                    if (g_bigInstance->getState(0xc) == 3)
                        t *= 1.2f;
                    if (g_bigInstance->getState(0xe) == 3)
                        t *= 1.5f;
                }
                else
                {
                    if (g_bigInstance->getState(0x12) == 3)
                        t *= 0.8f;
                    if (g_bigInstance->getState(0x14) == 3)
                        t *= 1.2f;
                    if (g_bigInstance->getState(0x16) == 3)
                        t *= 1.5f;
                }
                if (t > -5.0f)
                    v = t * t * -6.0f;
                else
                    v = t * 30.0f;
            }
        }
        else
        {
            float t = (float)g_bigInstance->delta[side];

            if (findListA(0) != 0)
            {
                if (g_bigInstance->getState(0xa) == 3)
                    t *= 0.8f;
                if (g_bigInstance->getState(0xc) == 3)
                    t *= 1.2f;
                if (g_bigInstance->getState(0xe) == 3)
                    t *= 1.5f;
            }
            else
            {
                if (g_bigInstance->getState(0x12) == 3)
                    t *= 0.8f;
                if (g_bigInstance->getState(0x14) == 3)
                    t *= 1.2f;
                if (g_bigInstance->getState(0x16) == 3)
                    t *= 1.5f;
            }
            if (5.0f > t)
                v = t * t * 6.0f;
            else
                v = t * 30.0f;
        }

        /* Step the paddle, reversed under the modifier, clamped to the
           playfield. */
        panel = s->player->f35c;
        x = panel->f18;
        if (g_1b2e894 > 0)
        {
            x = -v + x;
            hi = (float)(s->laneCount - 1) * width - half;
            if (x > hi)
                x = hi;
            else if (0.0f > x)
                x = 0.0f;
            panel->f18 = x;
        }
        else
        {
            x = x + v;
            hi = (float)(s->laneCount - 1) * width - half;
            if (x > hi)
                x = hi;
            else if (0.0f > x)
                x = 0.0f;
            panel->f18 = x;
        }

        if (g_1b2ebb4 > 0)
            g_1b2ebb4 -= 1;

        for (i = 0; i < s->laneCount; i++)
        {
            float lo = (float)i * width;

            if (g_1b2ef1c > 0)
            {
                if (g_1b2ebb4 > 0)
                    half = 0.0f;
                if (x > lo - half && lo + width > x)
                {
                    ((Obj426CA0 *)s->player->f36c)->setF0(s->lanes[i].id, 0);
                    flags[i] = 0;
                }
                else
                {
                    ((Obj426CA0 *)s->player->f36c)->setF0(s->lanes[i].id, 1);
                    flags[i] = 1;
                }
            }
            else if (x > lo - half && lo + width > x)
            {
                ((Obj426CA0 *)s->player->f36c)->setF0(s->lanes[i].id, 1);
                if (g_catchJudging != 0)
                    s->player->m473660(i, s->player->f370);
                flags[i] = 1;
            }
            else
            {
                ((Obj426CA0 *)s->player->f36c)->setF0(s->lanes[i].id, 0);
                if (g_catchJudging != 0)
                    s->player->m473660(i, s->player->f370);
                flags[i] = 0;
            }
        }
    }
}
