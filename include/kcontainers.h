#ifndef KCONTAINERS_H
#define KCONTAINERS_H

#include "kprofile.h"   /* KResourceContainer */

/* Two more KResourceContainer subclasses sharing the family's minimal 2-slot
   vtable { scalar deleting dtor, getName }. Neither destructor touches a
   member of its own - each just logs and lets the base dtor run - so no
   layout beyond the base is evidenced. */

/* vtable @0x4875ac */
struct KAniClipContainer : KResourceContainer
{
    KAniClipContainer(int size);        /* 0x401ab0 - no caller links it */
    virtual ~KAniClipContainer();       /* 0x401b10 (deleting ??_G 0x402db0) */
    virtual const char *getName();      /* 0x401aa0 -> "KAniClipContainer" */
};

/* vtable @0x48767c */
struct KBitmapContainer : KResourceContainer
{
    KBitmapContainer(int size);         /* 0x403e30 - no caller links it */
    virtual ~KBitmapContainer();        /* 0x403e90 (deleting ??_G 0x403f00) */
    virtual const char *getName();      /* 0x403e20 -> "KBitmapContainer" */
};

#endif /* KCONTAINERS_H */
