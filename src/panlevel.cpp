/* GFMainGameDirector::m45d5f0 @0x45d5f0 and CatchMainGameDirector::m46c0f0
   @0x46c0f0 - the GF and Catch copies of the pan-to-level mapper the pan
   stepper tail-calls. EZ2DJMainGameDirector::m420360 @0x420360 in
   src/maingameslots.cpp is the third, and the three differ in NOTHING but
   which two members hold the mixer controls (0x7e4/0x7e0, 0x778/0x774 and
   0x774/0x770), so this is a copy of that source with the class renamed.

   Each side of the pan is flat at its base level for the near half of the
   travel and ramps over the far half: the B control from 0x5938 up by
   11420/128 per step past centre, the A control from 0xaaaa up by 0x2b71/128
   per step. The two read the pan from opposite ends, which is what makes the
   pair a crossfade. */

#include "ezscreen.h"

/* src/mixer.cpp - one mixer line's level (setValue @0x412a10). */
struct MixerControl { void setValue(unsigned long value); };

// FUNCTION: EZ2AC_UNPACKED 0x0045d5f0
void GFMainGameDirector::m45d5f0(int level)
{
    int inv = 0x100 - level;
    int a;
    int b;

    if (inv <= 0x80)
    {
        a = 0x5938;
    }
    else
    {
        a = ((inv - 0x80) * 11420) / 128 + 0x5938;
    }

    if (level <= 0x80)
    {
        b = 0xaaaa;
    }
    else
    {
        b = ((level - 0x80) * 0x2b71) / 128 + 0xaaaa;
    }

    if (mixerB != 0)
    {
        mixerB->setValue(a);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(b);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046c0f0
void CatchMainGameDirector::m46c0f0(int level)
{
    int inv = 0x100 - level;
    int a;
    int b;

    if (inv <= 0x80)
    {
        a = 0x5938;
    }
    else
    {
        a = ((inv - 0x80) * 11420) / 128 + 0x5938;
    }

    if (level <= 0x80)
    {
        b = 0xaaaa;
    }
    else
    {
        b = ((level - 0x80) * 0x2b71) / 128 + 0xaaaa;
    }

    if (mixerB != 0)
    {
        mixerB->setValue(a);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(b);
    }
}
