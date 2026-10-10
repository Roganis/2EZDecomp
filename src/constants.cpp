/* Constant-return leaf functions. Calling convention is chosen to reproduce
   the exact `ret N` (arg cleanup); these are likely small members/virtuals but
   reccmp matches on bytes, which are identical for __stdcall(N) and a thiscall
   with N stack args. Class identities TBD. */

#include "ksound.h"
#include "kbase.h"

/* (@0x00401070 `or eax,-1; ret` was here as a bare retMinus1 until it was
   identified as slot 4 of KEngineApp's vtable @0x487418 — now a real virtual
   member in src/kengineapp.cpp, which the vtable pin requires.) */

/* (@0x00401678 `mov eax,1; ret 4` was here as a standalone ret1_arg1 until
   KRuntimeClass::isDerivedFrom @0x401660 was reconstructed: it is that
   function's out-of-line `return 1` tail, not a function of its own — nothing
   in the binary calls it — and annotating it truncated isDerivedFrom's size.) */

/* @0x00401690: mov eax,ecx; ret. Identified as the empty out-of-line ctor of
   the novtable engine base KBase (called by the KSong/EzChart/KSNDMixer
   ctors; novtable = no vftable store, ctors return this). Must stay outside
   the derived classes' TUs or cl inlines the call away. */
// FUNCTION: EZ2AC_UNPACKED 0x00401690
KBase::KBase()
{
}

/* What KBase's destructor tail-calls (src/stubs.cpp): empty, ICF-folded by
   the original's linker onto the shared `ret` @0x402880. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void kbaseRelease()
{
}

/* @0x00401770: xor eax,eax; ret 0xc  -> returns 0 (three stack args). ICF-folded
   with Obj46AA80's vtable slot 5 (src/resetscreen.cpp), hence `folded` on both:
   this file sorts first, so this one owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int __stdcall ret0_arg3(int, int, int) { return 0; }

/* @0x0040f420: xor eax,eax; ret  -> returns 0. Identified as the global
   sound-object finder called by EzChart::setSound @0x410970 (compiled out to
   `return 0` in this build). Must stay outside ezchart.cpp or cl inlines it. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f420
KSound *findSoundObject(int * /*index*/, KRuntimeClass * /*cls*/)
{
    return 0;
}

/* @0x0040f480: mov eax,1; ret 0xc  -> returns 1 (three stack args). */
// FUNCTION: EZ2AC_UNPACKED 0x0040f480
int __stdcall ret1_arg3(int, int, int) { return 1; }

/* (@0x0040ff81 and @0x0040ff89 were here as ret2_arg1/ret0_arg1. They are not
   functions: both are switch arms of Obj40FF50::entryKind @0x0040ff50, reached
   through its jump table at 0x40ff90 — see wip/entrykind.cpp. Their unaligned
   addresses were the tell.) */
