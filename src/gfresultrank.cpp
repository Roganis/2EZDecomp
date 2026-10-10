/* GFResultDirector::m466ee0 @0x466ee0 - the per-song top-five table, the GF
   twin of ResultDirector::m450690 (src/resultrank.cpp - the format, the
   frame, the fread-vs-stride bug and the swap quirks are all documented
   there and reproduced identically here). The deltas:

   - the seed record is the 8 bytes at "EZ2AC" (five characters, so the
     write takes three bytes of whatever .rdata holds next - kept);
   - the one-player path looks the player up with findPlayer @0x418f30 and
     reads its score at +(song+2)*64 with no +0x254 bias, and has no
     mode-10/11 override;
   - the game-over branch stores nothing beside the plate (no g_1b2efc0);
   - the plates land in m_295c[i] and the places in this class's own
     m_state @0x2db0.

   1,030 bytes, 26 calls. */

#include <stdio.h>
#include <io.h>
#include "ezscreen.h"
#include "directors.h"
#include "players.h"

extern int   countListC();                        /* 0x4184a0 */
extern int   getListAF8();                        /* 0x418340 (src/nodelists.cpp) */
extern ListNode *findListA(int key);              /* 0x418e30 */

extern char  g_modeName[256];                     /* 0x1b2e680 (src/modename.cpp) */
extern char  g_textSlots[32][128];                /* 0x1b2ec18 (src/modename.cpp) */
extern int   g_elemCount;                         /* 0x1b2e718 - the song index */
extern int   g_1b2ea28, g_1b2e7d8, g_1b5f178;

#define RANKREC 32

// FUNCTION: EZ2AC_UNPACKED 0x00466ee0
void GFResultDirector::m466ee0()
{
    char  path[260];
    int   names[5][RANKREC / 4];   /* an INT array - see src/resultrank.cpp */
    int   scores[5];
    FILE *f;
    int   i, j, k;

    sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);

    if (_access(path, 0) == -1)
    {
        f = fopen(path, "wb");
        for (i = 0; i < 5; i++)
        {
            fwrite("EZ2AC", 8, 1, f);
            scores[i] = 0;
        }
        fwrite(scores, 4, 5, f);
        fclose(f);
    }

    f = fopen(path, "rb");
    fread(names, 8, 5, f);      /* 40 contiguous bytes - see resultrank.cpp */
    fread(scores, 4, 5, f);
    fclose(f);

    /* Descending selection sort, carrying the name records with the scores. */
    for (i = 0; i < 4; i++)
    {
        for (j = i + 1; j < 5; j++)
        {
            int t = scores[i];

            if (t < scores[j])
            {
                scores[i] = scores[j];
                scores[j] = t;

                /* The char-temporary swap (src/resultrank.cpp) - dwords
                   1..4 only here (0x466fc6 anchors at the row + 4), where
                   the keys copy swaps 1..7. */
                {
                    int *a = names[i];
                    int *b = names[j];

                    for (k = 0; k < 4; k++)
                    {
                        char c = (char)a[k + 1];
                        a[k + 1] = b[k + 1];
                        b[k + 1] = c;
                    }
                }
            }
        }
    }

    f = fopen(path, "wb");
    fwrite(names, 8, 5, f);
    fwrite(scores, 4, 5, f);
    fclose(f);

    if (countListC() == 2)
    {
        for (i = 0; i < 2; i++)
        {
            int       e  = *(volatile int *)&g_elemCount;
            int       sc = *(int *)((char *)findListA(i) + 0x254 + e * 64);
            int       at;

            for (k = 0; k < 5; k++)
            {
                if (scores[k] < sc)
                {
                    m_state[i] = k;
                    break;
                }
            }

            at = m_state[i];
            if (at < 5)
            {
                for (k = 4; k > at; k--)
                    scores[k] = scores[k - 1];
                scores[at] = sc;
            }
        }

        if (m_state[1] <= m_state[0])
            m_state[0] += 1;
    }
    else if (countListC() == 1)
    {
        int sc;

        i = getListAF8();
        sc = *(int *)((char *)findPlayer(i) + (g_elemCount + 2) * 64);

        for (k = 0; k < 5; k++)
        {
            if (scores[k] < sc)
            {
                m_state[i] = k;
                break;
            }
        }
    }

    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\PLACE_f.bmp",
                                             &m_placeF, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\PLACE_c.bmp",
                                             &m_placeC, 0);

    if (countListC() == 1)
    {
        if (g_1b2ea28 == 1)
        {
            m_295c[0].setCell(1, 0, (int)m_placeF);
            return;
        }
        for (i = 0; i < 2; i++)
        {
            int place = m_state[i];

            if (place < 5)
            {
                if (g_1b5f178 >= 1)
                    m_295c[i].setCell(1, 0, (int)m_placeC);
                else
                    m_295c[i].setCell(1, 0, (int)f2d94[place]);
            }
            else
            {
                /* Both arms the same - the original keeps the test. */
                if (g_1b5f178 >= 1)
                    m_295c[i].setCell(1, 0, (int)m_placeC);
                else
                    m_295c[i].setCell(1, 0, (int)m_placeC);
            }
        }
        return;
    }

    if (g_1b2e7d8 >= 2)
    {
        m_295c[0].setCell(1, 0, (int)m_placeF);
        return;
    }
    /* The same walk again - written per half, as the original keeps it. */
    for (i = 0; i < 2; i++)
    {
        int place = m_state[i];

        if (place < 5)
        {
            if (g_1b5f178 >= 1)
                m_295c[i].setCell(1, 0, (int)m_placeC);
            else
                m_295c[i].setCell(1, 0, (int)f2d94[place]);
        }
        else
        {
            /* Both arms the same - the original keeps the test. */
            if (g_1b5f178 >= 1)
                m_295c[i].setCell(1, 0, (int)m_placeC);
            else
                m_295c[i].setCell(1, 0, (int)m_placeC);
        }
    }
}
