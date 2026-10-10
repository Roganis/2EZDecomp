#include <string.h>
#include "loader.h"

/* Unload all samples then clear the table @0x00411050 (__thiscall). Calls the
   sample owner's freeSample (0x40e620, STUB) for each non-null entry, then
   rep-stosd-zeroes the 0x800-entry table. */
// FUNCTION: EZ2AC_UNPACKED 0x00411050
void EziLoader::unloadSamples()
{
    if (field_0x4 != 0)
    {
        void **p = sampleTable;
        for (int i = 0x800; i != 0; i--)
        {
            if (*p)
                field_0x4->freeSample(*p);
            ++p;
        }
    }
    memset(sampleTable, 0, 0x800 * 4);
}
