/* Chart-load test harness: the decomp's decryptFile + loadEZ (VC7.1 build).
     ftload EXE LIST OUTDIR
   For input i: copy to OUTDIR\<base>.ez (the v8 title check needs the real
   basename), decrypt, loadEZ, dump OUTDIR\<i>.dump. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ezchart.h"

struct Track { Record28 records[0x800]; int cur; int count; };
extern Track g_tracks[96];
extern char g_keyTable[0x800];
static EzChart g_chart;

static void dump(FILE *o, EzChart *c)
{
    fprintf(o, "H %d %.6g %.6g %d name=%.64s\n", c->i88, c->f8c, c->f90, c->i98, &c->b8);
    for (int l = 0; l < c->i98; l++)
    {
        Group4C *g = &c->groups[l];
        fprintf(o, "L %d ticks=%d count=%d name=%.64s\n", l, g->f44, g->f48, g->name);
        int n = g->f48 > 0x800 ? 0x800 : g->f48;
        if (g->f48 > 0x800)
            fprintf(o, "OVERFLOW %d\n", g->f48);
        for (int k = 0; k < n; k++)
        {
            unsigned char *r = (unsigned char *)&g_tracks[l].records[k];
            if (r[4] == 1)
                fprintf(o, "N %u %u %u %u %u %u\n", *(unsigned int *)r,
                        *(unsigned short *)(r + 0xe), r[0x10], r[0x11], r[0x12],
                        *(unsigned short *)(r + 0x14));
            else if (r[4] == 5)
                fprintf(o, "B %u %.6g\n", *(unsigned int *)r, *(float *)(r + 0xc));
        }
    }
    /* the merged list: count and order */
    unsigned int prev = 0;
    int sorted = 1, total = 0;
    for (int l = 0; l < c->i98; l++)
        total += g_tracks[l].count;
    for (int k = 0; k < c->i94; k++)
    {
        if ((unsigned int)c->records[k].d0 < prev)
            sorted = 0;
        prev = c->records[k].d0;
    }
    fprintf(o, "M merged=%d lanesum=%d sorted=%d\n", c->i94, total, sorted);
}

int main(int argc, char **argv)
{
    FILE *e = fopen(argv[1], "rb");
    fseek(e, 0x4a74b0 - 0x400000, SEEK_SET);
    fread(g_keyTable, 1, 0x800, e);
    fclose(e);

    FILE *list = fopen(argv[2], "r");
    char line[1024], enc[1024], dec[1024], out[1024];
    int i = 0, ok = 0, rej = 0;
    EzChart *c = &g_chart;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '#')
        {
            sprintf(out, "%s\\%d.dump", argv[3], i);
            FILE *o = fopen(out, "w");
            fprintf(o, "SKIPPED\n");
            fclose(o);
            i++;
            continue;
        }
        const char *base = strrchr(line, '\\') + 1;
        strcpy(enc, base);
        strcpy(dec, enc);
        strcpy(strrchr(dec, '.'), ".es");
        CopyFileA(line, enc, FALSE);
        c->decryptFile(enc);
        int r = c->loadEZ(dec);
        sprintf(out, "%s\\%d.dump", argv[3], i);
        FILE *o = fopen(out, "w");
        if (r)
        {
            dump(o, c);
            ok++;
        }
        else
        {
            fprintf(o, "REJECTED\n");
            rej++;
        }
        fclose(o);
        DeleteFileA(enc);
        DeleteFileA(dec);
        i++;
    }
    printf("%d charts, %d loaded, %d rejected\n", i, ok, rej);
    return 0;
}
