/* EZ2AC::runSpaceMix @0x417850 - the SpaceMix campaign driver, the
   family's mid-size dialect: load the mode .gds, then up to three
   rounds of song select and the keys game. A g_1b2ea28 win past the
   first element (past the second with g_1b2efb4 down) runs the
   game-over screen and leaves with 1; verdict 3 runs game over and -
   with g_1b5f2e4 up and the ranking not suppressed - the ranking
   screen inside the name-entry window, then leaves with 1; verdict 2
   runs the result (a 1 leaves through runGameOver), the ranking
   screen inside the window when allowed, and the mode intro on the
   g_1b2ea34 flag. After three rounds six mode latches drop, free-play
   rises, and the ending leaves with 2.

   Its own TU, same reasoning as src/runcv2mix.cpp. */
#include "kcrt.h"
#include "ez2acapp.h"
#include "ezscreen.h"
#include "kengineapp.h"

extern int g_elemCount;             /* 0x1b2e718 */
extern int g_1b2ea28;               /* src/moderunner.cpp */
extern int g_1b2efb4;               /* 0x01b2efb4 (src/moderunner.cpp) */
extern int g_1b2ea34;               /* src/moderunner.cpp */
extern int g_1b2f024;               /* src/moderunner.cpp */
extern int g_1b5f2e4;               /* src/... */
extern int g_1b2ef70;
extern int g_1b2ef58;
extern int g_1b2ead0;
extern int g_1b2e7dc;
extern int g_1b2e5a4;
extern int g_1b2ea24;
extern int g_1b2e7c4;
extern int g_1b2ef40;               /* src/moderunner.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x00417850
int EZ2AC::runSpaceMix()
{
    int round;
    int code;
    int stageResult;

    loadModeGds();
    round = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;
        code = runMainGame();
        if (g_1b2efb4 == 1)
        {
            if (g_elemCount >= 1 && g_1b2ea28 == 1)
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
                return 1;
            }
        }
        else if (g_elemCount >= 2 && g_1b2ea28 == 1)
        {
            Screen48D32C *gameOver = new Screen48D32C;
            gameOver->run(g_iniProfile);
            delete gameOver;
            return 1;
        }
        if (code == 3)
        {
            Screen48D32C *gameOver = new Screen48D32C;
            gameOver->run(g_iniProfile);
            delete gameOver;
            if (g_1b5f2e4 == 1)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                {
                    Screen48D3F0 *ranking = new Screen48D3F0;
                    ranking->run(g_iniProfile);
                    delete ranking;
                }
            }
            return 1;
        }
        if (code == 2)
        {
            if (g_1b2ea28 == 0)
                stageResult = runResult();
            if (stageResult == 1)
            {
                runGameOver();
                return 1;
            }
            if (g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                runRankingScreen();
                g_1b2f024 = 0;
            }
            if (g_1b2ea34 == 1)
                runModeIntro();
        }
        round++;
    } while (round < 3);
    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    runEnding();
    return 2;
}
