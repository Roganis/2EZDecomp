/* EZ2DJSink3::v0 @0x4236f0, GFSink3::v0 @0x45fa50 and CatchSink3::v0
   @0x46ec00 - broadcast one key event to every node of the run's player
   list.

   The three differ in exactly three names: which list they walk (A for
   EZ2DJ, C for GF, B for catch - the same split the lane countdowns have),
   which per-node handler they call, and which director hook the acted-on
   keys go to. The EZ2DJ one also acts on two more keys than the other two,
   which is why its switch range is 31 wide against their 29.

   A key is only acted on when `what` is 2 or more, and only for the keys
   listed below; the LIST WALK happens either way. The original lowers the
   switch through a byte-index table, which is what a run of cases sharing
   one body compiles to - the range is 9..39 (9..37 for the other two) with
   everything else falling through to the default.

   `this` is the third sink, 0x18 into the director, hence the negative
   adjustment on the hook call.

   In its own TU: every callee is a STUB in src/stubs.cpp and the three list
   iterators live in src/nodelists.cpp. */

#include "players.h"
#include "ezscreen.h"

// FUNCTION: EZ2AC_UNPACKED 0x004236f0
void EZ2DJSink3::v0(int key, int what)
{
    ListNode *cur;

    if (what >= 2)
    {
        switch (key)
        {
        case 9: case 10: case 11: case 12:
        case 27: case 28:
        case 34: case 35: case 36: case 37: case 38: case 39:
            ((EZ2DJMainGameDirector *)((char *)this - 0x18))->m422f20(key, 1);
            break;
        }
    }

    cur = 0;

    while (nextListA(&cur) != 0)
    {
        ((PlayerSlot *)cur)->m42ee40(key, what);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0045fa50
void GFSink3::v0(int key, int what)
{
    ListNode *cur;

    if (what >= 2)
    {
        switch (key)
        {
        case 9: case 10: case 11: case 12:
        case 27: case 28:
        case 34: case 35: case 36: case 37:
            ((GFMainGameDirector *)((char *)this - 0x18))->m45f640(key, 1);
            break;
        }
    }

    cur = 0;

    while (nextListC(&cur) != 0)
    {
        ((PlayerRecord *)cur)->m466550(key, what);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ec00
void CatchSink3::v0(int key, int what)
{
    ListNode *cur;

    if (what >= 2)
    {
        switch (key)
        {
        case 9: case 10: case 11: case 12:
        case 27: case 28:
        case 34: case 35: case 36: case 37:
            ((CatchMainGameDirector *)((char *)this - 0x18))->m46e750(key, 1);
            break;
        }
    }

    cur = 0;

    while (nextListB(&cur) != 0)
    {
        ((CatchPlayer *)cur)->m473260(key, what);
    }
}
