/* SongTable::loadBinFile @0x469c30 - THE SONG DATABASE LOADER, and the fourth
   cipher in this binary.

   Both this and loadSongBin @0x469fb0 are MEMBERS: the outer one forwards ecx
   to the inner, and its own caller loads `mov ecx,0x3780958` - g_songTable.
   The first argument is a DIRECTORY name, not a destination, which is what the
   path format shows.

   THE .bin CIPHER, two 32-byte tables at 0x4ae9fc and 0x4ae9dc. For every byte
   of the file:

       buf[i] ^= key1[(i + 1) & 31] ^ (char)i;
       for (j = 0; j < 32; j++) {
           int k = j ? j : 12;
           buf[i] ^= key2[i % k] ^ key2[j] ^ key1[j];
       }

   cl unrolls the inner loop by four, which is why the listing shows the same
   three-XOR step at four consecutive key offsets. The `j ? j : 12` is a real
   division-by-zero dodge in the original - j is the divisor, and it is zero on
   the first pass.

   THE FORMAT, once deciphered:

       +0x06  u16 count      -> SongTable::count
       +0x08  u32 offset     -> `count` records of 0x56, copied straight into
                                the entries array
       +0x0c  u32 offset     -> 47 GROUPS, each a u16 count followed by that
                                many 0x10-byte records

   The group stride is 0x1b40, which is exactly 436 * 0x10 - the same 436 as
   `entries` - so each group holds one slot per song. That is what fixes the
   groups array's shape, and extending SongTable to model it is a 354 KB growth
   whose only collateral, checked with a full match, was the RankingDirector
   ctor coin that flips on any TU change.

   The whole file lives in a VirtualAlloc'd buffer that is VirtualFree'd before
   returning, so nothing of the raw bytes survives the call. */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "ezscreen.h"
#include "kfile.h"

// GLOBAL: EZ2AC_UNPACKED 0x004ae9dc
char g_binKey2[32];
// GLOBAL: EZ2AC_UNPACKED 0x004ae9fc
char g_binKey1[32];

// FUNCTION: EZ2AC_UNPACKED 0x00469c30
int SongTable::loadBinFile(const char *dir, const char *name)
{
    KFile         f;
    char          path[260];
    unsigned char *buf;
    unsigned int  size;
    unsigned int  i;
    int           j;
    int           n;
    int           r;
    int           g;
    int           p;

    // STRING: EZ2AC_UNPACKED 0x0049c7f0
    sprintf(path, "system\\%s\\%s", dir, name);
    f.open(path, 1);
    size = f.size();

    buf = (unsigned char *)VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
    f.read(buf, size);

    for (i = 0; i < size; i++)
    {
        buf[i] ^= g_binKey1[(i + 1) & 0x1f] ^ (char)i;
        for (j = 0; j < 0x20; j++)
        {
            int k = j ? j : 12;
            buf[i] ^= g_binKey2[i % k] ^ g_binKey2[j] ^ g_binKey1[j];
        }
    }

    n = *(unsigned short *)(buf + 6);
    count = n;
    for (r = 0; r < count; r++)
        memcpy(&entries[r], buf + *(int *)(buf + 8) + r * 0x56, 0x56);

    p = *(int *)(buf + 0xc);
    for (g = 0; g < 0x2f; g++)
    {
        int cnt = *(unsigned short *)(buf + p);
        int k;

        p += 2;
        for (k = 0; k < cnt; k++)
            memcpy(&groups[g][k], buf + p + k * 0x10, 0x10);
        groupCount[g] = cnt;
        p += cnt * 0x10;
    }

    VirtualFree(buf, 0, MEM_RELEASE);
    return 0;
}
