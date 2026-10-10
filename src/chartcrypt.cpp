/* EzChart::decryptFile @0x410a40 - THE CHART FILE CIPHER, integrated from
   wip/crypt.cpp where it was parked at ~57%.

   Integrated because it is a FILE FORMAT, which docs/PORTING.md puts near the
   top: a port cannot read the game's own data without it, and a decoded
   function sitting in wip/ is invisible to a build.

   IT IS A MEMBER, not the __stdcall free function the park wrote it as. Both
   call sites (0x4118bc for this one, 0x4117f4 for decryptFile2) load ecx
   first, and a member that never touches `this` is byte-identical to a
   __stdcall of the same arity - so the park had no way to tell and the stub,
   written from a call site, was right.

   THE BUFFERS ARE READ DIRECTLY (2026-10-05, 58% -> 87%). The earlier form
   reached `data` through `header + size + 0x7f`, assuming `data` sat right
   above `header`; in our build `out` did, so the reverse stage copied the
   memset fill and the whole decrypt produced garbage. Checked natively on
   game charts: the old body turns rubymix1p-music.ez into noise, this one
   into an `EZFF` header, and decryptFile2 turns a .ezi into its sample list.
   The frame says data[0x20000] at +0xa0 and out[0x20000] at +0x200a0, with
   4 bytes of pad before the cookie (0x4009c = 0x14 scalars + 0x80 + 2 *
   0x20000 + 8) - not the 131068-byte `out` the park had.

   What is left is the wall, now narrower: the original keeps the wrap
   counter `be` in MEMORY (`cmp [m],0x200` / `sub [m],1`). `volatile` gets it
   there and is worth 10 points, at the price of a load/sub/store for the
   decrement; `int be[1]` and `&be` both score 68%. `ki` must be computed
   from `rollA + n` rather than read back from the volatile (87.5 vs 77).
   Still different: p and ki swap edi/esi, the size-0 exit of the reverse
   loop is jump-threaded past the memcpy guard (the KParser ciphers have the
   same), and the loop alignment padding lands elsewhere. Declaring `flag`
   before `rollA` puts them in the original's slots. Also tried with no
   effect: four spellings of `p`, the update order inside the loop, the
   memcpy guard inside an inlined helper. */

#include <stdio.h>
#include <string.h>
#include "ezchart.h"

// GLOBAL: EZ2AC_UNPACKED 0x004a74b0
char g_keyTable[0x800];

// FUNCTION: EZ2AC_UNPACKED 0x00410a40
int EzChart::decryptFile(char *name)
{
    char header[128];
    char data[131072];
    char out[131072];

    FILE *f = fopen(name, "r+b");
    if (f == 0)
        return -1;

    fseek(f, 0, 2);
    unsigned int size = ftell(f);
    fseek(f, 0, 0);

    memset(data, 1, 0x20000);
    memset(out, 1, 0x20000);

    size = fread(data, 1, size, f);
    fclose(f);

    unsigned int i = 0;
    int flag = 0;
    int rollA = 0;

    /* Reverse `data` into `out`, then copy it back. `data` is addressed
       directly (see the header). */
    for (i = 0; i < size; i++)
        out[i] = data[size - 1 - i];

    /* q set here, ahead of the copy-back - as in m4133d0 / slurpFile */
    char *q = name;
    if (size > 0)
        memcpy(data, out, size);

    unsigned int pos = 0;
    while (pos < size)
    {
        {
            unsigned int n;
            if (pos + 8 > size)
            {
                n = size - pos;
            }
            else
            {
                if (flag == 0)
                {
                    n = 8;
                }
                else
                {
                    n = 0x10;
                    if (pos + 0x10 > size)
                        n = size - pos;
                }
                flag ^= 1;
            }

            unsigned int j = 0;
            if (n > 0)
            {
                volatile int be = rollA + n - 1;
                char *p = data + pos + n - 1;
                int ki = (rollA + n - 1) * 4;
                do
                {
                    int k = ki;
                    if (be >= 0x200)
                        k = ki - 0x800;
                    /* both bytes read into char locals and be stepped
                       between them: the original's load order */
                    char c = *p;
                    be--;
                    char key = g_keyTable[k];
                    out[j + pos] = c - key;
                    j++;
                    ki -= 4;
                    p--;
                } while (j < n);
            }

            rollA += n;
            if (rollA >= 0x200)
                rollA -= 0x200;
            if (8 < n)
                pos = (pos - 8) + n;
            pos += 8;
        }
    }

    int k = 0;
    do
    {
        if (*q == '.')
        {
            header[k]     = '.';
            header[k + 1] = 'e';
            header[k + 2] = 's';
            header[k + 3] = 0;
            break;
        }
        header[k] = *q;
        k++;
        q++;
    } while (k < 0x80);

    f = fopen(header, "w+b");
    fwrite(out, size, 1, f);
    fclose(f);
    return 0;
}

/* EzChart::decryptFile2 @0x410c90 - the second chart cipher, the sibling of
   the one above and the same wall. kin.py rates the two at 0.978: same shape,
   its own key table at 0x4a7cb0, and it is what the chart loader @0x411720
   calls with a filename. Also a member - its call site at 0x4117f4 loads ecx. */


// GLOBAL: EZ2AC_UNPACKED 0x004a7cb0
char g_keyTable2[0x800];

// FUNCTION: EZ2AC_UNPACKED 0x00410c90
int EzChart::decryptFile2(char *name)
{
    char header[128];
    char out[131072];          /* out above data: the original's frame */
    char data[131072];

    // STRING: EZ2AC_UNPACKED 0x004888b0
    FILE *f = fopen(name, "r+b");
    if (f == 0)
        return -1;

    fseek(f, 0, 2);
    unsigned int size = ftell(f);
    fseek(f, 0, 0);

    memset(data, 1, 0x20000);
    memset(out, 1, 0x20000);

    size = fread(data, 1, size, f);
    fclose(f);

    int rollA = 0;
    unsigned int i = 0;
    int flag = 0;

    /* Reverse `data` into `out`, then copy it back. `data` is addressed
       directly (see the header). */
    for (i = 0; i < size; i++)
        out[i] = data[size - 1 - i];

    char *q = name;            /* set here, as in decryptFile */
    if (size > 0)
        memcpy(data, out, size);

    unsigned int pos = 0;
    while (pos < size)
    {
        {
            unsigned int n;
            if (pos + 8 > size)
            {
                n = size - pos;
            }
            else
            {
                if (flag == 0)
                {
                    n = 8;
                }
                else
                {
                    n = 0x10;
                    if (pos + 0x10 > size)
                        n = size - pos;
                }
                flag ^= 1;
            }

            unsigned int j = 0;
            if (n > 0)
            {
                volatile int be = rollA + n - 1;
                char *p = data + pos + n - 1;
                int ki = (rollA + n - 1) * 4;
                do
                {
                    int k = ki;
                    if (be >= 0x200)
                        k = ki - 0x800;
                    be--;
                    out[j + pos] = *p - g_keyTable2[k];
                    j++;
                    ki -= 4;
                    p--;
                } while (j < n);
            }

            rollA += n;
            if (rollA >= 0x200)
                rollA -= 0x200;
            if (8 < n)
                pos = (pos - 8) + n;
            pos += 8;
        }
    }

    int k = 0;
    do
    {
        if (*q == '.')
        {
            header[k]     = '.';
            header[k + 1] = 'e';
            header[k + 2] = 'z';
            header[k + 3] = 'z';
            header[k + 4] = 0;
            break;
        }
        header[k] = *q;
        q++;
        k++;
    } while (k < 0x80);

    // STRING: EZ2AC_UNPACKED 0x004888ac
    f = fopen(header, "w+b");
    fwrite(out, size, 1, f);
    fclose(f);
    return 0;
}
