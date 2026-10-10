/* RankingDirector::update @0x45a290 - the ranking screen's per-frame draw, and
   a seven-arm state machine on the director's own mode at +0x10.

   The shape that repeats, and why it is written out rather than factored:

   * the FADE READOUT. Three arms step a level at +0x109b30 by 9 a tick,
     clamp it up to 0xff, splat it into all three colour channels with a
     shift/or chain and draw the two digits of +0x109ddc out of the ten
     glyphs at +0x1e2c. The colour is rebuilt between the two draws - the
     original reloads the level - so the chain is written twice per arm.
   * the TWO-PLAYER SPLIT. `countListA() == 2` puts the helper at x=63 and
     x=383, one player puts it at x=400.
   * the LAMP TEST. Identical in three arms: one player refreshes the first
     of the pair at +0x884, two players pick which counters to test from
     the flag at +0x85df8 and refresh either lamp whose counter is under 99.
     Each copy opens with a `countListA()` whose result is discarded, which
     is in the original and is reproduced by writing the bare call.

   The digits are drawn ones-first (the higher x) then tens, which is the
   opposite of ResultDirector::drawStageNumber @0x450ef0 but the same
   `m_digits[v / 10 % 10]` idiom.

   88.16%, and the whole residue is ONE scheduling coin with a long tail.
   Five of the six readout arms schedule the tens digit's divide AFTER the
   four coordinate pushes, so cl's tail merger folds them into two shared
   blocks - and the original does the same for five of them. The sixth,
   mode 1/2 with the mode flag set, schedules its divide BEFORE the pushes
   in the original, which breaks the common suffix and gives that arm its
   own copy of the divide, the draw, the doOp7 and the epilogue. cl puts
   the divide after the pushes there too, the arm merges, and the block is
   82 bytes shorter - which shifts every jump-table entry from case 4 on
   and reads as a much larger mismatch than it is (cases 0..3 start at the
   original's exact offsets).

   Nothing steers it. Byte-identical output from: the colour written as
   the argument expression instead of through two locals, the digit index
   hoisted into a named local in every arm, and the same hoist in the tens
   arms only. The one thing that DID move was the two-field copy into
   m_152c, which needs float locals in an inner block to come out as the
   original's mov/movss pair rather than two integer movs. */

#include "directors.h"
#include "ez2acapp.h"    /* KGraphics, g_sub1b2f054 */
#include "obj402860.h"

/* The graphics singleton under the names its matched COM wrappers use
   (src/com_wrappers.cpp); only the slot shape matters for the call. */
struct Obj40BB00
{
    void doOp7(int v);              /* 0x40a470 */
    void doOp10(unsigned int c);    /* 0x40a730 */
};

extern int countListA();            /* 0x418310 (src/nodelists.cpp) */

extern int g_1b2f024;               /* src/moderunner.cpp */
extern int g_elemCount;             /* 0x1b2e718 (src/loops.cpp) */
extern int g_1b2eb6c;               /* src/ez2acapp.cpp */
extern int g_1b2eb44;               /* src/gamestate.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0045a290
void RankingDirector::update()
{
    Obj402860 *p;
    int a, col;

    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);

    m_14.setCell(8, 0, f1ec0);
    m_44c.setCell(8, 0, f1ec0);

    if (f10 != 5)
    {
        m_1748.m40b710();
        if (g_1b2f024 == 1)
            m_10f4.m403940();
        else
            m_668.m403940();
    }

    if (countListA() > 0)
    {
        switch (f10)
        {
        case 0:
            m_14.m403940();
            break;

        case 1:
        case 2:
            if (g_1b2f024 != 0)
            {
                if (m_14.isDone() == 0)
                    break;

                m459bc0();
                m_44c.m403940();

                f109b30 += 9;
                if (f109b30 >= 0xfe)
                    f109b30 = 0xff;

                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

                a = f109b30;
                col = a;
                col = (col << 8) | a;
                col = (col << 8) | a;
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                     196.0f, 330.0f, 32.0f, 32.0f, col);
                a = f109b30;
                col = a;
                col = (col << 8) | a;
                col = (col << 8) | a;
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                     186.0f, 330.0f, 32.0f, 32.0f, col);
                ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
                return;
            }

            if (countListA() == 1)
            {
                m_44c.m403940();
                m459bc0();
            }

            if (countListA() == 2)
            {
                m459870(63.0f, f109b58, 0);
                m459870(383.0f, f109b5c, 1);
            }
            else if (countListA() == 1)
            {
                m459870(400.0f, f109b58, 0);
            }

            countListA();
            if (countListA() == 1)
            {
                m_884[0].m403940();
            }
            else if (countListA() == 2)
            {
                if (f85df8 != 0)
                {
                    if (f85dfc < 99)
                        m_884[0].m403940();
                    if (f85e00 < 99)
                        m_884[1].m403940();
                }
                else
                {
                    if (f109b48 < 99)
                        m_884[0].m403940();
                    if (f109b4c < 99)
                        m_884[1].m403940();
                }
            }

            f109b30 += 9;
            if (f109b30 >= 0xfe)
                f109b30 = 0xff;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

            a = f109b30;
            col = a;
            col = (col << 8) | a;
            col = (col << 8) | a;
            ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                 333.0f, 330.0f, 32.0f, 32.0f, col);
            a = f109b30;
            col = a;
            col = (col << 8) | a;
            col = (col << 8) | a;
            ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                 322.0f, 330.0f, 32.0f, 32.0f, col);
            ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
            return;

        case 3:
            if (g_1b2f024 != 0)
            {
                m459bc0();
                m_44c.m403940();
                m_152c.m403940();

                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                     196.0f, 330.0f, 32.0f, 32.0f, 0xffffff);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                     186.0f, 330.0f, 32.0f, 32.0f, 0xffffff);
                ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
                return;
            }

            m_44c.m403940();

            if (countListA() == 2)
            {
                m459870(63.0f, f109b58, 0);
                m459870(383.0f, f109b5c, 1);
            }
            else if (countListA() == 1)
            {
                m459870(400.0f, f109b58, 0);
            }

            m459bc0();
            /* The original addresses both stores off a live pointer to the
               member, not off `this` - so the member's address is held in a
               local across the refresh call. */
            p = &m_152c;
            p->m403940();
            {
                float x = f109b34;
                float y = f109b38;
                p->f214 = x;
                p->f218 = y;
            }

            countListA();
            if (countListA() == 1)
            {
                m_884[0].m403940();
            }
            else if (countListA() == 2)
            {
                if (f85df8 != 0)
                {
                    if (f85dfc < 99)
                        m_884[0].m403940();
                    if (f85e00 < 99)
                        m_884[1].m403940();
                }
                else
                {
                    if (f109b48 < 99)
                        m_884[0].m403940();
                    if (f109b4c < 99)
                        m_884[1].m403940();
                }
            }

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                 333.0f, 330.0f, 32.0f, 32.0f, -1);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                 322.0f, 330.0f, 32.0f, 32.0f, -1);
            ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
            return;

        case 4:
            if (g_1b2f024 != 0)
            {
                m_44c.m403940();

                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                     196.0f, 330.0f, 32.0f, 32.0f, 0xffffff);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                     186.0f, 330.0f, 32.0f, 32.0f, 0xffffff);
            }
            else
            {
                m_44c.m403940();

                if (countListA() == 2)
                {
                    m459870(63.0f, f109b58, 0);
                    m459870(383.0f, f109b5c, 1);
                }
                else if (countListA() == 1)
                {
                    m459870(400.0f, f109b58, 0);
                }

                countListA();
                if (countListA() == 1)
                {
                    m_884[0].m403940();
                }
                else if (countListA() == 2)
                {
                    if (f85df8 != 0)
                    {
                        if (f85dfc < 99)
                            m_884[0].m403940();
                        if (f85e00 < 99)
                            m_884[1].m403940();
                    }
                    else
                    {
                        if (f109b48 < 99)
                            m_884[0].m403940();
                        if (f109b4c < 99)
                            m_884[1].m403940();
                    }
                }

                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc % 10],
                                                     333.0f, 330.0f, 32.0f, 32.0f, -1);
                ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[f109ddc / 10 % 10],
                                                     322.0f, 330.0f, 32.0f, 32.0f, -1);
            }

            ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_109da0, -5.0f, -5.0f, 645.0f, 485.0f,
                                                 (f109da4 << 24) | 0xffffff);
            ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
            return;

        case 5:
            ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_109da0, 0.0f, 0.0f, 640.0f, 480.0f, -1);

            if (g_elemCount < 2 && g_1b2eb6c == 0)
            {
                if (m_109b68.isDone())
                    m_109b68.seek(m_109b68.f8 - 1);
                m_109b68.m403940();
            }
            break;

        case 6:
            if (g_elemCount < 2 && g_1b2eb6c == 0)
            {
                if (m_109b68.isDone())
                    m_109b68.seek(m_109b68.f8 - 1);
                m_109b68.m403940();
            }
            else if (g_1b2eb44 == 1)
            {
                if (m_109b68.isDone())
                    m_109b68.seek(m_109b68.f8 - 1);
                m_109b68.m403940();
            }
            break;
        }
    }

    ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
}
