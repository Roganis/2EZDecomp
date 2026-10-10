/* The three backgrounds' update2 - BackGround / GFBackGround @0x45ca90
   (ICF-folded onto one address) and CatchBackGround @0x46b6d0.

   Nothing happens until the background is running (+0x20 / +0x18). Then the
   run's elapsed time is turned into SIXTIETHS OF A SECOND and published to
   +0x102118 / +0x10210c: the saturating subtract @0x4145d0 gives the
   unsigned millisecond delta from the base at +0x1c / +0x14, and 0.06 is the
   ms-to-frame factor. That is the unit the judgement notice's ten-step
   debounce counts in.

   The sub-object at +0x1c0 / +0x1b8 is then ticked, and finally either the
   script walker runs (while the alternate-draw flag at +0xac / +0xa4 is up)
   or the other helper does.

   The unsigned delta is what puts the 2^32 fixup in front of the fild, and
   the destination being unsigned is what makes the store an inline
   fistp-to-int64 rather than a call into _ftol2.

   THE PROMOTION IS ITS OWN STATEMENT, AND THAT IS WHAT SPLITS THE ftol
   BLOCK AROUND THE NEXT CALL'S PUSH. Written as one expression this sat at
   93.18%: cl emitted fmul and then hoisted the following call's `push edi`
   into the FP shadow, leaving the control-word save (fnstcw/movzx) after
   both, where the original has it BEFORE the fmul and the push wedged
   between `or ah,0xc` and the fldcw. Naming the unsigned-to-double
   promotion in a `double` local and multiplying it afterwards gives the
   original's schedule exactly. Note it must be the PROMOTION that is named,
   not the product - `double d = SatSub(..) * 0.06;` is still 93.18%, as are
   a uint local, a __int64 local, an explicit (__int64) cast, `unsigned
   long`, reciprocal division, a separate object pointer and a `now + 0`
   argument. Eight forms, one works.

   BackGround and GFBackGround are layout-identical and the original's linker
   folded these two bodies onto one address (see include/backgrounds.h), so
   both carry a `folded` annotation and the body is written out twice; the
   BackGround copy works through a GFBackGround pointer purely so its two
   helper calls name the one address the fold left. */

#include "backgrounds.h"

extern unsigned int SatSub(unsigned int a, unsigned int b);  /* 0x4145d0 (src/misc.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0045ca90 folded
void GFBackGround::update2(int now, int /*b*/)
{
    if (f20 != 0)
    {
        double d = SatSub(now, f1c);
        f102118 = (unsigned int)(d * 0.06);
        m_1c0.m40b9f0(now);

        if (fac != 0)
        {
            m41b370(now);
        }
        else
        {
            m45c6a0(now);
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0045ca90 folded
void BackGround::update2(int now, int /*b*/)
{
    GFBackGround *g = (GFBackGround *)this;

    if (g->f20 != 0)
    {
        double d = SatSub(now, g->f1c);
        g->f102118 = (unsigned int)(d * 0.06);
        g->m_1c0.m40b9f0(now);

        if (g->fac != 0)
        {
            g->m41b370(now);
        }
        else
        {
            g->m45c6a0(now);
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b6d0
void CatchBackGround::update2(int now, int /*b*/)
{
    if (f18 != 0)
    {
        double d = SatSub(now, f14);
        f10210c = (unsigned int)(d * 0.06);
        m_1b8.m40b9f0(now);

        if (fa4 != 0)
        {
            m46b3e0(now);
        }
        else
        {
            m46ae40(now);
        }
    }
}
