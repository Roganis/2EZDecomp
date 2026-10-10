#include "ez2acapp.h"

/* Obj4323C0::drawOne @0x4323c0 - integrated at 69.16%, a scheduler wall.

   The last piece of what was wip/smallleaves.cpp: its five other functions all
   reached 100% in src/ without the park file being deleted, which is the
   staleness the wip/README notes. This one stayed open. */

struct Quad4323C0Item
{
    char  _pad0[0x80];
    float w;                 /* 0x80 */
    float h;                 /* 0x84 */
};

struct Quad4323C0Elem
{
    Quad4323C0Item *item;    /* 0x00 */
    char            _pad4[0x374 - 4];
};

struct Obj4323C0
{
    char            _pad0[0x7c];
    int             count;   /* 0x7c */
    char            _pad80[0x130 - 0x80];
    Quad4323C0Elem  elems[1];/* 0x130, stride 0x374 */

    void drawOne(int i, int unused, float x, float y, float off, float frac,
                 int grey);
};

// FUNCTION: EZ2AC_UNPACKED 0x004323c0
void Obj4323C0::drawOne(int i, int, float x, float y, float off, float frac,
                        int grey)
{
    Quad4323C0Item *p = elems[(i + 5 * count) % count].item;
    unsigned int c;
    float h;
    float qx;
    float qw;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    h = p->h * frac;
    qx = x - off - 50.0f;
    qw = p->w;

    c = (unsigned int)grey;
    c = c << 8 | (unsigned int)grey;
    c = c << 8 | (unsigned int)grey;
    c = c << 8 | (unsigned int)grey;

    ((KGraphics *)g_sub1b2f054)->m404c50(
        (KTexture *)p, qx, (p->h - h) * 0.5f + y + 41.0f,
        qw, h, 0, 0, 1.0f, 1.0f, c, -1);
}
