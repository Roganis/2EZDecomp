/* TitleDirector::update2 @0x44f4d0 - the title screen's per-frame tick.

   Four globals cleared, the pump run, then: the lamp period winds down, the
   coin-hold timer and the fade ramp advance, the attract song is (re)started
   on whichever object the cabinet's state selects, the three service inputs
   are polled, and finally the two sound objects' pan is set from the lamp
   period before the lamp pattern itself is stepped.

   Three things a port needs out of it:

     - THE THREE EARLY EXITS. Inputs 1, 0x1c and 3 each hand a different value
       to g_1b2e844's slot 0 and return immediately: 2 for input 1 (the value
       getState just returned), a literal 2 for 0x1c, and for input 3 either 1
       or 0 depending on whether the attract loop owns the lamps. Input 3 also
       clears the eight-word block at 0x1b2eb04 and sets g_1b2efb8 first, so it
       is the "start a game" path where the other two are menu escapes.
     - THE TWENTY-CREDIT SWEEP. When coins/credits reaches ten AND the coin
       hold timer has passed 0x7d0, the cabinet's coin count is zeroed, both
       sound objects are stopped (slot 5) and muted (slot 12), the coin chart
       is re-attached and the timer reset. That is the coin-drop the credit
       readout mirrors.
     - THE PAN RAMP is `(m_30 - 0xff) * 10000 / 300`, the same expression the
       GF result screen uses for its fade level, handed to slot 8 of both
       objects. The original spells the /300 as a magic multiply; write the
       division and cl regenerates it.

   Written for the port. 1,120 bytes, 298 instructions, 32 calls. */

#include <string.h>
#include "ezscreen.h"
#include "bigbase.h"
#include "obj402860.h"

extern void Log(const char *fmt, ...);            /* 0x402880 */
extern unsigned int SatSub(unsigned int a, unsigned int b);  /* 0x4145d0 */
/* The thunk form, not windows.h's dllimport one - see src/titlector.cpp. */
extern "C" DWORD __stdcall timeGetTime();

extern void *g_1b2e844;
extern int  g_advSound;                           /* 0x16963f8 */
extern int  g_1b2ea78, g_1b2eba8, g_1b2ef94, g_1b2efb8;
extern int  g_selectedKey;                        /* 0x1b2efd4 */
extern int  g_1b2eb04[8];

/* g_1b2e844's slot 0, the only one anything here reaches. */
struct TitleNotify { virtual void q00(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x0044f4d0
void TitleDirector::update2(int now, int)
{

    g_1b2ea78 = 0;
    g_1b2eba8 = 0;
    g_1b2ef94 = 0;
    g_selectedKey = 0;

    m_pump.pump();

    m_34 += -5;
    if (m_34 <= 0)
        m_34 = 0;

    if (g_bigInstance->have > 0)
        m_restart += 1;

    if (m_104 <= 0x14)
    {
        m_104 += 1;
        m_30 = 0xff;
    }
    else if (m_104 >= 0x15)
    {
        m_104 = 0x15;
    }

    if (g_bigInstance->have < 1)
        m_108 = 0xff;

    /* Which object carries the attract song, and its coin lamp. */
    if (g_advSound == 1)
    {
        if (m_attract != 1)
        {
            m_14e0->s4(0);
            tickCoinLamp((TitleLampDev *)m_14e0, 0x1e0);
        }
    }
    else if (g_bigInstance->have != 0 && m_attract != 1)
    {
        if (m_1c8 != 0)
        {
            m_14e0->s4(0);
            tickCoinLamp((TitleLampDev *)m_14e0, 0x1e0);
        }
        else
        {
            m_14e4->s4(0);
            tickCoinLamp((TitleLampDev *)m_14e4, 0x1e0);
        }
    }

    /* The three service inputs, each its own early exit. */
    if (g_bigInstance->getState(1) == 2)
    {
        ((TitleNotify *)g_1b2e844)->q00(2);
        return;
    }
    if (g_bigInstance->getState(0x1c) == 2)
    {
        ((TitleNotify *)g_1b2e844)->q00(2);
        return;
    }
    if (g_bigInstance->getState(0) == 2)
    {
        /* A memset: the original clears the block with its own zero
           register before storing g_1b2efb8 from ebx. */
        memset(g_1b2eb04, 0, sizeof(g_1b2eb04));
        g_1b2efb8 = 0;
        if (m_attract == 1)
            ((TitleNotify *)g_1b2e844)->q00(1);
        else
            ((TitleNotify *)g_1b2e844)->q00(0);
        return;
    }

    m_f8 = (int)SatSub(now, m_1628);

    /* Nothing in the slot: after ~46 seconds the screen dims and hands
       itself back to the attract pattern. ONE condition - every miss of
       the three tests lands on the start-button poll (0x44f73f), so the
       buttons are polled with no coin in as well, until the 46 s are up. */
    if (g_bigInstance->have == 0 && SatSub(now, m_1628) >= 0xb3b0 &&
        g_bigInstance->need != 0)
    {
        m_12c = 1;
        m_30 += -2;
        if (m_30 <= 10)
        {
            m_30 = 0;
            m_163c = 1;
        }
        if (m_attract != 1)
        {
            if (m_1cc != 1)
                m_attract = -1;
            if (m44dec0() == 1 && g_bigInstance->have == 0)
                m_1624 = 3;
        }
    }
    else if (m_104 >= 0x14)
    {
        /* 1P and 2P start, each latched so it only fires once. */
        if (g_bigInstance->getState(4) == 2 && m_1660 == 0)
        {
            m44e680(0);
            if (m_1654 == 1)
                m_1660 = 1;
            m_1630 = 0;
        }
        if (g_bigInstance->getState(5) == 2 && m_1660 == 0)
        {
            m44e680(1);
            if (m_1654 == 1)
                m_1660 = 1;
            m_1630 = 0;
        }
    }

    /* Enough coins for a credit: swap the chart over, once. */
    if (g_bigInstance->have >= g_bigInstance->need && m_1d0 == 0)
    {
        m_162c = (int)timeGetTime();
        /* BOTH arms attach m_c68 (0x44f7ec and 0x44f7f5 each load
           [esi+0xc68]) - the game's own duplicated operand. The C had
           `need > 1 ? m_c68 : m_c64`, which the binary never does. */
        if (g_bigInstance->need > 1)
            m_c6c.attach(m_c68, 1);
        else
            m_c6c.attach(m_c68, 1);
        m_1d0 = 1;
    }

    if (g_bigInstance->need != 0)
    {
        if (g_bigInstance->have / g_bigInstance->need >= 10 && m_restart >= 0x7d0)
        {
            g_bigInstance->have = 0;
            m_14e0->s5();
            m_14e0->s12(0);
            m_14e4->s5();
            m_14e4->s12(0);
            m_c6c.attach(m_c64, 1);
            m_1630 = 0;
            m_restart = 0;
            /* The same literal as the Test-menu reset in titledirector.cpp
               (one STRING marker serves both). */
            Log("Reset Coin Count!!");
            m_1d0 = 0;
        }
        if (g_bigInstance->have >= 1)
            m_10c = 1;
    }

    if (m_10c == 1)
        clearCommand();

    /* Both objects' pan follows the lamp period. */
    m_14e0->s8((m_30 - 0xff) * 10000 / 300);
    m_14e4->s8((m_30 - 0xff) * 10000 / 300);

    tickLamps();
    updateTitleStartLamps();
}
