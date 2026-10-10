#include "sound.h"
#include "ksongplayer.h"

/* Volume setter @0x0040f4a0. Writes a->0x10, then (if a->0x24 != 0) scales a
   volume through two /16129 (=127*127) divisions and dispatches the result-5000
   to a virtual method (vtable+0x20) on a->0x24. __thiscall. */

// FUNCTION: EZ2AC_UNPACKED 0x0040f4a0
void KEZPlayer::setVolume(SoundParam *a, int b)
{
    IVolTarget *p = a->field_0x24;
    a->field_0x10 = b;
    if (p == 0)
        return;

    int q1 = (f726c * a->field_0x1c * a->field_0x18) / 16129;
    int q2 = (q1 * b * 5000) / 16129;

    p->setLevel(q2 - 5000);
}

/* Pan setter @0x0040f550. Writes a->0x14, then (if a->0x24 != 0) adjusts b by a
   pan-position weighting, scales b*20000/127, and dispatches result-10000 to a
   virtual method (vtable+0x18). __thiscall (this unused). */

// FUNCTION: EZ2AC_UNPACKED 0x0040f550
void KEZPlayer::setPan(SoundParam *a, int b)
{
    IVolTarget *p = a->field_0x24;
    a->field_0x14 = b;
    if (p == 0)
        return;

    int diff = a->field_0x20 - 64;
    if (diff < 0)
        b += diff * b / 64;
    else if (diff > 0)
        b += (127 - b) * diff / 64;

    p->setPan(b * 20000 / 127 - 10000);
}

/* Rebind a channel to the sample for id `w`: a no-op when it is already
   bound to it; otherwise look the sample up (through the pass-through this
   still sitting in ecx), and if it exists swap it in - stopping the old
   target first when the event byte says 0x14 - and reset the cached volume
   and pan. Reports whether the channel ended up bound. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fbf0
int KEZPlayer::m40fbf0(SoundParam *a, int w, int b)
{
    IVolTarget *t;

    if (w == a->field_0x0c)
        return 1;
    t = m40f650(w);
    if (t == 0)
        return 0;
    if (a->field_0x24 != 0)
    {
        if (b == 0x14)
            a->field_0x24->vf05();
        a->field_0x24 = 0;
    }
    a->field_0x24 = t;
    a->field_0x0c = w;
    a->field_0x10 = -1;
    a->field_0x14 = -1;
    return 1;
}
