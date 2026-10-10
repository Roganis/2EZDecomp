/* Obj40C180::m4189e0 / m418a50 / m418af0 @0x4189e0 / 0x418a50 / 0x418af0 -
   bind or unbind three consecutive ranges of input slots to their channels.

   Each range maps slot n to channel n + 6, and passes -1 for every slot when
   the argument is zero, which is what leaves that slot unbound. Both of the
   longer two end on a REPEAT: 0x0f and 0x10 share channel 0x15, and 0x17 and
   0x18 share 0x1d - the original's own doing, and cl's CSE of the identical
   ternary is what produces the second call's re-use of the first's register.

   `on ? K : -1` compiles branchless (neg/sbb/and/add -1), which is the
   documented shape for a ternary between two constants; written as an if/else
   over two calls cl would cross-jump them into one instead. */

#include "bigbase.h"

// FUNCTION: EZ2AC_UNPACKED 0x004189e0
void Obj40C180::m4189e0(int on)
{
    setChannel(0x06, on ? 0x0a : -1);
    setChannel(0x07, on ? 0x0b : -1);
    setChannel(0x08, on ? 0x0c : -1);
    setChannel(0x09, on ? 0x0d : -1);
}

// FUNCTION: EZ2AC_UNPACKED 0x00418a50
void Obj40C180::m418a50(int on)
{
    setChannel(0x0a, on ? 0x10 : -1);
    setChannel(0x0b, on ? 0x11 : -1);
    setChannel(0x0c, on ? 0x12 : -1);
    setChannel(0x0d, on ? 0x13 : -1);
    setChannel(0x0e, on ? 0x14 : -1);
    setChannel(0x0f, on ? 0x15 : -1);
    setChannel(0x10, on ? 0x15 : -1);
}

// FUNCTION: EZ2AC_UNPACKED 0x00418af0
void Obj40C180::m418af0(int on)
{
    setChannel(0x12, on ? 0x18 : -1);
    setChannel(0x13, on ? 0x19 : -1);
    setChannel(0x14, on ? 0x1a : -1);
    setChannel(0x15, on ? 0x1b : -1);
    setChannel(0x16, on ? 0x1c : -1);
    setChannel(0x17, on ? 0x1d : -1);
    setChannel(0x18, on ? 0x1d : -1);
}
