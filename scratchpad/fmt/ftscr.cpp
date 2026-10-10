/* scratchpad/fmt/ftscr.cpp - .scr script parse test: the decomp's own
   BackGround::m41b680 @0x41b680 over a list of SCR0 scripts, with loadStr and
   loadTexture replaced by recorders (both need a live KGraphics). Linked
   WITHOUT loadtexture.obj and loadstr.obj - see run.sh.
     ftscr LIST    (run from the output dir; dumps <i>.dump there) */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

static char g_names[8192][0x110];
static int  g_nNames;
static int record(const char *name)
{
    if (g_nNames < 8192)
        strcpy(g_names[g_nNames], name);
    return ++g_nNames;      /* the fake handle: 1-based index into g_names */
}
int KGraphics::loadTexture(char *name, KTexture **out, char *)
{
    *out = (KTexture *)record(name);
    return 1;
}
int KGraphics::findOrCreateTexture(char *, KTexture **out, char *, int) { *out = 0; return 0; }
int KGraphics::loadStr(char *name, Obj402860Head **out)
{
    *out = (Obj402860Head *)record(name);
    return 1;
}

static char g_gfx[0x400];

int main(int argc, char **argv)
{
    g_sub1b2f054 = (EzSubsystem *)g_gfx;
    BackGround *bg = (BackGround *)calloc(1, 0x110000);
    FILE *list = fopen(argv[1], "r");
    char line[1024], out[64];
    int i = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        sprintf(out, "%d.dump", i++);
        FILE *o = fopen(out, "w");
        memset(bg, 0, 0x110000);
        FILE *f = fopen(line, "rb");
        if (!f) { fprintf(o, "MISSING\n"); fclose(o); continue; }
        fread(bg->m_scrText, 1, sizeof(bg->m_scrText) - 1, f);
        fclose(f);
        if (*(int *)bg->m_scrText != 0x30524353)
        {
            fprintf(o, "LEGACY\n");
            fclose(o);
            continue;
        }
        g_nNames = 0;
        int r = bg->m41b680();
        if (!r) { fprintf(o, "FAIL\n"); fclose(o); continue; }
        int *slot = (int *)((char *)bg + 0x102080);
        for (int s = 0; s < 10; s++, slot += 4)
        {
            char *rec = (char *)slot[-2], *end = (char *)slot[1];
            fprintf(o, "C %d\n", s);
            while (rec != 0 && rec < end)
            {
                int frame = ((int *)rec)[0], kind = ((int *)rec)[1];
                const char *nm = "";
                if (kind == 2 || kind == 3) { int id = *(int *)(rec + 8) - 1; nm = g_names[id]; }
                if (kind == 4 || kind == 5) { int id = *(int *)(rec + 8 + 0x1c) - 1; nm = g_names[id]; }
                fprintf(o, "E %d %d %s\n", frame, kind, nm);
                rec += 8 + (kind == 1 ? 0 : (kind <= 3 ? 0x80 : 0x9c));
            }
        }
        fclose(o);
    }
    printf("%d files\n", i);
    return 0;
}
