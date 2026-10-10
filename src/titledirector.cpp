#include "bigbase.h"
#include "ezscreen.h"

/* The title screen's lamp animation, one frame per call.

   The cabinet's 32 output channels are grouped: 0xa..0xd is one bank, 0x10..0x15
   the six keys, 0x18..0x1d the six pedals/effects, and 4 is the coin lamp. The
   pattern walks a step counter and lights a different group at each step; the
   attract path (m_attract == 1) instead flashes everything off, then everything
   on, on a longer cycle.

   Every group is written out call by call rather than looped: the original has
   no induction variable and no shared block, so the sixteen calls of each frame
   are sixteen statements. setOutput @0x40c780 lives in src/obj40c180.cpp - a
   different TU, or cl would inline its one store into all ninety call sites. */
// FUNCTION: EZ2AC_UNPACKED 0x0044e050
void TitleDirector::tickLamps()
{
    if (m_strobe != 0)
    {
        g_bigInstance->setOutput(0x4, 1);
        m_strobe = 0;
        m_strobeLeft = 8;
    }

    if (m_strobeLeft != 0)
    {
        m_strobeLeft--;
        if (m_strobeLeft == 0)
        {
            m_step++;
            g_bigInstance->setOutput(0x4, 0);
        }
    }

    if (m_attract == 1)
    {
        m_step++;
        if (m_step >= 0)
        {
        g_bigInstance->setOutput(0xa, 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
        g_bigInstance->setOutput(0x10, 0);
        g_bigInstance->setOutput(0x11, 0);
        g_bigInstance->setOutput(0x12, 0);
        g_bigInstance->setOutput(0x13, 0);
        g_bigInstance->setOutput(0x14, 0);
        g_bigInstance->setOutput(0x15, 0);
        g_bigInstance->setOutput(0x18, 0);
        g_bigInstance->setOutput(0x19, 0);
        g_bigInstance->setOutput(0x1a, 0);
        g_bigInstance->setOutput(0x1b, 0);
        g_bigInstance->setOutput(0x1c, 0);
        g_bigInstance->setOutput(0x1d, 0);
        }
        if (m_step > 5)
        {
        g_bigInstance->setOutput(0xa, 1);
        g_bigInstance->setOutput(0xb, 1);
        g_bigInstance->setOutput(0xc, 1);
        g_bigInstance->setOutput(0xd, 1);
        g_bigInstance->setOutput(0x10, 1);
        g_bigInstance->setOutput(0x11, 1);
        g_bigInstance->setOutput(0x12, 1);
        g_bigInstance->setOutput(0x13, 1);
        g_bigInstance->setOutput(0x14, 1);
        g_bigInstance->setOutput(0x15, 1);
        g_bigInstance->setOutput(0x18, 1);
        g_bigInstance->setOutput(0x19, 1);
        g_bigInstance->setOutput(0x1a, 1);
        g_bigInstance->setOutput(0x1b, 1);
        g_bigInstance->setOutput(0x1c, 1);
        g_bigInstance->setOutput(0x1d, 1);
        }
        if (m_step > 10)
        {
            m_step = 0;
        }
        return;
    }

    if (m_restart == 1)
    {
        m_step = 0;
    }

    if (m_step >= 0)
    {
        g_bigInstance->setOutput(0xa, 1);
        g_bigInstance->setOutput(0xb, 1);
        g_bigInstance->setOutput(0xc, 1);
        g_bigInstance->setOutput(0xd, 1);
        g_bigInstance->setOutput(0x10, 0);
        g_bigInstance->setOutput(0x11, 0);
        g_bigInstance->setOutput(0x12, 0);
        g_bigInstance->setOutput(0x13, 0);
        g_bigInstance->setOutput(0x14, 0);
        g_bigInstance->setOutput(0x15, 0);
        g_bigInstance->setOutput(0x18, 0);
        g_bigInstance->setOutput(0x19, 0);
        g_bigInstance->setOutput(0x1a, 0);
        g_bigInstance->setOutput(0x1b, 0);
        g_bigInstance->setOutput(0x1c, 0);
        g_bigInstance->setOutput(0x1d, 0);
    }
    if (m_step >= 2)
    {
        g_bigInstance->setOutput(0xa, 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
        g_bigInstance->setOutput(0x10, 1);
        g_bigInstance->setOutput(0x11, 1);
        g_bigInstance->setOutput(0x12, 1);
        g_bigInstance->setOutput(0x13, 1);
        g_bigInstance->setOutput(0x14, 1);
        g_bigInstance->setOutput(0x15, 1);
        g_bigInstance->setOutput(0x18, 0);
        g_bigInstance->setOutput(0x19, 0);
        g_bigInstance->setOutput(0x1a, 0);
        g_bigInstance->setOutput(0x1b, 0);
        g_bigInstance->setOutput(0x1c, 0);
        g_bigInstance->setOutput(0x1d, 0);
    }
    if (m_step >= 4)
    {
        g_bigInstance->setOutput(0xa, 0);
        g_bigInstance->setOutput(0xb, 0);
        g_bigInstance->setOutput(0xc, 0);
        g_bigInstance->setOutput(0xd, 0);
        g_bigInstance->setOutput(0x10, 0);
        g_bigInstance->setOutput(0x11, 0);
        g_bigInstance->setOutput(0x12, 0);
        g_bigInstance->setOutput(0x13, 0);
        g_bigInstance->setOutput(0x14, 0);
        g_bigInstance->setOutput(0x15, 0);
        g_bigInstance->setOutput(0x18, 1);
        g_bigInstance->setOutput(0x19, 1);
        g_bigInstance->setOutput(0x1a, 1);
        g_bigInstance->setOutput(0x1b, 1);
        g_bigInstance->setOutput(0x1c, 1);
        g_bigInstance->setOutput(0x1d, 1);
    }
    if (m_step >= 6)
    {
        g_bigInstance->setOutput(0xa, 1);
        g_bigInstance->setOutput(0xb, 1);
        g_bigInstance->setOutput(0xc, 1);
        g_bigInstance->setOutput(0xd, 1);
        g_bigInstance->setOutput(0x10, 1);
        g_bigInstance->setOutput(0x11, 1);
        g_bigInstance->setOutput(0x12, 1);
        g_bigInstance->setOutput(0x13, 1);
        g_bigInstance->setOutput(0x14, 1);
        g_bigInstance->setOutput(0x15, 1);
        g_bigInstance->setOutput(0x18, 1);
        g_bigInstance->setOutput(0x19, 1);
        g_bigInstance->setOutput(0x1a, 1);
        g_bigInstance->setOutput(0x1b, 1);
        g_bigInstance->setOutput(0x1c, 1);
        g_bigInstance->setOutput(0x1d, 1);
    }
    if (m_step >= 8)
    {
        m_step = 0;
    }
}

/* The title screen's own copy of the button map - same fifteen entries as the
   mode-select screen's, in its own array, which is why the original's linker
   kept both. */
// GLOBAL: EZ2AC_UNPACKED 0x00494cc0
const int g_titleKeys[15] =
{
    10, 11, 12, 13, 14,
    18, 19, 20, 21, 22,
    6, 7, 8, 9,
    -1
};

/* The same latch-and-log walk the mode-select screen runs, over this screen's
   own fields. */
// FUNCTION: EZ2AC_UNPACKED 0x0044f460
void TitleDirector::clearCommand()
{
    int i;
    int key;

    m_110 = 1;

    for (i = 0; (key = g_titleKeys[i]) != -1; i++)
    {
        if (g_bigInstance->getState(key) == 2)
        {
            m44f300(0, key);
            m44f300(1, key);
            Log("Check Command = %d", key);
        }
    }

    cmdLen[0] = 0;
}

/* Reset the coin count. Only slot 0, and only while its gate at +0xe4 is
   clear: the credit counter in the I/O object goes to zero, the two objects
   at +0x14e0 and +0x14e4 are stopped through slots 5 and 12, the chart at
   +0xc6c is re-attached to the head at +0xc64, and three flags are cleared.
   `have` @0x420284 is the counter the .ini's CreditCoins is measured
   against, which is what makes the log line literal. */
// FUNCTION: EZ2AC_UNPACKED 0x0044e920
char TitleDirector::resetCoins(int slot)
{
    if (m_e4[slot] == 0 && slot == 0)
    {
        g_bigInstance->have = 0;
        m_14e0->s5();
        m_14e0->s12(0);
        m_14e4->s5();
        m_14e4->s12(0);
        m_c6c.attach(m_c64, 1);
        m_1630 = 0;
        m_restart = 0;
        m_1d0 = 0;
        // STRING: EZ2AC_UNPACKED 0x00494d2c
        Log("Reset Coin Count!!");
    }

    return 0;
}

/* The title's song base: the pump's update() on it fires the coin lamp. Its
   `this` is the base subobject at +0x14, which is why the store lands at
   +0x1604 of it. */
struct TitleSongView { char _pad0[0x1604]; int strobe; };   /* +0x14 -> 0x1618 */
// FUNCTION: EZ2AC_UNPACKED 0x0044deb0
void TitleSong::step()
{
    ((TitleSongView *)this)->strobe = 1;
}

/* The title's sink: hand m_1624 to the sound bank's slot 0. */
extern void *g_1b2e844;   /* src/moderunner.cpp - the sound bank */
struct TitleSoundBank { virtual void v0(int a); };
struct TitleSinkView { char _pad0[0x1614]; int mode; };     /* +0x10 -> 0x1624 */
extern KRuntimeClass classTitleDirector;        /* src/classinfo.cpp - "TitleDirector" */
extern EzObjectInfo g_info_4adfe8;              /* src/classinfo.cpp */
// FUNCTION: EZ2AC_UNPACKED 0x0044df40
void TitleSink::setMode(int, int)
{
    ((TitleSoundBank *)g_1b2e844)->v0(((TitleSinkView *)this)->mode);
}

// FUNCTION: EZ2AC_UNPACKED 0x0044dca0
KRuntimeClass *TitleDirector::getRuntimeClass()
{
    return &classTitleDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0044dcb0
DrawGroup *TitleDirector::getObjectInfo()
{
    return &g_info_4adfe8;
}

/* Cleared by the destructor along with m_118. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea94
int g_1b2ea94;

/* TitleDirector::~TitleDirector @0x44dcc0: stop the song base, wind the pump
   down (suspend, drop its players, stop), stop the three stoppables (slot 5
   then slot 3, the third one twice when m_14ec asks), clear the two flags and
   the two lamps; then the members go - the pump, the nine charts - and the
   song base. */
// FUNCTION: EZ2AC_UNPACKED 0x0044dcc0
TitleDirector::~TitleDirector()
{
    stop(0);
    m_pump.suspend();
    m_pump.removeAll();
    m_pump.stopAll();
    if (m_14e8 != 0)
        m_14e8->s5();
    if (m_14ec != 0)
        m_14e8->s5();
    if (m_14e8 != 0)
        m_14e8->s3();
    if (m_14ec != 0)
        m_14e8->s3();
    if (m_14e0 != 0)
        m_14e0->s5();
    if (m_14e0 != 0)
        m_14e0->s3();
    if (m_14e4 != 0)
        m_14e4->s5();
    if (m_14e4 != 0)
        m_14e4->s3();
    g_1b2ea94 = 0;
    m_118 = 0;
    g_bigInstance->setOutput(8, 0);
    g_bigInstance->setOutput(9, 0);
}

// FUNCTION: EZ2AC_UNPACKED 0x0044f2e0
// TitleDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TitleDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00494d14 EzObject
// VTABLE: EZ2AC_UNPACKED 0x00494d10 TitleSink
// VTABLE: EZ2AC_UNPACKED 0x00494d0c TitleSong
struct TitleDirector;

/* The two coin-inserted flags the input tick @0x418800 raises together: the
   title screen consumes the first to start its coin chart and the second to
   keep drawing it. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ec10
int g_coinInserted;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f11c
int g_coinShown;
/* The graphics singleton's two frame ops (src/com_wrappers.cpp). */
struct Obj40BB00
{
    void doOp7(int v);            /* 0x40a470 */
    void doOp10(unsigned int v);  /* 0x40a730 */
};

/* TitleDirector::update @0x44df60 - the per-frame draw: clear, the logo
   chart, the coin chart while it runs and is not done, two more charts, the
   coin chart started on a fresh coin (outside the hold), and then either the
   attract chart - which asks the sound bank for 1 when it finishes - or the
   idle chart plus the coin chart while the coin is still shown. */
// FUNCTION: EZ2AC_UNPACKED 0x0044df60
void TitleDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    m_1d4.m403940();
    if (m_1648 != 0)
    {
        Obj402860 *c = &m_3f0;
        if (c->isDone() == 0)
            c->m403940();
    }
    m_12c0.m403940();
    m_a48.m403940();
    if (m_1638 == 0 && g_coinInserted == 1)
    {
        m_60c.reset();
        g_coinInserted = 0;
        m_1648 = 1;
    }
    if (m_attract > 0)
    {
        Obj402860 *c = &m_82c;
        c->m403940();
        if (c->isDone() != 0)
            ((TitleSoundBank *)g_1b2e844)->v0(1);
    }
    else
    {
        m_c6c.m403940();
        if (m_1638 == 0 && g_coinShown == 1)
            m_60c.m403940();
    }
}

#include <string.h>   /* memcmp, memset */

extern int g_titleCodes[3][20];   /* 0x4ae018 (src/globals.cpp) */

/* TitleDirector::m44f300 @0x44f300 - append one key to a stream and see
   whether it has become a hidden command. ModeSelectDirector::m44c690
   @0x44c690's twin, over this screen's own buffers at +0x3c and its own code
   table, and it differs in three ways that are all about what a hit DOES.

   The mode-select copy reads a per-code "already fired" array; this one asks
   resetCoins @0x44e920 instead, so the code's effect and the guard against
   repeating it are the same call - it answers nonzero only when it actually
   did something.

   A hit here CLEARS BOTH STREAMS outright (40 dwords from +0x3c, which is
   the pair) and zeroes this stream's length, where the mode-select copy just
   plays a sound and carries on. And the table is walked BY POINTER against a
   one-past-the-end bound rather than indexed, which is why the code index
   and the entry offset are separate induction variables.

   Everything else is the same: append, prefix-compare each code for exactly
   as many entries as have been entered, keep the stream alive on any prefix
   match, and restart it from this key alone if none matched.

   88.00%, and every remaining defect is a frame SLOT number: the original
   packs `base` into a real local slot and `i` into the dead parameter home,
   cl does the reverse. Same instructions, six operands renamed. That is the
   documented stack-packer signature, and declaration order does not move it
   - swapped both ways, identical result. */
// FUNCTION: EZ2AC_UNPACKED 0x0044f300
void TitleDirector::m44f300(int slot, int key)
{
    int *p;
    int  keep;
    int  i;

    cmdBuf[slot][cmdLen[slot]] = key;
    cmdLen[slot]++;

    keep = 0;
    p = cmdBuf[slot];

    /* THE `jl` SAYS THIS IS AN INT LOOP, NOT A POINTER WALK. Written as
       `for (code = g_titleCodes[0]; code < g_titleCodes[3]; code += 20)`
       with its own `base` and `i` counters this was 88.00%: a pointer
       comparison is UNSIGNED, so cl rebased the walk to an offset and ended
       it `cmp eax,0xf0; jb`, where the original compares the table's real
       end address and ends `jl`. Counting with a signed int and spelling the
       uses `g_titleCodes[i]` lets cl re-derive all three induction variables
       the original has - the walking pointer, the +20 element base, and `i`
       itself for resetCoins - while keeping the signed test.

       98.67% after that, and the one remaining instruction is an EFFECTIVE
       match: the loop's bound is `g_titleCodes + 0xf0`, one past the end,
       and that address belongs to whatever the linker put next. In the
       original it is `classShowCredit`; in ours it is `g_bmpInfo`. Same
       instruction, same meaning, and reccmp scores it on the name. This is
       the documented "A LOOP BOUND IS AN ADDRESS" case with no fix available
       - the cookbook's remedy is to span the region with ONE array so the
       bound is an interior reference, and the bound here is already one past
       the end of a single array. Making the two sides agree would mean
       placing our classShowCredit at exactly g_titleCodes+0xf0, which cannot
       be arranged: g_titleCodes is zero-initialised and lives in .bss while
       a class descriptor is initialised and lives in .data. */
    for (i = 0; i < 3; i++)
    {
        if (memcmp(g_titleCodes[i], p, cmdLen[slot] * 4) == 0)
        {
            keep = 1;

            if (g_titleCodes[i][cmdLen[slot]] == -1 &&
                resetCoins(i) != 0)
            {
                memset(cmdBuf, 0, sizeof(cmdBuf));
                cmdLen[slot] = 0;
                return;
            }
        }
    }

    if (keep == 0)
    {
        cmdLen[slot] = 1;
        p[0] = key;
    }
}

extern int g_1b5f2e0;    /* 0x1b5f2e0 (src/holdtick.cpp) - the autoplay toggle */
extern int g_1b2f034;    /* 0x1b2f034 (src/globals.cpp) */
extern int g_totalCoin;  /* 0x16963dc (src/moderunner.cpp) */
extern int g_serviceCoin;/* 0x16963e8 (src/moderunner.cpp) */

/* The third of the input tick's hidden toggles (key 0x21); nothing else
   references it yet. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb24
int g_1b2eb24;

/* The per-frame coin/service tick @0x418760: drop last frame's coin lamp,
   flip the three hidden toggles on their keys' edge state, accept a coin
   (key 3, only with the second test latch up), raise the two inserted flags
   whenever the count moved, accept a service credit (key 2, first latch),
   and fold in whatever the mechanical counter ring buffered - lighting the
   coin lamp for a frame when it did. */
// FUNCTION: EZ2AC_UNPACKED 0x00418760
void CabinetInput::serviceCoins()
{
    if (f42029c != 0)
    {
        setOutput(0xe, 0);
        f42029c = 0;
    }
    f42028c += 1;

    if (getState(0x1b) == 2)
        g_1b5f2e0 = 1 - g_1b5f2e0;
    if (getState(0x21) == 2)
        g_1b2eb24 = 1 - g_1b2eb24;
    if (getState(0x23) == 2)
        g_1b2f034 = 1 - g_1b2f034;

    if (getState(3) == 2 && testMode2 != 0)
    {
        f420288 = have;
        have += 1;
        g_totalCoin += 1;
    }
    if (have != f420288)
    {
        g_coinShown = 1;
        g_coinInserted = 1;
        f420288 = have;
    }
    if (getState(2) == 2 && testMode1 != 0)
    {
        g_coinShown = 1;
        have += 1;
        g_serviceCoin += 1;
        g_coinInserted = 1;
    }

    int n = pending();
    if (n != 0)
    {
        have += n;
        g_totalCoin += n;
        setOutput(0xe, 1);
        f42029c = 1;
    }
}
