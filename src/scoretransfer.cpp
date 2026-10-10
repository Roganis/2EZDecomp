/* ResultDirector::m450fa0 @0x450fa0 and GFResultDirector::m467720 @0x467720 -
   the battle result's score transfer, one tick of it.

   The two are BYTE-IDENTICAL (100.0% by mnemonic sequence) and the original
   did not fold them because each addresses its own class's copy of the
   group - the scores, the latches, the per-tick step and the winner/loser
   pair, which sit 0x330/0x340/0x340/0x348 apart between the two classes.

   Each tick moves the winner's step onto the winner's latch and takes the
   loser's step off the loser's latch, so the two counters roll towards each
   other on screen. When the loser's latch would go negative the animation is
   over: it is clamped to zero and BOTH latches are snapped to the real
   scores, so whatever rounding the steps accumulated is discarded.

   Mode 1 is excluded outright, and nothing happens once the loser's latch has
   already reached zero.

   Note the members are re-read at every use rather than cached - the source
   indexes through `winner` and `loser` inline, which is what the original's
   repeated loads of +0x2c94/+0x2c98 say. */

#include "directors.h"

extern int g_modeIndex;    /* 0x1b2e548 (src/modename.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00450fa0
void ResultDirector::m450fa0()
{
    if (g_modeIndex == 1)
        return;
    if (m_scoreLatch[loser] <= 0)
        return;

    m_scoreLatch[winner] += m_scoreStep[winner];
    m_scoreLatch[loser] -= m_scoreStep[winner];

    if (m_scoreLatch[loser] >= 0)
        return;

    m_scoreLatch[loser] = 0;
    m_scoreLatch[winner] = m_score[winner];
    m_scoreLatch[loser] = m_score[loser];
}

// FUNCTION: EZ2AC_UNPACKED 0x00467720
void GFResultDirector::m467720()
{
    if (g_modeIndex == 1)
        return;
    if (m_scoreLatch[loser] <= 0)
        return;

    m_scoreLatch[winner] += m_scoreStep[winner];
    m_scoreLatch[loser] -= m_scoreStep[winner];

    if (m_scoreLatch[loser] >= 0)
        return;

    m_scoreLatch[loser] = 0;
    m_scoreLatch[winner] = m_score[winner];
    m_scoreLatch[loser] = m_score[loser];
}

/* ResultDirector::m451040 @0x451040 and GFResultDirector::m4677c0 @0x4677c0 -
   advance every present player's latched score by its step.

   BYTE-IDENTICAL twins, found by scratchpad/twins2.py at a ratio of exactly
   1.0; the only difference is the three member offsets, 0x330 and 0x340 apart
   as usual for this pair of classes.

   The walk is m4510a0 @0x4510a0's, one screen over: nextListA guarded once
   and then re-tested at the bottom, with the player id read out of each node.
   The gate is the same odd one too - a KTexture pointer compared as a SIGNED
   int, cast rather than retyped so the draw that needs it as a texture keeps
   working. Only the body differs: where m4510a0 SNAPS the latch to the real
   score, this adds one step to it, which is what runs the roll-up the
   transfer @0x450fa0 counts down.

   And ONE more thing differs, which is the whole 70.59% -> 100%: m4510a0
   names the player id in a local, this pair does NOT - it reads cur->f8 at
   all three sites and lets cl common-subexpression it. The two forms give
   the same single load but opposite registers, and the inline one adds the
   `mov eax,ecx` the original has. Two functions three screens apart, one
   naming decision. */

extern int nextListA(struct ListNode **cur);   /* 0x4182e0 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00451040
void ResultDirector::m451040()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            if ((int)m_2c64[cur->f8] > 0)
                m_scoreLatch[cur->f8] += m_scoreStep[cur->f8];
        }
        while (nextListA(&cur) != 0);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x004677c0
void GFResultDirector::m4677c0()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            if ((int)m_2934[cur->f8] > 0)
                m_scoreLatch[cur->f8] += m_scoreStep[cur->f8];
        }
        while (nextListA(&cur) != 0);
    }
}

extern int g_1b2eb6c;   /* 0x1b2eb6c (src/ez2acapp.cpp) */

/* ResultDirector::m451180 @0x451180 - pay the pending bonus, once.

   The same nextListA walk as m451040 above, but the body fires at most once
   for the whole list: it is gated on a flag that it CLEARS, so the second
   player's pass finds nothing to do. Whichever player the walk reaches first
   while the flag is set gets its bonus added and the sound restarted -
   stopped through slot 5 and played through slot 4, which is how this engine
   retriggers a sound that may already be running.

   The bonus lands in the same word m451040 gates on, and that word is typed
   as a texture pointer because a matched draw needs it that way; the int use
   goes through a cast, which is the convention src/directors.cpp already set
   for it.

   And unlike m451040 this one DOES name the player id: the original reads
   the node into ecx and the id into eax with no copy, which is the named
   form - the inline form would put them the other way round and add a
   `mov eax,ecx`. The two functions are 0x140 bytes apart and disagree. */
// FUNCTION: EZ2AC_UNPACKED 0x00451180
void ResultDirector::m451180()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            if (g_1b2eb6c != 0 && m_2c3c == 1)
            {
                m_28ec->q5();
                m_28ec->q4(0);

                /* Read AFTER the two calls, not before them: declared at the
                   top of the block the id spans them, so cl parks it in a
                   callee-saved register and pays a `push edi` for it. */
                int id = cur->f8;

                *(int *)&m_2c64[id] += m_2c30[id];
                m_2c3c = 0;
            }
        }
        while (nextListA(&cur) != 0);
    }
}
