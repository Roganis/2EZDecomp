/* Effector6thStyle::update2 @0x41e620 - the effector input tick. Decoded
   in wip/effector-update2.cpp (the map and the section notes live in its
   git history); written for the port, iterated with callcmp.

   4,671 bytes: the gauge-over sting, the mode-10 chord toggle, the
   scroll-rate computation, the low-speed latch, and per-mode key ladders
   driving the sink at +0x10 (slot 0) with chart resets and seeks - the
   normal half over pick73/chart4, modes 4/8 over chart0, the radio modes
   over chart2, and the CV2 half dispatched on g_mixStyle. */

#include "ezscreen.h"
#include "bigbase.h"
#include "panels.h"

extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 */
extern int   countListA();          /* 0x418310 */
extern struct ListNode *findListA(int key);  /* 0x418e30 */
extern void  set_4184e0(int idx, int val);   /* 0x4184e0 (src/globals.cpp) */

extern int   g_demoPlay;       /* 0x1b2ef78 */
extern int   g_1b2eb6c;        /* the CV2-mix flag */
extern int   g_1b2eba4;
extern int   g_1b2e898;
extern int   g_1b2e894;
extern int   g_speedPercent;   /* 0x1b2e890 */
extern int   g_songBpm;        /* 0x1b2eac0 */
extern float g_1b2eae0;
extern int   g_1b5f158;
extern int   g_elemCount;      /* 0x1b2e718 */
extern int   g_modeIndex;      /* 0x1b2e548 */
extern int   g_1b2efa8;        /* the radio free-play flag */
extern int   g_mixStyle;       /* 0x1b2ebc8 */
extern int   g_bassBoostOn;    /* 0x1b2e980 */
extern int   g_trebleBoostOn;  /* 0x1b2e988 */
extern int   g_bgBoostOn;      /* 0x1b2e9d8 */
extern int   g_playBoostOn;    /* 0x1b2e9e0 */

/* The command receiver at +0x10: slot 0 takes (command, state). */
struct EffSink
{
    virtual void cmd(int c, int st);
};

/* The gauge-over samples' view: slot 4 is play. */
struct EffSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
};

/* The chart the normal half's handlers reset beside chart4: chart3 in
   modes 7/3, chart0 otherwise - the original re-reads the mode and picks
   at every site, which is this helper inlined. */
static __forceinline Obj402860 *pick73(Effector6thStyle *t)
{
    if (g_modeIndex == 7 || g_modeIndex == 3)
        return &t->chart3;
    return &t->chart0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041e620
void Effector6thStyle::update2(int a, int b)
{
    if (g_demoPlay == 1)
        return;

    /* The gauge-over sting - the CV2 mix keeps its own sample and only
       lets it go on the first two songs. */
    if (g_1b2eb6c == 0)
    {
        if (g_1b2eba4 == 1 && g_1b2e898 == 1)
            ((EffSnd *)m_gaugeOver)->play(0);
    }
    else if (g_1b2eba4 == 1 && g_elemCount <= 1 && g_1b2e898 == 1)
    {
        ((EffSnd *)m_gaugeOverMix)->play(0);
    }

    /* Mode 10: a start-held effector chord steps g_1b2e894 mod 2. */
    if (g_modeIndex == 10)
    {
        if (g_bigInstance->getState(4) == 3 || g_bigInstance->getState(5) == 3)
        {
            if (g_bigInstance->getState(0x11) == 2 || g_bigInstance->getState(0x19) == 2)
            {
                g_1b2e894 += 1;
                if (g_1b2e894 > 1)
                    g_1b2e894 = 0;
            }
        }
    }

    if (g_1b2eb6c == 0)
    {

    /* The scroll rate: the classic modes divide by 100 in integers, the
       rest scale by g_1b2eae0 and the two pool constants in doubles
       (fild the int product, the named promotion, fistpll through the
       control-word save - exact spelling on integration). */
    if (g_modeIndex != 0 && g_modeIndex != 1 && g_modeIndex != 2 &&
        g_modeIndex != 3 && g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 10 && g_modeIndex != 11)
    {
        double d = (double)(g_speedPercent * g_songBpm);
        m_scrollBpm = (int)(__int64)(((g_1b2eae0 * 0.625) * d) * 0.01);
    }
    else
    {
        m_scrollBpm = g_speedPercent * g_songBpm / 100;
    }

    /* chart4's row latches the low-speed flag. */
    if (chart4.f8 <= 0x8c)
        g_1b5f158 = 1;
    else if (chart4.f8 >= 0xaf)
        g_1b5f158 = 0;

    if (g_modeIndex == 5 || g_modeIndex == 9)
        goto radioLadder;                        /* 0x41ef0e */
    if (g_modeIndex == 3 || g_modeIndex == 7)
    {
        /* Proceed only when a joined side holds start. */
        if (g_bigInstance->getState(4) != 3 || findListA(0) == 0)
        {
            if (g_bigInstance->getState(5) != 3 || findListA(1) == 0)
                return;
        }
    }

    /* The TT keys. pick73() = chart3 in modes 7/3, chart0 otherwise;
       every handler resets it and chart4. */
    if (g_bigInstance->getState(6) == 2)
    {
        if (g_bassBoostOn != 0)
            set_4184e0(0x1b, 0);
        else
            set_4184e0(0x1b, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1b, 2);
        pick73(this)->reset();
        chart4.reset();
    }
    if (g_bigInstance->getState(7) == 2)
    {
        if (g_trebleBoostOn != 0)
            set_4184e0(0x1c, 0);
        else
            set_4184e0(0x1c, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1c, 2);
        pick73(this)->reset();
        chart4.reset();
    }
    /* Keys 8/9: the edge, or held half a second; the state is passed
       through. No null check on the sink from here on. */
    {
        int st = g_bigInstance->getState(8);
        if (st == 2 || (st == 3 && g_bigInstance->heldFor(8) >= 500))
        {
            ((EffSink *)m_sink)->cmd(0xa, st);
            pick73(this)->reset();
            chart4.reset();
        }
        st = g_bigInstance->getState(9);
        if (st == 2 || (st == 3 && g_bigInstance->heldFor(9) >= 500))
        {
            ((EffSink *)m_sink)->cmd(9, st);
            pick73(this)->reset();
            chart4.reset();
        }
    }

    if (g_modeIndex == 4 || g_modeIndex == 8)
        goto mode48Ladder;                       /* 0x41ecf7 */

    /* P1's speed section, gated on start held and joined: the seek to
       60, then the dial at big+0x420238 or keys 0x10/0xf, and the two
       rate keys 0xb/0xd (0xb passes the state variable, 0xd the
       literal). The constants 60 and -10 live in ebx/ebp. */
    if (g_bigInstance->getState(4) == 3 && findListA(0) != 0)
    {
        if (chart3.f8 >= 60 || chart0.f8 >= 60)
        {
            pick73(this)->seek(60);
            chart4.seek(60);
        }
        if (g_bigInstance->accum[0] < -10 || g_bigInstance->getState(0x10) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x23, 2);
            pick73(this)->reset();
            chart4.reset();
        }
        if (g_bigInstance->accum[0] > 10 || g_bigInstance->getState(0xf) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x22, 2);
            pick73(this)->reset();
            chart4.reset();
        }
        {
            int st = g_bigInstance->getState(0xb);
            if (st == 2)
            {
                ((EffSink *)m_sink)->cmd(0x25, st);
                pick73(this)->reset();
                chart4.reset();
            }
        }
        if (g_bigInstance->getState(0xd) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x24, 2);
            pick73(this)->reset();
            chart4.reset();
        }
    }

    /* P2, the same against big+0x42023c and keys 0x18/0x17/0x13/0x15 -
       with one extra unconditional chart4 seek, and the resets' second
       object addressed through the cached &chart4. */
    if (g_bigInstance->getState(5) == 3 && findListA(1) != 0)
    {
        if (chart3.f8 >= 60 || chart0.f8 >= 60)
        {
            pick73(this)->seek(60);
            chart4.seek(60);
        }
        chart4.seek(60);
        if (g_bigInstance->accum[1] < -10 || g_bigInstance->getState(0x18) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x23, 2);
            pick73(this)->reset();
            chart4.reset();
        }
        if (g_bigInstance->accum[1] > 10 || g_bigInstance->getState(0x17) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x22, 2);
            pick73(this)->reset();
            chart4.reset();
        }
        {
            int st = g_bigInstance->getState(0x13);
            if (st == 2)
            {
                ((EffSink *)m_sink)->cmd(0x25, st);
                pick73(this)->reset();
                chart4.reset();
            }
        }
        if (g_bigInstance->getState(0x15) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x24, 2);
            pick73(this)->reset();
            chart4.reset();
        }
    }
    return;

    /* Modes 4/8: one shared ladder over chart0+chart4, both dials and
       both sides' keys, gated only on either start being held. The
       constants live in registers: 60 in ebx, -10 in ebp, 10 in edi. */
mode48Ladder:                                    /* 0x41ecf7 */
    if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
        return;
    if (chart0.f8 >= 60)
    {
        chart0.seek(60);
        chart4.seek(60);
    }
    if (g_bigInstance->accum[0] < -10 || g_bigInstance->getState(0x10) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x23, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[0] > 10 || g_bigInstance->getState(0xf) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x22, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[1] < -10 || g_bigInstance->getState(0x18) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x23, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[1] > 10 || g_bigInstance->getState(0x17) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x22, 2);
        chart0.reset();
        chart4.reset();
    }
    {
        int st = g_bigInstance->getState(0xb);
        if (st == 2)
        {
            ((EffSink *)m_sink)->cmd(0x25, st);
            chart0.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(0xd) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x24, 2);
        chart0.reset();
        chart4.reset();
    }
    {
        int st = g_bigInstance->getState(0x13);
        if (st == 2)
        {
            ((EffSink *)m_sink)->cmd(0x25, st);
            chart0.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(0x15) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x24, 2);
        chart0.reset();
        chart4.reset();
    }
    return;

    /* Modes 5/9, over chart2+chart4. Free-play (g_1b2efa8 == 1) gets
       the ungated dial/key ladder first; then the chord section: with
       a start held it seeks and runs the gated ladder plus the channel
       keys 0x11/0x19 (set 0x1b/0x1c by polarity, null-checked sink);
       without, 0x11/0x19 alone fire sink 0xa/9. */
radioLadder:                                     /* 0x41ef0e */
    if (g_1b2efa8 == 1)
    {
        if (g_bigInstance->accum[0] < -10 || g_bigInstance->getState(0x10) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x23, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->accum[0] > 10 || g_bigInstance->getState(0xf) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x22, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->accum[1] < -10 || g_bigInstance->getState(0x18) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x23, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->accum[1] > 10 || g_bigInstance->getState(0x17) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x22, 2);
            chart2.reset();
            chart4.reset();
        }
        {
            int st = g_bigInstance->getState(0xb);
            if (st == 2)
            {
                ((EffSink *)m_sink)->cmd(0x25, st);
                chart2.reset();
                chart4.reset();
            }
        }
        if (g_bigInstance->getState(0xd) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x24, 2);
            chart2.reset();
            chart4.reset();
        }
        {
            int st = g_bigInstance->getState(0x13);
            if (st == 2)
            {
                ((EffSink *)m_sink)->cmd(0x25, st);
                chart2.reset();
                chart4.reset();
            }
        }
        if (g_bigInstance->getState(0x15) == 2)
        {
            ((EffSink *)m_sink)->cmd(0x24, 2);
            chart2.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
    {
        /* No start: 0x11/0x19 are plain effector taps. */
        {
            int st = g_bigInstance->getState(0x11);
            if (st == 2)
            {
                ((EffSink *)m_sink)->cmd(0xa, st);   /* 0xa is the 10 in edi */
                chart2.reset();
                chart4.reset();
            }
        }
        if (g_bigInstance->getState(0x19) == 2)
        {
            ((EffSink *)m_sink)->cmd(9, 2);
            chart2.reset();
            chart4.reset();
        }
        return;
    }
    if (chart2.f8 >= 60)
    {
        chart2.seek(60);
        chart4.seek(60);
    }
    if (g_bigInstance->accum[0] < -10 || g_bigInstance->getState(0x10) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x23, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[0] > 10 || g_bigInstance->getState(0xf) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x22, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[1] < -10 || g_bigInstance->getState(0x18) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x23, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->accum[1] > 10 || g_bigInstance->getState(0x17) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x22, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = g_bigInstance->getState(0x11);
        if (st == 2)
        {
            if (g_bassBoostOn != 0)
                set_4184e0(0x1b, 0);
            else
                set_4184e0(0x1b, 1);
            if (m_sink != 0)
                ((EffSink *)m_sink)->cmd(0x1b, 2);
            chart2.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(0x19) == 2)
    {
        if (g_trebleBoostOn != 0)
            set_4184e0(0x1c, 0);
        else
            set_4184e0(0x1c, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1c, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = g_bigInstance->getState(0xb);
        if (st == 2)
        {
            ((EffSink *)m_sink)->cmd(0x25, st);
            chart2.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(0xd) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x24, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = g_bigInstance->getState(0x13);
        if (st == 2)
        {
            ((EffSink *)m_sink)->cmd(0x25, st);
            chart2.reset();
            chart4.reset();
        }
    }
    if (g_bigInstance->getState(0x15) == 2)
    {
        ((EffSink *)m_sink)->cmd(0x24, 2);
        chart2.reset();
        chart4.reset();
    }
    return;

    }

    /* The CV2 half, dispatched on the CV2 state g_mixStyle. States 4/5
       are channel-key-only pages (their sink calls sit in TAIL
       position - the arg-slot-overwrite jmp); 2/6 gate the key ladder
       on a joined start; 0/1/3/8/9 run it ungated; 7 runs its own copy
       whose keys 8/9 also write set_4184e0(0x26/0x27). */
    if (g_mixStyle == 4)
    {
        if (g_bigInstance->getState(4) != 3 && g_bigInstance->getState(5) != 3)
        {
            int st = g_bigInstance->getState(0x11);
            if (st == 2)
                ((EffSink *)m_sink)->cmd(0xa, st);
            if (g_bigInstance->getState(0x19) != 2)
                return;
            ((EffSink *)m_sink)->cmd(9, 2);           /* tail */
            return;
        }
        if (g_bigInstance->getState(0x11) == 2)
        {
            if (g_bassBoostOn != 0)
                set_4184e0(0x1b, 0);
            else
                set_4184e0(0x1b, 1);
            if (m_sink != 0)
                ((EffSink *)m_sink)->cmd(0x1b, 2);
        }
        if (g_bigInstance->getState(0x19) != 2)
            return;
        if (g_trebleBoostOn != 0)
            set_4184e0(0x1c, 0);
        else
            set_4184e0(0x1c, 1);
        if (m_sink == 0)
            return;
        ((EffSink *)m_sink)->cmd(0x1c, 2);            /* tail */
        return;
    }
    if (g_mixStyle == 5)
    {
        if (g_bigInstance->getState(4) == 3)
        {
            if (g_bigInstance->getState(0x11) == 2)
            {
                if (g_bassBoostOn != 0)
                    set_4184e0(0x1b, 0);
                else
                    set_4184e0(0x1b, 1);
                if (m_sink != 0)
                    ((EffSink *)m_sink)->cmd(0x1b, 2);
            }
            if (g_bigInstance->getState(0x19) != 2)
                return;
            if (g_trebleBoostOn != 0)
                set_4184e0(0x1c, 0);
            else
                set_4184e0(0x1c, 1);
            if (m_sink == 0)
                return;
            ((EffSink *)m_sink)->cmd(0x1c, 2);        /* tail */
            return;
        }
        if (g_bigInstance->getState(5) != 3)
            return;
        {
            int st = g_bigInstance->getState(0x11);
            if (st == 2)
                ((EffSink *)m_sink)->cmd(0xa, st);
        }
        if (g_bigInstance->getState(0x19) != 2)
            return;
        ((EffSink *)m_sink)->cmd(9, 2);               /* tail */
        return;
    }
    if (g_mixStyle == 2 || g_mixStyle == 6)
    {
        /* Gate on a joined side holding start, then the shared key
           ladder below. */
        if (g_bigInstance->getState(4) == 3 && findListA(0) != 0)
            goto cv2Keys;
        if (g_bigInstance->getState(5) == 3 && findListA(1) != 0)
            goto cv2Keys;
        return;
    }
    if (g_mixStyle == 0 || g_mixStyle == 1 || g_mixStyle == 3 ||
        g_mixStyle == 8 || g_mixStyle == 9)
        goto cv2Keys;
    if (g_mixStyle != 7)
        return;

    /* State 7's own copy - keys 8/9 latch set_4184e0(0x26/0x27) from
       g_bgBoostOn/g_playBoostOn and the last sink call is the tail. */
    if (g_bigInstance->getState(6) == 2)
    {
        if (g_bassBoostOn != 0)
            set_4184e0(0x1b, 0);
        else
            set_4184e0(0x1b, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1b, 2);
    }
    if (g_bigInstance->getState(7) == 2)
    {
        if (g_trebleBoostOn != 0)
            set_4184e0(0x1c, 0);
        else
            set_4184e0(0x1c, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1c, 2);
    }
    if (g_bigInstance->getState(8) == 2)
    {
        if (g_bgBoostOn != 0)
            set_4184e0(0x26, 0);
        else
            set_4184e0(0x26, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x26, 2);
    }
    if (g_bigInstance->getState(9) != 2)
        return;
    if (g_playBoostOn != 0)
        set_4184e0(0x27, 0);
    else
        set_4184e0(0x27, 1);
    if (m_sink == 0)
        return;
    ((EffSink *)m_sink)->cmd(0x27, 2);                /* tail */
    return;

    /* The shared key ladder. The original lays it SPLIT - the key-7
       action and the 8/9 tail ahead of the 2/6 gate, the 6/7 tests
       after, jumping backward - but cl normalizes every source order
       to this natural one, so the static sequence differs from the
       original by that rotation alone (multiset exact). */
cv2Keys:                                         /* 0x41f800 */
    if (g_bigInstance->getState(6) == 2)
    {
        if (g_bassBoostOn != 0)
            set_4184e0(0x1b, 0);
        else
            set_4184e0(0x1b, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1b, 2);
    }
    if (g_bigInstance->getState(7) == 2)
    {
        if (g_trebleBoostOn != 0)
            set_4184e0(0x1c, 0);
        else
            set_4184e0(0x1c, 1);
        if (m_sink != 0)
            ((EffSink *)m_sink)->cmd(0x1c, 2);
    }
    /* Keys 8/9 here have no set_4184e0 - the edge or half a second
       held, passing the state through. */
    {
        int st = g_bigInstance->getState(8);
        if (st == 2 || (st == 3 && g_bigInstance->heldFor(8) >= 500))
            ((EffSink *)m_sink)->cmd(0xa, st);
        st = g_bigInstance->getState(9);
        if (st == 2 || (st == 3 && g_bigInstance->heldFor(9) >= 500))
            ((EffSink *)m_sink)->cmd(9, st);          /* the 41f7a7 exit */
    }
}
