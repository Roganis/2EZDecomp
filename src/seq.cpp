#include "ezchart.h"

/* Per-lane playback queue filled by LoadEZ @0x411460: 0x800 records + play
   cursor + count = 0x14008 bytes per track. */
struct Track
{
    Record28 records[0x800];  /* 0x00000 */
    int      cur;             /* 0x14000 play cursor */
    int      count;           /* 0x14004 record count */
};

/* Global track array (element count inferred from the 96-lane chart model;
   only the base address matters for matching). */
// GLOBAL: EZ2AC_UNPACKED 0x00d8ae38
extern Track g_tracks[96];
Track g_tracks[96];

/* Sequencer: pick the track in [lo,hi] whose next unplayed record has the
   earliest tick (d0, compared UNSIGNED); ties go to the lowest index
   (downward scan with <=). Returns the track index, or -1 if the range is
   empty or no track has pending records. Free __cdecl leaf.
   The pointer is anchored at the cur field so the +0x14000 folds into the
   absolute base (original reads 0xd9ee38) and the record lea carries -0x2800;
   *q must be dereferenced twice to force the address materialization. */
/* EzChart slots 6/7 stage lane records through g_tracks: extractLane
   @0x412690 (stubbed) copies a chart lane into a track (setting cur=0 and
   count), the loop restamps the staged records' lane byte, mergeTracks
   @0x411310 (stubbed) sort-inserts the staged tracks back into the chart. */

// FUNCTION: EZ2AC_UNPACKED 0x00412790
void EzChart::copyLane(int dstLane, int srcLane)
{
    extractLane(srcLane, 0);
    for (int i = 0; i < g_tracks[0].count; i++)
    {
        g_tracks[0].records[i].byte5 = (unsigned char)dstLane;
    }
    mergeTracks(0, 1);
}

/* Multi-track variant (slot 7). The outer loop provably repeats the same
   track each pass in the original (esi constant) — matched as-is. */
// FUNCTION: EZ2AC_UNPACKED 0x004127e0
void EzChart::copyLanes(int dstLane, int srcLane, int count)
{
    int i;
    for (i = 0; i < count; i++)
    {
        Track *t = &g_tracks[dstLane];
        extractLane(srcLane, dstLane);
        for (int j = 0; j < t->count; j++)
            t->records[j].byte5 = (unsigned char)dstLane;
    }
    mergeTracks(dstLane, count);
}

// FUNCTION: EZ2AC_UNPACKED 0x00411250
int SeqFindEarliestTrack(int lo, int hi)
{
    unsigned int best = (unsigned int)-1;
    int result = -1;
    int i = hi;
    if (i >= lo)
    {
        int *p = &g_tracks[i].cur;
        do
        {
            int cur = p[0];
            if (cur < p[1])
            {
                unsigned int *q = (unsigned int *)((char *)p + cur * 0x28 - 0x14000);
                if (*q <= best)
                {
                    result = i;
                    best = *q;
                }
            }
            i--;
            p -= sizeof(Track) / sizeof(int);
        } while (i >= lo);
    }
    return result;
}

/* Filter-less sibling of the parked findNext @0x411d90 (that one also takes
   a byte4 filter); this variant has NO standalone address — it exists only
   inlined into extractLane below. */
static __forceinline int FindNextInLane(EzChart *self, int lane, int after)
{
    if (lane < -1 || lane >= self->i98)
        return -1;
    if (after < -1)
        return -1;
    int cnt = self->i94;
    if (after >= cnt - 1)
        return -1;
    int i = after + 1;
    if (i >= cnt)
        return -1;

    unsigned char *p = &self->records[i].byte5;
    do
    {
        if (lane == -1 || *p == (unsigned char)lane)
            return i;
        i++;
        p += sizeof(Record28);
    } while (i < cnt);
    return -1;
}

/* Copy every record whose byte5 == lane (all records if lane == -1) into
   g_tracks[track], then set its count and rewind its play cursor. Defined
   AFTER its callers copyLane/copyLanes (caller-first rule). */
// FUNCTION: EZ2AC_UNPACKED 0x00412690
void EzChart::extractLane(int lane, int track)
{
    Track *t = &g_tracks[track];
    int count = 0;
    Record28 *dst = t->records;

    int i = FindNextInLane(this, lane, -1);
    while (i != -1)
    {
        count++;
        *dst++ = records[i];
        i = FindNextInLane(this, lane, i);
    }

    t->count = count;
    t->cur = 0;
}
