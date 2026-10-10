/* scratchpad/fmt/ftcipher.cpp - runs the decomp's own cipher code (VC7.1 build)
   over a list of files. Key tables are loaded from the original exe at run
   time. Usage:
     ftmain EXE MODE LIST OUTDIR
   MODE: ez | ezi | ini | rt (rt = slurpFile then m4133d0, round trip)
   Each input i is copied to OUTDIR\w.<ext>, processed, and the result moved to
   OUTDIR\<i>.out. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ezchart.h"
#include "kparser.h"

extern char g_keyTable[0x800];
extern char g_keyTable2[0x800];
extern char g_parserKey[0x800];

static void loadKey(const char *exe, long va, char *dst)
{
    FILE *f = fopen(exe, "rb");
    fseek(f, va - 0x400000, SEEK_SET);
    fread(dst, 1, 0x800, f);
    fclose(f);
}

static char g_dummy[16];

int main(int argc, char **argv)
{
    if (argc < 5)
        return 2;
    loadKey(argv[1], 0x4a74b0, g_keyTable);
    loadKey(argv[1], 0x4a7cb0, g_keyTable2);
    loadKey(argv[1], 0x4a8568, g_parserKey);

    const char *mode = argv[2];
    FILE *list = fopen(argv[3], "r");
    char line[1024], work[1024], res[1024], dst[1024];
    int i = 0, bad = 0;
    while (fgets(line, sizeof(line), list))
    {
        line[strcspn(line, "\r\n")] = 0;
        const char *inExt, *outExt;
        if (!strcmp(mode, "ez"))       { inExt = "ez";  outExt = "es"; }
        else if (!strcmp(mode, "ezi")) { inExt = "ezi"; outExt = "ezz"; }
        else                            { inExt = "ini"; outExt = "int"; }
        /* bare names: the ciphers rewrite the extension at the FIRST '.' in
           the path (and in 128 bytes), so ".\w.ez" would become "..es" */
        sprintf(work, "w.%s", inExt);
        sprintf(res, "w.%s", outExt);
        DeleteFileA(res);
        if (!CopyFileA(line, work, FALSE)) { bad++; i++; continue; }
        int r;
        if (!strcmp(mode, "ez"))       r = ((EzChart *)g_dummy)->decryptFile(work);
        else if (!strcmp(mode, "ezi")) r = ((EzChart *)g_dummy)->decryptFile2(work);
        else
        {
            r = ((KParser *)g_dummy)->slurpFile(work);
            if (!strcmp(mode, "rt") && r == 0)
            {
                /* encrypt the plaintext back: m4133d0 reads X and writes X.ini */
                char pt[1024];
                sprintf(pt, "p.int");
                DeleteFileA(pt);
                MoveFileA(res, pt);
                sprintf(res, "p.ini");
                DeleteFileA(res);
                r = ((KParser *)g_dummy)->m4133d0(pt);
            }
        }
        sprintf(dst, "%d.out", i);
        DeleteFileA(dst);
        if (r != 0 || !MoveFileA(res, dst))
            bad++;
        i++;
    }
    fclose(list);
    printf("%d files, %d failed\n", i, bad);
    return 0;
}
