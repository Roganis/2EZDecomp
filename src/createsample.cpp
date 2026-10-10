/* EzSubsystem::createSample @0x40e3c0 - the sound entry point every
   sample in the game comes through. Resolve and open the name
   (openSample writes the record fields into the 0x14-byte block, may
   rewrite the handle through its address, and hands the opened stream
   back through the slot cl packs into the dead name argument's home),
   then claim storage by the subsystem's kind - kind 1 takes the best
   free backend slot, kind 0 asks the subsystem's own slot 6 and
   registers the result - feed the sample to slot 4, log-and-release
   on either failure, and stamp the name into the sample last, off the
   register copy the rewrite never touched.

   Its own TU: openSample and the backend live elsewhere. The parked
   three-pass analysis in wip/createsample-40e3c0.md is superseded -
   its contradiction was the out-object in the dead argument slot. */
#include <string.h>
#include "ez2acapp.h"
#include "kengineapp.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (folded) */

/* The subsystem's own vtable, as far as this function reaches it. */
struct SubsysS46
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void s4(struct EzSample *p, int b, struct EzStream *in);  /* slot 4 */
    virtual void q5();
    virtual struct EzSample *s6(int b, void *block, int a);           /* slot 6 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0040e3c0
int EzSubsystem::createSample(char *name, int a, int b)
{
    int       block[5];
    EzStream *in;
    EzSample *r;

    if (name == 0)
        return 0;
    r = 0;
    switch (kind)
    {
    case 1:
        if (openSample(name, b, (char **)&b, (int)(void *)block, &in) == 1)
        {
            r = backend.m40dbc0((unsigned int)b, *(short *)((char *)block + 2),
                                block[1], a);
            if (r != 0)
                ((SubsysS46 *)this)->s4(r, b, in);
            else
            {
                // STRING: EZ2AC_UNPACKED 0x00488408
                Log("Error : KSoundDS::CreateSample(%s) - LockSample(%d)",
                    name, b);
                if (in != 0)
                    in->release(1);
            }
        }
        break;
    case 0:
        if (openSample(name, b, (char **)&b, (int)(void *)block, &in) == 1)
        {
            r = ((SubsysS46 *)this)->s6(b, block, a);
            if (r != 0)
            {
                ((SubsysS46 *)this)->s4(r, b, in);
                backend.m40dea0(r);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x004883e4
                Log("Error : KSoundDS::CreateSample(%s)", name);
                if (in != 0)
                    in->release(1);
            }
        }
        break;
    }
    if (r != 0)
    {
        strncpy(r->name, name, 0x7f);
        r->name[0x7f] = 0;
    }
    return (int)r;
}
