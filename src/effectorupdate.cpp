/* Effector6thStyle::update @0x41d920 - the effector panel's per-frame draw:
   pick the chart this mode uses, stamp the four or five readouts into it, and
   draw it. 921 bytes, 272 instructions, no float.

   Everything hangs off two selectors. g_1b2eb6c splits the whole body: clear,
   it is the MODE path keyed on g_modeIndex; set, it is the STYLE path keyed on
   g_mixStyle. Each picks one of the class's nine charts and stamps the same
   shape of readout into it - a texture chosen by a boost flag being up or down.

   THE MODE PATH re-tests g_modeIndex TWICE, once to choose the chart it draws
   and again to choose the chart it stamps. Both tests are written out because
   the original has both: modes 5 and 9 go to a separate block entirely, 7 and 3
   take chart3, everything else chart0. It also stamps the speed as four digits
   out of m_speedFont, each a separate divide - the ones, then /10, then /100,
   then /1000, every one of them modulo 10.

   THE STYLE PATH's four arms take chart2 (style 4), chart5 (style 5), chart1
   (anything else) and chart8 (style 7), and only style 7's stamps all four
   boosts - bass, treble, bg and play - which is what makes it the long one.

   The tail is shared by both and gated three ways: the audience panel's chart7
   is drawn unless the run says otherwise, and chart6 only in demo play. The
   `isDone` on chart7 publishing to g_1b2e714 is the one write this function
   makes that anything else reads. */

#include "ezscreen.h"
#include "gamepanels.h"
#include "obj402860.h"

extern int countListA();                          /* 0x418310 (src/nodelists.cpp) */

extern int g_1b2ea9c;
extern int g_1b2eb6c;                             /* src/ez2acapp.cpp */
extern int g_modeIndex;                           /* 0x1b2e548 (src/modename.cpp) */
extern int g_mixStyle;                            /* 0x1b2ebc8 (src/panels.cpp) */
extern int g_speedPercent;                        /* 0x1b2e890 (src/panels.cpp) */
extern int g_1b2e914;                             /* src/panels.cpp */
extern int g_bassBoostOn;                         /* 0x1b2e980 (src/gamestate.cpp) */
extern int g_trebleBoostOn;                       /* 0x1b2e988 (src/gamestate.cpp) */
extern int g_bgBoostOn;                           /* 0x1b2e9d8 (src/gamestate.cpp) */
extern int g_playBoostOn;                         /* 0x1b2e9e0 (src/gamestate.cpp) */
extern int g_useAudiencePanel;                    /* 0x1b2eb48 */
extern int g_1b2eba4;
extern int g_elemCount;                           /* 0x1b2e718 (src/loops.cpp) */
extern int g_demoPlay;                            /* 0x1b2ef78 */
extern int g_1b2ec14;
extern int g_1b5f2ac;                             /* src/globals.cpp */
extern int g_1b2e714;                             /* src/globals.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0041d920
void Effector6thStyle::update()
{
    Obj402860 *c;

    if (g_1b2ea9c == 0)
    {
        if (g_1b2eb6c == 0)
        {
            updateSpeedDigits();

            /* The chart this mode DRAWS. */
            if (g_modeIndex != 5 && g_modeIndex != 9)
            {
                if (g_modeIndex == 7 || g_modeIndex == 3)
                    c = &chart3;
                else
                    c = &chart0;
                c->m403940();

                /* ...and the chart it STAMPS, chosen by the same test again.
                   Writing the bass stamp INSIDE each arm - which the one `lea`
                   per branch suggests - is WORSE (274 instructions against the
                   original's 272, and 51% against 54%): cl declines the
                   cross-jump and emits both stamps in full. */
                if (g_modeIndex == 7 || g_modeIndex == 3)
                    c = &chart3;
                else
                    c = &chart0;

                if (g_bassBoostOn != 0)
                    c->setCell(4, 0, (int)m_effectOn);
                else
                    c->setCell(4, 0, (int)m_effectOff);
                if (g_trebleBoostOn != 0)
                    c->setCell(5, 0, (int)m_effectOn);
                else
                    c->setCell(5, 0, (int)m_effectOff);

                c->setCell(6, 0, (int)m_speedFont[g_speedPercent % 10]);
                c->setCell(7, 0, (int)m_speedFont[g_speedPercent / 10 % 10]);
                c->setCell(8, 0, (int)m_speedFont[g_speedPercent / 100 % 10]);
            }
            else
            {
                c = &chart2;
                if (g_bassBoostOn != 0)
                    c->setCell(4, 0, (int)m_effectOn);
                else
                    c->setCell(4, 0, (int)m_effectOff);
                if (g_trebleBoostOn != 0)
                    c->setCell(5, 0, (int)m_effectOn);
                else
                    c->setCell(5, 0, (int)m_effectOff);
                c->m403940();
            }
        }
        else if (g_mixStyle == 7)
        {
            c = &chart8;
            if (g_bassBoostOn != 0)
                c->setCell(5, 0, (int)m_effect[1]);
            else
                c->setCell(5, 0, (int)m_effect[0]);
            if (g_trebleBoostOn != 0)
                c->setCell(6, 0, (int)m_effect[3]);
            else
                c->setCell(6, 0, (int)m_effect[2]);
            if (g_bgBoostOn != 0)
                c->setCell(7, 0, (int)m_effect[5]);
            else
                c->setCell(7, 0, (int)m_effect[4]);
            if (g_playBoostOn != 0)
                c->setCell(8, 0, (int)m_effect[7]);
            else
                c->setCell(8, 0, (int)m_effect[6]);
            c->m403940();
        }
        else if (g_mixStyle == 4)
        {
            c = &chart2;
            if (g_bassBoostOn != 0)
                c->setCell(3, 0, (int)m_bassOn);
            else
                c->setCell(3, 0, (int)m_bassOff);
            if (g_trebleBoostOn != 0)
                c->setCell(5, 0, (int)m_trebleOn);
            else
                c->setCell(5, 0, (int)m_trebleOff);
            c->m403940();
        }
        else
        {
            if (g_mixStyle == 5)
                c = &chart5;
            else
                c = &chart1;
            if (g_bassBoostOn != 0)
                c->setCell(3, 0, (int)m_bassOn);
            else
                c->setCell(3, 0, (int)m_bassOff);
            if (g_trebleBoostOn != 0)
                c->setCell(5, 0, (int)m_trebleOn);
            else
                c->setCell(5, 0, (int)m_trebleOff);
            c->setCell(7, 0, (int)m_speed[g_1b2e914]);
            c->m403940();
        }
    }

    /* The shared tail. */
    if (g_1b2eb6c == 0)
    {
        if (countListA() == 2 || g_modeIndex == 1)
        {
            if (g_1b5f2ac <= 0 && g_modeIndex == 1 && g_1b2eba4 == 1)
                chart7.m403940();
        }
        else if (g_useAudiencePanel == 0 && g_1b2eba4 == 1)
        {
            /* No mode test here: this arm only runs with the mode NOT 1,
               and the C's `g_modeIndex == 1` made it dead code until
               2026-10-03 (docs/PORT-DELTAS.md). */
            chart7.m403940();
        }
    }
    else if (g_1b2eba4 == 1 && g_elemCount != 2)
    {
        chart7.m403940();
    }

    if (chart7.isDone() != 0)
        g_1b2e714 = 1;

    if (g_demoPlay != 0 && g_1b2ec14 == 1)
        chart6.m403940();
}

/* Effector::update2 @0x41ced0 - the effector tick. Demo play slides the
   overlay: above 25 it falls 10 a tick, at rest the dwell timer runs
   8000 of the caller's time units before the fall resumes, and past
   -375 it snaps back to 525 once (the fd08 latch). A live panel
   instead walks the five records, polling each readout by its mode -
   0 through the lamp-cycle stepper (the body ICF-shared with
   BattleMode's, reached through the cast), 1 through the transition
   poll. */
// FUNCTION: EZ2AC_UNPACKED 0x0041ced0
void Effector::update2(int a, int dt)
{
    int i;

    if (g_demoPlay == 1)
    {
        if (fd10 > 25.0f)
            fd10 = fd10 - 10.0f;
        else
        {
            fd14 += dt;
            if (fd14 > 0x1f40)
                fd10 = fd10 - 10.0f;
        }
        if (-375.0f > fd10 && fd08 != 1)
        {
            fd14 = 0;
            fd08 = 1;
            fd10 = 525.0f;
        }
        return;
    }
    if (f14 != 0)
    {
        for (i = 0; i < f18; i++)
        {
            /* Anchored at the record, as the original keeps it. */
            Effector::EffectorRec *r = &recs[i];

            if (r->mode == 0)
                ((BattleMode *)this)->m41ca30((struct BattleCycle *)r->readout);
            else if (r->mode == 1)
                m41cac0((struct EffectorReadout *)r->readout);
        }
    }
}
