/* SongSelectDirector::m4391b0 @0x4391b0 - the CV2 wheel page: a +-5
   window over the f80 entry table (positions in 100s, ring-wrapped into
   (-50*f80, 50*f80]), each visible entry drawn through the 0x4324b0
   sibling of drawOne at x=45 with the same banded brightness as m438d30
   (255 at the d==500 focus, (int)d capped 255 below it, 1155-(int)d
   fading past 900 - and the same UNINITIALIZED bright in the fractional
   499..501 band, reproduced faithfully). The y ramp is one chained
   d*0.390625 + 10 + 34 - 112 + 25.3. The tail re-seeds the version
   panel from the page row, eases the volume float f_20c640 (capped 96),
   draws the CV2 list bar and the rotating disc sprite, and publishes
   the selection: the entry key to g_selectedKey AND g_mixStyle, the
   trimmed name to g_songInfoName, the raw name to g_songShortName. */

#include "songselect.h"
#include "ez2acapp.h"

extern EzSubsystem *g_sub1b2f054;   /* 0x1b2f054 (src/ez2acapp.cpp) */
extern int  g_1b2ebf4;              /* src/gamestate.cpp */
extern int  g_selectedKey;          /* 0x1b2efd4 (src/globals.cpp) */
extern int  g_mixStyle;             /* 0x1b2ebc8 (src/panels.cpp) */
extern char g_songInfoName[64];     /* 0x1b2eb80 (src/panels.cpp) */
extern char g_songShortName[0x40];  /* 0x1b2f004 (src/catchpanelgds.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004391b0
void SongSelectDirector::m4391b0()
{
    int   i;
    int   j;
    int   bright;
    int   idx;
    float d;
    char *s;
    char *dst;
    char  c;

    /* A plain `for` over j = 0..f80-1, with the slot `i` and the position
       `j * 100` DERIVED from it: the original's guard is cl's inverted
       first test (`cmp [f80], 0`), it re-reads f80 at the loop head, and
       both -5 and 0 are set AFTER the guard - cl's induction variables,
       not source ones. The earlier do-while under `if (f80 > 0)` with its
       own pos/i counters cached f80 and set them before it (88.68%). */
    for (j = 0; j < f80; j++)
    {
        i = j - 5;
        {
            d = (float)(j * 100) - f_c4;
            if (d > (float)(f80 * 50))
                d -= (float)(f80 * 100);
            if ((float)(-f80 * 50) > d)
                d = (float)(f80 * 100) + d;
            /* `jb` after comiss: taken on unordered too, so these are
               the negated >= tests, not `<`. */
            if (!(d >= 85.0f))
                goto next;
            if (!(1200.0f >= d))
                goto next;
            idx = (i + f80 * 6) % f80;
            if (idx < 0)
                goto next;
            if (d == 500.0f)
            {
                bright = 0xff;
            }
            else if (d >= 501.0f)
            {
                if ((int)d >= 0x384)
                    bright = 0x483 - (int)d;
                else
                    bright = 0xff;
            }
            else if (499.0f >= d)
            {
                bright = (int)d;
                if (bright > 0xfa)
                    bright = 0xff;
            }
            m4324b0(i,
                    (int)*(float *)((char *)this + idx * 0x374 + 0x5e4d0),
                    45.0f, (d * 0.390625f + 10.0f) + 34.0f - 112.0f + 25.3f,
                    0.0f, 0.0f, bright);
        }
next:
        ;
    }

    m_bf05c.setCell(4, 0, *(int *)((char *)this + 0x12c + f84 * 0x374));
    if (m_bf05c.isDone() != 0)
        m_bf05c.seek(0x3d);

    f_20c640 = f_c4 * (1.0f / (float)g_1b2ebf4);
    if (f_20c640 >= 96.0f)
        f_20c640 = 96.0f;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bf058, 23.0f, 89.0f, 0.0f, 0.0f,
                                         0xffffff);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->m404250(f_20c0cc, 24.0f,
                                         f_20c640 * 2.65f + 90.0f,
                                         0.0f, 0.0f, 0.0f, 0xffffffff);

    g_selectedKey = *(int *)((char *)this + 0x5e62c + f84 * 0x374);
    f70 = (int)*(float *)((char *)this + 0x5e4d0 + f84 * 0x374);
    g_mixStyle = g_selectedKey;
    s = m430f50(f84);
    dst = g_songInfoName;
    do
    {
        c = *s;
        *dst = c;
        s++;
        dst++;
    } while (c != 0);
    s = (char *)this + 0x5e2c4 + f84 * 0x374;
    dst = g_songShortName;
    do
    {
        c = *s;
        *dst = c;
        s++;
        dst++;
    } while (c != 0);
}

extern Obj40C180 *g_bigInstance;    /* 0x1b2e710 (src/accessors.cpp) */
extern int g_modeIndex;             /* 0x1b2e548 (src/modename.cpp) */
struct ListNode;
extern ListNode *findListA(int key);   /* 0x418e30 */

/* The legacy RadioMix stage list @0x433980, run every tick while
   g_1b2e7e0 is set: keys 4/5 open the panel (f_dc) and reset its fade
   counter (f_e4); while open, the wheels past +-5 or keys 0xf/0x10 (P2
   0x17/0x18) step the course cursor f_d4 - P1's up-cap parks it at 14
   where P2's parks at 13, preserved as found - and the white keys
   confirm: reload the course through loadStageCourse and close. Closed,
   the fade counter runs up to 100. Mode 7 gates each side on its list
   entry; every other mode reads both sides unconditionally. */

// FUNCTION: EZ2AC_UNPACKED 0x00433980
void SongSelectDirector::m433980()
{
    int v;

    if (g_bigInstance->getState(4) == 2 || g_bigInstance->getState(5) == 2)
    {
        f_dc = 1;
        f_e4 = 0;
    }
    if (f_dc != 0)
    {
        if (g_modeIndex == 7)
        {
            if (findListA(0) != 0)
            {
                if (g_bigInstance->accum[0] > 5 ||
                    g_bigInstance->getState(0xf) == 2)
                {
                    v = f_d4;
                    if (v > 0xd)
                        f_d4 = 0xe;
                    else
                        f_d4 = v + 1;
                }
                if (g_bigInstance->accum[0] < -5 ||
                    g_bigInstance->getState(0x10) == 2)
                {
                    v = f_d4;
                    if (v <= 0)
                        f_d4 = 0;
                    else
                        f_d4 = v - 1;
                }
                if (g_bigInstance->getState(0xa) == 2 ||
                    g_bigInstance->getState(0xc) == 2 ||
                    g_bigInstance->getState(0xe) == 2)
                {
                    f_e0 = 1;
                    f_dc = 0;
                }
            }
            if (findListA(1) != 0)
            {
                if (g_bigInstance->accum[1] > 5 ||
                    g_bigInstance->getState(0x17) == 2)
                {
                    v = f_d4;
                    if (v > 0xd)
                        f_d4 = 0xd;
                    else
                        f_d4 = v + 1;
                }
                if (g_bigInstance->accum[1] < -5 ||
                    g_bigInstance->getState(0x18) == 2)
                {
                    v = f_d4;
                    if (v <= 0)
                        f_d4 = 0;
                    else
                        f_d4 = v - 1;
                }
                if (g_bigInstance->getState(0x12) == 2 ||
                    g_bigInstance->getState(0x14) == 2 ||
                    g_bigInstance->getState(0x16) == 2)
                {
                    f_e0 = 1;
                    f_dc = 0;
                }
            }
        }
        else
        {
            if (g_bigInstance->accum[0] > 5 ||
                g_bigInstance->getState(0xf) == 2)
            {
                v = f_d4;
                if (v > 0xd)
                    f_d4 = 0xe;
                else
                    f_d4 = v + 1;
            }
            if (g_bigInstance->accum[0] < -5 ||
                g_bigInstance->getState(0x10) == 2)
            {
                v = f_d4;
                if (v <= 0)
                    f_d4 = 0;
                else
                    f_d4 = v - 1;
            }
            if (g_bigInstance->accum[1] > 5 ||
                g_bigInstance->getState(0x17) == 2)
            {
                v = f_d4;
                if (v > 0xd)
                    f_d4 = 0xe;
                else
                    f_d4 = v + 1;
            }
            if (g_bigInstance->accum[1] < -5 ||
                g_bigInstance->getState(0x18) == 2)
            {
                v = f_d4;
                if (v <= 0)
                    f_d4 = 0;
                else
                    f_d4 = v - 1;
            }
            if (g_bigInstance->getState(0xa) == 2 ||
                g_bigInstance->getState(0xc) == 2 ||
                g_bigInstance->getState(0xe) == 2 ||
                g_bigInstance->getState(0x12) == 2 ||
                g_bigInstance->getState(0x14) == 2 ||
                g_bigInstance->getState(0x16) == 2)
            {
                f_e0 = 1;
                f_dc = 0;
            }
        }
    }
    if (f_e0 != 0)
    {
        loadStageCourse(f_d4);
        f_e0 = 0;
    }
    if (f_dc == 0)
    {
        f_e4 += 1;
        if (f_e4 >= 0x64)
            f_e4 = 0x64;
    }
}

/* drawOne's CV2 sibling @0x4324b0 - one wheel entry: the target-name
   panel pair (mask 0xbc5d4 under blend 9/6, panel 0xbc5d0 under 2/2) at
   the entry's centre, the entry texture itself at x-off-50, and the
   level as one or two 32x32 digit textures from f_c0320.

   The body is wip/songselect-banner-4324b0.cpp's (promoted 2026-10-01:
   60.24% against the 38.66% of the argument-home model it replaces). Its
   wall: three arms on y - `y <= 200` (inline, A), `y >= 300` (B), else
   (C, out of line) - draw the same thing, so cl merges them; in the
   original arms A and B additionally re-store x (`mov [x],edx` after the
   pushes) and C does not, and A and B differ only in whether x or w is
   loaded first. Twelve source forms tried (x assigned at top / in A,B /
   nowhere; the draws using x vs an inline ax - sx; the assignment before
   or after the draws) either merge the arms, split x into a temp plus a
   copy (frame 0x10 vs 0xc), or recompute ax - sx in x87 per arm. The
   original's frame: x @P+0x10, y @P+0x14, hh @P+0x18, w in idx's slot,
   t = y+38 in bright's slot (after bright is read), the dw*0.5 values in
   ay's slot. */

#define ARM \
    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6); \
    t = y + 40.0f - 2.0f; \
    c = bright; c = c << 8 | bright; c = c << 8 | bright; c = c << 8 | bright; \
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bc5d4, x, t, w, hh, c); \
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2); \
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bc5d0, x, t, w, hh, c); \
    ((KGraphics *)g_sub1b2f054)->drawTex(tex, x - 50.0f, y + 41.0f, w, hh, c);

// FUNCTION: EZ2AC_UNPACKED 0x004324b0
void SongSelectDirector::m4324b0(int idx, int level, float ax, float ay, float sx, float sy, int bright)
{
    KTexture *tex = *(KTexture **)((char *)this + 0x130 + ((idx + f80 * 5) % f80) * 0x374);
    KTexture *d;
    float x, y, w, hh, t;
    int c;
    int dw, dh;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    x = ax - sx;
    w = *(float *)((char *)tex + 0x80);
    hh = *(float *)((char *)tex + 0x84) * sy;
    y = (*(float *)((char *)tex + 0x84) - *(float *)((char *)tex + 0x84) * sy) * 0.5f + ay;
    if (y <= 200.0f)
    {
        x = ax - sx;
        ARM
    }
    else if (y >= 300.0f)
    {
        x = ax - sx;
        ARM
    }
    else
    {
        ARM
    }
    d = f_c0320[level % 10];
    dw = (int)(*(float *)((char *)d + 0x80) * sx);
    dh = (int)(*(float *)((char *)d + 0x84) * sy);
    if (level <= 9)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0320[level % 10], ax - (float)dw * 0.5f + 306.0f - 103.0f, ay + 19.0f - (float)dh * 0.5f + 40.0f, 32.0f, 32.0f, c);
    }
    else if (level >= 10)
    {
        float Y, X;
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        Y = ay + 19.0f - (float)dh * 0.5f + 40.0f;
        X = ax - (float)dw * 0.5f;
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0320[level / 10 % 10], X + 301.0f - 103.0f, Y, 32.0f, 32.0f, c);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(f_c0320[level % 10], X + 311.0f - 103.0f, Y, 32.0f, 32.0f, c);
    }
}
