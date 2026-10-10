#include <windows.h>
#include "kcrt.h"
#include "ktexture.h"

/* KTextureContainer: the minimal KResourceContainer subclass. Its 2-slot
   vtable @0x487c80 = { scalar deleting dtor, getName } - the same shape as
   KAniClipContainer @0x4875ac and KBitmapContainer @0x48767c. */
// VTABLE: EZ2AC_UNPACKED 0x00487c80
struct KTextureContainer;

/* The compiler-generated scalar deleting destructor; target it by name.
   operator delete inlines to a direct `free` via kcrt.h, matching the
   original's `call 0x4788ca`. */
// FUNCTION: EZ2AC_UNPACKED 0x00408b20
// KTextureContainer::`scalar deleting destructor'

/* Adds nothing of its own: restore the vptr, then tail-jump the base dtor. */
// FUNCTION: EZ2AC_UNPACKED 0x004072b0
KTextureContainer::~KTextureContainer()
{
}

/* Its unreferenced constructor, as KAniClipContainer's (src/kcontainers.cpp). */
// FUNCTION: EZ2AC_UNPACKED 0x00407250
KTextureContainer::KTextureContainer(int size)
{
    f8 = size;
    ((KProfile *)this)->init();
}

// FUNCTION: EZ2AC_UNPACKED 0x00407240
const char *KTextureContainer::getName()
{
    // STRING: EZ2AC_UNPACKED 0x00487c6c
    return "KTextureContainer";
}
