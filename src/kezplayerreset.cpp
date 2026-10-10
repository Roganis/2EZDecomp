/* KEZPlayer::m410110 @0x410110 - put the player back to its just-constructed
   state.

   Both embedded players are re-aimed at this object (KSongPlayer slot 1,
   setTarget, taking the owner as its target - the same wiring the
   constructor does), every counter is cleared, the speed goes back to 120
   and the master level to 0x7f, the global rate scale to 1.0, and both
   tables - the 0x1000-dword sample pair array and the 0x800-dword scratch -
   are wiped. The tail call rebuilds the 96-entry tick table.

   120.0f is the shared k120 pool constant (src/ezchart.cpp), which is why it
   is referenced by name rather than written as a literal; the 1.0f is an
   ordinary anonymous pool entry. */

#include <string.h>
#include "ksongplayer.h"
#include "ez2acapp.h"   /* EzSubsystem::freeSample @0x40e620 */

extern const float k120;   /* 0x4887c4 (src/ezchart.cpp) - 120 BPM */
extern float g_1b2e708;    /* 0x1b2e708 (src/kezplayer.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00410110
void KEZPlayer::m410110()
{
    f04 = 0;
    f08 = 0;
    f0c = 0;

    player0.setTarget((Slot34Target *)this);
    player1.setTarget((Slot34Target *)this);

    f7260 = k120;
    f7258 = 0;
    f725c = 0;
    g_1b2e708 = 1.0f;
    f7264 = 0;
    f7268 = 0;
    f726c = 0x7f;
    f7274 = 0;
    f7278 = 0;

    memset(m_pairs, 0, sizeof(m_pairs));
    memset(m_scratch, 0, sizeof(m_scratch));

    m40f690();
}

/* KEZPlayer::m40f880 @0x40f880 - drop every created sample and wipe both
   tables. The helper both binders run before they take a new chart or source.

   m_pairs is 0x800 PAIRS: the first word of each is the .ezi loader's own
   sample (borrowed - so it is only cleared, never freed here; the loader
   owns it), the second holds the CLONE the subsystem made through
   KSoundDS::_CreateSample and is handed back to it -
   but only while the subsystem itself is still there, which is why the null
   test on f08 sits INSIDE the sample test rather than hoisted out of the
   loop. The whole pass is skipped when nothing is bound (f04), and either
   way both tables are then memset whole, which re-clears the words the loop
   just wrote.

   The loop walks TWO pointers eight bytes apart rather than indexing, and
   needs the explicit down-counter the cookbook calls for - 0x800 iterations
   over a body this small is otherwise unrolled.

   82.19%, and the residue is one optimisation cl applies that the original's
   build did not: it MERGES the two induction variables, computing their
   (constant, -4) difference into a register in the preheader and addressing
   the flag as `[edi+esi*1]`, where the original simply walks both pointers
   and advances each by 8. Six forms give the identical listing - the two
   pointers in either declaration order, computed from `&m_pairs[i]` or from
   `(char *)this + 0x1210` separately, the flag as a `char *` advanced by 8,
   a single pointer read as p[0]/p[1] (59.5%, two instructions short) and a
   plain index (73.0%, also short). The two-pointer form is the right reading
   - it is the only one with the original's instruction COUNT - so the
   merging is the allocator's, not the source's. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f880
void KEZPlayer::m40f880()
{
    if (f04 != 0)
    {
        int *flag = &m_pairs[0];
        int *samp = &m_pairs[1];
        int  n = 0x800;

        do
        {
            *flag = 0;
            if (*samp != 0 && f08 != 0)
            {
                ((EzSubsystem *)f08)->freeSample((void *)*samp);
                *samp = 0;
            }
            flag += 2;
            samp += 2;
        }
        while (--n != 0);
    }

    memset(m_pairs, 0, sizeof(m_pairs));
    memset(m_scratch, 0, sizeof(m_scratch));
}
