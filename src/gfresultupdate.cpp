/* GFResultDirector::update @0x4680d0 - the GF result screen's per-frame draw.

   A header value is written into row 6 of the tween table at +0xaa8 - which
   of the two values at +0x2330 / +0x2334 depends on the run flag and the
   player count - then the video member draws, four tween tables refresh, the
   per-player panels draw, and every node currently on list A refreshes its
   own table out of the run at +0x1c. After that a series of guarded
   refreshes, and finally the overlay blit when the phase is 0 or 7.

   NOTE the branch at 0x46810d: when g_1b2ea28 is not 1 the player count is
   still tested but BOTH arms load +0x2330, so the test decides nothing. That
   is in the original - two separate blocks, both `mov reg,[esi+0x2330]` -
   and it is reproduced rather than tidied away. */

#include "directors.h"
#include "ez2acapp.h"    /* KGraphics, g_sub1b2f054 */
#include "obj402860.h"
#include "bigbase.h"   /* Obj40C180, g_bigInstance */

extern int  g_1b2ea28;              /* src/moderunner.cpp */
struct EzSoundObject;
extern int  g_1b2e7d8;              /* src/globals.cpp */
extern int  g_modeIndex;            /* 0x1b2e548 (src/modename.cpp) */

extern int  countListC();                    /* 0x4184a0 (src/nodelists.cpp) */
extern int  nextListA(struct ListNode **cur);/* 0x4182e0 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004680d0
void GFResultDirector::update()
{
    ListNode *p;
    int n;

    if (g_1b2ea28 == 1)
    {
        /* The second test is against the COUNT just taken, not a literal 2 -
           the original's `cmp [g_1b2e7d8],eax` reuses it. */
        n = countListC();
        if (n == 2)
        {
            if (g_1b2e7d8 >= n)
                m_aa8.setCell(6, 0, f2334);
            else
                m_aa8.setCell(6, 0, f2330);
        }
        else
        {
            m_aa8.setCell(6, 0, f2334);
        }
    }
    else
    {
        if (countListC() == 2)
            m_aa8.setCell(6, 0, f2330);
        else
            m_aa8.setCell(6, 0, f2330);
    }

    m_2438.m40b710();
    m_cc4.m403940();
    m_aa8.m403940();
    m_ee0.m403940();
    drawPlayers();

    p = 0;
    if (nextListA(&p))
    {
        do
        {
            m_1c[p->f8].m403940();
        }
        while (nextListA(&p));
    }

    if (countListC() == 1)
    {
        if (g_1b2ea28 == 1)
        {
            if (g_modeIndex != 6 && g_modeIndex != 7 &&
                g_modeIndex != 8 && g_modeIndex != 9)
                m_295c[0].m403940();
        }
        else if (g_modeIndex != 6 && g_modeIndex != 7 &&
                 g_modeIndex != 8 && g_modeIndex != 9)
        {
            if (countListC() == 1)
                m4672f0();
        }
    }

    if (countListC() == 2)
    {
        if (f14 == 1)
            m_196c.m403940();
        if (f18 == 1)
            m_1b88.m403940();
    }

    if (f2910 >= 4 && g_modeIndex != 1)
        m_10fc.m403940();

    m_88c.m403940();

    if (countListC() != 2 && f10 == 1)
        m_1534.m403940();

    m_1da4.m403940();

    if (f2910 == 0 || f2910 == 7)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_2240, -5.0f, -5.0f, 645.0f, 485.0f,
                                             (f2914 << 24) | 0xffffff);
    }
}

/* GFResultDirector::update2 @0x467b90 - the GF result screen's tick, an
   eight-arm state machine on the phase at +0x2910 with case 6 empty.

   The counter at +0x2918 is bumped once per tick by the shared tail, which
   also publishes the fade level. Phase 0 ramps the fade up and hands over to
   1; 1 announces the score once the counter reaches the announce mark, then
   waits out the cabinet or a frame count; 2 and 3 wait likewise and 3 picks
   the winner; 4 and 5 wait on their own marks with the outro sound; 7 runs
   the outro and ramps the fade back down, and when it bottoms out it clears
   two globals and, once both sounds have stopped, hands 0 to g_1b2e844.

   Every wait has the same shape: either the cabinet reports ready, or the
   counter has passed a mark held in the class (+0x2db8..+0x2dc8).

   77.44%, up from 76.09%. Four arms were laid out the wrong way round - cl
   puts the `if` body inline and the else out of line, so an original that
   falls THROUGH into a block wrote that block as the `if`. Flipped: the
   bonus-battle pick (m_21e0 is the fall-through, not m_21e4), case 4's
   cabinet test (the 0x1e0 test falls through, the f2dbc setter is the
   out-of-line arm, so the `&&` becomes the De Morgan `||` with the arms
   swapped), and case 7's two sound picks (m_21ec falls through in both).

   WHAT IS LEFT, AND WHAT NOT TO TRY. Case 3's `winner != -1 || loser == -2`
   is laid out [test1][A][test2][B] with test2's `je` jumping BACKWARD into
   A - one copy of A, reached two ways. Writing the two arms out with A
   duplicated gets that mnemonic sequence, but cl CSEs the `m_21ec` load
   above both tests, the two copies are then not identical, the tail merger
   declines, and the function grows 22 bytes - which shifts EVERY branch
   displacement in the first two thirds and costs more than the block shape
   wins (reccmp 75.29% against the 76.09% it started at). A `goto` back into
   the block is normalized away by cl (71.5% by mnemonic), and `&&` with B
   first is worse still. Five forms; the `||` as written is the best of them.
   The residue after that is the eax/edx alternation through the sound calls
   and one `xor edi,edi` scheduled a call early in case 7.

   NOT a template for ResultDirector::update2 @0x451550, despite twins.py
   putting them at 86.1%. That one is 352 instructions to this one's 302 and
   the extra fifty are real: a prologue that forces the phase from the
   cabinet's input 0x26 and pokes g_1b2e844 under g_1b2eb44, a third sound
   object at +0x28fc, several g_1b2eb6c-gated arms, and its own helper
   @0x451180 in place of m467900 - which it ALSO calls, at 0x467900, so the
   two classes share at least one ICF-folded helper. It needs its own read;
   only the field offsets transfer (+0x2c40/+0x2c44/+0x2c48 for the phase,
   fade and counter here). */

/* The sound objects, viewed for the three slots this function drives. */
struct GFSoundSlots
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual void s4(int a);   /* slot 4  -> +0x10 */
    virtual void s5();        /* slot 5  -> +0x14 */
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09(); virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15();
    virtual int  busy();      /* slot 16 -> +0x40 */
    virtual void tick();      /* slot 17 -> +0x44 */
};

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct Obj1B2E844b { virtual void q00(int v); };

extern void *g_1b2e844;     /* src/moderunner.cpp */
extern int   g_1b2ea78;     /* src/systemmenu.cpp */
extern int   g_1b5f19c;     /* src/playerslot.cpp */
extern int   g_1b2e704;     /* src/playerslot.cpp */
extern int   g_37dd06c;     /* 0x037dd06c (src/globals.cpp) */
extern void  Log(const char *fmt, ...);   /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x00467b90
void GFResultDirector::update2(int /*a*/, int /*b*/)
{
    ((GFSoundSlots *)m_21dc)->tick();

    if (m_ee0.isDone())
        m_ee0.seek(0x5f);

    m4679e0();

    switch (f2910)
    {
    case 0:
        g_1b2ea78 += 1;
        f2914 += 10;
        if (f2914 > 0xfe)
        {
            f2914 = 0xff;
            f2910 = 1;
        }
        break;

    case 1:
        if (f2918 == f2db8)
        {
            if (countListC() == 1)
            {
                if (g_1b2ea28 != 1)
                {
                    ((GFSoundSlots *)m_21e0)->s4(0);
                    m467900();
                }
                else
                {
                    ((GFSoundSlots *)m_21e4)->s4(0);
                }
            }
            else
            {
                if (g_1b2e7d8 != 2)
                    ((GFSoundSlots *)m_21e0)->s4(0);
                else
                    ((GFSoundSlots *)m_21e4)->s4(0);
                m467900();
                // STRING: EZ2AC_UNPACKED 0x00495c74
                Log("Set Bonus Battle Play");
            }
        }

        if (countListC() == 1)
        {
            if (g_bigInstance->m419780() != 0 && f2918 >= 0x9e)
            {
                m467900();
                f2918 = f2dc0 - 1;
                m467900();
                f2910 = 7;
                break;
            }
            if (f2918 >= f2dc0 - 1)
            {
                f2918 = f2dbc - 1;
                f2910 = 7;
            }
            break;
        }
        if (countListC() == 2 && f2918 >= 0x9e)
        {
            if (f2918 < 0x258 && g_bigInstance->m419780() == 0)
                break;
            f2918 = 0;
            f2910 = 4;
        }
        break;

    case 2:
        m467900();
        if (g_bigInstance->m419780() == 0 && f2918 < f2dc0 - 1)
            break;
        m467900();
        f2918 = f2dc0 - 1;
        f2910 = 7;
        break;

    case 3:
        m4677c0();
        if (g_bigInstance->m419780() == 0 && f2918 < f2dc8)
            break;
        m467900();
        m467820();
        if (winner != -1 || loser == -2)
        {
            f2918 = 0;
            f2910 = 4;
            ((GFSoundSlots *)m_21ec)->s4(0);
            break;
        }
        f2910 = 7;
        ((GFSoundSlots *)m_21ec)->s4(0);
        break;

    case 4:
        if (m_88c.isDone())
            m_88c.seek(0x78);
        if (m_10fc.f8 == 0x19)
        {
            ((GFSoundSlots *)m_21fc)->s5();
            ((GFSoundSlots *)m_21fc)->s4(0);
        }
        if (g_bigInstance->m419780() == 0 || f2918 < 0x3c)
        {
            if (f2918 >= 0x1e0)
                f2910 = 7;
        }
        else
        {
            f2918 = f2dbc - 1;
            f2910 = 7;
        }
        break;

    case 5:
        m467720();
        if (m_88c.isDone())
            m_88c.seek(0x78);
        if (g_bigInstance->m419780() != 0)
        {
            m467900();
            f2910 = 7;
            ((GFSoundSlots *)m_21ec)->s4(0);
            break;
        }
        if (f2918 >= 0x30c)
        {
            f2910 = 7;
            ((GFSoundSlots *)m_21ec)->s4(0);
        }
        break;

    case 6:
        break;

    case 7:
        if (countListC() == 1)
        {
            if ((((GFSoundSlots *)m_21ec)->busy() & 1) == 0 &&
                (((GFSoundSlots *)m_21f0)->busy() & 1) == 0 &&
                f2928 == 0)
            {
                if (g_1b2ea28 == 0)
                    ((GFSoundSlots *)m_21ec)->s4(0);
                else
                    ((GFSoundSlots *)m_21f0)->s4(0);
                f2928 = 1;
            }
        }
        else if ((((GFSoundSlots *)m_21ec)->busy() & 1) == 0 &&
                 (((GFSoundSlots *)m_21f0)->busy() & 1) == 0 &&
                 f2928 == 0)
        {
            if (g_1b2e7d8 <= 1)
                ((GFSoundSlots *)m_21ec)->s4(0);
            else
                ((GFSoundSlots *)m_21f0)->s4(0);
            f2928 = 1;
        }

        ((GFSoundSlots *)m_21dc)->s5();
        f2914 += -10;
        if (f2914 < 1)
        {
            f2914 = 1;
            g_1b5f19c = 0;
            g_1b2e704 = 0;
            if ((((GFSoundSlots *)m_21f0)->busy() & 1) == 0 &&
                (((GFSoundSlots *)m_21ec)->busy() & 1) == 0)
                ((Obj1B2E844b *)g_1b2e844)->q00(0);
        }
        break;
    }

    f2918 += 1;
    g_37dd06c = f2914;
}
