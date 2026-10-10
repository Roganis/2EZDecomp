#include "ezchart.h"

/* Three-key comparator for EzChart records (insertion-sort order): d0
   ascending (UNSIGNED compare), byte5 ascending, byte4 DESCENDING. Free
   __cdecl leaf; kept in its own TU so callers (EzChart::locate @0x4123e0,
   the record-insert loop @0x411310) cannot inline it. Consecutive if(>)/if(<)
   on the same operands make cl reuse one cmp's flags (jbe then jae). */
// FUNCTION: EZ2AC_UNPACKED 0x00410920
int recordCompare(const Record28 *a, const Record28 *b)
{
    if ((unsigned int)a->d0 > (unsigned int)b->d0)
        return 1;
    if ((unsigned int)a->d0 < (unsigned int)b->d0)
        return -1;
    if (a->byte5 > b->byte5)
        return 1;
    if (a->byte5 < b->byte5)
        return -1;
    if (a->byte4 < b->byte4)
        return 1;
    if (a->byte4 > b->byte4)
        return -1;
    return 0;
}
