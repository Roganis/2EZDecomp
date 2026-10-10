/* EZ2DJMainGameDirector::m4206e0 @0x4206e0 - the .gds JudgmentDelta section.

   Four floats in COOL, GOOD, MISS, FAIL order, read into +0x318, +0x314,
   +0x310 and +0x30c, and each logged back out. Two adjustments sit between:

   * modes 5 and 9 make the two POSITIVE deltas more generous - COOL by 0.05
     and GOOD by 0.02 - before anything else sees them;
   * with the flag at 0x1b2e89c set, the two NEGATIVE deltas are DOUBLED,
     from copies saved at +0x360 and +0x364 before any of this. That flag is
     the one SongSelectDirector::toggleOption @0x434f70 flips in the four
     course modes, so the course modes drain the gauge twice as fast on a
     miss or a fail. Worth knowing for the port - it is a scoring-adjacent
     behaviour that lives in the .gds reader rather than in the scorer.

   The COOL delta is also copied to +0x31c BEFORE the doubling, so whatever
   reads that gets the un-doubled value. */

#include "ezscreen.h"

extern int  g_modeIndex;    /* 0x1b2e548 (src/modename.cpp) */
extern int  g_1b2e89c;      /* src/gamestate.cpp */
extern int  readGdsInt(int *out, struct KParser *p);       /* 0x456270 */
extern int  readGdsFloat(float *out, struct KParser *p);   /* 0x456230 */
extern void Log(const char *fmt, ...);                     /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x004206e0
int EZ2DJMainGameDirector::m4206e0(KParser *p)
{
    readGdsFloat(&f318, p);
    readGdsFloat(&f314, p);
    readGdsFloat(&f310, p);
    readGdsFloat(&f30c, p);

    f360 = f310;
    f364 = f30c;

    if (g_modeIndex == 5 || g_modeIndex == 9)
    {
        f318 = f318 + 0.05f;
        f314 = f314 + 0.02f;
    }

    // STRING: EZ2AC_UNPACKED 0x0048f728
    Log("JUDGMENT_COOL = %3.2f", f318);
    // STRING: EZ2AC_UNPACKED 0x0048f710
    Log("JUDGMENT_GOOD = %3.2f", f314);

    f31c = f318;

    if (g_1b2e89c != 0)
    {
        f310 = f360 * 2.0f;
        f30c = f364 * 2.0f;
    }

    // STRING: EZ2AC_UNPACKED 0x0048f6f8
    Log("JUDGMENT_MISS = %3.2f", f310);
    // STRING: EZ2AC_UNPACKED 0x0048f6e0
    Log("JUDGMENT_FAIL = %3.2f", f30c);
    return 1;
}

/* GFMainGameDirector::m45d960 @0x45d960 - the same .gds section on the GF
   director, without the mode adjustment or the logging. The auto-play
   doubling of the two NEGATIVE deltas is here too, which is what makes it a
   rule of the game rather than a quirk of one mode - see PORT-DELTAS #15. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d960
int GFMainGameDirector::m45d960(KParser *p)
{
    readGdsFloat(&f30c, p);
    readGdsFloat(&f308, p);
    readGdsFloat(&f304, p);
    readGdsFloat(&f300, p);

    f324 = f304;
    f328 = f300;
    f310 = f30c;

    if (g_1b2e89c != 0)
    {
        f304 = f324 * 2.0f;
        f300 = f328 * 2.0f;
    }
    return 1;
}

/* CatchMainGameDirector::m46c2b0 @0x46c2b0 - and the Ez2Catch one, which
   does the same doubling AND ZEROES THE GOOD DELTA outright: in Ez2Catch a
   GOOD earns no gauge at all, whatever the .gds file says. That is the
   third mode's own rule and it is filed with the other two. */
// FUNCTION: EZ2AC_UNPACKED 0x0046c2b0
int CatchMainGameDirector::m46c2b0(KParser *p)
{
    readGdsFloat(&f324, p);
    readGdsFloat(&f320, p);
    readGdsFloat(&f31c, p);
    readGdsFloat(&f318, p);

    f328 = f324;
    f338 = f31c;
    f33c = f318;
    f320 = 0.0f;

    if (g_1b2e89c != 0)
    {
        f31c = f338 * 2.0f;
        f318 = f33c * 2.0f;
    }
    return 1;
}

/* EZ2DJMainGameDirector::m420640 @0x420640 - the .gds MeasureScale section,
   and it mostly throws the file's value away.

   An int and a float are read, and then the scale at +0x320 is FORCED TO 1.6
   unless the mode is one of 6, 7, 8, 9 or 12 - so outside the four course
   modes and mode 12 the .gds value never survives, which is why the field's
   default of 2.0 is never seen either.

   The global at 0x1b2eae0 gets the scale in the four course modes and the
   constant everywhere else - INCLUDING mode 12, which keeps its own file
   value in the field but still publishes 1.6. That asymmetry is in the two
   ladders and is deliberate enough to be worth reproducing rather than
   simplifying: the first ladder tests 6, 7, 8, 9, 12 and the second only
   6, 7, 8, 9. */

// GLOBAL: EZ2AC_UNPACKED 0x01b2eae0
float g_1b2eae0;

// FUNCTION: EZ2AC_UNPACKED 0x00420640
int EZ2DJMainGameDirector::m420640(KParser *p)
{
    float v;

    readGdsInt(&f2ec, p);
    readGdsFloat(&f320, p);

    /* The constant is only needed off the mode-6 path, and the original
       loads it after that first test - so it is assigned inside. */
    if (g_modeIndex != 6)
    {
        v = 1.6f;
        if (g_modeIndex != 7 && g_modeIndex != 8 &&
            g_modeIndex != 9 && g_modeIndex != 12)
            f320 = v;
    }

    if (g_modeIndex == 6 || g_modeIndex == 7 ||
        g_modeIndex == 8 || g_modeIndex == 9)
        v = f320;

    g_1b2eae0 = v;

    // STRING: EZ2AC_UNPACKED 0x0048f6c4
    Log("Measure Scale = %1.2f", f320);
    return 1;
}
