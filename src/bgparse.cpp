/* BackGround::m41b680 @0x41b680 and CatchBackGround::m46b9b0 @0x46b9b0 - the
   .scr SCRIPT PARSER, and what the two script loaders hand their buffer to.
   kin.py rates the pair at 0.984.

   THE FORMAT, which is the point of having this:

     "SCR0"          4-byte magic; anything else is rejected outright
     (8 more bytes)  the rest of a 12-byte header
     then TEN SLOTS, each   int f0; int count;   followed by `count` records

   A record is an 8-byte header {f0, kind} plus a payload whose size the kind
   decides - the same table the pump and its helpers use:

     kind 1        header only, 8 bytes, nothing to load
     kind 2, 3     0x80 payload; a name at record+8, extension forced to
                   ".str", loaded through m402de0
     kind 4, 5     0x9c payload; a name at record+0x24 (payload+0x1c, the
                   BgItem's tex slot, which the handle then overwrites),
                   extension forced to ".bmp", loaded through
                   KGraphics::loadTexture
     anything else logs "Not Supported Type" with the frame and kind, and
                   FAILS THE WHOLE PARSE - returning 0 rather than skipping

   Each slot ends up with four words: start, prev, cur and limit, with limit
   set to the record cursor when the slot had any records and 0 when it did
   not. That is the structure the pump @0x45c6a0 walks.

   THE NAMES ARE COPIED WITH strcpy - cl's /Oi expansion, `mov cl,[eax];
   mov [ebx+eax],cl; add eax,1; test cl,cl`, the base difference strength-
   reduced across the whole record walk. (An earlier note here said the
   opposite; that was before the zero register and the record pointer were
   right. Both parsers are byte-exact in the zoo since 2026-10-03.) And the
   kind-4/5 name is at payload+0x1c - the C read it at +0x14, i.e. out of
   the blend words, and wrote the texture over blendDst
   (docs/PORT-DELTAS.md). */

#include <string.h>
#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern void replaceExtension(char *name, char *ext);   /* 0x413000 */

// FUNCTION: EZ2AC_UNPACKED 0x0041b680
int BackGround::m41b680()
{
    char  path[260];
    int   i;
    int   n;
    int   k;
    int  *slot;
    char *rec;

    rec = m_scrText;
    if (*(int *)rec != 0x30524353)
        return ((BackGround *)this)->m41b0c0();

    rec += 0xc;
    i    = 0;
    slot = (int *)((char *)this + 0x102080);
    do
    {
        rec += 4;
        n = *(int *)rec;
        rec += 4;
        if (n == 0)
        {
            slot[-2] = 0;
            slot[0]  = 0;
        }
        else
        {
            slot[-2] = (int)rec;
            slot[0]  = (int)rec;
        }
        slot[-1] = 0;

        k = 0;
        if (n > 0)
        {
            do
            {
                int *hdr = (int *)rec;
                int kind;

                rec += 8;
                kind = hdr[1];

                if (kind == 2 || kind == 3)
                {
                    strcpy(path, rec);
                    replaceExtension(path, ".str");
                    {
                        /* Through a local, then stored: the original's frame
                           holds the handle before it reaches the record. */
                        struct Obj402860Head *h;
                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)rec = h;
                    }
                    rec += 0x80;
                }
                else if (kind == 1)
                {
                    /* nothing to load; tested second, as the original does */
                }
                else if (kind == 4)
                {
                    strcpy(path, rec + 0x1c);
                    replaceExtension(path, ".bmp");
                    {
                        struct KTexture *tex;
                        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &tex, 0);
                        *(struct KTexture **)(rec + 0x1c) = tex;
                    }
                    /* Kind 4 alone resets the three animation words drawItem4
                       steps - the accumulator (item+0x28), the wave phase
                       (+0x2c, which swings between 1.5 and 3.5 and starts at
                       2.6) and its direction flag (+0x30) - which is also why
                       the original does NOT cross-jump this arm into kind 5's.
                       (The C wrote 0 / 1.0 / 0 two words lower, over the
                       material, until 2026-10-01 - docs/PORT-DELTAS.md.) */
                    ((int *)rec)[8]   = 0;
                    ((float *)rec)[9] = 2.6f;
                    ((int *)rec)[10]  = 0;
                    rec += 0x9c;
                }
                else if (kind == 5)
                {
                    strcpy(path, rec + 0x1c);
                    replaceExtension(path, ".bmp");
                    {
                        struct KTexture *tex;
                        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &tex, 0);
                        *(struct KTexture **)(rec + 0x1c) = tex;
                    }
                    rec += 0x9c;
                }
                else
                {
                    Log("Script: %3dFrm, Type '%d': Not Supported Type",
                        hdr[0], hdr[1]);
                    return 0;
                }

                k++;
            } while (k < n);
        }

        slot[1] = (n != 0) ? (int)rec : 0;
        i++;
        slot += 4;
    } while (i < 10);

    /* A fresh script re-arms the judgement notice gate. (The C left it
       as it was until 2026-10-03: docs/PORT-DELTAS.md.) */
    fac = 0;
    return 1;
}

/* The catch copy - same parser, its own offsets: the script buffer at +0x694
   rather than +0x6a0, the slot array at +0x102074, and its own reject path. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b9b0
int CatchBackGround::m46b9b0()
{
    char  path[260];
    int   i;
    int   n;
    int   k;
    int  *slot;
    char *rec;

    rec = m_scrText;
    if (*(int *)rec != 0x30524353)
        return m46b0e0();

    rec += 0xc;
    i    = 0;
    slot = (int *)((char *)this + 0x102074);
    do
    {
        rec += 4;
        n = *(int *)rec;
        rec += 4;
        if (n == 0)
        {
            slot[-2] = 0;
            slot[0]  = 0;
        }
        else
        {
            slot[-2] = (int)rec;
            slot[0]  = (int)rec;
        }
        slot[-1] = 0;

        k = 0;
        if (n > 0)
        {
            do
            {
                int *hdr = (int *)rec;
                int kind;

                rec += 8;
                kind = hdr[1];

                if (kind == 2 || kind == 3)
                {
                    strcpy(path, rec);
                    // STRING: EZ2AC_UNPACKED 0x0048e218
                    replaceExtension(path, ".str");
                    {
                        /* Through a local, then stored: the original's frame
                           holds the handle before it reaches the record. */
                        struct Obj402860Head *h;
                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)rec = h;
                    }
                    rec += 0x80;
                }
                else if (kind == 1)
                {
                    /* nothing to load; tested second, as the original does */
                }
                else if (kind == 4)
                {
                    strcpy(path, rec + 0x1c);
                    // STRING: EZ2AC_UNPACKED 0x004875e4
                    replaceExtension(path, ".bmp");
                    {
                        struct KTexture *tex;
                        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &tex, 0);
                        *(struct KTexture **)(rec + 0x1c) = tex;
                    }
                    /* Kind 4 alone resets the three animation words drawItem4
                       steps - the accumulator (item+0x28), the wave phase
                       (+0x2c, which swings between 1.5 and 3.5 and starts at
                       2.6) and its direction flag (+0x30) - which is also why
                       the original does NOT cross-jump this arm into kind 5's.
                       (The C wrote 0 / 1.0 / 0 two words lower, over the
                       material, until 2026-10-01 - docs/PORT-DELTAS.md.) */
                    ((int *)rec)[8]   = 0;
                    ((float *)rec)[9] = 2.6f;
                    ((int *)rec)[10]  = 0;
                    rec += 0x9c;
                }
                else if (kind == 5)
                {
                    strcpy(path, rec + 0x1c);
                    replaceExtension(path, ".bmp");
                    {
                        struct KTexture *tex;
                        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &tex, 0);
                        *(struct KTexture **)(rec + 0x1c) = tex;
                    }
                    rec += 0x9c;
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x0048e290
                    Log("Script: %3dFrm, Type '%d': Not Supported Type",
                        hdr[0], hdr[1]);
                    return 0;
                }

                k++;
            } while (k < n);
        }

        slot[1] = (n != 0) ? (int)rec : 0;
        i++;
        slot += 4;
    } while (i < 10);

    fa4 = 0;
    return 1;
}
