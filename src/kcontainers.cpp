#include <windows.h>
#include "kcrt.h"
#include "loader.h"
#include "kcontainers.h"

/* Two more members of the KResourceContainer family. Both destructors have
   the same shape: restore the vptr, Log a "Free ..." line, then run the base
   destructor. The Log call is what pulls in the EH frame (the base dtor has
   to run if it throws), so unlike ~KTextureContainer the base call here is a
   real `call` under an EH state variable rather than a tail jump. */

// VTABLE: EZ2AC_UNPACKED 0x004875ac
struct KAniClipContainer;

// FUNCTION: EZ2AC_UNPACKED 0x00402db0
// KAniClipContainer::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401b10
KAniClipContainer::~KAniClipContainer()
{
    // STRING: EZ2AC_UNPACKED 0x004875cc
    Log("Free KAniClipContainer");
}

/* The out-of-line constructor, which nothing in the binary calls (the
   owners build their containers some other way) but the linker kept: the
   KProfile ctor's shape - a "New" line, size into the base's f8, then the
   search-tree reset @0x4143e0 (declared on KProfile, whose base this is). */
// FUNCTION: EZ2AC_UNPACKED 0x00401ab0
KAniClipContainer::KAniClipContainer(int size)
{
    // STRING: EZ2AC_UNPACKED 0x004875b4
    Log("New KAniClipContainer");
    f8 = size;
    ((KProfile *)this)->init();
}

// FUNCTION: EZ2AC_UNPACKED 0x00401aa0
const char *KAniClipContainer::getName()
{
    // STRING: EZ2AC_UNPACKED 0x00487598
    return "KAniClipContainer";
}

// VTABLE: EZ2AC_UNPACKED 0x0048767c
struct KBitmapContainer;

// FUNCTION: EZ2AC_UNPACKED 0x00403f00
// KBitmapContainer::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00403e90
KBitmapContainer::~KBitmapContainer()
{
    // STRING: EZ2AC_UNPACKED 0x00487684
    Log("Free KBitmapContainer");
}

/* Its unreferenced constructor: the same, without the Log line. */
// FUNCTION: EZ2AC_UNPACKED 0x00403e30
KBitmapContainer::KBitmapContainer(int size)
{
    f8 = size;
    ((KProfile *)this)->init();
}

// FUNCTION: EZ2AC_UNPACKED 0x00403e20
const char *KBitmapContainer::getName()
{
    // STRING: EZ2AC_UNPACKED 0x00487668
    return "KBitmapContainer";
}
