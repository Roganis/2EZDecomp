/* EzSubsystem::m40e6d0 @0x40e6d0 - hand every live sample to slot 7, then
   wipe the backend's table under its lock.

   The walk is the backend's two-level cursor: nextUsed advances the outer
   cursor to the next entry that has anything in it, nextUsedIn walks the two
   slots WITHIN that entry, and the inner cursor is reset for each entry.
   Both take their cursor by pointer and hand back 0 when nothing is left,
   so the pair reads as a nested loop with no index arithmetic here at all.

   The clear at the end goes through the backend's own table-clear @0x40da30,
   which is why it is reached by casting the member rather than by a method
   of its own. The lock around it is the same WaitForSingleObject/SetEvent
   pair the two cursor helpers take internally - an auto-reset event, so the
   wait acquires and SetEvent releases. */

#include <windows.h>
#include "ez2acapp.h"

/* src/accessors.cpp - the backend's 0xa008-byte table and its clear. */
struct Obj40DA30
{
    int buf[0x2800];
    int fa000;
    int fa004;
    void clear();
};

// FUNCTION: EZ2AC_UNPACKED 0x0040e6d0
void EzSubsystem::m40e6d0()
{
    SubsystemBackend *b = &backend;
    int   cur = -1;
    int   sub;
    void *e;

    while ((e = b->nextUsed(&cur)) != 0)
    {
        s7(e);
        sub = 0;
        e = b->nextUsedIn(cur, &sub);

        while (e != 0)
        {
            s7(e);
            e = b->nextUsedIn(cur, &sub);
        }
    }

    WaitForSingleObject((HANDLE)b->lockHandle, INFINITE);
    ((Obj40DA30 *)b)->clear();
    SetEvent((HANDLE)b->lockHandle);
}
