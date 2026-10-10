/* SongSelectDirector::m444ae0 @0x444ae0 - the difficulty-column stepper for
   the keys modes, run every tick while the select page is in state 5 (the
   difficulty pick after a song is chosen; update2 calls it with a=1).

   The cursor is the GLOBAL g_4abd14 (0..4; 0 is the leftmost column, the
   strip draw eases toward (g_4abd14 - 1) * 46). Every move publishes the
   previous position to g_37dced8 and steps through a per-column availability
   ladder: a column whose level float is zero is skipped toward column 1.
   The level floats are read at row-relative +0x104/+0x114/+0x120/+0x12c of
   rows[f84] - note the FIRST one is 4 below SongSelectRow::diff[0].level
   while the other three are diff[1..3].level exactly, so either diff really
   starts at +0x104 here or column 0's flag is the name's tail dword; left
   as raw offsets until a third reader decides it.

   Inputs, per side: down = key 0xb / wheel accum < -10 / key 0x10 (P2:
   0x13 / accum[1] / 0x18), up = 0xd / accum > 10 / 0xf (P2: 0x15 / accum[1]
   / 0x17). Modes 4/5 run one combined block over both sides' keys instead
   (and its up-gate reads only accum[1] - preserved as found). Key 6 held
   skips all input. Key 4 (P1) / key 5 (P2) backs out to state 4 when
   f_bfd48 is set; the white keys a/c/e (P2: 0x12/0x14/0x16) confirm to
   state 7 after 30 ticks with the m_sndMove2 voice.

   The tail clamps the cursor, resolves the drawn readout f_20c6d4 through
   the same availability floats, runs m434de0 @0x434de0 (STUB), and past 30
   ticks reloads the preview voice exactly like the mode-input handlers
   (same strings, annotated in src/songselectinput.cpp). */

#include <stdio.h>
#include "songselect.h"
#include "ez2acapp.h"
#include "bigbase.h"

struct ListNode;
extern ListNode *findListA(int key);            /* 0x418e30 */

extern Obj40C180 *g_bigInstance;                /* 0x1b2e710 (src/accessors.cpp) */
extern EzSubsystem *g_sub1b2eae4;               /* 0x1b2eae4 (src/ez2acapp.cpp) */
extern int g_modeIndex;                         /* 0x1b2e548 (src/modename.cpp) */
extern int g_1b2eb74;                           /* src/globals.cpp */
extern int g_4abd14;                            /* src/gamestate.cpp - the column cursor */
extern int g_37dced8;                           /* src/gamestate.cpp - its previous value */

/* rows[f84]'s four column-availability floats, this-relative. */
#define ROWF(off) (*(float *)((char *)this + f84 * 0x374 + (off)))

// FUNCTION: EZ2AC_UNPACKED 0x00444ae0
void SongSelectDirector::m444ae0(int a)
{
    char buf[260];
    int prev;
    int cur;

    g_1b2eb74 = 1;
    if (f_20c6bc != 0)
        return;
    if (f_bfdb4 != 5)
        return;
    f94 += 1;
    if ((char)a != 0)
    {
        if (g_bigInstance->getState(6) == 2 || g_bigInstance->getState(6) == 3)
            goto clampCursor;
        if (g_modeIndex == 4 || g_modeIndex == 5)
            goto bothSides;

        if (findListA(0) != 0)
        {
            /* P1 down. */
            if (g_bigInstance->getState(0xb) == 2 ||
                g_bigInstance->accum[0] < -10 ||
                g_bigInstance->getState(0x10) == 2)
            {
                cur = g_4abd14;
                prev = cur;
                cur = cur - 1;
                g_37dced8 = prev;
                g_4abd14 = cur;
                if (cur == 0)
                {
                    if (ROWF(0x33c) == 0.0f)
                        g_4abd14 = 1;
                }
                else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                    g_4abd14 = 1;
                else if (cur >= 3 && ROWF(0x358) == 0.0f)
                    g_4abd14 = 2;
                else if (cur >= 4 && ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                if (g_4abd14 != prev)
                {
                    m_bfd38->q5();
                    m_bfd38->q4(0);
                }
                else
                {
                    m_bfd3c->q5();
                    m_bfd3c->q4(0);
                }
            }
            /* P1 up. */
            if (g_bigInstance->getState(0xd) == 2 ||
                g_bigInstance->accum[0] > 10 ||
                g_bigInstance->getState(0xf) == 2)
            {
                cur = g_4abd14;
                prev = cur;
                cur = cur + 1;
                g_37dced8 = prev;
                g_4abd14 = cur;
                if (cur == 0)
                {
                    if (ROWF(0x33c) == 0.0f)
                        g_4abd14 = 1;
                }
                else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                    g_4abd14 = 1;
                else if (cur >= 3 && ROWF(0x358) == 0.0f)
                    g_4abd14 = 2;
                else if (cur >= 4)
                {
                    if (ROWF(0x364) == 0.0f)
                        g_4abd14 = 3;
                    else
                        g_4abd14 = 4;
                }
                if (g_4abd14 != prev)
                {
                    m_bfd38->q5();
                    m_bfd38->q4(0);
                }
                else
                {
                    m_bfd3c->q5();
                    m_bfd3c->q4(0);
                }
            }
            /* P1 back out, or confirm. */
            if (g_bigInstance->getState(4) == 2)
            {
                if (f_bfd48 != 0)
                {
                    g_1b2eb74 = 0;
                    f_bfdb4 = 4;
                }
            }
            else if (g_bigInstance->getState(0xa) == 2 ||
                     g_bigInstance->getState(0xc) == 2 ||
                     g_bigInstance->getState(0xe) == 2)
            {
                if (f94 >= 0x1e)
                {
                    m_sndMove2->q5();
                    m_sndMove2->q4(0);
                    f_bfdb4 = 7;
                }
            }
        }

        if (findListA(1) == 0)
            goto clampCursor;

        /* P2 down. */
        if (g_bigInstance->getState(0x13) == 2 ||
            g_bigInstance->accum[1] < -10 ||
            g_bigInstance->getState(0x18) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur - 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4 && ROWF(0x364) == 0.0f)
                g_4abd14 = 3;
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        /* P2 up. */
        if (g_bigInstance->getState(0x15) == 2 ||
            g_bigInstance->accum[1] > 10 ||
            g_bigInstance->getState(0x17) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur + 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4)
            {
                if (ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                else
                    g_4abd14 = 4;
            }
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        /* P2 back out with key 5, or confirm on 0x12/0x14/0x16. The exit
           and confirm bodies are written per arm - cl cross-jumps the
           copies back into the shared blocks the original lays out. */
        if (g_bigInstance->getState(5) == 2)
        {
            if (f_bfd48 != 0)
            {
                g_1b2eb74 = 0;
                f_bfdb4 = 4;
            }
            goto clampCursor;
        }
        if (g_bigInstance->getState(0x12) == 2 ||
            g_bigInstance->getState(0x14) == 2 ||
            g_bigInstance->getState(0x16) == 2)
        {
            if (f94 >= 0x1e)
            {
                m_sndMove2->q5();
                m_sndMove2->q4(0);
                f_bfdb4 = 7;
            }
        }
        goto clampCursor;

bothSides:
        /* Modes 4/5: one block over both sides' keys and wheels. */
        if (g_bigInstance->getState(0xb) == 2 ||
            g_bigInstance->getState(0x13) == 2 ||
            g_bigInstance->accum[0] < -10 ||
            g_bigInstance->accum[1] < -10 ||
            g_bigInstance->getState(0x10) == 2 ||
            g_bigInstance->getState(0x18) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur - 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4 && ROWF(0x364) == 0.0f)
                g_4abd14 = 3;
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        if (g_bigInstance->getState(0xd) == 2 ||
            g_bigInstance->getState(0x15) == 2 ||
            g_bigInstance->accum[1] > 10 ||
            g_bigInstance->getState(0xf) == 2 ||
            g_bigInstance->getState(0x17) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur + 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4)
            {
                if (ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                else
                    g_4abd14 = 4;
            }
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        if (g_bigInstance->getState(4) == 2 ||
            g_bigInstance->getState(5) == 2)
        {
            if (f_bfd48 != 0)
            {
                f_bfdb4 = 4;            /* this pair the other way round */
                g_1b2eb74 = 0;
            }
        }
        else if (g_bigInstance->getState(0xa) == 2 ||
                 g_bigInstance->getState(0xc) == 2 ||
                 g_bigInstance->getState(0xe) == 2)
        {
            if (f94 >= 0x1e)
            {
                m_sndMove2->q5();
                m_sndMove2->q4(0);
                f_bfdb4 = 7;
            }
        }
        else if (g_bigInstance->getState(0x12) == 2 ||
                 g_bigInstance->getState(0x14) == 2 ||
                 g_bigInstance->getState(0x16) == 2)
        {
            if (f94 >= 0x1e)
            {
                m_sndMove2->q5();
                m_sndMove2->q4(0);
                f_bfdb4 = 7;
            }
        }

clampCursor:
        if (g_4abd14 <= 0)
            g_4abd14 = 0;
        else if (g_4abd14 >= 4)
            g_4abd14 = 4;
        switch (g_4abd14)
        {
        case 0:
        case 1:
            f_20c6d4 = 0;
            break;
        case 2:
            if (g_4abd14 >= 1 && ROWF(0x34c) != 0.0f)
                f_20c6d4 = 1;
            else
            {
                f_20c6d4 = 0;
                g_4abd14 = 1;
            }
            break;
        case 3:
            if (g_4abd14 >= 1 && ROWF(0x358) != 0.0f)
                f_20c6d4 = 3;
            else
            {
                f_20c6d4 = 1;
                g_4abd14 = 2;
            }
            break;
        case 4:
            if (g_4abd14 >= 3 && ROWF(0x364) != 0.0f)
                f_20c6d4 = 4;
            else if (g_4abd14 >= 1 && ROWF(0x358) != 0.0f)
            {
                f_20c6d4 = 3;
                g_4abd14 = 3;
            }
            else
            {
                f_20c6d4 = 1;
                g_4abd14 = 2;
            }
            break;
        }
    }

    /* The drawn readout may not point at a column whose level vanished. */
    if (ROWF(0x364) == 0.0f && f_20c6d4 == 4)
        f_20c6d4 = 0;
    if (ROWF(0x358) == 0.0f && f_20c6d4 == 3)
        f_20c6d4 = 0;
    if (ROWF(0x34c) == 0.0f && f_20c6d4 == 1)
        f_20c6d4 = 0;
    m434de0();

    /* The preview-voice pump, as in the mode-input handlers (the two
       strings are annotated in src/songselectinput.cpp). */
    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }
}

/* The stepper's reduced twin @0x4458d0 - the same difficulty-column walk
   with only the two step keys per side (no wheel, no back-out, no
   confirm), and none of the state guards: update2 runs it for the pages
   whose select state machine lives elsewhere. Modes 4/5 combine both
   sides' keys; the move-up body is written per arm and cl folds the P2
   and combined copies into the original's shared block. */

// FUNCTION: EZ2AC_UNPACKED 0x004458d0
void SongSelectDirector::m4458d0(int a)
{
    char buf[260];
    int prev;
    int cur;

    if (f_20c6bc != 0)
        return;
    if ((char)a != 0)
    {
        if (g_bigInstance->getState(6) == 2 || g_bigInstance->getState(6) == 3)
            goto clampCursor2;
        if (g_modeIndex == 4 || g_modeIndex == 5)
            goto bothSides2;

        if (findListA(0) != 0)
        {
            /* P1 down. */
            if (g_bigInstance->getState(0xb) == 2)
            {
                cur = g_4abd14;
                prev = cur;
                cur = cur - 1;
                g_37dced8 = prev;
                g_4abd14 = cur;
                if (cur == 0)
                {
                    if (ROWF(0x33c) == 0.0f)
                        g_4abd14 = 1;
                }
                else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                    g_4abd14 = 1;
                else if (cur >= 3 && ROWF(0x358) == 0.0f)
                    g_4abd14 = 2;
                else if (cur >= 4 && ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                if (g_4abd14 != prev)
                {
                    m_bfd38->q5();
                    m_bfd38->q4(0);
                }
                else
                {
                    m_bfd3c->q5();
                    m_bfd3c->q4(0);
                }
            }
            /* P1 up. */
            if (g_bigInstance->getState(0xd) == 2)
            {
                cur = g_4abd14;
                prev = cur;
                cur = cur + 1;
                g_37dced8 = prev;
                g_4abd14 = cur;
                if (cur == 0)
                {
                    if (ROWF(0x33c) == 0.0f)
                        g_4abd14 = 1;
                }
                else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                    g_4abd14 = 1;
                else if (cur >= 3 && ROWF(0x358) == 0.0f)
                    g_4abd14 = 2;
                else if (cur >= 4)
                {
                    if (ROWF(0x364) == 0.0f)
                        g_4abd14 = 3;
                    else
                        g_4abd14 = 4;
                }
                if (g_4abd14 != prev)
                {
                    m_bfd38->q5();
                    m_bfd38->q4(0);
                }
                else
                {
                    m_bfd3c->q5();
                    m_bfd3c->q4(0);
                }
            }
        }

        if (findListA(1) == 0)
            goto clampCursor2;

        /* P2 down. */
        if (g_bigInstance->getState(0x13) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur - 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4 && ROWF(0x364) == 0.0f)
                g_4abd14 = 3;
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        /* P2 up. */
        if (g_bigInstance->getState(0x15) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur + 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4)
            {
                if (ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                else
                    g_4abd14 = 4;
            }
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        goto clampCursor2;

bothSides2:
        if (g_bigInstance->getState(0xb) == 2 ||
            g_bigInstance->getState(0x13) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur - 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4 && ROWF(0x364) == 0.0f)
                g_4abd14 = 3;
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }
        if (g_bigInstance->getState(0xd) == 2 ||
            g_bigInstance->getState(0x15) == 2)
        {
            cur = g_4abd14;
            prev = cur;
            cur = cur + 1;
            g_37dced8 = prev;
            g_4abd14 = cur;
            if (cur == 0)
            {
                if (ROWF(0x33c) == 0.0f)
                    g_4abd14 = 1;
            }
            else if (cur >= 2 && ROWF(0x34c) == 0.0f)
                g_4abd14 = 1;
            else if (cur >= 3 && ROWF(0x358) == 0.0f)
                g_4abd14 = 2;
            else if (cur >= 4)
            {
                if (ROWF(0x364) == 0.0f)
                    g_4abd14 = 3;
                else
                    g_4abd14 = 4;
            }
            if (g_4abd14 != prev)
            {
                m_bfd38->q5();
                m_bfd38->q4(0);
            }
            else
            {
                m_bfd3c->q5();
                m_bfd3c->q4(0);
            }
        }

clampCursor2:
        if (g_4abd14 <= 0)
            g_4abd14 = 0;
        else if (g_4abd14 >= 4)
            g_4abd14 = 4;
        switch (g_4abd14)
        {
        case 0:
        case 1:
            f_20c6d4 = 0;
            break;
        case 2:
            if (g_4abd14 >= 1 && ROWF(0x34c) != 0.0f)
                f_20c6d4 = 1;
            else
            {
                f_20c6d4 = 0;
                g_4abd14 = 1;
            }
            break;
        case 3:
            if (g_4abd14 >= 1 && ROWF(0x358) != 0.0f)
                f_20c6d4 = 3;
            else
            {
                f_20c6d4 = 1;
                g_4abd14 = 2;
            }
            break;
        case 4:
            if (g_4abd14 >= 3 && ROWF(0x364) != 0.0f)
                f_20c6d4 = 4;
            else if (g_4abd14 >= 1 && ROWF(0x358) != 0.0f)
            {
                f_20c6d4 = 3;
                g_4abd14 = 3;
            }
            else
            {
                f_20c6d4 = 1;
                g_4abd14 = 2;
            }
            break;
        }
    }

    if (ROWF(0x364) == 0.0f && f_20c6d4 == 4)
        f_20c6d4 = 0;
    if (ROWF(0x358) == 0.0f && f_20c6d4 == 3)
        f_20c6d4 = 0;
    if (ROWF(0x34c) == 0.0f && f_20c6d4 == 1)
        f_20c6d4 = 0;
    m434de0();

    f_20e894++;
    if (f_20e894 > 0x1e)
    {
        if (!(m_sndPreview->q16() & 1))
        {
            sprintf(buf, "System\\Preview\\%s.wav", rows[f84].name);
            g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf);
            if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, buf) == 0)
            {
                m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
                    "System\\Preview\\0Blank.wav", 1, 0);
            }
            if (m_sndPreview) m_sndPreview->q4(1);
        }
    }
}

/* The genre-strip pager @0x4445c0 - keys 7 (up) and 8 (down), or their
   150-tick holds counted in f_c0e28/f_c0e2c, step the category index
   f_bc57c through 0..0x2e with per-mode skip bands (mode 1 jumps 0x1e-0x24
   -> 0x25 going down and 0x1f-0x25 -> 0x1e going up; mode 0 the 0x23/0x24
   pair), wrap at both ends, and re-seed the page with m435070. Each step
   resets the preview pump and repoints the version/sortstyle charts. The
   tail eases f_bc584 a third of the way toward index * 100 around the
   4700-unit ring, snapping inside two one-unit bands. Skipped in state 5
   (the difficulty stepper owns the keys there) and in the radio modes. */

// FUNCTION: EZ2AC_UNPACKED 0x004445c0
void SongSelectDirector::m4445c0()
{
    int v;
    float target;
    float delta;
    float t;

    if (f_20c6bc == 0 && f_bfdb4 != 5)
    {
        if (g_bigInstance->getState(0x11) != 2 &&
            g_bigInstance->getState(0x11) != 3 &&
            g_bigInstance->getState(0x19) != 2 &&
            g_bigInstance->getState(0x19) != 3)
        {
            if (g_bigInstance->getState(7) == 3 &&
                g_bigInstance->heldFor(7) >= 0x96)
                f_c0e28 += 1;
            if (g_bigInstance->getState(8) == 3 &&
                g_bigInstance->heldFor(8) >= 0x96)
                f_c0e2c += 1;
            if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
                g_modeIndex != 9 && g_modeIndex != 0xc)
            {
                /* Page down: key 8 or its hold counter. */
                if ((g_bigInstance->getState(8) == 2 || f_c0e2c > 3) &&
                    f_20c6bc == 0)
                {
                    v = f_bc57c;
                    if (v >= 0x2e)
                        f_bc57c = 0;
                    else if (g_modeIndex == 1 && v >= 0x1e && v <= 0x24)
                        f_bc57c = 0x25;
                    else if (g_modeIndex == 0 && v >= 0x23 && v <= 0x24)
                        f_bc57c = 0x25;
                    else
                        f_bc57c = v + 1;
                    f_c0e2c = 0;
                    f84 = 0;
                    f34 = 1;
                    m435070(f_bc57c, 0, 1);
                    f_f8 = 1;
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    f_20e894 = 0;
                    if (m_sndPreview) m_sndPreview->q5();
                    v = f_bc57c;
                    if (v >= 2 && v <= 0x12)
                    {
                        m_beaf4.setCell(2, 0, f_bffc0[rows[f84].f264]);
                        m_beaf4.reset();
                    }
                    v = f_bc57c;
                    if ((v >= 0 && v <= 1) || (v >= 0x13 && v <= 0x2e))
                    {
                        if (v == 0)
                            m_bed10.setCell(2, 0, f_c000c[0]);
                        else if (v == 1)
                            m_bed10.setCell(2, 0, f_c000c[1]);
                        else if (v >= 0x13 && v <= 0x2e)
                            m_bed10.setCell(2, 0, f_bffc0[v + 2]);
                        m_bed10.reset();
                    }
                }
                /* Page up: key 7 or its hold counter. */
                if ((g_bigInstance->getState(7) == 2 || f_c0e28 > 3) &&
                    f_20c6bc == 0)
                {
                    m_sndMove->q5();
                    m_sndMove->q4(0);
                    v = f_bc57c;
                    if (v > 0)
                    {
                        if (g_modeIndex == 1 && v <= 0x25 && v >= 0x1f)
                            f_bc57c = 0x1e;
                        else if (g_modeIndex == 0 && v <= 0x25 && v >= 0x24)
                            f_bc57c = 0x23;
                        else
                            f_bc57c = v - 1;
                    }
                    else
                        f_bc57c = 0x2e;
                    f_c0e28 = 0;
                    f84 = 0;
                    f34 = 1;
                    m435070(f_bc57c, 0, 1);
                    f_f8 = 1;
                    if (f7c == 1)
                        f24 = 1;
                    f_20e894 = 0;
                    if (m_sndPreview) m_sndPreview->q5();
                    v = f_bc57c;
                    if (v >= 2 && v <= 0x12)
                    {
                        m_beaf4.setCell(2, 0, f_bffc0[rows[f84].f264]);
                        m_beaf4.reset();
                    }
                    v = f_bc57c;
                    if ((v >= 0 && v <= 1) || (v >= 0x13 && v <= 0x2e))
                    {
                        if (v == 0)
                            m_bed10.setCell(2, 0, f_c000c[0]);
                        else if (v == 1)
                            m_bed10.setCell(2, 0, f_c000c[1]);
                        else if (v >= 0x13 && v <= 0x2e)
                            m_bed10.setCell(2, 0, f_bffc0[v + 2]);
                        m_bed10.reset();
                    }
                }
            }
        }
    }

    /* Ease the strip a third of the way toward index * 100 around the
       4700-unit ring, snapping inside the two one-unit bands. */
    target = (float)f_bc57c * 100.0f;
    delta = target - f_bc584;
    *(float *)&f_bc578 = target;
    if (delta > 2350.0f)
        f_bc584 = f_bc584 - (4700.0f - delta) * 0.33333334f;
    else
    {
        t = delta;
        if (-2350.0f > delta)
            t = delta + 4700.0f;
        f_bc584 = t * 0.33333334f + f_bc584;
    }
    if (1.0f > delta && delta > -1.0f)
        f_bc584 = target;
    if (-4699.0f > delta && delta > -4701.0f)
        f_bc584 = target;
    if (f_bc584 > 4700.0f)
        f_bc584 = f_bc584 - 4700.0f;
    if (0.0f > f_bc584)
        f_bc584 = f_bc584 + 4700.0f;
}
