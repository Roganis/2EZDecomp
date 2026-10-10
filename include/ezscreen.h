#ifndef EZSCREEN_H
#define EZSCREEN_H

#include "kbase.h"

#include "bigbase.h"
#include "players.h"
#include "obj414750.h"
#include "panels.h"   /* EzObject */

/* The five screens EZ2AC::runGame @0x417e90 creates, one per vftable. Each is
   0x48 bytes, adds nothing to the base layout, and overrides only slots 4 and 5
   (everything else comes from Obj401780 - see include/bigbase.h). runGame is the
   only place they are constructed: the ctor is compiler-generated and inlined to
   the vptr store, exactly as in the original.

   Named after their vftable addresses; only the second one can be placed with
   any confidence - its two slots sit in TitleDirector's address range
   (0x44d950..0x44dca0, see src/classinfo.cpp). */

/* vftable 0x48d40c - run once, when m_f18 is still 1. */
struct Screen48D40C : Obj401780
{
    /* its OnCreateGame forwards to a factory; see below. 0x45c3f0 - what s5 tail-jumps to */

    virtual void s4();   /* 0x45c190 */
    virtual void s5();   /* 0x45c1b0 */
};

/* The credits/announcement overlay (KRuntimeClass "ShowCredit" @0x4ae108, its
   record 0x4ae118 => 0x7b4 bytes, ctor 0x44fc30) - only what Screen48D348::s5
   needs of it. setPos stores x at +0x18 and y at +0x1c and derives +0x20; a
   -1.0f coordinate is the "work it out yourself" sentinel. */
struct Sprite; /* include/ez2acapp.h */

struct ShowCredit : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x44f930 */
    virtual DrawGroup     *getObjectInfo();    /* 0x44f940 */
    virtual void           update();           /* 0x450040 (STUB) */
    /* Play the coin or credit sound when the counter moves, then publish
       it - `ret 8`, the two arguments ignored (src/showcredit.cpp). */
    virtual void           update2(int, int);  /* 0x44fb40 */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~ShowCredit();                             /* 0x44f950 */

    char  b10;       /* 0x10 */
    char  _pad11[0x14 - 0x11];
    int   f14;       /* 0x14 - 1 once the constructor is through */
    float x;         /* 0x18 */
    float y;         /* 0x1c */
    float x130;      /* 0x20 */
    float y8;        /* 0x24 */
    float x150;      /* 0x28 */
    float y9;        /* 0x2c */
    float x168;      /* 0x30 */
    float y9b;       /* 0x34 - the same value as y9 */
    float f38;       /* 0x38 - always 64 */
    Sprite *sprite;  /* 0x3c - what setPos measures to place itself */

    Sprite *maskTex;         /* 0x40  - Credits_mask.bmp */
    Sprite *digits[10];      /* 0x44  - Credits_%d.bmp */
    Sprite *seriesS[11];     /* 0x6c  - Credits_S<n>, eleven of them */
    Sprite *seriesSb[11];    /* 0x98  - Credits_Sb<n> */
    Sprite *cvSprite;        /* 0xc4  - the CV2Mix copies of all five */
    Sprite *cvMaskTex;       /* 0xc8 */
    Sprite *cvDigits[10];    /* 0xcc */
    Sprite *cvSeriesS[11];   /* 0xf4 */
    Sprite *cvSeriesSb[11];  /* 0x120 */
    void *sampleA;   /* 0x14c - freed only if it was ever loaded */
    void *sampleB;   /* 0x150 */
    void *cvSampleA; /* 0x154 */
    void *cvSampleB; /* 0x158 */
    int   credits;   /* 0x15c - the engine object's coin count at construction */
    Obj402860 m_160; /* 0x160 */
    Obj402860 m_37c; /* 0x37c */
    Obj402860 m_598; /* 0x598 */

    ShowCredit();                      /* 0x44fc30 */
    void setPos(float px, float py);   /* 0x44fa50 */
};

/* Load a numbered run of textures - `count` of them named <base><n> - into an
   array. cdecl with four arguments, and the graphics singleton comes in as the
   last one rather than in ecx. */
/* The numbered-texture-series loader ShowCredit's constructor uses - the
   third member of loadTexSeries @0x456010's family (src/engine.cpp). */
int  loadSeries(Sprite **dst, char *base, int count, struct KGraphics *g); /* 0x4560c0 */

/* What m450570 @0x450570 returns; the main-game directors hand it their own
   first secondary-base pointer at the end of construction.

   THE FACTORY IS READY TO WRITE (twins.py, 2026-08-16 - it came up as an 88%
   near-copy of createKSong @0x40ef00, which is the same shape with malloc
   instead of the pool). @0x450570 is:

       g_objectRecord = &rec@0x4ae154;   push 0x20;  call 0x4016a0
       call 0x401690                     <- EzObject's ctor, inlined
       mov [esi],0x495400                <- the vftable
       mov [esi+0x10],0                  <- f10
       mov [esi+0x18],0                  <- f18

   so it is `return new ScrFader;` with an INLINE ctor zeroing f10 and f18,
   the class deriving from EzObject and EzPooled. The 0x20 it pushes is
   exactly this struct's size, and EzObject is 0x10, so replacing `_pad0`
   with the base preserves every offset m450420 already depends on.

   What is still needed: the vftable @0x495400 has to be PINNED before the
   inlined ctor can match (an unpinned vptr store prints raw hex on the
   original side against our name), and a pin requires every slot to be an
   entity - so read the slot count out of .rdata at 0x495400 first and give
   each one a STUB. */
struct ScrFader : EzObject, EzPooled
{
    /* Slots 0 and 1 are the root's; the vftable @0x495400 has six. Slot 4 is
       the folded empty @0x475dc0 and slot 5 the folded @0x401770, which is
       why both are declared here only to be pointed at their owners. */
    virtual KRuntimeClass *getRuntimeClass();          /* 0x4503f0 */
    virtual ~ScrFader();                               /* 0x450410 / 0x450550 */
    virtual EzObjectInfo   *getObjectInfo();           /* 0x450400 */
    /* One frame of the fade: publish the level, step it, and tell the sink
       when the ramp ends (src/directors.cpp). */
    virtual void            update();                  /* 0x450450 */
    virtual void            v4(int, int);              /* 0x475dc0 (folded) */
    virtual int             v5(int, int, int);         /* 0x401770 (folded) */

    /* Inlined into the factory @0x450570: base ctor, vptr, two zeros. */
    ScrFader() { f10 = 0; f18 = 0; }

    /* The third argument is the caller's own FIRST secondary-base pointer, so
       it is a different type per director - void * rather than one of them. */
    void m450420(int a, int b, void *s);               /* 0x450420 */

    int   f10;    /* 0x10 - the first argument */
    int   f14;    /* 0x14 - the second */
    int   f18;    /* 0x18 - 0x100 unless the first argument is 1 */
    void *f1c;    /* 0x1c - the caller's sink */
};

/* KRuntimeClass "TitleDirector" @0x4adfd8, record 0x4adfe8 => 0x1664 bytes,
   ctor 0x44e9b0. Only what the lamp ticker @0x44e050 touches is modeled - which
   is enough to place it inside the 0x1664 the record claims. */
struct __declspec(novtable) TitleStoppable
{
    virtual void s0(); virtual void s1(); virtual void s2();
    virtual void s3();            /* slot 3  -> vtable+0x0c */
    virtual void s4(int a);       /* slot 4  -> vtable+0x10 */
    virtual void s5();            /* slot 5  -> vtable+0x14 */
    virtual void s6(); virtual void s7();
    virtual void s8(int v);       /* slot 8  -> vtable+0x20 - the pan ramp */
    virtual void s9(); virtual void s10(); virtual void s11();
    virtual void s12(int a);      /* slot 12 -> vtable+0x30 */
};

/* What the coin-lamp tick drives - only the six slots it uses. */
struct __declspec(novtable) TitleLampDev
{
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual void d4(int a);        /* slot 4  -> vtable+0x10 */
    virtual void d5();             /* slot 5  -> vtable+0x14 */
    virtual void d6(); virtual void d7();
    virtual void d8(int a);        /* slot 8  -> vtable+0x20 */
    virtual void d9(); virtual void d10(); virtual void d11();
    virtual void d12(int a);       /* slot 12 -> vtable+0x30 */
    virtual void d13(); virtual void d14(); virtual void d15();
    virtual int  d16();            /* slot 16 -> vtable+0x40 */
    virtual void d17();            /* slot 17 -> vtable+0x44 */
};

/* The title screen's two secondary bases, each one slot (the dtor @0x44dcc0
   restores three vptrs: the primary, +0x10 and +0x14, and the three tables sit
   back to back at 0x494d0c/0x494d10/0x494d14). The song base is a
   KSongPlayerBase whose update override fires the coin lamp - novtable, like
   every secondary base here, so the constructor stores only the derived
   tables. */
struct __declspec(novtable) TitleSink
{
    virtual void setMode(int unused, int mode);   /* 0x44df40 */
};
struct __declspec(novtable) TitleSong : KSongPlayerBase
{
    virtual void step();                          /* 0x44deb0 */
};

struct TitleDirector : EzObject, TitleSink, TitleSong, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x44dca0 */
    virtual DrawGroup     *getObjectInfo();    /* 0x44dcb0 */
    virtual void           update();           /* 0x44df60 - the per-frame draw */
    virtual void           update2(int, int);  /* 0x44f4d0 (src/titleupdate.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~TitleDirector();                          /* 0x44dcc0 */
    TitleDirector();                           /* 0x44e9b0 (STUB) */

    int  m_30;         /* 0x0030 - the lamp period the tick scales */
    int  m_34;         /* 0x0034 - the ctor seeds it 0xff */
    int  m_38;         /* 0x0038 */
    /* The hidden-command entry buffers, one per stream - the same shape
       ModeSelectDirector keeps at +0x945c (m44f300 @0x44f300). */
    int  cmdBuf[2][20];/* 0x003c .. 0x00dc */
    int  cmdLen[2];    /* 0x00dc - [0] is what clearCommand clears */
    /* Per-slot gate the coin reset checks - only slot 0 with a zero here
       does anything. */
    int  m_e4[3];      /* 0x00e4 .. 0x00f0 */
    int  m_attract;    /* 0xf0  - 1 while the attract loop owns the lamps */
    char _padf4[0xf8 - 0xf4];
    int  m_f8;         /* 0x00f8 */
    int  m_restart;    /* 0xfc  - 1 asks the pattern to start over */
    int  m_100;        /* 0x0100 */
    int  m_104;        /* 0x0104 */
    int  m_108;        /* 0x0108 */
    int  m_10c;        /* 0x010c */
    int  m_110;        /* 0x0110 - set while clearCommand runs */
    int  m_114;        /* 0x0114 */
    int  m_118;        /* 0x0118 - cleared by the destructor */
    int  m_11c;        /* 0x011c */
    int  m_120;        /* 0x0120 */
    char _pad124[0x12c - 0x124];
    int  m_12c;        /* 0x012c */
    char _pad130[0x198 - 0x130];
    /* fadeblack.bmp, then the ten ver_%d.bmp digit textures the version
       readout indexes - the ctor loads both under System\\Title. */
    struct KTexture *m_198;    /* 0x0198 */
    char _pad19c[0x1a0 - 0x19c];
    struct KTexture *m_1a0[10];/* 0x01a0 .. 0x01c8 */
    int  m_1c8;        /* 0x01c8 */
    int  m_1cc;        /* 0x01cc */
    int  m_1d0;        /* 0x01d0 - cleared with m_restart by the coin reset */
    /* Nine charts, in destruction order 0x12c0 first - so declared here
       first to last. */
    Obj402860 m_1d4;   /* 0x01d4 */
    Obj402860 m_3f0;   /* 0x03f0 */
    Obj402860 m_60c;   /* 0x060c */
    /* Where the version readout's script head lands - loadStr's out-param,
       read straight back into m_a48's attach. */
    Obj402860Head *m_828;   /* 0x0828 */
    Obj402860 m_82c;   /* 0x082c */
    Obj402860 m_a48;   /* 0x0a48 */
    Obj402860Head *m_c64;   /* 0x0c64 - what the chart below re-attaches to */
    /* The ctor loads BOTH coin scripts and picks between them on the cabinet:
       InsertCoin into m_c64, Pressstart into this one. */
    Obj402860Head *m_c68;   /* 0x0c68 */
    Obj402860 m_c6c;   /* 0x0c6c */
    Obj402860 m_e88;   /* 0x0e88 */
    Obj402860 m_10a4;  /* 0x10a4 */
    Obj402860 m_12c0;  /* 0x12c0 */
    char _pad14dc[0x14e0 - 0x14dc];
    /* Two objects the coin reset stops: slot 5 then slot 12 with a zero. The
       destructor stops all three (slot 5, then slot 3), the third twice when
       m_14ec asks. */
    struct TitleStoppable *m_14e0;  /* 0x14e0 */
    struct TitleStoppable *m_14e4;  /* 0x14e4 */
    struct TitleStoppable *m_14e8;  /* 0x14e8 */
    int  m_14ec;       /* 0x14ec */
    /* the hiddencmd sample - created by the ctor, not one of the three the
       destructor stops */
    void *m_14f0;      /* 0x14f0 */
    Obj414750 m_pump;  /* 0x14f4 - the attract song's pump, 0x124 bytes */
    int  m_strobe;     /* 0x1618 - the lamp beat: raised every 405.4054 ms
                          by TitleSong::step @0x44deb0 (the pump thread, via
                          the song base at +0x14, hence its +0x1604 store);
                          tickLamps lights output 4, clears it */
    int  m_strobeLeft; /* 0x161c - frames output 4 stays lit (8); m_step++
                          when it runs out */
    int  m_1620;       /* 0x1620 - the coin lamp's countdown */
    int  m_1624;       /* 0x1624 - what the sink's setMode hands on */
    int  m_1628;       /* 0x1628 - the timeGetTime the ctor stamps */
    int  m_162c;       /* 0x162c */
    int  m_1630;       /* 0x1630 - cleared by the coin reset */
    int  m_1634;       /* 0x1634 */
    int  m_1638;       /* 0x1638 - holds the coin chart back while up */
    int  m_163c;       /* 0x163c */
    int  m_1640;       /* 0x1640 */
    int  m_1644;       /* 0x1644 */
    int  m_1648;       /* 0x1648 - the coin chart is running */
    int  m_164c;       /* 0x164c */
    int  m_1650;       /* 0x1650 */
    /* 2 while the screen is taking a second player rather than a first -
       m44e680 @0x44e680 branches on it. */
    int  m_1654;       /* 0x1654 */
    int  m_step;       /* 0x1658 - which frame of the lamp pattern is showing */
    int  m_165c;       /* 0x165c */
    char m_1660;       /* 0x1660 - a BYTE, cleared with bl */
    char _pad1661[0x1664 - 0x1661];

    void tickLamps();  /* 0x44e050 (src/titledirector.cpp) */
    /* Its ONE call site - the tick @0x44f4d0 - sets ecx, so it is a member
       even though its body never touches `this` and it matched at 100% while
       declared a free function. Defined in src/lamps.cpp. */
    void updateTitleStartLamps();  /* 0x44e780 */
    void clearCommand();          /* 0x44f460 */
    /* Append one key to a stream and match it against the three hidden
       codes (src/titledirector.cpp). */
    void m44f300(int slot, int key); /* 0x44f300 */
    char resetCoins(int slot);    /* 0x44e920 */
    void tickCoinLamp(TitleLampDev *dev, int frames); /* 0x44e880 */
    /* Seat the player in `slot`: the second-player path when m_1654 is 2,
       the first-player path otherwise (src/titleseat.cpp). */
    void m44e680(int slot);       /* 0x44e680 */
    /* Whatever the second-player path asks before it commits. */
    int  m44dec0();               /* 0x44dec0 (STUB) */
};

/* The song table the main game looks its entry up in. Each record's tail is
   FOUR PACKED nine-byte entries - a byte and two floats - at 0x32, 0x3b,
   0x44 and 0x4d, which is why the floats sit on unaligned offsets and the
   record needs pack(1); 0x32 + 4 * 9 is 0x56, the stride, so there is no
   trailing pad. The count at +0x9278 is 436 * 0x56, which is what fixes the
   array's length. */
#pragma pack(push, 1)
struct SongRec9
{
    char  b;
    float f0;
    float f1;
};

struct SongEntry
{
    char     b00;               /* 0x00 */
    char     _pad01[0x10 - 1];
    char     b10;               /* 0x10 */
    char     _pad11[0x30 - 0x11];
    char     b30;               /* 0x30 */
    char     b31;               /* 0x31 */
    SongRec9 r[4];              /* 0x32 .. 0x56 */
};
#pragma pack(pop)

/* One group slot. 0x1b40 - the stride between groups - is exactly 436 * 0x10,
   the same 436 as `entries`, so each of the 47 groups holds one slot per song
   (SongTable::loadBinFile @0x469c30). */
struct SongGroupRec { char b[0x10]; };

struct SongTable
{
    SongEntry entries[436];            /* 0x0000 .. 0x9278 */
    int       count;                   /* 0x9278 */
    /* The .bin's second table: 47 groups, each a count and up to 436 records.
       Written by loadBinFile @0x469c30. */
    int       groupCount[0x2f];        /* 0x927c .. 0x9338 */
    SongGroupRec groups[0x2f][436];    /* 0x9338 .. 0x59458 */

    /* Both MEMBERS - every call site loads ecx with &g_songTable, and the
       first argument is a DIRECTORY name, not a destination. */
    int  loadBinFile(const char *dir, const char *name);  /* 0x469c30 */
    int  loadSongBin(const char *dir);                    /* 0x469fb0 */
    int  find(char *name, void *out);  /* 0x469ed0 (src/songinfo.cpp) */
    void clearAll();                   /* 0x469f40 */
};

extern SongTable g_songTable;          /* 0x3780958 */

/* KRuntimeClass "RankingDirector" @0x4ae418, its record 0x4ae428 => 0x109de8
   bytes, ctor 0x457a90 (both already in src/classinfo.cpp). Nothing of the class
   itself is modeled - only its factory is called. */
struct RankingDirector;

/* EzPooled and g_objectRecord moved to include/bigbase.h so the panel
   classes can use them too. */

/* The director each screen's OnCreateGame brings up. Only SongSelect's is more
   than an opaque pointer here: it takes the credits overlay at +0x20bdc4. */
struct SongSelectDirectorObj
{
    char       _pad[0x20bdc4];
    ShowCredit *credit;      /* 0x20bdc4 */
};

/* Screen48D40C's director is WarningDirector (include/directors.h): 0x23c bytes
   out of record 0x4ae4d0 - the same record that screen's OnRegisterClass hands
   to itself, and the record its getObjectInfo @0x45c1d0 returns. */
struct WarningDirector *createWarningDirector();   /* 0x0045c3f0 */

/* Screen48D310's director is TestModeDirector (include/directors.h): 0x24
   bytes out of record 0x4afb70 - the same record that screen's
   OnRegisterClass hands to itself. */
struct TestModeDirector *createTestModeDirector();   /* 0x00476830 */

/* Screen48D428's director is WarningShortDirector (include/directors.h):
   0x230 bytes out of record 0x4ae974 - the same record that screen's
   OnRegisterClass hands to itself, and the size its factory pushes. */
struct WarningShortDirector *createWarningShortDirector();   /* 0x00469bc0 */

/* Screen48D3D4's director: 0x28 bytes out of record 0x4aea68 - the same record
   that screen's OnRegisterClass hands to itself. Its constructor visits
   "System\ResetScreen", so that is what it is for. Six virtuals, vftable
   @0x49cfd0; the last of them is the shared `return 0` @0x401770. */
/* Derives from EzObject (NOT KBase, and not from nothing): slot 0 is the
   runtime-class getter and slot 1 the virtual destructor, which is what gives
   the constructor its unwind frame - wip/resetscreen-ctor.cpp got the shape
   right and the base wrong. The destructor is what settles it: this class
   tail-jumps to the folded `ret` @0x402880 DIRECTLY, which is EzObject's
   destructor; KBase reaches the same body through the thunk @0x426e40. The
   constructor call @0x401690 cannot distinguish them - both are folded onto it.
   EzPooled supplies the __stdcall operator new the factory needs. */
struct Obj46AA80 : EzObject, EzPooled
{
    Obj46AA80();                  /* 0x0046aa80 (STUB; wip/resetscreen-ctor.cpp) */
    virtual KRuntimeClass *getRuntimeClass(); /* 0x46a4e0 - slot 0 */
    virtual ~Obj46AA80();         /* 0x46a500; ??_G 0x46ab00 - slot 1 */
    virtual void *getObjectInfo();/* 0x46a4f0 - slot 2 */
    virtual void v3();            /* 0x46a6d0 - the update-screen draw */
    virtual void v4(int, int);    /* 0x46a880 - the update-screen tick */
    virtual int  v5(int, int, int); /* 0x401770, ICF-folded onto ret0_arg3 */

    void deleteRankings();        /* 0x46a510 */

    /* No pad here: EzObject already occupies 0x00..0x10. */
    int   f10;                    /* 0x10 - seconds left, from g_resetTime */
    int   f14;                    /* 0x14 */
    int   f18;                    /* 0x18 - always 0x78 */
    int   _pad1c;
    int   f20;                    /* 0x20 */
    unsigned int f24;             /* 0x24 - timeGetTime() at construction */
};

Obj46AA80 *createObj46AA80();

struct AudienceRatePanel *createAudienceRatePanel();  /* 0x41ad50 */   /* 0x0046ab20 */

/* Both directors' start() have the same skeleton, so the three kinds of object
   they hold are modelled once here. Only the slots they use are typed.

   The pending object is asked to stop (slot 1, one argument) and then forgotten;
   its byte at +0x2d gates the whole start. */
struct DirectorPending
{
    virtual void v0();
    virtual void v1(int flag);   /* slot 1 -> +0x04 */
    char _pad4[0x14 - 4];
    /* The two live scores the EZ2DJ tick republishes here (update2
       @0x4249e0). */
    int  f14;                    /* 0x14 */
    int  f18;                    /* 0x18 */
    char _pad1c[0x2d - 0x1c];
    char ready;                  /* 0x2d */
};

/* The notifiable is poked through vtable slot 8, with no arguments. */
struct DirectorNotify
{
    virtual void v0(); virtual void v1(); virtual void v2();
    virtual void v3(); virtual void v4(); virtual void v5();
    virtual void v6(); virtual void v7();
    virtual void v8();           /* slot 8 -> +0x20 */
    virtual void v9(); virtual void v10();
    virtual void v11(int a);     /* slot 11 -> +0x2c */
    virtual void v12();
    /* slot 13 -> +0x34: set one lamp. EZ2DJMainGameDirector::m422f20 sweeps
       two ranges through it, 0x13..0x40 and 0x00..0x12. */
    virtual void v13(int lamp, int value);
    virtual void q14(); virtual void q15(); virtual void q16();
    virtual void q17(); virtual void q18(); virtual void q19();
    virtual void q20(); virtual void q21();
    virtual int  v22();          /* slot 22 -> +0x58: the running beat */
    virtual void q23(); virtual void q24(); virtual void q25();
    virtual void q26();
    /* slot 27 -> +0x6c: KEZPlayer::getI7274 @0x40f490, the play state -
       0 never started / the chart walk ran off the end (drainQueue
       @0x40fd70 drops it) / reset, 1 playing (play @0x40fa00), 2 stopped
       (stop @0x40fad0). EZ2DJMainGameDirector::update2 ends the stage on 0,
       so it is the walk's end that ends a song, never a stop. (Was
       mis-described as "nonzero while held" until 2026-09-30.) */
    virtual int  playState();
};

/* The per-frame consumer takes the current time. Each director's is a different
   class - the two entry points are different functions. */
/* The per-frame time consumers each main-game director owns. Same class twice
   over at different offsets: a guard, a start time and a started flag, and an
   Obj40B2E0 they forward the time to. */
struct CatchTimed
{
    void m46b760(unsigned int now);                     /* 0x46b760 */

    char         _pad0[0x14];
    unsigned int m_start;      /* 0x014 */
    int          m_started;    /* 0x018 */
    char         _pad1c[0xa0 - 0x1c];
    int          m_guard;      /* 0x0a0 */
    char         _pada4[0x1b8 - 0xa4];
    Obj40B2E0    m_sub;        /* 0x1b8 */
};

struct MainTimed
{
    void m41b460(unsigned int now);                     /* 0x41b460 */

    char         _pad0[0x1c];
    unsigned int m_start;      /* 0x01c */
    int          m_started;    /* 0x020 */
    char         _pad24[0xa8 - 0x24];
    int          m_guard;      /* 0x0a8 */
    char         _padac[0x1c0 - 0xac];
    Obj40B2E0    m_sub;        /* 0x1c0 */
};

/* THE THREE MAIN-GAME DIRECTORS. One per game family, and structurally the same
   class three times over: the root, THREE more polymorphic bases at +0x10, +0x14
   and +0x18 (see the multiple-inheritance rule in CLAUDE.md), the pump, and a
   teardown that turns two mixer controls down, runs two of its own methods,
   releases the per-frame consumer, resets the cabinet and puts the device's
   render state 0x16 back to 3.

     vtable    name                   size    dtor      deleting ~  pump
     0x48f8b0  EZ2DJMainGameDirector  0xdb0   0x422df0  0x4246b0    0x24
     0x49a16c  GFMainGameDirector     0xd3c   0x45f240  0x460720    0x24
     0x49da84  CatchMainGameDirector  0xd0c   0x46d250  0x46e530    0x40

   The three secondary interfaces have the same shape in all three (`ret 8`,
   `ret 0xc`, `ret 8`), but each class's overrides are its own functions and an
   override compiled for a secondary `this` has to be declared ON that base -
   so every class gets its own three. 0x4236f0 makes the offset visible: it
   does `add ecx,0xffffffe8` (-0x18) to get back to the derived object.

   All three leave slot 3 (update) pointing at the shared folded empty `ret`
   @0x402880. Every sink is __declspec(novtable): the constructors store only
   the DERIVED vftables at +0x10/+0x14/+0x18, with no base-subobject vptr
   stores in front of them. */
struct MixerControl;   /* src/mixer.cpp - setValue @0x412a10 */

/* Report the run's outcome to the screen driver (src/ez2djsink.cpp). */
struct __declspec(novtable) EZ2DJSink1 { virtual void v0(int a, int what); };  /* 0x4248c0 */
/* The lane-expiry sink: slot 0 is handed the SlotSound whose countdown ran
   out, plus the two values the commit and the timed object both take. */
struct __declspec(novtable) EZ2DJSink2
{
    virtual void v0(struct SlotSound *s, int a, int b);   /* 0x4202f0 */
};
/* Broadcast one key event to every node of the run's player list
   (src/sink3.cpp). */
struct __declspec(novtable) EZ2DJSink3 { virtual void v0(int key, int what); };  /* 0x4236f0 */

/* EZ2DJ's main-game director. Same skeleton as CatchMainGameDirector's start,
   with the mode taken into account: which of the two players' pending object
   survives, and a mode-4 special case for the field at +0xda4.
   0x424980 sits immediately before its runtime-class getter @0x41f970, which is
   what named it before its own KRuntimeClass @0x4a9030 confirmed it. */
/* One entry of a slot block: only the first dword is ever written. */
struct GdsSlotItem
{
    int  v;                    /* 0x00 */
    char _pad4[0x10 - 4];
};

/* 0x1c4 bytes; the count at +4 bounds the item array at +0x18. */
struct GdsSlotBlock
{
    int         f0;            /* 0x000 */
    int         count;         /* 0x004 */
    char        _pad8[0x18 - 8];
    GdsSlotItem items[26];     /* 0x018 .. 0x1b8 */
    char        _pad1b8[0x1c4 - 0x1b8];
};

/* One JudgmentLink entry - a length and that many ints, 0x48 bytes. */
struct GdsLinkEntry
{
    int n;              /* 0x00 */
    int v[0x11];        /* 0x04 .. 0x48 */
};

struct EZ2DJMainGameDirector : EzObject, EZ2DJSink1, EZ2DJSink2, EZ2DJSink3, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x41f970 */
    virtual DrawGroup     *getObjectInfo();    /* 0x41f980 */
    virtual void           update();           /* 0x402880 (STUB, folded) */
    virtual void           update2(int, int);  /* 0x4249e0 (src/ez2djupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~EZ2DJMainGameDirector();                  /* 0x422df0 */
    int m420810(struct KParser *p);            /* 0x420810 */
    /* The per-frame lane input sweep (src/ez2djinput.cpp). */
    /* Build the background and load its script (src/bgload.cpp). */
    int  m420a60();                            /* 0x420a60 */
    void m41ffa0();                            /* 0x41ffa0 */
    /* The in-play sweep with the autoplay filter and the three assist
       devices (src/ez2djinput.cpp). */
    void m41fb20();                            /* 0x41fb20 */
    /* The same sweep, also paying out held keys (src/ez2djinput.cpp). */
    void m420130();                            /* 0x420130 */
    /* The end-of-song machine: has every live player finished
       (src/ez2djgameover.cpp), kick the current fader, and step past the
       song - next stage or the results (src/ez2djfinish.cpp). */
    int  m4204b0();                            /* 0x4204b0 */
    void m424830();                            /* 0x424830 */
    int  m420e30();                            /* 0x420e30 (src/ez2djfinish.cpp) */

    /* Set while the chart load has failed, cleared once it succeeds
       (the chart loader @0x423a90, src/ez2djload.cpp). */
    int             f1c;       /* 0x01c */
    int             f20;       /* 0x020 - 0 or 2, picks which f15c/f160 is used */
    Obj414750       pump;      /* 0x024 */
    /* The pump ends exactly at 0x148, and the four bytes that used to be
       padding are the sample sink every lane operation goes through
       (include/players.h); the object at +0x14c below is the clock beside
       it. */
    struct SlotSampleSink *sink;  /* 0x148 */
    DirectorNotify *notify;    /* 0x14c */
    char            _pad150[0x154 - 0x150];
    /* The Judge the chart loader @0x423a90 builds and arms. */
    struct Judge   *judge;     /* 0x154 */
    /* The credit readout m420d90 @0x420d90 builds and places. */
    struct ShowCredit *credit; /* 0x158 */
    struct ScrFader *f15c;    /* 0x15c */
    struct ScrFader *f160;    /* 0x160 */
    /* The in-game song-select overlay this director owns (m420d40 @0x420d40
       builds it and hands it back a pointer to this director). */
    struct InGameSongSelect *f164;  /* 0x164 */
    int             f168;      /* 0x168 */
    int             f16c[0x60];/* 0x16c .. 0x2ec */
    /* The JudgmentDelta window and the gauge rates, all with defaults the .gds
       overrides (loadGds @0x423790). */
    int             f2ec;      /* 0x2ec - 0x63 */
    char            _pad2f0[0x2f8 - 0x2f0];
    int             f2f8;      /* 0x2f8 - 0x48 */
    int             f2fc;      /* 0x2fc - 0x24 */
    int             f300;      /* 0x300 - 0x18 */
    int             f304;      /* 0x304 - 6 */
    char            _pad308[0x30c - 0x308];
    float           f30c;      /* 0x30c - -4.8 */
    float           f310;      /* 0x310 - -1.8 */
    float           f314;      /* 0x314 - 0.1 */
    float           f318;      /* 0x318 - 0.2 */
    /* A copy of the COOL delta, taken before the auto-play doubling
       (m4206e0 @0x4206e0). */
    float           f31c;      /* 0x31c */
    float           f320;      /* 0x320 - 2.0 */
    /* The audience-panel section, read by m420810 @0x420810: two ints and
       seven floats, of which four are published to globals afterwards. */
    int             f324;      /* 0x324 - "Use Audience Panel" */
    int             f328;      /* 0x328 */
    char            _pad32c[0x330 - 0x32c];
    float           f330;      /* 0x330 */
    float           f334;      /* 0x334 */
    float           f338;      /* 0x338 */
    float           f33c;      /* 0x33c */
    float           f340;      /* 0x340 */
    float           f344;      /* 0x344 */
    float           f348;      /* 0x348 */
    /* Ticks per measure - the chart's own header value times four, which
       the loader @0x423a90 also logs as "m_nTPB". */
    int             f34c;      /* 0x34c */
    int             f350;      /* 0x350 */
    int             f354;      /* 0x354 */
    int             f358;      /* 0x358 */
    int             f35c;      /* 0x35c */
    float           f360;      /* 0x360 */
    float           f364;      /* 0x364 */
    DirectorPending *pending;  /* 0x368 */
    unsigned char   ready;     /* 0x36c - set on the way out; read unsigned (movzx) */
    char            _pad36d[0x370 - 0x36d];
    int             f370;      /* 0x370 - Background */
    MainTimed      *timed;     /* 0x374 */
    int             f378;      /* 0x378 */
    int             f37c;      /* 0x37c - zeroed by m421190, then its gate */
    int             f380[0xe2];/* 0x380 .. 0x708 */
    char            f708[0x788 - 0x708];  /* 0x708 - Panel, a name token */
    /* The three overlays m420e30 builds out of the mode's Panel dir. */
    struct Effector          *f788;    /* 0x788 */
    struct Effector6thStyle  *f78c;    /* 0x78c */
    struct AudienceRatePanel *f790;    /* 0x790 */
    int             f794;      /* 0x794 - Gauge */
    char            _pad798[0x79c - 0x798];
    int             f79c;      /* 0x79c */
    int             f7a0;      /* 0x7a0 - 0x7f */
    int             f7a4;      /* 0x7a4 */
    int             f7a8;      /* 0x7a8 */
    float           f7ac;      /* 0x7ac */
    float           f7b0;      /* 0x7b0 - 435 */
    float           f7b4;      /* 0x7b4 - ShowCredit's second pair */
    float           f7b8;      /* 0x7b8 */
    int             f7bc;      /* 0x7bc */
    char            _pad7c0[0x7c4 - 0x7c0];
    /* General's other three ints - m420f00 @0x420f00, still a STUB. */
    int             f7c4;      /* 0x7c4 */
    int             f7c8;      /* 0x7c8 */
    int             f7cc;      /* 0x7cc */
    char            _pad7d0[0x7d4 - 0x7d0];
    int             f7d4;      /* 0x7d4 */
    char            f7d8[4];   /* 0x7d8 - the .gds "SongOriginalName" token */
    int             f7dc;      /* 0x7dc */
    MixerControl   *mixerA;    /* 0x7e0 - addControl(0, 2) */
    MixerControl   *mixerB;    /* 0x7e4 - addControl(0, 1) */
    MixerControl   *mixerC;    /* 0x7e8 - addControl(0, 0) */
    MixerControl   *mixerD;    /* 0x7ec - addControl(1, 0) */
    int             f7f0;      /* 0x7f0 - the tick's own latch */
    /* JudgmentLink - two counts and two runs of 0x48-byte entries. See the
       handler in src/maingame.cpp; the layout is forced by all three
       directors agreeing on the deltas. */
    int             f7f4;      /* 0x7f4 - first count  */
    int             f7f8;      /* 0x7f8 - second count */
    char            _pad7fc[0x800 - 0x7fc];
    GdsLinkEntry    link_a[8];  /* 0x800 .. 0xa40 */
    GdsLinkEntry    link_b[7];  /* 0xa40 .. 0xc38 */
    char            _padc38[0xc7c - 0xc38];
    int             fc7c;      /* 0xc7c */
    /* Per player, per lane: a countdown the tick steps and, on reaching
       zero, reports through that player's track sink (m422d30 @0x422d30).
       Two players of eighteen lanes. */
    int             fc80[0x24];/* 0xc80 .. 0xd10 */
    int             fd10;      /* 0xd10 - 2 */
    int             fd14;      /* 0xd14 - what rotateLanes is told on option 0x15 */
    int             fd18;      /* 0xd18 */
    int             fd1c[8];   /* 0xd1c .. 0xd3c */
    char            _padd3c[0xd44 - 0xd3c];
    unsigned int    started;   /* 0xd44 */
    int             fd48;      /* 0xd48 */
    /* The song-table entry m469ed0 copies in, keyed by the mode name. */
    char            songEntry[0x56];  /* 0xd4c .. 0xda2 */
    char            _padda2[0xda4 - 0xda2];
    int             fda4;      /* 0xda4 - 0 only in mode 4 with two elements */
    char            _padda8[0xdb0 - 0xda8];

    void start();      /* 0x41fa10 */
    /* Step the shared pan by seven and re-apply it, snapping to centre and
       locking for fifteen frames when it crosses (src/panstep.cpp). */
    void stepPan(int dir);                     /* 0x420400 */
    /* Begin a stage: release the old slots, load the chart, and start the
       run under whichever option kind is set (src/ez2djstage.cpp). */
    void m420530();    /* 0x420530 (src/stageteardown.cpp) */
    void m4246d0();    /* 0x4246d0 */
    EZ2DJMainGameDirector();                   /* 0x423f40 */
    /* Read the mode's .gds and dispatch its sections. */
    int  loadGds(char *name);                  /* 0x423790 */
    char *m4208f0();                           /* 0x4208f0 (STUB) */
    /* The .gds MeasureScale section (src/gaugerates.cpp). */
    int  m420640(struct KParser *p);           /* 0x420640 */
    int  m4206e0(struct KParser *p);           /* 0x4206e0 (src/gaugerates.cpp) */
    void m420ec0();    /* 0x420ec0 (STUB) */
    /* Release the run's notify, sample sink and judge, then park the pump
       (src/stageteardown.cpp). */
    /* Release the run's notify, sample sink and judge, then park the pump
       (src/stageteardown.cpp). */
    void m4209d0();    /* 0x4209d0 */
    void m420360(int level);                   /* 0x420360 (STUB) */
    int  m421190(char *name);                  /* 0x421190 - the director's .gds */
    /* The .gds section handlers m421190 dispatches to. */
    int  m420f60(struct KParser *p, int i);    /* 0x420f60 - SlotN */
    int  m420f00(struct KParser *p);           /* 0x420f00 - General (STUB) */
    int  m421000(struct KParser *p);           /* 0x421000 - ShowCredit */
    int  m421060(struct KParser *p);           /* 0x421060 - JudgmentLink */
    int  m421040(struct KParser *p);           /* 0x421040 - InGameSongSelect */
    /* Build the credit readout, place it by mode, and mark it for the
       single-player modes (src/showcreditsetup.cpp). */
    int  m420d90();                            /* 0x420d90 */
    int  m423a90();                            /* 0x423a90 (src/ez2djload.cpp) */
    void m423ed0();                            /* 0x423ed0 (STUB) */
    /* Build the in-game song-select overlay and hand the first player in
       list A to its own setter. */
    int  m420d40();                            /* 0x420d40 */
    /* Chain three key ranges together across one slot's entry table. */
    void m421600(struct ListNode *n, int a, int b, int c, int d);
                                               /* 0x421600 */
    void m421510(int from, int to, int c, int d);  /* 0x421510 */
    /* Mirror the player's playable lanes over the time range - the EZ2DJ
       twin of CatchMainGameDirector::m46cfc0 (src/catchdirector.cpp is the
       catch pair; this one lives in src/ez2djstage.cpp). */
    void m422870(struct ListNode *n, int a, int b);  /* 0x422870 */
    /* Step every present player's per-lane countdowns and report the ones
       that reach zero (src/ez2djstage.cpp). */
    void m422d30();                                  /* 0x422d30 */
    /* Shuffle the slot's lanes: sixteen successful random pair-swaps through
       the spare 21, drawing within one half under the 4/8 special flag. */
    void m421c30(struct ListNode *n, int check, int c, int d);  /* 0x421c30 */
    /* Reverse the two key banks: bank A mirror-swapped; bank B mirror-swapped
       too unless it holds nine or more, which gets a fixed scramble instead. */
    void m4221b0(struct ListNode *n, int c, int d);  /* 0x4221b0 */
    /* The time-sliced shuffle: walk [start, limit) in steps of eight
       measures, six random pair-swaps per slice, refusing pairs whose lanes
       sit inside a hold and stretching each slice's window past any hold the
       swapped lanes carry. */
    void m421750(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                     /* 0x421750 */
    /* The same shuffle at quarter-measure slices. */
    void m4219c0(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                     /* 0x4219c0 */
    /* Mirror-reverse the whole run, then reverse the two five-key banks -
       bank B by a fixed three-pair scramble once it holds seven or more. */
    void m421ed0(struct ListNode *n, int c, int d);  /* 0x421ed0 */
    /* The mode-aware bank swap: radio-family modes pair bank A against
       bank B; the rest reverse each bank in place. */
    void m422410(struct ListNode *n, int c, int d);  /* 0x422410 */
    /* Pick two DIFFERENT free lanes at random below `range`, giving up after
       32 tries and falling back to `base` for both. */
    void pickTwoLanes(struct ListNode *n, int base, int range, int check,
                      int *outA, int *outB);      /* 0x421680 */
    /* Is this lane one of the four the mode leaves free? */
    int  laneBusy(int key);                       /* 0x421490 */
    /* Reverse the run's lane order, swapping each lane with its mirror. */
    void reverseLanes(struct ListNode *n, int c, int d);  /* 0x421df0 */
    /* Rotate the playable lanes by one, last to first. */
    void rotateLanes(struct ListNode *n, int c, int d);   /* 0x422970 */
    /* Reverse the SCRATCH lanes among themselves. Decoded, parked on which
       list field cl folds: wip/reversescratch.cpp. */
    /* Arm one note kind: (kind, flag). */
    void m422f20(int kind, int flag);          /* 0x422f20 (STUB) */
};

/* Report the run's outcome to the screen driver (src/directors.cpp). */
struct __declspec(novtable) GFSink1 { virtual void v0(int a, int what); };     /* 0x45d910 */
/* The lane-expiry sink, the twin of EZ2DJSink2. */
struct __declspec(novtable) GFSink2
{
    virtual void v0(struct SlotSound *s, int a, int b);   /* 0x45d580 */
};
/* Broadcast one key event to every node of the run's player list
   (src/sink3.cpp). */
struct __declspec(novtable) GFSink3 { virtual void v0(int key, int what); };  /* 0x45fa50 */

/* GF IS SCRATCHMIX - THE TURNTABLE MODE, mode 9. The prefix is the binary's
   own (GFMainGameDirector, GFModeSelectDirector, GFSongSelectDirector,
   GFResultDirector and GFRankingDirector are all literal class-name strings)
   but it expands nowhere, so read it as a codename and not as a description.
   What identifies the mode is what the director loads:
   `System\CV2Mix\ScratchMix\ScratchMix.gds` @0x49a1a8, with its own
   "GFMainGameDirector: Can't Load %s" beside it. The mode-name table
   @0x4872b8 (0x20 stride) then places it: 0 StreetMix, 1 7StreetMix,
   2 ClubMix, 3 SpaceMix, 4 5RadioMix, 5 RadioMix, 6 10RadioMix, 7 14RadioMix,
   8 EZ2CATCH, 9 ScratchMix, 10 CV2Mix - which is the g_modeIndex the lamp
   drivers in src/lamps.cpp switch on.

   Comments in this tree called it "guitar" for several sessions on nothing
   but the two letters; that was wrong and has been swept. The playable lane
   COUNT is still unverified either way - it comes out of the .gds, not the
   executable. What the executable does say is which lanes are protected from
   the shuffles: laneBusy @0x45e460 excludes 15, 17, 23 and 25 here and in the
   EZ2DJ director alike, plus 6..9 when f768 is set. */

/* Only what its destructor evidences. */
struct GFMainGameDirector : EzObject, GFSink1, GFSink2, GFSink3, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x45d060 */
    virtual DrawGroup     *getObjectInfo();    /* 0x45d070 */
    virtual void           update();           /* 0x402880 (STUB, folded) */
    virtual void           update2(int, int);  /* 0x45f350 (src/gfupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~GFMainGameDirector();                     /* 0x45f240 */
    void start();                              /* 0x45d100 (STUB) */
    /* Has every live player finished - the GF twin of
       EZ2DJMainGameDirector::m4204b0 (src/gfgameover.cpp). */
    int  m45d740();                            /* 0x45d740 */

    /* Left set when the chart load fails, cleared when it succeeds. */
    int             f1c;       /* 0x01c */
    int             f20;       /* 0x020 */
    Obj414750       pump;      /* 0x024 */
    /* Same as the EZ2DJ director: the pump ends at 0x148 and the sink lives
       immediately after it. */
    struct SlotSampleSink *sink;  /* 0x148 */
    /* Same slot as the EZ2DJ director's - start() @0x45d100 calls its v8. */
    DirectorNotify *notify;    /* 0x14c */
    /* The Judge the loader @0x45fd50 builds. */
    struct Judge   *judge;     /* 0x150 */
    struct ShowCredit *credit; /* 0x154 */
    struct ScrFader *f158;    /* 0x158 */
    struct ScrFader *f15c;    /* 0x15c */
    int             f160[0x60];/* 0x160 .. 0x2e0 */
    /* The same .gds-overridable set as EZ2DJ's, one section shorter
       (loadGds @0x45faf0). */
    int             f2e0;      /* 0x2e0 - 0x63 */
    char            _pad2e4[0x2ec - 0x2e4];
    int             f2ec;      /* 0x2ec */
    int             f2f0;      /* 0x2f0 */
    int             f2f4;      /* 0x2f4 */
    int             f2f8;      /* 0x2f8 */
    char            _pad2fc[0x300 - 0x2fc];
    float           f300;      /* 0x300 */
    float           f304;      /* 0x304 */
    float           f308;      /* 0x308 */
    float           f30c;      /* 0x30c */
    /* The COOL delta again, copied before the auto-play doubling
       (m45d960 @0x45d960). */
    float           f310;      /* 0x310 */
    float           f314;      /* 0x314 - 2.0, then always 1.6 from the .gds */
    /* The measure length in ticks, the unit both shuffles slice by: m45e770
       @0x45e770 takes eight measures per slice and m45e9f0 @0x45e9f0 a
       quarter of one. The EZ2DJ director keeps the same value at +0x34c. */
    int             f318;      /* 0x318 */
    int             f31c;      /* 0x31c */
    int             f320;      /* 0x320 */
    float           f324;      /* 0x324 */
    float           f328;      /* 0x328 */
    void           *f32c;      /* 0x32c */
    char            b330;      /* 0x330 */
    char            _pad331[0x334 - 0x331];
    int             f334;      /* 0x334 - Background */
    void           *timed;     /* 0x338 */

    /* Build the background and load its script (src/bgload.cpp). */
    int  m45db70();            /* 0x45db70 */
    /* Rebase every list-C player's sounds (src/accessors.cpp). */
    void rebaseListC();        /* 0x45d1c0 */
    void m45d200();            /* 0x45d200 (STUB) */
    /* What the third sink hands the acted-on keys to. */
    void m45f640(int key, int flag);           /* 0x45f640 (STUB) */
    /* Start the run-out fade once, unless the phase is already 1 or 2. */
    void m45d8d0();            /* 0x45d8d0 */
    void m45d7c0();            /* 0x45d7c0 (STUB) */
    int             f33c;      /* 0x33c - zeroed by m45e190, then its gate */
    int             f340[0xe2];/* 0x340 .. 0x6c8 */
    char            f6c8[0x748 - 0x6c8];  /* 0x6c8 - Panel, a name token */
    /* The two effector objects m45de70 @0x45de70 builds, out of the mode's
       own System\<mode>\Panel directory. */
    struct Effector         *m_effector;     /* 0x748 */
    struct Effector6thStyle *m_effector6th;  /* 0x74c */
    int             f750;      /* 0x750 - Gauge */
    /* The credit overlay's fade level, ramped by update2 @0x45f350. */
    int             f754;      /* 0x754 */
    float           f758;      /* 0x758 - the credit overlay's x */
    float           f75c;      /* 0x75c - and its y */
    char            _pad760[0x768 - 0x760];
    /* General's other three ints - m45df30 @0x45df30, still a STUB. */
    int             f768;      /* 0x768 */
    int             f76c;      /* 0x76c */
    int             f770;      /* 0x770 */
    MixerControl   *mixerA;    /* 0x774 - addControl(0, 2) */
    MixerControl   *mixerB;    /* 0x778 - addControl(0, 1) */
    MixerControl   *mixerC;    /* 0x77c - addControl(0, 0) */
    MixerControl   *mixerD;    /* 0x780 - addControl(1, 0) */
    /* 1 while nothing in the first player's track list is still running. */
    int             f784;      /* 0x784 */
    /* JudgmentLink - two counts and two runs of 0x48-byte entries. See the
       handler in src/maingame.cpp; the layout is forced by all three
       directors agreeing on the deltas. */
    int             f788;      /* 0x788 - first count  */
    int             f78c;      /* 0x78c - second count */
    char            _pad790[0x794 - 0x790];
    GdsLinkEntry    link_a[8];  /* 0x794 .. 0x9d4 */
    GdsLinkEntry    link_b[7];  /* 0x9d4 .. 0xbcc */
    char            _padbcc[0xc10 - 0xbcc];
    /* Tick every live player's lane countdowns (src/ez2djstage.cpp). */
    void m45f1a0();                        /* 0x45f1a0 */

    int             fc10;      /* 0xc10 */
    /* The per-lane countdowns m45f1a0 @0x45f1a0 ticks down, 18 per player -
       EZ2DJMainGameDirector::fc80's twin. */
    int             fc14[0x24];/* 0xc14 .. 0xca4 */
    int             fca4;      /* 0xca4 - 2 */
    int             fca8;      /* 0xca8 - handed to every ladder handler */
    int             fcac[8];   /* 0xcac .. 0xccc */
    char            _padccc[0xcd4 - 0xccc];
    unsigned int    started;   /* 0xcd4 - stamped by start() */

    int             fcd8;      /* 0xcd8 */
    /* The song-table entry, exactly as EZ2DJ's at +0xd4c. */
    char            songEntry[0x56];  /* 0xcdc .. 0xd32 */
    char            _padd32[0xd38 - 0xd32];
    int             fd38;      /* 0xd38 - the EZ2DJ director's fda4 */


    GFMainGameDirector();                      /* 0x460110 */
    int   loadGds(char *name);                 /* 0x45faf0 */
    char *m45da00();                           /* 0x45da00 (STUB) */
    /* The .gds JudgmentDelta section (src/gaugerates.cpp). */
    int   m45d960(struct KParser *p);          /* 0x45d960 */
    void m45def0();    /* 0x45def0 (STUB) */
    /* The GF copy of EZ2DJMainGameDirector::m4209d0. */
    void m45dae0();    /* 0x45dae0 */
    int  m45e190(char *name);                  /* 0x45e190 - the director's .gds */
    int  m45df90(struct KParser *p, int i);    /* 0x45df90 - SlotN        */
    int  m45df30(struct KParser *p);           /* 0x45df30 - General */
    /* EZ2DJMainGameDirector::m421600's twin on this director's own slot table,
       and it calls THAT class's @0x421510 with its own `this` - one function
       serving both directors. */
    void m45e630(struct ListNode *n, int a, int b, int c, int d);
                                               /* 0x45e630 */
    /* Find the first hold (width > 6) on lane h no earlier than a and no
       later than limit; copy its state out when the caller wants it. */
    int  m45e5a0(int h, unsigned int a, unsigned int limit,
                 struct SlotState *out);       /* 0x45e5a0 */
    /* Rotate the playable lanes by one, last to first. */
    void rotateLanes(struct ListNode *n, int c, int d);   /* 0x45ee70 */
    /* Pick two DIFFERENT free lanes at random out of [base, base+range),
       giving up after 32 tries and falling back to base for both. */
    void pickTwoLanes(struct ListNode *n, int base, int range, int check,
                      int *outA, int *outB);   /* 0x45e6b0 */
    /* Is this lane one of the ones the mode leaves free? */
    int  laneBusy(int key);                    /* 0x45e460 */
    int  m45e060(struct KParser *p);           /* 0x45e060 - JudgmentLink */
    int  m45fd50();                            /* 0x45fd50 (src/gfload.cpp) */
    void m4600a0();                            /* 0x4600a0 */
    /* The GF copy of the pan stepper (src/panstep.cpp). */
    void stepPan(int dir);                     /* 0x45d690 */
    void m45d5f0(int level);                   /* 0x45d5f0 (src/panlevel.cpp) */
    /* Build the two effector objects out of System\<mode>\Panel. */
    int  m45de70();                            /* 0x45de70 */
    /* Start the fade IN - phase 0 through the first fader, phases 2 and
       3 through the second. Any other phase does nothing. */
    void m45d810();                            /* 0x45d810 */
    /* The scratch copy of reverseLanes. */
    void m45ed90(struct ListNode *n, int a, int b);          /* 0x45ed90 */
    /* Called by both shuffles once their slicing is done, with the track list
       and the ORIGINAL (un-advanced) window. */
    void m45e4e0(struct TrackList *t, unsigned int start, unsigned int limit);
                                               /* 0x45e4e0 (STUB) */
    void m45e9f0(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                   /* 0x45e9f0 */
    void m45e770(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                   /* 0x45e770 */
    void m45ec70(struct ListNode *n, int check, int c, int d);
                                               /* 0x45ec70 */
};

struct __declspec(novtable) CatchSink1 { virtual void v0(int a, int b); };        /* 0x46c280 */
/* The lane-expiry sink, the catch member of the family. */
struct __declspec(novtable) CatchSink2
{
    virtual void v0(struct SlotSound *s, int a, int b);   /* 0x46c060 */
};
/* Broadcast one key event to every node of the run's player list
   (src/sink3.cpp). */
struct __declspec(novtable) CatchSink3 { virtual void v0(int key, int what); };  /* 0x46ec00 */

/* Ez2Catch's director; what start @0x46bfa0 touches plus what the destructor
   adds. */
struct CatchMainGameDirector : EzObject, CatchSink1, CatchSink2, CatchSink3, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x46bf10 */
    virtual DrawGroup     *getObjectInfo();    /* 0x46bf20 */
    virtual void           update();           /* 0x402880 (STUB, folded) */
    virtual void           update2(int a, int b); /* 0x46e550 (src/catchupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    /* Two DIFFERENT values in [base, base+span), or `base` twice after
       thirty-two ties. Two of its six arguments are unused, and `this`
       with them. */
    void m46dea0(int a, int base, int span, int d, int *outA, int *outB);
                                               /* 0x46dea0 */
    /* The catch copy of GFMainGameDirector::m45e5a0 - the same function on
       the same sink, four bytes along. */
    int  m46ce70(int h, unsigned int a, unsigned int limit, SlotState *out);
                                               /* 0x46ce70 */
    /* And its copy of EZ2DJMainGameDirector::m421510, likewise. */
    void m46cd30(int from, int to, int c, int d);
                                               /* 0x46cd30 */
    ~CatchMainGameDirector();                  /* 0x46d250 */

    char            _pad1c[0x38 - 0x1c];
    /* Set while the load has failed, cleared once it has succeeded
       (the chart loader @0x46dba0). */
    int             f38;       /* 0x038 */
    int             f3c;       /* 0x03c */
    Obj414750       pump;      /* 0x040 - start() tail-jumps its restart() */
    /* The catch director's own sample sink, four bytes below `notify` where
       the other two directors keep theirs at 0x148 - read by m46ce70. */
    struct SlotSampleSink *sink;  /* 0x164 */
    DirectorNotify *notify;    /* 0x168 */
    /* The Judge the chart loader @0x46dba0 builds and arms. */
    struct Judge   *judge;     /* 0x16c */
    struct ShowCredit *credit; /* 0x170 */
    struct ScrFader *f174;    /* 0x174 - what m450570 hands back */
    int             f178[0x60];/* 0x178 .. 0x2f8 */
    /* The same .gds-overridable set again, plus a RaceMode flag of its own
       (loadGds @0x46d8d0). */
    int             f2f8;      /* 0x2f8 - 0x63 */
    char            _pad2fc[0x304 - 0x2fc];
    int             f304;      /* 0x304 - 0x48 */
    int             f308;      /* 0x308 - 0x24 */
    int             f30c;      /* 0x30c - 0x18 */
    int             f310;      /* 0x310 - 6 */
    char            _pad314[0x318 - 0x314];
    float           f318;      /* 0x318 - -4.8 */
    float           f31c;      /* 0x31c - -1.8 */
    float           f320;      /* 0x320 - 0.1 */
    float           f324;      /* 0x324 - 0.2 */
    /* The COOL delta again, copied before the auto-play doubling
       (m46c2b0 @0x46c2b0). */
    float           f328;      /* 0x328 */
    float           f32c;      /* 0x32c - 1.5, then always 1.6 from the .gds */
    int             f330;      /* 0x330 - RaceMode, published to 0x1b2ebac */
    /* The measure length in ticks, the unit both time-sliced shuffles slice
       by - the EZ2DJ director's +0x34c and the scratch director's +0x318. */
    int             f334;      /* 0x334 */
    float           f338;      /* 0x338 */
    float           f33c;      /* 0x33c */
    int             f340;      /* 0x340 - Background */
    CatchTimed     *timed;     /* 0x344 */
    int             f348;      /* 0x348 - General's first int; also zeroed
                                   at the top of m46ca30 */
    int             f34c[0xde];/* 0x34c .. 0x6a4 */
    char            f6c4[0x744 - 0x6c4];  /* 0x6c4 - Panel, a name token */
    /* The catch copy of the GF pair, four bytes lower (m46c760 @0x46c760). */
    struct Effector         *m_effector;     /* 0x744 */
    struct Effector6thStyle *m_effector6th;  /* 0x748 */
    int             f74c;      /* 0x74c - Gauge */
    int             f750;      /* 0x750 */
    /* The fade level update2 @0x46e550 ramps, clamped up to 0xff. */
    int             f754;      /* 0x754 */
    float           f758;      /* 0x758 - the credit overlay's x, -1 = auto */
    float           f75c;      /* 0x75c - and its y */
    float           f760;      /* 0x760 - ShowCredit's second pair */
    float           f764;      /* 0x764 */
    int             f768;      /* 0x768 - General's third int */
    int             f76c;      /* 0x76c - and its fourth */
    MixerControl   *mixerA;    /* 0x770 - addControl(0, 2) */
    MixerControl   *mixerB;    /* 0x774 - addControl(0, 1) */
    MixerControl   *mixerC;    /* 0x778 - addControl(0, 0) */
    MixerControl   *mixerD;    /* 0x77c - addControl(1, 0) */
    /* Whether the owner's tracks are all idle, refreshed every tick. */
    int             f780;      /* 0x780 */
    /* JudgmentLink - two counts and two runs of 0x48-byte entries. See the
       handler in src/maingame.cpp; the layout is forced by all three
       directors agreeing on the deltas. */
    int             f784;      /* 0x784 - first count  */
    int             f788;      /* 0x788 - second count */
    char            _pad78c[0x790 - 0x78c];
    GdsLinkEntry    link_a[8];  /* 0x790 .. 0x9d0 */
    GdsLinkEntry    link_b[7];  /* 0x9d0 .. 0xbc8 */
    char            _padbc8[0xc0c - 0xbc8];
    int             fc0c;      /* 0xc0c */
    int             fc10[0x24];/* 0xc10 .. 0xca0 */
    int             fca0;      /* 0xca0 - 2 */
    int             fca4;      /* 0xca4 - handed to every ladder handler */
    DirectorPending *pending;  /* 0xca8 */
    /* Latched once the pending screen has reported ready and start() has
       been run for it. */
    char            bcac;      /* 0xcac */
    /* The song-table entry, as in the other two directors. */
    char            songEntry[0x56];  /* 0xcad .. 0xd03 */
    char            _padd03[0xd04 - 0xd03];
    unsigned int    started;   /* 0xd04 - timeGetTime() when start() ran */
    char            bd08;      /* 0xd08 */
    char            _padd09[0xd0c - 0xd09];

    CatchMainGameDirector();                   /* 0x46eca0 */
    int   loadGds(char *name);                 /* 0x46d8d0 */
    char *m46c360();                           /* 0x46c360 (STUB) */
    /* The .gds JudgmentDelta section (src/gaugerates.cpp). */
    int   m46c2b0(struct KParser *p);          /* 0x46c2b0 */
    /* The Ez2Catch copy of the pan stepper (src/panstep.cpp). */
    void  stepPan(int dir);                    /* 0x46c190 */
    void  m46c0f0(int level);                  /* 0x46c0f0 (src/panlevel.cpp) */
    /* Rebase every list-B player's sounds (src/accessors.cpp). Its body never
       touches `this`, but the original declared it a member - update2's call
       site sets ecx, which only a member call does. */
    void  rebaseListB();                       /* 0x46c020 */
    void start();      /* 0x46bfa0 */
    void m46d360();    /* 0x46d360 (STUB) */
    /* What the third sink hands the acted-on keys to. */
    void m46e750(int key, int flag);   /* 0x46e750 (STUB) */
    /* The catch twin of GFMainGameDirector::m45d8d0 - it releases the
       cursor clip where the GF one runs a helper. */
    void m46c240();    /* 0x46c240 */
    void m46c7e0();    /* 0x46c7e0 (STUB) */
    /* The catch copy of EZ2DJMainGameDirector::m4209d0. */
    void m46c440();    /* 0x46c440 */
    int  m46ca30(char *name);                  /* 0x46ca30 - the director's .gds */
    int  m46c820(struct KParser *p, int i);    /* 0x46c820 - SlotN      */
    int  m46c8c0(struct KParser *p);           /* 0x46c8c0 - ShowCredit */
    int  m46c900(struct KParser *p);           /* 0x46c900 - JudgmentLink */
    int  m46dba0();                            /* 0x46dba0 (STUB) */
    void m46e4c0();                            /* 0x46e4c0 (STUB) */
    /* Build the background: make the CatchBackGround, point it back at this
       director, and try the song's own .scr then the common fallbacks. */
    int  m46c4d0();                            /* 0x46c4d0 (src/catchbgload.cpp) */
    /* The catch copy of GFMainGameDirector::m45de70. */
    int  m46c760();                            /* 0x46c760 */
    /* The eight per-note handlers the constructor's mode ladder walks list B
       with. Same shape, different arities. */
    /* Mirror the player's lane list over the time range, swapping each
       pair through scratch lane 0x15 (src/catchdirector.cpp). */
    void m46cf00(struct ListNode *n, int a, int b);          /* 0x46cf00 */
    /* Mirror only the PLAYABLE lanes: collect them first, then swap the
       collected list end for end (src/catchdirector.cpp). */
    void m46cfc0(struct ListNode *n, int a, int b);          /* 0x46cfc0 */
    /* m46cfc0's twin over the OTHER six lane kinds, and one entry shorter
       (src/catchdirector.cpp). */
    void m46d0f0(struct ListNode *n, int a, int b);          /* 0x46d0f0 */
    void m46df00(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                   /* 0x46df00 */
    void m46e170(struct ListNode *n, unsigned int start, unsigned int limit,
                 int check);                   /* 0x46e170 */
    void m46e3e0(struct ListNode *n, int a, int b, int c);   /* 0x46e3e0 */
};

EZ2DJMainGameDirector *createMainGameDirector(); /* 0x424980 (STUB) */
CatchMainGameDirector *createCatchDirector();    /* 0x46f290 */
GFMainGameDirector    *createGFDirector();       /* 0x460740 */

SongSelectDirectorObj *createSongSelectDirector(); /* 0x4463e0 */
void *createGameOverDirector();   /* 0x44dbf0 */
struct EndingDirector *createEndingDirector();  /* 0x455fb0 */
struct DrawModeIntroDirector *createDrawModeIntroDirector();  /* 0x46a440 */
void *createModeSelectDirector();  /* 0x44c790 */
/* Screen48D498's handoff builds the scratch result screen (src/ezscreens.cpp). */
struct GFResultDirector *createGFResultDirector();        /* 0x469970 */
/* The result screen's two, picked between by mode (src/ezscreens.cpp). */
struct ResultDirector      *createResultDirector();       /* 0x454400 */
struct TotalResultDirector *createTotalResultDirector();  /* 0x455ad0 */

/* The engine's new-plus-ctor factories: each publishes its object record at
   0x530b74, calls operator new with the instance size and runs the ctor. */
ShowCredit    *createShowCredit();     /* 0x450390 */
TitleDirector *createTitleDirector();  /* 0x44f400 */
RankingDirector *createRankingDirector(); /* 0x45bbd0 (STUB) */

/* vftable 0x48d348 - run every round; its return code drives everything. */
struct Screen48D348 : Obj401780
{
    virtual void s4();   /* 0x44dc50 */
    virtual void s5();   /* 0x44dc80 */
};

/* vftable 0x48d310 - the round returned 2. */
struct Screen48D310 : Obj401780
{
    /* its OnCreateGame forwards to a factory; see below. 0x476830 - what s5 tail-jumps to */

    virtual void s4();   /* 0x476140 */
    virtual void s5();   /* 0x4761b0 */
};

/* vftable 0x48d428 - the round returned 3. */
struct Screen48D428 : Obj401780
{
    /* its OnCreateGame forwards to a factory; see below. 0x469bc0 - what s5 tail-jumps to */

    virtual void s4();   /* 0x4699d0 */
    virtual void s5();   /* 0x4699f0 */
};

/* vftable 0x48d364 - the screen every mode runner ends with (runEnding
   @0x415db0). Its slots 4 and 5 sit immediately in front of
   EndingDirector_getRuntimeClass @0x455ba0 (src/classinfo.cpp), i.e. in that
   class's TU, and slot 6 is the shared texture report. */
struct Screen48D364 : Obj401780
{
    virtual void s4();   /* 0x455b30 */
    virtual void s5();   /* 0x455b50 */
};

/* (The player-record types moved to include/players.h.) */

/* vftable 0x48d3d4 - run when m_f1c has gone to 0. */
struct Screen48D3D4 : Obj401780
{
    /* its OnCreateGame forwards to a factory; see below. 0x46ab20 - what s5 tail-jumps to */

    virtual void s4();   /* 0x46a4a0 */
    virtual void s5();   /* 0x46a4d0 */
};

/* The eight further screens the game-mode helpers in src/moderunner.cpp create.
   Same 7-slot shape as the five runGame builds: slots 0/1/2/3 and (mostly) 6 come
   from Obj401780, 4 and 5 are per screen. Named after their vftables, with the
   role each one's slot-4 records / slot-5 factory identifies. */
/* vftable 0x48d2f4 - the main game (EZ2DJMainGameDirector region). */
struct Screen48D2F4 : Obj401780
{
    virtual void s4();   /* 0x41f8c0 */
    virtual void s5();   /* 0x41f960 */
};

/* vftable 0x48d32c - game over. */
struct Screen48D32C : Obj401780
{
    virtual void s4();   /* 0x44d8f0 */
    virtual void s5();   /* 0x44d920 */
};

/* vftable 0x48d380 - the result screen (ResultDirector region). */
struct Screen48D380 : Obj401780
{
    virtual void s4();   /* 0x4505f0 */
    virtual void s5();   /* 0x450620 */
};

/* vftable 0x48d3b8 - song select (SongSelectDirector region). */
struct SongSelectScreen : Obj401780
{
    virtual void s4();   /* 0x430a80 */
    virtual void s5();   /* 0x430ab0 */
    virtual void s6();   /* 0x430af0 - overridden, not the shared report */
};

/* vftable 0x48d3f0 - ranking (RankingDirector region). */
struct Screen48D3F0 : Obj401780
{
    virtual void s4();   /* 0x4566f0 */
    virtual void s5();   /* 0x456710 */
};

/* vftable 0x48d444 - the per-mode intro. */
struct DrawModeIntro : Obj401780
{
    virtual void s4();   /* 0x469fd0 */
    virtual void s5();   /* 0x46a000 */
    virtual void s6();   /* 0x46a030 - overridden, not the shared report */
};

/* vftable 0x48d460 - the Catch game. */
struct Screen48D460 : Obj401780
{
    virtual void s4();   /* 0x46be80 */
    virtual void s5();   /* 0x46bf00 */
};

/* vftable 0x48d47c - the GF (ScratchMix) game. Never constructed through a
   factory: the two runners that show it (@0x417a30 and @0x417c70) allocate it
   with malloc and store the vptr inline, so no constructor of it exists
   anywhere and cl emits no vftable for it either - which is exactly the
   original's shape. */
struct Screen48D47C : Obj401780
{
    virtual void s4();   /* 0x45cfd0 */
    virtual void s5();   /* 0x45d050 */
};

/* vftable 0x48d498 - the GF result screen. */
struct Screen48D498 : Obj401780
{
    virtual void s4();   /* 0x466e80 */
    virtual void s5();   /* 0x466ea0 */
};

/* What a screen actually drives every frame (Obj401780::s1 @0x418620). A screen
   owns two lists of GROUPS - one walked to tick, one to draw - and each group
   owns a list of nodes. A group is on both lists at once, with a separate link
   for each; a node is ticked or drawn only when its flags say live (bit 0) and
   not hidden (bit 1). Only what s1 touches is modeled. */
struct DrawNode
{
    virtual void v0();
    virtual void v1(int flag);                       /* slot 1 */
    virtual void v2();
    virtual void draw();                             /* slot 3 */
    virtual void tick(unsigned int now, unsigned int elapsed); /* slot 4 */

    DrawNode     *prev;    /* 0x04 - operator new threads both links */
    DrawNode     *next;    /* 0x08 */
    unsigned int  flags;   /* 0x0c: bit 0 = live, bit 1 = hidden */
};

/* A group IS one of the {objectSize, count, flags} records a screen registers
   with itself through addInfo @0x401850 (src/classinfo.cpp holds the storage):
   m4017a0 @0x4017a0 sizes one pool per record and carves them all out of a single
   VirtualAlloc, so the record carries its pool bounds and its live-node list too. */
/* DrawGroup is EzObjectInfo, defined in include/bigbase.h. */

/* vftable 0x48d39c - the screen EZ2AC::m415f00 @0x415f00 runs to decide whether
   the game may start a mode; runGame gates its whole dispatch on the result. */
struct ModeSelectScreen : Obj401780
{
    virtual void s4();   /* 0x448eb0 */
    virtual void s5();   /* 0x448ee0 */
    virtual void s6();   /* 0x448f10 - overridden, not the shared report */
};

#endif /* EZSCREEN_H */
