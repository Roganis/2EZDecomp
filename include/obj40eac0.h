#ifndef OBJ40EAC0_H
#define OBJ40EAC0_H

#include "ez2acapp.h"
#include "icom.h"

/* The subsystem behind the eight-slot vtable @0x4884e8: EzSubsystem plus the
   backend and two COM objects. Constructor @0x40eac0, destructor @0x40eae0. */
struct Obj40EAC0 : EzSubsystem
{
    virtual void *getClassDesc();  /* 0x40e790 -> &g_4a7310 */
    virtual ~Obj40EAC0();          /* ??_G 0x40ef80, ??1 0x40eae0 */
    virtual int  init();           /* 0x40e7f0 (src/ksoundds.cpp) */
    /* Run the base's two teardowns, then drop the two COM objects. */
    virtual void shutdown();       /* 0x40e8c0 */
    /* Fill the target's sound buffer from the stream, once (src/obj40d4e0.cpp,
       beside the Obj40D230 model it works on). */
    virtual int s4(struct Obj40D230 *t, unsigned int bytes,
                   struct SoundFeed *in);          /* 0x40eda0 */
    /* Clone a sample (KSoundDS::_CreateSample, src/ksoundds.cpp). */
    virtual struct EzSample *s5(struct EzSample *src);      /* 0x40eb80 */
    /* Make a sample from scratch (KSoundDS::CreateSample). */
    virtual struct EzSample *s6(int bytes, void *wfx, int flags); /* 0x40e910 */
    /* freeSample's own hook: quiet the sample's device, run its slot 3
       teardown, quiet it again, and report that it was handled. */
    virtual int  s7(void *sample); /* 0x40ea80 */

    Obj40EAC0();                   /* 0x40eac0 */
    IThing *fa060;                 /* 0xa060 */
    IThing *fa064;                 /* 0xa064 */
};

Obj40EAC0 *createObj40EAC0();  /* 0x40ef00 */

#endif /* OBJ40EAC0_H */
