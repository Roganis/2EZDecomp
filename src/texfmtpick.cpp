/* pickTextureFormat @0x004072c0 - the texture-format enumeration callback
   KGraphics::m407360 @0x407360 hands to IDirect3DDevice7::EnumTextureFormats.

   It picks ONE format per pass and stops: the pass asks for a bit depth and
   for alpha or no alpha, and the first uncompressed format that matches is
   copied whole into the caller's slot. 1 keeps the enumeration going, 0 ends
   it - D3DENUMRET_OK and D3DENUMRET_CANCEL. The rejections in the original's
   order: a null format or context, a non-zero FourCC (compressed formats
   never qualify), an alpha mask of exactly 0xf000 (the 4-bit-alpha 4444
   layout), the wrong alpha-ness for this pass, and the wrong bit count.

   THE SEVEN EXITS ARE TWO `||` CHAINS, NOT SEVEN `return`s. This was parked
   at 67.83% for want of that: written as separate `if (...) return 1;`
   statements cl gives each null check its own pop-and-return block and defers
   `push ebx` until after the first test, where the original pushes both saves
   in the prologue - the signature of the two null checks being ONE basic
   block, i.e. one `||`. Grouping them (and the four rejections after the Log
   as a second chain) gives 56 instructions against the original's 56 with the
   prologue, both tails and the interior entry point all right.

   The one instruction left is a scheduling coin: cl hoists `mov eax,1` above
   the FourCC branch, so that exit reaches the tail's pops-only entry, where
   the original leaves it below and lets the FourCC exit take the entry that
   materializes 1 for itself. Seeding the result into a variable between the
   two chains - the documented lever for a hoisted constant - does not move
   it, and neither did the four forms tried before this one (see git history
   for wip/texfmtpick.cpp).

   TexFmtPick is what @0x407360 puts on its stack and passes as the context;
   DdPixelFormat now lives in include/icom.h. */

#include <string.h>
#include "loader.h"   /* Log */


/* What @0x407360 puts on its stack and passes as the context. */
struct TexFmtPick
{
    int             wantAlpha;   /* 0x00 */
    unsigned int    bitCount;    /* 0x04 */
    int             found;       /* 0x08 */
    DdPixelFormat  *out;         /* 0x0c */
};

// FUNCTION: EZ2AC_UNPACKED 0x004072c0
int __stdcall pickTextureFormat(DdPixelFormat *pf, TexFmtPick *pick)
{
    /* The null guard through an int flag, and the accept path written as
       the body of `fourCC == 0` with the rejection after it: together they
       give the original's layout (neither alone does; scratchpad/perm). */
    int bad = pf == 0 || pick == 0;
    if (bad)
        return 1;

    // STRING: EZ2AC_UNPACKED 0x00487c88
    Log("Texture Format: %x %x %x %x %x %x %x",
        pf->flags, pf->fourCC, pf->bitCount, pf->rMask, pf->gMask,
        pf->bMask, pf->aMask);

    if (pf->fourCC == 0)
    {
        if (pf->aMask == 0xf000 ||
            (pick->wantAlpha == 1 && (pf->flags & 1) == 0) ||
            (pick->wantAlpha == 0 && (pf->flags & 1) != 0) ||
            pf->bitCount != pick->bitCount)
            return 1;

        memcpy(pick->out, pf, sizeof(DdPixelFormat));
        pick->found = 1;
        return 0;
    }
    return 1;
}
