#ifndef BACKGROUNDS_H
#define BACKGROUNDS_H

#include "panels.h"      /* EzObject, Obj40B2E0, BackGround itself */

/* The other two members of the background family. All three are ~1 MB objects
   with the same four members and a byte-for-byte identical destructor body -
   only the vtable constant and (for CatchBackGround) the offsets differ:

     vtable    name             size      dtor      deleting ~  helper  charts
     0x48e1b8  BackGround       0x103540  0x41aeb0  0x41b660    0x1c0   0x1006a0
     0x499df0  GFBackGround     0x103540  0x45c550  0x45cd20    0x1c0   0x1006a0
     0x49d708  CatchBackGround  0x103538  0x46ac80  0x46b990    0x1b8   0x100694

   BackGround and GFBackGround share slots 3 and 4 outright - the original's
   linker ICF-folded their update/update2 onto 0x45c930/0x45ca90 - which is why
   those two need `folded` annotations. */
/* Only the sub-object the script clock comes through, and only the one slot
   the script walker asks for. */
struct ScriptClock
{
    virtual void v0();  virtual void v1();  virtual void v2();  virtual void v3();
    virtual void v4();  virtual void v5();  virtual void v6();  virtual void v7();
    virtual void v8();  virtual void v9();  virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21();
    /* slot 22 -> +0x58: the run position in quarter-ticks. */
    virtual unsigned int tick();
};

struct ScriptOwner
{
    char         _pad0[0x14c];
    ScriptClock *clock;                        /* 0x14c */
};

/* The catch background reads the same clock through a DIFFERENT owner - its
   pointer sits at +0x168, not +0x14c, so the two are separate classes rather
   than one shifted layout. */
struct CatchScriptOwner
{
    char         _pad0[0x168];
    ScriptClock *clock;                        /* 0x168 */
};

/* What one of a background's three script latches points at.

   The state at +0x00 is what m45c840 @0x45c840 and m41b2f0 @0x41b2f0 both
   switch on: 1 arms the latch, 2 re-arms it while it is held. The cell table
   it owns is at +0x04 - NOT the latch itself, which is why m41b2f0 passes
   `table` to Obj402860::attach and m45c840 passes a null.

   Only the three offsets those two functions touch are evidenced. */
struct BgScriptLatch
{
    int  state;                    /* 0x00 */
    struct Obj402860Head *table;   /* 0x04 */
    char _pad8[0x84 - 8];
    int  f84;                      /* 0x84 */
};

struct GFBackGround : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x45c450 */
    virtual DrawGroup     *getObjectInfo();    /* 0x45c460 */
    virtual void           update();           /* 0x45c930 (STUB, folded) */
    virtual void           update2(int, int);  /* 0x45ca90 (folded, src/bgupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~GFBackGround();                           /* 0x45c550 */
    GFBackGround();                            /* 0x45cbb0 */

    char *videoName();                         /* 0x45c470 */
    /* Try one background script by name; nonzero once one has loaded. */
    int m45cd40(char *name);                   /* 0x45cd40 */

    int  f10;                                  /* 0x10 */
    int  f14;                                  /* 0x14 */
    /* The owner the script clock is read from: m45cb00's tick comes from
       this->owner->f14c through that object's slot 22. */
    ScriptOwner *owner;                        /* 0x18 */
    int  f1c;                                  /* 0x1c */
    int  f20;                                  /* 0x20 */
    int  f24;                                  /* 0x24 */
    char f28[0xa8 - 0x28];      /* 0x28 - the load-time readout */
    int  fa8;                                  /* 0xa8 */
    int  fac;                                  /* 0xac - the judgement notice gate */
    char _padb0[0xb8 - 0xb0];
    char m_videoPath[260];                     /* 0xb8 */
    char _pad1bc[0x1c0 - (0xb8 + 260)];
    Obj40B2E0 m_1c0;                           /* 0x1c0 */
    int  f698;                                 /* 0x698 */
    char _pad69c[0x6a0 - 0x69c];
    /* The .scr script m45cd40 @0x45cd40 reads whole - the same megabyte buffer
       at the same offset as BackGround's. */
    char m_scrText[0x100000];   /* 0x6a0 .. 0x1006a0 */
    Obj402860 m_charts[10];                    /* 0x1006a0 */
    GfxFlag   m_flag;                          /* 0x101bb8 */
    void     *m_mesh;                          /* 0x102074 */
    int       m_102078[40];                    /* 0x102078 */
    int       f102118;                         /* 0x102118 */
    /* The background script: a table of 0x14-byte records at +0x102134, an
       index and a count, and a cursor onto the current record. Each record is
       a measure, a beat within it, and three payload pointers. */
    int       m_scriptCount;                   /* 0x10211c */
    int       m_scriptIndex;                   /* 0x102120 */
    int       f102124;                          /* 0x102124 */
    void     *m_scriptLatch[3];                /* 0x102128 */
    struct BgScriptRec
    {
        int   measure;                         /* +0x00 */
        int   beat;                            /* +0x04 */
        void *payload[3];                      /* +0x08 */
    }         m_script[1];                     /* 0x102134 */
    char _pad102148[0x103534 - 0x102148];
    BgScriptRec *m_scriptScratch;              /* 0x103534 */
    BgScriptRec *m_scriptCur;                  /* 0x103538 */
    /* When the last judgement notice fired, in script-index units. */
    int  f10353c;                              /* 0x10353c */

    /* Fire the background script records whose time has come. */
    void m41b370(int a);                       /* 0x41b370 */
    /* Detach the chart a latched script owns, when that script is in
       state 1 or 2. Latch 0 does not clear the cursor. */
    void m45c840(int i);                       /* 0x45c840 */
    /* Arm one latch's chart: state 1 attaches its table unheld and, for any
       latch but 0 whose +0x84 is set, raises the cursor flag; state 2
       attaches it held. */
    void m41b2f0(int i);                       /* 0x41b2f0 */
    /* What update2 runs in place of the script walker while the alternate
       draw is off. */
    void m45c5f0(int slot, int rec);           /* 0x45c5f0 */
    void m45c6a0(int now);                     /* 0x45c6a0 */
    /* Flash the judgement notice, at most once every ten script steps.
       Called from the lane-expiry sinks with the grade. */
    void m45cb20(int grade, int flag);         /* 0x45cb20 */
};


struct CatchBackGround : EzObject, EzPooled
{
    /* Load <name> and hand back nonzero on success; the background
       loader @0x46c4d0 tries three names through it. */
    int m46b0e0();                              /* 0x46b0e0 */
    int m46b9b0();                              /* 0x46b9b0 */
    int m46bc30(char *name);                    /* 0x46bc30 */
    virtual KRuntimeClass *getRuntimeClass();  /* 0x46ab80 */
    virtual DrawGroup     *getObjectInfo();    /* 0x46ab90 */
    virtual void           update();           /* 0x46b570 (STUB) */
    virtual void           update2(int, int);  /* 0x46b6d0 (src/bgupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~CatchBackGround();                        /* 0x46ac80 */
    CatchBackGround();                         /* 0x46b820 */

    char *videoName();                         /* 0x46aba0 */
    /* BackGround::drawItem's twin, on this class's own mesh. */
    void  drawItem(struct BgItem *it);         /* 0x46afe0 */
    void  drawItem4(struct BgItem *it);        /* 0x46aed0 (src/bgdrawitem4.cpp) */
    void  drawList();                          /* 0x46b060 */
    /* Tick the three payload charts of the script record in force
       (src/bgscripttick.cpp). */
    void  m46b4d0();                           /* 0x46b4d0 */

    /* NOT the other two shifted by eight: the pair BackGround keeps at
       0x10/0x14 sits at 0xac/0xb0 here, right in front of the path buffer. */
    /* The owner the script clock is read from - see m46b3e0. */
    CatchScriptOwner *owner;                   /* 0x10 */
    /* The base time update2 measures the run from - at 0x14 here, where the
       other two keep it at 0x1c. */
    int  f14;                                  /* 0x14 */
    int  f18;                                  /* 0x18 */
    int  f1c;                                  /* 0x1c */
    char f20[0xa0 - 0x20];      /* 0x20 - the load-time readout */
    int  fa0;                                  /* 0xa0 */
    int  fa4;                                  /* 0xa4 - the judgement notice gate */
    char _pada8[0xac - 0xa8];
    int  fac;                                  /* 0xac */
    int  fb0;                                  /* 0xb0 */
    char m_videoPath[260];                     /* 0xb4 - 0xb4 + 260 == 0x1b8 */
    Obj40B2E0 m_1b8;                           /* 0x1b8 */
    int  f690;                                 /* 0x690 */
    /* The .scr script m46bc30 @0x46bc30 reads whole - the same megabyte, at
       this class's own offset. */
    char m_scrText[0x100000];   /* 0x694 .. 0x100694 */
    Obj402860 m_charts[10];                    /* 0x100694 */
    GfxFlag   m_flag;                          /* 0x101bac */
    void     *m_mesh;                          /* 0x102068 */
    BgSlot    m_items[10];                     /* 0x10206c */
    int       f10210c;                         /* 0x10210c */
    /* The background script, the same shape GFBackGround keeps 0xc higher. */
    int       m_scriptCount;                   /* 0x102110 */
    int       m_scriptIndex;                   /* 0x102114 */
    /* Cleared once the payload the flag guards has finished tweening
       (m46b4d0 @0x46b4d0); the twin's copy is BackGround::f102124. */
    int       f102118;                         /* 0x102118 */
    void     *m_scriptLatch[3];                /* 0x10211c */
    GFBackGround::BgScriptRec m_script[1];     /* 0x102128 */
    char _pad10213c[0x103528 - 0x10213c];
    GFBackGround::BgScriptRec *m_scriptScratch; /* 0x103528 */
    GFBackGround::BgScriptRec *m_scriptCur;     /* 0x10352c */
    /* When the last judgement notice fired, in script-index units. */
    int  f103530;                              /* 0x103530 */
    char _pad103534[0x103538 - 0x103534];

    /* GFBackGround::m41b370's twin over this class's own script. */
    void m46b3e0(int a);                       /* 0x46b3e0 */
    /* The catch copy of GFBackGround::m45c840, three members 0xc lower. */
    void m46b310(int i);                       /* 0x46b310 */
    /* The catch copy of GFBackGround::m45cb20 - no log lines. */
    void m46b7a0(int grade, int flag);         /* 0x46b7a0 */
    /* The catch copy of GFBackGround::m41b2f0. */
    void m46b360(int i);                       /* 0x46b360 */
    void m46add0(int slot, int rec);           /* 0x46add0 */
    void m46ad20(int slot, int rec);           /* 0x46ad20 */
    void m46ae40(int now);                     /* 0x46ae40 */
};

#endif /* BACKGROUNDS_H */
