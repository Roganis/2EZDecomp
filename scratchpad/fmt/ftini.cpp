/* scratchpad/fmt/ftini.cpp - per-chart .ini test: the decomp's own
   EZ2DJMainGameDirector::loadGds @0x423790 (it decrypts .ini through
   KParser::readFile) over a list of chart .ini files, on a zeroed director.
   g_modeIndex is 6 so MeasureScale and the gauge rates are kept as read.
     ftini EXE LIST    (run from the output dir; dumps <i>.dump there) */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ezscreen.h"

extern char g_parserKey[0x800];
extern char g_parserDelims[16];
extern int  g_parserTable[14];
extern int  g_modeIndex;
extern int  g_1b2e89c;
extern int  g_useAudiencePanel;

int main(int argc, char **argv)
{
    FILE *e = fopen(argv[1], "rb");
    fseek(e, 0x4a8568 - 0x400000, SEEK_SET);
    fread(g_parserKey, 1, 0x800, e);
    fseek(e, 0x4a8520 - 0x400000, SEEK_SET);
    fread(g_parserDelims, 1, 16, e);
    fread(g_parserTable, 4, 14, e);
    fclose(e);
    g_modeIndex = 6;
    g_1b2e89c = 0;

    EZ2DJMainGameDirector *d = (EZ2DJMainGameDirector *)calloc(1, sizeof(EZ2DJMainGameDirector) + 0x100);
    FILE *list = fopen(argv[2], "r");
    char line[1024], out[64], local[32];
    int i = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        sprintf(out, "%d.dump", i);
        sprintf(local, "c%d.ini", i);
        i++;
        FILE *o = fopen(out, "w");
        if (!CopyFileA(line, local, FALSE)) { fprintf(o, "MISSING\n"); fclose(o); continue; }
        memset(d, 0, sizeof(EZ2DJMainGameDirector));
        g_useAudiencePanel = 0;
        int r = d->loadGds(local);
        fprintf(o, "load=%d\n", r);
        fprintf(o, "Level=%d MeasureScale=%.6g\n", d->f2ec, d->f320);
        fprintf(o, "Judgment=%d,%d,%d,%d\n", d->f304, d->f300, d->f2fc, d->f2f8);
        fprintf(o, "Gauge=%.6g,%.6g,%.6g,%.6g\n", d->f318, d->f314, d->f310, d->f30c);
        fprintf(o, "Audience=%d,%d,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g,%.6g\n", d->f324, d->f328,
                d->f340, d->f33c, d->f338, d->f334, d->f330, d->f344, d->f348);
        fclose(o);
        DeleteFileA(local);
    }
    printf("%d files\n", i);
    return 0;
}
