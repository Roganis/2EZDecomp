/* SystemMenu - the operator test menu (see include/systemmenu.h). Only its
   leaves and its factory are reconstructed here; the two big per-frame methods
   are still STUBs in src/stubs.cpp. */

#include "systemmenu.h"
#include "kengineapp.h"  /* g_iniProfile */
#include "kprofile.h"
#include "kparser.h"
#include "kengineapp.h"
#include <string.h>
#include <stdio.h>
#include <windows.h>
#include "directors.h"   /* TestModeSink */
#include "bigbase.h"     /* Obj401780, DrawGroup */

/* src/misc.cpp / src/stubs.cpp - the graphics text colour and the engine's
   printf-to-screen (see src/testinput.cpp for the same pair). */
struct Obj406C00 { void setField12c(int v); };   /* 0x406c00 */
extern void drawTextf(KGraphics *g, int x, int y, const char *fmt, ...);

/* src/settings.cpp / src/ez2acapp.cpp - two settings the game option page
   publishes when it commits. */
extern int g_advSound;    /* 0x16963f8 */
extern int g_1696400;     /* 0x1696400 */

/* src/moderunner.cpp / src/classinfo.cpp - the screen singleton every page
   walks the live-instance lists through, and TestModeDirector's record. */
extern void *g_1b2e844;              /* 0x1b2e844 */
extern EzObjectInfo g_info_4afb70;   /* 0x4afb70 */

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classSystemMenu;   /* 0x4aec68 */
extern EzObjectInfo  g_info_4aec78;     /* 0x4aec78 */

// FUNCTION: EZ2AC_UNPACKED 0x00474310
KRuntimeClass *SystemMenu::getRuntimeClass()
{
    return &classSystemMenu;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474320
DrawGroup *SystemMenu::getObjectInfo()
{
    return &g_info_4aec78;
}

/* Empty - the pages it holds are plain storage inside the object. cl still
   emits the vptr store and the tail-jump to the folded base destructor. */
// FUNCTION: EZ2AC_UNPACKED 0x00474330
SystemMenu::~SystemMenu()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00474640
// SystemMenu::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`, ICF-folded across every class
   that has one; src/constants.cpp owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int SystemMenu::v5(int, int, int)
{
    return 0;
}

/* Publish the record, take a slot out of its pool and construct in place - the
   engine's standard new-plus-ctor shape, with the constructor inlined. */
// FUNCTION: EZ2AC_UNPACKED 0x004747a0
SystemMenu *createSystemMenu()
{
    g_objectRecord = &g_info_4aec78;
    return new SystemMenu;
}

/* One frame of the menu: the page title, then every row - dimmed when the row
   is disabled, recoloured again when it is the selected one, and followed by
   its current choice when the row is a setting rather than a caption. Each
   colour pair is written as two separate calls, which is what lets cl
   cross-jump them into the original's single call fed by two pushes. */
// FUNCTION: EZ2AC_UNPACKED 0x00474340
void SystemMenu::update()
{
    int i;

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
    drawTextf((KGraphics *)g_sub1b2f054, 20, 20, m_title[m_language]);

    for (i = 0; i < m_rowCount; i++)
    {
        MenuRow *row = &m_rows[i];
        int y = m_rowPitch * i + 0x50;

        if (row->enabled != 0)
        {
            ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        }
        else
        {
            ((Obj406C00 *)g_sub1b2f054)->setField12c(0x808080);
        }

        if (m_index == i)
        {
            if (m_altHighlight != 0)
            {
                ((Obj406C00 *)g_sub1b2f054)->setField12c(0xff);
            }
            else
            {
                ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffff);
            }
        }

        if (row->f0 == 0)
        {
            drawTextf((KGraphics *)g_sub1b2f054, 0x64, y,
                      row->label[m_language]);
        }
        else if (row->f0 == 1)
        {
            drawTextf((KGraphics *)g_sub1b2f054, 0x64, y, "%s",
                      row->label[m_language]);
            drawTextf((KGraphics *)g_sub1b2f054, m_choiceX + 0x64, m_choiceY + y,
                      "%s", row->choices[m_language][row->value]);
        }
    }
}

/* One poll of the menu. Service (2/5) steps to the next enabled row and 4
   steps back; Test (1/6) commits the selected row and 9 steps its value down.
   Both commit paths report through the owner's sink - onMenu(0, row, 0) for a
   caption row and onMenu(1, row, value) for a setting - and cl cross-jumps the
   two identical caption calls into one.

   NOTE the row pointer is taken ONCE at entry, before the cursor moves, while
   the value edit re-derives the row from the CURRENT m_index. The two can be
   different rows; that is what the original does. */
// FUNCTION: EZ2AC_UNPACKED 0x00474660
void SystemMenu::update2(int, int)
{
    MenuRow *row = &m_rows[m_index];

    if (g_bigInstance->getState(2) == 2 || g_bigInstance->getState(5) == 2)
    {
        nextRow();
    }

    if (g_bigInstance->getState(4) == 2)
    {
        do
        {
            m_index--;
            if (m_index < 0)
            {
                m_index = m_rowCount - 1;
            }
        } while (m_rows[m_index].enabled == 0);
    }

    if (g_bigInstance->getState(1) == 2 || g_bigInstance->getState(6) == 2)
    {
        if (row->f0 == 0)
        {
            ((TestPageSink *)m_owner)->onMenu(0, row->index, 0);
        }
        else if (row->f0 == 1)
        {
            MenuRow *cur = &m_rows[m_index];

            cur->value++;
            if (cur->value >= cur->choiceCount)
            {
                cur->value = 0;
            }
            ((TestPageSink *)m_owner)->onMenu(1, row->index, row->value);
        }
    }
    else if (g_bigInstance->getState(9) == 2)
    {
        if (row->f0 == 0)
        {
            ((TestPageSink *)m_owner)->onMenu(0, row->index, 0);
        }
        else if (row->f0 == 1)
        {
            prevChoice();
            ((TestPageSink *)m_owner)->onMenu(1, row->index, row->value);
        }
    }
}

/* Step the selected row's value down, wrapping round to its last choice. */
// FUNCTION: EZ2AC_UNPACKED 0x00474490
void SystemMenu::prevChoice()
{
    MenuRow *row = &m_rows[m_index];

    if (--row->value < 0)
    {
        row->value = row->choiceCount - 1;
    }
}

/* Move the cursor to the next ENABLED row, wrapping at the end. */
// FUNCTION: EZ2AC_UNPACKED 0x004744c0
void SystemMenu::nextRow()
{
    do
    {
        m_index++;
        if (m_index >= m_rowCount)
        {
            m_index = 0;
        }
    } while (m_rows[m_index].enabled == 0);
}

/* Give a row its label in the CURRENT language, and - the first time an index
   is used - open the row: bump the row count, mark it enabled and clear its
   choice list. m_language is re-read for the terminator because strncpy could
   have touched it. */
// FUNCTION: EZ2AC_UNPACKED 0x00474500
void SystemMenu::setLabel(int index, const char *label)
{
    MenuRow *row = &m_rows[index];

    strncpy(row->label[m_language], label, 0x40);
    row->label[m_language][0x3f] = 0;

    if (index >= m_rowCount)
    {
        m_rowCount = index + 1;
        row->enabled = 1;
        row->index = index;
        row->f0 = 0;
        row->choiceCount = 0;
        row->value = 0;
    }
}

/* One of a row's choices, again in the current language. A null string is
   ignored, which is how a caption row (count 0) gets no choices at all. Note
   the terminator is written at choiceCount, not at the slot just filled -
   which is what the original does, one instruction at a time. */
// FUNCTION: EZ2AC_UNPACKED 0x00474570
void SystemMenu::setChoice(int index, int slot, const char *text)
{
    MenuRow *row;

    if (text == 0)
    {
        return;
    }

    row = &m_rows[index];
    strncpy(row->choices[m_language][slot], text, 0x40);
    row->choices[m_language][row->choiceCount][0x3f] = 0;

    if (slot >= row->choiceCount)
    {
        row->choiceCount = slot + 1;
        row->f0 = 1;
    }
}

/* Lay one row down: its label, then each of the values it cycles through. The
   choice table is re-read every iteration because setChoice could have moved
   it. */
// FUNCTION: EZ2AC_UNPACKED 0x00474600
void SystemMenu::addEntry(int index, const MenuEntry *entry)
{
    int i;

    setLabel(index, entry->label);

    for (i = 0; i < entry->count; i++)
    {
        setChoice(index, i, entry->choices[i]);
    }
}

// VTABLE: EZ2AC_UNPACKED 0x0049e9b8
struct SystemMenu;

/* ---- TestLighting, the menu's LIGHTING page --------------------------- */

extern KRuntimeClass classTestLighting;   /* 0x4af268 */
extern EzObjectInfo  g_info_4af278;       /* 0x4af278 */

/* src/testinput.cpp - the caption language. */
extern int g_language;

// FUNCTION: EZ2AC_UNPACKED 0x00475d80
KRuntimeClass *TestLighting::getRuntimeClass()
{
    return &classTestLighting;
}

// FUNCTION: EZ2AC_UNPACKED 0x00475d90
DrawGroup *TestLighting::getObjectInfo()
{
    return &g_info_4af278;
}

/* Empty - the menu it built is pooled storage, not its own. Both vptrs are
   restored, which is what says the class really has two. */
// FUNCTION: EZ2AC_UNPACKED 0x00475da0
TestLighting::~TestLighting()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x004760c0
// TestLighting::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestLighting::v5(int, int, int)
{
    return 0;
}

/* Build the page's menu, point it at this page's caption language and open it
   on entry 2, then arm the three cabinet-lamp channels the page drives. */
// FUNCTION: EZ2AC_UNPACKED 0x00476020
TestLighting::TestLighting()
{
    m_menu = 0;
    buildMenu();
    m_menu->m_language = g_language;
    m_menu->m_index = 2;
    g_bigInstance->m4189e0(1);
    g_bigInstance->m418a50(1);
    g_bigInstance->m418af0(1);
}

// FUNCTION: EZ2AC_UNPACKED 0x004760e0
TestLighting *createTestLighting()
{
    g_objectRecord = &g_info_4af278;
    return new TestLighting;
}

/* Every channel in the table off, then the one the menu's row selects on -
   except row 5, which lights the first four together. The `push 1` above the
   branch is cl merging the shared second argument across both arms. It never
   reads `this`, but its call site @0x475ec9 sets ecx, so it is a member. */
// FUNCTION: EZ2AC_UNPACKED 0x00475dd0
void TestLighting::setLamps(int index, int count, const int *lamps)
{
    int i;
    int ch;

    for (i = 0; i < count; i++)
    {
        g_bigInstance->setOutput(lamps[i], 0);
    }

    ch = lamps[index];
    if (ch != -1)
    {
        if (index == 5)
        {
            g_bigInstance->setOutput(0, 1);
            g_bigInstance->setOutput(1, 1);
            g_bigInstance->setOutput(2, 1);
            g_bigInstance->setOutput(3, 1);
        }
        else
        {
            g_bigInstance->setOutput(ch, 1);
        }
    }
}

/* The menu reporting one of the page's three rows. Row 0 drives one lamp
   straight from its value, row 1 hands the whole channel table to setLamps,
   row 2 leaves the page. As in the bookkeeping page, `this` is the SINK, so
   the menu is reached through the sink-relative view. */
// FUNCTION: EZ2AC_UNPACKED 0x00475e60
void TestLightingSink::onMenu(int, int what, int)
{
    TestPageSinkFields *f = (TestPageSinkFields *)this;

    if (what == 0)
    {
        g_bigInstance->setOutput(4, f->menu->m_rows[0].value);
    }
    else if (what == 1)
    {
        int lamps[5] = { -1, 0, 1, 2, 3 };

        ((TestLighting *)((char *)this - 0x10))->setLamps(f->menu->m_rows[1].value, 5,
                                                          lamps);
    }
    else if (what == 2)
    {
        DrawNode *node;

        g_bigInstance->setFlags(0);
        delete f->menu;
        node = 0;
        ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
        ((TestModeSink *)((char *)node + 0x10))->onMenu(
            2, (int)((char *)this - 0x10), 0);
        delete (TestLighting *)((char *)this - 0x10);
    }
}

// VTABLE: EZ2AC_UNPACKED 0x0049f484 EzObject
struct TestLighting;

// VTABLE: EZ2AC_UNPACKED 0x0049f480 TestLightingSink
struct TestLighting;

// VTABLE: EZ2AC_UNPACKED 0x0049ec9c
struct TestPageSink;

/* ---- TestBookkeeping, the menu's BOOKKEEPING page ---------------------- */

/* src/strutil.cpp - the game's chdir pair. */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */

extern KRuntimeClass classTestBookkeeping;   /* 0x4aeca4 */
extern EzObjectInfo  g_info_4aecb4;          /* 0x4aecb4 */

/* src/moderunner.cpp and src/ez2acapp.cpp - the three persistent counters the
   page reports, named by the .ini keys loadCounters @0x4156c0 reads them with. */
extern int g_totalCoin;      /* 0x16963dc */
extern int g_serviceCoin;    /* 0x16963e8 */
extern int g_totalPlay;      /* 0x16963cc */

// FUNCTION: EZ2AC_UNPACKED 0x00474840
KRuntimeClass *TestBookkeeping::getRuntimeClass()
{
    return &classTestBookkeeping;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474850
DrawGroup *TestBookkeeping::getObjectInfo()
{
    return &g_info_4aecb4;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474860
TestBookkeeping::~TestBookkeeping()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00474e20
// TestBookkeeping::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestBookkeeping::v5(int, int, int)
{
    return 0;
}

/* Snapshot the counters, then build the menu and open it on entry 6. */
// FUNCTION: EZ2AC_UNPACKED 0x00474d80
TestBookkeeping::TestBookkeeping()
{
    m_totalCoin = g_totalCoin;
    m_serviceCoin = g_serviceCoin;
    m_totalPlay = g_totalPlay;
    m_coins = m_totalCoin + m_serviceCoin;
    m_menu = 0;
    buildMenu();
    m_menu->m_language = g_language;
    m_menu->m_index = 6;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474e40
TestBookkeeping *createTestBookkeeping()
{
    g_objectRecord = &g_info_4aecb4;
    return new TestBookkeeping;
}

/* Write the page's counters back to the machine .ini and re-read it. The
   0x5134 frame is the 260-byte name buffer plus a whole KParser (0x502c) plus
   the cookie, which is what dates the parser local. Note it writes the SUM at
   +0x14 under the TotalCoin key - reproduced as the original has it. */
// FUNCTION: EZ2AC_UNPACKED 0x00474880
void TestBookkeeping::saveCounters()
{
    char name[260];

    sprintf(name, "%s.ini", g_app->className());
    g_iniProfile->setInt("TotalCoin", m_coins);
    g_iniProfile->setInt("ServiceCoin", m_serviceCoin);
    g_iniProfile->writeFile(name);

    KParser parser;
    parser.m4133d0(name);
}

/* Publish the four snapshotted counters into the first four rows' single
   choice string. One 128-byte buffer, reused four times - the frame is 0x84,
   i.e. the buffer plus the cookie and nothing else. */
// FUNCTION: EZ2AC_UNPACKED 0x00474970
void TestBookkeeping::refreshCounters()
{
    char buf[128];

    // STRING: EZ2AC_UNPACKED 0x00488c04
    sprintf(buf, "%d", m_coins);
    m_menu->setChoice(0, 0, buf);
    sprintf(buf, "%d", m_totalCoin);
    m_menu->setChoice(1, 0, buf);
    sprintf(buf, "%d", m_serviceCoin);
    m_menu->setChoice(2, 0, buf);
    sprintf(buf, "%d", m_totalPlay);
    m_menu->setChoice(3, 0, buf);
}

/* What the page's "clear all rankings" arm calls: wipe every .bin out of the
   two directories the rankings live in.

   NOTE for whoever reconstructs TestBookkeepingSink::onMenu @0x474b10: its
   call site does `lea ecx,[esi-0x10]; call 0x474a40`, so this is really a
   __thiscall member of TestBookkeeping that never touches `this`. The body is
   byte-identical either way, but the CALLER will only emit that `lea` if the
   declaration is a member - move it into the class at that point.
 FindClose runs even when the search
   never opened, which is why the guard only skips the loop. The IAT slots are
   hoisted into registers because the calls sit in a loop. */
// FUNCTION: EZ2AC_UNPACKED 0x00474a40
void TestBookkeeping::deleteRankingFiles()
{
    WIN32_FIND_DATAA fd;
    HANDLE h;

    // STRING: EZ2AC_UNPACKED 0x0049d05c
    changeDir("sound\\", 1);
    // STRING: EZ2AC_UNPACKED 0x0049cfe8
    h = FindFirstFileA("*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
            {
                DeleteFileA(fd.cFileName);
            }
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    popDirectory();

    // STRING: EZ2AC_UNPACKED 0x00498894
    changeDir("system\\ranking\\", 1);
    h = FindFirstFileA("*.bin", &fd);
    if (h != 0)
    {
        do
        {
            if (fd.dwFileAttributes != FILE_ATTRIBUTE_DIRECTORY)
            {
                DeleteFileA(fd.cFileName);
            }
        } while (FindNextFileA(h, &fd));
    }
    FindClose(h);
    popDirectory();
}

/* The menu reporting one of the page's three actions. `this` is the SINK, so
   the page's own fields are reached through a sink-relative view; only the
   METHOD calls need the page pointer, and only arm 5 materializes it once. */
// FUNCTION: EZ2AC_UNPACKED 0x00474b10
void TestBookkeepingSink::onMenu(int, int what, int)
{
    TestBookkeepingSinkFields *f = (TestBookkeepingSinkFields *)this;

    if (what == 5)
    {
        TestBookkeeping *page;
        int saved;

        f->m_coins = 0;
        f->m_totalCoin = 0;
        f->m_serviceCoin = 0;
        f->m_totalPlay = 0;
        g_totalCoin = 0;
        g_serviceCoin = 0;
        g_totalPlay = 0;

        saved = f->m_menu->m_language;
        f->m_menu->m_language = 0;
        page = (TestBookkeeping *)((char *)this - 0x10);
        page->refreshCounters();
        f->m_menu->m_language = 1;
        page->refreshCounters();
        f->m_menu->m_language = saved;
        page->saveCounters();
    }
    else if (what == 4)
    {
        ((TestBookkeeping *)((char *)this - 0x10))->deleteRankingFiles();
    }
    else if (what == 6)
    {
        DrawNode *node;

        delete f->m_menu;
        node = 0;
        ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
        ((TestModeSink *)((char *)node + 0x10))->onMenu(
            2, (int)((char *)this - 0x10), 0);
        delete (TestBookkeeping *)((char *)this - 0x10);
    }
}

// VTABLE: EZ2AC_UNPACKED 0x0049eca4 EzObject
struct TestBookkeeping;

// VTABLE: EZ2AC_UNPACKED 0x0049eca0 TestBookkeepingSink
struct TestBookkeeping;

/* ---- TestGameOption, the menu's GAME OPTION page ----------------------- */

extern KRuntimeClass classTestGameOption;    /* 0x4af188 */
extern EzObjectInfo  g_info_4af198;          /* 0x4af198 */

// FUNCTION: EZ2AC_UNPACKED 0x00474ea0
KRuntimeClass *TestGameOption::getRuntimeClass()
{
    return &classTestGameOption;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474eb0
DrawGroup *TestGameOption::getObjectInfo()
{
    return &g_info_4af198;
}

// FUNCTION: EZ2AC_UNPACKED 0x00474ec0
TestGameOption::~TestGameOption()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00475330
// TestGameOption::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestGameOption::v5(int, int, int)
{
    return 0;
}

/* Read the two machine settings the page edits, fold CreditCoins into the
   0..3 list index, clamp both, then build the menu and open it on entry 3. */
// FUNCTION: EZ2AC_UNPACKED 0x00475220
TestGameOption::TestGameOption()
{
    m_advSound = 1;
    f24 = 0;
    g_iniProfile->getInt("CreditCoins", &m_creditCoins);
    g_iniProfile->getInt("AdvSound", &m_advSound);

    if (m_creditCoins == 0)
        m_coinIndex = 0;
    else if (m_creditCoins == 1)
        m_coinIndex = 1;
    else if (m_creditCoins == 4)
        m_coinIndex = 2;
    else if (m_creditCoins == 5)
        m_coinIndex = 3;

    if (m_coinIndex < 0)
        m_coinIndex = 0;
    if (m_coinIndex > 3)
        m_coinIndex = 3;
    if (m_advSound < 0)
        m_advSound = 0;
    if (m_advSound > 1)
        m_advSound = 1;

    m_menu = 0;
    buildMenu();
    m_menu->m_language = g_language;
    m_menu->m_index = 3;
}

// FUNCTION: EZ2AC_UNPACKED 0x00475350
TestGameOption *createTestGameOption()
{
    g_objectRecord = &g_info_4af198;
    return new TestGameOption;
}

// VTABLE: EZ2AC_UNPACKED 0x0049ef6c EzObject
struct TestGameOption;

// VTABLE: EZ2AC_UNPACKED 0x0049ef68 TestGameOptionSink
struct TestGameOption;

/* ---- TestLighting's menu ---------------------------------------------- */

/* Placeholder storage for the row records and the two page titles: only the
   addresses have to line up, and each record is pushed at its exact base. */
// GLOBAL: EZ2AC_UNPACKED 0x0049f398
MenuEntry g_menu_49f398;
// GLOBAL: EZ2AC_UNPACKED 0x0049f3a4
MenuEntry g_menu_49f3a4;
// GLOBAL: EZ2AC_UNPACKED 0x004af2b4
MenuEntry g_menu_4af2b4;
// GLOBAL: EZ2AC_UNPACKED 0x004af2d8
MenuEntry g_menu_4af2d8;
// GLOBAL: EZ2AC_UNPACKED 0x004af2ec
MenuEntry g_menu_4af2ec;
// GLOBAL: EZ2AC_UNPACKED 0x004af310
MenuEntry g_menu_4af310;
// GLOBAL: EZ2AC_UNPACKED 0x004af2a4
char *g_lightingTitleKor;
// GLOBAL: EZ2AC_UNPACKED 0x004af2a8
char *g_lightingTitleEng;

// FUNCTION: EZ2AC_UNPACKED 0x00475f40
void TestLighting::buildMenu()
{
    m_menu = createSystemMenu();
    /* The cast pointer NAMED in a local, as the original stores it
       (the inline cast was one instruction off, 2 points). */
    TestLightingSink *owner = (TestLightingSink *)this;
    m_menu->m_owner = owner;

    m_menu->m_language = 0;
    strcpy(m_menu->m_title[m_menu->m_language], g_lightingTitleKor);
    m_menu->addEntry(0, &g_menu_4af2b4);
    m_menu->addEntry(1, &g_menu_4af2d8);
    m_menu->addEntry(2, &g_menu_49f398);

    m_menu->m_language = 1;
    strcpy(m_menu->m_title[m_menu->m_language], g_lightingTitleEng);
    m_menu->addEntry(0, &g_menu_4af2ec);
    m_menu->addEntry(1, &g_menu_4af310);
    m_menu->addEntry(2, &g_menu_49f3a4);
}

/* ---- TestBookkeeping's menu ---------------------------------------------- */

/* Placeholder storage for the row records and the two page titles: only the
   addresses have to line up, and each record is pushed at its exact base. */
// GLOBAL: EZ2AC_UNPACKED 0x0049eb70
MenuEntry g_menu_49eb70;
// GLOBAL: EZ2AC_UNPACKED 0x0049eb7c
MenuEntry g_menu_49eb7c;
// GLOBAL: EZ2AC_UNPACKED 0x0049eb88
MenuEntry g_menu_49eb88;
// GLOBAL: EZ2AC_UNPACKED 0x0049eb94
MenuEntry g_menu_49eb94;
// GLOBAL: EZ2AC_UNPACKED 0x0049eba0
MenuEntry g_menu_49eba0;
// GLOBAL: EZ2AC_UNPACKED 0x0049ebac
MenuEntry g_menu_49ebac;
// GLOBAL: EZ2AC_UNPACKED 0x004aecec
MenuEntry g_menu_4aecec;
// GLOBAL: EZ2AC_UNPACKED 0x004aecfc
MenuEntry g_menu_4aecfc;
// GLOBAL: EZ2AC_UNPACKED 0x004aed0c
MenuEntry g_menu_4aed0c;
// GLOBAL: EZ2AC_UNPACKED 0x004aed1c
MenuEntry g_menu_4aed1c;
// GLOBAL: EZ2AC_UNPACKED 0x004aed2c
MenuEntry g_menu_4aed2c;
// GLOBAL: EZ2AC_UNPACKED 0x004aed3c
MenuEntry g_menu_4aed3c;
// GLOBAL: EZ2AC_UNPACKED 0x004aed4c
MenuEntry g_menu_4aed4c;
// GLOBAL: EZ2AC_UNPACKED 0x004aed5c
MenuEntry g_menu_4aed5c;
// GLOBAL: EZ2AC_UNPACKED 0x004aece0
char *g_bookkeepingTitleKor;
// GLOBAL: EZ2AC_UNPACKED 0x004aece4
char *g_bookkeepingTitleEng;

// FUNCTION: EZ2AC_UNPACKED 0x00474bf0
void TestBookkeeping::buildMenu()
{
    m_menu = createSystemMenu();
    /* The cast pointer NAMED in a local, as the original stores it
       (the inline cast was one instruction off, 2 points). */
    TestBookkeepingSink *owner = (TestBookkeepingSink *)this;
    m_menu->m_owner = owner;

    m_menu->m_language = 0;
    strcpy(m_menu->m_title[m_menu->m_language], g_bookkeepingTitleKor);
    m_menu->addEntry(0, &g_menu_4aecec);
    m_menu->addEntry(1, &g_menu_4aecfc);
    m_menu->addEntry(2, &g_menu_4aed0c);
    m_menu->addEntry(3, &g_menu_4aed1c);
    m_menu->addEntry(4, &g_menu_49eb70);
    m_menu->addEntry(5, &g_menu_49eb7c);
    m_menu->addEntry(6, &g_menu_49eb88);
    refreshCounters();

    m_menu->m_language = 1;
    strcpy(m_menu->m_title[m_menu->m_language], g_bookkeepingTitleEng);
    m_menu->addEntry(0, &g_menu_4aed2c);
    m_menu->addEntry(1, &g_menu_4aed3c);
    m_menu->addEntry(2, &g_menu_4aed4c);
    m_menu->addEntry(3, &g_menu_4aed5c);
    m_menu->addEntry(4, &g_menu_49eb94);
    m_menu->addEntry(5, &g_menu_49eba0);
    m_menu->addEntry(6, &g_menu_49ebac);
    refreshCounters();

    m_menu->m_rows[0].enabled = 0;
    m_menu->m_rows[1].enabled = 0;
    m_menu->m_rows[2].enabled = 0;
    m_menu->m_rows[3].enabled = 0;
}

/* ---- TestGameOption's menu ---------------------------------------------- */

/* Placeholder storage for the row records and the two page titles: only the
   addresses have to line up, and each record is pushed at its exact base. */
// GLOBAL: EZ2AC_UNPACKED 0x0049ee60
MenuEntry g_menu_49ee60;
// GLOBAL: EZ2AC_UNPACKED 0x0049ee6c
MenuEntry g_menu_49ee6c;
// GLOBAL: EZ2AC_UNPACKED 0x0049ee78
MenuEntry g_menu_49ee78;
// GLOBAL: EZ2AC_UNPACKED 0x0049ee84
MenuEntry g_menu_49ee84;
// GLOBAL: EZ2AC_UNPACKED 0x004af1dc
MenuEntry g_menu_4af1dc;
// GLOBAL: EZ2AC_UNPACKED 0x004af1f0
MenuEntry g_menu_4af1f0;
// GLOBAL: EZ2AC_UNPACKED 0x004af20c
MenuEntry g_menu_4af20c;
// GLOBAL: EZ2AC_UNPACKED 0x004af220
MenuEntry g_menu_4af220;
// GLOBAL: EZ2AC_UNPACKED 0x004af1c4
char *g_gameoptionTitleKor;
// GLOBAL: EZ2AC_UNPACKED 0x004af1c8
char *g_gameoptionTitleEng;

// FUNCTION: EZ2AC_UNPACKED 0x004750e0
void TestGameOption::buildMenu()
{
    m_menu = createSystemMenu();
    /* The cast pointer NAMED in a local, as the original stores it
       (the inline cast was one instruction off, 2 points). */
    TestGameOptionSink *owner = (TestGameOptionSink *)this;
    m_menu->m_owner = owner;

    m_menu->m_language = 0;
    strcpy(m_menu->m_title[m_menu->m_language], g_gameoptionTitleKor);
    m_menu->addEntry(0, &g_menu_4af1dc);
    m_menu->addEntry(1, &g_menu_4af1f0);
    m_menu->addEntry(2, &g_menu_49ee60);
    m_menu->addEntry(3, &g_menu_49ee6c);

    m_menu->m_language = 1;
    strcpy(m_menu->m_title[m_menu->m_language], g_gameoptionTitleEng);
    m_menu->addEntry(0, &g_menu_4af20c);
    m_menu->addEntry(1, &g_menu_4af220);
    m_menu->addEntry(2, &g_menu_49ee78);
    m_menu->addEntry(3, &g_menu_49ee84);

    f28 = 0;
    m_menu->m_rows[0].value = m_coinIndex;
    m_menu->m_rows[1].value = m_advSound;
}

/* ---- the test menu's OWN page ------------------------------------------ */

/* Placeholder storage for the eight rows of each language and the two page
   titles - the same convention as the pages above. */
// GLOBAL: EZ2AC_UNPACKED 0x004afbac
MenuEntry g_menu_4afbac;
// GLOBAL: EZ2AC_UNPACKED 0x0049f7e0
MenuEntry g_menu_49f7e0;
// GLOBAL: EZ2AC_UNPACKED 0x0049f7ec
MenuEntry g_menu_49f7ec;
// GLOBAL: EZ2AC_UNPACKED 0x0049f7f8
MenuEntry g_menu_49f7f8;
// GLOBAL: EZ2AC_UNPACKED 0x0049f804
MenuEntry g_menu_49f804;
// GLOBAL: EZ2AC_UNPACKED 0x0049f810
MenuEntry g_menu_49f810;
// GLOBAL: EZ2AC_UNPACKED 0x0049f81c
MenuEntry g_menu_49f81c;
// GLOBAL: EZ2AC_UNPACKED 0x0049f828
MenuEntry g_menu_49f828;
// GLOBAL: EZ2AC_UNPACKED 0x0049f834
MenuEntry g_menu_49f834;
// GLOBAL: EZ2AC_UNPACKED 0x0049f840
MenuEntry g_menu_49f840;
// GLOBAL: EZ2AC_UNPACKED 0x0049f84c
MenuEntry g_menu_49f84c;
// GLOBAL: EZ2AC_UNPACKED 0x0049f858
MenuEntry g_menu_49f858;
// GLOBAL: EZ2AC_UNPACKED 0x0049f864
MenuEntry g_menu_49f864;
// GLOBAL: EZ2AC_UNPACKED 0x0049f870
MenuEntry g_menu_49f870;
// GLOBAL: EZ2AC_UNPACKED 0x0049f87c
MenuEntry g_menu_49f87c;
/* Cleared every frame by the page above; nothing reconstructed yet reads it. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea78
int g_1b2ea78;

extern void *g_1b2e844;   /* 0x1b2e844 (src/moderunner.cpp) */

// GLOBAL: EZ2AC_UNPACKED 0x004afb9c
char *g_testModeTitleKor;
// GLOBAL: EZ2AC_UNPACKED 0x004afba0
char *g_testModeTitleEng;

/* The page's per-frame half. The menu is built lazily on the first frame - the
   flag at +0x14 is what the constructor leaves set - and the language and the
   opening row are pushed into it right afterwards. Service held down (input 0
   reading 2) hands control back to the sound bank instead and skips the whole
   thing. */
// FUNCTION: EZ2AC_UNPACKED 0x004765e0
void TestModeDirector::update2(int, int)
{
    g_1b2ea78 = 0;

    if (g_bigInstance->getState(0) == 2)
    {
        ((SoundBank *)g_1b2e844)->v0(0);
        return;
    }

    if (f14 == 1)
    {
        f14 = 0;
        buildMenu();
        m_menu->m_language = g_language;
        m_menu->m_index = f20;
    }
}

/* The root page of the operator menu: row 0 is the language toggle - the SAME
   record in both languages, since it names both in one string - and rows 1..7
   are the seven pages. The cursor opens on whatever g_language says, which is
   also the row the toggle drives. */
// FUNCTION: EZ2AC_UNPACKED 0x00476260
void TestModeDirector::buildMenu()
{
    m_menu = createSystemMenu();
    /* The cast pointer NAMED in a local, as the original stores it
       (the inline cast was one instruction off, 2 points). */
    TestModeSink *owner = (TestModeSink *)this;
    m_menu->m_owner = owner;

    m_menu->m_language = 0;
    strcpy(m_menu->m_title[m_menu->m_language], g_testModeTitleKor);
    m_menu->addEntry(0, &g_menu_4afbac);
    m_menu->addEntry(1, &g_menu_49f7e0);
    m_menu->addEntry(2, &g_menu_49f7ec);
    m_menu->addEntry(3, &g_menu_49f7f8);
    m_menu->addEntry(4, &g_menu_49f804);
    m_menu->addEntry(5, &g_menu_49f810);
    m_menu->addEntry(6, &g_menu_49f81c);
    m_menu->addEntry(7, &g_menu_49f828);

    m_menu->m_language = 1;
    strcpy(m_menu->m_title[m_menu->m_language], g_testModeTitleEng);
    m_menu->addEntry(0, &g_menu_4afbac);
    m_menu->addEntry(1, &g_menu_49f834);
    m_menu->addEntry(2, &g_menu_49f840);
    m_menu->addEntry(3, &g_menu_49f84c);
    m_menu->addEntry(4, &g_menu_49f858);
    m_menu->addEntry(5, &g_menu_49f864);
    m_menu->addEntry(6, &g_menu_49f870);
    m_menu->addEntry(7, &g_menu_49f87c);

    m_menu->m_rows[0].value = g_language;
}

/* The test-menu director itself: put the cabinet into test state - the three
   lamp channels off, both test-mode latches and the credit count cleared,
   all four flag words dropped, the four button lamps lit - clamp the caption
   language to its two values, then build the root page and open it on row 0
   in that language. The vptr dance the prologue shows (the abstract sink's
   table at +0x10, then this class's two) is TestPageSink's own constructor
   inlined ahead of the body. */
// FUNCTION: EZ2AC_UNPACKED 0x004766e0
TestModeDirector::TestModeDirector()
{
    m_menu = 0;
    f14 = 0;
    f18 = 0;
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->testMode1 = 0;
    g_bigInstance->testMode2 = 0;
    g_bigInstance->have = 0;
    g_bigInstance->setFlags(0);
    g_bigInstance->setOutput(8, 1);
    g_bigInstance->setOutput(9, 1);
    g_bigInstance->setOutput(10, 1);
    g_bigInstance->setOutput(13, 1);
    if (g_language < 0)
        g_language = 0;
    else if (g_language > 1)
        g_language = 1;
    buildMenu();
    m_menu->m_language = g_language;
    m_menu->m_index = 0;
}
