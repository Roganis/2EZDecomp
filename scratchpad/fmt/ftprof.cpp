/* Profile-load test harness: the decomp's KProfile::Load (which decrypts via
   KParser::readFile -> slurpFile, then tokenizes) over a list of .ini files.
     ftprof EXE LIST    (run from the output dir; dumps <i>.dump there) */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "kprofile.h"

extern char g_parserKey[0x800];
extern char g_parserDelims[16];
extern int  g_parserTable[14];
static FILE *g_o;

static void walk(KProfileNode *n)
{
    while (n != 0)
    {
        walk(n->left);
        if (n->type == 1)
            fprintf(g_o, "%s\tI\t%d\n", n->key, n->value);
        else if (n->type == 2)
            fprintf(g_o, "%s\tF\t%.6g\n", n->key, *(float *)&n->value);
        else
            fprintf(g_o, "%s\tS\t%s\n", n->key, (char *)&n->value);
        n = n->right;
    }
}

int main(int argc, char **argv)
{
    FILE *e = fopen(argv[1], "rb");
    fseek(e, 0x4a8568 - 0x400000, SEEK_SET);
    fread(g_parserKey, 1, 0x800, e);
    fseek(e, 0x4a8520 - 0x400000, SEEK_SET);
    fread(g_parserDelims, 1, 16, e);
    fread(g_parserTable, 4, 14, e);
    fclose(e);

    static KProfile prof(0xc0000, 0);
    FILE *list = fopen(argv[2], "r");
    char line[1024], local[64], out[64];
    int i = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        sprintf(local, "p%d.ini", i);
        CopyFileA(line, local, FALSE);
        prof.init();
        int r = prof.Load(local);
        sprintf(out, "%d.dump", i);
        g_o = fopen(out, "w");
        fprintf(g_o, "#load=%d nodes=%d\n", r, prof.f4);
        walk(prof.root);
        fclose(g_o);
        DeleteFileA(local);
        i++;
    }
    printf("%d files\n", i);
    return 0;
}
