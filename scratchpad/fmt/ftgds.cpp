/* .gds header test: loadModeGds's own sequence (readFile, readGdsSection,
   three readGdsInt) over a list; prints "section v2 v1 v0" per file. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <new>
#include "kparser.h"
extern char g_parserDelims[16];
extern int  g_parserTable[14];
int main(int argc, char **argv)
{
    FILE *e = fopen(argv[1], "rb");
    fseek(e, 0x4a8520 - 0x400000, SEEK_SET);
    fread(g_parserDelims, 1, 16, e);
    fread(g_parserTable, 4, 14, e);
    fclose(e);
    FILE *list = fopen(argv[2], "r");
    char line[1024];
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        CopyFileA(line, "g.gds", FALSE);
        static KParser p;
        new (&p) KParser;
        int v0 = -99, v1 = -99, v2 = -99;
        char section[256] = "";
        if (p.readFile("g.gds") == 0) { printf("READFAIL\n"); continue; }
        readGdsSection(section, &p);
        readGdsInt(&v2, &p);
        readGdsInt(&v1, &p);
        readGdsInt(&v0, &p);
        printf("%s %d %d %d\n", section, v2, v1, v0);
    }
    return 0;
}
