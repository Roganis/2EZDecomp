/* SongSelectDirector::m435070 @0x435070 - seed the select page from one
   category. Walks the category's 16-byte entry names (g_catNames, up to
   g_catCounts[cat] of them), finds each among the loaded song entries by
   case-insensitive name, and gathers the matches' full 0x374-byte records
   (each starting 0x10c before its name) into a 385KB stack table in
   category order. The rows whose first level float is nonzero are then
   copied into the director's page table at +0x12c, f7c takes the gathered
   count, and the cursor is restored to the entry matching the remembered
   name in g_1b2eb04 (only meaningful when `keep` had the caller copy it -
   the ctor passes 1 to restore, the genre pager 0 to reset). The strip
   floats re-seed to cursor * 100. The third argument is unused. */

#include <string.h>
#include "songselect.h"

extern "C" int __cdecl _stricmp(const char *, const char *);

extern int  g_1b2eb04[8];       /* 0x1b2eb04 (src/globals.cpp) - the
                                   remembered song name */
extern int  g_catCounts[];      /* 0x3789bd4 (src/globals.cpp) */
extern char g_catNames[47][436][16];  /* 0x3789c90 (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00435070
void SongSelectDirector::m435070(int cat, int keep, int a3)
{
    char buf[32];
    char localRows[436][0x374];
    int  found;
    int  count;
    int  i;
    int  j;
    int  sel;
    char c;

    found = 0;
    count = g_catCounts[cat];
    if (keep != 0)
    {
        i = 0;
        do
        {
            c = ((char *)g_1b2eb04)[i];
            buf[i] = c;
            i++;
        } while (c != 0);
    }

    for (i = 0; i < count; i++)
    {
        for (j = 0; j < f80; j++)
        {
            if (_stricmp(g_catNames[cat][i],
                         (char *)this + 0x5e3c8 + j * 0x374) == 0)
            {
                memcpy(localRows[found], (char *)this + 0x5e2bc + j * 0x374,
                       0x374);
                found++;
                break;
            }
        }
    }

    f7c = found;
    f84 = 0;
    j = 0;
    for (i = 0; i < found; i++)
    {
        if (*(float *)(localRows[i] + 0x214) != 0.0f)
        {
            memcpy((char *)this + 0x12c + j * 0x374, localRows[i], 0x374);
            j++;
        }
    }

    for (sel = 0; sel < f7c; sel++)
    {
        if (_stricmp(buf, rows[sel].name) == 0)
            goto found_sel;
    }
    sel = 0;
found_sel:

    f84 = sel;
    f8c = sel;
    f90 = f88;
    f1c = cat;
    f_cc = f_c4 = (float)sel * 100.0f;
}
