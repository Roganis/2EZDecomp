/* Obj426CA0::m4265b0 @0x4265b0 - the judge's "is another hit in the way"
   walker, in a TU of its own so that m4266c0 (src/accessors.cpp), which
   calls it, cannot inline it. Reads record i's two widest windows (+0x28
   early / +0x40 late = a.w[2] / b.w[2] as accessors.cpp names them), then
   walks every OTHER sound claimed on `lane`: 1 if any has a plain hit
   starting before now + late, or a hold hit starting after now - early
   (unsigned throughout, the lower bound clamped at 0). */

#include "players.h"

/* The two lane-slot views src/accessors.cpp owns (Obj426D80 there). */
struct WalkLaneSlot { int kind; int count; int ids[16]; };   /* 0x48 */
struct WalkRec { int f0; char _pad4[0x28 - 4]; unsigned int early; char _pad2c[0x40 - 0x2c]; unsigned int late; char _pad44[0x5c - 0x44]; };

struct Obj426CA0
{
    char         _pad0[0x14];
    WalkRec      recs[96];      /* 0x0014 .. 0x2294 */
    int          count;         /* 0x2294 */
    WalkLaneSlot lanes[8];      /* 0x2298 .. 0x24d8 */
    int          f24d8;
    SlotSampleSink *sink;       /* 0x24dc */

    int  m4265b0(unsigned int now, int i, int lane);
};

// FUNCTION: EZ2AC_UNPACKED 0x004265b0
int Obj426CA0::m4265b0(unsigned int now, int i, int lane)
{
    WalkLaneSlot *slot;
    unsigned int early;
    unsigned int late;
    SlotState st;
    SlotSound sn;
    unsigned int tmin;
    unsigned int tmax;
    int k;

    if (lane == -1)
        return 0;
    early = recs[i].early;
    late = recs[i].late;
    if (now < early)
        tmin = 0;
    else
        tmin = now - early;
    tmax = late + now;
    slot = &lanes[lane];
    for (k = 0; k < slot->count; k++)
    {
        int id = slot->ids[k];
        if (id == i)
            continue;
        if (sink->begin(id, now, 1, 1, &st, &sn) == 1 && (unsigned int)st.start < tmax)
            return 1;
        if (sink->begin(id, now, 2, 1, &st, &sn) == 1 && (unsigned int)st.start > tmin)
            return 1;
    }
    return 0;
}
