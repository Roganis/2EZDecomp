/* TestInput - the operator test menu's INPUT TEST page (see include/testinput.h).

   The constructor is the whole screen layout: twenty button markers, laid out
   in five rows. Reading the table off, the first row (y=53) is the six panel
   keys of one side, y=180 four scratch/pedal markers in blue, y=233 the six
   keys of the other side, y=223 the two effector positions at the far left and
   right, and y=292 two more centred markers. */

#include "testinput.h"
#include "ez2acapp.h"   /* EzSubsystem, g_sub1b2eae4 */
#include "loader.h"     /* Log */
#include "directors.h"  /* TestModeSink */
#include "bigbase.h"    /* Obj401780, g_bigInstance */
#include "kprofile.h"
#include "kparser.h"
#include "kengineapp.h"
#include <stdio.h>
#include <string.h>

/* src/moderunner.cpp - the machine volume, and the screen singleton every page
   walks the live-instance lists through. */
extern int g_gameVolume;   /* 0x4a8d78 */
extern void *g_1b2e844;    /* 0x1b2e844 - an Obj401780 (set by its own s2) */

/* src/classinfo.cpp - TestModeDirector's object record. */
extern EzObjectInfo g_info_4afb70;   /* 0x4afb70 */

/* Paired with the page's m_adjustVolume. */
// GLOBAL: EZ2AC_UNPACKED 0x016963fc
int g_16963fc;

/* src/panels.cpp - the attract-mode flag the input page's lamps follow. */
extern int g_demoPlay;   /* 0x1b2ef78 */

/* src/com_wrappers.cpp - the graphics singleton's COM-wrapper methods, and
   src/misc.cpp - its text colour setter. */
struct Obj40BB00 { void doOp7(int v); void doOp10(unsigned int v); };
struct Obj406C00 { void setField12c(int v); };

/* src/stubs.cpp - the engine's printf-to-screen. */
extern void drawTextf(KGraphics *g, int x, int y, const char *fmt, ...);

/* src/mixer.cpp - one mixer line's level. */
struct MixerControl { void setValue(unsigned long value); };   /* 0x412a10 */

/* src/mixer.cpp - the mixer singleton's control factory. */
struct KSNDMixer { void *addControl(int dst, int type); };     /* 0x412af0 */

/* The test menu's caption language: every page in the family reads it to pick
   between a Korean and an English string, and the menu itself clamps it to
   [0, 1] (@0x4767b5) - so it is a two-value toggle, 0 = Korean. */
// GLOBAL: EZ2AC_UNPACKED 0x016963f4
int g_language;

/* The build number the pages print as "Version %d.%02d", i.e. hundredths.
   Both globals are read by the page's own update @0x477260 below, which is at
   96.43% for one register - see the note there. */
// GLOBAL: EZ2AC_UNPACKED 0x004a8d6c
int g_version;

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classTestInput;    /* 0x4af22c */
extern EzObjectInfo  g_info_4af23c;     /* 0x4af23c */

// FUNCTION: EZ2AC_UNPACKED 0x004753b0
KRuntimeClass *TestInput::getRuntimeClass()
{
    return &classTestInput;
}

// FUNCTION: EZ2AC_UNPACKED 0x004753c0
DrawGroup *TestInput::getObjectInfo()
{
    return &g_info_4af23c;
}

/* Empty: nothing here owns anything. cl still emits the vptr store and the
   tail-jump to the folded base destructor. */
// FUNCTION: EZ2AC_UNPACKED 0x004753d0
TestInput::~TestInput()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00475a70
// TestInput::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`, ICF-folded across every class
   that has one; src/constants.cpp owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestInput::v5(int, int, int)
{
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004754b0
TestInput::TestInput()
{
    add(0x04, 165.0f, 53.0f, 0xffffffffu, 0x80808080u);
    add(0x06, 245.0f, 53.0f, 0xffff0000u, 0x80800000u);
    add(0x07, 286.0f, 53.0f, 0xffff0000u, 0x80800000u);
    add(0x08, 332.0f, 53.0f, 0xffff0000u, 0x80800000u);
    add(0x09, 378.0f, 53.0f, 0xffff0000u, 0x80800000u);
    add(0x05, 462.0f, 53.0f, 0xffffffffu, 0x80808080u);
    add(0x0b, 195.0f, 180.0f, 0xff0000ffu, 0x80000080u);
    add(0x0d, 252.0f, 180.0f, 0xff0000ffu, 0x80000080u);
    add(0x13, 380.0f, 180.0f, 0xff0000ffu, 0x80000080u);
    add(0x15, 436.0f, 180.0f, 0xff0000ffu, 0x80000080u);
    add(0x0a, 180.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x0c, 225.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x0e, 272.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x12, 365.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x14, 411.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x16, 458.0f, 233.0f, 0xffffffffu, 0x80808080u);
    add(0x0f, 55.0f, 223.0f, 0xffffffffu, 0x80808080u);
    add(0x17, 545.0f, 223.0f, 0xffffffffu, 0x80808080u);
    add(0x11, 209.0f, 292.0f, 0xffffffffu, 0x80808080u);
    add(0x19, 394.0f, 292.0f, 0xffffffffu, 0x80808080u);
}

/* One button marker. The two analog readouts (kinds 0xf and 0x17) print the
   raw port value as three digits; the two turntables (0x11 and 0x19) draw a
   wide flat box; every key draws a tall one - lit at raw state >= 2, dim
   below. The && guard is written so the turntable arm trails the function,
   which is where the original lays it. */
// FUNCTION: EZ2AC_UNPACKED 0x00475a90
void TestInput::drawItem(TestInputItem *item)
{
    int state;

    if (item->kind == 0xf)
    {
        drawTextf((KGraphics *)g_sub1b2f054, (int)item->x, (int)item->y,
                  "%03d", g_bigInstance->port103);
        return;
    }
    if (item->kind == 0x17)
    {
        drawTextf((KGraphics *)g_sub1b2f054, (int)item->x, (int)item->y,
                  "%03d", g_bigInstance->port104);
        return;
    }
    if (item->kind != 0x11 && item->kind != 0x19)
    {
        state = g_bigInstance->getState(item->kind);
        ((KGraphics *)g_sub1b2f054)->drawRect(item->x, item->y,
                                              item->x + 22.0f, item->y + 33.0f,
                                              state >= 2 ? item->lit : item->dim);
        return;
    }
    state = g_bigInstance->getState(item->kind);
    ((KGraphics *)g_sub1b2f054)->drawRect(item->x, item->y,
                                          item->x + 54.0f, item->y + 21.0f,
                                          state >= 2 ? item->lit : item->dim);
}

/* One frame of the page: reset the blend state, draw the title and footer in
   the caption language, the build number, and then every button marker. The
   twin of TestVideo::update @0x477260 (parked) - same two caption blocks, and
   the footer's setField12c is duplicated into both arms here too. */
// FUNCTION: EZ2AC_UNPACKED 0x00475bc0
void TestInput::update()
{
    int i;

    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, 0);
    ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
    ((KGraphics *)g_sub1b2f054)->m408130(0);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
    if (g_language == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049f1e4
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, "\300\324\267\302 \305\327\275\272\306\256 ");
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049f1d8
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, "Input Test");
    }

    if (g_language == 0)
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x0049f1b8
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, "\300\314\306\345\305\315 1\271\370 + 3\271\370 : \301\326 \270\336\264\272\267\316 ");
    }
    else
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x0049f1a0
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, "Effect 1 + 3  to Exit");
    }

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffc8c8);
    // STRING: EZ2AC_UNPACKED 0x0049f190
    drawTextf((KGraphics *)g_sub1b2f054, 460, 440, "Version %d.%02d",
              g_version / 100, g_version % 100);

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
    for (i = 0; i < m_count; i++)
    {
        drawItem(&m_items[i]);
    }
}

/* One poll: Test (input 1) or both effector keys held (6 and 8 at state 3)
   leaves the page; otherwise the three lamp channels follow the demo-play
   flag. */
// FUNCTION: EZ2AC_UNPACKED 0x004753e0
void TestInput::update2(int, int)
{
    if (g_bigInstance->getState(1) != 2 &&
        (g_bigInstance->getState(6) != 3 || g_bigInstance->getState(8) != 3))
    {
        g_bigInstance->m418a50(g_demoPlay == 0);
        g_bigInstance->m418af0(g_demoPlay == 0);
        g_bigInstance->m4189e0(g_demoPlay == 0);
    }
    else
    {
        DrawNode *node = 0;

        ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
        ((TestModeSink *)((char *)node + 0x10))->onMenu(2, (int)this, 0);
        delete this;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00475d20
TestInput *createTestInput()
{
    g_objectRecord = &g_info_4af23c;
    return new TestInput;
}

// VTABLE: EZ2AC_UNPACKED 0x0049f130
struct TestInput;

/* ---- TestSound, the test menu's SOUND page ---------------------------- */

extern KRuntimeClass classTestSound;   /* 0x4afbb8 */
extern EzObjectInfo  g_info_4afbc8;    /* 0x4afbc8 */

// FUNCTION: EZ2AC_UNPACKED 0x00476890
KRuntimeClass *TestSound::getRuntimeClass()
{
    return &classTestSound;
}

// FUNCTION: EZ2AC_UNPACKED 0x004768a0
DrawGroup *TestSound::getObjectInfo()
{
    return &g_info_4afbc8;
}

// FUNCTION: EZ2AC_UNPACKED 0x004768b0
TestSound::~TestSound()
{
    g_sub1b2eae4->freeSample((void *)m_all);
}

/* Load the three test tones, take the machine volume as the page's initial
   on/off state, then build the menu and open it on entry 9. */
// FUNCTION: EZ2AC_UNPACKED 0x00477000
TestSound::TestSound()
{
    m_menu = 0;
    // STRING: EZ2AC_UNPACKED 0x0049fe64
    m_all = (TestTone *)g_sub1b2eae4->createSample("system\\TestMode\\soundtest_All.wav", 1, 0);
    // STRING: EZ2AC_UNPACKED 0x0049fe40
    m_bass = (TestTone *)g_sub1b2eae4->createSample("system\\TestMode\\SoundTest_Bass.wav", 1, 0);
    // STRING: EZ2AC_UNPACKED 0x0049fe18
    m_treble = (TestTone *)g_sub1b2eae4->createSample("system\\TestMode\\SoundTest_Treble.wav", 1, 0);

    m_adjustVolume = 0;
    m_volume = g_gameVolume;
    if (m_volume < 0)
        m_volume = 0;
    if (m_volume > 1)
        m_volume = 1;

    // STRING: EZ2AC_UNPACKED 0x0049fe04
    Log("Initialize Settings");
    // STRING: EZ2AC_UNPACKED 0x0049fdec
    Log("m_bAdjustVolume = %d", m_adjustVolume);

    buildMenu();
    m_menu->m_language = g_language;
    m_menu->m_index = 9;
}

// FUNCTION: EZ2AC_UNPACKED 0x00477110
TestSound *createTestSound()
{
    g_objectRecord = &g_info_4afbc8;
    return new TestSound;
}

// FUNCTION: EZ2AC_UNPACKED 0x004770f0
// TestSound::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestSound::v5(int, int, int)
{
    return 0;
}

/* The menu reporting a row of the SOUND page. Rows 0..4 are the tone tests -
   each sets both tones' volume and pan and starts whichever is not already
   playing - row 5 stops them, row 7 steps the machine volume through the
   mixer, row 8 restarts and resets it, row 9 commits and leaves. Row 6 does
   nothing, but it HAS a jump-table entry, so the source had an arm for it.

   `this` is the SINK, so everything is reached through the view struct. */
// FUNCTION: EZ2AC_UNPACKED 0x00476950
void TestSoundSink::onMenu(int, int what, int)
{
    TestSoundSinkFields *f = (TestSoundSinkFields *)this;

    switch (what)
    {
    case 0:
        f->m_bass->setVolume(-10000);
        f->m_treble->setVolume(-10000);
        f->m_bass->setPan(0);
        f->m_treble->setPan(0);
        if ((f->m_bass->status() & 1) == 0)
        {
            f->m_bass->play(1);
        }
        if ((f->m_treble->status() & 1) == 0)
        {
            f->m_treble->play(1);
        }
        break;

    case 1:
        f->m_bass->setVolume(10000);
        f->m_treble->setVolume(10000);
        f->m_bass->setPan(0);
        f->m_treble->setPan(0);
        if ((f->m_bass->status() & 1) == 0)
        {
            f->m_bass->play(1);
        }
        if ((f->m_treble->status() & 1) == 0)
        {
            f->m_treble->play(1);
        }
        break;

    case 2:
        f->m_bass->setVolume(0);
        f->m_treble->setVolume(0);
        f->m_bass->setPan(0);
        f->m_treble->setPan(0);
        if ((f->m_bass->status() & 1) == 0)
        {
            f->m_bass->play(1);
        }
        if ((f->m_treble->status() & 1) == 0)
        {
            f->m_treble->play(1);
        }
        break;

    case 3:
        f->m_bass->setVolume(0);
        f->m_treble->setVolume(0);
        f->m_bass->setPan(0);
        f->m_treble->setPan(-10000);
        if ((f->m_bass->status() & 1) == 0)
        {
            f->m_bass->play(1);
        }
        if ((f->m_treble->status() & 1) == 0)
        {
            f->m_treble->play(1);
        }
        break;

    case 4:
        f->m_bass->setVolume(0);
        f->m_treble->setVolume(0);
        f->m_bass->setPan(-10000);
        f->m_treble->setPan(0);
        if ((f->m_bass->status() & 1) == 0)
        {
            f->m_bass->play(1);
        }
        if ((f->m_treble->status() & 1) == 0)
        {
            f->m_treble->play(1);
        }
        break;

    case 5:
        f->m_bass->stop();
        f->m_bass->q12(0);
        f->m_treble->stop();
        f->m_treble->q12(0);
        break;

    case 6:
        break;

    case 7:
        f->m_line = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
        f->m_cd = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);
        f->m_master = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
        f->m_wave = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
        if (f->m_volume >= 1)
        {
            // STRING: EZ2AC_UNPACKED 0x0049fdd8
            Log("Change Volume LOW");
            if (f->m_master != 0)
            {
                f->m_master->setValue(0x7dc2);
            }
            if (f->m_wave != 0)
            {
                f->m_wave->setValue(0x5bef);
            }
            f->m_volume = 0;
        }
        else
        {
            f->m_volume = f->m_volume + 1;
            // STRING: EZ2AC_UNPACKED 0x0049fdc4
            Log("Change Volume HIGH");
            if (f->m_master != 0)
            {
                f->m_master->setValue(0xdf6a);
            }
            if (f->m_wave != 0)
            {
                f->m_wave->setValue(0xbd97);
            }
        }
        /* ONE statement, which the tail-duplicator copies into both arms - the
           constant arm keeps its re-read that way. Written into each arm, cl
           folds the `= 0` case into `mov g_gameVolume, 0`. */
        g_gameVolume = f->m_volume;
        break;

    case 8:
        f->m_bass->stop();
        f->m_bass->q12(0);
        f->m_treble->stop();
        f->m_treble->q12(0);
        f->m_volume = 1;
        g_gameVolume = 1;
        f->m_adjustVolume = 0;
        g_16963fc = 0;
        f->m_menu->m_rows[7].value = f->m_volume;
        if (f->m_master != 0)
        {
            f->m_master->setValue(0xdf6a);
        }
        if (f->m_wave != 0)
        {
            f->m_wave->setValue(0xbd97);
        }
        break;

    case 9:
        {
            DrawNode *node;

            f->m_bass->stop();
            f->m_treble->stop();
            /* the volume read into a local BEFORE the path buffer is
               declared - which moves the buffer in the frame */
            int vol = f->m_menu->m_rows[7].value;
            char name[260];

            f->m_volume = vol;

            sprintf(name, "%s.ini", g_app->className());
            g_iniProfile->setInt("GameVolume", f->m_volume);
            g_iniProfile->writeFile(name);

            {
                KParser parser;

                parser.m4133d0(name);

                g_gameVolume = f->m_volume;
                g_16963fc = f->m_adjustVolume;

                delete f->m_menu;
                node = 0;
                ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
                ((TestModeSink *)((char *)node + 0x10))->onMenu(
                    2, (int)((char *)this - 0x10), 0);
                delete (TestSound *)((char *)this - 0x10);
            }
        }
        break;
    }
}


// VTABLE: EZ2AC_UNPACKED 0x0049fdac EzObject
struct TestSound;

// VTABLE: EZ2AC_UNPACKED 0x0049fda8 TestSoundSink
struct TestSound;

/* ---- TestVideo, the test menu's VIDEO page ---------------------------- */

extern KRuntimeClass classTestVideo;   /* 0x4afc24 */
extern EzObjectInfo  g_info_4afc34;    /* 0x4afc34 */

// FUNCTION: EZ2AC_UNPACKED 0x00477170
KRuntimeClass *TestVideo::getRuntimeClass()
{
    return &classTestVideo;
}

// FUNCTION: EZ2AC_UNPACKED 0x00477180
DrawGroup *TestVideo::getObjectInfo()
{
    return &g_info_4afc34;
}

/* Empty - the two textures belong to the graphics profile, not to the page.
   cl still emits the vptr store and the tail-jump to the folded base dtor. */
// FUNCTION: EZ2AC_UNPACKED 0x00477190
TestVideo::~TestVideo()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00477410
// TestVideo::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int TestVideo::v5(int, int, int)
{
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00477390
TestVideo::TestVideo()
{
    m_index = 0;
    // STRING: EZ2AC_UNPACKED 0x004a00d4
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\TestMode\\Grid.bmp",
                                             &m_tex[0], 0);
    // STRING: EZ2AC_UNPACKED 0x004a00b4
    ((KGraphics *)g_sub1b2f054)->loadTexture("system\\TestMode\\ColorBar.bmp",
                                             &m_tex[1], 0);
}

/* One poll of the page. The Test button (input 1 or 6) leaves: find the live
   TestModeDirector among the instances of its record, tell its sink the page
   is done, and delete the page. Service (2, 5 or 4) just flips the image.

   The five tests sit in one contiguous run with both arms out of line, which
   is the grouped-`&&` / `||` shape, not an if/else-if chain. */
// FUNCTION: EZ2AC_UNPACKED 0x004771a0
void TestVideo::update2(int, int)
{
    if (g_bigInstance->getState(1) != 2 && g_bigInstance->getState(6) != 2)
    {
        if (g_bigInstance->getState(2) == 2 ||
            g_bigInstance->getState(5) == 2 ||
            g_bigInstance->getState(4) == 2)
        {
            m_index++;
            if (m_index >= 2)
            {
                m_index = 0;
            }
        }
    }
    else
    {
        DrawNode *node = 0;

        ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
        ((TestModeSink *)((char *)node + 0x10))->onMenu(2, (int)this, 0);
        delete this;
    }
}

/* The page's own new-plus-ctor factory, the same shape as the four screen
   directors' (src/ezscreens.cpp). */
// FUNCTION: EZ2AC_UNPACKED 0x00477430
TestVideo *createTestVideo()
{
    g_objectRecord = &g_info_4afc34;
    return new TestVideo;
}

// VTABLE: EZ2AC_UNPACKED 0x004a0038
struct TestVideo;

/* ---- TestSound's menu ---------------------------------------------------

   Twenty MenuEntry records, ten per language, plus the two page titles. All
   placeholder storage: only the ADDRESSES have to line up, and a `push` of an
   interior pointer would not resolve, so each record is its own entity rather
   than one array (reccmp names a push immediate only at an entity's base). */
// GLOBAL: EZ2AC_UNPACKED 0x0049fbc8
MenuEntry g_menu_49fbc8;
// GLOBAL: EZ2AC_UNPACKED 0x0049fbd4
MenuEntry g_menu_49fbd4;
// GLOBAL: EZ2AC_UNPACKED 0x0049fbe0
MenuEntry g_menu_49fbe0;
// GLOBAL: EZ2AC_UNPACKED 0x0049fbec
MenuEntry g_menu_49fbec;
// GLOBAL: EZ2AC_UNPACKED 0x0049fbf8
MenuEntry g_menu_49fbf8;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc04
MenuEntry g_menu_49fc04;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc10
MenuEntry g_menu_49fc10;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc1c
MenuEntry g_menu_49fc1c;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc28
MenuEntry g_menu_49fc28;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc34
MenuEntry g_menu_49fc34;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc40
MenuEntry g_menu_49fc40;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc4c
MenuEntry g_menu_49fc4c;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc58
MenuEntry g_menu_49fc58;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc64
MenuEntry g_menu_49fc64;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc70
MenuEntry g_menu_49fc70;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc7c
MenuEntry g_menu_49fc7c;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc88
MenuEntry g_menu_49fc88;
// GLOBAL: EZ2AC_UNPACKED 0x0049fc94
MenuEntry g_menu_49fc94;
// GLOBAL: EZ2AC_UNPACKED 0x004afc04
MenuEntry g_menu_4afc04;
// GLOBAL: EZ2AC_UNPACKED 0x004afc18
MenuEntry g_menu_4afc18;

/* The page title, one pointer per language. */
// GLOBAL: EZ2AC_UNPACKED 0x004afbf4
char *g_soundTitleKor;
// GLOBAL: EZ2AC_UNPACKED 0x004afbf8
char *g_soundTitleEng;

/* Build the SOUND page's menu: ten rows in Korean, then the same ten in
   English, then the volume row's current value. Every row goes through the
   menu pointer read back out of the object, which is why it is reloaded before
   each call. */
// FUNCTION: EZ2AC_UNPACKED 0x00476e40
void TestSound::buildMenu()
{
    m_menu = createSystemMenu();
    /* The cast pointer NAMED in a local, as the original stores it
       (the inline cast was one instruction off, 2 points). */
    TestSoundSink *owner = (TestSoundSink *)this;
    m_menu->m_owner = owner;

    m_menu->m_language = 0;
    strcpy(m_menu->m_title[m_menu->m_language], g_soundTitleKor);
    m_menu->addEntry(0, &g_menu_49fbc8);
    m_menu->addEntry(1, &g_menu_49fbd4);
    m_menu->addEntry(2, &g_menu_49fbe0);
    m_menu->addEntry(3, &g_menu_49fbec);
    m_menu->addEntry(4, &g_menu_49fbf8);
    m_menu->addEntry(5, &g_menu_49fc04);
    m_menu->addEntry(6, &g_menu_49fc10);
    m_menu->addEntry(7, &g_menu_4afc04);
    m_menu->addEntry(8, &g_menu_49fc1c);
    m_menu->addEntry(9, &g_menu_49fc28);

    m_menu->m_language = 1;
    strcpy(m_menu->m_title[m_menu->m_language], g_soundTitleEng);
    m_menu->addEntry(0, &g_menu_49fc34);
    m_menu->addEntry(1, &g_menu_49fc40);
    m_menu->addEntry(2, &g_menu_49fc4c);
    m_menu->addEntry(3, &g_menu_49fc58);
    m_menu->addEntry(4, &g_menu_49fc64);
    m_menu->addEntry(5, &g_menu_49fc70);
    m_menu->addEntry(6, &g_menu_49fc7c);
    m_menu->addEntry(7, &g_menu_4afc18);
    m_menu->addEntry(8, &g_menu_49fc88);
    m_menu->addEntry(9, &g_menu_49fc94);

    m_menu->m_rows[7].value = m_volume;
    m_menu->m_rows[6].enabled = 0;
}

/* TestVideo::update @0x477260 - the operator video-test page, and
   TestInput::update above with less on it: no per-key grid, just whichever
   of the two test images is up drawn full-screen, a caption bar behind each
   of the two text lines, and the version. The service button steps the image
   and the test button leaves, which is what the second line says.

   96.43%, and the whole difference is ONE register plus the jmp displacement
   that follows from its encoding: the original loads the graphics singleton
   for the footer caption with `mov eax,ds:g_sub1b2f054` (the 5-byte A1 form)
   where cl picks edx (6 bytes), so the preceding jmp is 0x17 against 0x18.
   Nothing else in the 300 bytes differs.

   Load-bearing, and confirmed by the diff - do not "tidy" any of it:
   - the singleton is RE-READ at every call site; hoisting it into a local
     holds it in esi across the body and costs ~40%;
   - the FOOTER's caption block duplicates setField12c(0xffffff) into BOTH
     arms while the TITLE's calls it once before the `if`. That is not a
     stylistic guess: a global's `cmp` cannot be hoisted across a call cl
     cannot see, so the original's `cmp g_language,0` sitting ABOVE the call
     proves the compare came first in the source there. One call before the
     `if` in the footer scores 92.12%;
   - both captions are two separate calls, not a ternary - cl cross-jumps
     them back into the one `call drawTextf` the original has.

   Swept without moving the register: if/else, inverted if/else, two separate
   ifs, switch/default, goto-into-arm, a trailing label, an extra scope, a
   local in the else arm only, drawTextf/setField12c returning int, a
   __forceinline caption() helper, `volatile int g_language`, and cl builds
   13.10.2179 and 13.10.4035 as well as 3077.

   THE LEAD WORTH FOLLOWING is not a source shape. TestInput::update above is
   this function's twin - same two caption blocks, same duplicated
   setField12c - and it lands 100% on the first build. The one structural
   difference is what precedes the footer: TestInput goes straight into it,
   while this function first calls KGraphics::m40a6a0 @0x40a6a0, WHICH IS
   STILL A STUB. Per the fictional-clobber-set rule (and the setLamps
   @0x475dd0 precedent, where reconstructing the stubbed callee fixed its
   caller with no change to the caller's own source), reconstruct m40a6a0 and
   retry. It is decoded already - wip/kgraphics-colorfill.cpp, a DirectDraw
   colour fill parked at 58.54% on its own rect stores - so the experiment
   costs an integration, not an analysis. */
// FUNCTION: EZ2AC_UNPACKED 0x00477260
void TestVideo::update()
{
    ((Obj40BB00 *)g_sub1b2f054)->doOp10(0);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_tex[m_index], 0, 0, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->m40a6a0(0x14, 0x14, 0x56, 0x11, 0);

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
    if (g_language == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049f8c4
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, "\310\255\270\351 \305\327\275\272\306\256 ");
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049f8a0
        drawTextf((KGraphics *)g_sub1b2f054, 20, 20, "Video Test");
    }

    ((KGraphics *)g_sub1b2f054)->m40a6a0(0x14, 0x1b8, 0x258, 0x11, 0);

    if (g_language == 0)
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x004a0080
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, "\274\255\272\361\275\272 \271\366\306\260 : \307\327\270\361 \300\314\265\277,  \305\327\275\272\306\256 \271\366\306\260 : \301\326 \270\336\264\272\267\316 ");
    }
    else
    {
        ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffffff);
        // STRING: EZ2AC_UNPACKED 0x004a0050
        drawTextf((KGraphics *)g_sub1b2f054, 20, 440, "Service Button to Select,  Test Button to Exit");
    }

    ((Obj406C00 *)g_sub1b2f054)->setField12c(0xffc8c8);
    drawTextf((KGraphics *)g_sub1b2f054, 460, 440, "Version %d.%02d",
              g_version / 100, g_version % 100);
}

/* TestSound::update2 @0x476910 - pump the page's three test tones, each only
   while it exists. Slot 17 (+0x44), the same pump slot GFResultDirector's
   sound objects use. The page loads all three by name but only ever frees
   the first, so the other two can be null independently of it - hence three
   separate guards rather than one. */
// FUNCTION: EZ2AC_UNPACKED 0x00476910
void TestSound::update2(int, int)
{
    if (m_all != 0)
        m_all->tick();
    if (m_bass != 0)
        m_bass->tick();
    if (m_treble != 0)
        m_treble->tick();
}
