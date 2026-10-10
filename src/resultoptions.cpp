/* ResultDirector::m4512b0 @0x4512b0 - the option-icon readout, called at the
   top of ResultDirector::update2 @0x451550 every frame. 663 bytes, 213
   instructions, no float: three cells of one chart, each filled by a ladder of
   option flags picking one texture.

   Every arm writes the setCell itself rather than assigning a texture and
   calling once, and the chart is named INLINE (m_89c.setCell) rather than
   through a local: a named `Obj402860 *c = &m_89c;` hoists its `lea` to the
   top of the function, where the original materializes the address once inside
   the shared call block the arms cross-jump into. That is what the original has - cl cross-jumps the arms back
   into ONE call fed by `push X; jmp`, which is the documented shape for two
   calls that must not go branchless - and it is why the ladders read as they
   do here.

   CELL 3 IS A MODE LADDER over a second selector, g_1b5f178, which this
   function shows is an ENUM and not the boolean its first use suggested: it is
   tested against 1, 2, 3 and 4 with a default, twice over. Modes 5 and 9 get a
   three-way version of it, modes 0 and 10 get one fixed icon, and everything
   else the four-way version.

   The four flags at 0x1b2e8b0/0x1b2e8b8/0x1b2e8d8/0x1b2e8e0 are elements 1, 2,
   6 and 7 of the stride-8 block that starts at g_table4184 @0x1b2e8a8, whose
   elements 3, 4 and 5 were already named individually. */

#include "ezscreen.h"
#include "directors.h"
#include "obj402860.h"

struct Pair4184 { int a; int b; };
extern Pair4184 g_table4184[];   /* 0x1b2e8a8 (src/globals.cpp) */

extern int g_modeIndex;          /* 0x1b2e548 (src/modename.cpp) */
extern int g_1b2e588;
extern int g_kind8b0, g_kind8b8, g_kind8c0, g_kind8c8, g_kind8d0;
extern int g_kind8d8, g_kind8e0, g_kind958, g_kind960, g_kind970;
extern int g_kind978, g_kind990;
extern int g_1b2e998, g_1b2e9a0, g_1b2ebbc, g_1b2ef38;
extern int g_1b5f178;

// FUNCTION: EZ2AC_UNPACKED 0x004512b0
void ResultDirector::m4512b0()
{
    /* Cell 2 - the mode badge. */
    if (g_table4184[0].a != 0)
        m_89c.setCell(2, 0, (int)m_optTex[26]);
    else if (g_kind8b0 != 0)
        m_89c.setCell(2, 0, (int)m_optTex[27]);
    else if (g_kind8b8 != 0)
        m_89c.setCell(2, 0, (int)m_optTex[28]);
    else if (g_kind8d8 != 0)
        m_89c.setCell(2, 0, (int)m_optTex[22]);
    else if (g_kind8e0 != 0)
        m_89c.setCell(2, 0, (int)m_optTex[23]);
    else if (g_1b2e588 == 1)
        m_89c.setCell(2, 0, (int)m_optTex[24]);
    else
        m_89c.setCell(2, 0, (int)m_optTex[25]);

    /* Cell 1 - the play-option badge. Two different flags select the SAME
       texture (index 6), which is in the original too. */
    if (g_kind8c8 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[2]);
    else if (g_kind990 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[3]);
    else if (g_kind8c0 != 0)
    {
        if (g_1b2ef38 <= 0 && g_1b2ebbc == 0)
            m_89c.setCell(1, 0, (int)m_optTex[1]);
        else
            m_89c.setCell(1, 0, (int)m_optTex[4]);
    }
    else if (g_kind8d0 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[5]);
    else if (g_kind958 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[7]);
    else if (g_1b2e9a0 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[8]);
    else if (g_kind960 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[6]);
    else if (g_1b2e998 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[6]);
    else if (g_kind970 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[9]);
    else if (g_kind978 != 0)
        m_89c.setCell(1, 0, (int)m_optTex[10]);
    else
        m_89c.setCell(1, 0, (int)m_optTex[0]);

    /* Cell 3 - the difficulty badge, over g_1b5f178. */
    if (g_modeIndex == 5 || g_modeIndex == 9)
    {
        if (g_1b5f178 == 1)
            m_89c.setCell(3, 0, (int)m_optTex[17]);
        else if (g_1b5f178 == 2)
            m_89c.setCell(3, 0, (int)m_optTex[18]);
        else if (g_1b5f178 == 3)
            m_89c.setCell(3, 0, (int)m_optTex[19]);
        else
            m_89c.setCell(3, 0, (int)m_optTex[16]);
    }
    else if (g_modeIndex == 0 || g_modeIndex == 10)
    {
        m_89c.setCell(3, 0, (int)m_optTex[20]);
    }
    else if (g_1b5f178 == 1)
        m_89c.setCell(3, 0, (int)m_optTex[12]);
    else if (g_1b5f178 == 2)
        m_89c.setCell(3, 0, (int)m_optTex[13]);
    else if (g_1b5f178 == 3)
        m_89c.setCell(3, 0, (int)m_optTex[14]);
    else if (g_1b5f178 == 4)
        m_89c.setCell(3, 0, (int)m_optTex[15]);
    else
        m_89c.setCell(3, 0, (int)m_optTex[11]);
}

/* GFResultDirector::m4679e0 @0x4679e0 - the same readout for the scratch
   result screen, and the twin of m4512b0 above (0.563 on the masked-opcode
   ratio, which for this tree is a real relationship rather than the 0.09 noise
   two false leads scored). 423 bytes, 121 instructions.

   Cells 2 and 1 are the SAME two ladders over the same globals; only the
   textures and the chart differ. Cell 3 is far shorter - one test on
   g_modeIndex for mode 11 and then the SWITCH pair - where the keys copy has
   two nested g_1b5f178 ladders.

   The other real difference is inside cell 1's g_kind8c0 arm: this one tests
   only `g_1b2ef38 > 0`, where the keys copy also requires g_1b2ebbc == 0. The
   two textures it picks between are at the same relative offsets in both
   classes (+4 and +0x10 from the strip's base), so it is the CONDITION that
   differs, not the choice.

   AND IT NAMES THE WHOLE LADDER. This class's option strip was already carved
   with its asset paths by the constructor's asset map, so the ladder reads
   directly: cell 1 is the RANDOM effector icon (SRANDOM, PS, FR, MRANDOM,
   MIRROR, HRANDOM, KEY, SP...) and cell 2 the FADE/REVERSE one. The keys
   copy's m_optTex[] indices are the same options in the same order - worth
   renaming there when that class's constructor is mapped. */

extern int g_1b2e7d0;            /* src/gamestate.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x004679e0
void GFResultDirector::m4679e0()
{
    /* Cell 2 - the fade / reverse icon. */
    if (g_table4184[0].a != 0)
        m_88c.setCell(2, 0, (int)m_23a8);
    else if (g_kind8b0 != 0)
        m_88c.setCell(2, 0, (int)m_23ac);
    else if (g_kind8b8 != 0)
        m_88c.setCell(2, 0, (int)m_23b0);
    else if (g_kind8d8 != 0)
        m_88c.setCell(2, 0, (int)m_239c);
    else if (g_kind8e0 != 0)
        m_88c.setCell(2, 0, (int)m_23a0);
    else if (g_1b2e588 == 1)
        m_88c.setCell(2, 0, (int)m_23bc);
    else
        m_88c.setCell(2, 0, (int)m_23a4);

    /* Cell 1 - the random-effector icon. Two flags pick HRANDOM, as in the
       keys copy. */
    if (g_kind8c8 != 0)
        m_88c.setCell(1, 0, (int)m_2354);
    else if (g_kind990 != 0)
        m_88c.setCell(1, 0, (int)m_2358);
    else if (g_kind8c0 != 0)
    {
        if (g_1b2ef38 > 0)
            m_88c.setCell(1, 0, (int)m_235c);
        else
            m_88c.setCell(1, 0, (int)m_2350);
    }
    else if (g_kind8d0 != 0)
        m_88c.setCell(1, 0, (int)m_2360);
    else if (g_kind958 != 0)
        m_88c.setCell(1, 0, (int)m_2368);
    else if (g_1b2e9a0 != 0)
        m_88c.setCell(1, 0, (int)m_236c);
    else if (g_kind960 != 0)
        m_88c.setCell(1, 0, (int)m_2364);
    else if (g_1b2e998 != 0)
        m_88c.setCell(1, 0, (int)m_2364);
    else if (g_kind970 != 0)
        m_88c.setCell(1, 0, (int)m_2370);
    else if (g_kind978 != 0)
        m_88c.setCell(1, 0, (int)m_2374);
    else
        m_88c.setCell(1, 0, (int)m_234c);

    /* Cell 3 - the SWITCH icon, mode 11 only. */
    if (g_modeIndex == 11)
    {
        if (g_1b2e7d0 == 0)
            m_88c.setCell(3, 0, (int)m_23b4);
        else if (g_1b2e7d0 == 1)
            m_88c.setCell(3, 0, (int)m_23b8);
    }
}
