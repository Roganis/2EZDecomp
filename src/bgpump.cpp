/* GFBackGround::m45c6a0 @0x45c6a0 and CatchBackGround::m46ae40 @0x46ae40 - the
   background script PUMP, one of kin.py's 1.000 pairs: the two are the same
   instruction sequence to the byte apart from three constants.

   Ten script slots are walked every call. A slot holds three words - the
   record it last played, the record it is waiting on, and the end of its list -
   and when the clock has reached that record's timestamp the pump retires the
   old one, arms the new one, and advances the cursor past it.

   THE RECORD'S SIZE DEPENDS ON ITS KIND, which is what the ladder is for:
   kinds 2 and 3 are 0x88 bytes, kinds 4 and 5 are 0xa4, and anything else is 8
   - the header alone. Running off the end of the list parks the slot at zero.

   The slot array is reached by CAST rather than a carved field. It starts four
   bytes BELOW the offset the loop walks from (the pump reads [esi-4]) and its
   ten 0x10-byte entries end exactly where the script bookkeeping begins, so
   carving it would have to restructure the placeholder int[40] the two classes
   already carry there. Not worth disturbing a modelled region for addressing
   that reads the same either way.

   The three constants: the walk base (0x102080 for GF, 0x102074 for catch),
   the clock field (0x102118 / 0x10210c), and which class owns the two helpers -
   GF reaches its first through BackGround's copy, exactly as the script loader
   reaches the parser hook. */

#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

// FUNCTION: EZ2AC_UNPACKED 0x0045c6a0
void GFBackGround::m45c6a0(int)
{
    int *s;
    int  i;

    i = 0;
    s = (int *)((char *)this + 0x102080);
    do
    {
        int *cur = (int *)s[0];

        if (cur != 0 && f102118 >= cur[0])
        {
            char *next;
            int   kind;

            ((BackGround *)this)->m41af50(i, s[-1]);
            m45c5f0(i, s[0]);
            s[-1] = s[0];

            next = (char *)s[0] + 8;
            kind = ((int *)s[0])[1];
            if (kind == 2 || kind == 3)
                next += 0x80;
            else if (kind == 4)
                next += 0x9c;
            else if (kind == 5)
                next += 0x9c;
            s[0] = (int)next;
            if ((unsigned int)next >= (unsigned int)s[1])
                s[0] = 0;
        }
        i++;
        s += 4;
    } while (i < 10);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ae40
void CatchBackGround::m46ae40(int)
{
    int *s;
    int  i;

    i = 0;
    s = (int *)((char *)this + 0x102074);
    do
    {
        int *cur = (int *)s[0];

        if (cur != 0 && f10210c >= cur[0])
        {
            char *next;
            int   kind;

            m46add0(i, s[-1]);
            m46ad20(i, s[0]);
            s[-1] = s[0];

            next = (char *)s[0] + 8;
            kind = ((int *)s[0])[1];
            if (kind == 2 || kind == 3)
                next += 0x80;
            else if (kind == 4)
                next += 0x9c;
            else if (kind == 5)
                next += 0x9c;
            s[0] = (int)next;
            if ((unsigned int)next >= (unsigned int)s[1])
                s[0] = 0;
        }
        i++;
        s += 4;
    } while (i < 10);
}

/* The two helpers the pump above drives, each a 1.000 pair across two classes.
   Both dispatch on the record's KIND at +4, and between them they are the
   whole background script language:

     kind 1  clear the slot's chart
     kind 2  attach the chart to the record's head, flag 0
     kind 3  the same with flag 1
     kind 4  set the flag object's vector from the record's two floats
     kind 5  build the worm mesh from the record's four values

   RETIRE is the smaller half - it only has to undo kinds 2, 3 and 5 - and ARM
   is the one that reads the record's payload. Kinds 2 and 3 differ by one
   pushed constant, which cl cross-jumps into a single call in both classes.

   These four also CONFIRMED setVec2 @0x406600 is a KGraphics member rather
   than the free function it was declared as: the kind 4 arm loads the graphics
   singleton into ecx before calling it, which is the tell scratchpad/ecxsweep.py
   flagged and could not settle without a reconstructed caller. */

/* The record the script is made of; the two float fields are read as ints here
   and cast at use, because kind 4 and kind 5 read different pairs of them. */
struct BgRec { int f0; int kind; int f8; int fc; int f10; int f14; };

// FUNCTION: EZ2AC_UNPACKED 0x0041af50
void BackGround::m41af50(int slot, int rec)
{
    BgRec *r = (BgRec *)rec;

    if (r == 0)
        return;
    if (r->kind == 2)
    {
        m_charts[slot].clear();
        return;
    }
    if (r->kind == 3)
    {
        m_charts[slot].clear();
        return;
    }
    if (r->kind == 5)
        ((KGraphics *)g_sub1b2f054)->freeBuffers(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046add0
void CatchBackGround::m46add0(int slot, int rec)
{
    BgRec *r = (BgRec *)rec;

    if (r == 0)
        return;
    if (r->kind == 2)
    {
        m_charts[slot].clear();
        return;
    }
    if (r->kind == 3)
    {
        m_charts[slot].clear();
        return;
    }
    if (r->kind == 5)
        ((KGraphics *)g_sub1b2f054)->freeBuffers(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c5f0
void GFBackGround::m45c5f0(int slot, int rec)
{
    BgRec *r = (BgRec *)rec;

    if (r->kind == 2)
    {
        m_charts[slot].attach((Obj402860Head *)r->f8, 0);
        return;
    }
    if (r->kind == 3)
    {
        m_charts[slot].attach((Obj402860Head *)r->f8, 1);
        return;
    }
    if (r->kind == 4)
    {
        ((KGraphics *)g_sub1b2f054)->setVec2((Vec2 *)&m_flag,
                                             *(float *)&r->f10,
                                             *(float *)&r->f14);
        return;
    }
    if (r->kind == 5)
    {
        ((KGraphics *)g_sub1b2f054)->createWorm(&m_mesh, r->f8,
                                                *(float *)&r->fc,
                                                *(float *)&r->f10, r->f14);
        return;
    }
    if (r->kind == 1)
        m_charts[slot].clear();
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ad20
void CatchBackGround::m46ad20(int slot, int rec)
{
    BgRec *r = (BgRec *)rec;

    if (r->kind == 2)
    {
        m_charts[slot].attach((Obj402860Head *)r->f8, 0);
        return;
    }
    if (r->kind == 3)
    {
        m_charts[slot].attach((Obj402860Head *)r->f8, 1);
        return;
    }
    if (r->kind == 4)
    {
        ((KGraphics *)g_sub1b2f054)->setVec2((Vec2 *)&m_flag,
                                             *(float *)&r->f10,
                                             *(float *)&r->f14);
        return;
    }
    if (r->kind == 5)
    {
        ((KGraphics *)g_sub1b2f054)->createWorm(&m_mesh, r->f8,
                                                *(float *)&r->fc,
                                                *(float *)&r->f10, r->f14);
        return;
    }
    if (r->kind == 1)
        m_charts[slot].clear();
}
