#ifndef KSONGPLAYER_H
#define KSONGPLAYER_H

#include <windows.h>
#include <mmsystem.h>
#include "loader.h"   /* Log */
#include "panels.h"   /* EzObject - the root of the 0x4887d0 class */

/* Delegate/listener interface: only slot 34 (vtable offset 0x88) matters.
   Declared, never instantiated. */
struct Slot34Target
{
    virtual void v00(); virtual void v01(); virtual void v02();
    virtual void v03(); virtual void v04(); virtual void v05();
    virtual void v06(); virtual void v07(); virtual void v08();
    virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17();
    virtual void v18(); virtual void v19(); virtual void v20();
    virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26();
    virtual void v27(); virtual void v28(); virtual void v29();
    virtual void v30(); virtual void v31(); virtual void v32();
    virtual void v33();
    virtual void v34();   /* vtable offset 0x88 */
};

/* KLock: an event wrapper with NO standalone code in the binary - every
   ctor/dtor/method is inlined at its use sites (the bodies must stay in
   this header for that reason). "KLock::KLock() Creating Handle Failed"
   @0x4882f4 is its failure log (STRING-pinned in src/ksongplayerbase.cpp). */
class KLock
{
public:
    KLock(int manual, int initial)
    {
        m_h = CreateEventA(0, manual, initial, 0);
        if (m_h == 0)
            Log("KLock::KLock() Creating Handle Failed");
    }
    ~KLock()
    {
        if (m_h != 0)
        {
            CloseHandle(m_h);
            m_h = 0;
        }
    }
    /* wait() returns the WaitForSingleObject result: KSongPlayerBase::start
       @0x414630 branches on it (a 0-timeout poll of the "playing" gate). */
    DWORD wait(int timeout) { return WaitForSingleObject(m_h, timeout); }
    void set() { SetEvent(m_h); }
    void reset() { ResetEvent(m_h); }

    HANDLE m_h;
};

/* Playback-driver base, size 0x1c: rate(1000.0f)/pos/time + three KLock
   events (auto-signaled, manual, auto-signaled). novtable is load-bearing:
   the ctor @0x414c10 stores no vftable. The ctor/dtor live in
   src/ksongplayerbase.cpp - they must not be visible to the derived class's
   TU or cl inlines them into the (matched) KSongPlayer ctor. */
class __declspec(novtable) KSongPlayerBase
{
public:
    KSongPlayerBase();                        /* 0x414c10 */
    ~KSongPlayerBase();                       /* 0x414cd0 (non-virtual) */

    /* m_lock1 guards m_rate; m_lock2 is the manual-reset "playing" gate
       (start signals it, stop resets it); m_lock3 is the critical-section
       style gate the driver holds while it advances the song. */
    void setRate(float rate);                 /* 0x414600 */
    void start();                             /* 0x414630 */
    void stop(int lock);                      /* 0x414660 */
    void advance(DWORD now);                  /* 0x4146a0 */
    /* The one virtual: TitleDirector's song base @0x494d0c has exactly this
       one slot, so setTarget is KSongPlayer's own (slot 1 there). */
    virtual void step() = 0;                  /* slot 0 - `step`, not `update`: TitleDirector has both this and the screen update() */

    /* Public: the pump (Obj414750) writes m_pos/m_time on its slots directly,
       e.g. rewindAll @0x4148c0, rather than going through start(). */
public:
    float m_rate;   /* 0x04 = 1000.0f in base ctor */
    volatile float m_pos;  /* 0x08 = 0.0f */
    volatile DWORD m_time; /* 0x0c = timeGetTime() */
    KLock m_lock1;  /* 0x10 */
    KLock m_lock2;  /* 0x14 */
    KLock m_lock3;  /* 0x18 */
};

/* KSongPlayer: small playback driver, class name from the "KSongPlayer"
   string @0x488a70 directly before its 2-slot vtable @0x488a7c (pinned in
   src/ksongplayer.cpp). Object size 0x20; embedded instances are
   constructed at +0x7210/+0x7230 of a bigger class (vtable 0x4887d0), and
   a further subclass overwrites the vtable with 0x488790. */
class KSongPlayer : public KSongPlayerBase
{
public:
    KSongPlayer();                            /* 0x412970 */
    virtual void step();                      /* slot 0 @0x4129a0 */
    virtual void setTarget(Slot34Target *v);  /* slot 1 @0x412990 */

    Slot34Target *m_target;   /* 0x1c */
};

/* The big owner class whose ctor @0x4102c0 embeds two KSongPlayers (at
   +0x7210 and +0x7230) and registers ITSELF as the second one's target - its
   tick @0x40fe90 works on this same object's fields at +0x7250/+0x7254.
   Only that one method is modelled here (STUB in src/stubs.cpp). */
/* One entry of the table tick() walks; only the object it starts with is
   known, and only that its slot 17 is what gets ticked. */
struct TickTarget
{
    virtual void v0();  virtual void v1();  virtual void v2();
    virtual void v3();  virtual void v4();  virtual void v5();
    virtual void v6();  virtual void v7();  virtual void v8();
    virtual void v9();  virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16();
    virtual void v17();                       /* slot 17 -> vtable+0x44 */
};

/* The KSongPlayer subclass used for the embedded player at +0x7230: the
   outer ctor calls KSongPlayer::KSongPlayer and then patches the vptr to
   0x488790, so the subclass has no constructor of its own and overrides
   only update(). No name string exists for it anywhere, hence the repo's
   address-derived naming. */
struct Obj410100 : KSongPlayer
{
    /* Declared out-of-line purely so cl emits the vtable: MSVC only lays a
       class's vftable down in the TU where it generates a ctor/dtor, and in
       the original this ctor exists only INLINED into the owner's ctor
       (`call KSongPlayer::KSongPlayer; mov [edi],0x488790`). The standalone
       copy our build emits is unannotated and therefore unscored. */
    /* Defined INLINE on purpose: the owner's constructor builds this member
       with `call KSongPlayer::KSongPlayer; mov [edi],vftable` and no call of
       its own, so cl must be able to expand it. Declaring it at all is still
       what makes cl lay the vftable @0x488790 down. */
    Obj410100() {}
    virtual void step();                      /* slot 0 @0x410100 */
};

/* The class behind the eight-slot vtable @0x4887d0 - the "bigger class" the
   comment on KSongPlayer above already predicted. Four members with
   destructors, which is what its constructor's EH states 0..3 and its
   destructor's opening 4 count: two KSongPlayers and two KLocks. The second
   player is the Obj410100 subclass, so its construction is
   `call KSongPlayer::KSongPlayer; mov [edi],0x488790` with no ctor of its own.

   Slots 5, 6 and 7 hand back the ROOT's three dwords (EzObject::f04/f08/f0c);
   the first of them is the counted table tick() walks. */
struct KEZPlayer : EzObject
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x40f320 -> &classKEZPlayer */
    virtual ~KEZPlayer();                     /* ??_G 0x4104a0, ??1 0x410390 */
    /* Bind an EzChart-derived object as the player's chart, but only while
       nothing is playing. Returns whether it took. */
    virtual int  v2(void *obj);                /* 0x40f8f0 */
    /* Bind a KSound-derived object as the player's source, but only while
       nothing is playing. Returns whether it took. */
    virtual int  v3(void *obj);               /* 0x40f340 */
    /* Bind the pump and start both embedded players: the second gets a flat
       1000, the first the rate derived from the sample's frame size. */
    virtual int  v4(void *pump);              /* 0x40f960 */
    virtual int  getA();                      /* 0x40f380 (folded) */
    virtual int  getB();                      /* 0x40f390 */
    virtual int  getC();                      /* 0x40f3a0 */
    /* Slots 8..34 - the full 35-slot table (extended 2026-10-01; it used to
       be modelled as this 8-slot prefix). Every slot is reached only
       through the table. The `folded` ones share an address the linker
       ICF'd with an identical body elsewhere; the ones whose bodies were
       first written against stand-in classes forward into them
       (src/misc.cpp, src/accessors.cpp, src/getbit.cpp). */
    virtual int   play();                     /* 0x40fa00 - slot 8 */
    virtual void  stop();                     /* 0x40fad0 - slot 9 */
    /* Teardown: take the idle lock, drop the gate, stop both players, clear
       the tick table and the scratch, and release. */
    virtual void  m410230();                  /* 0x410230 - slot 10 */
    virtual void  setMasterVolume(int v);     /* 0x40f3b0 - slot 11 */
    virtual int   v12(int, int);              /* 0x40f420 (folded) - slot 12 */
    virtual void  setEntryVolume(int i, int v); /* 0x40fb10 - slot 13 */
    virtual int   getEntryVolume(int i);      /* 0x40fb50 - slot 14 */
    virtual void  m40f430(int);               /* 0x40f430 - slot 15 */
    virtual int   v16(int, int);              /* 0x40f420 (folded) - slot 16 */
    virtual void  setEntryPan(int i, int v);  /* 0x40fb70 - slot 17 */
    virtual int   getEntryState(int i);       /* 0x40fbb0 - slot 18 */
    virtual void  v19(int);                   /* 0x40f430 (folded) - slot 19 */
    virtual float v20();                      /* 0x40f440 (folded) - slot 20 */
    virtual void  v21(int);                   /* 0x40f430 (folded) - slot 21 */
    virtual int   getClock();                 /* 0x40f450 - slot 22 */
    virtual void  v23(int);                   /* 0x40f430 (folded) - slot 23 */
    virtual float v24();                      /* 0x40f440 (folded) - slot 24 */
    virtual float getF7260();                 /* 0x40f470 - slot 25 */
    virtual int   v26(int, int, int);         /* 0x40f480 (folded) - slot 26 */
    virtual int   getI7274();                 /* 0x40f490 - slot 27 */
    virtual int   getEntryBit(int i);         /* 0x40fbd0 - slot 28 */
    virtual void  runLanes(int a);            /* 0x40f610 - slot 29 */
    virtual void  setEntryState(int i, int mode); /* 0x40ff00 - slot 30 */
    virtual int   entryKind(int i);           /* 0x40ff50 - slot 31 */
    virtual void  reseedLane(int i);          /* 0x40ffa0 - slot 32 */
    virtual void  runEntry(int i);            /* 0x4100d0 - slot 33 */
    virtual void  drainQueue();               /* 0x40fd70 - slot 34 */

    KEZPlayer();                              /* 0x4102c0 */

    /* NOT pooled: its deleting destructor ends in the CRT operator delete, not
       EzObject::operator delete. */
    static void operator delete(void *p) { ::operator delete(p); }

    void tick();                              /* 0x40fe90 */
    /* Load one sample slot through the object at +0x08 and record it in the
       pair array. The loop is written 1..2 in the original. */
    int  createSample(int slot, int sample);  /* 0x40f7b0 */
    /* Walk the .ezi sample table and create every slot that needs it. */
    int  createSamples();                     /* 0x40f800 */
    /* Put the player back to its just-constructed state: both embedded
       players re-aimed at this object, every counter cleared, the speed
       back to 120 and both tables wiped (src/kezplayerreset.cpp). */
    void m410110();                           /* 0x410110 */
    /* Rebuild the 96-entry tick table - the reset's tail call. */
    void m40f690();                           /* 0x40f690 (src/kezplayertable.cpp) */
    /* Full stop: drop both embedded players from the pump, clear the
       bindings and re-arm the gate. */
    void m4101c0();                           /* 0x4101c0 */
    void m40f880();                           /* 0x40f880 (STUB) */

    /* How many entries of the table below are live lives in the object the
       root's f04 points at, which tick() re-reads on every iteration - a v17()
       may change it. */
    /* What the root's f04 points at: the sample the player is bound to. It is
       polymorphic (start() calls its slot 23), +0x88 is its frame size, +0x8c
       the base speed and +0x98 the live entry count tick() re-reads. Only the
       slot shape matters - it is never instantiated here. */
    struct Counted
    {
        virtual void q0();  virtual void q1();  virtual void q2();
        virtual void q3();  virtual void q4();  virtual void q5();
        virtual void q6();  virtual void q7();  virtual void q8();
        virtual void q9();  virtual void q10(); virtual void q11();
        virtual void q12(); virtual void q13(); virtual void q14();
        virtual void q15(); virtual void q16(); virtual void q17();
        virtual void q18(); virtual void q19(); virtual void q20();
        virtual void q21(); virtual void q22();
        virtual void q23();                 /* slot 23 -> +0x5c */

        char  _pad4[0x88 - 4];
        int   f88;      /* 0x88 - the frame size */
        float f8c;      /* 0x8c - the base speed */
        char  _pad90[0x98 - 0x90];
        int   count;    /* 0x98 */
    };
    /* The 96-entry tick table, 0x30 bytes each, filling 0x10 .. 0x1210
       exactly. m40f690 @0x40f690 builds it from the base; m410230 @0x410230
       clears only `obj`, which is why that one indexes from +0x34 - the
       addresses are the same either way. */
    struct Entry
    {
        int   f00;                 /* +0x00 - 1 */
        int   flags;               /* +0x04 - bit 0 is cleared by m40fd20 */
        int   f08;                 /* +0x08 */
        int   f0c;                 /* +0x0c - -1 */
        int   f10;                 /* +0x10 - -1 */
        int   f14;                 /* +0x14 - -1 */
        int   f18;                 /* +0x18 - 0x7f */
        int   f1c;                 /* +0x1c - 0x7f */
        int   f20;                 /* +0x20 - 0x40 */
        TickTarget *obj;           /* +0x24 */
        void *src;                 /* +0x28 - into the sample's own table */
        int   f2c;                 /* +0x2c */
    } entries[96];                            /* 0x10 .. 0x1210 */
    /* Two words per .ezi sample slot: the sample pointer, then whatever
       createSample @0x40f7b0 loads for it. Cleared whole by createSamples
       @0x40f800, which is what fixes the 0x800 pairs. */
    int  m_pairs[0x1000];                     /* 0x1210 .. 0x5210 */
    int  m_scratch[0x800];                    /* 0x5210 .. 0x7210 */
    KSongPlayer player0;                      /* 0x7210 */
    Obj410100   player1;                      /* 0x7230 - vptr 0x488790 */
    KLock       ev0;                          /* 0x7250 - the lock */
    KLock       ev1;                          /* 0x7254 - the gate */
    int   f7258;                              /* 0x7258 */
    int   f725c;                              /* 0x725c */
    float f7260;                              /* 0x7260 - the playback speed */
    /* Two more counters the reset @0x410110 zeroes with the rest. */
    int   f7264;                              /* 0x7264 */
    int   f7268;                              /* 0x7268 */
    int   f726c;                              /* 0x726c - the master level the
                                                 volume setter scales by */
    char  _pad7270[0x7274 - 0x7270];
    int   f7274;                              /* 0x7274 - 1 once playing */

    /* Stop every live tick target and drop bit 0 of its entry. */
    void m40fd20();                           /* 0x40fd20 */
    int   f7278;                              /* 0x7278 */
                                              /* 0x727c - what the factory
                                                 allocates */

    /* The three that used to be the address-named Obj40F470 in
       src/accessors.cpp: two getters and the setter that derives the base
       player's rate from the bound sample's own divisor. */
    void  setSpeed(float v);                  /* 0x40f5c0 */
    /* Arm everything and let both players run. Idempotent - the flag at
       +0x7274 is what makes the second call do only the two starts. */
    /* Blank the tick table and re-seed it from the bound sample's own
       table: every entry gets its source pointer, and the ones whose
       source has a non-zero +0x48 are marked live. */
    void m40f730();                           /* 0x40f730 */
    /* Let go: take the lock, close the gate, mark stopped, release. */
    /* The three that used to be the address-named Obj40F4A0 in
       include/sound.h - its this+0x726c is this class's. */
    void  setVolume(struct SoundParam *a, int b);  /* 0x40f4a0 */
    void  setPan(struct SoundParam *a, int b);     /* 0x40f550 */
    void  m40f520(struct SoundParam *a);           /* 0x40f520 (STUB) */
    /* Look the sample for id `w` up in the bind table. */
    struct IVolTarget *m40f650(int w);             /* 0x40f650 (src/kezplayertable.cpp) */
    /* Rebind a channel to that sample (src/sound.cpp). */
    int   m40fbf0(struct SoundParam *a, int w, int b); /* 0x40fbf0 */
    /* Apply one queued event to a channel. */
    void  applyEvent(struct SoundParam *a, struct SoundEvent *ev); /* 0x40fc50 */

};

KEZPlayer *createKEZPlayer();                 /* 0x410440 */


#endif /* KSONGPLAYER_H */
