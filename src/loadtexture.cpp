#include <string.h>
#include "ez2acapp.h"
#include "kparser.h"

extern int  hasExtension(const char *ext, char *name);   /* src/strutil.cpp */
extern void replaceExtension(char *name, char *ext);     /* 0x413000 */

/* KGraphics::loadTexture @0x408d00 - the texture cache, integrated from
   wip/loadtexture.cpp at 99.07%.

   Every texture in the game comes through here. It is keyed in the KProfile at
   +0x11c under the caller's key when one is given and under the file name
   otherwise: a hit hands back the existing payload and reports 1, a miss
   carves a new 0xa0-byte payload out of the profile, loads the image into it
   and reports whatever the loader said.

   WHICH FILE IS LOADED DEPENDS ON THE EXTENSION. A .bmp is redirected to the
   .abm of the same name when the flag at +0xd4 is set - which is the
   alternate-artwork switch - and anything that is not a .bmp goes through the
   "TGA" path. Once loaded the surface gets a zero source colour key, and its
   size is measured off the surface description and added to the running
   texture total.

   ONE INSTRUCTION OUT OF PLACE, and it is a position rather than a difference:
   the `desc.dwSize = 0x7c` store. The original emits it INSIDE the call
   sequence, between `push eax` (the `this` for GetSurfaceDesc) and the call
   itself; cl emits it before the vtable load. All 112 other instructions match,
   including the frame - 0x18c = alt[260] + desc(0x7c) + key(8) + cookie, which
   is how the buffer size was derived - both profile calls, the three-way
   extension dispatch, the colour key and the size arithmetic.

   The instruction SET is identical in every form tried; only the slot the
   dwSize store lands in moves, and cl sinks it as far as its own list
   scheduler wants. Nothing at source level pushes it past the second push. */
// FUNCTION: EZ2AC_UNPACKED 0x00408d00
int KGraphics::loadTexture(char *name, KTexture **out, char *key)
{
    DDSurfaceDesc2 desc;
    DDColorKey ck;
    char alt[260];
    KProfileNode *n;
    KTexture *t;
    int r;

    n = m_profile11c->find(key != 0 ? key : name);

    if (n != 0)
    {
        *out = (KTexture *)&n->value;
        return 1;
    }

    t = (KTexture *)&m_profile11c->create(key != 0 ? key : name, 0, 0xa0)->value;

    if (hasExtension(".bmp", name))
    {
        if (fd4 != 0)
        {
            strcpy(alt, name);
            // STRING: EZ2AC_UNPACKED 0x004876a4
            replaceExtension(alt, ".abm");
            r = loadBmpTexture(alt, t);
        }
        else
        {
            r = loadBmpTexture(name, t);
        }
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x004876a0
        r = loadBmpTexture("TGA", t);
    }

    ck.dwColorSpaceLowValue = 0;
    ck.dwColorSpaceHighValue = 0;
    t->surface->SetColorKey(8, &ck);
    desc.dwSize = 0x7c;
    t->f88 = 0;
    t->surface->GetSurfaceDesc(&desc);
    t->bytes = (desc.dwRGBBitCount >> 3) * desc.dwHeight * desc.dwWidth;
    m_texBytes += t->bytes;
    m_texCount++;
    *out = t;
    return r;
}

/* loadTexture's sibling for a texture with no file behind it: find `key` (or
   `name` when there is no key) in the cache, else create the node and give it
   a fresh 256x256 system-memory surface - flags 1 and 0 take the same size
   (two arms in the source, cross-jumped onto one call; any other flag leaves
   the slot alone) - copy the creation descriptor into the texture, read the
   real format back, and book the bytes. Always returns 1. The video player's
   load is the single caller. */
// FUNCTION: EZ2AC_UNPACKED 0x00409200
int KGraphics::findOrCreateTexture(char *name, KTexture **out, char *key, int flag)
{
    DDSurfaceDesc2 got;
    DDSurfaceDesc2 desc;
    KProfileNode *n;
    KTexture *t;

    n = m_profile11c->find(key != 0 ? key : name);
    if (n != 0)
    {
        *out = (KTexture *)&n->value;
        return 1;
    }

    t = (KTexture *)&m_profile11c->create(key != 0 ? key : name, 0, 0xa0)->value;
    if (flag == 1)
        t->surface = createSystemTextureSurface((DdSurfaceDesc2 *)&desc, 0x100, 0x100, 0);
    else if (flag == 0)
        t->surface = createSystemTextureSurface((DdSurfaceDesc2 *)&desc, 0x100, 0x100, 0);
    /* The same scheduler wall loadTexture above is parked on: the
       instruction set is identical (84 of 84) but cl sinks the dwSize store
       and the descriptor lea two slots later than the original's list
       scheduler did. Naming the surface is load-bearing (it hoists the
       [ebx] read above the struct copy); naming the descriptor pointer
       changes nothing. */
    IDirectDrawSurface7 *s = t->surface;
    t->desc = desc;
    t->f88 = 0;
    got.dwSize = 0x7c;
    s->GetSurfaceDesc(&got);
    t->bytes = (got.dwRGBBitCount >> 3) * got.dwHeight * got.dwWidth;
    m_texBytes += t->bytes;
    m_texCount++;
    *out = t;
    return 1;
}
