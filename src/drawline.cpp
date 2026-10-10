/* EzObject::drawLine @0x467670 - print a number into a chart row: only
   while the chart is idle, sprintf the value through the caller's
   format (the two middle parameters keep the loose types every matched
   call site compiled against - the "texture" is the number, the "int"
   the format), then walk the text from its END, each digit setting the
   next column's cell from the caller's digit table (the '0' bias folds
   into the displacement), non-digits still advancing the column. */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "ezscreen.h"
#include "obj402860.h"

// FUNCTION: EZ2AC_UNPACKED 0x00467670
void EzObject::drawLine(Obj402860 *chart, int col, KTexture *c, int d,
                        void *e)
{
    char buf[0x40];
    int  i;

    if (chart->isDone() != 0)
        return;
    sprintf(buf, (const char *)d, (int)c);
    for (i = (int)strlen(buf) - 1; i >= 0; i--)
    {
        if (isdigit(buf[i]) != 0)
            chart->setCell(col, 0, ((int *)e)[buf[i] - '0']);
        col++;
    }
}
