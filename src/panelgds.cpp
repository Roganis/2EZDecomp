/* Panel::LoadPVI @0x42d700 - the main note field's .gds skin reader, and the
   THIRD and last of the family. Decoded in wip/panel-loadpvi.cpp first; this
   is that analysis integrated.

   There are exactly three of these and the binary says so: grepping for
   "Unknown Section" finds one log string per reader, each naming its own class -

       Panel::LoadPVI       @0x42d700   27 sections   <- this file
       GFPanel::LoadPVI     @0x4654b0   23 sections   src/gfpanelgds.cpp
       CatchPanel::LoadPVI  @0x472650   17 sections   src/catchpanelgds.cpp

   Same skeleton as its two siblings - a KParser over a buffer, an early return
   if the file will not open, a `TrackN` arm that splits the digits off the
   section name, an _stricmp chain, and an unknown section that logs and ABORTS
   (its log block falls through into the return-0 epilogue with no jump).

   What is specific to this one is all in the tail:

     - TWO KParser objects, both constructed up front and destroyed in reverse,
       SHARING one 0x4800 buffer which is memset between the two uses.
     - after the loop it loads "System\common\auto.bmp" and measures the auto
       marker back from f1848, less a pool 8.0 and the texture's own width.
     - it then changes into "System\infoview" and reads a SECOND file,
       "infoview.pvi", with the second parser - a one-arm loop that only
       recognizes "Score".

   It is also the only one of the three with an aligned frame (push ebp; mov
   ebp,esp; and esp,-8), which is why its parameter is read off ebp. */

#include "gamepanels.h"
#include "kparser.h"
#include <string.h>
#include <stdlib.h>

extern int  readGdsInt(int *out, KParser *p);       /* 0x456270 */
extern int  readGdsSection(char *out, KParser *p);  /* 0x4561b0 */
extern void changeDir(char *path, int savePrev);    /* 0x412dd0 */
extern void popDirectory();                         /* 0x412e60 */
extern void Log(const char *fmt, ...);              /* 0x402880 */
extern EzSubsystem *g_sub1b2f054;

/* What the auto.bmp handle is read through - only its width is wanted here. */
struct AutoTexHead
{
    char  _pad0[0x84];
    float f84;                  /* 0x84 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0042d700
int Panel::LoadPVI(char *name)
{
    char    sec[0x100];
    char    digits[0x80];
    KParser parser;
    char    buf[0x4800];        /* SHARED with parser2 */
    KParser parser2;
    float   w;
    int     n;

    parser.setBuffer(buf, 0x4800);

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f1cd4 = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_strnicmp("Track", sec, 5) == 0)
        {
            strcpy(digits, sec + 5);
            n = atoi(digits);
            loadGdsItem(&parser, n - 1);
        }
        else if (_stricmp("GroupTexture", sec) == 0)
        {
            ((GFPanel *)this)->m460ac0(&parser);
        }
        else if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&f12c, &parser);
        }
        else if (_stricmp("Judgment", sec) == 0)
        {
            m427f30(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490690
        else if (_stricmp("JudgmentTex", sec) == 0)
        {
            m428000(&parser);
        }
        else if (_stricmp("MeasureLine", sec) == 0)
        {
            m427cf0(&parser);
        }
        else if (_stricmp("CoolBomb", sec) == 0)
        {
            m428150(&parser);
        }
        else if (_stricmp("LongNoteBomb", sec) == 0)
        {
            m428760(&parser);
        }
        else if (_stricmp("CoolCombo", sec) == 0)
        {
            m42b830(&parser);
        }
        else if (_stricmp("KeyPanel", sec) == 0)
        {
            m428a80(&parser);
        }
        else if (_stricmp("TargetBar", sec) == 0)
        {
            m428ae0(&parser);
        }
        else if (_stricmp("MaxCoolCombo", sec) == 0)
        {
            m428d80(&parser);
        }
        else if (_stricmp("Score", sec) == 0)
        {
            m428e10(&parser);
        }
        else if (_stricmp("ComboGauge", sec) == 0)
        {
            m429050(&parser);
        }
        else if (_stricmp("GrooveGauge", sec) == 0)
        {
            m4290e0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490608
        else if (_stricmp("RubyGauge", sec) == 0)
        {
            m42ce40(&parser);
        }
        else if (_stricmp("Effector", sec) == 0)
        {
            m4294a0(&parser);
        }
        else if (_stricmp("AudienceRating", sec) == 0)
        {
            m428f30(&parser);
        }
        else if (_stricmp("GoalAudienceRating", sec) == 0)
        {
            m428fc0(&parser);
        }
        else if (_stricmp("GrooveLight", sec) == 0)
        {
            m429400(&parser);
        }
        else if (_stricmp("SongSelectPane", sec) == 0)
        {
            m4295c0(&parser);
        }
        else if (_stricmp("ComboEffect", sec) == 0)
        {
            m429650(&parser);
        }
        else if (_stricmp("RubyKeyPanel", sec) == 0)
        {
            m4296e0(&parser);
        }
        else if (_stricmp("GoodBomb", sec) == 0)
        {
            m428450(&parser);
        }
        else if (_stricmp("PuzzleNote", sec) == 0)
        {
            m429770(&parser);
        }
        else if (_stricmp("SpecialNote", sec) == 0)
        {
            m429890(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0049057c
        else if (_stricmp("FastSlowTex", sec) == 0)
        {
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0049053c
            Log("Panel::LoadPVI: '%s' Unknown Section", sec);
            return 0;
        }
    }

    popDirectory();
    m427750();

    // STRING: EZ2AC_UNPACKED 0x00490564
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\common\\auto.bmp", &f110, 0);

    if (f1840 != 0)
    {
        w = ((AutoTexHead *)f110)->f84;
    }
    else
    {
        w = 0.0f;
    }

    f114 = f1848 - 8.0f - w;

    // STRING: EZ2AC_UNPACKED 0x00490528
    changeDir("System\\infoview", 1);

    /* An aggregate INITIALIZER, not a strcpy: the original copies the twelve
       bytes and then zero-fills the remaining 243, which is what an
       initializer does and a strcpy does not. */
    // STRING: EZ2AC_UNPACKED 0x00490518
    char infoName[256] = "infoview.pvi";

    memset(buf, 0, sizeof(buf));
    parser2.setBuffer(buf, 0x4800);

    if (parser2.readFile(infoName) != 0)
    {
        /* The SAME section buffer the main loop used - the original has only
           one, which is what keeps its frame at 0xeaec. */
        while (readGdsSection(sec, &parser2) != 0)
        {
            if (_stricmp("Score", sec) == 0)
            {
                m428ea0(&parser2);
            }
        }
    }

    return 1;
}

extern int readGdsFloat2(float *a, float *b, KParser *p);            /* 0x456590 */
extern int readGdsTexture(KTexture **out, KGraphics *g, KParser *p); /* 0x4563a0 */

/* Panel's GrooveLight section - m429400 @0x429400. The third copy of the
   loader GFPanel has at 0x462220 and CatchPanel at 0x470ee0: it writes the
   element the running count at f1cd4 selects and bumps it, so a .gds may
   carry several GrooveLight sections. LoadPVI above is what zeroes the count,
   and matching it is what identified both the counter and the pair. */
// FUNCTION: EZ2AC_UNPACKED 0x00429400
int Panel::m429400(KParser *p)
{
    GdsGrooveLight *e = &s1cd8[f1cd4];

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsTexture(&e->f14, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&e->f18, &e->f1c, p);
    readGdsFloat2(&e->f20, &e->f24, p);
    readGdsTexture(&e->f28, (KGraphics *)g_sub1b2f054, p);
    f1cd4++;
    return 1;
}

extern int readGdsFloat(float *out, KParser *p);                     /* 0x456230 */
extern int readGdsStr(Obj402860Head **out, KGraphics *g, KParser *p);/* 0x456500 */
extern int readGdsToken(char *out, KParser *p);                      /* 0x4561e0 */
extern int readGdsTexSeries(KTexture **dst, int n, KGraphics *g, KParser *p);
extern int readGdsInt2(int *a, int *b, KParser *p);          /* 0x456600 */
extern int loadTexSeries(KTexture **dst, int n, KGraphics *g, char *base);
extern Obj402860 g_panelCharts3[2];
extern Obj402860 g_panelCharts4[2];
extern Obj402860 g_panelChartSingle;
extern Obj402860 g_panelCharts7[2];
extern int g_1b2ea30;
extern int g_1b2e920;

/* Panel's MeasureLine - m427cf0 @0x427cf0. GFPanel's m460f00 twin, clamp and
   all: the clamp is on s1460.f08, the CoolBomb run count, not on anything this
   loader reads. */
// FUNCTION: EZ2AC_UNPACKED 0x00427CF0
int Panel::m427cf0(KParser *p)
{
    readGdsInt(&f137c, p);
    readGdsFloat(&f1384, p);
    readGdsFloat2(&f1388, &f138c, p);
    readGdsInt(&f1380, p);
    readGdsTexSeries(f1390, 6, (KGraphics *)g_sub1b2f054, p);

    if (f138c == 0.0f)
    {
        f138c = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f1390[0]);
    }

    if (s1460.f08 > 0x20)
    {
        s1460.f08 = 0x20;
    }

    return 1;
}

/* Panel's CoolBomb - m428150 @0x428150. The third panel's copy of the bomb
   loader; Panel's other two are written out flat as m428450 / m428760. */
// FUNCTION: EZ2AC_UNPACKED 0x00428150
int Panel::m428150(KParser *p)
{
    char buf[0x1000];
    char *s;
    int i;
    int n;

    readGdsInt(&s1460.f00, p);
    readGdsFloat(&s1460.f0c, p);
    readGdsFloat2(&s1460.f10, &s1460.f14, p);
    readGdsInt(&s1460.f04, p);
    readGdsInt(&s1460.f08, p);

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

    loadTexSeries(s1460.f18, s1460.f08, (KGraphics *)g_sub1b2f054, s);

    if (s1460.f08 > 0x20)
    {
        s1460.f08 = 0x20;
    }

    for (i = 0; i < f12c; i++)
    {
        s1460.f98[i] = -1;
        s1460.fe0[i] = 0;
    }

    return 1;
}

/* Panel's ComboEffect - m429650 @0x429650. CatchPanel::m471200's twin plus one
   store the others do not have: a SECOND chart, pinned to the fixed element
   g_panelCharts6[1] rather than the one the slot selects. */
// FUNCTION: EZ2AC_UNPACKED 0x00429650
int Panel::m429650(KParser *p)
{
    readGdsInt(&f1dd4, p);
    readGdsStr(&f1dd8, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1ddc, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1de0, (KGraphics *)g_sub1b2f054, p);
    f1de4 = &g_panelCharts7[slot->index];
    optional = &g_panelChartSingle;
    f1dd4 = g_1b2e920;
    return 1;
}

/* Panel's RubyKeyPanel - m4296e0 @0x4296e0. GFPanel's m4624f0 twin. */
// FUNCTION: EZ2AC_UNPACKED 0x004296E0
int Panel::m4296e0(KParser *p)
{
    readGdsInt(&f1de8, p);
    readGdsFloat2(&f1dec, &f1df0, p);
    readGdsFloat2(&f1df4, &f1df8, p);
    readGdsTexture(&f1dfc, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1e00, (KGraphics *)g_sub1b2f054, p);
    f1e04 = &g_panelCharts3[slot->index];
    return 1;
}

/* Panel's SpecialNote - m429890 @0x429890. One six-texture run, nothing else. */
// FUNCTION: EZ2AC_UNPACKED 0x00429890
int Panel::m429890(KParser *p)
{
    readGdsTexSeries(f288c, 6, (KGraphics *)g_sub1b2f054, p);
    return 1;
}

extern int g_1b2e584;   /* src/modeselectctor.cpp */

/* Panel's CoolCombo - m42b830 @0x42b830. GFPanel's m463ff0 twin, offsetting
   the first of its float pair by the panel's own fe4 before storing it into
   the chart the slot picks. */
// FUNCTION: EZ2AC_UNPACKED 0x0042B830
int Panel::m42b830(KParser *p)
{
    Obj402860 *c;
    float      x;
    float      y;

    readGdsInt(&f17d8, p);
    readGdsTexSeries(f17dc, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f1814, &f1818, p);
    readGdsStr(&f1804, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1808, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f180c, (KGraphics *)g_sub1b2f054, p);
    readGdsStr(&f1810, (KGraphics *)g_sub1b2f054, p);

    x = fe4 + f1814;
    y = f1818;
    c = &g_panelCharts4[slot->index];
    f1820 = c;
    c->f214 = x;
    c->f218 = y;
    return 1;
}

/* Panel's TargetBar - m428ae0 @0x428ae0. GFPanel's m461c60 twin: one geometry
   block per name variant with all but the last discarded, a zero width or
   height taken from the first texture, and a flag raised for the extra
   cabinets. LoadPVI reads f1840 and f1848 back out of this record. */
// FUNCTION: EZ2AC_UNPACKED 0x00428AE0
int Panel::m428ae0(KParser *p)
{
    char buf[0x1000];
    int  i;

    readGdsInt(&f1840, p);

    for (i = 0; i < g_1b2ea30; i++)
    {
        readGdsFloat2(&f1844, &f1848, p);
        readGdsFloat2(&f184c, &f1850, p);
        readGdsToken(buf, p);
    }

    readGdsFloat2(&f1844, &f1848, p);
    readGdsFloat2(&f184c, &f1850, p);
    readGdsTexSeries(f1854, 6, (KGraphics *)g_sub1b2f054, p);

    f18d8 = f1848;
    f18e0 = 0.0f;
    f18dc = 0.0f;

    if (f184c == 0.0f)
    {
        f184c = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)f1854[0]);
    }

    if (f1850 == 0.0f)
    {
        f1850 = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)f1854[0]);
    }

    f18d4 = f1848 + f1850;

    if (g_1b2e584 != 0 || g_1b2ea30 != 0)
    {
        f1840 = 1;
    }

    return 1;
}

/* Panel's MaxCoolCombo - m428d80 @0x428d80. The texture-run section's field
   order and read order, with a 0x80 name buffer rather than 0x40. */
// FUNCTION: EZ2AC_UNPACKED 0x00428D80
int Panel::m428d80(KParser *p)
{
    readGdsInt(&f18e4, p);
    readGdsFloat2(&f18e8, &f18ec, p);
    readGdsTexSeries(f18fc, 10, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat2(&f18f0, &f18f4, p);
    readGdsFloat(&f18f8, p);
    readGdsToken(n1924, p);
    readGdsInt2(&f19a4, &f19a8, p);
    return 1;
}

/* Panel::m427750 @0x427750 - the note field's bounding box, run by LoadPVI
   after the .gds is read and skipped entirely unless the slot asks for it.

   The pointer is anchored at items[0].f04, not at items[0], which is what
   names the item's geometry: (f04, f08) is its position and (f0c, f10) its
   size. Only the x axis is reduced - a minimum seeded with the screen width
   and a maximum seeded with zero - while the y pair is simply whatever the
   LAST item had, so those two locals are read uninitialized when the table is
   empty and cl gives them one shared stack slot. */
// FUNCTION: EZ2AC_UNPACKED 0x00427750
void Panel::m427750()
{
    float minx;
    float maxx;
    float y;
    float yh;
    int   i;

    if (slot->f534 == 0)
    {
        return;
    }

    // STRING: EZ2AC_UNPACKED 0x00490474
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\ModeSelect\\gray.bmp", &f108, 0);

    minx = 640.0f;
    maxx = 0.0f;

    for (i = 0; i < f12c; i++)
    {
        if (minx > items[i].f04)
        {
            minx = items[i].f04;
        }

        if (items[i].f04 + items[i].f0c > maxx)
        {
            maxx = items[i].f04 + items[i].f0c;
        }

        y  = items[i].f08;
        yh = items[i].f08 + items[i].f10;
    }

    fec[0] = minx;
    fec[1] = maxx;
    fec[2] = y;
    fec[3] = yh;
    fec[4] = minx;
    f104 = 1;
}

/* Panel::m427c50 @0x427c50 - the auto-play badge over one item of the table.

   The badge's width is two thirds of the item's, and it is CENTRED on the
   item: half the leftover width goes on each side, on top of the panel's own
   x origin and the item's. Its y and its texture are the pair LoadPVI derives
   from "System\\common\\auto.bmp", and its height is a flat 54. Drawn with the
   normal blend and the mode put back to (5, 6) afterwards. */
// FUNCTION: EZ2AC_UNPACKED 0x00427c50
void Panel::m427c50(int n)
{
    GdsItem *it = &items[n];
    float    w;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    w = it->f0c * (2.0f / 3.0f);
    ((KGraphics *)g_sub1b2f054)->drawTex(f110,
                                         (it->f0c - w) * 0.5f + fe4 + it->f04,
                                         f114, w, 54.0f, -1);

    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
}

/* Panel::m427bd0 @0x427bd0 - a rectangle's two VERTICAL edges, drawn as two
   untextured lines with stage 0 unbound first. The right edge is at
   x + w - 1, so the pair spans exactly w pixels inclusive.

   Nothing in the body touches `this`; it is a member only because every call
   site sets ecx (and two different panel classes share it). */
// FUNCTION: EZ2AC_UNPACKED 0x00427BD0
void Panel::m427bd0(float x, float y, float w, float h, int c0, int c1)
{
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->drawLine(x, y, x, y + h, 1.0f, c0, c1);
    ((KGraphics *)g_sub1b2f054)->drawLine(x + w - 1.0f, y, x + w - 1.0f, y + h,
                                          1.0f, c0, c1);
}
