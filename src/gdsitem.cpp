/* CatchPanel::loadItem - read one .gds item into the panel's own table. See
   include/gdsitem.h; the whole body is the .gds reader alphabet from
   src/engine.cpp applied field by field. */

#include <string.h>
#include "gamepanels.h"   /* CatchPanel, and gdsitem.h through it */

extern EzSubsystem *g_sub1b2f054;    /* the graphics singleton */

/* How many of the semicolon-separated name variants are allowed - the run is
   picked by skipping that many, so it selects the last one within the cap.
   Storage in src/modeselectctor.cpp. */
extern int g_1b2ea30;

// FUNCTION: EZ2AC_UNPACKED 0x0046f660
int CatchPanel::loadItem(KParser *p, int index)
{
    char buf[0x1000];
    GdsItem *e = &items[index];
    int n;
    int len;
    char *s;

    readGdsInt(&e->f00, p);
    readGdsFloat2(&e->f04, &e->f08, p);
    readGdsFloat2(&e->f0c, &e->f10, p);
    readGdsInt2(&e->f14, &e->f18, p);
    readGdsColor(&e->f1c, p);
    readGdsColor(&e->f20, p);

    if (index == 0)
    {
        f14 = e->f04;
    }

    if (f20 == 0.0f)
    {
        f20 = e->f0c;
    }

    e->f24 = 0;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f28, p);
    readGdsColor(&e->f2c, p);
    p->parseToken(7);

    e->f30 = 1;
    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsFloat(&e->f34, p);
    readGdsColor(&e->f38, p);
    p->parseToken(7);

    readGdsFloat2(&e->f3c, &e->f40, p);
    readGdsTexture(&e->f44, (KGraphics *)g_sub1b2f054, p);
    readGdsColor(&e->f48, p);
    readGdsInt2(&e->f4c, &e->f50, p);
    readGdsColor(&e->f54, p);
    readGdsTexture(&e->f58, (KGraphics *)g_sub1b2f054, p);
    readGdsFloat(&e->f5c, p);
    readGdsFloat(&e->f60, p);
    readGdsFloat(&e->f64, p);

    readGdsToken(buf, p);
    len = strlen(buf);
    buf[len + 1] = 0;

    n = 0;
    s = buf;

    if (*s != 0)
    {
        do
        {
            if (*s == ';')
            {
                *s = 0;
                n++;
            }

            s++;
        }
        while (*s != 0);
    }

    if (n >= g_1b2ea30)
    {
        n = g_1b2ea30;
    }

    s = buf;

    while (n != 0)
    {
        s += strlen(s) + 1;
        n--;
    }

    loadTexSeries(e->f68, 6, (KGraphics *)g_sub1b2f054, s);
    ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)e->f68[0]);
    e->ff0 = e->f0c * 0.5f + e->f04;
    return 1;
}
