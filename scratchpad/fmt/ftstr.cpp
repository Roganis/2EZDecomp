/* scratchpad/fmt/ftstr.cpp - .str clip parse test: the decomp's own
   KGraphics::m401b80 @0x401b80 (matched 100%) over a list of .str files,
   with loadTexture replaced by a recorder (it needs a DirectDraw device).
   Linked WITHOUT build/obj/loadtexture.obj - see run.sh.
     ftstr LIST    (run from the output dir; dumps <i>.dump there) */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ez2acapp.h"
#include "obj402860.h"

/* ---- the recorder standing in for loadtexture.obj ---------------------- */
static char g_names[4096][0x110];
static int  g_nNames;
int KGraphics::loadTexture(char *name, KTexture **out, char *)
{
    if (g_nNames < 4096)
        strcpy(g_names[g_nNames], name);
    *out = (KTexture *)(g_nNames + 1);   /* non-null: the cell remembers which */
    g_nNames++;
    return 1;
}
int KGraphics::findOrCreateTexture(char *, KTexture **out, char *, int) { *out = 0; return 0; }

static char g_gfx[0x400];

int main(int argc, char **argv)
{
    FILE *list = fopen(argv[1], "r");
    char line[1024], out[64];
    int i = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        sprintf(out, "%d.dump", i++);
        FILE *o = fopen(out, "w");
        FILE *f = fopen(line, "rb");
        if (!f) { fprintf(o, "MISSING\n"); fclose(o); continue; }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        /* loadStr's layout: the file sits 0x2b210 into the clip node payload */
        char *clip = (char *)calloc(1, size + 0x2b210 + 16);
        fread(clip + 0x2b210, 1, size, f);
        fclose(f);
        g_nNames = 0;
        Obj402860Head *h = 0;
        int r = ((KGraphics *)g_gfx)->m401b80(clip, &h, 0);
        if (!r || !h)
        {
            fprintf(o, "FAIL\n");
        }
        else
        {
            fprintf(o, "H fps=%d frames=%d layers=%d\n", h->f0, h->f4, h->rowCount);
            for (int l = 0; l < h->rowCount; l++)
            {
                Obj402860Row *row = &h->rows[l];
                fprintf(o, "L %d textures=%d keys=%d\n", l, row->count, row->keyCount);
                for (int t = 0; t < row->count && t < 312; t++)
                {
                    int id = row->data[t] - 1;
                    fprintf(o, "T %s\n", (id >= 0 && id < 4096) ? g_names[id] : "?");
                }
                unsigned char *k = (unsigned char *)row->keys;
                for (int q = 0; q < row->keyCount; q++, k += 0x7c)
                {
                    fprintf(o, "K ");
                    for (int b = 0; b < 0x7c; b += 4)
                        fprintf(o, "%08x", *(unsigned int *)(k + b));
                    fprintf(o, "\n");
                }
            }
        }
        fclose(o);
        free(clip);
    }
    printf("%d files\n", i);
    return 0;
}
