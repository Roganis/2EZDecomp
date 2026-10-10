/* SongSelectDirector::toggleOption @0x434f70 - one of the three option
   toggles the song-select screen offers.

   A set lock at +0x20eb0c refuses the toggle outright. Otherwise: toggle 0
   flips the auto-play flag, but only in the four course modes; toggle 1
   flips FIX TARGETBAR and logs which way it went; toggle 2 flips the
   catch-judging flag. All three play the same confirm blip first, and every
   path returns zero. */

#include "songselect.h"

/* The sound object, viewed for the two slots this uses. */
struct SsSoundSlots
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03();
    virtual void s4(int a);   /* slot 4 -> +0x10 */
    virtual void s5();        /* slot 5 -> +0x14 */
};

extern int g_modeIndex;      /* 0x1b2e548 (src/modename.cpp) */
extern int g_catchJudging;   /* 0x1b2e58c (src/playerslot.cpp) */
extern int g_1b2e89c;        /* src/gamestate.cpp */
extern void Log(const char *fmt, ...);   /* 0x402880 */

// GLOBAL: EZ2AC_UNPACKED 0x01b2e568
int g_1b2e568;

// FUNCTION: EZ2AC_UNPACKED 0x00434f70
char SongSelectDirector::toggleOption(int which)
{
    if (f20eb0c[which] != 0)
        return 0;

    switch (which)
    {
    case 0:
        if (g_modeIndex == 6 || g_modeIndex == 7 ||
            g_modeIndex == 8 || g_modeIndex == 9)
        {
            ((SsSoundSlots *)m_sndToggle)->s5();
            ((SsSoundSlots *)m_sndToggle)->s4(0);
            g_1b2e89c = (g_1b2e89c == 0);
        }
        break;

    case 1:
        ((SsSoundSlots *)m_sndToggle)->s5();
        ((SsSoundSlots *)m_sndToggle)->s4(0);
        if (g_1b2e568 == 0)
        {
            g_1b2e568 = 1;
            // STRING: EZ2AC_UNPACKED 0x00491ce0
            Log("Enable FIX TARGETBAR");
            return 0;
        }
        g_1b2e568 = 0;
        // STRING: EZ2AC_UNPACKED 0x00491cc8
        Log("Disable FIX TARGETBAR");
        return 0;

    case 2:
        ((SsSoundSlots *)m_sndToggle)->s5();
        ((SsSoundSlots *)m_sndToggle)->s4(0);
        g_catchJudging = (g_catchJudging == 0);
        return 0;
    }

    return 0;
}
