#include "ksongplayer.h"

/* KSongPlayerBase ctor/dtor in their own TU: the derived (matched)
   KSongPlayer ctor calls the base ctor as a real `call` - visible bodies
   would inline. The KLock members' ctors/dtors DO inline here (from the
   header), which is the original's shape: three CreateEventA blocks with
   member-construction EH states, and reverse-order CloseHandle+null
   teardown with the IAT load CSE'd into edi. */

// STRING: EZ2AC_UNPACKED 0x004882f4
static const char *const k_klockFailMsg = "KLock::KLock() Creating Handle Failed";

/* The base rate constant (named so the pool literal maps; k120 pattern). */
// GLOBAL: EZ2AC_UNPACKED 0x00487c60
extern const float k_rate1000 = 1000.0f;

// FUNCTION: EZ2AC_UNPACKED 0x00414c10
KSongPlayerBase::KSongPlayerBase()
    : m_lock1(0, 1), m_lock2(1, 0), m_lock3(0, 1)
{
    m_rate = k_rate1000;
    m_pos = 0.0f;
    m_time = timeGetTime();
}

/* Teardown handshake before the closes: wake the consumer (lock2), drop the
   run gate (lock3) after waiting for it. */
// FUNCTION: EZ2AC_UNPACKED 0x00414cd0
KSongPlayerBase::~KSongPlayerBase()
{
    m_lock3.wait(-1);
    m_lock2.reset();
    m_lock3.set();
}

/* Mutex-protected rate setter: cl tail-jumps the closing SetEvent, reusing
   the incoming argument slot for the handle (`mov [esp+4],ecx; jmp SetEvent`),
   so the callee's `ret 4` cleans our stack argument. */
// FUNCTION: EZ2AC_UNPACKED 0x00414600
void KSongPlayerBase::setRate(float rate)
{
    m_lock1.wait(-1);
    m_rate = rate;
    m_lock1.set();
}

/* Start playing, but only if the "playing" gate is not already signaled -
   a 0-timeout poll returning non-zero means WAIT_TIMEOUT, i.e. stopped. */
// FUNCTION: EZ2AC_UNPACKED 0x00414630
void KSongPlayerBase::start()
{
    if (m_lock2.wait(0) != 0)
    {
        m_pos = 0.0f;
        m_time = timeGetTime();
        m_lock2.set();
    }
}

/* Stop playing. `lock` selects whether to take m_lock3 around the reset;
   the closing set() is again a tail jump (the else arm is the bare ret 4). */
// FUNCTION: EZ2AC_UNPACKED 0x00414660
void KSongPlayerBase::stop(int lock)
{
    if (lock)
        m_lock3.wait(-1);

    m_lock2.reset();

    if (lock)
        m_lock3.set();
}

/* The driver tick, called from the owning thread with timeGetTime(). m_pos is
   a COUNTDOWN in milliseconds, not a position: elapsed time is subtracted from
   it, and while it has run out the player steps (virtual step(), slot 0) and
   one m_rate interval is added back - so a slow tick catches up with several
   steps in one call. m_lock3 is held across each step, m_lock1 only across the
   m_rate read (which is why the rate is spilled before SetEvent clobbers xmm0).

   The unsigned delta -> float conversion is cl's fild + conditional 2^32 fixup
   (0x487c18); the pool constant resolves on its own.

   m_pos/m_time are `volatile` (see the header) - that is what stops cl folding
   them as memory operands, giving the original's load-then-operate shape in all
   three places: `fld [m_pos]` + register `fsubp` instead of `fsubr [m_pos]`,
   `mov ecx,[m_time]` + register `sub` instead of `sub ecx,[m_time]`, and
   `movss xmm0,[m_pos]` + `comiss xmm1,xmm0` instead of a folded comiss. The
   whole update must also stay ONE expression: splitting the countdown into a
   `float pos` local costs the x87 pop (`fsubr st,st(1)` instead of `fsubp`),
   and computing the delta first swaps the m_pos/m_time read order. */
// FUNCTION: EZ2AC_UNPACKED 0x004146a0
void KSongPlayerBase::advance(DWORD now)
{
    if (m_lock2.wait(0) == 0)
    {
        m_pos = m_pos - (float)(now - m_time);
        m_time = now;

        while (m_pos <= 0.0f)
        {
            m_lock3.wait(-1);
            step();

            m_lock1.wait(-1);
            float rate = m_rate;
            m_lock1.set();

            m_pos = m_pos + rate;
            m_lock3.set();
        }
    }
}
