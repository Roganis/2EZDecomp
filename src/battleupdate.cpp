/* BattleMode::update2 @0x41c660 - the battle overlay's tick. While the
   service flag g_1b2ef78 is up it just clears charts 0 and 3 and arms the
   f2d latch after 60 ticks. Otherwise: once chart0 reports state 1 past
   beat 205 the player lamps blink on a 60-tick cycle (on for 40); the f2d
   latch holds them dark. Input 0 clears the chart and sets the latch;
   input 0x26 also skips the channel - logging the goal audience and
   bumping it into both 0x1b2e860/864 mirrors. Three timed voice cues fire
   off chart0's beat (205/265/325 normally; 25/85/145 under the CV2 flag
   unless Street Mix 1st), the second restarting the first sample, and the
   tail keeps both samples looping. */

#include <math.h>
#include <stdlib.h>
#include "panels.h"
#include "bigbase.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */
extern Obj40C180 *g_bigInstance;         /* 0x1b2e710 (src/accessors.cpp) */
extern int   g_demoPlay;                 /* 0x1b2ef78 (src/panels.cpp) */
extern int   g_1b2eb6c;                  /* src/ez2acapp.cpp */
extern int   g_mixStyle;                 /* 0x1b2ebc8 (src/panels.cpp) */
extern int   g_1b5f184;                  /* src/gamepanels.cpp */
extern int   g_1b2ea9c;                  /* src/globals.cpp */
extern int   g_1b2eb44;                  /* src/gamestate.cpp */
extern int   g_modeIndex;                /* 0x1b2e548 (src/modename.cpp) */
extern int   countListA();               /* 0x418310 (src/nodelists.cpp) */
extern float g_rateTarget;               /* 0x1b2ebd8 (src/panels.cpp) */
extern float g_rate1p;                   /* 0x1b2e860 (src/panels.cpp) */
extern float g_rate2p;                   /* 0x1b2e864 (src/panels.cpp) */

/* The slots update2 uses of the two samples. */
struct BattleSnd
{
    virtual void q0(); virtual void q1(); virtual void q2(); virtual void q3();
    virtual void play(int loop);     /* +0x10 */
    virtual void stop();             /* +0x14 */
    virtual void q6(); virtual void q7(); virtual void q8(); virtual void q9();
    virtual void q10(); virtual void q11();
    virtual void rewind(int a);      /* +0x30 */
    virtual void q13(); virtual void q14(); virtual void q15();
    virtual int  playing();          /* +0x40 - bit 0 while playing */
    virtual void restart();          /* +0x44 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0041c660
void BattleMode::update2(int, int)
{
    m_f3c += 1;
    if (g_demoPlay != 0)
    {
        chart0.clear();
        chart3.clear();
        if (m_f3c >= 0x3c)
            m_f2d = 1;
        return;
    }
    if (chart0.f210 == 1 && g_1b2eb6c == 0 && chart0.f8 >= 0xcd)
    {
        m_f38 += 1;
        if (m_f38 >= 0x3c)
            m_f38 = 0;
        if (m_f38 >= 0x28)
            setPlayerLamps(0);
        else
            setPlayerLamps(1);
    }
    if (m_f2d != 0)
        setPlayerLamps(0);
    if (g_bigInstance->getState(0) == 2)
    {
        chart0.clear();
        m_f2d = 1;
    }
    if (g_bigInstance->getState(0x26) == 2)
    {
        chart0.clear();
        m_f2d = 1;
        g_1b5f184 = 0;
        g_rate1p = g_rateTarget + 1.0f;
        g_rate2p = g_rateTarget + 1.0f;
        // STRING: EZ2AC_UNPACKED 0x0048e6a4
        Log("Goal Audience : %3.2f", g_rateTarget);
        // STRING: EZ2AC_UNPACKED 0x0048e694
        Log("CHANNEL SKIP");
    }
    if (g_1b2eb6c == 0)
    {
        if (m_ff30 == 0 && chart0.f8 >= 0xcd && g_demoPlay == 0)
        {
            ((BattleSnd *)m_a)->play(0);
            m_ff30 = 1;
        }
        if (m_ff31 == 0 && chart0.f8 >= 0x109 && g_demoPlay == 0)
        {
            ((BattleSnd *)m_a)->stop();
            ((BattleSnd *)m_a)->rewind(0);
            ((BattleSnd *)m_a)->play(0);
            m_ff31 = 1;
        }
        if (m_ff32 == 0 && chart0.f8 >= 0x145 && g_demoPlay == 0)
        {
            ((BattleSnd *)m_b)->play(0);
            m_ff32 = 1;
        }
    }
    else if (g_mixStyle != 7)
    {
        if (m_ff30 == 0 && chart0.f8 >= 0x19 && g_demoPlay == 0)
        {
            ((BattleSnd *)m_a)->play(0);
            m_ff30 = 1;
        }
        if (m_ff31 == 0 && chart0.f8 >= 0x55 && g_demoPlay == 0)
        {
            ((BattleSnd *)m_a)->stop();
            ((BattleSnd *)m_a)->rewind(0);
            ((BattleSnd *)m_a)->play(0);
            m_ff31 = 1;
        }
        if (m_ff32 == 0 && chart0.f8 >= 0x91 && g_demoPlay == 0)
        {
            ((BattleSnd *)m_b)->play(0);
            m_ff32 = 1;
        }
    }
    if (((BattleSnd *)m_a)->playing() & 1)
        ((BattleSnd *)m_a)->restart();
    if (((BattleSnd *)m_b)->playing() & 1)
        ((BattleSnd *)m_b)->restart();
}

/* BattleMode::m41c060 @0x41c060 - the battle meter needle. The target
   deflection is the square root of the score difference (signed), the
   step f28 is re-based whenever it moves, and each tick walks one
   degree back toward the target with a two-degree jitter alternating on
   the f2c phase, clamped to +/-135 degrees; the needle sprite draws
   rotated at (270, 238). */
// FUNCTION: EZ2AC_UNPACKED 0x0041c060
void BattleMode::m41c060()
{
    int   diff = m_f18 - m_f14;
    float r = (float)sqrt((double)abs(diff));
    float v, j, x;

    if (diff < 0)
        v = 0.0f - r;
    else
        v = r;
    if (v != m_f20)
    {
        m_f28 = v - m_f20;
        m_f20 = v;
        m_f24 = v;
    }
    if (m_f2c != 0)
    {
        j = 2.0f;
        m_f2c = 0;
    }
    else
    {
        j = -2.0f;
        m_f2c = 1;
    }
    if (m_f28 > 0.0f && m_f2c != 0)
    {
        m_f28 = m_f28 - 1.0f;
        x = m_f24 - m_f28;
    }
    else if (m_f28 < 0.0f && m_f2c == 0)
    {
        m_f28 = m_f28 + 1.0f;
        x = m_f24 - m_f28;
    }
    else
        x = m_f24;
    x += j;
    if (x > 135.0f)
        x = 135.0f;
    else if (x < -135.0f)
        x = -135.0f;
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->m404250((Sprite *)m_counter, 270.0f, 238.0f,
                                         0.0f, 0.0f, x, 0xffffffff);
}

/* BattleMode::m41bca0 @0x41bca0 - put the right winner banner into
   chart1's cell (3,0): 1P ahead, 2P ahead, or the tie, each latched on
   m_f1c so the cell is only rewritten when the lead changes (the tie
   arm rewrites every tick). */
// FUNCTION: EZ2AC_UNPACKED 0x0041bca0
void BattleMode::m41bca0()
{
    if (m_f14 > m_f18)
    {
        if (m_f1c == 1)
            return;
        chart1.setCell(3, 0, (int)m_win1p);
        m_f1c = 1;
        return;
    }
    if (m_f14 < m_f18)
    {
        if (m_f1c == 2)
            return;
        chart1.setCell(3, 0, (int)m_win2p);
        m_f1c = 2;
        return;
    }
    chart1.setCell(3, 0, (int)m_win3p);
    m_f1c = 0;
}

/* One lamp cycle: a key, its latched state, a cursor, and `count`
   16-byte entries each naming a lamp. */
struct BattleCycle
{
    int key;        /* 0x00 */
    int state;      /* 0x04 */
    int cur;        /* 0x08 */
    int count;      /* 0x0c */
    struct { int v; int pad[3]; } items[1];   /* 0x10, stride 0x10 */
};

/* Whatever m_f10 points at; only slot 0 is reached. */
struct BattleRaise
{
    virtual void q00(int a, int b);
};

extern void set_4184e0(int idx, int val);   /* 0x4184e0 (src/globals.cpp) */

/* BattleMode::m41ca30 @0x41ca30 - step one lamp cycle when its key
   goes to a fresh press: advance the cursor with wraparound, drop
   every lamp in the cycle, raise the one the cursor names, and tell
   the listener. */
// FUNCTION: EZ2AC_UNPACKED 0x0041ca30
void BattleMode::m41ca30(BattleCycle *p)
{
    int i, n;

    p->state = g_bigInstance->getState(p->key);
    if (p->state != 2)
        return;
    p->cur++;
    if (p->cur >= p->count)
        p->cur = 0;
    for (i = 0; i < p->count; i++)
        set_4184e0(p->items[i].v, 0);
    n = p->items[p->cur].v;
    if (n >= 0)
        set_4184e0(n, 1);
    if (m_f10 != 0)
        ((BattleRaise *)m_f10)->q00(n, 2);
}

/* BattleMode::update @0x41c8e0 - the battle overlay's draw tick.
   CV2Mix (style 7) just ticks charts 4 and 5 and latches f2d.
   Otherwise, unless g_1b2ea9c pauses everything: with a real two-up
   keys battle (not mode 1, not CV2, not held off by g_1b2eb44) the
   winner banner and the needle draw between charts 1 and 2, and until
   chart0 reports done it keeps playing, latching f2d when it ends.
   Demo play adds chart6 in tail position. */
// FUNCTION: EZ2AC_UNPACKED 0x0041c8e0
void BattleMode::update()
{
    if (g_mixStyle != 7)
    {
        if (g_1b2ea9c == 0)
        {
            if (g_1b2eb44 == 0 && countListA() > 1 && g_modeIndex != 1 &&
                g_1b2eb6c == 0)
            {
                m41bca0();
                chart1.m403940();
                m41c060();
                chart2.m403940();
            }
            if (m_f2d == 0)
            {
                chart0.m403940();
                if (chart0.isDone() != 0)
                    m_f2d = 1;
            }
        }
    }
    else
    {
        chart4.m403940();
        chart5.m403940();
        m_f2d = 1;
    }
    if (g_demoPlay != 0)
        chart6.m403940();
}
