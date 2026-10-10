/* KEZPlayer::m40f690 @0x40f690 - rebuild the 96-entry tick table, the tail
   call of the reset @0x410110. It has to stay OUT of that reset's TU: cl
   would inline it and the original's `jmp` would disappear.

   The table is 96 records of 0x30 bytes filling +0x10 .. +0x1210 exactly.
   m410230 @0x410230 clears only each record's object pointer at +0x24, so
   it indexes from +0x34 - the same addresses, a different anchor, and
   modelling the array from its real base is what lets both spell it as the
   class's own member. Every record is zeroed and then given its defaults: a
   1, three -1s, two 0x7f levels and a 0x40.

   The second pass hands each record a pointer into the bound sample's own
   table - 0x4c-byte entries starting at its +0x9c, one per live entry. The
   sample pointer is re-read from the member at every use, which is what the
   original's reload of [ebx+4] inside the loop says: the store through the
   record could alias it.

   100%* EFFECTIVE: the second loop's `lea` swaps the SIB base and index -
   the original bases on the offset counter and indexes by the sample
   pointer, cl does the reverse. Same seven bytes, same address, and no
   spelling of the pointer arithmetic moves it (four were swept: the offset
   first, the offset last, a parenthesised inner sum, and an int + pointer
   form). Everything else is byte-identical. */

#include <string.h>
#include "ksongplayer.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040f690
void KEZPlayer::m40f690()
{
    int  *p;
    void **d;
    int   n;
    int   i;
    int   k;

    memset(entries, 0, sizeof(entries));

    /* Anchored at the record's +0x10, which is where the original puts the
       walking pointer - `lea eax,[this+0x20]` and displacements from -0x10
       to +0x10. Anchoring it at the record base instead makes cl share the
       address with the memset's and keep it in ebp for the whole function. */
    p = &entries[0].f10;
    n = 96;
    do
    {
        p[-4] = 1;      /* f00 */
        p[-1] = -1;     /* f0c */
        p[0]  = -1;     /* f10 */
        p[1]  = -1;     /* f14 */
        p[2]  = 0x7f;   /* f18 */
        p[4]  = 0x40;   /* f20 */
        p[3]  = 0x7f;   /* f1c */
        p = (int *)((char *)p + 0x30);
    }
    while (--n != 0);

    if (f04 != 0)
    {
        i = 0;
        if (i < ((Counted *)f04)->count)
        {
            k = 0;
            d = &entries[0].src;
            do
            {
                *d = (char *)f04 + 0x9c + k;
                i++;
                k += 0x4c;
                d = (void **)((char *)d + 0x30);
            }
            while (i < ((Counted *)f04)->count);
        }
    }
}

/* Pick the sample bound for id `w`: each id owns TWO slots of the pair table
   and a round-robin cursor in m_scratch; step the cursor (mod 2) up to twice
   until a non-null slot comes up, park the cursor there, and hand the slot
   back - null when both are empty.

   WHAT THE TWO SLOTS HOLD (read across createSamples @0x40f800,
   createSample @0x40f7b0 and m40f880 @0x40f880 - src/kezplayer.cpp and
   src/kezplayerreset.cpp): the pair is NOT two equal copies.
     word 0 = the .ezi loader's OWN sample object, stored unconditionally
              for every id the loader holds (borrowed: m40f880 only clears
              the word, it never frees it);
     word 1 = a CLONE made through the subsystem's slot 5
              (KSoundDS::_CreateSample @0x40eb80 - its own DirectSound
              buffer, duplicated from word 0's), made ONLY when bit 0 of the
              sample's +0x04 is clear, i.e. for a static sample; a streamed
              sample (bit 0 set, the 0x58000-byte ring) gets no clone
              (owned: m40f880 hands it back to the subsystem).
   So a static keysound has two independently playable voices and this
   round-robin alternates between them per trigger, while a stream has one
   and always comes back to it. m40fbf0 @0x40fbf0 installs whichever came
   up WITHOUT stopping the previous one (unless the event byte is 0x14), and
   m40f520 @0x40f520 then stops-rewinds-plays the NEW voice only - so two
   quick triggers of one static sample overlap on two buffers rather than
   cutting each other. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f650
IVolTarget *KEZPlayer::m40f650(int w)
{
    int cur = m_scratch[w];
    int i = 0;
    IVolTarget *t;

    do
    {
        cur += 1;
        if (cur >= 2)
            cur = 0;
        t = (IVolTarget *)m_pairs[w * 2 + cur];
        i += 1;
    } while (t == 0 && i < 2);

    m_scratch[w] = cur;
    return t;
}
