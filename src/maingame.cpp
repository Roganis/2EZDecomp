/* The three main-game directors' teardown (see include/ezscreen.h for the
   layouts and the family table).

   All three do the same work in slightly different order: turn the two mixer
   controls down, run two of their own methods, release the per-frame consumer
   and null it, put four to six pieces of cabinet state back, restore the
   device's render state 0x16 and clear the global at 0xd36cb0, then let the
   pump go. EZ2DJ's does the graphics pair FIRST and has two extra cabinet
   setters; the other two do it last. */

#include <stdio.h>
#include <string.h>
#include "ezscreen.h"
#include "kparser.h"
#include "directors.h"   /* SoundBank */
#include "ez2acapp.h"   /* KGraphics, EzStream, g_sub1b2f054 */
#include "bigbase.h"

struct MixerControl { void setValue(unsigned long value); };  /* 0x412a10 */

/* The mixer singleton's control factory (src/mixer.cpp @0x412af0). */
struct KSNDMixer { void *addControl(int dst, int type); };

/* Three free functions the Catch constructor calls, all still unmatched. */
struct ListNode *findListB(int key);   /* 0x418eb0 (src/nodelists.cpp) */
ScrFader *m450570();     /* 0x450570 (STUB) */
BattleMode *createBattleMode();   /* 0x41c9a0 (src/misc.cpp) */

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classEZ2DJMainGameDirector;  /* 0x4a9030 */
extern EzObjectInfo  g_info_4a9040;               /* 0x4a9040 */
extern KRuntimeClass classGFMainGameDirector;     /* 0x4ae68c */
extern EzObjectInfo  g_info_4ae69c;               /* 0x4ae69c */
extern KRuntimeClass classCatchMainGameDirector;  /* 0x4aeb78 */
extern EzObjectInfo  g_info_4aeb88;               /* 0x4aeb88 */

/* The frame counter every director clears on the way out. */
// GLOBAL: EZ2AC_UNPACKED 0x00d36cb0
int g_d36cb0;

extern int  g_elemCount;            /* 0x1b2e718 (src/loops.cpp) */
extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */

/* EZ2DJMainGameDirector's own trim buffer - each class owns one, which is why the
   original's ICF left these identical bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037da8c8
static char g_ez2djVideoName[260];

/* The same trim as BackGround::videoName @0x41add0: copy the current text
   slot and drop a trailing "-xx" / "-xxx" / "-xxxx". */
// FUNCTION: EZ2AC_UNPACKED 0x004208f0
char *EZ2DJMainGameDirector::m4208f0()
{
    strcpy(g_ez2djVideoName, g_textSlots[g_elemCount]);

    if (g_ez2djVideoName[strlen(g_ez2djVideoName) - 3] == '-')
        g_ez2djVideoName[strlen(g_ez2djVideoName) - 3] = 0;
    else if (g_ez2djVideoName[strlen(g_ez2djVideoName) - 4] == '-')
        g_ez2djVideoName[strlen(g_ez2djVideoName) - 4] = 0;
    else if (g_ez2djVideoName[strlen(g_ez2djVideoName) - 5] == '-')
        g_ez2djVideoName[strlen(g_ez2djVideoName) - 5] = 0;

    return g_ez2djVideoName;
}

extern char g_gfMainVideoName[260];   /* 0x37dd388 (src/globals.cpp) */

/* The same trim as BackGround::videoName @0x41add0: copy the current text
   slot and drop a trailing "-xx" / "-xxx" / "-xxxx". */
// FUNCTION: EZ2AC_UNPACKED 0x0045da00
char *GFMainGameDirector::m45da00()
{
    strcpy(g_gfMainVideoName, g_textSlots[g_elemCount]);

    if (g_gfMainVideoName[strlen(g_gfMainVideoName) - 3] == '-')
        g_gfMainVideoName[strlen(g_gfMainVideoName) - 3] = 0;
    else if (g_gfMainVideoName[strlen(g_gfMainVideoName) - 4] == '-')
        g_gfMainVideoName[strlen(g_gfMainVideoName) - 4] = 0;
    else if (g_gfMainVideoName[strlen(g_gfMainVideoName) - 5] == '-')
        g_gfMainVideoName[strlen(g_gfMainVideoName) - 5] = 0;

    return g_gfMainVideoName;
}

/* CatchMainGameDirector's own trim buffer - each class owns one, which is why the
   original's ICF left these identical bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037df438
static char g_catchMainVideoName[260];

/* The same trim as BackGround::videoName @0x41add0: copy the current text
   slot and drop a trailing "-xx" / "-xxx" / "-xxxx". */
// FUNCTION: EZ2AC_UNPACKED 0x0046c360
char *CatchMainGameDirector::m46c360()
{
    strcpy(g_catchMainVideoName, g_textSlots[g_elemCount]);

    if (g_catchMainVideoName[strlen(g_catchMainVideoName) - 3] == '-')
        g_catchMainVideoName[strlen(g_catchMainVideoName) - 3] = 0;
    else if (g_catchMainVideoName[strlen(g_catchMainVideoName) - 4] == '-')
        g_catchMainVideoName[strlen(g_catchMainVideoName) - 4] = 0;
    else if (g_catchMainVideoName[strlen(g_catchMainVideoName) - 5] == '-')
        g_catchMainVideoName[strlen(g_catchMainVideoName) - 5] = 0;

    return g_catchMainVideoName;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041f970
KRuntimeClass *EZ2DJMainGameDirector::getRuntimeClass()
{
    return &classEZ2DJMainGameDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0041f980
DrawGroup *EZ2DJMainGameDirector::getObjectInfo()
{
    return &g_info_4a9040;
}

// FUNCTION: EZ2AC_UNPACKED 0x00422df0
EZ2DJMainGameDirector::~EZ2DJMainGameDirector()
{
    if (mixerB != 0)
    {
        mixerB->setValue(0x5938);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(0xaaaa);
    }
    ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 3);
    g_d36cb0 = 0;
    m420ec0();
    m4209d0();
    if (timed != 0)
    {
        ((EzStream *)timed)->release(1);
    }
    timed = 0;
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->m418b90(0);
    g_bigInstance->routeOutputs(0);
    g_bigInstance->setFlags(0);
}

// FUNCTION: EZ2AC_UNPACKED 0x004246b0
// EZ2DJMainGameDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int EZ2DJMainGameDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0048f8b0 EzObject
struct EZ2DJMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0048f8ac EZ2DJSink1
struct EZ2DJMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0048f8a8 EZ2DJSink2
struct EZ2DJMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0048f8a4 EZ2DJSink3
struct EZ2DJMainGameDirector;

// FUNCTION: EZ2AC_UNPACKED 0x0045d060
KRuntimeClass *GFMainGameDirector::getRuntimeClass()
{
    return &classGFMainGameDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045d070
DrawGroup *GFMainGameDirector::getObjectInfo()
{
    return &g_info_4ae69c;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045f240
GFMainGameDirector::~GFMainGameDirector()
{
    if (mixerB != 0)
    {
        mixerB->setValue(0x5938);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(0xaaaa);
    }
    m45def0();
    m45dae0();
    if (timed != 0)
    {
        ((EzStream *)timed)->release(1);
    }
    timed = 0;
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->setFlags(0);
    ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 3);
    g_d36cb0 = 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00460720
// GFMainGameDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int GFMainGameDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049a16c EzObject
struct GFMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049a168 GFSink1
struct GFMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049a164 GFSink2
struct GFMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049a160 GFSink3
struct GFMainGameDirector;

// FUNCTION: EZ2AC_UNPACKED 0x0046bf10
KRuntimeClass *CatchMainGameDirector::getRuntimeClass()
{
    return &classCatchMainGameDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046bf20
DrawGroup *CatchMainGameDirector::getObjectInfo()
{
    return &g_info_4aeb88;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046d250
CatchMainGameDirector::~CatchMainGameDirector()
{
    if (mixerB != 0)
    {
        mixerB->setValue(0x5938);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(0xaaaa);
    }
    m46c7e0();
    m46c440();
    if (timed != 0)
    {
        ((EzStream *)timed)->release(1);
    }
    timed = 0;
    g_bigInstance->m4189e0(0);
    g_bigInstance->m418a50(0);
    g_bigInstance->m418af0(0);
    g_bigInstance->setFlags(0);
    ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 3);
    g_d36cb0 = 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046e530
// CatchMainGameDirector::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int CatchMainGameDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049da84 EzObject
struct CatchMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049da80 CatchSink1
struct CatchMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049da7c CatchSink2
struct CatchMainGameDirector;

// VTABLE: EZ2AC_UNPACKED 0x0049da78 CatchSink3
struct CatchMainGameDirector;

/* ---- CatchMainGameDirector's constructor ------------------------------ */

extern int  nextListB(struct ListNode **cur);   /* 0x4183b0 (src/nodelists.cpp) */
extern void set_4184e0(int idx, int val);       /* 0x4184e0 (src/globals.cpp) */
extern int  g_1b2eb6c;                          /* src/ez2acapp.cpp */
extern char g_modeName[256];                    /* 0x1b2e680 (src/modename.cpp) */
extern int  g_gameVolume;                       /* 0x4a8d78 (src/moderunner.cpp) */
extern void *g_1b2e844;                         /* src/moderunner.cpp */
extern ShowCredit *createShowCredit();          /* 0x450390 */

/* Which of the eight per-mode note kinds this round uses: the constructor's
   ladder tests them in this order and takes the first that is set. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e958
int g_kind958;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e8c8
int g_kind8c8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e990
int g_kind990;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e8c0
int g_kind8c0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e8d0
int g_kind8d0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e960
int g_kind960;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e970
int g_kind970;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e978
int g_kind978;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e91c
int g_1b2e91c;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e924
int g_1b2e924;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e898
int g_1b2e898;
// GLOBAL: EZ2AC_UNPACKED 0x01b2eba4
int g_1b2eba4;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef60
int g_1b2ef60;

extern int g_1b2e588;   /* src/modeselectctor.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0046eca0
CatchMainGameDirector::CatchMainGameDirector()
{
    ListNode *p;
    char gds[260];

    if (g_1b2e588 == 1)
    {
        ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 1);
        g_d36cb0 = 1;
    }
    fca0 = 2;
    f3c = 0;
    fc0c = 0;
    memset(fc10, 0, sizeof(fc10));
    f174 = 0;
    mixerA = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
    mixerB = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);
    mixerC = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
    mixerD = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
    if (mixerC != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerC->setValue(0x7dc2);
        }
        else
        {
            mixerC->setValue(0xdf6a);
        }
    }
    if (mixerD != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerD->setValue(0x5bef);
        }
        else
        {
            mixerD->setValue(0xbd97);
        }
    }
    f338 = 0.0f;
    f33c = 0.0f;
    g_1b2e91c = 0x80;
    if (mixerB != 0)
    {
        mixerB->setValue(0x5938);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(0xaaaa);
    }
    g_1b2e924 = 1;
    memset(f34c, 0, sizeof(f34c));
    memset(f178, 0, sizeof(f178));
    f758 = -1.0f;
    f75c = 432.0f;
    g_1b2e898 = 0;
    g_1b2eba4 = 0;
    g_bigInstance->setOutput(0, 0);
    g_bigInstance->setOutput(1, 0);
    g_bigInstance->setOutput(2, 0);
    g_bigInstance->setOutput(3, 0);
    f750 = 0;
    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049db38
        sprintf(gds, "System\\CV2Mix\\EZ2Catch\\EZ2Catch.gds");
        // STRING: EZ2AC_UNPACKED 0x0048fa20
        Log("GDS File Name : %s", gds);
    }
    else
    {
        sprintf(gds, "System\\%s\\%s.gds", g_modeName, g_modeName);
    }
    m46ca30(gds);
    credit = createShowCredit();
    credit->setPos(f758, f75c);
    f174 = m450570();
    pending = (DirectorPending *)createBattleMode();
    if (g_1b2ef60 != 0)
    {
        set_4184e0(6, 1);
    }
    if (m46dba0() == 0)
    {
        ((SoundBank *)g_1b2e844)->v0(1);
    }
    else
    {
        m46e4c0();
        g_bigInstance->m4189e0(1);
        if (findListB(0) != 0)
        {
            g_bigInstance->m418a50(1);
        }
        if (findListB(1) != 0)
        {
            g_bigInstance->m418af0(1);
        }
        m46c4d0();
        m46c760();
        if (g_kind958 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46cf00(p, 0, fca4); }
        }
        else if (g_kind8c8 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46e170(p, 0, fca4, 0); }
        }
        else if (g_kind990 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46df00(p, 0, fca4, 0); }
        }
        else if (g_kind8c0 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46e3e0(p, 1, 0, fca4); }
        }
        else if (g_kind8d0 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46e3e0(p, 0, 0, fca4); }
        }
        else if (g_kind960 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46e170(p, 0, fca4, 0); }
        }
        else if (g_kind970 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46cfc0(p, 0, fca4); }
        }
        else if (g_kind978 != 0)
        {
            p = 0;
            while (nextListB(&p)) { m46d0f0(p, 0, fca4); }
        }
        bd08 = 0;
        ((KGraphics *)g_sub1b2f054)->m408c40(1);
        f174->m450420(1, 10, (CatchSink1 *)this);
    }
}

/* ---- GFMainGameDirector's constructor --------------------------------- */

extern int  nextListC(struct ListNode **cur);      /* 0x418470 (src/nodelists.cpp) */
extern struct PlayerRecord *findPlayer(int id);    /* 0x418f30 (src/nodelists.cpp) */
extern int  countListC();                          /* 0x4184a0 (src/nodelists.cpp) */
extern int  g_1b2eb7c;                             /* src/gamestate.cpp */
extern int  g_demoPlay;                            /* 0x1b2ef78 (src/panels.cpp) */

/* The style-name prefix this mode builds, and the buffer it is copied into. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e5ac
char g_styleTag[36];
// GLOBAL: EZ2AC_UNPACKED 0x01b2e5d0
char g_stylePrefix[64];

// GLOBAL: EZ2AC_UNPACKED 0x01b2ea18
int g_1b2ea18;
// GLOBAL: EZ2AC_UNPACKED 0x01b5f180
int g_1b5f180;

// FUNCTION: EZ2AC_UNPACKED 0x00460110
GFMainGameDirector::GFMainGameDirector()
{
    ListNode *p;
    char style[128];
    char gds[260];

    if (g_1b2e588 == 1)
    {
        ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 1);
        g_d36cb0 = 1;
    }
    fca4 = 2;
    f20 = 0;
    fc10 = 0;
    f15c = 0;
    f158 = 0;
    f31c = 0;
    f32c = 0;
    b330 = 0;
    fcd8 = 0;
    g_1b2eba4 = 0;
    g_1b2e898 = 0;
    f324 = 0.0f;
    f328 = 0.0f;
    f320 = 0;
    g_bigInstance->setOutput(0, 0);
    g_bigInstance->setOutput(1, 0);
    g_bigInstance->setOutput(2, 0);
    g_bigInstance->setOutput(3, 0);
    fcac[0] = 0;
    fcac[1] = 0;
    fcac[2] = 0;
    fcac[3] = 0;
    fcac[4] = 0;
    fcac[5] = 0;
    fcac[6] = 0;
    fcac[7] = 0;
    memset(fc14, 0, sizeof(fc14));
    mixerA = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
    mixerB = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);
    mixerC = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
    mixerD = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
    if (mixerC != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerC->setValue(0x7dc2);
        }
        else
        {
            mixerC->setValue(0xdf6a);
        }
    }
    if (mixerD != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerD->setValue(0x5bef);
        }
        else
        {
            mixerD->setValue(0xbd97);
        }
    }
    g_1b2e91c = 0x80;
    if (mixerB != 0)
    {
        mixerB->setValue(0x5938);
    }
    if (mixerA != 0)
    {
        mixerA->setValue(0xaaaa);
    }
    memset(f340, 0, sizeof(f340));
    memset(f160, 0, sizeof(f160));
    f758 = -1.0f;
    f75c = 432.0f;
    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049a1a8
        sprintf(gds, "System\\CV2Mix\\ScratchMix\\ScratchMix.gds");
        if (g_1b2eb7c == 1)
        {
            // STRING: EZ2AC_UNPACKED 0x0048fa78
            sprintf(style, "STYLE_%s1_Black_", g_styleTag);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0048d510
            sprintf(style, "STYLE_%s1_", g_styleTag);
        }
        strcpy(g_stylePrefix, style);
        Log("GDS File Name : %s", gds);
    }
    else
    {
        sprintf(gds, "System\\%s\\%s.gds", g_modeName, g_modeName);
    }
    m45e190(gds);
    credit = createShowCredit();
    credit->setPos(f758, f75c);
    if (countListC() < 2)
    {
        credit->b10 = 1;
    }
    f32c = createBattleMode();
    g_1b5f180 = 1;
    f158 = m450570();
    f15c = m450570();
    if (g_1b2ea18 != 0)
    {
        set_4184e0(6, 1);
    }
    if (m45fd50() == 0)
    {
        ((SoundBank *)g_1b2e844)->v0(1);
    }
    else
    {
        m4600a0();
        g_bigInstance->m4189e0(g_demoPlay == 0);
        if (findPlayer(0) != 0)
        {
            g_bigInstance->m418a50(g_demoPlay == 0);
        }
        if (findPlayer(1) != 0)
        {
            g_bigInstance->m418af0(g_demoPlay == 0);
        }
        m45db70();
        m45de70();
        if (g_kind958 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45ed90(p, 0, fca8); }
        }
        else if (g_kind8c8 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45e9f0(p, 0, fca8, 1); }
        }
        else if (g_kind990 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45e770(p, 0, fca8, 1); }
        }
        else if (g_kind8c0 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45ec70(p, 1, 0, fca8); }
        }
        else if (g_kind8d0 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45ec70(p, 0, 0, fca8); }
        }
        else if (g_kind960 != 0)
        {
            p = 0;
            while (nextListC(&p)) { m45e9f0(p, 0, fca8, 0); }
        }
        ((KGraphics *)g_sub1b2f054)->m408c40(1);
        m45d810();
    }
}

/* ---- EZ2DJMainGameDirector's constructor ------------------------------ */

extern int g_mixStyle;     /* 0x1b2ebc8 (src/panels.cpp) */
extern int g_1b2e914;      /* src/panels.cpp */
extern int g_speedPercent; /* 0x1b2e890 (src/panels.cpp) */
extern int g_advSound;     /* 0x16963f8 (src/settings.cpp) */
extern int g_1b2eb44;      /* src/gamestate.cpp */
extern int g_1b2f084;      /* src/modeselectctor.cpp */
extern int g_1b2ea30;      /* src/modeselectctor.cpp */
extern int g_modeIndex;    /* 0x1b2e548 (src/modename.cpp) */
extern struct ListNode *findListA(int key);   /* 0x418e30 (src/nodelists.cpp) */

/* The mode's own file-name stem, indexed by g_mixStyle at a 0x20 stride.
   Placeholder storage - only the base, the stride and the copy out of it are
   evidenced here. */
// GLOBAL: EZ2AC_UNPACKED 0x0048f520
char g_mixStyleNames[13][32];

// GLOBAL: EZ2AC_UNPACKED 0x01b2eac8
int g_1b2eac8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e9a8
int g_1b2e9a8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e9b0
int g_1b2e9b0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e9a0
int g_1b2e9a0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2e998
int g_1b2e998;

// FUNCTION: EZ2AC_UNPACKED 0x00423f40
EZ2DJMainGameDirector::EZ2DJMainGameDirector()
{
    char style[128];
    char gds[260];

    if (g_1b2e588 == 1)
    {
        ((KGraphics *)g_sub1b2f054)->device->SetRenderState(0x16, 1);
        g_d36cb0 = 1;
    }
    fd18 = g_1b2f084;
    fd10 = 2;
    f20 = 0;
    fc7c = 0;
    f160 = 0;
    f15c = 0;
    f168 = 0;
    f350 = 0;
    ready = 0;
    fd48 = 0;
    f378 = 0;
    f7d4 = 0;
    f7a4 = 0;
    f7a8 = 0;
    f35c = 0;
    f358 = 0;
    f360 = 0.0f;
    f364 = 0.0f;
    f354 = 0;
    g_1b2e898 = 0;
    g_1b2eba4 = 0;
    g_1b2eac8 = 0;
    f79c = 0;
    fd1c[0] = 0;
    fd1c[1] = 0;
    fd1c[2] = 0;
    fd1c[3] = 0;
    fd1c[4] = 0;
    fd1c[5] = 0;
    fd1c[6] = 0;
    fd1c[7] = 0;
    g_bigInstance->setOutput(0, 0);
    g_bigInstance->setOutput(1, 0);
    g_bigInstance->setOutput(2, 0);
    g_bigInstance->setOutput(3, 0);
    f7a0 = 0x7f;
    memset(fc80, 0, sizeof(fc80));
    if (g_demoPlay != 0)
    {
        g_1b2e91c = 0x80;
        set_4184e0(0x0f, 0);
        set_4184e0(0x10, 0);
        set_4184e0(0x08, 0);
        set_4184e0(0x00, 0);
        set_4184e0(0x01, 0);
        set_4184e0(0x02, 0);
        set_4184e0(0x18, 0);
        set_4184e0(0x03, 0);
        set_4184e0(0x04, 0);
        set_4184e0(0x1d, 0);
        set_4184e0(0x16, 0);
        set_4184e0(0x05, 0);
        set_4184e0(0x17, 0);
        set_4184e0(0x20, 0);
        set_4184e0(0x21, 0);
        set_4184e0(0x1f, 0);
        set_4184e0(0x1e, 0);
        set_4184e0(0x19, 0);
        set_4184e0(0x1a, 0);
        set_4184e0(0x06, 0);
        set_4184e0(0x07, 0);
        set_4184e0(0x11, 0);
        g_1b2e914 = 5;
        g_speedPercent = 250;
    }
    if (g_1b2eb44 != 0)
    {
        g_1b2e91c = 0x80;
        set_4184e0(0x0f, 0);
        set_4184e0(0x10, 0);
        set_4184e0(0x08, 0);
        set_4184e0(0x00, 0);
        set_4184e0(0x01, 0);
        set_4184e0(0x02, 0);
        set_4184e0(0x18, 0);
        set_4184e0(0x03, 0);
        set_4184e0(0x04, 0);
        set_4184e0(0x1d, 0);
        set_4184e0(0x16, 0);
        set_4184e0(0x05, 0);
        set_4184e0(0x17, 0);
        set_4184e0(0x1f, 0);
        set_4184e0(0x1e, 0);
        set_4184e0(0x19, 0);
        set_4184e0(0x1a, 0);
        set_4184e0(0x06, 0);
        set_4184e0(0x07, 0);
        set_4184e0(0x11, 0);
        g_1b2e914 = 5;
    }
    mixerA = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
    mixerB = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);
    mixerC = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
    mixerD = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
    if (mixerC != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerC->setValue(0x7dc2);
        }
        else
        {
            mixerC->setValue(0xdf6a);
        }
    }
    if (mixerD != 0)
    {
        if (g_gameVolume == 0)
        {
            mixerD->setValue(0x5bef);
        }
        else
        {
            mixerD->setValue(0xbd97);
        }
    }
    m420360(g_1b2e91c);
    memset(f380, 0, sizeof(f380));
    memset(f16c, 0, sizeof(f16c));
    f7ac = -1.0f;
    f7b0 = 435.0f;
    if (g_1b2eb6c != 0)
    {
        strcpy(g_styleTag, g_mixStyleNames[g_mixStyle]);
        if (g_mixStyle == 6)
        {
            // STRING: EZ2AC_UNPACKED 0x0048faa4
            sprintf(gds, "System\\CV2Mix\\%s\\%s_2p.gds",
                    // STRING: EZ2AC_UNPACKED 0x0048fac0
                    "7streetmix", "7streetmix");
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0048fa8c
            sprintf(gds, "System\\CV2Mix\\%s\\%s.gds", g_styleTag, g_styleTag);
        }
        if (g_1b2eb7c == 1)
        {
            sprintf(style, "STYLE_%s1_Black_", g_styleTag);
        }
        else
        {
            sprintf(style, "STYLE_%s1_", g_styleTag);
        }
        if (g_mixStyle == 6)
        {
            if (g_1b2eb7c == 1)
            {
                // STRING: EZ2AC_UNPACKED 0x0048fa5c
                sprintf(style, "STYLE_7streetmix1_Black_");
            }
            else
            {
                // STRING: EZ2AC_UNPACKED 0x0048fa48
                sprintf(style, "STYLE_7streetmix1_");
            }
        }
        else if (g_mixStyle == 7)
        {
            // STRING: EZ2AC_UNPACKED 0x0048fa34
            sprintf(style, "STYLE_streetmix1_");
            g_1b2ea30 = 2;
        }
        else if (g_1b2eb7c == 1)
        {
            sprintf(style, "STYLE_%s1_Black_", g_styleTag);
        }
        else
        {
            sprintf(style, "STYLE_%s1_", g_styleTag);
        }
        strcpy(g_stylePrefix, style);
        Log("GDS File Name : %s", gds);
    }
    else
    {
        sprintf(gds, "System\\%s\\%s.gds", g_modeName, g_modeName);
    }
    m421190(gds);
    m420d90();
    f15c = m450570();
    f160 = m450570();
    if (m423a90() == 0)
    {
        ((SoundBank *)g_1b2e844)->v0(1);
    }
    else
    {
        if (g_demoPlay != 0 && g_advSound == 0)
        {
            notify->v11(0);
        }
        m423ed0();
        if (f7bc != 0)
        {
            m420d40();
        }
        m420a60();
        m420e30();
        pending = (DirectorPending *)createBattleMode();
        if (g_modeIndex == 4 || g_modeIndex == 8)
        {
            if (findListA(0) != 0)
            {
                m422f20(0x21, 0);
            }
            else if (findListA(1) != 0)
            {
                m422f20(0x21, 1);
            }
        }
        if (g_1b2e9a8 != 0)
        {
            m422f20(0x20, 1);
        }
        if (g_1b2e9b0 != 0)
        {
            m422f20(0x21, 1);
        }
        if (g_kind958 != 0)
        {
            m422f20(0x16, 1);
        }
        else if (g_1b2e9a0 != 0)
        {
            m422f20(0x1f, 1);
        }
        else if (g_kind8c8 != 0)
        {
            m422f20(0x04, 1);
        }
        else if (g_kind990 != 0)
        {
            m422f20(0x1d, 1);
        }
        else if (g_kind8c0 != 0)
        {
            m422f20(0x03, 1);
        }
        else if (g_kind8d0 != 0)
        {
            m422f20(0x05, 1);
        }
        else if (g_kind960 != 0)
        {
            m422f20(0x17, 1);
        }
        else if (g_1b2e998 != 0)
        {
            m422f20(0x1e, 1);
        }
        else if (g_kind970 != 0)
        {
            m422f20(0x19, 1);
        }
        else if (g_kind978 != 0)
        {
            m422f20(0x1a, 1);
        }
        ((KGraphics *)g_sub1b2f054)->m408c40(1);
        if (f20 == 0)
        {
            f15c->m450420(1, 10, (EZ2DJSink1 *)this);
        }
        else if (f20 == 2)
        {
            f160->m450420(1, 10, (EZ2DJSink1 *)this);
        }
    }
}

/* Whether the audience panel is drawn at all, and the two rate limits the
   panel itself reads back. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb48
int g_useAudiencePanel;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e7d4
int g_1b2e7d4;

extern float g_rateTarget;      /* 0x1b2ebd8 (src/panels.cpp) */

// GLOBAL: EZ2AC_UNPACKED 0x01b2f04c
float g_1b2f04c;

/* Seven floats read from the highest member down to the lowest, then the two
   trailing ones, and four values published to globals. */
// FUNCTION: EZ2AC_UNPACKED 0x00420810
int EZ2DJMainGameDirector::m420810(KParser *p)
{
    int use;

    readGdsInt(&f324, p);
    readGdsInt(&f328, p);
    readGdsFloat(&f340, p);
    readGdsFloat(&f33c, p);
    readGdsFloat(&f338, p);
    readGdsFloat(&f334, p);
    readGdsFloat(&f330, p);
    readGdsFloat(&f344, p);
    readGdsFloat(&f348, p);

    use = f324;
    g_useAudiencePanel = use;
    g_1b2e7d4 = f328;
    // STRING: EZ2AC_UNPACKED 0x0048f744
    Log("Use Audience Panel = %s",
        // STRING: EZ2AC_UNPACKED 0x0048f764
        use != 0 ? "TRUE" :
        // STRING: EZ2AC_UNPACKED 0x0048f75c
        "FALSE");
    g_rateTarget = f344;
    g_1b2f04c = f348;
    return 1;
}

/* (g_songTable's storage moved to src/globals.cpp when SongTable gained its
   real 0x927c layout - a 37 KB placeholder does not belong in a TU whose own
   data is referenced with a bias.) */

/* One byte of the matched song-table entry, published for the panels. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eba0
int g_1b2eba0;

/* Read the mode's .gds and run its sections. A file that will not open leaves
   the whole set of defaults behind and reports failure; anything else is
   dispatched by section name, and an unknown one turns the audience panel off
   and says so. */
// FUNCTION: EZ2AC_UNPACKED 0x00423790
int EZ2DJMainGameDirector::loadGds(char *name)
{
    char sec[0x100];
    KParser parser;

    if (g_songTable.find(m4208f0(), songEntry) != 0)
    {
        g_1b2eba0 = (unsigned char)songEntry[0x30];
    }

    if (parser.readFile(name) == 0)
    {
        f320 = 2.0f;
        f318 = 0.2f;
        f314 = 0.1f;
        f310 = -1.8f;
        f2ec = 0x63;
        f304 = 6;
        f300 = 0x18;
        f2fc = 0x24;
        f2f8 = 0x48;
        f30c = -4.8f;
        return 0;
    }

    g_useAudiencePanel = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048e974
        if (_stricmp("General", sec) == 0)
        {
            m420640(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f958
        else if (_stricmp("JudgmentDelta", sec) == 0)
        {
            readGdsInt(&f304, &parser);
            readGdsInt(&f300, &parser);
            readGdsInt(&f2fc, &parser);
            readGdsInt(&f2f8, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f948
        else if (_stricmp("GaugeUpDownRate", sec) == 0)
        {
            m4206e0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f934
        else if (_stricmp("SongOriginalName", sec) == 0)
        {
            readGdsToken(f7d8, &parser);
            readGdsInt(&f7dc, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f924
        else if (_stricmp("AudienceRate", sec) == 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0048f908
            Log("Found Audience Rate Line");
            m420810(&parser);
        }
        else
        {
            g_useAudiencePanel = 0;
            // STRING: EZ2AC_UNPACKED 0x0048f8e8
            Log("Not Found Audience Rate Line");
        }
    }

    return 1;
}

/* GFMainGameDirector's copy: three sections instead of five, no audience panel,
   and its General arm overwrites the float it just read with a constant. */
// FUNCTION: EZ2AC_UNPACKED 0x0045faf0
int GFMainGameDirector::loadGds(char *name)
{
    char sec[0x100];
    KParser parser;

    if (g_songTable.find(m45da00(), songEntry) != 0)
    {
        g_1b2eba0 = (unsigned char)songEntry[0x30];
    }

    if (parser.readFile(name) == 0)
    {
        f314 = 2.0f;
        f2e0 = 0x63;
        f2f8 = 0;
        f2f4 = 0;
        f2f0 = 0;
        f2ec = 0;
        f30c = 0.0f;
        f308 = 0.0f;
        f304 = 0.0f;
        f300 = 0.0f;
        return 0;
    }

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&f2e0, &parser);
            readGdsFloat(&f314, &parser);
            f314 = 1.6f;
        }
        else if (_stricmp("JudgmentDelta", sec) == 0)
        {
            readGdsInt(&f2f8, &parser);
            readGdsInt(&f2f4, &parser);
            readGdsInt(&f2f0, &parser);
            readGdsInt(&f2ec, &parser);
        }
        else if (_stricmp("GaugeUpDownRate", sec) == 0)
        {
            m45d960(&parser);
        }
    }

    return 1;
}

extern int g_1b2ebac;   /* 0x1b2ebac (src/gamepanels.cpp) */

/* And Catch's: EZ2DJ's five sections with AudienceRate and SongOriginalName
   replaced by a single RaceMode flag. */
/* Pick two DIFFERENT values in [base, base+span): thirty-two attempts, and
   if every pair ties both outputs get `base`. Two of the six arguments are
   never read, and neither is `this`. */
// FUNCTION: EZ2AC_UNPACKED 0x0046dea0
void CatchMainGameDirector::m46dea0(int, int base, int span, int,
                                    int *outA, int *outB)
{
    int i;

    for (i = 0; i < 32; i++)
    {
        int a = rand() % span + base;
        int b = rand() % span + base;

        if (a != b)
        {
            *outA = a;
            *outB = b;
            return;
        }
    }

    *outA = base;
    *outB = base;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046d8d0
int CatchMainGameDirector::loadGds(char *name)
{
    char sec[0x100];
    KParser parser;

    if (g_songTable.find(m46c360(), songEntry) != 0)
    {
        g_1b2eba0 = (unsigned char)songEntry[0x30];
    }

    if (parser.readFile(name) == 0)
    {
        f32c = 1.5f;
        f324 = 0.2f;
        f320 = 0.1f;
        f31c = -1.8f;
        f2f8 = 0x63;
        f310 = 6;
        f30c = 0x18;
        f308 = 0x24;
        f304 = 0x48;
        f318 = -4.8f;
        return 0;
    }

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&f2f8, &parser);
            readGdsFloat(&f32c, &parser);
            f32c = 1.6f;
        }
        else if (_stricmp("JudgmentDelta", sec) == 0)
        {
            readGdsInt(&f310, &parser);
            readGdsInt(&f30c, &parser);
            readGdsInt(&f308, &parser);
            readGdsInt(&f304, &parser);
        }
        else if (_stricmp("GaugeUpDownRate", sec) == 0)
        {
            m46c2b0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0049dac4
        else if (_stricmp("RaceMode", sec) == 0)
        {
            readGdsInt(&f330, &parser);
            g_1b2ebac = f330;
        }
    }

    return 1;
}

extern int readGdsToken(char *out, KParser *p);   /* 0x4561e0 */
extern int countListB();                          /* 0x4183e0 */

/* Only the id nextListB hands back; the node's real class is not modelled. */
struct SlotNodeHead
{
    char _pad0[8];
    int  f8;                   /* 0x08 */
};

/* CatchMainGameDirector::m46ca30 @0x46ca30 - the Catch DIRECTOR's own .gds,
   which is a different and smaller file from the note field's (whose reader is
   CatchPanel::LoadPVI @0x472650). Seven sections and no unknown-section log -
   it simply ignores anything it does not know.

   The tail is the interesting part: when the round has exactly ONE list node
   and that node's id is 1, the second of two parallel 0x10-byte arrays is
   overwritten from the first, element by element. The two sit 0x1bc apart,
   which is why the original addresses the source as [eax-0x1bc] off the
   destination walk. Both live inside f34c, so they are indexed rather than
   carved out - splitting the array would cost the constructor's memset. */
// FUNCTION: EZ2AC_UNPACKED 0x0046CA30
int CatchMainGameDirector::m46ca30(char *name)
{
    char      sec[0x100];
    char      digits[0x80];
    KParser   parser;
    /* Initialized HERE, not just before nextListB: the original zeroes this
       slot between the parser's constructor and readFile, and again later. */
    ListNode *cur = 0;
    int       n;
    int       i;
    int       discard;

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f348 = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0048f89c
        if (_strnicmp("Slot", sec, 4) == 0)
        {
            strcpy(digits, sec + 4);
            n = atoi(digits);
            m46c820(&parser, n - 1);
        }
        else if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&f348, &parser);
            readGdsInt(&discard, &parser);
            readGdsInt(&f768, &parser);
            readGdsInt(&f76c, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f890
        else if (_stricmp("Background", sec) == 0)
        {
            readGdsInt(&f340, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f888
        else if (_stricmp("Panel", sec) == 0)
        {
            readGdsToken(f6c4, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f880
        else if (_stricmp("Gauge", sec) == 0)
        {
            readGdsInt(&f74c, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f874
        else if (_stricmp("ShowCredit", sec) == 0)
        {
            m46c8c0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f864
        else if (_stricmp("JudgmentLink", sec) == 0)
        {
            m46c900(&parser);
        }
    }

    if (countListB() == 1)
    {
        cur = 0;
        nextListB(&cur);

        if (((SlotNodeHead *)cur)->f8 == 1)
        {
            for (i = 0; i < f34c[0x70]; i++)
            {
                f34c[0x73 + i * 4] = f34c[0x73 + i * 4 - 0x6f];
            }
        }
    }

    return 1;
}

/* GFMainGameDirector::m45e190 @0x45e190 - the GF director's .gds, the third
   and last of this family. Catch's @0x46ca30 is the near twin; the difference
   is all in the tail, which here has the extra whole-block branch that the
   PARKED EZ2DJ one @0x421190 also has: when f33c is 1 the entire 0x1c4-byte
   slot block is copied over the second one, otherwise the second block's
   elements are fixed up one 0x10 at a time.

   ShowCredit is shared with the Catch director outright - both call
   @0x46c8c0 - which works because that loader never touches `this`. */
// FUNCTION: EZ2AC_UNPACKED 0x0045E190
int GFMainGameDirector::m45e190(char *name)
{
    char      sec[0x100];
    char      digits[0x80];
    KParser   parser;
    ListNode *cur = 0;
    int       n;
    int       i;

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f33c = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_strnicmp("Slot", sec, 4) == 0)
        {
            strcpy(digits, sec + 4);
            n = atoi(digits);
            m45df90(&parser, n - 1);
        }
        else if (_stricmp("General", sec) == 0)
        {
            m45df30(&parser);
        }
        else if (_stricmp("Background", sec) == 0)
        {
            readGdsInt(&f334, &parser);
        }
        else if (_stricmp("Panel", sec) == 0)
        {
            readGdsToken(f6c8, &parser);
        }
        else if (_stricmp("Gauge", sec) == 0)
        {
            readGdsInt(&f750, &parser);
        }
        else if (_stricmp("ShowCredit", sec) == 0)
        {
            ((CatchMainGameDirector *)this)->m46c8c0(&parser);
        }
        else if (_stricmp("JudgmentLink", sec) == 0)
        {
            m45e060(&parser);
        }
    }

    if (countListC() == 1)
    {
        cur = 0;
        nextListC(&cur);

        if (((SlotNodeHead *)cur)->f8 == 1)
        {
            if (f33c == 1)
            {
                memcpy(&f340[0x71], &f340[0], 0x1c4);
            }
            else
            {
                for (i = 0; i < f340[0x72]; i++)
                {
                    f340[0x77 + i * 4] = f340[0x77 + i * 4 - 0x71];
                }
            }
        }
    }

    return 1;
}

extern int countListA();                       /* 0x418310 */
extern int nextListA(struct ListNode **cur);   /* 0x4182e0 */

/* EZ2DJMainGameDirector::m421190 @0x421190 - the EZ2DJ director's .gds, and
   the LAST of the binary's fourteen KParser file readers. Decoded long ago
   (wip/maingame-gds.cpp) and parked at 53.69% on a frame that could not be
   accounted for; what unstuck it was matching the close twin
   GFMainGameDirector::m45e190 @0x45e190 above and writing this one in ITS
   shape - same declaration order, same `= 0` on the cursor at its
   declaration, same indexing into f380 rather than carving the tail's two
   blocks out of it.

   The extra arm over GF's is InGameSongSelect. */
// FUNCTION: EZ2AC_UNPACKED 0x00421190
int EZ2DJMainGameDirector::m421190(char *name)
{
    char      sec[0x100];
    char      digits[0x80];
    KParser   parser;
    ListNode *cur = 0;
    int       n;
    int       i;

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f37c = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_strnicmp("Slot", sec, 4) == 0)
        {
            strcpy(digits, sec + 4);
            n = atoi(digits);
            m420f60(&parser, n - 1);
        }
        else if (_stricmp("General", sec) == 0)
        {
            m420f00(&parser);
        }
        else if (_stricmp("Background", sec) == 0)
        {
            readGdsInt(&f370, &parser);
        }
        else if (_stricmp("Panel", sec) == 0)
        {
            readGdsToken(f708, &parser);
        }
        else if (_stricmp("Gauge", sec) == 0)
        {
            readGdsInt(&f794, &parser);
        }
        else if (_stricmp("ShowCredit", sec) == 0)
        {
            m421000(&parser);
        }
        else if (_stricmp("JudgmentLink", sec) == 0)
        {
            m421060(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048f850
        else if (_stricmp("InGameSongSelect", sec) == 0)
        {
            m421040(&parser);
        }
    }

    if (countListA() == 1)
    {
        cur = 0;
        nextListA(&cur);

        if (((SlotNodeHead *)cur)->f8 == 1)
        {
            if (f37c == 1)
            {
                memcpy(&f380[0x71], &f380[0], 0x1c4);
            }
            else
            {
                for (i = 0; i < f380[0x72]; i++)
                {
                    f380[0x77 + i * 4] = f380[0x77 + i * 4 - 0x71];
                }
            }
        }
    }

    return 1;
}

/* The ShowCredit section - m46c8c0 @0x46c8c0. Two float pairs, the credit
   overlay's position and size. Modelled on CatchMainGameDirector because that
   is whose address range it sits in, but the GF director calls it too - the
   same borrowing that GroupTexture does among the panels, and possible for
   the same reason: it reads nothing off `this` that the two do not share. */
// FUNCTION: EZ2AC_UNPACKED 0x0046C8C0
int CatchMainGameDirector::m46c8c0(KParser *p)
{
    readGdsFloat2(&f758, &f75c, p);
    readGdsFloat2(&f760, &f764, p);
    return 1;
}

/* THE JudgmentLink SECTION - one handler per main-game director, the same 110
   instructions three times over at three different base offsets:

       CatchMainGameDirector::m46c900 @0x46c900   base 0x784
       GFMainGameDirector::m45e060    @0x45e060   base 0x788
       EZ2DJMainGameDirector::m421060 @0x421060   base 0x7f4

   Two counts, and after each a run of that many { length, that many ints }
   entries at a 0x48 stride. The record is modelled as a VIEW cast off `this`
   rather than carved into the three classes: the offsets are all it needs, the
   cast costs no code, and it keeps three big classes - one of them the class
   the cookbook warns about - untouched.

   The layout is forced rather than guessed: all three bases give the same
   deltas (countB at +4, the first run at +0xc, the second at +0x24c), and
   0x24c - 0xc is exactly 8 * 0x48. */
// FUNCTION: EZ2AC_UNPACKED 0x0046C900
int CatchMainGameDirector::m46c900(KParser *p)
{
    int i;

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f784, p);

    for (i = 0; i < f784; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_a[i].n, p);
        readGdsIntN(link_a[i].v, link_a[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f788, p);

    for (i = 0; i < f788; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_b[i].n, p);
        readGdsIntN(link_b[i].v, link_b[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045E060
int GFMainGameDirector::m45e060(KParser *p)
{
    int i;

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f788, p);

    for (i = 0; i < f788; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_a[i].n, p);
        readGdsIntN(link_a[i].v, link_a[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f78c, p);

    for (i = 0; i < f78c; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_b[i].n, p);
        readGdsIntN(link_b[i].v, link_b[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00421060
int EZ2DJMainGameDirector::m421060(KParser *p)
{
    int i;

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f7f4, p);

    for (i = 0; i < f7f4; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_a[i].n, p);
        readGdsIntN(link_a[i].v, link_a[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);

    p->parseToken(2);
    p->parseToken(0xc);
    p->parseToken(6);
    readGdsInt(&f7f8, p);

    for (i = 0; i < f7f8; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsInt(&link_b[i].n, p);
        readGdsIntN(link_b[i].v, link_b[i].n, p);
        p->parseToken(7);
    }

    p->parseToken(7);
    return 1;
}


/* The SlotN section - m46c820 @0x46c820. One slot record per section, picked
   by the index the reader split off the section name, at a 0x1bc stride
   inside f34c - the same 0x1bc the reader's own tail uses as the bias between
   its two parallel blocks, which is what confirms the stride.

   Each record is a count followed by that many 0x10-byte entries of
   { int pair; int; a copy of that int }. Indexed rather than carved: f34c is
   one array and splitting it would cost the constructor's memset. */
// FUNCTION: EZ2AC_UNPACKED 0x0046C820
int CatchMainGameDirector::m46c820(KParser *p, int index)
{
    int *s = &f34c[0x6f * index];
    int  i;

    readGdsInt(&s[1], p);

    for (i = 0; i < s[1]; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsIntN(&s[2 + i * 4], 2, p);
        readGdsInt(&s[4 + i * 4], p);
        p->parseToken(7);
        s[5 + i * 4] = s[4 + i * 4];
    }

    return 1;
}

/* EZ2DJ's copy of the SlotN handler. NOT quite Catch's: its slot stride is
   0x1c4 rather than 0x1bc - which is the same bias its own reader's tail uses,
   so each director's two constants agree - and it republishes the count and a
   zero into the record before walking it. */
// FUNCTION: EZ2AC_UNPACKED 0x00420F60
int EZ2DJMainGameDirector::m420f60(KParser *p, int index)
{
    int *s = &f380[0x71 * index];
    int  i;

    readGdsInt(&s[1], p);
    s[2] = s[1];
    s[3] = 0;

    for (i = 0; i < s[1]; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsIntN(&s[4 + i * 4], 2, p);
        readGdsInt(&s[6 + i * 4], p);
        p->parseToken(7);
        s[7 + i * 4] = s[6 + i * 4];
    }

    return 1;
}

extern char g_modeName[256];                    /* 0x1b2e680 */
extern struct PlayerRecord *findPlayer(int id); /* 0x418f30 */

/* GF's SlotN handler - the odd one of the three. Same 0x1c4-stride record as
   EZ2DJ's, but in ClubMix, when player 0 is already registered, the slot is
   published one short; otherwise ClubMix raises a flag instead. The other two
   directors have no mode-dependent case at all. */
// FUNCTION: EZ2AC_UNPACKED 0x0045DF90
int GFMainGameDirector::m45df90(KParser *p, int index)
{
    int *s = &f340[0x71 * index];
    int  i;

    readGdsInt(&s[1], p);

    // STRING: EZ2AC_UNPACKED 0x0048cf0c
    if (_stricmp(g_modeName, "ClubMix") == 0)
    {
        if (findPlayer(0) != 0)
        {
            s[2] = s[1] - 1;
            s[3] = 0;
        }
        else
        {
            s[3] = 1;
            s[2] = s[1];
        }
    }
    else
    {
        s[2] = s[1];
        s[3] = 0;
    }

    for (i = 0; i < s[1]; i++)
    {
        p->parseToken(2);
        p->parseToken(0xc);
        p->parseToken(6);
        readGdsIntN(&s[4 + i * 4], 2, p);
        readGdsInt(&s[6 + i * 4], p);
        p->parseToken(7);
        s[7 + i * 4] = s[6 + i * 4];
    }

    return 1;
}

/* The remaining director .gds handlers.

   The two "General" copies: four readGdsInt calls - the first into the same
   flag the reader zeroes before its loop (f33c / f37c), then three consecutive
   ints. What earlier readings missed is the GATE between the third and fourth
   read: a global forces the third int to 1. That store is why the third
   target's address has to survive its own call, which is what takes the
   callee-saved register (`push ebx` / `lea ebx,[esi+0x76c]`) and lets cl defer
   the first three cleanups into one `add esp,0x18`. Written without the gate
   the function is four independent calls and can only reach 62.50%.

   The two directors read DIFFERENT globals for the same gate - GF's is
   0x1696400, EZ2DJ's is g_eventMode @0x1696404. */
extern int g_1696400;   /* 0x1696400 (src/ez2acapp.cpp) */
extern int g_eventMode; /* 0x1696404 (src/settings.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x0045df30
int GFMainGameDirector::m45df30(KParser *p)
{
    readGdsInt(&f33c, p);
    readGdsInt(&f768, p);
    readGdsInt(&f76c, p);

    if (g_1696400 != 0)
    {
        f76c = 1;
    }

    readGdsInt(&f770, p);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00420f00
int EZ2DJMainGameDirector::m420f00(KParser *p)
{
    readGdsInt(&f37c, p);
    readGdsInt(&f7c4, p);
    readGdsInt(&f7c8, p);

    if (g_eventMode != 0)
    {
        f7c8 = 1;
    }

    readGdsInt(&f7cc, p);
    return 1;
}

/* EZ2DJ's ShowCredit - the same two float pairs the other two directors share
   through @0x46c8c0, but on its own fields rather than borrowed. */
// FUNCTION: EZ2AC_UNPACKED 0x00421000
int EZ2DJMainGameDirector::m421000(KParser *p)
{
    readGdsFloat2(&f7ac, &f7b0, p);
    readGdsFloat2(&f7b4, &f7b8, p);
    return 1;
}

/* InGameSongSelect - one int, and the only section no other director has. */
// FUNCTION: EZ2AC_UNPACKED 0x00421040
int EZ2DJMainGameDirector::m421040(KParser *p)
{
    readGdsInt(&f7bc, p);
    return 1;
}

/* Bring up the in-game song-select overlay.

   The overlay is built out of the pool by createInGameSongSelect @0x426310 and
   given a pointer straight back to this director. Then the first node of list A
   is handed to its own setter with "is this player 1?" - and if the list is
   empty the whole thing reports failure, which is the only reason the return
   value exists. */
// FUNCTION: EZ2AC_UNPACKED 0x00420d40
int EZ2DJMainGameDirector::m420d40()
{
    ListNode *cur;

    f164 = createInGameSongSelect();
    f164->f14 = this;

    cur = 0;
    if (nextListA(&cur) == 0)
    {
        return 0;
    }

    ((PlayerSlot *)cur)->m430500(((PlayerSlot *)cur)->id == 0);
    return 1;
}
