#ifndef OBJ405F80_H
#define OBJ405F80_H

#include "kbase.h"

/* The singleton the engine publishes into 0xd36cac as it is built - a KBase
   derivative with a FOUR-slot vtable @0x487b80 (getClassDesc, the destructor,
   and two more), not one of the six-slot EzObject family.

   Only what its constructor @0x405f80 writes and its destructor @0x405ff0
   reads is modeled; the size is a lower bound from the highest field it
   touches. */
struct Obj405F80 : KBase
{
    virtual void *getClassDesc();   /* 0x405570 -> &g_4a5218 */
    virtual ~Obj405F80();           /* ??_G 0x4065e0, ??1 0x405ff0 */
    virtual int  v2();              /* 0x406170 - the bring-up (src/obj405f80.cpp) */
    /* Teardown: drop the GDI object, empty both KProfiles, release the
       seven interfaces and free the global table. */
    virtual void v3();              /* 0x405dd0 */
    void m409320();                 /* 0x409320 (STUB) */

    Obj405F80();                    /* 0x405f80 */
    /* Drop the cooperative level and release all seven interfaces, in the
       order the original does; returns 0x8200000c if either the device or
       DirectDraw itself still had references (src/comrelease.cpp). */
    int  releaseAll();              /* 0x405580 */

    int  f04[5];        /* 0x004 .. 0x018 */
    char _pad18[0x28 - 0x18];
    int  f28[8];        /* 0x028 .. 0x048 */
    char _pad48[0xd0 - 0x48];
    int  fd0;           /* 0x0d0 */
    int  fd4;           /* 0x0d4 */
    char _padd8[0x12c - 0xd8];
    int  f12c;          /* 0x12c - 0xffffff */
    int  f130;          /* 0x130 */
    char _pad134[0x1c8 - 0x134];
    int  f1c8;          /* 0x1c8 */
    int  f1cc;          /* 0x1cc */
    char _pad1d0[0x1d4 - 0x1d0];   /* the factory allocates 0x1d4 */
};

Obj405F80 *createObj405F80();       /* 0x406580 */

/* Free, no arguments: walks a global table at 0xd3111c freeing each entry. */
void freeTable405530();             /* 0x405530 (STUB) */

#endif /* OBJ405F80_H */
