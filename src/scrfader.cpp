#include "ezscreen.h"

/* ScrFader - the screen fader the three main-game directors build at the end
   of construction and hand their own first secondary-base pointer to.

   The class was found from its FACTORY. scratchpad/twins.py ranked @0x450570
   as an 88% near-copy of createKSong @0x40ef00, which is the same shape with
   malloc where this one has the pool; reading the two side by side gave the
   record (@0x4ae154), the size (0x20, exactly the struct that was already
   modelled from m450420), the vftable (@0x495400) and the inlined
   constructor. The runtime class @0x4ae144 is what supplies the NAME - it
   was already sitting in src/classinfo.cpp as `classScrFader`.

   Six vtable slots, of which four were already entities somewhere else:
   slots 0 and 2 were matched as FREE functions in src/classinfo.cpp and had
   to move in here, because a slot is scored on the recomp address of the
   entity matched to it and a lookalike beside it will not do; slot 4 is the
   folded empty @0x475dc0 and slot 5 the folded `xor eax,eax; ret 0xc`
   @0x401770, so both are aliases rather than bodies. */

extern KRuntimeClass classScrFader;   /* 0x4ae144 (src/classinfo.cpp) */
extern EzObjectInfo g_info_4ae154;    /* 0x4ae154 (src/classinfo.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004503f0
KRuntimeClass *ScrFader::getRuntimeClass()
{
    return &classScrFader;
}

// FUNCTION: EZ2AC_UNPACKED 0x00450400
EzObjectInfo *ScrFader::getObjectInfo()
{
    return &g_info_4ae154;
}

/* `mov [ecx],0x495400; jmp 0x402880` - restore the vptr, tail-jump to the
   root's folded empty destructor. Nothing of its own to destroy. */
// FUNCTION: EZ2AC_UNPACKED 0x00450410
ScrFader::~ScrFader()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00450550
// ScrFader::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`; src/constants.cpp owns the
   address, this is one more alias on it. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int ScrFader::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00495400
struct ScrFader;

/* The factory. The constructor is INLINED into it - the base's ctor, the
   vptr store and two zeroed members - which is why ScrFader::ScrFader has to
   be declared and defined inline rather than out of line. */
// FUNCTION: EZ2AC_UNPACKED 0x00450570
ScrFader *m450570()
{
    g_objectRecord = &g_info_4ae154;
    return new ScrFader;
}
