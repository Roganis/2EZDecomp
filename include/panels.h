#ifndef PANELS_H
#define PANELS_H

#include "ksound.h"     /* KRuntimeClass */
#include "bigbase.h"    /* DrawGroup */
#include "obj402860.h"
#include "ez2acapp.h" /* GfxFlag, KGraphics */

struct KTexture;   /* include/loader.h */

/* The engine's MFC-style object base, as the pooled panel classes see it: a
   runtime-class descriptor at slot 0, a virtual destructor at slot 1, and the
   pool's own operator delete.

   It is NOT include/kbase.h's KBase, even though the two have the same slot
   shape. KBase's destructor is the thunk @0x426e40 (`jmp 0x402880`), which is
   what ~KSong @0x410882 and four others call; the panel classes call the bare
   folded `ret` @0x402880 DIRECTLY, so they sit one level closer to the root.
   Whether KBase derives from this or is a sibling is not evidenced yet - all
   that is settled here is which destructor address each family reaches. */
struct __declspec(novtable) EzObject
{
    EzObject();                                 /* 0x401690 (folded) */
    virtual KRuntimeClass *getRuntimeClass();   /* slot 0 */
    virtual ~EzObject();                        /* slot 1; 0x402880 (folded) */

    /* The counterpart of EzPooled::operator new @0x4016a0, and __stdcall for
       the same reason: 0x401720 unlinks the object from its record's node list
       and ends with `ret 4`, so the caller pushes and does not clean up. */
    static void __stdcall operator delete(void *p);   /* 0x401720 (STUB) */

    /* The engine's shared line draw, reached by fifty-eight call sites across
       unrelated classes with `this` in ecx and never read - so it hangs off
       the root. */
    void drawLine(struct Obj402860 *chart, int a, struct KTexture *tex,
                  int b, void *out);                  /* 0x467670 (STUB) */

    /* The root's own instance size is 0x10, not 4: its object record @0x4a8e60
       says {0x10, 1, 0}. What the twelve bytes after the vptr hold is not
       evidenced yet, but the size is - the directors that inherit a SECOND
       polymorphic base put that base's vptr at +0x10 exactly. Three dwords,
       named because the class @0x4887d0 exposes them through vtable slots 5, 6
       and 7 (its getA/getB/getC). */
    int f04;                                    /* 0x04 */
    int f08;                                    /* 0x08 */
    int f0c;                                    /* 0x0c */
};

/* The 0x554-byte render helper whose destructor is @0x40b2e0: it releases seven
   COM objects at +0x458..+0x470 behind an `if (this->f438)` guard - one through
   slot 15 and six through slot 2 (IUnknown::Release) - so it owns a surface set.
   The three background classes embed one, and so does ModeSelectDirector
   (include/modeselect.h). Only its size and the fact that it HAS a destructor
   are evidenced; the destructor itself is a STUB.

   0x474 is a LOWER bound, not the real size: the destructor's highest access is
   the dword at +0x470. GFResultDirector caps it at 0x524 (its next member is
   0x524 past this one); every other embedder leaves a wide gap. So every use
   site pads explicitly to the following member rather than relying on the
   size. */
/* The video player: a DirectShow filter graph with a Sample Grabber in it,
   built by the constructor @0x40b180 (0x4d8 bytes - the draw @0x40b710 reads
   +0x4d0, and every owner's next member sits at +0x4d8 or beyond). Only what the constructor sets is named. */
struct Obj40B2E0
{
    Obj40B2E0();                                /* 0x40b180 */
    ~Obj40B2E0();                               /* 0x40b2e0 */
    int  load(struct KGraphics *g, char *name);    /* 0x40b370 - build the graph for a video file */
    void play(unsigned int now);                /* 0x40b8b0 - run the graph, stamp the start time */
    int  isRunning();                           /* 0x40b920 */
    void m40b710();                             /* 0x40b710 - draw, and pull the next frame */
    void m40b9f0(int a);                        /* 0x40b9f0 - the event pump */
    int   f0;                                   /* 0x000 - 1 */
    int   f4;
    int   f8;
    int   fc;
    int   f10;
    int   f14;
    char  path[0x104];                          /* 0x018 - the video file */
    char  iniPath[0x104];                       /* 0x11c - its .ini beside it */
    char  _pad220[0x428 - 0x220];
    float x;                                    /* 0x428 */
    float y;                                    /* 0x42c */
    float w;                                    /* 0x430 - 640 */
    float h;                                    /* 0x434 - 480 */
    int   f438;
    int   f43c;
    int   f440;
    int   f444;
    unsigned int f448;                          /* 0x448 - play() start time */
    int   f44c;                                 /* 0x44c - frame counter */
    struct KGraphics      *gfx;                 /* 0x450 - who draws the frame */
    struct IGraphBuilder  *graph;               /* 0x454 */
    struct IMediaControl  *control;             /* 0x458 */
    struct IBaseFilter    *grabberFilter;       /* 0x45c */
    struct ISampleGrabber *grabber;             /* 0x460 */
    struct IBasicAudio    *audio;               /* 0x464 */
    struct IVideoWindow   *window;              /* 0x468 */
    struct IMediaSeeking  *seeking;             /* 0x46c */
    struct IMediaEventEx  *events;              /* 0x470 */
    struct Guid16 { int d[4]; };                /* a GUID, as the 16-byte block the copies treat it as */
    struct AmMediaType { Guid16 majortype, subtype; int bFixedSizeSamples, bTemporalCompression;
        unsigned long lSampleSize; Guid16 formattype; void *pUnk; unsigned long cbFormat; void *pbFormat; } mt; /* 0x474 - AM_MEDIA_TYPE */
    struct VideoInfoHeader *format;             /* 0x4bc - the grabber's VIDEOINFOHEADER */
    __int64 duration;                           /* 0x4c0 */
    int   bufSize;                              /* 0x4c8 - GetCurrentBuffer's size */
    unsigned char *buf;                         /* 0x4cc - the frame just grabbed */
    struct KTexture *tex;                       /* 0x4d0 - where the frame is blitted */
    struct KTexture *gray;                      /* 0x4d4 - gray.bmp; 0x4d8 bytes in all: every owner pads to it */
};

/* THE PANEL FAMILY. Five pooled in-game display classes, all with the same
   six-slot vtable {getRuntimeClass, ~, getObjectInfo, update, update2,
   ret0_arg3} and the same adjacent {rtc getter, info getter} leaf pair. Each
   slot-0 getter's KRuntimeClass names its class:

     vtable     name               size      ctor      dtor      deleting ~
     0x48ddd8   AudienceRatePanel  0x2c8     -         0x419a10  0x41ad30
     0x48e1b8   BackGround         0x103540  -         0x41aeb0  0x41b660
     0x48e498   BattleMode         0xf34     0x41c1a0  0x41bbb0  0x41c640
     0x48e8f0   Effector           0x10dc    -         0x41ca20  0x41d040
     0x48eb30   Effector6thStyle   0x14e0    0x41dcc0  0x41d0e0  0x41e600

   Only the members each destructor touches are modeled; the sizes come from
   the object records their slot-2 getters return. */

/* One Obj402860 at +0x2c and nothing else with a destructor. */
struct AudienceRatePanel : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x4199f0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x419a00 */
    virtual void           update();           /* 0x41a910 */
    /* The three draw variants update tail-jumps to. All three render the rate
       as a 4-digit percentage in HUNDREDTHS - digits are `v / 1000 % 10`,
       `/ 100`, `/ 10`, `% 10`, so 8750 reads "87.50" - laid out at fixed x of
       269, 282, [point] 291, 300, 313 and y = m_y (+0x28) + a pool constant.
       The digit textures are an array at +0x298, the decimal point is +0x2c4
       (first row) or +0x2c0 (second). The leading digit is suppressed by a
       `>= 1000` guard, and each function draws the current rate once, then a
       SECOND value in one of two tints picked by `w >= v`, the two arms
       sharing a cross-jumped ones digit. A port re-implements these against
       its own renderer; the layout above is the whole content. */
    void drawFull();                           /* 0x419a70 (STUB) */
    void drawSolo();                           /* 0x419e70 (STUB) */
    void drawIdle();                           /* 0x41a270 (STUB) */
    /* Latch the rates as percentages and step the blink tint. `ret 8` -
       the two arguments the screen hands every update2 are ignored here
       (src/panels.cpp). */
    virtual void           update2(int, int);  /* 0x41a950 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~AudienceRatePanel();                      /* 0x419a10 */

    char _pad10[0x14 - 0x10];
    /* The tint the third row draws in - the two-up arm where the other side
       is at or above the target. */
    int  f14;               /* 0x14 */
    /* A 0/1/2 blink phase, bumped once per tick; phase 0 is the dim tint
       and 1 and 2 the bright one (update2 @0x41a950). */
    int  f18;               /* 0x18 */
    int  f1c;               /* 0x1c - one side's rate as a percentage */
    int  f20;               /* 0x20 - the other side's, only in a two-up game */
    int  f24;               /* 0x24 - the target */
    float f28;              /* 0x28 */
    Obj402860 chart;        /* 0x2c */
    struct KTexture *cw[10];    /* 0x248 - Point_cw%02d.bmp */
    char _pad270[0x298 - 0x270];
    struct KTexture *next[10];  /* 0x298 - Point_next%02d.bmp */
    struct KTexture *cwPoint;   /* 0x2c0 - point_cwpoint.bmp */
    struct KTexture *nextPoint; /* 0x2c4 - point_nextpoint.bmp */

    AudienceRatePanel();                       /* 0x41aa30 */
};

/* The 1 MB background, and the head of a family of three: GFBackGround and
   CatchBackGround (include/backgrounds.h) have the same members, the same
   destructor body and - for the first two - literally the same update/update2,
   ICF-folded onto one address.

   Its destructor @0x41aeb0 is entered at EH state 2, so TWO members have
   destructors: the flag block's neighbour at +0x1c0 (state 0) and the ten
   Obj402860 at +0x1006a0 (state 1, through __ehvec_dtor_iterator). The rest of
   the body is two graphics calls that tear down the flag at +0x101bb8 and free
   the mesh at +0x102074. */
/* One entry of a background's draw list. The list itself is ten 0x10-byte
   records whose SECOND word points at the item, and the item's own +4 says
   what kind it is - 2 and 3 are charts, 4 and 5 are two different draws
   (@0x45c730 and @0x41afc0). Only what the kind-5 draw reads is modelled. */
struct BgSlot
{
    int    f00;               /* 0x00 */
    struct BgItem *item;      /* 0x04 */
    char   _pad8[0x10 - 0x8];
};

struct BgItem
{
    int   f00;                /* 0x00 */
    int   kind;               /* 0x04 */
    /* The kind-4 draw's two rates: f08 winds the rotation on every tick and
       f0c the wave phase, which bounces between 1.5 and 3.5 (drawItem4
       @0x45c730 / @0x46aed0, src/bgdrawitem4.cpp). */
    int   f08;                /* 0x08 */
    float f0c;                /* 0x0c */
    char  _pad10[0x18 - 0x10];
    int   blendSrc;           /* 0x18 */
    int   blendDst;           /* 0x1c */
    int   f20;                /* 0x20 */
    struct KTexture *tex;     /* 0x24 */
    int   f28;                /* 0x28 - the wound rotation */
    float f2c;                /* 0x2c - the wave phase */
    int   f30;                /* 0x30 - 1 while the phase is winding down */
};

/* What the debug overlay walks: three sub-objects, each of which may be
   null, and whose own +0 selects whether its +0x84 is worth printing. */
struct BgDbg
{
    char  _pad0[8];
    int  *f8;                  /* 0x08 */
    int  *fc;                  /* 0x0c */
    int  *f10;                 /* 0x10 */
};

struct BackGround : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x41adb0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x41adc0 */
    virtual void           update();           /* 0x45c930 (STUB, folded; wip/bgupdate-45c930.cpp) */
    virtual void           update2(int, int);  /* 0x45ca90 (folded, src/bgupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~BackGround();                             /* 0x41aeb0 */
    BackGround();                              /* 0x41b4f0 */

    /* The current text slot with its trailing "-xxx" suffix trimmed, in a
       per-class static buffer. Called with `this` in ecx and never reads it;
       the three copies differ only by which buffer they own, which is why the
       original's ICF did NOT fold them together. */
    char *videoName();                         /* 0x41add0 */
    /* Draw one kind-5 item of the list through the background's own mesh. */
    void  drawItem(BgItem *it);                /* 0x41afc0 */
    void  drawItem4(BgItem *it);               /* 0x45c730 (src/bgdrawitem4.cpp) */
    /* Tick the three payload charts of the script record in force
       (src/bgscripttick.cpp). */
    void  m45c890();                           /* 0x45c890 */
    /* Walk the ten slots and draw each one according to its kind. */
    void  drawList();                          /* 0x41b040 */
    /* Try one background script by name; nonzero once one has loaded. */
    void  m41af50(int slot, int rec);          /* 0x41af50 */
    int   m41b0c0();                           /* 0x41b0c0 */
    int   m41b680();                           /* 0x41b680 */
    int   m41b900(char *name);                 /* 0x41b900 */

    int  f10;                                  /* 0x10 */
    int  f14;                                  /* 0x14 */
    char _pad18[0x1c - 0x18];
    /* The base time update2 measures the run from. */
    int  f1c;                                  /* 0x1c */
    int  f20;                                  /* 0x20 */
    int  f24;                                  /* 0x24 */
    /* The name the debug overlay prints with %s (update @0x45c930). */
    char f28[0xa8 - 0x28];     /* 0x28 */
    int  fa8;                                  /* 0xa8 */
    /* Non-zero puts the background in its alternate draw and turns the
       debug overlay on. */
    int  fac;                  /* 0xac */
    char _padb0[0xb8 - 0xb0];
    /* "<cwd>\Video\<name>\<name>.spv". 260 is not evidenced - all the frame
       proves is that it ends before m_1c0 - but its sibling CatchBackGround
       fills 0xb4..0x1b8 exactly, which is 260. */
    char m_videoPath[260];                     /* 0xb8 */
    char _pad1bc[0x1c0 - (0xb8 + 260)];
    Obj40B2E0 m_1c0;                           /* 0x1c0 */
    int  f698;                                 /* 0x698 */
    char _pad69c[0x6a0 - 0x69c];
    /* The .scr script BackGround::m41b900 @0x41b900 reads whole - a megabyte
       exactly, which is the size its "Background Buf Over" check tests. */
    char m_scrText[0x100000];   /* 0x6a0 .. 0x1006a0 */
    Obj402860 m_charts[10];                    /* 0x1006a0 */
    GfxFlag   m_flag;                          /* 0x101bb8 */
    void     *m_mesh;                          /* 0x102074 */
    BgSlot    m_items[10];                     /* 0x102078 */
    int       f102118;                         /* 0x102118 */
    char _pad10211c[0x102124 - 0x10211c];
    int  f102124;              /* 0x102124 - the overlay's first number */
    char _pad102128[0x103534 - 0x102128];
    /* The three-slot block the overlay reads its pointers out of. */
    struct BgDbg *f103534;     /* 0x103534 */
    char _pad103538[0x103540 - 0x103538];
};

/* A sound object as its owners see it: only slot 5 is evidenced, called with
   no arguments on the way out. BattleMode holds two (both from
   EzSubsystem::createSample), and the result/ranking directors hold six or
   seven each and release them the same way. */
struct EzSoundObject
{
    virtual void q0();
    virtual void q1();
    virtual void q2();
    virtual void q3();
    virtual void q4(int loop);   /* play - the handler @0x43b8d0 passes 0 / 1 */
    virtual void q5();   /* stop - what ~BattleMode calls through [vptr+0x14] */
    virtual void q6(); virtual void q7();
    /* +0x20: the volume, in the mixer's -10000..0 scale - the song-select
       tick ramps it with (level - 255) * 10000 / 300. */
    virtual void q8(int vol);
    virtual void q9();
    virtual void q10(); virtual void q11(); virtual void q12(); virtual void q13();
    virtual void q14(); virtual void q15();
    virtual int  q16();  /* +0x40: bit 0 set while playing (same handler) */
    /* +0x44: restart - update2's preview pump calls it when q16 says
       the voice stopped. */
    virtual void q17();
};

/* Seven Obj402860 from +0x48, three of which are cleared before the teardown
   proper. Ctor @0x41c1a0. */
struct BattleMode : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x41bb90 */
    virtual DrawGroup     *getObjectInfo();    /* 0x41bba0 */
    virtual void           update();           /* 0x41c8e0 (src/battleupdate.cpp) */
    virtual void           update2(int, int);  /* 0x41c660 (src/battleupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~BattleMode();                             /* 0x41bbb0 */

    /* The object m41ca30 raises the picked entry to (slot 0, args n and
       2); zero when nothing listens. */
    void *m_f10;            /* 0x10 */
    int   m_f14;            /* 0x14 */
    int   m_f18;            /* 0x18 */
    int   m_f1c;            /* 0x1c */
    float m_f20;            /* 0x20 */
    float m_f24;            /* 0x24 */
    float m_f28;            /* 0x28 */
    char  m_f2c;            /* 0x2c - the only one that starts set */
    char  m_f2d;            /* 0x2d */
    char  _pad2e[0x30 - 0x2e];
    int   m_f30;            /* 0x30 - 0x3c */
    int   m_f34;            /* 0x34 - 1 */
    int   m_f38;            /* 0x38 */
    int   m_f3c;            /* 0x3c */
    EzSoundObject *m_a;   /* 0x40 */
    EzSoundObject *m_b;   /* 0x44 */
    Obj402860 chart0;       /* 0x48 */
    Obj402860 chart1;       /* 0x264 */
    Obj402860 chart2;       /* 0x480 */
    Obj402860 chart3;       /* 0x69c */
    Obj402860 chart4;       /* 0x8b8 */
    Obj402860 chart5;       /* 0xad4 */
    Obj402860 chart6;       /* 0xcf0 */
    /* Nine textures the constructor loads, then three flags. */
    struct KTexture *m_beat;      /* 0xf0c - BATTLE_BEAT_%02d */
    struct KTexture *m_beatMask;  /* 0xf10 */
    struct KTexture *m_getMask;   /* 0xf14 - typo_get_mask */
    struct KTexture *m_get;       /* 0xf18 - typo_get */
    struct KTexture *m_counter;   /* 0xf1c */
    struct KTexture *m_win1p;     /* 0xf20 */
    struct KTexture *m_win2p;     /* 0xf24 */
    struct KTexture *m_win3p;     /* 0xf28 */
    struct KTexture *m_info;      /* 0xf2c - "<name>_info.bmp" */
    char m_ff30;                  /* 0xf30 */
    char m_ff31;                  /* 0xf31 */
    char m_ff32;                  /* 0xf32 */
    char _padf33[0xf34 - 0xf33];

    BattleMode();                              /* 0x41c1a0 */
    /* Ease and draw the battle meter needle (src/battleupdate.cpp). */
    void m41c060();                            /* 0x41c060 */
    /* Pick the winner banner into chart1's cell (3,0) (src/battleupdate.cpp). */
    void m41bca0();                            /* 0x41bca0 */
    /* Step a lamp cycle on its key's press (src/battleupdate.cpp). */
    void m41ca30(struct BattleCycle *p);       /* 0x41ca30 */

    /* Set both players' START/effect lamps (src/lamps.cpp). */
    void setPlayerLamps(int on);               /* 0x41bd20 */
};

/* Nothing with a destructor at all: ~Effector @0x41ca20 is eleven bytes, the
   vptr store and a tail jump to the base's folded `ret`. */
struct Effector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x41ca00 */
    virtual DrawGroup     *getObjectInfo();    /* 0x41ca10 */
    /* Draw every live effector, then the demo-play overlay
       (src/panels.cpp). */
    virtual void           update();           /* 0x41cda0 */
    virtual void           update2(int, int);  /* 0x41ced0 (src/effectorupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~Effector();                               /* 0x41ca20 */
    Effector();                                /* 0x41cfb0 */

    /* Poll one readout's two input slots and report every transition
       (src/effectordraw.cpp). */
    void m41cac0(struct EffectorReadout *e);   /* 0x41cac0 */

    /* Where the readouts report their input transitions - m41cac0
       @0x41cac0 calls its slot 0 with a code and the new state. */
    struct EffectorReport *f10;                /* 0x10 */
    char f14;                                  /* 0x14 */
    char _pad15[0x18 - 0x15];
    /* How many of the five records below are live. */
    int  f18;                                  /* 0x18 */
    /* One per effector: a mode word and the readout the draw and the input
       poll both take. Five of them fill 0x1c .. 0xd00 exactly. */
    struct EffectorRec
    {
        int  mode;                             /* +0x000 */
        char readout[0x294 - 4];               /* +0x004 - an EffectorReadout */
    }    recs[5];                              /* 0x1c .. 0xd00 */
    /* Only loaded in demo play - the constructor skips both unless
       g_demoPlay is set. */
    struct KTexture *m_demoPlay;               /* 0xd00 */
    struct KTexture *m_demoPlayMask;           /* 0xd04 */
    /* The demo overlay's slide: the latch, the y, and the dwell timer
       (update2 @0x41ced0). */
    int   fd08;                                /* 0xd08 */
    float fd10_pad;                            /* 0xd0c */
    float fd10;                                /* 0xd10 */
    unsigned int fd14;                         /* 0xd14 */
    char _padd18[0x10dc - 0xd18];
};

/* "Effector6thStyle" (the KRuntimeClass @0x4a8ff4 its slot 0 returns names it)
   - the in-game read-out that shows the song's tempo, the player's speed
   setting and the scroll rate they produce. One of five sibling panel classes
   with the same six-slot vtable {getRuntimeClass, ~, getObjectInfo, update,
   update2, ret0_arg3}: the others are AudienceRatePanel @0x48ddd8 and the three
   at 0x48e1b8 / 0x48e498 / 0x48e8f0, each with its own ctor/dtor pair and the
   same {rtc getter, info getter} adjacent leaf pair.

   The nine charts come out of the destructor @0x41d0e0, which tears them down
   one at a time - so they are nine members, not an array (an array member would
   have gone through __ehvec_dtor_iterator). +0x1e4 with the 0x21c stride ends
   at 0x14d0, and 0x14d0 + 0x10 is exactly the 0x14e0 the object record
   @0x4a9004 claims. Only what the destructor and updateSpeedDigits @0x41d1d0
   touch is modeled. */
struct Effector6thStyle : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x41d0c0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x41d0d0 */
    virtual void           update();           /* 0x41d920 (STUB) */
    virtual void           update2(int, int);  /* 0x41e620 (src/effectorinput.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~Effector6thStyle();                       /* 0x41d0e0 */

    /* Everything up to the charts is textures, named by the paths the
       constructor @0x41dcc0 loads them from. */
    struct EffSink *m_sink;   /* 0x10 - update2's command receiver */
    KTexture *m_demoPlay;     /* 0x14 - ..\..\Common\DEMOPLAY */
    KTexture *m_demoPlayMask; /* 0x18 */
    void     *m_gaugeOver;    /* 0x1c - the GaugeOver.wav sample */
    void     *m_gaugeOverMix; /* 0x20 - the CV2Mix one, only when g_1b2eb6c==1 */
    char _pad24[0x28 - 0x24];
    int       m_speedIndex;   /* 0x28 - taken from 0x1b2e914; indexes m_speed */
    char _pad2c[0x40 - 0x2c];
    KTexture *m_bassOff;      /* 0x40 - CV2Mix\InGameEffector\effector_* */
    KTexture *m_bassOn;       /* 0x44 */
    KTexture *m_trebleOff;    /* 0x48 */
    KTexture *m_trebleOn;     /* 0x4c */
    KTexture *m_speed[21];    /* 0x50 - Effector_Speed_%02d, 1..21 */
    char _pada4[0xe0 - 0xa4];
    KTexture *m_speedIcon[11];/* 0xe0 - InGameEffector\Speed_%d */
    KTexture *m_effect[8];    /* 0x10c - streetmix1st EFFECT1..4 and -ON */
    KTexture *m_speedFont[10];/* 0x12c - TT\Speedfont_%d */
    KTexture *m_effectOff;    /* 0x154 - TT\effect_off */
    KTexture *m_effectOn;     /* 0x158 */
    /* The digit glyphs, three parallel sets of ten. A figure is drawn as TWO
       chart rows: the odd row takes its texture from m_fontM and the even one
       from m_fontW, except the scroll figure whose second row comes from
       m_fontG. All three are loaded by one loop, which is why cl anchors its
       walking pointer on the middle one. */
    KTexture *m_fontW[10];  /* 0x15c - TT\speedfontw_%d */
    KTexture *m_fontM[10];  /* 0x184 - TT\speedfontm_%d */
    KTexture *m_fontG[10];  /* 0x1ac - TT\speedfontG_%d */
    /* Where the charts are placed, in whole pixels, chosen per mode and per
       mix by the two jump tables at 0x41e5a4 and 0x41e5d4. */
    int  m_originX;         /* 0x1d4 */
    int  m_originY;         /* 0x1d8 */
    /* The scroll rate the notes actually move at: g_songBpm * g_speedPercent
       / 100, computed by update2 @0x41e620 (at 0x41e76e). Four digits. */
    int  m_scrollBpm;       /* 0x1dc */
    int  m_f1e0;            /* 0x1e0 */
    Obj402860 chart0;       /* 0x1e4 */
    Obj402860 chart1;       /* 0x400 */
    Obj402860 chart2;       /* 0x61c */
    Obj402860 chart3;       /* 0x838 */
    Obj402860 chart4;       /* 0xa54 - the one the speed read-out writes */
    Obj402860 chart5;       /* 0xc70 */
    Obj402860 chart6;       /* 0xe8c */
    Obj402860 chart7;       /* 0x10a8 */
    Obj402860 chart8;       /* 0x12c4 - ends flush at 0x14e0, the whole size */

    Effector6thStyle();         /* 0x41dcc0 */
    void updateSpeedDigits();   /* 0x41d1d0 */
};

#endif /* PANELS_H */
