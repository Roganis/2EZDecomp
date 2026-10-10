/* The three modes' pan stepper - EZ2DJMainGameDirector::stepPan @0x420400,
   GFMainGameDirector::stepPan @0x45d690 and CatchMainGameDirector::stepPan
   @0x46c190. All three are BYTE-IDENTICAL to each other (100.0% by mnemonic
   sequence); the original's ICF did not fold them because each addresses its
   own class's lock field and tail-calls its own level mapper.

   Move the shared pan at 0x1b2e91c seven steps in `dir`'s direction and
   clamp it to [0, 0x100], then re-apply it. The interesting part is the
   CENTRE DETENT: if the step would carry the pan across 0x80, it is snapped
   to 0x80 instead and a fifteen-frame lock is set, during which the stepper
   refuses to move at all. So the pan cannot be nudged past centre in one
   motion - it stops there first.

   The comparison helper is written with all four arms because that is what
   the original has at the second call site; at the first, cl knows the value
   is not 0x80 (the enclosing test just proved it) and drops the middle arm
   by itself, which is exactly the asymmetry the two blocks show.

   The step itself is a ternary between two constants so that it stays
   branchless, and the re-apply is a TAIL CALL - the original overwrites its
   own argument slot and jumps, which is what a void-to-void same-arity call
   compiles to.

   THE DEAD FALLBACK'S READ HAS TO HAPPEN INSIDE THE HELPER. This sat at
   79.21% for want of one thing: the original re-reads `dir` from its home
   slot at each of the two dead fallbacks, where cl cached it in edi and
   paid a `push edi` for it. A volatile alias at the CALL SITE - the
   documented lever for a memory-resident parameter - is worse (76.60%),
   because the read is then evaluated as an argument and hoisted above the
   helper's branches, where the original's sits after them. Passing the
   address instead and dereferencing it in the fallback line -
   `cmp3(pan, 0x80, &dir)` against `return *v;` - puts the load in the
   block that actually uses it, and the address-take is also what keeps
   `dir` in its home for the ternary. All three went 79.21% -> 100%.
   So when a value the original reads from memory is read inside an
   INLINED helper, qualify the helper's parameter, not the argument. */

#include "ezscreen.h"

extern int g_1b2e91c;    /* src/effectordraw.cpp - the shared pan, 0..0x100 */

static int cmp3(int a, int b, volatile int *v)
{
    if (a > b)
        return 1;
    if (a == b)
        return 0;
    if (a < b)
        return -1;
    return *v;
}

// FUNCTION: EZ2AC_UNPACKED 0x00420400
void EZ2DJMainGameDirector::stepPan(int dir)
{
    int pan, next;

    if (fc7c != 0)
        return;

    pan = g_1b2e91c;
    next = pan + (dir == 1 ? 7 : -7);
    if (next > 0x100)
        next = 0x100;
    else if (next < 0)
        next = 0;

    if (pan != 0x80 && cmp3(pan, 0x80, &dir) != cmp3(next, 0x80, &dir))
    {
        fc7c = 0xf;
        next = 0x80;
    }

    g_1b2e91c = next;
    m420360(next);
}

// FUNCTION: EZ2AC_UNPACKED 0x0045d690
void GFMainGameDirector::stepPan(int dir)
{
    int pan, next;

    if (fc10 != 0)
        return;

    pan = g_1b2e91c;
    next = pan + (dir == 1 ? 7 : -7);
    if (next > 0x100)
        next = 0x100;
    else if (next < 0)
        next = 0;

    if (pan != 0x80 && cmp3(pan, 0x80, &dir) != cmp3(next, 0x80, &dir))
    {
        fc10 = 0xf;
        next = 0x80;
    }

    g_1b2e91c = next;
    m45d5f0(next);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046c190
void CatchMainGameDirector::stepPan(int dir)
{
    int pan, next;

    if (fc0c != 0)
        return;

    pan = g_1b2e91c;
    next = pan + (dir == 1 ? 7 : -7);
    if (next > 0x100)
        next = 0x100;
    else if (next < 0)
        next = 0;

    if (pan != 0x80 && cmp3(pan, 0x80, &dir) != cmp3(next, 0x80, &dir))
    {
        fc0c = 0xf;
        next = 0x80;
    }

    g_1b2e91c = next;
    m46c0f0(next);
}
