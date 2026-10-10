/* KEZPlayer - see include/ksongplayer.h.

   THE CLASS NAMES ITSELF, through the audio loaders. KWavLoader's descriptor
   @0x4a7330 carries, in order, the sample class's name ("KEZPlayer"), its
   KRuntimeClass (0x4a84b0 = "KSongPlayer") and its factory (0x410440) - and
   0x410440 constructs THIS class. The runtime class this file's slot 0 returns
   @0x4a733c confirms it from the other side: its name string is "KEZPlayer"
   and its base is that same "KSongPlayer" record, whose own base is
   "KBasicObject" @0x4a50d4 - which is KBase. So the chain is

       KBasicObject -> KSongPlayer -> KEZPlayer

   and the file was called src/obj4102c0.cpp until the loaders were matched. */

#include "kcrt.h"   /* operator delete = free, for the deleting destructor */
#include "ksongplayer.h"
#include "sound.h"   /* SoundParam, SoundEvent */
#include <windows.h>
#include <string.h>

/* The last speed anything set, published for the rest of the engine. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e708
float g_1b2e708;

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */

extern KRuntimeClass classKEZPlayer;   /* 0x4a733c (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0040f320
KRuntimeClass *KEZPlayer::getRuntimeClass()
{
    return &classKEZPlayer;
}

/* Slot 5 shares its address with EzChart::getSound - both are `mov eax,[ecx+4];
   ret` and the original's linker folded them; src/ezchart.cpp sorts first and
   owns it. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f380 folded
int KEZPlayer::getA()
{
    return f04;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f390
int KEZPlayer::getB()
{
    return f08;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f3a0
int KEZPlayer::getC()
{
    return f0c;
}

/* The two CreateEventA calls and their failure logs are the KLocks' inlined
   constructors; the second player's vptr patch is Obj410100's. */
// FUNCTION: EZ2AC_UNPACKED 0x004102c0
KEZPlayer::KEZPlayer() : ev0(0, 1), ev1(1, 0)
{
    m410110();
}

/* Everything after the body is the four members going out of scope: the two
   KLocks close their handles inline, then the two players. */
// FUNCTION: EZ2AC_UNPACKED 0x00410390
KEZPlayer::~KEZPlayer()
{
    m4101c0();
}

// FUNCTION: EZ2AC_UNPACKED 0x004104a0
// KEZPlayer::`scalar deleting destructor'

// VTABLE: EZ2AC_UNPACKED 0x004887d0
struct KEZPlayer;

/* The pair that used to be src/accessors.cpp's Obj40F470, and the setter that
   ties them to the base player. The rate is 60000 divided by the bound
   sample's own divisor scaled by the speed - a milliseconds-per-something
   figure, which is what KSongPlayerBase::advance measures its position in. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f470
float KEZPlayer::getF7260()
{
    return f7260;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f490
int KEZPlayer::getI7274()
{
    return f7274;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f5c0
void KEZPlayer::setSpeed(float v)
{
    f7260 = v;
    player0.setRate(60000.0f / ((float)(((Counted *)f04)->f88 / 4) * v));
}

/* Arm everything and let both players run.

   The flag at +0x7274 makes it idempotent: on a second call only the gate and
   the two starts happen. The first call also clears the 0x2000-byte scratch
   block, seeds the speed from the sample's own base rate, pushes the derived
   rate into the base player exactly as setSpeed does, and tells the sample
   itself (slot 23) that it is going. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fa00
int KEZPlayer::play()
{
    if (f7274 == 0)
    {
        memset(m_scratch, 0, sizeof(m_scratch));
        f7278 = 0;
        f7258 = 1;
        f725c = 1;
        f7260 = ((Counted *)f04)->f8c;
        player0.setRate(60000.0f /
                        ((float)(((Counted *)f04)->f88 / 4) * f7260));
        ((Counted *)f04)->q23();
        m40f730();
    }

    ev1.set();
    f7274 = 1;
    player1.start();
    player0.start();
    return 1;
}

/* Let go. The gate is closed under the lock so a pump already inside the tick
   finishes first, and the state word goes to 2 rather than 0 - "stopped", not
   "never started". */
// FUNCTION: EZ2AC_UNPACKED 0x0040fad0
void KEZPlayer::stop()
{
    ev0.wait(-1);
    ev1.reset();
    f7274 = 2;
    ev0.set();
}

/* Apply one queued event to a channel. Eight kinds, and the original's DIRECT
   jump table says all eight had their own arm in the source even though 2, 3
   and 6 do nothing - written as a group they would have become the byte-index
   form instead. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fc50
void KEZPlayer::applyEvent(SoundParam *a, SoundEvent *ev)
{
    switch (ev->kind)
    {
    case 1:
        if (a->field_0x00 != 1)
        {
            m40fbf0(a, ev->v.n.w0e, ev->v.n.b12);
            setVolume(a, ev->v.n.b10);
            setPan(a, ev->v.n.b11);
            m40f520(a);
        }
        break;
    case 2:
        break;
    case 3:
        break;
    case 4:
        a->field_0x18 = ev->v.bvalue;
        a->field_0x10 = -1;
        break;
    case 5:
        setSpeed(ev->v.fvalue);
        break;
    case 6:
        break;
    case 7:
        g_1b2e708 = ev->v.fvalue;
        break;
    case 8:
        // STRING: EZ2AC_UNPACKED 0x004887c8
        Log("Stop");
        break;
    }
}

/* THE SAMPLE CREATION PAIR. Both were modelled anonymously in
   src/accessors.cpp before the header was grepped - the log line
   "Error in KEZPlayer::CreateSample()" is what names the class, and the pair
   array at +0x1210 ends exactly where m_scratch begins, which confirms it.

   createSample loads one slot through the object at +0x08 and records the
   result in the second word of that slot's pair; the loop really is written
   1..2 in the original, counter and all. */
/* The object at +0x08 is an EzSubsystem; its m40e520 @0x40e520 is the
   per-slot sample load (src/subsystemload.cpp). */

// FUNCTION: EZ2AC_UNPACKED 0x0040f7b0
int KEZPlayer::createSample(int slot, int sample)
{
    int i;

    for (i = 1; i < 2; i++)
    {
        int r = (int)((EzSubsystem *)f08)->m40e520((EzSample *)sample);

        if (r == 0)
            return 0;

        m_pairs[i + slot * 2] = r;
    }

    return 1;
}

/* And the walk: clear all 0x800 pairs, then for every .ezi sample the loader
   holds, record it and create it unless bit 0 of its +4 says it is ready.
   The scratch past the pairs is cleared on the way out. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f800
int KEZPlayer::createSamples()
{
    int *dst;
    int  i;

    memset(m_pairs, 0, sizeof(m_pairs));
    i = 0;
    dst = m_pairs;

    do
    {
        int s = (int)((EziLoader *)f04)->sampleTable[i];

        if (s != 0)
        {
            *dst = s;

            if ((((char *)s)[4] & 1) == 0)
            {
                if (createSample(i, s) == 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x004887a0
                    Log("Error in KEZPlayer::CreateSample()");
                    return 0;
                }
            }
        }

        i++;
        dst += 2;
    }
    while (i < 0x800);

    memset(m_scratch, 0, sizeof(m_scratch));
    return 1;
}
