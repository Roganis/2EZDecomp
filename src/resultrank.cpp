/* ResultDirector::m450690 @0x450690 - the per-song top-five table: read it,
   sort it, work out where this play's score lands, and pick each player's
   place plate.

   THE FILE. `sound\rank_%s_%s.bin`, named from the song info name at
   g_modeName at 0x1b2e680 and a second name out of g_textSlots (stride 128).
   If it does not exist it is created with five 8-byte "EZ2AC_TT" name records
   followed by five zeroed 4-byte scores, and that is exactly the layout it is
   then read back with. So the format is: 5 x char[8], then 5 x int32, 60 bytes.

   THE FRAME, derived by hand - scratchpad/frame.py restores esp at every
   `call` and these calls are cdecl, so its offsets drift from the first
   sprintf onward and it reports the _access argument as a second buffer when
   the raw displacements make it the same one. Entry esp = F:

     F-0x1c4  `this`                          F-0x1a8  names[5][32]  (160)
     F-0x1c0  the sort's outer counter        F-0x108  path[260]
     F-0x1bc  scores[5]              (20)     F-0x004  the /GS cookie

   That accounts for all 0x1c4 with no slack: the names end exactly on the
   path buffer's base and the path ends exactly on the cookie, which is what
   fixes it at 260 rather than 256.

   AND THAT ARITHMETIC IS WHAT EXPOSES A BUG IN THE ORIGINAL. The names array
   is 160 bytes - the sort strides it by 0x20, and 5 x 0x20 lands precisely on
   the path buffer - but the fread that fills it is `fread(names, 8, 5, f)`,
   which writes forty CONTIGUOUS bytes. So only record 0 is filled from its own
   file record; record 1 gets the tail of the packed block and records 2..4 get
   nothing. The names are never displayed by this function, which is why it has
   never mattered. Reproduced faithfully here rather than silently repaired -
   a port that "fixes" it would write a file the cabinet cannot read back.

   The swap is a second bug of the original's: the seven dwords at offsets
   0..24 of the two records are exchanged through a CHAR temporary, so the
   near record gets the far one's dword and the far record gets the near
   dword's LOW BYTE, sign-extended (bytes 28..31 never move). Corrected
   2026-10-01 - this file used to read it as "bytes 4..31, only the first
   dword through a char"; the unrolled listing is 7 x (movsx byte [a+4k],
   dword [b+4k]). And the records are an INT array: that is what puts them
   below the path buffer in the /GS frame, as the original has it.

   THE PLACE. Two players: each player's score for this song is read from its
   list node at +0x254 + songIndex*64, its insertion point is found in the
   sorted table and INSERTED (shifting the rest down), and if the second
   player did not beat the first the first is pushed down one so they cannot
   claim the same place. One player: the same lookup, but modes 10 and 11 take
   the score from g_1b2f058/g_1b2f094 instead, and the place is only found -
   nothing is inserted. A player who beats nothing keeps whatever m_state
   already held, which is what the `>= 5` test downstream is for.

   Written for the port. 1,194 bytes, 377 instructions, 30 calls. */

#include <stdio.h>
#include <io.h>
#include "ezscreen.h"
#include "directors.h"
#include "players.h"

extern int   countListA();                        /* 0x418310 (src/nodelists.cpp) */
extern int   getListAF8();                        /* 0x418340 (src/nodelists.cpp) */
extern ListNode *findListA(int key);              /* 0x418e30 */

extern char  g_modeName[256];                     /* 0x1b2e680 (src/modename.cpp) */
extern char  g_textSlots[32][128];                /* 0x1b2ec18 (src/modename.cpp) */
extern int   g_elemCount;                         /* 0x1b2e718 - the song index */
extern int   g_modeIndex;                         /* 0x1b2e548 (src/modename.cpp) */
extern int   g_1b2f058[5], g_1b2f094[5];
extern int   g_1b2efc0[4];
extern int   g_1b2ea28, g_1b2e7d8, g_1b5f178;

/* Each name record is 32 bytes in memory even though the file holds 8. */
#define RANKREC 32

// FUNCTION: EZ2AC_UNPACKED 0x00450690
void ResultDirector::m450690()
{
    char  path[260];
    int   names[5][RANKREC / 4];
    int   scores[5];
    FILE *f;
    int   i, j, k;

    sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);

    if (_access(path, 0) == -1)
    {
        f = fopen(path, "wb");
        for (i = 0; i < 5; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x00495c44
            fwrite("EZ2AC_TT", 8, 1, f);
            scores[i] = 0;
        }
        fwrite(scores, 4, 5, f);
        fclose(f);
    }

    f = fopen(path, "rb");
    fread(names, 8, 5, f);      /* see the header - 40 contiguous bytes */
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

                /* Seven dwords swapped through a CHAR temporary - the
                   original's bug: the near record keeps the far one's
                   dword, the far record gets the near dword's low byte,
                   sign-extended. Dwords 1..7 (bytes 4..31); the first
                   dword never moves - the swap pointer is the row + 4
                   (0x450776: `lea ecx,[esp+0x30]` against the fread's
                   `+0x2c`). */
                {
                    int *a = names[i];
                    int *b = names[j];

                    for (k = 0; k < 7; k++)
                    {
                        char t = (char)a[k + 1];
                        a[k + 1] = b[k + 1];
                        b[k + 1] = t;
                    }
                }
            }
        }
    }

    /* Sorted, so put it back - the same two writes the seed path uses. */
    f = fopen(path, "wb");
    fwrite(names, 8, 5, f);
    fwrite(scores, 4, 5, f);
    fclose(f);

    if (countListA() == 2)
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

            /* Untouched above means this score beat nothing, and m_state keeps
               whatever it already held - which the >= 5 test below catches. */
            at = m_state[i];
            if (at < 5)
            {
                for (k = 4; k > at; k--)
                    scores[k] = scores[k - 1];
                scores[at] = sc;
            }
        }

        /* Two players cannot hold one place. */
        if (m_state[1] <= m_state[0])
            m_state[0] += 1;
    }
    else if (countListA() == 1)
    {
        int       id = getListAF8();
        int       e  = *(volatile int *)&g_elemCount;
        int       sc = *(int *)((char *)findListA(id) + 0x254 + e * 64);

        if (g_modeIndex == 10)
            sc = g_1b2f058[g_elemCount];
        else if (g_modeIndex == 11)
            sc = g_1b2f094[g_elemCount];

        for (k = 0; k < 5; k++)
        {
            if (scores[k] < sc)
            {
                m_state[id] = k;
                break;
            }
        }
    }

    // STRING: EZ2AC_UNPACKED 0x00495c28
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\PLACE_f.bmp",
                                             &m_placeF, 0);
    // STRING: EZ2AC_UNPACKED 0x00495c0c
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\result\\PLACE_c.bmp",
                                             &m_placeC, 0);

    if (countListA() == 1)
    {
        if (g_1b2ea28 == 1)
        {
            m_2ca4[0].setCell(1, 0, (int)m_placeF);
            g_1b2efc0[g_elemCount] = 1;
            return;
        }
        for (i = 0; i < 2; i++)
        {
            int place = m_state[i];

            /* The >= 5 arm tests g_1b5f178 too and does the same thing
               either way - the original keeps the dead compare. */
            if (place < 5)
            {
                if (g_1b5f178 >= 1)
                    m_2ca4[i].setCell(1, 0, (int)m_placeC);
                else
                    m_2ca4[i].setCell(1, 0, (int)m_rankPlate[place]);
            }
            else
            {
                if (g_1b5f178 >= 1)
                    m_2ca4[i].setCell(1, 0, (int)m_placeC);
                else
                    m_2ca4[i].setCell(1, 0, (int)m_placeC);
            }
        }
        return;
    }

    if (g_1b2e7d8 >= 2)
    {
        m_2ca4[0].setCell(1, 0, (int)m_placeF);
        return;
    }
    /* The same walk again - the original writes it out per half rather than
       sharing it, which is what its two copies of the block are. */
    for (i = 0; i < 2; i++)
    {
        int place = m_state[i];

        if (place < 5)
        {
            if (g_1b5f178 >= 1)
                m_2ca4[i].setCell(1, 0, (int)m_placeC);
            else
                m_2ca4[i].setCell(1, 0, (int)m_rankPlate[place]);
        }
        else
        {
            if (g_1b5f178 >= 1)
                m_2ca4[i].setCell(1, 0, (int)m_placeC);
            else
                m_2ca4[i].setCell(1, 0, (int)m_placeC);
        }
    }
}
