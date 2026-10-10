#ifndef GDSITEM_H
#define GDSITEM_H

#include "kparser.h"
#include "ez2acapp.h"    /* KTexture, KGraphics, Sprite */

/* One entry of the .gds item table loaded by Obj46F660::loadItem @0x46f660.
   Every field is named by the reader that fills it, in file order: an id, two
   float pairs, an int pair, two colours, then two brace-delimited groups of
   { float, colour }, a float pair, a texture, a colour, an int pair, a colour,
   a second texture, three floats and finally a semicolon-separated list of
   base names for a six-texture run. 0x104 bytes - which is the stride the
   owner's `lea 0x2fc(this, index*65*4)` proves. */
struct GdsItem
{
    int          f00;    /* 0x00 */
    float        f04;    /* 0x04 */
    float        f08;    /* 0x08 */
    float        f0c;    /* 0x0c */
    float        f10;    /* 0x10 */
    int          f14;    /* 0x14 */
    int          f18;    /* 0x18 */
    unsigned int f1c;    /* 0x1c */
    unsigned int f20;    /* 0x20 */
    int          f24;    /* 0x24 - always 0 */
    float        f28;    /* 0x28 */
    unsigned int f2c;    /* 0x2c */
    int          f30;    /* 0x30 - always 1 */
    float        f34;    /* 0x34 */
    unsigned int f38;    /* 0x38 */
    float        f3c;    /* 0x3c */
    float        f40;    /* 0x40 */
    KTexture    *f44;    /* 0x44 */
    unsigned int f48;    /* 0x48 */
    int          f4c;    /* 0x4c */
    int          f50;    /* 0x50 */
    unsigned int f54;    /* 0x54 */
    KTexture    *f58;    /* 0x58 */
    float        f5c;    /* 0x5c */
    float        f60;    /* 0x60 */
    float        f64;    /* 0x64 */
    KTexture    *f68[6]; /* 0x68 */
    char         _pad80[0xe8 - 0x80];
    /* The lane's scroll rate, and the track entry that feeds it. @0x427680
       fills both from the run's track list. */
    float        fe8;    /* 0xe8 */
    int          fec;    /* 0xec - swapped between two items by @0x429950 */
    float        ff0;    /* 0xf0 */
    float        ff4;    /* 0xf4 - the fade level @0x42a450 drives */
    int          ff8;    /* 0xf8 - 0 idle, 1 fading in, 2 fading out */
    int          ffc;    /* 0xfc - cleared by the note-run walkers */
    int          f100;   /* 0x100 - GFPanel's LongNoteBomb needs both it and ffc up */
};

/* The class that owns the table WAS this file's `Obj46F660`, an address-named
   placeholder kept because its only caller @0x472650 was unreconstructed. That
   caller is now matched and it is CatchPanel::LoadPVI, which calls loadItem
   with `this` - so Obj46F660 was CatchPanel all along, and the struct is gone.
   Three independent checks agree: the placeholder's f14 and f20 land on
   CatchPanel's `float f14` / `float f20`, and its table at 0x2fc is exactly the
   0x1248 bytes CatchPanel had as `int f2fc[0x492]` - 18 * 0x104, eighteen
   items, the same count Panel carries. `loadItem` now lives on CatchPanel
   (include/gamepanels.h). */

#endif /* GDSITEM_H */
