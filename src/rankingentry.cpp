/* RankingDirector::m456870 @0x456870 - the constructor's go/no-go for
   name entry. Reads the mode's five-slot rank file (rank_/e_rank_ by the
   0x1b2eb70 flag), defaulting to five "EZ2AC_FN" / 0 entries; takes the
   winner's score from their list node (+0x254 + g_elemCount*0x40), or
   the course totals g_1b2f058/g_1b2f094 in modes 10/11; selection-sorts
   the five slots descending - swapping only bytes 1..7 of each 8-byte
   name, as the original does - and answers 1 when the score belongs in
   the table and the player's mode-wide name (names[rank[0]]) is not
   already there with an equal-or-better score. */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "directors.h"

struct ListNode;
extern ListNode *findListA(int key);   /* 0x418e30 */

extern char g_modeName[256];      /* 0x1b2e680 (src/modename.cpp) */
extern char g_textSlots[32][128]; /* 0x1b2ec18 (src/modename.cpp) */
extern int  g_elemCount;          /* 0x1b2e718 (src/loops.cpp) */
extern int  g_modeIndex;          /* 0x1b2e548 (src/modename.cpp) */
extern int  g_1b2eb70;            /* src/resultctor.cpp */
extern int  g_1b2f058[5];         /* src/gamestate.cpp */
extern int  g_1b2f094[5];         /* src/resultctor.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x00456870
int RankingDirector::m456870()
{
    char  path[260];
    char  names[5][8];
    int   scores[5];
    int   score;
    int   dup;
    int   i;
    int   j;
    FILE *f;
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
    e = g_elemCount;
    if (g_1b2eb70 != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00491c80 is owned by src/songselect.cpp
        sprintf(path, "sound\\e_rank_%s_%s.bin", g_modeName,
                g_textSlots[e]);
    }
    else
    {
        sprintf(path, "sound\\rank_%s_%s.bin", g_modeName,
                g_textSlots[e]);
    }
    f = fopen(path, "rb");
    if (f != 0)
    {
        fread(names, 8, 5, f);
        fread(scores, 4, 5, f);
        fclose(f);
    }

    score = *(int *)((char *)findListA(f109de4) + e * 0x40 + 0x254);
    if (g_modeIndex == 0xa)
        score = g_1b2f058[g_elemCount];
    else if (g_modeIndex == 0xb)
        score = g_1b2f094[g_elemCount];

    dup = 0;
    for (i = 0; i < 4; i++)
    {
        for (j = i + 1; j < 5; j++)
        {
            if (scores[i] < scores[j])
            {
                int   t;
                char  c;

                t = scores[i];
                scores[i] = scores[j];
                c = names[i][1]; names[i][1] = names[j][1]; names[j][1] = c;
                c = names[i][2]; names[i][2] = names[j][2]; names[j][2] = c;
                c = names[i][3]; names[i][3] = names[j][3]; names[j][3] = c;
                c = names[i][4]; names[i][4] = names[j][4]; names[j][4] = c;
                c = names[i][5]; names[i][5] = names[j][5]; names[j][5] = c;
                c = names[i][6]; names[i][6] = names[j][6]; names[j][6] = c;
                c = names[i][7]; names[i][7] = names[j][7];
                scores[j] = t;
                names[j][7] = c;
            }
        }
    }

    for (i = 4; i >= 0; i--)
    {
        if (memcmp(names[i],
                   (char *)this + f109b48 * 0x32 + 0x20cc, 8) == 0 &&
            scores[i] >= score)
            dup = 1;
    }
    if (dup == 0)
    {
        for (i = 0; i < 5; i++)
        {
            if (scores[i] < score)
                return 1;
        }
    }
    return 0;
}
