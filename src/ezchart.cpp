#include <string.h>
#include "kcrt.h"   /* operator delete = free, for the deleting destructor */
#include "ezchart.h"
#include "loader.h"   /* EziLoader - the same object seen by the sample loader */

/* The default tempo constant (120 BPM). The original binary is packer-dumped
   with its relocation table stripped, so reccmp can't auto-detect the float
   pool entry at 0x4887c4; pin it with a GLOBAL marker and reference it. */
// GLOBAL: EZ2AC_UNPACKED 0x004887c4
extern const float k120;
const float k120 = 120.0f;

/* Placeholder storage for the class-registry record and the KSound runtime
   class descriptor (contents unreconstructed; only the addresses matter). */
// GLOBAL: EZ2AC_UNPACKED 0x004a7370
EzClassInfo g_ezChartClassInfo;

// GLOBAL: EZ2AC_UNPACKED 0x004a7300
KRuntimeClass classKSound;

// FUNCTION: EZ2AC_UNPACKED 0x00410960
EzClassInfo *EzChart::getClassInfo()
{
    return &g_ezChartClassInfo;
}

/* Validate/lookup a KSound and attach it at this+4. */
// FUNCTION: EZ2AC_UNPACKED 0x00410970
int EzChart::setSound(KSound *snd)
{
    if (snd == 0)
    {
        int idx = -1;
        snd = findSoundObject(&idx, &classKSound);
        if (snd == 0)
            return 0;
    }
    else
    {
        if (!snd->getRuntimeClass()->isDerivedFrom(&classKSound))
            return 0;
    }
    f4 = (int)snd;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004109d0
void EzChart::reset()
{
    f8c = k120;
    b8 = 0;
    b48 = 0;
    i88 = 0xc0;
    f90 = 0.0f;
    i98 = 0;
    i94 = 0;
    f4 = 0;
    memset(groups, 0, sizeof(groups));
    memset(records, 0, sizeof(records));
    memset(sampleTable, 0, sizeof(sampleTable));
}

// FUNCTION: EZ2AC_UNPACKED 0x004112a0
void EzChart::rebuildIndex()
{
    int j = 0;
    if (i98 > 0)
    {
        Group4C *g = groups;
        do
        {
            g->f48 = 0;
            g->f44 = 0;
            j++;
            g++;
        } while (j < i98);
    }

    int i = 0;
    if (i94 > 0)
    {
        Record28 *r = records;
        do
        {
            Group4C *gb = &groups[r->byte5];
            gb->f48++;
            gb->f44 = r->d0;
            i++;
            r++;
        } while (i < i94);
    }
}

/* Thin wrappers around vtable slot 11 (search): the calls are indirect
   ([eax+0x2c]) because search is virtual and cl 13.10 never devirtualizes. */
// FUNCTION: EZ2AC_UNPACKED 0x00411010
int EzChart::findFirst(int lane, Record28 *outRec, RecInfo *outInfo)
{
    return search(lane, 0, 0, 0, outRec, outInfo);
}

// FUNCTION: EZ2AC_UNPACKED 0x00411030
int EzChart::findLast(int lane, Record28 *outRec, RecInfo *outInfo)
{
    return search(lane, -1, 2, 0, outRec, outInfo);
}

// FUNCTION: EZ2AC_UNPACKED 0x00411b80
void EzChart::getState(int *out, int flags)
{
    if (flags & 1)
        out[0] = (int)&b8;
    if (flags & 2)
        out[1] = *(int *)&f8c;
    if (flags & 4)
        out[2] = i88 / 4;
    if (flags & 8)
        out[3] = 4;
    if (flags & 0x10)
        out[4] = records[i94 - 1].d0;
    if (flags & 0x20)
        out[5] = *(int *)&f90;
    if (flags & 0x40)
        out[6] = i98;
    if (flags & 0x80)
        out[7] = i94;
}

/* Setter mirror of getState: flag 1 = name string, flag 4 = rate (rescales all
   record times to the new rate). Defined BEFORE rescale so cl cannot inline
   the call (the original has a real `call 0x411c10`). */
// FUNCTION: EZ2AC_UNPACKED 0x00411cd0
void EzChart::setState(int *in, int flags)
{
    if (flags & 1)
        strcpy(&b8, (const char *)in[0]);
    if (flags & 4)
    {
        rescale(in[2], i88 / 4);
        i88 = in[2] * 4;
    }
}

/* Scale all record timestamps by newRate/oldRate, then reindex via virtual
   slot 23. The (unsigned) casts produce the x87 fild + 2^32-fixup fadd and the
   __ftol2 truncating store; the compiler-generated pool constant needs the
   __real@41f0000000000000 GLOBAL-by-name pair in src/library.cpp. */
// FUNCTION: EZ2AC_UNPACKED 0x00411c10
void EzChart::rescale(int newRate, int oldRate)
{
    if (newRate == oldRate)
        return;

    Record28 *r = records;
    double ratio = (double)newRate / (double)oldRate;
    for (int i = 0; i < i94; i++, r++)
    {
        r->d0 = (unsigned int)((unsigned int)r->d0 * ratio);
        if (r->byte4 == 1)
        {
            if (r->word14 > 6)
                r->word14 = (unsigned short)(r->word14 * ratio);
        }
    }
    rebuildIndex();
}

/* Flag-driven group-field extractor, sibling of getState. */
// FUNCTION: EZ2AC_UNPACKED 0x00411d30
int EzChart::getGroupInfo(int idx, int *out, int flags)
{
    if (idx >= 0 && idx < i98)
    {
        Group4C *g = &groups[idx];
        if (flags & 1)
            out[0] = (int)g->name;
        if (flags & 2)
            out[1] = g->f44;
        if (flags & 4)
            out[2] = g->f48;
        return 1;
    }
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00411e00
int EzChart::findPrev(int lane, int before, int filter)
{
    if (lane < -1 || lane >= i98)
        return -1;
    if (before <= 0)
        return -1;
    if (before > i94)
        return -1;
    int i = before - 1;
    if (i < 0)
        return -1;

    unsigned char *p = &records[i].byte4;
    unsigned char f = *(volatile unsigned char *)&filter;
    do
    {
        if (lane == -1 || p[1] == (unsigned char)lane)
        {
            if (f == 0 || p[0] == f)
                return i;
        }
        i--;
        p -= sizeof(Record28);
    } while (i >= 0);
    return -1;
}

/* Mode-dispatched record search (vtable slot 11): dispatches to the four
   by-time finders below (defined AFTER this so cl cannot inline them), then on
   hit fills the RecInfo pair and copies the record. Mode order 0/1/2/3 =
   findFrom/findNextAfter/findPrevAt/findBefore — modes 1/2 are swapped vs
   address order, do not "fix". */
// FUNCTION: EZ2AC_UNPACKED 0x00412020
int EzChart::search(int lane, int value, int mode, int filter,
                    Record28 *outRec, RecInfo *outInfo)
{
    if (lane >= -1 && lane < i98)
    {
        int idx;
        switch (mode)
        {
        case 0: idx = findFrom(lane, value, filter); break;
        case 1: idx = findNextAfter(lane, value, filter); break;
        case 2: idx = findPrevAt(lane, value, filter); break;
        case 3: idx = findBefore(lane, value, filter); break;
        default: idx = -1; break;
        }
        if (idx != -1)
        {
            if (outInfo != 0)
            {
                outInfo->lane = lane;
                outInfo->index = idx;
            }
            if (outRec != 0)
                *outRec = records[idx];
            return 1;
        }
        if (outInfo != 0)
        {
            outInfo->lane = lane;
            outInfo->index = -1;
        }
    }
    return 0;
}

/* The same scan keyed on an INDEX rather than a tick: start at `from + 1`
   and hand back the first record whose lane and filter match, with no
   position test at all. Its only caller is the entry-kind dispatcher
   @0x40ff50, which is parked in wip/entrykind.cpp. Found by
   scratchpad/twins.py at 68% against findFrom below - the guards and the
   missing tick test are the difference. */
// FUNCTION: EZ2AC_UNPACKED 0x00411d90
int EzChart::findAfter(int lane, int from, int filter)
{
    if (lane < -1 || lane >= i98)
        return -1;

    if (from >= -1)
    {
        int cnt = i94;

        if (from < cnt - 1)
        {
            int i = from + 1;

            if (i < cnt)
            {
                unsigned char *p = &records[i].byte4;
                unsigned char f = *(volatile unsigned char *)&filter;

                do
                {
                    if (lane == -1 || p[1] == (unsigned char)lane)
                    {
                        if (f == 0 || p[0] == f)
                            return i;
                    }
                    i++;
                    p += sizeof(Record28);
                }
                while (i < cnt);
            }
        }
    }
    return -1;
}

/* Forward scan for the first record at tick >= pos (unsigned). */
// FUNCTION: EZ2AC_UNPACKED 0x00411e60
int EzChart::findFrom(int lane, unsigned int pos, int filter)
{
    if (lane < -1 || lane >= i98)
        return -1;

    int cnt = i94;
    int i = 0;
    if (cnt > 0)
    {
        unsigned char f = *(volatile unsigned char *)&filter;
        unsigned char *p = &records[0].byte4;
        do
        {
            if (lane == -1 || p[1] == (unsigned char)lane)
            {
                if (f == 0 || p[0] == f)
                {
                    if (*(unsigned int *)(p - 4) >= pos)
                        return i;
                }
            }
            i++;
            p += sizeof(Record28);
        } while (i < cnt);
    }
    return -1;
}

/* Backward scan for the latest record at tick <= time (unsigned). */
// FUNCTION: EZ2AC_UNPACKED 0x00411ed0
int EzChart::findPrevAt(int lane, unsigned int time, int filter)
{
    if (lane < -1 || lane >= i98)
        return -1;
    int i = i94 - 1;
    if (i >= 0)
    {
        unsigned char *p = &records[i].byte4;
        unsigned char f = *(volatile unsigned char *)&filter;
        do
        {
            if (lane == -1 || p[1] == (unsigned char)lane)
            {
                if (f == 0 || p[0] == f)
                {
                    if (*(unsigned int *)(p - 4) <= time)
                        return i;
                }
            }
            i--;
            p -= sizeof(Record28);
        } while (i >= 0);
    }
    return -1;
}

/* Forward scan for the first record at tick > time (unsigned). */
// FUNCTION: EZ2AC_UNPACKED 0x00411f40
int EzChart::findNextAfter(int lane, unsigned int time, int filter)
{
    if (lane < -1 || lane >= i98)
        return -1;

    int cnt = i94;
    int i = 0;
    if (cnt > 0)
    {
        unsigned char f = *(volatile unsigned char *)&filter;
        unsigned char *p = &records[0].byte4;
        do
        {
            if (lane == -1 || p[1] == (unsigned char)lane)
            {
                if (f == 0 || p[0] == f)
                {
                    if (*(unsigned int *)(p - 4) > time)
                        return i;
                }
            }
            i++;
            p += sizeof(Record28);
        } while (i < cnt);
    }
    return -1;
}

/* Backward scan for the newest record at tick < time (unsigned). The i<0
   early-exit must share the loop-exhaustion tail via the goto (pins push esi
   above the js); p/f are declared uninitialized up top because the goto may
   cross their initialization. */
// FUNCTION: EZ2AC_UNPACKED 0x00411fb0
int EzChart::findBefore(int lane, unsigned int time, int filter)
{
    unsigned char *p;
    unsigned char f;
    int i;

    if (lane < -1 || lane >= i98)
        return -1;
    i = i94 - 1;
    if (i < 0)
        goto notfound;

    p = &records[i].byte4;
    f = *(volatile unsigned char *)&filter;
    do
    {
        if (lane == -1 || p[1] == (unsigned char)lane)
        {
            if (f == 0 || p[0] == f)
            {
                if (*(unsigned int *)(p - 4) < time)
                    return i;
            }
        }
        i--;
        p -= sizeof(Record28);
    } while (i >= 0);
notfound:
    return -1;
}

/* Cursor-based forward record search (slot 12, the forward mirror of
   findPrevRecord below — same cursor protocol, scans up from index+1). */
// FUNCTION: EZ2AC_UNPACKED 0x00412100
int EzChart::findNextRecord(int filter, Record28 *out, RecInfo *info)
{
    int lane = info->lane;
    if (lane < -1 || lane >= i98)
        return 0;
    int idx = info->index;
    if (idx < 0 || idx >= i94)
        return 0;
    int cnt = i94;
    int i = idx + 1;
    if (i < cnt)
    {
        unsigned char *p = &records[i].byte4;
        unsigned char f = (unsigned char)filter;
        do
        {
            Record28 *r = (Record28 *)(p - 4);
            if (lane == -1 || p[1] == lane)
            {
                if (f == 0 || p[0] == f)
                {
                    info->index = i;
                    if (out)
                        *out = *r;
                    return 1;
                }
            }
            i++;
            p += sizeof(Record28);
        } while (i < cnt);
    }
    info->index = -1;
    return 0;
}

/* Cursor-based backward record search (the RecInfo sibling of findPrev):
   scans down from info->index-1; on hit updates info->index and copies the
   record out; early bound failures return 0 without touching info->index.
   The `Record28 *r` at the top of the loop body is load-bearing: it re-derives
   the record base per iteration (lea esi,[eax-4]) instead of strength-reducing
   into a second induction register. */
// FUNCTION: EZ2AC_UNPACKED 0x004121b0
int EzChart::findPrevRecord(int filter, Record28 *out, RecInfo *info)
{
    int lane = info->lane;
    if (lane < -1 || lane >= i98)
        return 0;
    int idx = info->index;
    if (idx <= 0 || idx >= i94)
        return 0;
    int i = idx - 1;
    if (i >= 0)
    {
        unsigned char *p = &records[i].byte4;
        unsigned char f = (unsigned char)filter;
        do
        {
            Record28 *r = (Record28 *)(p - 4);
            if (lane == -1 || p[1] == lane)
            {
                if (f == 0 || p[0] == f)
                {
                    info->index = i;
                    if (out)
                        *out = *r;
                    return 1;
                }
            }
            i--;
            p -= sizeof(Record28);
        } while (i >= 0);
    }
    info->index = -1;
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00412250
int EzChart::find(int lane, int nth, Record28 *outRec, int *outInfo)
{
    if (lane < -1 || lane >= i98)
        return 0;

    int cnt = i94;
    int matched = 0;
    int i = 0;
    if (cnt > 0)
    {
        Record28 *r = records;
        do
        {
            if (lane == -1 || r->byte5 == lane)
            {
                if (nth == matched)
                {
                    if (outInfo != 0)
                    {
                        outInfo[0] = lane;
                        outInfo[1] = i;
                    }
                    if (outRec != 0)
                        *outRec = *r;
                    return 1;
                }
                matched++;
            }
            i++;
            r++;
        } while (i < cnt);
    }

    if (outInfo != 0)
    {
        outInfo[0] = lane;
        outInfo[1] = -1;
    }
    return -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004122f0
int EzChart::getRecord(RecInfo *info, Record28 *out)
{
    if (info->lane < -1 || info->lane >= i98)
        return 0;
    int idx = info->index;
    if (idx < 0 || idx >= i94)
        return 0;
    *out = records[idx];
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00412340
int EzChart::setRecord(RecInfo *info, Record28 *src)
{
    if (info->lane < -1 || info->lane >= i98)
        return 0;
    int idx = info->index;
    if (idx < 0 || idx >= i94)
        return 0;
    records[idx] = *src;
    return 1;
}

/* Sorted insert (slot 19): copy *src, stamp lane/tick, find the position via
   locate (defined LATER in this TU — caller-first, no inlining), raw-insert
   via virtual slot 20, falling back to append (tables.cpp) when no
   predecessor sorts before it. The Record28 local is why Record28's pads must
   be short/int typed (see ezchart.h) — char pads would add a /GS cookie. */
// FUNCTION: EZ2AC_UNPACKED 0x004124b0
int EzChart::insert(int lane, int tick, Record28 *src, RecInfo *out)
{
    if (lane >= 0 && lane < i98)
    {
        RecInfo pos;
        Record28 rec = *src;
        rec.byte5 = (unsigned char)lane;
        rec.d0 = tick;
        if (locate(&rec, &pos) == 1)
        {
            int r = insertAt(&rec, &pos);
            if (r == 1 && out != 0)
            {
                out->lane = pos.lane;
                out->index = pos.index;
            }
            return r;
        }
        return append(&rec, out);
    }
    return 0;
}

/* Raw insert primitive (slot 20): shift records up one slot from
   info->index+1 and drop the record in. Defined BEFORE copyRecords so the
   call stays a real call. */
// FUNCTION: EZ2AC_UNPACKED 0x00412560
int EzChart::insertAt(Record28 *rec, RecInfo *info)
{
    int idx = info->index + 1;
    int dst = idx + 1;
    if (idx < 0x5000 && dst < 0x5000)
    {
        copyRecords(dst, idx, i94 - idx);
        records[idx] = *rec;
        i94++;
        return 1;
    }
    return 0;
}

/* Raw insert-before primitive (slot 21): insertAt minus the +1 bias — shifts
   up from info->index and drops the record in AT that index. Also
   caller-first vs copyRecords. */
// FUNCTION: EZ2AC_UNPACKED 0x004125d0
int EzChart::insertBefore(Record28 *rec, RecInfo *info)
{
    int idx = info->index;
    int dst = idx + 1;
    if (idx < 0x5000 && dst < 0x5000)
    {
        copyRecords(dst, idx, i94 - idx);
        records[idx] = *rec;
        i94++;
        return 1;
    }
    return 0;
}

/* Block-copy count records from records[src] to records[dst] (regions may
   overlap). Must be memmove, NOT memcpy: /Oi expands memcpy as the inline
   rep-movs intrinsic, but the original emits a real call to the CRT routine
   at 0x479630 (annotated as memmove in src/library.cpp). */
// FUNCTION: EZ2AC_UNPACKED 0x00412390
void EzChart::copyRecords(int dst, int src, int count)
{
    if (src != dst)
    {
        if (count > 0)
            memmove(&records[dst], &records[src], count * sizeof(Record28));
    }
}

/* Backward scan for the last record sorting strictly before *key per
   recordCompare — an insertion-point locator for the time-sorted record list. */
// FUNCTION: EZ2AC_UNPACKED 0x004123e0
int EzChart::locate(Record28 *key, RecInfo *out)
{
    int i = i94 - 1;
    if (i >= 0)
    {
        Record28 *r = &records[i];
        do
        {
            if (recordCompare(r, key) < 0)
            {
                out->lane = r->byte5;
                out->index = i;
                return 1;
            }
            i--;
            r--;
        } while (i >= 0);
    }
    out->lane = -1;
    out->index = -1;
    return 0;
}

/* Remove the record at info->index (slot 22): the body is copyRecords
   INLINED (defined above — callee-first on purpose), shifting everything
   down one slot; the dead src!=dst compare is the inlined guard. */
// FUNCTION: EZ2AC_UNPACKED 0x00412630
int EzChart::removeRecord(RecInfo *info)
{
    int next = info->index + 1;
    copyRecords(next - 1, next, i94 - next);
    i94--;
    return 1;
}

/* Slot 3 is a plain getter for the attached sound, ICF-folded onto
   Obj40F380::getA @0x40f380 (src/accessors.cpp, which sorts first and so owns
   the address). */
// FUNCTION: EZ2AC_UNPACKED 0x0040f380 folded
int EzChart::getSound()
{
    return f4;
}

/* Unload the samples through the loader view of this same object (EziLoader and
   EzChart are one class under two placeholder names - src/loader.cpp holds
   unloadSamples @0x411050 and calls it with EzChart's own `this`), then reset.
   The base's empty destructor is the `call 0x402880` at EH state -1. */
// FUNCTION: EZ2AC_UNPACKED 0x004128f0
EzChart::~EzChart()
{
    ((EziLoader *)this)->unloadSamples();
    reset();
}

// FUNCTION: EZ2AC_UNPACKED 0x00412950
// EzChart::`scalar deleting destructor'

/* The class-registry factory (slot 8 of g_ezChartClassInfo). `new EzChart` -
   operator new is the inlined malloc from kcrt.h, and the constructor is
   expanded here because it is defined inline in the header. */
// FUNCTION: EZ2AC_UNPACKED 0x00412860
EzChart *createEzChart()
{
    return new EzChart();
}

// VTABLE: EZ2AC_UNPACKED 0x00488a10
struct EzChart;
