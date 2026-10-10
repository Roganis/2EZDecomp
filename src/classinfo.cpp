/* Descriptor getters for the game's Director/screen classes.

   Two record kinds sit in .data, usually as an adjacent pair per class:
     - the MFC-style runtime-class descriptor {name, base, createFn, pad}
       (KRuntimeClass, see include/ksound.h) - every one of these chains to
       the same base descriptor at 0x4a8e50;
     - a 0x10-byte object record {objectSize, count, flags, 0} - the sizes are
       plausible instance sizes (0x109de8 for RankingDirector, matching the
       ~0x420000-byte engine base these classes derive from).

   Each getter is the cookbook's singleton form (`mov eax, offset g; ret`), so
   only the ADDRESS has to line up: the storage here is an unreconstructed
   placeholder, exactly as src/globals.cpp and classKSound @0x4a7300 do. */

#include "ksound.h"
#include "bigbase.h" /* EzObjectInfo */

/* Make one instance of the class this descriptor describes - the MFC-style
   CreateObject. The null path returns the null creator itself, which is why the
   original needs no `xor eax,eax`: the pointer is already in eax. */
// FUNCTION: EZ2AC_UNPACKED 0x00401680
void *KRuntimeClass::createObject()
{
    void *(*create)();

    create = (void *(*)())createFn;

    if (create == 0)
    {
        return (void *)create;
    }

    return create();
}

/* The MFC-style IsKindOf test: walk the descriptor's base chain looking for
   `target`. Defined here rather than next to its caller (EzChart::setSound in
   src/ezchart.cpp) so cl cannot inline it away. */
// FUNCTION: EZ2AC_UNPACKED 0x00401660
int KRuntimeClass::isDerivedFrom(KRuntimeClass *target)
{
    KRuntimeClass *p = this;

    while (p != 0)
    {
        if (p == target)
        {
            return 1;
        }
        p = p->base;
    }

    return 0;
}

/* ({objectSize, count, flags, 0} - the non-KRuntimeClass record kind - is
   declared in include/bigbase.h now that Obj401780::addInfo takes one.) */


/* THE ROOT OF THE HIERARCHY. Every KRuntimeClass in this file chains its `base`
   to 0x4a8e50, and 0x4a8e50's own name string says "KEZ2DJObject" - so that is
   the engine's CObject, and include/kbase.h's KBase is the same class under a
   placeholder name (they share the constructor @0x401690 and the destructor
   @0x426e40). Its vtable is 0x48d830, the same six slots every derived class
   has, with slot 3 the empty folded `ret` @0x402880, slot 4 `ret 8` @0x475dc0
   and slot 5 the shared `xor eax,eax; ret 0xc` @0x401770 - i.e. the root's
   update/update2/v5 all do nothing, which is why so many classes leave them.
   Both getters stay free functions here until KBase is renamed and merged. */
/* "KEZ2DJObject" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8e50
KRuntimeClass classKEZ2DJObject;

// FUNCTION: EZ2AC_UNPACKED 0x00418240
KRuntimeClass *KEZ2DJObject_getRuntimeClass()
{
    return &classKEZ2DJObject;
}

/* objectSize 0x10, count 1, flags 0 */
// GLOBAL: EZ2AC_UNPACKED 0x004a8e60
EzObjectInfo g_info_4a8e60;

// FUNCTION: EZ2AC_UNPACKED 0x00418250
EzObjectInfo *getInfo_4a8e60()
{
    return &g_info_4a8e60;
}

/* The five panel classes' descriptors. Every one of their getters is a vtable
   slot rather than a free function, so all ten live in src/panels.cpp with the
   classes; only the storage stays here with the rest of the family. */
/* "AudienceRatePanel" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8ed0
KRuntimeClass classAudienceRatePanel;

/* objectSize 0x2c8, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a8ee0
EzObjectInfo g_info_4a8ee0;

/* "BackGround" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f0c
KRuntimeClass classBackGround;

/* objectSize 0x103540, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f1c
EzObjectInfo g_info_4a8f1c;

/* "BattleMode" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f48
KRuntimeClass classBattleMode;

/* objectSize 0xf34, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f58
EzObjectInfo g_info_4a8f58;

/* "Effector" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f84
KRuntimeClass classEffector;

/* objectSize 0x10dc, count 1, flags 0 */
// GLOBAL: EZ2AC_UNPACKED 0x004a8f94
EzObjectInfo g_info_4a8f94;

/* "Effector6thStyle" */
// GLOBAL: EZ2AC_UNPACKED 0x004a8ff4
KRuntimeClass classEffector6thStyle;

/* objectSize 0x14e0, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a9004
EzObjectInfo g_info_4a9004;

/* "EZ2DJMainGameDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004a9030
KRuntimeClass classEZ2DJMainGameDirector;

/* (a vtable slot - see src/maingame.cpp) */

/* objectSize 0xdb0, count 1, flags 2 */
// GLOBAL: EZ2AC_UNPACKED 0x004a9040
EzObjectInfo g_info_4a9040;

/* (a vtable slot - see src/maingame.cpp) */

/* "InGameSongSelect" */
// GLOBAL: EZ2AC_UNPACKED 0x004a906c
KRuntimeClass classInGameSongSelect;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x39a58, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a907c
EzObjectInfo g_info_4a907c;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x24f0, count 1, flags 2 - "Judge" (its KRuntimeClass is with
   the rest of the scanned descriptors below). Both of its getters are vtable
   slots 0 and 2 of the class @0x48ffb0, not free functions, so they live in
   src/judge.cpp as members - a slot is scored on the recomp address of the
   entity matched to it, so a lookalike beside the class can never make the
   slot compare. Only the storage stays here. */
// GLOBAL: EZ2AC_UNPACKED 0x004a90b8
EzObjectInfo g_info_4a90b8;

/* objectSize 0x36d8, count 2, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004a9130
EzObjectInfo g_info_4a9130;

/* (a vtable slot - see src/gamepanels.cpp) */

/* "SongSelectDirector". Its two getters are NOT here: they are vtable slots 0
   and 2 of the class @0x49193c, and a slot is scored on the recomp address of
   the entity matched to it, so a free-function lookalike beside the class can
   never make the slot compare. Both live in src/songselect.cpp as members. */
// GLOBAL: EZ2AC_UNPACKED 0x004aaed8
KRuntimeClass classSongSelectDirector;

/* "ModeSelectDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ac748
KRuntimeClass classModeSelectDirector;

/* (Its getters are vtable slots 0 and 2 of the class @0x494004, so they have
   to BE the slots - both live in src/modeselect.cpp as members.) */

/* objectSize 0x9630, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ac758
EzObjectInfo g_info_4ac758;


/* "GameOverDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004adf98
KRuntimeClass classGameOverDirector;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x23c, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004adfa8
EzObjectInfo g_info_4adfa8;

/* (a vtable slot - see src/directors.cpp) */

/* "TitleDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004adfd8
KRuntimeClass classTitleDirector;

/* (TitleDirector::getRuntimeClass @0x44dca0 is vtable slot 0 - see
   src/titledirector.cpp.) */

/* "ShowCredit" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae108
KRuntimeClass classShowCredit;

/* (a vtable slot) */

/* objectSize 0x7b4, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae118
EzObjectInfo g_info_4ae118;

/* (a vtable slot) */

/* objectSize 0x20, count 2, flags 4 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae154
EzObjectInfo g_info_4ae154;

/* Slot 2 moved into the class - src/scrfader.cpp. */

/* "ResultDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae1bc
KRuntimeClass classResultDirector;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x3118, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae1cc
EzObjectInfo g_info_4ae1cc;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x1438, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae2c0
EzObjectInfo g_info_4ae2c0;

/* (a vtable slot) */

/* "EndingDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae370
KRuntimeClass classEndingDirector;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x460, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae380
EzObjectInfo g_info_4ae380;

/* (a vtable slot - see src/directors.cpp) */

/* "RankingDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae418
KRuntimeClass classRankingDirector;

/* (a vtable slot - see src/directors.cpp) */

/* objectSize 0x109de8, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae428
EzObjectInfo g_info_4ae428;

/* (a vtable slot - see src/directors.cpp) */

/* The object records the five screens' vftable slot 4 registers with themselves
   (src/ezscreens.cpp). Placeholder storage exactly like the getters above; the
   sizes/counts in the comments are identification only. */

/* 0x1664 / 1 / 6 - TitleDirector's own record. */
// GLOBAL: EZ2AC_UNPACKED 0x004adfe8
EzObjectInfo g_info_4adfe8;

/* 0x23c / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae4d0
EzObjectInfo g_info_4ae4d0;

/* 0x230 / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae974
EzObjectInfo g_info_4ae974;

/* 0x28 / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004aea68
EzObjectInfo g_info_4aea68;

/* 0x71330 / 2 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004aec78
EzObjectInfo g_info_4aec78;

/* 0x2c / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004aecb4
EzObjectInfo g_info_4aecb4;

/* 0x70c8 / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004af198
EzObjectInfo g_info_4af198;

/* 0x1b8 / 2 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004af23c
EzObjectInfo g_info_4af23c;

/* 0x18 / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004af278
EzObjectInfo g_info_4af278;

/* 0x24 / 1 / 6 - Screen48D310's own record (its getter is 0x4761d0, and its
   KRuntimeClass partner sits right in front at 0x4afb60). */
// GLOBAL: EZ2AC_UNPACKED 0x004afb70
EzObjectInfo g_info_4afb70;

/* 0x40 / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004afbc8
EzObjectInfo g_info_4afbc8;

/* 0x1c / 1 / 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004afc34
EzObjectInfo g_info_4afc34;

/* More object records the screens' OnRegisterClass slots hand to themselves. */

// GLOBAL: EZ2AC_UNPACKED 0x004a90f4
EzObjectInfo g_info_4a90f4;

// GLOBAL: EZ2AC_UNPACKED 0x004aaee8
EzObjectInfo g_info_4aaee8;

// GLOBAL: EZ2AC_UNPACKED 0x004ae190
EzObjectInfo g_info_4ae190;

// GLOBAL: EZ2AC_UNPACKED 0x004ae794
EzObjectInfo g_info_4ae794;

// GLOBAL: EZ2AC_UNPACKED 0x004aea2c
EzObjectInfo g_info_4aea2c;

// GLOBAL: EZ2AC_UNPACKED 0x004aeb4c
EzObjectInfo g_info_4aeb4c;

// GLOBAL: EZ2AC_UNPACKED 0x004aeb88
EzObjectInfo g_info_4aeb88;

// GLOBAL: EZ2AC_UNPACKED 0x004aec00
EzObjectInfo g_info_4aec00;

/* ------------------------------------------------------------------------- */
/* The rest of the family, found by scanning every `mov eax,imm; ret` leaf in
   0x401000..0x478000 and keeping the ones whose immediate is a descriptor: a
   KRuntimeClass is {name, base == classKEZ2DJObject, 0, 0}, an object record is
   {size, count, flags, 0}. Every class in the game registers this same adjacent
   pair, so the scan also NAMES twenty-two more classes - the whole test menu,
   and the Catch and GF families. Free functions until each class is modeled. */

/* "Judge" - now modeled (include/judge.h), so its getter moved into the class
   as vtable slot 0; only the storage stays here. */
// GLOBAL: EZ2AC_UNPACKED 0x004a90a8
KRuntimeClass classJudge;

/* "Panel" */
// GLOBAL: EZ2AC_UNPACKED 0x004a9120
KRuntimeClass classPanel;

/* (a vtable slot - see src/gamepanels.cpp) */

/* (getInfo_4aaee8 @0x430b40 is SongSelectDirector::getObjectInfo, vtable slot
   2 - see src/songselect.cpp.) */

/* (getInfo_4adfe8 @0x44dcb0 is TitleDirector::getObjectInfo, vtable slot 2 -
   see src/titledirector.cpp.) */

/* "ScrFader" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae144
KRuntimeClass classScrFader;

/* Slot 0 moved into the class - src/scrfader.cpp. */

/* "TotalResultDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae2b0
KRuntimeClass classTotalResultDirector;

/* (a vtable slot) */

/* "WarningDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae4c0
KRuntimeClass classWarningDirector;

/* (a vtable slot - see src/directors.cpp) */

/* (a vtable slot - see src/directors.cpp) */

/* "GFBackGround" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae650
KRuntimeClass classGFBackGround;

/* (Its getters are vtable slots 0 and 2 - see src/backgrounds.cpp.) */

/* objectSize 0x103540, count 1, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae660
EzObjectInfo g_info_4ae660;


/* "GFMainGameDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae68c
KRuntimeClass classGFMainGameDirector;

/* (a vtable slot - see src/maingame.cpp) */

/* objectSize 0xd3c, count 1, flags 2 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae69c
EzObjectInfo g_info_4ae69c;

/* (a vtable slot - see src/maingame.cpp) */

/* "GFPanel" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae6c8
KRuntimeClass classGFPanel;

/* (a vtable slot - see src/gamepanels.cpp) */

/* objectSize 0x34dc, count 2, flags 6 */
// GLOBAL: EZ2AC_UNPACKED 0x004ae6d8
EzObjectInfo g_info_4ae6d8;

/* (a vtable slot - see src/gamepanels.cpp) */

/* "GFResultDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae784
KRuntimeClass classGFResultDirector;

/* (a vtable slot - see src/directors.cpp) */

/* (a vtable slot - see src/directors.cpp) */

/* "WarningShortDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004ae964
KRuntimeClass classWarningShortDirector;

/* (a vtable slot - see src/directors.cpp) */

/* (a vtable slot - see src/directors.cpp) */

/* "DrawModeIntroDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004aea1c
KRuntimeClass classDrawModeIntroDirector;

/* (a vtable slot) */

/* (a vtable slot) */

/* "CatchBackGround" */
// GLOBAL: EZ2AC_UNPACKED 0x004aeb3c
KRuntimeClass classCatchBackGround;

/* (Its getters are vtable slots 0 and 2 - see src/backgrounds.cpp.) */


/* "CatchMainGameDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004aeb78
KRuntimeClass classCatchMainGameDirector;

/* (a vtable slot - see src/maingame.cpp) */

/* (a vtable slot - see src/maingame.cpp) */

/* "CatchPanel" */
// GLOBAL: EZ2AC_UNPACKED 0x004aebf0
KRuntimeClass classCatchPanel;

/* (a vtable slot - see src/gamepanels.cpp) */

/* (a vtable slot - see src/gamepanels.cpp) */

/* "SystemMenu" */
// GLOBAL: EZ2AC_UNPACKED 0x004aec68
KRuntimeClass classSystemMenu;

/* (Its two getters are vtable slots 0 and 2 of the class @0x49e9b8, so they
   have to BE the slots - both live in src/systemmenu.cpp as members.) */

/* "TestBookkeeping" */
// GLOBAL: EZ2AC_UNPACKED 0x004aeca4
KRuntimeClass classTestBookkeeping;

/* (Its two getters are vtable slots 0 and 2 of the class @0x49eca4, so they
   have to BE the slots - both live in src/systemmenu.cpp as members.) */

/* "TestGameOption" */
// GLOBAL: EZ2AC_UNPACKED 0x004af188
KRuntimeClass classTestGameOption;

/* (Its two getters are vtable slots 0 and 2 of the class @0x49ef6c, so they
   have to BE the slots - both live in src/systemmenu.cpp as members.) */

/* "TestInput" */
// GLOBAL: EZ2AC_UNPACKED 0x004af22c
KRuntimeClass classTestInput;

/* (Its two getters are vtable slots 0 and 2 of the class @0x49f130, so they
   have to BE the slots - both live in src/testinput.cpp as members.) */

/* "TestLighting" */
// GLOBAL: EZ2AC_UNPACKED 0x004af268
KRuntimeClass classTestLighting;

/* (Its two getters are vtable slots 0 and 2 of the class @0x49f484, so they
   have to BE the slots - both live in src/systemmenu.cpp as members.) */

/* "TestModeDirector" */
// GLOBAL: EZ2AC_UNPACKED 0x004afb60
KRuntimeClass classTestModeDirector;

/* (a vtable slot - see src/directors.cpp) */

/* (a vtable slot - see src/directors.cpp) */

/* "TestSound" */
// GLOBAL: EZ2AC_UNPACKED 0x004afbb8
KRuntimeClass classTestSound;

/* (a vtable slot - see src/testinput.cpp) */

/* (a vtable slot - see src/testinput.cpp) */

/* "TestVideo" */
// GLOBAL: EZ2AC_UNPACKED 0x004afc24
KRuntimeClass classTestVideo;

/* (Its two getters are vtable slots 0 and 2 of the class @0x4a0038, so they
   have to BE the slots - both live in src/testinput.cpp as members.) */
