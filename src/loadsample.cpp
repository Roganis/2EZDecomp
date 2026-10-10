#include <string.h>
#include "ez2acapp.h"

extern void Log(const char *fmt, ...);   /* 0x402880 */

/* Slot 4 through a mirror, as include/ez2acapp.h says slot 5 is reached - the
   base's own declaration is typed for a different override. */
struct SubsysS4
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual EzSample *s4(EzSample *dst, char *name, EzStream *in);
};

/* EzSubsystem::loadSample @0x40e590 - integrated at 53.78%. The semantics are
   read and the model (EzSample, EzStream, openSample, subsystem slot 4) is in
   include/ez2acapp.h; what is left is register allocation.

   THE ORIGINAL USES FOUR CALLEE-SAVED REGISTERS WITH SPLIT SAVES, and that is
   the whole defect. ebx and esi are pushed in the prologue - ebx takes the
   caller's original name pointer and esi `this`, then the result - while ebp
   and edi are pushed LATER, past the failure return, because they are only
   live on the success path: ebp the stream out-parameter, edi the destination.
   Our build saves all of them up front.

   That is the prologue-push signature CLAUDE.md describes, read from the other
   side: normally an extra push in OUR build means a cached parameter, and here
   the original is the one deferring its saves. FIXED 2026-10-01 by reading
   the stream through a local after the open (below): the saves now split
   exactly as the original's. What is left is scheduling - the original
   loads `orig` into ebx straight from the parameter at entry and pushes
   ebx as openSample's first argument, where cl loads eax, pushes it and
   copies to ebx after; and on the success path it loads the resolved name
   before the stream. Tried: passing `orig` as the first argument, the
   declaration order, a separate resolved-name local (right registers, but
   it takes a new frame slot instead of the parameter's), `s = in` inside
   the call, and naming the resolved name too. */

/* Load one sample by name. openSample resolves the name - it takes the ADDRESS
   of the caller's pointer and may rewrite it, which is why the original keeps
   its own copy of the incoming pointer in a callee-saved register and uses that
   for the error message and for the name it remembers, while the backend gets
   the resolved one. If the backend refuses the sample, the stream is released.

   Written with `name` itself passed by address, not through a fresh local: the
   original hands over the parameter's own stack slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e590
void *EzSubsystem::loadSample(EzSample *dst, char *name)
{
    char *orig = name;
    EzStream *in = 0;
    void *sample;

    if (!openSample(name, 0, &name, 0, &in))
    {
        // STRING: EZ2AC_UNPACKED 0x0048843c
        Log("Open error : LoadSample %s", orig);
        return 0;
    }

    /* The stream through a fresh local: the original loads it into ebp
       once, for the call and the release, and that is also what defers the
       ebp/edi saves past the failure return (they were pushed up front
       while `in` was read from the frame at each use). */
    EzStream *s = in;
    sample = ((SubsysS4 *)this)->s4(dst, name, s);
    if (sample == 0 && s != 0)
    {
        s->release(1);
    }

    strncpy(dst->name, orig, 0x7f);
    dst->name[0x7f] = 0;
    return sample;
}
