/* The three backgrounds' per-frame draw, integrated from wip/bgupdate-45c930.cpp.

   BackGround::update and GFBackGround::update are THE SAME ADDRESS @0x45c930 -
   the original's linker ICF-folded them - so both carry a `folded` FUNCTION
   annotation and both bodies are written out, per the rule that a folded alias
   must carry the real body. CatchBackGround::update @0x46b570 is the same code
   against its own class's offsets and is its own function.

   The device is cleared, the draw list run (or the alternate draw when the
   +0xac flag is set), the texture stages unbound; and then, only with both the
   +0x24 and +0xac flags set, a two-line debug overlay is printed at x=320.

   THE OVERLAY'S DEFAULTS ARE WHY IT LOOKS ODD. Each of the three sub-objects
   at +0x103534 may be null, and the second and third only contribute their
   +0x84 when their own first word reads 1 - so the defaults are -1, and two of
   the values printed are the FIRST words rather than the +0x84s.

   g_d36cb0 is saved, zeroed for the duration and put back: the quad drawer
   reads it, so this suppresses whatever it gates while the background draws -
   the same suppression the script loader does around its own work.

   IT WAS PARKED AT 58.94% ON A FRAME-SIZE WALL, and the park's own next
   experiment was right. The original reserves 0x10 of locals and touches only
   EIGHT bytes of them; cl reserved 8. Two dword slots round to 8 and three
   round to 16, so a THIRD dword local owned a slot whose store did not
   survive - the address-exposed-aggregate signature. `volatile` on the obvious
   candidate keeps the store but NOT the slot, which is why it bought nothing.
   Declaring the three overlay readouts as ONE `int ov[3]` reserves all three
   slots and the frame comes out at 0x10, matching.

   What is left is register assignment inside the overlay block - the original
   threads it through ecx where cl uses edi - with every instruction present
   and one branch a byte short as a consequence. That is the ordinary
   allocation coin, not a structural defect. */

#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

/* The graphics singleton under the names its matched wrappers use. */
struct Obj40BB00 { void doOp10(unsigned int c); };   /* 0x40a730 */
struct Obj406C00 { void setField12c(int v); };       /* 0x406c00 */

extern int  g_d36cb0;    /* src/kgraphicsquad.cpp */
extern void drawTextf(struct KGraphics *g, int x, int y, const char *fmt, ...);
                         /* 0x4071d0 */

// FUNCTION: EZ2AC_UNPACKED 0x0045c930 folded
void BackGround::update()
{
    BgDbg *d;
    int saved;
    int ov[3];   /* v8, vc, v10 as one aggregate - see the header */
    int p1, p2;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);

    if (f20 != 0)
    {
        if (fac != 0)
            m45c890();
        else
            drawList();

        ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
        ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
        ((KGraphics *)g_sub1b2f054)->m408130(0);

        if (f24 != 0)
        {
            ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff00);

            if (fac != 0)
            {
                ov[0] = -1;
                p1 = -1;
                p2 = -1;
                ov[1] = -1;
                ov[2] = -1;

                d = f103534;
                if (d != 0)
                {
                    if (d->f8 != 0)
                        ov[0] = d->f8[0];
                    if (d->fc != 0)
                    {
                        ov[1] = d->fc[0];
                        if (ov[1] == 1)
                            p1 = d->fc[0x84 / 4];
                    }
                    if (d->f10 != 0)
                    {
                        ov[2] = d->f10[0];
                        if (ov[2] == 1)
                            p2 = d->f10[0x84 / 4];
                    }
                }

                // STRING: EZ2AC_UNPACKED 0x0048e238
                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0, "%s %d: [ %d, %d, %d]",
                          f28, f102124, ov[0], ov[1], ov[2]);
                // STRING: EZ2AC_UNPACKED 0x0048e220
                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0x14, "(%p:%d, %p:%d, %p:%d)",
                          m_charts[0].head, m_charts[0].f4,
                          m_charts[1].head, p1,
                          m_charts[2].head, p2);
            }
        }
    }

    g_d36cb0 = saved;
}

/* The folded twin - same address, same bytes, its own class's names. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c930 folded
void GFBackGround::update()
{
    BgDbg *d;
    int saved;
    int ov[3];   /* v8, vc, v10 as one aggregate - see the header */
    int p1, p2;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);

    if (f20 != 0)
    {
        if (fac != 0)
            ((BackGround *)this)->m45c890();
        else
            ((BackGround *)this)->drawList();

        ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
        ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
        ((KGraphics *)g_sub1b2f054)->m408130(0);

        if (f24 != 0)
        {
            ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff00);

            if (fac != 0)
            {
                ov[0] = -1;
                p1 = -1;
                p2 = -1;
                ov[1] = -1;
                ov[2] = -1;

                d = *(BgDbg **)((char *)this + 0x103534);
                if (d != 0)
                {
                    if (d->f8 != 0)
                        ov[0] = d->f8[0];
                    if (d->fc != 0)
                    {
                        ov[1] = d->fc[0];
                        if (ov[1] == 1)
                            p1 = d->fc[0x84 / 4];
                    }
                    if (d->f10 != 0)
                    {
                        ov[2] = d->f10[0];
                        if (ov[2] == 1)
                            p2 = d->f10[0x84 / 4];
                    }
                }

                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0, "%s %d: [ %d, %d, %d]",
                          f28, f102124, ov[0], ov[1], ov[2]);
                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0x14, "(%p:%d, %p:%d, %p:%d)",
                          m_charts[0].head, m_charts[0].f4,
                          m_charts[1].head, p1,
                          m_charts[2].head, p2);
            }
        }
    }

    g_d36cb0 = saved;
}

/* The catch copy - its own function, and its own offsets: the two flags the
   others keep at +0x20/+0x24 are at +0x18/+0x1c here, the alternate-draw gate
   at +0xa4, the overlay's number at +0x102118 and its sub-object block at
   +0x103528. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b570
void CatchBackGround::update()
{
    BgDbg *d;
    int saved;
    int ov[3];   /* v8, vc, v10 as one aggregate - see the header */
    int p1, p2;

    saved = g_d36cb0;
    g_d36cb0 = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);

    if (f18 != 0)
    {
        if (fa4 != 0)
            m46b4d0();
        else
            drawList();

        ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
        ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
        ((KGraphics *)g_sub1b2f054)->m408130(0);

        if (f1c != 0)
        {
            ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff00);

            if (fa4 != 0)
            {
                ov[0] = -1;
                p1 = -1;
                p2 = -1;
                ov[1] = -1;
                ov[2] = -1;

                d = *(BgDbg **)((char *)this + 0x103528);
                if (d != 0)
                {
                    if (d->f8 != 0)
                        ov[0] = d->f8[0];
                    if (d->fc != 0)
                    {
                        ov[1] = d->fc[0];
                        if (ov[1] == 1)
                            p1 = d->fc[0x84 / 4];
                    }
                    if (d->f10 != 0)
                    {
                        ov[2] = d->f10[0];
                        if (ov[2] == 1)
                            p2 = d->f10[0x84 / 4];
                    }
                }

                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0, "%s %d: [ %d, %d, %d]",
                          f20, f102118, ov[0], ov[1], ov[2]);
                drawTextf((KGraphics *)g_sub1b2f054, 0x140, 0x14, "(%p:%d, %p:%d, %p:%d)",
                          m_charts[0].head, m_charts[0].f4,
                          m_charts[1].head, p1,
                          m_charts[2].head, p2);
            }
        }
    }

    g_d36cb0 = saved;
}
