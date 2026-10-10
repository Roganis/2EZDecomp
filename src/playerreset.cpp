/* PlayerRecord::reset @0x466690 - put one list-C player back to its starting
   state under a new id: drop its list link, clear the five words above its
   0x180-byte block and the block itself, then hand off to the method after its
   constructor. Integrated from wip/playerreset.cpp at 86.96%.

   EXHAUSTED, NOT UNDIAGNOSED. Every instruction matches except the SCHEDULING
   of two that set up the inline memset: cl emits `xor eax,eax; lea edi,[edx+
   0x5c]` one store early, where the original puts `mov [edx+0x34],esi` first.

   The documented lever WAS applied and is what makes it two instructions
   rather than three: writing the five words at +0x1dc as a `for` loop cl
   unrolls gives the scheduler something to anchor on, and `push edi` then
   lands where the original has it. The matched sibling PlayerSlot::reset
   @0x42efb0 has a real loop in the same place, which is why it comes out
   clean; this one has nothing that big to anchor on.

   The alternative - writing those five words as a second memset - DOES place
   the pair correctly, and then costs three new differences: the zero constant
   moves from esi to ecx, `next` is stored with an immediate, and the five are
   addressed through a lea. Measured, not assumed. Also swept with no effect:
   the memset first/last/between, six spellings of its destination, an explicit
   clear loop, and /G7 /G6 /G5 /GB.

   What is missing is a scheduling barrier that emits no code, and nothing at
   source level is one. */

#include <string.h>
#include "players.h"

/* Put one list-C player back to its starting state under a new id: drop its list
   link, clear the five words above its 0x180-byte block and the block itself,
   and hand off to the method after its constructor. In its own TU because
   src/nodelists.cpp calls it. */
// FUNCTION: EZ2AC_UNPACKED 0x00466690
void PlayerRecord::reset(int newId)
{
    int i;

    id = newId;
    next = 0;

    for (i = 0; i < 5; i++)
    {
        ((int *)&f1dc)[i] = 0;
    }

    f34 = 0;
    memset(block, 0, sizeof(block));
    f3c = 0;
    m4659f0();
}
