/* RankingDirector::m4570a0 @0x4570a0 - push the finished play into the
   CHANNEL ranking: ninety-nine places in
   system\ranking\ranking_%s_%s.bin keyed by the mode and channel
   names. The same shape as the song top-five (src/rankinginsert.cpp):
   seed "EZ2AC_FN" everywhere, read, sort descending carrying names,
   dedup against this entry's name from the SECOND table (f85dfc),
   insert the four-slot bonus sum, write back and clear the slot's
   pending bonus. No player gate and no node stamping here.

   Re-read against the listing 2026-10-01: the sort's name swap moves
   bytes 1..8 of the two records - the last of them is the NEXT record's
   first byte - which is the original's off-by-one (the C swapped 0..7);
   and the dedup compares straight against the name table (an inline
   two-dword `repz cmpsd`), with no copy of the name in the frame. */

#include <stdio.h>
#include "ezscreen.h"
#include "directors.h"

extern void Log(const char *fmt, ...);      /* 0x402880 (folded) */
extern char g_modeName[256];                /* 0x1b2e680 */
extern char g_channelName[0x80];            /* 0x1b2e728 */
extern int  g_elemCount;                    /* 0x1b2e718 */
extern int  g_bonusScore[4];                /* 0x1b2e54c */
extern int  g_1b2e590[5];                   /* src/totalresultctor.cpp */

#define NAME(k) (((char (*)[50])((char *)this + 0x85e04))[f85dfc][k])

// FUNCTION: EZ2AC_UNPACKED 0x004570a0
void RankingDirector::m4570a0()
{
    char  path[260];
    char  names[99][8];
    int   scores[99];
    FILE *f;
    int   i, j, sc, at, already;
    char *tbl;

    for (i = 0; i < 99; i++)
        scores[i] = 0;
    for (i = 0; i < 99; i++)
    {
        names[i][0] = 'E';
        names[i][1] = 'Z';
        names[i][2] = '2';
        names[i][3] = 'A';
        names[i][4] = 'C';
        names[i][5] = '_';
        names[i][6] = 'F';
        names[i][7] = 'N';
    }

    sprintf(path, "system\\ranking\\ranking_%s_%s.bin", g_modeName,
            g_channelName);
    f = fopen(path, "rb");
    if (f != 0)
    {
        fread(names, 8, 99, f);
        fread(scores, 4, 99, f);
        fclose(f);
    }

    sc = g_1b2e590[2] + g_1b2e590[3] + g_1b2e590[1] + g_1b2e590[0];

    at = 98;
    already = 0;
    for (i = 0; i < 98; i++)
    {
        for (j = i + 1; j < 99; j++)
        {
            int t = scores[i];
            int u = scores[j];

            if (t < u)
            {
                char b;

                scores[i] = u;
                scores[j] = t;
                /* Bytes 1..8, not 0..7 - the original's off-by-one: the
                   last byte swapped is the NEXT record's first. */
                b = names[i][1]; names[i][1] = names[j][1]; names[j][1] = b;
                b = names[i][2]; names[i][2] = names[j][2]; names[j][2] = b;
                b = names[i][3]; names[i][3] = names[j][3]; names[j][3] = b;
                b = names[i][4]; names[i][4] = names[j][4]; names[j][4] = b;
                b = names[i][5]; names[i][5] = names[j][5]; names[j][5] = b;
                b = names[i][6]; names[i][6] = names[j][6]; names[j][6] = b;
                b = names[i][7]; names[i][7] = names[j][7]; names[j][7] = b;
                b = names[i + 1][0]; names[i + 1][0] = names[j + 1][0]; names[j + 1][0] = b;

            }
        }
    }

    tbl = (char *)this + 0x85e04 + f85dfc * 50;

    for (i = 98; i >= 0; i--)
    {
        if (memcmp(names[i], tbl, 8) == 0)
        {
            if (scores[i] < sc)
                at = i;
            else
                already = 1;
        }
    }

    if (already == 0)
    {
        for (i = 0; i < 99; i++)
        {
            if (scores[i] < sc)
                break;
        }
        if (i < 99)
        {
            for (j = at; j > i; j--)
            {
                int k;

                scores[j] = scores[j - 1];
                for (k = 0; k < 8; k++)
                    names[j][k] = names[j - 1][k];
            }
            scores[i] = sc;
            names[i][0] = NAME(0);
            names[i][1] = NAME(1);
            names[i][2] = NAME(2);
            names[i][3] = NAME(3);
            names[i][4] = NAME(4);
            names[i][5] = NAME(5);
            names[i][6] = NAME(6);
            names[i][7] = NAME(7);
        }
    }

    sprintf(path, "system\\ranking\\ranking_%s_%s.bin", g_modeName,
            g_channelName);
    f = fopen(path, "wb");
    fwrite(names, 8, 99, f);
    fwrite(scores, 4, 99, f);
    fclose(f);

    g_bonusScore[g_elemCount] = 0;
    Log("Clear Bonus Score");
}
#undef NAME
