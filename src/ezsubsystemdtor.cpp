/* EzSubsystem's inline destructor (include/ez2acapp.h), emitted out of line
   at 0x40e7a0 - linked but referenced by nothing: destroy the backend at
   +0x54, then ~KBase, under the EH frame the member needs. No vptr store:
   the class is novtable. Its own TU so neither callee's body is visible.

   The qualified call below makes cl emit that copy (??1EzSubsystem) and
   tail-jump it; the marker names the emitted destructor. The helper is a
   FREE function on purpose: declaring a new member on EzSubsystem, even an
   unused one, moved a register in RankingDirector's constructor. */
#include <windows.h>
#include "ez2acapp.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040e7a0
// EzSubsystem::~EzSubsystem

void emitEzSubsystemDtor(EzSubsystem *p)
{
    p->EzSubsystem::~EzSubsystem();
}
