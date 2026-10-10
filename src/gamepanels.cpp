/* The three game panels' teardown (see include/gamepanels.h).

   Each destructor clears a fixed list of global chart pairs, indexed by the
   panel's slot, and then lets its own charts go. The globals are placeholder
   storage pinned at the original's addresses - the same convention as
   src/globals.cpp - and are declared in ADDRESS order; the destructors call
   them in the original's order, which is not address order in any of the
   three. */

#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gamepanels.h"
#include "players.h"   /* SlotOwner, TrackList */
#include "gfxtext.h"
#include "ez2acapp.h"   /* KGraphics, g_sub1b2f054 */
#include "loader.h"     /* KTexture */
#include "kparser.h"    /* the .gds readers */

extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */

/* Panel's eight chart pairs (0x37da9d8 .. 0x37dc980). */
// GLOBAL: EZ2AC_UNPACKED 0x037da9d8
Obj402860 g_panelCharts0[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dae10
Obj402860 g_panelCharts1[2];

// GLOBAL: EZ2AC_UNPACKED 0x037db248
Obj402860 g_panelCharts2[2];

// GLOBAL: EZ2AC_UNPACKED 0x037db680
Obj402860 g_panelCharts3[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dbab8
Obj402860 g_panelCharts4[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dbef0
Obj402860 g_panelCharts5[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dc328
Obj402860 g_panelCharts6[2];

/* A NINTH chart, and the only one that is not part of a pair: ComboEffect
   @0x429650 pins it directly instead of indexing it by the slot. Its own
   0x21c ends four bytes short of g_panelCharts7, so the gap between the two
   pairs is exactly one Obj402860 plus alignment - which is what says this is
   a global of its own rather than g_panelCharts6[1]. */
// GLOBAL: EZ2AC_UNPACKED 0x037dc760
Obj402860 g_panelChartSingle;

// GLOBAL: EZ2AC_UNPACKED 0x037dc980
Obj402860 g_panelCharts7[2];

/* GFPanel's seven (0x37dd498 .. 0x37dede8). */
// GLOBAL: EZ2AC_UNPACKED 0x037dd498
Obj402860 g_gfPanelCharts0[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dd8d0
Obj402860 g_gfPanelCharts1[2];

// GLOBAL: EZ2AC_UNPACKED 0x037ddd08
Obj402860 g_gfPanelCharts2[2];

// GLOBAL: EZ2AC_UNPACKED 0x037de140
Obj402860 g_gfPanelCharts3[2];

// GLOBAL: EZ2AC_UNPACKED 0x037de578
Obj402860 g_gfPanelCharts4[2];

// GLOBAL: EZ2AC_UNPACKED 0x037de9b0
Obj402860 g_gfPanelCharts5[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dede8
Obj402860 g_gfPanelCharts6[2];

/* CatchPanel's six (0x37df540 .. 0x37e0a58). */
// GLOBAL: EZ2AC_UNPACKED 0x037df540
Obj402860 g_catchPanelCharts0[2];

// GLOBAL: EZ2AC_UNPACKED 0x037df978
Obj402860 g_catchPanelCharts1[2];

// GLOBAL: EZ2AC_UNPACKED 0x037dfdb0
Obj402860 g_catchPanelCharts2[2];

// GLOBAL: EZ2AC_UNPACKED 0x037e01e8
Obj402860 g_catchPanelCharts3[2];

// GLOBAL: EZ2AC_UNPACKED 0x037e0620
Obj402860 g_catchPanelCharts4[2];

// GLOBAL: EZ2AC_UNPACKED 0x037e0a58
Obj402860 g_catchPanelCharts5[2];

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classPanel;        /* 0x4a9120 */
extern EzObjectInfo  g_info_4a9130;     /* 0x4a9130 */
extern KRuntimeClass classGFPanel;      /* 0x4ae6c8 */
extern EzObjectInfo  g_info_4ae6d8;     /* 0x4ae6d8 */
extern KRuntimeClass classCatchPanel;   /* 0x4aebf0 */
extern EzObjectInfo  g_info_4aec00;     /* 0x4aec00 */

// FUNCTION: EZ2AC_UNPACKED 0x00426fa0
KRuntimeClass *Panel::getRuntimeClass()
{
    return &classPanel;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426fb0
DrawGroup *Panel::getObjectInfo()
{
    return &g_info_4a9130;
}

/* @0x00426fc0 (__cdecl): how far through the current period `pos` is, in
   `steps` - `(pos % period) * steps / period` - stepped back by one and
   clamped at nothing. The clamp is the branchless `sets`/`sub`/`and` a
   ternary compiles to, not a branch. */
// FUNCTION: EZ2AC_UNPACKED 0x00426fc0
int periodStep(unsigned int pos, unsigned int steps, unsigned int period)
{
    int t = (int)((pos % period) * steps / period) - 1;

    return t < 0 ? 0 : t;
}

/* The biased fade ramp @0x00426ff0: the same 48ths-of-a-period alpha the
   catch panel's @0x46f370 computes, minus an eighth of the period, and with
   the negative end clamped to nothing. __cdecl - it takes both operands on
   the stack and cleans neither, and it never touches ecx. */
// FUNCTION: EZ2AC_UNPACKED 0x00426ff0
int fadeAlphaBiased(int elapsed, int period)
{
    int t = (int)((48.0f / (float)period) * (float)elapsed) - period / 8;
    int half = period / 2;

    if (t >= half)
        return 255;
    if (t < 0)
        return 0;

    return t * 255 / half;
}

// FUNCTION: EZ2AC_UNPACKED 0x004271a0
Panel::~Panel()
{
    g_panelCharts1[slot->index].clear();
    g_panelCharts4[slot->index].clear();
    g_panelCharts5[slot->index].clear();
    g_panelCharts2[slot->index].clear();
    g_panelCharts6[slot->index].clear();
    g_panelCharts0[slot->index].clear();
    g_panelCharts7[slot->index].clear();
    g_panelCharts3[slot->index].clear();
    if (optional != 0)
    {
        optional->clear();
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00429db0
// Panel::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int Panel::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00490454
struct Panel;

// FUNCTION: EZ2AC_UNPACKED 0x004607a0
KRuntimeClass *GFPanel::getRuntimeClass()
{
    return &classGFPanel;
}

// FUNCTION: EZ2AC_UNPACKED 0x004607b0
DrawGroup *GFPanel::getObjectInfo()
{
    return &g_info_4ae6d8;
}

// FUNCTION: EZ2AC_UNPACKED 0x004608c0
GFPanel::~GFPanel()
{
    g_gfPanelCharts2[slot->index].clear();
    g_gfPanelCharts3[slot->index].clear();
    g_gfPanelCharts4[slot->index].clear();
    g_gfPanelCharts0[slot->index].clear();
    g_gfPanelCharts5[slot->index].clear();
    g_gfPanelCharts1[slot->index].clear();
    g_gfPanelCharts6[slot->index].clear();
}

// FUNCTION: EZ2AC_UNPACKED 0x00462ad0
// GFPanel::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int GFPanel::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049a498
struct GFPanel;

// FUNCTION: EZ2AC_UNPACKED 0x0046f2f0
KRuntimeClass *CatchPanel::getRuntimeClass()
{
    return &classCatchPanel;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046f300
DrawGroup *CatchPanel::getObjectInfo()
{
    return &g_info_4aec00;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046f480
CatchPanel::~CatchPanel()
{
    g_catchPanelCharts2[slot->index].clear();
    g_catchPanelCharts3[slot->index].clear();
    g_catchPanelCharts4[slot->index].clear();
    g_catchPanelCharts0[slot->index].clear();
    g_catchPanelCharts5[slot->index].clear();
    g_catchPanelCharts1[slot->index].clear();
    m_28.clear();
}

// FUNCTION: EZ2AC_UNPACKED 0x00471570
// CatchPanel::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int CatchPanel::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049e174
struct CatchPanel;

/* CatchPanel's constructor: one Obj402860 member, a long run of -1 slots, three
   zeroed blocks and the panel's own texture. */
// FUNCTION: EZ2AC_UNPACKED 0x00471380
CatchPanel::CatchPanel()
{
    int i;

    f2f8 = 0;
    f1b30 = 0;
    f19e0 = 0;
    f19e4 = 0;
    f19e8 = 0;
    f244 = 4;
    f14 = 0.0f;
    f18 = 0.0f;
    f20 = 0.0f;
    f24 = 0.0f;
    f1b28 = 0.0f;
    f1b2c = 0.0f;
    for (i = 0; i < 40; i++)
    {
        f258[i] = -1;
    }
    memset(items, 0, sizeof(items));
    memset(s1b58, 0, sizeof(s1b58));
    memset(f1c64, 0, sizeof(f1c64));
    memset(f1c7c, 0, sizeof(f1c7c));
    // STRING: EZ2AC_UNPACKED 0x0049e18c
    ((KGraphics *)g_sub1b2f054)->loadTexture("p_opaque.bmp", &tex, 0);
}

/* The BPM read-out's own scratch (src/panels.cpp owns the tempo next to it). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eac4
int g_1b2eac4;

/* Panel's constructor: the two charts, a 640x480 screen rectangle, forty -1
   slots and four zeroed blocks. */
// FUNCTION: EZ2AC_UNPACKED 0x00429b70
Panel::Panel()
{
    int i;

    fec[0] = 0.0f;
    fec[1] = 0.0f;
    fec[2] = 0.0f;
    fec[3] = 0.0f;
    fec[4] = 0.0f;
    fec[5] = 0.0f;
    fe8 = 512.0f;
    f130 = 0;
    f10c = 0;
    f12c = 0;
    f1bf8 = 0.0f;
    f1bfc = 0.0f;
    f1c00 = 0;
    f1c04 = 0;
    f104 = 1;
    g_1b2eac4 = 0;
    f11c = 640.0f;
    f30 = 1;
    b118 = 0;
    optional = 0;
    f110 = 0;
    f1c = 0;
    f120 = 480.0f;
    f124 = 0.0f;
    f128 = 0.0f;
    f38 = 4;
    f34 = 0;
    for (i = 0; i < 40; i++)
    {
        f3c[i] = -1;
    }
    memset(items, 0, sizeof(items));
    memset(s1cd8, 0, sizeof(s1cd8));
    memset(f3230, 0, sizeof(f3230));
    memset(f3248, 0, sizeof(f3248));
}


/* One .gds section for the panel: note the three .str resources are read in
   the order 0x1d34, 0x1d3c, 0x1d38 rather than in address order, and the
   eleven-texture run comes between the two float pairs. */
// FUNCTION: EZ2AC_UNPACKED 0x004294a0
int Panel::m4294a0(KParser *p)
{
    PanelSlot *s;

    readGdsInt(&f1d30, p);
    readGdsStr(&f1d34, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1d3c, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1d38, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1d40, &f1d44, p);
    readGdsTexSeries(f1d54, 0xb, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1d48, &f1d4c, p);
    readGdsFloat(&f1d50, p);
    readGdsToken(f1d80, p);
    readGdsInt2(&f1dc0, &f1dc4, p);

    s = slot;
    f1dc8 = &g_panelCharts2[s->index];
    f1dd0 = &g_panelCharts6[s->index];
    f1dcc = &g_panelCharts0[s->index];
    return 1;
}

/* Eleven six-texture runs, each in a 32-pointer slot. */
// FUNCTION: EZ2AC_UNPACKED 0x00429770
int Panel::m429770(KParser *p)
{
    readGdsInt(&f1e08, p);
    readGdsTexSeries(f1e0c[0], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[1], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[2], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[3], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[4], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[5], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[6], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1e0c[7], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f268c, 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f278c, 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f280c, 6, (KGraphics *)g_sub1b2f054, p);
    return 1;
}

/* GFPanel's copy of the same two sections. It has no single float between the
   two float pairs, and its own three chart pairs. */
// FUNCTION: EZ2AC_UNPACKED 0x004622c0
int GFPanel::m4622c0(KParser *p)
{
    PanelSlot *s;

    readGdsInt(&f1b4c, p);
    readGdsStr(&f1b50, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1b58, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1b54, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1b5c, &f1b60, p);
    readGdsTexSeries(f1b6c, 0xb, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1b64, &f1b68, p);
    readGdsToken(f1b98, p);
    readGdsInt2(&f1bd8, &f1bdc, p);

    s = slot;
    f1be0 = &g_gfPanelCharts4[s->index];
    f1be8 = &g_gfPanelCharts0[s->index];
    f1be4 = &g_gfPanelCharts5[s->index];
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00462580
int GFPanel::m462580(KParser *p)
{
    readGdsInt(&f1c0c, p);
    readGdsTexSeries(f1c10[0], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[1], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[2], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[3], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[4], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[5], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[6], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f1c10[7], 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f2490, 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f2590, 6, (KGraphics *)g_sub1b2f054, p);
    readGdsTexSeries(f2610, 6, (KGraphics *)g_sub1b2f054, p);
    return 1;
}

/* And CatchPanel's, identical to GFPanel's down to the chart-pair indices. */
/* CatchPanel's own copy of the quarter-step scale ladder - identical to the
   one at 0x48e6c8 that src/effectordraw.cpp names. */
// GLOBAL: EZ2AC_UNPACKED 0x0049deb8
float g_catchScaleLadder[34];

extern int g_elemCount;   /* 0x1b2e718 (src/loops.cpp) */
extern int g_mixStyle;    /* 0x1b2ebc8 (src/panels.cpp) */
extern int g_speedPercent; /* 0x1b2e890 (src/panels.cpp) */

/* What Panel's cycling marker shifts its y by; GFPanel's does not. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef8c
float g_1b2ef8c;

/* The live player's rung of the same ladder, one per slot. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef20
float g_playerScale[2];

// FUNCTION: EZ2AC_UNPACKED 0x00471060
int CatchPanel::m471060(KParser *p)
{
    PanelSlot *s;

    readGdsInt(&f1bb0, p);
    readGdsStr(&f1bb4, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1bbc, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1bb8, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1bc0, &f1bc4, p);
    readGdsTexSeries(f1bd0, 0xb, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1bc8, &f1bcc, p);
    readGdsToken(f1bfc, p);
    readGdsInt2(&f1c3c, &f1c40, p);

    s = slot;
    f1c44 = &g_catchPanelCharts4[s->index];
    f1c4c = &g_catchPanelCharts0[s->index];
    f1c48 = &g_catchPanelCharts5[s->index];
    return 1;
}

/* Cleared whenever the panel's own .str set is reloaded. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efb0
int g_1b2efb0;

/* Five .str resources - read from the highest member down, which is how the
   original's five calls come out - a float pair, and one chart pair. */
// FUNCTION: EZ2AC_UNPACKED 0x00427f30
int Panel::m427f30(KParser *p)
{
    PanelSlot *s;

    readGdsInt(&f1410, p);
    readGdsFloat2(&f1414, &f1418, p);
    readGdsStr(&f1430, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f142c, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1428, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1424, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1420, (KGraphics *)g_sub1b2f054, p);

    s = slot;
    f1440 = &g_panelCharts1[s->index];
    g_1b2efb0 = 0;
    return 1;
}

/* Six single textures. */
// FUNCTION: EZ2AC_UNPACKED 0x00428000
int Panel::m428000(KParser *p)
{
    readGdsInt(&f1444, p);
    readGdsTexture(&f1448[0], (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&f1448[1], (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&f1448[2], (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&f1448[3], (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&f1448[4], (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&f1448[5], (KGraphics *)g_sub1b2f054, p);
    return 1;
}

/* Which way round the outermost pair of the catch panel's five .str resources
   is read. Both arms are written out in full; cl's tail merger folds the last
   call back into one block, which is what the original has. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebac
int g_1b2ebac;

// FUNCTION: EZ2AC_UNPACKED 0x004700f0
int CatchPanel::m4700f0(KParser *p)
{
    PanelSlot *s;

    readGdsInt(&f1544, p);
    readGdsFloat2(&f1548, &f154c, p);

    if (g_1b2ebac != 0)
    {
        readGdsStr(&f1554, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1560, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f155c, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1558, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1564, (KGraphics *)g_sub1b2f054, p);
    }
    else
    {
        readGdsStr(&f1564, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1560, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f155c, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1558, (KGraphics *)g_sub1b2f054, p);
        readGdsStr(&f1554, (KGraphics *)g_sub1b2f054, p);
    }

    s = slot;
    f1568 = &g_catchPanelCharts2[s->index];
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0042ce40
int Panel::m42ce40(KParser *p)
{
    Obj402860 *c;
    float x, y;

    readGdsInt(&f1c28, p);
    readGdsFloat2(&f1c2c, &f1c30, p);
    readGdsStr(&f1c34, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1c38, &f1c3c, p);
    readGdsFloat2(&f1c40, &f1c44, p);
    readGdsInt(&f1c48, p);
    readGdsInt(&f1c4c, p);
    readGdsTexSeries(f1c50, f1c4c, (KGraphics *)g_sub1b2f054, p);

    f1cd0 = &g_panelCharts5[slot->index];
    f1cd0->attach(f1c34, 1);
    c = f1cd0;
    x = fe4 + f1c2c;
    y = f1c30;
    c->f214 = x;
    c->f218 = y;
    return 1;
}

extern int g_1b2ea30;   /* src/modeselectctor.cpp - how many name variants */

// FUNCTION: EZ2AC_UNPACKED 0x00428450
int Panel::m428450(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&f1588, p);
    readGdsFloat(&f1594, p);
    readGdsFloat2(&f1598, &f159c, p);
    readGdsInt(&f158c, p);
    readGdsInt(&f1590, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(f15a0, f1590, (KGraphics *)g_sub1b2f054, s);

    if (f1590 > 0x20)
    {
        f1590 = 0x20;
    }

    for (i = 0; i < f12c; i++)
    {
        f1620[i] = -1;
        f1668[i] = 0;
    }

    f38 = f1588 != 0 ? 3 : 4;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00428760
int Panel::m428760(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&f16b0, p);
    readGdsFloat(&f16bc, p);
    readGdsFloat2(&f16c0, &f16c4, p);
    readGdsInt(&f16b4, p);
    readGdsInt(&f16b8, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(f16c8, f16b8, (KGraphics *)g_sub1b2f054, s);

    if (f16b8 > 0x20)
    {
        f16b8 = 0x20;
    }

    for (i = 0; i < f12c; i++)
    {
        f1748[i] = 0;
        f1790[i] = 0;
    }

    return 1;
}

/* One .gds item into the panel's own table - Obj46F660::loadItem (src/gdsitem.cpp)
   on the panel's copy of the same record, with two things it does not have:
   the two colours are BRIGHTENED (each channel times three, clamped to 255)
   whenever either of them is set, and the item's rectangle is accumulated into
   the panel's bounding box at +0x11c..+0x128 - which is why the constructor
   seeds those four with 640/480/0/0. The second colour drops its top byte; the
   first keeps it, so the two packs are written out rather than shared. */
// FUNCTION: EZ2AC_UNPACKED 0x00427820
int Panel::loadGdsItem(KParser *p, int index)
{
    char buf[0x1000];
    GdsItem *e = &items[index];
    int n;
    int len;
    char *s;
    unsigned int *pa;
    unsigned int *pb;
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    float w;

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsInt2(&e->f14, &e->f18, p);
    pa = &e->f1c;
    readGdsColor(pa, p);
    pb = &e->f20;
    readGdsColor(pb, p);

    if (e->f1c != 0 || e->f20 != 0)
    {
        a = ((unsigned char *)pa)[3];
        r = (e->f1c & 0xff) * 3;
        g = ((e->f1c >> 8) & 0xff) * 3;
        b = ((e->f1c >> 16) & 0xff) * 3;
        if (b >= 255) b = 255;
        if (g >= 255) g = 255;
        if (r >= 255) r = 255;
        e->f1c = (a << 24) | (b << 16) | (g << 8) | r;

        r = (e->f20 & 0xff) * 3;
        g = ((e->f20 >> 8) & 0xff) * 3;
        b = ((e->f20 >> 16) & 0xff) * 3;
        if (b >= 255) b = 255;
        if (g >= 255) g = 255;
        if (r >= 255) r = 255;
        e->f20 = (b << 16) | (g << 8) | r;
    }

    e->f24 = 0;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f28, p);
    readGdsColor(&e->f2c, p);
    p->parseToken(7);

    e->f30 = 1;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f34, p);
    readGdsColor(&e->f38, p);
    p->parseToken(7);

    readGdsFloat2(&e->f3c, &e->f40, p);
    readGdsTexture(&e->f44, (KGraphics *)g_sub1b2f054, p);
    readGdsColor(&e->f48, p);
    readGdsInt2(&e->f4c, &e->f50, p);
    readGdsColor(&e->f54, p);
    readGdsTexture(&e->f58, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat(&e->f5c, p);
    readGdsFloat(&e->f60, p);
    readGdsFloat(&e->f64, p);

    readGdsToken(buf, p);
    len = strlen(buf);
    buf[len + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(e->f68, 6, (KGraphics *)g_sub1b2f054, s);
    ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)e->f68[0]);
    e->ff0 = e->f0c * 0.5f + e->f04;

    if (f11c > e->f04)
    {
        f11c = e->f04;
    }
    w = e->f0c + e->f04;
    if (w > f124 + f11c)
    {
        f124 = w - f11c;
    }
    if (f120 > e->f08)
    {
        f120 = e->f08;
    }
    w = e->f10 + e->f08;
    if (w > f128 + f120)
    {
        f128 = w - f120;
    }
    return 1;
}

/* GFPanel's copy of Panel::loadGdsItem above - the same function on the GF
   panel, differing only in where the item table and the bounding box live
   (+0xfc and +0xe8..+0xf4 instead of +0x134 and +0x11c..+0x128). */
// FUNCTION: EZ2AC_UNPACKED 0x00460b50
int GFPanel::loadGdsItem(KParser *p, int index)
{
    char buf[0x1000];
    GdsItem *e = &items[index];
    int n;
    int len;
    char *s;
    unsigned int *pa;
    unsigned int *pb;
    unsigned int a;
    unsigned int r;
    unsigned int g;
    unsigned int b;
    float w;

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsInt2(&e->f14, &e->f18, p);
    pa = &e->f1c;
    readGdsColor(pa, p);
    pb = &e->f20;
    readGdsColor(pb, p);

    if (e->f1c != 0 || e->f20 != 0)
    {
        a = ((unsigned char *)pa)[3];
        r = (e->f1c & 0xff) * 3;
        g = ((e->f1c >> 8) & 0xff) * 3;
        b = ((e->f1c >> 16) & 0xff) * 3;
        if (b >= 255) b = 255;
        if (g >= 255) g = 255;
        if (r >= 255) r = 255;
        e->f1c = (a << 24) | (b << 16) | (g << 8) | r;

        r = (e->f20 & 0xff) * 3;
        g = ((e->f20 >> 8) & 0xff) * 3;
        b = ((e->f20 >> 16) & 0xff) * 3;
        if (b >= 255) b = 255;
        if (g >= 255) g = 255;
        if (r >= 255) r = 255;
        e->f20 = (b << 16) | (g << 8) | r;
    }

    e->f24 = 0;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f28, p);
    readGdsColor(&e->f2c, p);
    p->parseToken(7);

    e->f30 = 1;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f34, p);
    readGdsColor(&e->f38, p);
    p->parseToken(7);

    readGdsFloat2(&e->f3c, &e->f40, p);
    readGdsTexture(&e->f44, (KGraphics *)g_sub1b2f054, p);
    readGdsColor(&e->f48, p);
    readGdsInt2(&e->f4c, &e->f50, p);
    readGdsColor(&e->f54, p);
    readGdsTexture(&e->f58, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat(&e->f5c, p);
    readGdsFloat(&e->f60, p);
    readGdsFloat(&e->f64, p);

    readGdsToken(buf, p);
    len = strlen(buf);
    buf[len + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(e->f68, 6, (KGraphics *)g_sub1b2f054, s);
    ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)e->f68[0]);
    e->ff0 = e->f0c * 0.5f + e->f04;

    if (fe8 > e->f04)
    {
        fe8 = e->f04;
    }
    w = e->f0c + e->f04;
    if (w > ff0 + fe8)
    {
        ff0 = w - fe8;
    }
    if (fec > e->f08)
    {
        fec = e->f08;
    }
    w = e->f10 + e->f08;
    if (w > ff4 + fec)
    {
        ff4 = w - fec;
    }
    return 1;
}

/* THE EIGHT TEXTURE-RUN SECTION LOADERS - one straight line of reader calls
   each, over the shared GdsTexSection record (include/gamepanels.h). The only
   thing that differs between them is which section they fill and whether the
   run is ten textures or eleven. */

// FUNCTION: EZ2AC_UNPACKED 0x00428E10
int Panel::m428e10(KParser *p)
{
    readGdsInt(&s19ac.f00, p);
    readGdsFloat2(&s19ac.f04, &s19ac.f08, p);
    readGdsTexSeries(s19ac.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s19ac.f0c, &s19ac.f10, p);
    readGdsFloat(&s19ac.f14, p);
    readGdsToken(s19ac.name, p);
    readGdsInt2(&s19ac.f80, &s19ac.f84, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00428EA0
int Panel::m428ea0(KParser *p)
{
    readGdsInt(&s1a34.f00, p);
    readGdsFloat2(&s1a34.f04, &s1a34.f08, p);
    readGdsTexSeries(s1a34.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1a34.f0c, &s1a34.f10, p);
    readGdsFloat(&s1a34.f14, p);
    readGdsToken(s1a34.name, p);
    readGdsInt2(&s1a34.f80, &s1a34.f84, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00428F30
int Panel::m428f30(KParser *p)
{
    readGdsInt(&s1abc.f00, p);
    readGdsFloat2(&s1abc.f04, &s1abc.f08, p);
    readGdsTexSeries(s1abc.f18, 11, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1abc.f0c, &s1abc.f10, p);
    readGdsFloat(&s1abc.f14, p);
    readGdsToken(s1abc.name, p);
    readGdsInt2(&s1abc.f84, &s1abc.f88, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00428FC0
int Panel::m428fc0(KParser *p)
{
    readGdsInt(&s1b48.f00, p);
    readGdsFloat2(&s1b48.f04, &s1b48.f08, p);
    readGdsTexSeries(s1b48.f18, 11, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1b48.f0c, &s1b48.f10, p);
    readGdsFloat(&s1b48.f14, p);
    readGdsToken(s1b48.name, p);
    readGdsInt2(&s1b48.f84, &s1b48.f88, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00461EE0
int GFPanel::m461ee0(KParser *p)
{
    readGdsInt(&s1880.f00, p);
    readGdsFloat2(&s1880.f04, &s1880.f08, p);
    readGdsTexSeries(s1880.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1880.f0c, &s1880.f10, p);
    readGdsFloat(&s1880.f14, p);
    readGdsToken(s1880.name, p);
    readGdsInt2(&s1880.f80, &s1880.f84, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00461F70
int GFPanel::m461f70(KParser *p)
{
    readGdsInt(&s1908.f00, p);
    readGdsFloat2(&s1908.f04, &s1908.f08, p);
    readGdsTexSeries(s1908.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1908.f0c, &s1908.f10, p);
    readGdsFloat(&s1908.f14, p);
    readGdsToken(s1908.name, p);
    readGdsInt2(&s1908.f80, &s1908.f84, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00470BC0
int CatchPanel::m470bc0(KParser *p)
{
    readGdsInt(&s19f4.f00, p);
    readGdsFloat2(&s19f4.f04, &s19f4.f08, p);
    readGdsTexSeries(s19f4.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s19f4.f0c, &s19f4.f10, p);
    readGdsFloat(&s19f4.f14, p);
    readGdsToken(s19f4.name, p);
    readGdsInt2(&s19f4.f80, &s19f4.f84, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00470CC0
int CatchPanel::m470cc0(KParser *p)
{
    readGdsInt(&s1a7c.f00, p);
    readGdsFloat2(&s1a7c.f04, &s1a7c.f08, p);
    readGdsTexSeries(s1a7c.f18, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1a7c.f0c, &s1a7c.f10, p);
    readGdsFloat(&s1a7c.f14, p);
    readGdsToken(s1a7c.name, p);
    readGdsInt2(&s1a7c.f80, &s1a7c.f84, p);
    return 1;
}

/* The plain rectangle-plus-texture sections (GdsRectTex), one per panel. */

// FUNCTION: EZ2AC_UNPACKED 0x00428A80
int Panel::m428a80(KParser *p)
{
    readGdsInt(&s1824.f00, p);
    readGdsFloat2(&s1824.f04, &s1824.f08, p);
    readGdsFloat2(&s1824.f0c, &s1824.f10, p);
    readGdsTexture(&s1824.f14, (KGraphics *)g_sub1b2f054, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00461C00
int GFPanel::m461c00(KParser *p)
{
    readGdsInt(&s17c4.f00, p);
    readGdsFloat2(&s17c4.f04, &s17c4.f08, p);
    readGdsFloat2(&s17c4.f0c, &s17c4.f10, p);
    readGdsTexture(&s17c4.f14, (KGraphics *)g_sub1b2f054, p);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00470940
int CatchPanel::m470940(KParser *p)
{
    readGdsInt(&s1930.f00, p);
    readGdsFloat2(&s1930.f04, &s1930.f08, p);
    readGdsFloat2(&s1930.f0c, &s1930.f10, p);
    readGdsTexture(&s1930.f14, (KGraphics *)g_sub1b2f054, p);
    return 1;
}


/* And the three-texture variant (GdsRectTex3), two per panel: the last texture
   is loaded straight from the parser's own token text rather than through a
   reader, so it is a KGraphics::loadTexture call and not a readGdsTexture. */

// FUNCTION: EZ2AC_UNPACKED 0x00429050
int Panel::m429050(KParser *p)
{
    readGdsInt(&s1bd4.f00, p);
    readGdsFloat2(&s1bd4.f04, &s1bd4.f08, p);
    readGdsTexture(&s1bd4.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1bd4.f10, &s1bd4.f14, p);
    readGdsTexture(&s1bd4.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1bd4.f1c, 0);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x004290E0
int Panel::m4290e0(KParser *p)
{
    readGdsInt(&s1c08.f00, p);
    readGdsFloat2(&s1c08.f04, &s1c08.f08, p);
    readGdsTexture(&s1c08.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1c08.f10, &s1c08.f14, p);
    readGdsTexture(&s1c08.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1c08.f1c, 0);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00462100
int GFPanel::m462100(KParser *p)
{
    readGdsInt(&s1aa0.f00, p);
    readGdsFloat2(&s1aa0.f04, &s1aa0.f08, p);
    readGdsTexture(&s1aa0.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1aa0.f10, &s1aa0.f14, p);
    readGdsTexture(&s1aa0.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1aa0.f1c, 0);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00462190
int GFPanel::m462190(KParser *p)
{
    readGdsInt(&s1ad0.f00, p);
    readGdsFloat2(&s1ad0.f04, &s1ad0.f08, p);
    readGdsTexture(&s1ad0.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1ad0.f10, &s1ad0.f14, p);
    readGdsTexture(&s1ad0.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1ad0.f1c, 0);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00470DC0
int CatchPanel::m470dc0(KParser *p)
{
    readGdsInt(&s1b04.f00, p);
    readGdsFloat2(&s1b04.f04, &s1b04.f08, p);
    readGdsTexture(&s1b04.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1b04.f10, &s1b04.f14, p);
    readGdsTexture(&s1b04.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1b04.f1c, 0);
    return 1;
}


// FUNCTION: EZ2AC_UNPACKED 0x00470E50
int CatchPanel::m470e50(KParser *p)
{
    readGdsInt(&s1b34.f00, p);
    readGdsFloat2(&s1b34.f04, &s1b34.f08, p);
    readGdsTexture(&s1b34.f0c, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1b34.f10, &s1b34.f14, p);
    readGdsTexture(&s1b34.f18, (KGraphics *)g_sub1b2f054, p);
    ((KGraphics *)g_sub1b2f054)->loadTexture(p->tok, &s1b34.f1c, 0);
    return 1;
}

/* GFPanel's pair of eleven-texture sections without the lone float. */

// FUNCTION: EZ2AC_UNPACKED 0x00462000
int GFPanel::m462000(KParser *p)
{
    readGdsInt(&s1990.f00, p);
    readGdsFloat2(&s1990.f04, &s1990.f08, p);
    readGdsTexSeries(s1990.f14, 11, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1990.f0c, &s1990.f10, p);
    readGdsToken(s1990.name, p);
    readGdsInt2(&s1990.f80, &s1990.f84, p);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00462080
int GFPanel::m462080(KParser *p)
{
    readGdsInt(&s1a18.f00, p);
    readGdsFloat2(&s1a18.f04, &s1a18.f08, p);
    readGdsTexSeries(s1a18.f14, 11, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&s1a18.f0c, &s1a18.f10, p);
    readGdsToken(s1a18.name, p);
    readGdsInt2(&s1a18.f80, &s1a18.f84, p);
    return 1;
}

/* Two rectangles with no cross-panel twin: three textures and two. */

// FUNCTION: EZ2AC_UNPACKED 0x004295c0
int Panel::m4295c0(KParser *p)
{
    readGdsInt(&s3210.f00, p);
    readGdsFloat2(&s3210.f04, &s3210.f08, p);
    readGdsFloat2(&s3210.f0c, &s3210.f10, p);
    readGdsTexture(&s3210.f14, (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&s3210.f18, (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&s3210.f1c, (KGraphics *)g_sub1b2f054, p);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004623d0
int GFPanel::m4623d0(KParser *p)
{
    readGdsInt(&s3014.f00, p);
    readGdsFloat2(&s3014.f04, &s3014.f08, p);
    readGdsFloat2(&s3014.f0c, &s3014.f10, p);
    readGdsTexture(&s3014.f14, (KGraphics *)g_sub1b2f054, p);
    readGdsTexture(&s3014.f18, (KGraphics *)g_sub1b2f054, p);
    return 1;
}

/* CatchPanel's ComboEffect section - m471200 @0x471200. Not one of the shared
   record shapes: three .str resources of its own, then the chart of the pair
   the panel's slot picks, and finally a global written over the int the
   loader read first (the same overwrite GFMainGameDirector's General arm
   does). g_catchPanelCharts1's stride is what proves Obj402860 is 540 bytes:
   the original builds the element address with 135 * 4 * index. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e920
int g_1b2e920;

// FUNCTION: EZ2AC_UNPACKED 0x00471200
int CatchPanel::m471200(KParser *p)
{
    readGdsInt(&f1c50, p);
    readGdsStr(&f1c54, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1c58, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1c5c, (KGraphics *)g_sub1b2f054, p);
    f1c60 = &g_catchPanelCharts1[slot->index];
    f1c50 = g_1b2e920;
    return 1;
}


/* CatchPanel's GrooveLight section - m470ee0 @0x470ee0. The only APPENDING
   loader in the panel: it writes the element the running count at f1b54
   selects and then bumps that count, so a .gds may carry more than one
   GrooveLight section. LoadPVI @0x472650 is what zeroes the count. */
// FUNCTION: EZ2AC_UNPACKED 0x00470EE0
int CatchPanel::m470ee0(KParser *p)
{
    GdsGrooveLight *e = &s1b58[f1b54];

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsTexture(&e->f14, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&e->f18, &e->f1c, p);
    readGdsFloat2(&e->f20, &e->f24, p);
    readGdsTexture(&e->f28, (KGraphics *)g_sub1b2f054, p);
    f1b54++;
    return 1;
}


/* CatchPanel's CoolCombo section - m471750 @0x471750. The digits' texture run
   plus four .str resources, and then the float pair is copied straight into
   the chart the slot selects, which is the only loader that writes through
   one of the global charts rather than just recording its address. */
// FUNCTION: EZ2AC_UNPACKED 0x00471750
int CatchPanel::m471750(KParser *p)
{
    Obj402860 *c;
    float      x;
    float      y;

    readGdsInt(&f18e4, p);
    readGdsTexSeries(f18e8, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1920, &f1924, p);
    readGdsStr(&f1910, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1914, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1918, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f191c, (KGraphics *)g_sub1b2f054, p);

    x = f1920;
    y = f1924;
    c = &g_catchPanelCharts3[slot->index];
    f192c = c;
    c->f214 = x;
    c->f218 = y;
    return 1;
}


/* CatchPanel's three bomb sections - m470210 / m4704f0 / m470650 @0x470210,
   0x4704f0, 0x470650. Byte-for-byte the same loader three times over on three
   adjacent 0x128-byte records, and the same one Panel has flat at 0x1588. */

// FUNCTION: EZ2AC_UNPACKED 0x00470210
int CatchPanel::m470210(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s156c.f00, p);
    readGdsFloat(&s156c.f0c, p);
    readGdsFloat2(&s156c.f10, &s156c.f14, p);
    readGdsInt(&s156c.f04, p);
    readGdsInt(&s156c.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s156c.f18, s156c.f08, (KGraphics *)g_sub1b2f054, s);

    if (s156c.f08 > 0x20)
    {
        s156c.f08 = 0x20;
    }

    for (i = 0; i < f2f8; i++)
    {
        s156c.f98[i] = -1;
        s156c.fe0[i] = 0;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004704f0
int CatchPanel::m4704f0(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s1694.f00, p);
    readGdsFloat(&s1694.f0c, p);
    readGdsFloat2(&s1694.f10, &s1694.f14, p);
    readGdsInt(&s1694.f04, p);
    readGdsInt(&s1694.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s1694.f18, s1694.f08, (KGraphics *)g_sub1b2f054, s);

    if (s1694.f08 > 0x20)
    {
        s1694.f08 = 0x20;
    }

    for (i = 0; i < f2f8; i++)
    {
        s1694.f98[i] = -1;
        s1694.fe0[i] = 0;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00470650
int CatchPanel::m470650(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s17bc.f00, p);
    readGdsFloat(&s17bc.f0c, p);
    readGdsFloat2(&s17bc.f10, &s17bc.f14, p);
    readGdsInt(&s17bc.f04, p);
    readGdsInt(&s17bc.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s17bc.f18, s17bc.f08, (KGraphics *)g_sub1b2f054, s);

    if (s17bc.f08 > 0x20)
    {
        s17bc.f08 = 0x20;
    }

    for (i = 0; i < f2f8; i++)
    {
        s17bc.f98[i] = 0;
        s17bc.fe0[i] = 0;
    }

    return 1;
}


/* The GroupTexture section - m460ac0 @0x460ac0. Modelled on GFPanel because
   that is whose address range it sits in, but BOTH GFPanel::LoadPVI and
   CatchPanel::LoadPVI call it - which is possible precisely because it never
   touches `this`. A count, then that
   many .txg atlas names, each loaded as it is read; the three parseToken
   calls around the run step the parser over the braces. It never touches
   `this` - the caller sets ecx and the callee ignores it - and cl parks the
   count in the parameter's own home slot rather than growing the frame. */
// FUNCTION: EZ2AC_UNPACKED 0x00460AC0
int GFPanel::m460ac0(KParser *p)
{
    char buf[0x40];
    int  n;
    int  i;

    readGdsInt(&n, p);

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);

    for (i = 0; i < n; i++)
    {
        readGdsToken(buf, p);
        ((KGraphics *)g_sub1b2f054)->loadTXG(buf);
    }

    p->parseToken(7);
    return 1;
}


/* CatchPanel's TargetBar section - m4709a0 @0x4709a0. The .gds carries one
   geometry block per name variant; the loader reads and DISCARDS the first
   g_1b2ea30 of them into the same four fields and keeps the next, which is the
   same variant-selection the bomb loaders do with semicolons. A width or
   height left at zero is then filled in from the first texture of the run. */
// FUNCTION: EZ2AC_UNPACKED 0x004709A0
int CatchPanel::m4709a0(KParser *p)
{
    char buf[0x1000];
    int  i;

    readGdsInt(&f1948, p);

    for (i = 0; i < g_1b2ea30; i++)
    {
        readGdsFloat2(&f194c, &f1950, p);
        readGdsFloat2(&f1954, &f1958, p);
        readGdsToken(buf, p);
    }

    readGdsFloat2(&f194c, &f1950, p);
    readGdsFloat2(&f1954, &f1958, p);
    readGdsTexSeries(f195c, 6, (KGraphics *)g_sub1b2f054, p);

    if (f1954 == 0.0f)
    {
        f1954 = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)f195c[0]);
    }

    if (f1958 == 0.0f)
    {
        f1958 = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f195c[0]);
    }

    f19ec = f1950;
    f19dc = f1950 + f1958;
    return 1;
}


/* GFPanel's GrooveLight section - m462220 @0x462220. CatchPanel::m470ee0's
   twin down to the record and the array length: it writes the element the
   running count at f1af0 selects and bumps it, so a .gds may carry several. */
// FUNCTION: EZ2AC_UNPACKED 0x00462220
int GFPanel::m462220(KParser *p)
{
    GdsGrooveLight *e = &s1af4[f1af0];

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsTexture(&e->f14, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&e->f18, &e->f1c, p);
    readGdsFloat2(&e->f20, &e->f24, p);
    readGdsTexture(&e->f28, (KGraphics *)g_sub1b2f054, p);
    f1af0++;
    return 1;
}


/* GFPanel's MeasureLine section - m460f00 @0x460f00. A six-texture run with
   the same zero-means-take-the-texture's rule TargetBar uses, and then a clamp
   on s1400.f08 - the CoolBomb run count, which is why the clamp reads as
   belonging to no section of this loader's own. */
// FUNCTION: EZ2AC_UNPACKED 0x00460F00
int GFPanel::m460f00(KParser *p)
{
    readGdsInt(&f1344, p);
    readGdsFloat(&f134c, p);
    readGdsFloat2(&f1350, &f1354, p);
    readGdsInt(&f1348, p);
    readGdsTexSeries(f1358, 6, (KGraphics *)g_sub1b2f054, p);

    if (f1354 == 0.0f)
    {
        f1354 = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f1358[0]);
    }

    if (s1400.f08 > 0x20)
    {
        s1400.f08 = 0x20;
    }

    return 1;
}


extern int g_1b2e584;   /* src/modeselectctor.cpp */

/* GFPanel's TargetBar section - m461c60 @0x461c60. CatchPanel::m4709a0's twin
   - one geometry block per name variant, all but the last discarded, and a
   zero width or height filled in from the first texture of the run - plus
   three derived fields and a flag raised when either cabinet global is set. */
// FUNCTION: EZ2AC_UNPACKED 0x00461C60
int GFPanel::m461c60(KParser *p)
{
    char buf[0x1000];
    int  i;

    readGdsInt(&f17dc, p);

    for (i = 0; i < g_1b2ea30; i++)
    {
        readGdsFloat2(&f17e0, &f17e4, p);
        readGdsFloat2(&f17e8, &f17ec, p);
        readGdsToken(buf, p);
    }

    readGdsFloat2(&f17e0, &f17e4, p);
    readGdsFloat2(&f17e8, &f17ec, p);
    readGdsTexSeries(f17f0, 6, (KGraphics *)g_sub1b2f054, p);

    f1874 = f17e4;
    f187c = 0.0f;
    f1878 = 0.0f;

    if (f17e8 == 0.0f)
    {
        f17e8 = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)f17f0[0]);
    }

    if (f17ec == 0.0f)
    {
        f17ec = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f17f0[0]);
    }

    f1870 = f17e4 + f17ec;

    if (g_1b2e584 != 0 || g_1b2ea30 != 0)
    {
        f17dc = 1;
    }

    return 1;
}


/* GFPanel's three bomb sections - m461060 / m461360 / m461670. The same loader
   the other two panels have, on three adjacent 0x128 records. GoodBomb alone
   republishes its id into f34; LongNoteBomb alone zeroes both state arrays
   where its twins set the first to -1. */

// FUNCTION: EZ2AC_UNPACKED 0x00461060
int GFPanel::m461060(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s1400.f00, p);
    readGdsFloat(&s1400.f0c, p);
    readGdsFloat2(&s1400.f10, &s1400.f14, p);
    readGdsInt(&s1400.f04, p);
    readGdsInt(&s1400.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s1400.f18, s1400.f08, (KGraphics *)g_sub1b2f054, s);

    if (s1400.f08 > 0x20)
    {
        s1400.f08 = 0x20;
    }

    for (i = 0; i < ff8; i++)
    {
        s1400.f98[i] = -1;
        s1400.fe0[i] = 0;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00461360
int GFPanel::m461360(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s1528.f00, p);
    readGdsFloat(&s1528.f0c, p);
    readGdsFloat2(&s1528.f10, &s1528.f14, p);
    readGdsInt(&s1528.f04, p);
    readGdsInt(&s1528.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s1528.f18, s1528.f08, (KGraphics *)g_sub1b2f054, s);

    if (s1528.f08 > 0x20)
    {
        s1528.f08 = 0x20;
    }

    for (i = 0; i < ff8; i++)
    {
        s1528.f98[i] = -1;
        s1528.fe0[i] = 0;
    }

    f34 = s1528.f00 != 0 ? 3 : 4;

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00461670
int GFPanel::m461670(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s1650.f00, p);
    readGdsFloat(&s1650.f0c, p);
    readGdsFloat2(&s1650.f10, &s1650.f14, p);
    readGdsInt(&s1650.f04, p);
    readGdsInt(&s1650.f08, p);

    readGdsToken(buf, p);
    buf[strlen(buf) + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(s1650.f18, s1650.f08, (KGraphics *)g_sub1b2f054, s);

    if (s1650.f08 > 0x20)
    {
        s1650.f08 = 0x20;
    }

    for (i = 0; i < ff8; i++)
    {
        s1650.f98[i] = 0;
        s1650.fe0[i] = 0;
    }

    return 1;
}


/* GFPanel's Judgment section - m460fa0 @0x460fa0. Five .str resources read in
   DESCENDING address order, exactly as Panel::m427f30 reads its five, then the
   chart of the pair the slot picks. */
// FUNCTION: EZ2AC_UNPACKED 0x00460FA0
int GFPanel::m460fa0(KParser *p)
{
    readGdsInt(&f13d8, p);
    readGdsFloat2(&f13dc, &f13e0, p);
    readGdsStr(&f13f8, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f13f4, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f13f0, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f13ec, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f13e8, (KGraphics *)g_sub1b2f054, p);
    f13fc = &g_gfPanelCharts2[slot->index];
    return 1;
}

/* GFPanel's RubyKeyPanel section - m4624f0 @0x4624f0. */
// FUNCTION: EZ2AC_UNPACKED 0x004624F0
int GFPanel::m4624f0(KParser *p)
{
    readGdsInt(&f1bec, p);
    readGdsFloat2(&f1bf0, &f1bf4, p);
    readGdsFloat2(&f1bf8, &f1bfc, p);
    readGdsTexture(&f1c00, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1c04, (KGraphics *)g_sub1b2f054, p);
    f1c08 = &g_gfPanelCharts6[slot->index];
    return 1;
}

/* GFPanel's SpecialNote section - m4626a0 @0x4626a0. Eleven instructions: one
   six-texture run and nothing else, the smallest loader in the family. */
// FUNCTION: EZ2AC_UNPACKED 0x004626A0
int GFPanel::m4626a0(KParser *p)
{
    readGdsTexSeries(f2690, 6, (KGraphics *)g_sub1b2f054, p);
    return 1;
}


/* GFPanel's CoolCombo section - m463ff0 @0x463ff0. CatchPanel::m471750's twin
   on the same 0x44 record, with one difference in the copy: where CatchPanel
   passes its float pair through unchanged, this one offsets the first by the
   panel's own fe0 before storing it. */
// FUNCTION: EZ2AC_UNPACKED 0x00463FF0
int GFPanel::m463ff0(KParser *p)
{
    Obj402860 *c;
    float      x;
    float      y;

    readGdsInt(&f1778, p);
    readGdsTexSeries(f177c, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f17b4, &f17b8, p);
    readGdsStr(&f17a4, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f17a8, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f17ac, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f17b0, (KGraphics *)g_sub1b2f054, p);

    x = fe0 + f17b4;
    y = f17b8;
    c = &g_gfPanelCharts3[slot->index];
    f17c0 = c;
    c->f214 = x;
    c->f218 = y;
    return 1;
}

/* CatchPanel::m471170 @0x471170 - the readout the section above loads.

   Everything about it comes out of the .gds record: the rectangle, the format
   string the panel keeps inline at +0x1bfc, the texture run at +0x1bd0 and the
   blend pair at +0x1c3c. The step is a third of the width, and the scale is
   whichever rung of the quarter-step ladder the SLOT asks for - which is what
   named PanelSlot::f474 and, with it, corrected drawNumF's sixth argument from
   a font pointer to a float (include/gfxtext.h). */
// FUNCTION: EZ2AC_UNPACKED 0x00471170
void CatchPanel::m471170()
{
    if (f1bb0 == 0)
    {
        return;
    }

    ((KGraphics *)g_sub1b2f054)->setBlend(f1c3c, f1c40);
    ((GfxText *)g_sub1b2f054)->drawNumF(f1bc0, f1bc4, f1bc8, f1bcc,
                                        f1bc8 * (1.0f / 3.0f),
                                        g_catchScaleLadder[slot->f474],
                                        f1bfc, -1, f1bd0);
}

/* GFPanel's three readouts - and the float-to-int idiom the whole family uses.

   The rounding helper is INLINE ASSEMBLY, not a cast: `(int)f` compiles to a
   call into the CRT's _ftol2 (C requires truncation), while the original does
   a bare `fistp`, which follows the current rounding mode. The giveaway is
   that BOTH arguments are spilled to the frame and read straight back -
   `lea eax,[esp+4]` / `movss [esp+0xc]` / `mov [esp+8],eax` / `fld [esp+0xc]` /
   `mov edx,[esp+8]` / `fistp [edx]` - which is exactly what cl must do around
   an __asm block, since inline assembly cannot see values held in registers.
   This is the period's well-known FastFloatToInt. */
__forceinline void FloatToInt(int *i, float f)
{
    __asm fld   f
    __asm mov   edx, i
    __asm fistp dword ptr [edx]
}

/* Walk the panel's items and draw each armed one that is fading in: set its
   blend pair on the renderer, round its scrolled x with the fast converter,
   and blit its texture at the item's own y. */
// FUNCTION: EZ2AC_UNPACKED 0x0042a580
void Panel::m42a580()
{
    int i = 0;

    if (f12c > 0)
    {
        char *q = (char *)&items[0].f4c;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x4c);

            if (e->f00 != 0)
            {
                ((KGraphics *)g_sub1b2f054)->setBlend(e->f4c, e->f50);
                if (e->ff8 == 1)
                {
                    int n;

                    FloatToInt(&n, e->f3c + fe4);
                    ((KGraphics *)g_sub1b2f054)->drawTex(e->f44, (float)n,
                                                         e->f40, 0, 0,
                                                         (int)e->f48);
                }
            }
            i++;
            q += 0x104;
        }
        while (i < f12c);
    }
}

/* Drive every armed item's fade and draw it: state 1 raises the level by
   f60 toward the f5c ceiling, state 2 lowers it by f64 and disarms at
   the 1e-4 floor and disarms, anything else skips the item; the survivor
   is drawn 2/2-blended at its slid x and its level-lifted, rounded y. */
/* Walk one item's note run: open its lane at `start`, and for every hit
   before the run's window closes draw it - holds through m42a630, taps
   through m42af30 - carrying the item's own baseline t. While either
   reskin gate is up, a hit whose offset names a loaded override texture
   swaps the voice; skin -1 or a missing override falls back to the
   column's (or first) skin. The window is the time of item*4 notes at
   48-ticks-per-measure of the item's rate, truncated the double-x87 way.

   95.73%, and its GFPanel twin m463b40 @0x463b40 the same to the hundredth.
   Every instruction is present and the residue is FIVE lines: the inline
   ftol's scratch slot (the original's control word and fistp target live at
   [esp+0x14], ours at [esp+0x50]) and, following from it, the restoring
   `fldcw` scheduled two instructions early. That is the stack packer, and it
   is not reachable from the conversion's source shape - SIX forms all give
   the identical listing: the conversion inlined into its use with no named
   `n`, `n` hoisted to function scope, `n` declared before `voice`, the
   quotient `q` lifted out of its inner block, an intermediate `float u`
   between the divide and the cast, and a distinct local instead of reusing
   the `item` parameter (the last on the theory that our temp had landed in
   the reassigned parameter's home - it had not). Do not re-sweep this one.
   */
// FUNCTION: EZ2AC_UNPACKED 0x0042b2f0
void Panel::m42b2f0(int item, int start, int col)
{
    SlotSound snd;
    SlotState st;
    GdsItem *e = (GdsItem *)((char *)this + item * 0x104 + 0x134);

    if (e->f00 != 0)
    {
        float t = f1848 - e->f08 + f1850;
        int n;

        item = f2c;
        {
            double q = 48.0f / item;

            n = (int)(float)((t + e->f08 - e->f08) / (q * e->fe8));
        }
        item = n + item * 4 + f28;
        /* voice is looked up AFTER the item index, not with t */
        KTexture *voice = e->f68[col];
        e->ffc = 0;
        {
            int r = slot->f3d4->begin(e->fec, start, 0, 1, &st, &snd);

            while (r != 0)
            {
                if ((unsigned int)st.start >= (unsigned int)item)
                    break;
                if (slot->f4f4 != 0 || slot->f51c != 0)
                {
                    int v = st.skin;

                    if (v == -1)
                        voice = e->f68[col];
                    else
                    {
                        KTexture *o = f1e0c[v][col];

                        if (o == 0)
                            voice = e->f68[0];
                        else
                            voice = o;
                    }
                }
                if (st.width > 6)
                    r = m42a630(e, &st, voice, t);
                else
                    r = m42af30(e, &st, voice, t);
                if (r == 2)
                    break;
                r = slot->f3d4->next(1, &st, &snd);
            }
        }
    }
}

/* The scratch panels' slot pointer lands on the stage object 0x466700
   initializes, not on PanelSlot - its sink and the two reskin gates sit at
   its own offsets. A view, since the classes only share the pointer slot. */
struct GFLifeView
{
    char _pad0[0x330];
    char b330;                     /* 0x330 - up when the bpm comes from the clock */
    char _pad331[0x76c - 0x331];
    int  state;                    /* 0x76c - 2 alive, 3 topped out */
};

struct GFStageView
{
    char _pad0[0x38];
    float f38;                     /* 0x38 - the second gauge's level */
    char _pad3c[0x1e0 - 0x3c];
    struct GFLifeView *life;       /* 0x1e0 */
    struct SlotSampleSink *sink;   /* 0x1e4 */
    char _pad1e8[0x304 - 0x1e8];
    int  gateA;                    /* 0x304 */
    char _pad308[0x32c - 0x308];
    int  gateB;                    /* 0x32c */
};

// FUNCTION: EZ2AC_UNPACKED 0x00463b40
void GFPanel::m463b40(int item, int start, int col)
{
    SlotSound snd;
    SlotState st;
    GdsItem *e = (GdsItem *)((char *)this + item * 0x104 + 0xfc);

    if (e->f00 != 0)
    {
        float t = f17e4 - e->f08 + f17ec;
        int n;

        item = f28;
        {
            double q = 48.0f / item;

            n = (int)(float)((t + e->f08 - e->f08) / (q * e->fe8));
        }
        item = n + item * 4 + f24;
        /* voice is looked up AFTER the item index, not with t */
        KTexture *voice = e->f68[col];
        e->ffc = 0;
        {
            int r = ((GFStageView *)slot)->sink->begin(e->fec, start, 0, 1, &st, &snd);

            while (r != 0)
            {
                if ((unsigned int)st.start >= (unsigned int)item)
                    break;
                if (((GFStageView *)slot)->gateA != 0 || ((GFStageView *)slot)->gateB != 0)
                {
                    int v = st.skin;

                    if (v == -1)
                        voice = e->f68[col];
                    else
                    {
                        KTexture *o = f1c10[v][col];

                        if (o == 0)
                            voice = e->f68[0];
                        else
                            voice = o;
                    }
                }
                if (st.width > 6)
                    r = m463210(e, &st, voice, t);
                else
                    r = m4637f0(e, &st, voice, t);
                if (r == 2)
                    break;
                r = ((GFStageView *)slot)->sink->next(1, &st, &snd);
            }
        }
    }
}

/* Draw the measure lines: 2/2-blended, one f1390 texture per line from the
   run's current measure to the window's edge, each placed a line-spacing
   further up the scroll; the texture index follows the beat phase, clamped
   to the first line. The window math is the note-run walkers' double-x87
   truncation over the beat lane (item 1); the per-line position is plain
   SSE float - the (48.0f / fc) parenthesization with fc named is what
   assigns the original's registers.

   95.37%: the 48.0 pool width again, plus the cnt-spill / zero-seed /
   fmul scheduling positions around the divides - the twins' residue
   family. */
// FUNCTION: EZ2AC_UNPACKED 0x00427d90
void Panel::m427d90()
{
    unsigned int cnt;
    SlotSound snd;
    SlotState st;
    int idx;
    int lim;
    int n;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    {
        unsigned int pos;
        /* t is built through DOUBLE locals, and the 48/cnt step below is an
           inline `((double)(...))` with the outer parentheses: together
           they fix the last pair, `cnt = f2c`'s load scheduled ahead of the
           fsub (99.07% with either one alone, 96.30% before). Found by
           scratchpad/perm's name_subexpr. */
        double d0 = f1848 - g_1b2ef8c;
        double d1 = items[1].f08;
        double d2 = d0 - d1 + f1850;
        float t = d2;

        cnt = f2c;
        pos = f28;
        idx = (int)(pos % cnt * f1380 / cnt) - 1;
        idx = idx < 0 ? 0 : idx;
        {
            /* `48.0f / (int)cnt` divided as a FLOAT and only then widened -
               `48.0f / (double)(int)cnt` promotes the constant and cl emits
               `fdivr qword` against the original's `fdivr dword`. */
            n = (int)(float)((t + items[1].f08 - items[1].f08) /
                             (((double)(48.0f / (int)cnt)) * items[1].fe8));
        }
        lim = n + pos + cnt * 4;
        {
            int r = slot->f3d4->begin(0, 0, 0, 6, &st, &snd);

            while (r != 0)
            {
                if ((unsigned int)st.start >= (unsigned int)lim)
                    break;
                {
                    int x = st.start - f28;
                    float fc = (float)(int)f2c;

                    ((KGraphics *)g_sub1b2f054)->drawTex(
                        f1390[idx], f1384 + fe4,
                        t - (48.0f / fc) * (float)x * items[1].fe8 +
                            items[1].f08 - f138c * 0.5f + g_1b2ef8c,
                        f1388, f138c, -1);
                }
                r = slot->f3d4->next(6, &st, &snd);
            }
        }
    }
}

/* Draw the lane frame and the guide lines: reset the raster stages, and
   when item 0 asks (either gate up) blend the screen rect in its pair;
   then 5/6-blended, each item contributes up to two guide lines - one per
   armed side - placed off its base by the side's own inset (or the shared
   fallback when the side flag is down), each a m40ac80 quad between the
   rounded x and its span. */
/* Panel::m42a240 on the GF panel's offsets: the frame x gains six when
   the slot is the second side, and the rect narrows by four. */
// FUNCTION: EZ2AC_UNPACKED 0x00462e00
void GFPanel::m462e00()
{
    int na;
    int i;

    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    if (items[0].f1c != 0 || items[0].f20 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(items[0].f14, items[0].f18);
        FloatToInt(&na, fe8 + fe0);
        if (slot->index == 1)
            na += 6;
        ((KGraphics *)g_sub1b2f054)->m40a5a0((float)na, fec, ff0 - 4.0f,
                                             ff4, 0x96000000, 0xff000000);
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    i = 0;
    if (ff8 > 0)
    {
        char *q = (char *)&items[0].f28;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x28);

            if (e->f28 != 0.0f)
            {
                float d;

                if (e->f24 == 0)
                    d = e->f04 - e->f28;
                else
                    d = e->f04 + e->f0c;
                FloatToInt(&na, fe0 + d);
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    (float)na, e->f08, fe0 + d, e->f10 + e->f08,
                    e->f28, *(float *)&e->f2c);
            }
            if (e->f34 != 0.0f)
            {
                float d;
                int nb;

                if (e->f30 == 0)
                    d = e->f04 - e->f34;
                else
                    d = e->f04 + e->f0c;
                FloatToInt(&nb, fe0 + d);
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    (float)nb, e->f08, fe0 + d, e->f10 + e->f08,
                    e->f34, *(float *)&e->f38);
            }
            i++;
            q += 0x104;
        }
        while (i < ff8);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0042a240
void Panel::m42a240()
{
    int na;
    int i;

    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    if (items[0].f1c != 0 || items[0].f20 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(items[0].f14, items[0].f18);
        FloatToInt(&na, f11c + fe4);
        ((KGraphics *)g_sub1b2f054)->m40a5a0((float)na, f120, f124, f128,
                                             0x96000000, 0xff000000);
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    i = 0;
    if (f12c > 0)
    {
        char *q = (char *)&items[0].f28;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x28);

            if (e->f28 != 0.0f)
            {
                float d;

                if (e->f24 == 0)
                    d = e->f04 - e->f28;
                else
                    d = e->f04 + e->f0c;
                FloatToInt(&na, fe4 + d);
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    (float)na, e->f08, fe4 + d, e->f10 + e->f08,
                    e->f28, *(float *)&e->f2c);
            }
            if (e->f34 != 0.0f)
            {
                float d;
                int nb;

                if (e->f30 == 0)
                    d = e->f04 - e->f34;
                else
                    d = e->f04 + e->f0c;
                FloatToInt(&nb, fe4 + d);
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    (float)nb, e->f08, fe4 + d, e->f10 + e->f08,
                    e->f34, *(float *)&e->f38);
            }
            i++;
            q += 0x104;
        }
        while (i < f12c);
    }
}

extern int g_d36cb0;   /* 0x00d36cb0 (src/maingame.cpp) */
extern int g_eventMode; /* 0x01696404 (src/settings.cpp) */
extern int g_useAudiencePanel;  /* 0x01b2eb48 (src/maingame.cpp) */
extern int g_1b2e7d4;           /* (src/maingame.cpp) */
extern float g_rateTarget;      /* 0x01b2ebd8 (src/panels.cpp) */
extern float g_rate1p;          /* 0x01b2e860 (src/panels.cpp) */
extern float g_rate2p;          /* 0x01b2e864 (src/panels.cpp) */
extern int g_1b2eb6c;           /* (src/ez2acapp.cpp) */
extern int g_1b2eba4;           /* (src/maingame.cpp) */
extern int g_1b2efb4;           /* (src/moderunner.cpp) */
extern struct ListNode *findListA(int key);  /* 0x418e30 (src/nodelists.cpp) */
extern int countListA();        /* 0x418310 (src/nodelists.cpp) */

/* Whether the audience rate is currently failing - the second gauge's
   danger lamp. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f184
int g_1b5f184;

/* The rate field of a list-A player, read by the danger tree. */
struct RateNode
{
    char  _pad0[0x1f4];
    float rate;                /* 0x1f4 */
};
extern int g_1b2eb6c;   /* the mode's ladder flag */
extern int g_1b2eba4;   /* 0x01b2eba4 (src/maingame.cpp) */
extern int g_1b2efb4;   /* 0x01b2efb4 (src/moderunner.cpp) */
extern int countListC();  /* 0x4184a0 (src/nodelists.cpp) */

/* Draw the gauge. The plate blits 2/1-blended at its rounded x; the clip
   chart (when armed) is re-based to the scroll and pumped; the level is
   chased by a fifth every frame with a four-point jolt on each beat and
   each measure boundary, clamped to the bar's height, and drawn as a
   partial sprite span - one span while alive, and once the level tops
   100, the same span twice in random colors.

   77.10%, the slot/schedule family: the original packs fr at 0x1c with h2
   at 0x20 (ours inverts, shifting every FloatToInt home), reloads the
   slot pointer in the heightOf gap where ours loads the run position, and
   keeps a handful of xmm renames in the span arms. Declaration order and
   inlining the level read do not steer the packer. Semantics complete. */
// FUNCTION: EZ2AC_UNPACKED 0x0042c0f0
void Panel::m42c0f0()
{
    int nd;
    int nh;
    int nx;
    int n0;
    float h;
    float disp;
    float fnd;
    float h2;
    float fr;
    unsigned int q;

    if (s1bd4.f00 != 0)
    {
        FloatToInt(&n0, s1bd4.f04 + fe4);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(s1bd4.f0c, (float)n0,
                                             s1bd4.f08, 0, 0, -1);
        if (f1d30 != 0)
        {
            Obj402860 *o;
            int nc;

            FloatToInt(&nc, fe4);
            o = f1dd0;
            o->f214 = (float)nc;
            o->f218 = 0.0f;
            f1dd0->m403940();
            if (f1dd0->isDone() != 0)
                f1dd0->attach(0, 0);
        }
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1bd4.f18);
        h2 = h + s1c08.f14;
        {
            f1bf8 = (slot->f1f4 - f1bf8) * 0.2f + f1bf8;
            q = f28 / f2c;
            disp = (f1bf8 * 0.01f) * h;
            if (f1c00 != (int)q)
            {
                f1c00 = q;
                f1bf8 = f1bf8 + 4.0f;
            }
            q = f28 / (unsigned int)f130;
            if (f1c04 != (int)q)
            {
                f1c04 = q;
                f1bf8 = f1bf8 + 4.0f;
            }
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&nx, s1bd4.f10 + fe4);
        FloatToInt(&nh, h2 - (float)nd);
        if (0.0f >= slot->f1f4)
            b118 = 1;
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        if (g_d36cb0 == 0)
        {
            if (slot->f1f4 > 0.0f && b118 == 0)
            {
                fnd = (float)nd;
                fr = 1.0f / h;
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1bd4.f18, (float)nx, (float)nh, 0,
                    fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, -1);
            }
        }
        else if (slot->f1f4 > 0.0f && b118 == 0)
        {
            fnd = (float)nd;
            ((KGraphics *)g_sub1b2f054)->m404660(
                (KTexture *)s1bd4.f18, (float)nx, (float)nh, 0,
                fnd, 0, 0, 1.0f, fnd / h2, -1);
        }
        if (slot->f1f4 >= 100.0f)
        {
            unsigned int c;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            fnd = (float)nd;
            fr = 1.0f / h;
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->m404660(
                (KTexture *)s1bd4.f18, (float)nx, (float)nh, 0,
                fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, c);
            fnd = (float)nd;
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->m404660(
                (KTexture *)s1bd4.f18, (float)nx, (float)nh, 0,
                fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, c);
        }
    }
}

/* The GF gauge: Panel::m42c0f0 with the scratch panel's own level copy, a
   single beat latch with an eight-point jolt, the ready/danger state (3
   topped out, 2 alive) pushed into the stage owner's record unless event
   mode is on - topping out also pins the bar full - and the over-100
   flash drawn as the named texture twice in random colors.

   65.66%: semantically complete; the beat latch is compared but NEVER
   stored here (the store was tried and is wrong), and with the true
   semantics cl re-rolls `this` from esi into ebx and every operand
   renames - the score is that cascade plus the usual FloatToInt home
   inversions, not wrong structure. Seed family; re-score later. */
// FUNCTION: EZ2AC_UNPACKED 0x00464580
void GFPanel::m464580()
{
    int nh;
    int nx;
    int nd;
    int n0;
    float h;
    float disp;
    float fnd;
    float h2;
    float fr;
    unsigned int q;

    if (s1aa0.f00 != 0)
    {
        FloatToInt(&n0, s1aa0.f04 + fe0);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(s1aa0.f0c, (float)n0,
                                             s1aa0.f08, 0, 0, -1);
        if (f1b4c != 0)
        {
            Obj402860 *o;
            int nc;

            FloatToInt(&nc, fe0);
            o = f1be8;
            o->f214 = (float)nc;
            o->f218 = 0.0f;
            f1be8->m403940();
            if (f1be8->isDone() != 0)
                f1be8->attach(0, 0);
        }
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1aa0.f18);
        q = f24 / f28;
        f1ac8 = (f1ac0 - f1ac8) * 0.2f + f1ac8;
        disp = (f1ac8 * h) * 0.01f;
        h2 = h + s1aa0.f14;    /* after disp, as the original orders them */
        if ((unsigned int)f1acc != q)
            f1ac8 = f1ac8 + 8.0f;
        if (g_eventMode == 0)
        {
            if (f1ac0 >= 100.0f)
            {
                ((GFStageView *)slot)->life->state = 3;
                disp = h;
            }
            else
                ((GFStageView *)slot)->life->state = 2;
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&nx, s1aa0.f10 + fe0);
        FloatToInt(&nh, h2 - (float)nd);
        if (g_d36cb0 == 0)
        {
            float frac;

            fnd = (float)nd;
            fr = 1.0f / h;
            frac = fr * fnd;
            if (frac > 0.0f && be4 == 0)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1aa0.f18, (float)nx, (float)nh, 0,
                    (float)nd, 0, (h - fnd) * fr, 1.0f, frac, -1);
        }
        else
        {
            float frac;

            fnd = (float)nd;
            frac = fnd / h;
            if (frac > 0.0f && be4 == 0)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1aa0.f18, (float)nx, (float)nh, 0,
                    (float)nd, 0, 0, 1.0f, frac, -1);
        }
        if (f1ac0 >= 100.0f)
        {
            unsigned int c;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->drawTex(s1aa0.f1c, (float)nx,
                                                 (float)nh, 0, 0, c);
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->drawTex(s1aa0.f1c, (float)nx,
                                                 (float)nh, 0, 0, c);
        }
    }
}

/* The second GF gauge, over s1ad0: the same plate/clip/chase/span shape
   as m464580, but the level lives in the stage object (+0x38) chased into
   f1ac4, this copy OWNS the shared beat latch (compares and stores), a
   death here raises the game-over flags (g_1b2efb4 solo with no elements,
   g_1b2eba4 otherwise, unless two players remain), and there is no
   ready/danger state block.

   56.83%: semantically complete - the residue is the FloatToInt home
   cascade (nd sits at the frame's first slot in the original, unreachable
   by declaration order) plus ebx's early pop and the arm xmm renames.
   The gauges' seed family. */
/* 58.99%. Both frames normalized with scratchpad/frame.py (2026-08-16),
   which CORRECTS the earlier reading that this function was short of
   locals: it is not. Both sides use exactly the same four top slots and
   permute them.

       original   int/float A = F-0xc   float B = F-0x10   homes F-0x8/F-0x4
       ours       int/float A = F-0x10  float B = F-0x4    homes F-0xc/F-0x8

   A is a FloatToInt destination that is later reused as a float - both
   sides reuse it, at different addresses - and the homes rotate between
   value and pointer identically. So this is the stack packer's permutation,
   the same family as the two-volatile-locals park in wip/screenrun.cpp,
   not a missing variable. Hoisting the inner block's two locals to function
   scope costs 1.4 points (57.55%), and scoping every local into the guarded
   block changes nothing. */
// FUNCTION: EZ2AC_UNPACKED 0x004649c0
void GFPanel::m4649c0()
{
    int nh;
    int nx;
    int nd;
    int n0;
    float disp;
    float fnd;
    float h;
    float h2;
    float fr;
    unsigned int q;

    if (s1ad0.f00 != 0)
    {
        FloatToInt(&n0, s1ad0.f04 + fe0);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(s1ad0.f0c, (float)n0,
                                             s1ad0.f08, 0, 0, -1);
        if (f1b4c != 0)
        {
            Obj402860 *o;
            int nc;

            FloatToInt(&nc, fe0);
            o = f1be4;
            o->f214 = (float)nc;
            o->f218 = 0.0f;
            f1be4->m403940();
            if (f1be4->isDone() != 0)
                f1be4->attach(0, 0);
        }
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1ad0.f18);
        h2 = h + s1ad0.f14;
        q = f24 / f28;
        f1ac4 = (((GFStageView *)slot)->f38 - f1ac4) * 0.2f + f1ac4;
        disp = (f1ac4 * 0.01f) * h;
        if (f1acc != (int)q)
        {
            f1ac4 = f1ac4 + 8.0f;
            f1acc = q;
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&nx, s1ad0.f10 + fe0);
        FloatToInt(&nh, h2 - (float)nd);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        if (0.0f >= ((GFStageView *)slot)->f38)
            be4 = 1;
        if (be4 == 1)
        {
            if (countListC() != 2)
            {
                if (g_1b2eb6c == 0 && g_elemCount == 0)
                    g_1b2efb4 = 1;
                else
                    g_1b2eba4 = 1;
            }
        }
        if (g_d36cb0 == 0)
        {
            float frac;

            fnd = (float)nd;
            fr = 1.0f / h;
            frac = fr * fnd;
            if (frac > 0.0f && be4 == 0)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1ad0.f18, (float)nx, (float)nh, 0,
                    (float)nd, 0, (h - fnd) * fr, 1.0f, frac, -1);
        }
        else
        {
            float frac;

            fnd = (float)nd;
            fr = 1.0f / h;
            frac = fr * fnd;
            if (frac > 0.0f && be4 == 0)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1ad0.f18, (float)nx, (float)nh, 0,
                    (float)nd, 0, 0, 1.0f, frac, -1);
        }
        if (((GFStageView *)slot)->f38 >= 100.0f)
        {
            /* Full: the gauge texture drawn twice more over the bar, each
               time in a fresh random colour - the same quad and UVs as the
               full-frame arm above, through m404660 (0x464dd3 / 0x464ea6). */
            unsigned int c;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            fnd = (float)nd;
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->m404660(
                (KTexture *)s1ad0.f18, (float)nx, (float)nh, 0, fnd,
                0, (h - fnd) * fr, 1.0f, fr * fnd, c);
            fnd = (float)nd;
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->m404660(
                (KTexture *)s1ad0.f18, (float)nx, (float)nh, 0, fnd,
                0, (h - fnd) * fr, 1.0f, fr * fnd, c);
        }
    }

}

/* The second gauge, over s1c08: the same plate/clip/chase/span as
   m42c0f0 (the chase state f1bf8 and both beat latches are SHARED with
   it), then the audience-rate danger tree - the rate target against each
   present player's window, with the players' own rates consulted when
   both are listed - lighting g_1b5f184, or in plain play just latching
   the death byte; the game-over globals; and the drawing: mix style 7
   gets a red-to-green dial rotated by the level (m404250, 106x106, angle
   180 - nd*1.8, spun the other way for the second side), everything else
   the usual sprite span with the over-100 double random-color flash.

   54.16%. The danger tree's SEMANTICS ARE VERIFIED against the exact
   branch graph (2026-08-15): the dispatch is a sub/je switch on
   g_1b2e7d4 with the default arm laid first (source order default, 2,
   1), the ==2 arm consults the players' own rates when both are listed,
   and the cross-jumped [eax+0x1f4] compare serves slot->f1f4 on one path
   and the second player's rate on the other. A switch-spelling scored
   WORSE (51%) than this if-chain, so the tree stays as written; the
   residual deficit is the function-wide zero-in-ebp hoist and the usual
   slot family. Safe for the port. */
/* 55.51%, AND 514 OF ITS 520 INSTRUCTIONS ARE RIGHT. The six missing are
   `push ebp`, `xor ebp,ebp` and four `pop ebp`: the original keeps ZERO in
   ebp and spends it nineteen times (every `push 0` argument, five `cmp
   [mem],ebp`, a `mov [g_1b5f184],ebp` and a `cmovle eax,ebp`), and cl
   declines the register and writes immediates. That missing push shifts
   every esp-relative offset by four, which is why a function this close
   scores this low. Both sides' push sequences were extracted and compared -
   19 `push ebp` against 18 `push 0`, no argument is missing. Do not chase
   it: see the zero-register rule in CLAUDE.md. */
// FUNCTION: EZ2AC_UNPACKED 0x0042c5f0
void Panel::m42c5f0()
{
    int nd;
    int nx;
    int nh;
    int n0;
    float disp;
    float h;
    float fnd;
    float h2;
    float fr;
    unsigned int q;

    if (s1c08.f00 != 0)
    {
        FloatToInt(&n0, s1c08.f04 + fe4);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(s1c08.f0c, (float)n0,
                                             s1c08.f08, 0, 0, -1);
        if (f1d30 != 0)
        {
            Obj402860 *o;
            int nc;

            FloatToInt(&nc, fe4);
            o = f1dcc;
            o->f214 = (float)nc;
            o->f218 = 0.0f;
            f1dcc->m403940();
            if (f1dcc->isDone() != 0)
                f1dcc->attach(0, 0);
        }
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1c08.f18);
        h2 = h + s1c08.f14;
        {
            float lvl = slot->f1f4;

            q = f28 / f2c;
            f1bf8 = (lvl - f1bf8) * 0.2f + f1bf8;
            disp = (f1bf8 * 0.01f) * h;
            if (f1c00 != (int)q)
            {
                f1c00 = q;
                f1bf8 = f1bf8 + 4.0f;
            }
            q = f28 / (unsigned int)f130;
            if (f1c04 != (int)q)
            {
                f1c04 = q;
                f1bf8 = f1bf8 + 4.0f;
            }
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&nx, s1c08.f10 + fe4);
        FloatToInt(&nh, h2 - (float)nd);
        if (g_useAudiencePanel == 1)
        {
            /* The danger flag per audience mode: 1 when the target rate
               is above the rate(s) that matter (and, in mode 2, the
               gauge is empty). */
            switch (g_1b2e7d4)
            {
            default:
                if (countListA() == 1)
                {
                    if (findListA(0) != 0)
                    {
                        if (g_rateTarget > g_rate1p)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                    else
                    {
                        if (g_rateTarget > g_rate2p)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                }
                else
                {
                    if (g_rateTarget > g_rate1p && g_rateTarget > g_rate2p)
                        g_1b5f184 = 1;
                    else
                        g_1b5f184 = 0;
                }
                break;
            case 2:
                if (countListA() == 1)
                {
                    if (findListA(0) != 0)
                    {
                        if (g_rateTarget > g_rate1p && 0.0f >= slot->f1f4)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                    else
                    {
                        if (g_rateTarget > g_rate2p && 0.0f >= slot->f1f4)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                }
                else
                {
                    if (g_rateTarget > g_rate1p && g_rateTarget > g_rate2p &&
                        0.0f >= ((RateNode *)findListA(0))->rate &&
                        0.0f >= ((RateNode *)findListA(1))->rate)
                        g_1b5f184 = 1;
                    else
                        g_1b5f184 = 0;
                }
                break;
            case 1:
                if (countListA() == 1)
                {
                    if (findListA(0) == 0)
                    {
                        if (g_rateTarget > g_rate2p)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                    else
                    {
                        if (g_rateTarget > g_rate1p)
                            g_1b5f184 = 1;
                        else
                            g_1b5f184 = 0;
                    }
                }
                else
                {
                    if (g_rateTarget > g_rate1p && g_rateTarget > g_rate2p)
                        g_1b5f184 = 1;
                    else
                        g_1b5f184 = 0;
                }
                break;
            }
        }
        else if (0.0f >= slot->f1f4)
            b118 = 1;

after:
        if (b118 == 1)
        {
            if (g_1b2eb6c == 0 && g_elemCount == 0)
                g_1b2efb4 = 1;
            else
                g_1b2eba4 = 1;
        }
        if (g_mixStyle != 7)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
            if (g_d36cb0 == 0)
            {
                if (slot->f1f4 > 0.0f && b118 == 0)
                {
                    fnd = (float)nd;
                    fr = 1.0f / h;
                    ((KGraphics *)g_sub1b2f054)->m404660(
                        (KTexture *)s1c08.f18, (float)nx, (float)nh, 0,
                        fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, -1);
                }
            }
            else if (slot->f1f4 > 0.0f && b118 == 0)
            {
                fnd = (float)nd;
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1c08.f18, (float)nx, (float)nh, 0,
                    fnd, 0, 0, 1.0f, fnd / h2, -1);
            }
            if (slot->f1f4 >= 100.0f)
            {
                unsigned int c;

                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                fnd = (float)nd;
                fr = 1.0f / h;
                c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                      rand() % 256) << 8) | rand() % 256;
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1c08.f18, (float)nx, (float)nh, 0,
                    fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, c);
                fnd = (float)nd;
                c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                      rand() % 256) << 8) | rand() % 256;
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1c08.f18, (float)nx, (float)nh, 0,
                    fnd, 0, (h - fnd) * fr, 1.0f, fnd * fr, c);
            }
        }
        else if (g_d36cb0 == 0)
        {
            int v;
            unsigned int color;
            float a;
            float lim;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            fnd = (float)nd;
            lim = fe8 * 0.5f;
            a = fnd * 5.12f;
            if (lim > a)
            {
                v = (int)(a * 127.5f / lim);
                if (v <= 0)
                    v = 0;
            }
            else
            {
                v = (int)(a * 127.5f / lim + 30.0f);
                if (v > 255)
                    v = 255;
            }
            color = ((0xffffff00 | (unsigned int)(-1 - v)) << 8 | v) << 8;
            if (fnd / h2 > 0.0f && b118 == 0)
            {
                float ang = 180.0f - (float)nd * 1.8f;

                if (findListA(0) != 0)
                    ((KGraphics *)g_sub1b2f054)->m404250(
                        (Sprite *)s1c08.f18, (float)nx, s1c08.f14,
                        106.0f, 106.0f, ang, color);
                else
                    ((KGraphics *)g_sub1b2f054)->m404250(
                        (Sprite *)s1c08.f18, (float)nx, s1c08.f14,
                        106.0f, 106.0f, 0.0f - ang, color);
            }
        }
        else
        {
            fnd = (float)nd;
            if (fnd / h2 > 0.0f && b118 == 0)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1c08.f18, (float)nx, (float)nh, 0,
                    fnd, 0, 0, 1.0f, fnd / h2, -1);
        }
    }
}

/* The tempo-summary block the sink's query fills - a struct, because an
   int-array local is a /GS buffer and grows a cookie the original's
   frame does not have. */
struct QGrid
{
    int f0;
    int f4;
    int f8;
    int beats;                 /* 0x0c */
    int f10;
    int f14;
    int f18;
    int f1c;
};

/* Draw the measure grid: the sink summarizes the run's tempo, the first
   line lands on the measure boundary at or after the visible start (the
   whole run when f30 asks), and each tempo change re-segments the grid -
   the sink's next(9) walk hands back each segment's start and its beats
   per measure, the line spacing follows, and a line scrolled above the
   panel's top stops the walk. Every line is the f1390[phase] texture at
   the m427d90 height formula.

   70.36%: semantics complete; the residue is the cnt/grid register roles
   (edi/ebx/ebp rotated), the low-slot shuffles, and the original's dead
   qinfo.beats load ours dead-code-eliminates even through a volatile
   read. A char[3] pad in SlotState briefly gave every state local a /GS
   cookie - single chars fixed it (the players.h warning applies to ANY
   char array member, not just big ones). */
// FUNCTION: EZ2AC_UNPACKED 0x0042b550
void Panel::m42b550()
{
    int segend;
    float t;
    int stop;
    int seglen;
    unsigned int cnt;
    int idx;
    SlotSound snd;
    QGrid qinfo;
    SlotState st;

    if (f137c != 0)
    {
        int cur;
        unsigned int mlen;
        int grid;
        int lim;

        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        cnt = f2c;
        t = f1848 - g_1b2ef8c - items[1].f08 + f1850;
        idx = (int)(f28 % cnt * f1380 / cnt) - 1;
        idx = idx < 0 ? 0 : idx;
        slot->f3d4->query((int *)&qinfo, 8);
        mlen = *(volatile int *)&qinfo.beats;
        if (f30 != 0)
            cur = f28;
        else
        {
            cur = (int)((t + items[1].f08 - 480.0f) /
                        ((48.0f / (float)(int)f2c) * items[1].fe8)) + f28;
            if (cur < 0)
                cur = 0;
        }
        if (slot->f3d4->begin(0, cur, 2, 9, &st, &snd) != 0)
        {
            int base = st.start;

            mlen = st.beats * cnt;
            f130 = mlen;
            grid = ((cur - base) / mlen + 1) * mlen + base;
            {
                double q = 48.0f / (int)f2c;

                lim = (int)(float)((t + items[1].f08 - items[1].f08) /
                                   (q * items[1].fe8)) + f28 + mlen;
            }
            stop = 0;
            if (grid < lim)
            {
                do
                {
                    if (stop != 0)
                        break;
                    if (snd.f0 != -1)
                    {
                        if (slot->f3d4->next(9, &st, &snd) == 1)
                        {
                            seglen = st.beats * cnt;
                            segend = st.start;
                            if (st.start > lim)
                                segend = lim;
                        }
                        else
                            segend = lim;
                    }
                    else
                        segend = lim;
                    while (grid < segend)
                    {
                        int x = f34 == 1 ? grid - 12 : grid;
                        float y;

                        x -= f28;
                        y = t - (48.0f / (float)(int)f2c) * (float)x *
                                items[1].fe8 + items[1].f08 -
                            f138c * 0.5f;
                        if (items[1].f08 - f138c > y)
                        {
                            stop = 1;
                            break;
                        }
                        ((KGraphics *)g_sub1b2f054)->drawTex(
                            f1390[idx], f1384 + fe4, y + g_1b2ef8c,
                            f1388, f138c, -1);
                        grid += mlen;
                    }
                    mlen = seglen;
                }
                while (grid < lim);
            }
        }
    }
}

/* Panel::m42b550 on the scratch panel: no scroll-global bias, no f130
   latch, and the line's y goes to the blit as its own local. 66.85%,
   the twin's register-role residue. */
// FUNCTION: EZ2AC_UNPACKED 0x00463d30
void GFPanel::m463d30()
{
    int segend;
    float t;
    int stop;
    int seglen;
    unsigned int cnt;
    int idx;
    SlotSound snd;
    QGrid qinfo;
    SlotState st;

    if (f1344 != 0)
    {
        int cur;
        unsigned int mlen;
        int grid;
        int lim;

        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        cnt = f28;
        t = f17e4 - items[1].f08 + f17ec;
        idx = (int)(f24 % cnt * f1348 / cnt) - 1;
        idx = idx < 0 ? 0 : idx;
        ((GFStageView *)slot)->sink->query((int *)&qinfo, 8);
        mlen = *(volatile int *)&qinfo.beats;
        if (f2c != 0)
            cur = f24;
        else
        {
            cur = (int)((t + items[1].f08 - 480.0f) /
                        ((48.0f / (float)(int)f28) * items[1].fe8)) + f24;
            if (cur < 0)
                cur = 0;
        }
        if (((GFStageView *)slot)->sink->begin(0, cur, 2, 9, &st, &snd) != 0)
        {
            int base = st.start;

            mlen = st.beats * cnt;
            grid = ((cur - base) / mlen + 1) * mlen + base;
            {
                double q = 48.0f / (int)f28;

                lim = (int)(float)((t + items[1].f08 - items[1].f08) /
                                   (q * items[1].fe8)) + f24 + mlen;
            }
            stop = 0;
            if (grid < lim)
            {
                do
                {
                    if (stop != 0)
                        break;
                    if (snd.f0 != -1)
                    {
                        if (((GFStageView *)slot)->sink->next(9, &st, &snd) == 1)
                        {
                            seglen = st.beats * cnt;
                            segend = st.start;
                            if (st.start > lim)
                                segend = lim;
                        }
                        else
                            segend = lim;
                    }
                    else
                        segend = lim;
                    while (grid < segend)
                    {
                        int x = f30 == 1 ? grid - 12 : grid;
                        float y;

                        x -= f24;
                        y = t - (48.0f / (float)(int)f28) * (float)x *
                                items[1].fe8 + items[1].f08 -
                            f1354 * 0.5f;
                        if (items[1].f08 - f1354 > y)
                        {
                            stop = 1;
                            break;
                        }
                        ((KGraphics *)g_sub1b2f054)->drawTex(
                            f1358[idx], f134c + fe0, y,
                            f1350, f1354, -1);
                        grid += mlen;
                    }
                    mlen = seglen;
                }
                while (grid < lim);
            }
        }
    }
}

/* GFPanel::m463030 on the EZ2DJ panel's own offsets. */
// FUNCTION: EZ2AC_UNPACKED 0x0042a450
void Panel::m42a450()
{
    int i = 0;

    if (f12c > 0)
    {
        char *q = (char *)&items[0].f5c;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x5c);

            if (e->f00 != 0)
            {
                if (e->ff8 == 1)
                {
                    e->ff4 = e->f60 + e->ff4;
                    if (e->ff4 > e->f5c)
                        e->ff4 = e->f5c;
                }
                else
                {
                    if (e->ff8 != 2)
                        goto skip;
                    e->ff4 = e->ff4 - e->f64;
                    if (0.0f >= e->ff4)
                    {
                        e->ff4 = 0.0001f;
                        e->ff8 = 0;
                    }
                }
                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                {
                    int n;

                    FloatToInt(&n, e->f08 + e->f10 - e->ff4);
                    ((KGraphics *)g_sub1b2f054)->drawTex(e->f58,
                                                         e->f04 + fe4,
                                                         (float)n, e->f0c,
                                                         e->ff4,
                                                         (int)e->f54);
                }
            }
skip:
            i++;
            q += 0x104;
        }
        while (i < f12c);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00463030
void GFPanel::m463030()
{
    int i = 0;

    if (ff8 > 0)
    {
        char *q = (char *)&items[0].f5c;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x5c);

            if (e->f00 != 0)
            {
                if (e->ff8 == 1)
                {
                    e->ff4 = e->f60 + e->ff4;
                    if (e->ff4 > e->f5c)
                        e->ff4 = e->f5c;
                }
                else
                {
                    if (e->ff8 != 2)
                        goto skip;
                    e->ff4 = e->ff4 - e->f64;
                    if (0.0f >= e->ff4)
                    {
                        e->ff4 = 0.0001f;
                        e->ff8 = 0;
                    }
                }
                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                {
                    int n;

                    FloatToInt(&n, e->f08 + e->f10 - e->ff4);
                    ((KGraphics *)g_sub1b2f054)->drawTex(e->f58,
                                                         e->f04 + fe0,
                                                         (float)n, e->f0c,
                                                         e->ff4,
                                                         (int)e->f54);
                }
            }
skip:
            i++;
            q += 0x104;
        }
        while (i < ff8);
    }
}

/* Panel::m42a580 on the GF panel's own offsets. */
// FUNCTION: EZ2AC_UNPACKED 0x00463160
void GFPanel::m463160()
{
    int i = 0;

    if (ff8 > 0)
    {
        char *q = (char *)&items[0].f4c;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x4c);

            if (e->f00 != 0)
            {
                ((KGraphics *)g_sub1b2f054)->setBlend(e->f4c, e->f50);
                if (e->ff8 == 1)
                {
                    int n;

                    FloatToInt(&n, e->f3c + fe0);
                    ((KGraphics *)g_sub1b2f054)->drawTex(e->f44, (float)n,
                                                         e->f40, 0, 0,
                                                         (int)e->f48);
                }
            }
            i++;
            q += 0x104;
        }
        while (i < ff8);
    }
}


/* The same again as an expression. The difference is not cosmetic: the
   destination is then a compiler TEMPORARY rather than a named local, and cl
   packs the two kinds differently - a named local reserves its slot for the
   whole function and pushes every __asm temp above it, while a temporary
   overlaps freely. m42bbe0's frame only comes out right with the first two
   conversions written as locals and the third as this. */
__forceinline int RoundToInt(float f)
{
    int i;

    FloatToInt(&i, f);
    return i;
}

/* Two readouts drawn through @0x40a970, one per section, each at the rung of
   the scale ladder its own SLOT field asks for. */
// FUNCTION: EZ2AC_UNPACKED 0x00464250
void GFPanel::m464250()
{
    int n;

    if (s1880.f00 == 0)
    {
        return;
    }

    FloatToInt(&n, s1880.f04 + fe0);
    ((KGraphics *)g_sub1b2f054)->setBlend(s1880.f80, s1880.f84);
    ((GfxText *)g_sub1b2f054)->m40a970((float)n, s1880.f08, s1880.f0c,
                                       s1880.f10, s1880.f14, slot->f2c,
                                       s1880.name, -1, s1880.f18);
}

// FUNCTION: EZ2AC_UNPACKED 0x00464300
void GFPanel::m464300()
{
    int n;

    if (s1908.f00 == 0)
    {
        return;
    }

    FloatToInt(&n, s1908.f04 + fe0);
    ((KGraphics *)g_sub1b2f054)->setBlend(s1908.f80, s1908.f84);
    ((GfxText *)g_sub1b2f054)->m40a970((float)n, s1908.f08, s1908.f0c,
                                       s1908.f10, s1908.f14, slot->f30,
                                       s1908.name, -1, s1908.f18);
}

/* The third goes through drawNumF instead, takes its scale from the LIVE
   player's rung rather than the slot's, and has no stored step - its record is
   the eleven-texture shape, which spends that float on the extra texture, so
   the step is a third of the width. */
// FUNCTION: EZ2AC_UNPACKED 0x004644c0
void GFPanel::m4644c0()
{
    int n;

    if (s1a18.f00 == 0)
    {
        return;
    }

    FloatToInt(&n, s1a18.f04 + fe0);
    ((KGraphics *)g_sub1b2f054)->setBlend(s1a18.f80, s1a18.f84);
    ((GfxText *)g_sub1b2f054)->drawNumF((float)n, s1a18.f08, s1a18.f0c,
                                        s1a18.f10, s1a18.f0c * (1.0f / 3.0f),
                                        g_playerScale[g_elemCount],
                                        s1a18.name, -1, s1a18.f14);
}

/* CatchPanel's pair - the same readout twice, on the two GdsTexSection10
   records m470bc0 / m470cc0 load, and with no rounding: the x goes straight
   out of the record. */
// FUNCTION: EZ2AC_UNPACKED 0x00470c50
void CatchPanel::m470c50()
{
    if (s19f4.f00 == 0)
    {
        return;
    }

    ((KGraphics *)g_sub1b2f054)->setBlend(s19f4.f80, s19f4.f84);
    ((GfxText *)g_sub1b2f054)->m40a970(s19f4.f04, s19f4.f08, s19f4.f0c,
                                       s19f4.f10, s19f4.f14, slot->f1e0,
                                       s19f4.name, -1, s19f4.f18);
}

// FUNCTION: EZ2AC_UNPACKED 0x00470d50
void CatchPanel::m470d50()
{
    if (s1a7c.f00 == 0)
    {
        return;
    }

    ((KGraphics *)g_sub1b2f054)->setBlend(s1a7c.f80, s1a7c.f84);
    ((GfxText *)g_sub1b2f054)->m40a970(s1a7c.f04, s1a7c.f08, s1a7c.f0c,
                                       s1a7c.f10, s1a7c.f14, slot->f1e8,
                                       s1a7c.name, -1, s1a7c.f18);
}

/* Panel::m42bd80 @0x42bd80 - the counter readout.

   In mix style 7 it is RIGHT-ALIGNED: "%d", "%02d", "%03d" or "%04d" according
   to how many digits the value has, with the x pulled left by one character
   width per extra digit - which is what names f18f8 as the character width.
   Every other style just prints it with the record's own inline format string
   and no shift. Five figures and up draw nothing at all.

   The four arms are written out in full; cl's tail merger folds their shared
   argument pushes back into the two blocks the original has. */
// FUNCTION: EZ2AC_UNPACKED 0x0042BD80
void Panel::m42bd80()
{
    int n;

    if (f18e4 == 0)
    {
        return;
    }

    FloatToInt(&n, f18e8 + fe4);

    if (g_mixStyle != 7)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(f19a4, f19a8);
        ((GfxText *)g_sub1b2f054)->m40a970((float)n, f18ec, f18f0, f18f4,
                                           f18f8, slot->f1e0, n1924, -1,
                                           f18fc);
        return;
    }

    ((KGraphics *)g_sub1b2f054)->setBlend(f19a4, f19a8);

    {
        /* Read AFTER the setBlend call: declared before it, the value would
           have to survive the call and would take a callee-saved register the
           original does not use. */
        int v = slot->f1e0;

        if (v < 10)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n, f18ec, f18f0, f18f4,
                                               f18f8, v,
            /* 0x488c04 - already STRING-pinned in src/systemmenu.cpp; one
               marker per address is enough. */
                                               "%d",
                                               -1, f18fc);
        }
        else if (v < 100)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - f18f8, f18ec, f18f0,
                                               f18f4, f18f8, v,
            // STRING: EZ2AC_UNPACKED 0x004904e0
                                               "%02d",
                                               -1, f18fc);
        }
        else if (v < 1000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - f18f8 * 2.0f, f18ec,
                                               f18f0, f18f4, f18f8, v,
            // STRING: EZ2AC_UNPACKED 0x004904d8
                                               "%03d",
                                               -1, f18fc);
        }
        else if (v < 10000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - f18f8 * 3.0f, f18ec,
                                               f18f0, f18f4, f18f8, v,
            // STRING: EZ2AC_UNPACKED 0x004904d0
                                               "%04d",
                                               -1, f18fc);
        }
    }
}

/* Panel::m42bf30 @0x42bf30 - m42bd80's twin on the next record, and the same
   right-alignment scheme carried out to SIX digits. Two differences beyond the
   record and the counter: the blend is set once before the style test rather
   than in both arms, and so is the slot load. */
// FUNCTION: EZ2AC_UNPACKED 0x0042BF30
void Panel::m42bf30()
{
    int n;

    if (s19ac.f00 == 0)
    {
        return;
    }

    FloatToInt(&n, s19ac.f04 + fe4);
    ((KGraphics *)g_sub1b2f054)->setBlend(s19ac.f80, s19ac.f84);

    if (g_mixStyle != 7)
    {
        ((GfxText *)g_sub1b2f054)->m40a970((float)n, s19ac.f08, s19ac.f0c,
                                           s19ac.f10, s19ac.f14, slot->f1e4,
                                           s19ac.name, -1, s19ac.f18);
        return;
    }

    {
        int v = slot->f1e4;

        if (v < 10)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n, s19ac.f08, s19ac.f0c,
                                               s19ac.f10, s19ac.f14, v,
                                               "%d", -1, s19ac.f18);
        }
        else if (v < 100)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - s19ac.f14, s19ac.f08,
                                               s19ac.f0c, s19ac.f10, s19ac.f14,
                                               v, "%02d", -1, s19ac.f18);
        }
        else if (v < 1000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - s19ac.f14 * 2.0f,
                                               s19ac.f08, s19ac.f0c, s19ac.f10,
                                               s19ac.f14, v, "%03d", -1,
                                               s19ac.f18);
        }
        else if (v < 10000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - s19ac.f14 * 3.0f,
                                               s19ac.f08, s19ac.f0c, s19ac.f10,
                                               s19ac.f14, v, "%04d", -1,
                                               s19ac.f18);
        }
        else if (v < 100000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - s19ac.f14 * 4.0f,
                                               s19ac.f08, s19ac.f0c, s19ac.f10,
            // STRING: EZ2AC_UNPACKED 0x004904f0
                                               s19ac.f14, v, "%05d", -1,
                                               s19ac.f18);
        }
        else if (v < 1000000)
        {
            ((GfxText *)g_sub1b2f054)->m40a970((float)n - s19ac.f14 * 5.0f,
                                               s19ac.f08, s19ac.f0c, s19ac.f10,
            // STRING: EZ2AC_UNPACKED 0x004904e8
                                               s19ac.f14, v, "%06d", -1,
                                               s19ac.f18);
        }
    }
}

/* What GFPanel's readout @0x4643b0 gates its decay on. `slot->f1e0` is read as
   a POINTER here and as an integer counter by Panel's @0x42bd80, so the three
   panels evidently do not point `slot` at the same object; the cast keeps both
   readings honest until that is settled. */
struct GaugeGate
{
    char _pad0[0x330];
    char b330;                 /* 0x330 */
    char _pad331[0x784 - 0x331];
    int  f784;                 /* 0x784 */
};

/* GFPanel::m4643b0 @0x4643b0 - the level readout, which BLEEDS. When the gate
   allows it the level drops 0.0017 a frame and is floored at 0.001 (not at
   zero - the readout keeps showing something), and what is printed is the
   level less 0.05. */
// FUNCTION: EZ2AC_UNPACKED 0x004643B0
void GFPanel::m4643b0()
{
    int n;

    if (s1990.f00 == 0)
    {
        return;
    }

    {
        GaugeGate *g = *(GaugeGate **)&slot->f1e0;

        if (g->f784 == 0 && g->b330 != 0)
        {
            slot->f54 = slot->f54 - 0.0017f;
            if (0.0f > slot->f54)
            {
                slot->f54 = 0.001f;
            }
        }
    }

    FloatToInt(&n, s1990.f04 + fe0);
    ((KGraphics *)g_sub1b2f054)->setBlend(s1990.f80, s1990.f84);
    ((GfxText *)g_sub1b2f054)->drawNumF((float)n, s1990.f08, s1990.f0c,
                                        s1990.f10, s1990.f0c * (1.0f / 3.0f),
                                        slot->f54 - 0.05f,
                                        s1990.name, -1, s1990.f14);
}

/* GFPanel's own copy of the quarter-step scale ladder - the third, after
   0x48e6c8 and 0x49deb8. */
// GLOBAL: EZ2AC_UNPACKED 0x0049a1e0
float g_gfScaleLadder[34];

/* GFPanel::m465000 @0x465000 - the readout m4622c0 loads. Its gate sits three
   dwords ahead of the record proper (the section carries three .str resources
   before its geometry), and its value is the rung of the ladder the slot asks
   for. */
// FUNCTION: EZ2AC_UNPACKED 0x00465000
void GFPanel::m465000()
{
    int n;

    if (f1b4c == 0)
    {
        return;
    }

    FloatToInt(&n, f1b5c + fe0);
    ((KGraphics *)g_sub1b2f054)->setBlend(f1bd8, f1bdc);
    ((GfxText *)g_sub1b2f054)->drawNumF((float)n, f1b60, f1b64, f1b68,
                                        f1b64 * (1.0f / 3.0f),
                                        g_gfScaleLadder[slot->f2f0],
                                        f1b98, -1, f1b6c);
}

/* Panel::m42bbe0 @0x42bbe0 and GFPanel::m4640d0 @0x4640d0 - the single-texture
   section, drawn at INTEGER coordinates (both are rounded through the __asm
   helper), followed by parking a chart at the panel's own x.

   Only three of the record's six fields are used: the position pair and the
   texture. The chart's y is set to zero outright, and the panel's x origin is
   rounded a third time to get its x. The two differ in one line - Panel sets
   the blend to (2, 1) first and GFPanel leaves it alone. */
// FUNCTION: EZ2AC_UNPACKED 0x0042BBE0
void Panel::m42bbe0()
{
    int x;
    int y;

    if (s1824.f00 == 0)
    {
        return;
    }

    FloatToInt(&x, s1824.f04 + fe4);
    FloatToInt(&y, s1824.f08);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
    ((KGraphics *)g_sub1b2f054)->drawTex(s1824.f14, (float)x, (float)y,
                                         0.0f, 0.0f, -1);

    if (f1d30 == 0)
    {
        return;
    }

    {
        /* The value FIRST, then the pointer: one load serves both stores (two
           member writes make cl reload between them) but it must not be
           hoisted above the conversion, which naming the pointer earlier
           does. The call afterwards does reload it. */
        float v = (float)RoundToInt(fe4);
        Obj402860 *c = f1dc8;

        c->f214 = v;
        c->f218 = 0.0f;
    }
    f1dc8->m403940();
}

// FUNCTION: EZ2AC_UNPACKED 0x004640D0
void GFPanel::m4640d0()
{
    int x;
    int y;

    if (s17c4.f00 == 0)
    {
        return;
    }

    FloatToInt(&x, s17c4.f04 + fe0);
    FloatToInt(&y, s17c4.f08);
    ((KGraphics *)g_sub1b2f054)->drawTex(s17c4.f14, (float)x, (float)y,
                                         0.0f, 0.0f, -1);

    if (f1b4c == 0)
    {
        return;
    }

    {
        float v = (float)RoundToInt(fe0);
        Obj402860 *c = f1be0;

        c->f214 = v;
        c->f218 = 0.0f;
    }
    f1be0->m403940();
}

/* The frame the marker's animation is showing. The counter is taken modulo the
   period, scaled to six steps, and stepped back one - a frame of -1 (the whole
   first period) clamps to 0, which the original does branchlessly with
   `setl`/`sub`/`and`. */
static __forceinline int cycleFrame(unsigned int counter, unsigned int period)
{
    int i = (int)((counter % period) * 6 / period) - 1;

    /* A SELECT, not an overwrite: this is the shape cl if-converts to the
       original's branchless `setl` / `sub` / `and`. */
    return (i < 0) ? 0 : i;
}

/* Panel::m42bce0 @0x42bce0 and GFPanel::m4641c0 @0x4641c0 - the cycling
   marker: one of six textures, chosen by where the counter sits in its period,
   drawn at the panel's x (rounded) and the record's own y. Panel's y is
   further offset by a global; GFPanel's is not. */
// FUNCTION: EZ2AC_UNPACKED 0x0042BCE0
void Panel::m42bce0()
{
    int x;

    if (f1840 == 0)
    {
        return;
    }

    {
        int frame = cycleFrame(f28, f2c);

        FloatToInt(&x, f1844 + fe4);
        ((KGraphics *)g_sub1b2f054)->drawTex(f1854[frame], (float)x,
                                             f1848 - g_1b2ef8c, 0.0f, 0.0f, -1);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x004641C0
void GFPanel::m4641c0()
{
    int x;

    if (f17dc == 0)
    {
        return;
    }

    {
        int frame = cycleFrame(f24, f28);

        FloatToInt(&x, f17e0 + fe0);
        ((KGraphics *)g_sub1b2f054)->drawTex(f17f0[frame], (float)x, f17e4,
                                             0.0f, 0.0f, -1);
    }
}

/* Panel::m42d290 @0x42d290 and GFPanel::m4650c0 @0x4650c0 - park the chart at
   the panel's origin (rounded, y zeroed) and refresh it; the refresh's return
   value decides whether anything else is drawn, and when it is zero cl spends
   that same register on the draw's two zero sizes. */
// FUNCTION: EZ2AC_UNPACKED 0x0042D290
void Panel::m42d290()
{
    int a;

    if (f1de8 == 0)
    {
        return;
    }

    FloatToInt(&a, fe4);
    {
        Obj402860 *c = f1e04;

        c->f214 = (float)a;
        c->f218 = 0.0f;
    }

    if (f1e04->m403940() != 0)
    {
        return;
    }

    {
        int x;
        int y;

        FloatToInt(&x, f1dec + fe4);
        FloatToInt(&y, f1df0);
        ((KGraphics *)g_sub1b2f054)->drawTex(f1dfc, (float)x, (float)y,
                                             0.0f, 0.0f, -1);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x004650C0
void GFPanel::m4650c0()
{
    int a;

    if (f1bec == 0)
    {
        return;
    }

    FloatToInt(&a, fe0);
    {
        Obj402860 *c = f1c08;

        c->f214 = (float)a;
        c->f218 = 0.0f;
    }

    if (f1c08->m403940() != 0)
    {
        return;
    }

    {
        int x;
        int y;

        FloatToInt(&x, f1bf0 + fe0);
        FloatToInt(&y, f1bf4);
        ((KGraphics *)g_sub1b2f054)->drawTex(f1c00, (float)x, (float)y,
                                             0.0f, 0.0f, -1);
    }
}

/* CatchPanel::m471b80 @0x471b80 - the third cycling marker, on the TargetBar
   record. Same six-step cycle as Panel's and GFPanel's, but its x is the
   record's own (no panel origin) and its y is an INT field converted and added
   to the record's. */
// FUNCTION: EZ2AC_UNPACKED 0x00471B80
void CatchPanel::m471b80()
{
    int x;

    if (f1948 == 0)
    {
        return;
    }

    {
        int frame = cycleFrame(f250, f254);

        FloatToInt(&x, f194c);
        ((KGraphics *)g_sub1b2f054)->drawTex(f195c[frame], (float)x,
                                             (float)f19e4 + f1950,
                                             0.0f, 0.0f, -1);
    }
}

/* CatchPanel::m471ae0 @0x471ae0 - the plain rectangle m470940 loads, drawn at
   integer coordinates with the texture's own size, and then a chart refresh
   that is gated on a different section's flag entirely. */
// FUNCTION: EZ2AC_UNPACKED 0x00471AE0
void CatchPanel::m471ae0()
{
    int y;
    int x;

    if (s1930.f00 == 0)
    {
        return;
    }

    FloatToInt(&x, s1930.f04);
    FloatToInt(&y, s1930.f08);
    ((KGraphics *)g_sub1b2f054)->drawTex(s1930.f14, (float)x, (float)y,
                                         0.0f, 0.0f, -1);

    if (f1bb0 == 0)
    {
        return;
    }

    f1c44->m403940();
}

/* How far through its period the fade is, in 0..254 - the same shape as
   cycleFrame but with 255 steps instead of six, and floored the same way. */
static __forceinline int fadeStep(unsigned int counter, unsigned int period)
{
    int i = (int)((counter % period) * 255 / period) - 1;

    return (i < 0) ? 0 : i;
}

/* Panel::m42cf40 @0x42cf40 and GFPanel::m464ec0 @0x464ec0 - draw the
   GrooveLight records. Each one is drawn twice: once additively at full
   brightness, then again over it with the normal blend and a grey that fades
   out across the counter's period. The alpha is written `-1 - step` rather
   than `255 - step`, which is the same byte but a separate register - the
   original computes both. */
/* The record's fields as offsets in ints from its +0x10, which is where the
   original anchors the loop's induction pointer; indexed as s1cd8[i] cl
   anchors at +4 instead and every displacement in the body is off by 12. */
enum
{
    kGlEnabled = -4,   /* f00 */
    kGlX1      = -3,   /* f04 */
    kGlY1      = -2,   /* f08 */
    kGlW1      = -1,   /* f0c */
    kGlH1      =  0,   /* f10 */
    kGlTex1    =  1,   /* f14 */
    kGlX2      =  2,   /* f18 */
    kGlY2      =  3,   /* f1c */
    kGlW2      =  4,   /* f20 */
    kGlH2      =  5,   /* f24 */
    kGlTex2    =  6    /* f28 */
};

#define GL_F(p, k) (*(const float *)&(p)[k])
#define GL_T(p, k) (*(KTexture *const *)&(p)[k])
/* Writable views of the same, for the loops that step a level in place. */
#undef GL_F
#define GL_F(p, k) (*(float *)&(p)[k])
#define GL_I(p, k) (*(int *)&(p)[k])

// FUNCTION: EZ2AC_UNPACKED 0x0042CF40
void Panel::m42cf40()
{
    int i;
    int step = fadeStep(f28, f2c);

    /* A GUARDED do-while, not a for: the count is tested once before the
       pointer is set up, so that setup lands in the loop's preheader and
       `push esi` with it. A plain `for` with the pointer in its init makes esi
       live from the prologue; an `if` around a `for` emits the test twice. */
    /* `i = 0` sits BEFORE the guard so the test compares against the zeroed
       counter register, exactly as the original does, and the pointer is set
       up inside so its `push esi` lands in the preheader rather than the
       prologue. */
    i = 0;

    if (i < f1cd4)
    {
        const int *p = (const int *)&s1cd8[0].f10;

    do
    {
        if (p[kGlEnabled] != 0)
        {
            int v;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
            FloatToInt(&v, GL_F(p, kGlX1) + fe4);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex1), (float)v,
                                                 GL_F(p, kGlY1),
                                                 GL_F(p, kGlW1),
                                                 GL_F(p, kGlH1), -1);

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            FloatToInt(&v, GL_F(p, kGlX2) + fe4);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex2), (float)v,
                                                 GL_F(p, kGlY2),
                                                 GL_F(p, kGlW2),
                                                 GL_F(p, kGlH2),
                                                 ((-1 - step) << 24) |
                                                 ((255 - step) << 16) |
                                                 ((255 - step) << 8) |
                                                 (255 - step));
        }

        i++;
        p += sizeof(GdsGrooveLight) / 4;
    } while (i < f1cd4);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00464EC0
void GFPanel::m464ec0()
{
    int i;
    int step = fadeStep(f24, f28);

    /* A GUARDED do-while, not a for: the count is tested once before the
       pointer is set up, so that setup lands in the loop's preheader and
       `push esi` with it. A plain `for` with the pointer in its init makes esi
       live from the prologue; an `if` around a `for` emits the test twice. */
    i = 0;

    if (i < f1af0)
    {
        const int *p = (const int *)&s1af4[0].f10;

    do
    {
        if (p[kGlEnabled] != 0)
        {
            int v;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
            FloatToInt(&v, GL_F(p, kGlX1) + fe0);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex1), (float)v,
                                                 GL_F(p, kGlY1),
                                                 GL_F(p, kGlW1),
                                                 GL_F(p, kGlH1), -1);

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            FloatToInt(&v, GL_F(p, kGlX2) + fe0);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex2), (float)v,
                                                 GL_F(p, kGlY2),
                                                 GL_F(p, kGlW2),
                                                 GL_F(p, kGlH2),
                                                 ((-1 - step) << 24) |
                                                 ((255 - step) << 16) |
                                                 ((255 - step) << 8) |
                                                 (255 - step));
        }

        i++;
        p += sizeof(GdsGrooveLight) / 4;
    } while (i < f1af0);
    }
}

/* A fourth copy of the quarter-step ladder, this one indexed by the slot's
   +0x4e0 - the same index as g_panelRateLadder's 21 entries, so [16] was
   short; 34 floats like the others (see src/effectordraw.cpp). */
// GLOBAL: EZ2AC_UNPACKED 0x004901a0
float g_panelSpeedLadder[34];

/* Panel::m42d080 @0x42d080 - the speed readout, and the first user of the
   family's ten-argument base @0x40aaa0 (drawNumF is a nine-argument forwarder
   into it).

   Above ten the mode's own rate is printed as "%2.1f" from a hard-coded
   format; below it the PLAYER's speed percent is printed with the record's own
   inline format instead. The second test is redundant given the first and the
   original emits it anyway. */
// FUNCTION: EZ2AC_UNPACKED 0x0042D080
void Panel::m42d080()
{
    int x;

    if (f1d30 == 0)
    {
        return;
    }

    FloatToInt(&x, f1d40 + fe4);
    ((KGraphics *)g_sub1b2f054)->setBlend(f1dc0, f1dc4);

    if (10.0f > g_panelSpeedLadder[slot->f4e0])
    {
        ((GfxText *)g_sub1b2f054)->m40aaa0((float)x, f1d44, f1d48, f1d4c,
                                           f1d50, f1d50 * 0.4f,
                                           (float)g_speedPercent * 0.01f,
                                           f1d80, -1, f1d54);
    }
    else if (g_panelSpeedLadder[slot->f4e0] >= 10.0f)
    {
        ((GfxText *)g_sub1b2f054)->m40aaa0((float)x, f1d44, f1d48, f1d4c,
                                           f1d50, f1d50 * 0.4f,
                                           g_panelSpeedLadder[slot->f4e0],
        // STRING: EZ2AC_UNPACKED 0x00490510
                                           "%2.1f",
                                           -1, f1d54);
    }
}

/* The rate ladder the readout below prints - 1.0, 1.5, 2.0, 2.25 ... indexed
   by the same slot field as the quarter-step one. */
// GLOBAL: EZ2AC_UNPACKED 0x00490228
float g_panelRateLadder[21];   /* 21: 1, 1.5, 2, quarter steps to 6, 10, 99 */

/* Panel::m42d190 @0x42d190 - the same record as m42d080 and the same ten
   fields, but it prints the mode's RATE in both arms; only the format string
   differs, the record's own below ten and "%2.1f" above it. */
// FUNCTION: EZ2AC_UNPACKED 0x0042D190
void Panel::m42d190()
{
    int x;

    if (f1d30 == 0)
    {
        return;
    }

    FloatToInt(&x, f1d40 + fe4);
    ((KGraphics *)g_sub1b2f054)->setBlend(f1dc0, f1dc4);

    if (10.0f > g_panelSpeedLadder[slot->f4e0])
    {
        ((GfxText *)g_sub1b2f054)->m40aaa0((float)x, f1d44, f1d48, f1d4c,
                                           f1d50, f1d50 * 0.4f,
                                           g_panelRateLadder[slot->f4e0],
                                           f1d80, -1, f1d54);
    }
    else if (g_panelSpeedLadder[slot->f4e0] >= 10.0f)
    {
        ((GfxText *)g_sub1b2f054)->m40aaa0((float)x, f1d44, f1d48, f1d4c,
                                           f1d50, f1d50 * 0.4f,
                                           g_panelRateLadder[slot->f4e0],
                                           "%2.1f", -1, f1d54);
    }
}

/* The fade alpha the panels' bomb effects use: how opaque the effect still is
   `elapsed` into a run of `period`. The ramp reaches full transparency at the
   half-way point - `48 / period * elapsed` measured against `period / 2` - and
   the result is inverted, so it counts DOWN from 255. Both callers shift it
   into the alpha byte of an otherwise white colour.

   Panel's copy is __cdecl and CatchPanel's is __stdcall, which is why the
   original's linker did not fold two byte-identical bodies together. */
// FUNCTION: EZ2AC_UNPACKED 0x00427050
int fadeAlpha(int elapsed, int period)
{
    int t = (int)((48.0f / (float)period) * (float)elapsed);
    int half = period / 2;
    int v;

    if (t >= half)
    {
        v = 255;
    }
    else
    {
        v = t * 255 / half;
    }

    return 255 - v;
}

/* Its catch twin @0x0046f310, byte for byte the same but __stdcall. */
// FUNCTION: EZ2AC_UNPACKED 0x0046f310
int __stdcall catchFadeAlphaBiased(int elapsed, int period)
{
    int t = (int)((48.0f / (float)period) * (float)elapsed) - period / 8;
    int half = period / 2;

    if (t >= half)
        return 255;
    if (t < 0)
        return 0;

    return t * 255 / half;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046f370
int __stdcall catchFadeAlpha(int elapsed, int period)
{
    int t = (int)((48.0f / (float)period) * (float)elapsed);
    int half = period / 2;
    int v;

    if (t >= half)
    {
        v = 255;
    }
    else
    {
        v = t * 255 / half;
    }

    return 255 - v;
}

/* CatchPanel::init @0x471710 - attach to a slot, then ask the slot's timing
   object for the run's period. Slot 8 fills four words and only the third is
   kept, which is what the frame's size and the read's offset say. */
// FUNCTION: EZ2AC_UNPACKED 0x00471710
void CatchPanel::init(PanelSlot *s)
{
    int buf[8];

    slot = s;
    resetLanes();
    s->f364->q8(buf, 4);
    f254 = buf[2];
}

extern EzObjectInfo g_info_4a9130, g_info_4aec00;  /* 0x4a9130, 0x4aec00 */

/* Panel's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0042d560
Panel *createPanel()
{
    g_objectRecord = &g_info_4a9130;
    return new Panel;
}

/* CatchPanel's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x00472490
CatchPanel *createCatchPanel()
{
    g_objectRecord = &g_info_4aec00;
    return new CatchPanel;
}

/* Give every lane of the run its scroll rate: the owner's base rate, times a
   per-lane factor between 0.5 and 2.5 when the slot asks for one. The slot is
   re-read at each use because the stores in between could have written it. */
// FUNCTION: EZ2AC_UNPACKED 0x00427680
void Panel::resetLanes()
{
    TrackList *t = &slot->f3d0->tracks[slot->index];
    int i;

    i = 0;
    if (i < t->count)
    {
        const TrackEntry *e = t->items;
        int *out = f3248;

        do
        {
            items[i].fec = e->handle;
            items[i].fe8 = slot->f3d0->f320;

            if (slot->f4a4 != 0)
                items[i].fe8 = ((float)(rand() % 1000) * 0.002f + 0.5f) * items[i].fe8;

            *out = *(int *)&items[i].fe8;
            out++;
            i++;
            e++;
        }
        while (i < t->count);
    }

    f3290 = 1.0f;
}

/* The scratch panel's copy, over its own members. */
// FUNCTION: EZ2AC_UNPACKED 0x004609f0
void GFPanel::resetLanes()
{
    TrackList *t = &((GFSlotOwner *)slot->f1e0)->tracks[slot->index];
    int i;

    i = 0;
    if (i < t->count)
    {
        const TrackEntry *e = t->items;
        int *out = f3048;

        do
        {
            items[i].fec = e->handle;
            items[i].fe8 = ((GFSlotOwner *)slot->f1e0)->f314;

            if (slot->f2b4 != 0)
                items[i].fe8 = ((float)(rand() % 1000) * 0.002f + 0.5f) * items[i].fe8;

            *out = *(int *)&items[i].fe8;
            out++;
            i++;
            e++;
        }
        while (i < t->count);
    }

    f3090 = 1.0f;
}

/* And the catch panel's, over its own: the owner through slot->f360, the
   0x1bc-stride CatchTrackList, and the randomize gate at slot->f438 where
   Panel reads f4a4. Found by scratchpad/twins.py at 81% against
   Panel::resetLanes - the stride is what confirms the list type. */
// FUNCTION: EZ2AC_UNPACKED 0x0046f590
void CatchPanel::resetLanes()
{
    CatchTrackList *t = &((CatchSlotOwner *)slot->f360)->tracks[slot->index];
    int i;

    i = 0;
    if (i < t->count)
    {
        int *out = (int *)f1c7c;
        const TrackEntry *e = t->items;
        GdsItem *it = items;

        do
        {
            it->fec = e->key;
            it->fe8 = ((CatchSlotOwner *)slot->f360)->f32c;

            if (slot->f438 != 0)
                it->fe8 = ((float)(rand() % 1000) * 0.002f + 0.5f) * it->fe8;

            *out = *(int *)&it->fe8;
            i++;
            out++;
            it++;
            e++;
        }
        while (i < t->count);
    }

    f1cc4 = 1.0f;
}

/* Re-attach the scratch panel's three chart heads, on mode 2 and only when
   the gate at +0x1b4c is set. The heads go back in the order 0, 2, 1 - which
   is the source's order, not the fields'. The first argument is unused. */
// FUNCTION: EZ2AC_UNPACKED 0x004627c0
void GFPanel::m4627c0(int, int mode)
{
    if (mode == 2 && f1b4c != 0)
    {
        f1be0->attach(f1b50, 0);
        f1be8->attach(f1b58, 0);
        f1be4->attach(f1b54, 0);
    }
}

/* Expire the forty per-note timers: any entry that is not -1 and whose age
   has passed eight periods is cleared and reported to the slot. Gated on the
   slot's f4f4. `now` is this+0x28 and the period this+0x2c; the eight comes
   out as three doublings. */
// FUNCTION: EZ2AC_UNPACKED 0x004298c0
void Panel::m4298c0()
{
    if (slot->f4f4 != 0)
    {
        int  i;
        int *p;

        for (i = 0, p = f3c; i < 40; i++)
        {
            if (*p != -1 && f28 - (unsigned int)*p >= f2c * 8)
            {
                *p = -1;
                ((PlayerSlot *)slot)->m42ebf0(i, 0);
            }
            p++;
        }
    }
}

/* The scratch panel's copy: its own timers at +0x38, its clock at +0x24, and
   the gate four hundred bytes earlier in the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x004626d0
void GFPanel::m4626d0()
{
    if (slot->f304 != 0)
    {
        int  i;
        int *p;

        for (i = 0, p = f38; i < 40; i++)
        {
            if (*p != -1 && f24 - (unsigned int)*p >= f28 * 8)
            {
                *p = -1;
                ((PlayerRecord *)slot)->m4662d0(i, 0);
            }
            p++;
        }
    }
}

/* Stamp one lane's timer, and re-lane on the scratch. Lane 6 always rebuilds
   the lane map first - written out in BOTH arms, which is what gives the
   original its two call sites rather than one shared tail. Only the "on"
   call stamps the timer with the clock at +0x250; the timers are the same
   f258 array CatchPanel's expiry walks.

   It lives in this file rather than beside CatchPanel's other members
   because resetLanes is defined here: cl only keeps `this` in ecx across a
   call whose body it can see, which is what the first arm's bare `call`
   needs. */
// FUNCTION: EZ2AC_UNPACKED 0x00471290
void CatchPanel::m471290(int lane, int on)
{
    if (on != 0)
    {
        if (lane == 6)
            resetLanes();

        f258[lane] = f250;
    }
    else if (lane == 6)
    {
        resetLanes();
    }
}

/* The keys and scratch copies of CatchPanel::m471290 - stamp one lane's timer,
   and rebuild the lane map on lane 6, with the rebuild written out in both
   arms so the original's two call sites survive. Each takes its own clock and
   its own timer array, and each has to live in this file because its
   resetLanes is defined here: the first arm's bare `call` only keeps `this`
   in ecx for a callee cl can see. */
// FUNCTION: EZ2AC_UNPACKED 0x00429910
void Panel::m429910(int lane, int on)
{
    if (on != 0)
    {
        if (lane == 6)
            resetLanes();

        f3c[lane] = f28;
    }
    else if (lane == 6)
    {
        resetLanes();
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00462720
void GFPanel::m462720(int lane, int on)
{
    if (on != 0)
    {
        if (lane == 6)
            resetLanes();

        f38[lane] = f24;
    }
    else if (lane == 6)
    {
        resetLanes();
    }
}

/* Panel::m4270a0 @0x4270a0 - the fade colour for the current window: always
   white, with the alpha computed one of three ways depending on which of the
   slot's three mode flags is set, and 0xffffffff when none is.

   Mode 474 is fadeAlpha @0x427050's calculation NOT inverted, and biased down
   by an eighth of the period. Mode 47c is fadeAlpha itself. Mode 484 works on
   the position within the period rather than the elapsed time, doubles it,
   and counts down from 255 - and its arithmetic is UNSIGNED throughout, where
   the other two are signed.

   The colour goes through an inlined helper, and that is load-bearing. The
   two constant arms emit `mov eax,0xff; shl eax,0x18; or eax,0xffffff` rather
   than the folded 0xffffffff, so the alpha reaches the shift as a PARAMETER -
   the rule in CLAUDE.md about a constant passed through an inlined helper not
   being folded. Written as `(0xff << 24) | 0xffffff` inline, cl folds both
   arms to a literal and the function stops at 88.20%. */
static unsigned int whiteWithAlpha(int alpha)
{
    return (alpha << 24) | 0xffffff;
}

// FUNCTION: EZ2AC_UNPACKED 0x004270a0
unsigned int Panel::m4270a0(unsigned int from, unsigned int to)
{
    if (slot->f474 != 0)
    {
        int period = (int)f2c;
        int t = (int)((48.0f / (float)period) * (float)(int)(to - from))
                - period / 8;
        int half = period / 2;

        int alpha;

        if (t >= half)
            alpha = 0xff;
        else if (t < 0)
            alpha = 0;
        else
            alpha = t * 255 / half;

        return whiteWithAlpha(alpha);
    }

    if (slot->f47c != 0)
        return whiteWithAlpha(fadeAlpha(to - from, f2c));

    if (slot->f484 != 0)
    {
        unsigned int period = f2c;
        unsigned int pos = from % period;
        int v = (int)(pos * 255 * 2 / period) - 1;
        int alpha;

        v = (v < 0) ? 0 : v;
        alpha = 255 - v;
        if (alpha < 0)
            alpha = 0;
        return whiteWithAlpha(alpha);
    }

    return 0xffffffff;
}

/* GFPanel::m4607c0 @0x4607c0 - Panel::m4270a0's twin, 85 of 85 instructions
   identical after masking, but with TWO edits rather than the one the sameness
   suggests. The period is this class's +0x28 rather than Panel's +0x2c - and
   the three fade flags sit at +0x284, +0x28c and +0x294 on the slot, a uniform
   0x1f0 BELOW where PanelSlot keeps them.

   That means GFPanel's slot is not a PanelSlot however it is declared. Rather
   than retype a member other matched functions read, the flags are reached
   through a view here and the discrepancy is left visible. */
struct GFFadeFlags
{
    char _pad0[0x284];
    int  f284;                 /* 0x284 */
    char _pad288[0x28c - 0x288];
    int  f28c;                 /* 0x28c */
    char _pad290[0x294 - 0x290];
    int  f294;                 /* 0x294 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004607c0
unsigned int GFPanel::m4607c0(unsigned int from, unsigned int to)
{
    if (((GFFadeFlags *)slot)->f284 != 0)
    {
        int period = (int)f28;
        int t = (int)((48.0f / (float)period) * (float)(int)(to - from))
                - period / 8;
        int half = period / 2;
        int alpha;

        if (t >= half)
            alpha = 0xff;
        else if (t < 0)
            alpha = 0;
        else
            alpha = t * 255 / half;

        return whiteWithAlpha(alpha);
    }

    if (((GFFadeFlags *)slot)->f28c != 0)
        return whiteWithAlpha(fadeAlpha(to - from, f28));

    if (((GFFadeFlags *)slot)->f294 != 0)
    {
        unsigned int period = f28;
        unsigned int pos = from % period;
        int v = (int)(pos * 255 * 2 / period) - 1;
        int alpha;

        v = (v < 0) ? 0 : v;
        alpha = 255 - v;
        if (alpha < 0)
            alpha = 0;
        return whiteWithAlpha(alpha);
    }

    return 0xffffffff;
}

/* The song select's BLACK option (set by update2's 0x19 option cycle when
   g_37dced0 reaches 1, 0x446f32; cleared at 0x44ebe4): when on, the catch
   panel's update draws its opaque sheet over the play area first. Not a
   debug flag, as an earlier comment here had it. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e5a8
int g_catchOverlay;

/* CatchPanel::update @0x4724f0 - the per-frame draw: stamp the clock, the
   debug sheet, the ten draw helpers in order, the two chart tweens placed
   off the panel origin, five more helpers, and the optional chart at 0x1c60. */
// FUNCTION: EZ2AC_UNPACKED 0x004724f0
void CatchPanel::update()
{
    f250 = slot->clock->now();
    if (g_catchOverlay != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(tex, 98.0f, -5.0f, 444.0f, 432.0f, 0x99ffffff);
    }
    m46f8e0();
    m471b80();
    m46fa50();
    m470070();
    m471ae0();
    m46fb40();
    m470f80();
    m471170();
    m470c50();
    m470d50();
    if (f1544 != 0)
    {
        float ox = f18 + f14 - 280.0f;
        float oy = f154c;
        Obj402860 *c = f1568;
        c->f214 = ox;
        c->f218 = oy;
        f1568->m403940();
    }
    {
        float ox = f18 + f14 - 10.0f;
        float oy = f1950 - 10.0f;
        Obj402860 *c = &m_28;
        c->f214 = ox;
        c->f218 = oy;
        c->m403940();
    }
    m471c20();
    m472020();
    m471830();
    m470370();
    m4707c0();
    if (f1c50 != 0)
        f1c60->m403940();
}

/* CatchPanel::m46fa50 @0x46fa50 - the item bars: each enabled item whose
   fade state is running grows (state 1, clamped at f5c) or shrinks (state 2,
   stopping at 0 and going idle) by its step, and is drawn as a bar rising from
   the bottom of its rectangle. Same guarded do-while / anchored-pointer shape
   as Panel::m42cf40 above, anchored where the original does, at +0x5c. */
enum
{
    kIbEnabled = -0x5c / 4,   /* f00 */
    kIbX       = -0x58 / 4,   /* f04 */
    kIbY       = -0x54 / 4,   /* f08 */
    kIbW       = -0x50 / 4,   /* f0c */
    kIbH       = -0x4c / 4,   /* f10 */
    kIbColor   = -8 / 4,      /* f54 */
    kIbTex     = -4 / 4,      /* f58 */
    kIbMax     = 0,           /* f5c */
    kIbUp      = 1,           /* f60 */
    kIbDown    = 2,           /* f64 */
    kIbLevel   = 0x98 / 4,    /* ff4 */
    kIbState   = 0x9c / 4     /* ff8 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0046fa50
void CatchPanel::m46fa50()
{
    int i;

    i = 0;
    if (i < f2f8)
    {
        int *p = (int *)&items[0].f5c;

    do
    {
        if (p[kIbEnabled] != 0)
        {
            int state = GL_I(p, kIbState);
            if (state == 1)
            {
                float v = GL_F(p, kIbUp) + GL_F(p, kIbLevel);
                GL_F(p, kIbLevel) = v;
                if (v > GL_F(p, kIbMax))
                    GL_F(p, kIbLevel) = GL_F(p, kIbMax);
            }
            else if (state == 2)
            {
                float v = GL_F(p, kIbLevel) - GL_F(p, kIbDown);
                GL_F(p, kIbLevel) = v;
                if (v < 0.0f)
                {
                    GL_F(p, kIbLevel) = 0.0f;
                    GL_I(p, kIbState) = 0;
                }
            }
            else
                goto next;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kIbTex), GL_F(p, kIbX),
                                                 GL_F(p, kIbY) + GL_F(p, kIbH) - GL_F(p, kIbLevel),
                                                 GL_F(p, kIbW), GL_F(p, kIbLevel),
                                                 GL_I(p, kIbColor));
        }
next:
        i++;
        p += sizeof(GdsItem) / 4;
    } while (i < f2f8);
    }
}

/* CatchPanel::m46fb40 @0x46fb40 - the item glows: each enabled item sets its
   own blend, and the ones in fade state 1 draw their first texture at its
   position with a zero size (the texture's own). Anchored at +0x4c. */
enum
{
    kIgEnabled = -0x4c / 4,   /* f00 */
    kIgX       = -0x10 / 4,   /* f3c */
    kIgY       = -0x0c / 4,   /* f40 */
    kIgTex     = -0x08 / 4,   /* f44 */
    kIgColor   = -0x04 / 4,   /* f48 */
    kIgSrc     = 0,           /* f4c */
    kIgDst     = 1,           /* f50 */
    kIgState   = 0xac / 4     /* ff8 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0046fb40
void CatchPanel::m46fb40()
{
    int i;

    i = 0;
    if (i < f2f8)
    {
        int *p = (int *)&items[0].f4c;

    do
    {
        if (p[kIgEnabled] != 0)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(GL_I(p, kIgSrc), GL_I(p, kIgDst));
            if (GL_I(p, kIgState) == 1)
                ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kIgTex), GL_F(p, kIgX),
                                                     GL_F(p, kIgY), 0.0f, 0.0f,
                                                     GL_I(p, kIgColor));
        }
        i++;
        p += sizeof(GdsItem) / 4;
    } while (i < f2f8);
    }
}

/* CatchPanel::m4707c0 @0x4707c0 - the LongNoteBomb: for every item flagged
   at ffc, draw the bomb's current frame centred on the item's x at the panel
   origin's y, with the record's size or the texture's own where a size is 0;
   the per-lane counter advances the frame every f04 ticks, four frames round.
   Two walking pointers: the item (anchored at +0xc) and the lane's frame
   index, whose counter sits eighteen ints above it. */
enum
{
    kLbX    = -0x08 / 4,   /* f04 */
    kLbW    = 0,           /* f0c */
    kLbLit  = 0xf0 / 4     /* ffc */
};

// FUNCTION: EZ2AC_UNPACKED 0x004707c0
void CatchPanel::m4707c0()
{
    int i;

    if (s17bc.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < f2f8)
        {
            int *p = (int *)&items[0].f0c;
            int *frame = &s17bc.f98[0];

        do
        {
            if (GL_I(p, kLbLit) != 0)
            {
                float w;
                float h;
                if (s17bc.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s17bc.f18[*frame]);
                else
                    w = s17bc.f10;
                if (s17bc.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s17bc.f18[*frame]);
                else
                    h = s17bc.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s17bc.f18[*frame],
                                                     GL_F(p, kLbX) + GL_F(p, kLbW) * 0.5f - w * 0.5f,
                                                     f1950 + s17bc.f0c, w, h, -1);
                {
                    int period = s17bc.f04;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= 4)
                            *frame = 0;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < f2f8);
        }
    }
}

/* CatchPanel::m46f8e0 @0x46f8e0 - the lane frame and the guide lines, the
   catch dialect of Panel::m42a240 / GFPanel::m462e00: reset the raster
   stages; every item blends its own rect in its colour pair; then 5/6-blended,
   each item contributes up to two guide lines, one per armed side, placed off
   its base by the side's inset (or the width when the side flag is up), each a
   m40ac80 quad between the x and its span - without the per-lane offset or
   the rounding the other two panels apply. */
// FUNCTION: EZ2AC_UNPACKED 0x0046f8e0
void CatchPanel::m46f8e0()
{
    int i;

    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    i = 0;
    if (i < f2f8)
    {
        char *q = (char *)&items[0].f14;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x14);

            ((KGraphics *)g_sub1b2f054)->setBlend(e->f14, e->f18);
            ((KGraphics *)g_sub1b2f054)->m40a5a0(e->f04, e->f08, e->f0c, e->f10,
                                                 e->f1c, e->f20);
            i++;
            q += 0x104;
        }
        while (i < f2f8);
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    i = 0;
    if (i < f2f8)
    {
        char *q = (char *)&items[0].f28;

        do
        {
            GdsItem *e = (GdsItem *)(q - 0x28);

            if (e->f28 != 0.0f)
            {
                float d;

                if (e->f24 == 0)
                    d = e->f04 - e->f28;
                else
                    d = e->f04 + e->f0c;
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    d, e->f08, d, e->f10 + e->f08,
                    e->f28, *(float *)&e->f2c);
            }
            if (e->f34 != 0.0f)
            {
                float d;

                if (e->f30 == 0)
                    d = e->f04 - e->f34;
                else
                    d = e->f04 + e->f0c;
                ((KGraphics *)g_sub1b2f054)->m40ac80(
                    d, e->f08, d, e->f10 + e->f08,
                    e->f34, *(float *)&e->f38);
            }
            i++;
            q += 0x104;
        }
        while (i < f2f8);
    }
}

/* CatchPanel::m470f80 @0x470f80 - the GrooveLight records, Panel::m42cf40's
   twin without the per-lane x offset: additive at full brightness, then the
   second texture over it with a grey that fades across the counter's period. */
// FUNCTION: EZ2AC_UNPACKED 0x00470F80
void CatchPanel::m470f80()
{
    int i;
    int step = fadeStep(f250, f254);

    i = 0;
    if (i < f1b54)
    {
        const int *p = (const int *)&s1b58[0].f0c;

    do
    {
        if (p[kGlEnabled + 1] != 0)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex1 + 1), GL_F(p, kGlX1 + 1),
                                                 GL_F(p, kGlY1 + 1),
                                                 GL_F(p, kGlW1 + 1),
                                                 GL_F(p, kGlH1 + 1), -1);

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(GL_T(p, kGlTex2 + 1), GL_F(p, kGlX2 + 1),
                                                 GL_F(p, kGlY2 + 1),
                                                 GL_F(p, kGlW2 + 1),
                                                 GL_F(p, kGlH2 + 1),
                                                 ((-1 - step) << 24) |
                                                 ((255 - step) << 16) |
                                                 ((255 - step) << 8) |
                                                 (255 - step));
        }

        i++;
        p += sizeof(GdsGrooveLight) / 4;
    } while (i < f1b54);
    }
}

/* CatchPanel::m471830 @0x471830 - the CoolCombo readout. Below 1, or with
   the gate at f248 down, the chart is detached; otherwise the combo is spelt
   out with _itoa, the digits turned into texture indices in place, and - only
   when the combo changed - the chart for its digit count is attached and
   its cells 2.. filled with the digit textures, least significant first. The
   chart then tweens at the panel origin minus 100, and the combo is
   remembered. */
// FUNCTION: EZ2AC_UNPACKED 0x00471830
void CatchPanel::m471830()
{
    char buf[128];
    int  v;
    int  len;
    int  i;

    if (f18e4 == 0)
        return;
    v = slot->f1dc;
    if (v < 1 || f248 == 0)
    {
        f192c->attach(0, 0);
        return;
    }
    _itoa(v, buf, 10);
    len = strlen(buf);
    for (i = 0; i < len; i++)
        buf[i] -= '0';

    v = slot->f1dc;
    if (v < 10)
    {
        if (f1928 != v)
        {
            f192c->attach(f1910, 0);
            f192c->setCell(2, 0, (int)f18e8[buf[0]]);
        }
    }
    else if (v < 100)
    {
        if (f1928 != v)
        {
            f192c->attach(f1914, 0);
            f192c->setCell(2, 0, (int)f18e8[buf[1]]);
            f192c->setCell(3, 0, (int)f18e8[buf[0]]);
        }
    }
    else if (v < 1000)
    {
        if (f1928 != v)
        {
            f192c->attach(f1918, 0);
            f192c->setCell(2, 0, (int)f18e8[buf[2]]);
            f192c->setCell(3, 0, (int)f18e8[buf[1]]);
            f192c->setCell(4, 0, (int)f18e8[buf[0]]);
        }
    }
    else if (v < 10000)
    {
        if (f1928 != v)
        {
            f192c->attach(f191c, 0);
            f192c->setCell(2, 0, (int)f18e8[buf[3]]);
            f192c->setCell(3, 0, (int)f18e8[buf[2]]);
            f192c->setCell(4, 0, (int)f18e8[buf[1]]);
            f192c->setCell(5, 0, (int)f18e8[buf[0]]);
        }
    }
    {
        float ox = f18 + f14 - 100.0f;
        float oy = f1924;
        Obj402860 *c = f192c;
        c->f214 = ox;
        c->f218 = oy;
        f192c->m403940();
    }
    f1928 = slot->f1dc;
}

/* CatchPanel::m470370 @0x470370 - the CoolBomb, m4707c0's twin on the s156c
   record: a lane whose frame index is not -1 draws the bomb's frame centred
   on the item's x at the panel origin's y, and its counter advances the frame
   every f04 ticks until the run's f08 frames are spent, when it goes idle
   (-1). Anchored at +0x4. */
enum
{
    kCbX    = 0,           /* f04 */
    kCbW    = 0x08 / 4     /* f0c */
};

// FUNCTION: EZ2AC_UNPACKED 0x00470370
void CatchPanel::m470370()
{
    int i;

    if (s156c.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < f2f8)
        {
            int *p = (int *)&items[0].f04;
            int *frame = &s156c.f98[0];

        do
        {
            if (*frame != -1)
            {
                float w;
                float h;
                if (s156c.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s156c.f18[*frame]);
                else
                    w = s156c.f10;
                if (s156c.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s156c.f18[*frame]);
                else
                    h = s156c.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s156c.f18[*frame],
                                                     GL_F(p, kCbX) + GL_F(p, kCbW) * 0.5f - w * 0.5f,
                                                     f1950 + s156c.f0c, w, h, -1);
                {
                    int period = s156c.f04;
                    int count = s156c.f08;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= count)
                            *frame = -1;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < f2f8);
        }
    }
}

/* The keys panel's sway: the phase its update advances from the song clock
   over the period at f2c, and the flag that publishes the swayed x as the
   global draw origin. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcdb8
float g_panelSwayPhase;
// GLOBAL: EZ2AC_UNPACKED 0x037da9d0
int g_panelSwayPublish;
extern float g_drawOriginX;                      /* 0x37da9d4 (src/effectordraw.cpp) */
/* The scratch panel's own copies of the three. */
// GLOBAL: EZ2AC_UNPACKED 0x037df220
float g_gfPanelSwayPhase;
// GLOBAL: EZ2AC_UNPACKED 0x037dd490
int g_gfPanelSwayPublish;
// GLOBAL: EZ2AC_UNPACKED 0x037dd494
float g_gfDrawOriginX;
/* The song clock, by its slot 22 - the same view wip/smallleaves.cpp's
   swaying readouts use. */
struct PanelClock22
{
    virtual void c00(); virtual void c01(); virtual void c02();
    virtual void c03(); virtual void c04(); virtual void c05();
    virtual void c06(); virtual void c07(); virtual void c08();
    virtual void c09(); virtual void c10(); virtual void c11();
    virtual void c12(); virtual void c13(); virtual void c14();
    virtual void c15(); virtual void c16(); virtual void c17();
    virtual void c18(); virtual void c19(); virtual void c20();
    virtual void c21();
    virtual unsigned int now();   /* slot 22 -> vtable+0x58 */
};
/* Set once the keys panel has drawn a frame. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f090
int g_panelDrawn;

/* Panel::update @0x42d5c0 - the per-frame draw: stamp the song clock, sway
   the x origin (a cosine of the phase, published globally when asked),
   advance the phase, then the draw helpers in order - with the style-7
   readout placed before the lane lights in that style and after them
   otherwise, and the CV2 flag picking between two beat draws. */
// FUNCTION: EZ2AC_UNPACKED 0x0042d5c0
void Panel::update()
{
    float x;
    unsigned int t;
    double a;

    /* The phase is read BEFORE the stamp is stored: the original's `fld;
       fadd` sit above the store, which a plain `f28 = now();` first statement
       cannot give. */
    t = ((PanelClock22 *)slot->f3d8)->now();
    a = g_panelSwayPhase + 3.1415927f;
    f28 = t;
    x = (float)(cos(a) * fe0 + fdc);
    fe4 = x;
    if (g_panelSwayPublish != 0)
        g_drawOriginX = x;
    g_panelSwayPhase = (float)f28 * 0.7853982f / (int)f2c;
    m42a240();
    m42a450();
    m42b550();
    m427d90();
    m42bce0();
    m42b470();
    m427490();
    if (g_mixStyle == 7)
        m42c5f0();
    m42bbe0();
    m42d290();
    m42a580();
    m42cf40();
    if (g_1b2eb6c == 0)
        m42d080();
    else
        m42d190();
    m42bd80();
    m42bf30();
    m42c0f0();
    if (g_mixStyle != 7)
        m42c5f0();
    m429170();
    m4280a0();
    m42b910();
    m4282b0();
    m4285c0();
    m4288d0();
    g_panelDrawn = 1;
}

/* GFPanel::update @0x465380 - the scratch panel's, on its own clock (the
   slot's +0x1e8), its own sway globals and its own helper list; the optional
   chart at 0x13fc tweens mid-list and the one at 0x32b8 tail-calls when f10
   is set. */
// FUNCTION: EZ2AC_UNPACKED 0x00465380
void GFPanel::update()
{
    float x;
    unsigned int t;
    double a;

    t = ((PanelClock22 *)slot->f1e8)->now();
    a = g_gfPanelSwayPhase + 3.1415927f;
    f24 = t;
    x = (float)(cos(a) * fdc + fd8);
    fe0 = x;
    if (g_gfPanelSwayPublish != 0)
        g_gfDrawOriginX = x;
    g_gfPanelSwayPhase = (float)f24 * 0.7853982f / (int)f28;
    m462e00();
    m462450();
    m4641c0();
    m463030();
    m463d30();
    m463cc0();
    m4640d0();
    m4650c0();
    m463160();
    m464ec0();
    m465000();
    m464250();
    m464300();
    m4643b0();
    m4644c0();
    m464580();
    m4649c0();
    if (f13d8 != 0)
        f13fc->m403940();
    m461990();
    m4611c0();
    m4614d0();
    m4617e0();
    if (f10 != 0)
        m_32b8.m403940();
}

extern int g_1b2e7a8;   /* 0x1b2e7a8 (src/modeselectctor.cpp) */

/* Panel::m42b910 @0x42b910 - the keys panel's CoolCombo readout, the twin of
   CatchPanel::m471830 with one difference: the chart's position is only set
   (to the swayed x and the record's y, with no arithmetic) when the combo
   changed AND the global at 0x1b2e7a8 is up; an unchanged combo, or five
   digits and more, go straight to the tween. */
// FUNCTION: EZ2AC_UNPACKED 0x0042b910
void Panel::m42b910()
{
    char buf[128];
    int  v;
    int  len;
    int  i;

    if (f17d8 == 0)
        return;
    v = slot->f1dc;
    if (v < 1 || f20 == 0)
    {
        f1820->attach(0, 0);
        return;
    }
    _itoa(v, buf, 10);
    len = strlen(buf);
    for (i = 0; i < len; i++)
        buf[i] -= '0';

    v = slot->f1dc;
    if (v < 10)
    {
        if (f181c == v)
            goto tween;
        f1820->attach(f1804, 0);
        f1820->setCell(2, 0, (int)f17dc[buf[0]]);
        if (g_1b2e7a8 != 0)
        {
            float oy = f1818;
            Obj402860 *c = f1820;
            c->f214 = fe4;
            c->f218 = oy;
        }
    }
    else if (v < 100)
    {
        if (f181c == v)
            goto tween;
        f1820->attach(f1808, 0);
        f1820->setCell(2, 0, (int)f17dc[buf[1]]);
        f1820->setCell(3, 0, (int)f17dc[buf[0]]);
        if (g_1b2e7a8 != 0)
        {
            float oy = f1818;
            Obj402860 *c = f1820;
            c->f214 = fe4;
            c->f218 = oy;
        }
    }
    else if (v < 1000)
    {
        if (f181c == v)
            goto tween;
        f1820->attach(f180c, 0);
        f1820->setCell(2, 0, (int)f17dc[buf[2]]);
        f1820->setCell(3, 0, (int)f17dc[buf[1]]);
        f1820->setCell(4, 0, (int)f17dc[buf[0]]);
        if (g_1b2e7a8 != 0)
        {
            float oy = f1818;
            Obj402860 *c = f1820;
            c->f214 = fe4;
            c->f218 = oy;
        }
    }
    else if (v < 10000)
    {
        if (f181c == v)
            goto tween;
        f1820->attach(f1810, 0);
        f1820->setCell(2, 0, (int)f17dc[buf[3]]);
        f1820->setCell(3, 0, (int)f17dc[buf[2]]);
        f1820->setCell(4, 0, (int)f17dc[buf[1]]);
        f1820->setCell(5, 0, (int)f17dc[buf[0]]);
        if (g_1b2e7a8 != 0)
        {
            float oy = f1818;
            Obj402860 *c = f1820;
            c->f214 = fe4;
            c->f218 = oy;
        }
    }
tween:
    f1820->m403940();
    f181c = slot->f1dc;
}

/* GFPanel::m463cc0 @0x463cc0 - the item pools: the cycling frame and the
   pool's start time (the counter less sixteen periods, floored at 0), then
   m463b40 on every item. */
// FUNCTION: EZ2AC_UNPACKED 0x00463cc0
void GFPanel::m463cc0()
{
    int i;
    int frame = cycleFrame(f24, f28);
    unsigned int start;

    if (f24 > f28 * 16)
        start = f24 - f28 * 16;
    else
        start = 0;
    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    for (i = 0; i < ff8; i++)
        m463b40(i, start, frame);
}

/* The judge's per-lane kind, by the record array's first word (src/accessors.cpp). */
struct Obj426CA0
{
    int  getF0(int i);          /* 0x426d10 */
};
extern int g_1b5f2e0;           /* 0x1b5f2e0 (src/holdtick.cpp) */

/* Panel::m42b470 @0x42b470 - the keys panel's item pools (m463cc0's twin,
   with a forty-eight period start) and, per lane, the hold glow m427c50 when
   the judge reports the lane as a hold (kind 1), the global hold gate is
   down and the panel has the glow textures. */
// FUNCTION: EZ2AC_UNPACKED 0x0042b470
void Panel::m42b470()
{
    int i;
    int frame = cycleFrame(f28, f2c);
    unsigned int start;

    if (f28 > f2c * 48)
        start = f28 - f2c * 48;
    else
        start = 0;
    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    for (i = 0; i < f12c; i++)
    {
        m42b2f0(i, start, frame);
        if (((Obj426CA0 *)slot->f3dc)->getF0(slot->f3d0->tracks[slot->index].items[i].handle) == 1 &&
            g_1b5f2e0 == 0 && f1840 != 0)
            m427c50(i);
    }
}

/* GFPanel::m462450 @0x462450 - the s3014 marker: at one of its two positions
   (f3010 picks), offset by the swayed x, the second texture when the life
   has topped out (state == g_elemCount + 1) and the first otherwise. */
// FUNCTION: EZ2AC_UNPACKED 0x00462450
void GFPanel::m462450()
{
    float x;
    float y;

    if (s3014.f00 != 0)
    {
        if (f3010 == 0)
        {
            x = s3014.f04;
            y = s3014.f08;
        }
        else
        {
            x = s3014.f0c;
            y = s3014.f10;
        }
        if (((GFStageView *)slot)->life->state == g_elemCount + 1)
            ((KGraphics *)g_sub1b2f054)->drawTex(s3014.f18, fe0 + x, y, 0.0f, 0.0f, -1);
        else
            ((KGraphics *)g_sub1b2f054)->drawTex(s3014.f14, fe0 + x, y, 0.0f, 0.0f, -1);
    }
}

/* GFPanel::m4617e0 @0x4617e0 - the LongNoteBomb, CatchPanel::m4707c0's twin
   on the s1650 record: the lanes with both ffc and f100 up draw the bomb's
   current frame centred on the item's x (plus the swayed offset) at the
   item's bottom edge, placed off the panel's y span; the per-lane counter
   advances the frame every f04 ticks, four frames round. Anchored at +0x100. */
enum
{
    kGbX    = -0xfc / 4,   /* f04 */
    kGbY    = -0xf8 / 4,   /* f08 */
    kGbW    = -0xf4 / 4,   /* f0c */
    kGbH    = -0xf0 / 4,   /* f10 */
    kGbLit  = -0x04 / 4,   /* ffc */
    kGbLit2 = 0            /* f100 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004617e0
void GFPanel::m4617e0()
{
    int i;

    if (s1650.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < ff8)
        {
            int *frame = &s1650.f98[0];
            int *p = (int *)&items[0].f100;

        do
        {
            if (GL_I(p, kGbLit) != 0 && GL_I(p, kGbLit2) != 0)
            {
                float w;
                float h;
                if (s1650.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1650.f18[*frame]);
                else
                    w = s1650.f10;
                if (s1650.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1650.f18[*frame]);
                else
                    h = s1650.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s1650.f18[*frame],
                                                     GL_F(p, kGbX) + GL_F(p, kGbW) * 0.5f - w * 0.5f + fe0,
                                                     (GL_F(p, kGbY) + GL_F(p, kGbH)) + (f17e4 - f1874) + s1650.f0c,
                                                     w, h, -1);
                {
                    int period = s1650.f04;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= 4)
                            *frame = 0;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < ff8);
        }
    }
}

/* GFPanel::m461990 @0x461990 - the scratch panel's CoolCombo readout: the
   CatchPanel/Panel shape on the slot's +0x28 combo, gated on f1c, with no
   position store at all - the chart tweens where it was attached. */
// FUNCTION: EZ2AC_UNPACKED 0x00461990
void GFPanel::m461990()
{
    char buf[128];
    int  v;
    int  len;
    int  i;

    if (f1778 == 0)
        return;
    v = slot->f28;
    if (v < 1 || f1c == 0)
    {
        f17c0->attach(0, 0);
        return;
    }
    _itoa(v, buf, 10);
    len = strlen(buf);
    for (i = 0; i < len; i++)
        buf[i] -= '0';

    v = slot->f28;
    if (v < 10)
    {
        if (f17bc != v)
        {
            f17c0->attach(f17a4, 0);
            f17c0->setCell(2, 0, (int)f177c[buf[0]]);
        }
    }
    else if (v < 100)
    {
        if (f17bc != v)
        {
            f17c0->attach(f17a8, 0);
            f17c0->setCell(2, 0, (int)f177c[buf[1]]);
            f17c0->setCell(3, 0, (int)f177c[buf[0]]);
        }
    }
    else if (v < 1000)
    {
        if (f17bc != v)
        {
            f17c0->attach(f17ac, 0);
            f17c0->setCell(2, 0, (int)f177c[buf[2]]);
            f17c0->setCell(3, 0, (int)f177c[buf[1]]);
            f17c0->setCell(4, 0, (int)f177c[buf[0]]);
        }
    }
    else if (v < 10000)
    {
        if (f17bc != v)
        {
            f17c0->attach(f17b0, 0);
            f17c0->setCell(2, 0, (int)f177c[buf[3]]);
            f17c0->setCell(3, 0, (int)f177c[buf[2]]);
            f17c0->setCell(4, 0, (int)f177c[buf[1]]);
            f17c0->setCell(5, 0, (int)f177c[buf[0]]);
        }
    }
    f17c0->m403940();
    f17bc = slot->f28;
}

/* GFPanel::m4611c0 @0x4611c0 - the CoolBomb, CatchPanel::m470370's twin on
   the s1400 record and placed the way m4617e0 places its bomb: the frame
   centred on the item's x plus the sway, at the item's bottom edge off the
   panel's y span; -1 is idle and the run's f08 frames end it. Anchored at
   +0x8. */
enum
{
    kGcX    = -0x04 / 4,   /* f04 */
    kGcY    = 0,           /* f08 */
    kGcW    = 0x04 / 4,    /* f0c */
    kGcH    = 0x08 / 4     /* f10 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004611c0
void GFPanel::m4611c0()
{
    int i;

    if (s1400.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < ff8)
        {
            int *p = (int *)&items[0].f08;
            int *frame = &s1400.f98[0];

        do
        {
            if (*frame != -1)
            {
                float w;
                float h;
                if (s1400.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1400.f18[*frame]);
                else
                    w = s1400.f10;
                if (s1400.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1400.f18[*frame]);
                else
                    h = s1400.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s1400.f18[*frame],
                                                     GL_F(p, kGcW) * 0.5f + GL_F(p, kGcX) - w * 0.5f + fe0,
                                                     (GL_F(p, kGcH) + GL_F(p, kGcY)) + (f17e4 - f1874) + s1400.f0c,
                                                     w, h, -1);
                {
                    int period = s1400.f04;
                    int count = s1400.f08;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= count)
                            *frame = -1;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < ff8);
        }
    }
}

/* GFPanel::m4614d0 @0x4614d0 - the same again on the s1528 bomb. */
// FUNCTION: EZ2AC_UNPACKED 0x004614d0
void GFPanel::m4614d0()
{
    int i;

    if (s1528.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < ff8)
        {
            int *p = (int *)&items[0].f08;
            int *frame = &s1528.f98[0];

        do
        {
            if (*frame != -1)
            {
                float w;
                float h;
                if (s1528.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1528.f18[*frame]);
                else
                    w = s1528.f10;
                if (s1528.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1528.f18[*frame]);
                else
                    h = s1528.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s1528.f18[*frame],
                                                     GL_F(p, kGcW) * 0.5f + GL_F(p, kGcX) - w * 0.5f + fe0,
                                                     (GL_F(p, kGcH) + GL_F(p, kGcY)) + (f17e4 - f1874) + s1528.f0c,
                                                     w, h, -1);
                {
                    int period = s1528.f04;
                    int count = s1528.f08;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= count)
                            *frame = -1;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < ff8);
        }
    }
}

/* Panel::m4282b0 @0x4282b0 - the keys panel's CoolBomb on s1460, the shape
   of GFPanel::m4611c0 with the y measured from the judge line (f1848 less
   the global scroll and f18d8) and the x swayed by fe4. Anchored at +0x8. */
// FUNCTION: EZ2AC_UNPACKED 0x004282b0
void Panel::m4282b0()
{
    int i;

    if (s1460.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < f12c)
        {
            int *p = (int *)&items[0].f08;
            int *frame = &s1460.f98[0];

        do
        {
            if (*frame != -1)
            {
                float w;
                float h;
                if (s1460.f10 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1460.f18[*frame]);
                else
                    w = s1460.f10;
                if (s1460.f14 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1460.f18[*frame]);
                else
                    h = s1460.f14;
                ((KGraphics *)g_sub1b2f054)->drawTex(s1460.f18[*frame],
                                                     GL_F(p, kGcW) * 0.5f + GL_F(p, kGcX) - w * 0.5f + fe4,
                                                     (GL_F(p, kGcH) + GL_F(p, kGcY)) + (f1848 - g_1b2ef8c - f18d8) + s1460.f0c,
                                                     w, h, -1);
                {
                    int period = s1460.f04;
                    int count = s1460.f08;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= count)
                            *frame = -1;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < f12c);
        }
    }
}

/* Panel::m4285c0 @0x4285c0 - the same bomb again on the record Panel spells
   out flat at 0x1588. */
// FUNCTION: EZ2AC_UNPACKED 0x004285c0
void Panel::m4285c0()
{
    int i;

    if (f1588 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < f12c)
        {
            int *p = (int *)&items[0].f08;
            int *frame = &f1620[0];

        do
        {
            if (*frame != -1)
            {
                float w;
                float h;
                if (f1598 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)f15a0[*frame]);
                else
                    w = f1598;
                if (f159c == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f15a0[*frame]);
                else
                    h = f159c;
                ((KGraphics *)g_sub1b2f054)->drawTex(f15a0[*frame],
                                                     GL_F(p, kGcW) * 0.5f + GL_F(p, kGcX) - w * 0.5f + fe4,
                                                     (GL_F(p, kGcH) + GL_F(p, kGcY)) + (f1848 - g_1b2ef8c - f18d8) + f1594,
                                                     w, h, -1);
                {
                    int period = f158c;
                    int count = f1590;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= count)
                            *frame = -1;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < f12c);
        }
    }
}

/* Panel::m4288d0 @0x4288d0 - the keys panel's LongNoteBomb, GFPanel::m4617e0's
   shape on the record Panel spells out flat at 0x16b0, with the y measured
   from the judge line as its CoolBomb does. Anchored at +0x100. */
// FUNCTION: EZ2AC_UNPACKED 0x004288d0
void Panel::m4288d0()
{
    int i;

    if (f16b0 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        i = 0;
        if (i < f12c)
        {
            int *frame = &f1748[0];
            int *p = (int *)&items[0].f100;

        do
        {
            if (GL_I(p, kGbLit) != 0 && GL_I(p, kGbLit2) != 0)
            {
                float w;
                float h;
                if (f16c0 == 0.0f)
                    w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)f16c8[*frame]);
                else
                    w = f16c0;
                if (f16c4 == 0.0f)
                    h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f16c8[*frame]);
                else
                    h = f16c4;
                ((KGraphics *)g_sub1b2f054)->drawTex(f16c8[*frame],
                                                     GL_F(p, kGbX) + GL_F(p, kGbW) * 0.5f - w * 0.5f + fe4,
                                                     (GL_F(p, kGbY) + GL_F(p, kGbH)) + (f1848 - g_1b2ef8c - f18d8) + f16bc,
                                                     w, h, -1);
                {
                    int period = f16b4;
                    frame[18]++;
                    if (frame[18] >= period)
                    {
                        (*frame)++;
                        if (*frame >= 4)
                            *frame = 0;
                        frame[18] = 0;
                    }
                }
            }
            i++;
            frame++;
            p += sizeof(GdsItem) / 4;
        } while (i < f12c);
        }
    }
}

/* Which of the grader's FAST / SLOW faces the panel should be showing
   (named by the grader, Judge::grade @0x426500 in src/judgegrade.cpp). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea1c
int g_showSlow;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef3c
int g_showFast;

/* Panel::m4280a0 @0x4280a0 - the judgement marker: outside a two-player list
   and while f1444 asks, cell 1 of the chart takes the texture for the last
   grade - KOOL, COOL, GOOD, and for a MISS the slow or fast face (whichever
   the grader flagged), or the last texture for a FAIL; then the chart tweens. */
// FUNCTION: EZ2AC_UNPACKED 0x004280a0
void Panel::m4280a0()
{
    if (f1410 == 0)
        return;
    if (countListA() != 2 && f1444 == 1)
    {
        switch (g_1b2efb0)
        {
        case 1:
            f1440->setCell(1, 0, (int)f1448[5]);
            break;
        case 2:
            f1440->setCell(1, 0, (int)f1448[4]);
            break;
        case 3:
            f1440->setCell(1, 0, (int)f1448[3]);
            break;
        case 4:
            if (g_showSlow == 1)
                f1440->setCell(1, 0, (int)f1448[2]);
            else if (g_showFast == 1)
                f1440->setCell(1, 0, (int)f1448[1]);
            break;
        case 5:
            f1440->setCell(1, 0, (int)f1448[0]);
            break;
        }
    }
    f1440->m403940();
}

/* The CV2 mode's speed ladder (1, 1.5, 2, then quarter steps to 6, 10, 99),
   indexed by the slot's +0x474 rung like the scale ladder; 21 entries. */
// GLOBAL: EZ2AC_UNPACKED 0x0049df40
float g_catchSpeedLadder[21];

/* The song's current bpm as a float, for whoever reads it back. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2dc
float g_songBpmF;
extern int g_songBpm;     /* 0x1b2eac0 (src/panels.cpp) */
extern float g_1b2e708;   /* the chart's scroll multiplier (src/kezplayer.cpp) */
extern int g_1b2e584;     /* src/modeselectctor.cpp */
extern "C" unsigned long __stdcall timeGetTime(void);

/* Chase the scroll rate a tenth of the way to its target, and re-rate the
   lanes when it moved. Inlined into both arms of update2. */
static __forceinline void chaseScrollRate(CatchPanel *p, float target)
{
    float d = target - p->f1cc4;
    if (d != 0.0f)
        p->setScrollRate(p->f1cc4 = d * 0.1f + p->f1cc4);
}

/* CatchPanel::update2 @0x471590 - the per-frame tick: stamp the time, read
   the bpm (from the timing block's summary, or straight off the clock when
   the owner says so) into both globals, chase the scroll rate a tenth of the
   way to its target each frame - the speed percentage, or the slot's ladder
   rung in the CV2 mode - and re-rate every lane when it moved; then the
   counter and sway tick. */
// FUNCTION: EZ2AC_UNPACKED 0x00471590
void CatchPanel::update2(int, int)
{
    float buf[8];

    f1cc8 = timeGetTime();
    if (((CatchSlotOwner *)slot->f360)->bcac == 0)
    {
        slot->f364->q8(buf, 2);
        g_songBpmF = buf[1];
    }
    else
    {
        g_songBpmF = slot->clock->bpm();
    }
    if (((CatchSlotOwner *)slot->f360)->bcac == 0)
    {
        slot->f364->q8(buf, 2);
        g_songBpm = (int)buf[1];
    }
    else
    {
        g_songBpm = (int)slot->clock->bpm();
    }
    if (g_1b2eb6c == 0)
        chaseScrollRate(this, ((float)g_speedPercent * 0.01f) * g_1b2e708);
    else
        chaseScrollRate(this, g_catchSpeedLadder[slot->f474] * g_1b2e708);
    m470af0();
}

/* Give every enabled lane its scroll rate: the per-lane factor times r. */
// FUNCTION: EZ2AC_UNPACKED 0x00471330
void CatchPanel::setScrollRate(float r)
{
    int i;

    i = 0;
    if (i < f2f8)
    {
        const float *f = &f1c7c[0];
        GdsItem *e = &items[0];

        do
        {
            if (e->f00 != 0)
                e->fe8 = *f * r;
            i++;
            f++;
            e++;
        } while (i < f2f8);
    }
}

/* The frame counter (ten frames of the clock's period, then idle) and, in the
   swaying mode, the panel origin and readout y stepping by f19f0 and turning
   round at the record's f19ec and at 150. */
// FUNCTION: EZ2AC_UNPACKED 0x00470af0
void CatchPanel::m470af0()
{
    unsigned int t = slot->clock->now() / f254;

    if (t != f19e8)
    {
        f19e4 = -1;
        f19e0 = 10;
        f19e8 = t;
    }
    if (f19e0 > 0)
    {
        f19e0--;
        if (f19e0 <= 0)
        {
            f19e0 = 0;
            f19e4 = 0;
        }
    }
    if (g_1b2e584 != 0)
    {
        f1950 += f19f0;
        f19dc += f19f0;
        if (f1950 == f19ec || f1950 == 150.0f)
            f19f0 = 0.0f - f19f0;
    }
}

/* The scratch game's speed ladder - the same 21 entries as the catch one. */
// GLOBAL: EZ2AC_UNPACKED 0x0049a268
float g_gfSpeedLadder[21];
/* The two players' second-gauge levels, published for the result screens. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eaa0
float g_gauge1p;
// GLOBAL: EZ2AC_UNPACKED 0x01b2eaa4
float g_gauge2p;
extern PlayerRecord *findPlayer(int id);   /* 0x418f30 (src/nodelists.cpp) */

static __forceinline void chaseGfScrollRate(GFPanel *p, float target)
{
    float d = target - p->f3090;
    if (d != 0.0f)
        p->setScrollRate(p->f3090 = d * 0.1f + p->f3090);
}

/* GFPanel::update2 @0x462af0 - the scratch panel's tick: publish the gauge
   level(s) - both players' when two are listed, else this slot's - read the
   bpm into both globals, stamp the time, report the timers, chase the scroll
   rate (CatchPanel::update2's shape on the scratch ladder) and sway. */
// FUNCTION: EZ2AC_UNPACKED 0x00462af0
void GFPanel::update2(int, int)
{
    float buf[8];

    if (countListA() >= 2)
    {
        g_gauge1p = ((GFStageView *)findPlayer(0))->f38;
        g_gauge2p = ((GFStageView *)findPlayer(1))->f38;
    }
    else
    {
        g_gauge1p = ((GFStageView *)slot)->f38;
    }
    if (((GFStageView *)slot)->life->b330 == 0)
    {
        ((GFStageView *)slot)->sink->query((int *)buf, 2);
        g_songBpmF = buf[1];
    }
    else
    {
        g_songBpmF = ((SlotClock *)slot->f1e8)->bpm();
    }
    if (((GFStageView *)slot)->life->b330 == 0)
    {
        ((GFStageView *)slot)->sink->query((int *)buf, 2);
        g_songBpm = (int)buf[1];
    }
    else
    {
        g_songBpm = (int)((SlotClock *)slot->f1e8)->bpm();
    }
    f3094 = timeGetTime();
    m4626d0();
    if (g_1b2eb6c == 0)
        chaseGfScrollRate(this, ((float)g_speedPercent * 0.01f) * g_1b2e708);
    else
        chaseGfScrollRate(this, g_gfSpeedLadder[slot->f2f0] * g_1b2e708);
    m461de0();
}

/* Give every enabled lane its scroll rate: the per-lane factor times r. */
// FUNCTION: EZ2AC_UNPACKED 0x00462820
void GFPanel::setScrollRate(float r)
{
    int i;

    i = 0;
    if (i < ff8)
    {
        const float *f = (const float *)&f3048[0];
        GdsItem *e = &items[0];

        do
        {
            if (e->f00 != 0)
                e->fe8 = *f * r;
            i++;
            f++;
            e++;
        } while (i < ff8);
    }
}

extern int g_useAudiencePanel;   /* 0x1b2eb48 (src/maingame.cpp) */
extern int g_1b2e7d4;            /* src/maingame.cpp */
extern Obj40C180 *g_bigInstance; /* 0x1b2e710 (src/accessors.cpp) */
extern float g_rateTarget;       /* 0x1b2ebd8 (src/panels.cpp) */
extern int g_1b2ea2c;            /* src/globals.cpp */
extern float g_rate1p;           /* 0x1b2e860 (src/panels.cpp) */
extern float g_rate2p;           /* 0x1b2e864 */
extern ListNode *findListA(int key);   /* 0x418e30 (src/nodelists.cpp) */
/* The keys player, as the list-A node findListA returns it. */
struct KeysPlayerView
{
    char  _pad0[0x1f4];
    float gauge;      /* 0x1f4 */
    char  _pad1f8[0x21c - 0x1f8];
    float rate;       /* 0x21c */
};

static __forceinline void chaseKeysScrollRate(Panel *p, float target)
{
    float d = target - p->f3290;
    if (d != 0.0f)
        p->setScrollRate(p->f3290 = d * 0.1f + p->f3290);
}

/* Panel::update2 @0x429dd0 - the keys panel's tick: publish the gauge(s),
   read the bpm into both globals with the frame delta's fade tick between,
   stamp the time; with the audience panel on, key 0x26 held sets the
   player's rate to the target plus one (and flags it), and while the owner
   lets it the rate decays by 0.0017 a frame to a floor of 0.001, then the
   one or two players' rates are published; then the lamps, the scroll-rate
   chase on the keys ladder, and the sway. */
// FUNCTION: EZ2AC_UNPACKED 0x00429dd0
void Panel::update2(int, unsigned int dt)
{
    float buf[8];

    if (countListA() >= 2)
    {
        g_gauge1p = ((KeysPlayerView *)findListA(0))->gauge;
        g_gauge2p = ((KeysPlayerView *)findListA(1))->gauge;
    }
    else
    {
        g_gauge1p = slot->f1f4;
    }
    if (slot->f3d0->b36c == 0)
    {
        slot->f3d4->query((int *)buf, 2);
        g_songBpmF = buf[1];
    }
    else
    {
        g_songBpmF = ((SlotClock *)slot->f3d8)->bpm();
    }
    f3294 = timeGetTime();
    if (dt > 1000)
        m427300(0);
    else
        m427300(dt);
    if (slot->f3d0->b36c == 0)
    {
        slot->f3d4->query((int *)buf, 2);
        g_songBpm = (int)buf[1];
    }
    else
    {
        g_songBpm = (int)((SlotClock *)slot->f3d8)->bpm();
    }
    if (g_useAudiencePanel == 1)
    {
        if (g_bigInstance->getState(0x26) == 2)
        {
            slot->f21c = g_rateTarget + 1.0f;
            g_1b2ea2c = 1;
        }
        if (g_1b2e7d4 == 0 && slot->f3d0->f7f0 == 0 && slot->f3d0->b36c != 0)
        {
            slot->f21c -= 0.0017f;
            if (slot->f21c < 0.0f)
                slot->f21c = 0.001f;
        }
        if (countListA() == 1)
        {
            if (findListA(0) != 0)
                g_rate1p = ((KeysPlayerView *)findListA(0))->rate;
            else
                g_rate2p = ((KeysPlayerView *)findListA(1))->rate;
        }
        else
        {
            g_rate1p = ((KeysPlayerView *)findListA(0))->rate;
            g_rate2p = ((KeysPlayerView *)findListA(1))->rate;
        }
    }
    m4298c0();
    if (g_1b2eb6c == 0)
        chaseKeysScrollRate(this, ((float)g_speedPercent * 0.01f) * g_1b2e708);
    else
        chaseKeysScrollRate(this, g_panelRateLadder[slot->f4e0] * g_1b2e708);
    m428c60();
}

/* Give every enabled lane its scroll rate: the per-lane factor times r. */
// FUNCTION: EZ2AC_UNPACKED 0x00429b20
void Panel::setScrollRate(float r)
{
    int i;

    i = 0;
    if (i < f12c)
    {
        const float *f = (const float *)&f3248[0];
        GdsItem *e = &items[0];

        do
        {
            if (e->f00 != 0)
                e->fe8 = *f * r;
            i++;
            f++;
            e++;
        } while (i < f12c);
    }
}

/* Panel::m427300 @0x427300 - the bouncing marker's tick, fed the frame's
   delta. While the slot's +0x534 gate is up: heading right (f104 > 0) it
   accelerates by a tenth of the delta up to 30, and within 30 of the right
   edge it clamps there and decelerates, turning round once stopped; heading
   left it is the mirror against the left edge. fec[4] is the position and
   fec[5] the speed.
   The defect was a load pair in the last block (`movss xmm0,[step]` /
   `addss xmm0,[fec+5]` the other way round, 97.37%), and five spellings of
   that sum changed nothing. The lever was elsewhere: the PARENTHESES around
   `lim`'s `fec[1] - 30.0f` in the FIRST arm (cf. the cookbook's
   parentheses-as-a-barrier rule), which fixes that later pair's order for
   reasons not pinned down. Found by scratchpad/perm. */
// FUNCTION: EZ2AC_UNPACKED 0x00427300
void Panel::m427300(unsigned int dt)
{
    float step;

    if (slot->f534 == 0)
        return;
    if (f104 > 0)
    {
        float lim = (fec[1] - 30.0f);
        if (fec[4] >= lim)
        {
            fec[4] = lim;
            fec[5] = fec[5] - (float)dt * 0.1f;
            if (fec[5] <= 0.0f)
            {
                f104 = -1;
                fec[5] = 0.0f;
            }
        }
        else
        {
            step = (float)dt * 0.1f;
            fec[4] = fec[4] + step;
            if (fec[5] < 30.0f)
                fec[5] = step + fec[5];
            else
                fec[5] = 30.0f;
        }
    }
    else
    {
        if (fec[0] >= fec[4])
        {
            fec[4] = fec[0];
            fec[5] = fec[5] - (float)dt * 0.1f;
            if (fec[5] <= 0.0f)
            {
                f104 = 1;
                fec[5] = 0.0f;
            }
        }
        else
        {
            step = (float)dt * 0.1f;
            fec[4] = fec[4] - step;
            if (fec[5] < 30.0f)
                fec[5] = step + fec[5];
            else
                fec[5] = 30.0f;
        }
    }
}

extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */

/* Panel::init @0x42a0c0 - attach the keys panel to its slot: re-rate the
   lanes, take the run period out of the sink's summary, clear the global
   draw origin, and set the sway by mode (centre / amplitude pairs per mode,
   the modes 4 and 5 publishing the swayed x as the origin, the others not;
   no sway at all without the option - the current x just zeroed); the
   second player's sway is mirrored. */
// FUNCTION: EZ2AC_UNPACKED 0x0042a0c0
void Panel::init(PanelSlot *s)
{
    int buf[8];

    slot = s;
    resetLanes();
    s->f3d4->query(buf, 4);
    f2c = buf[2];
    g_drawOriginX = 0.0f;
    if (g_1b2e7a8 == 1)
    {
        if (g_modeIndex == 1)
        {
            fe0 = 200.0f;
            fdc = 190.0f;
            g_panelSwayPublish = 0;
        }
        else if (g_modeIndex == 2)
        {
            fe0 = 220.0f;
            fdc = 210.0f;
            g_panelSwayPublish = 0;
        }
        else if (g_modeIndex == 3)
        {
            fe0 = 190.0f;
            fdc = 175.0f;
            g_panelSwayPublish = 0;
        }
        else if (g_modeIndex == 6)
        {
            fe0 = 195.0f;
            fdc = 185.0f;
            g_panelSwayPublish = 0;
        }
        else if (g_modeIndex == 7)
        {
            fe0 = 195.0f;
            fdc = 185.0f;
            g_panelSwayPublish = 0;
        }
        else if (g_modeIndex == 4)
        {
            fe0 = 60.0f;
            fdc = 0.0f;
            g_panelSwayPublish = 1;
        }
        else if (g_modeIndex == 5)
        {
            fe0 = 60.0f;
            fdc = 0.0f;
            g_panelSwayPublish = 1;
        }
    }
    else
    {
        fe4 = 0.0f;
    }
    if (s->index != 0)
    {
        fe0 = fe0 * -1.0f;
        fdc = fdc * -1.0f;
    }
}

/* GFPanel::init @0x462cb0 - Panel::init on the scratch panel: the sink off
   the slot's +0x1e4, the period into f28, its own sway globals, and modes
   2, 3, 7, 4, 5 only (no 1 or 6). */
// FUNCTION: EZ2AC_UNPACKED 0x00462cb0
void GFPanel::init(PanelSlot *s)
{
    int buf[8];

    slot = s;
    resetLanes();
    ((GFStageView *)s)->sink->query(buf, 4);
    f28 = buf[2];
    g_gfDrawOriginX = 0.0f;
    if (g_1b2e7a8 == 1)
    {
        if (g_modeIndex == 2)
        {
            fdc = 220.0f;
            fd8 = 210.0f;
            g_gfPanelSwayPublish = 0;
        }
        else if (g_modeIndex == 3)
        {
            fdc = 190.0f;
            fd8 = 175.0f;
            g_gfPanelSwayPublish = 0;
        }
        else if (g_modeIndex == 7)
        {
            fdc = 195.0f;
            fd8 = 185.0f;
            g_gfPanelSwayPublish = 0;
        }
        else if (g_modeIndex == 4)
        {
            fdc = 60.0f;
            fd8 = 0.0f;
            g_gfPanelSwayPublish = 1;
        }
        else if (g_modeIndex == 5)
        {
            fdc = 60.0f;
            fd8 = 0.0f;
            g_gfPanelSwayPublish = 1;
        }
    }
    else
    {
        fe0 = 0.0f;
    }
    if (s->index != 0)
    {
        fdc = fdc * -1.0f;
        fd8 = fd8 * -1.0f;
    }
}

/* Panel::m429aa0 @0x429aa0 - re-attach the three chart heads. The EZ2DJ twin
   of GFPanel::m4627c0 @0x4627c0 above, and the heads go back in the same
   order 0, 2, 1 - the source's order, not the fields'. The first argument is
   unused here too.

   What it adds over the scratch copy is two guards ahead of the mode test:
   neither of the cabinet's inputs 4 and 5 may be in state 3. So a head that
   is being held is left where it is rather than being snapped back. */
// FUNCTION: EZ2AC_UNPACKED 0x00429aa0
void Panel::m429aa0(int, int mode)
{
    if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3 &&
        mode == 2 && f1d30 != 0)
    {
        f1dc8->attach(f1d34, 0);
        f1dd0->attach(f1d3c, 0);
        f1dcc->attach(f1d38, 0);
    }
}

/* The scratch panel's clock at the slot's +0x1e8; only slot 22 is called. */
struct GFPanelClock
{
    virtual void c00(); virtual void c01(); virtual void c02();
    virtual void c03(); virtual void c04(); virtual void c05();
    virtual void c06(); virtual void c07(); virtual void c08();
    virtual void c09(); virtual void c10(); virtual void c11();
    virtual void c12(); virtual void c13(); virtual void c14();
    virtual void c15(); virtual void c16(); virtual void c17();
    virtual void c18(); virtual void c19(); virtual void c20();
    virtual void c21();
    virtual unsigned int now();   /* slot 22 -> vtable+0x58 */
};

/* GFPanel::m461de0 @0x461de0 - the scratch panel's swaying readout, the twin
   of Panel::m428c60 without the clock doubling and the global copy.

   Its -100 is a FLOAT-typed subtraction rather than a double one: left as a
   double, cl folds the literal to a qword constant and the original's
   `fsub dword ptr` can never appear. */
// FUNCTION: EZ2AC_UNPACKED 0x00461de0
void GFPanel::m461de0()
{
    unsigned int t = ((GFPanelClock *)slot->f1e8)->now();

    if (g_1b2e584 != 0)
    {
        unsigned int d = f28 * 2;
        float y = (float)(sin((double)(((t >> 5) + 0x60) % d) * 6.28 /
                              (int)d) * 100.0 + f1874) - 100.0f;

        f17e4 = y;
        f1870 = y + f17ec;
    }
    else
    {
        unsigned int span = f28;
        double y = sin((double)((t >> 3) % span) * 6.28 /
                       ((int)span * 0.5f));

        f17e4 = (float)(int)(y + y) + f1874;
        f1870 = f17ec + f17e4;
    }
}

/* The keys panel's own copy of the sway offset, published from g_1b2ef8c on
   the way past. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea80
float g_1b2ea80;

/* Panel::m428c60 @0x428c60 - the keys panel's swaying readout, the twin of
   GFPanel::m461de0 above. It doubles the clock first, publishes one global
   float into another on the way past, and keeps its span at +0x2c and its
   outputs at +0x1848/+0x18d4.

   The span is read INSIDE each arm, not into a local across the branch -
   read across it, cl spills it once before the branch as well as inside,
   which is the whole difference between this and the original. */
// FUNCTION: EZ2AC_UNPACKED 0x00428c60
void Panel::m428c60()
{
    unsigned int t = ((GFPanelClock *)slot->f3d8)->now() * 2;

    g_1b2ea80 = g_1b2ef8c;

    if (g_1b2e584 != 0)
    {
        unsigned int d = f2c * 2;
        float y = (float)(sin((double)(((t >> 5) + 0x60) % d) * 6.28 /
                              (int)d) * 100.0 + f18d8) - 100.0f;

        f1848 = y;
        f18d4 = y + f1850;
    }
    else
    {
        unsigned int span = f2c;
        double y = sin((double)((t >> 3) % span) * 6.28 /
                       ((int)span * 0.5f));

        f1848 = (float)(int)(y + y) + f18d8 + g_1b2ea80;
        f18d4 = f1850 + f1848 + g_1b2ea80;
    }
}

/* Panel::m429950 @0x429950 - swap two items' handles, or just move the
   active mark between them. The mode is latched in f320c either way; mode 0
   exchanges the two +0xec handles and marks b active, anything else only
   moves the mark to a.

   BOTH POINTERS ARE ANCHORED AT THE ROW, NOT AT THE ITEM, AND BOTH ARE
   DECLARED BEFORE THE TEMPORARY. This sat at 63-73% through six forms
   because the two things it needs pull against each other in the obvious
   spellings: subscripting the array (`items[a].fec`) gives the original's
   DISPLACEMENTS - base `this`, index i*260, the member offsets folded in as
   0x134 and 0x220 - but makes cl read B's handle first and pay a second
   callee-saved register for it; naming `GdsItem *pa = &items[a];` gives the
   original's READ ORDER but folds the array's own 0x134 base into the
   pointer, so the fields fall to +0 and +0xec.

   C cannot express the original through either, because the array's base
   offset belongs to whichever of the two the source names. Anchoring the
   pointers at `this + i*260` instead - one row below the item - and reaching
   the fields by their absolute offsets gives both at once. Declaring BOTH
   before the temporary is the other half: with `pb` declared after `t`, cl
   schedules b's read ahead of a's and the register shape goes back to two
   saves (61.11%).

   Six earlier forms, for the record: a's pointer named 72.73%, both named
   69.70%, plain indexing 63.64%, the two handles read into named locals in
   order 63.64%, b's pointer named after t 60.61%, both named with t first
   59.38%. */
// FUNCTION: EZ2AC_UNPACKED 0x00429950
void Panel::m429950(int mode, int a, int b)
{
    f320c = mode;

    if (mode == 0)
    {
        /* Rows, not items: `items` starts 0x134 into the object, so the
           item's own +0x00 and +0xec are +0x134 and +0x220 from here. */
        int *pa = (int *)this + 65 * a;
        int *pb = (int *)this + 65 * b;
        int  t  = pa[0x220 / 4];

        pa[0x220 / 4] = pb[0x220 / 4];
        pb[0x220 / 4] = t;
        pa[0x134 / 4] = 0;
        pb[0x134 / 4] = 1;
    }
    else
    {
        items[a].f00 = 1;
        items[b].f00 = 0;
    }
}

/* Panel::m427490 @0x427490 - the scroll strip's GHOST TRAIL: up to five
   fading copies of the gray strip, alpha (5-i)*0x33 walking 255 down to
   51, drawn only while the slot's +0x534 flag is up. Scrolling one way
   (f104 > 0) each segment reaches BACK from the carried position by the
   window f100 and clamps at fec[0]; scrolling the other it reaches
   FORWARD and clamps at fec[1]; a clamped-away segment ends the chain
   early through its `continue`. The first segment is the strip itself -
   position fec[4], length 30 - and each draw is
   (fe4 + pos, fec[2], len, fec[3] - fec[2]). */
// FUNCTION: EZ2AC_UNPACKED 0x00427490
void Panel::m427490()
{
    float pos, len, h;
    int   i;

    if (*(int *)((char *)slot + 0x534) == 0)
        return;
    pos = fec[4];
    len = 30.0f;
    for (i = 0; i < 5; i++)
    {
        if (f104 > 0)
        {
            float start;

            if (i != 0)
            {
                float top = pos - fec[5];

                if (top < fec[0])
                    top = fec[0];
                h = pos - top;
                pos = top;
                len = h;
                if (h <= 0.0f)
                    continue;
                start = top;
            }
            else
            {
                start = fec[4];
                h = 30.0f;
            }
            ((KGraphics *)g_sub1b2f054)->drawTex(f108, fe4 + start, fec[2],
                                                 h, fec[3] - fec[2],
                                                 (((0x5050505 - i) * 0x33) << 24) |
                                                 0xffffff);
        }
        else
        {
            float start;

            if (i != 0)
            {
                float end = pos + len;

                h = fec[5];
                if (end >= fec[1] - fec[5])
                    h = fec[1] - end;
                pos = end;
                len = h;
                if (h <= 0.0f)
                    continue;
                start = end;
            }
            else
            {
                start = fec[4];
                h = 30.0f;
            }
            ((KGraphics *)g_sub1b2f054)->drawTex(f108, fe4 + start, fec[2],
                                                 h, fec[3] - fec[2],
                                                 (((0x5050505 - i) * 0x33) << 24) |
                                                 0xffffff);
        }
    }
}
