/* EzSubsystem::m40e520 @0x40e520 - load one sample for `owner`.

   Its own TU because it CALLS the two backend methods rather than inlining
   them: m40db00 and m40de30 are defined in src/subsystembase.cpp, and cl
   would expand either one it could see (the visible-body rule in CLAUDE.md).

   The dispatch is on `kind`, the "which of two backends" field at +0x50 that
   every EzSubsystem entry point switches on. Backend 0 goes through the
   object's own virtual slot 5 and then registers the result in the backend's
   table; backend 1 asks the table for the owner's second slot directly.

   THE DEFAULT ARM RETURNS AN UNINITIALIZED VALUE, and that is the original's
   shape, not a transcription slip: `kind` neither 0 nor 1 jumps straight to
   the epilogue at 0x40e57e with edi never assigned, skipping the name copy
   as well. A port should treat any other `kind` as unreachable rather than
   reproduce the read.

   The tail copies the owner's name into the new sample - an inlined strcpy at
   +0x26 of both, which is cl's /Oi expansion (`mov cl,[eax]` / `mov
   [edx+eax],cl` with edx holding dst-src), so the source says strcpy. */
#include <string.h>
#include "ez2acapp.h"

/* EzSubsystem's own slot 5, with the signature its ONE call site proves.
   Obj40EAC0 overrides that slot as `void s5()` - a placeholder from when
   nothing called it - so retyping the base would break the override; a mirror
   with the same slot shape is what CLAUDE.md prescribes for exactly this, and
   the call comes out as the original's `call [eax+0x14]` either way. */
struct SubsystemSlot5
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4();
    virtual EzSample *s5(EzSample *owner);   /* slot 5 -> +0x14 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0040e520
EzSample *EzSubsystem::m40e520(EzSample *owner)
{
    EzSample *r;

    switch (kind)
    {
    case 0:
        r = ((SubsystemSlot5 *)this)->s5(owner);
        if (r != 0)
            backend.m40de30(owner, (int)r);
        break;
    case 1:
        r = backend.m40db00(owner);
        break;
    default:
        return r;
    }

    if (r != 0)
        strcpy((char *)r + 0x26, (char *)owner + 0x26);

    return r;
}
