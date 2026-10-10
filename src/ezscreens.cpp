#include "gamepanels.h"   /* GFPanel, for its factory */
#include <string.h>
#include "ezscreen.h"
#include "ez2acapp.h" /* KGraphics, g_sub1b2f054 */
#include "loader.h"   /* Log */
#include "directors.h" /* InGameSongSelect */

/* src/mixer.cpp - the mixer view classes Obj401780::s2 drives. Declared, never
   defined here: their bodies must stay in their own TU. The declarations mirror
   src/mixer.cpp exactly so the mangled names line up. */
class MixerControl
{
public:
    HMIXEROBJ m_hmx;
    DWORD     m_controlId;

    DWORD getValue();
    void  setValue(DWORD value);
};

/* The 0x1b2e70c singleton IS the mixer: v2 @0x415cb0 brings it up through vtable
   slot 2 and v3 @0x414ff0 shuts it down through slot 3, which is exactly
   KSNDMixer's init @0x412a60 / close @0x412d40 pair. It is modeled as
   EzSubsystem there because only the slot shape mattered; here the concrete type
   does, so the calls cast. Only the two methods s2 uses are declared - the
   mangling does not depend on the base class. */
/* src/modename.cpp / src/moderunner.cpp - what the result screen's OnCreateGame
   branches on. */
extern int g_modeIndex;
extern int g_1b2e7c4;

/* src/misc.cpp - the saturating unsigned subtract. (The stride-0x14 element
   accessors that used to be modelled separately here are now Obj40C180's own
   getState/setChannel/getStateB - see include/bigbase.h.) */
extern unsigned int SatSub(unsigned int a, unsigned int b);

/* Another already-matched view of the graphics singleton (src/accessors.cpp):
   the KProfile-through-+0x1d0 call. */
struct ObjWithProfile { char _pad[0x1d0]; void *profile; void initProfile(); };

/* src/library.cpp - WINMM's timeGetTime through the import thunk. */
extern "C" DWORD __stdcall timeGetTime(void);

/* src/moderunner.cpp - the screen s2 publishes, and the flag that picks between
   the two master/wave levels it forces. */
extern void *g_1b2e844;
extern int   g_gameVolume;

class KSNDMixer
{
public:
    void *addControl(int dst, int type);
    int   remove(void *key);
    int   clear();
};

/* src/classinfo.cpp - the object records the slot-4 overrides register. */
extern EzObjectInfo g_info_4adfe8, g_info_4ae118, g_info_4ae154, g_info_4ae4d0;
extern EzObjectInfo g_info_4ae974, g_info_4aea68, g_info_4aec78, g_info_4aecb4;
extern EzObjectInfo g_info_4af198, g_info_4af23c, g_info_4af278, g_info_4afb70;
extern EzObjectInfo g_info_4afbc8, g_info_4afc34, g_info_4ae428, g_info_4ae380;
extern EzObjectInfo g_info_4adfa8, g_info_4ae1cc, g_info_4ae2c0, g_info_4a9040;
extern EzObjectInfo g_info_4a90b8, g_info_4a8f1c, g_info_4a9130, g_info_4a907c;
extern EzObjectInfo g_info_4a8f94, g_info_4a9004, g_info_4a8ee0, g_info_4a8f58;
extern EzObjectInfo g_info_4a90f4, g_info_4ae190, g_info_4aeb88, g_info_4aeb4c;
extern EzObjectInfo g_info_4aec00, g_info_4ae794, g_info_4aaee8, g_info_4aea2c;
extern EzObjectInfo g_info_4ac758, g_info_4ae4d0;
extern EzObjectInfo g_info_4ae69c, g_info_4ae660, g_info_4ae6d8;

/* The record the next pooled allocation comes from (src/moderunner.cpp). */

/* Methods of the five screen classes (see include/ezscreen.h).

   Four of the screens have a slot-5 that is nothing but a one-line forwarder,
   compiled to a bare `jmp` because the call is in tail position and `this` needs
   no adjustment. The body it forwards to has to live in ANOTHER TU
   (src/stubs.cpp): visible here, cl would inline it and the `jmp` would be gone.
   Screen48D348::s5 @0x44dc80 is the one screen whose slot 5 has a real body. */

// FUNCTION: EZ2AC_UNPACKED 0x0045c1b0
void Screen48D40C::s5()
{
    createWarningDirector();
}

// FUNCTION: EZ2AC_UNPACKED 0x004761b0
void Screen48D310::s5()
{
    createTestModeDirector();
}

// FUNCTION: EZ2AC_UNPACKED 0x004699f0
void Screen48D428::s5()
{
    createWarningShortDirector();
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a4d0
void Screen48D3D4::s5()
{
    createObj46AA80();
}

/* Slot 4 of every screen has the same straight-line shape: register the object
   records this screen's pools need with the screen itself (addInfo @0x401850
   links each record into up to three intrusive lists rooted at
   this+0x10/+0x18/+0x20, selected by the record's flags). Written as plain
   statements, cl caches `this` in esi across the run of thiscalls, which is the
   whole body. The records are the placeholder storage in src/classinfo.cpp. */

// FUNCTION: EZ2AC_UNPACKED 0x0044dc50
void Screen48D348::s4()
{
    addInfo(&g_info_4adfe8);
    addInfo(&g_info_4ae118);
    addInfo(&g_info_4ae154);
}

/* The one screen whose slot 5 has a real body: bring up the credits overlay at
   (sentinel-x, y = 435) and then the title screen. The trailing factory call is
   a tail `jmp` - s5 returns void and the factory takes no arguments. */
// FUNCTION: EZ2AC_UNPACKED 0x0044dc80
void Screen48D348::s5()
{
    ShowCredit *credit = createShowCredit();

    credit->setPos(-1.0f, 435.0f);
    createTitleDirector();
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c190
void Screen48D40C::s4()
{
    addInfo(&g_info_4ae4d0);
    addInfo(&g_info_4ae154);
}

// FUNCTION: EZ2AC_UNPACKED 0x004699d0
void Screen48D428::s4()
{
    addInfo(&g_info_4ae974);
    addInfo(&g_info_4ae154);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a4a0
void Screen48D3D4::s4()
{
    addInfo(&g_info_4aea68);
    addInfo(&g_info_4ae154);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x00476140
void Screen48D310::s4()
{
    addInfo(&g_info_4afb70);
    addInfo(&g_info_4afc34);
    addInfo(&g_info_4af23c);
    addInfo(&g_info_4af278);
    addInfo(&g_info_4afbc8);
    addInfo(&g_info_4af198);
    addInfo(&g_info_4aecb4);
    addInfo(&g_info_4aec78);
}

/* Slot 6 of the screen base, shared by all five screens: dump the texture
   statistics. It ignores `this` entirely - every number comes from the graphics
   singleton. Reading the global once per argument is deliberate: the original
   loads it three times because countSurfaces() clobbers ecx, and cl cannot CSE a
   global across a call, so hoisting it into a local would NOT match. */
// FUNCTION: EZ2AC_UNPACKED 0x00455b70
void Obj401780::s6()
{
    // STRING: EZ2AC_UNPACKED 0x0048d9f8
    Log("Texture Report - Cnt=%-4d, Surfaces=%-4d, %-9d Bytes", ((KGraphics *)g_sub1b2f054)->m_texCount, ((KGraphics *)g_sub1b2f054)->countSurfaces(), ((KGraphics *)g_sub1b2f054)->m_texBytes);
}

/* The ranking screen's two slots. s4 is the same addInfo run as its siblings; s5
   is the second screen slot 5 with a real body - bring up the RankingDirector,
   then the same credits overlay at (sentinel-x, 435) Screen48D348::s5 uses. The
   trailing setPos is a `call`, not a tail `jmp`, because its two arguments are
   already pushed. */
// FUNCTION: EZ2AC_UNPACKED 0x004566f0
void Screen48D3F0::s4()
{
    addInfo(&g_info_4ae428);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x00456710
void Screen48D3F0::s5()
{
    ShowCredit *credit;

    createRankingDirector();
    credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
}

/* Slot 2 of the screen base, shared by all the screens: take the four mixer
   controls the game cares about, remember what the machine had them set to (in
   f34..f40, so slot 3 can restore them) and force the game's own levels, then
   hand off to the graphics subsystem. The four `addControl` calls read the mixer
   singleton from its global each time - cl cannot CSE a global across a call,
   which is exactly what the original does. The two levels that depend on
   g_gameVolume are written as two calls in an if/else, not a ternary: cl turns the
   ternary into the branchless `neg/sbb` flag trick, while the two arms
   cross-jump into the original's single call fed by push/jmp/push. */
// FUNCTION: EZ2AC_UNPACKED 0x004184f0
void Obj401780::s2()
{
    MixerControl *master;
    MixerControl *wave;
    MixerControl *line;
    MixerControl *cd;

    g_1b2e844 = this;

    master = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
    wave   = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
    line   = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
    cd     = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);

    if (master != 0)
    {
        f34 = master->getValue();
        if (g_gameVolume == 0)
        {
            master->setValue(0x7dc2);
        }
        else
        {
            master->setValue(0xdf6a);
        }
    }

    if (wave != 0)
    {
        f38 = wave->getValue();
        if (g_gameVolume == 0)
        {
            wave->setValue(0x5bef);
        }
        else
        {
            wave->setValue(0xbd97);
        }
    }

    if (line != 0)
    {
        f3c = line->getValue();
        line->setValue(0xaaaa);
    }

    if (cd != 0)
    {
        f40 = cd->getValue();
        cd->setValue(0x5938);
    }

    ((KSNDMixer *)g_sub1b2e70c)->remove(master);
    ((KSNDMixer *)g_sub1b2e70c)->remove(wave);
    ((KSNDMixer *)g_sub1b2e70c)->remove(line);
    ((KSNDMixer *)g_sub1b2e70c)->remove(cd);

    ((KGraphics *)g_sub1b2f054)->resetFrameTimer();
}

/* Slot 1 of the screen base: one frame. Advance the screen's clock (f30 holds
   the last timestamp, and the elapsed time is the SATURATING difference, so a
   wrapped timer yields 0 rather than a huge step), tick the engine, then walk the
   two group lists - tick every live, unhidden node, then draw them between the
   graphics subsystem's frame calls. The mid-frame check reads element 0x1a of the
   engine object's table; only mode 2 gets the extra call.

   Obj401780::run @0x401930 pumps this through vtable slot 1 until f8 goes to
   zero, so this is where every screen actually spends its time. */
// FUNCTION: EZ2AC_UNPACKED 0x00418620
void Obj401780::s1()
{
    unsigned int now;
    unsigned int elapsed;
    DrawGroup *group;
    DrawNode *node;

    now = timeGetTime();
    elapsed = SatSub(now, f30);
    f30 = now;
    g_bigInstance->tick(now);

    group = f18;
    while (group != 0)
    {
        node = group->nodes;

        while (node != 0)
        {
            if ((node->flags & 1) != 0 && (node->flags & 2) == 0)
            {
                node->tick(now, elapsed);
            }

            node = node->next;
        }

        group = group->nextTick;
    }

    group = f20;
    ((KGraphics *)g_sub1b2f054)->beginFrame();
    ((KGraphics *)g_sub1b2f054)->m405e10();

    while (group != 0)
    {
        node = group->nodes;

        while (node != 0)
        {
            if ((node->flags & 1) != 0 && (node->flags & 2) == 0)
            {
                node->draw();
            }

            node = node->next;
        }

        group = group->nextDraw;
    }

    if (g_bigInstance->getState(0x1a) == 2)
    {
        ((KGraphics *)g_sub1b2f054)->screenShot();
    }

    ((KGraphics *)g_sub1b2f054)->endFrame();
}

/* The three key ranges s3 resets, one helper call each. Written as helpers taking
   the object because that is what the original's codegen says: the engine global
   is read ONCE per range (`mov ecx,[g]; mov esi,ecx`) and copied to ecx for the
   rest of the range's calls - the inlined-parameter shape, not the reload-per-call
   one a bare `g_bigInstance->clearSlot(...)` sequence would give. */
static __forceinline void clearSlots6to9(Obj40C180 *p)
{
    p->setChannel(6, -1);
    p->setChannel(7, -1);
    p->setChannel(8, -1);
    p->setChannel(9, -1);
}

static __forceinline void clearSlots10to16(Obj40C180 *p)
{
    p->setChannel(10, -1);
    p->setChannel(11, -1);
    p->setChannel(12, -1);
    p->setChannel(13, -1);
    p->setChannel(14, -1);
    p->setChannel(15, -1);
    p->setChannel(16, -1);
}

static __forceinline void clearSlots18to24(Obj40C180 *p)
{
    p->setChannel(18, -1);
    p->setChannel(19, -1);
    p->setChannel(20, -1);
    p->setChannel(21, -1);
    p->setChannel(22, -1);
    p->setChannel(23, -1);
    p->setChannel(24, -1);
}

/* Slot 3 of the screen base: leave the screen. The mixer half mirrors s2 - the
   same four controls, but this time the line and CD levels are RESTORED from
   f3c/f40 while master and wave are re-forced to the same constants s2 used - and
   then the engine is wound down: three ranges of key slots reset, the flags
   cleared, one last tick, the graphics teardown, and the mixer's controls
   released wholesale in a tail call. */
// FUNCTION: EZ2AC_UNPACKED 0x00418fa0
void Obj401780::s3()
{
    MixerControl *master;
    MixerControl *wave;
    MixerControl *line;
    MixerControl *cd;

    master = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 0);
    wave   = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(1, 0);
    line   = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 2);
    cd     = (MixerControl *)((KSNDMixer *)g_sub1b2e70c)->addControl(0, 1);

    if (master != 0)
    {
        if (g_gameVolume == 0)
        {
            master->setValue(0x7dc2);
        }
        else
        {
            master->setValue(0xdf6a);
        }
    }

    if (wave != 0)
    {
        if (g_gameVolume == 0)
        {
            wave->setValue(0x5bef);
        }
        else
        {
            wave->setValue(0xbd97);
        }
    }

    if (line != 0)
    {
        line->setValue(f3c);
    }

    if (cd != 0)
    {
        cd->setValue(f40);
    }

    ((KSNDMixer *)g_sub1b2e70c)->remove(master);
    ((KSNDMixer *)g_sub1b2e70c)->remove(wave);
    ((KSNDMixer *)g_sub1b2e70c)->remove(line);
    ((KSNDMixer *)g_sub1b2e70c)->remove(cd);

    clearSlots6to9((Obj40C180 *)g_bigInstance);
    clearSlots10to16((Obj40C180 *)g_bigInstance);
    clearSlots18to24((Obj40C180 *)g_bigInstance);
    g_bigInstance->setFlags(0);

    g_bigInstance->tick(timeGetTime());

    ((ObjWithProfile *)g_sub1b2f054)->initProfile();
    ((KGraphics *)g_sub1b2f054)->m403f60();
    ((KGraphics *)g_sub1b2f054)->m408c80();
    g_sub1b2eae4->m40e770();

    ((KSNDMixer *)g_sub1b2e70c)->clear();
}

// FUNCTION: EZ2AC_UNPACKED 0x00455b30
void Screen48D364::s4()
{
    addInfo(&g_info_4ae380);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x0044d8f0
void Screen48D32C::s4()
{
    addInfo(&g_info_4adfa8);
    addInfo(&g_info_4ae118);
    addInfo(&g_info_4ae154);
}

// FUNCTION: EZ2AC_UNPACKED 0x004505f0
void Screen48D380::s4()
{
    addInfo(&g_info_4ae1cc);
    addInfo(&g_info_4ae2c0);
    addInfo(&g_info_4ae118);
}

/* Tear every group the screen owns down: tell each node to release (slot 1 with
   the flag set), then unlink the group from all three lists. The node's `next` is
   read BEFORE the call - the release may destroy the node - and the group's own
   next likewise before its links are cleared. */
// FUNCTION: EZ2AC_UNPACKED 0x004018c0
void Obj401780::m4018c0()
{
    DrawGroup *group = f10;

    while (group != 0)
    {
        DrawNode *node = group->nodes;

        while (node != 0)
        {
            DrawNode *next = node->next;

            node->v1(1);
            node = next;
        }

        DrawGroup *nextGroup = group->nextOwned;

        group->nextOwned = 0;
        group->nextDraw = 0;
        group->nextTick = 0;
        group = nextGroup;
    }
}

/* Carve one allocation into a pool per registered record: sum every record's
   objectSize * count, VirtualAlloc that much, zero it, then walk the records
   again handing each one its slice (base at +0x1c, end at +0x20) and resetting
   its cursors. Obj401780::run calls this once the screen's slot 4 has registered
   everything, and frees f28 on the way out. */
// FUNCTION: EZ2AC_UNPACKED 0x004017a0
int Obj401780::m4017a0()
{
    DrawGroup *group;
    unsigned int total;
    char *p;

    total = 0;

    for (group = f10; group != 0; group = group->nextOwned)
    {
        total += group->objectSize * group->count;
    }

    f28 = (char *)VirtualAlloc(0, total, MEM_COMMIT, PAGE_READWRITE);

    if (f28 == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487570
        Log("AllocateClass Error");
        return 0;
    }

    memset(f28, 0, total);
    f2c = f28 + total;

    p = f28;

    for (group = f10; group != 0; group = group->nextOwned)
    {
        group->poolBase = p;
        p += group->objectSize * group->count;
        group->live = 0;
        group->nodes = 0;
        group->last = 0;
        group->poolEnd = p;
    }

    // STRING: EZ2AC_UNPACKED 0x00487548
    Log("ObjPoolSize = %d Bytes...[%p <=> %p]", total, f28, f2c);
    return 1;
}

/* ---------------------------------------------------------------------------
   The rest of the screens' per-screen slots. The original NAMES these three slots
   in its own log lines: slot 4 is OnRegisterClass, slot 5 is OnCreateGame and
   slot 6 is OnDestroyGame - which is also what named SongSelectScreen,
   DrawModeIntro and ModeSelectScreen (their tags read "SongSelect::",
   "DrawModeIntro::" and "ModeSelect::").
   --------------------------------------------------------------------------- */

// FUNCTION: EZ2AC_UNPACKED 0x0041f8c0
void Screen48D2F4::s4()
{
    addInfo(&g_info_4a9040);
    addInfo(&g_info_4a90b8);
    addInfo(&g_info_4a8f1c);
    addInfo(&g_info_4a9130);
    addInfo(&g_info_4a907c);
    addInfo(&g_info_4a8f94);
    addInfo(&g_info_4ae118);
    addInfo(&g_info_4a9004);
    addInfo(&g_info_4a8ee0);
    addInfo(&g_info_4a8f58);
    addInfo(&g_info_4ae154);
    addInfo(&g_info_4a90f4);
    addInfo(&g_info_4ae190);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046be80
void Screen48D460::s4()
{
    addInfo(&g_info_4aeb88);
    addInfo(&g_info_4a90b8);
    addInfo(&g_info_4aeb4c);
    addInfo(&g_info_4ae190);
    addInfo(&g_info_4aec00);
    addInfo(&g_info_4a8f58);
    addInfo(&g_info_4a8f94);
    addInfo(&g_info_4a9004);
    addInfo(&g_info_4ae118);
    addInfo(&g_info_4ae154);
}

/* The GF game screen's OnRegisterClass. */
// FUNCTION: EZ2AC_UNPACKED 0x0045cfd0
void Screen48D47C::s4()
{
    addInfo(&g_info_4ae69c);
    addInfo(&g_info_4a90b8);
    addInfo(&g_info_4ae660);
    addInfo(&g_info_4ae190);
    addInfo(&g_info_4ae6d8);
    addInfo(&g_info_4a8f94);
    addInfo(&g_info_4a9004);
    addInfo(&g_info_4a8f58);
    addInfo(&g_info_4ae118);
    addInfo(&g_info_4ae154);
}

// FUNCTION: EZ2AC_UNPACKED 0x00466e80
void Screen48D498::s4()
{
    addInfo(&g_info_4ae794);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x00430a80
void SongSelectScreen::s4()
{
    // STRING: EZ2AC_UNPACKED 0x00491408
    Log("SongSelect::OnRegisterClass");
    addInfo(&g_info_4aaee8);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x00469fd0
void DrawModeIntro::s4()
{
    // STRING: EZ2AC_UNPACKED 0x0049c9b0
    Log("DrawModeIntro::OnRegisterClass");
    addInfo(&g_info_4aea2c);
    addInfo(&g_info_4ae118);
}

// FUNCTION: EZ2AC_UNPACKED 0x00448eb0
void ModeSelectScreen::s4()
{
    // STRING: EZ2AC_UNPACKED 0x00493da0
    Log("ModeSelect::OnRegisterClass");
    addInfo(&g_info_4ac758);
    addInfo(&g_info_4ae118);
}

/* These three screens override slot 6 to log their own tag first and then repeat
   the shared texture report verbatim. */
// FUNCTION: EZ2AC_UNPACKED 0x00430af0
void SongSelectScreen::s6()
{
    // STRING: EZ2AC_UNPACKED 0x00491440
    Log("SongSelect::OnDestroyGame");
    Log("Texture Report - Cnt=%-4d, Surfaces=%-4d, %-9d Bytes", ((KGraphics *)g_sub1b2f054)->m_texCount, ((KGraphics *)g_sub1b2f054)->countSurfaces(), ((KGraphics *)g_sub1b2f054)->m_texBytes);
}

/* These three screens override slot 6 to log their own tag first and then repeat
   the shared texture report verbatim. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a030
void DrawModeIntro::s6()
{
    // STRING: EZ2AC_UNPACKED 0x0049c9ec
    Log("DrawModeIntro::OnDestroyGame");
    Log("Texture Report - Cnt=%-4d, Surfaces=%-4d, %-9d Bytes", ((KGraphics *)g_sub1b2f054)->m_texCount, ((KGraphics *)g_sub1b2f054)->countSurfaces(), ((KGraphics *)g_sub1b2f054)->m_texBytes);
}

/* These three screens override slot 6 to log their own tag first and then repeat
   the shared texture report verbatim. */
// FUNCTION: EZ2AC_UNPACKED 0x00448f10
void ModeSelectScreen::s6()
{
    // STRING: EZ2AC_UNPACKED 0x00493dd8
    Log("ModeSelect::OnDestroyGame");
    Log("Texture Report - Cnt=%-4d, Surfaces=%-4d, %-9d Bytes", ((KGraphics *)g_sub1b2f054)->m_texCount, ((KGraphics *)g_sub1b2f054)->countSurfaces(), ((KGraphics *)g_sub1b2f054)->m_texBytes);
}

/* The OnCreateGame slots. Every one of them brings the credits overlay up at
   (sentinel-x, 435) exactly as Screen48D348::s5 does; what differs is which
   director is created first and what the screen hands off to afterwards. */

// FUNCTION: EZ2AC_UNPACKED 0x0044d920
void Screen48D32C::s5()
{
    createGameOverDirector();
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
}

// FUNCTION: EZ2AC_UNPACKED 0x00455b50
void Screen48D364::s5()
{
    createEndingDirector();
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
}

// FUNCTION: EZ2AC_UNPACKED 0x00466ea0
void Screen48D498::s5()
{
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
    createGFResultDirector();
}

/* The result screen picks its handoff by mode: the four radio modes with the
   0x1b2e7c4 flag set go one way, everything else the other. Both are tail calls. */
// FUNCTION: EZ2AC_UNPACKED 0x00450620
void Screen48D380::s5()
{
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);

    if ((g_modeIndex == 6 || g_modeIndex == 7 || g_modeIndex == 8 || g_modeIndex == 9) &&
        g_1b2e7c4 >= 1)
    {
        createTotalResultDirector();
    }
    else
    {
        createResultDirector();
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00430ab0
void SongSelectScreen::s5()
{
    SongSelectDirectorObj *director;
    ShowCredit *credit;

    // STRING: EZ2AC_UNPACKED 0x00491424
    Log("SongSelect::OnCreateGame");
    director = createSongSelectDirector();
    credit = createShowCredit();
    director->credit = credit;
    credit->setPos(-1.0f, 435.0f);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046a000
void DrawModeIntro::s5()
{
    // STRING: EZ2AC_UNPACKED 0x0049c9d0
    Log("DrawModeIntro::OnCreateGame");
    createDrawModeIntroDirector();
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
}

// FUNCTION: EZ2AC_UNPACKED 0x00448ee0
void ModeSelectScreen::s5()
{
    // STRING: EZ2AC_UNPACKED 0x00493dbc
    Log("ModeSelect::OnCreateGame");
    createModeSelectDirector();
    /* Named, so the call runs before the setPos pushes as it does in the
       original (a call feeding `this` is otherwise evaluated last). */
    ShowCredit *credit = createShowCredit();
    credit->setPos(-1.0f, 435.0f);
}

/* The two game screens' OnCreateGame: make the director and start it. The start
   is a tail `jmp` with the factory's result straight from eax into ecx. */
// FUNCTION: EZ2AC_UNPACKED 0x0041f960
void Screen48D2F4::s5()
{
    createMainGameDirector()->start();
}

// FUNCTION: EZ2AC_UNPACKED 0x0046bf00
void Screen48D460::s5()
{
    createCatchDirector()->start();
}

// FUNCTION: EZ2AC_UNPACKED 0x0045d050
void Screen48D47C::s5()
{
    createGFDirector()->start();
}

/* The four screens' director factories: publish the record, allocate one object
   out of its pool and construct it - the engine's standard new-plus-ctor shape,
   with the EH state the allocation needs so a throwing ctor gives the memory
   back. Each screen's OnCreateGame is a bare tail `jmp` into its own. */

// FUNCTION: EZ2AC_UNPACKED 0x0045c3f0
WarningDirector *createWarningDirector()
{
    g_objectRecord = &g_info_4ae4d0;
    return new WarningDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00476830
TestModeDirector *createTestModeDirector()
{
    g_objectRecord = &g_info_4afb70;
    return new TestModeDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00469bc0
WarningShortDirector *createWarningShortDirector()
{
    g_objectRecord = &g_info_4ae974;
    return new WarningShortDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ab20
Obj46AA80 *createObj46AA80()
{
    g_objectRecord = &g_info_4aea68;
    return new Obj46AA80;
}

/* The scratch panel's factory - the last of the engine's pooled ones. Its
   constructor @0x462870 is still parked in wip/gfpanel-ctor.cpp, but a
   factory only needs the call, and the `push` audits the class either way. */
// FUNCTION: EZ2AC_UNPACKED 0x00465320
GFPanel *createGFPanel()
{
    g_objectRecord = &g_info_4ae6d8;
    return new GFPanel;
}

/* The audience-rate overlay. Its constructor @0x41aa30 is in src/panels.cpp,
   so cl cannot see it from here. */
// FUNCTION: EZ2AC_UNPACKED 0x0041ad50
AudienceRatePanel *createAudienceRatePanel()
{
    g_objectRecord = &g_info_4a8ee0;
    return new AudienceRatePanel;
}

/* The GF (ScratchMix) game's director. Its constructor @0x460110 lives in
   src/maingame.cpp, so cl cannot see it from here either. */
// FUNCTION: EZ2AC_UNPACKED 0x00460740
GFMainGameDirector *createGFDirector()
{
    g_objectRecord = &g_info_4ae69c;
    return new GFMainGameDirector;
}

/* The main game's own song-select overlay, built the same way. Its constructor
   lives in src/directors.cpp so cl cannot see it from here. */
// FUNCTION: EZ2AC_UNPACKED 0x00426310
InGameSongSelect *createInGameSongSelect()
{
    g_objectRecord = &g_info_4a907c;
    return new InGameSongSelect;
}

/* The three remaining pooled factories, all built to the same four lines as
   the ones above. Each `push` is the class's own size and audits the layout:
   0x1664 for TitleDirector, 0x3118 for ResultDirector, 0x1438 for
   TotalResultDirector - and all three agree with the modelled structs to the
   byte. Their constructors live in src/stubs.cpp, src/resultctor.cpp and
   src/totalresultctor.cpp respectively, so cl cannot see any of them from
   here and the calls survive. */

// FUNCTION: EZ2AC_UNPACKED 0x0044f400
TitleDirector *createTitleDirector()
{
    g_objectRecord = &g_info_4adfe8;
    return new TitleDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00454400
ResultDirector *createResultDirector()
{
    g_objectRecord = &g_info_4ae1cc;
    return new ResultDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00455ad0
TotalResultDirector *createTotalResultDirector()
{
    g_objectRecord = &g_info_4ae2c0;
    return new TotalResultDirector;
}

/* The scratch result screen. Its constructor @0x4682e0 is unreconstructed and
   stubbed in src/stubs.cpp, which is another TU, so the call survives; the
   0x2dd0 it pushes is the modelled struct's size to the byte. */
// FUNCTION: EZ2AC_UNPACKED 0x00469970
GFResultDirector *createGFResultDirector()
{
    g_objectRecord = &g_info_4ae794;
    return new GFResultDirector;
}
