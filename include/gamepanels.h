#ifndef GAMEPANELS_H
#define GAMEPANELS_H

#include "panels.h"      /* EzObject, Obj402860 */
#include "gdsitem.h"     /* GdsItem - the .gds item record the panels share */

/* THE THREE GAME PANELS - the in-game note field, one per game family.

     vtable    name        size     dtor      deleting ~  slot ptr  charts
     0x490454  Panel       0x36d8   0x4271a0  0x429db0    +0x14     8 globals
     0x49a498  GFPanel     0x34dc   0x4608c0  0x462ad0    +0x14     7 globals
     0x49e174  CatchPanel  0x1cd0   0x46f480  0x471570    +0x10     6 globals

   None of them has a second base. Each destructor clears a fixed list of GLOBAL
   Obj402860 pairs, every one indexed by the same runtime value - the int at +8
   of the object the panel points at - and then lets its own charts go. The
   pairs are packed at a 0x438 stride, i.e. exactly two Obj402860 each, which is
   what fixes the element count at two: the index cannot reach further without
   running into the next pair. Two players, one chart each. */
/* Never instantiated - only the slot shape matters. */
struct SlotTiming
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4(); virtual void q5();
    virtual void q6(); virtual void q7();
    virtual void q8(void *out, int count);   /* slot 8 -> +0x20 */
};

struct PanelSlot
{
    char _pad0[8];
    int  index;      /* 0x08 - which of the pair each global chart is taken from */
    char _padc[0x28 - 0xc];
    int  f28;        /* 0x28 - the scratch combo GFPanel's readout spells */
    /* Four counters the readouts print. They are ints, not scales: Panel's
       @0x42bd80 compares f1e0 against 10 / 100 / 1000 / 10000 to choose a
       format string, which is score-sized. */
    int  f2c;        /* 0x2c */
    int  f30;        /* 0x30 */
    char _pad34[0x54 - 0x34];
    /* A level GFPanel's readout @0x4643b0 bleeds down every frame and prints. */
    float f54;       /* 0x54 */
    char _pad58[0x1c0 - 0x58];
    /* The catch commit @0x472fc0: the combo latch, the per-judge counters,
       and the running combo. */
    int  f1c0;       /* 0x1c0 */
    int  f1c4[6];    /* 0x1c4 .. 0x1dc */
    int  f1dc;       /* 0x1dc */
    /* GFPanel's @0x4643b0 DEREFERENCES this same +0x1e0 (`[slot->f1e0 +
       0x784]`) through a cast, so the three panels evidently do not all point
       `slot` at the same object. */
    /* An int here because most sites pass it straight on as one; GFPanel is
       the one that DEREFERENCES it, as the scratch game's run owner, and casts
       (see GFPanel::resetLanes). */
    int  f1e0;       /* 0x1e0 */
    int  f1e4;       /* 0x1e4 */
    int  f1e8;       /* 0x1e8 */
    /* The catch player's life - its second gauge chases this one. */
    float f1ec;      /* 0x1ec */
    char _pad1f0[0x1f4 - 0x1f0];
    /* The life level, 0..100+ - the gauge chases and draws it. */
    float f1f4;      /* 0x1f4 */
    char _pad1f8[0x21c - 0x1f8];
    float f21c;      /* 0x21c - the keys player's rate, stepped down by update2 */
    char _pad220[0x2b4 - 0x220];
    int  f2b4;       /* 0x2b4 - the scratch game's randomize flag */
    char _pad2b8[0x2f0 - 0x2b8];
    /* Three rungs of the quarter-step ladder, one per readout that asks for
       one (@0x465000, @0x471170 and @0x42d080 in turn). */
    int  f2f0;       /* 0x2f0 */
    char _pad2f4[0x304 - 0x2f4];
    /* The scratch panel's copy of the f4f4 gate - GFPanel::m4626d0 reads it
       where Panel::m4298c0 reads f4f4. */
    int  f304;       /* 0x304 */
    char _pad308[0x360 - 0x308];
    /* The per-judge gauge-rate carrier the catch commit multiplies from. */
    void *f360;      /* 0x360 */
    /* The object CatchPanel::init asks for the run's timing; only its slot 8
       is used, and only the third word it fills. */
    struct SlotTiming *f364;   /* 0x364 */
    struct SlotClock  *clock;  /* 0x368 - CatchPanel::update stamps its now() */
    char _pad36c[0x3d0 - 0x36c];
    struct SlotOwner *f3d0;    /* 0x3d0 - the run's owner */
    /* The sink the note-run walkers open lanes through - the same object
       the owner keeps at its own 0x148. */
    struct SlotSampleSink *f3d4;  /* 0x3d4 */
    /* The song clock - Panel::m428c60 takes its slot 22 for the sway. */
    void *f3d8;      /* 0x3d8 */
    /* The judge - Panel::m42b470 asks it each lane's kind. */
    void *f3dc;      /* 0x3dc */
    char _pad3e0[0x408 - 0x3e0];
    /* The catch panel's three fade modes, tested in this order by
       CatchPanel::m46f3d0 @0x46f3d0 - its own set, not the 0x474 trio. */
    int  f408;       /* 0x408 */
    char _pad40c[0x410 - 0x40c];
    int  f410;       /* 0x410 */
    char _pad414[0x418 - 0x414];
    int  f418;       /* 0x418 */
    char _pad41c[0x438 - 0x41c];
    /* The catch panel's randomize gate - CatchPanel::resetLanes @0x46f590
       reads it where Panel::resetLanes reads f4a4. */
    int  f438;       /* 0x438 */
    char _pad43c[0x474 - 0x43c];
    /* Three mutually-exclusive fade modes, tested in this order by
       Panel::m4270a0 @0x4270a0 and its GFPanel twin. */
    int  f474;       /* 0x474 */
    char _pad478[0x47c - 0x478];
    int  f47c;       /* 0x47c */
    char _pad480[0x484 - 0x480];
    int  f484;       /* 0x484 */
    char _pad488[0x494 - 0x488];
    /* Nonzero debits a hold's width by 24 rows instead of 12 when its
       judged advance lands (Panel::m42a630). */
    int  f494;       /* 0x494 */
    char _pad498[0x4a4 - 0x498];
    int  f4a4;       /* 0x4a4 - nonzero asks for a randomized scroll rate */
    char _pad4a8[0x4e0 - 0x4a8];
    int  f4e0;       /* 0x4e0 */
    char _pad4e4[0x4f4 - 0x4e4];
    /* Either of these asks the walkers to reskin notes per st.offset. */
    int  f4f4;       /* 0x4f4 */
    char _pad4f8[0x51c - 0x4f8];
    int  f51c;       /* 0x51c */
    char _pad520[0x534 - 0x520];
    int  f534;       /* 0x534 - gates Panel::m427750 entirely */
    char _pad538[0x54c - 0x538];
    int  f54c;       /* 0x54c - the alt mode's CRASH count */
    int  f550;       /* 0x550 */

    /* The catch judge commit: combo, score (with the log10 bonus), the
       gauge, and the game-over decisions. */
    void m472fc0(int judge, int n, int flag);    /* 0x472fc0 */

};

/* THE TEXTURE-RUN SECTION - the .gds record eight identical loaders read, two
   per panel plus two more in Panel itself: an id, a float pair, a numbered
   texture run, a second float pair, one more float, a name, and an int pair.
   The run's LENGTH is the only thing that varies between them (ten or eleven),
   which is what makes the record 0x88 or 0x8c bytes; everything else - the
   field order and the order the loader reads them in - is identical. */
/* Two more records the same eight-way way: a plain rectangle with one texture
   (0x18), and one with a texture between the two float pairs plus a third
   loaded from the parser's own token text (0x20). Each has one loader per
   panel - three and six respectively. */
struct GdsRectTex
{
    int       f00;    /* 0x00 */
    float     f04;    /* 0x04 */
    float     f08;    /* 0x08 */
    float     f0c;    /* 0x0c */
    float     f10;    /* 0x10 */
    KTexture *f14;    /* 0x14 */
};

struct GdsRectTex3
{
    int       f00;    /* 0x00 */
    float     f04;    /* 0x04 */
    float     f08;    /* 0x08 */
    KTexture *f0c;    /* 0x0c */
    float     f10;    /* 0x10 */
    float     f14;    /* 0x14 */
    KTexture *f18;    /* 0x18 */
    KTexture *f1c;    /* 0x1c - by name from the parser's token text */
};

/* The GrooveLight record - m470ee0 @0x470ee0. Two rectangles with a texture
   each rather than one, which is what makes it 0x2c instead of GdsRectTex3's
   0x20. Its loader appends: it indexes by the running count at +0x1b54 and
   bumps it, so a .gds may carry several GrooveLight sections. */
/* THE BOMB RECORD - 0x128 bytes, three per CatchPanel (CoolBomb, GoodBomb,
   LongNoteBomb) and the same shape Panel spells out flat at 0x1588 / 0x16b0.
   A texture run selected from a semicolon-separated list of name variants,
   plus two per-lane state arrays the loader resets. Its size is self-checking:
   3 * 0x128 is exactly the 0x378 between CatchPanel's 0x156c and the
   CoolCombo record at 0x18e4. */
struct GdsBomb
{
    int       f00;         /* 0x00 */
    int       f04;         /* 0x04 */
    int       f08;         /* 0x08 - how many of the run, clamped to 32 */
    float     f0c;         /* 0x0c */
    float     f10;         /* 0x10 */
    float     f14;         /* 0x14 */
    KTexture *f18[32];     /* 0x18 .. 0x98 */
    int       f98[18];     /* 0x98 .. 0xe0 - per lane, reset to -1 */
    int       fe0[18];     /* 0xe0 .. 0x128 - per lane, reset to 0 */
};

struct GdsGrooveLight
{
    int       f00;    /* 0x00 */
    float     f04;    /* 0x04 */
    float     f08;    /* 0x08 */
    float     f0c;    /* 0x0c */
    float     f10;    /* 0x10 */
    KTexture *f14;    /* 0x14 */
    float     f18;    /* 0x18 */
    float     f1c;    /* 0x1c */
    float     f20;    /* 0x20 */
    float     f24;    /* 0x24 */
    KTexture *f28;    /* 0x28 */
};

/* The same 0x88 section as GdsTexSection10/11 but with NO lone float after the
   second pair - eleven textures, and the four bytes go to the run instead.
   GFPanel is the only panel with a pair of these. */
struct GdsTexSection11b
{
    int       f00;         /* 0x00 */
    float     f04;         /* 0x04 */
    float     f08;         /* 0x08 */
    float     f0c;         /* 0x0c */
    float     f10;         /* 0x10 */
    KTexture *f14[11];     /* 0x14 .. 0x40 */
    char      name[0x40];  /* 0x40 .. 0x80 */
    int       f80;         /* 0x80 */
    int       f84;         /* 0x84 */
};

/* Two rectangles-with-textures that have no cross-panel twin: three textures
   (Panel) and two (GFPanel). */
struct GdsRect3Tex
{
    int       f00;    /* 0x00 */
    float     f04;    /* 0x04 */
    float     f08;    /* 0x08 */
    float     f0c;    /* 0x0c */
    float     f10;    /* 0x10 */
    KTexture *f14;    /* 0x14 */
    KTexture *f18;    /* 0x18 */
    KTexture *f1c;    /* 0x1c */
};

struct GdsRect2Tex
{
    int       f00;    /* 0x00 */
    float     f04;    /* 0x04 */
    float     f08;    /* 0x08 */
    float     f0c;    /* 0x0c */
    float     f10;    /* 0x10 */
    KTexture *f14;    /* 0x14 */
    KTexture *f18;    /* 0x18 */
};

struct GdsTexSection10
{
    int       f00;         /* 0x00 */
    float     f04;         /* 0x04 */
    float     f08;         /* 0x08 */
    float     f0c;         /* 0x0c */
    float     f10;         /* 0x10 */
    float     f14;         /* 0x14 */
    KTexture *f18[10];     /* 0x18 .. 0x40 */
    char      name[0x40];  /* 0x40 .. 0x80 */
    int       f80;         /* 0x80 */
    int       f84;         /* 0x84 */
};

struct GdsTexSection11
{
    int       f00;         /* 0x00 */
    float     f04;         /* 0x04 */
    float     f08;         /* 0x08 */
    float     f0c;         /* 0x0c */
    float     f10;         /* 0x10 */
    float     f14;         /* 0x14 */
    KTexture *f18[11];     /* 0x18 .. 0x44 */
    char      name[0x40];  /* 0x44 .. 0x84 */
    int       f84;         /* 0x84 */
    int       f88;         /* 0x88 */
};

struct Panel : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x426fa0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x426fb0 */
    virtual void           update();           /* 0x42d5c0 - the per-frame draw */
    void m42b470();                            /* 0x42b470 - the item pools + hold glows */
    void m427490();                            /* 0x427490 - the scroll ghost trail */
    void m429170();                            /* 0x429170 (STUB) */
    void m4280a0();                            /* 0x4280a0 - the judgement marker */
    void m42b910();                            /* 0x42b910 - the CoolCombo readout */
    void m4282b0();                            /* 0x4282b0 - the CoolBomb */
    void m4285c0();                            /* 0x4285c0 - the second bomb */
    void m4288d0();                            /* 0x4288d0 - the LongNoteBomb */
    virtual void           update2(int, unsigned int dt); /* 0x429dd0 - the per-frame tick */
    void setScrollRate(float r);               /* 0x429b20 */
    void m427300(unsigned int dt);             /* 0x427300 - the bouncing marker's tick */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~Panel();                                  /* 0x4271a0 */

    int loadGdsItem(struct KParser *p, int index);  /* 0x427820 */
    /* The fade colour for the current window - white with a computed alpha. */
    unsigned int m4270a0(unsigned int from, unsigned int to);  /* 0x4270a0 */

    int        f10;            /* 0x0010 */
    PanelSlot *slot;           /* 0x0014 */
    int        f18;            /* 0x0018 */
    int        f1c;            /* 0x001c */
    int        f20;            /* 0x0020 - the CoolCombo readout is gated on it */
    int        f24;            /* 0x0024 */
    /* The same counter/period pair GFPanel keeps at 0x24/0x28. */
    unsigned int f28;          /* 0x0028 */
    unsigned int f2c;          /* 0x002c */
    int        f30;            /* 0x0030 - 1 */
    int        f34;            /* 0x0034 */
    int        f38;            /* 0x0038 - 4 */
    int        f3c[40];        /* 0x003c .. 0x00dc - all -1 */
    float      fdc;            /* 0x00dc - the sway's centre... */
    float      fe0;            /* 0x00e0 - ...and amplitude: update sets fe4 = fdc + fe0 * cos(phase + pi) */
    float      fe4;            /* 0x00e4 */
    float      fe8;            /* 0x00e8 - 512 */
    float      fec[6];         /* 0x00ec .. 0x0104 */
    int        f104;           /* 0x0104 - 1 */
    KTexture  *f108;           /* 0x0108 - "System\\ModeSelect\\gray.bmp" */
    int        f10c;           /* 0x010c */
    /* LoadPVI @0x42d700 loads "System\\common\\auto.bmp" into f110 and
       derives f114 from the texture's own width. */
    KTexture  *f110;           /* 0x0110 */
    float      f114;           /* 0x0114 */
    char       b118;           /* 0x0118 */
    char _pad119[0x11c - 0x119];
    float      f11c;           /* 0x011c - 640, the screen width */
    float      f120;           /* 0x0120 - 480, the screen height */
    float      f124;           /* 0x0124 */
    float      f128;           /* 0x0128 */
    int        f12c;           /* 0x012c */
    int        f130;           /* 0x0130 */
    /* loadGdsItem @0x427820: eighteen .gds items, the same 0x104-byte record
       Obj46F660 owns (include/gdsitem.h) - 18 * 0x104 is exactly the 0x1248
       this pad used to cover. */
    struct GdsItem items[18];  /* 0x0134 .. 0x137c */
    /* MeasureLine - m427cf0 @0x427cf0, the twin of GFPanel's m460f00. */
    int        f137c;          /* 0x137c */
    int        f1380;          /* 0x1380 */
    float      f1384;          /* 0x1384 */
    float      f1388;          /* 0x1388 */
    float      f138c;          /* 0x138c - 0 means "take the texture's" */
    KTexture  *f1390[6];       /* 0x1390 .. 0x13a8 */
    char _pad13a8[0x1410 - 0x13a8];
    /* m427f30 @0x427f30: five .str resources read in DESCENDING address order,
       a float pair, and the chart pair the level meter draws from. */
    int        f1410;          /* 0x1410 */
    float      f1414;          /* 0x1414 */
    float      f1418;          /* 0x1418 */
    /* With the five below, the six press-flash heads m42d370 indexes as
       (&f141c)[kind]. */
    Obj402860Head *f141c;      /* 0x141c */
    Obj402860Head *f1420;      /* 0x1420 */
    Obj402860Head *f1424;      /* 0x1424 */
    Obj402860Head *f1428;      /* 0x1428 */
    Obj402860Head *f142c;      /* 0x142c */
    Obj402860Head *f1430;      /* 0x1430 */
    char _pad1434[0x1440 - 0x1434];
    Obj402860 *f1440;          /* 0x1440 - g_panelCharts1 */
    /* m428000 @0x428000: six single textures. */
    int        f1444;          /* 0x1444 */
    KTexture  *f1448[6];       /* 0x1448 .. 0x1460 */
    /* CoolBomb - m428150 @0x428150. Panel spells its other two bombs out flat
       at 0x1588 and 0x16b0; this one is the same 0x128 record, and s1460.f08
       is the run count MeasureLine clamps. */
    GdsBomb    s1460;          /* 0x1460 .. 0x1588 */
    /* m428450 @0x428450: the section whose texture run's base name is picked
       out of a semicolon-separated list, capped at 32, and which resets two
       parallel per-lane arrays afterwards. */
    int        f1588;          /* 0x1588 */
    int        f158c;          /* 0x158c */
    int        f1590;          /* 0x1590 - how many of the run, clamped to 32 */
    float      f1594;          /* 0x1594 */
    float      f1598;          /* 0x1598 */
    float      f159c;          /* 0x159c */
    KTexture  *f15a0[32];      /* 0x15a0 .. 0x1620 */
    int        f1620[18];      /* 0x1620 .. 0x1668 */
    int        f1668[18];      /* 0x1668 .. 0x16b0 */
    /* m428760 @0x428760: m428450's twin - the same list-picked texture run,
       with both per-lane arrays reset to zero and no 3-or-4 store. */
    int        f16b0;          /* 0x16b0 */
    int        f16b4;          /* 0x16b4 */
    int        f16b8;          /* 0x16b8 - clamped to 32 */
    float      f16bc;          /* 0x16bc */
    float      f16c0;          /* 0x16c0 */
    float      f16c4;          /* 0x16c4 */
    KTexture  *f16c8[32];      /* 0x16c8 .. 0x1748 */
    int        f1748[18];      /* 0x1748 .. 0x1790 */
    int        f1790[18];      /* 0x1790 .. 0x17d8 */
    /* CoolCombo - m42b830 @0x42b830, GFPanel's m463ff0 twin. */
    int            f17d8;      /* 0x17d8 */
    KTexture      *f17dc[10];  /* 0x17dc .. 0x1804 */
    Obj402860Head *f1804;      /* 0x1804 */
    Obj402860Head *f1808;      /* 0x1808 */
    Obj402860Head *f180c;      /* 0x180c */
    Obj402860Head *f1810;      /* 0x1810 */
    float          f1814;      /* 0x1814 */
    float          f1818;      /* 0x1818 */
    int            f181c;      /* 0x181c - the combo the cells were last set for */
    Obj402860     *f1820;      /* 0x1820 */
    GdsRectTex      s1824;       /* 0x1824 - m428a80 */
    char _pad183c[0x1840 - 0x183c];
    /* TargetBar - m428ae0 @0x428ae0, GFPanel's m461c60 twin. LoadPVI reads
       f1840 and f1848 back out of it to place the auto marker. */
    int        f1840;          /* 0x1840 - gates the auto marker's width */
    float      f1844;          /* 0x1844 */
    float      f1848;          /* 0x1848 - what f114 is measured back from */
    float      f184c;          /* 0x184c - 0 means "take the texture's" */
    float      f1850;          /* 0x1850 - likewise */
    KTexture  *f1854[6];       /* 0x1854 .. 0x186c */
    char _pad186c[0x18d4 - 0x186c];
    float      f18d4;          /* 0x18d4 */
    float      f18d8;          /* 0x18d8 */
    float      f18dc;          /* 0x18dc */
    float      f18e0;          /* 0x18e0 */
    /* MaxCoolCombo - m428d80 @0x428d80. GdsTexSection10's field order with a
       0x80 name buffer instead of 0x40, which is what makes it 0xc8 - and
       0x18e4 + 0xc8 is exactly where the next section starts. */
    int        f18e4;          /* 0x18e4 */
    float      f18e8;          /* 0x18e8 */
    float      f18ec;          /* 0x18ec */
    float      f18f0;          /* 0x18f0 */
    float      f18f4;          /* 0x18f4 */
    float      f18f8;          /* 0x18f8 */
    KTexture  *f18fc[10];      /* 0x18fc .. 0x1924 */
    char       n1924[0x80];    /* 0x1924 .. 0x19a4 */
    int        f19a4;          /* 0x19a4 */
    int        f19a8;          /* 0x19a8 */
    /* m428e10 / m428ea0 / m428f30 / m428fc0 @0x428e10..0x428fc0 - four of the
       eight identical texture-run sections; the last two carry eleven. */
    GdsTexSection10 s19ac;     /* 0x19ac */
    GdsTexSection10 s1a34;     /* 0x1a34 */
    GdsTexSection11 s1abc;     /* 0x1abc */
    GdsTexSection11 s1b48;     /* 0x1b48 .. 0x1bd4 */
    GdsRectTex3     s1bd4;       /* 0x1bd4 - m429050 */
    char _pad1bf4[0x1bf8 - 0x1bf4];
    float      f1bf8;          /* 0x1bf8 */
    float      f1bfc;          /* 0x1bfc */
    int        f1c00;          /* 0x1c00 */
    int        f1c04;          /* 0x1c04 */
    GdsRectTex3     s1c08;       /* 0x1c08 - m4290e0 */
    /* m42ce40 @0x42ce40: the section whose texture run's LENGTH is read out of
       the .gds first, and which ends by attaching a chart pair and seeding its
       position from the pair of floats it just read. */
    int        f1c28;          /* 0x1c28 */
    float      f1c2c;          /* 0x1c2c */
    float      f1c30;          /* 0x1c30 */
    Obj402860Head *f1c34;      /* 0x1c34 */
    float      f1c38;          /* 0x1c38 */
    float      f1c3c;          /* 0x1c3c */
    float      f1c40;          /* 0x1c40 */
    float      f1c44;          /* 0x1c44 */
    int        f1c48;          /* 0x1c48 */
    int        f1c4c;          /* 0x1c4c - how many of the run below to load */
    KTexture  *f1c50[32];      /* 0x1c50 .. 0x1cd0 */
    Obj402860 *f1cd0;          /* 0x1cd0 - g_panelCharts5 */
    int        f1cd4;          /* 0x1cd4 - LoadPVI zeroes it before the loop */
    /* GrooveLight, appended to by m429400 and counted by f1cd4 above - the
       third copy of the pair GFPanel has at 0x1af4 and CatchPanel at 0x1b58. */
    GdsGrooveLight s1cd8[2];   /* 0x1cd8 .. 0x1d30 */
    /* What m4294a0 @0x4294a0 reads out of the .gds: three .str resources, five
       floats, an eleven-texture run, a name, an int pair, and three pointers
       into the global chart array picked by this panel's slot. */
    int        f1d30;          /* 0x1d30 */
    Obj402860Head *f1d34;      /* 0x1d34 */
    Obj402860Head *f1d38;      /* 0x1d38 */
    Obj402860Head *f1d3c;      /* 0x1d3c */
    float      f1d40;          /* 0x1d40 */
    float      f1d44;          /* 0x1d44 */
    float      f1d48;          /* 0x1d48 */
    float      f1d4c;          /* 0x1d4c */
    float      f1d50;          /* 0x1d50 */
    KTexture  *f1d54[11];      /* 0x1d54 .. 0x1d80 */
    char       f1d80[0x40];    /* 0x1d80 */
    int        f1dc0;          /* 0x1dc0 */
    int        f1dc4;          /* 0x1dc4 */
    /* Three of the eight global chart pairs the destructor also clears, picked
       by this panel's slot (src/gamepanels.cpp). */
    Obj402860 *f1dc8;          /* 0x1dc8 - g_panelCharts2 */
    Obj402860 *f1dcc;          /* 0x1dcc - g_panelCharts0 */
    Obj402860 *f1dd0;          /* 0x1dd0 - g_panelCharts6 */

    /* Re-attach the three chart heads, on mode 2 and only when the gate at
       +0x1d30 is set - GFPanel::m4627c0 @0x4627c0's twin, with two extra
       guards on the cabinet's own inputs (src/gamepanels.cpp). */
    void m429aa0(int, int mode);               /* 0x429aa0 */
    /* ComboEffect - m429650 @0x429650. */
    int            f1dd4;      /* 0x1dd4 - overwritten with a global */
    Obj402860Head *f1dd8;      /* 0x1dd8 */
    Obj402860Head *f1ddc;      /* 0x1ddc */
    Obj402860Head *f1de0;      /* 0x1de0 */
    Obj402860     *f1de4;      /* 0x1de4 */
    /* RubyKeyPanel - m4296e0 @0x4296e0. */
    int            f1de8;      /* 0x1de8 */
    float          f1dec;      /* 0x1dec */
    float          f1df0;      /* 0x1df0 */
    float          f1df4;      /* 0x1df4 */
    float          f1df8;      /* 0x1df8 */
    KTexture      *f1dfc;      /* 0x1dfc */
    Obj402860Head *f1e00;      /* 0x1e00 */
    Obj402860     *f1e04;      /* 0x1e04 */
    /* And what m429770 @0x429770 reads: eleven six-texture runs, each in a
       32-pointer slot of its own. */
    int        f1e08;          /* 0x1e08 */
    KTexture  *f1e0c[8][32];   /* 0x1e0c .. 0x220c */
    char _pad220c[0x268c - 0x220c];
    KTexture  *f268c[32];      /* 0x268c */
    char _pad270c[0x278c - 0x270c];
    KTexture  *f278c[32];      /* 0x278c */
    KTexture  *f280c[32];      /* 0x280c */
    /* SpecialNote - m429890 @0x429890, eleven instructions like GFPanel's. */
    KTexture  *f288c[6];       /* 0x288c .. 0x28a4 */
    char _pad28a4[0x320c - 0x28a4];
    int        f320c;          /* 0x320c - the mode m429950 latches */
    GdsRect3Tex       s3210;      /* 0x3210 - m4295c0 */
    int        f3230[6];       /* 0x3230 .. 0x3248 */
    int        f3248[0x12];    /* 0x3248 .. 0x3290 */
    float      f3290;          /* 0x3290 - reset to 1 by resetLanes */
    unsigned int f3294;        /* 0x3294 - update2's timeGetTime stamp */
    Obj402860 *optional;       /* 0x3298 - cleared only when it is set */
    Obj402860Head *f329c;      /* 0x329c - what `optional` shows on a press */
    Obj402860  m_32a0;         /* 0x32a0 */
    Obj402860  m_34bc;         /* 0x34bc */

    Panel();                                   /* 0x429b70 */
    /* Give every lane of the run its scroll rate, randomized per lane when the
       slot asks for it. */
    void m4298c0();                            /* 0x4298c0 */
    void m429950(int mode, int a, int b);      /* 0x429950 */
    void m429910(int lane, int on);            /* 0x429910 */
    void m428c60();                            /* 0x428c60 */
    void resetLanes();                         /* 0x427680 */
    int m4294a0(struct KParser *p);            /* 0x4294a0 */
    int m429770(struct KParser *p);            /* 0x429770 */
    int m427f30(struct KParser *p);            /* 0x427f30 */
    int m428000(struct KParser *p);            /* 0x428000 */
    int m42ce40(struct KParser *p);            /* 0x42ce40 */
    int m428450(struct KParser *p);            /* 0x428450 */
    int m428760(struct KParser *p);            /* 0x428760 */
    int m428e10(struct KParser *p);            /* 0x428e10 */
    int m428ea0(struct KParser *p);            /* 0x428ea0 */
    int m428f30(struct KParser *p);            /* 0x428f30 */
    int m428fc0(struct KParser *p);            /* 0x428fc0 */
    int m428a80(struct KParser *p);            /* 0x428a80 */
    int m429050(struct KParser *p);            /* 0x429050 */
    int m4290e0(struct KParser *p);            /* 0x4290e0 */
    int m4295c0(struct KParser *p);            /* 0x4295c0 */

    /* The .gds reader - twenty-seven section arms, the widest of the three.
       See src/panelgds.cpp. */
    int  LoadPVI(char *name);                  /* 0x42d700 */
    void init(PanelSlot *s);                   /* 0x42a0c0 - attach to a slot */
    void m427750();                            /* 0x427750 */
    /* Draw the auto-play badge over one item of the table. */
    void m427c50(int n);                       /* 0x427c50 */
    /* The counter readout: right-aligned, one character width per extra
       digit, and only in mix style 7. */
    void m42bd80();                            /* 0x42bd80 */
    /* Draw every armed item that is fading in: its blend pair, then the
       texture at its scrolled-and-rounded x. */
    void m42a580();                            /* 0x42a580 */
    /* Advance every armed item's fade and draw it (GFPanel::m463030's
       twin). */
    void m42a450();                            /* 0x42a450 */
    /* Walk one item's note run from `start` and draw every hit before the
       window closes, reskinning per-note when the slot asks. */
    void m42b2f0(int item, int start, int col);  /* 0x42b2f0 */
    /* Draw the measure lines: one texture per line across the visible
       window, phased by the run position against the beat lane. */
    void m427d90();                              /* 0x427d90 */
    /* Draw the lane frame (item 0's blend and the screen rect) and each
       item's two guide lines. */
    void m42a240();                              /* 0x42a240 */
    /* Draw the gauge: its plate, the chart clip, and the bar as a partial
       sprite span - flashing in random colors once the level tops 100. */
    void m42c0f0();                              /* 0x42c0f0 */
    /* The second gauge, on s1c08: the same bar plus the audience-rate
       danger tree and mix style 7's rotating dial. */
    void m42c5f0();                              /* 0x42c5f0 */
    /* Draw the measure grid across the visible window, re-segmented at
       every tempo change the sink reports. */
    void m42b550();                              /* 0x42b550 */
    int  m42a630(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x42a630 (STUB) */
    int  m42af30(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x42af30 (STUB) */
    /* The same again on the next record, with six digit widths instead of
       four and its own slot counter. */
    void m42bf30();                            /* 0x42bf30 */
    /* The single-texture section m428a80 loads, plus the chart it parks at
       the panel's own x. */
    void m42bbe0();                            /* 0x42bbe0 */
    /* The six-frame cycling marker. */
    void m42bce0();                            /* 0x42bce0 */
    /* Park the chart at the panel's origin, and if it says so draw one
       texture beside it. */
    void m42d290();                            /* 0x42d290 */
    /* Draw the GrooveLight records: a solid pass and a fading one. */
    void m42cf40();                            /* 0x42cf40 */
    /* The speed readout: the mode's own rate above ten, the player's percent
       below it. */
    void m42d080();                            /* 0x42d080 */
    /* The same record again, printing the mode's own rate rather than the
       player's percent. */
    void m42d190();                            /* 0x42d190 */
    /* The two vertical edges of a rectangle. */
    void m427bd0(float x, float y, float w, float h, int c0, int c1);
                                               /* 0x427bd0 */
    int  m427cf0(struct KParser *p);           /* 0x427cf0 - MeasureLine   */
    int  m428150(struct KParser *p);           /* 0x428150 - CoolBomb      */
    int  m42b830(struct KParser *p);           /* 0x42b830 - CoolCombo     */
    int  m428ae0(struct KParser *p);           /* 0x428ae0 - TargetBar     */
    int  m428d80(struct KParser *p);           /* 0x428d80 - MaxCoolCombo  */
    int  m429400(struct KParser *p);           /* 0x429400 - GrooveLight   */
    int  m429650(struct KParser *p);           /* 0x429650 - ComboEffect   */
    int  m4296e0(struct KParser *p);           /* 0x4296e0 - RubyKeyPanel  */
    int  m429890(struct KParser *p);           /* 0x429890 - SpecialNote   */
};

struct GFPanel : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x4607a0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x4607b0 */
    virtual void           update();           /* 0x465380 - the per-frame draw */
    void m462450();                            /* 0x462450 - the full-life marker */
    void m463cc0();                            /* 0x463cc0 - the item pools */
    void m461990();                            /* 0x461990 - the CoolCombo readout */
    void m4611c0();                            /* 0x4611c0 - the CoolBomb */
    void m4614d0();                            /* 0x4614d0 - the second bomb */
    void m4617e0();                            /* 0x4617e0 - the LongNoteBomb */
    virtual void           update2(int, int);  /* 0x462af0 - the per-frame tick */
    void setScrollRate(float r);               /* 0x462820 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    /* Its ctor is still parked in wip/gfpanel-ctor.cpp at 96.18%; the
       factory only needs the call, so the body is a STUB for now. */
    GFPanel();                                 /* 0x462870 (src/gfpanelctor.cpp) */
    ~GFPanel();                                /* 0x4608c0 */

    int loadGdsItem(struct KParser *p, int index);  /* 0x460b50 */
    void resetLanes();                              /* 0x4609f0 */
    void m4626d0();                                 /* 0x4626d0 */
    void m462720(int lane, int on);                 /* 0x462720 */
    void m461de0();                                 /* 0x461de0 */
    void m4627c0(int a, int mode);                  /* 0x4627c0 */

    int        f10;            /* 0x0010 */
    PanelSlot *slot;           /* 0x0014 */
    int        f18;            /* 0x0018 */
    int        f1c;            /* 0x001c - the CoolCombo readout is gated on it */
    int        f20;            /* 0x0020 */
    /* A frame counter and its period: m4641c0 turns them into a six-step
       cycle and picks a texture with it. */
    unsigned int f24;          /* 0x0024 */
    unsigned int f28;          /* 0x0028 */
    /* Panel::m4270a0's twin - the fade colour, over this class's own period
       at +0x28. */
    unsigned int m4607c0(unsigned int from, unsigned int to);  /* 0x4607c0 */
    int        f2c;            /* 0x002c - 1 */
    int        f30;            /* 0x0030 */
    int        f34;            /* 0x0034 - 4 */
    int        f38[40];        /* 0x0038 .. 0x00d8 - all -1 */
    float      fd8;            /* 0x00d8 - the sway's centre and amplitude, as Panel's fdc/fe0 */
    float      fdc;            /* 0x00dc */
    float      fe0;            /* 0x00e0 - m463ff0 offsets a chart by it */
    char       be4;            /* 0x00e4 */
    char _pade5[0xe8 - 0xe5];
    float      fe8;            /* 0x00e8 - 640, the screen width */
    float      fec;            /* 0x00ec - 480, the screen height */
    float      ff0;            /* 0x00f0 */
    float      ff4;            /* 0x00f4 */
    int        ff8;            /* 0x00f8 */
    struct GdsItem items[18];  /* 0x00fc .. 0x1344 - see Panel::items */

    /* Panel::m42a580's twin: draw every armed fading-in item. */
    void m463160();            /* 0x463160 */
    /* Advance every armed item's fade (in toward f5c by f60, out by f64 to
       zero) and draw it at its faded height. */
    void m463030();            /* 0x463030 */
    /* Panel::m42a240's twin: the lane frame and the guide lines. */
    void m462e00();                              /* 0x462e00 */
    /* Panel::m42c0f0's twin: the gauge, with the ready/danger state pushed
       into the stage's owner and the flash drawn from the named texture. */
    void m464580();                              /* 0x464580 */
    /* Panel::m42b550's twin: the tempo-segmented measure grid. */
    void m463d30();                              /* 0x463d30 */
    /* The second GF gauge, on s1ad0: chases the stage's own level and owns
       the shared beat latch. */
    void m4649c0();                              /* 0x4649c0 */
    /* Panel::m42b2f0's twin: walk one item's note run and draw its hits. */
    void m463b40(int item, int start, int col);  /* 0x463b40 */
    int  m463210(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x463210 (STUB) */
    int  m4637f0(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x4637f0 (STUB) */
    /* MeasureLine - m460f00 @0x460f00. A six-texture run whose height is
       filled in from the first texture when the .gds leaves it at zero. */
    int        f1344;          /* 0x1344 */
    int        f1348;          /* 0x1348 */
    float      f134c;          /* 0x134c */
    float      f1350;          /* 0x1350 */
    float      f1354;          /* 0x1354 - 0 means "take the texture's" */
    KTexture  *f1358[6];       /* 0x1358 .. 0x1370 */
    char _pad1370[0x13d8 - 0x1370];
    /* Judgment - m460fa0 @0x460fa0. Five .str resources read in DESCENDING
       address order, the same way Panel::m427f30 reads its five. */
    int            f13d8;      /* 0x13d8 */
    float          f13dc;      /* 0x13dc */
    float          f13e0;      /* 0x13e0 */
    /* With the five below, the six press-flash heads m4651a0 indexes as
       (&f13e4)[kind]. */
    Obj402860Head *f13e4;      /* 0x13e4 */
    Obj402860Head *f13e8;      /* 0x13e8 - read LAST */
    Obj402860Head *f13ec;      /* 0x13ec */
    Obj402860Head *f13f0;      /* 0x13f0 */
    Obj402860Head *f13f4;      /* 0x13f4 */
    Obj402860Head *f13f8;      /* 0x13f8 - read FIRST */
    Obj402860     *f13fc;      /* 0x13fc */
    /* The three bombs, back to back - m461060 / m461360 / m461670, the same
       0x128 GdsBomb the other two panels carry. s1400.f08 is the run count
       MeasureLine clamps, which is why that clamp reads as belonging to no
       section of its own. */
    GdsBomb    s1400;          /* 0x1400 - CoolBomb     */
    GdsBomb    s1528;          /* 0x1528 - GoodBomb     */
    GdsBomb    s1650;          /* 0x1650 - LongNoteBomb */
    /* CoolCombo - m463ff0 @0x463ff0. The 0x44 record CatchPanel also has at
       0x18e4: a ten-texture digit run, four .str resources and a float pair
       that is copied into the chart the slot selects. */
    int            f1778;      /* 0x1778 */
    KTexture      *f177c[10];  /* 0x177c .. 0x17a4 */
    Obj402860Head *f17a4;      /* 0x17a4 */
    Obj402860Head *f17a8;      /* 0x17a8 */
    Obj402860Head *f17ac;      /* 0x17ac */
    Obj402860Head *f17b0;      /* 0x17b0 */
    float          f17b4;      /* 0x17b4 */
    float          f17b8;      /* 0x17b8 */
    int            f17bc;      /* 0x17bc - the combo the cells were last set for */
    Obj402860     *f17c0;      /* 0x17c0 */
    GdsRectTex      s17c4;       /* 0x17c4 - m461c00 */
    /* TargetBar - m461c60 @0x461c60. CatchPanel::m4709a0's twin plus three
       derived fields and a flag the loader raises for the extra cabinets. */
    int        f17dc;          /* 0x17dc */
    float      f17e0;          /* 0x17e0 */
    float      f17e4;          /* 0x17e4 */
    float      f17e8;          /* 0x17e8 - 0 means "take the texture's" */
    float      f17ec;          /* 0x17ec - likewise */
    KTexture  *f17f0[6];       /* 0x17f0 .. 0x1808 */
    char _pad1808[0x1870 - 0x1808];
    float      f1870;          /* 0x1870 */
    float      f1874;          /* 0x1874 */
    float      f1878;          /* 0x1878 */
    float      f187c;          /* 0x187c */
    /* m461ee0 / m461f70 @0x461ee0 / 0x461f70 - Panel's s19ac / s1a34 twins. */
    GdsTexSection10 s1880;     /* 0x1880 */
    GdsTexSection10 s1908;     /* 0x1908 .. 0x1990 */
    /* m462000 / m462080 @0x462000 / 0x462080. */
    GdsTexSection11b s1990;    /* 0x1990 */
    GdsTexSection11b s1a18;    /* 0x1a18 .. 0x1aa0 */
    GdsRectTex3     s1aa0;       /* 0x1aa0 - m462100 */
    /* The life level the gauge chases - GFPanel keeps its own copy. */
    float      f1ac0;          /* 0x1ac0 */
    float      f1ac4;          /* 0x1ac4 */
    float      f1ac8;          /* 0x1ac8 */
    int        f1acc;          /* 0x1acc */
    GdsRectTex3     s1ad0;       /* 0x1ad0 - m462190 */
    int        f1af0;          /* 0x1af0 - LoadPVI zeroes it before the loop */
    /* GrooveLight, appended to by m462220 and counted by f1af0 above - the
       same record and the same two-element array CatchPanel has at 0x1b58. */
    GdsGrooveLight s1af4[2];   /* 0x1af4 .. 0x1b4c */
    /* m4622c0 @0x4622c0 - Panel::m4294a0's twin, minus its single float. */
    /* Re-attach the three chart heads below to their charts, on mode 2 only
       and only when this gate is set - m4627c0 @0x4627c0. */
    int        f1b4c;          /* 0x1b4c */
    Obj402860Head *f1b50;      /* 0x1b50 */
    Obj402860Head *f1b54;      /* 0x1b54 */
    Obj402860Head *f1b58;      /* 0x1b58 */
    float      f1b5c;          /* 0x1b5c */
    float      f1b60;          /* 0x1b60 */
    float      f1b64;          /* 0x1b64 */
    float      f1b68;          /* 0x1b68 */
    KTexture  *f1b6c[11];      /* 0x1b6c .. 0x1b98 */
    char       f1b98[0x40];    /* 0x1b98 */
    int        f1bd8;          /* 0x1bd8 */
    int        f1bdc;          /* 0x1bdc */
    Obj402860 *f1be0;          /* 0x1be0 - g_gfPanelCharts4 */
    Obj402860 *f1be4;          /* 0x1be4 - g_gfPanelCharts5 */
    Obj402860 *f1be8;          /* 0x1be8 - g_gfPanelCharts0 */
    /* RubyKeyPanel - m4624f0 @0x4624f0. */
    int            f1bec;      /* 0x1bec */
    float          f1bf0;      /* 0x1bf0 */
    float          f1bf4;      /* 0x1bf4 */
    float          f1bf8;      /* 0x1bf8 */
    float          f1bfc;      /* 0x1bfc */
    KTexture      *f1c00;      /* 0x1c00 */
    Obj402860Head *f1c04;      /* 0x1c04 */
    Obj402860     *f1c08;      /* 0x1c08 */
    /* m462580 @0x462580 - Panel::m429770's twin, same eleven runs. */
    int        f1c0c;          /* 0x1c0c */
    KTexture  *f1c10[8][32];   /* 0x1c10 .. 0x2010 */
    char _pad2010[0x2490 - 0x2010];
    KTexture  *f2490[32];      /* 0x2490 */
    char _pad2510[0x2590 - 0x2510];
    KTexture  *f2590[32];      /* 0x2590 */
    KTexture  *f2610[32];      /* 0x2610 */
    /* SpecialNote - m4626a0 @0x4626a0, the whole eleven-instruction loader. */
    KTexture      *f2690[6];   /* 0x2690 .. 0x26a8 */
    char _pad26a8[0x3010 - 0x26a8];
    int        f3010;          /* 0x3010 - picks the second position of s3014 */
    GdsRect2Tex       s3014;      /* 0x3014 - m4623d0 */
    int        f3030[6];       /* 0x3030 .. 0x3048 */
    int        f3048[0x12];    /* 0x3048 .. 0x3090 */
    float      f3090;          /* 0x3090 - reset to 1 by resetLanes */
    unsigned int f3094;        /* 0x3094 - update2's timeGetTime stamp */
    void      *m_sample;       /* 0x3098 - "newdj.wav" */
    Obj402860  m_309c;         /* 0x309c */
    Obj402860  m_32b8;         /* 0x32b8 */
    char       b34d4;          /* 0x34d4 */
    char _pad34d5[0x34dc - 0x34d5];
    int m4622c0(struct KParser *p);            /* 0x4622c0 */
    int m462580(struct KParser *p);            /* 0x462580 */
    int m461ee0(struct KParser *p);            /* 0x461ee0 */
    int m461f70(struct KParser *p);            /* 0x461f70 */
    int m461c00(struct KParser *p);            /* 0x461c00 */
    int m462100(struct KParser *p);            /* 0x462100 */
    int m462190(struct KParser *p);            /* 0x462190 */
    int m462000(struct KParser *p);            /* 0x462000 */
    int m462080(struct KParser *p);            /* 0x462080 */
    int m4623d0(struct KParser *p);            /* 0x4623d0 */
    /* The three readouts the sections above drive: two through @0x40a970 with
       the SLOT's own scale, and the third through drawNumF with the live
       player's. */
    void m464250();                            /* 0x464250 */
    void m464300();                            /* 0x464300 */
    void m4644c0();                            /* 0x4644c0 */
    /* The bleeding level: decayed here and printed in the same breath. */
    void m4643b0();                            /* 0x4643b0 */
    /* The section m4622c0 loads, printed at the slot's own rung. */
    void m465000();                            /* 0x465000 */
    /* Panel::m42bbe0's twin, without the blend change. */
    void m4640d0();                            /* 0x4640d0 */
    /* Panel::m42bce0's twin. */
    void m4641c0();                            /* 0x4641c0 */
    /* Panel::m42d290's twin. */
    void m4650c0();                            /* 0x4650c0 */
    /* Panel::m42cf40's twin. */
    void m464ec0();                            /* 0x464ec0 */

    /* The .gds reader, twenty-three section arms - the widest of the three.
       Every loader below is one of them. */
    int  LoadPVI(char *name);                  /* 0x4654b0 */
    void init(PanelSlot *s);                   /* 0x462cb0 - attach to a slot */
    /* Shared: CatchPanel::LoadPVI calls this one too. It never touches
       `this`, which is what lets a second class borrow it. */
    int  m460ac0(struct KParser *p);           /* 0x460ac0 - GroupTexture       */
    int  m460fa0(struct KParser *p);           /* 0x460fa0 - Judgment           */
    int  m460f00(struct KParser *p);           /* 0x460f00 - MeasureLine        */
    int  m461060(struct KParser *p);           /* 0x461060 - CoolBomb           */
    int  m461670(struct KParser *p);           /* 0x461670 - LongNoteBomb       */
    int  m463ff0(struct KParser *p);           /* 0x463ff0 - CoolCombo          */
    int  m461c60(struct KParser *p);           /* 0x461c60 - TargetBar          */
    int  m462220(struct KParser *p);           /* 0x462220 - GrooveLight        */
    int  m4624f0(struct KParser *p);           /* 0x4624f0 - RubyKeyPanel       */
    int  m461360(struct KParser *p);           /* 0x461360 - GoodBomb           */
    int  m4626a0(struct KParser *p);           /* 0x4626a0 - SpecialNote        */

    /* The constructor @0x462870 is reconstructed but PARKED at 96.18% -
       wip/gfpanel-ctor.cpp. The layout above is what it writes, and the
       destructor confirms it. */
};

struct CatchPanel : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x46f2f0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x46f300 */
    virtual void           update();           /* 0x4724f0 - the per-frame draw */
    virtual void           update2(int, int);  /* 0x471590 - the per-frame tick */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */
    void setScrollRate(float r);               /* 0x471330 */
    void m470af0();                            /* 0x470af0 - the frame counter and the sway */

    ~CatchPanel();                             /* 0x46f480 */

    PanelSlot *slot;           /* 0x0010 */
    float      f14;            /* 0x0014 */
    float      f18;            /* 0x0018 */
    int        f1c;            /* 0x001c - the ctor leaves it alone */
    float      f20;            /* 0x0020 */
    float      f24;            /* 0x0024 */
    Obj402860  m_28;           /* 0x0028 - cleared explicitly before it goes */
    int        f244;           /* 0x0244 - 4 */
    int        f248;           /* 0x248 - the CoolCombo readout is gated on it */
    int        f24c;           /* 0x24c */
    /* The same counter/period pair the other two panels keep near +0x28. */
    unsigned int f250;         /* 0x0250 */
    unsigned int f254;         /* 0x0254 */
    int        f258[40];       /* 0x0258 .. 0x02f8 - all -1 */
    int        f2f8;           /* 0x02f8 - General, via readGdsInt */
    /* The .gds item table, filled one entry at a time by the `TrackN` sections
       of LoadPVI @0x472650. 18 * 0x104 = 0x1248 is exactly the span that used
       to be `int f2fc[0x492]`, and matches Panel's own eighteen items. */
    GdsItem    items[18];      /* 0x02fc .. 0x1544 */

    /* The three-mode fade colour, Panel::m4270a0's catch twin
       (src/catchfade.cpp). */
    unsigned int m46f3d0(unsigned int from, unsigned int to); /* 0x46f3d0 */
    void m46f8e0();                              /* 0x46f8e0 - the lane frame and guide lines */
    void m46fa50();                              /* 0x46fa50 - the item bars */
    void m46fb40();                              /* 0x46fb40 - the item glows */
    void m470370();                              /* 0x470370 - the CoolBomb */
    void m4707c0();                              /* 0x4707c0 - the LongNoteBomb */
    void m471830();                              /* 0x471830 - the CoolCombo readout */
    void m470070();                              /* 0x470070 */
    void m470f80();                              /* 0x470f80 - the GrooveLight records */
    void m471290(int lane, int on);              /* 0x471290 */
    void m4712d0(int a, int mode);               /* 0x4712d0 */
    /* The catch copy of Panel::resetLanes, over its own members. */
    void resetLanes();                           /* 0x46f590 */
    /* The note-run walker, Panel::m42b2f0 without the reskin gates. */
    void m46ff40(int item, int start, int col);  /* 0x46ff40 */
    /* The catch gauge - the family's horizontal variant. */
    void m471c20();                              /* 0x471c20 */
    /* The catch player-life gauge on s1b34: chases the slot's f1ec, owns
       the beat latch, raises the game-over flags, no flash. */
    void m472020();                              /* 0x472020 */
    int  m46fbb0(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x46fbb0 (STUB) */
    int  m46fe30(struct GdsItem *e, struct SlotState *st,
                 struct KTexture *voice, float t);  /* 0x46fe30 (STUB) */
    /* m4700f0 @0x4700f0: five .str resources whose ORDER depends on a global -
       the first and the last swap, the middle three do not. */
    int        f1544;          /* 0x1544 */
    float      f1548;          /* 0x1548 */
    float      f154c;          /* 0x154c */
    /* With the five below, the six press-flash heads m472300 indexes as
       (&f1550)[kind]. */
    Obj402860Head *f1550;      /* 0x1550 */
    Obj402860Head *f1554;      /* 0x1554 */
    Obj402860Head *f1558;      /* 0x1558 */
    Obj402860Head *f155c;      /* 0x155c */
    Obj402860Head *f1560;      /* 0x1560 */
    Obj402860Head *f1564;      /* 0x1564 */
    Obj402860 *f1568;          /* 0x1568 - g_catchPanelCharts2 */
    /* The three bombs, back to back - m470210 / m4704f0 / m470650. */
    GdsBomb    s156c;          /* 0x156c - CoolBomb     */
    GdsBomb    s1694;          /* 0x1694 - GoodBomb     */
    GdsBomb    s17bc;          /* 0x17bc - LongNoteBomb */
    /* CoolCombo - m471750 @0x471750. A ten-texture run, four .str resources,
       a float pair, and the chart of the pair the slot picks - into whose
       f214/f218 the loader copies that same float pair. */
    int            f18e4;      /* 0x18e4 */
    KTexture      *f18e8[10];  /* 0x18e8 .. 0x1910 */
    Obj402860Head *f1910;      /* 0x1910 */
    Obj402860Head *f1914;      /* 0x1914 */
    Obj402860Head *f1918;      /* 0x1918 */
    Obj402860Head *f191c;      /* 0x191c */
    float          f1920;      /* 0x1920 */
    float          f1924;      /* 0x1924 */
    int            f1928;      /* 0x1928 - the combo the cells were last set for */
    Obj402860     *f192c;      /* 0x192c */
    GdsRectTex      s1930;       /* 0x1930 - m470940 */
    /* TargetBar - m4709a0 @0x4709a0. Its geometry is read once per name
       variant and kept only from the last, and a zero width or height is
       filled in from the first texture of the run. */
    int        f1948;          /* 0x1948 */
    float      f194c;          /* 0x194c */
    float      f1950;          /* 0x1950 */
    float      f1954;          /* 0x1954 - 0 means "take the texture's" */
    float      f1958;          /* 0x1958 - likewise */
    KTexture  *f195c[6];       /* 0x195c .. 0x1974 */
    char _pad1974[0x19dc - 0x1974];
    float      f19dc;          /* 0x19dc */
    int        f19e0;          /* 0x19e0 */
    int        f19e4;          /* 0x19e4 */
    int        f19e8;          /* 0x19e8 */
    float      f19ec;          /* 0x19ec */
    float      f19f0;          /* 0x19f0 - the sway step, negated at either end */
    /* m470bc0 / m470cc0 @0x470bc0 / 0x470cc0 - the same pair again. */
    GdsTexSection10 s19f4;     /* 0x19f4 */
    GdsTexSection10 s1a7c;     /* 0x1a7c .. 0x1b04 */
    GdsRectTex3     s1b04;       /* 0x1b04 - m470dc0 */
    /* The life level the catch gauge chases. */
    float      f1b24;          /* 0x1b24 */
    float      f1b28;          /* 0x1b28 */
    float      f1b2c;          /* 0x1b2c */
    int        f1b30;          /* 0x1b30 */
    GdsRectTex3     s1b34;       /* 0x1b34 - m470e50 */
    int        f1b54;          /* 0x1b54 - LoadPVI zeroes it before the loop */
    /* GrooveLight, appended to by m470ee0 and counted by f1b54 above.
       2 * 0x2c is exactly the 0x58 this used to be as `int f1b58[0x16]`. */
    GdsGrooveLight s1b58[2];   /* 0x1b58 .. 0x1bb0 */
    /* m471060 @0x471060 - the third copy of Panel::m4294a0. */
    /* The catch copy's gate - m4712d0 @0x4712d0. */
    int        f1bb0;          /* 0x1bb0 */
    Obj402860Head *f1bb4;      /* 0x1bb4 */
    Obj402860Head *f1bb8;      /* 0x1bb8 */
    Obj402860Head *f1bbc;      /* 0x1bbc */
    float      f1bc0;          /* 0x1bc0 */
    float      f1bc4;          /* 0x1bc4 */
    float      f1bc8;          /* 0x1bc8 */
    float      f1bcc;          /* 0x1bcc */
    KTexture  *f1bd0[11];      /* 0x1bd0 .. 0x1bfc */
    char       f1bfc[0x40];    /* 0x1bfc */
    int        f1c3c;          /* 0x1c3c */
    int        f1c40;          /* 0x1c40 */
    Obj402860 *f1c44;          /* 0x1c44 - g_catchPanelCharts4 */
    Obj402860 *f1c48;          /* 0x1c48 - g_catchPanelCharts5 */
    Obj402860 *f1c4c;          /* 0x1c4c - g_catchPanelCharts0 */
    /* ComboEffect - m471200 @0x471200. An int that the loader immediately
       overwrites with a global, three .str resources, and the chart of the
       pair the panel's slot selects. */
    int            f1c50;      /* 0x1c50 */
    Obj402860Head *f1c54;      /* 0x1c54 */
    Obj402860Head *f1c58;      /* 0x1c58 */
    Obj402860Head *f1c5c;      /* 0x1c5c */
    Obj402860     *f1c60;      /* 0x1c60 */
    int        f1c64[6];       /* 0x1c64 .. 0x1c7c */
    float      f1c7c[0x12];    /* 0x1c7c .. 0x1cc4 - per-lane scroll factors */
    /* Reset to 1 by resetLanes, as Panel's f3290 and GFPanel's f3090 are. */
    float      f1cc4;          /* 0x1cc4 */
    unsigned int f1cc8;        /* 0x1cc8 - update2's timeGetTime stamp */
    struct KTexture *tex;      /* 0x1ccc - "p_opaque.bmp" */

    CatchPanel();                              /* 0x471380 */
    int m471060(struct KParser *p);            /* 0x471060 */
    /* Draw the section m471060 loads: a number scaled by the slot's own rung
       of the ladder. */
    void m471170();                            /* 0x471170 */
    /* The two readouts m470bc0 / m470cc0 load, each at the rung its own slot
       field asks for. */
    void m470c50();                            /* 0x470c50 */
    void m470d50();                            /* 0x470d50 */
    /* The third cycling marker, on the TargetBar record. */
    void m471b80();                            /* 0x471b80 */
    /* The plain rectangle m470940 loads, plus a chart refresh. */
    void m471ae0();                            /* 0x471ae0 */
    /* Attach to a slot and take the run's period from it. */
    void init(PanelSlot *s);                   /* 0x471710 */
    int m4700f0(struct KParser *p);            /* 0x4700f0 */
    int m470bc0(struct KParser *p);            /* 0x470bc0 */
    int m470cc0(struct KParser *p);            /* 0x470cc0 */
    int m470940(struct KParser *p);            /* 0x470940 */
    int m470dc0(struct KParser *p);            /* 0x470dc0 */
    int m470e50(struct KParser *p);            /* 0x470e50 */

    /* The .gds reader. LoadPVI names itself in its own error log, and every
       method below is one of its seventeen section arms. */
    int  LoadPVI(char *name);                  /* 0x472650 */
    int  loadItem(struct KParser *p, int i);   /* 0x46f660 - "TrackN"      */
    int  m470210(struct KParser *p);           /* 0x470210 - CoolBomb      */
    int  m470650(struct KParser *p);           /* 0x470650 - LongNoteBomb  */
    int  m471750(struct KParser *p);           /* 0x471750 - CoolCombo     */
    int  m4709a0(struct KParser *p);           /* 0x4709a0 - TargetBar     */
    int  m470ee0(struct KParser *p);           /* 0x470ee0 - GrooveLight   */
    int  m471200(struct KParser *p);           /* 0x471200 - ComboEffect   */
    int  m4704f0(struct KParser *p);           /* 0x4704f0 - GoodBomb      */

    /* The catch panel's judgement display - Panel::m42d370's twin. */
    void m472300(int lane, int judge, int flag); /* 0x472300 - the press flash (src/panelpress.cpp) */
};

#endif /* GAMEPANELS_H */
