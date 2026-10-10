#include "players.h"
#include "ezscreen.h"    /* EZ2DJMainGameDirector - what the slot's owner is */
#include "gamepanels.h"  /* Panel::m429910 */

/* PlayerSlot::m42ebf0 @0x42ebf0 - switch one of the player's forty options
   on or off, and apply the ones that act at once. On: the option word is set
   and, by option - 9/10 step the speed index up (to 20) or down (to 0) under
   the CV2 flag; 3 and 5 scramble the run (3 with the check); 4 and 0x17
   shuffle it in quarter slices (4 with the check); 0x16 mirrors it; 0x15
   rotates it once; anything else with option 7 up reskins the hits;
   otherwise 0x13/0x14 step the speed index down / up (to 33), 0x12 adds 5 to
   the life and 0x11 re-applies the tracks. Off: the word is cleared and
   0x13/0x14 put the speed index back to the stage's. Either way the panel
   hears about it. Every timed effect runs from the clock's now over the
   run period at +0x3e0 (x2 .. x10). Its own TU: the director's helpers and
   the panel's must not be inlined. */
struct OptionSlot
{
    char _pad0[0x474];
    struct { int a; int b; } opts[40];   /* 0x474 .. 0x5b4 */
    char _pad5b4[0xe105c0 - 0x5b4];
    int  speedBase;                      /* 0xe105c0 - the stage's speed index */
};
extern int g_1b2eb6c;    /* src/ez2acapp.cpp - the CV2 flag */

// FUNCTION: EZ2AC_UNPACKED 0x0042ebf0
void PlayerSlot::m42ebf0(int opt, int on)
{
    OptionSlot *me = (OptionSlot *)this;
    unsigned int now;

    if (on != 0)
    {
        me->opts[opt].a = 1;
        if (opt == 9)
        {
            if (g_1b2eb6c == 1)
            {
                f4e0++;
                if (f4e0 >= 0x15)
                    f4e0 = 0x14;
            }
        }
        else if (opt == 10)
        {
            if (g_1b2eb6c == 1)
            {
                f4e0--;
                if (f4e0 < 0)
                    f4e0 = 0;
            }
        }
        else if (opt == 3 || opt == 5)
        {
            now = ((SlotClock *)f3d8)->now();
            ((EZ2DJMainGameDirector *)f3d0)->m421c30((ListNode *)this, opt == 3, now + f3e0 * 2, now + f3e0 * 10);
        }
        else if (opt == 4)
        {
            now = ((SlotClock *)f3d8)->now();
            ((EZ2DJMainGameDirector *)f3d0)->m4219c0((ListNode *)this, now + f3e0 * 2, now + f3e0 * 10, 1);
        }
        else if (opt == 0x16)
        {
            now = ((SlotClock *)f3d8)->now();
            ((EZ2DJMainGameDirector *)f3d0)->reverseLanes((ListNode *)this, now + f3e0 * 2, now + f3e0 * 10);
        }
        else if (opt == 0x15)
        {
            now = ((SlotClock *)f3d8)->now();
            ((EZ2DJMainGameDirector *)f3d0)->rotateLanes((ListNode *)this, now + 1, ((EZ2DJMainGameDirector *)f3d0)->fd14);
        }
        else if (opt == 0x17)
        {
            now = ((SlotClock *)f3d8)->now();
            ((EZ2DJMainGameDirector *)f3d0)->m4219c0((ListNode *)this, now + f3e0 * 2, now + f3e0 * 10, 0);
        }
        else if (me->opts[7].a != 0)
        {
            now = ((SlotClock *)f3d8)->now();
            f3d0->randomizeHits((ListNode *)this, now + f3e0 * 2, now + f3e0 * 10);
        }
        else if (opt == 0x13)
        {
            f4e0--;
            if (f4e0 < 0)
                f4e0 = 0;
        }
        else if (opt == 0x14)
        {
            f4e0++;
            if (f4e0 >= 0x22)
                f4e0 = 0x21;
        }
        else if (opt == 0x12)
        {
            gauge += 5.0f;
        }
        else if (opt == 0x11)
        {
            now = ((SlotClock *)f3d8)->now();
            f3d0->applyTracks(this, now, now + f3e0 * 8);
        }
    }
    else
    {
        me->opts[opt].a = 0;
        if (opt >= 0x13 && opt <= 0x14)
            f4e0 = me->speedBase;
    }
    ((Panel *)f3cc)->m429910(opt, on);
}

/* PlayerRecord::m4662d0 @0x4662d0 - the scratch player's option switch, the
   keys one above with the scratch director's helpers, one extra timed effect
   (0x1d, the eight-measure shuffle without the check) and the rotate told
   the director's +0xca8. */
struct GFOptionSlot
{
    char _pad0[0x38];
    float gauge;                         /* 0x038 */
    char _pad3c[0x284 - 0x3c];
    struct { int a; int b; } opts[40];   /* 0x284 .. 0x3c4 - speed = pair 13's .b at 0x2f0 */
    char _pad3c4[0x534 - 0x3c4];
    int  speedBase;                      /* 0x534 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004662d0
void PlayerRecord::m4662d0(int opt, int on)
{
    GFOptionSlot *me = (GFOptionSlot *)this;
    unsigned int now;

    if (on != 0)
    {
        me->opts[opt].a = 1;
        if (opt == 9)
        {
            if (g_1b2eb6c == 1)
            {
                me->opts[13].b++;
                if (me->opts[13].b >= 0x15)
                    me->opts[13].b = 0x14;
            }
        }
        else if (opt == 10)
        {
            if (g_1b2eb6c == 1)
            {
                me->opts[13].b--;
                if (me->opts[13].b < 0)
                    me->opts[13].b = 0;
            }
        }
        else if (opt == 3 || opt == 5)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->m45ec70((ListNode *)this, opt == 3, now + f1f0 * 2, now + f1f0 * 10);
        }
        else if (opt == 4)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->m45e9f0((ListNode *)this, now + f1f0 * 2, now + f1f0 * 10, 1);
        }
        else if (opt == 0x1d)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->m45e770((ListNode *)this, now + f1f0 * 2, now + f1f0 * 10, 1);
        }
        else if (opt == 0x16)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->m45ed90((ListNode *)this, now + f1f0 * 2, now + f1f0 * 10);
        }
        else if (opt == 0x15)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->rotateLanes((ListNode *)this, now + 1, ((GFMainGameDirector *)f1e0)->fca8);
        }
        else if (opt == 0x17)
        {
            now = ((SlotClock *)f1e8)->now();
            ((GFMainGameDirector *)f1e0)->m45e9f0((ListNode *)this, now + f1f0 * 2, now + f1f0 * 10, 0);
        }
        else if (me->opts[7].a != 0)
        {
            now = ((SlotClock *)f1e8)->now();
            f1e0->randomizeHits((ListNode *)this, now + f1f0 * 2, now + f1f0 * 10);
        }
        else if (opt == 0x13)
        {
            me->opts[13].b--;
            if (me->opts[13].b < 0)
                me->opts[13].b = 0;
        }
        else if (opt == 0x14)
        {
            me->opts[13].b++;
            if (me->opts[13].b >= 0x22)
                me->opts[13].b = 0x21;
        }
        else if (opt == 0x12)
        {
            me->gauge += 5.0f;
        }
        else if (opt == 0x11)
        {
            now = ((SlotClock *)f1e8)->now();
            f1e0->applyTracks(this, now, now + f1f0 * 8);
        }
    }
    else
    {
        me->opts[opt].a = 0;
        if (opt >= 0x13 && opt <= 0x14)
            me->opts[13].b = me->speedBase;
    }
    ((GFPanel *)f1dc)->m462720(opt, on);
}
