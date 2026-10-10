/* EZ2AC::runCv2Mix @0x417a30 - ITS OWN TU: SongTable::loadSongBin is
   defined in src/moderunner.cpp and must stay a call (in that TU cl
   inlines it into loadBinFile's two-argument form). The run* screens it
   drives all live there too and become plain calls from here. */
#include "kcrt.h"
#include "ez2acapp.h"
#include "ezscreen.h"
#include "kengineapp.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (folded) */
extern void setModeName(const char *name);           /* 0x414e50 */
extern int  g_1b2eb6c;             /* src/ez2acapp.cpp */
extern int  g_elemCount;           /* 0x1b2e718 */
extern int  g_1b2ea28;             /* src/moderunner.cpp */
extern int  g_1b2ef40;             /* src/moderunner.cpp */
extern struct PlayerRecord *addPlayer(int id);       /* 0x418410 */
extern int  g_mixStyle;            /* 0x1b2ebc8 (src/panels.cpp) */
extern int  getListAF8();          /* 0x418340 (src/nodelists.cpp) */
extern int  g_stageLevels[5];      /* 0x1b2ea50 (src/totalresultctor.cpp) */
extern void engineTick418ee0();    /* 0x418ee0 */
extern void engineTick418f60();    /* 0x418f60 */
extern struct CatchPlayer *addCatchPlayer(int id);   /* 0x418350 */
extern struct SongTable g_songTable;                 /* 0x3780958 (src/globals.cpp) */

// VTABLE: EZ2AC_UNPACKED 0x0048d47c
struct Screen48D47C;

/* EZ2AC::runCv2Mix @0x417a30 - the CV2 campaign driver: flag CV2, point
   the mode and the song table at CV2Mix, and run up to three rounds of
   song select plus the style's own game - Ez2Catch through the catch
   runner with a CatchPlayer added, ScratchMix through its screen with a
   PlayerRecord added (the screen constructed inline here, its only
   builder), anything else through the keys game. A round that ends in
   the g_1b2ea28 win zeroes its stage level and moves on; otherwise the
   result screen runs - written once per flag arm, as the original has
   it - and a game-over verdict runs the ticks and the game-over screen
   and leaves with 1. After three rounds the free-play flag rises and
   the run ends through game over (2) or the ending (2). */
// FUNCTION: EZ2AC_UNPACKED 0x00417a30
int EZ2AC::runCv2Mix()
{
    int round;
    int code;

    g_1b2eb6c = 1;
    setModeName("CV2Mix");
    g_songTable.loadSongBin("CV2Mix");
    round = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;
        if (g_mixStyle == 8)
        {
            addCatchPlayer(getListAF8());
            runCatchGame();
        }
        else if (g_mixStyle == 9)
        {
            addPlayer(getListAF8());
            Screen48D47C *game = new Screen48D47C;
            game->run(g_iniProfile);
            delete game;
            // STRING: EZ2AC_UNPACKED 0x0048d628
            Log("Created GF Player");
        }
        else
            runMainGame();
        if (g_1b2ea28 == 1)
        {
            g_stageLevels[round] = 0;
        }
        else if (g_1b2ea28 == 0)
        {
            Screen48D380 *result = new Screen48D380;
            code = result->run(g_iniProfile);
            delete result;
            if (code == 1)
            {
                engineTick418ee0();
                engineTick418f60();
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
                return 1;
            }
        }
        else
        {
            Screen48D380 *result = new Screen48D380;
            code = result->run(g_iniProfile);
            delete result;
            if (code == 1)
            {
                engineTick418ee0();
                engineTick418f60();
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
                return 1;
            }
        }
        round++;
    } while (round < 3);
    g_1b2ef40 = 1;
    if (g_1b2ea28 != 0)
    {
        Screen48D32C *gameOver = new Screen48D32C;
        gameOver->run(g_iniProfile);
        delete gameOver;
        return 2;
    }
    runEnding();
    return 2;
}
