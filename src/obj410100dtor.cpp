/* Obj410100's implicit destructor (include/ksongplayer.h), emitted out of
   line at 0x40f330 - linked, referenced by nothing (the owner's destructor
   expands it in place): nothing of its own to destroy, so a bare tail jump
   into ~KSongPlayerBase. The helper makes cl emit it; the marker names the
   emitted destructor. A free function, not a member, for the reason in
   src/ezsubsystemdtor.cpp. */
#include <windows.h>
#include "ksongplayer.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040f330
// Obj410100::~Obj410100

void emitObj410100Dtor(Obj410100 *p)
{
    p->~Obj410100();
}
