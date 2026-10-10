#include <stdlib.h>
#include <time.h>
#include "ingame.h"

/* Draw eight DISTINCT numbers below m_randRange[kind] into `out`. The retry is
   a do-while around the whole draw: the candidate is stored into out[i] before
   the duplicate scan, and the scan runs the full i entries with no early exit -
   it only sets a flag.

   That flag is a `char`, not an int: the original clears it with `xor cl,cl`,
   sets it with `mov cl,1` and tests it with `test cl,cl`. An int flag produces
   the same three instructions on ecx and nothing else changes. */
// FUNCTION: EZ2AC_UNPACKED 0x00433cf0
void InGame::pickDistinct(int *out, int kind)
{
    int  i, j, r;
    char dup;

    srand((unsigned int)time(0));
    for (i = 0; i < 8; i++)
    {
        do
        {
            r = rand() % m_randRange[kind];
            out[i] = r;
            dup = 0;
            for (j = 0; j < i; j++)
            {
                if (r == out[j])
                    dup = 1;
            }
        } while (dup);
    }
}
