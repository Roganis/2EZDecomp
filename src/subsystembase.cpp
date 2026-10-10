/* SubsystemBase's constructor, alone in its own TU.

   It has to be here rather than beside Obj40D4E0: with the body visible cl
   inlines the whole thing into the derived constructor and the `call` the
   original makes disappears. Same rule as EzObject::EzObject and KBase::KBase.
   */

#include <string.h>
#include "obj40d4e0.h"
#include "ez2acapp.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040d0f0
SubsystemBase::SubsystemBase()
{
    f04 = 0;
    f08 = 0;
    f0c = 0;
    f10 = 0;
    memset(f14, 0, sizeof(f14));
    memset(f26, 0, sizeof(f26));
}

/* EzSubsystem::SubsystemBackend's two guarded table scans, @0x40df20 and
   @0x40df90. The table is 0x800 entries of stride 0x14 starting at +4 - i.e.
   immediately after the first dword of `buf` - and each entry holds two slots,
   at +0 and +8. nextUsed walks the ENTRIES looking at slot 0; nextUsedIn walks
   the two SLOTS of one entry. Both take the cursor by pointer, leave it on the
   slot they return, and hand back 0 when nothing is left.

   The lock is `WaitForSingleObject(h, INFINITE)` / `SetEvent(h)`, not a mutex
   pair: the constructor @0x40da50 builds the handle with
   `CreateEventA(0, 0, 1, 0)` - an auto-reset event, initially signalled - and
   logs "KLock::KLock()" on failure, so the wait acquires and SetEvent
   releases.

   Identifying those two imports needed a detour worth recording. Both go
   through IAT slots (0x48709c, 0x4870a0) whose stored name RVAs - 0x33e3208
   and 0x33e321e - fall OUTSIDE the 0xb7000-byte dump, so they cannot be read
   the way every other import can. What settles it is that the two RVAs are
   0x16 apart, and the strings `WaitForSingleObject` and `SetEvent` sit in the
   file at 0x4b120a and 0x4b1220 - hint/name entries at 0x4b1208 and 0x4b121e,
   also 0x16 apart, with the same low bytes 0x208 and 0x21e. The packer moved
   the name table wholesale; the offsets within it survived.

   The table is addressed off `buf` rather than carved into its own member:
   the constructor memsets `buf` with `sizeof`, so changing its extent would
   move that call's argument and break a matched function. */

// FUNCTION: EZ2AC_UNPACKED 0x0040df20
void *EzSubsystem::SubsystemBackend::nextUsed(int *cur)
{
    struct Slot  { void *p; int _pad4; };
    struct Entry { Slot slot[2]; int _pad10; };
    void *r = 0;
    int   i;

    WaitForSingleObject(lockHandle, INFINITE);

    i = *cur + 1;
    if (i < 0x800)
    {
        Entry *e = &((Entry *)&buf[1])[i];

        do
        {
            r = e->slot[0].p;
            if (r != 0)
            {
                *cur = i;
                break;
            }
            i++;
            e++;
        }
        while (i < 0x800);
    }

    SetEvent(lockHandle);
    return r;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040df90
void *EzSubsystem::SubsystemBackend::nextUsedIn(int idx, int *cur)
{
    struct Slot  { void *p; int _pad4; };
    struct Entry { Slot slot[2]; int _pad10; };
    void *r = 0;
    int   i;

    WaitForSingleObject(lockHandle, INFINITE);

    i = *cur + 1;
    if (i < 2)
    {
        Slot *p = &((Entry *)&buf[1])[idx].slot[i];

        do
        {
            r = p->p;
            if (r != 0)
            {
                *cur = i;
                break;
            }
            i++;
            p++;
        }
        while (i < 2);
    }

    SetEvent(lockHandle);
    return r;
}

/* The sample seen through its vtable - EzSample itself is modelled without
   one (its `name` sits at +0x26, so declaring virtuals there would shift
   every matched user), so the two things this function needs are reached
   through a view and a cast, which cost no code. */
struct SubView
{
    virtual void q0();
    virtual void release(int flag);    /* slot 1 -> +0x04 */
};

struct SampleView
{
    virtual void q0();  virtual void q1();  virtual void q2();
    virtual void q3();  virtual void q4();
    virtual void s5();                 /* slot 5  -> +0x14 */
    virtual void s6(int a);            /* slot 6  -> +0x18 */
    virtual void q7();
    virtual void s8(int a);            /* slot 8  -> +0x20 */
    virtual void q9();
    virtual void q10(int v);           /* slot 10 -> +0x28 */

    int          f4;                   /* 0x04 - bit 0 is the kind gate */
    SubView     *f8;                   /* 0x08 */
    int          fc;                   /* 0x0c */
    unsigned int f10;                  /* 0x10 - what the pick minimises */
    char         _pad14[0x16 - 0x14];
    short        f16;                  /* 0x16 - matched against the caller's */
    int          f18;                  /* 0x18 */
    char         _pad1c[0xa8 - 0x1c];
    int          fa8;                  /* 0xa8 */
};

/* Find the entry whose first slot belongs to `owner`, or -1. A separate
   helper because the caller keeps the provably-dead `i == -1` test the
   original emits - the signature of an inlined search that returns -1 on a
   miss (see the cookbook rule). */
static int findOwner(struct SlotEntry *base, void *owner);

struct Slot2 { void *p; int busy; };
struct SlotEntry { Slot2 slot[2]; int _pad10; };

static int findOwner(SlotEntry *base, void *owner)
{
    int i;

    for (i = 0; i < 0x800; i++)
    {
        if (base[i].slot[0].p == owner)
            return i;
    }

    return -1;
}

/* EzSubsystem::SubsystemBackend::m40db00 @0x40db00 - hand out the owner's
   SECOND slot.

   Under the lock, find the entry whose first slot belongs to `owner`; if its
   second slot holds an object that nobody has taken, claim it, copy the
   owner's +0xa8 across and tell it through slot 10, then bump the handed-out
   counter. Anything missing - no entry, or the slot busy or empty - hands
   back 0. The second-slot walk is written as a loop from 1, which is what
   the original's `mov ecx,1` / `cmp ecx,2` say for a single iteration.

   The last defect (96.72%) was the slot address computed in ecx where the
   original computes it destructively in eax (`lea eax,[eax+eax*4]` /
   `lea eax,[esi+eax*4+0xc]`). Three spellings of the address left it; what
   moved it was the LOOP: `k = 1` set ahead of the pointer and a `while`
   with the increment at the bottom, not a `for` (why that moves the
   address into eax is not understood). Found by scratchpad/perm.

   Two earlier levers landed too, worth 54 points between them: the second slot is
   reached by a WALKING pointer anchored at `slot[1]` (indexing `slot[k]`
   makes cl keep the entry base and scale k into the addressing mode), and
   the call's argument is the field READ BACK from the object rather than the
   local that was just stored into it - which is what puts the push ahead of
   the store and the value in eax. */
// FUNCTION: EZ2AC_UNPACKED 0x0040db00
EzSample *EzSubsystem::SubsystemBackend::m40db00(EzSample *owner)
{
    EzSample *r;
    int i;
    int k;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    r = 0;
    i = findOwner((SlotEntry *)&buf[1], owner);
    if (i != -1)
    {
        /* An explicit WALKING pointer anchored at the second slot, which is
           what the original's `lea eax,[esi+i*20+0xc]` and `add eax,8` are;
           indexing `e->slot[k]` instead makes cl keep the entry base and
           scale k into the addressing mode. */
        k = 1;
        Slot2 *s = &((SlotEntry *)&buf[1])[i].slot[1];

        while (k < 2)
        {
            if (s->busy == 0 && s->p != 0)
            {
                r = (EzSample *)s->p;
                s->busy = 1;
                ((SampleView *)r)->fa8 = ((SampleView *)owner)->fa8;
                ((SampleView *)r)->q10(((SampleView *)r)->fa8);
                fa004++;
                break;
            }

            s++;
            k++;
        }
    }

    SetEvent((HANDLE)lockHandle);
    return r;
}

/* EzSubsystem::SubsystemBackend::m40dd10 @0x40dd10 - release every claimed
   slot in the table.

   One pass over all 0x800 entries and both of each entry's slots. A slot
   whose busy flag is exactly 1 is un-marked, and if it still holds an object
   that object is wound down in order: its +0x18 is saved, slot 5 stops it,
   slots 6 and 8 are handed a zero, slot 10 gets the saved value back and it
   is written to +0xa8 as well, and finally the sub-object at +8 is released
   with a 1 and forgotten. The handed-out counter is zeroed at the end.

   The object is RE-READ from the slot before every one of those calls - any
   of them could swap it - which is why the body works through the member
   rather than through a local. The one exception is the first call, which
   reuses the value the null test already loaded because nothing intervenes.

   The outer walk is memory-resident: pointer and counter both live in the
   frame, which is what the original's `mov esi,[esp+0x10]` / `sub [esp+0x14],1`
   pair says under this much register pressure. */
// FUNCTION: EZ2AC_UNPACKED 0x0040dd10
void EzSubsystem::SubsystemBackend::m40dd10()
{
    SlotEntry *e;
    int n;
    int k;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    e = (SlotEntry *)&buf[1];
    n = 0x800;
    do
    {
        Slot2 *s = e->slot;

        k = 2;
        do
        {
            if (s->busy == 1)
            {
                SampleView *p = (SampleView *)s->p;

                s->busy = 0;
                if (p != 0)
                {
                    int v = p->f18;

                    p->s5();
                    ((SampleView *)s->p)->s6(0);
                    ((SampleView *)s->p)->s8(0);
                    ((SampleView *)s->p)->q10(v);
                    ((SampleView *)s->p)->fa8 = v;

                    /* A NAMED local: the original loads the sub-object once
                       and calls straight through it (`mov ecx,[edx+8]` then
                       `call [eax+4]`). Re-reading the member after the null
                       test costs an extra `mov ecx,eax`. */
                    SubView *q = ((SampleView *)s->p)->f8;

                    if (q != 0)
                    {
                        q->release(1);
                        ((SampleView *)s->p)->f8 = 0;
                    }
                }
            }

            s++;
        }
        while (--k != 0);

        e++;
    }
    while (--n != 0);

    fa004 = 0;
    SetEvent((HANDLE)lockHandle);
}

/* EzSubsystem::SubsystemBackend::m40dbc0 @0x40dbc0 - claim the best free slot.

   Two different searches over the same 0x800 entries, chosen by bit 0 of
   `flags`. With the bit set, take the FIRST free slot whose object has its
   own bit 0 set and stop looking. Without it, take the free slot whose
   object is NOT gated, whose +0x16 equals `kind`, and whose +0x10 is the
   lowest one at or above `floor` - so it is a minimising pick over the whole
   table, and it runs to the end.

   `best` starts at 0xffffffff and the comparisons are UNSIGNED, which is
   what makes the first candidate always win; the -1 that seeds it is the
   same -1 the wait was given, and cl keeps it in one register that then
   doubles as the "nothing found" index.

   Claiming marks the slot busy, hands the object `floor` at +0x0c and `arg`
   at +0xa8, tells it through slot 10, and bumps the handed-out counter. */
// FUNCTION: EZ2AC_UNPACKED 0x0040dbc0
EzSample *EzSubsystem::SubsystemBackend::m40dbc0(unsigned int floor, short kind,
                                                 int arg, int flags)
{
    EzSample    *r;
    SlotEntry   *e;
    unsigned int best;
    int          hit;
    int          i;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    r = 0;
    best = 0xffffffff;
    hit = -1;
    /* The counter is zeroed BEFORE the walking pointer is built - that is
       what puts the original's `xor ecx,ecx` ahead of its `lea edx,[esi+4]`.
       Left in the for-init, cl emits the lea first. */
    i = 0;
    e = (SlotEntry *)&buf[1];

    for (; i < 0x800; i++)
    {
        SampleView *p;

        if (e->slot[0].busy == 0)
        {
            p = (SampleView *)e->slot[0].p;

            if (p != 0)
            {
                if ((flags & 1) != 0)
                {
                    if ((p->f4 & 1) != 0)
                    {
                        hit = i;
                        break;
                    }
                }
                else if ((p->f4 & 1) == 0 && p->f16 == kind)
                {
                    unsigned int v = p->f10;

                    if (v >= floor && v < best)
                    {
                        best = v;
                        hit = i;
                    }
                }
            }
        }

        e++;
    }

    if (hit != -1)
    {
        Slot2 *s = &((SlotEntry *)&buf[1])[hit].slot[0];

        r = (EzSample *)s->p;
        s->busy = 1;
        ((SampleView *)r)->fc = floor;
        ((SampleView *)r)->fa8 = arg;
        ((SampleView *)r)->q10(((SampleView *)r)->fa8);
        fa004++;
    }

    SetEvent((HANDLE)lockHandle);
    return r;
}

/* The record @0x40dca0 tears down - the same two words as Slot2, seen by the
   name src/accessors.cpp gives them. */
struct Rec40DCA0 { void *obj; int kind; };

/* The table seen from ITS OWN base rather than from the entry array: each
   0x14-byte record is a count followed by its two slots, so what nextUsed
   calls `_pad10` of entry i is the count of entry i+1. Both readings give
   the same addresses; this one is what m40e0f0's `addl $-1,(%eax,%esi,1)`
   needs, and it costs nothing to view it either way. */
struct TableEntry { int count; Slot2 slot[2]; };

/* Which of an entry's two slots holds `p`, or -1. Its own function because
   the caller keeps a provably-dead `cmp eax,-1` on the hit path. */
static int findSlotIn(Slot2 *base, void *p)
{
    Slot2 *s;
    int    k;

    /* `k = 0` first, then the pointer copy - the original's `xor eax,eax`
       precedes its `mov ecx,esi`. */
    for (k = 0, s = base; k < 2; k++)
    {
        if (s->p == p)
            return k;

        s++;
    }

    return -1;
}

/* EzSubsystem::SubsystemBackend::findSlotOf @0x40e000 - where in the table
   `sample` sits. Hands back the entry and slot indices and 1, or -1/-1 and
   0 if it is nowhere. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e000
int EzSubsystem::SubsystemBackend::findSlotOf(void *sample, int *entryOut,
                                              int *slotOut)
{
    SlotEntry *e;
    int i;

    /* Counter zeroed before the walking pointer is built - the same ordering
       lever m40dbc0 needed. */
    i = 0;
    e = (SlotEntry *)&buf[1];

    for (; i < 0x800; i++)
    {
        int k = findSlotIn(e->slot, sample);

        if (k != -1)
        {
            *entryOut = i;
            *slotOut = k;
            return 1;
        }

        e++;
    }

    *entryOut = -1;
    *slotOut = -1;
    return 0;
}

/* EzSubsystem::SubsystemBackend::m40e070 @0x40e070 - wind the sample's record
   down where it stands, leaving it in the table. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e070
int EzSubsystem::SubsystemBackend::m40e070(void *sample)
{
    int r = 0;
    int entry;
    int slot;

    if (sample == 0)
        return 0;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    if (findSlotOf(sample, &entry, &slot) == 1)
    {
        if (release((Rec40DCA0 *)&((TableEntry *)buf)[entry].slot[slot]) == 1)
        {
            fa004--;
            r = 1;
        }
    }

    SetEvent((HANDLE)lockHandle);
    return r;
}

/* EzSubsystem::SubsystemBackend::m40e0f0 @0x40e0f0 - forget the sample's
   record outright. Unlike m40e070 nothing is told: the slot is simply
   emptied and both the entry's own count and the table-wide one drop. The
   answer is 1 whenever the sample was FOUND, even if its slot was already
   empty. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e0f0
int EzSubsystem::SubsystemBackend::m40e0f0(void *sample)
{
    int r = 0;
    int entry;
    int slot;

    if (sample == 0)
        return 0;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    if (findSlotOf(sample, &entry, &slot) == 1)
    {
        /* ONE named pointer, to the slot. Naming the entry as well makes cl
           fold `this` into the shared base and re-derive the slot address at
           every store; with only the slot named it keeps the byte offset in
           a register, adds `this` in the addressing modes, and stores twice
           through the one `lea` - which is the original. */
        Slot2 *s = &((TableEntry *)buf)[entry].slot[slot];

        if (s->p != 0)
        {
            s->p = 0;
            s->busy = 0;
            ((TableEntry *)buf)[entry].count--;
            fa000--;
        }

        r = 1;
    }

    SetEvent((HANDLE)lockHandle);
    return r;
}

/* EzSubsystem::SubsystemBackend::m40de30 @0x40de30 - hand `owner`'s entry to
   the worker.

   Under the lock, find the entry whose first slot belongs to `owner` and let
   the worker @0x40ddd0 act on it; a miss hands back 0. The provably-dead
   `if (i == -1)` on the hit path is the same inlined-search tell m40db00 has.

   THE RESULT MUST BE ASSIGNED, NOT RETURNED, and the release written ONCE.
   wip/mutexlookup.cpp sat at 42.35% with every instruction read correctly,
   because writing the miss as `{ SetEvent(...); return r; }` lets cl fold `r`
   to a constant at that return - it can prove the variable is still 0 there -
   and the zero then never reaches a register.  One tail that cl DUPLICATES
   into both paths keeps it a variable, so both copies come out `mov eax,edi`
   against the edi zeroed before the wait.  (The cookbook's "cl duplicates a
   shared tail AFTER constant folding" rule; the park predates it.) */
// FUNCTION: EZ2AC_UNPACKED 0x0040de30
int EzSubsystem::SubsystemBackend::m40de30(void *owner, int a)
{
    int r;
    int i;

    r = 0;
    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    i = findOwner((SlotEntry *)&buf[1], owner);
    if (i != -1)
        r = m40ddd0(i, a);

    SetEvent((HANDLE)lockHandle);
    return r;
}

/* EzSubsystem::SubsystemBackend::m40dea0 @0x40dea0 - park `p` in a free entry.

   A null pointer is refused before the lock is even taken.  Otherwise the
   same 0x800-entry scan runs looking for a FREE first slot - findOwner with
   a null key - and the hit gets `p` stored, its busy flag cleared, its own
   counter bumped and the live count raised.

   The single zero register does four jobs here (the null test, the search
   key, the cleared flag and the result), which is what earns it edi. */
// FUNCTION: EZ2AC_UNPACKED 0x0040dea0
int EzSubsystem::SubsystemBackend::m40dea0(void *p)
{
    /* The entry as the STORES see it, one dword BELOW the `p` field the scan
       walks: the count that leads it is what `add [esi+i*20],1` bumps. */
    struct Entry { int n; void *p; int busy; void *p2; int busy2; };
    Entry *e;
    int ok;
    int i;

    ok = 0;
    if (p == 0)
        return ok;

    WaitForSingleObject((HANDLE)lockHandle, INFINITE);

    i = findOwner((SlotEntry *)&buf[1], 0);
    if (i != -1)
    {
        e = &((Entry *)buf)[i];
        e->n += 1;
        e->busy = 0;
        e->p = p;
        fa000++;
        ok = 1;
    }

    SetEvent((HANDLE)lockHandle);
    return ok;
}
