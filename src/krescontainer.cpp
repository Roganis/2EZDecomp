#include <windows.h>
#include "kcrt.h"
#include "kprofile.h"

/* KResourceContainer: the base class of KProfile (vtable @0x488c24). Owns a
   VirtualAlloc'd arena at +0x0c that its destructor releases. This TU pins the
   base vtable (reccmp source_root=src) and holds all four base methods. */
// VTABLE: EZ2AC_UNPACKED 0x00488c24
struct KResourceContainer;

/* 0x414580 is the compiler-generated scalar deleting destructor; target it by
   name. The real ~KResourceContainer @0x4143b0 is the definition below. */
// FUNCTION: EZ2AC_UNPACKED 0x00414580
// KResourceContainer::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x004143b0
KResourceContainer::~KResourceContainer()
{
    f4 = 0;
    if (arena != 0)
    {
        VirtualFree(arena, 0, MEM_RELEASE);
        arena = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00414380
const char *KResourceContainer::getName()
{
    // STRING: EZ2AC_UNPACKED 0x00488c10
    return "KResourceContainer";
}

// FUNCTION: EZ2AC_UNPACKED 0x00414390
KResourceContainer::KResourceContainer()
{
    arena = 0;
    f4 = 0;
}
