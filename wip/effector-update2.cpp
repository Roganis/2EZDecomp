/* Effector6thStyle::update2 @0x41e620 - IN-PROGRESS wip transcription.
   4.6KB, 0x41e620..0x41f85f, ret 8 (two args). 254 calls. Regenerate the
   listing with objdump 0x41e620..0x41fa00 + decode.py.

   THE MAP (by original label):
     head        - g_demoPlay==1 -> the demo half @0x41ecef. Gauge-over
                   sample plays (m_gaugeOver / m_gaugeOverMix by g_1b2eb6c,
                   the CV2-mix flag). Mode 10 chord toggles g_1b2e894 mod 2.
     41e6ec      - g_1b2eb6c!=0 -> the CV2 half @0x41f3d7.
     41e76e      - m_scrollBpm: modes 0-5/10/11 take the int divide
                   (g_speedPercent*g_songBpm/100), the rest the double
                   product with g_1b2eae0 and the two pool constants.
     41e792      - chart4.f8 thresholds latch g_1b5f158 (<=0x8c set,
                   >=0xaf clear).
     41e7b4      - mode dispatch: 5/9 -> 41ef0e, 3/7 -> 41e7d6 (start-held
                   + findListA join gate), else 41e81c.
     41e81c..    - the TT keys: getState(6/7)==2 -> set_4184e0(0x1b/0x1c),
                   sink cmd, chart3+chart4 reset; getState(8/9) with
                   heldFor -> sink(0xa/0x9, held); getState(4/5)==3 +
                   findListA -> chart3/chart4 seek; speed keys 0x10/0xf
                   (sink 0x23/0x22), 0xb/0xd (sink 0x25/0x24 with speed
                   index), 0x13/0x15 for the 2P side, 0x18/0x17.
     41ecef..    - the demo half: same key ladder against chart2(0x61c)
                   wait - actually chart0(0x1e4)+chart4 pairs first, then
                   chart2+chart4.
     41ef0e..    - modes 5/9 ladder (chart2 0x61c + chart4).
     41f0e2..    - start+0x11/0x19 chord handlers, seeks, speed keys.
     41f274..    - 0x11/0x19 == 2 -> set_4184e0(0x1b/0x1c, 1) + sink.
     41f3d7..    - the CV2 half: its own ladder (start gates, 0x11/0x19,
                   6/7/8/9 -> set_4184e0 0x1b/0x1c/0x26/0x27, heldFor
                   sink 0xa/0x9, start+findListA join gate at the tail).
     exits       - ret 8 at 0x41ecf4 (via 0x41ecee), 0x41ef0b, 0x41f7b7.

   The factory @0x41f860 and the draw-group dereg @0x41f8c0 that follow
   are separate, already-matched functions. */

#include "ezscreen.h"
#include "panels.h"

extern int g_demoPlay;      /* 0x1b2ef78 */
extern int g_1b2eb6c;       /* the CV2-mix flag */
extern int g_1b2eba4;
extern int g_1b2e898;
extern int g_1b2e894;
extern int g_speedPercent;  /* 0x1b2e890 */
extern int g_songBpm;       /* 0x1b2eac0 */
extern float g_1b2eae0;
extern int g_1b5f158;
extern int g_elemCount;     /* 0x1b2e718 */
extern int g_modeIndex;     /* 0x1b2e548 */

#if 0
/* The sample view (slot 4 = play). */
struct EffSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
};

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
        if (getState(4) == 3 || getState(5) == 3)
        {
            if (getState(0x11) == 2 || getState(0x19) == 2)
            {
                g_1b2e894 += 1;
                if (g_1b2e894 > 1)
                    g_1b2e894 = 0;
            }
        }
    }

    if (g_1b2eb6c != 0)
        goto cv2Half;

    /* The scroll rate: the classic modes divide by 100 in integers, the
       rest scale by g_1b2eae0 and the two pool constants in doubles
       (fild the int product, the named promotion, fistpll through the
       control-word save - exact spelling on integration). */
    if (g_modeIndex != 0 && g_modeIndex != 1 && g_modeIndex != 2 &&
        g_modeIndex != 3 && g_modeIndex != 4 && g_modeIndex != 5 &&
        g_modeIndex != 10 && g_modeIndex != 11)
    {
        double d = (double)(g_speedPercent * g_songBpm);
        m_scrollBpm = (int)(g_1b2eae0 * K48F1D8 * d * K48F1D0);
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
        if (getState(4) != 3 || findListA(0) == 0)
        {
            if (getState(5) != 3 || findListA(1) == 0)
                return;
        }
    }

    /* The TT keys. pick73() = chart3 in modes 7/3, chart0 otherwise;
       every handler resets it and chart4. */
    if (getState(6) == 2)
    {
        set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1b, 2);
        pick73()->reset();
        chart4.reset();
    }
    if (getState(7) == 2)
    {
        set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1c, 2);
        pick73()->reset();
        chart4.reset();
    }
    /* Keys 8/9: the edge, or held half a second; the state is passed
       through. No null check on the sink from here on. */
    {
        int st = getState(8);
        if (st == 2 || (st == 3 && heldFor(8) >= 500))
        {
            m_sink->cmd(0xa, st);
            pick73()->reset();
            chart4.reset();
        }
        st = getState(9);
        if (st == 2 || (st == 3 && heldFor(9) >= 500))
        {
            m_sink->cmd(9, st);
            pick73()->reset();
            chart4.reset();
        }
    }

    if (g_modeIndex == 4 || g_modeIndex == 8)
        goto mode48Ladder;                       /* 0x41ecf7 */

    /* P1's speed section, gated on start held and joined: the seek to
       60, then the dial at big+0x420238 or keys 0x10/0xf, and the two
       rate keys 0xb/0xd (0xb passes the state variable, 0xd the
       literal). The constants 60 and -10 live in ebx/ebp. */
    if (getState(4) == 3 && findListA(0) != 0)
    {
        if (chart3.f8 >= 60 || chart0.f8 >= 60)
        {
            pick73()->seek(60);
            chart4.seek(60);
        }
        if (g_bigInstance->f420238 < -10 || getState(0x10) == 2)
        {
            m_sink->cmd(0x23, 2);
            pick73()->reset();
            chart4.reset();
        }
        if (g_bigInstance->f420238 > 10 || getState(0xf) == 2)
        {
            m_sink->cmd(0x22, 2);
            pick73()->reset();
            chart4.reset();
        }
        {
            int st = getState(0xb);
            if (st == 2)
            {
                m_sink->cmd(0x25, st);
                pick73()->reset();
                chart4.reset();
            }
        }
        if (getState(0xd) == 2)
        {
            m_sink->cmd(0x24, 2);
            pick73()->reset();
            chart4.reset();
        }
    }

    /* P2, the same against big+0x42023c and keys 0x18/0x17/0x13/0x15 -
       with one extra unconditional chart4 seek, and the resets' second
       object addressed through the cached &chart4. */
    if (getState(5) == 3 && findListA(1) != 0)
    {
        if (chart3.f8 >= 60 || chart0.f8 >= 60)
        {
            pick73()->seek(60);
            chart4.seek(60);
        }
        chart4.seek(60);
        if (g_bigInstance->f42023c < -10 || getState(0x18) == 2)
        {
            m_sink->cmd(0x23, 2);
            pick73()->reset();
            chart4.reset();
        }
        if (g_bigInstance->f42023c > 10 || getState(0x17) == 2)
        {
            m_sink->cmd(0x22, 2);
            pick73()->reset();
            chart4.reset();
        }
        {
            int st = getState(0x13);
            if (st == 2)
            {
                m_sink->cmd(0x25, st);
                pick73()->reset();
                chart4.reset();
            }
        }
        if (getState(0x15) == 2)
        {
            m_sink->cmd(0x24, 2);
            pick73()->reset();
            chart4.reset();
        }
    }
    return;

    /* Modes 4/8: one shared ladder over chart0+chart4, both dials and
       both sides' keys, gated only on either start being held. The
       constants live in registers: 60 in ebx, -10 in ebp, 10 in edi. */
mode48Ladder:                                    /* 0x41ecf7 */
    if (getState(4) != 3 && getState(5) != 3)
        return;
    if (chart0.f8 >= 60)
    {
        chart0.seek(60);
        chart4.seek(60);
    }
    if (g_bigInstance->f420238 < -10 || getState(0x10) == 2)
    {
        m_sink->cmd(0x23, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->f420238 > 10 || getState(0xf) == 2)
    {
        m_sink->cmd(0x22, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->f42023c < -10 || getState(0x18) == 2)
    {
        m_sink->cmd(0x23, 2);
        chart0.reset();
        chart4.reset();
    }
    if (g_bigInstance->f42023c > 10 || getState(0x17) == 2)
    {
        m_sink->cmd(0x22, 2);
        chart0.reset();
        chart4.reset();
    }
    {
        int st = getState(0xb);
        if (st == 2)
        {
            m_sink->cmd(0x25, st);
            chart0.reset();
            chart4.reset();
        }
    }
    if (getState(0xd) == 2)
    {
        m_sink->cmd(0x24, 2);
        chart0.reset();
        chart4.reset();
    }
    {
        int st = getState(0x13);
        if (st == 2)
        {
            m_sink->cmd(0x25, st);
            chart0.reset();
            chart4.reset();
        }
    }
    if (getState(0x15) == 2)
    {
        m_sink->cmd(0x24, 2);
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
        if (g_bigInstance->f420238 < -10 || getState(0x10) == 2)
        {
            m_sink->cmd(0x23, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->f420238 > 10 || getState(0xf) == 2)
        {
            m_sink->cmd(0x22, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->f42023c < -10 || getState(0x18) == 2)
        {
            m_sink->cmd(0x23, 2);
            chart2.reset();
            chart4.reset();
        }
        if (g_bigInstance->f42023c > 10 || getState(0x17) == 2)
        {
            m_sink->cmd(0x22, 2);
            chart2.reset();
            chart4.reset();
        }
        {
            int st = getState(0xb);
            if (st == 2)
            {
                m_sink->cmd(0x25, st);
                chart2.reset();
                chart4.reset();
            }
        }
        if (getState(0xd) == 2)
        {
            m_sink->cmd(0x24, 2);
            chart2.reset();
            chart4.reset();
        }
        {
            int st = getState(0x13);
            if (st == 2)
            {
                m_sink->cmd(0x25, st);
                chart2.reset();
                chart4.reset();
            }
        }
        if (getState(0x15) == 2)
        {
            m_sink->cmd(0x24, 2);
            chart2.reset();
            chart4.reset();
        }
    }
    if (getState(4) != 3 && getState(5) != 3)
    {
        /* No start: 0x11/0x19 are plain effector taps. */
        {
            int st = getState(0x11);
            if (st == 2)
            {
                m_sink->cmd(0xa, st);   /* 0xa is the 10 in edi */
                chart2.reset();
                chart4.reset();
            }
        }
        if (getState(0x19) == 2)
        {
            m_sink->cmd(9, 2);
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
    if (g_bigInstance->f420238 < -10 || getState(0x10) == 2)
    {
        m_sink->cmd(0x23, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->f420238 > 10 || getState(0xf) == 2)
    {
        m_sink->cmd(0x22, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->f42023c < -10 || getState(0x18) == 2)
    {
        m_sink->cmd(0x23, 2);
        chart2.reset();
        chart4.reset();
    }
    if (g_bigInstance->f42023c > 10 || getState(0x17) == 2)
    {
        m_sink->cmd(0x22, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = getState(0x11);
        if (st == 2)
        {
            set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
            if (m_sink != 0)
                m_sink->cmd(0x1b, 2);
            chart2.reset();
            chart4.reset();
        }
    }
    if (getState(0x19) == 2)
    {
        set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1c, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = getState(0xb);
        if (st == 2)
        {
            m_sink->cmd(0x25, st);
            chart2.reset();
            chart4.reset();
        }
    }
    if (getState(0xd) == 2)
    {
        m_sink->cmd(0x24, 2);
        chart2.reset();
        chart4.reset();
    }
    {
        int st = getState(0x13);
        if (st == 2)
        {
            m_sink->cmd(0x25, st);
            chart2.reset();
            chart4.reset();
        }
    }
    if (getState(0x15) == 2)
    {
        m_sink->cmd(0x24, 2);
        chart2.reset();
        chart4.reset();
    }
    return;

    /* The CV2 half, dispatched on the CV2 state g_1b2ebc8. States 4/5
       are channel-key-only pages (their sink calls sit in TAIL
       position - the arg-slot-overwrite jmp); 2/6 gate the key ladder
       on a joined start; 0/1/3/8/9 run it ungated; 7 runs its own copy
       whose keys 8/9 also write set_4184e0(0x26/0x27). */
cv2Half:                                         /* 0x41f3d7 */
    if (g_1b2ebc8 == 4)
    {
        if (getState(4) != 3 && getState(5) != 3)
        {
            int st = getState(0x11);
            if (st == 2)
                m_sink->cmd(0xa, st);
            if (getState(0x19) != 2)
                return;
            m_sink->cmd(9, 2);           /* tail */
            return;
        }
        if (getState(0x11) == 2)
        {
            set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
            if (m_sink != 0)
                m_sink->cmd(0x1b, 2);
        }
        if (getState(0x19) != 2)
            return;
        set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
        if (m_sink == 0)
            return;
        m_sink->cmd(0x1c, 2);            /* tail */
        return;
    }
    if (g_1b2ebc8 == 5)
    {
        if (getState(4) == 3)
        {
            if (getState(0x11) == 2)
            {
                set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
                if (m_sink != 0)
                    m_sink->cmd(0x1b, 2);
            }
            if (getState(0x19) != 2)
                return;
            set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
            if (m_sink == 0)
                return;
            m_sink->cmd(0x1c, 2);        /* tail */
            return;
        }
        if (getState(5) != 3)
            return;
        {
            int st = getState(0x11);
            if (st == 2)
                m_sink->cmd(0xa, st);
        }
        if (getState(0x19) != 2)
            return;
        m_sink->cmd(9, 2);               /* tail */
        return;
    }
    if (g_1b2ebc8 == 2 || g_1b2ebc8 == 6)
    {
        /* Gate on a joined side holding start, then the shared key
           ladder below. */
        if (getState(4) == 3 && findListA(0) != 0)
            goto cv2Keys;
        if (getState(5) == 3 && findListA(1) != 0)
            goto cv2Keys;
        return;
    }
    if (g_1b2ebc8 == 0 || g_1b2ebc8 == 1 || g_1b2ebc8 == 3 ||
        g_1b2ebc8 == 8 || g_1b2ebc8 == 9)
        goto cv2Keys;
    if (g_1b2ebc8 != 7)
        return;

    /* State 7's own copy - keys 8/9 latch set_4184e0(0x26/0x27) from
       g_1b2e9d8/g_1b2e9e0 and the last sink call is the tail. */
    if (getState(6) == 2)
    {
        set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1b, 2);
    }
    if (getState(7) == 2)
    {
        set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1c, 2);
    }
    if (getState(8) == 2)
    {
        set_4184e0(0x26, g_1b2e9d8 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x26, 2);
    }
    if (getState(9) != 2)
        return;
    set_4184e0(0x27, g_1b2e9e0 == 0 ? 1 : 0);
    if (m_sink == 0)
        return;
    m_sink->cmd(0x27, 2);                /* tail */
    return;

cv2Keys:                                         /* 0x41f800 */
    if (getState(6) == 2)
    {
        set_4184e0(0x1b, g_1b2e980 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1b, 2);
    }
    if (getState(7) == 2)
    {
        set_4184e0(0x1c, g_1b2e988 == 0 ? 1 : 0);
        if (m_sink != 0)
            m_sink->cmd(0x1c, 2);
    }
    /* Keys 8/9 here have no set_4184e0 - the edge or half a second
       held, passing the state through. */
    {
        int st = getState(8);
        if (st == 2 || (st == 3 && heldFor(8) >= 500))
            m_sink->cmd(0xa, st);
        st = getState(9);
        if (st == 2 || (st == 3 && heldFor(9) >= 500))
            m_sink->cmd(9, st);          /* the 41f7a7 exit */
    }
}
#endif
