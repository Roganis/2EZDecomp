#include <stdio.h>
#include <string.h>
#include "kcrt.h" /* operator new/delete == an inlined malloc/free */
#include "ezscreen.h"
#include "ez2acapp.h" /* g_iniProfile, g_1b2eb6c */
#include "kparser.h"

/* src/moderunner.cpp defines saveSettings @0x415310 below; runGame calls it. */

/* Helpers shared by the game's 13 mode runners (the per-mode entry points
   EZ2AC::runGame @0x417e90 dispatches to). Their callees stay in src/stubs.cpp:
   visible here, cl would inline them and the calls would disappear. */

/* src/nodelists.cpp - the list helpers the runners use. Only the pointers'
   nullness matters here, so an incomplete type is enough. */
struct ListNode;
extern int g_totalPlay;     /* src/ez2acapp.cpp - the round counter */
extern int g_useGameOver; /* src/ez2acapp.cpp - the "UseGameOver" setting */
extern int g_elemCount;   /* src/loops.cpp - the round counter the runners publish */
extern int countListA();
extern ListNode *findListA(int key);
extern ListNode *addToListA(int id);

/* src/nodelists.cpp - the per-round list drains the escape paths perform. */
extern void engineTick418e60();
extern void engineTick418ee0();

/* src/com_wrappers.cpp - two more already-matched methods of the same graphics
   singleton (it holds a COM interface at +0x30). Only the class name matters for
   the call, so a minimal redeclaration is enough. */
struct Obj40BB00
{
    void doOp7(int v);   /* 0x40a470 */
    void doOp10(unsigned int v);  /* 0x40a730 */
};

/* src/stubs.cpp - the generic file loader loadSongBin wraps. */


/* src/modename.cpp - the mode-name globals and the setter that publishes one. */
extern void setModeName(const char *name);
extern char g_textSlots[32][128];

/* src/modename.cpp - the current mode's name, which spells the .gds path. */
extern char g_modeName[256];

/* src/maingame.cpp - the style PREFIX the tutorial runs build from the mode
   name (0x1b2e5d0; the C wrote it into g_styleTag @0x1b2e5ac until
   2026-10-01). */
extern char g_stylePrefix[64];

/* src/songselect.cpp - the channel whose eyecatch the radio loop builds. */
extern char g_channelName[0x80];   /* 0x1b2e728 */

/* The tutorial-round arm flag and the two the round test reads. */
extern int g_1b2eb44;   /* src/gamestate.cpp */
extern int g_1b2efb4;   /* src/catchpanelgds.cpp */
extern int g_1b2ea30;   /* src/modeselectctor.cpp */

/* src/ez2acapp.cpp - the flag that suppresses the .gds path build. */
extern int g_1b2eb6c;

/* src/nodelists.cpp - the active-player list helpers. */
extern int countListC();
extern struct PlayerRecord *addPlayer(int id);   /* 0x418410 */
extern int g_1b2e7d8;                            /* src/ez2acapp.cpp */
extern int getListCF8();

/* Never raised: its only two writers in the binary, runEnding @0x415db0
   and runScratchMix @0x417c70, both store 0, and no code takes its address
   - so every `g_1b2ef70 == 0` test below is always true. (Byte scan,
   2026-10-01.) */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef70
int g_1b2ef70;

// VTABLE: EZ2AC_UNPACKED 0x0048d364
struct Screen48D364;

/* Every mode runner ends here: blank the players' initials and zero any score
   they had, then run the ending screen and hand its return code back.
   Two players are cleared when two are active, otherwise just the one the
   list names. The record lookup is repeated for every single field because cl
   cannot CSE a call - which is exactly what the original does (seven calls for
   one test and six stores). */
// FUNCTION: EZ2AC_UNPACKED 0x00415db0
int EZ2AC::runEnding()
{
    int i;
    int id;

    g_1b2ef70 = 0;

    if (countListC() >= 2)
    {
        for (i = 0; i < 2; i++)
        {
            if (findPlayer(i)->score > 0)
            {
                findPlayer(i)->score = 0;
                findPlayer(i)->initials[0] = ' ';
                findPlayer(i)->initials[1] = ' ';
                findPlayer(i)->initials[2] = ' ';
                findPlayer(i)->initials[3] = ' ';
                findPlayer(i)->initials[4] = ' ';
            }
        }
    }
    else if (countListC() >= 1)
    {
        id = getListCF8();

        if (findPlayer(id)->score > 0)
        {
            findPlayer(id)->score = 0;
            findPlayer(id)->initials[0] = ' ';
            findPlayer(id)->initials[1] = ' ';
            findPlayer(id)->initials[2] = ' ';
            findPlayer(id)->initials[3] = ' ';
            findPlayer(id)->initials[4] = ' ';
        }
    }

    Screen48D364 *ending = new Screen48D364;
    int code = ending->run(g_iniProfile);
    delete ending;
    return code;
}

/* Load the current mode's .gds config - "System\<mode>\<mode>.gds" - through a
   stack KParser, read one section and three ints out of it, and return the last
   int read. Every mode runner calls this. The parser is a 0x502c-byte local, so
   the frame goes through _chkstk and the scoped destructor gives the function its
   EH funclet and the two dtor call sites (one per exit). */
// FUNCTION: EZ2AC_UNPACKED 0x004150d0
int EZ2AC::loadModeGds()
{
    int v0;
    int v1;
    int v2;
    char name[260];
    char section[256];
    KParser parser;

    if (g_1b2eb6c != 1)
    {
        // STRING: EZ2AC_UNPACKED 0x0048cfa4
        sprintf(name, "System\\%s\\%s.gds", g_modeName, g_modeName);
    }

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    readGdsSection(section, &parser);
    readGdsInt(&v2, &parser);
    readGdsInt(&v1, &parser);
    readGdsInt(&v0, &parser);
    return v0;
}

/* Load the song table: the same generic loader as everything else, with the file
   name supplied. Called from runCv2Mix @0x417a30 and from 0x44d794. It cannot
   tail-jump the callee because the two argument counts differ (`ret 4` here
   against the callee's `ret 8`), so the forwarded return value just stays in eax. */
// FUNCTION: EZ2AC_UNPACKED 0x00469fb0
int SongTable::loadSongBin(const char *dir)
{
    // STRING: EZ2AC_UNPACKED 0x0049c800
    return loadBinFile(dir, "song.bin");
}

/* The machine's persistent counters and the twenty-two unlock flags the settings
   writeback @0x415310 pushes back into the .ini. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01696324
int g_rubyCount;
// GLOBAL: EZ2AC_UNPACKED 0x016963dc
int g_totalCoin;
// GLOBAL: EZ2AC_UNPACKED 0x016963e4
int g_wide;
// GLOBAL: EZ2AC_UNPACKED 0x016963e8
int g_serviceCoin;
/* The twenty-two unlock flags. Declared one at a time, not as an array: they
   ARE contiguous (and are in fact elements 14..35 of the wider flag array at
   0x1b2e7b0 that @0x453743 indexes), but loadCounters @0x4156c0 passes the
   ADDRESS of each one, and reccmp will only name an immediate that is an
   entity's exact base - an interior `&arr[i]` would print raw and differ. The
   original's twenty-two separate key literals say the source enumerated them
   one by one anyway. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7e8
int g_evolution0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7ec
int g_evolution1;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7f0
int g_evolution2;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7f4
int g_evolution3;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7f8
int g_evolution4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7fc
int g_evolution5;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e800
int g_evolution6;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e804
int g_evolution7;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e808
int g_evolution8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e80c
int g_evolution9;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e810
int g_evolution10;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e814
int g_evolution11;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e818
int g_evolution12;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e81c
int g_evolution13;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e820
int g_evolution14;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e824
int g_evolution15;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e828
int g_evolution16;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e82c
int g_evolution17;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e830
int g_evolution18;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e834
int g_evolution19;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e838
int g_evolution20;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e83c
int g_evolution21;

/* Per-round state the mode runners publish. Placeholder storage; only the
   reads/writes the runners make are evidenced. */
/* The screen the mixer setup @0x4184f0 published; and the flag that picks
   between the two master/wave levels it forces. Placeholder storage. */
/* The object record the pool allocator @0x4016a0 takes its next slot from; every
   factory publishes one here just before its `new`. */
// GLOBAL: EZ2AC_UNPACKED 0x00530b74
EzObjectInfo *g_objectRecord;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e844
void *g_1b2e844;
// GLOBAL: EZ2AC_UNPACKED 0x004a8d78
int g_gameVolume;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e5a4
int g_1b2e5a4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7c4
int g_1b2e7c4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7dc
int g_1b2e7dc;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea24
int g_1b2ea24;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea28
int g_1b2ea28;   /* the GAME-OVER latch: raised (1) by the sinks when a gauge
                    hits zero (m4204b0 / m420570 / the GF and catch twins /
                    the gauge-over ramp in update2), cleared by song select,
                    the stage start, the ranking and the ending; the runners
                    read it after runMainGame - 1 = game-over screen once the
                    round threshold is met, 0 = the result screen. Never
                    "the win". */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea34
int g_1b2ea34;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ead0
int g_1b2ead0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef40
int g_1b2ef40;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef58
int g_1b2ef58;
/* The game-over latch. Set to 1 on a death by the three panels (keys
   Panel::m42c5f0, GFPanel::m4649c0, CatchPanel::m472020); the ONLY clear in
   the binary is the TitleDirector ctor @0x44e9b0 (0x44eb6d), and no code
   takes its address - so it holds for the rest of the session and drops
   when the title screen is built again. (Byte scan, 2026-10-01.) */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efb4
int g_1b2efb4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f024
int g_1b2f024;

/* ---------------------------------------------------------------------------
   The screen helpers the 13 mode runners share. Every one is the same three
   statements - new the screen, drive it through Obj401780::run with the app's
   .ini profile, delete it - and every one returns the screen's exit code (a few
   callers discard it). The compiler-generated ctor is inlined to the vptr store
   and `delete` is kcrt.h's inlined free, so no ctor or dtor call appears, exactly
   as in EZ2AC::runGame. Each vftable is pinned HERE because this is the TU whose
   inlined ctor makes cl lay it down.

   Three of them have a second arm: an escape code puts up the test-mode screen
   (vftable 0x48d310, the same one runGame uses on its own code 2) and, for the
   two game screens, drains that round's node list. Independently found by three
   agents, which is what identifies 0x48d310 as the test-mode screen.
   --------------------------------------------------------------------------- */

// VTABLE: EZ2AC_UNPACKED 0x0048d32c
struct Screen48D32C;

/* runGameOver @0x415d70 lives in src/moderunscreens.cpp. */

// VTABLE: EZ2AC_UNPACKED 0x0048d380
struct Screen48D380;

/* runResult @0x415ec0 lives in src/moderunscreens.cpp. */

// VTABLE: EZ2AC_UNPACKED 0x0048d3b8
struct SongSelectScreen;

/* Song select - every runner's first step, looped while it returns 0. Code 2 is
   the operator's test-menu request: that screen runs first and the 2 still
   reaches the runner, which is what makes it leave the mode. */
// FUNCTION: EZ2AC_UNPACKED 0x00415f70
int EZ2AC::runSongSelect()
{
    int code;

    SongSelectScreen *select = new SongSelectScreen;
    code = select->run(g_iniProfile);
    delete select;

    if (code == 2)
    {
        Screen48D310 *test = new Screen48D310;
        test->run(g_iniProfile);
        delete test;
    }

    return code;
}

// VTABLE: EZ2AC_UNPACKED 0x0048d3f0
struct Screen48D3F0;

/* runRankingScreen @0x415fe0 lives in src/moderunscreens.cpp. */

// VTABLE: EZ2AC_UNPACKED 0x0048d444
struct DrawModeIntro;

/* runModeIntro @0x416020 lives in src/moderunscreens.cpp. */

// VTABLE: EZ2AC_UNPACKED 0x0048d460
struct Screen48D460;

/* The Catch game - EZ2CATCH (mode 10) and CV2Mix (mode 12), the two modes that
   share that engine. Code 6 escapes to test mode and drops list B. */
// FUNCTION: EZ2AC_UNPACKED 0x00416060
int EZ2AC::runCatchGame()
{
    Screen48D460 *game = new Screen48D460;
    int code = game->run(g_iniProfile);
    delete game;

    if (code == 6)
    {
        Screen48D310 *test = new Screen48D310;
        test->run(g_iniProfile);
        delete test;
        engineTick418ee0();
    }

    return code;
}

// VTABLE: EZ2AC_UNPACKED 0x0048d498
struct Screen48D498;

/* runGFResult @0x4162e0 lives in src/moderunscreens.cpp. */

// VTABLE: EZ2AC_UNPACKED 0x0048d2f4
struct Screen48D2F4;

/* The main game screen - ten runners. Code 6 is the in-game escape to test mode,
   after which the active-player list is drained. */
// FUNCTION: EZ2AC_UNPACKED 0x00416320
int EZ2AC::runMainGame()
{
    Screen48D2F4 *game = new Screen48D2F4;
    int code = game->run(g_iniProfile);
    delete game;

    if (code == 6)
    {
        Screen48D310 *test = new Screen48D310;
        test->run(g_iniProfile);
        delete test;
        engineTick418e60();
    }

    return code;
}

// VTABLE: EZ2AC_UNPACKED 0x0048d39c
struct ModeSelectScreen;

/* The gate runGame checks before it dispatches a mode: run the 0x48d39c screen
   and hand its code back; code 4 means the operator asked for the test menu, so
   that screen runs too (and the 4 still reaches runGame). */
// FUNCTION: EZ2AC_UNPACKED 0x00415f00
int EZ2AC::m415f00()
{
    ModeSelectScreen *gate = new ModeSelectScreen;
    int code = gate->run(g_iniProfile);
    delete gate;

    if (code == 4)
    {
        Screen48D310 *test = new Screen48D310;
        test->run(g_iniProfile);
        delete test;
    }

    return code;
}

/* The "TestGame" harness EZ2AC::v4 runs when the app .ini asks for it: append an
   underscore to the current mode name (so the loader picks up the test data),
   then loop the game forever - register the players, gate on m415f00, and run
   song select plus the main game. Two players are registered for every mode
   except the four that are single-player. It only leaves when the gate returns
   non-zero, and then it returns 0. */
// FUNCTION: EZ2AC_UNPACKED 0x004164f0
int EZ2AC::runTestGame()
{
    char name[128];

    g_useGameOver = 0;
    // STRING: EZ2AC_UNPACKED 0x0048d4d4
    sprintf(name, "%s_", g_modeName);
    setModeName(name);
    g_elemCount = 0;

    for (;;)
    {
        // STRING: EZ2AC_UNPACKED 0x0048d4c0
        if (_stricmp("RadioMix", g_modeName) != 0 &&
            // STRING: EZ2AC_UNPACKED 0x0048d4b4
            _stricmp("5RadioMix", g_modeName) != 0 &&
            // STRING: EZ2AC_UNPACKED 0x0048cf20
            _stricmp("StreetMix", g_modeName) != 0 &&
            // STRING: EZ2AC_UNPACKED 0x0048cf14
            _stricmp("7StreetMix", g_modeName) != 0)
        {
            addToListA(0);
        }
        else
        {
            addToListA(0);
            addToListA(1);
        }

        if (m415f00() != 0)
        {
            return 0;
        }

        if (runSongSelect() == 0)
        {
            runMainGame();
        }

        engineTick418e60();
    }
}

/* The "TestSong" harness: take the song and mix the app .ini names, publish the
   song into text slot 0 and the mix as the current mode, then run one round of
   the main game with the test-data mode name. Same single-vs-two-player split as
   runTestGame, on a slightly different set of modes. */
// FUNCTION: EZ2AC_UNPACKED 0x004163a0
int EZ2AC::runTestSong()
{
    char *mixName;
    char *songName;
    char name[128];
    int code;

    // STRING: EZ2AC_UNPACKED 0x0048d4e4
    g_iniProfile->getValue("TestSongName", &songName);
    // STRING: EZ2AC_UNPACKED 0x0048d4d8
    g_iniProfile->getValue("TestMixName", &mixName);

    strcpy(g_textSlots[0], songName);
    setModeName(mixName);

    g_useGameOver = 0;
    g_elemCount = 0;
    sprintf(name, "%s_", g_modeName);
    setModeName(name);

    // STRING: EZ2AC_UNPACKED 0x0048d4cc
    if (_stricmp("RubyMix", g_modeName) != 0 &&
        _stricmp("StreetMix", g_modeName) != 0 &&
        _stricmp("7StreetMix", g_modeName) != 0 &&
        _stricmp("RadioMix", g_modeName) != 0 &&
        _stricmp("5RadioMix", g_modeName) != 0)
    {
        addToListA(1);
    }
    else
    {
        addToListA(0);
        addToListA(1);
    }

    code = runMainGame();
    engineTick418e60();
    return code;
}

/* The cross-fade the four radio modes run between stages: load three textures,
   then draw all three every frame for 256 steps, raising the fade level by the
   caller's step each time. The first texture is drawn opaque at the origin and
   the other two at (-2, -2) with a different blend, so this is a layered
   transition. The handles land in three locals - cl gives the first one the dead
   `first` parameter's slot. */
// FUNCTION: EZ2AC_UNPACKED 0x00415200
void EZ2AC::crossFade(char *first, char *second, char *third, int step)
{
    KGraphics *g = (KGraphics *)g_sub1b2f054;
    KTexture *a;
    KTexture *b;
    KTexture *c;
    int level;

    g->loadTexture(first, &a, 0);
    g->loadTexture(second, &b, 0);
    g->loadTexture(third, &c, 0);

    ((Obj40BB00 *)g)->doOp7(0);

    for (level = 0; level <= 255; level += step)
    {
        g->m405e10();
        ((Obj40BB00 *)g)->doOp10(0);

        g->setBlend(2, 2);
        g->drawTex(a, 0.0f, 0.0f, 0.0f, 0.0f, -1);

        g->setBlend(9, 6);
        g->drawTex(b, -2.0f, -2.0f, 0.0f, 0.0f, -1);

        g->setBlend(2, 2);
        g->drawTex(c, -2.0f, -2.0f, 0.0f, 0.0f, -1);

        g->setFade(level);
        g->endFrame();
    }

    ((Obj40BB00 *)g)->doOp7(1);
    g->m403f60();
    g->m408c80();
}

/* Write the machine's settings back to <ClassName>.ini: the coin/play counters,
   the volume and wide mode, then the twenty-two "EvolutionN" unlock flags (kept
   as one array of ints, each written as a 0/1), then flush the profile and hand
   the file to a parser. Every runner calls this on its way out.

   Each flag goes through its own named local. Passing `g_evolution[N] > 0`
   straight as the argument makes cl schedule the compare BEFORE the this-load and
   round-robin the setg register (edx, eax, ecx, ...) to break the dependency on
   the previous call; the local pins the original's uniform `mov ecx,[profile];
   xor eax,eax; cmp; setg al` for all twenty-two. */
/* The read side of the same set, and what names every one of these globals:
   the machine's persistent counters and the twenty-two unlock flags, pulled out
   of the .ini into the exact variables saveSettings @0x415310 pushes back. The
   profile pointer is re-read before every call because a call may write it, and
   `this` is never touched - the caller passes it (`mov ecx,esi; call`) but the
   body only ever works through the global profile.

   The key literals need no STRING markers here: saveSettings and
   KEngineApp::loadProfiles already annotate all twenty-seven addresses, and a
   marker anywhere under src/ registers the address for every TU that uses the
   same literal. */
// FUNCTION: EZ2AC_UNPACKED 0x004156c0
void EZ2AC::loadCounters()
{
    g_iniProfile->getInt("TotalCoin", &g_totalCoin);
    g_iniProfile->getInt("ServiceCoin", &g_serviceCoin);
    g_iniProfile->getInt("TotalPlay", &g_totalPlay);
    g_iniProfile->getInt("RubyCount", &g_rubyCount);
    g_iniProfile->getInt("Wide", &g_wide);
    g_iniProfile->getInt("Evolution0", &g_evolution0);
    g_iniProfile->getInt("Evolution1", &g_evolution1);
    g_iniProfile->getInt("Evolution2", &g_evolution2);
    g_iniProfile->getInt("Evolution3", &g_evolution3);
    g_iniProfile->getInt("Evolution4", &g_evolution4);
    g_iniProfile->getInt("Evolution5", &g_evolution5);
    g_iniProfile->getInt("Evolution6", &g_evolution6);
    g_iniProfile->getInt("Evolution7", &g_evolution7);
    g_iniProfile->getInt("Evolution8", &g_evolution8);
    g_iniProfile->getInt("Evolution9", &g_evolution9);
    g_iniProfile->getInt("Evolution10", &g_evolution10);
    g_iniProfile->getInt("Evolution11", &g_evolution11);
    g_iniProfile->getInt("Evolution12", &g_evolution12);
    g_iniProfile->getInt("Evolution13", &g_evolution13);
    g_iniProfile->getInt("Evolution14", &g_evolution14);
    g_iniProfile->getInt("Evolution15", &g_evolution15);
    g_iniProfile->getInt("Evolution16", &g_evolution16);
    g_iniProfile->getInt("Evolution17", &g_evolution17);
    g_iniProfile->getInt("Evolution18", &g_evolution18);
    g_iniProfile->getInt("Evolution19", &g_evolution19);
    g_iniProfile->getInt("Evolution20", &g_evolution20);
    g_iniProfile->getInt("Evolution21", &g_evolution21);
}

// FUNCTION: EZ2AC_UNPACKED 0x00415310
void saveSettings()
{
    char name[260];

    // STRING: EZ2AC_UNPACKED 0x0048d148
    sprintf(name, "%s.ini", g_app->className());

    // STRING: EZ2AC_UNPACKED 0x0048d13c
    g_iniProfile->setInt("ServiceCoin", g_serviceCoin);
    // STRING: EZ2AC_UNPACKED 0x0048d130
    g_iniProfile->setInt("TotalCoin", g_totalCoin);
    // STRING: EZ2AC_UNPACKED 0x0048d124
    g_iniProfile->setInt("TotalPlay", g_totalPlay);
    // STRING: EZ2AC_UNPACKED 0x0048d118
    g_iniProfile->setInt("RubyCount", g_rubyCount);
    // STRING: EZ2AC_UNPACKED 0x0048d10c
    g_iniProfile->setInt("GameVolume", g_gameVolume);
    g_iniProfile->setInt("Wide", g_wide);
    {
        int on = g_evolution0 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d100
        g_iniProfile->setInt("Evolution0", on);
    }
    {
        int on = g_evolution1 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0f4
        g_iniProfile->setInt("Evolution1", on);
    }
    {
        int on = g_evolution2 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0e8
        g_iniProfile->setInt("Evolution2", on);
    }
    {
        int on = g_evolution3 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0dc
        g_iniProfile->setInt("Evolution3", on);
    }
    {
        int on = g_evolution4 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0d0
        g_iniProfile->setInt("Evolution4", on);
    }
    {
        int on = g_evolution5 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0c4
        g_iniProfile->setInt("Evolution5", on);
    }
    {
        int on = g_evolution6 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0b8
        g_iniProfile->setInt("Evolution6", on);
    }
    {
        int on = g_evolution7 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0ac
        g_iniProfile->setInt("Evolution7", on);
    }
    {
        int on = g_evolution8 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d0a0
        g_iniProfile->setInt("Evolution8", on);
    }
    {
        int on = g_evolution9 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d094
        g_iniProfile->setInt("Evolution9", on);
    }
    {
        int on = g_evolution10 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d088
        g_iniProfile->setInt("Evolution10", on);
    }
    {
        int on = g_evolution11 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d07c
        g_iniProfile->setInt("Evolution11", on);
    }
    {
        int on = g_evolution12 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d070
        g_iniProfile->setInt("Evolution12", on);
    }
    {
        int on = g_evolution13 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d064
        g_iniProfile->setInt("Evolution13", on);
    }
    {
        int on = g_evolution14 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d058
        g_iniProfile->setInt("Evolution14", on);
    }
    {
        int on = g_evolution15 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d04c
        g_iniProfile->setInt("Evolution15", on);
    }
    {
        int on = g_evolution16 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d040
        g_iniProfile->setInt("Evolution16", on);
    }
    {
        int on = g_evolution17 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d034
        g_iniProfile->setInt("Evolution17", on);
    }
    {
        int on = g_evolution18 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d028
        g_iniProfile->setInt("Evolution18", on);
    }
    {
        int on = g_evolution19 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d01c
        g_iniProfile->setInt("Evolution19", on);
    }
    {
        int on = g_evolution20 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d010
        g_iniProfile->setInt("Evolution20", on);
    }
    {
        int on = g_evolution21 > 0;
        // STRING: EZ2AC_UNPACKED 0x0048d004
        g_iniProfile->setInt("Evolution21", on);
    }

    g_iniProfile->writeFile(name);

    KParser parser;
    parser.m4133d0(name);
}

/* Cleared beside g_1b2ea30 when the tutorial round arms itself; nothing else
   in the tree reads it yet. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7c8
int g_1b2e7c8;

/* --------------------------------------------------------------------------
   EZ2AC::runStreetMix @0x4168c0 - a MODE LOOP, not one of the thin runners
   above: three stages of song-select + game, with the ranking and intro
   screens interleaved and an ending screen at the bottom.

   It belongs in THIS TU and nowhere else. Three of its screen blocks are the
   bodies of runResult, runGameOver and runRankingScreen written out inline -
   no call, just malloc / vptr store / run / free - which is exactly what cl
   does with a sibling it can see. Put this function anywhere else and those
   three become calls.

   Its shape:
     - the TUTORIAL round, gated on g_1b2eb44 and armed once: build the style
       tag from the mode name, name text slot 0 "Tutorial", play a game and a
       ranking screen, then clear the flag;
     - the STAGE LOOP, three rounds, publishing the round number to
       g_elemCount before each song select. Song select returning non-zero is
       the operator escape and returns 0 outright;
     - the GAME OVER test, which is a single player who has reached the last
       round - round 1 when g_1b2efb4 is up and round 2 otherwise;
     - code 2 runs the result screen (only when g_1b2ea28 is clear) and keeps
       ITS code in a local that OUTLIVES the iteration: the `!= 0` path skips
       the store and tests whatever the previous round left, which is why the
       variable is declared outside the loop and not beside the call;
     - code 5 puts a player back on list A and repeats the round.

   The frame says the same thing: 0x8c with the round at F-0x8c, that carried
   result code at F-0x88 and the 128-byte tag buffer at F-0x84 running exactly
   to the cookie.

   Written for the port; 718 bytes, 209 instructions, 29 calls. */

// FUNCTION: EZ2AC_UNPACKED 0x004168c0
int EZ2AC::runStreetMix()
{
    char tag[128];
    int  round;
    int  resultCode;
    int  code;
    int  c;

    loadModeGds();

    round = 0;
    do
    {
        if (g_1b2eb44 == 1)
        {
            int  i;
            char c;

            sprintf(tag, "STYLE_%s1_", g_modeName);
            /* Both copies are hand-written index loops in the original, not
               strcpy: cl constant-folds strcpy of a literal into a dword and
               byte store, which is not what the second one has. */
            i = 0;
            do
            {
                c = tag[i];
                g_stylePrefix[i] = c;
                i++;
            } while (c != 0);
            g_1b2e7c8 = 0;
            g_1b2ea30 = 0;
            i = 0;
            do
            {
                // STRING: EZ2AC_UNPACKED 0x0048d504
                c = "Tutorial"[i];
                g_textSlots[0][i] = c;
                i++;
            } while (c != 0);
            // STRING: EZ2AC_UNPACKED 0x0048d4f4
            Log("TUTORIAL ENABLE");
            runMainGame();
            {
                Screen48D3F0 *ranking = new Screen48D3F0;
                ranking->run(g_iniProfile);
                delete ranking;
            }
            g_1b2eb44 = 0;
        }

        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        code = runMainGame();

        /* One player on the last round of the set, and the set says so. */
        if (countListA() == 1)
        {
            c = g_1b2ea28;
            if (g_1b2efb4 == 1)
            {
                if (g_elemCount >= 1 && c == 1)
                {
                    runGameOver();
                    return 1;
                }
            }
            else if (g_elemCount >= 2 && c == 1)
            {
                runGameOver();
                return 1;
            }
        }

        if (code == 2)
        {
            if (g_1b2ea28 == 0)
                {
                    Screen48D380 *result = new Screen48D380;
                    resultCode = result->run(g_iniProfile);
                    delete result;
                }
            if (resultCode == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
            if (countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                    runRankingScreen();
                g_1b2f024 = 0;
            }
            if (g_1b2ea34 == 1)
                runModeIntro();
            else if (countListA() >= 2 && g_elemCount <= 1)
                runModeIntro();
        }
        else if (code == 5)
        {
            /* Two calls, not a ternary - cl cross-jumps them back into one. */
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            round -= 1;
        }

        round = round + 1;
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

/* EZ2AC::runRubyMix @0x416600 - the same mode loop as runStreetMix above, and
   found by scratchpad/twins.py at 62%. 208 instructions against that one's
   210; the tutorial round, the stage loop, the code 5 re-round and the ending
   block are all the same source. Four things differ:

     - IT NEVER RETURNS 2. Both the song-select escape and the normal end fall
       into one `xor eax,eax` exit, so it returns 0 or 1 where runStreetMix
       returns 0, 1 or 2.
     - THE GAME OVER TEST COMES FIRST and is spelled the other way round: this
       one branches on g_1b2efb4 and then checks the round, where runStreetMix
       asks countListA() == 1 first and folds g_1b2efb4 into the round it
       compares against.
     - CODE 3 IS ITS OWN ARM, another game over, which runStreetMix has no
       equivalent of. All three of its game-over sites are INLINED - and note
       that runResult at 0x415ec0 is a REAL CALL here where runStreetMix
       inlines it, from the same TU. That is cl's per-site inlining choice, not
       anything either source says, and it is the same asymmetry noted on
       runStreetMix.
     - The ranking-screen block is additionally gated on `round <= 3`, which
       cannot be false with a three-round loop but is in the original.

   The frame tiles exactly: 0x88 = the round at F-0x88 and a 128-byte tag
   buffer at F-0x84 running to the cookie, with none of runStreetMix's carried
   result slot because this one keeps the round in a register throughout. */

// FUNCTION: EZ2AC_UNPACKED 0x00416600
int EZ2AC::runRubyMix()
{
    char tag[128];
    int  round;
    int  code;

    loadModeGds();

    round = 0;
    do
    {
        if (g_1b2eb44 == 1)
        {
            int  i;
            char c;

            sprintf(tag, "STYLE_%s1_", g_modeName);
            i = 0;
            do
            {
                c = tag[i];
                g_stylePrefix[i] = c;
                i++;
            } while (c != 0);
            g_1b2e7c8 = 0;
            g_1b2ea30 = 0;
            i = 0;
            do
            {
                c = "Tutorial"[i];
                g_textSlots[0][i] = c;
                i++;
            } while (c != 0);
            Log("TUTORIAL ENABLE");
            runMainGame();
            {
                Screen48D3F0 *ranking = new Screen48D3F0;
                ranking->run(g_iniProfile);
                delete ranking;
            }
            g_1b2eb44 = 0;
        }

        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        code = runMainGame();

        if (g_1b2efb4 == 1)
        {
            if (g_elemCount >= 1 && g_1b2ea28 == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
        }
        else if (g_elemCount >= 2 && g_1b2ea28 == 1)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }

        if (code == 2)
        {
            if (g_1b2ea28 == 0)
                runResult();
            if (round <= 3 && countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                    runRankingScreen();
                g_1b2f024 = 0;
            }
            if (g_1b2ea34 == 1)
                runModeIntro();
            else if (countListA() >= 2 && g_elemCount <= 1)
                runModeIntro();
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            round -= 1;
        }

        round = round + 1;
    } while (round < 3);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    runEnding();
    return 0;
}

/* EZ2AC::runRadioMix @0x416d60 - the radio channels' mode loop, and a
   DIFFERENT shape from runStreetMix and runRubyMix above (0.133 against the
   ruby one; kin.py groups it with its own three siblings instead). 172
   instructions, frame 0x314.

   What makes it its own thing is the EYECATCH: every stage builds three
   bitmap paths - the channel's own, a shared mask and a numbered stage plate -
   and from stage 1 onward cross-fades between them before the game starts.
   That is what the 0x314 frame is for, and it tiles exactly: the stage count
   at F-0x314 and three char[260] running to the cookie.

   THE STAGE COUNT COMES FROM loadModeGds. Its return value is kept for the
   whole function: the loop ends a round early when `stage + 1` reaches it -
   running the ranking screen and the ending there rather than after four - and
   code 6 jumps straight to it. The other two runners have no equivalent; they
   count to a literal 3.

   Two more differences worth a port's attention. Song select is run ONCE, up
   front, not per stage. And code 2 has a prior test: with g_1b2ea28 up it is a
   game over and an immediate return, and only otherwise does it run the result
   screen - the `push 0x48` is hoisted above that branch because both arms
   allocate the same size, which is cl's merged push, not two allocations. */

// FUNCTION: EZ2AC_UNPACKED 0x00416d60
int EZ2AC::runRadioMix()
{
    char eyecatch[260];
    char mask[260];
    char plate[260];
    int  stages;
    int  stage;
    int  code;

    stages = loadModeGds();
    g_elemCount = 0;
    if (runSongSelect() != 0)
        return 0;

    stage = 0;
    do
    {
        g_elemCount = stage;
        // STRING: EZ2AC_UNPACKED 0x0048d578
        sprintf(eyecatch, "system\\channel_eyecatch\\radio\\%s.bmp", g_channelName);
        // STRING: EZ2AC_UNPACKED 0x0048d548
        sprintf(mask, "system\\channel_eyecatch\\common\\Stage_Mask.bmp");
        // STRING: EZ2AC_UNPACKED 0x0048d51c
        sprintf(plate, "system\\channel_eyecatch\\common\\Stage_%d.bmp", stage + 1);
        if (stage >= 1)
            crossFade(eyecatch, mask, plate, 10);

        code = runMainGame();

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }
        if (code == 2)
        {
            if (g_1b2ea28 == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
            {
                /* The first result screen written out; the second below is a
                   real call (0x416e..: `new` then `call 0x415ec0`). */
                Screen48D380 *result = new Screen48D380;
                result->run(g_iniProfile);
                delete result;
            }
            if (countListA() != 2 && g_elemCount >= 3)
            {
                g_1b2e7c4 = 1;
                g_1b2ea34 = 0;
                runResult();
            }
            if (stage + 1 == stages)
            {
                runRankingScreen();
                runEnding();
            }
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            stage -= 1;
        }
        else if (code == 6)
        {
            stage = stages;
        }

        stage += 1;
    } while (stage < 4);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    return 2;
}

/* EZ2AC::run10RadioMix @0x417230 - the 10-channel radio loop. The same
   source as runRadioMix above with THREE differences, and 0x417230 and
   0x417450 are byte-identical to each other apart from their one string:

     - the eyecatch folder ("10radio" rather than "radio");
     - the second result screen is gated on the round alone, with no
       countListA() != 2 term;
     - there is NO code 5 arm at all, so a player rejoining does not repeat
       the round the way it does in the plain radio loop.
*/
// FUNCTION: EZ2AC_UNPACKED 0x00417230
int EZ2AC::run10RadioMix()
{
    char eyecatch[260];
    char mask[260];
    char plate[260];
    int  stages;
    int  stage;
    int  code;

    stages = loadModeGds();
    g_elemCount = 0;
    if (runSongSelect() != 0)
        return 0;

    stage = 0;
    do
    {
        g_elemCount = stage;
        // STRING: EZ2AC_UNPACKED 0x0048d5d8
        sprintf(eyecatch, "system\\channel_eyecatch\\10radio\\%s.bmp", g_channelName);
        sprintf(mask, "system\\channel_eyecatch\\common\\Stage_Mask.bmp");
        sprintf(plate, "system\\channel_eyecatch\\common\\Stage_%d.bmp", stage + 1);
        if (stage >= 1)
            crossFade(eyecatch, mask, plate, 10);

        code = runMainGame();

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }
        if (code == 2)
        {
            if (g_1b2ea28 == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
            {
                /* The first result screen written out; the second below is a
                   real call (0x416e..: `new` then `call 0x415ec0`). */
                Screen48D380 *result = new Screen48D380;
                result->run(g_iniProfile);
                delete result;
            }
            if (g_elemCount >= 3)
            {
                g_1b2e7c4 = 1;
                g_1b2ea34 = 0;
                runResult();
            }
            if (stage + 1 == stages)
            {
                runRankingScreen();
                runEnding();
            }
        }
        else if (code == 6)
        {
            stage = stages;
        }

        stage += 1;
    } while (stage < 4);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    return 2;
}

/* EZ2AC::run14RadioMix @0x417450 - the 14-channel radio loop. The same
   source as runRadioMix above with THREE differences, and 0x417230 and
   0x417450 are byte-identical to each other apart from their one string:

     - the eyecatch folder ("14radio" rather than "radio");
     - the second result screen is gated on the round alone, with no
       countListA() != 2 term;
     - there is NO code 5 arm at all, so a player rejoining does not repeat
       the round the way it does in the plain radio loop.
*/
// FUNCTION: EZ2AC_UNPACKED 0x00417450
int EZ2AC::run14RadioMix()
{
    char eyecatch[260];
    char mask[260];
    char plate[260];
    int  stages;
    int  stage;
    int  code;

    stages = loadModeGds();
    g_elemCount = 0;
    if (runSongSelect() != 0)
        return 0;

    stage = 0;
    do
    {
        g_elemCount = stage;
        // STRING: EZ2AC_UNPACKED 0x0048d600
        sprintf(eyecatch, "system\\channel_eyecatch\\14radio\\%s.bmp", g_channelName);
        sprintf(mask, "system\\channel_eyecatch\\common\\Stage_Mask.bmp");
        sprintf(plate, "system\\channel_eyecatch\\common\\Stage_%d.bmp", stage + 1);
        if (stage >= 1)
            crossFade(eyecatch, mask, plate, 10);

        code = runMainGame();

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }
        if (code == 2)
        {
            if (g_1b2ea28 == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
            {
                /* The first result screen written out; the second below is a
                   real call (0x416e..: `new` then `call 0x415ec0`). */
                Screen48D380 *result = new Screen48D380;
                result->run(g_iniProfile);
                delete result;
            }
            if (g_elemCount >= 3)
            {
                g_1b2e7c4 = 1;
                g_1b2ea34 = 0;
                runResult();
            }
            if (stage + 1 == stages)
            {
                runRankingScreen();
                runEnding();
            }
        }
        else if (code == 6)
        {
            stage = stages;
        }

        stage += 1;
    } while (stage < 4);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    return 2;
}

/* EZ2AC::run5RadioMix @0x416fc0 - the 5-channel radio loop, and the fourth of
   kin.py's radio family. It is runRadioMix above with the "5radio" folder and
   ONE extra statement: the eyecatch path is logged as soon as it is built.

   That one Log is why its buffers are laid out the other way round. cl orders
   /GS buffers by USE, not declaration, and the extra reference makes the
   eyecatch the most-used of the three - so it lands at the BOTTOM of the frame
   here (F-0x310) where the plain radio loop puts it at the top (F-0x108),
   with the other two shifted past it. Same 0x314 frame, same three char[260],
   opposite order, and nothing in the source says so. */

// FUNCTION: EZ2AC_UNPACKED 0x00416fc0
int EZ2AC::run5RadioMix()
{
    char eyecatch[260];
    char mask[260];
    char plate[260];
    int  stages;
    int  stage;
    int  code;

    stages = loadModeGds();
    g_elemCount = 0;
    if (runSongSelect() != 0)
        return 0;

    stage = 0;
    do
    {
        g_elemCount = stage;
        // STRING: EZ2AC_UNPACKED 0x0048d5b0
        sprintf(eyecatch, "system\\channel_eyecatch\\5radio\\%s.bmp", g_channelName);
        // STRING: EZ2AC_UNPACKED 0x0048d5a0
        Log(" szChannel = %s", eyecatch);
        sprintf(mask, "system\\channel_eyecatch\\common\\Stage_Mask.bmp");
        sprintf(plate, "system\\channel_eyecatch\\common\\Stage_%d.bmp", stage + 1);
        if (stage >= 1)
            crossFade(eyecatch, mask, plate, 10);

        code = runMainGame();

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }
        if (code == 2)
        {
            if (g_1b2ea28 == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
            {
                /* The first result screen written out; the second below is a
                   real call (0x416e..: `new` then `call 0x415ec0`). */
                Screen48D380 *result = new Screen48D380;
                result->run(g_iniProfile);
                delete result;
            }
            if (countListA() != 2 && g_elemCount >= 3)
            {
                g_1b2e7c4 = 1;
                g_1b2ea34 = 0;
                runResult();
            }
            if (stage + 1 == stages)
            {
                runRankingScreen();
                runEnding();
            }
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            stage -= 1;
        }
        else if (code == 6)
        {
            stage = stages;
        }

        stage += 1;
    } while (stage < 4);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef40 = 1;
    return 2;
}

/* EZ2AC::runClubMix @0x417670 - integrated from wip/runclubmix.cpp, where it
   was parked at 31.17% on a four-way register permutation.

   INTEGRATED RATHER THAN CHASED, on the park's own instruction: it says the
   function is functionally complete and not to spend decomp time on it while a
   recompilation is the goal. Its one remaining idea - make cl coalesce the
   zero constant with the screen-pointer variable - was disproven inside the
   same note (hoisting a single `Screen48D32C *over = 0;` to the top and
   reusing it at all three sites gives byte-identical output).

   THE WALL, recorded so nobody re-derives it: a 4-way permutation of the same
   four long-lived values, same live ranges, same definition order.

       value      original   ours
       this       ebx        ebp
       0          esi        edi
       round      ebp        ebx
       1          edi        esi

   The knock-on is what makes the diff long rather than four names: in the
   original the zero register IS esi, which is also the screen-pointer
   variable, so a failed `new` needs no store at all - `cmp eax,esi; je` falls
   straight into the shared tail with esi already 0. With the zero in edi, ours
   must materialize the null pointer, so each of the three game-over sites
   grows a jump to a shared `xor esi,esi`.

   TWO FINDINGS FROM THIS PARK ALREADY LANDED IN src/: loadModeGds @0x4150d0
   and runEnding @0x415db0 are EZ2AC MEMBERS, not free functions.

   The two screens written out INLINE are deliberate - the original does not
   call runGameOver or runResult here, it repeats their three statements, and
   cl cross-jumps the three game-over copies into the single run/free/return
   tail at 0x41782a. And `stageResult` is deliberately read before it is ever
   written when the result screen is skipped (g_1b2ea28 != 0 on the first
   round); the original's local is uninitialized there too. */
// FUNCTION: EZ2AC_UNPACKED 0x00417670
int EZ2AC::runClubMix()
{
    int round;
    int code;
    int c;

    loadModeGds();
    round = 0;

    /* A do-while, not the `for (;;)` the park had: with the infinite form cl
       PEELS the first iteration, which duplicates the song-select call site
       and costs forty-four instructions. The call multiset catches it in one
       line - runSongSelect 1 against 2. */
    do
    {
        g_elemCount = round;

        if (runSongSelect() != 0)
        {
            return 0;
        }

        code = runMainGame();
        c = g_1b2ea28;

        if (g_1b2efb4 == 1)
        {
            if (g_elemCount >= 1 && c == 1)
            {
                Screen48D32C *over = new Screen48D32C;
                over->run(g_iniProfile);
                delete over;
                return 1;
            }
        }
        else if (g_elemCount >= 2 && c == 1)
        {
            Screen48D32C *over = new Screen48D32C;
            over->run(g_iniProfile);
            delete over;
            return 1;
        }

        if (code == 2)
        {
            /* Scoped to this block: its live range is what decides whether cl
               parks it in the frame slot (as the original does, keeping `this`
               in ebx) or in a callee-saved register. */
            int stageResult;

            if (c == 0)
            {
                Screen48D380 *result = new Screen48D380;
                stageResult = result->run(g_iniProfile);
                delete result;
            }

            if (stageResult == 1)
            {
                Screen48D32C *over = new Screen48D32C;
                over->run(g_iniProfile);
                delete over;
                return 1;
            }

            if (countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;

                if (g_1b2ef70 == 0)
                {
                    runRankingScreen();
                }

                g_1b2f024 = 0;
            }

            if (g_1b2ea34 == 1)
            {
                runModeIntro();
            }
        }
        else if (code == 5)
        {
            /* Two calls, not a ternary - cl cross-jumps them back into one, which
           is the shape the original has (and what the other four runners in
           this TU needed). */
        if (findListA(0) != 0)
            addToListA(1);
        else
            addToListA(0);
            round--;
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


/* EZ2AC::run5KeyMix @0x416b90 - 5KeyMix and 7StreetMix share this address: their
   bodies were identical, so the original's linker ICF-folded them. Both are
   written out - a folded alias must carry the real body - and both stay
   distinct functions, which is what keeps runGame's two switch arms from being
   cross-jumped into one.

   It is runClubMix's shape with runRubyMix's guard: the game-over test is
   reached only when countListA() == 1, and inside it g_1b2efb4 chooses whether
   the last round is 1 or 2. The two screens are written INLINE as in ClubMix,
   and the original resolves them differently at the two sites - the
   countListA() == 1 game over is a real call to 0x415d70 while the stageResult
   one is inlined - which is cl's per-site choice, not the source's.

   `stageResult` is read before it is ever written when the result screen is
   skipped (g_1b2ea28 != 0 on the first round); the original's local is
   uninitialized there too. */
// FUNCTION: EZ2AC_UNPACKED 0x00416b90 folded
int EZ2AC::run5KeyMix()
{
    int round;
    int code;
    int stageResult;
    int c;

    loadModeGds();
    round = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        code = runMainGame();

        if (countListA() == 1)
        {
            c = g_1b2ea28;
            if (g_1b2efb4 == 1)
            {
                if (g_elemCount >= 1 && c == 1)
                {
                    runGameOver();
                    return 1;
                }
            }
            else if (g_elemCount >= 2 && c == 1)
            {
                runGameOver();
                return 1;
            }
        }

        if (code == 2)
        {
            if (g_1b2ea28 == 0)
                {
                    Screen48D380 *result = new Screen48D380;
                    stageResult = result->run(g_iniProfile);
                    delete result;
                }

            if (stageResult == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }

            if (countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                    runRankingScreen();
                g_1b2f024 = 0;
            }

            if (g_1b2ea34 == 1)
                runModeIntro();
            else if (countListA() >= 2 && g_elemCount <= 1)
                runModeIntro();
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            round--;
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

/* EZ2AC::run7StreetMix @0x416b90 - 5KeyMix and 7StreetMix share this address: their
   bodies were identical, so the original's linker ICF-folded them. Both are
   written out - a folded alias must carry the real body - and both stay
   distinct functions, which is what keeps runGame's two switch arms from being
   cross-jumped into one.

   It is runClubMix's shape with runRubyMix's guard: the game-over test is
   reached only when countListA() == 1, and inside it g_1b2efb4 chooses whether
   the last round is 1 or 2. The two screens are written INLINE as in ClubMix,
   and the original resolves them differently at the two sites - the
   countListA() == 1 game over is a real call to 0x415d70 while the stageResult
   one is inlined - which is cl's per-site choice, not the source's.

   `stageResult` is read before it is ever written when the result screen is
   skipped (g_1b2ea28 != 0 on the first round); the original's local is
   uninitialized there too. */
// FUNCTION: EZ2AC_UNPACKED 0x00416b90 folded
int EZ2AC::run7StreetMix()
{
    int round;
    int code;
    int stageResult;
    int c;

    loadModeGds();
    round = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        code = runMainGame();

        if (countListA() == 1)
        {
            c = g_1b2ea28;
            if (g_1b2efb4 == 1)
            {
                if (g_elemCount >= 1 && c == 1)
                {
                    runGameOver();
                    return 1;
                }
            }
            else if (g_elemCount >= 2 && c == 1)
            {
                runGameOver();
                return 1;
            }
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

            if (countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                    runRankingScreen();
                g_1b2f024 = 0;
            }

            if (g_1b2ea34 == 1)
                runModeIntro();
            else if (countListA() >= 2 && g_elemCount <= 1)
                runModeIntro();
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
                addToListA(1);
            else
                addToListA(0);
            round--;
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

/* EZ2AC::runEz2Catch @0x4160e0 - the Ez2Catch mode loop. Same family as
   ClubMix but read cold: it diffs at only ~0.55 against it, and three things
   are genuinely different.

     - IT RUNS runCatchGame @0x416060, not runMainGame.
     - THERE IS NO code 5 ARM. A player rejoining does not repeat the round.
     - THE GAME-OVER TEST IS WRITTEN THREE TIMES, and the third one is DEAD.
       After the g_1b2efb4 branch has already tested `round >= 1 && c == 1` on
       one side and `round >= 2 && c == 1` on the other, the code tests
       `round >= 2 && c == 1` again - which cannot be true on either path,
       since the first arm would have taken it and the second jumped away.
       It is in the original and it is reproduced, because a port that deleted
       it would diverge on any future build of this source. Four game-over
       sites in the source; cl merges two of them, leaving the three blocks at
       0x41613b, 0x41625b and 0x416285.

   `stageResult` is initialized to zero here, unlike ClubMix and 5KeyMix where
   it is read uninitialized - and cl loads it from the round's slot, which is
   the same zero. */
// FUNCTION: EZ2AC_UNPACKED 0x004160e0
int EZ2AC::runEz2Catch()
{
    int round;
    int code;
    int stageResult;
    int c;

    loadModeGds();
    round = 0;
    stageResult = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        code = runCatchGame();
        c = g_1b2ea28;

        if (g_1b2efb4 == 1)
        {
            if (g_elemCount >= 1 && c == 1)
            {
                {
                    Screen48D32C *gameOver = new Screen48D32C;
                    gameOver->run(g_iniProfile);
                    delete gameOver;
                }
                return 1;
            }
        }
        else if (g_elemCount >= 2 && c == 1)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }

        /* Dead on both paths - see the header. */
        if (g_elemCount >= 2 && c == 1)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }

        if (code == 2)
        {
            if (c == 0)
                {
                    Screen48D380 *result = new Screen48D380;
                    stageResult = result->run(g_iniProfile);
                    delete result;
                }

            if (stageResult == 1)
            {
                {
                    Screen48D32C *o4 = new Screen48D32C;
                    o4->run(g_iniProfile);
                    delete o4;
                }
                return 1;
            }

            if (countListA() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
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

/* EZ2AC::runScratchMix @0x417c70 - the ScratchMix mode loop, and the most
   divergent of the family. Four things set it apart:

     - IT COUNTS LIST C, not list A. Every population test here is
       countListC() @0x4184a0.
     - THE GAME SCREEN IS INLINE. There is no runXxxGame call at all: the
       Screen48D47C construction is written out in the loop, which is how that
       class was identified in the first place.
     - THE RESULT SCREEN IS runGFResult @0x4162e0, the scratch-only one.
     - THE GAME-OVER GUARD IS TWO-PART AND SHARED. Reaching the last round
       runs the game-over screen only when the run is a single player, OR two
       players with g_1b2e7d8 >= 2; otherwise it returns 1 without one. Both
       arms of the g_1b2efb4 test carry their own copy of that guard, which is
       why it appears twice in the listing (0x417cee and 0x417e27).

   Its code 5 arm also does more than the others': as well as putting the
   player back on list A it calls addPlayer @0x418410 with the same id, and cl
   merges the two cdecl cleanups into one `add esp,8`.

   The ending block clears one global more than the rest of the family -
   g_1b2ef70 - which is the flag the ranking screen is gated on. */
// FUNCTION: EZ2AC_UNPACKED 0x00417c70
int EZ2AC::runScratchMix()
{
    int round;
    int code;

    loadModeGds();
    round = 0;
    do
    {
        g_elemCount = round;
        if (runSongSelect() != 0)
            return 0;

        {
            Screen48D47C *game = new Screen48D47C;
            code = game->run(g_iniProfile);
            delete game;
        }

        if (countListC() == 1)
        {
            if (g_1b2efb4 == 1)
            {
                if (g_elemCount >= 1 && g_1b2ea28 == 1)
                {
                    if (countListC() == 1 ||
                        (countListC() >= 2 && g_1b2e7d8 >= 2))
                        runGameOver();
                    return 1;
                }
            }
            else if (g_elemCount >= 2 && g_1b2ea28 == 1)
            {
                if (countListC() == 1 ||
                    (countListC() >= 2 && g_1b2e7d8 >= 2))
                    runGameOver();
                return 1;
            }
        }

        if (code == 3)
        {
            {
                Screen48D32C *gameOver = new Screen48D32C;
                gameOver->run(g_iniProfile);
                delete gameOver;
            }
            return 1;
        }

        if (code == 2)
        {
            if (g_1b2ea28 == 0)
                runGFResult();

            if (round <= 3 && countListC() < 2 && g_1b2ea34 == 0)
            {
                g_1b2f024 = 1;
                if (g_1b2ef70 == 0)
                    runRankingScreen();
                g_1b2f024 = 0;
            }

            if (g_1b2ea34 == 1)
                runModeIntro();
            else if (countListC() >= 2 && g_elemCount <= 1)
                runModeIntro();
        }
        else if (code == 5)
        {
            if (findListA(0) != 0)
            {
                addToListA(1);
                addPlayer(1);
            }
            else
            {
                addToListA(0);
                addPlayer(0);
            }
            round--;
        }

        round++;
    } while (round < 3);

    g_1b2ef58 = 0;
    g_1b2ead0 = 0;
    g_1b2e7dc = 0;
    g_1b2e5a4 = 0;
    g_1b2ea24 = 0;
    g_1b2e7c4 = 0;
    g_1b2ef70 = 0;
    g_1b2ef40 = 1;
    runEnding();
    return 2;
}
