/* ModeSelectDirector::update @0x4491b0 - the mode wheel's 8.4KB draw
   tick, transcribed end to end from the listing (the map is in the
   stub's old comment, kept in git history). The screen is a cross-fade
   machine: five "pages" of mode labels (basic rows, stco columns,
   course columns, the casual pair, the CV2 special), each coloured by
   a channel triple (f93e4/f93f8/f940c[i]) ramping +-0x14/0x32 toward
   the ff/46/e1 caps, sliding on the eases f9420[i] (512 out, 1/6-decay
   in) against the column bases {64, 91, 214, 337, 460}. f95e4 is the
   screen state (0 intro, 2 fading, 3 confirmed - which skips straight
   to the shared tail - 5 the diff-style pick); f9628 the page phase;
   f18 the selected mode. Three original quirks are preserved: the dead
   countListA compare in phase 3's f18==10 arm, catch's 1P-only rule,
   and phase 4's blue cap storing 0xff where every other cap is 0xe1.
   The tail draws the mode-label chart, the countdown digits, the
   press-start prompts (picked by have >= need - enough credits), four
   snap-eases, the notice bar, and the confirmed hold chart. */

#include "ez2acapp.h"
#include "modeselect.h"
#include "bigbase.h"
extern EzSubsystem *g_sub1b2f054;   /* 0x1b2f054 */
extern Obj40C180 *g_bigInstance;   /* 0x1b2e710 (src/accessors.cpp) */
struct ListNode;
extern ListNode *findListA(int key);   /* 0x418e30 */
extern int countListA();               /* 0x418310 */
extern int getListAF8();               /* 0x418340 */

struct Obj40BB00
{
    void doOp10(unsigned int color);   /* 0x40a730 (src/com_wrappers.cpp) */
    void doOp7(int v);                 /* 0x40a470 (src/com_wrappers.cpp) */
};

// FUNCTION: EZ2AC_UNPACKED 0x004491b0
void ModeSelectDirector::update()
{
    unsigned int c;
    int r;
    int col;

    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    m_83b8.m40b710();
    m_2f58.m403940();
    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(bgGrad, 0.0f, 100.0f, 0.0f, 0.0f,
                                         0x96969696);
    if (f95e4 == 3)
        goto confirmed;                /* @0x44ad3a */

    m_1c5c.m403940();
    if (f9454 >= 10 && countListA() != 2)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(cateSpecialM, 471.0f, 403.0f,
                                             0.0f, 0.0f, 0xffffffff);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(cateSpecial, 471.0f, 403.0f,
                                             0.0f, 0.0f, 0x66666666);
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    if (f95e4 == 0)
    {
        /* Intro fade: the next row's channels held at zero, the backdrop
           scale eases down to 1.0, row 0 ramps in, the glow chart 0
           loops, and the three basic rows slide in from the right. */
        f93e4[3] = 0;
        f93f8[3] = 0;
        f940c[3] = 0;
        if (1.0f >= f9510)
            f9510 = 1.0f;
        else
            f9510 = f9510 - f9510 * 0.06666667f;

        f93e4[0] += 0x14;
        f93f8[0] += 0x14;
        f940c[0] += 0x14;
        if (f93e4[0] > 0xfe)
            f93e4[0] = 0xff;
        if (f93f8[0] > 0x45)
            f93f8[0] = 0x46;
        if (f940c[0] > 0xe0)
            f940c[0] = 0xe1;

        f9420[0] = f9420[0] - f9420[0] * 0.16666667f;
        if (0.01f >= f9420[0])
            f9420[0] = 0.01f;

        m_890c[3].reset();
        m_890c[0].m403940();
        if (m_890c[0].isDone() != 0)
            m_890c[0].seek(0x1e);

        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)f93f8[0];
        c = c << 8 | (unsigned int)f940c[0];
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[0],
            f9420[0] + 64.0f, 131.0f, 0.0f, 0.0f, c);

        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[1],
            f9420[0] + 64.0f, 212.0f, 0.0f, 0.0f, c);

        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[2],
            f9420[0] + 64.0f, 293.0f, 0.0f, 0.0f, c);

        m_890c[0].m403940();
        if (m_890c[0].isDone() != 0)
            m_890c[0].seek(0x1e);
        goto tail;                     /* @0x44ac55 */
    }

    switch (f9628 - 1)
    {
    case 0:
        /* Phase 1: the basic rows fade out left, the stco columns fade
           in; the f18 arm picks which column carries the full colour. */
        m_890c[0].reset();
        m_890c[1].m403940();
        if (m_890c[1].isDone() != 0)
            m_890c[1].seek(0x1e);

        f93e4[0] += -0x32;
        f93f8[0] += -0x32;
        f940c[0] += -0x32;
        if (f93e4[0] <= 0)
            f93e4[0] = 0;
        if (f93f8[0] <= 0)
            f93f8[0] = 0;
        if (f940c[0] <= 0)
            f940c[0] = 0;

        f93e4[1] += 0x14;
        f93f8[1] += 0x14;
        f940c[1] += 0x14;
        if (f93e4[1] > 0xfe)
            f93e4[1] = 0xff;
        if (f93f8[1] > 0x45)
            f93f8[1] = 0x46;
        if (f940c[1] > 0xe0)
            f940c[1] = 0xe1;

        f9420[0] = f9420[0] * 2.6f;
        if (f9420[0] >= 512.0f)
            f9420[0] = 512.0f;
        f9420[1] = f9420[1] - f9420[1] * 0.16666667f;
        if (0.01f >= f9420[1])
            f9420[1] = 0.01f;

        /* The three basic rows slide out, grey from channel 0. */
        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[0],
            64.0f - f9420[0], 131.0f, 0.0f, 0.0f, c);
        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[1],
            64.0f - f9420[0], 212.0f, 0.0f, 0.0f, c);
        r = f93e4[0];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[2],
            64.0f - f9420[0], 293.0f, 0.0f, 0.0f, c);

        /* The four stco columns at y 211, x {91,214,337,460} + f9420[1];
           the f18 arm's own column takes the packed triple, the rest the
           channel-0 grey; arms 2/3 skip the two-player gate. */
        switch (f18 - 2)
        {
        case 0:
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[1];
            c = c << 8 | (unsigned int)f940c[1];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[0],
                f9420[1] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[1],
                f9420[1] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            if (countListA() == 2)
                goto tail;
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[2],
                f9420[1] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[3],
                f9420[1] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            goto tail;
        case 1:
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[0],
                f9420[1] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[1];
            c = c << 8 | (unsigned int)f940c[1];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[1],
                f9420[1] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            if (countListA() == 2)
                goto tail;
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[2],
                f9420[1] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[3],
                f9420[1] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            goto tail;
        case 2:
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[0],
                f9420[1] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[1],
                f9420[1] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[1];
            c = c << 8 | (unsigned int)f940c[1];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[2],
                f9420[1] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[3],
                f9420[1] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            goto tail;
        case 3:
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[0],
                f9420[1] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[1],
                f9420[1] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[2],
                f9420[1] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[1];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[1];
            c = c << 8 | (unsigned int)f940c[1];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(stco[3],
                f9420[1] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            goto tail;
        }
        goto tail;
    case 1:
        /* Phase 2: stco out, course in; the previous columns fade as a
           rolled loop this time (two iterations with two players), and
           the f95e4 == 5 diff-style overlay draws at the end. */
        m_890c[1].reset();
        m_890c[2].m403940();
        if (m_890c[2].isDone() != 0)
            m_890c[2].seek(0x1e);

        f93e4[1] += -0x32;
        f93f8[1] += -0x32;
        f940c[1] += -0x32;
        if (f93e4[1] <= 0)
            f93e4[1] = 0;
        if (f93f8[1] <= 0)
            f93f8[1] = 0;
        if (f940c[1] <= 0)
            f940c[1] = 0;

        f93e4[2] += 0x14;
        f93f8[2] += 0x14;
        f940c[2] += 0x14;
        if (f93e4[2] > 0xfe)
            f93e4[2] = 0xff;
        if (f93f8[2] > 0x45)
            f93f8[2] = 0x46;
        if (f940c[2] > 0xe0)
            f940c[2] = 0xe1;

        f9420[1] = f9420[1] * 2.6f;
        if (f9420[1] >= 512.0f)
            f9420[1] = 512.0f;
        f9420[2] = f9420[2] - f9420[2] * 0.16666667f;
        if (0.01f >= f9420[2])
            f9420[2] = 0.01f;

        /* The stco columns slide out - a rolled loop, cut to two
           columns with two players. */
        if (countListA() != 2)
        {
            for (r = 0, col = 0; r < 0x1ec; r += 0x7b, col++)
            {
                int q = f93e4[1];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(stco[col],
                    (float)r + 91.0f - f9420[1], 211.0f, 0.0f, 0.0f, c);
            }
        }
        else
        {
            for (r = 0, col = 0; r < 0xf6; r += 0x7b, col++)
            {
                int q = f93e4[1];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(stco[col],
                    (float)r + 91.0f - f9420[1], 211.0f, 0.0f, 0.0f, c);
            }
        }

        /* The course columns, same per-arm packing as phase 1, on
           f18 = 6..9. */
        switch (f18 - 6)
        {
        case 0:
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[2];
            c = c << 8 | (unsigned int)f940c[2];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[0],
                f9420[2] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[1],
                f9420[2] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            if (countListA() == 2)
                goto phase2Tail;
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[2],
                f9420[2] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[3],
                f9420[2] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            break;
        case 1:
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[0],
                f9420[2] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[2];
            c = c << 8 | (unsigned int)f940c[2];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[1],
                f9420[2] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            if (countListA() == 2)
                goto phase2Tail;
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[2],
                f9420[2] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[3],
                f9420[2] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            break;
        case 2:
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[0],
                f9420[2] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[1],
                f9420[2] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[2];
            c = c << 8 | (unsigned int)f940c[2];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[2],
                f9420[2] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[3],
                f9420[2] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            break;
        case 3:
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[0],
                f9420[2] + 91.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[1],
                f9420[2] + 214.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[2],
                f9420[2] + 337.0f, 211.0f, 0.0f, 0.0f, c);
            r = f93e4[2];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[2];
            c = c << 8 | (unsigned int)f940c[2];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(course[3],
                f9420[2] + 460.0f, 211.0f, 0.0f, 0.0f, c);
            break;
        }
phase2Tail:
        /* The diff-style pick overlay while f95e4 == 5. */
        if (f95e4 == 5)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
            ((KGraphics *)g_sub1b2f054)->drawTex(black, -2.0f, 209.0f,
                f9588[3], 64.0f, 0xffffffff);
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(diffStyle[0], 32.0f,
                f9588[0] - 100.0f, f9588[1] + 256.0f, f9588[1] + 64.0f,
                0xffffffff);
            ((KGraphics *)g_sub1b2f054)->drawTex(diffStyle[1], 352.0f,
                f9588[0] - 100.0f, f9588[1] + 256.0f, f9588[1] + 64.0f,
                0xffffffff);
            ((KGraphics *)g_sub1b2f054)->drawTex(diffStyleCur,
                f9588[2] + 32.0f, f9588[0] - 42.0f, 0.0f, 0.0f,
                0xffffffff);
        }
        goto tail;
    case 2:
        /* Phase 3: course out, the casual pair (catch/scratch) in on
           f18 10/11. */
        f93e4[2] += -0x32;
        f93f8[2] += -0x32;
        f940c[2] += -0x32;
        if (f93e4[2] <= 0)
            f93e4[2] = 0;
        if (f93f8[2] <= 0)
            f93f8[2] = 0;
        if (f940c[2] <= 0)
            f940c[2] = 0;

        f93e4[3] += 0x14;
        f93f8[3] += 0x14;
        f940c[3] += 0x14;
        if (f93e4[3] > 0xfe)
            f93e4[3] = 0xff;
        if (f93f8[3] > 0x45)
            f93f8[3] = 0x46;
        if (f940c[3] > 0xe0)
            f940c[3] = 0xe1;

        /* Two zero stores the scheduler wedges into the ease. */
        f93e4[4] = 0;
        f93e4[0] = 0;
        f9420[2] = f9420[2] * 2.6f;
        if (f9420[2] >= 512.0f)
            f9420[2] = 512.0f;
        f9420[3] = f9420[3] - f9420[3] * 0.16666667f;
        if (0.01f >= f9420[3])
            f9420[3] = 0.01f;

        /* The course columns slide out - rolled, two with two players. */
        if (countListA() != 2)
        {
            for (r = 0, col = 0; r < 0x1ec; r += 0x7b, col++)
            {
                int q = f93e4[2];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(course[col],
                    (float)r + 91.0f - f9420[2], 211.0f, 0.0f, 0.0f, c);
            }
        }
        else
        {
            for (r = 0, col = 0; r < 0xf6; r += 0x7b, col++)
            {
                int q = f93e4[2];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(course[col],
                    (float)r + 91.0f - f9420[2], 211.0f, 0.0f, 0.0f, c);
            }
        }

        switch (f18)
        {
        case 10:
            /* Both arms identical: the original merged them and kept
               the compare - the dead `cmp eax,2` it carries. */
            if (countListA() == 2)
            {
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)f93f8[3];
                c = c << 8 | (unsigned int)f940c[3];
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualCatch,
                    f9420[3] + 64.0f, 161.0f, 0.0f, 0.0f, c);
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                    f9420[3] + 64.0f, 242.0f, 0.0f, 0.0f, c);
            }
            else
            {
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)f93f8[3];
                c = c << 8 | (unsigned int)f940c[3];
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualCatch,
                    f9420[3] + 64.0f, 161.0f, 0.0f, 0.0f, c);
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                    f9420[3] + 64.0f, 242.0f, 0.0f, 0.0f, c);
            }
            break;
        case 11:
            if (countListA() != 2)
            {
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualCatch,
                    f9420[3] + 64.0f, 161.0f, 0.0f, 0.0f, c);
                r = f93e4[3];
                c = (unsigned int)r;
                c = c << 8 | (unsigned int)f93f8[3];
                c = c << 8 | (unsigned int)f940c[3];
                c = c << 8 | (unsigned int)r;
                ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                    f9420[3] + 64.0f, 242.0f, 0.0f, 0.0f, c);
                }
                else
                {
                    /* Catch is 1P-only: with two players only scratch shows,
                       carrying the full colour. */
                    r = f93e4[3];
                    c = (unsigned int)r;
                    c = c << 8 | (unsigned int)f93f8[3];
                    c = c << 8 | (unsigned int)f940c[3];
                    c = c << 8 | (unsigned int)r;
                    ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                        f9420[3] + 64.0f, 242.0f, 0.0f, 0.0f, c);
                }
            break;
        }
        /* Close: glow chart 2 reset, chart 3 looping - the same text as
           state 0's close; the original cross-jumps into it. */
        m_890c[2].reset();
        m_890c[3].m403940();
        if (m_890c[3].isDone() != 0)
            m_890c[3].seek(0x1e);
        goto tail;
    case 3:
        /* Phase 4: the casual pair out, the CV2 special panel in. The
           blue cap of triple 4 stores 0xff, not 0xe1 - an original
           quirk, preserved. */
        f93e4[3] += -0x32;
        f93f8[3] += -0x32;
        f940c[3] += -0x32;
        if (f93e4[3] <= 0)
            f93e4[3] = 0;
        if (f93f8[3] <= 0)
            f93f8[3] = 0;
        if (f940c[3] <= 0)
            f940c[3] = 0;

        f93e4[4] += 0x14;
        f93f8[4] += 0x14;
        f940c[4] += 0x14;
        if (f93e4[4] > 0xfe)
            f93e4[4] = 0xff;
        if (f93f8[4] > 0x45)
            f93f8[4] = 0x46;
        if (f940c[4] > 0xe0)
            f940c[4] = 0xff;   /* sic - the quirk */

        /* The casual columns leave on a COMBINED ease, and slot 0's
           ease is re-armed to 512; a zero store to channel 0 rides
           the schedule. */
        f93e4[0] = 0;
        f9420[3] = f9420[2] * 1.6f + f9420[3];
        if (f9420[3] >= 512.0f)
            f9420[3] = 512.0f;
        f9420[4] = f9420[4] - f9420[4] * 0.16666667f;
        if (0.01f >= f9420[4])
            f9420[4] = 0.01f;
        f9420[0] = 512.0f;

        /* BEHAVIOUR: with two players catch is hidden and scratch leaves
           from catch's row, y=161 - not 242 (the original pushes 161.0 on
           that path, into the shared scratch draw). */
        if (countListA() != 2)
        {
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualCatch,
                64.0f - f9420[3], 161.0f, 0.0f, 0.0f, c);
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                64.0f - f9420[3], 242.0f, 0.0f, 0.0f, c);
        }
        else
        {
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                64.0f - f9420[3], 161.0f, 0.0f, 0.0f, c);
        }

        r = f93e4[4];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)f93f8[4];
        c = c << 8 | (unsigned int)f940c[4];
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(specialCv2,
            f9420[4] + 64.0f, 208.0f, 0.0f, 0.0f, c);

        m_890c[3].reset();
        m_890c[4].m403940();
        if (m_890c[4].isDone() != 0)
            m_890c[4].seek(0x1e);
        goto tail;

    default:
        /* The return-to-start phase (f9628 == 0 wraps here through the
           unsigned dispatch): everything departing, the basic rows
           returning with the f18-selected row carrying the colour. */
        f93e4[4] += -0x32;
        f93f8[4] += -0x32;
        f940c[4] += -0x32;
        f93e4[1] = 0;
        f93f8[1] = 0;
        f940c[1] = 0;
        f93e4[2] = 0;
        f93f8[2] = 0;
        f940c[2] = 0;
        if (f93e4[4] <= 0)
            f93e4[4] = 0;
        if (f93f8[4] <= 0)
            f93f8[4] = 0;
        if (f940c[4] <= 0)
            f940c[4] = 0;

        f93e4[3] += -0x32;
        f93f8[3] += -0x32;
        f940c[3] += -0x32;
        if (f93e4[3] <= 0)
            f93e4[3] = 0;
        if (f93f8[3] <= 0)
            f93f8[3] = 0;
        if (f940c[3] <= 0)
            f940c[3] = 0;

        f93e4[0] += 0x14;
        f93f8[0] += 0x14;
        f940c[0] += 0x14;
        if (f93e4[0] > 0xfe)
            f93e4[0] = 0xff;
        if (f93f8[0] > 0x45)
            f93f8[0] = 0x46;
        if (f940c[0] > 0xe0)
            f940c[0] = 0xe1;

        f9420[4] = f9420[4] * 2.6f;
        if (f9420[4] >= 512.0f)
            f9420[4] = 512.0f;
        f9420[3] = f9420[3] * 2.6f;
        if (f9420[3] >= 512.0f)
            f9420[3] = 512.0f;
        f9420[0] = f9420[0] - f9420[0] * 0.16666667f;
        if (0.01f >= f9420[0])
            f9420[0] = 0.01f;

        m_890c[4].reset();
        m_890c[0].m403940();
        if (m_890c[0].isDone() != 0)
            m_890c[0].seek(0x1e);

        /* The casual pair leaves left; catch only without two players. */
        /* BEHAVIOUR: with two players scratch alone leaves from y=131 here
           (the original's 0x43030000), not 242. */
        if (countListA() != 2)
        {
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualCatch,
                64.0f - f9420[3], 161.0f, 0.0f, 0.0f, c);
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                64.0f - f9420[3], 242.0f, 0.0f, 0.0f, c);
        }
        else
        {
            r = f93e4[3];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(casualScratch,
                64.0f - f9420[3], 131.0f, 0.0f, 0.0f, c);
        }

        /* The basic rows return at f9420[0] + 64; the f18-selected row
           carries the packed triple. A SWITCH, arms in the order 2, 1, 0:
           the original dispatches with cl's sub/je ladder and lays case 2
           inline, then 1, then 0 (an if/else-if chain lays them 0, 1, 2). */
        switch (f18)
        {
        case 2:
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[0],
                f9420[0] + 64.0f, 131.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[1],
                f9420[0] + 64.0f, 212.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[0];
            c = c << 8 | (unsigned int)f940c[0];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[2],
                f9420[0] + 64.0f, 293.0f, 0.0f, 0.0f, c);
            break;
        case 1:
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[0],
                f9420[0] + 64.0f, 131.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[0];
            c = c << 8 | (unsigned int)f940c[0];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[1],
                f9420[0] + 64.0f, 212.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[2],
                f9420[0] + 64.0f, 293.0f, 0.0f, 0.0f, c);
            break;
        case 0:
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)f93f8[0];
            c = c << 8 | (unsigned int)f940c[0];
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[0],
                f9420[0] + 64.0f, 131.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[1],
                f9420[0] + 64.0f, 212.0f, 0.0f, 0.0f, c);
            r = f93e4[0];
            c = (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            c = c << 8 | (unsigned int)r;
            ((KGraphics *)g_sub1b2f054)->drawTex(basicMode[2],
                f9420[0] + 64.0f, 293.0f, 0.0f, 0.0f, c);
            break;
        }

        /* The CV2 special leaves right, then the stco columns leave. */
        r = f93e4[4];
        c = (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        c = c << 8 | (unsigned int)r;
        ((KGraphics *)g_sub1b2f054)->drawTex(specialCv2,
            f9420[3] + 64.0f, 208.0f, 0.0f, 0.0f, c);

        f93e4[1] += -0x32;
        f93f8[1] += -0x32;
        f940c[1] += -0x32;
        if (f93e4[1] <= 0)
            f93e4[1] = 0;
        if (f93f8[1] <= 0)
            f93f8[1] = 0;
        if (f940c[1] <= 0)
            f940c[1] = 0;
        f9420[1] = f9420[1] * 2.6f;
        if (f9420[1] >= 512.0f)
            f9420[1] = 512.0f;
        if (countListA() != 2)
        {
            for (r = 0, col = 0; r < 0x1ec; r += 0x7b, col++)
            {
                int q = f93e4[1];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(stco[col],
                    (float)r + 91.0f - f9420[1], 211.0f, 0.0f, 0.0f, c);
            }
        }
        else
        {
            for (r = 0, col = 0; r < 0xf6; r += 0x7b, col++)
            {
                int q = f93e4[1];
                c = (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                c = c << 8 | (unsigned int)q;
                ((KGraphics *)g_sub1b2f054)->drawTex(stco[col],
                    (float)r + 91.0f - f9420[1], 211.0f, 0.0f, 0.0f, c);
            }
        }
        break;
    }

tail:
    /* The shared tail @0x44ac55: the per-mode label chart out of the
       36-array (side variant with one player, the "both" variant
       otherwise), the countdown digits, and from L_44ad3a - which is
       ALSO where f95e4 == 3 joins from the entry - the press-start
       prompts, the player charts, the fade overlay, the mode-cursor
       eases, the notice bar, and the mode-12/basic hold charts. */
    if (f18 >= 0 && f18 <= 0xb)
    {
        if (countListA() == 1)
            m_37c8[f18 * 3 + getListAF8()].m403940();
        else
            m_37c8[f18 * 3 + 2].m403940();
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->drawTex(timeFont[f95d8 / 10 % 10],
        569.0f, 20.0f, 32.0f, 32.0f, 0xffffffff);
    ((KGraphics *)g_sub1b2f054)->drawTex(timeFont[f95d8 % 10],
        591.0f, 20.0f, 32.0f, 32.0f, 0xffffffff);

confirmed:
    /* @0x44ad3a. The press-start prompt: pick the P1/P2 arrow chart by
       which side joined, blinking off the input board's two counters. */
    if (f18 >= 0 && f18 <= 6 && countListA() == 1)
    {
        if (getListAF8() == 1)
        {
            if (g_bigInstance->have >= g_bigInstance->need)
                m_24cc.m403940();
            else
                m_2094.m403940();
        }
        else if (getListAF8() == 0)
        {
            if (g_bigInstance->have >= g_bigInstance->need)
                m_26e8.m403940();
            else
                m_22b0.m403940();
        }
    }
    if (findListA(0) != 0)
        m_3174.m403940();
    if (findListA(1) != 0)
        m_3390.m403940();

    if (f95e4 == 0 || f95e4 == 2 || f95e4 == 3)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(fadeBlack, -5.0f, -5.0f,
            645.0f, 485.0f, f95e8 << 0x18 | 0xffffff);
    }

    /* f30[2] = the selected mode's index within its page. */
    if (countListA() == 2)
    {
        if (f9628 == 0)
            f30[2] = f18;
        else if (f9628 == 1)
        {
            /* two stores, not a named r: the original's `add ecx,-3` */
            if (f18 >= 4)
                f30[2] = f18 - 3;
            else
                f30[2] = f18;
        }
        else if (f9628 == 2)
            f30[2] = f18 - 8;
    }
    else
    {
        if (f9628 == 0)
            f30[2] = f18;
        else if (f9628 == 1)
            f30[2] = f18;
        else if (f9628 == 2)
            f30[2] = f18 - 8;
    }

    /* The cursor strip chases f38 * 28 - 40, or f38 * 62 with the wheel
       on the first page, snapping inside +-0.5, and seeks the strip chart
       to frame 0 while it moves. INERT in the shipped game: f9438 is read
       nowhere else, and m_1e78 is only ever seek(0)'d - the ctor @0x44b8f0
       never attaches it - so the -40 the old C had on page 1 changed a
       value nothing draws (found by the port session, 2026-10-09). */
    {
        float target;
        float k, b;
        float d;

        /* NOT `- 40` on both pages: the original subtracts 0.0 (an
           xorps) when f9628 == 0 and 40.0 only with the 28 multiplier -
           both constants picked by one branch, then one mulss/subss. */
        b = 0.0f;
        if (f9628 != 0)
        {
            b = 40.0f;
            k = 28.0f;
        }
        else
            k = 62.0f;
        target = (float)f30[2] * k - b;
        d = target - f9438;
        if (0.5 > d && d > -0.5)
            f9438 = target;
        else
        {
            f9438 = d * 0.33333334f + f9438;
            m_1e78.seek(0);
        }
    }
    {
        float target;
        float d;

        target = (float)f95f4 * 320.0f;
        d = target - f9588[2];
        if (0.5 > d && d > -0.5)
            f9588[2] = target;
        else
            f9588[2] = d * 0.33333334f + f9588[2];
    }
    if (f95e4 == 5)
        f9598 = 1;
    else
        f9598 = 0;
    {
        float ta;
        float tb;
        float d;

        /* tb first: the original's xmm order for the 645/302 pair */
        tb = (float)f9598 * 645.0f;
        ta = (float)f9598 * 302.0f;
        d = ta - f9588[0];
        if (0.5 > d && d > -0.5)
            f9588[0] = ta;
        else
            f9588[0] = d * 0.33333334f + f9588[0];
        d = tb - f9588[3];
        if (0.5 > d && d > -0.5)
            f9588[3] = tb;
        else
            f9588[3] = d * 0.33333334f + f9588[3];
    }

    /* The notice bar: colour = 255 - f9448 in every channel; the alpha
       counter shrinks by a tenth (floor 1) while f_e0 holds, else grows
       by a sixth (cap 255). */
    c = (unsigned int)(-1 - f9448) << 0x18 |
        (unsigned int)(0xff - f9448) << 0x10 |
        (unsigned int)(0xff - f9448) << 8 |
        (unsigned int)(0xff - f9448);
    if (fe0 == 1)
    {
        r = f9448 - f9448 / 10;
        f9448 = r;
        if (r <= 1)
            f9448 = 1;
    }
    else
    {
        r = f9448 + f9448 / 6;
        f9448 = r;
        if (r >= 0xff)
            f9448 = 0xff;
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(noticeShadow, 0.0f, 0.0f,
        520.1f - f9444, 482.0f, 0xffffffff);
    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(noticeM, f9444 + 126.0f, 0.0f,
        0.0f, 0.0f, c);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->drawTex(notice, f9444 + 126.0f, 0.0f,
        0.0f, 0.0f, c);

    /* The confirmed-mode hold chart: mode 12 uses m_2d3c, the rest
       m_1a40; held on its last frame once done. */
    if (f18 != 0xc)
    {
        if (f95e4 == 3)
        {
            if (m_1a40.isDone() != 0)
                m_1a40.seek(m_1a40.f8 - 1);
            m_1a40.m403940();
        }
    }
    else if (f95e4 == 3)
    {
        if (m_2d3c.isDone() != 0)
            m_2d3c.seek(m_2d3c.f8 - 1);
        m_2d3c.m403940();
    }

    if (f95e4 == 0 || f95e4 == 2)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(6, 5);
        ((KGraphics *)g_sub1b2f054)->drawTex(fadeBlack, -5.0f, -5.0f,
            645.0f, 485.0f, f95e8 << 0x18 | 0xffffff);
    }
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(1);
}
