#include <stdio.h>
#include <string.h>
#include "panels.h"
#include "ez2acapp.h"   /* KGraphics, EzSubsystem, the subsystem singletons */
#include "kfile.h"
#include "loader.h"     /* KTexture */

/* The in-game speed read-out: three decimal figures written into the fifth of
   the panel's nine charts, then handed to that chart's refresh @0x403940.

   Every digit is `table[value / place % 10]`, which cl 13.10 compiles to a
   magic multiply for the division by 10/100/1000 and a real `idiv` for the
   `% 10` - the original's exact idiom, confirmed in the zoo.

   The four modes 6..9 shift every row down by two: the two blocks are written
   out in full because the original has them in full, one per branch, each with
   its own tail call. */

/* The song's own tempo, read off the player and rounded to an int
   (@0x429ebd cvttss2si, @0x429ee6 fistp). Three digits. */
/* The audience-rate figures, all three in the 0..1 range. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebd8
float g_rateTarget;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e860
float g_rate1p;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e864
float g_rate2p;

// GLOBAL: EZ2AC_UNPACKED 0x01b2eac0
int g_songBpm;

/* The player's speed setting as a percentage: adjusted in steps of 25 and
   clamped to [50, 999] by the +/- handlers @0x422f48 and @0x422fd7. Three
   digits, and it is this that multiplies g_songBpm into m_scrollBpm. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e890
int g_speedPercent;

extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0041d1d0
void Effector6thStyle::updateSpeedDigits()
{
    if (g_modeIndex != 6 && g_modeIndex != 7 && g_modeIndex != 8 &&
        g_modeIndex != 9)
    {
        Obj402860 *c = &chart4;

        c->setCell(7, 0, (int)m_fontM[g_songBpm / 100 % 10]);
        c->setCell(9, 0, (int)m_fontM[g_songBpm / 10 % 10]);
        c->setCell(11, 0, (int)m_fontM[g_songBpm % 10]);
        c->setCell(8, 0, (int)m_fontW[g_songBpm / 100 % 10]);
        c->setCell(10, 0, (int)m_fontW[g_songBpm / 10 % 10]);
        c->setCell(12, 0, (int)m_fontW[g_songBpm % 10]);
        c->setCell(13, 0, (int)m_fontM[g_speedPercent / 100 % 10]);
        c->setCell(15, 0, (int)m_fontM[g_speedPercent / 10 % 10]);
        c->setCell(17, 0, (int)m_fontM[g_speedPercent % 10]);
        c->setCell(14, 0, (int)m_fontW[g_speedPercent / 100 % 10]);
        c->setCell(16, 0, (int)m_fontW[g_speedPercent / 10 % 10]);
        c->setCell(18, 0, (int)m_fontW[g_speedPercent % 10]);
        c->setCell(19, 0, (int)m_fontM[m_scrollBpm / 1000 % 10]);
        c->setCell(21, 0, (int)m_fontM[m_scrollBpm / 100 % 10]);
        c->setCell(23, 0, (int)m_fontM[m_scrollBpm / 10 % 10]);
        c->setCell(25, 0, (int)m_fontM[m_scrollBpm % 10]);
        c->setCell(20, 0, (int)m_fontG[m_scrollBpm / 1000 % 10]);
        c->setCell(22, 0, (int)m_fontG[m_scrollBpm / 100 % 10]);
        c->setCell(24, 0, (int)m_fontG[m_scrollBpm / 10 % 10]);
        c->setCell(26, 0, (int)m_fontG[m_scrollBpm % 10]);
        c->m403940();
    }
    else
    {
        Obj402860 *c = &chart4;

        c->setCell(9, 0, (int)m_fontM[g_songBpm / 100 % 10]);
        c->setCell(11, 0, (int)m_fontM[g_songBpm / 10 % 10]);
        c->setCell(13, 0, (int)m_fontM[g_songBpm % 10]);
        c->setCell(10, 0, (int)m_fontW[g_songBpm / 100 % 10]);
        c->setCell(12, 0, (int)m_fontW[g_songBpm / 10 % 10]);
        c->setCell(14, 0, (int)m_fontW[g_songBpm % 10]);
        c->setCell(15, 0, (int)m_fontM[g_speedPercent / 100 % 10]);
        c->setCell(17, 0, (int)m_fontM[g_speedPercent / 10 % 10]);
        c->setCell(19, 0, (int)m_fontM[g_speedPercent % 10]);
        c->setCell(16, 0, (int)m_fontW[g_speedPercent / 100 % 10]);
        c->setCell(18, 0, (int)m_fontW[g_speedPercent / 10 % 10]);
        c->setCell(20, 0, (int)m_fontW[g_speedPercent % 10]);
        c->setCell(21, 0, (int)m_fontM[m_scrollBpm / 1000 % 10]);
        c->setCell(23, 0, (int)m_fontM[m_scrollBpm / 100 % 10]);
        c->setCell(25, 0, (int)m_fontM[m_scrollBpm / 10 % 10]);
        c->setCell(27, 0, (int)m_fontM[m_scrollBpm % 10]);
        c->setCell(22, 0, (int)m_fontG[m_scrollBpm / 1000 % 10]);
        c->setCell(24, 0, (int)m_fontG[m_scrollBpm / 100 % 10]);
        c->setCell(26, 0, (int)m_fontG[m_scrollBpm / 10 % 10]);
        c->setCell(28, 0, (int)m_fontG[m_scrollBpm % 10]);
        c->m403940();
    }
}

/* The two adjacent leaf getters that are slots 0 and 2 of every panel class's
   vtable. Storage for both descriptors is in src/classinfo.cpp with the rest of
   the family. */
extern KRuntimeClass classEffector6thStyle;   /* 0x4a8ff4 */
extern DrawGroup g_info_4a9004;               /* 0x4a9004 */

// FUNCTION: EZ2AC_UNPACKED 0x0041d0c0
KRuntimeClass *Effector6thStyle::getRuntimeClass()
{
    return &classEffector6thStyle;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041d0d0
DrawGroup *Effector6thStyle::getObjectInfo()
{
    return &g_info_4a9004;
}

/* Nine member destructors in reverse declaration order, each preceded by its
   EH state, then the base's - which is the bare folded `ret` @0x402880, called
   DIRECTLY rather than through KBase's thunk @0x426e40. ~Obj402860 and
   ~EzObject both have to live in other TUs (src/subobj402860.cpp and
   src/stubs.cpp) or cl inlines them and all ten calls vanish. */
// FUNCTION: EZ2AC_UNPACKED 0x0041d0e0
Effector6thStyle::~Effector6thStyle()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0041e600
// Effector6thStyle::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`, ICF-folded across every class
   that has one; src/constants.cpp owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int Effector6thStyle::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048eb30
struct Effector6thStyle;

/* ------------------------------------------------------------------------- */
/* The four sibling panel classes. Storage for every descriptor is in
   src/classinfo.cpp; the getters are vtable slots, so they belong here. */

extern KRuntimeClass classAudienceRatePanel;  /* 0x4a8ed0 */
extern DrawGroup g_info_4a8ee0;               /* 0x4a8ee0 */
extern KRuntimeClass classBackGround;         /* 0x4a8f0c */
extern DrawGroup g_info_4a8f1c;               /* 0x4a8f1c */
extern KRuntimeClass classBattleMode;         /* 0x4a8f48 */
extern DrawGroup g_info_4a8f58;               /* 0x4a8f58 */
extern KRuntimeClass classEffector;           /* 0x4a8f84 */
extern DrawGroup g_info_4a8f94;               /* 0x4a8f94 */

// FUNCTION: EZ2AC_UNPACKED 0x004199f0
KRuntimeClass *AudienceRatePanel::getRuntimeClass()
{
    return &classAudienceRatePanel;
}

// FUNCTION: EZ2AC_UNPACKED 0x00419a00
DrawGroup *AudienceRatePanel::getObjectInfo()
{
    return &g_info_4a8ee0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00419a10
AudienceRatePanel::~AudienceRatePanel()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0041ad30
// AudienceRatePanel::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int AudienceRatePanel::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048ddd8
struct AudienceRatePanel;

// FUNCTION: EZ2AC_UNPACKED 0x0041adb0
KRuntimeClass *BackGround::getRuntimeClass()
{
    return &classBackGround;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041adc0
DrawGroup *BackGround::getObjectInfo()
{
    return &g_info_4a8f1c;
}

/* Tear the flag block down and free the mesh's buffers; the helper at +0x1c0
   and the ten charts go on the way out. Byte-identical to its two siblings in
   src/backgrounds.cpp apart from the vtable constant. */
// FUNCTION: EZ2AC_UNPACKED 0x0041aeb0
BackGround::~BackGround()
{
    ((KGraphics *)g_sub1b2f054)->destroyFlag(&m_flag);
    ((KGraphics *)g_sub1b2f054)->freeBuffers(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0041b660
// BackGround::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int BackGround::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048e1b8
struct BackGround;

// FUNCTION: EZ2AC_UNPACKED 0x0041bb90
KRuntimeClass *BattleMode::getRuntimeClass()
{
    return &classBattleMode;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041bba0
DrawGroup *BattleMode::getObjectInfo()
{
    return &g_info_4a8f58;
}

/* The two children are notified through slot 5 before the charts are cleared;
   the three cleared charts are 1, 2 and 0, in that order. */
// FUNCTION: EZ2AC_UNPACKED 0x0041bbb0
BattleMode::~BattleMode()
{
    if (m_a != 0)
    {
        m_a->q5();
    }

    if (m_b != 0)
    {
        m_b->q5();
    }

    chart1.clear();
    chart2.clear();
    chart0.clear();
}

// FUNCTION: EZ2AC_UNPACKED 0x0041c640
// BattleMode::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int BattleMode::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048e498
struct BattleMode;

// FUNCTION: EZ2AC_UNPACKED 0x0041ca00
KRuntimeClass *Effector::getRuntimeClass()
{
    return &classEffector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041ca10
DrawGroup *Effector::getObjectInfo()
{
    return &g_info_4a8f94;
}

/* No member has a destructor, so this is the vptr store and a tail jump to the
   base's folded `ret` - eleven bytes, and no EH frame. */
// FUNCTION: EZ2AC_UNPACKED 0x0041ca20
Effector::~Effector()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x0041d040
// Effector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int Effector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048e8f0
struct Effector;

extern int g_demoPlay;   /* 0x1b2ef78, below */

/* The whole body is the demo-play overlay: outside demo play the effector has
   nothing to load, so the two textures stay null. */
// FUNCTION: EZ2AC_UNPACKED 0x0041cfb0
Effector::Effector()
{
    f14 = 0;

    /* Both literals are already annotated by Effector6thStyle's constructor
       further down; the entity is keyed by address, so one marker is enough. */
    if (g_demoPlay != 0)
    {
        ((KGraphics *)g_sub1b2f054)->loadTexture("..\\..\\Common\\DEMOPLAY.bmp", &m_demoPlay, 0);
        ((KGraphics *)g_sub1b2f054)->loadTexture("..\\..\\Common\\DEMOPLAY_mask.bmp", &m_demoPlayMask, 0);
    }
}

/* ------------------------------------------------------------------------- */

extern int g_1b2eb6c;          /* src/ez2acapp.cpp */
extern int countListA();                        /* 0x418310 (src/nodelists.cpp) */
struct ListNode;
extern ListNode *findListA(int key);            /* 0x418e30 (src/nodelists.cpp) */
extern void changeDir(char *path, int savePrev);/* 0x412dd0 (src/strutil.cpp) */
extern void popDirectory();                     /* 0x412e60 (src/strutil.cpp) */
extern int g_elemCount;        /* 0x1b2e718 (src/loops.cpp) */

/* Which mix is running; 7 selects the Street Mix 1st layout throughout. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebc8
int g_mixStyle;

/* ---- the background's video path ---------------------------------------- */

extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */

/* The graphics singleton again, under the name its already-matched COM
   wrappers use (src/com_wrappers.cpp). Only the slot shape matters here. */
struct Obj40BB00
{
    void doOp7(int v);   /* 0x40a470 */
};

/* Reset to 0 by every one of the three background constructors and by nothing
   else that is reconstructed yet, so only its type is evidenced. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea14
int g_1b2ea14;

/* The fallback name used whenever g_1b2eb6c is set: it spells both the video
   directory and the file stem, so the same pointer goes into both `%s`. It is
   already declared further down as g_songInfoName - the same buffer the
   "<name>_info.bmp" overlay is built from. */
extern char g_songInfoName[64];   /* 0x1b2eb80 */

/* BackGround's own copy of the trim buffer. Each of the three classes owns a
   distinct one, which is exactly why the original's ICF left the three
   otherwise identical videoName() bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037da7c0
static char g_bgVideoName[260];

/* Copy the current text slot and drop a trailing "-xx" / "-xxx" / "-xxxx"
   suffix. strlen is recomputed at every use because the source calls it at
   every use - cl expands it inline and does not common the copies. */
// FUNCTION: EZ2AC_UNPACKED 0x0041add0
char *BackGround::videoName()
{
    strcpy(g_bgVideoName, g_textSlots[g_elemCount]);

    if (g_bgVideoName[strlen(g_bgVideoName) - 3] == '-')
        g_bgVideoName[strlen(g_bgVideoName) - 3] = 0;
    else if (g_bgVideoName[strlen(g_bgVideoName) - 4] == '-')
        g_bgVideoName[strlen(g_bgVideoName) - 4] = 0;
    else if (g_bgVideoName[strlen(g_bgVideoName) - 5] == '-')
        g_bgVideoName[strlen(g_bgVideoName) - 5] = 0;

    return g_bgVideoName;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041b4f0
BackGround::BackGround()
{
    char cwd[260];

    f102118 = 0;
    f24 = 0;
    f698 = 0;
    f20 = 0;
    g_1b2ea14 = 0;
    f10 = 0;
    f14 = 0;
    memset(m_items, 0, sizeof(m_items));
    fa8 = 0;
    ((KGraphics *)g_sub1b2f054)->createFlag(&m_flag, 0.3f, 0.00012f);
    m_mesh = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    GetCurrentDirectoryA(260, cwd);

    if (g_1b2eb6c == 0)
        // STRING: EZ2AC_UNPACKED 0x0048e27c
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, videoName(), videoName());
    else
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, g_songInfoName, g_songInfoName);
}

/* Nonzero while the attract loop owns the screen. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef78
int g_demoPlay;

/* The song name the "<name>_info.bmp" overlay is built from. Size is not
   evidenced - placeholder storage, only the address matters. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb80
char g_songInfoName[64];

// GLOBAL: EZ2AC_UNPACKED 0x01b5f154
int g_1b5f154;

/* Battle mode's panel: two samples, nine textures and up to five charts, each
   read out of a .str under whichever base directory the mix selects. The KFile
   local is never used - the original still builds and tears one down around the
   Street Mix block, which is what its EH state 8 covers.

   Three shapes here are cl's doing rather than the source's. The `push 1` that
   feeds changeDir sits ABOVE the branch that picks its path, because the merger
   found it common to both arms. Each pair of "load this .str or that one"
   calls is written as two separate calls, which cl cross-jumps back into one -
   a ternary between the two names would go branchless instead. And the three
   arms of the closing chart write share their last setCell. */
// FUNCTION: EZ2AC_UNPACKED 0x0041c1a0
BattleMode::BattleMode()
{
    Obj402860Head *res;
    char name[260];
    char info[260];

    m_f14 = 0;
    m_f18 = 0;
    m_f20 = 0.0f;
    m_f24 = 0.0f;
    m_f28 = 0.0f;
    m_f2c = 1;
    m_f2d = 0;
    m_f30 = 0x3c;
    m_f34 = 1;
    m_f1c = 0;
    m_f3c = 0;
    m_f38 = 0;
    g_1b5f154 = 0;

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e680
        changeDir("System\\BattleMode\\", 1);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048e664
        changeDir("System\\CV2Mix\\BattleMode\\", 1);
    }

    // STRING: EZ2AC_UNPACKED 0x0048e654
    m_a = (EzSoundObject *)g_sub1b2eae4->createSample("battle-01.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0048e644
    m_b = (EzSoundObject *)g_sub1b2eae4->createSample("battle-02.wav", 0, 0);

    // STRING: EZ2AC_UNPACKED 0x0048e630
    ((KGraphics *)g_sub1b2f054)->loadTexture("typo_get_mask.bmp", &m_getMask, 0);
    // STRING: EZ2AC_UNPACKED 0x0048e620
    ((KGraphics *)g_sub1b2f054)->loadTexture("typo_get.bmp", &m_get, 0);

    // STRING: EZ2AC_UNPACKED 0x0048e604
    sprintf(name, "BATTLE_BEAT_%02d_mask.bmp", g_elemCount + 1);
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &m_beatMask, 0);

    // STRING: EZ2AC_UNPACKED 0x0048e5ec
    sprintf(name, "BATTLE_BEAT_%02d.bmp", g_elemCount + 1);
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &m_beat, 0);

    if (countListA() > 1 && g_modeIndex != 1)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e5d8
        ((KGraphics *)g_sub1b2f054)->loadStr("BATTLE_READY.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048e5c8
        ((KGraphics *)g_sub1b2f054)->loadStr("READYCOUNT.str", &res);
    }
    chart0.attach(res, 0);

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e5b8
        ((KGraphics *)g_sub1b2f054)->loadStr("battle_OBJ.str", &res);
        chart1.attach(res, 1);

        // STRING: EZ2AC_UNPACKED 0x0048e5a8
        ((KGraphics *)g_sub1b2f054)->loadStr("battle_OBJ2.str", &res);
        chart2.attach(res, 1);

        // STRING: EZ2AC_UNPACKED 0x0048e594
        ((KGraphics *)g_sub1b2f054)->loadTexture("battle_Counter.bmp", &m_counter, 0);
        // STRING: EZ2AC_UNPACKED 0x0048e580
        ((KGraphics *)g_sub1b2f054)->loadTexture("BATTLE_1pwin.bmp", &m_win1p, 0);
        // STRING: EZ2AC_UNPACKED 0x0048e56c
        ((KGraphics *)g_sub1b2f054)->loadTexture("BATTLE_2pwin.bmp", &m_win2p, 0);
        // STRING: EZ2AC_UNPACKED 0x0048e558
        ((KGraphics *)g_sub1b2f054)->loadTexture("BATTLE_3pwin.bmp", &m_win3p, 0);

        chart1.setCell(3, 0, (int)m_win3p);
    }

    popDirectory();

    KFile f;

    if (g_mixStyle == 7)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e53c
        changeDir("System\\cv2mix\\1steyecatch\\", 1);

        if (findListA(0) != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0048e52c
            ((KGraphics *)g_sub1b2f054)->loadStr("songinfo_1p.str", &res);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0048e51c
            ((KGraphics *)g_sub1b2f054)->loadStr("songinfo_2p.str", &res);
        }
        chart4.attach(res, 0);

        // STRING: EZ2AC_UNPACKED 0x0048e510
        sprintf(info, "%s_info.bmp", g_songInfoName);
        ((KGraphics *)g_sub1b2f054)->loadTexture(info, &m_info, 0);

        popDirectory();

        // STRING: EZ2AC_UNPACKED 0x0048e4dc
        ((KGraphics *)g_sub1b2f054)->loadStr("system\\cv2mix\\streetmix1st\\effector\\Wait_Player.str", &res);
        chart5.attach(res, 1);

        chart4.setCell(3, 0, (int)m_info);
    }

    if (g_demoPlay != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e4bc
        ((KGraphics *)g_sub1b2f054)->loadStr("system\\Common\\Demonstration.str", &res);
        chart6.attach(res, 1);
    }

    m_ff30 = 0;
    m_ff31 = 0;
    m_ff32 = 0;

    if (countListA() > 1 && g_modeIndex != 1)
    {
        chart0.setCell(5, 0, (int)m_beatMask);
        chart0.setCell(6, 0, (int)m_beat);
    }
    else if (g_1b2eb6c == 0)
    {
        chart0.setCell(5, 0, (int)m_getMask);
        chart0.setCell(6, 0, (int)m_get);
    }
    else
    {
        chart0.setCell(6, 0, (int)m_beat);
    }
}

/* ------------------------------------------------------------------------- */

extern int g_1b2e914;          /* src/panels.cpp, below */
extern void set_4184e0(int idx, int val);   /* 0x4184e0 (src/globals.cpp) */
extern void Log(const char *fmt, ...);      /* 0x402880 (src/applog.cpp) */

/* Which of the twenty speed textures is showing. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e914
int g_1b2e914;

// GLOBAL: EZ2AC_UNPACKED 0x01b5f158
int g_1b5f158;

/* The effector panel's constructor: nine charts, then every texture and .str
   the read-out needs, picked three ways over - by the mix (g_1b2eb6c), by the
   style within it (g_mixStyle) and by the game mode (g_modeIndex).

   Four separate 260-byte buffers, one per sprintf loop; the frame is 0x41c and
   the four buffers sit exactly 0x104 apart. The graphics singleton is read at
   every call site, as everywhere else in this file. */
// FUNCTION: EZ2AC_UNPACKED 0x0041dcc0
Effector6thStyle::Effector6thStyle()
{
    Obj402860Head *res;
    Obj402860 *c;
    float ox;
    float oy;
    char nameA[260];
    char nameB[260];
    char nameC[260];
    char nameD[260];
    int i;

    if (g_demoPlay != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e958
        ((KGraphics *)g_sub1b2f054)->loadTexture("..\\..\\Common\\DEMOPLAY.bmp", &m_demoPlay, 0);
        // STRING: EZ2AC_UNPACKED 0x0048e938
        ((KGraphics *)g_sub1b2f054)->loadTexture("..\\..\\Common\\DEMOPLAY_mask.bmp", &m_demoPlayMask, 0);
    }

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048f1a8
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\InGameEffector\\TT\\effect_off.bmp", &m_effectOff, 0);
        // STRING: EZ2AC_UNPACKED 0x0048f180
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\InGameEffector\\TT\\effect_on.bmp", &m_effectOn, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048f14c
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\InGameEffector\\effector_bass_off.bmp", &m_bassOff, 0);
        // STRING: EZ2AC_UNPACKED 0x0048f118
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\InGameEffector\\effector_bass_on.bmp", &m_bassOn, 0);
        // STRING: EZ2AC_UNPACKED 0x0048f0e0
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\InGameEffector\\effector_treble_off.bmp", &m_trebleOff, 0);
        // STRING: EZ2AC_UNPACKED 0x0048f0ac
        ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\InGameEffector\\effector_treble_on.bmp", &m_trebleOn, 0);

        if (g_mixStyle == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0048f07c
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT1.bmp", &m_effect[0], 0);
            // STRING: EZ2AC_UNPACKED 0x0048f048
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT1-ON.bmp", &m_effect[1], 0);
            // STRING: EZ2AC_UNPACKED 0x0048f018
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT2.bmp", &m_effect[2], 0);
            // STRING: EZ2AC_UNPACKED 0x0048efe4
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT2-ON.bmp", &m_effect[3], 0);
            // STRING: EZ2AC_UNPACKED 0x0048efb4
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT3.bmp", &m_effect[4], 0);
            // STRING: EZ2AC_UNPACKED 0x0048ef80
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT3-ON.bmp", &m_effect[5], 0);
            // STRING: EZ2AC_UNPACKED 0x0048ef50
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT4.bmp", &m_effect[6], 0);
            // STRING: EZ2AC_UNPACKED 0x0048ef1c
            ((KGraphics *)g_sub1b2f054)->loadTexture("System\\CV2Mix\\streetmix1st\\effector\\EFFECT4-ON.bmp", &m_effect[7], 0);
        }
    }

    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0048eef0
        sprintf(nameA, "System\\InGameEffector\\TT\\speedfontw_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(nameA, &m_fontW[i], 0);
        // STRING: EZ2AC_UNPACKED 0x0048eec4
        sprintf(nameA, "System\\InGameEffector\\TT\\speedfontm_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(nameA, &m_fontM[i], 0);
        // STRING: EZ2AC_UNPACKED 0x0048ee98
        sprintf(nameA, "System\\InGameEffector\\TT\\speedfontG_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(nameA, &m_fontG[i], 0);
    }

    if (g_1b2eb6c == 0)
    {
        for (i = 0; i < 10; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ee6c
            sprintf(nameB, "System\\InGameEffector\\TT\\Speedfont_%d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(nameB, &m_speedFont[i], 0);
        }
    }
    else
    {
        for (i = 0; i < 21; i++)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ee34
            sprintf(nameC, "System\\CV2Mix\\InGameEffector\\Effector_Speed_%02d.bmp", i + 1);
            ((KGraphics *)g_sub1b2f054)->loadTexture(nameC, &m_speed[i], 0);
        }
    }

    for (i = 0; i < 11; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0048ee10
        sprintf(nameD, "System\\InGameEffector\\Speed_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(nameD, &m_speedIcon[i], 0);
    }

    set_4184e0(0x1b, 0);
    set_4184e0(0x1c, 0);

    if (g_mixStyle == 7)
    {
        set_4184e0(0x26, 0);
        set_4184e0(0x27, 0);
        // STRING: EZ2AC_UNPACKED 0x0048ee00
        Log("FALSE EFFECTOR");
    }
    else
    {
        set_4184e0(0x26, 1);
        set_4184e0(0x27, 1);
    }

    /* Every mode gets its own arm even where the bodies are identical. With
       the repeats folded into one label cl compacts the dispatch to a
       byte-index table; written out, it builds the direct twelve-entry table
       the original has and its tail merger folds the bodies afterwards, down
       to the same four blocks. */
    switch (g_modeIndex)
    {
    case 1:
        m_originY = -330;
        m_originX = 0;
        break;
    case 4:
        m_originX = -235;
        m_originY = -380;
        break;
    case 8:
        m_originX = -235;
        m_originY = -380;
        break;
    case 5:
        m_originY = 0;
        m_originX = 0;
        break;
    case 9:
        m_originY = 0;
        m_originX = 0;
        break;
    case 0:
        m_originY = -380;
        m_originX = 0;
        break;
    case 2:
        m_originY = -380;
        m_originX = 0;
        break;
    case 3:
        m_originY = -380;
        m_originX = 0;
        break;
    case 6:
        m_originY = -380;
        m_originX = 0;
        break;
    case 7:
        m_originY = -380;
        m_originX = 0;
        break;
    case 10:
        m_originY = -380;
        m_originX = 0;
        break;
    case 11:
        m_originY = -380;
        m_originX = 0;
        break;
    default:
        m_originY = -380;
        m_originX = 0;
        break;
    }

    if (g_1b2eb6c != 0)
    {
        /* Ten arms and no default: out of range skips the whole thing. */
        switch (g_mixStyle)
        {
        case 3:
            m_originX = -235;
            m_originY = 0;
            break;
        case 0:
            m_originX = 0;
            m_originY = 0;
            break;
        case 1:
            m_originX = 0;
            m_originY = 0;
            break;
        case 2:
            m_originX = 0;
            m_originY = 0;
            break;
        case 4:
            m_originX = 0;
            m_originY = 0;
            break;
        case 5:
            m_originX = 0;
            m_originY = 0;
            break;
        case 6:
            m_originX = 0;
            m_originY = 0;
            break;
        case 7:
            m_originX = 0;
            m_originY = 0;
            break;
        case 8:
            m_originX = 0;
            m_originY = 0;
            break;
        case 9:
            m_originX = 0;
            m_originY = 0;
            break;
        }
    }

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048ede0
        ((KGraphics *)g_sub1b2f054)->loadStr("System\\GaugeOver\\GaugeOver.str", &res);
    }
    else if (g_mixStyle == 7)
    {
        // STRING: EZ2AC_UNPACKED 0x0048edac
        ((KGraphics *)g_sub1b2f054)->loadStr("System\\CV2Mix\\Streetmix1st\\GaugeOver\\GaugeOver.str", &res);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048ed84
        ((KGraphics *)g_sub1b2f054)->loadStr("System\\CV2Mix\\GaugeOver\\GaugeOver.str", &res);
    }
    chart7.attach(res, 0);

    if (g_1b2eb6c == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048ed6c
        changeDir("System\\InGameEffector", 1);

        if (g_modeIndex != 9 && g_modeIndex != 5 && g_modeIndex != 7 &&
            g_modeIndex != 3)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ed54
            ((KGraphics *)g_sub1b2f054)->loadStr("TT\\normal_effect.str", &res);
            chart0.attach(res, 0);

            if (g_modeIndex != 4 && g_modeIndex != 8)
            {
                if (g_modeIndex != 6)
                {
                    // STRING: EZ2AC_UNPACKED 0x0048ed38
                    ((KGraphics *)g_sub1b2f054)->loadStr("TT\\normal_effect_point.str", &res);
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x0048ed18
                    ((KGraphics *)g_sub1b2f054)->loadStr("TT\\normal_effect_point_c.str", &res);
                }
            }
            else if (g_modeIndex == 4)
            {
                // STRING: EZ2AC_UNPACKED 0x0048ed00
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\10k_effect_point.str", &res);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x0048ece4
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\10k_effect_point_c.str", &res);
            }
            chart4.attach(res, 0);
        }
        else if (g_modeIndex == 5 || g_modeIndex == 9)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ec84
            ((KGraphics *)g_sub1b2f054)->loadStr("TT\\spacemix_effect.str", &res);
            chart2.attach(res, 0);

            if (g_modeIndex != 9)
            {
                // STRING: EZ2AC_UNPACKED 0x0048ec6c
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\14k_effect_point.str", &res);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x0048ec50
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\14k_effect_point_c.str", &res);
            }
            chart4.attach(res, 0);
        }
        else if (g_modeIndex == 3 || g_modeIndex == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ecd0
            ((KGraphics *)g_sub1b2f054)->loadStr("TT\\7k_effect.str", &res);
            chart3.attach(res, 0);

            if (g_modeIndex != 7)
            {
                // STRING: EZ2AC_UNPACKED 0x0048ecb8
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\7k_effect_point.str", &res);
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x0048ec9c
                ((KGraphics *)g_sub1b2f054)->loadStr("TT\\7k_effect_point_c.str", &res);
            }
            chart4.attach(res, 0);
        }

        if (g_modeIndex == 1)
        {
            chart0.f214 = 0.0f;
            chart0.f218 = 55.0f;
            chart4.f214 = 0.0f;
            chart4.f218 = 55.0f;
        }

        if (g_modeIndex == 4 || g_modeIndex == 8)
        {
            chart0.f214 = -239.0f;
            chart0.f218 = 0.0f;
        }

        popDirectory();
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048ec28
        changeDir("System\\CV2Mix\\InGameEffector", 1);

        // STRING: EZ2AC_UNPACKED 0x0048ec1c
        ((KGraphics *)g_sub1b2f054)->loadStr("icon.str", &res);
        c = &chart1;
        c->attach(res, 1);
        oy = (float)m_originY;
        ox = (float)m_originX;
        c->f214 = ox;
        c->f218 = oy;

        // STRING: EZ2AC_UNPACKED 0x0048ec0c
        ((KGraphics *)g_sub1b2f054)->loadStr("Spaceicon.str", &res);
        c = &chart2;
        c->attach(res, 1);
        oy = (float)m_originY;
        ox = (float)m_originX;
        c->f214 = ox;
        c->f218 = oy;

        // STRING: EZ2AC_UNPACKED 0x0048ebfc
        ((KGraphics *)g_sub1b2f054)->loadStr("14andromix.str", &res);
        c = &chart5;
        c->attach(res, 1);
        oy = (float)m_originY;
        ox = (float)m_originX;
        c->f214 = ox;
        c->f218 = oy;

        popDirectory();

        if (g_mixStyle == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0048ebc4
            ((KGraphics *)g_sub1b2f054)->loadStr("system\\CV2Mix\\streetmix1st\\Effector\\1st_effector.str", &res);
        }
        chart8.attach(res, 1);
    }

    g_1b5f158 = 1;
    m_f1e0 = 0;
    m_speedIndex = g_1b2e914;
    m_scrollBpm = 0;

    if (g_modeIndex == 5 || g_modeIndex == 9)
    {
        chart1.setCell(3, 0, (int)m_bassOff);
        chart1.setCell(5, 0, (int)m_trebleOff);
    }
    else
    {
        chart1.setCell(3, 0, (int)m_bassOff);
        chart1.setCell(5, 0, (int)m_trebleOff);

        if (g_1b2eb6c != 0)
        {
            chart1.setCell(7, 0, (int)m_speed[m_speedIndex]);
        }
    }

    // STRING: EZ2AC_UNPACKED 0x0048eba4
    m_gaugeOver = (void *)g_sub1b2eae4->createSample("System\\GaugeOver\\GaugeOver.wav", 0, 0);

    if (g_1b2eb6c == 1)
    {
        if (g_mixStyle != 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0048eb7c
            m_gaugeOverMix = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\GaugeOver\\GaugeOver.wav", 0, 0);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0048eb48
            m_gaugeOverMix = (void *)g_sub1b2eae4->createSample("System\\CV2Mix\\Streetmix1st\\GaugeOver\\GaugeOver.wav", 0, 0);
        }
    }
}

extern int countListA();                       /* 0x418310 (src/nodelists.cpp) */
extern struct ListNode *findListA(int key);    /* 0x418e30 (src/nodelists.cpp) */

/* The audience-rate read-out. The panel exists in two skins - Point for a
   single-player game and Pointdp for a versus one - and everything below
   picks between them on the same test, `countListA() == 1`.

   The rates come in as floats in the 0..1 range and are stored as whole
   percents; the one-up case reads its second figure from whichever side is
   actually playing. */
// FUNCTION: EZ2AC_UNPACKED 0x0041aa30
AudienceRatePanel::AudienceRatePanel()
{
    Obj402860 *c = &chart;
    Obj402860Head *res;
    char name[260];
    char path[260];
    int i;
    int n;

    f28 = 0.0f;
    if (countListA() == 1)
    {
        f24 = (int)(g_rateTarget * 100.0f);
        if (findListA(0) != 0)
        {
            f1c = (int)(g_rate1p * 100.0f);
        }
        else
        {
            f1c = (int)(g_rate2p * 100.0f);
        }
    }
    else if (countListA() == 2)
    {
        f24 = (int)(g_rateTarget * 100.0f);
        f1c = (int)(g_rate1p * 100.0f);
        f20 = (int)(g_rate2p * 100.0f);
    }
    for (i = 0; i < 10; i++)
    {
        if (countListA() == 1)
        {
            // STRING: EZ2AC_UNPACKED 0x0048dfe0
            sprintf(name, "system\\AudiencePanel\\Point\\Point_cw%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(name, &cw[i], 0);
            // STRING: EZ2AC_UNPACKED 0x0048dfb0
            sprintf(name, "system\\AudiencePanel\\Point\\Point_next%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(name, &next[i], 0);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0048df80
            sprintf(name, "system\\AudiencePanel\\Pointdp\\Pointdp_cw%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(name, &cw[i], 0);
            // STRING: EZ2AC_UNPACKED 0x0048df4c
            sprintf(name, "system\\AudiencePanel\\Pointdp\\Pointdp_next%02d.bmp", i);
            ((KGraphics *)g_sub1b2f054)->loadTexture(name, &next[i], 0);
        }
    }
    if (countListA() == 1)
    {
        if (findListA(0) != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0048df24
            sprintf(path, "system\\AudiencePanel\\Point\\point_1p.str");
        }
        else if (findListA(1) != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0048defc
            sprintf(path, "system\\AudiencePanel\\Point\\point_2p.str");
        }
        // STRING: EZ2AC_UNPACKED 0x0048decc
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\AudiencePanel\\Point\\point_nextpoint.bmp", &nextPoint, 0);
        // STRING: EZ2AC_UNPACKED 0x0048de9c
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\AudiencePanel\\Point\\point_cwpoint.bmp", &cwPoint, 0);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0048de70
        sprintf(path, "system\\AudiencePanel\\Pointdp\\point_dp.str");
        // STRING: EZ2AC_UNPACKED 0x0048de3c
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\AudiencePanel\\pointdp\\pointdp_nextpoint.bmp", &nextPoint, 0);
        // STRING: EZ2AC_UNPACKED 0x0048de08
        ((KGraphics *)g_sub1b2f054)->loadTexture("system\\AudiencePanel\\pointdp\\pointdp_cwpoint.bmp", &cwPoint, 0);
    }
    ((KGraphics *)g_sub1b2f054)->loadStr(path, &res);
    c->attach(res, 1);
    /* The count is read BEFORE the f214 store: cl cannot hoist a call above a
       store to memory it might touch, so the original having the call first
       means the source did too. */
    n = countListA();
    c->f214 = 0.0f;
    if (n == 1)
    {
        c->f218 = 292.0f;
    }
    else
    {
        c->f218 = 252.0f;
    }
}

/* One frame of the audience-rate panel: refresh its cell block, then pick one
   of three draw variants. With exactly one player in the round it draws the
   full or the solo layout depending on whether that player's list entry
   exists; otherwise the idle one. All three are TAIL JUMPS, so they must stay
   in another TU or cl inlines them and the `jmp`s disappear. */
// FUNCTION: EZ2AC_UNPACKED 0x0041a910
void AudienceRatePanel::update()
{
    chart.m403940();

    if (countListA() == 1)
    {
        if (findListA(0) != 0)
        {
            drawFull();
        }
        else
        {
            drawSolo();
        }
    }
    else
    {
        drawIdle();
    }
}

/* BackGround's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0041bb30
BackGround *createBackGround()
{
    g_objectRecord = &g_info_4a8f1c;
    return new BackGround;
}

/* Effector's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0041d060
Effector *createEffector()
{
    g_objectRecord = &g_info_4a8f94;
    return new Effector;
}

/* Effector6thStyle's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0041f860
Effector6thStyle *createEffector6thStyle()
{
    g_objectRecord = &g_info_4a9004;
    return new Effector6thStyle;
}

extern int countListA();                   /* 0x418310 (src/nodelists.cpp) */
extern struct ListNode *findListA(int key);/* 0x418e30 (src/nodelists.cpp) */

/* AudienceRatePanel::update2 @0x41a950 - latch the rates and step the blink.

   With ONE player present the panel shows the target and that player's own
   rate, picked by which side is in list A: side 0's rate if it is there,
   side 1's otherwise. With TWO it shows the target and both rates, one in
   each slot. Any other count latches nothing at all and only the blink
   moves - and the count is asked TWICE, once per arm, rather than read into
   a local.

   The rates are stored as percentages, `* 100` truncated, which is what the
   draw's four-digit hundredths rendering expects.

   The tint is a branchless ternary between two constants, which is why it
   comes out as neg/sbb/and/add rather than a branch: green 0x41 on phase 0
   and 0xff on phases 1 and 2, dropped into a colour whose other three bytes
   never change. */
// FUNCTION: EZ2AC_UNPACKED 0x0041a950
void AudienceRatePanel::update2(int, int)
{
    if (countListA() == 1)
    {
        f24 = (int)(g_rateTarget * 100.0f);

        if (findListA(0) != 0)
            f1c = (int)(g_rate1p * 100.0f);
        else
            f1c = (int)(g_rate2p * 100.0f);
    }
    else if (countListA() == 2)
    {
        f24 = (int)(g_rateTarget * 100.0f);
        f1c = (int)(g_rate1p * 100.0f);
        f20 = (int)(g_rate2p * 100.0f);
    }

    f18 += 1;
    if (f18 >= 3)
        f18 = 0;

    f14 = ((f18 != 0 ? 0xff : 0x41) << 8) | 0xff0e00a4;
}

extern float g_drawOriginX;   /* 0x37da9d4 (src/effectordraw.cpp) */
extern int   g_demoPlay;      /* 0x1b2ef78, above */
extern int   g_1b2eadc;       /* 0x1b2eadc (src/showcredit.cpp) - the game */

/* src/effectordraw.cpp - the readout draw, reached with `this` as the effector.
   Forward-declared only: the type name is what the mangling needs, and it must
   NOT be defined here or cl would inline the call. */
struct EffectorReadout;
struct EffectorDraw { void drawReadout(struct EffectorReadout *e); }; /* 0x41cb90 */

/* A mode-0 record, which is the SAME 0x290 bytes as a readout read another
   way: a level, an index, and a run of 16-byte bars. Only the bar the index
   picks is drawn. */
struct EffectorBar   { int f0; float x; float y; struct KTexture *tex; };
struct EffectorGauge { int f0; int level; int index; int fc; EffectorBar bars[1]; };

/* Effector::update @0x41cda0 - draw every live effector, then the demo-play
   overlay.

   Game 1 draws nothing at all. Otherwise render state 7 is cleared, the blend
   set, and each live record drawn according to its MODE word - which sits
   four bytes ahead of the readout, so the loop walks the readout and reads
   the mode at [-1], the way the original anchors it. Mode 0 is a bar gauge:
   the record is the same bytes seen as a level, an index and a run of
   16-byte bars, and the indexed bar is drawn only once the level reaches 2.
   Mode 1 hands the record to the readout draw. Anything else draws nothing.

   The bar pointer is computed BEFORE the level guard, which is where the
   original's `lea` sits.

   The demo-play overlay is last and is its own gate: mask first in an
   additive blend, then the image, both at the same fixed 229/216/184/45. */
// FUNCTION: EZ2AC_UNPACKED 0x0041cda0
void Effector::update()
{
    if (g_1b2eadc == 1)
        return;

    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);

    if (f14 != 0)
    {
        int i = 0;

        if (i < f18)
        {
            int *r = (int *)recs[0].readout;

            do
            {
                if (r[-1] == 0)
                {
                    EffectorGauge *g = (EffectorGauge *)r;
                    EffectorBar   *b = &g->bars[g->index];

                    if (g->level >= 2)
                    {
                        ((KGraphics *)g_sub1b2f054)->setBlend(1, 3);
                        ((KGraphics *)g_sub1b2f054)->drawTex(b->tex,
                                                             b->x + g_drawOriginX,
                                                             b->y, 0.0f, 0.0f, -1);
                    }
                }
                else if (r[-1] == 1)
                {
                    ((EffectorDraw *)this)->drawReadout((EffectorReadout *)r);
                }

                i++;
                r = (int *)((char *)r + 0x294);
            }
            while (i < f18);
        }
    }

    if (g_demoPlay != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_demoPlayMask,
                                             229.0f, 216.0f, 184.0f, 45.0f, -1);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_demoPlay,
                                             229.0f, 216.0f, 184.0f, 45.0f, -1);
    }
}
