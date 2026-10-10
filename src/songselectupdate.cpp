/* SongSelectDirector::update @0x443380 - the song-select screen's per-frame
   draw, 4,672 bytes / 1,332 instructions / 221 calls. Decoded 2026-08-26 with
   scratchpad/decode.py (one line per call, pending pushes as arguments) over
   the raw listing, and the five direct jump tables read out of .text.

   THE SHAPE: Clear + doOp7(0), the shared header grid, then a NINE-PHASE
   switch on f_bfdb4 (1..9, phase 3 empty), four of whose arms contain their
   own 13-way switch on f78 - the director's mode - picking the page draw
   (m439f00 for the plain keys pages, drawWheel / drawWheelWithList for the
   wheel modes, m439dc0 for mode 10). Every phase funnels into doOp7(1) and
   returns; four arms carry their own copy of that tail, which is what the
   original's five doOp7(1) sites are.

   The recurring blocks, written out per phase as the original lays them:
   the (390,420) backdrop pair with the brightness byte splatted into RGB,
   the genre strip gated off the radio modes and 0xc, the course panel
   cluster (m434080 title / drawLevels / drawCourseRankTable / m4394e0, in a
   different order per phase), the slide-in offsets written into the readout
   grids' f214/f218 before their refresh, the preview volume ramp
   (level - 255) * 10000 / 300 through EzSoundObject slot 8, the per-player
   panel refreshes behind countListA/getListAF8, and the (-5,-5) 645x485
   f_bc454 fade quad with f_bfdb8 in the alpha byte.

   callcmp: 174 call sites for the original's 221, every callee present and
   16 of 21 counts exact. The whole shortfall is ONE mechanism: our cl
   cross-jumps the mode switches' byte-identical arms (call + jmp) into
   shared blocks, where the original keeps a block per case - 33 m439f00
   sites, 7 drawWheel, 7 of the m403940 pairs. Duplicating the arms' tails
   in the source does not stop it (cl merges the whole arm, leading call
   included), which is the cookbook's ICF signature: the original's arms
   almost certainly called DISTINCT per-page functions the linker folded
   onto 0x439f00 / drawWheel / drawWheelWithList / m439dc0. Unfolding them
   with per-arm `folded` aliases is the matching follow-up; behaviour is
   identical either way. */

#include "songselect.h"
#include "bigbase.h"
#include "ez2acapp.h"

extern int countListA();       /* 0x418310 (src/nodelists.cpp) */
extern int getListAF8();       /* 0x418340 (src/nodelists.cpp) */

extern int g_modeIndex;        /* 0x1b2e548 (src/modename.cpp) */
extern int g_1b2e89c;          /* src/gamestate.cpp - the auto-play/course toggle */
extern int g_1b2ea28;          /* src/globals.cpp */
extern int g_1b2eb6c;          /* src/ez2acapp.cpp - hit-percent mode */

/* The two device wrappers (src/com_wrappers.cpp), declared under their real
   class name so the calls link - the same local-mirror pattern as
   src/bgupdate.cpp. */
struct Obj40BB00
{
    void doOp7(int v);            /* 0x40a470 */
    void doOp10(unsigned int c);  /* 0x40a730 - device->Clear */
};

// FUNCTION: EZ2AC_UNPACKED 0x00443380
void SongSelectDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);

    if (f_bfdb4 != 8 && f_bfdb4 != 9)
    {
        m_bc890.m403940();
        if (g_1b2e89c != 0)
        {
            m_c0c04.m403940();
            if (m_c0c04.isDone())
                m_c0c04.seek(0x1e);
        }
        else if (m_c0c04.f8 != 0)
        {
            m_c0c04.reset();
        }
    }

    switch (f_bfdb4)
    {
    case 1:
        if (g_1b2eb6c != 0)
            break;
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
            drawGenreStrip();
        m_bc674.m403940();
        g_1b2ea28 = 0;
        ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
        return;

    case 4:
        if (f6c < 0x1e)
            f6c++;
        switch (f78)
        {
        case 1:
            pageMode1();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 2:
            pageMode2();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 3:
            pageMode3();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 6:
            drawWheel();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 7:
            drawWheelWithList();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 4:
            pageMode4();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 5:
            pageMode5();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 8:
            wheelMode8();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 9:
            wheelMode9();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 12:
            m439dc0();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 10:
            pageMode10();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 11:
            pageMode11();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 0:
            m439f00();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        }

        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0484, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0480, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
        {
            int r = f_bc57c;
            if (r >= 2 && r <= 0x12)
                m_beaf4.m403940();
            else if ((r >= 0 && r <= 1) || (r >= 0x13 && r <= 0x2e))
                m_bed10.m403940();
        }
        if (f_bfdb4 >= 2 &&
            g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
        {
            m434080(235.0f, 250.0f, f84, f_20c6d4);
            drawLevels();
            drawCourseRankTable(577.0f, 152.0f, f84, f_20c6d4, 1);
            m4394e0();
        }
        if (g_1b2eb6c == 0)
        {
            Obj402860 *o = &m_20cb10;
            Obj402860 *c;

            o->f214 = 511.0f - f_20c6c0;
            o->f218 = 0.0f;
            o->m403940();
            c = &m_c05a8;
            c->f214 = 833.0f - f_20c6c0;
            c->f218 = 317.0f;
            c->m403940();
            if (c->isDone())
                c->seek(0x10);
        }
        if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            m_20e898.m403940();
        if (g_1b2eb6c == 0)
            drawCounter();
        m_sndPreview->q8((f_20c6c8 - 0xff) * 10000 / 300);
        if (countListA() == 2 || (getListAF8() == 0 && countListA() == 1))
            m_bd3c0.m403940();
        if (countListA() == 2 || (getListAF8() == 1 && countListA() == 1))
            m_bd5dc.m403940();
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9)
        {
            if (g_modeIndex == 0xc)
                break;
            drawGenreStrip();
        }
        if (g_modeIndex == 0xc)
            break;
        if (m_bc674.isDone())
            break;
        m_bc674.m403940();
        ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
        return;

    case 5:
        if (f6c < 0x1e)
            f6c++;
        switch (f78)
        {
        case 1:
            pageMode1();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 2:
            pageMode2();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 3:
            pageMode3();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 4:
            pageMode4();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 5:
            pageMode5();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 10:
            pageMode10();
            if (getListAF8() == 1)
                m_be068.m403940();
            if (getListAF8() == 0)
                m_be284.m403940();
            break;
        case 11:
            pageMode11();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 0:
            m439f00();
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
            break;
        case 6: break;
        case 7: break;
        case 8: break;
        case 9: break;
        }

        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
            drawGenreStrip();
        if (g_1b2eb6c == 0)
        {
            Obj402860 *o = &m_20cb10;
            Obj402860 *c;
            float      y;

            o->f214 = 511.0f - f_20c6c0;
            o->f218 = 0.0f;
            o->m403940();
            y = 572.0f - f_20c6c0;
            c = &m_c05a8;
            c->f214 = 322.0f;
            c->f218 = y;
            c->m403940();
            if (c->isDone())
                c->seek(0x10);
        }
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0484, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0480, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        if (f_bfdb4 >= 2 &&
            g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
        {
            m4394e0();
            m434080(235.0f, 250.0f, f84, f_20c6d4);
            drawLevels();
            drawCourseRankTable(577.0f, 152.0f, f84, f_20c6d4, 1);
        }
        if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            m_20e898.m403940();
        if (g_1b2eb6c == 0)
            drawCounter();
        if (countListA() == 2 || (getListAF8() == 0 && countListA() == 1))
            m_bd3c0.m403940();
        if (countListA() == 2 || (getListAF8() == 1 && countListA() == 1))
        {
            m_bd5dc.m403940();
            ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
            return;
        }
        break;

    case 2:
        switch (f78)
        {
        case 1:  pageMode1();           break;
        case 2:  pageMode2();           break;
        case 7:  drawWheelWithList(); break;
        case 4:  pageMode4();           break;
        case 5:  pageMode5();           break;
        case 3:  pageMode3();           break;
        case 6:  drawWheel();         break;
        case 8:  wheelMode8();         break;
        case 9:  wheelMode9();         break;
        case 10: pageMode10();           break;
        case 11: pageMode11();           break;
        case 0:  m439f00();           break;
        case 12: m439dc0();           break;
        }
        if (f_bfdb4 >= 2)
        {
            if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
                g_modeIndex != 9 && g_modeIndex != 0xc)
            {
                m4394e0();
                drawCourseRankTable(577.0f, 152.0f, f84, f_20c6d4, 1);
                m434080(235.0f, 250.0f, f84, f_20c6d4);
            }
            if (g_modeIndex == 6)
                goto p2_backdrop;
        }
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
            drawGenreStrip();
p2_backdrop:
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0484, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0480, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            m_20e898.m403940();
        if (g_1b2eb6c == 0)
            drawCounter();
        if (g_1b2eb6c == 1)
        {
            if (countListA() != 2)
            {
                if (getListAF8() == 1)
                    m_be068.m403940();
                if (getListAF8() == 0)
                    m_be284.m403940();
            }
        }
        if (countListA() == 2 || (getListAF8() == 0 && countListA() == 1))
            m_bd3c0.m403940();
        if (countListA() == 2 || (getListAF8() == 1 && countListA() == 1))
            m_bd5dc.m403940();
        if (g_modeIndex == 0xc)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
            ((KGraphics *)g_sub1b2f054)->drawTex(f_bc454, -5.0f, -5.0f,
                645.0f, 485.0f, f_bfdb8 << 24 | 0xffffff);
            if (g_modeIndex == 0xc)
                break;
        }
        m_bc674.m403940();
        ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
        return;

    case 7:
        switch (f78)
        {
        case 1:  pageMode1(); m_c09e8.m403940(); break;
        case 2:  pageMode2(); m_c09e8.m403940(); break;
        case 7:  drawWheelWithList();          break;
        case 4:  pageMode4(); m_c09e8.m403940(); break;
        case 5:  pageMode5(); m_c09e8.m403940(); break;
        case 3:  pageMode3(); m_c09e8.m403940(); break;
        case 6:  drawWheel();                  break;
        case 8:  wheelMode8();                  break;
        case 9:  wheelMode9();                  break;
        case 10: pageMode10(); m_c09e8.m403940(); break;
        case 11: pageMode11(); m_c09e8.m403940(); break;
        case 0:  m439f00(); m_c09e8.m403940(); break;
        case 12: m439dc0();                    break;
        }
        if (g_modeIndex != 7 && g_modeIndex != 6 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
            drawLevels();
        if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            m_20e898.m403940();
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0484, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0480, 390.0f, 420.0f, 0.0f, 0.0f,
            f_c0488 << 24 | f_c0488 << 16 | f_c0488 << 8 | f_c0488);
        m_sndPreview->q8((f_bfdb8 - 0xff) * 10000 / 300);
        if (g_1b2eb6c != 0)
            m_bfd44->q8((f_bfdb8 - 0xff) * 10000 / 300);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        if (f_bfdb4 >= 2 && g_modeIndex != 7)
        {
            if (g_modeIndex == 6)
                goto p7_slide;
            if (g_modeIndex != 8 && g_modeIndex != 9 && g_modeIndex != 0xc)
            {
                m4394e0();
                drawCourseRankTable(577.0f, 152.0f, f84, f_20c6d4, 1);
                m434080(235.0f, 250.0f, f84, f_20c6d4);
            }
        }
        if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 0xc)
            drawGenreStrip();
p7_slide:
        if (g_1b2eb6c == 0)
        {
            Obj402860 *o = &m_20cb10;
            Obj402860 *c;
            float      y;

            o->f214 = 511.0f - f_20c6c0;
            o->f218 = 0.0f;
            o->m403940();
            y = 572.0f - f_20c6c0;
            c = &m_c05a8;
            c->f214 = 322.0f;
            c->f218 = y;
            c->m403940();
            if (c->isDone())
                c->seek(0x10);
        }
        if (g_1b2eb6c == 0)
            drawCounter();
        if (countListA() == 2 || (getListAF8() == 0 && countListA() == 1))
            m_bd3c0.m403940();
        if (countListA() == 2 || (getListAF8() == 1 && countListA() == 1))
            m_bd5dc.m403940();
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc454, -5.0f, -5.0f,
            645.0f, 485.0f, f_bfdb8 << 24 | 0xffffff);
        break;

    case 6:
        switch (f78)
        {
        case 1:  pageMode1();           break;
        case 2:  pageMode2();           break;
        case 7:  drawWheelWithList(); break;
        case 4:  pageMode4();           break;
        case 5:  pageMode5();           break;
        case 3:  pageMode3();           break;
        case 6:  drawWheel();         break;
        case 8:  wheelMode8();         break;
        case 9:  wheelMode9();         break;
        case 10: pageMode10();           break;
        case 11: pageMode11();           break;
        case 0:  m439f00();           break;
        case 12: m439dc0();           break;
        }
        if (f78 == 7 || f78 == 6 || f78 == 8 || f78 == 9)
            m_20e898.m403940();
        if (g_1b2eb6c == 0)
            drawCounter();
        if (countListA() == 2 || (getListAF8() == 0 && countListA() == 1))
            m_bd3c0.m403940();
        if (countListA() == 2 || (getListAF8() == 1 && countListA() == 1))
            m_bd5dc.m403940();
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc454, -5.0f, -5.0f,
            645.0f, 485.0f, f_bfdb8 << 24 | 0xffffff);
        break;

    case 8:
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd04, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd08, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd0c, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc454, -5.0f, -5.0f,
            645.0f, 485.0f, f_bfdb8 << 24 | 0xffffff);
        break;

    case 9:
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc454, -5.0f, -5.0f,
            645.0f, 485.0f, f_bfdb8 << 24 | 0xffffff);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd04, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd08, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bfd0c, 0.0f, 0.0f, 0.0f, 0.0f, -1);
        break;

    case 3:
        break;
    }

    ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
}
