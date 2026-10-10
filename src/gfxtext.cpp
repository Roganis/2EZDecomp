/* The two sprite-font number draws @0x40b100 and @0x40b140 - both pure
   forwarders into the family's wider base, and both duplicate their THIRD
   argument into the base's FIFTH slot.

   That is the whole content of them: nine `mov`/`push` pairs and a tail call,
   with `this` inherited in ecx (neither touches it, and neither reloads it).
   Reading the duplication off the disassembly needs the push depth tracked
   line by line, because every source operand is written `[esp+X]` against an
   esp that has already moved - drawNumI's `mov eax,[esp+0x18]` is argument 3,
   not argument 6. Pushed right to left, drawNumI's ten pushes spell
   (a1, a2, a3, a4, a3, a5, a6, a7, a8) and drawNumF's eleven spell
   (a1, a2, a3, a4, a3, a5, a6, a7, a8, a9); in both, the repeat lands on the
   base's fifth float, and the argument types agree with the base's signature
   slot for slot either way round.

   They must live away from both their callers (src/gamepanels.cpp and
   src/effectordraw.cpp) and from their callees, which are still stubs in
   src/stubs.cpp - either neighbour would let cl inline one side away and
   the tail call would disappear. */

#include "gfxtext.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040b100
void GfxText::drawNumI(float x, float y, float w, float h, int value,
                       const char *name, int flag, void *ctx)
{
    m40a970(x, y, w, h, w, value, name, flag, ctx);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040b140
void GfxText::drawNumF(float x, float y, float w, float h, float e, float f,
                       const char *name, int flag, void *ctx)
{
    m40aaa0(x, y, w, h, w, e, f, name, flag, ctx);
}
