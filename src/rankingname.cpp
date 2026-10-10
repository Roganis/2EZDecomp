/* RankingDirector::m45aaf0 @0x45aaf0 - the ranking name-entry stepper:
   the wheel keys/dial, the glyph wheel with its eased scroll, the two
   50-byte-record name tables, and the confirm path - including the
   original's easter egg: an all-'A' or all-space name is rewritten to
   "EZ2AC_FN" before it is copied out to the player node and the
   last-name global. Derived in wip/ranking-nameentry.cpp's history.

   `a` picks the slot pair, `b` the side (0 steps +1, else -1). */

#include "ezscreen.h"
#include "directors.h"
#include "players.h"

extern Obj40C180 *g_bigInstance;   /* 0x1b2e710 */
extern int getListAF8();           /* 0x418340 */
extern struct ListNode *findListA(int key);  /* 0x418e30 */
extern int g_modeIndex;            /* 0x1b2e548 */

/* The glyph wheel - rankingctor.cpp's kEntryChars: A-Z, 0-9, the
   symbols, then space; rows 0x30/0x31 are the END and DEL specials. */
extern char kEntryChars[];
/* The per-side key codes: down {0xf,0x17}, up {0x10,0x18}, and the
   five-per-side effector group at 0x4ae498. */
extern int g_rankKeyDown[2];
extern int g_rankKeyUp[2];
extern int g_rankKeyRows[2][5];
extern char g_savedName[8];   /* 0x4a8e8c (src/rankingctor.cpp) */

/* The sample view (the family's stop/play pair). */
struct RnkSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
    virtual void stop();             /* +0x14 */
};

/* The two 50-byte-record name tables, keyed by (&f109b48)[a] and
   (&f85dfc)[a]. */
#define NAMEA(k) (((char (*)[50])((char *)this + 0x20cc))[(&f109b48)[a]][k])
#define NAMEB(k) (((char (*)[50])((char *)this + 0x85e04))[(&f85dfc)[a]][k])

// FUNCTION: EZ2AC_UNPACKED 0x0045aaf0
void RankingDirector::m45aaf0(int a, int b)
{
    int   step = (b == 0) ? 1 : -1;
    int   idx;
    float target, d;

    if (!((&f85dfc)[a] < 99 && (&f109dd0)[a] == 0 && f85df8 == 1))
    {
        if (!((&f109b48)[a] < 99 && (&f109dd0)[a] == 0 && f85df8 == 0))
            goto wheel;
    }

    if (g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 8 && g_modeIndex != 9)
    {
        if (g_bigInstance->accum[b] < -5 ||
            g_bigInstance->getState(g_rankKeyUp[b]) == 2)
        {
            ((RnkSnd *)f109d8c)->stop();
            ((RnkSnd *)f109d8c)->play(0);
            if (f109db0[a] == 8)
            {
                int n = f109da8[a] + step;

                if (n > 0x2f && n <= 0x31)
                    f109da8[a] = n;
            }
            else
                f109da8[a] += step;
        }
        if (g_bigInstance->accum[b] > 5 ||
            g_bigInstance->getState(g_rankKeyDown[b]) == 2)
        {
            ((RnkSnd *)f109d8c)->stop();
            ((RnkSnd *)f109d8c)->play(0);
            goto stepDown;
        }
    }
    else
    {
        /* Two-side modes route by which side joined. */
        /* Two-side modes route by which side joined. */
        if (getListAF8() == 0)
        {
            if (g_bigInstance->accum[0] < -5 ||
                g_bigInstance->accum[1] < -5 ||
                g_bigInstance->getState(g_rankKeyUp[0]) == 2 ||
                g_bigInstance->getState(g_rankKeyUp[1]) == 2)
            {
                ((RnkSnd *)f109d8c)->stop();
                ((RnkSnd *)f109d8c)->play(0);
                if (f109db0[a] == 8)
                {
                    int n = f109da8[a] + step;

                    if (n > 0x2f && n <= 0x31)
                        f109da8[a] = n;
                }
                else
                    f109da8[a] += step;
            }
            if (g_bigInstance->accum[0] > 5 ||
                g_bigInstance->accum[1] > 5 ||
                g_bigInstance->getState(g_rankKeyDown[0]) == 2 ||
                g_bigInstance->getState(g_rankKeyDown[1]) == 2)
            {
                ((RnkSnd *)f109d8c)->stop();
                ((RnkSnd *)f109d8c)->play(0);
                if (f109db0[a] == 8)
                {
                    int n = f109da8[a] - step;

                    if (n > 0x2f && n <= 0x31)
                        f109da8[a] = n;
                }
                else
                    f109da8[a] -= step;
            }
        }
        else if (getListAF8() == 1)
        {
            if (g_bigInstance->accum[0] > 5 ||
                g_bigInstance->accum[1] > 5 ||
                g_bigInstance->getState(g_rankKeyDown[0]) == 2 ||
                g_bigInstance->getState(g_rankKeyDown[1]) == 2)
            {
                ((RnkSnd *)f109d8c)->stop();
                ((RnkSnd *)f109d8c)->play(0);
                if (f109db0[a] == 8)
                {
                    int n = f109da8[a] + step;

                    if (n > 0x2f && n <= 0x31)
                        f109da8[a] = n;
                }
                else
                    f109da8[a] += step;
            }
            if (g_bigInstance->accum[0] < -5 ||
                g_bigInstance->accum[1] < -5 ||
                g_bigInstance->getState(g_rankKeyUp[0]) == 2 ||
                g_bigInstance->getState(g_rankKeyUp[1]) == 2)
            {
                ((RnkSnd *)f109d8c)->stop();
                ((RnkSnd *)f109d8c)->play(0);
stepDown:
                if (f109db0[a] == 8)
                {
                    int n = f109da8[a] - step;

                    if (n > 0x2f && n <= 0x31)
                        f109da8[a] = n;
                }
                else
                    f109da8[a] -= step;
            }
        }
    }

wheel:
    /* The wheel's eased scroll - the in-game wheel's ring ease over a
       5000-unit ring at a third of the gap. */
    idx = (f109da8[a] + 50) % 50;
    f109da8[a] = idx;
    target = (float)idx * 100.0f;
    f109dc8[a] = target;
    d = target - f109dc0[a];
    if (d > 2500.0f)
        f109dc0[a] -= (5000.0f - d) * (1.0f / 3.0f);
    else
    {
        if (-2500.0f > d)
            f109dc0[a] += (d + 5000.0f) * (1.0f / 3.0f);
        else
            f109dc0[a] += d * (1.0f / 3.0f);
    }
    if (1.0f > d && d > -1.0f)
        f109dc0[a] = target;
    if (-4999.0f > d && d > -5001.0f)
        f109dc0[a] = target;
    if (f109dc0[a] > 5000.0f)
        f109dc0[a] -= 5000.0f;
    if (0.0f > f109dc0[a])
        f109dc0[a] += 5000.0f;

    /* The typing half. The keys alternate per side: rows 1 and 3 are
       backspace, rows 0, 2 and 4 type (the C had them the other way round
       until 2026-10-03: docs/PORT-DELTAS.md). */
    if (!((&f85dfc)[a] < 99 && (&f109dd0)[a] == 0 && f85df8 == 1))
    {
        if (!((&f109b48)[a] < 99 && (&f109dd0)[a] == 0 && f85df8 == 0))
            goto done;
    }

    if (g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 8 && g_modeIndex != 9)
    {
        if (findListA(0) != 0 && g_bigInstance->getState(4) == 2)
        {
            ((RnkSnd *)f109d94)->stop();
            ((RnkSnd *)f109d94)->play(0);
            (&f109dd0)[a] = 1;
        }
        if (findListA(1) != 0 && g_bigInstance->getState(5) == 2)
        {
            ((RnkSnd *)f109d94)->stop();
            ((RnkSnd *)f109d94)->play(0);
            (&f109dd0)[a] = 1;
        }
        if (g_bigInstance->getState(g_rankKeyRows[b][1]) == 2 ||
            g_bigInstance->getState(g_rankKeyRows[b][3]) == 2)
        {
            /* Backspace. */
            if (f109db0[a] > 0)
            {
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                if (f109db0[a] > 0)
                {
                    f109db0[a] -= 1;
                    NAMEA(f109db0[a]) = ' ';
                    if (f109da8[a] == 0x31)
                        f109da8[a] = 0;
                    if (f85df8 != 0)
                        NAMEB(f109db0[a]) = ' ';
                }
                if (f109db0[a] == 7)
                    f109da8[a] = 0;
            }
        }
        else if (g_bigInstance->getState(g_rankKeyRows[b][0]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[b][2]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[b][4]) == 2)
        {
            if (f109da8[a] < 0x30)
            {
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                NAMEA(f109db0[a]) = kEntryChars[f109da8[a]];
                if (f85df8 != 0)
                    NAMEB(f109db0[a]) = kEntryChars[f109da8[a]];
                f109db0[a] += 1;
                if (f109db0[a] == 8)
                {
                    m4577b0();
                    f109da8[a] = 0x30;
                    f109dc8[a] = 4800.0f;
                    f109dc0[a] = 4800.0f;
                }
            }
            else if (f109da8[a] == 0x31)
            {
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                if (f109db0[a] > 0)
                {
                    f109db0[a] -= 1;
                    NAMEA(f109db0[a]) = ' ';
                    if (f85df8 != 0)
                        NAMEB(f109db0[a]) = ' ';
                }
            }
            else if (f109da8[a] == 0x30)
            {
                ((RnkSnd *)f109d94)->stop();
                ((RnkSnd *)f109d94)->play(0);
                (&f109dd0)[a] = 1;
            }
        }
    }
    else
    {
        if (g_bigInstance->getState(4) == 2 ||
            g_bigInstance->getState(5) == 2)
        {
            ((RnkSnd *)f109d94)->stop();
            ((RnkSnd *)f109d94)->play(0);
            (&f109dd0)[a] = 1;
        }
        if (g_bigInstance->getState(g_rankKeyRows[0][1]) == 2 ||
            g_bigInstance->getState(g_rankKeyRows[0][3]) == 2 ||
            g_bigInstance->getState(g_rankKeyRows[1][1]) == 2 ||
            g_bigInstance->getState(g_rankKeyRows[1][3]) == 2)
        {
            /* Backspace. */
            if (f109db0[a] > 0)
            {
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                if (f109db0[a] > 0)
                {
                    f109db0[a] -= 1;
                    NAMEA(f109db0[a]) = ' ';
                    if (f109da8[a] == 0x31)
                        f109da8[a] = 0;
                    if (f85df8 != 0)
                        NAMEB(f109db0[a]) = ' ';
                }
                if (f109db0[a] == 7)
                    f109da8[a] = 0;
            }
        }
        else if (g_bigInstance->getState(g_rankKeyRows[0][0]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[0][2]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[0][4]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[1][0]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[1][2]) == 2 ||
                 g_bigInstance->getState(g_rankKeyRows[1][4]) == 2)
        {
            if (f109da8[a] < 0x30)
            {
                /* Type the glyph into both tables. */
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                NAMEA(f109db0[a]) = kEntryChars[f109da8[a]];
                if (f85df8 != 0)
                    NAMEB(f109db0[a]) = kEntryChars[f109da8[a]];
                f109db0[a] += 1;
                if (f109db0[a] == 8)
                {
                    m4577b0();
                    f109da8[a] = 0x30;
                    f109dc8[a] = 4800.0f;
                    f109dc0[a] = 4800.0f;
                }
            }
            else if (f109da8[a] == 0x31)
            {
                /* DEL. */
                ((RnkSnd *)f109d90)->stop();
                ((RnkSnd *)f109d90)->play(0);
                if (f109db0[a] > 0)
                {
                    f109db0[a] -= 1;
                    NAMEA(f109db0[a]) = ' ';
                    if (f85df8 != 0)
                        NAMEB(f109db0[a]) = ' ';
                }
            }
            else if (f109da8[a] == 0x30)
            {
                /* END. */
                ((RnkSnd *)f109d94)->stop();
                ((RnkSnd *)f109d94)->play(0);
                (&f109dd0)[a] = 1;
            }
        }
    }

done:
    /* The commit: once confirmed, rewrite a blank or all-'A' name to
       the original's own "EZ2AC_FN", then copy it out. Every byte goes
       through the row index again - a char store may alias it. */
    if (!((&f85dfc)[a] < 99 && (&f109dd0)[a] == 1 && f85df8 == 1))
    {
        if (!((&f109b48)[a] < 99 && (&f109dd0)[a] == 1 && f85df8 == 0))
            return;
    }
    if ((NAMEA(0) == 'A' && NAMEA(1) == 'A' && NAMEA(2) == 'A' &&
         NAMEA(3) == 'A' && NAMEA(4) == 'A' && NAMEA(5) == 'A' &&
         NAMEA(6) == 'A' && NAMEA(7) == 'A') ||
        (NAMEA(0) == ' ' && NAMEA(1) == ' ' && NAMEA(2) == ' ' &&
         NAMEA(3) == ' ' && NAMEA(4) == ' ' && NAMEA(5) == ' ' &&
         NAMEA(6) == ' ' && NAMEA(7) == ' '))
    {
        NAMEA(0) = 'E'; NAMEA(1) = 'Z'; NAMEA(2) = '2'; NAMEA(3) = 'A';
        NAMEA(4) = 'C'; NAMEA(5) = '_'; NAMEA(6) = 'F'; NAMEA(7) = 'N';
    }
    if (f85df8 != 0)
    {
        if ((NAMEB(0) == 'A' && NAMEB(1) == 'A' && NAMEB(2) == 'A' &&
             NAMEB(3) == 'A' && NAMEB(4) == 'A' && NAMEB(5) == 'A' &&
             NAMEB(6) == 'A' && NAMEB(7) == 'A') ||
            (NAMEB(0) == ' ' && NAMEB(1) == ' ' && NAMEB(2) == ' ' &&
             NAMEB(3) == ' ' && NAMEB(4) == ' ' && NAMEB(5) == ' ' &&
             NAMEB(6) == ' ' && NAMEB(7) == ' '))
        {
            NAMEB(0) = 'E'; NAMEB(1) = 'Z'; NAMEB(2) = '2'; NAMEB(3) = 'A';
            NAMEB(4) = 'C'; NAMEB(5) = '_'; NAMEB(6) = 'F'; NAMEB(7) = 'N';
        }
    }
    {
        char *dst = (char *)findListA(b) + 0x1f8;

        /* Copy the confirmed name out to the player node and the saved
           name - from the second table when it is in use, else from the
           first. (The C had only the first arm until 2026-10-01, so a name
           confirmed with f85df8 clear never reached the node:
           docs/PORT-DELTAS.md.) */
        if (f85df8 != 0)
        {
            dst[0] = NAMEB(0); dst[1] = NAMEB(1); dst[2] = NAMEB(2);
            dst[3] = NAMEB(3); dst[4] = NAMEB(4); dst[5] = NAMEB(5);
            dst[6] = NAMEB(6); dst[7] = NAMEB(7);
            g_savedName[0] = NAMEB(0); g_savedName[1] = NAMEB(1);
            g_savedName[2] = NAMEB(2); g_savedName[3] = NAMEB(3);
            g_savedName[4] = NAMEB(4); g_savedName[5] = NAMEB(5);
            g_savedName[6] = NAMEB(6); g_savedName[7] = NAMEB(7);
        }
        else
        {
            dst[0] = NAMEA(0); dst[1] = NAMEA(1); dst[2] = NAMEA(2);
            dst[3] = NAMEA(3); dst[4] = NAMEA(4); dst[5] = NAMEA(5);
            dst[6] = NAMEA(6); dst[7] = NAMEA(7);
            g_savedName[0] = NAMEA(0); g_savedName[1] = NAMEA(1);
            g_savedName[2] = NAMEA(2); g_savedName[3] = NAMEA(3);
            g_savedName[4] = NAMEA(4); g_savedName[5] = NAMEA(5);
            g_savedName[6] = NAMEA(6); g_savedName[7] = NAMEA(7);
        }
    }
}
