/* Obj40C180::anyPlayHeld @0x4188c0 - is any play button currently held?

   Seventeen codes tested in turn - 0x0a through 0x19, and then 0x00 last -
   each for state 2, with the first hit answering yes.

   IT HAS TO BE ONE `||` CHAIN. Seventeen separate `if (...) return 1;`
   statements put the `mov eax,1; ret` INLINE after the first test and make
   the other sixteen jump backwards to it (77.17%); seventeen `goto`s to a
   trailing label get the direction right but cl then swaps the two tail
   blocks, so the last test inverts to `jne` and every displacement is eight
   out (79.56%). The single conjunction gives the original exactly: all
   seventeen `je` forward, `xor eax,eax; ret` first, `mov eax,1; ret` last. */

#include "bigbase.h"

// FUNCTION: EZ2AC_UNPACKED 0x004188c0
int Obj40C180::anyPlayHeld()
{
    if (getState(0x0a) == 2 ||
        getState(0x0b) == 2 ||
        getState(0x0c) == 2 ||
        getState(0x0d) == 2 ||
        getState(0x0e) == 2 ||
        getState(0x0f) == 2 ||
        getState(0x10) == 2 ||
        getState(0x11) == 2 ||
        getState(0x12) == 2 ||
        getState(0x13) == 2 ||
        getState(0x14) == 2 ||
        getState(0x15) == 2 ||
        getState(0x16) == 2 ||
        getState(0x17) == 2 ||
        getState(0x18) == 2 ||
        getState(0x19) == 2 ||
        getState(0x00) == 2)
        return 1;
    return 0;
}

/* Obj40C180::m419780 @0x419780 - is any player still holding a play button?

   The same seven codes as anyPlayHeld's front half - 0x0a/0x0c/0x0e for the
   left side, 0x12/0x14/0x16 for the right, then 0x00 - asked once per list.
   List A's pair is gated on the player actually existing, EXCEPT in the four
   modes 4/5/8/9, where the whole gate is dropped and the seven codes are
   asked flat; lists B and C are always gated.  Every hit answers yes.

   The else arm FALLS THROUGH into list B's section, so it is not an
   alternative to the whole body - only to list A's three statements. */

extern int g_modeIndex;                            /* 0x1b2e548 (src/modename.cpp) */
extern struct ListNode      *findListA(int key);   /* 0x418e30 (src/nodelists.cpp) */
extern struct ListNode      *findListB(int key);   /* 0x418eb0 (src/nodelists.cpp) */
extern struct PlayerRecord  *findPlayer(int id);   /* 0x418f30 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00419780
int Obj40C180::m419780()
{
    if (g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 8 && g_modeIndex != 9)
    {
        if (findListA(0) != 0 &&
            (getState(0x0a) == 2 || getState(0x0c) == 2 || getState(0x0e) == 2))
            return 1;
        if (findListA(1) != 0 &&
            (getState(0x12) == 2 || getState(0x14) == 2 || getState(0x16) == 2))
            return 1;
        if (getState(0x00) == 2)
            return 1;
    }
    else
    {
        if (getState(0x0a) == 2 ||
            getState(0x0c) == 2 ||
            getState(0x0e) == 2 ||
            getState(0x12) == 2 ||
            getState(0x14) == 2 ||
            getState(0x16) == 2 ||
            getState(0x00) == 2)
            return 1;
    }

    if (findListB(0) != 0 &&
        (getState(0x0a) == 2 || getState(0x0c) == 2 || getState(0x0e) == 2))
        return 1;
    if (findListB(1) != 0 &&
        (getState(0x12) == 2 || getState(0x14) == 2 || getState(0x16) == 2))
        return 1;
    if (getState(0x00) == 2)
        return 1;

    if (findPlayer(0) != 0 &&
        (getState(0x0a) == 2 || getState(0x0c) == 2 || getState(0x0e) == 2))
        return 1;
    if ((findPlayer(1) != 0 &&
         (getState(0x12) == 2 || getState(0x14) == 2 || getState(0x16) == 2)) ||
        getState(0x00) == 2)
        return 1;

    return 0;
}

/* Obj40C180::m4191c0 @0x4191c0 - the wider twin of m419780 above.

   Same three-list sweep, but the per-side code set is the full nine -
   0x0a..0x11 plus 0x04 for the left player, 0x12..0x19 plus 0x05 for the
   right - where m419780 asks only 0x0a/0x0c/0x0e and 0x12/0x14/0x16.
   The 4/5/8/9 else arm again drops list A's gate and asks all nineteen
   codes flat before falling through into list B. */

// FUNCTION: EZ2AC_UNPACKED 0x004191c0
int Obj40C180::m4191c0()
{
    if (g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 8 && g_modeIndex != 9)
    {
        if (findListA(0) != 0 &&
            (getState(0x0a) == 2 ||
             getState(0x0b) == 2 ||
             getState(0x0c) == 2 ||
             getState(0x0d) == 2 ||
             getState(0x0e) == 2 ||
             getState(0x0f) == 2 ||
             getState(0x10) == 2 ||
             getState(0x11) == 2 ||
             getState(0x04) == 2))
            return 1;
        if (findListA(1) != 0 &&
            (getState(0x12) == 2 ||
             getState(0x13) == 2 ||
             getState(0x14) == 2 ||
             getState(0x15) == 2 ||
             getState(0x16) == 2 ||
             getState(0x17) == 2 ||
             getState(0x18) == 2 ||
             getState(0x19) == 2 ||
             getState(0x05) == 2))
            return 1;
        if (getState(0x00) == 2)
            return 1;
    }
    else
    {
        if (getState(0x0a) == 2 ||
            getState(0x0b) == 2 ||
            getState(0x0c) == 2 ||
            getState(0x0d) == 2 ||
            getState(0x0e) == 2 ||
            getState(0x0f) == 2 ||
            getState(0x10) == 2 ||
            getState(0x11) == 2 ||
            getState(0x04) == 2 ||
            getState(0x12) == 2 ||
            getState(0x13) == 2 ||
            getState(0x14) == 2 ||
            getState(0x15) == 2 ||
            getState(0x16) == 2 ||
            getState(0x17) == 2 ||
            getState(0x18) == 2 ||
            getState(0x19) == 2 ||
            getState(0x05) == 2 ||
            getState(0x00) == 2)
            return 1;
    }

    if (findListB(0) != 0 &&
        (getState(0x0a) == 2 ||
         getState(0x0b) == 2 ||
         getState(0x0c) == 2 ||
         getState(0x0d) == 2 ||
         getState(0x0e) == 2 ||
         getState(0x0f) == 2 ||
         getState(0x10) == 2 ||
         getState(0x11) == 2 ||
         getState(0x04) == 2))
        return 1;
    if (findListB(1) != 0 &&
        (getState(0x12) == 2 ||
         getState(0x13) == 2 ||
         getState(0x14) == 2 ||
         getState(0x15) == 2 ||
         getState(0x16) == 2 ||
         getState(0x17) == 2 ||
         getState(0x18) == 2 ||
         getState(0x19) == 2 ||
         getState(0x05) == 2))
        return 1;
    if (getState(0x00) == 2)
        return 1;

    if (findPlayer(0) != 0 &&
        (getState(0x0a) == 2 ||
         getState(0x0b) == 2 ||
         getState(0x0c) == 2 ||
         getState(0x0d) == 2 ||
         getState(0x0e) == 2 ||
         getState(0x0f) == 2 ||
         getState(0x10) == 2 ||
         getState(0x11) == 2 ||
         getState(0x04) == 2))
        return 1;
    if ((findPlayer(1) != 0 &&
         (getState(0x12) == 2 ||
          getState(0x13) == 2 ||
          getState(0x14) == 2 ||
          getState(0x15) == 2 ||
          getState(0x16) == 2 ||
          getState(0x17) == 2 ||
          getState(0x18) == 2 ||
          getState(0x19) == 2 ||
          getState(0x05) == 2)) ||
        getState(0x00) == 2)
        return 1;

    return 0;
}
