/* ModeSelectDirector::m44b4b0 @0x44b4b0 - the button-driven twin of
   m44b750 @0x44b750, which is matched at 100% in src/modeselecttimer.cpp.
   The confirm and seat halves are the same code; this one adds its own two
   input arms in front of them. Integrated at 56.53%.

   Three constant registers are live throughout (ebp = 1, edi = 2, ebx = 0).
   The park checked that against the refinement added for
   TotalResultDirector::update2 - which says a constant register cl declines is
   evidence your CONSTANTS are misread before it is evidence of a coin - and
   the constants here are right, so the coin is what it looks like. */

#include "modeselect.h"
#include "bigbase.h"

/* The same local view src/modeselecttimer.cpp uses for its twin. */
struct SoundSlots
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual void s4(int a);   /* slot 4 -> +0x10 */
    virtual void s5();        /* slot 5 -> +0x14 */
};

extern char g_modeTables[0x1660];   /* 0x4ac788 (src/modeselectctor.cpp) */
extern int  g_1b2ea20;              /* src/modeselectctor.cpp */
extern int  g_1b2eadc;              /* src/ez2acapp.cpp */

extern void setModeName(const char *name);          /* 0x414e50 */
extern int  countListA();                           /* 0x418310 */
extern int  getListAF8();                           /* 0x418340 */
extern struct ListNode      *findListB(int key);    /* 0x418eb0 */
extern struct PlayerRecord  *findPlayer(int id);    /* 0x418f30 */
extern struct PlayerRecord  *addPlayer(int id);     /* 0x418410 */
extern struct CatchPlayer   *addCatchPlayer(int id);/* 0x418350 */

/* (The declarations this needs are the ones at the top of
   src/modeselecttimer.cpp - SoundSlots, the g_modeTables/g_1b2eadc globals
   and the nodelists externs. Copy them in when this moves to src/.) */

/* ModeSelectDirector::m44b4b0 @0x44b4b0 - the same confirm as m44b750
   above, reached from a player's buttons rather than from the clock.

   `who` picks which player's inputs are read: 1 looks at codes 0x0a/0x0c/
   0x0e and start 4, 2 at 0x12/0x14/0x16 and start 5. Either way one of the
   three must read 2, the start button must read neither 2 nor 3, and the
   list must either be this player's own or hold both - and then the mode is
   committed and the players seated exactly as the countdown does it.

   The two arms are written out in full, and they must NOT be spelled the
   same: written identically, cl cross-jumps the whole confirm (sound,
   setModeName, the f9620 pick) into one copy, where the original keeps
   both and only shares the `== 11` test (56.53%). What keeps the copies
   apart AND lets the second arm jump into the first's `== 11` test is
   spelling the two picks differently: the first as `else if (f18 != 11)
   f9620 = 0; else f9620 = 2;`, the second as a nested if/else under
   `f18 == 10`. Either change alone: 94.21% / 62.78%. Found by
   scratchpad/perm (from 90.05%). */
// FUNCTION: EZ2AC_UNPACKED 0x0044b4b0
void ModeSelectDirector::m44b4b0(int who)
{
    int kind;

    if (f95e4 != 1)
        return;

    if (who == 1)
    {
        f9614 = 0;
        if (g_bigInstance->getState(0x0a) != 2 &&
            g_bigInstance->getState(0x0c) != 2 &&
            g_bigInstance->getState(0x0e) != 2)
            goto done;
        if (getListAF8() != 0 && countListA() != 2)
            goto done;
        if (g_bigInstance->getState(4) == 2)
            goto done;
        if (g_bigInstance->getState(4) == 3)
            goto done;

        ((SoundSlots *)sndDecide)->s5();
        ((SoundSlots *)sndDecide)->s4(0);
        setModeName(&g_modeTables[f18 * 260]);
        f95e4 = 2;
        if (f18 == 10)
            f9620 = 1;
        else if (f18 != 11)
            f9620 = 0;
        else
            f9620 = 2;
    }
    else if (who == 2)
    {
        f9614 = 0;
        if (g_bigInstance->getState(0x12) != 2 &&
            g_bigInstance->getState(0x14) != 2 &&
            g_bigInstance->getState(0x16) != 2)
            goto done;
        if (g_bigInstance->getState(5) == 2)
            goto done;
        if (g_bigInstance->getState(5) == 3)
            goto done;
        if (getListAF8() != 1 && countListA() != 2)
            goto done;

        ((SoundSlots *)sndDecide)->s5();
        ((SoundSlots *)sndDecide)->s4(0);
        setModeName(&g_modeTables[f18 * 260]);
        f95e4 = 2;
        if (f18 == 10)
            f9620 = 1;
        else
        {
            if (f18 == 11)
                f9620 = 2;
            else
                f9620 = 0;
        }
    }
    else
    {
        return;
    }

    kind = f9620;
    if (kind == 1)
    {
        g_1b2eadc = 1;
        if (countListA() == 1)
        {
            if (findListB(getListAF8()) != 0)
                return;
            addCatchPlayer(getListAF8());
            g_1b2eadc = 0;
            return;
        }
    }
    else if (kind == 2)
    {
        g_1b2eadc = 2;
        if (countListA() == 1)
        {
            if (findPlayer(getListAF8()) != 0)
                return;
            addPlayer(getListAF8());
        }
        else if (countListA() == 2)
        {
            if (findPlayer(0) != 0)
                return;
            addPlayer(0);
            if (findPlayer(1) != 0)
                return;
            addPlayer(1);
        }
    }

    g_1b2eadc = 0;
    return;

done:
    ;
}
