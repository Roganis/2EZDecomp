/* EzChart's base destructor, alone in its own TU.

   It is empty, so it compiles to a bare `ret` and the original's linker
   ICF-folded it onto the shared 0x402880 - the same address as the compiled-out
   logger, which owns the annotation (src/applog.cpp, alphabetically first). The
   `folded` marker makes this an alias so ~EzChart's `call 0x402880` at EH
   state -1 symbolizes identically on both sides.

   It cannot live in src/ezchart.cpp: a visible empty body gets inlined into
   ~EzChart, which then loses its whole EH frame. */

#include "ezchart.h"

// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
EzBase::~EzBase()
{
}

/* And its constructor, equally empty and equally folded - onto KBase's
   `mov eax,ecx; ret` @0x401690 (src/constants.cpp owns that address). It is
   what the factory's `call 0x401690` reaches, and it too has to live outside
   src/ezchart.cpp or cl deletes the call. */
// FUNCTION: EZ2AC_UNPACKED 0x00401690 folded
EzBase::EzBase()
{
}
