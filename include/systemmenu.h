#ifndef SYSTEMMENU_H
#define SYSTEMMENU_H

#include "panels.h"      /* EzObject, KRuntimeClass, DrawGroup */
#include "ezscreen.h"    /* EzPooled, g_objectRecord */
#include "bigbase.h"     /* g_bigInstance */
#include "directors.h"   /* TestPageSink - the abstract page-sink base */

/* SystemMenu - the operator test menu itself, the page list the Test button
   brings up. Named by its KRuntimeClass @0x4aec68 and sized 0x71330 by its
   record @0x4aec78 (count 2, flags 6).

   Its update2 @0x474660 addresses the block at +0x1c with `index * 0x3894`, so
   the bulk of the object is a table of 0x3894-byte page entries and +0x14 is
   the cursor into it. The five words past the table are what the constructor
   sets: two of them to constants (0x20 and 0x118) and three to zero.

   The constructor is INLINE because the original has none of its own - its
   whole body is expanded into the factory @0x4747a0, which is the only thing
   that builds one. */
/* One drawn row of a page. The size is self-checking twice over: the labels
   and the choices are both indexed by the menu's language, the choice block is
   [2][112][0x40] = 0x3800 bytes at +0x94 (so the row is exactly 0x3894), and
   32 rows at +0x1c end exactly where the page titles begin at +0x7129c.
   setLabel @0x474500 and setChoice @0x474570 write every field here. */
struct MenuRow
{
    int  f0;                        /* 0x0000 - 1 once the row has choices */
    int  index;                     /* 0x0004 */
    int  enabled;                   /* 0x0008 */
    char label[2][0x40];            /* 0x000c */
    int  choiceCount;               /* 0x008c */
    int  value;                     /* 0x0090 - which choice is selected */
    char choices[2][112][0x40];     /* 0x0094 .. 0x3894 */
};

/* One row of a page as its BUILDER describes it: a label and, when the row is
   a setting rather than a caption, the list of values it cycles through. */
struct MenuEntry
{
    const char  *label;    /* 0x00 */
    int          count;    /* 0x04 - how many choices */
    const char **choices;  /* 0x08 */
};

struct SystemMenu : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();     /* 0x474310 */
    virtual DrawGroup     *getObjectInfo();       /* 0x474320 */
    virtual void           update();              /* 0x474340 */
    virtual void           update2(int a, int b); /* 0x474660 */
    virtual int            v5(int, int, int);     /* 0x401770 (folded) */

    SystemMenu();
    ~SystemMenu();                                /* 0x474330 */

    /* Set row `index` to a menu entry: its label, and the list of choices the
       entry offers (@0x474500 takes the label, @0x474570 one choice each). */
    void addEntry(int index, const struct MenuEntry *entry); /* 0x474600 */
    void setLabel(int index, const char *label);            /* 0x474500 */
    void setChoice(int index, int slot, const char *text);  /* 0x474570 */
    /* The two navigation steps update2 drives: step the selected row's value
       down (wrapping to the last choice) and move the cursor to the next
       ENABLED row. */
    void prevChoice();                                      /* 0x474490 */
    void nextRow();                                         /* 0x4744c0 */

    /* The page that owns this menu, as its SINK sub-object: every owner's
       builder does `lea ecx,[this+0x10]; mov [menu+0x10],ecx` (@0x474bfc). */
    void *m_owner;                   /* 0x10 */
    int  m_index;                    /* 0x14 - which row is selected */
    int  m_rowCount;                 /* 0x18 - how many rows are in use */
    MenuRow m_rows[32];              /* 0x1c .. 0x7129c */
    /* The page title, one per language - and 0x7129c + 2 * 0x40 is exactly
       0x7131c, which is what says there are two. */
    char m_title[2][0x40];           /* 0x7129c */
    /* Picks which of two colours the SELECTED row is drawn in. */
    int  m_altHighlight;             /* 0x7131c */
    int  m_rowPitch;                 /* 0x71320 - always 0x20 */
    int  m_language;                 /* 0x71324 - the caption language */
    int  m_choiceX;                  /* 0x71328 - always 0x118 */
    int  m_choiceY;                  /* 0x7132c */
};

inline SystemMenu::SystemMenu()
{
    m_rowCount = 0;
    m_index = 0;
    m_altHighlight = 0;
    m_language = 0;
    m_choiceY = 0;
    m_rowPitch = 0x20;
    m_choiceX = 0x118;
}

SystemMenu *createSystemMenu();                   /* 0x4747a0 */

/* ---- the menu's PAGES -------------------------------------------------

   Every page is `EzObject` plus a second polymorphic base at +0x10 that the
   menu calls back through. That base is ABSTRACT: its own vftable @0x49ec9c
   holds `_purecall`, and each page's constructor stores it at +0x10 before
   overwriting it with the page's own - which is exactly what an ordinary
   (non-novtable) base with a pure virtual does.

   The page's override sits in its secondary vftable with NO adjustor thunk in
   front of it, so it is compiled for the SINK's `this`, not the page's. That
   is modelled by giving each page its own __declspec(novtable) sink derived
   from the abstract one: novtable keeps the intermediate from storing a vptr
   of its own (the original stores exactly two), and declaring the override
   there is what puts the plain address in the slot. */
/* TestPageSink itself is defined in include/directors.h, where
   TestModeDirector's own sink derives from it too. */

/* Sink+4 is the page's own +0x14: the override reads the menu pointer through
   its own `this`, so the page's fields appear 0x10 lower. A view, not a
   layout. */
struct TestPageSinkFields
{
    char        _pad0[4];
    SystemMenu *menu;                /* sink+0x04 = page+0x14 */
};

struct __declspec(novtable) TestLightingSink : TestPageSink
{
    virtual void onMenu(int a, int what, int c);      /* 0x475e60 */
};

/* TestLighting - the test menu's LIGHTING page. KRuntimeClass @0x4af268,
   record @0x4af278 sizes it 0x18: the root, the sink's vptr and the menu. */
struct TestLighting : EzObject, TestLightingSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();     /* 0x475d80 */
    virtual DrawGroup     *getObjectInfo();       /* 0x475d90 */
    virtual void           update();              /* 0x402880 (STUB, folded) */
    virtual void           update2(int a, int b); /* 0x475dc0 (STUB, folded) */
    virtual int            v5(int, int, int);     /* 0x401770 (folded) */

    TestLighting();                               /* 0x476020 */
    ~TestLighting();                              /* 0x475da0 */

    void buildMenu();                             /* 0x475f40 */
    /* Drive the cabinet lamps: every channel in the table off, then the one
       the menu's row selects on. */
    void setLamps(int index, int count, const int *lamps); /* 0x475dd0 */

    SystemMenu *m_menu;                           /* 0x14 */
};

TestLighting *createTestLighting();               /* 0x4760e0 */

/* What TestBookkeepingSink::onMenu writes through its OWN `this`: the page's
   four counters and its menu, 0x10 lower than they sit in the page. A view,
   not the base's layout - and it is what keeps the sink in esi where the
   original keeps it (a `page` local hoists into esi instead and costs 30%). */
struct TestBookkeepingSinkFields
{
    char        _pad0[4];
    int         m_coins;         /* sink+0x04 = page+0x14 */
    int         m_totalCoin;     /* sink+0x08 */
    int         m_serviceCoin;   /* sink+0x0c */
    int         m_totalPlay;     /* sink+0x10 */
    char        _pad14[4];
    SystemMenu *m_menu;          /* sink+0x18 = page+0x28 */
};

struct __declspec(novtable) TestBookkeepingSink : TestPageSink
{
    virtual void onMenu(int a, int what, int c);      /* 0x474b10 */
};

/* TestBookkeeping - the test menu's BOOKKEEPING page. KRuntimeClass @0x4aeca4,
   record @0x4aecb4 sizes it 0x2c. It snapshots the three persistent counters
   on entry and its sink's "clear" arm (what == 5) zeroes both the snapshot and
   the counters themselves - which is what names the four words. */
struct TestBookkeeping : EzObject, TestBookkeepingSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();     /* 0x474840 */
    virtual DrawGroup     *getObjectInfo();       /* 0x474850 */
    virtual void           update();              /* 0x402880 (STUB, folded) */
    virtual void           update2(int a, int b); /* 0x475dc0 (STUB, folded) */
    virtual int            v5(int, int, int);     /* 0x401770 (folded) */

    TestBookkeeping();                            /* 0x474d80 */
    ~TestBookkeeping();                           /* 0x474860 */

    void buildMenu();                             /* 0x474bf0 */
    /* Reformat the four counters into the menu's choice strings; run once per
       language, right after that language's rows are laid down. */
    void refreshCounters();                       /* 0x474970 */
    /* Write the four counters back into the machine profile. */
    void saveCounters();                          /* 0x474880 */
    /* Never touches `this`, but its call site @0x474b89 sets ecx, so the
       original declared it here (src/systemmenu.cpp). */
    void deleteRankingFiles();                    /* 0x474a40 */

    int         m_coins;                          /* 0x14 - total + service */
    int         m_totalCoin;                      /* 0x18 */
    int         m_serviceCoin;                    /* 0x1c */
    int         m_totalPlay;                      /* 0x20 */
    int         _pad24;                           /* 0x24 */
    SystemMenu *m_menu;                           /* 0x28 */
};

TestBookkeeping *createTestBookkeeping();         /* 0x474e40 */

/* What TestGameOptionSink::onMenu writes through its OWN `this`: the page's
   fields 0x10 lower, plus the menu at the far end of the object. */
struct TestGameOptionSinkFields
{
    char        _pad0[4];
    int         m_creditCoins;   /* sink+0x04 = page+0x14 */
    int         m_coinIndex;     /* sink+0x08 = page+0x18 */
    int         f0c;             /* sink+0x0c = page+0x1c */
    int         m_advSound;      /* sink+0x10 = page+0x20 */
    int         f14;             /* sink+0x14 = page+0x24 */
    char        _pad18[0x70b4 - 0x18];
    SystemMenu *m_menu;          /* sink+0x70b4 = page+0x70c4 */
};

struct __declspec(novtable) TestGameOptionSink : TestPageSink
{
    virtual void onMenu(int a, int what, int c);      /* 0x474ee0 (STUB;
                                       wip/testgameoption-onmenu.cpp) */
};

/* TestGameOption - the test menu's GAME OPTION page. KRuntimeClass @0x4af188,
   record @0x4af198 sizes it 0x70c8. The constructor reads two .ini keys and
   folds the raw CreditCoins value (0, 1, 4 or 5 coins per credit) into the
   0..3 index the page's list is drawn from. */
struct TestGameOption : EzObject, TestGameOptionSink, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();     /* 0x474ea0 */
    virtual DrawGroup     *getObjectInfo();       /* 0x474eb0 */
    virtual void           update();              /* 0x402880 (STUB, folded) */
    virtual void           update2(int a, int b); /* 0x475dc0 (STUB, folded) */
    virtual int            v5(int, int, int);     /* 0x401770 (folded) */

    TestGameOption();                             /* 0x475220 */
    ~TestGameOption();                            /* 0x474ec0 */

    void buildMenu();                             /* 0x4750e0 */

    int         m_creditCoins;                    /* 0x14 - raw .ini value */
    int         m_coinIndex;                      /* 0x18 - 0/1/4/5 -> 0..3 */
    int         f1c;                              /* 0x1c */
    int         m_advSound;                       /* 0x20 */
    int         f24;                              /* 0x24 */
    int         f28;                              /* 0x28 */
    char        _pad2c[0x70c4 - 0x2c];
    SystemMenu *m_menu;                           /* 0x70c4 */
};

TestGameOption *createTestGameOption();           /* 0x475350 */

#endif /* SYSTEMMENU_H */
