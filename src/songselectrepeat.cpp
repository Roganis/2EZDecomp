/* SongSelectDirector::m446440 @0x446440 - the two hold-repeat scans.

   Each half is the same shape: if its hold button is down (0x11 for the
   first player, 0x19 for the second) and the list belongs to that player or
   holds both, walk a table of input codes and, for each that is currently
   held, step the repeat for BOTH players. Otherwise clear that half's latch.

   The two halves SHARE ONE CURSOR into the table at 0x491858. The first
   half starts at its head and leaves the index one past the terminator when
   it runs, so the second half then finds -1 immediately and does nothing;
   when the first half does NOT run the index is still zero and the second
   half walks the whole table. That is one walk split by which hold button is
   down, not two independent loops, and it is what the original's shared ebx
   says. */

#include "songselect.h"
#include "bigbase.h"     /* Obj40C180, g_bigInstance */

// GLOBAL: EZ2AC_UNPACKED 0x00491858
const int g_repeatCodes[15] = { 6, 7, 8, 9, 10, 11, 12, 13, 14,
                                18, 19, 20, 21, 22, -1 };

extern int countListA();     /* 0x418310 (src/nodelists.cpp) */
extern int getListAF8();     /* 0x418340 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00446440
void SongSelectDirector::m446440()
{
    const int *p;
    int i;
    int code;

    i = 0;
    if (g_bigInstance->getState(0x11) == 3 &&
        (getListAF8() == 0 || countListA() == 2))
    {
        code = 6;
        do
        {
            if (g_bigInstance->getState(code) == 2)
            {
                m4462e0(0, code);
                m4462e0(1, code);
            }
            code = g_repeatCodes[i + 1];
            i++;
        }
        while (code != -1);
    }
    else
    {
        seqLen[0] = 0;
    }

    if (g_bigInstance->getState(0x19) == 3 &&
        (getListAF8() == 1 || countListA() == 2))
    {
        /* WALK WITH `p++; code = *p;`, NOT `code = p[1]; p++;`. The two
           are the same statement pair in the other order and they are not
           the same code: reading p[1] lets cl fold the +1 into the pointer,
           so it anchors one element ALONG and reads [ebx] where the original
           reads [ebx+4] - and the `lea` that materializes the pointer then
           lands after the guard's `je` instead of before it. Bumping first
           and dereferencing keeps the anchor on the entry itself. 91.02% ->
           100%; the index form and guarding on *p both leave it at 91.02%. */
        p = &g_repeatCodes[i];
        code = *p;
        if (code != -1)
        {
            do
            {
                if (g_bigInstance->getState(code) == 2)
                {
                    m4462e0(0, code);
                    m4462e0(1, code);
                }
                p++;
                code = *p;
            }
            while (code != -1);
        }
    }
    else
    {
        seqLen[1] = 0;
    }
}
