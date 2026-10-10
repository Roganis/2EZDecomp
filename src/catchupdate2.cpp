/* CatchMainGameDirector::update2 @0x46e550 - the Ez2Catch director's tick.

   Hold off entirely until the pending screen reports ready, then start once
   and latch it. After that: pump the sound thread, quit on cabinet input 0,
   refresh the idle flag, count the hold-off down, rebase the sound list, and
   drive lamp output 4 from where the clock sits inside the measure - lit
   except in the first eight ticks and the last eight.

   The fade at +0x754 then ramps, and which way depends on the run: outside a
   CV2 run it falls by 50 a tick and the level is handed to the player's sink
   directly; inside one, with a single element up, it RISES by 50 and the
   sink is given (level - 255) * 10000 / 300 instead. Two elements just tear
   the player down. (The notify object is the KEZPlayer: slot 11 is its
   setMasterVolume, slot 10 its teardown - CATCH-REVIEW.md.) The level is clamped up to 0xff either way.

   The last instruction came from rebaseListB @0x46c020: the original sets ecx
   before calling it, so it is a MEMBER even though its body never touches
   `this` - which is exactly why it kept matching while declared as a free
   function, since a __thiscall member with no stack argument and a __cdecl
   free function compile to the same bytes. Declaring it on the class (the
   definition stays in src/accessors.cpp, unchanged) gives the call site its
   `mov ecx,esi` and the callee stays at 100%. Same fix as the GF twin's
   rebaseListC @0x45d1c0. */

#include "ezscreen.h"
#include "bigbase.h"     /* Obj40C180, g_bigInstance */
#include "players.h"
#include <windows.h>

struct Obj1B2E844c { virtual void q00(int v); };

/* The director's notify object, viewed for the four slots this tick uses.
   The dummies are what place them at +0x28, +0x2c, +0x58 and +0x6c. */
struct CatchNotify
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09();
    virtual void teardown();       /* slot 10 -> +0x28 - KEZPlayer::m410230 */
    virtual void setMasterVolume(int v); /* slot 11 -> +0x2c - KEZPlayer @0x40f3b0 */
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21();
    virtual unsigned int now();    /* slot 22 -> +0x58 */
    virtual void q23(); virtual void q24(); virtual void q25();
    virtual void q26();
    virtual int  busy();           /* slot 27 -> +0x6c */
};

extern void *g_1b2e844;     /* src/moderunner.cpp */
extern int   g_1b2ea28;     /* src/moderunner.cpp */
extern int   g_1b2eb6c;     /* src/ez2acapp.cpp */
extern int   g_1b2eba4;     /* 0x01b2eba4 (src/maingame.cpp) */
extern int   g_1b2e898;     /* src/maingame.cpp */
extern int   g_elemCount;   /* 0x1b2e718 (src/loops.cpp) */

extern unsigned int SatSub(unsigned int a, unsigned int b); /* 0x4145d0 */

// GLOBAL: EZ2AC_UNPACKED 0x01b2f08c
int g_1b2f08c;

// FUNCTION: EZ2AC_UNPACKED 0x0046e550
void CatchMainGameDirector::update2(int a, int /*b*/)
{
    DirectorPending *p;
    unsigned int now;
    int level;

    p = pending;
    if (p != 0)
    {
        if (p->ready == 0)
            return;
        if (bcac == 0)
        {
            g_1b2ea28 = 0;
            bcac = 1;
            start();
        }
    }

    pump.pump();

    if (g_bigInstance->getState(0) == 2)
    {
        ((Obj1B2E844c *)g_1b2e844)->q00(0);
        ClipCursor(0);
    }

    f780 = ((CatchSlotOwner *)this)->tracksIdle();

    if (fc0c > 0)
        fc0c -= 1;

    rebaseListB();

    /* Lit except in the first eight ticks of the measure and the last eight. */
    now = ((CatchNotify *)notify)->now() % (unsigned int)f334;
    if (f334 - 8 > (int)now && now > 8)
        g_bigInstance->setOutput(4, 0);
    else
        g_bigInstance->setOutput(4, 1);

    if (bd08 != 0)
        m46d360();

    if (g_1b2eb6c == 0)
    {
        if (g_1b2eba4 == 1)
        {
            g_1b2e898 += 1;
            f754 += -50;
            ((CatchNotify *)notify)->setMasterVolume(f754);
            ((CatchNotify *)notify)->teardown();
        }
    }
    else if (g_1b2eba4 == 1)
    {
        if (g_elemCount <= 1)
        {
            g_1b2e898 += 1;
            f754 += 50;
            ((CatchNotify *)notify)->setMasterVolume((f754 - 0xff) * 10000 / 300);
            ((CatchNotify *)notify)->teardown();
        }
        else
        {
            ((CatchNotify *)notify)->teardown();
            ((Obj1B2E844c *)g_1b2e844)->q00(0);
        }
    }

    level = 0xff;
    if (f754 >= level)
        f754 = level;

    if (((CatchNotify *)notify)->busy() == 0)
    {
        if (g_1b2eb6c != 0)
        {
            if (g_1b2eba4 != 1 || g_elemCount == 2 || g_1b2e898 >= 0x78)
                m46c240();
        }
        else if (g_1b2eba4 != 1 || g_1b2e898 >= 0x78)
        {
            m46c240();
        }
    }

    /* The caller's first argument, not a zero - the original pushes
       [esp+0xc] as SatSub's first operand. */
    g_1b2f08c = SatSub(a, started);
}
