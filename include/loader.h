#ifndef LOADER_H
#define LOADER_H

#include "ez2acapp.h"

/* The sample owner IS the sound subsystem: freeSample @0x40e620 dispatches on
   the same +0x50 selector and the same +0x54 backend as m40e770 @0x40e770,
   which the screens call on the subsystem singleton. */
typedef EzSubsystem SampleOwner;

/* .ezi sample table holder. Table of 0x800 sample pointers @0xc9d1c. */
struct EziLoader
{
    int          field_0x0;
    SampleOwner *field_0x4;
    char         _pad[0xc9d1c - 8];
    void        *sampleTable[0x800];

    void unloadSamples();       /* 0x00411050 */
    int  loadEZI(char *name);   /* 0x00411090 */
};

/* Variadic logger @0x00402880 (empty in release). STUB (stubs.cpp). */
extern void Log(const char *fmt, ...);

#endif /* LOADER_H */
