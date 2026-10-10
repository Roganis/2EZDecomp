#include <string.h>
#include "ez2acapp.h"
#include "loader.h"

/* The subsystem's two backend dispatchers. Their callers (the screens, and the
   .ezi loader) and their callees (the backend entry points, STUBs in
   src/stubs.cpp) both live elsewhere, which is what keeps the tail calls tail
   calls. Both switches list case 1 before case 0 because that is the original's
   block order - the tests still come out in numeric order. */

void extractDotName(char *p, char *out);   /* 0x412f60 (src/strutil.cpp) */

/* EzSubsystem::m40e200 @0x0040e200 - the handle at +0x48 if it is already
   resolved, otherwise the dot-name of the argument looked up through
   m40e1c0. One exit: the original leaves the answer in eax on both paths
   and shares the cookie check, so the source keeps one variable rather
   than returning from inside the branch. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e200
void *EzSubsystem::m40e200(char *name)
{
    char  buf[0x20];
    void *r = f48;

    if (r == 0)
    {
        extractDotName(name, buf);
        r = m40e1c0(buf);
    }

    return r;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040e770
void EzSubsystem::m40e770()
{
    switch (kind)
    {
    case 1:
        backend.m40dd10();
        break;
    case 0:
        m40e6d0();
        break;
    }
}

/* Free one sample through whichever backend owns it. Backend 0 asks the
   subsystem's own hook first and only goes on to the backend if that returned 1;
   an unknown backend reports success without doing anything.

   The result is SEEDED with the unknown-backend answer before the dispatch, so
   case 0 and the default share one `return r` tail - that is what puts `mov
   eax,1` above the switch and leaves the selector in ecx. Written with a
   trailing `return 1` instead, the default gets its own epilogue. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e620
int EzSubsystem::freeSample(void *sample)
{
    int r;

    if (sample == 0)
    {
        return 0;
    }

    r = 1;

    switch (kind)
    {
    case 1:
        return backend.m40e070(sample);
    case 0:
        r = s7(sample);

        if (r == 1)
        {
            r = backend.m40e0f0(sample);
        }

        break;
    }

    return r;
}


/* The backend's constructor, and the whole of its layout. The KLock at the end
   has no standalone code anywhere in the binary, so its body is written out by
   hand here - which also puts the CreateEventA before the memset, exactly
   where a member's constructor would run. The failure log's string is pinned
   in src/ksongplayerbase.cpp; one marker per address is enough.

   It has to stay out of the TU that calls it (EzSubsystem's own constructor,
   a STUB @0x40e670) or the call would be inlined away. */
// FUNCTION: EZ2AC_UNPACKED 0x0040da50
EzSubsystem::SubsystemBackend::SubsystemBackend()
{
    lockHandle = CreateEventA(0, 0, 1, 0);
    if (lockHandle == 0)
    {
        Log("KLock::KLock() Creating Handle Failed");
    }

    memset(buf, 0, sizeof(buf));
    fa000 = 0;
    fa004 = 0;
}

/* ~SubsystemBackend @0x40daa0 - the constructor run backwards, under its own
   lock. Take the event, wipe the table and both counters, release it, and
   only then close the handle - the close is guarded because the constructor
   logs and carries on when CreateEventA fails, so a backend can reach here
   with no handle at all.

   Same TU rule as the constructor: it must stay out of ~EzSubsystem or the
   call would be inlined away. */
// FUNCTION: EZ2AC_UNPACKED 0x0040daa0
EzSubsystem::SubsystemBackend::~SubsystemBackend()
{
    WaitForSingleObject(lockHandle, INFINITE);

    memset(buf, 0, sizeof(buf));
    fa000 = 0;
    fa004 = 0;

    SetEvent(lockHandle);

    if (lockHandle != 0)
    {
        CloseHandle(lockHandle);
        lockHandle = 0;
    }
}

/* EzSubsystem::SubsystemBackend::m40ddd0 @0x40ddd0 - the worker m40de30 hands
   its table hit to, so it runs with the lock already held and takes none of
   its own.

   Park `a` in the entry's first FREE slot, bumping that entry's own counter
   and the live count - the mirror of m40dea0 @0x40dea0, which searches the
   whole table for an entry whose first slot matches an owner where this one
   searches ONE entry's two slots for an empty one.

   The provably-dead `cmp edx,-1` after the search is the documented
   signature of an inlined helper that returns -1 on a miss: cl threads the
   miss straight to `return 0` but keeps the test on the hit path, and only a
   real `static` search reproduces both tails. Same reason findOwner exists in
   src/subsystembase.cpp.

   It has to stay out of that file, because m40de30 - its caller - lives
   there and cl would inline it away.

   91.43%, and the residue is three instructions of address arithmetic: the
   original forms `this + entry * 20` entirely in esi AFTER pushing it
   (`mov esi,[esp+0xc]` / `lea esi,[esi+esi*4]` / `lea esi,[ecx+esi*4]`),
   where cl reads `entry` into edx BEFORE the push and folds the last step to
   `lea esi,[ecx+edx*4]` - one instruction fewer in the same count, so it is
   the schedule and the register, not the expression. Four declaration orders
   of e/ok/i give the identical listing. */
struct Slot2b { void *p; int busy; };
struct Entry5 { int n; Slot2b slot[2]; };

/* The entry's first empty slot, or -1. */
static int findFreeSlot(Slot2b *s)
{
    int i;

    for (i = 0; i < 2; i++)
    {
        if (s[i].p == 0)
            return i;
    }

    return -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040ddd0
int EzSubsystem::SubsystemBackend::m40ddd0(int entry, int a)
{
    Entry5 *e;
    int ok;
    int i;

    ok = 0;
    if (a == 0)
        return ok;

    e = &((Entry5 *)buf)[entry];
    i = findFreeSlot(e->slot);
    if (i != -1)
    {
        e->slot[i].busy = 0;
        e->slot[i].p = (void *)a;
        e->n += 1;
        fa000 += 1;
        ok = 1;
    }

    return ok;
}
