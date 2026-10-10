/* TrackSink::m42d370 @0x42d370 - the keys panel's PRESS flash, reached
   through the slot's sink pointer so it keeps the TrackSink identity;
   the body is the Panel's. On a press of `lane` with grade-kind `a`:
   an auto-played lane (the counter object confirms it, no sustain mode,
   a real grade, and the slot's wide-hit byte down) only clears the
   bomb's per-lane latch; otherwise the latch (or the second table for
   kind 3) clears, the kind's flash timer arms at 15, the press lamps
   light - per player two-up, both banks one-up - the press-flash chart
   re-attaches with its x eased to fe4, the combo chart fires every
   fifth combo with head (combo / 10) % 3 - so a head holds for two
   firings - the RubyKey chart picks its head by kind, and the `optional`
   chart fires once per slot f214 window.

   The head index is a SEPARATE division (magic 0x66666667, `sar 2` =
   /10) from the %5 gate's idiv; the C had `c / 5 % 3`, which cl folded
   into the gate's quotient, until 2026-10-01 (docs/PORT-DELTAS.md). The
   press-chart floats go through named values and a named pointer read
   once after the attach (cookbook). What is left is register choice -
   which scratch register carries the handle, the lamp index and the
   slot pointer; a local for the slot pointer and dropping the `p` local
   do not move it. */
#include "players.h"
#include "gamepanels.h"
#include <math.h>
#include "bigbase.h"
#include "sink-view.h"

extern int  countListA();           /* 0x418310 (src/nodelists.cpp) */
extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */
extern int  g_1b5f2e0;              /* src/holdtick.cpp */
extern int  kPressLamps[12];        /* 0x490420 (src/globals.cpp) */

struct Obj426CA0 { int getF0(int i); };   /* 0x426d10 */

// FUNCTION: EZ2AC_UNPACKED 0x0042d370
void TrackSink::m42d370(int i, int a, int b)
{
    Panel      *p = (Panel *)this;
#define SLOT ((PlayerSlot *)p->slot)

    p->f20 = 1;
    if (((Obj426CA0 *)SLOT->f3dc)->getF0(SLOT->f3d0->tracks[SLOT->id].items[i].handle) == 1 &&
        g_1b5f2e0 == 0)
    {
        if (a >= 4)
        {
            if (*((char *)SLOT + 0xe105b8) == 0)
            {
                p->s1460.f98[i] = 0;
                return;
            }
        }
    }
    if (a >= 4)
        p->s1460.f98[i] = 0;
    else if (a == 3)
        p->f1620[i] = 0;
    p->f3230[a] = 0xf;
    if (countListA() > 1)
        g_bigInstance->setOutput(kPressLamps[SLOT->id * 6 + a], 1);
    else
    {
        g_bigInstance->setOutput(kPressLamps[a], 1);
        g_bigInstance->setOutput(kPressLamps[a + 6], 1);
    }
    if (p->f1410 != 0)
    {
        p->f1440->attach((&p->f141c)[a], 0);
        float x = p->f1414 + p->fe4;
        float y = p->f1418;
        Obj402860 *o = p->f1440;
        o->f214 = x;
        o->f218 = y;
    }
    if (p->f1dd4 != 0)
    {
        int c = SLOT->f1dc;

        if (c >= 5 && c % 5 == 0)
            p->f1de4->attach((&p->f1dd8)[SLOT->f1dc / 10 % 3], 0);
    }
    if (p->f1de8 != 0)
    {
        if (a == 2 || a == 1)
            p->f1e04->attach(p->f1e00, 0);
        else
            p->f1e04->attach(0, 0);
    }
    if (SLOT->f214 != 0 && SLOT->f5b4 != 0)
    {
        p->optional->attach(p->f329c, 0);
        SLOT->f5b4 = 0;
    }
#undef SLOT
}

extern int countListB();            /* 0x4183e0 (src/nodelists.cpp) */
extern int g_1b2ef1c;               /* the catch invert */
extern int kCatchPressLamps[12];    /* 0x49e138 (src/globals.cpp) */

/* CatchPanel::m472300 @0x472300 - the catch press flash, the keys
   twin's dialect: judges 3 and 4 fold to 0; a judge-5 (a caught one)
   clears the bomb's per-lane latch and - outside invert, with the
   gauge item live - feeds the catch gauge log10(combo)/12 eight
   pixels a side; the judge's flash timer arms at 15; the press lamps
   light per player two-up, both banks one-up; the press chart
   re-attaches with its own float pair; and the combo chart fires
   every fifth combo with head (combo / 10) % 3, as the keys twin
   (the C had /5 until 2026-10-01). */
// FUNCTION: EZ2AC_UNPACKED 0x00472300
void CatchPanel::m472300(int lane, int judge, int flag)
{
    if (judge == 3 || judge == 4)
        judge = 0;
    f248 = 1;
    if (judge == 5)
    {
        s156c.f98[lane] = 0;
        if (g_1b2ef1c <= 0 && s1b04.f00 != 0)
        {
            int n = ((PlayerSlot *)slot)->f1dc;

            if (n > 0)
                f1b24 = (float)log10((double)n) * 0.0833333358f + f1b24;
            f1b2c = f1b2c + 8.0f;
            f1b28 = f1b28 + 8.0f;
        }
    }
    f1c64[judge] = 0xf;
    if (countListB() > 1)
        g_bigInstance->setOutput(kCatchPressLamps[((PlayerSlot *)slot)->id * 6 + judge], 1);
    else
    {
        g_bigInstance->setOutput(kCatchPressLamps[judge], 1);
        g_bigInstance->setOutput(kCatchPressLamps[judge + 6], 1);
    }
    if (f1544 != 0)
    {
        f1568->attach((&f1550)[judge], 0);
        float x = f1548;
        float y = f154c;
        Obj402860 *o = f1568;
        o->f214 = x;
        o->f218 = y;
    }
    if (f1c50 != 0)
    {
        int c = ((PlayerSlot *)slot)->f1dc;

        if (c >= 5 && c % 5 == 0)
            f1c60->attach((&f1c54)[((PlayerSlot *)slot)->f1dc / 10 % 3], 0);
    }
}

extern int countListC();            /* 0x4184a0 (src/nodelists.cpp) */
extern int kGFPressLamps[12];       /* 0x49a460 (src/globals.cpp) */

/* GFTrackSink::m4651a0 @0x4651a0 - the scratch press flash, the third
   dialect: a real grade (>= 4) clears the CoolBomb's per-lane latch
   and feeds the gauge log10(combo)/12 with eight a side (no invert
   gate here, and the combo comes off the sink slot's own counter);
   grade 3 clears the GoodBomb's latch instead. The flash timer, the
   countListC lamp split, and the press chart - its x eased by the
   sway fe0 - follow the family; the RubyKey arm returns through its
   own attach(0) epilogue. */
// FUNCTION: EZ2AC_UNPACKED 0x004651a0
void GFTrackSink::m4651a0(int i, int a, int b)
{
    GFPanel *p = (GFPanel *)this;

    p->f1c = 1;
    if (a >= 4)
    {
        p->s1400.f98[i] = 0;
        if (p->s1aa0.f00 != 0)
        {
            int n = ((GFSinkSlot *)p->slot)->combo;

            if (n > 0)
                p->f1ac0 = (float)log10((double)n) * 0.0833333358f + p->f1ac0;
            p->f1ac8 = p->f1ac8 + 8.0f;
            p->f1ac4 = p->f1ac4 + 8.0f;
        }
    }
    else if (a == 3)
        p->s1528.f98[i] = 0;
    p->f3030[a] = 0xf;
    if (countListC() > 1)
        g_bigInstance->setOutput(kGFPressLamps[((PlayerSlot *)p->slot)->id * 6 + a], 1);
    else
    {
        g_bigInstance->setOutput(kGFPressLamps[a], 1);
        g_bigInstance->setOutput(kGFPressLamps[a + 6], 1);
    }
    if (p->f13d8 != 0)
    {
        p->f13fc->attach((&p->f13e4)[a], 0);
        {
            float      vy = p->f13e0;
            float      vx = p->f13dc + p->fe0;
            Obj402860 *ch = p->f13fc;

            ch->f214 = vx;
            ch->f218 = vy;
        }
    }
    if (p->f1bec != 0)
    {
        if (a == 2 || a == 1)
            p->f1c08->attach(p->f1c04, 0);
        else
            p->f1c08->attach(0, 0);
    }
}
