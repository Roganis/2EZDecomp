/* Obj414750 - the song-player pump (see include/obj414750.h for the layout
   and for why the class keeps an address-derived name). */

#include <string.h>
#include "obj414750.h"
#include <mmsystem.h>

/* Four KLocks with distinct manual/initial pairs, then the slot array zeroed
   with the memset intrinsic (`rep stosd` of 0x40 dwords) and the scalar fields
   defaulted - priority and sleep both 1. The three EH state updates ([esp+0x24]
   = 0/1/2) are cl unwinding the partially constructed members. */
// FUNCTION: EZ2AC_UNPACKED 0x00414750
Obj414750::Obj414750()
    : m_lock110(0, 1), m_lock114(1, 0), m_lock118(1, 1), m_run(1, 0)
{
    memset(m_players, 0, sizeof(m_players));
    m_count = 0;
    m_hThread = 0;
    m_priority = 1;
    m_sleep = 1;
}

/* CreateThread entry point. The object pointer arrives as the thread
   parameter; the null guard is the original's out-of-line `xor eax,eax`
   tail, so the call must be the if-BODY (hot path inline). This has to be a
   real function in this TU - start() below takes its address. */
// FUNCTION: EZ2AC_UNPACKED 0x00414da0
static DWORD WINAPI pumpThreadProc(LPVOID param)
{
    if (param != 0)
        return ((Obj414750 *)param)->run();

    return 0;
}

/* Open the run gate and, when asked to drive itself, spin up the worker.
   lpThreadId is the address of our own `sleep` parameter - the original
   passes `lea edx,[esp+8]`, i.e. the incoming argument slot, reused as the
   throwaway out-param. */
// FUNCTION: EZ2AC_UNPACKED 0x00414dc0
int Obj414750::start(DWORD sleep, int useThread)
{
    m_sleep = sleep;
    m_run.set();
    m_useThread = useThread;

    if (useThread != 0 && m_hThread == 0)
    {
        m_hThread = CreateThread(0, 0, pumpThreadProc, this, 0, &sleep);
        if (m_hThread == 0)
            return 0;

        SetThreadPriority(m_hThread, m_priority);
    }

    return 1;
}

/* Close the run gate and join the worker, killing it if it will not come back
   within 5 s. Returns 0 when there was nothing to stop - both guards jump to
   the original's out-of-line `xor eax,eax` tail, so the work is the if-body. */
// FUNCTION: EZ2AC_UNPACKED 0x00414850
int Obj414750::stop()
{
    if (m_useThread != 0 && m_hThread != 0)
    {
        m_run.reset();

        if (WaitForSingleObject(m_hThread, 5000) == WAIT_TIMEOUT)
            TerminateThread(m_hThread, 1);

        m_hThread = 0;
        return 1;
    }

    return 0;
}

/* The body is just the join; everything after it is the four KLock members
   destructing in reverse declaration order (0x11c, 0x118, 0x114, 0x110), with
   the CloseHandle IAT load CSE'd into edi and the null constant into ebx.
   cl keeps stop() as a real call here rather than inlining it. */
// FUNCTION: EZ2AC_UNPACKED 0x00414d30
Obj414750::~Obj414750()
{
    stop();
}

/* Rewind every slot. The lock is taken and released once PER SLOT, not once
   around the whole sweep, and the players' fields are written directly rather
   than through KSongPlayerBase::start (which would also touch its own gate). */
// FUNCTION: EZ2AC_UNPACKED 0x004148c0
void Obj414750::rewindAll()
{
    for (int i = 0; i < 64; i++)
    {
        m_lock110.wait(-1);

        KSongPlayerBase *p = m_players[i];
        if (p != 0)
        {
            p->m_pos = 0.0f;
            p->m_time = timeGetTime();
        }

        m_lock110.set();
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00414910
int Obj414750::restart()
{
    rewindAll();
    m_lock114.set();
    return 1;
}

/* Park the pump. When it drives its own thread this has to hand the worker a
   chance to notice (reset both gates, then block on m_lock118 until the worker
   signals it); when the caller drives, just flip the gates. */
// FUNCTION: EZ2AC_UNPACKED 0x00414930
void Obj414750::suspend()
{
    if (m_useThread != 0)
    {
        m_lock118.reset();
        m_lock114.reset();
        m_lock118.wait(-1);
    }
    else
    {
        m_lock118.set();
        m_lock114.reset();
    }
}

/* The search is a separate inlined helper returning -1 on miss - that is what
   leaves the provably-dead `cmp eax,-1` before the store (writing the scan
   inline in addPlayer instead merges the two exits and loses it). */
static int findFreeSlot(KSongPlayerBase **players)
{
    for (int i = 0; i < 64; i++)
        if (players[i] == 0)
            return i;

    return -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00414980
int Obj414750::addPlayer(KSongPlayerBase *player)
{
    m_lock110.wait(-1);

    if (m_count < 64)
    {
        int i = findFreeSlot(m_players);
        if (i != -1)
        {
            m_players[i] = player;
            m_count++;
        }
    }

    m_lock110.set();
    return 1;
}

/* Same inlined-search shape as addPlayer, matching a pointer instead of NULL.
   The null-argument guard's `return 0` is the INLINE arm here (the original
   falls through the `jne` into it) and the work is the out-of-line tail. */
static int findSlotOf(KSongPlayerBase **players, KSongPlayerBase *player)
{
    for (int i = 0; i < 64; i++)
        if (players[i] == player)
            return i;

    return -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004149e0
int Obj414750::removePlayer(KSongPlayerBase *player)
{
    if (player == 0)
        return 0;

    m_lock110.wait(-1);

    int i = findSlotOf(m_players, player);
    if (i != -1)
    {
        player->m_lock2.reset();
        m_players[i] = 0;
        m_count--;
    }

    m_lock110.set();
    return 1;
}

/* Drop every player: quiesce each one's playing gate, then clear the array in
   one memset (rep stosd) and reset the count. */
// FUNCTION: EZ2AC_UNPACKED 0x00414a50
int Obj414750::removeAll()
{
    m_lock110.wait(-1);

    for (int i = 0; i < 64; i++)
    {
        KSongPlayerBase *p = m_players[i];
        if (p != 0)
            p->m_lock2.reset();
    }

    memset(m_players, 0, sizeof(m_players));
    m_count = 0;

    m_lock110.set();
    return 1;
}

/* The worker body. One pass per iteration: tick all 64 slots (re-taking
   m_lock110 per slot, as rewindAll does) and sleep m_sleep, or - when the
   m_lock114 gate is closed - acknowledge the suspend handshake by signalling
   m_lock118 back and idle at 1 ms. Both arms end in Sleep(), which cl
   cross-jumps into a single shared `call` with only the pushed argument
   differing (the `push m_sleep; jmp` into the other arm's `push 1`). */
// FUNCTION: EZ2AC_UNPACKED 0x00414ab0
DWORD Obj414750::run()
{
    while (m_run.wait(0) == 0)
    {
        if (m_lock114.wait(0) == 0)
        {
            DWORD now = timeGetTime();

            for (int i = 0; i < 64; i++)
            {
                m_lock110.wait(-1);

                KSongPlayerBase *p = m_players[i];
                if (p != 0)
                    p->advance(now);

                m_lock110.set();
            }

            Sleep(m_sleep);
        }
        else
        {
            if (m_lock118.wait(0) != 0)
                m_lock118.set();

            Sleep(1);
        }
    }

    return 0;
}

/* One pass, driven by the caller instead of the worker - the body of run()'s
   loop with the loop taken off. `now` lands in a stack slot here (the `push
   ecx` prologue) rather than a register, because ebp is holding the SetEvent
   IAT for the inner loop. */
// FUNCTION: EZ2AC_UNPACKED 0x00414b70
void Obj414750::pump()
{
    if (m_run.wait(0) == 0)
    {
        if (m_lock114.wait(0) == 0)
        {
            DWORD now = timeGetTime();

            for (int i = 0; i < 64; i++)
            {
                m_lock110.wait(-1);

                KSongPlayerBase *p = m_players[i];
                if (p != 0)
                    p->advance(now);

                m_lock110.set();
            }
        }
        else
        {
            if (m_lock118.wait(0) != 0)
                m_lock118.set();
        }
    }
}
