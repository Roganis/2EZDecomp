/* KGraphics::m401b80 @0x401b80 - turn a raw .str payload into the cell
   table the clip's header points at. The payload sits 0x2b214 into the
   clip: a version word (0x94), three header words copied to the head,
   five words skipped, then per row a cell count (logged past 0x138), the
   cells' texture names in 0x80-byte slots - prefixed with the caller's
   directory when one is given, defaulted to .bmp - each loaded through
   the texture cache (a miss zeroes *out and fails), and finally the
   row's tween-key count with the keys LEFT IN PLACE, the row pointing
   straight into the payload (TweenKey stride 0x7c = the 31-word skip).

   Its own TU: loadTexture is defined in src/loadtexture.cpp and must
   stay a call. */
#include <stdio.h>
#include <string.h>
#include "ez2acapp.h"
#include "obj402860.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (folded) */

// FUNCTION: EZ2AC_UNPACKED 0x00401b80
int KGraphics::m401b80(char *clip, Obj402860Head **out, char *dir)
{
    char buf[260];
    Obj402860Head *h = (Obj402860Head *)clip;
    int *cur = (int *)(clip + 0x2b214);
    int magic;
    int i, j;

    magic = *cur++;
    h->f0 = *cur++;
    h->f4 = *cur++;
    h->rowCount = *cur;
    cur += 5;
    if (magic != 0x94)
    {
        // STRING: EZ2AC_UNPACKED 0x00487618
        Log("LoadAniClip: Version is incorrect ");
        return 0;
    }
    for (i = 0; i < h->rowCount; i++)
    {
        Obj402860Row *row = &h->rows[i];

        row->count = *cur;
        cur++;
        if (row->count > 0x138)
        {
            // STRING: EZ2AC_UNPACKED 0x004875f4
            Log("EZ2V: Exceed Layer Ani Tex Frame");
        }
        for (j = 0; j < row->count; j++)
        {
            if (dir != 0)
            {
                // STRING: EZ2AC_UNPACKED 0x004875ec
                sprintf(buf, "%s\\%s", dir, (char *)cur);
            }
            else
                strcpy(buf, (char *)cur);
            if (strchr(buf, '.') == 0)
                strcat(buf, ".bmp");
            loadTexture(buf, (KTexture **)&row->data[j], 0);
            if (row->data[j] == 0)
            {
                *out = 0;
                return 0;
            }
            cur += 0x20;
        }
        row->keyCount = *cur;
        cur++;
        row->keys = (TweenKey *)cur;
        cur += row->keyCount * 31;
    }
    *out = h;
    return 1;
}
