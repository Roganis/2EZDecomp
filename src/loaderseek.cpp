/* KEzwLoader::seek @0x40cf60 and KWavLoader::seek @0x40f1a0 - the two audio
   loaders' seek, integrated from wip/loaders-seek.cpp. The rest of both classes
   is matched in src/loaders.cpp; only ONE constant differs between the twins,
   the header size (0x12 for .ssf, 0x2c for .wav).

   THE SEMANTICS READ ODDLY AND ARE WHAT THE BYTES SAY. Arms 1 and 2 compute an
   ABSOLUTE position and then still hand fseek the caller's origin, so a
   SEEK_CUR or SEEK_END call moves twice; the default arm leaves pos at 0 and
   passes `offset` itself as the origin. Faithful, not tidy - a port that
   "fixes" either would desynchronise playback against the original.

   IT WAS PARKED AT 71.43% ON A BLOCK-PLACEMENT WALL, and integrating it
   deliberately - a decoded function sitting in wip/ is invisible to a build,
   and this one is finished for the port even though it is not byte-exact.
   IT NOW SCORES 85.71%. Nothing about the source changed; the park's number
   was measured before some of the reccmp operand patches landed, so a stale
   score in a wip note is not evidence of where a function stands today.
   Re-measure before believing a park.

       original:  sub eax,1 / jne DEFAULT      case2 case1 case0 DEFAULT clamp
       cl 13.10:  sub eax,1 / je  CASE2        DEFAULT case2 case1 case0 clamp

   cl always emits the default arm first, immediately after the dispatch tests,
   then the cases in descending order; the original emits the cases first and
   the default last, falling through into the join. Eleven source forms were
   compiled and the arm layout never moved - see the wip note for the full
   list. Two of them each fix HALF: an if/else-if chain gets the block order
   right but compiles to `test`/`cmp` instead of the original's destructive
   `sub` chain (and cl then folds `mode` away entirely, because it can prove
   `mode == whence` in each arm), and `int mode = offset;` with no default
   label also orders the blocks right but hoists `offset` into a register at
   the top, moving three other instructions. The `sub` chain is the fingerprint
   that says the original really is a switch, so the switch is what is kept. */

#include <stdio.h>
#include "loaders.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040cf60
int KEzwLoader::seek(int offset, int whence)
{
    int   pos = 0;
    int   mode;
    FILE *f = m_file;

    if (f == 0)
        return 0;

    switch (whence)
    {
    case 0:  pos = offset;           mode = 0;      break;
    case 1:  pos = m_pos + offset;   mode = 1;      break;
    case 2:  pos = m_total + offset; mode = 2;      break;
    default:                         mode = offset; break;
    }

    if (pos >= m_total)
        pos = m_total - 1;

    if (pos != m_pos)
    {
        if (fseek(f, pos + 0x12, mode) != 0)
            return 0;
        m_pos = pos;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f1a0
int KWavLoader::seek(int offset, int whence)
{
    int   pos = 0;
    int   mode;
    FILE *f = m_file;

    if (f == 0)
        return 0;

    switch (whence)
    {
    case 0:  pos = offset;           mode = 0;      break;
    case 1:  pos = m_pos + offset;   mode = 1;      break;
    case 2:  pos = m_total + offset; mode = 2;      break;
    default:                         mode = offset; break;
    }

    if (pos >= m_total)
        pos = m_total - 1;

    if (pos != m_pos)
    {
        if (fseek(f, pos + 0x2c, mode) != 0)
            return 0;
        m_pos = pos;
    }

    return 1;
}
