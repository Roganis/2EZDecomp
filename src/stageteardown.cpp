/* EZ2DJMainGameDirector::m4209d0 @0x4209d0, GFMainGameDirector::m45dae0
   @0x45dae0 and CatchMainGameDirector::m46c440 @0x46c440 - release the run's
   three engine objects and park the pump.

   The notify object is told to stop (its slot 10), then handed back to the
   subsystem through slot 8; the sample sink goes back through slot 5; the
   judge is deleted. Each is nulled as it goes, and the two subsystem
   handbacks happen only while the subsystem itself is up - the global is
   RE-READ between them, because the first call could have changed it.

   Finally the pump is suspended and stopped, the stop in tail position.

   The three differ only in their four member offsets. EZ2DJ and GF share
   0x148/0x14c and a pump at 0x24 and differ in the judge alone (0x154
   against 0x150); catch has all four elsewhere (0x164/0x168/0x16c, pump at
   0x40).

   In its own TU: both pump methods are defined elsewhere and these must
   keep their calls. */

#include "ezscreen.h"
#include "obj414750.h"
#include "judge.h"

/* The subsystem, viewed for the two release slots. Its own header leaves
   slot 5 as `void s5()` and does not declare slot 8 at all; retyping it
   would disturb Obj40EAC0's override, so this follows the mirror-struct
   precedent in src/subsystemload.cpp - the slot INDEX is all a call needs. */
struct SubsystemRelease
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4();
    virtual void releaseSink(void *p);     /* slot 5 -> +0x14 */
    virtual void q6(); virtual void q7();
    virtual void releaseNotify(void *p);   /* slot 8 -> +0x20 */
};

extern struct EzSubsystem *g_sub1b2ef98;   /* 0x1b2ef98 (src/ez2acapp.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004209d0
void EZ2DJMainGameDirector::m4209d0()
{
    if (notify != 0)
    {
        notify->v10();
    }

    if (g_sub1b2ef98 != 0)
    {
        if (notify != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseNotify(notify);
            notify = 0;
        }

        if (sink != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseSink(sink);
            sink = 0;
        }
    }

    if (judge != 0)
    {
        delete judge;
        judge = 0;
    }

    pump.suspend();
    pump.stopAll();
}

// FUNCTION: EZ2AC_UNPACKED 0x0045dae0
void GFMainGameDirector::m45dae0()
{
    if (notify != 0)
    {
        notify->v10();
    }

    if (g_sub1b2ef98 != 0)
    {
        if (notify != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseNotify(notify);
            notify = 0;
        }

        if (sink != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseSink(sink);
            sink = 0;
        }
    }

    if (judge != 0)
    {
        delete judge;
        judge = 0;
    }

    pump.suspend();
    pump.stopAll();
}

// FUNCTION: EZ2AC_UNPACKED 0x0046c440
void CatchMainGameDirector::m46c440()
{
    if (notify != 0)
    {
        notify->v10();
    }

    if (g_sub1b2ef98 != 0)
    {
        if (notify != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseNotify(notify);
            notify = 0;
        }

        if (sink != 0)
        {
            ((SubsystemRelease *)g_sub1b2ef98)->releaseSink(sink);
            sink = 0;
        }
    }

    if (judge != 0)
    {
        delete judge;
        judge = 0;
    }

    pump.suspend();
    pump.stopAll();
}

/* What the first sink runs before the stage teardown @0x420530 - hand the
   run's fader the "1, 10" ramp, choosing between the two faders on the same
   +0x20 selector the rest of the class uses (0 or 2, nothing else acts). The
   third argument is this object's own first secondary base, the way every
   other m450420 call site passes it. */
// FUNCTION: EZ2AC_UNPACKED 0x00420530
void EZ2DJMainGameDirector::m420530()
{
    if (f20 == 0)
        f15c->m450420(1, 10, (char *)this + 0x10);
    else if (f20 == 2)
        f160->m450420(1, 10, (char *)this + 0x10);
}
