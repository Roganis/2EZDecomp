/* SongSelectDirector::m4462e0 @0x4462e0 - THE COMMAND-CODE MATCHER.

   The song-select screen watches for entered command SEQUENCES. Each player
   has ten slots at the director's +0x20eab4 and a length beside them at
   +0x20eb04; this appends one command to that player's buffer and then walks
   the command table @0x4aaf18 - three entries of ten dwords, terminated by
   -1 - looking for one whose first `len` entries are what has been entered.

   Three outcomes:
     the buffer is a PREFIX of some command      -> keep it and wait
     it is a WHOLE command (the next table entry
     is the -1 terminator) and the option it
     names accepts the flip                      -> clear BOTH players'
                                                    buffers and this length
     it matches nothing                          -> restart the buffer from
                                                    this one command

   The "keep it" flag is set as soon as the prefix matches, before the
   terminator is tested, which is why a prefix of a longer command survives
   even when it is not itself complete.

   The table entry index doubles as toggleOption's argument, so the three
   commands map onto the three option toggles in order. The command DATA
   itself lives in the original's .rdata and is left as an external load. */

#include "songselect.h"
#include <string.h>

/* The command table: three sequences of up to ten commands, each terminated
   by -1. Placeholder storage - the contents are the original's.

   DELIBERATELY OVER-SIZED. The loop's bound is the one-past-the-end address
   0x4aaf90, and a bound is an ADDRESS that has to resolve the same way in
   both images: at exactly thirty entries it lands on whatever the linker
   puts next and our side names THAT, where the original's names its own
   table. Ten spare entries keep the bound interior to this array on both
   sides. */
// GLOBAL: EZ2AC_UNPACKED 0x004aaf18
int g_commandCodes[40];

// FUNCTION: EZ2AC_UNPACKED 0x004462e0
void SongSelectDirector::m4462e0(int who, int code)
{
    int  found;
    int  k;
    int *p;

    m_seq[who * 10 + seqLen[who]] = code;
    seqLen[who] += 1;

    found = 0;
    p = &m_seq[who * 10];

    /* ONE signed loop variable, not a walking pointer: cl strength-reduces
       k*10 into both the dword index the terminator test uses and the
       pointer memcmp gets, but keeps the SIGNED compare the source wrote -
       which is what makes the bound test `jl` rather than a pointer's
       `jb`. */
    for (k = 0; k < 3; k++)
    {
        if (memcmp(&g_commandCodes[k * 10], p, seqLen[who] * 4) == 0)
        {
            found = 1;
            if (g_commandCodes[seqLen[who] + k * 10] == -1)
            {
                if (toggleOption(k) != 0)
                {
                    memset(m_seq, 0, sizeof(m_seq));
                    seqLen[who] = 0;
                    return;
                }
            }
        }
    }

    if (found == 0)
    {
        seqLen[who] = 1;
        p[0] = code;
    }
}
