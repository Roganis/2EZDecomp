/* RankingDirector::m456b20 @0x456b20 - push the finished play into the
   song's top five. Seeds five "EZ2AC_FN" records, reads the same
   rank_%s_%s.bin the result screens keep (names[5][8] then scores[5]),
   adds the pending bonus (g_1b2e54c, logged and cleared here) to the
   node's score - the radio modes take their own accumulators with no
   bonus - sorts descending carrying the names, skips the insert when
   this exact entry name already sits at or above its score, writes the
   file back, and stamps the entry name (and the "ranked" flag) into
   the player node at +0x220, one findListA call per byte, as the
   original has it. Gated so a single-player game only runs for the
   screen's own side (getListAF8, or getListCF8 in mode 0xb).

   Re-read 2026-10-01: the sort's name swap moves bytes 1..8 (the
   original's off-by-one, as in the channel table @0x4570a0) and the dedup
   compares straight against the name table - no copy in the frame. */

#include <stdio.h>
#include "ezscreen.h"
#include "directors.h"
#include "players.h"

extern int countListA();                    /* 0x418310 */
extern int getListAF8();                    /* 0x418340 */
extern int getListCF8();                    /* 0x4184d0 */
extern struct ListNode *findListA(int key); /* 0x418e30 */
extern void Log(const char *fmt, ...);      /* 0x402880 (folded) */

extern char g_modeName[256];                /* 0x1b2e680 */
extern char g_textSlots[32][128];           /* 0x1b2ec18 */
extern int  g_elemCount;                    /* 0x1b2e718 */
extern int  g_modeIndex;                    /* 0x1b2e548 */
extern int  g_bonusScore[4];                /* 0x1b2e54c (src/rankingctor.cpp) */
extern int  g_1b2f058[5], g_1b2f094[5];

#define NAME(k) (((char (*)[50])((char *)this + 0x20cc))[f109b48][k])

// FUNCTION: EZ2AC_UNPACKED 0x00456b20
void RankingDirector::m456b20()
{
    char  path[260];
    char  names[5][8];
    int   scores[5];
    FILE *f;
    int   i, j, sc, at, already;
    char *tbl;
    char *song;
    int   e;

    scores[0] = 0;
    scores[1] = 0;
    scores[2] = 0;
    scores[3] = 0;
    scores[4] = 0;
    for (i = 0; i < 5; i++)
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

    if (countListA() == 1)
    {
        if (f109de4 != getListAF8())
            return;
    }
    else if (countListA() == 1 && g_modeIndex == 0xb)
    {
        if (f109de4 != getListCF8())
            return;
    }

    e = g_elemCount;
    song = g_textSlots[e];
    sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, song);
    f = fopen(path, "rb");
    if (f != 0)
    {
        fread(names, 8, 5, f);
        fread(scores, 4, 5, f);
        fclose(f);
    }

    sc = *(int *)((char *)findListA(f109de4) + 0x254 + e * 64);
    if (g_modeIndex == 10)
        sc = g_1b2f058[g_elemCount];
    else if (g_modeIndex == 11)
        sc = g_1b2f094[g_elemCount];
    at = 4;
    already = 0;
    if (g_modeIndex != 10 && g_modeIndex != 11)
        sc = sc + g_bonusScore[g_elemCount];

    // STRING: EZ2AC_UNPACKED 0x0049871c
    Log("Set Bonus Score = %d", g_bonusScore[g_elemCount]);

    /* Descending sort, carrying the name records. */
    for (i = 0; i < 4; i++)
    {
        for (j = i + 1; j < 5; j++)
        {
            if (scores[i] < scores[j])
            {
                int t = scores[i];
                char b;

                scores[i] = scores[j];
                /* Bytes 1..8, the original's off-by-one (see
                   src/rankingchannel.cpp): byte 8 is the next record's. */
                b = names[i][1]; names[i][1] = names[j][1]; names[j][1] = b;
                b = names[i][2]; names[i][2] = names[j][2]; names[j][2] = b;
                b = names[i][3]; names[i][3] = names[j][3]; names[j][3] = b;
                b = names[i][4]; names[i][4] = names[j][4]; names[j][4] = b;
                b = names[i][5]; names[i][5] = names[j][5]; names[j][5] = b;
                b = names[i][6]; names[i][6] = names[j][6]; names[j][6] = b;
                b = names[i][7]; names[i][7] = names[j][7]; names[j][7] = b;
                b = names[i + 1][0]; names[i + 1][0] = names[j + 1][0];
                scores[j] = t;
                names[j + 1][0] = b;
            }
        }
    }

    /* This entry's name, straight out of the ranking table. */
    tbl = (char *)this + 0x20cc + f109b48 * 50;

    /* If the same name is already ranked at or above this score, do not
       double-insert; otherwise remember its row as the shift floor. */
    for (i = 4; i >= 0; i--)
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
        for (i = 0; i < 5; i++)
        {
            if (scores[i] < sc)
                break;
        }
        if (i < 5)
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

    sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, song);
    f = fopen(path, "wb");
    fwrite(names, 8, 5, f);
    fwrite(scores, 4, 5, f);
    fclose(f);

    g_bonusScore[g_elemCount] = 0;
    // STRING: EZ2AC_UNPACKED 0x00498708
    Log("Clear Bonus Score");

    /* Stamp the name into the player node, one lookup per byte - as
       the original spells it - then the ranked flag. */
    ((char *)findListA(f109de4))[0x220] = NAME(0);
    ((char *)findListA(f109de4))[0x221] = NAME(1);
    ((char *)findListA(f109de4))[0x222] = NAME(2);
    ((char *)findListA(f109de4))[0x223] = NAME(3);
    ((char *)findListA(f109de4))[0x224] = NAME(4);
    ((char *)findListA(f109de4))[0x225] = NAME(5);
    ((char *)findListA(f109de4))[0x226] = NAME(6);
    ((char *)findListA(f109de4))[0x227] = NAME(7);
    *(int *)((char *)findListA(f109de4) + 0x228) = 1;
}
#undef NAME
