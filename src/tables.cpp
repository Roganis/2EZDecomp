#include "ezchart.h"

/* Append a record to the chart's bounded table (called by insert @0x4124b0
   when no sorted predecessor exists; formerly modeled as Obj412450 before the
   owning class was identified). Copies *src into records[count]; if out != 0
   reports {src lane byte, count}; bumps count. Returns 1 on success, 0 if
   full. Kept in its own TU so callers cannot inline it. */

// FUNCTION: EZ2AC_UNPACKED 0x00412450
int EzChart::append(Record28 *src, RecInfo *out)
{
    int c = i94;
    if (c >= 0x5000)
        return 0;

    records[c] = *src;
    if (out != 0)
    {
        out->lane = ((unsigned char *)src)[5];
        out->index = i94;
    }
    i94++;
    return 1;
}
