#include "ez2ac.h"

/* First byte-match target: a __thiscall accessor returning this->field_0x438.
   Original bytes @0x0040b970:  8b 81 38 04 00 00 c3
     mov eax, [ecx+0x438]
     ret */

// FUNCTION: EZ2AC_UNPACKED 0x0040b970
int Obj40B970::getField438()
{
    return field_0x438;
}

/* (The two-field setter @0x0040d0d0 is slot 2 of the vtable @0x488320 - moved
   into Obj40D4E0, src/obj40d4e0.cpp.) */
