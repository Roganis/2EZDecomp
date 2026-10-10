/* ModeSelectDirector::m44b750 @0x44b750 - the mode-select countdown, and the
   confirm it turns into.

   The tick counts frames at +0x95dc and every sixtieth one drops the seconds
   at +0x95d8. When the seconds reach zero the confirm flag at +0x95fc goes
   up; while they are still running and under ten, the count-down sound is
   restarted each second. The seconds are clamped at zero either way.

   Once the flag is up the mode is committed: its name is fetched out of the
   260-byte-stride table at 0x4ac788 by the selection index at +0x18, the
   decide sound plays, and the game KIND at +0x9620 is derived - 1 for
   selection 10, 2 for 11, 0 otherwise. Kind 1 seats a catch player, kind 2
   seats one or two ordinary players depending on the list count, and
   anything else just clears the busy flag. Every path that actually seats
   somebody clears g_1b2eadc on the way out; the paths that find the player
   already there leave it set, which is in the original and is not a slip. */

#include "modeselect.h"
#include "bigbase.h"   /* Obj40C180, g_bigInstance - the input board */

/* The sound objects, viewed for the two slots this function drives. */
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

// FUNCTION: EZ2AC_UNPACKED 0x0044b750
void ModeSelectDirector::m44b750()
{
    int kind;

    f95dc += 1;
    if (f95dc == 60)
    {
        f95d8 -= 1;
        f95dc = 0;
        if (f95d8 <= 0)
            f95fc = 1;
        if (f95fc == 0 && f95d8 < 10)
        {
            ((SoundSlots *)sndTimeCount)->s5();
            ((SoundSlots *)sndTimeCount)->s4(0);
        }
    }

    if (f95d8 <= 0)
        f95d8 = 0;

    if (f95fc != 1)
        return;

    setModeName(&g_modeTables[f18 * 260]);
    ((SoundSlots *)sndDecide)->s4(0);
    f95e4 = 2;

    if (f18 == 10)
        f9620 = 1;
    else if (f18 == 11)
        f9620 = 2;
    else
        f9620 = 0;

    g_1b2ea20 = 0;

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
            g_1b2eadc = 0;
            return;
        }
        if (countListA() == 2)
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
