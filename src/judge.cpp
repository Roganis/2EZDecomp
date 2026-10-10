/* "Judge" @0x48ffb0 - a sixth class with the panel family's six-slot vtable,
   reached only from the screen constructor @0x426f7b. Everything known about
   it comes from its own constructor: it is one big zeroed object. */

#include <string.h>
#include "judge.h"
#include "players.h"

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classJudge;    /* 0x4a90a8 */
extern EzObjectInfo  g_info_4a90b8; /* 0x4a90b8 */

// FUNCTION: EZ2AC_UNPACKED 0x00426370
KRuntimeClass *Judge::getRuntimeClass()
{
    return &classJudge;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426380
DrawGroup *Judge::getObjectInfo()
{
    return &g_info_4a90b8;
}

/* Judge declares NO destructor of its own: the deleting destructor @0x426e20
   calls @0x426e40, which is a bare `jmp` into the folded `ret` with no vptr
   store in front of it. A destructor of Judge's own would have to restore the
   vptr first (that is what every panel class's does), so @0x426e40 can only be
   the BASE's - and the class is left with just the compiler-generated ??_G. */
// FUNCTION: EZ2AC_UNPACKED 0x00426e20
// Judge::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void Judge::update()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int Judge::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048ffb0
struct Judge;

/* Two zero blocks with a lone int between them, and three more on the end.
   The 24-byte rule does not apply at this size - 0x2280 and 0x240 bytes are
   unambiguously memset, and cl expands both to `rep stosd`. */
// FUNCTION: EZ2AC_UNPACKED 0x00426dd0
Judge::Judge()
{
    f24dc = 0;
    f24e0 = 0;
    f24e4 = 0;
    f2294 = 0;
    memset(lanes, 0, sizeof(lanes));
    f24d8 = 0;
    memset(m_2298, 0, sizeof(m_2298));
}

extern EzObjectInfo g_info_4a90b8;            /* 0x4a90b8 */

/* Judge's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x00426f40
Judge *createJudge()
{
    g_objectRecord = &g_info_4a90b8;
    return new Judge;
}

/* THE PRESS VERDICT (event kind 2) - see wip/judge-release-426700.cpp for
   the full analysis (the file keeps its old, wrong name). The original
   keeps three separate `return m4266c0` blocks, two byte-identical: they
   called three ICF-folded copies (m4266c0 / m4266c0b / m4266c0c, all
   @0x4266c0), which is the only way cl keeps such tails apart. Semantically
   complete. Find the hit a press of sound h at `now` belongs to
   (the nearer of a hold hit and its successor, else a plain hit; a hit whose
   head is already graded, or none, is the wrong-key FAIL m4266c0), grade it
   against the hit's START, and with `commit` write the head grade f7 - and
   f8 as well for a tap or a grade below the hold threshold, which closes it -
   and hand it back to the sink. Its caller commits with flag 1, the arm of
   m42fd90 that starts a hold. (Named judgeRelease until 2026-09-28; the
   port's REQUESTS.md caught the swap.) */
struct Obj426CA0 { int m4266c0(int a, int i); int m4266c0b(int a, int i); int m4266c0c(int a, int i); };   /* src/accessors.cpp @0x4266c0 */

struct JClock
{
    virtual void s0();  virtual void s1();  virtual void s2();  virtual void s3();
    virtual void s4();  virtual void s5();  virtual void s6();  virtual void s7();
    virtual void s8();  virtual void s9();  virtual void s10(); virtual void s11();
    virtual void s12(); virtual void s13(); virtual void s14(); virtual void s15();
    virtual void s16(); virtual void s17(); virtual void s18(); virtual void s19();
    virtual void s20(); virtual void s21();
    virtual unsigned int now();             /* slot 22 -> +0x58 */
    virtual void s23();
    virtual void s24(); virtual void s25(); virtual void s26();
    virtual int  running();                 /* slot 27 -> +0x6c */
};

// FUNCTION: EZ2AC_UNPACKED 0x00426700
int Judge::judgePress(int now, int h, int commit, SlotState *outSt, SlotSound *outSnd)
{
    JudgeLane *lane = &lanes[h];
    SlotSampleSink *sink;
    SlotState st1;
    SlotSound sn1;
    SlotState st2;
    SlotSound sn2;
    SlotState *st;
    SlotSound *sn;
    int r;
    int d1;
    int d2;

    if (((JClock *)f24e0)->running() == 0)
        return 0;
    if (((SlotSampleSink *)f24dc)->begin(h, now, 2, 1, &st1, &sn1) != 0)
    {
        sn2 = sn1;
        if (((SlotSampleSink *)f24dc)->next(1, &st2, &sn2) == 0)
        {
            if (st1.f7 == 0)
                goto USE1;
            return ((Obj426CA0 *)this)->m4266c0b(now, h);
        }
    }
    else
    {
        if (((SlotSampleSink *)f24dc)->begin(h, now, 1, 1, &st2, &sn2) == 0)
            return ((Obj426CA0 *)this)->m4266c0c(now, h);
        if (st2.f7 != 0)
            return ((Obj426CA0 *)this)->m4266c0(now, h);
        goto USE2;
    }
    if (st2.f7 != 0)
        return ((Obj426CA0 *)this)->m4266c0(now, h);
    d1 = now - st1.start;
    d2 = now - st2.start;
    if (d1 < 0)
        d1 = -d1;
    if (d2 < 0)
        d2 = -d2;
    if (d1 > d2)
        goto USE2;
    if (st1.f7 == 0)
        goto USE1;
USE2:
    sn = &sn2;
    st = &st2;
    goto GRADE;
USE1:
    sn = &sn1;
    st = &st1;
GRADE:
    r = grade(lane, now, st->start);
    if (r == 0)
        return ((Obj426CA0 *)this)->m4266c0(now, h);
    if (commit != 0)
    {
        st->f7 = (unsigned char)r;
        if (st->width <= 6 || r < *(int *)&_pad10[0])
            st->f8 = (unsigned char)r;
        sink = (SlotSampleSink *)f24dc;
        sink->setState(sn, st);
    }
    if (outSt != 0)
        *outSt = *st;
    if (outSnd != 0)
    {
        outSnd->f0 = sn->f0;
        outSnd->id = sn->id;
    }
    return r;
}

/* THE RELEASE VERDICT (event kind 1) - the press's twin, ~74% by masked diff (the same
   register/slot coin: `now` in ebp where the original has ebx, the two sound
   locals' slots swapped, and 6 hoisted into a register), semantically
   complete. Ask the sink for the hold hit (kind 2) of sound h at `now` and
   its successor - or, without a hold, the plain hit (kind 1); a candidate
   counts only when the query said 1 and it is a long hit (width > 6) whose
   head is graded (f7 >= 2) but whose tail is not (f8 == 0); a hold that does
   not qualify is advanced instead (slot 13). Grade the release against the
   hit's END (start + width - 1); a miss becomes 2. `result` is the memory
   word the original keeps in h's argument slot. */
/* SlotSampleSink with slot 13 named. */
struct JSink
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09(); virtual void q10();
    virtual int  begin(int h, int a, int b, int c, SlotState *st, SlotSound *s);
    virtual int  next(int a, SlotState *st, SlotSound *s);
    virtual int  advance(int a, SlotState *st, SlotSound *s);   /* slot 13 -> +0x34 */
    virtual void q14(); virtual void q15(); virtual void q16();
    virtual int  getState(SlotSound *s, SlotState *st);
    virtual void setState(SlotSound *s, SlotState *st);
};

// FUNCTION: EZ2AC_UNPACKED 0x004268c0
int Judge::judgeRelease(int now, int h, int commit, SlotState *outSt, SlotSound *outSnd)
{
    JudgeLane *lane = &lanes[h];
    SlotState stA;
    SlotSound snB;
    SlotState stB;
    SlotSound snA;
    SlotState *st;
    SlotSound *sn;
    volatile int result = 0;
    int ok1;
    int ok2;
    int r;

    if (((JClock *)f24e0)->running() == 0)
        return 0;
    ok1 = ((JSink *)f24dc)->begin(h, now, 2, 1, &stA, &snA);
    if (ok1 != 0)
    {
        snB = snA;
        ok2 = ((JSink *)f24dc)->next(1, &stB, &snB);
        if (!(stA.width > 6 && stA.f7 != 0 && stA.f7 != 1 && stA.f8 == 0))
            ok1 = ((JSink *)f24dc)->advance(1, &stA, &snA);
        if (ok1 == 0 && ok2 == 0)
            return 0;
    }
    else
    {
        ok2 = ((JSink *)f24dc)->begin(h, now, 1, 1, &stB, &snB);
        if (ok2 == 0)
            return 0;
    }
    if (ok1 == 1 && stA.width > 6 && stA.f7 != 0 && stA.f7 != 1 && stA.f8 == 0)
    {
        st = &stA;
        sn = &snA;
    }
    else if (ok2 == 1 && stB.width > 6 && stB.f7 != 0 && stB.f7 != 1 && stB.f8 == 0)
    {
        st = &stB;
        sn = &snB;
    }
    else
        return result;
    r = grade(lane, now, st->start + st->width - 1);
    if (r == 0)
    {
        result = 2;
        r = result;
    }
    if (commit != 0)
    {
        st->f8 = (unsigned char)r;
        ((JSink *)f24dc)->setState(sn, st);
    }
    if (outSt != 0)
        *outSt = *st;
    if (outSnd != 0)
    {
        outSnd->f0 = sn->f0;
        outSnd->id = sn->id;
    }
    return r;
}

/* THE JUDGE'S TICK - the auto-miss pass, ~90% (the loop counter's temporary
   is ebx where the original has eax, which rotates the vtable/lea scratch
   registers with it - the same coin as the other three judge functions).
   Nothing to do without a miss sink, a clock, or a running song. For every
   live record: resume its cursor into the sink (or begin from the first hit
   when the cursor is -1) and walk the hits: an unjudged hit (f4 == 1, f7 ==
   0) whose window has run out - start + (widest late window + 1) <= now, or
   immediately on a hold-only lane - is graded 1 (5 on a hold-only lane),
   the grade also written to f8 when the hit is short or the grade is 1 or
   2, stored back through setState and reported to the miss sink with flag
   1; a long hit whose head was hit but whose tail is unjudged is closed
   with 5 (flag 0) once its end has passed. The cursor advances with `next`;
   a hit still in the future ends that record's walk. `g_1b2ef1c > 0`
   selects the fifth late window over the second. */
/* Selects the fifth late window over the second for the auto-miss limit -
   read only here and by the catch judge @0x46d790. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef1c
int g_1b2ef1c;

struct JMissSink { virtual void onJudge(SlotSound *sn, int grade, int flag); };

/* 88.54%. Every instruction is present, the prologue matches (the aligned
   frame, all four callee-saved pushes, and `mov ebp,0x1` reusing the saved
   frame pointer as the constant 1), and the whole residue is one register
   decision: the ORIGINAL rematerializes zero per site and keeps it in eax -
   `xor eax,eax` for `i = 0` at the top and a second `xor eax,eax` for
   `limit = 0` inside the kind==1 arm - which leaves ebx free to be `g`, the
   value that has to survive the setState/onJudge calls. cl instead PINS zero
   in ebx for the whole function and pushes `g` elsewhere, so `limit` ends up
   in ecx and the `add` runs the other way (`add ecx,eax` against the
   original's `add eax,ecx`), and every later block inherits the rotation.

   This is the constant-register coin, checked the documented way first: the
   competitor register is identified (ebx is `g`, not a spill), the push
   counts are equal, so there is no cached parameter hiding in it - the lever
   that un-parked GFMainGameDirector::update2 does not apply. Four
   declaration permutations of `limit` and `g` (either order in the inner
   block, either one lifted to function scope) give byte-identical listings.
   Park; do not re-sweep declaration order. */
// FUNCTION: EZ2AC_UNPACKED 0x00426ab0
void Judge::tick()
{
    SlotSound sn;
    SlotState st;
    unsigned int now;
    int i;

    if (f24e4 == 0)
        return;
    if (f24e0 == 0)
        return;
    if (((JClock *)f24e0)->running() != 1)
        return;
    now = ((JClock *)f24e0)->now();
    for (i = 0; i < f2294; i++)
    {
        JudgeLane *rec = &lanes[i];
        int ok;
        if (rec->f04 == 0)
            continue;
        if (rec->curId == -1)
        {
            ok = ((SlotSampleSink *)f24dc)->begin(i, 0, 0, 1, &st, &sn);
        }
        else
        {
            sn = *(SlotSound *)&rec->curIndex;
            ok = ((SlotSampleSink *)f24dc)->next(1, &st, &sn);
        }
        if (ok == 0)
            continue;
        do
        {
            if (st.f4 == 1 && st.f7 == 0)
            {
                unsigned int limit;
                int g;
                if (rec->kind == 1)
                {
                    limit = 0;
                    g = 5;
                }
                else
                {
                    if (g_1b2ef1c > 0)
                        limit = rec->windows[1][5] + 1;
                    else
                        limit = rec->windows[1][2] + 1;
                    g = 1;
                }
                limit += st.start;
                if (limit > now)
                    break;
                if (g == 0)
                    break;
                st.f7 = (unsigned char)g;
                if (st.width <= 6 || g == 1 || g == 2)
                    st.f8 = (unsigned char)g;
                ((SlotSampleSink *)f24dc)->setState(&sn, &st);
                ((JMissSink *)f24e4)->onJudge(&sn, g, 1);
            }
            if (st.width > 6 && st.f4 == 1 && st.f8 == 0)
            {
                if ((unsigned int)(st.start + st.width - 1) > now)
                    break;
                st.f8 = 5;
                ((SlotSampleSink *)f24dc)->setState(&sn, &st);
                ((JMissSink *)f24e4)->onJudge(&sn, 5, 0);
            }
            rec->curIndex = sn.f0;
            rec->curId = sn.id;
        }
        while (((SlotSampleSink *)f24dc)->next(1, &st, &sn) != 0);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00426e50
void Judge::update2(int, int)
{
    tick();
}
