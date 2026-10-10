#include <stdlib.h>
#include "kcrt.h"
#include "tinyclasses.h"
#include "judge.h"

/* See include/tinyclasses.h. Each class contributes ctor + dtor + deleting
   dtor + a one-slot vtable; operator delete inlines to a direct `free` via
   kcrt.h, matching the original's `call 0x4788ca`. */

// VTABLE: EZ2AC_UNPACKED 0x004911c8
struct Obj42DE30;

// FUNCTION: EZ2AC_UNPACKED 0x0042de30
Obj42DE30::Obj42DE30()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0042f060
// Obj42DE30::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x0042de40
Obj42DE30::~Obj42DE30()
{
}

// VTABLE: EZ2AC_UNPACKED 0x0049a680
struct Obj4659D0;

// FUNCTION: EZ2AC_UNPACKED 0x004659d0
Obj4659D0::Obj4659D0()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x004666e0
// Obj4659D0::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x004659e0
Obj4659D0::~Obj4659D0()
{
}

// VTABLE: EZ2AC_UNPACKED 0x0049e428
struct Obj472B20;

// FUNCTION: EZ2AC_UNPACKED 0x00472b20
Obj472B20::Obj472B20()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x004733e0
// Obj472B20::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00472b30
Obj472B20::~Obj472B20()
{
}

/* Judge (include/judge.h) declares no destructor, so cl generates ??1Judge for
   it - and ours stores the vptr where the original's @0x426e40 does not, which
   is the whole reason the class is modeled without one. The address is already
   owned by KBase::~KBase (src/stubs.cpp), the empty base destructor five other
   classes tail-jump; this `folded` alias only exists so that OUR generated
   destructor symbolizes under that same name and Judge's deleting destructor
   compares.

   It has to live HERE rather than in src/judge.cpp: reccmp resolves a shared
   address in source-file order, and "judge.cpp" sorts ahead of "stubs.cpp",
   which would take the address away from KBase and drop its own entry
   (that breaks all five of the other destructors that call it). KBase's is
   a real FUNCTION at 100%, and this alias is a FUNCTION too (a STUB alias
   flags the whole shared entry `stub`). */
// FUNCTION: EZ2AC_UNPACKED 0x00426e40 folded
Judge::~Judge()
{
}

/* Obj414750::stopAll @0x4148b0 - a bare forwarder to stop(); it lives here
   rather than in src/obj414750.cpp so cl cannot inline the callee. */
#include "obj414750.h"
// FUNCTION: EZ2AC_UNPACKED 0x004148b0
void Obj414750::stopAll()
{
    stop();
}
