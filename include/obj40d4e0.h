#ifndef OBJ40D4E0_H
#define OBJ40D4E0_H

#include <windows.h>
#include "kbase.h"
#include "icom.h"        /* IThing */
#include "ksongplayer.h" /* KLock */

/* The intermediate base the subsystem classes share: KBase plus ~0xa6 bytes of
   its own, all of it zeroed by its constructor @0x40d0f0 and none of it needing
   a destructor (the derived destructors reach KBase's thunk @0x426e40 straight
   away). __declspec(novtable) for the usual reason - it never stores a vftable
   of its own. */
struct __declspec(novtable) SubsystemBase : KBase
{
    SubsystemBase();               /* 0x40d0f0 */

    int  f04;
    int  f08;
    int  f0c;
    int  f10;
    char f14[18];                  /* 0x14 .. 0x26 - cleared as one block */
    char f26[0x80];                /* 0x26 .. 0xa6 - unaligned, so char */
    char _pada6[0xac - 0xa6];
};

/* The class behind the eight-slot vtable @0x488320: two events, one COM object
   and a handful of counters. Constructor @0x40d4e0, destructor @0x40d5a0. */
struct Obj40D4E0 : SubsystemBase
{
    virtual void *getClassDesc();  /* 0x40d130 -> &g_4a72d4 */
    virtual ~Obj40D4E0();          /* ??_G 0x40d9f0, ??1 0x40d5a0 */
    virtual void setPair(int a, int b);  /* 0x40d0d0 */
    virtual void v3();             /* 0x40d140 (STUB) */
    /* Latch bit 1 of f04 from the argument, mark f0ac dirty, push the
       resulting run flag into the COM object and, while bit 0 of f04 is
       up, wake the gate. */
    virtual void v4(int on);       /* 0x40d640 */
    /* Stop: take the lock, clear the dirty bit, tell the COM object to
       stop and rewind, and release the lock. */
    virtual void v5();             /* 0x40d6a0 */
    virtual void put16(int v);     /* 0x40d290 */
    virtual int  get7();           /* 0x40d2b0 */
    /* Slots 8..18, the rest of the 19-slot vftable. Their bodies were
       first written against placeholder views of this same object -
       EngineObj (src/com_wrappers.cpp), Obj40D420 (src/accessors.cpp) and
       Obj40D230 (src/obj40d4e0.cpp) - and the slots now carry the markers,
       forwarding into those bodies where they are kept. Nothing calls any
       of these directly; every use is through the table. */
    virtual void put15(int v);     /* 0x40d2e0 - slot 8 */
    virtual int  get6();           /* 0x40d300 - slot 9 */
    virtual void put17(int v);     /* 0x40d330 - slot 10 */
    virtual int  get8();           /* 0x40d350 - slot 11 */
    virtual void seek(int pos);    /* 0x40d380 - slot 12 */
    virtual int  get4();           /* 0x40d3f0 - slot 13 */
    virtual int  scaleTo(float k); /* 0x40d420 - slot 14 */
    virtual float ratio();         /* 0x40d460 - slot 15 */
    virtual int  targetBusy();     /* 0x40d490 - slot 16 */
    virtual int  refill();         /* 0x40d700 - slot 17 */
    virtual int  stopTarget(int pos); /* 0x40d230 - slot 18 */

    Obj40D4E0();                   /* 0x40d4e0 */

    int         fac;               /* 0x0ac */
    /* Two KLocks. Their inlined constructors ARE the two CreateEventA calls and
       the two failure logs; their inlined destructors ARE the two CloseHandle
       blocks, in reverse order. That is also where the constructor's EH states
       0 and 1 and the destructor's initial state 2 come from - the members are
       tracked even though neither body emits a call. */
    KLock       ev0;               /* 0x0b0 - KLock(0, 1) */
    KLock       ev1;               /* 0x0b4 - KLock(1, 0) */
    IThing     *fb8;               /* 0x0b8 - released through slot 2 */
    int         fbc;               /* 0x0bc */
    int         fc0;               /* 0x0c0 */
    int         fc4;               /* 0x0c4 */
    int         fc8;               /* 0x0c8 - -1 */
    int         fcc;               /* 0x0cc */
};

/* The destructor calls its own slot 5 through the vtable CONSTANT rather than
   through the stored vptr, which only happens when the call is made on a
   pointer of a DIFFERENT class type - see the ctor/dtor rule in CLAUDE.md. */
struct Obj40D4E0Mirror
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4();
    virtual void q5();
};

#endif /* OBJ40D4E0_H */
