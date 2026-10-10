#ifndef JUDGE_H
#define JUDGE_H

#include "panels.h"    /* EzObject */

/* "Judge" - the KRuntimeClass @0x4a90a8 its slot 0 returns names it, and the
   object record @0x4a90b8 gives {0x24f0, 1, 2}. Six-slot vtable @0x48ffb0,
   the same shape as the five panel classes in include/panels.h, with only
   update2 doing anything: slot 3 is the folded empty `ret` @0x402880 and
   slot 5 the shared `xor eax,eax; ret 0xc` @0x401770.

   Its destructor is empty - the deleting destructor @0x426e20 calls
   @0x426e40, which is a bare `jmp` to the base's folded `ret` - so the whole
   class is described by its constructor @0x426dd0, which zeroes it: two
   large int blocks with a single int between them and three more on the end.
   Nothing yet says what any of them hold. */
/* One lane's block inside the Judge's m_14 blob - base Judge+0x14, stride
   0x5c, count at +0x2294. Only what the grader @0x426500 reads is named: TWO
   six-entry window tables, early presses then late, and both are walked from
   the LAST entry DOWNWARD, so the tightest window is the highest address.
   The defaults @0x48ff98 are {-1, -1, 24, 12, 4, 2} in memory order, i.e.
   2 ticks for the top grade - and the two -1s are what make grades 1 and 0
   unreachable, since no |dt| is ever <= -1. */
struct JudgeLane
{
    /* 0x00 - THE AUTOPLAY FLAG. Settled 2026-08-23; this tree previously
       called it "a hold-only lane", which was wrong, and port/REQUESTS.md's
       "autoplay" was right. The chain:

         Judge::setup @0x426390 zeroes it for all 96 lanes (0x426429);
         the only non-zero writer is the stage setup, per track handle -
           `setF0(h, g_1b5f2e0 != 0)`   @0x426ca0, src/stagesetup.cpp:134
                                        and src/gfstagesetup.cpp:120
         beside `setKind(h, (g_1b5f2e0 != 0) + 1)` on the sound clock;
         the catch setup passes a hard 0 (src/catchstagesetup.cpp:116).

       So `kind == 1` iff the g_1b5f2e0 option is up, and tick() @0x426ab0
       then grades every hit 5 AT ITS START with no input at all - which is
       what autoplay is. The same option gives the sound kind 2, drives a
       panel effect from the keys commit, and gates a draw in
       src/gamepanels.cpp. */
    int  kind;                                 /* 0x00 - 1 = autoplay */
    int  f04;                                  /* 0x04 - live */
    char _pad08[0x20 - 0x08];
    int  windows[2][6];                        /* 0x20 early, 0x38 late */
    int  lane;                                 /* 0x50 - which lane slot claimed it */
    /* The tick's cursor into the sink - a SlotSound (index, id) it resumes
       `next` from; id -1 means start over with `begin`. */
    int  curIndex;                             /* 0x54 */
    int  curId;                                /* 0x58 */
};

struct Judge : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x426370 */
    virtual DrawGroup     *getObjectInfo();    /* 0x426380 */
    virtual void           update();           /* 0x402880 (folded) */
    virtual void           update2(int, int);  /* 0x426e50 (STUB) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    ~Judge();                                  /* 0x426e40 (folded) */
    Judge();                                   /* 0x426dd0 */

    /* Grade one press against one note. @0x426500 */
    int grade(JudgeLane *lane, int pressTime, int noteTime);
    /* Judge a press (against the hit's head) / a release (against a hold's
       tail) of sound handle `h` at time `now`: find the
       hit it belongs to, grade it, and (when `commit`) write the grade into
       the hit's state; the state and sound come back through the two out
       pointers when given. 0 = nothing to judge (or whatever m4266c0 says). */
    int judgePress(int now, int h, int commit, struct SlotState *outSt, struct SlotSound *outSnd);  /* 0x426700 */
    int judgeRelease(int now, int h, int commit, struct SlotState *outSt, struct SlotSound *outSnd);    /* 0x4268c0 */
    /* The per-frame pass: every live record's hits that fell past their
       window are missed (grade 1, or 5 on a hold-only lane) and reported to
       the director's second sink; a hold whose head was hit but whose tail
       ran out is closed with 5. update2 @0x426e50 is `tick()`. */
    void tick();                               /* 0x426ab0 */

    char _pad10[0x14 - 0x10];
    /* One 0x5c-byte record per sound handle - the grader's window tables. */
    JudgeLane lanes[96];                       /* 0x14 .. 0x2294 */
    int  f2294;                                /* 0x2294 */
    int  m_2298[0x90];                         /* 0x2298 */
    int  f24d8;                                /* 0x24d8 */
    int  f24dc;                                /* 0x24dc */
    int  f24e0;                                /* 0x24e0 - the SlotClock (setup @0x426390) */
    int  f24e4;                                /* 0x24e4 */
    /* Carved out of the tail pad, same size and alignment: the grader latches
       which side of the note the press fell on. */
    int  f24e8;                                /* 0x24e8 - early */
    int  f24ec;                                /* 0x24ec - late */
};

#endif /* JUDGE_H */
