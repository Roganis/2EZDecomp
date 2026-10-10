/* ModeSelectDirector's constructor (see include/modeselect.h for the layout).

   3,449 bytes and only thirteen branches: eighteen member constructors, a long
   run of field and global resets, ten sound samples, the screen's own texture
   set and eight small loops over three string tables in .data.

   The nine five-element global arrays below are reset in INTERLEAVED order -
   element 0 of all nine, then element 1 of all nine - which is a five-iteration
   loop with nine statements, fully unrolled by cl. They are written out here
   because the unroll is what the original has and a loop over nine separate
   global arrays does not reproduce it. */

#include <stdio.h>
#include "modeselect.h"
#include "ez2acapp.h"   /* KGraphics, GVec3, g_sub1b2f054, EzSubsystem */
#include "bigbase.h"
#include "loader.h"     /* KTexture */

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */
extern void changeDir(char *path, int savePrev);   /* 0x412dd0 */
extern void popDirectory();                        /* 0x412e60 */
extern int  countListA();                          /* 0x418310 */
extern "C" unsigned long __stdcall timeGetTime(void);

/* Already matched under placeholder class names - see src/modeselect.cpp. */
struct Obj40BBB0 { void loadIdentity(); };
struct Obj40BB00 { void doSub(); };
/* Obj40B2E0's own +0x428/+0x42c and +0x43c setters, matched in
   src/accessors.cpp and src/misc.cpp under placeholder class names. */
struct Obj40B9C0 { void setXY(float x, float y); };
struct Obj40B9E0 { void setField43c(int v); };

/* The three 0x104-byte string tables in .data the mode-text loops walk. Only
   their addresses, strides and counts are evidenced here; the contents are
   placeholder storage, the same convention as src/globals.cpp. */
/* ONE entity, not three arrays. Each of these loops ends on a `cmp` against
   its one-past-the-end address, and that only compares equal when it resolves
   the same way on both sides - as three separate globals our linker put other
   things at the boundaries and every bound mismatched. Spanning the whole
   region with a single array makes all four bounds interior references to it,
   which is identical by construction. */
// GLOBAL: EZ2AC_UNPACKED 0x004ac788
char g_modeTables[0x1660];

/* Offsets of the three 260-byte-per-entry tables inside it. */
static const int kModeNames  = 0x0000;   /* 12 entries, 0x4ac788 */
static const int kStyleNames = 0x0d38;   /*  4 entries, 0x4ad4c0 */
static const int kGlowNames  = 0x1148;   /*  5 entries, 0x4ad8d0 */

/* The same nine parallel five-element tables resetGlobals430b50 @0x430b50
   clears (src/gamestate.cpp), in this function's own order. */
extern int g_1b2f058[5], g_1b2ebe0[5], g_1b5f15c[5], g_1b2f120[5], g_1b2e650[5];
extern int g_1b2ea38[5], g_1b2e9fc[5], g_1b2e668[5], g_1b2ef44[5];

/* The rest of the globals the constructor resets. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e720
int g_1b2e720;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea20
int g_1b2ea20;
extern int g_1b2eb28;   /* src/gamestate.cpp */
extern int g_1b5f2ec;   /* src/gamestate.cpp */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef38
int g_1b2ef38;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea4c
int g_1b2ea4c;
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2e4
int g_1b5f2e4;
extern int g_1b2f024;   /* src/moderunner.cpp */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f084
int g_1b2f084;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea30
int g_1b2ea30;
// GLOBAL: EZ2AC_UNPACKED 0x01b2efa0
int g_1b2efa0;
extern int g_1b2eb44;   /* src/gamestate.cpp */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e584
int g_1b2e584;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7a8
int g_1b2e7a8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e588
int g_1b2e588;

/* Two runs in the 53 MB .data region. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd048
int g_37dd048[8];
extern int g_37dcf0c;   /* src/gamestate.cpp */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f300
int g_1b5f300[5];

/* Set from src/ez2acapp.cpp; read here only to pick 12 or 8. */
extern int g_onePlayerOnly;   /* 0x1696420 (src/settings.cpp) */

extern int g_d36cb0;   /* 0xd36cb0 (src/maingame.cpp) */

/* 0.6f at 0x48fd50 is an ordinary merged pool constant: InGameSongSelect::
   m425b40 multiplies by the same address. It was modeled here as a named
   `k06` until 2026-10-01; the literal gives this constructor the same code
   (two fewer diff lines, in fact) and lets m425b40 reach 100%. */

// FUNCTION: EZ2AC_UNPACKED 0x0044b8f0
ModeSelectDirector::ModeSelectDirector()
{
    Obj402860Head *res;
    char name[260];
    char logoName[260];
    Obj40C180 *g;
    int i;
    GVec3 eye;
    GVec3 at;
    GVec3 up;

    g_1b2e720 = 0;
    g_1b2ea20 = 0;
    f9510 = 6.0f;
    f9444 = 520.0f;
    /* fd0's store belongs 0x120 bytes further down, with f9604's - it is here
       because SOMETHING has to create the zero in xmm0 at this point. That is
       what forces 0.6 into xmm1 and lets the `xorps` sit ahead of the 0.6
       store, which is the original's shape; with fd0 in its natural place cl
       gives 0.6 xmm0 and the whole block comes out one register wrong (99.62%).
       The residual is this one store's position - reccmp calls the result a
       100% EFFECTIVE match: same instructions, same registers, two independent
       ones swapped. */
    fd0 = 0.0f;
    f9450 = 0.6f;
    f9628 = 0;
    f24 = 0;
    fdc = 0;
    fd8 = 0;
    fe8 = 0;
    f30[0] = 0;
    f30[1] = 0;
    f30[2] = 0;
    f30[3] = 0;
    f30[4] = 0;
    f30[5] = 0;
    f30[6] = 0;
    f30[7] = 0;
    f30[8] = 0;
    f9448 = 0xff;
    f944c = 0;
    fe0 = 0;
    f93e0 = 0;
    f95f4 = 0;
    f95f0 = 0;
    f9434 = 0;
    f9598 = 0;
    f9588[0] = 0.0f;
    f9588[1] = 0.0f;
    f9588[2] = 0.0f;
    f9588[3] = 0.0f;
    f93e4[4] = 0;
    f93e4[3] = 0;
    f93e4[2] = 0;
    f93e4[1] = 0;
    f93e4[0] = 0;
    f93f8[3] = 0;
    f93f8[2] = 0;
    f93f8[1] = 0;
    f93f8[0] = 0;
    f940c[3] = 0;
    f940c[2] = 0;
    f940c[1] = 0;
    f940c[0] = 0;
    fe4 = 0;
    fec = 0;
    f9420[4] = 512.0f;
    f9420[3] = 512.0f;
    f9420[2] = 512.0f;
    f9420[1] = 512.0f;
    f9420[0] = 512.0f;
    f9604 = 0;
    g_1b2eb28 = 0;
    fcc = 0;
    f9608 = 0.0f;
    f10 = 0;
    f960c = 0.02f;
    for (i = 0; i < 5; i++)
    {
        g_1b2f058[i] = 0;
        g_1b2ebe0[i] = 0;
        g_1b5f15c[i] = 0;
        g_1b2f120[i] = 0;
        g_1b2e650[i] = 0;
        g_1b2ea38[i] = 0;
        g_1b2e9fc[i] = 0;
        g_1b2e668[i] = 0;
        g_1b2ef44[i] = 0;
    }
    f9610 = 0;
    g_37dd048[4] = 0;
    g_37dd048[5] = 0;
    g_37dd048[6] = 0;
    g_37dd048[7] = 0;
    g_37dd048[0] = 0;
    g_37dd048[2] = 0;
    g_37dd048[3] = 0;
    f9614 = 0;
    g_1b5f2ec = 0;
    g_37dd048[1] = 0;
    g_1b2ef38 = 0;
    g_1b2ea4c = 0;
    g_1b5f300[0] = 7;
    g_1b5f300[1] = 7;
    g_1b5f300[2] = 7;
    g_1b5f300[3] = 7;
    g_1b5f300[4] = 7;
    // STRING: EZ2AC_UNPACKED 0x00494528
    Log("ModeSelectDirector::Create");
    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    ((KGraphics *)g_sub1b2f054)->setCamera(&eye, &at, &up);
    ((KGraphics *)g_sub1b2f054)->setProjection(1.0f, 10000.0f, 0.785398185f);
    ((KGraphics *)g_sub1b2f054)->setMaterial(-1);
    // STRING: EZ2AC_UNPACKED 0x00494508
    sndSelect = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\select.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x004944e8
    sndDecide = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\decide.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00493524
    sndTimeCount = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\timecount.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x004944c4
    sndModeTap = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\modetap_fx.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x004944a4
    sprintf(name, "System\\ModeSelect\\NoticeTap.wav");
    sndNoticeTap = (void *)g_sub1b2eae4->createSample(name, 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049447c
    sndBgm = (void *)g_sub1b2eae4->createSample("System\\ModeSelect\\ModeSelect_bgm.wav", 1, 0);
    sndModeIntro = (void *)g_sub1b2eae4->createSample("System\\ModeSelect\\ModeIntro.wav", 1, 0);
    // STRING: EZ2AC_UNPACKED 0x00493564
    sndHidden = (void *)g_sub1b2eae4->createSample("system\\ModeSelect\\hiddencmd.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x0049443c
    sndStart = (void *)g_sub1b2eae4->createSample("system\\CV2Mix\\title\\start.wav", 0, 0);
    // STRING: EZ2AC_UNPACKED 0x00494414
    m_83b8.load((KGraphics *)g_sub1b2f054, "System\\ModeSelect\\BG\\ModeSelect_BG.spv");
    ((Obj40B9E0 *)&m_83b8)->setField43c(1);
    m_83b8.play(timeGetTime());
    ((Obj40B9C0 *)&m_83b8)->setXY(0.0f, 0.0f);
    f9624 = 0;
    f9620 = 0;
    f20 = 0x14;
    for (i = 0; i < 12; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004943ec
        sprintf(logoName, "system\\modeselect\\vf_inout\\Logo_%s.bmp", &g_modeTables[kModeNames + i * 260]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(logoName, &logo[i], 0);
    }
    f18 = 0;
    f95ec = 0;
    f14 = (g_onePlayerOnly == 0) ? 12 : 8;
    g_1b5f2e4 = 0;
    g_1b2f024 = 0;
    g_1b2f084 = 0;
    g_1b2ea30 = 0;
    g_1b2efa0 = 0;
    g_1b2eb44 = 0;
    g_1b2e584 = 0;
    g_1b2e7a8 = 0;
    g_1b2e588 = 0;
    g_37dcf0c = 0;
    // STRING: EZ2AC_UNPACKED 0x00492f30
    changeDir("System\\ModeSelect\\", 1);
    if (countListA() == 2)
    {
        f18 = 0;
    }
    /* `i` is dead - the printed number and the walking pointer are both their
       own induction variables - so cl needs a separate trip counter, and with
       registers this tight it spills that counter to the frame. */
    for (i = 0; i < 12; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00492f10
        sprintf(name, "VF_ModeInOut\\modetext%02d.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &modeText[i], 0);
        // STRING: EZ2AC_UNPACKED 0x00492ef0
        sprintf(name, "VF_ModeInOut\\modetext%02d_m.bmp", i + 1);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &modeTextM[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004943d0
    sprintf(name, "VF_ModeInout\\Mode_Intro.str");
    ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
    m_1a40.attach(res, 0);
    // STRING: EZ2AC_UNPACKED 0x004943b8
    ((KGraphics *)g_sub1b2f054)->loadStr("ModeSelect_Layout.str", &res);
    m_1c5c.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x004943a8
    ((KGraphics *)g_sub1b2f054)->loadStr("bg\\Scanline.str", &res);
    m_2f58.attach(res, 1);
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x0049437c
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\PatchNote\\TT_ModeSelect_Notice.bmp", &notice, 0);
    // STRING: EZ2AC_UNPACKED 0x00494350
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\PatchNote\\TT_ModeSelect_Notice_m.bmp", &noticeM, 0);
    // STRING: EZ2AC_UNPACKED 0x00494330
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\0_black.bmp", &black, 0);
    // STRING: EZ2AC_UNPACKED 0x004942fc
    ((KGraphics *)g_sub1b2f054)->loadTexture("System\\PatchNote\\TT_ModeSelect_Notice_shadow.bmp", &noticeShadow, 0);
    // STRING: EZ2AC_UNPACKED 0x00492da8
    changeDir("System\\common\\", 1);
    // STRING: EZ2AC_UNPACKED 0x00492d78
    ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERInsertCoin.str", &res);
    m_2094.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492d60
    ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERInsertCoin.str", &res);
    m_22b0.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492d48
    ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERPressStart.str", &res);
    m_24cc.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492d30
    ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERPressStart.str", &res);
    m_26e8.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492d20
    ((KGraphics *)g_sub1b2f054)->loadStr("1PLAYERWait.str", &res);
    m_2904.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x00492d10
    ((KGraphics *)g_sub1b2f054)->loadStr("2PLAYERWait.str", &res);
    m_2b20.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x004942f0
    ((KGraphics *)g_sub1b2f054)->loadStr("1Player.str", &res);
    m_3174.attach(res, 1);
    // STRING: EZ2AC_UNPACKED 0x004942e4
    ((KGraphics *)g_sub1b2f054)->loadStr("2Player.str", &res);
    m_3390.attach(res, 1);
    popDirectory();
    changeDir("System\\ModeSelect\\", 1);
    popDirectory();
    // STRING: EZ2AC_UNPACKED 0x004942c4
    ((KGraphics *)g_sub1b2f054)->loadStr("system\\cv2mix\\title\\intro.str", &res);
    m_2d3c.attach(res, 0);
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\fadeblack.bmp", &fadeBlack, 0);
    // STRING: EZ2AC_UNPACKED 0x004942a0
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\bg\\bg_grad.bmp", &bgGrad, 0);
    for (i = 0; i < 4; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00494278
        sprintf(name, "system\\ModeSelect\\ModeText\\stco_%s.bmp", &g_modeTables[kStyleNames + i * 260]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &stco[i], 0);
    }
    for (i = 0; i < 4; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x0049424c
        sprintf(name, "system\\ModeSelect\\ModeText\\course_%s.bmp", &g_modeTables[kStyleNames + i * 260]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &course[i], 0);
    }
    for (i = 0; i < 3; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00494220
        sprintf(name, "system\\ModeSelect\\ModeText\\basicmode_%s.bmp", &g_modeTables[kModeNames + i * 260]);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &basicMode[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x004941f0
    sprintf(name, "system\\ModeSelect\\ModeText\\casual_EZ2Catch.bmp");
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &casualCatch, 0);
    // STRING: EZ2AC_UNPACKED 0x004941bc
    sprintf(name, "system\\ModeSelect\\ModeText\\casual_ScratchMix.bmp");
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &casualScratch, 0);
    // STRING: EZ2AC_UNPACKED 0x00494188
    sprintf(name, "system\\ModeSelect\\ModeText\\speicalmode_cv2mix.bmp");
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &specialCv2, 0);
    // STRING: EZ2AC_UNPACKED 0x00494150
    sprintf(name, "system\\ModeSelect\\ModeText\\speicalmode_legacyradio.bmp");
    ((KGraphics *)g_sub1b2f054)->loadTexture(name, &specialLegacy, 0);
    // STRING: EZ2AC_UNPACKED 0x00494128
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Modeselect\\t_cate_special.bmp", &cateSpecial, 0);
    // STRING: EZ2AC_UNPACKED 0x00494100
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\Modeselect\\t_cate_special_m.bmp", &cateSpecialM, 0);
    for (i = 0; i < 10; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004940d8
        sprintf(name, "system\\ModeSelect\\TimeFont\\Time_%d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &timeFont[i], 0);
    }
    for (i = 0; i < 5; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x004940a4
        sprintf(name, "system\\ModeSelect\\CategoryGlow\\category_glow_%s.str", &g_modeTables[kGlowNames + i * 260]);
        ((KGraphics *)g_sub1b2f054)->loadStr(name, &res);
        m_890c[i].attach(res, 0);
    }
    for (i = 0; i < 2; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00494078
        sprintf(name, "system\\modeselect\\diffstyleselect_%02d.bmp", i);
        ((KGraphics *)g_sub1b2f054)->loadTexture(name, &diffStyle[i], 0);
    }
    // STRING: EZ2AC_UNPACKED 0x0049404c
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\modeselect\\diffstyleselect_cur.bmp", &diffStyleCur, 0);
    f95f8 = 0;
    fd4 = 0;
    f95fc = 0;
    f9600 = 0;
    f95dc = 0;
    f95e0 = 0;
    f95d8 = 0x28;
    g = g_bigInstance;
    g->accum[0] = 0;
    g->accum[1] = 0;
    g->limit = 0x14;
    f95e4 = 0;
    f95e8 = 10;
}

/* ModeSelectDirector::tickCoinLamp @0x44b450 - one frame of the coin lamp.

   TitleDirector::tickCoinLamp @0x44e880 (src/coinlamp.cpp) with two things
   taken out: this screen has no attract song, so the lamp does not stop and
   restart one, and it has no lamp period to rescale, so there is no
   unconditional slot 8 call at the end. Everything else is the same function
   - slot 17 ticked every frame, the lamp fired on slot 5 when the countdown
   is idle and slot 16's low bit is clear, and slots 12 and 4 put it back on
   the frame the countdown reaches zero.

   Same `add eax,-1` shape too: the countdown is read into a local and the
   decrement written as an EXPRESSION into the member, not a reassignment of
   the local. */

/* The lamp device, as include/ezscreen.h models it - only the five slots
   this reaches, redeclared so that header need not be pulled in. */
struct LampDev
{
    virtual void d0(); virtual void d1(); virtual void d2(); virtual void d3();
    virtual void d4(int a);        /* slot 4  -> +0x10 */
    virtual void d5();             /* slot 5  -> +0x14 */
    virtual void d6(); virtual void d7();
    virtual void d8(int a);
    virtual void d9(); virtual void d10(); virtual void d11();
    virtual void d12(int a);       /* slot 12 -> +0x30 */
    virtual void d13(); virtual void d14(); virtual void d15();
    virtual int  d16();            /* slot 16 -> +0x40 */
    virtual void d17();            /* slot 17 -> +0x44 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0044b450
void ModeSelectDirector::tickCoinLamp(void *device, int frames)
{
    LampDev *dev = (LampDev *)device;
    int      n;

    dev->d17();

    if (f9624 == 0 && (dev->d16() & 1) == 0)
    {
        dev->d5();
        f9624 = frames;
    }

    n = f9624;
    if (n != 0)
    {
        f9624 = n - 1;
        if (f9624 == 0)
        {
            dev->d12(0);
            dev->d4(0);
        }
    }
}

#include <string.h>   /* memcmp */

extern int g_1b2eb70;          /* 0x1b2eb70 (src/resultctor.cpp) */
extern int g_hiddenCodes[3][20];   /* 0x4adde8 (src/globals.cpp) */

/* ModeSelectDirector::m44c690 @0x44c690 - append one key to a stream and see
   whether the stream has become a hidden command.

   clearCommand @0x44c7f0 feeds every held key to BOTH streams, so the two
   are independent attempts at the same code. The key is appended, and the
   stream is then compared against each of the three codes for exactly as
   many entries as have been entered - a prefix test, which is what the
   memcmp over `cmdLen * 4` bytes is. Any prefix match sets the keep flag; a
   prefix match that has reached the code's -1 terminator, on a code that has
   not fired yet, is the code itself.

   Codes 0 and 1 both play the hidden sound, and code 0 additionally raises
   g_1b2eb70 - the flag the song-select screen reads to pick its EZ2CATCH
   rank names. Code 2 is matched and counted but does nothing here.

   If NO code kept its prefix the stream is restarted from this key alone
   rather than cleared, so a false start does not cost the key that begins
   the next attempt.

   The memcmp takes the CODE first and the stream second - cl's expansion
   puts the first argument in esi and the second in edi, and the original's
   `repz cmpsb` has the stream in edi. Written the other way round it is
   91.55%, and that one swap is worth six points.

   97.18%: what is left is the polarity of the case-1 test, which the
   original writes `jne default` with the fall-through jumping to the body
   and cl writes `je body` with the fall-through jumping to default. Same
   four bytes either way. An explicit leading `default: break;` changes
   nothing, and writing the two arms as an if-chain with the sound
   duplicated is much worse (85.71%). */
/* 97.18%, and the residue is two instructions that say the same thing: the
   hidden-code switch's second test is `sub ecx,edx; jne <skip>; jmp <case 1>`
   in the original and `sub ecx,edx; je <case 1>; jmp <skip>` here. Identical
   block layout, identical count - only which of the two branches carries the
   condition. Five spellings of that switch were swept and the plain one is
   the best of them: an explicit `default: break;` changes nothing, two
   separate `if`s and an `i != 2` guard are 52.1%, and duplicating the sound
   block into both arms is 85.9%. */
// FUNCTION: EZ2AC_UNPACKED 0x0044c690
void ModeSelectDirector::m44c690(int slot, int key)
{
    int *p;
    int  keep;
    int  i;

    cmdBuf[slot][cmdLen[slot]] = key;
    cmdLen[slot]++;

    keep = 0;
    p = cmdBuf[slot];

    for (i = 0; i < 3; i++)
    {
        if (memcmp(g_hiddenCodes[i], p, cmdLen[slot] * 4) == 0)
        {
            keep = 1;

            if (g_hiddenCodes[i][cmdLen[slot]] == -1 && cmdDone[i] == 0)
            {
                /* Two full arms, not a fall-through: cl cross-jumps the
                   shared sound calls, and that is what gives the original's
                   `jne <loop tail>; jmp <case 1>` dispatch. */
                switch (i)
                {
                case 0:
                    g_1b2eb70 = 1;
                    ((EzSoundObject *)sndHidden)->q5();
                    ((EzSoundObject *)sndHidden)->q4(0);
                    break;
                case 1:
                    ((EzSoundObject *)sndHidden)->q5();
                    ((EzSoundObject *)sndHidden)->q4(0);
                    break;
                }
            }
        }
    }

    if (keep == 0)
    {
        cmdLen[slot] = 1;
        p[0] = key;
    }
}
