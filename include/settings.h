#ifndef SETTINGS_H
#define SETTINGS_H

/* The machine's configuration, one variable per .ini key. Every name here is
   evidence, not invention: readSettings @0x415a40 and saveSettings @0x415310
   read and write each of these through the key it is named after. */

/* EZ2AC.ini (src/settings.cpp unless noted) */
extern int  g_gameLevel;
extern int  g_gameVolume;              /* src/moderunner.cpp */
extern int  g_advSound;
extern int  g_eventMode;
extern int  g_eventModeMixName;
extern int  g_eventModeSongNameIdx;
extern char g_eventModeSongName[0x40];
extern int  g_onePlayerOnly;
extern int  g_autoScratchL;
extern int  g_autoScratchR;
extern int  g_autoPedal;
extern int  g_wide;                    /* src/moderunner.cpp */

/* patch.ini - the operator's maintenance switches */
extern int g_restartSystem;
extern int g_resetTime;                /* src/resetscreen.cpp */
extern int g_remainPatch;
extern int g_initializePatch;
extern int g_allRankingFileDelete;
extern int g_delTurntableRanking;
extern int g_delStreetMixRanking;
extern int g_delSpaceMixRanking;
extern int g_delRubyMixRanking;
extern int g_delEz2CatchRanking;
extern int g_delClubMixRanking;
extern int g_del7StreetMixRanking;
extern int g_del5KeyMixRanking;

/* The version profile, and where readSettings republishes it. */
extern int g_gameVersion;
extern int g_1b2f088;                  /* src/kengineapp.cpp */

/* The persistent counters (src/moderunner.cpp), and the twenty-two unlock flags
   next to them. */
extern int g_totalCoin;
extern int g_serviceCoin;
extern int g_totalPlay;                /* src/ez2acapp.cpp */
extern int g_rubyCount;

/* Gates readSettings' last hand-off (src/settings.cpp). */
extern int g_530b70;

#endif /* SETTINGS_H */
