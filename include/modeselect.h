#ifndef MODESELECT_H
#define MODESELECT_H

#include "panels.h"      /* EzObject, KRuntimeClass, DrawGroup */
#include "obj402860.h"

/* ModeSelectDirector - the mode-select screen's director. Named by its own
   KRuntimeClass @0x4ac748 and sized by its object record @0x4ac758: 0x9630
   bytes, count 1, flags 6.

   Its destructor @0x448f70 is the member map, and this one is unusually tidy -
   everything from +0xf0 to +0x9398 is back-to-back with no padding at all:

     state  0     Obj402860[12]  @0xf0     (via __ehvec_dtor_iterator @0x47a12b)
     states 1-14  Obj402860      @0x1a40..0x35ac, fourteen SEPARATE members
                                 (separate `lea ecx; call` pairs, not an array)
     state  15    Obj402860[36]  @0x37c8   (iterator again)
     state  16    Obj40B2E0      @0x83b8
     state  17    Obj402860[5]   @0x890c
     state  -1    the base

   Eighteen members, which is exactly the state the destructor is entered at
   (0x12). */
struct ModeSelectDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x448f50 */
    virtual DrawGroup     *getObjectInfo();    /* 0x448f60 */
    virtual void           update();           /* 0x4491b0 (src/modeselectupdate.cpp) */
    virtual void           update2(int, int);  /* 0x44c860 (src/modeselectupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~ModeSelectDirector();                     /* 0x448f70 */

    int       f10;           /* 0x0010 */
    int       f14;           /* 0x0014 - 12 while g_1696420 is clear, else 8 */
    int       f18;           /* 0x0018 */
    /* The last f18 the selection blip fired for. */
    int       f1c;           /* 0x001c */
    int       f20;           /* 0x0020 - 0x14 */
    int       f24;           /* 0x0024 */
    char _pad28[0x30 - 0x28];
    int       f30[9];        /* 0x0030 .. 0x0054 */
    char _pad54[0xcc - 0x54];
    int       fcc;           /* 0x00cc */
    float     fd0;           /* 0x00d0 */
    int       fd4;           /* 0x00d4 */
    int       fd8;           /* 0x00d8 */
    int       fdc;           /* 0x00dc */
    int       fe0;           /* 0x00e0 */
    int       fe4;           /* 0x00e4 */
    int       fe8;           /* 0x00e8 */
    int       fec;           /* 0x00ec */
    Obj402860 m_f0[12];        /* 0x00f0 */
    Obj402860 m_1a40;          /* 0x1a40 */
    Obj402860 m_1c5c;          /* 0x1c5c */
    Obj402860 m_1e78;          /* 0x1e78 */
    Obj402860 m_2094;          /* 0x2094 */
    Obj402860 m_22b0;          /* 0x22b0 */
    Obj402860 m_24cc;          /* 0x24cc */
    Obj402860 m_26e8;          /* 0x26e8 */
    Obj402860 m_2904;          /* 0x2904 */
    Obj402860 m_2b20;          /* 0x2b20 */
    Obj402860 m_2d3c;          /* 0x2d3c */
    Obj402860 m_2f58;          /* 0x2f58 */
    Obj402860 m_3174;          /* 0x3174 */
    Obj402860 m_3390;          /* 0x3390 */
    Obj402860 m_35ac;          /* 0x35ac */
    Obj402860 m_37c8[36];      /* 0x37c8 */
    Obj40B2E0 m_83b8;          /* 0x83b8 */
    struct KTexture *timeFont[10];   /* 0x8890 - TimeFont\Time_%d.bmp */
    struct KTexture *fadeBlack;      /* 0x88b8 */
    char _pad88bc[0x88c0 - 0x88bc];
    /* Loaded with Video_Blank_256 for the fade-out's video handoff. */
    struct KTexture *f88c0;          /* 0x88c0 */
    char _pad88c4[0x88dc - 0x88c4];
    struct KTexture *logo[12];       /* 0x88dc - vf_inout\Logo_%s.bmp */
    Obj402860 m_890c[5];       /* 0x890c - the five category-glow charts */
    struct KTexture *basicMode[3];   /* 0x9398 - ModeText\basicmode_%s.bmp */
    struct KTexture *stco[4];        /* 0x93a4 - ModeText\stco_%s.bmp */
    struct KTexture *course[4];      /* 0x93b4 - ModeText\course_%s.bmp */
    struct KTexture *casualCatch;    /* 0x93c4 */
    struct KTexture *casualScratch;  /* 0x93c8 */
    struct KTexture *specialCv2;     /* 0x93cc */
    struct KTexture *specialLegacy;  /* 0x93d0 */
    struct KTexture *cateSpecial;    /* 0x93d4 */
    struct KTexture *cateSpecialM;   /* 0x93d8 */
    struct KTexture *bgGrad;         /* 0x93dc */
    int       f93e0;         /* 0x93e0 */
    int       f93e4[5];      /* 0x93e4 .. 0x93f8 */
    int       f93f8[4];      /* 0x93f8 .. 0x9408 */
    char _pad9408[0x940c - 0x9408];
    int       f940c[4];      /* 0x940c .. 0x941c */
    char _pad941c[0x9420 - 0x941c];
    float     f9420[5];      /* 0x9420 .. 0x9434 - all 512 */
    int       f9434;         /* 0x9434 */
    /* The mode cursor's eased x - chases f38 * 28 - 40 (62 on the
       first page), snapping inside +-0.5. */
    float     f9438;         /* 0x9438 */
    struct KTexture *notice;         /* 0x943c */
    struct KTexture *noticeM;        /* 0x9440 */
    float     f9444;         /* 0x9444 - 520 */
    int       f9448;         /* 0x9448 - 0xff */
    int       f944c;         /* 0x944c */
    float     f9450;         /* 0x9450 - 0.6 */
    /* update draws the 2P-invite panel once this passes 10. */
    int       f9454;         /* 0x9454 */
    /* Cleared by the page-cycle chord. */
    int       f9458;         /* 0x9458 */
    /* The hidden-command entry buffers, one per stream: every key the panel
       reports held is appended to both and both are matched against the code
       table (m44c690 @0x44c690). */
    int       cmdBuf[2][20]; /* 0x945c .. 0x94fc */
    int       cmdLen[2];     /* 0x94fc - [0] is what clearCommand clears */
    /* One per code: set once it has fired, so it only fires once. */
    int       cmdDone[3];    /* 0x9504 .. 0x9510 */
    float     f9510;         /* 0x9510 - 6 */
    struct KTexture *noticeShadow;   /* 0x9514 */
    struct KTexture *diffStyle[2];   /* 0x9518 - diffstyleselect_%02d.bmp */
    struct KTexture *diffStyleCur;   /* 0x9520 */
    struct KTexture *black;          /* 0x9524 - 0_black.bmp */
    struct KTexture *modeText[12];   /* 0x9528 - modetext%02d.bmp */
    struct KTexture *modeTextM[12];  /* 0x9558 - modetext%02d_m.bmp */
    float     f9588[4];      /* 0x9588 .. 0x9598 */
    int       f9598;         /* 0x9598 */
    void     *sndNoticeTap;  /* 0x959c */
    void     *sndSelect;     /* 0x95a0 */
    void     *sndDecide;     /* 0x95a4 */
    char _pad95a8[0x95ac - 0x95a8];
    void     *sndModeTap;    /* 0x95ac */
    char _pad95b0[0x95bc - 0x95b0];
    void     *sndTimeCount;  /* 0x95bc */
    void     *sndHidden;     /* 0x95c0 */
    void     *sndModeIntro;  /* 0x95c4 */
    void     *sndStart;      /* 0x95c8 */
    char _pad95cc[0x95d4 - 0x95cc];
    /* Cleared with the option counters on every mode step. */
    int       f95d4;         /* 0x95d4 */
    int       f95d8;         /* 0x95d8 - 0x28 */
    int       f95dc;         /* 0x95dc */
    int       f95e0;         /* 0x95e0 */
    int       f95e4;         /* 0x95e4 */
    int       f95e8;         /* 0x95e8 - 10 */
    int       f95ec;         /* 0x95ec */
    int       f95f0;         /* 0x95f0 */
    int       f95f4;         /* 0x95f4 */
    int       f95f8;         /* 0x95f8 */
    int       f95fc;         /* 0x95fc */
    int       f9600;         /* 0x9600 */
    int       f9604;         /* 0x9604 */
    float     f9608;         /* 0x9608 */
    float     f960c;         /* 0x960c - 0.02 */
    int       f9610;         /* 0x9610 */
    int       f9614;         /* 0x9614 */
    void     *sndBgm;        /* 0x9618 */
    /* Set once the BGM has started - gates the coin lamp tick. */
    int       f961c;         /* 0x961c */
    int       f9620;         /* 0x9620 */
    int       f9624;         /* 0x9624 */
    int       f9628;         /* 0x9628 */
    char _pad962c[0x9630 - 0x962c];

    ModeSelectDirector();                      /* 0x44b8f0 */
    /* Latch every button of the panel map that is currently held. */
    void clearCommand();                       /* 0x44c7f0 */
    /* Called twice by update2 @0x44c860: claim a slot for `id` if the
       input board has a pending event. */
    void m44b290(int id);    /* 0x44b290 */
    /* One frame of the coin lamp - TitleDirector::tickCoinLamp @0x44e880
       without the attract song or the period rescale (src/modeselectctor.cpp).
       `dev` is a TitleLampDev (include/ezscreen.h); it is taken as void * so
       this header need not pull that one in. */
    void tickCoinLamp(void *dev, int frames);  /* 0x44b450 */
    /* Append one key to a stream and match the result against the three
       hidden codes (src/modeselectctor.cpp). */
    void m44c690(int slot, int key);           /* 0x44c690 */
    /* The one-second countdown and what happens when it runs out: pick the
       mode name out of the table, decide the game kind, and seat the
       players for it (src/modeselecttimer.cpp). */
    void m44b750();                            /* 0x44b750 */
    /* The same confirm, reached from a player's buttons instead of the
       clock: `who` is 1 or 2. DECODED but not matched - see
       wip/modeselect-buttons-44b4b0.cpp. */
    void m44b4b0(int who);                     /* 0x44b4b0 (wip) */
};

#endif /* MODESELECT_H */
