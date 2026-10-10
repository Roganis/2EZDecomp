/* scratchpad/fmt/ftgdsmode.cpp - mode .gds test: the decomp's own
   EZ2DJMainGameDirector::m421190 @0x421190 (the whole file: General,
   Background, Panel, Gauge, ShowCredit, JudgmentLink, InGameSongSelect and
   every SlotN through m420f60) on a zeroed director.
     ftgdsmode EXE LIST    (run from the output dir; dumps <i>.dump there) */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ezscreen.h"

extern char g_parserDelims[16];
extern int  g_parserTable[14];

int main(int argc, char **argv)
{
    FILE *e = fopen(argv[1], "rb");
    fseek(e, 0x4a8520 - 0x400000, SEEK_SET);
    fread(g_parserDelims, 1, 16, e);
    fread(g_parserTable, 4, 14, e);
    fclose(e);
    EZ2DJMainGameDirector *d = (EZ2DJMainGameDirector *)calloc(1, sizeof(EZ2DJMainGameDirector) + 0x1000);
    FILE *list = fopen(argv[2], "r");
    char line[1024], out[64];
    int i = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        sprintf(out, "%d.dump", i++);
        FILE *o = fopen(out, "w");
        CopyFileA(line, "m.gds", FALSE);
        memset(d, 0, sizeof(EZ2DJMainGameDirector));
        int r = d->m421190("m.gds");
        fprintf(o, "load=%d\n", r);
        fprintf(o, "General=%d,%d,%d,%d\n", d->f37c, d->f7c4, d->f7c8, d->f7cc);
        fprintf(o, "Background=%d Gauge=%d InGameSongSelect=%d Panel=%.32s\n", d->f370, d->f794, d->f7bc, d->f708);
        fprintf(o, "ShowCredit=%.6g,%.6g,%.6g,%.6g\n", d->f7ac, d->f7b0, d->f7b4, d->f7b8);
        for (int s = 0; s < 2; s++)
        {
            int *b = &d->f380[0x71 * s];
            fprintf(o, "Slot%d n=%d", s + 1, b[1]);
            for (int t = 0; t < b[1] && t < 27; t++)
                fprintf(o, " %d,%d:%d", b[4 + t * 4], b[5 + t * 4], b[6 + t * 4]);
            fprintf(o, "\n");
        }
        for (int s = 0; s < 2; s++)
        {
            int n = s ? d->f7f8 : d->f7f4;
            GdsLinkEntry *L = s ? d->link_b : d->link_a;
            fprintf(o, "Link%d groups=%d", s + 1, n);
            for (int g = 0; g < n && g < (s ? 7 : 8); g++)
            {
                fprintf(o, " [");
                for (int k = 0; k < L[g].n && k < 0x11; k++)
                    fprintf(o, k ? ",%d" : "%d", L[g].v[k]);
                fprintf(o, "]");
            }
            fprintf(o, "\n");
        }
        fclose(o);
    }
    printf("%d files\n", i);
    return 0;
}
