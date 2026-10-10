#ifndef KTEXTURE_H
#define KTEXTURE_H

#include "kprofile.h"   /* KResourceContainer */

/* KTextureContainer: a KResourceContainer subclass with the family's minimal
   2-slot vtable @0x487c80 = { scalar deleting dtor, getName }. Its own
   destructor adds nothing - it only restores the vptr and tail-jumps
   ~KResourceContainer @0x4143b0 - so no members are evidenced here. */
struct KTextureContainer : KResourceContainer
{
    KTextureContainer(int size);        /* 0x407250 - no caller links it */
    virtual ~KTextureContainer();       /* 0x4072b0 (deleting ??_G 0x408b20) */
    virtual const char *getName();      /* 0x407240 -> "KTextureContainer" */
};

#endif /* KTEXTURE_H */
