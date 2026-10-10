/* ScoreKeeper::rank @0x472b40 - the grade the round is awarded.
 *
 * Two ladders, chosen by the mode flag at 0x1b2eb6c:
 *   flag set   -> a 0..6 grade from the HIT percentage
 *   flag clear -> a 0..10 grade from the SCORE percentage
 * Both fall through to 0, and cl cross-jumps the shared tails so the seven
 * lower `return`s are written once.
 *
 * The maximum score is not a constant: every note is worth 250 plus a
 * logarithmic bonus, accumulated one note at a time. That loop is why the
 * function exists, and it is the part a reimplementation cannot guess.
 */

#include <math.h>
#include "scorekeeper.h"

/* src/loops.cpp - which player slot the round is working on. */
extern int g_elemCount;   /* 0x1b2e718 */
/* src/ez2acapp.cpp - picks which of the two grade ladders applies. */
extern int g_1b2eb6c;
/* src/gamepanels.cpp - whether the round's own counter pair overrides the
   accumulated ones. */
extern int g_1b2ebac;

/* src/gamestate.cpp and src/modeselectctor.cpp - the five-element per-player
   arrays the round publishes its result into. */
extern int g_1b2f058[5], g_1b2ebe0[5], g_1b5f15c[5], g_1b2f120[5];
extern int g_1b2ea38[5], g_1b2e9fc[5], g_1b2e668[5];

// FUNCTION: EZ2AC_UNPACKED 0x00472b40
int ScoreKeeper::rank()
{
    ScoreRecord *r = &rec[g_elemCount];
    int count = r->f0;
    int total = 0;
    int i = 0;
    float hitPct;
    float scorePct;

    if (count > 0)
    {
        do
        {
            /* The two constants must stay two multiplies: adjacent float
               literals get folded into one by cl (see CLAUDE.md), and the
               original has both. */
            total += 250 - (int)((log10(i + 1.0f) * 0.2f) * -250.0f);
            i++;
        } while (i < count);
    }

    hitPct = (float)(r->f18 + r->f14) / (float)count * 100.0f;
    scorePct = (float)m_1e8 / (float)total * 100.0f;

    if (g_1b2eb6c != 0)
    {
        if (hitPct >= 95.0f) return 6;
        if (hitPct >= 90.0f) return 5;
        if (hitPct >= 80.0f) return 4;
        if (hitPct >= 70.0f) return 3;
        if (hitPct >= 60.0f) return 2;
        if (hitPct >= 50.0f) return 1;
        return 0;
    }

    if (scorePct >= 100.0f) return 10;
    if (scorePct >= 98.0f) return 9;
    if (scorePct >= 95.0f) return 8;
    if (scorePct >= 93.0f) return 7;
    if (scorePct >= 90.0f) return 6;
    if (scorePct >= 85.0f) return 5;
    if (scorePct >= 80.0f) return 4;
    if (scorePct >= 70.0f) return 3;
    if (scorePct >= 60.0f) return 2;
    if (scorePct >= 50.0f) return 1;
    return 0;
}

/* ScoreKeeper::commit @0x472d10 - fold the round's counters into the current
   player's record, then publish the headline numbers into the per-player
   arrays and clear the round's own counters.

   It lives beside rank() because it keeps `this` in ecx across both calls to
   it, which cl only does for a callee whose body it can see; while rank was a
   stub this function sat at 33.82% purely on the fictional clobber set.

   THE ARMS SHARE THEIR LAST ADD, AND THAT MAKES THE VALUE A NAMED LOCAL.
   Both arms end `mov edx,<the f8 addend>` and fall into ONE
   `add [esi+8],edx`; written as `r->f8 += m_54c;` / `r->f8 += m_1c8;` the two
   arms pick different registers (the alternation starts at eax in each), so
   they are not identical code and cl duplicates the add instead of sharing it
   - 69 instructions against the original's 68, and every offset after the join
   shifts. Assigning to one local and adding after the join gives cl a single
   allocation for the value, so both arms end in the same register and the
   cross-jump happens. (Same family as the shared-tail-after-constant-folding
   rule: what is shared has to be shared in the SOURCE.)

   Still not byte-identical: that one local lands in eax where the original has
   edx, and the tail's whole eax/edx alternation follows it. Swept without
   effect - declaration order (four permutations), swapping the header stores,
   reading the value back from m_1c8 instead of m_54c, moving the shared add
   one statement later, and ten source orderings of the else-arm's five adds. */
// FUNCTION: EZ2AC_UNPACKED 0x00472d10
void ScoreKeeper::commit()
{
    ScoreRecord *r;
    int p;
    int v;

    r = &rec[g_elemCount];
    r->f0 += m_558;

    /* The if-arm folds the values it has just stored straight into its adds,
       which is why both arms are written out rather than the stores alone. */
    if (g_1b2ebac != 0)
    {
        m_1d8 = m_550;
        m_1c8 = m_54c;
        r->f18 += m_550;
        r->f14 += m_1d4;
        r->f10 += m_1d0;
        r->fc  += m_1cc;
        v = m_54c;
    }
    else
    {
        r->f18 += m_1d8;
        r->f14 += m_1d4;
        r->f10 += m_1d0;
        r->fc  += m_1cc;
        v = m_1c8;
    }

    r->f8  += v;
    r->f1c += m_1dc;
    r->f20 += m_1e0;
    r->f24 += m_1e8;
    r->f2c += m_1ec;
    r->f30 = rank();

    /* Re-read: the call may have moved it. */
    p = g_elemCount;
    g_1b2f058[p] = m_1e8;
    g_1b2ebe0[p] = m_1e4;
    g_1b5f15c[p] = m_1d8;
    g_1b2f120[p] = m_1c8;
    g_1b2ea38[p] = m_558;
    g_1b2e9fc[p] = rank();
    g_1b2e668[p] = m_1dc;

    m_1e8 = 0;
    m_1e0 = 0;
    m_1e4 = 0;
    m_35c = 0;
}
