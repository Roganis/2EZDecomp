/* The settings reader. Its own TU because EZ2AC::v4 @0x4181b0 (src/ez2acapp.cpp)
   calls it, EZ2AC::loadCounters @0x4156c0 (src/moderunner.cpp) is called from it,
   and the two callees at the end must stay unseen so the tail `jmp` survives. */

#include "ez2acapp.h"
#include "kprofile.h"
#include "kengineapp.h"   /* the three profiles */
#include "loader.h"       /* Log */
#include "settings.h"

/* Every one of these is named by the .ini key readSettings reads it from - the
   function IS the naming evidence. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x04a8d74
int g_gameLevel;
// GLOBAL: EZ2AC_UNPACKED 0x016963f8
int g_advSound;
// GLOBAL: EZ2AC_UNPACKED 0x01696404
int g_eventMode;
// GLOBAL: EZ2AC_UNPACKED 0x01696408
int g_eventModeMixName;
// GLOBAL: EZ2AC_UNPACKED 0x0169640c
int g_eventModeSongNameIdx;
// GLOBAL: EZ2AC_UNPACKED 0x01696330
char g_eventModeSongName[0x40];
// GLOBAL: EZ2AC_UNPACKED 0x01696420
int g_onePlayerOnly;
// GLOBAL: EZ2AC_UNPACKED 0x01696424
int g_autoScratchL;
// GLOBAL: EZ2AC_UNPACKED 0x01696428
int g_autoScratchR;
// GLOBAL: EZ2AC_UNPACKED 0x0169642c
int g_autoPedal;
// GLOBAL: EZ2AC_UNPACKED 0x016963c8
int g_restartSystem;
// GLOBAL: EZ2AC_UNPACKED 0x016963f0
int g_remainPatch;
// GLOBAL: EZ2AC_UNPACKED 0x016963d8
int g_initializePatch;
// GLOBAL: EZ2AC_UNPACKED 0x016963d0
int g_allRankingFileDelete;
// GLOBAL: EZ2AC_UNPACKED 0x016963b4
int g_delTurntableRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963b0
int g_delStreetMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963c0
int g_delSpaceMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963d4
int g_delRubyMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x01696328
int g_delEz2CatchRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963b8
int g_delClubMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963e0
int g_del7StreetMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963ec
int g_del5KeyMixRanking;
// GLOBAL: EZ2AC_UNPACKED 0x016963bc
int g_gameVersion;


/* Gates the last hand-off. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x00530b70
int g_530b70;

/* Pull the whole configuration in: the machine settings from EZ2AC.ini, the
   maintenance switches from patch.ini, and the build number from the version
   profile. The version is republished at 0x1b2f088 and logged; a machine that is
   not being restarted arms the game loop; then the persistent counters come in
   and two more readers run, the last of them only when 0x530b70 says so - a tail
   call, which is why it has to live in another TU. */
// FUNCTION: EZ2AC_UNPACKED 0x00415a40
void EZ2AC::readSettings()
{
    // STRING: EZ2AC_UNPACKED 0x0048cf5c
    g_iniProfile->getInt("GameLevel", &g_gameLevel);
    g_iniProfile->getInt("GameVolume", &g_gameVolume);
    // STRING: EZ2AC_UNPACKED 0x0048d2e8
    g_iniProfile->getInt("AdvSound", &g_advSound);
    // STRING: EZ2AC_UNPACKED 0x0048d2dc
    g_iniProfile->getInt("EventMode", &g_eventMode);
    // STRING: EZ2AC_UNPACKED 0x0048d2c8
    g_iniProfile->getInt("EventModeMixName", &g_eventModeMixName);
    // STRING: EZ2AC_UNPACKED 0x0048d2b0
    g_iniProfile->getInt("EventModeSongNameIdx", &g_eventModeSongNameIdx);
    // STRING: EZ2AC_UNPACKED 0x0048d29c
    g_iniProfile->getStr("EventModeSongName", g_eventModeSongName);
    // STRING: EZ2AC_UNPACKED 0x0048d290
    g_iniProfile->getInt("1PlayerOnly", &g_onePlayerOnly);
    // STRING: EZ2AC_UNPACKED 0x0048d280
    g_iniProfile->getInt("AutoScratchL", &g_autoScratchL);
    // STRING: EZ2AC_UNPACKED 0x0048d270
    g_iniProfile->getInt("AutoScratchR", &g_autoScratchR);
    // STRING: EZ2AC_UNPACKED 0x0048d264
    g_iniProfile->getInt("AutoPedal", &g_autoPedal);
    g_iniProfile->getInt("Wide", &g_wide);
    // STRING: EZ2AC_UNPACKED 0x0048d234
    g_patchProfile->getInt("RestartSystem", &g_restartSystem);
    // STRING: EZ2AC_UNPACKED 0x0048d228
    g_patchProfile->getInt("ResetTime", &g_resetTime);
    // STRING: EZ2AC_UNPACKED 0x0048d21c
    g_patchProfile->getInt("RemainPatch", &g_remainPatch);
    // STRING: EZ2AC_UNPACKED 0x0048d20c
    g_patchProfile->getInt("InitializePatch", &g_initializePatch);
    // STRING: EZ2AC_UNPACKED 0x0048d1f4
    g_patchProfile->getInt("AllRankingFileDelete", &g_allRankingFileDelete);
    // STRING: EZ2AC_UNPACKED 0x0048d1e0
    g_patchProfile->getInt("DelTurntableRanking", &g_delTurntableRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d1cc
    g_patchProfile->getInt("DelStreetMixRanking", &g_delStreetMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d1b8
    g_patchProfile->getInt("DelSpaceMixRanking", &g_delSpaceMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d1a4
    g_patchProfile->getInt("DelRubyMixRanking", &g_delRubyMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d190
    g_patchProfile->getInt("DelEz2CatchRanking", &g_delEz2CatchRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d17c
    g_patchProfile->getInt("DelClubMixRanking", &g_delClubMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d164
    g_patchProfile->getInt("Del7StreetMixRanking", &g_del7StreetMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d150
    g_patchProfile->getInt("Del5keymixRanking", &g_del5KeyMixRanking);
    // STRING: EZ2AC_UNPACKED 0x0048d258
    g_versionProfile->getInt("GameVersion", &g_gameVersion);

    g_1b2f088 = g_gameVersion;
    // STRING: EZ2AC_UNPACKED 0x0048d244
    Log("Game Version = %d", g_gameVersion);

    if (g_restartSystem == 0)
    {
        m_f1c = 1;
    }

    loadCounters();
    readPatchSettings();
    if (g_530b70 != 0)
    {
        readBookkeeping();
    }
}
