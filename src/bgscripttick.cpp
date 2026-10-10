/* BackGround::m45c890 @0x45c890 and CatchBackGround::m46b4d0 @0x46b4d0 -
   one tick of the three payload charts belonging to the background script
   record currently in force.

   The two are byte-identical apart from three member offsets, which sit
   exactly 0xc apart between the two classes; the original's ICF left them
   unfolded for that reason.

   The record pointer is re-read from the object on EVERY iteration, not
   hoisted - the tween tick can fire a script action that swaps it - so the
   loop indexes through the member rather than through a local copy.

   Slot 0 is special: while the latch is set, slot 0 is ticked and nothing
   else, so the latch holds the sequence at that payload. Any other slot (or
   slot 0 once the latch has cleared) ticks and then, if its payload is in
   state 1 and its chart has run out, clears the latch and releases the hold.
   The `i != 0` test in that condition is redundant with the latch test above
   it - the original emits both. */

#include "panels.h"
#include "backgrounds.h"

typedef GFBackGround::BgScriptRec BgRec;

// FUNCTION: EZ2AC_UNPACKED 0x0045c890
void BackGround::m45c890()
{
    int i;

    if (f103534 == 0)
        return;

    for (i = 0; i < 3; i++)
    {
        int *e = (int *)((BgRec *)f103534)->payload[i];
        int st = *e;

        if (st != 1 && st != 2)
            continue;

        if (i == 0 && f102124 != 0)
        {
            m_charts[0].m403940();
            continue;
        }

        {
            Obj402860 *c = &m_charts[i];
            c->m403940();
            if (f102124 != 0 && *e == 1 && i != 0 && c->isDone())
                f102124 = 0;
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b4d0
void CatchBackGround::m46b4d0()
{
    int i;

    if (m_scriptScratch == 0)
        return;

    for (i = 0; i < 3; i++)
    {
        int *e = (int *)m_scriptScratch->payload[i];
        int st = *e;

        if (st != 1 && st != 2)
            continue;

        if (i == 0 && f102118 != 0)
        {
            m_charts[0].m403940();
            continue;
        }

        {
            Obj402860 *c = &m_charts[i];
            c->m403940();
            if (f102118 != 0 && *e == 1 && i != 0 && c->isDone())
                f102118 = 0;
        }
    }
}


/* Detach one latched background script's chart. The latch's state has to be
   1 or 2 - anything else, and a null latch, is left alone.

   attach(0, 0) looks like a no-op and is: the callee returns immediately on a
   null head. The original calls it anyway, which is why both zeros are pushed.
   Latch 0 is the one that holds the sequence (see the tick above), so clearing
   the cursor is skipped for it. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c840
void GFBackGround::m45c840(int i)
{
    BgScriptLatch *h = (BgScriptLatch *)m_scriptLatch[i];

    if (h != 0 && (h->state == 1 || h->state == 2))
    {
        if (i != 0)
        {
            f102124 = 0;
        }

        m_charts[i].attach(0, 0);
    }
}


/* CatchBackGround::m46b310 @0x46b310 - the same detach on the catch class,
   whose three members sit 0xc below the GF ones. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b310
void CatchBackGround::m46b310(int i)
{
    BgScriptLatch *h = (BgScriptLatch *)m_scriptLatch[i];

    if (h != 0 && (h->state == 1 || h->state == 2))
    {
        if (i != 0)
        {
            f102118 = 0;
        }

        m_charts[i].attach(0, 0);
    }
}


/* GFBackGround::m41b2f0 @0x41b2f0 - ARM one latch's chart, the counterpart
   of m45c840 above.

   State 1 attaches the latch's own cell table unheld, and then - for any
   latch but 0 whose +0x84 is set - raises the cursor flag that m45c840
   clears. State 2 attaches the same table HELD. Any other state, and a null
   latch, is left alone.

   Note it passes `h->table`, not `h` - the latch is a wrapper around the
   table, which is what m45c840's attach(0, 0) reads as too. */
// FUNCTION: EZ2AC_UNPACKED 0x0041b2f0
void GFBackGround::m41b2f0(int i)
{
    BgScriptLatch *h = (BgScriptLatch *)m_scriptLatch[i];

    if (h != 0)
    {
        if (h->state == 1)
        {
            m_charts[i].attach(h->table, 0);

            if (i != 0 && h->f84 != 0)
            {
                f102124 = 1;
            }
        }
        else if (h->state == 2)
        {
            m_charts[i].attach(h->table, 1);
        }
    }
}

/* CatchBackGround::m46b360 @0x46b360 - the catch copy, three members 0xc
   lower as usual. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b360
void CatchBackGround::m46b360(int i)
{
    BgScriptLatch *h = (BgScriptLatch *)m_scriptLatch[i];

    if (h != 0)
    {
        if (h->state == 1)
        {
            m_charts[i].attach(h->table, 0);

            if (i != 0 && h->f84 != 0)
            {
                f102118 = 1;
            }
        }
        else if (h->state == 2)
        {
            m_charts[i].attach(h->table, 1);
        }
    }
}
