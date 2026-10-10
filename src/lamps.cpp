#include "bigbase.h"
#include "modeinput.h"
#include "songselect.h"   /* updateStartLamps is a SongSelectDirector member */
#include "ezscreen.h"   /* TitleDirector - updateTitleStartLamps is its member */

/* src/nodelists.cpp - the joined-player list. Its body must stay there: the
   original CALLS it twice here, so cl must not be able to inline it. */
struct ListNode;
extern ListNode *findListA(int key);

/* src/modename.cpp - which mode is being played. */
extern int g_modeIndex;

/* Drive (or drop) the panel lamps that belong to the players, in the shape the
   current mode wants. The channel groups are the ones TitleDirector::tickLamps
   @0x44e050 also uses: 0x10..0x15 is player one's row and 0x18..0x1d player
   two's, with 0xa..0xd a shared bank.

   Three shapes:
     - modes 4, 5, 8 and 9 light both rows unconditionally, whether or not the
       seat is occupied;
     - modes 0, 1, 2, 6, 10 and 11 light a row only for a player who is actually
       in the game, and hold the sixth lamp of each row back in mode 0;
     - modes 3 and 7 add two of the shared bank to each occupied row.
   Anything else leaves the lamps alone.

   A BattleMode MEMBER: both callers (in update2 @0x41c660) load ecx with
   `this` - the earlier "@0x41c6d1 does not set ecx" reading missed the
   mov two instructions above its branch. thiscall with one stack arg is
   byte-identical to the old __stdcall model, so the body is unchanged.
   The `mode != 4 && mode == 8` guard is written as the original tests it - cl
   keeps both compares. */
// FUNCTION: EZ2AC_UNPACKED 0x0041bd20
void BattleMode::setPlayerLamps(int on)
{
    int mode = g_modeIndex;

    if (mode == 4 || mode == 5 || mode == 8 || mode == 9)
    {
        g_bigInstance->setOutput(0x10, on);
        g_bigInstance->setOutput(0x11, on);
        g_bigInstance->setOutput(0x12, on);
        g_bigInstance->setOutput(0x13, on);
        g_bigInstance->setOutput(0x14, on);
        g_bigInstance->setOutput(0x18, on);
        g_bigInstance->setOutput(0x19, on);
        g_bigInstance->setOutput(0x1a, on);
        g_bigInstance->setOutput(0x1b, on);
        g_bigInstance->setOutput(0x1c, on);
        if (g_modeIndex != 4 && g_modeIndex == 8)
        {
            g_bigInstance->setOutput(0xa, on);
            g_bigInstance->setOutput(0xb, on);
            g_bigInstance->setOutput(0xc, on);
            g_bigInstance->setOutput(0xd, on);
        }
        g_bigInstance->setOutput(0x15, on);
        g_bigInstance->setOutput(0x1d, on);
    }
    else if (mode == 0 || mode == 1 || mode == 2 || mode == 6 || mode == 10 || mode == 11)
    {
        if (findListA(0))
        {
            g_bigInstance->setOutput(0x10, on);
            g_bigInstance->setOutput(0x11, on);
            g_bigInstance->setOutput(0x12, on);
            g_bigInstance->setOutput(0x13, on);
            g_bigInstance->setOutput(0x14, on);
            if (g_modeIndex != 0)
            {
                g_bigInstance->setOutput(0x15, on);
            }
        }
        if (findListA(1))
        {
            g_bigInstance->setOutput(0x18, on);
            g_bigInstance->setOutput(0x19, on);
            g_bigInstance->setOutput(0x1a, on);
            g_bigInstance->setOutput(0x1b, on);
            g_bigInstance->setOutput(0x1c, on);
            if (g_modeIndex != 0)
            {
                g_bigInstance->setOutput(0x1d, on);
            }
        }
    }
    else if (mode == 3 || mode == 7)
    {
        if (findListA(0))
        {
            g_bigInstance->setOutput(0x10, on);
            g_bigInstance->setOutput(0x11, on);
            g_bigInstance->setOutput(0x12, on);
            g_bigInstance->setOutput(0x13, on);
            g_bigInstance->setOutput(0x14, on);
            g_bigInstance->setOutput(0xa, on);
            g_bigInstance->setOutput(0xb, on);
            g_bigInstance->setOutput(0x15, on);
        }
        if (findListA(1))
        {
            g_bigInstance->setOutput(0x18, on);
            g_bigInstance->setOutput(0x19, on);
            g_bigInstance->setOutput(0x1a, on);
            g_bigInstance->setOutput(0x1b, on);
            g_bigInstance->setOutput(0x1c, on);
            g_bigInstance->setOutput(0xc, on);
            g_bigInstance->setOutput(0xd, on);
            g_bigInstance->setOutput(0x1d, on);
        }
    }
}

/* The phase of the attract blink, bumped once per call. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd068
int g_blinkPhase;

/* Blink every other key lamp: on for fifteen frames out of thirty, and only for
   a seat that actually has a player in it. The shared bank 0xa..0xd is held down
   throughout.

   `g_blinkPhase++ % 30` is written as the post-increment it is - that is what
   keeps the counter live across the divide, so cl takes a callee-saved register
   for the divisor and stores the bumped value AFTER the idiv. Splitting it into
   `int c = g; g = c + 1; if (c % 30 ...)` lets cl store first and use ecx
   instead, which is the same eleven instructions in a different order.

   The single `push 0` at the top does double duty - findListA(0)'s argument on
   one side of the branch and setOutput's `off` value on the other - which is
   cl's own push merging, not something the source expresses. */
// FUNCTION: EZ2AC_UNPACKED 0x0044b330
void blinkAlternateLamps()
{
    if (g_blinkPhase++ % 30 < 15)
    {
        if (findListA(0))
        {
            g_bigInstance->setOutput(0x10, 1);
            g_bigInstance->setOutput(0x12, 1);
            g_bigInstance->setOutput(0x14, 1);
        }
        if (findListA(1))
        {
            g_bigInstance->setOutput(0x18, 1);
            g_bigInstance->setOutput(0x1a, 1);
            g_bigInstance->setOutput(0x1c, 1);
        }
    }
    else
    {
        g_bigInstance->setOutput(0x10, 0);
        g_bigInstance->setOutput(0x12, 0);
        g_bigInstance->setOutput(0x14, 0);
        g_bigInstance->setOutput(0x18, 0);
        g_bigInstance->setOutput(0x1a, 0);
        g_bigInstance->setOutput(0x1c, 0);
    }
    g_bigInstance->setOutput(0xa, 0);
    g_bigInstance->setOutput(0xb, 0);
    g_bigInstance->setOutput(0xc, 0);
    g_bigInstance->setOutput(0xd, 0);
}

/* The two START lamps, 8 and 9. With enough credit in the machine they light
   for a seat that is EMPTY - an invitation to join - and go out for one that is
   taken; with not enough credit both stay dark.

   `have` and `need` are the coin counters readBookkeeping @0x415070 fills from
   bookkeeping.ini, which is what makes the reading certain.

   Both `findListA(...) == 0` results go through the neg/sbb/add form, the
   ternary's own shape, and the single `push 0` before the branch is doing
   double duty again - findListA's argument on one side, setOutput's `off` value
   on the other.

   A THISCALL MEMBER whose body never touches `this`: all fourteen call sites
   load ecx first (the mode-input handlers with their own SongSelectDirector,
   @0x44d82f from another class entirely), and declared free the callers lose
   that `mov ecx,esi`. It hangs off SongSelectDirector because thirteen of the
   fourteen callers are its methods; the fourteenth casts. */
// FUNCTION: EZ2AC_UNPACKED 0x0044b2c0
void SongSelectDirector::updateStartLamps()
{
    Obj40C180 *g = g_bigInstance;

    if (g->have >= g->need)
    {
        g_bigInstance->setOutput(8, findListA(0) == 0);
        g_bigInstance->setOutput(9, findListA(1) == 0);
    }
    else
    {
        g_bigInstance->setOutput(8, 0);
        g_bigInstance->setOutput(9, 0);
    }
}

/* @0x0044e780 - THE TITLE SCREEN'S copy of updateStartLamps above, which
   is ModeSelect's. Once the credit counter has reached the
   coins-per-credit setting, each lamp lights for a side that has NOT yet
   joined: lamp 8 for side 0, lamp 9 for side 1, and which list is consulted
   depends on the game - A, B or C for the three of them. Below the credit
   requirement both lamps go dark.

   The `!= 0 ? 0 : 1` comes out of the original as neg/sbb/add, which is what
   comparing the found pointer against zero gives; on the first arm cl even
   reuses the just-tested register as the literal 0 it passes. */
extern int g_1b2eadc;                       /* src/showcredit.cpp */
extern ListNode *findListB(int key);        /* 0x418eb0 */
extern struct PlayerRecord *findPlayer(int id); /* 0x418f30 */

// FUNCTION: EZ2AC_UNPACKED 0x0044e780
void TitleDirector::updateTitleStartLamps()
{
    if (g_bigInstance->have >= g_bigInstance->need)
    {
        if (g_1b2eadc == 0)
        {
            g_bigInstance->setOutput(8, findListA(0) == 0);
            g_bigInstance->setOutput(9, findListA(1) == 0);
        }
        else if (g_1b2eadc == 1)
        {
            g_bigInstance->setOutput(8, findListB(0) == 0);
            g_bigInstance->setOutput(9, findListB(1) == 0);
        }
        else if (g_1b2eadc == 2)
        {
            g_bigInstance->setOutput(8, findPlayer(0) == 0);
            g_bigInstance->setOutput(9, findPlayer(1) == 0);
        }
    }
    else
    {
        g_bigInstance->setOutput(8, 0);
        g_bigInstance->setOutput(9, 0);
    }
}
