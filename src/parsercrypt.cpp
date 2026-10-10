/* KParser::slurpFile @0x4135f0 - the profile-file reader, and the THIRD
   instance of the chart cipher. wip/slurpdecrypt.cpp identified it and parked
   it without a draft, on the grounds that any reconstruction hits the same
   inner-loop wall as the two decryptors in src/chartcrypt.cpp; that is true,
   and it is written anyway for the same reason those were - it is a FILE
   FORMAT, and a port cannot read a profile without it.

   The cipher is identical to EzChart::decryptFile's: fopen "r+b", read the
   whole file, byte-REVERSE it, then walk it in alternating 8- and 16-byte
   blocks subtracting a key stream, with a wrap counter that rolls at 0x200.
   Three things differ:

     - ITS OWN KEY TABLE at 0x4a8568, stride-4 indexed like the others.
     - THE OUTPUT EXTENSION IS ".int" rather than ".es", written as four byte
       stores after copying the name up to its first '.'.
     - IT HAS TWO 0x20000 BUFFERS AND NO SEPARATE HEADER. The chart pair use
       header/data/out; this one reverses into the second buffer and copies
       back with the rep movs that cl generates for a size-counted loop.

   The frame is big enough to force _chkstk and the aligned prologue, which is
   why it opens `push ebp; mov ebp,esp; and esp,-8`.

   ITS WRAP COUNTER IS RECOMPUTED, NOT CARRIED, and that is what
   wip/slurpdecrypt.cpp got wrong when it declared this the same wall as the
   chart decryptors. The original evaluates `rollA + (n - j) - 1` fresh on
   every iteration, reloading rollA from its frame slot each time - there is no
   counter to allocate, so there is nothing for cl to disagree about. Writing
   the loop that way rather than decrementing a `be` is worth twelve points
   (51.37% -> 63.91%). The chart pair really do decrement a memory-resident
   counter (`sub DWORD PTR [esp+0x14],0x1` inside their loop) and really are
   walled; this one never was. */

#include <stdio.h>
#include <string.h>
#include "kparser.h"

// GLOBAL: EZ2AC_UNPACKED 0x004a8568
char g_parserKey[0x800];

// FUNCTION: EZ2AC_UNPACKED 0x004135f0
int KParser::slurpFile(char *name)
{
    char path[128];
    char data[131072];
    /* f is declared BETWEEN the two buffers: that is the original's
       frame order (scratchpad/perm, 2026-10-09) */
    FILE *f = fopen(name, "r+b");

    char out[131072];
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
    int rollA = 0;
    int flag = 0;

    /* Reverse `data` into `out`, then copy it back. `data` is addressed
       directly: an earlier form read it through `path + size + 0x7f`, which
       assumes `data` sits directly above `path` - in our build `out` did, so
       this stage read and wrote the same buffer. */
    for (i = 0; i < size; i++)
        out[i] = data[size - 1 - i];

    /* q set here, ahead of the copy-back - as in m4133d0 */
    char *q = name;
    if (size > 0)
        memcpy(data, out, size);

    unsigned int pos = 0;
    while (pos < size)
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
            char *p = data + pos + n - 1;
            do
            {
                /* RECOMPUTED per iteration from the memory-resident rollA,
                   not carried in a register - see the header. */
                /* UNSIGNED: the wrap test is a `jb` (0x413711). */
                unsigned int be = (n - j) + rollA - 1;
                if (be >= 0x200)
                    be -= 0x200;
                out[j + pos] = *p - g_parserKey[be * 4];
                j++;
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

    int k = 0;
    do
    {
        if (*q == '.')
        {
            path[k]     = '.';
            path[k + 1] = 'i';
            path[k + 2] = 'n';
            path[k + 3] = 't';
            path[k + 4] = 0;
            break;
        }
        path[k] = *q;
        k++;
        q++;
    } while (k < 0x80);

    f = fopen(path, "w+b");
    fwrite(out, size, 1, f);
    fclose(f);
    return 0;
}

/* KParser::m4133d0 @0x4133d0 - the ENCRYPT direction, and the last member of
   the cipher family. It shares slurpFile's key table and buffers, and is its
   mirror in three ways:

     - THE CIPHER ADDS where slurpFile subtracts, so this is the writer.
     - THE STAGES RUN IN THE OPPOSITE ORDER: cipher, copy, reverse, where
       slurpFile does reverse, copy, cipher.
     - ITS WRAP COUNTER IS ONE VARIABLE COUNTING UP and reset to 0 at 0x200,
       carried across blocks. slurpFile recomputes `rollA + n - 1` per block
       and counts DOWN. That is why this one has no separate rollA.

   The output extension is ".ini" - and cl notices the two 'i's are the same
   byte, loading 0x69 into cl once and storing it twice, which is what the
   original's `mov cl,0x69` before the run of byte stores is.

   Six call sites, all passing a KParser on the stack in ecx, which is what
   confirms the member shape. */
// FUNCTION: EZ2AC_UNPACKED 0x004133d0
int KParser::m4133d0(char *name)
{
    char path[128];
    char data[131072];
    char out[131072];

    FILE *f = fopen(name, "r+b");
    if (f == 0)
        return -1;

    fseek(f, 0, 2);
    unsigned int size = ftell(f);
    fseek(f, 0, 0);

    /* The cipher reads `data` and writes `out`; the original has `data` at
       +0x98 directly above `path` and `out` at +0x20098. (An earlier form
       read `data` through `path + ... + 0x7f`, which only worked while the
       layout agreed - in our build it read `out`.) */
    memset(data, 1, 0x20000);
    memset(out, 1, 0x20000);

    size = fread(data, 1, size, f);
    fclose(f);

    int be = 0;
    int flag = 0;
    unsigned int pos = 0;

    while (pos < size)
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
            char *p = data + pos + n - 1;
            char *d = out + pos;
            do
            {
                d[j] = g_parserKey[be * 4] + *p;
                be++;
                if (be >= 0x200)
                    be = 0;
                j++;
                p--;
            } while (j < n);
        }

        if (8 < n)
            pos = (pos - 8) + n;
        pos += 8;
    }

    /* q is set HERE, ahead of the copy-back, not beside its loop: that is
       what gives the decrypt loop's reverse pointer the original's
       `lea edi,[edx+eax]` (3 -> 1 by scratchpad/perm). */
    char *q = name;
    if (size > 0)
        memcpy(data, out, size);

    {
        unsigned int i;
        for (i = 0; i < size; i++)
            out[i] = data[size - 1 - i];
    }

    int k = 0;
    do
    {
        if (*q == '.')
        {
            path[k]     = '.';
            path[k + 1] = 'i';
            path[k + 2] = 'n';
            path[k + 3] = 'i';
            path[k + 4] = 0;
            break;
        }
        path[k] = *q;
        k++;
        q++;
    } while (k < 0x80);

    f = fopen(path, "w+b");
    fwrite(out, size, 1, f);
    fclose(f);
    return 0;
}
