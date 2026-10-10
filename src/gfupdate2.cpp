/* GFMainGameDirector::update2 @0x45f350 - the scratch director's tick, and
   CatchMainGameDirector::update2 @0x46e550's twin (src/catchupdate2.cpp).

   Same spine: hold off until the pending screen reports ready, start once and
   latch it, pump the sound thread, quit on cabinet input 0, refresh the idle
   flag, count the hold-off down, rebase the sound list, and drive lamp output
   4 off where the clock sits inside the measure. Then the credit overlay's
   fade ramps, one way outside a CV2 run and the other inside it, and the
   whole thing ends by publishing the elapsed time.

   What it does differently:

     - the lamp is lit for the FIRST THIRD of the measure, where the catch
       copy lights it except in the first eight ticks and the last eight;
     - the latched start flag is also published to 0x1b2ec14 every tick;
     - the lane countdowns are ticked here (m45f1a0 @0x45f1a0) when the
       g_1b5f2e0 option is up;
     - the demo-play branch is richer: with credits in hand it fades at once,
       otherwise it waits twenty seconds from the start before doing so, and
       with demo play off it runs m45d200 instead;
     - the non-CV2 fade is skipped when list C holds exactly two players;
     - and the two panels' +0x30 are copied into the pending screen at the
       end - TWICE, in two identical guarded blocks the original writes out
       rather than sharing.

   THE ZERO REGISTER WAS NOT A COIN - IT WAS THE `a` PARAMETER. This was
   parked at 76.96% reading as the constant-register wall: cl materialized 0
   in ebp and spelled every zero test and push against it (`cmp eax,ebp`,
   `push ebp`, `mov [g_1b2ea28],ebp`) where the original uses immediates and
   keeps only its 1 in ebx, at the cost of an extra `push ebp`. What the
   prologue actually says is that the original saves ebx and esi ONLY, and
   uses edi as a mid-function scratch it pops before the tail - so the
   question was never "why does cl want a zero register", it was "what is
   cl doing with the TWO callee-saved registers the original does not have".
   The answer: it caches `a` in edi for the whole function, where the
   original re-reads it from its home slot at both SatSub calls
   (`[esp+0x10]` at depth 3, `[esp+0xc]` at depth 2 after the pop). Reading
   it through a volatile alias at both uses frees edi, cl then uses edi for
   the notify vptr exactly as the original does, and the zero register goes
   with it: 76.96% -> 92.7% by mnemonic, with the instruction counts equal.
   So: when cl takes a callee-saved register the original does not have,
   identify what is IN it before believing the constant-register coin - the
   competitor is the finding.

   The rest was the demo-play block order. The original lays `fca4 = 4`
   inline and the twenty-second wait out of line, ending it with a BACKWARD
   jump into the shared m450420 call. Swapping the arms alone does nothing
   (cl re-inverts it, because the other arm holds the early exit); writing
   the shared tail out in BOTH arms is what does it - cl's merger folds the
   second copy into the first and produces the backward jump itself. With
   the `fca4 = 4` arm written first that is 100%; with the arms the other
   way round, or with the early exit hoisted into an `&&` guard, it is
   91.2% and two or three instructions long.

   One thing that WAS fixable: rebaseListC @0x45d1c0 is a MEMBER, not the
   free function src/accessors.cpp had it as - the original sets ecx before
   calling it, which only a member call does, and the body is identical
   either way. Same finding the catch twin records about rebaseListB
   @0x46c020 and declines to act on; acting on it is worth a third of a
   point here and keeps the callee at 100%. */

#include "ezscreen.h"
#include "bigbase.h"
#include "players.h"

struct Obj1B2E844g { virtual void q00(int v); };

/* The director's notify object, viewed for the four slots this tick uses. */
struct GFNotify
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09();
    virtual void close();          /* slot 10 -> +0x28 */
    virtual void setLevel(int v);  /* slot 11 -> +0x2c */
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21();
    virtual unsigned int now();    /* slot 22 -> +0x58 */
    virtual void q23(); virtual void q24(); virtual void q25();
    virtual void q26();
    virtual int  busy();           /* slot 27 -> +0x6c */
};

/* The pending screen, and the two panels whose +0x30 it is given. */
struct GFPending { char _pad0[0x14]; int f14; int f18; char _pad1c[0x2d - 0x1c]; char ready; };
struct GFCounter { char _pad0[0x2c]; int f2c; int f30; };

extern void *g_1b2e844;     /* src/moderunner.cpp */
extern int   g_1b2ea28;     /* src/moderunner.cpp */
extern int   g_1b2eb6c;     /* src/ez2acapp.cpp */
extern int   g_1b2eba4;     /* 0x01b2eba4 (src/maingame.cpp) */
extern int   g_1b2e898;     /* src/maingame.cpp */
extern int   g_elemCount;   /* 0x1b2e718 (src/loops.cpp) */
extern int   g_1b5f2e0;     /* src/holdtick.cpp */
extern int   g_demoPlay;    /* 0x1b2ef78 (src/panels.cpp) */
extern int   g_1b2f08c;     /* src/catchupdate2.cpp */
extern Obj40C180 *g_bigInstance;                        /* 0x1b2e710 */
extern int  countListC();                               /* 0x4184a0 */
extern unsigned int SatSub(unsigned int a, unsigned int b); /* 0x4145d0 */

/* The latched start flag, republished every tick. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ec14
int g_1b2ec14;

// FUNCTION: EZ2AC_UNPACKED 0x0045f350
void GFMainGameDirector::update2(int a, int /*b*/)
{
    GFPending *p;
    unsigned int now;
    int level;

    p = (GFPending *)f32c;
    if (p != 0)
    {
        if (p->ready == 0)
            return;
        if (b330 == 0)
        {
            g_1b2ea28 = 0;
            b330 = 1;
            start();
        }
    }

    g_1b2ec14 = (unsigned char)b330;
    pump.pump();

    if (g_bigInstance->getState(0) == 2)
        ((Obj1B2E844g *)g_1b2e844)->q00(0);

    f784 = ((GFSlotOwner *)this)->tracksIdle();

    if (fc10 > 0)
        fc10 -= 1;

    rebaseListC();

    if (g_1b5f2e0 == 1)
        m45f1a0();

    /* Lit for the first third of the measure. */
    now = ((GFNotify *)notify)->now() % (unsigned int)f318;
    if ((unsigned int)f318 / 3 > now)
        g_bigInstance->setOutput(4, 1);
    else
        g_bigInstance->setOutput(4, 0);

    if (g_demoPlay != 0)
    {
        if (g_bigInstance->have != 0)
        {
            fca4 = 4;
            /* +0x10 is inside the base sub-objects, so it is reached by cast. */
            f158->m450420(2, 10, (char *)this + 0x10);
            f20 = 1;
        }
        else
        {
            if (SatSub(*(volatile int *)&a, started) < 0x4e20)
                goto ramp;
            fca4 = 0;
            f158->m450420(2, 10, (char *)this + 0x10);
            f20 = 1;
        }
    }
    else
    {
        m45d200();
    }

ramp:
    if (g_1b2eb6c == 0)
    {
        if (g_1b2eba4 == 1 && countListC() != 2)
        {
            g_1b2e898 += 1;
            f754 += -50;
            ((GFNotify *)notify)->setLevel(f754);
            ((GFNotify *)notify)->close();
        }
    }
    else if (g_1b2eba4 == 1)
    {
        if (g_elemCount <= 1)
        {
            g_1b2e898 += 1;
            f754 += 50;
            ((GFNotify *)notify)->setLevel((f754 - 0xff) * 10000 / 300);
            ((GFNotify *)notify)->close();
        }
        else
        {
            ((GFNotify *)notify)->close();
            ((Obj1B2E844g *)g_1b2e844)->q00(0);
        }
    }

    level = 0xff;
    if (f754 >= level)
        f754 = level;

    if (((GFNotify *)notify)->busy() == 0)
    {
        if (g_1b2eb6c != 0)
        {
            if (g_1b2eba4 != 1 || g_elemCount == 2 || g_1b2e898 >= 0x78)
                m45d8d0();
        }
        else if (g_1b2eba4 != 1 || g_1b2e898 >= 0x78)
        {
            m45d8d0();
        }
    }

    if (f32c != 0 && countListC() != 1)
    {
        ((GFPending *)f32c)->f14 = ((GFCounter *)f340[(0x470 - 0x340) / 4])->f30;
        ((GFPending *)f32c)->f18 = ((GFCounter *)f340[(0x634 - 0x340) / 4])->f30;
    }
    if (f32c != 0 && countListC() != 1)
    {
        ((GFPending *)f32c)->f14 = ((GFCounter *)f340[(0x470 - 0x340) / 4])->f30;
        ((GFPending *)f32c)->f18 = ((GFCounter *)f340[(0x634 - 0x340) / 4])->f30;
    }

    g_1b2f08c = SatSub(*(volatile int *)&a, started);
}

extern int g_1b2ebd0;   /* 0x01b2ebd0 (src/gamestate.cpp) */

/* The GF run's beat nudge @0x45d7c0 - add up whichever of the two counters
   are armed (+0x340 gates the first, +0x504 the second), take the sum's low
   digit modulo three, and move the shared value at 0x1b2ebd0 by that minus
   one, so it drifts by -1, 0 or +1 a call. Both counters are read through
   the same +0x2c the panel readouts use at +0x30. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d7c0
void GFMainGameDirector::m45d7c0()
{
    int v = 0;

    if (f340[0] != 0)
        v = ((GFCounter *)f340[(0x470 - 0x340) / 4])->f2c;
    if (f340[(0x504 - 0x340) / 4] != 0)
        v += ((GFCounter *)f340[(0x634 - 0x340) / 4])->f2c;

    g_1b2ebd0 += v % 10 % 3 - 1;
}
