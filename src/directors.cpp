/* RankingDirector, ResultDirector and GFResultDirector - three more directors
   whose destructors are their member maps (see include/directors.h).

   The two result directors share a body: release their sound objects through
   slot 5, put four pieces of cabinet state back to zero, and let the members
   go. The sound objects are released in the order the source lists them, which
   is NOT address order in either class. */

#include <string.h>

#include "directors.h"
#include "players.h"   /* ListNode, nextListA */
#include <stdio.h>
#include "bigbase.h"
#include "ez2acapp.h"   /* KGraphics, GVec3, g_sub1b2f054 */
#include "loader.h"     /* KTexture */
#include "kparser.h"
#include "systemmenu.h"
#include "testinput.h"    /* the test pages' factories */
#include "kengineapp.h"   /* g_app, g_iniProfile */
#include "kprofile.h"

/* Already matched under placeholder class names (src/accessors.cpp @0x40bbb0,
   src/com_wrappers.cpp @0x40bb00) - see src/modeselect.cpp. */
struct Obj40BBB0 { void loadIdentity(); };
struct Obj40BB00 { void doSub(); void doOp10(unsigned int v); };  /* 0x40bb00 / 0x40a730 */

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */

/* Which stage the round is on - the number the result screen prints. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ec0c
int g_stageNumber;

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classRankingDirector;   /* 0x4ae418 */
extern EzObjectInfo  g_info_4ae428;          /* 0x4ae428 */
extern KRuntimeClass classResultDirector;    /* 0x4ae1bc */
extern EzObjectInfo  g_info_4ae1cc;          /* 0x4ae1cc */
extern KRuntimeClass classGFResultDirector;  /* 0x4ae784 */
extern EzObjectInfo  g_info_4ae794;          /* 0x4ae794 */

extern int  g_elemCount;            /* 0x1b2e718 (src/loops.cpp) */
extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */

/* ResultDirector's own trim buffer - each class owns one, which is why the
   original's ICF left these identical bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd070
static char g_resultVideoName[260];

/* The same trim as BackGround::videoName @0x41add0: copy the current text
   slot and drop a trailing "-xx" / "-xxx" / "-xxxx". */
// FUNCTION: EZ2AC_UNPACKED 0x00450c00
char *ResultDirector::m450c00()
{
    strcpy(g_resultVideoName, g_textSlots[g_elemCount]);

    if (g_resultVideoName[strlen(g_resultVideoName) - 3] == '-')
        g_resultVideoName[strlen(g_resultVideoName) - 3] = 0;
    else if (g_resultVideoName[strlen(g_resultVideoName) - 4] == '-')
        g_resultVideoName[strlen(g_resultVideoName) - 4] = 0;
    else if (g_resultVideoName[strlen(g_resultVideoName) - 5] == '-')
        g_resultVideoName[strlen(g_resultVideoName) - 5] = 0;

    return g_resultVideoName;
}

/* GFResultDirector's own trim buffer - each class owns one, which is why the
   original's ICF left these identical bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037df228
static char g_gfResultVideoName[260];

/* The same trim as BackGround::videoName @0x41add0: copy the current text
   slot and drop a trailing "-xx" / "-xxx" / "-xxxx". */
// FUNCTION: EZ2AC_UNPACKED 0x004673a0
char *GFResultDirector::m4673a0()
{
    strcpy(g_gfResultVideoName, g_textSlots[g_elemCount]);

    if (g_gfResultVideoName[strlen(g_gfResultVideoName) - 3] == '-')
        g_gfResultVideoName[strlen(g_gfResultVideoName) - 3] = 0;
    else if (g_gfResultVideoName[strlen(g_gfResultVideoName) - 4] == '-')
        g_gfResultVideoName[strlen(g_gfResultVideoName) - 4] = 0;
    else if (g_gfResultVideoName[strlen(g_gfResultVideoName) - 5] == '-')
        g_gfResultVideoName[strlen(g_gfResultVideoName) - 5] = 0;

    return g_gfResultVideoName;
}

// FUNCTION: EZ2AC_UNPACKED 0x00456730
KRuntimeClass *RankingDirector::getRuntimeClass()
{
    return &classRankingDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00456740
DrawGroup *RankingDirector::getObjectInfo()
{
    return &g_info_4ae428;
}

// FUNCTION: EZ2AC_UNPACKED 0x00456750
RankingDirector::~RankingDirector()
{
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00459850
// RankingDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int RankingDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00498680
struct RankingDirector;

// FUNCTION: EZ2AC_UNPACKED 0x00450670
KRuntimeClass *ResultDirector::getRuntimeClass()
{
    return &classResultDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00450680
DrawGroup *ResultDirector::getObjectInfo()
{
    return &g_info_4ae1cc;
}

// FUNCTION: EZ2AC_UNPACKED 0x00450ce0
ResultDirector::~ResultDirector()
{
    m_28e4->q5();
    if (m_28fc != 0)
    {
        m_28fc->q5();
    }
    m_28e8->q5();
    m_28f0->q5();
    m_28f4->q5();
    m_28f8->q5();
    m_28ec->q5();
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00451ae0
// ResultDirector::`scalar deleting destructor'

/* The stage number, drawn digit by digit out of the director's own run of ten
   textures. Under ten it is one digit at x=372; from ten up it is the tens
   digit at 367 and the ones at 377. Both draws pass zero for the size, which
   is this engine's "use the texture's own" - and the second reads the global
   afresh rather than reusing the quotient. */
// FUNCTION: EZ2AC_UNPACKED 0x00450ef0
void ResultDirector::drawStageNumber()
{
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    if (g_stageNumber < 10)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[g_stageNumber],
                                             372.0f, 373.0f, 0.0f, 0.0f, -1);
        return;
    }

    ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[g_stageNumber / 10 % 10],
                                         367.0f, 373.0f, 0.0f, 0.0f, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_digits[g_stageNumber % 10],
                                         377.0f, 373.0f, 0.0f, 0.0f, -1);
}

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int ResultDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00495c5c
struct ResultDirector;

// FUNCTION: EZ2AC_UNPACKED 0x00466ec0
KRuntimeClass *GFResultDirector::getRuntimeClass()
{
    return &classGFResultDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00466ed0
DrawGroup *GFResultDirector::getObjectInfo()
{
    return &g_info_4ae794;
}

// FUNCTION: EZ2AC_UNPACKED 0x00467480
GFResultDirector::~GFResultDirector()
{
    m_21e0->q5();
    m_21e4->q5();
    m_21ec->q5();
    m_21f0->q5();
    m_21f4->q5();
    m_21e8->q5();
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00468030
// GFResultDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int GFResultDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049b210
struct GFResultDirector;

/* ---- The warning / game-over screens ---------------------------------- */

extern KRuntimeClass classWarningShortDirector;  /* 0x4ae964 */
extern EzObjectInfo  g_info_4ae974;              /* 0x4ae974 */
extern KRuntimeClass classGameOverDirector;      /* 0x4adf98 */
extern EzObjectInfo  g_info_4adfa8;              /* 0x4adfa8 */
extern KRuntimeClass classWarningDirector;       /* 0x4ae4c0 */
extern EzObjectInfo  g_info_4ae4d0;              /* 0x4ae4d0 */

// FUNCTION: EZ2AC_UNPACKED 0x00469a00
KRuntimeClass *WarningShortDirector::getRuntimeClass()
{
    return &classWarningShortDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00469a10
DrawGroup *WarningShortDirector::getObjectInfo()
{
    return &g_info_4ae974;
}

// FUNCTION: EZ2AC_UNPACKED 0x00469a20
WarningShortDirector::~WarningShortDirector()
{
}

/* src/strutil.cpp - the game's chdir pair (declared again further down, beside
   EndingDirector's constructor, and again beside the in-game tables). */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
/* WINMM's timeGetTime WITHOUT dllimport - the original reaches it through the
   linker's 5-byte thunk @0x4145f0, not the indirect IAT call that mmsystem.h's
   own dllimport declaration would give. Same shape as src/ez2acapp.cpp's. */
extern "C" unsigned long __stdcall timeGetTime(void);

/* Whatever g_1b2e844 points at; only its slot 0 is reached (the same view
   src/ez2djstage.cpp declares). */
struct Obj1B2E844 { virtual void q00(int v); };
extern void *g_1b2e844;      /* src/moderunner.cpp */
extern int   g_1b2ea78;      /* src/systemmenu.cpp */

/* GameOverSink::setMode @0x44da10 - forward the mode and nothing else. Its
   first argument is dead here, as it is in WarningSink's override: the
   original reads only [esp+8]. */
// FUNCTION: EZ2AC_UNPACKED 0x0044da10
void GameOverSink::setMode(int, int mode)
{
    ((Obj1B2E844 *)g_1b2e844)->q00(mode);
}

/* WarningDirector::update2 @0x45c2b0 - the warning screen's per-frame tick.
   Pressing button 0 forwards a 0 to the same object GameOverSink::setMode
   forwards to, the menu flag is cleared unconditionally, and the frame
   counter only advances while the latched mode is 0 or 2.

   That last test is a SWITCH, not an `if`: the original's `sub eax,0` /
   `sub eax,2` chain is the switch lowering, where any `if` form emits `cmp`. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c2b0
void WarningDirector::update2(int, int)
{
    if (g_bigInstance->getState(0) == 2)
        ((Obj1B2E844 *)g_1b2e844)->q00(0);

    g_1b2ea78 = 0;

    switch (m_238)
    {
    case 0:
    case 2:
        m_14++;
        break;
    }
}

/* WarningShortDirector::WarningShortDirector @0x469b00 - the short warning
   screen's director, and the near-twin of WarningDirector's constructor
   @0x45c320 (twindelta.py: 0.690 semantic, 7 regions).

   Same shape - base ctor, vptr, the chart member's ctor, a .str loaded and
   attached - with four differences: one vptr rather than two (this class has
   no sink base), the chart member at +0x10 rather than +0x18, the animation
   attached with flag 0 rather than 1, and the whole thing wrapped in a
   working-directory change so the .str name is relative. It ends by stamping
   timeGetTime into +0x22c, which is what named that field.

   The class was already modelled here from its destructor; the constructor
   was stubbed under the placeholder name Obj469B00 in include/ezscreen.h -
   the same collision WarningDirector had this morning, found the same way
   (twins2 pairs the two constructors once one of them is matched). */
// FUNCTION: EZ2AC_UNPACKED 0x00469b00
WarningShortDirector::WarningShortDirector()
{
    Obj402860Head *h;

    // STRING: EZ2AC_UNPACKED 0x0049c47c
    changeDir("System\\WarningShort\\", 1);
    // STRING: EZ2AC_UNPACKED 0x0049c468
    ((KGraphics *)g_sub1b2f054)->loadStr("Warning_Short.str", &h);
    m_10.attach(h, 0);
    popDirectory();
    m_22c = timeGetTime();
}

// FUNCTION: EZ2AC_UNPACKED 0x00469ba0
// WarningShortDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int WarningShortDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049c450
struct WarningShortDirector;

// FUNCTION: EZ2AC_UNPACKED 0x0044d940
KRuntimeClass *GameOverDirector::getRuntimeClass()
{
    return &classGameOverDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0044d950
DrawGroup *GameOverDirector::getObjectInfo()
{
    return &g_info_4adfa8;
}

// FUNCTION: EZ2AC_UNPACKED 0x0044d960
GameOverDirector::~GameOverDirector()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0044db50
// GameOverDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int GameOverDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x004948bc EzObject
struct GameOverDirector;

// VTABLE: EZ2AC_UNPACKED 0x004948b8 GameOverSink
struct GameOverDirector;


// FUNCTION: EZ2AC_UNPACKED 0x0045c1c0
KRuntimeClass *WarningDirector::getRuntimeClass()
{
    return &classWarningDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c1d0
DrawGroup *WarningDirector::getObjectInfo()
{
    return &g_info_4ae4d0;
}

/* WarningDirector::WarningDirector @0x45c320 - the warning screen's director.

   Load the warning bitmap into the object's own texture slot, then load the
   shared loading animation's .str and attach it to the chart member. The two
   trailing fields the sink writes through - the object's +0x14 and +0x238 -
   are cleared last.

   This class was stubbed under the placeholder name Obj45C320 in
   include/ezscreen.h at the same time as it was modelled here from its
   destructor; the factory @0x45c3f0 pushing exactly 0x23c is what confirms
   the two are one class. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c320
WarningDirector::WarningDirector()
{
    Obj402860Head *h;

    // STRING: EZ2AC_UNPACKED 0x00498ebc
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\warning.bmp", &m_tex, 0);
    // STRING: EZ2AC_UNPACKED 0x00498ea4
    ((KGraphics *)g_sub1b2f054)->loadStr("System\\main_loading.str", &h);
    m_18.attach(h, 1);

    m_14 = 0;
    m_238 = 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c1e0
WarningDirector::~WarningDirector()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c3d0
// WarningDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int WarningDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00498e8c EzObject
struct WarningDirector;

// VTABLE: EZ2AC_UNPACKED 0x00498e88 WarningSink
struct WarningDirector;

/* Slot 0 of that second vtable: a guarded mode latch. The constant 2 is hoisted
   into eax and reused for both the compare and the store. */
// FUNCTION: EZ2AC_UNPACKED 0x0045c300
void WarningSink::setMode(int, int mode)
{
    WarningSinkFields *p = (WarningSinkFields *)this;

    if (mode == 2)
    {
        p->f228 = 2;
        p->f4 = 0;
    }
}

/* ---- EndingDirector, InGameSongSelect, TestModeDirector --------------- */

extern KRuntimeClass classEndingDirector;    /* 0x4ae370 */
extern EzObjectInfo  g_info_4ae380;          /* 0x4ae380 */
extern KRuntimeClass classInGameSongSelect;  /* 0x4a906c */
extern EzObjectInfo  g_info_4a907c;          /* 0x4a907c */
extern KRuntimeClass classTestModeDirector;  /* 0x4afb60 */
extern EzObjectInfo  g_info_4afb70;          /* 0x4afb70 */

// FUNCTION: EZ2AC_UNPACKED 0x00455ba0
KRuntimeClass *EndingDirector::getRuntimeClass()
{
    return &classEndingDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00455bb0
DrawGroup *EndingDirector::getObjectInfo()
{
    return &g_info_4ae380;
}

// FUNCTION: EZ2AC_UNPACKED 0x00455bc0
EndingDirector::~EndingDirector()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00455f90
// EndingDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int EndingDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00497530
struct EndingDirector;

// FUNCTION: EZ2AC_UNPACKED 0x00425120
KRuntimeClass *InGameSongSelect::getRuntimeClass()
{
    return &classInGameSongSelect;
}

// FUNCTION: EZ2AC_UNPACKED 0x00425130
DrawGroup *InGameSongSelect::getObjectInfo()
{
    return &g_info_4a907c;
}

// FUNCTION: EZ2AC_UNPACKED 0x00425830
InGameSongSelect::~InGameSongSelect()
{
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00425b20
// InGameSongSelect::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int InGameSongSelect::v5(int, int, int)
{
    return 0;
}

/* src/strutil.cpp - the game's chdir pair (also declared again below, beside
   EndingDirector's constructor). */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */

/* The in-game song select's four cell tables, taken from two directories: the
   two difficulty meters out of the ClubMix panel set, and the skip prompt plus
   the TR readout out of the mode-select set. Each directory is visited, its two
   .str resources attached, and popped again. */
// FUNCTION: EZ2AC_UNPACKED 0x004259c0
InGameSongSelect::InGameSongSelect()
{
    Obj402860Head *res;

    // STRING: EZ2AC_UNPACKED 0x0048fdb4
    changeDir("system\\clubmix\\panel\\", 1);
    // STRING: EZ2AC_UNPACKED 0x0048fda0
    ((KGraphics *)g_sub1b2f054)->loadStr("difficulty_1p.str", &res);
    m_39130.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x0048fd8c
    ((KGraphics *)g_sub1b2f054)->loadStr("difficulty_2p.str", &res);
    m_3934c.attach(res, 1);
    popDirectory();

    // STRING: EZ2AC_UNPACKED 0x0048fd78
    changeDir("system\\ModeSelect\\", 1);
    // STRING: EZ2AC_UNPACKED 0x0048fd68
    ((KGraphics *)g_sub1b2f054)->loadStr("StartSkip.str", &res);
    m_39568.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x0048fd60
    ((KGraphics *)g_sub1b2f054)->loadStr("TR.str", &res);
    m_39784.attach(res, 1);
    popDirectory();

    f10 = 0;
    m425250();
}

extern int g_elemCount;   /* 0x1b2e718 (src/loops.cpp) - the live slot index */
extern char g_modeName[256];   /* 0x1b2e680 (src/modename.cpp) */

/* One entry of the list. The frame goes down under an additive blend, the
   entry's own art over it under the normal one; both are centred on the point
   and both take the same tint, one byte replicated into all four channels. The
   index wraps by adding the count before the modulo, so a negative n still
   lands in range. */
// FUNCTION: EZ2AC_UNPACKED 0x004258d0
void InGameSongSelect::m4258d0(int n, float x, float y, float w, float h, int a)
{
    int   col;
    float py;
    float px;
    int   cnt;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);

    col = ((((a << 8) | a) << 8 | a) << 8) | a;
    py = y - h * 0.5f;
    px = x - w * 0.5f;
    ((KGraphics *)g_sub1b2f054)->drawTex(m_399a0, px, py, w, h, col);

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    cnt = m_count[g_elemCount];
    ((KGraphics *)g_sub1b2f054)->drawTex(
        m_entry[g_elemCount][(n + cnt) % cnt].tex, px, py, w, h, col);
}

/* One int out of a per-mode .ini - "<mode>1p-<name>.ini", one section and one
   value. Nothing is read back out of the section name, so the read is only
   there to step the parser past it. */
// FUNCTION: EZ2AC_UNPACKED 0x00425140
int InGameSongSelect::m425140(char *name)
{
    int     v;
    char    path[260];
    char    sec[256];

    /* The parser is declared AFTER this statement, not at the top: the original
       does the subtraction while `this` is still in ecx and never touches the
       object again, which is what frees esi to be the function's zero. */
    f39a50 -= f39a54;

    KParser parser;

    // STRING: EZ2AC_UNPACKED 0x0048fcb0
    sprintf(path, "%s1p-%s.ini", g_modeName, name);
    if (parser.readFile(path) == 0)
    {
        return 0;
    }

    readGdsSection(sec, &parser);
    v = 0;
    readGdsInt(&v, &parser);
    return v;
}

// VTABLE: EZ2AC_UNPACKED 0x0048fd2c
struct InGameSongSelect;

/* The flag the whole cabinet watches to know the operator menu is up. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e67c
int g_testModeFlag;

// FUNCTION: EZ2AC_UNPACKED 0x004761c0
KRuntimeClass *TestModeDirector::getRuntimeClass()
{
    return &classTestModeDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x004761d0
DrawGroup *TestModeDirector::getObjectInfo()
{
    return &g_info_4afb70;
}

// FUNCTION: EZ2AC_UNPACKED 0x004761e0
TestModeDirector::~TestModeDirector()
{
    g_bigInstance->testMode1 = 1;
    g_bigInstance->testMode2 = 1;
    g_testModeFlag = 1;
    // STRING: EZ2AC_UNPACKED 0x0049f954
    Log("Go Test Mode TRUE");
}

// FUNCTION: EZ2AC_UNPACKED 0x00476810
// TestModeDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestModeDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049f96c EzObject
struct TestModeDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049f968 TestModeSink
struct TestModeDirector;

/* ---- TotalResultDirector and DrawModeIntroDirector -------------------- */

extern KRuntimeClass classTotalResultDirector;    /* 0x4ae2b0 */
extern EzObjectInfo  g_info_4ae2c0;               /* 0x4ae2c0 */
extern KRuntimeClass classDrawModeIntroDirector;  /* 0x4aea1c */
extern EzObjectInfo  g_info_4aea2c;               /* 0x4aea2c */

// FUNCTION: EZ2AC_UNPACKED 0x00454460
KRuntimeClass *TotalResultDirector::getRuntimeClass()
{
    return &classTotalResultDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00454470
DrawGroup *TotalResultDirector::getObjectInfo()
{
    return &g_info_4ae2c0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004545d0
TotalResultDirector::~TotalResultDirector()
{
    m_1198->q5();
    m_119c->q5();
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00454970
// TotalResultDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TotalResultDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00496ecc
struct TotalResultDirector;

// FUNCTION: EZ2AC_UNPACKED 0x0046a070
KRuntimeClass *DrawModeIntroDirector::getRuntimeClass()
{
    return &classDrawModeIntroDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a080
DrawGroup *DrawModeIntroDirector::getObjectInfo()
{
    return &g_info_4aea2c;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a090
DrawModeIntroDirector::~DrawModeIntroDirector()
{
    GVec3 eye;
    GVec3 at;
    GVec3 up;

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    ((Obj40BBB0 *)g_sub1b2f054)->loadIdentity();
    ((Obj40BB00 *)g_sub1b2f054)->doSub();
    ((KGraphics *)g_sub1b2f054)->setCamera(&eye, &at, &up);
    ((KGraphics *)g_sub1b2f054)->setProjection(1.0f, 10000.0f, 0.785398185f);
    g_bigInstance->setFlags(0);
    Log("ModeSelectDirector::Delete");
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a420
// DrawModeIntroDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int DrawModeIntroDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049cbc8
struct DrawModeIntroDirector;

/* ---- EndingDirector's constructor ------------------------------------- */

extern void *g_1b2e844;               /* src/moderunner.cpp */
extern int   g_testModeFlag;          /* 0x1b2e67c, above */
extern int   g_1b2eb6c;               /* src/ez2acapp.cpp - the CV2 flag */
extern void  changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void  popDirectory();                        /* 0x412e60 */
extern "C" unsigned long __stdcall timeGetTime(void);

/* The one flag with no other reader yet: the ending only nudges the sound bank
   a second time when it is above zero. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7cc
int g_1b2e7cc;

/* Two charts, the ending's two .str resources, its sample and its fade bitmap.
   The two directory arms are written out in full - the original cross-jumps
   only the very last `attach` call, with both arms pushing its arguments
   themselves, which is the tail merger's doing and not a source shape. */
// FUNCTION: EZ2AC_UNPACKED 0x00455df0
EndingDirector::EndingDirector()
{
    Obj402860Head *res;

    if (g_testModeFlag != 0)
    {
        ((SoundBank *)g_1b2e844)->v0(0);
    }
    if (g_1b2e7cc > 0)
    {
        ((SoundBank *)g_1b2e844)->v0(0);
    }
    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00497570
        changeDir("System\\cv2mix\\ending", 1);
        // STRING: EZ2AC_UNPACKED 0x00497564
        ((KGraphics *)g_sub1b2f054)->loadStr("Ending.str", &res);
        m_10.attach(res, 0);
        // STRING: EZ2AC_UNPACKED 0x00494918
        ((KGraphics *)g_sub1b2f054)->loadStr("gameover.str", &res);
        m_22c.attach(res, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00497554
        changeDir("System\\Ending", 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("Ending.str", &res);
        m_10.attach(res, 0);
        ((KGraphics *)g_sub1b2f054)->loadStr("gameover.str", &res);
        m_22c.attach(res, 0);
    }
    // STRING: EZ2AC_UNPACKED 0x00497548
    m_sample = (void *)g_sub1b2eae4->createSample("ending.wav", 1, 0);
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x004932d8
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp",
                                             &m_fade, 0);
    f454 = 0xff;
    f450 = 0;
    f44c = 1;
    m_started = timeGetTime();
}

extern int g_1b2eadc;                 /* 0x1b2eadc (src/ez2acapp.cpp) */

/* EndingDirector's constructor one mix over: one chart, one .str, one sample.
   Three directory arms this time (Catch, CV2, plain), each with its own
   loadStr - the original merges everything from the `mov ecx, <graphics>`
   onward, which is the tail merger and not a source shape, exactly as in
   EndingDirector below. The sample's two names are two separate calls for
   the same reason: written as a ternary the name would go branchless.
   Everything it needs is declared further down; see the block above
   EndingDirector::EndingDirector. */
// FUNCTION: EZ2AC_UNPACKED 0x0044da30
GameOverDirector::GameOverDirector()
{
    Obj402860Head *res;

    if (g_1b2e7cc > 0)
    {
        ((SoundBank *)g_1b2e844)->v0(0);
    }
    if (g_1b2eadc == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x00494960
        changeDir("System\\EZ2Catch\\Gameover", 1);
        // STRING: EZ2AC_UNPACKED 0x00494950
        ((KGraphics *)g_sub1b2f054)->loadStr("c_gameover.str", &res);
    }
    else if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00494938
        changeDir("System\\cv2mix\\Gameover", 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("gameover.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00494928
        changeDir("System\\Gameover", 1);
        ((KGraphics *)g_sub1b2f054)->loadStr("gameover.str", &res);
    }
    m_18.attach(res, 0);
    popDirectory();

    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004948f4
        m_sample = g_sub1b2eae4->createSample("system\\cv2mix\\gameover\\gameover.wav", 1, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x004948d4
        m_sample = g_sub1b2eae4->createSample("system\\gameover\\gameover.wav", 1, 0);
    }
    m_started = timeGetTime();
}

/* ---- DrawModeIntroDirector's constructor ------------------------------ */

/* The 0x28-byte block the video-blit setter takes BY VALUE, and the object it
   takes by pointer. Both are placeholder storage pinned at their addresses. */
struct GfxBlob28
{
    int v[10];
};

// GLOBAL: EZ2AC_UNPACKED 0x01b2e868
GfxBlob28 g_1b2e868;

// GLOBAL: EZ2AC_UNPACKED 0x01b2f138
int g_1b2f138;

extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */

/* The twelve mode plates and their masks, plus the blank video frame and the
   intro sample. The two file-name formats differ only by the "_m" suffix, and
   the loop numbers them from 1. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a260
DrawModeIntroDirector::DrawModeIntroDirector()
{
    Obj402860Head *res;
    char buf[260];
    int i;

    f230 = 0;
    // STRING: EZ2AC_UNPACKED 0x004937dc
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\Common\\Video_Blank_256.bmp",
                                             &m_video, 0);
    ((KGraphics *)g_sub1b2f054)->m408040(m_video, &g_1b2f138, g_1b2e868);
    /* `i` itself is dead here: both uses become their own induction variables
       (the printed number and the walking pointer), which is what leaves cl
       needing a separate down-counter for the trip test. Writing the loop from
       1 to 12 and indexing [i - 1] keeps `i` live and gets a `cmp` instead. */
    for (i = 0; i < 12; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0049cc44
        sprintf(buf, "system\\modeselect\\VF_ModeInOut\\modetext%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_tex[i], 0);
        // STRING: EZ2AC_UNPACKED 0x0049cc10
        sprintf(buf, "system\\modeselect\\VF_ModeInOut\\modetext%02d_m.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(buf, &m_maskTex[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x0049cbe0
    sprintf(buf, "system\\modeselect\\VF_ModeInout\\Mode_Intro.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(buf, &res);
    m_10.attach(res, 0);
    m_10.setCell(4, 0, (int)m_maskTex[g_modeIndex]);
    m_10.setCell(5, 0, (int)m_tex[g_modeIndex]);
    // STRING: EZ2AC_UNPACKED 0x0049445c
    m_sample = (void *)g_sub1b2eae4->createSample("System\\ModeSelect\\ModeIntro.wav", 1, 0);
}

/* The ending screen's new-plus-ctor factory: publish the record, take a slot
   out of its pool and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x00455fb0
EndingDirector *createEndingDirector()
{
    g_objectRecord = &g_info_4ae380;
    return new EndingDirector;
}

/* Four fields at once, and the third is a branchless pick between two
   constants - which is exactly what a ternary compiles to (CLAUDE.md); an
   if/else would branch. */
// FUNCTION: EZ2AC_UNPACKED 0x00450420
void ScrFader::m450420(int a, int b, void *s)
{
    f10 = a;
    f14 = b;
    f1c = s;
    f18 = (a != 1) ? 0x100 : 0;
}

/* What the Catch director's first sink reports. `this` is the SINK, so the two
   fields it touches are reached through a view (the same rule as the test-menu
   pages). Kind 2 hands one of the owner's values to the screen driver's slot 0;
   kind 1 just latches a byte. */
struct CatchSink1Fields
{
    char _pad0[0xc90];
    int  fc90;                 /* sink+0xc90 */
    char _padc94[0xcf8 - 0xc94];
    char fcf8;                 /* sink+0xcf8 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0046c280
void CatchSink1::v0(int, int what)
{
    CatchSink1Fields *f = (CatchSink1Fields *)this;

    if (what == 2)
    {
        ((Obj401780 *)g_1b2e844)->armAndSetC(f->fc90);
        return;
    }
    if (what == 1)
    {
        f->fcf8 = 1;
    }
}

/* One frame of the short warning screen: reset the blend state, refresh its one
   cell block and, once that block reports it has finished playing out, hand the
   driver the same exit code the Start button would. */
// FUNCTION: EZ2AC_UNPACKED 0x00469a80
void WarningShortDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    m_10.m403940();
    if (m_10.isDone() != 0)
    {
        ((Obj401780 *)g_1b2e844)->armAndSetC(0);
    }
}

/* The short warning screen's poll: the Start button, or the engine reporting it
   already has a credit, both hand the screen driver its exit code - 0 and 1
   respectively, through slot 0. */
// FUNCTION: EZ2AC_UNPACKED 0x00469ac0
void WarningShortDirector::update2(int, int)
{
    if (g_bigInstance->getState(0) == 2)
    {
        ((Obj401780 *)g_1b2e844)->armAndSetC(0);
    }
    if (g_bigInstance->have != 0)
    {
        ((Obj401780 *)g_1b2e844)->armAndSetC(1);
    }
}

/* Load a numbered run of ten textures - "<base>0.bmp" through "<base>9.bmp" -
   into a caller-supplied array. The engine's own loadTexSeries @0x456010 does
   the same job with a different name format; this class rolls its own. */
// FUNCTION: EZ2AC_UNPACKED 0x00454480 folded
int TotalResultDirector::loadDigits(char *base, KTexture **dst)
{
    char name[0x40];
    int  i;

    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00495c50
        sprintf(name, "%s%01d.bmp", base, i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &dst[i], 0);
    }

    return 1;
}

/* ResultDirector's own copy of the digit loader. The original's linker
   ICF-folded the two, so both annotations carry the SAME address and both
   must be FUNCTIONs: a `// STUB: ... folded` alias here turns 0x454480 into a
   stub entity and the real body above stops being scored altogether - which
   is exactly what had happened (`TotalResultDirector::loadDigits is a stub`
   in the table, with the reconstruction sitting right there, correct). */
// FUNCTION: EZ2AC_UNPACKED 0x00454480 folded
int ResultDirector::loadDigits(char *base, KTexture **dst)
{
    char name[0x40];
    int  i;

    for (i = 0; i < 10; i++)
    {
        sprintf(name, "%s%01d.bmp", base, i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &dst[i], 0);
    }

    return 1;
}

/* And GFResultDirector's copy - same fold, same rule as above. */
// FUNCTION: EZ2AC_UNPACKED 0x00454480 folded
int GFResultDirector::loadDigits(char *base, KTexture **dst)
{
    char name[0x40];
    int  i;

    for (i = 0; i < 10; i++)
    {
        sprintf(name, "%s%01d.bmp", base, i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &dst[i], 0);
    }

    return 1;
}

/* Two 0x50-byte records in .rdata, one per player, that the result screens
   index by the walked node's id. Only the two fields the draw reads are named;
   the records' contents are the original's data and are not reproduced. */
struct ResultLine
{
    int  f0;
    int  f4;
    char _pad8[0x50 - 8];
};

// GLOBAL: EZ2AC_UNPACKED 0x004957b0
ResultLine g_resultLines[2];
/* The GF readout's full position table - ten pairs per side; the old
   g_gfResultLines @0x49b128 was pair 8 of this. The two row indexes
   and the clear-marker pointer pair follow it. */
// GLOBAL: EZ2AC_UNPACKED 0x0049b0e8
GfrLine g_gfrPos[2][10];
// GLOBAL: EZ2AC_UNPACKED 0x0049b1d8
int g_rankRow[2];
// GLOBAL: EZ2AC_UNPACKED 0x0049b1e0
int g_rankRowSmall[2];
// GLOBAL: EZ2AC_UNPACKED 0x0049b1e8
char *g_clearMark[5];        /* 20 zero bytes in .rdata up to the class name; read at [0] or [2] */

/* Walk list A and draw one line per player: the player's own chart, the two
   constants its record carries and the texture the screen holds for it. The
   node's id is what indexes all three. */
// FUNCTION: EZ2AC_UNPACKED 0x00451b00
void ResultDirector::drawPlayers()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            int id = cur->f8;
            /* A named pointer, not two indexed reads: the original computes
               the record's address once and reads both fields through it. */
            ResultLine *r = &g_resultLines[id];

            drawLine(&m_2c[id], r->f4, m_2c64[id], r->f0, &m_2a0c);
        }
        while (nextListA(&cur) != 0);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00468050
void GFResultDirector::drawPlayers()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            int id = cur->f8;
            /* 0x49b128 + id*0x50 - pair 8 of the side's row, which is
               where the old two-record g_gfResultLines model pointed. */
            ResultLine *r = (ResultLine *)&g_gfrPos[id][8];

            drawLine(&m_1c[id], r->f4, m_2934[id], r->f0, &m_228c);
        }
        while (nextListA(&cur) != 0);
    }
}

/* ResultDirector::m4510a0 @0x4510a0 - latch each player's score, then settle
   the battle pair.

   Every joined player whose +0x2c64 entry is positive gets its score copied
   into the latch. Then, if a winner was decided, the winner's node takes the
   COMBINED total of both players and the loser's takes zero; the -2 case
   instead gives each of the two nodes its own score untouched.

   The `> 0` test really is signed - the original is `cmp [..],0; jle`, not
   the `test/je` a pointer test compiles to - even though the same array is
   passed to drawLine as a KTexture* by drawPlayers @0x451b00 just above. The
   type is left as the matched function needs it and the comparison is done
   through a cast rather than changing it on this function's word alone.

   100% EFFECTIVE, not byte-identical: `total` lands in edi where the original
   uses ebx, and nothing else differs. Hoisting it to function scope - the
   declaration-order lever - changes nothing, so it is the register coin. */
extern ListNode *findListA(int key);   /* 0x418e30 (src/nodelists.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x004510a0
void ResultDirector::m4510a0()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            int id = cur->f8;

            if ((int)m_2c64[id] > 0)
                m_scoreLatch[id] = m_score[id];
        }
        while (nextListA(&cur) != 0);
    }

    if (winner != -1)
    {
        findListA(winner)->f1ec = m_score[winner] + m_score[loser];
        findListA(loser)->f1ec = 0;
    }
    else if (loser == -2)
    {
        findListA(0)->f1ec = m_score[0];
        findListA(1)->f1ec = m_score[1];
    }
}

/* GFResultDirector::m467820 @0x467820 - ResultDirector::m4510a0's twin, 56 of
   56 instructions identical after masking.

   The offsets are NOT a uniform shift, which is worth stating because the
   instruction-level sameness invites assuming one: the score pair sits 0x330
   below its ResultDirector counterpart, the latch 0x340 and the winner pair
   0x348. The padding between the three groups differs between the classes. */
// FUNCTION: EZ2AC_UNPACKED 0x00467820
void GFResultDirector::m467820()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            int id = cur->f8;

            if ((int)m_2934[id] > 0)
                m_scoreLatch[id] = m_score[id];
        }
        while (nextListA(&cur) != 0);
    }

    if (winner != -1)
    {
        findListA(winner)->f1ec = m_score[winner] + m_score[loser];
        findListA(loser)->f1ec = 0;
    }
    else if (loser == -2)
    {
        findListA(0)->f1ec = m_score[0];
        findListA(1)->f1ec = m_score[1];
    }
}

/* THE SONG-LIST LOADER @0x425250 - the constructor's last act. Reads
   system\<mode>\Stage.ini: for each of the two slots, STAGE<n> (n = the
   element + 2 + slot) is a `;`-list of song names appended to the slot,
   then HID<k>-<n> for k = 1..22 while g_hidCounts[k-1] > 0 (a second read
   after a probe), each list split in place (double-terminated, `;` -> 0)
   and copied token by token, up to 436 per slot. Slot 1 additionally runs
   the same block on an EMPTY buffer after clearing f39a48 (dead in effect,
   kept as the original has it). Then every entry gets its level from
   "<mode>1p-<name>.ini" under sound\<name-without-"-xx">\ and its
   discsmall texture, the slot's f3c = count * 50; finally slot 1's count
   loses f39a48, the disc mask and 19 difficulty textures load, and f4c is 90
   with a first list-A player, else 550.

   ~40% by reccmp (2026-08-19) - decoded and semantically complete. The
   original keeps the constant 1 in esi through the HID loop and homes k, g
   and n in the frame (five slots below the KProfile); cl here keeps them in
   registers and the frame is 0x10 smaller - the constant-register wall of
   CLAUDE.md, plus a this-in-ebx / esi rotation. Inline helpers were tried
   and are worse (cl does not inline them). */
extern int g_hidCounts[22];   /* 0x1b2e848 - HID<k> list counts (src/globals.cpp) */
extern int getListAF8();      /* 0x418340 (src/nodelists.cpp) */

/* 38.98% -> 56.66%, and the whole of the first jump was ONE MISSING LOCAL
   that the frame arithmetic named. With every `lea [esp+X]` normalised to
   the entry esp (tracking the pending pushes, and remembering that
   `lea 0x0(%esp),%esp` is an alignment nop and not a site), the layout is:

     KProfile  F-0x2258   0x1c     ends exactly where the next object starts
     key       F-0x223c   0x10     <- THIS, the sixteen bytes we were short
     path      F-0x222c   0x108
     name      F-0x2124   0x108
     buf       F-0x201c   0x2000   ends at F-0x1c

   The HID key gets its OWN buffer; this reconstruction had been building it
   in `path`. Sixteen bytes, one local, seventeen points.

   The rest is register rotation - the original's `this` is ebx and its zero
   esi where cl uses esi and edi - plus the loop index sitting at [esp+0x10]
   against our [esp+0x18], so there is still an 8-byte difference in the
   scalar area BELOW the KProfile. Same method applies; the scalars are
   harder because they have no leas to normalise. */
// FUNCTION: EZ2AC_UNPACKED 0x00425250
void InGameSongSelect::m425250()
{
    char path[0x108];
    char name[0x108];
    /* The HID key gets its OWN sixteen bytes - it is not built in `path`.
       That is the whole of the frame's 0x10 shortfall. */
    char key[0x10];
    char buf[0x2000];
    int i;
    int j;
    char *p;

    memset(buf, 0, sizeof(buf));
    // STRING: EZ2AC_UNPACKED 0x0048cff0
    sprintf(path, "system\\%s\\Stage.ini", g_modeName);
    KProfile prof(0xc0000, 0);
    prof.Load(path);
    f39a48 = 0;
    for (i = 0; i < 2; i++)
    {
        int n;
        int k;
        int *g;

        m_count[i] = 0;
        f34[i] = 0;
        f2c[i] = 0;
        // STRING: EZ2AC_UNPACKED 0x0048fd24
        sprintf(path, "STAGE%d", g_elemCount + i + 2);
        prof.getStr(path, buf);
        buf[strlen(buf) + 1] = 0;
        p = buf;
        if (*p != 0)
        {
            do
            {
                if (*p == ';')
                    *p = 0;
                p++;
            }
            while (*p != 0);
        }
        p = buf;
        if (*p != 0)
        {
            do
            {
                strcpy(m_entry[i][m_count[i]++].name, p);
                p += strlen(p) + 1;
            }
            while (m_count[i] != 0x1b4 && *p != 0);
        }
        k = 1;
        g = g_hidCounts;
        for (n = 22; n != 0; n--)
        {
            if (*g > 0)
            {
                // STRING: EZ2AC_UNPACKED 0x0048fd18
                sprintf(key, "HID%d-%d", k, g_elemCount + i + 2);
                if (prof.getStr(key, buf) == 0)
                {
                    g++;
                    k++;
                }
                else
                {
                    buf[0] = 0;
                    prof.getStr(key, buf);
                    buf[strlen(buf) + 1] = 0;
                    p = buf;
                    if (*p != 0)
                    {
                        do
                        {
                            if (*p == ';')
                                *p = 0;
                            p++;
                        }
                        while (*p != 0);
                    }
                    p = buf;
                    if (*p != 0)
                    {
                        do
                        {
                            strcpy(m_entry[i][m_count[i]++].name, p);
                            p += strlen(p) + 1;
                        }
                        while (*p != 0);
                    }
                    g++;
                    k++;
                }
            }
        }
        if (i == 1)
        {
            memset(buf, 0, sizeof(buf));
            f39a48 = 0;
            buf[strlen(buf) + 1] = 0;
            p = buf;
            if (*p != 0)
            {
                do
                {
                    if (*p == ';')
                        *p = 0;
                    p++;
                }
                while (*p != 0);
            }
            p = buf;
            if (*p != 0)
            {
                do
                {
                    strcpy(m_entry[1][m_count[1]++].name, p);
                    p += strlen(p) + 1;
                }
                while (m_count[1] != 0x1b4 && *p != 0);
            }
        }
        for (j = 0; j < m_count[i]; j++)
        {
            strcpy(name, m_entry[i][j].name);
            if (name[strlen(name) - 3] == '-')
                name[strlen(name) - 3] = 0;
            // STRING: EZ2AC_UNPACKED 0x0048fd0c
            sprintf(path, "sound\\%s\\", name);
            changeDir(path, 1);
            m_entry[i][j].level = m425140(m_entry[i][j].name);
            popDirectory();
            // STRING: EZ2AC_UNPACKED 0x0048cfd0
            sprintf(path, "system\\discsmall\\%s.bmp", m_entry[i][j].name);
            ((KGraphics *)g_sub1b2f054)->loadTexture(path, &m_entry[i][j].tex, 0);
        }
        f3c[i] = (float)m_count[i] * 50.0f;
    }
    m_count[1] -= f39a48;
    // STRING: EZ2AC_UNPACKED 0x0048fcec
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\disc\\disc-mask.bmp", &m_399a0, 0);
    for (i = 0; i < 19; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0048fcc4
        sprintf(path, "system\\clubmix\\panel\\difficulty_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, &m_diff[i], 0);
    }
    f4c = (getListAF8() == 0) ? 550.0f : 90.0f;
}

/* THE TEST-MENU SINK @0x4763e0 - TestModeDirector as its second base, so
   the director's fields sit at (offset - 0x10) from this `this`. `what` 1 =
   a value changed: row 0 is the language, published to g_language and back
   into the menu. `what` 0 = a row chosen: remember it in f20, then delete
   the menu and open that page's own screen (1 video, 6 game option, 2 input,
   3 lighting, 4 sound, 5 bookkeeping), or for row 7 write the language into
   <exe>.ini, re-encrypt it through a KParser and tell the sound bank.
   `what` 2 = leave: f14 = 1. */
struct TestModeSinkView
{
    char _pad0[4];
    int  f14;                  /* the director's f14 */
    int  f18;
    SystemMenu *m_menu;        /* the director's m_menu */
    int  f20;                  /* the director's f20 */
};
extern int g_language;        /* 0x16963f4 (src/testinput.cpp) */
// FUNCTION: EZ2AC_UNPACKED 0x004763e0
void TestModeSink::onMenu(int what, int page, int c)
{
    TestModeSinkView *d = (TestModeSinkView *)this;
    int item = page;
    char path[0x104];

    if (what == 1)
    {
        if (item == 0)
        {
            g_language = d->m_menu->m_rows[0].value;
            d->m_menu->m_language = g_language;
        }
    }
    else if (what == 0)
    {
        d->f20 = item;
        if (item == 1)
        {
            delete d->m_menu;
            createTestVideo();
        }
        else if (item == 6)
        {
            delete d->m_menu;
            createTestGameOption();
        }
        else if (item == 2)
        {
            delete d->m_menu;
            createTestInput();
        }
        else if (item == 3)
        {
            delete d->m_menu;
            createTestLighting();
        }
        else if (item == 4)
        {
            delete d->m_menu;
            createTestSound();
        }
        else if (item == 5)
        {
            delete d->m_menu;
            createTestBookkeeping();
        }
        else if (item == 7)
        {
            g_language = d->m_menu->m_rows[0].value;
            sprintf(path, "%s.ini", g_app->className());
            // STRING: EZ2AC_UNPACKED 0x0049f984
            g_iniProfile->setInt("Language", g_language);
            g_iniProfile->writeFile(path);
            {
                KParser p;
                p.m4133d0(path);
                ((SoundBank *)g_1b2e844)->v0(0);
            }
        }
    }
    else if (what == 2)
    {
        d->f14 = 1;
    }
}

/* TestModeDirector's own draw @0x476640: clear, the navigation hint in the
   menu's language (Korean row placeholder storage @0x49f9e0), and the
   version footer. 93%: the English arm loads the graphics pointer into edx
   where the original has eax - one temporary, three forms tried. */
extern void drawTextf(struct KGraphics *g, int x, int y, const char *fmt, ...);   /* 0x4071d0 */
struct Obj406C00 { void setField12c(int v); };            /* 0x406c00 - the text colour */
extern int g_version;                                     /* 0x4a8d6c (src/testinput.cpp) */
// GLOBAL: EZ2AC_UNPACKED 0x0049f9e0
char g_strTestNavKor[0x50];

// FUNCTION: EZ2AC_UNPACKED 0x00476640
void TestModeDirector::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    if (m_menu->m_language == 0)
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, g_strTestNavKor);
    }
    else
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x0049f990
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, "1P Start Button to Up, 2P Start Button to Down, Effect 1 Button to Enter");
    }
    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffc8c8);
    drawTextf((KGraphics *)g_sub1b2f054, 525, 22, "Version %d.%02d", g_version / 100, g_version % 100);
}

/* What ScrFader's +0x1c points at - the director's own sink, whose slot 0 is
   told which ramp finished. Only that one slot is ever reached. */
struct FadeSink { virtual void onFade(ScrFader *sender, int code); };

extern int g_37dd06c;   /* 0x037dd06c (src/globals.cpp) */

/* ScrFader::update @0x450450 - one frame of a screen fade.

   State 0 is idle and returns without even publishing. Otherwise the level
   goes to the device as a fade (except in state 3, which dims rather than
   fades and drives only the published copy), then the state steps it:

     1  fade IN  - climb by f14 to 0xff, clamp there; at 0xff the ramp is
        over, so the state clears and the sink is told 1;
     2  fade OUT - fall by f14 to 0, clamp there; at 0 the state clears and
        the sink is told 2;
     3  DIM      - fall by f14 to 0x80 and clamp there; at or below 0x80 the
        sink is told 4 and the state becomes 4, not 0, so the dim latches
        instead of ending.

   States 1 and 2 clear the state BEFORE telling the sink; state 3 sets it
   AFTER. Every path but the idle one ends by publishing the level to
   g_37dd06c, and cl duplicates that tail into the paths that return early. */
// FUNCTION: EZ2AC_UNPACKED 0x00450450
void ScrFader::update()
{
    if (f10 == 0)
        return;

    if (f10 != 3)
        ((KGraphics *)g_sub1b2f054)->setFade(f18);

    if (f10 == 1)
    {
        if (f18 == 0xff)
        {
            f10 = 0;
            ((FadeSink *)f1c)->onFade(this, 1);
        }
        else
        {
            f18 = f18 + f14;
            if (f18 >= 0xff)
                f18 = 0xff;
        }
    }
    else if (f10 == 2)
    {
        if (f18 == 0)
        {
            f10 = 0;
            ((FadeSink *)f1c)->onFade(this, 2);
        }
        else
        {
            f18 = f18 - f14;
            if (f18 < 0)
                f18 = 0;
        }
    }
    else if (f10 == 3)
    {
        if (f18 <= 0x80)
        {
            ((FadeSink *)f1c)->onFade(this, 4);
            f10 = 4;
        }
        else
        {
            f18 = f18 - f14;
            if (f18 < 0x80)
                f18 = 0x80;
        }
    }

    /* ONE publish, written once. cl duplicates it into every path that
       returns early, and each copy RE-READS f18 out of the member - which is
       what the original's `mov ecx,[esi+0x18]` before each store is. Written
       out per path instead, cl forwards the store it just made and publishes
       the register. */
    g_37dd06c = f18;
}

/* What the GF director's first sink reports. `this` is the SINK, so its three
   fields go through a view, as the catch one above does. Only kind 2 does
   anything: run state 3 hands the driver 5 outright; run state 1 hands it 3
   when the flag at sink+0x30c is up and the stored code at sink+0xc94
   otherwise. Anything else is ignored.

   NOTE the flag test is against 1, not against "the run state": the original
   spells it `cmp [ecx+0x30c],eax` only because eax still holds the 1 the
   state was just compared with. */
struct GFSink1Fields
{
    char _pad0[0x10];
    int  f10;                  /* sink+0x010 - the run state */
    char _pad14[0x30c - 0x14];
    int  f30c;                 /* sink+0x30c */
    char _pad310[0xc94 - 0x310];
    int  fc94;                 /* sink+0xc94 - the exit code */
};

// FUNCTION: EZ2AC_UNPACKED 0x0045d910
void GFSink1::v0(int, int what)
{
    GFSink1Fields *f = (GFSink1Fields *)this;

    if (what == 2)
    {
        if (f->f10 == 3)
        {
            ((Obj401780 *)g_1b2e844)->armAndSetC(5);
            return;
        }

        if (f->f10 == 1)
        {
            if (f->f30c == 1)
            {
                ((Obj401780 *)g_1b2e844)->armAndSetC(3);
                return;
            }

            ((Obj401780 *)g_1b2e844)->armAndSetC(f->fc94);
        }
    }
}
