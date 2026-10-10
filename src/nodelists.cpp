#include "players.h"

/* Three singly-linked global lists with an identical helper trio each.

   The nodes are only partly evidenced: a `next` at +4 (the iterator walks it)
   and an int at +8 (the getters return it). Nothing reads +0, so it is left
   as opaque padding.

   The iterate/count pair is the interesting bit: count is literally
   `while (next(&p)) n++;` with next() INLINED, which reproduces the original's
   odd-looking loop (the head is re-tested every iteration, and the two `n++`
   sites are the inlined function's two success returns). next() itself must be
   written with a SHARED `return 0` tail - `if (head != 0) { ...; return 1; }
   return 0;` - otherwise cl computes the last result with `setne` instead of
   branching to the common `mov eax,1` block. */

/* The shared list plumbing of the three player object kinds in
   include/players.h: whichever kind a list holds, the walkers only ever touch
   these fields. */
/* ListNode now lives in include/players.h - the result screens walk it too. */

// GLOBAL: EZ2AC_UNPACKED 0x01b2ef68
ListNode *g_listA;

// GLOBAL: EZ2AC_UNPACKED 0x01b2eb40
ListNode *g_listB;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ef6c
ListNode *g_listC;

/* Base of list A's two 0xE105C4-byte player slots. Placeholder storage - the
   stride is applied by hand rather than through sizeof, so 14 MB of unknown state
   does not have to be declared. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5fdd0
static char g_playerSlots[4];

static const int kPlayerSlotSize = 0xe105c4;

/* List B's two Ez2Catch players and list C's two records (include/players.h). */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f318
CatchPlayer g_catchPlayers[2];

// GLOBAL: EZ2AC_UNPACKED 0x037d9d50
static PlayerRecord g_players[2];

// FUNCTION: EZ2AC_UNPACKED 0x004182e0
int nextListA(ListNode **cur)
{
    if (g_listA != 0)
    {
        ListNode *p = *cur;
        if (p == 0)
        {
            *cur = g_listA;
            return 1;
        }

        p = p->next;
        *cur = p;
        if (p != 0)
            return 1;
    }

    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00418310
int countListA()
{
    int n = 0;
    ListNode *p = 0;

    while (nextListA(&p))
        n++;

    return n;
}

/* Returns the head's field at +8, or 0 when the list is empty. The null case
   is the INLINE arm (the original's bare `ret` right after the `jne`). */
// FUNCTION: EZ2AC_UNPACKED 0x00418340
int getListAF8()
{
    ListNode *p = g_listA;
    if (p == 0)
        return 0;

    return p->f8;
}

// FUNCTION: EZ2AC_UNPACKED 0x004183b0
int nextListB(ListNode **cur)
{
    if (g_listB != 0)
    {
        ListNode *p = *cur;
        if (p == 0)
        {
            *cur = g_listB;
            return 1;
        }

        p = p->next;
        *cur = p;
        if (p != 0)
            return 1;
    }

    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004183e0
int countListB()
{
    int n = 0;
    ListNode *p = 0;

    while (nextListB(&p))
        n++;

    return n;
}

// FUNCTION: EZ2AC_UNPACKED 0x00418470
int nextListC(ListNode **cur)
{
    if (g_listC != 0)
    {
        ListNode *p = *cur;
        if (p == 0)
        {
            *cur = g_listC;
            return 1;
        }

        p = p->next;
        *cur = p;
        if (p != 0)
            return 1;
    }

    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004184a0
int countListC()
{
    int n = 0;
    ListNode *p = 0;

    while (nextListC(&p))
        n++;

    return n;
}

// FUNCTION: EZ2AC_UNPACKED 0x004184d0
int getListCF8()
{
    ListNode *p = g_listC;
    if (p == 0)
        return 0;

    return p->f8;
}

/* The two per-round engine ticks EZ2AC::runGame @0x417e90 drives (and v4 before
   it). Both live HERE rather than with the other runGame callees because each
   INLINES the list walker defined above - that inlining is what produces the
   re-tested head and the duplicated store in the collect loop; from another TU
   the call would stay a call and nothing would match. */

/* src/stubs.cpp - declared, not defined, so cl cannot inline them away. */
extern void resetGlobals430b50();
extern void hook402880();

/* The per-node teardown the collect loop feeds. Its body compiled to nothing in
   the original, so the release loop leaves no code at all - but it is what keeps
   `n` live past the collect loop, and that is the whole reason the original
   indexes the array (`mov [esp+ecx*4],eax` / `add ecx,1`) instead of walking it
   with a pointer. */
static void releaseListNode(ListNode *)
{
}

/* Drain list A: collect every node into a 2-slot local array, release them, then
   drop the head. `p = 0;` must precede `n = 0;` - the init order is emitted
   verbatim as `xor eax,eax; xor ecx,ecx`. */
// FUNCTION: EZ2AC_UNPACKED 0x00418e60
void engineTick418e60()
{
    ListNode *taken[2];
    ListNode *p;
    int n;
    int i;

    resetGlobals430b50();

    p = 0;
    n = 0;
    while (nextListA(&p))
    {
        taken[n] = p;
        n++;
    }

    for (i = 0; i < n; i++)
    {
        releaseListNode(taken[i]);
    }

    g_listA = 0;
}

/* The same for list B, with the per-node action compiled out entirely: only the
   loop's induction survives. `p` and `n` must be initialized AFTER the hook call
   or their live ranges cross it and cl parks them in esi - a push/pop pair the
   original does not have. */
// FUNCTION: EZ2AC_UNPACKED 0x00418ee0
void engineTick418ee0()
{
    ListNode *nodes[2];

    hook402880();

    {
        ListNode *p = 0;
        int n = 0;
        int i;

        while (nextListB(&p))
        {
            nodes[n++] = p;
        }

        for (i = 0; i < n; i++)
        {
        }
    }

    g_listB = 0;
}

/* Find the list-A node whose id at +8 equals `key`, or 0. nextListA is INLINED
   again (hence the head re-test in ecx every iteration); the hit is the loop's
   fall-through and both misses share the out-of-line `xor eax,eax` tail, which is
   what writing the miss as the trailing `return 0` produces. */
// FUNCTION: EZ2AC_UNPACKED 0x00418e30
ListNode *findListA(int key)
{
    ListNode *p = 0;

    while (nextListA(&p))
    {
        if (p->f8 == key)
        {
            return p;
        }
    }

    return 0;
}

/* And list B's: the same search on the other head. */
// FUNCTION: EZ2AC_UNPACKED 0x00418eb0
ListNode *findListB(int key)
{
    ListNode *p = 0;

    while (nextListB(&p))
    {
        if (p->f8 == key)
        {
            return p;
        }
    }

    return 0;
}

/* ---- Joining and leaving the three lists -------------------------------------
   Each "add" resets the caller's player object and appends it to its list, unless
   a node with that id is already linked, in which case it returns 0 and leaves the
   object unlinked. In all three the EMPTY-LIST case is written FIRST so it lands
   inline (fall-through) with the walk out-of-line, matching the original; and the
   walk is the rotated form - id test first, then append-on-null-next - which is
   what makes cl load `next` once per iteration and reuse it as the cursor. The
   reset entry points live in src/stubs.cpp so the calls survive. */

// FUNCTION: EZ2AC_UNPACKED 0x00418260
ListNode *addToListA(int id)
{
    ListNode *p = (ListNode *)(g_playerSlots + id * kPlayerSlotSize);
    ListNode *q;

    ((PlayerSlot *)p)->reset(id);

    q = g_listA;
    if (q == 0)
    {
        g_listA = p;
        return p;
    }

    while (q->f8 != id)
    {
        if (q->next == 0)
        {
            q->next = p;
            return p;
        }

        q = q->next;
    }

    return 0;
}

/* Leave list A: clear the node's +0x228 and drop it from the head. Nobody uses
   the result, but the function DOES return one - writing it `void` swaps the
   register roles and stops matching. The single zero is reused three times
   (store, null test, unlink), which is where the original's `xor edx,edx` comes
   from. */
// FUNCTION: EZ2AC_UNPACKED 0x004182b0
ListNode *removeFromListA(ListNode *p)
{
    ListNode *head;

    p->f228 = 0;

    head = g_listA;
    if (head == 0)
    {
        return p;
    }

    if (p == head)
    {
        g_listA = p->next;
        return g_listA;
    }

    head->next = 0;
    return p;
}

// FUNCTION: EZ2AC_UNPACKED 0x00418350
CatchPlayer *addCatchPlayer(int id)
{
    CatchPlayer *n = &g_catchPlayers[id];
    n->reset(id);

    if (g_listB == 0)
    {
        g_listB = (ListNode *)n;
        return n;
    }

    {
        ListNode *p = g_listB;

        for (;;)
        {
            if (p->f8 == id)
            {
                return 0;
            }

            if (p->next == 0)
            {
                p->next = (ListNode *)n;
                return n;
            }

            p = p->next;
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00418410
PlayerRecord *addPlayer(int id)
{
    PlayerRecord *p = &g_players[id];
    PlayerRecord *q;

    p->reset(id);

    q = (PlayerRecord *)g_listC;
    if (q == 0)
    {
        g_listC = (ListNode *)p;
        return p;
    }

    while (q->id != id)
    {
        if (q->next == 0)
        {
            q->next = p;
            return p;
        }

        q = q->next;
    }

    return 0;
}

/* The third per-round drain, for list C - the same collect-then-drop shape as the
   other two, minus the hook call: nextListC is INLINED (the head stays in a
   register because nothing in the loop calls out, so it is re-TESTED rather than
   re-read), and the trailing empty loop is what keeps `n` live and so keeps the
   collect loop's stores indexed. */
// FUNCTION: EZ2AC_UNPACKED 0x00418f60
void engineTick418f60()
{
    ListNode *nodes[2];
    ListNode *p = 0;
    int n = 0;
    int i;

    while (nextListC(&p))
    {
        nodes[n++] = p;
    }

    for (i = 0; i < n; i++)
    {
    }

    g_listC = 0;
}

/* Find the list-C record with this id - the same inlined-walker shape as
   findListA above, on the other list. */
// FUNCTION: EZ2AC_UNPACKED 0x00418f30
PlayerRecord *findPlayer(int id)
{
    ListNode *p = 0;

    while (nextListC(&p))
    {
        if (p->f8 == id)
        {
            return (PlayerRecord *)p;
        }
    }

    return 0;
}
