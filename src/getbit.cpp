/* Obj40FBD0::getBit @0x0040fbd0 - read bit 0 of the byte at arr[i].b[0x14],
   the array being stride 0x30 at this+0. Integrated from wip/getbit.cpp, where
   it was parked at 72.73%, and
   re-measuring says it is STILL 72.73% - this one has not moved.

   Reads bit 0 of the byte at arr[i].field_0x14 (array stride 0x30 at this+0):
     return arr[i].b[0x14] & 1;

   The ONLY diff is one instruction: the original zero-extends the byte load
     movzx eax, byte ptr [eax+ecx+0x14]  ; then  and eax,1
   while our cl 13.10.3077 emits a partial-register load
     mov al, byte ptr [eax+ecx+0x14]     ; then  and eax,1
   because the index (i*0x30, computed `lea eax,[eax+eax*2]; shl eax,4`) has its
   low 4 bits provably 0, so cl knows eax bit 0 is already clear and skips the
   zero-extend. Both are functionally identical; only the bytes differ.

   Verified NOT reproducible: uchar-array read, named uchar field, 1-bit
   bitfield, explicit int temp, (int)/(unsigned) casts, `!= 0`, pointer-to-
   element — all give `mov al`. Swept again 2026-10-01, all `mov al` too:
   a volatile read, `& 1u`, `1 & x`, `% 2`, `(x >> 0) & 1`, `x << 31 >> 31`,
   `(uchar)(x & 1)`, `r &= 1` in a named int, a ushort field cast to uchar,
   and a ternary. A `bool` field DOES give the movzx - and then cl drops the
   `and eax,1` as redundant, so it is not the original's type either. It is a peephole choice that flipped between the
   original's build and ours (same 13.10.3077 reported version). Parked so the
   sibling accessors stay 100%. */

#include <windows.h>
#include "ksongplayer.h"

struct Elem30b { char b[0x30]; };
struct Obj40FBD0 { Elem30b arr[1]; int getBit(int i); };

__forceinline int Obj40FBD0::getBit(int i)
{
    return (unsigned char)arr[i].b[0x14] & 1;
}

/* KEZPlayer's vftable slot 28 (include/ksongplayer.h): the body above,
   forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fbd0
int KEZPlayer::getEntryBit(int i)
{
    return ((Obj40FBD0 *)this)->getBit(i);
}

