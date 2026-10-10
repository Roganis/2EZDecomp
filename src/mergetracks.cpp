/* EzChart::mergeTracks @0x411310 - sort-merge the staged tracks back into the
   chart's own record list. Integrated from wip/mergetracks.cpp,
   which recorded 85.58%; it measures 80.40% today.

   It snapshots the current records into the scratch buffer at 0x150b138,
   zeroes each staged track's play cursor, then runs a two-way merge of the
   scratch stream against the earliest-tick pending track record into the
   records array, stores the new count and rebuilds the index through virtual
   slot 23. Its callers copyLane and copyLanes are matched in src/seq.cpp.

   EXHAUSTED, AND THE DIAGNOSIS CHECKS OUT. Every instruction, immediate,
   displacement and relocation matches; what differs is the colouring of two
   callee-saved registers, with the scratch cursor and the track-base pointer
   swapped between ebx and edi:

     original  counter -> ebp   track -> ebx   a(=sc) -> edi   b -> esi
     ours      counter -> ebp   track -> edi   a(=sc) -> ebx   b -> esi

   Three named variants plus roughly a hundred permutations were swept: naming
   the counter `count` puts it in ebx (wrong axis entirely), renaming it to
   `consumed` and adding an explicit `Track *tr = 0;` fixes the counter but
   swaps a against track, and declaring `tr` before a/b breaks the counter
   again with a 54-instruction diff. Reordering the three loop-body pointer
   declarations moves the errors around without ever landing all four at once,
   which is the tight-coupling signature.

   AND ITS RECORDED SCORE WAS OPTIMISTIC - 85.58% in the note against 80.40%
   measured. That is the opposite direction from the two parks whose numbers
   had drifted UP with the reccmp operand patches, so a stale park number can
   move either way and neither direction is safe to quote.

   The note offers one explanation - that the original's colouring is forced by
   the prologue memcpy's rep movs, esi carrying into `b` and edi into `a` - and
   that half does not survive reading the draft: `a` already IS the memcpy's
   destination here and still lands in ebx, while `b` is a track record rather
   than the memcpy's source. So the mechanism is unexplained even though the
   sweep is thorough; recorded as such rather than repeated. */

#include <string.h>
#include "ezchart.h"

struct Track { Record28 records[0x800]; int cur; int count; };
extern Track g_tracks[96];
int SeqFindEarliestTrack(int lo, int hi);

// GLOBAL: EZ2AC_UNPACKED 0x0150b138
extern Record28 g_recScratch[0x5000];
Record28 g_recScratch[0x5000];

// FUNCTION: EZ2AC_UNPACKED 0x00411310
void EzChart::mergeTracks(int first, int count)
{
    if (count >= 1)
    {
        int last = first + count - 1;
        memcpy(g_recScratch, records, i94 * sizeof(Record28));
        for (int i = first; i <= last; i++)
            g_tracks[i].cur = 0;

        int outCount = 0;
        Record28 *sc = g_recScratch;
        Record28 *dst = records;
        int consumed = 0;
        for (;;)
        {
            Record28 *a = 0;
            Record28 *b = 0;
            Track *tr = 0;
            if (consumed < i94)
                a = sc;
            int t = SeqFindEarliestTrack(first, last);
            if (t != -1)
            {
                tr = &g_tracks[t];
                b = &tr->records[tr->cur];
            }
            if (a == 0 && b == 0)
                break;
            if (a != 0 && (b == 0 || recordCompare(b, a) > 0))
            {
                consumed++;
                sc++;
                b = a;
            }
            else
            {
                tr->cur++;
            }
            outCount++;
            *dst++ = *b;
        }
        i94 = outCount;
        rebuildIndex();
    }
}
