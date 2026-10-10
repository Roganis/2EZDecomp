#include <string.h>

#include "kcrt.h"   /* operator new = malloc */
#include "ksongplayer.h"

/* Class definitions in include/ksongplayer.h; the vtable entity is pinned
   from this TU (markers in headers are ignored — source_root is src/). The
   base ctor/dtor live in src/ksongplayerbase.cpp so they cannot inline into
   the ctor below. */

// VTABLE: EZ2AC_UNPACKED 0x00488a7c
class KSongPlayer;

// FUNCTION: EZ2AC_UNPACKED 0x00412970
KSongPlayer::KSongPlayer()
    : m_target(0)
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00412990
void KSongPlayer::setTarget(Slot34Target *v) { m_target = v; }

/* Fire the stored delegate's slot-34 callback if present (guarded virtual
   dispatch, tail-called as jmp [eax+0x88]). */
// FUNCTION: EZ2AC_UNPACKED 0x004129a0
void KSongPlayer::step()
{
    if (m_target != 0)
        m_target->v34();
}

/* The subclass installed on the embedded player at +0x7230 of KEZPlayer.
   Its owner constructs it as a plain KSongPlayer and then patches the vptr,
   so there is no constructor here - only the update() override, which calls
   the owner's tick() NON-virtually (the target's concrete type is known). */
// VTABLE: EZ2AC_UNPACKED 0x00488790
struct Obj410100;

// FUNCTION: EZ2AC_UNPACKED 0x00410100
void Obj410100::step()
{
    if (m_target != 0)
        ((KEZPlayer *)m_target)->tick();
}

/* KEZPlayer's factory, the one KWavLoader's descriptor @0x4a7330 advertises.
   It has to stay out of src/kezplayer.cpp: the original CALLS the constructor
   rather than inlining it, which is what cl emits only when it cannot see the
   body. Not pooled - a plain malloc through kcrt.h's operator new. */
/* KEZPlayer::m410230 @0x00410230 - the player's teardown. ev0 is used as a
   mutex (wait to take it, set to release), ev1 is the "playing" gate; the
   two embedded players are stopped with the lock flag, the 96-entry tick
   table is cleared entry by entry and the 0x800-dword scratch with one
   memset. */
// FUNCTION: EZ2AC_UNPACKED 0x00410230
void KEZPlayer::m410230()
{
    int i;

    ev0.wait(INFINITE);
    ev1.reset();
    f7274 = 0;
    player0.stop(1);
    player1.stop(1);
    m40fd20();
    for (i = 0; i < 96; i++)
    {
        if (entries[i].obj != 0)
            entries[i].obj = 0;
    }
    memset(m_scratch, 0, sizeof(m_scratch));
    ev0.set();
}

// FUNCTION: EZ2AC_UNPACKED 0x00410440
KEZPlayer *createKEZPlayer()
{
    return new KEZPlayer;
}
