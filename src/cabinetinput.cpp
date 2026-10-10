/* CabinetInput (include/bigbase.h) - the concrete subsystem object behind
   g_bigInstance. Its out-of-line constructor and destructor are linked but
   nothing references either (the static at 0x170dff0 is built another way);
   they live in their own TU so the base constructor stays a real call and
   the implicit destructor a bare `jmp` into ~Obj40C180, as in the original. */
#include <windows.h>
#include "bigbase.h"

// VTABLE: EZ2AC_UNPACKED 0x0048cf50
struct CabinetInput;

// FUNCTION: EZ2AC_UNPACKED 0x00414fc0
CabinetInput::CabinetInput()
{
}

/* The implicit destructor, out of line (see include/bigbase.h): the helper
   makes cl emit it, the marker names it. */
// FUNCTION: EZ2AC_UNPACKED 0x00414fe0
// CabinetInput::~CabinetInput

void emitCabinetInputDtor(CabinetInput *p)
{
    p->~CabinetInput();
}
