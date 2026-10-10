/* KGraphics::m406900 @0x406900 - one frame of the FLAG's wave: lock the
   11x11 vertex mesh createFlag built, and set every vertex's z to
   amplitude * sin(row - t*speed*6) * row / 11 - the wave sweeping the
   free edge harder than the pole. The clock is timeGetTime through the
   non-dllimport thunk, converted UNSIGNED (the 2^32 float fixup).

   Its own TU: the vertex-buffer model and createFlag live in
   src/kgraphics2.cpp. */
#include <math.h>
#include "ez2acapp.h"
#include "icom.h"

extern "C" unsigned long __stdcall timeGetTime(void);   /* the 0x4145f0 thunk */

// FUNCTION: EZ2AC_UNPACKED 0x00406900
void KGraphics::m406900(GfxFlag *f)
{
    float phase = (float)timeGetTime() * f->b;
    float w;
    void *bits;
    int   j, k;

    f->vb->Lock(1, &bits, 0);
    /* `j = 0` ahead of `w` and a `while`, not `for (j = 0; ...)` - both are
       needed (96.61% with either alone). Found by scratchpad/perm. */
    j = 0;
    w = phase * 6.0f;
    while (j < 11)
    {
        float rj = (float)j;
        float s = (float)sin((double)(rj - w));
        int   off = j * 0x20;

        for (k = 11; k != 0; k--)
        {
            *(float *)((char *)bits + off + 8) =
                f->a * s * rj * 0.0909090936f;
            off += 0x160;
        }

        j++;
    }
    f->vb->Unlock();
}
