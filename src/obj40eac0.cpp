/* Obj40EAC0 - see include/obj40eac0.h. */

#include "kcrt.h"   /* operator delete = free, for the deleting destructor */
#include "obj40eac0.h"

extern int g_4a7310;   /* src/globals.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0040e790
void *Obj40EAC0::getClassDesc()
{
    return &g_4a7310;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040eac0
Obj40EAC0::Obj40EAC0()
{
    fa060 = 0;
    fa064 = 0;
}

/* The `lea ecx,[esi+0x54]; call ~SubsystemBackend` after this body is
   EzSubsystem's INLINE destructor, and the base's own is KBase's thunk. */
// FUNCTION: EZ2AC_UNPACKED 0x0040eae0
Obj40EAC0::~Obj40EAC0()
{
    m40e770();
    m40e6d0();
    if (fa064 != 0)
    {
        fa064->v2();
        fa064 = 0;
    }
    if (fa060 != 0)
    {
        fa060->v2();
        fa060 = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0040ef80
// Obj40EAC0::`scalar deleting destructor'

// VTABLE: EZ2AC_UNPACKED 0x004884e8
struct Obj40EAC0;

/* Obj40EAC0's factory. Unlike KEZPlayer's it lives in the SAME TU as the
   constructor, because the original INLINES the constructor here - the base
   call, the vptr store and the two zero stores are written out at the
   allocation site with no `call Obj40EAC0::Obj40EAC0` anywhere. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ef00
Obj40EAC0 *createObj40EAC0()
{
    return new Obj40EAC0;
}
