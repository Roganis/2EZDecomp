#include "kcrt.h"
#include "loader.h"
#include "ksong.h"
#include "ksound.h"
#include "ezchart.h"
#include "kbase.h"
#include "kprofile.h"
#include "kparser.h"
#include "ksongplayer.h"
#include "sound.h"
#include "ez2acapp.h"
#include "bigbase.h"
#include "ezscreen.h"
#include "obj402860.h"
#include "panels.h"
#include "songselect.h"
#include "testinput.h"
#include "systemmenu.h"
#include "modeselect.h"
#include "backgrounds.h"
#include "directors.h"
#include "gamepanels.h"
#include "obj405f80.h"
#include "obj40d4e0.h"
#include "obj40eac0.h"
#include "judge.h"
#include "scorekeeper.h"
#include "gfxtext.h"
#include "loaders.h"
#include "sink-view.h"

struct ListNode;
ScrFader *m450570();
void m450420(void *self, int a, int b, struct CatchSink1 *s);

/* Definitions of not-yet-matched callees, kept in this separate TU so the
   matched callers that call them cannot inline/optimize them away (which would
   change the caller's codegen). reccmp maps the address via a STUB marker and
   does not compare this body. */


/* Declared here as well as in src/kezplayer.cpp, where its one caller lives -
   the model is local to that file. */

/* The loader Obj40F7B0::fill @0x40f7b0 calls twice - a dispatcher on the
   object's own +0x50 kind. */
/* The DirectDraw enumeration callback KGraphics::enumerateDevices @0x4054d0
   hands to DirectDrawEnumerateExA. */

/* enumDevice @0x405310 and its three callbacks (@0x404f70/@0x404fb0/
   @0x405060) are reconstructed in src/kgraphics.cpp. */

/* m406050, m405680 and m405b60 - the whole surface bring-up chain - are
   reconstructed in src/kgraphics.cpp. */




/* Obj40C180::shutdown @0x40cc80 is a REAL match in src/obj40c180.cpp; the
   resetFlags stub it needs is there too, because it has to be VISIBLE to it. */

/* The tail-jump target of Obj40C180::shutdown. Must stay in a different TU from
   it: visible here, cl would inline it into the tail position and the original's
   `jmp` would disappear. */
/* Two of init's bring-up callees. These must stay OUT of its TU: the original
   reloads ecx after each of them (both really do clobber it), which is what cl
   emits when it cannot see the body. */
/* The screen base's shared virtuals (slots 1/2/3/6 of every screen vftable) and
   its run method - all reached from EZ2AC::runGame @0x417e90. */

/* The first of the 13 mode runners, PARKED in wip/runclubmix.cpp - structurally
   complete, blocked on a 4-register permutation. FOR A PORT IT IS DONE. */

/* The screen driver, PARKED at 88.50% in wip/screenrun.cpp (a stack-slot
   permutation of two volatile locals), plus its two non-virtual callees. */

/* loadTexture itself is reconstructed but parked at 99.07% on the position of a
   single store - see wip/loadtexture.cpp. */

/* What loadTexture @0x408d00 hands the file name and the payload to. */

/* The graphics calls the cross-fade @0x415200 drives. */


/* setFade @0x40b080's three callees. m4080c0 @0x4080c0 is reconstructed in
   src/kgraphics2.cpp - a TU of its own so setFade keeps its two real calls. */
/* m408130 @0x408130 is reconstructed in src/kgraphics2.cpp. */


/* The four graphics calls that bracket Obj401780::s1 @0x418620's draw walk, and
   the engine tick it opens with. All must stay out of s1's TU. beginFrame
   @0x40a770 is reconstructed in src/kgraphics.cpp, which is such a TU. */

/* Obj40C180::tick @0x40cc90's callee m40c4b0 is reconstructed in
   src/analogread.cpp (promoted from wip at its parked 42.9%). */


/* The three graphics teardown calls and the sound-owner call s3 makes on its way
   out. All must stay out of s3's TU. */
/* freeNodes @0x403f20 is reconstructed in src/kprofile.cpp - which is a
   different TU from its caller m403f60, as it has to be (m403f60 reloads ecx
   after the call, so cl must not be able to see the body). */

/* What endFrame @0x405e30 tail-jumps once the scene is closed, m405d80
   @0x405d80, is reconstructed in src/kgraphics2.cpp. */

/* freeNodes408b40 @0x408b40, m408c80 @0x408c80's callee, is reconstructed in
   src/kprofile.cpp - another TU from its caller, like freeNodes. */

/* The backend entry points the two dispatchers reach, and the subsystem's
   own case-0 hand-off. All of them must stay out of src/ezsubsystem.cpp: with
   any of them visible cl inlines it and the original's tail `jmp` disappears.
   m40dd10 @0x40dd10 is reconstructed in src/subsystembase.cpp - another TU,
   so its caller's jmp survives. */


/* The Obj4766E0 placeholder was TestModeDirector; its constructor
   @0x4766e0 is reconstructed in src/systemmenu.cpp. */









/* The directors and handoffs the screens' OnCreateGame slots call. All must stay
   in this TU, out of src/ezscreens.cpp. */
/* ResultDirector's constructor helpers (src/resultctor.cpp). */
/* The sample pick m42f080 stamps into a claimed hold note. */
/* pickSample42de50 is reconstructed in src/picksample.cpp. */

/* The per-stage score sums (src/resultctor.cpp's per-player loop). */
/* The eight further screens' per-screen vftable slots. Only Screen48D3F0's two
   are reconstructed (src/ezscreens.cpp); the rest are address maps so the
   vtables compare equal. All must stay out of the TUs that construct them. */

/* Screen48D364's two per-screen slots, and the player-record lookup runEnding
   @0x415db0 calls seven times (it walks the list at 0x1b2ef6c comparing each
   node's id at +8). All three must stay out of runEnding's TU. */
/* The three per-player resets the list joiners run before linking (each stores
   the id at +8, clears a scatter of fields and tail-calls its class's own
   initializer). All must stay out of src/nodelists.cpp. */
/* What PlayerSlot::reset @0x42efb0 finishes with. */

/* What PlayerRecord::reset @0x466690 finishes with - the method right after the
   class's constructor. */
/* reset itself is reconstructed but parked at 86.96% on a three-instruction
   scheduling difference - see wip/playerreset.cpp. */

/* The screens' slot 4 and 5 overrides live in src/ezscreens.cpp; the base's own
   s4/s5 have no original counterpart (no vftable holds them) so they stay
   unannotated. */

/* Obj401780::s4/s5 have no base definition of their own - only the overrides
   above are ever in a vftable - but cl needs a body for the base's slots. They
   are deliberately NOT annotated (nothing in the original corresponds). */
void Obj401780::s4()
{
}

void Obj401780::s5()
{
}

/* The engine-wide per-round helpers runGame calls, and the check that gates its
   mode dispatch. (0x4145f0 is not one of them: it is WINMM's timeGetTime import
   thunk — see src/ez2acapp.cpp; 0x418e60 and 0x418ee0 are REAL matches in
   src/nodelists.cpp, where the list walkers they inline are defined.) */

/* A fourth `folded` alias on the shared bare `ret` @0x402880: the zero-argument
   compiled-out routine engineTick418ee0 calls (no arguments are pushed and no
   `this` is loaded, so it is neither Log nor one of the folded members). The
   alias keeps src/applog.cpp's Log the winner of the address, so the call
   symbolizes identically on both sides. Must stay in this TU - visible to its
   caller, cl would inline the empty body and delete the call. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void hook402880()
{
}
/* The 13 mode runners (g_modeIndex -> kModeNames). 5KeyMix and 7StreetMix share
   one address: their bodies must have been identical, so the original's linker
   ICF-folded them. They have to stay two distinct functions here, because that
   is what keeps runGame's two switch arms from being cross-jumped into one. */
/* Both reconstructed in src/moderunner.cpp. */



/* ClubMix is reconstructed but PARKED (wip/runclubmix.cpp): structurally complete,
   four registers permuted. */

/* runSpaceMix is reconstructed in src/runspacemix.cpp. */







/* Callees of EZ2AC::v4 @0x4181b0 (the app's main loop): the settings reader and
   the three entry points it picks between. */

/* readSettings @0x415a40 is reconstructed in src/settings.cpp, and the two
   readers it finishes with are in src/ez2acapp.cpp - a TU it does not share. */

/* EZ2AC vtable slots 2-4 (init / shutdown / main loop), called by runApp
   @0x4015e0 through the vtable. Declared virtual in include/ez2acapp.h, so they
   have to be DEFINED for cl to lay the vftable down; these stubs map their
   addresses until they are reconstructed. */

/* Log's stub @0x402880 moved to src/applog.cpp: that address is ICF-folded and
   the annotation that comes FIRST in (case-insensitively sorted) file order
   wins it, with the rest aliased onto it — so Log has to live in a file that
   sorts ahead of the `folded` virtuals in src/kengineapp.cpp. */



/* Callee of KEngineApp::loadProfiles @0x401240: the profile-file record loader.
   Fully reconstructed but PARKED (wip/kprofile-load.cpp) — the frame-alignment
   wall. */

/* THE SONG DATABASE LOADER - fully decoded 2026-08-26, not yet written.
   It is `SongTable::loadBinFile(const char *dir, const char *name)`: a MEMBER,
   not the __stdcall free function this stub declares. Its only caller is
   loadSongBin @0x469fb0, which forwards ecx from ITS caller, and that caller
   loads `mov ecx,0x3780958` - g_songTable. So both are members, which settles
   two of scratchpad/ecxsweep.py's open candidates at once.

   THE PATH is sprintf("system\\%s\\%s", dir, name), the file is read whole
   into a VirtualAlloc'd buffer (MEM_COMMIT / PAGE_READWRITE), and the buffer is
   VirtualFree'd at the end - so nothing of the raw file survives the call.

   THE .bin CIPHER, which is the part worth having written down. Two 32-byte
   key tables, key1 @0x4ae9fc and key2 @0x4ae9dc, and for every byte i:

       buf[i] ^= key1[(i + 1) & 31] ^ (char)i;
       for (j = 0; j < 32; j++) {
           int k = j ? j : 12;                 // the divisor is 12 when j == 0
           buf[i] ^= key2[i %% k] ^ key2[j] ^ key1[j];
       }

   cl unrolls the inner loop by four, which is why the listing shows the same
   three-XOR step at 0x4ae9fc/fd/fe/fb. The `j ? j : 12` guard is a real
   division-by-zero dodge in the original, not an artefact.

   THE FORMAT, once deciphered:

       +0x06  u16 count          -> g_songTable.count
       +0x08  u32 offset         -> `count` records of 0x56, copied straight
                                    into g_songTable.entries
       +0x0c  u32 offset         -> 0x2f GROUPS, each `u16 n` followed by n
                                    records of 0x10 bytes

   The group counts land at SongTable+0x927c (0x2f ints, ending at 0x9338) and
   the group records at SongTable+0x9338, 0x1b40 bytes apart - so 0x2f * 0x1b40
   is about 325 KB the struct does not yet model, and extending it is a .data
   move of the kind CLAUDE.md warns about. That layout change is why this is
   banked rather than written: it wants its own pass with a full match, not the
   tail of one.

   __stdcall with two args (`ret 8`) as declared, and an EH function in its own
   right. */

/* Callee of KParser::readFile @0x413bc0: decrypts an .ini to a .int temp
   file. PARKED (wip/slurpdecrypt.cpp) — same be-spill cipher wall as
   DecryptFile. */

/* Callee of KProfile::Load @0x413fe0: the tokenizer. PARKED
   (wip/parsetoken.cpp) - per-iteration cursor reload + have-flag spill. */

/* KBase's virtual dtor @0x426e40: a bare `jmp 0x402880`. Not a linker thunk
   (as this was long parked as): it is a tail call of kbaseRelease(), an empty
   member the original's linker folded onto the shared `ret`. */
void kbaseRelease();   /* 0x402880 folded (src/constants.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00426e40
KBase::~KBase()
{
    kbaseRelease();
}

/* Callees of EzChart::load @0x411720 (src/ezload.cpp). The decryptors are
   fully reconstructed but PARKED (wip/crypt.cpp, wip/crypt2.cpp - single
   register-allocation quirks); loadEZ @0x411460 is a wave-3 target. */




/* Callee of EzChart::copyLane/copyLanes (extractLane @0x412690 is a REAL
   match in src/seq.cpp). mergeTracks @0x411310 is fully reconstructed but
   PARKED (wip/mergetracks.cpp) - a single register-pair allocation swap. */


/* Callee of EzChart::setSound @0x410970 (findSoundObject @0x40f420 is a REAL
   match — it lives in src/constants.cpp so the caller cannot inline it). */



/* loadSample @0x40e590 is decoded but parked on register allocation - see
   wip/loadsample.cpp. Both it and the resolver it calls are stubbed here. */


/* Panel::init @0x42a0c0 - attach the keys panel to its slot (the catch
   twin @0x471710 is matched). */


/* The judge's release and press verdicts - both five-argument. */
/* THE GRADING CORE - decoded in wip/judge-grade.cpp (84%, one register). */

/* The panel family's object base. Its destructor is the ICF-folded bare `ret`
   @0x402880 (owned by the Log annotation in src/applog.cpp), reached DIRECTLY
   rather than through KBase's thunk @0x426e40 - see include/effector.h. Both
   this and operator delete have to stay out of the derived classes' TUs or cl
   inlines them and the calls disappear. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
EzObject::~EzObject()
{
}


/* Effector6thStyle's two update slots - address-mapped so its vftable @0x48eb30
   compares. update @0x41d920 is what calls the matched updateSpeedDigits
   @0x41d1d0; update2 @0x41e620 is what computes m_scrollBpm. */


/* BattleMode::update is reconstructed in src/battleupdate.cpp. */

/* What every .str-backed chart is built from: the loader that turns a resource
   name into a cell table, and the setter that hands the table to the record. */

/* The panel base's constructor. Empty and novtable, so it compiles to
   `mov eax,ecx; ret` and the linker ICF-folded it onto the same address as
   KBase's (src/constants.cpp, which owns it). Out of the derived classes' TUs
   or cl inlines the call away. */
// FUNCTION: EZ2AC_UNPACKED 0x00401690 folded
EzObject::EzObject()
{
}

/* The projection half of the camera pair SongSelectDirector's destructor
   resets with. __thiscall on the graphics singleton @0x1b2f054, pure float
   math (no calls), stubbed until it is worth reconstructing on its own.
   Its sibling setCamera @0x40bd40 is a real body in src/setcamera.cpp
   (integrated at 16.41%, parked on the stack packer - the analysis is in
   that file's header). */


/* Something inputRadioMix runs every tick while the legacy flag g_1b2e7e0
   is set. */

/* update's per-page draw helpers (m439f00 is real now,
   src/songselectpage.cpp). */
/* loadSprite @0x408e80 is reconstructed in src/kgraphics2.cpp. */

/* TestInput's two unreconstructed vtable slots. Slot 4 @0x4753e0 is the page's
   per-frame poll: it reads several inputs off the engine object @0x1b2e710 and
   drives the cabinet lamps from them. */
/* drawItem @0x475a90 is reconstructed in src/testinput.cpp. */


/* ModeSelectDirector's two unreconstructed vtable slots. */

/* The background family's shared update/update2. BackGround's and
   GFBackGround's are the SAME two addresses - the original's linker ICF-folded
   them - so all four annotations are `folded`; CatchBackGround's differ. */
/* All three reconstructed in src/bgupdate.cpp. */

/* Three more cabinet-state setters on the engine object, always called with 0
   next to setFlags when a screen is torn down. */
/* The three directors' unreconstructed update/update2 slots. */

/* RankingDirector's two private draw helpers, driven by update() @0x45a290
   (src/rankingupdate.cpp). */

/* The ranking screen's tick - a seven-arm state machine on +0x10 over a SCORE
   COUNT-UP ANIMATION, and denser than its 0.106 calls/insn suggests.

   The animation is the part worth writing down. Two counters at +0x109b58 ramp
   toward two targets at +0x109b60 (all four FLOAT - the header is corrected),
   and each step compares the counter's displayed value before and after to
   decide whether to tick the sound: the displayed value is
   `(int)counter / 46`, which the original spells as the magic multiply
   0xb21642c9 with `add edx,ecx` and `sar 5`. Forty-six is not a round number
   and was confirmed against the Hacker's Delight reconstruction rather than
   guessed. Above the counter the ramp is `target * K`, eased by a factor when
   it overshoots and by a different one when it does not.

   The mode ladder (6, 7, 8, 9 and 11 all special) picks countListA against
   countListC for the two-player test, and the tail's pan ramp is the same
   `(a - 0xff) * 10000 / 300` the title tick uses.

   FIELDS ARE CARVED (2026-08-25) - twelve of them, and the four counters
   retyped to float. What is left is the body, and it wants a sitting: the
   float easing and the seven arms are the cost, not the 39 calls. */


/* What the keys result tick @0x451550 runs before its phase switch. */






/* TestSound's remaining slots. Slot 3 is the shared folded empty `ret`. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void TestSound::update()
{
}






/* TestBookkeeping's remaining slots, the same three shared leaves plus its own
   sink override (what == 5 is "clear": it zeroes the page's snapshot AND the
   three persistent counters) and its menu builder. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void TestBookkeeping::update()
{
}

/* Slot 4 of ScrFader's vftable @0x495400 is the same folded empty. */
// FUNCTION: EZ2AC_UNPACKED 0x00475dc0 folded
void ScrFader::v4(int /*a*/, int /*b*/)
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00475dc0 folded
void TestBookkeeping::update2(int /*a*/, int /*b*/)
{
}







/* TestLighting's remaining slots. Slot 3 is the shared folded empty `ret` and
   slot 4 the equally shared `ret 8`; the sink override reads the page's menu
   through its OWN `this` (page+0x14 = sink+4). */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void TestLighting::update()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00475dc0 folded
void TestLighting::update2(int /*a*/, int /*b*/)
{
}





/* SystemMenu's two unreconstructed vtable slots: the per-frame draw of the
   page list and the per-frame poll that moves the cursor through it. */




/* TestGameOption's sink override, PARKED at 78.15% on a register rotation -
   wip/testgameoption-onmenu.cpp has the full reconstruction. */

/* TestVideo's one unreconstructed vtable slot: the per-frame draw, parked at
   96.43% on one register - wip/testvideo-update.cpp. */

/* What each main-game director's slot entries hold; released by the pass in
   src/maingameslots.cpp. */
/* The GF slot object is a PlayerRecord; its stage-end fold @0x465c70 is
   reconstructed as PlayerRecord::m465c70 in src/gfstagesetup.cpp. */

/* The main game's .gds loader, PARKED at 53.69% on a frame-layout gap - see
   wip/maingame-gds.cpp, which has the full section decode. */

/* Its five section handlers. */

/* The audience-rate panel's three draw variants, tail-jumped to from its
   update @0x41a910 - they must stay out of that TU or the jumps vanish. */
/* The profile tree writer, reached from KProfile::writeFile @0x4142b0. Must
   stay out of that TU or cl inlines it and the call disappears. */

/* The test pages' caption bar: a DirectDraw colour fill, fully decoded but
   PARKED at 58.54% on the rect stores - see wip/kgraphics-colorfill.cpp. */

/* drawTextf @0x4071d0 is reconstructed in src/kgraphics2.cpp. */



/* The three main-game directors' own methods, their nine secondary-interface
   overrides and their remaining vtable slots. Slot 3 is the shared folded
   empty `ret` in all three. */

// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void EZ2DJMainGameDirector::update()
{
}

/* THE MAIN GAMEPLAY TICK, and NOT the shape its GF and Catch siblings have -
   they are 193 and 137 instructions and both matched; this one is 500, with 68
   calls. What it adds is a LAMP-ROUTING LADDER: the mode index picks which of
   Obj40C180::m4189e0 / m418b90 / routeOutputs / m418a50 each present player's
   lamps go through, with the argument being `g_1b2ef78 == 0` at every one of
   the eight sites, and mode 12 reaching it through a _stricmp against
   g_1b2e5ac. Modes 4, 5, 8 and 9 share one arm, mode 0 another.

   Ahead of that: the pending-screen gate and one-shot start at +0x368/+0x36c
   (the twin of the GF tick's +0x32c/+0x330), and an autoplay block gated on
   g_1b2ec14 that walks 0x40 lanes calling notify slot 13 with 0x76 when the
   mix style is 7.

   FIELDS ARE DONE - the class was already modelled and only +0x7f0 needed
   carving; +0x4b0 and +0x674 are f380[76] and f380[189] inside the existing
   array, and [esi+0x10] is the secondary-base pointer idiom, not a field.
   Wants a sitting rather than a 25-minute tick: it is the ladder that costs,
   not the calls. */



// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void GFMainGameDirector::update()
{
}


// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void CatchMainGameDirector::update()
{
}

/* Reconstructed in src/directors.cpp. */

/* The catch commit and notify the lane-expiry sink drives. */

/* The three game panels' update/update2. */
/* The keys panel's draw helpers its update @0x42d5c0 still stubs. */







/* The keys panel's fade tick (update2 feeds it the frame's delta). */

/* The keys panel's swaying readout - wip/smallleaves.cpp, 86.36%. */
/* And the scratch panel's. */






/* The scratch panel's swaying readout - reconstructed at 98.44% in
   wip/smallleaves.cpp, stubbed here for update2's call. */
/* The catch panel's draw helpers its update @0x4724f0 calls in order. */








/* EndingDirector / InGameSongSelect / TestModeDirector's remaining slots. */

/* The GF director's start(), the third of the family (EZ2DJ's @0x41fa10 and
   Catch's @0x46bfa0 are matched in src/catchdirector.cpp). It has to stay out
   of src/ezscreens.cpp so Screen48D47C::s5's tail `jmp` survives. */


/* ShowCredit / TotalResultDirector / DrawModeIntroDirector's remaining slots. */
/* EzChart's one unreconstructed vtable slot, so the vftable @0x488a10 has
   every slot address-mapped. */

/* The video-blit setter DrawModeIntroDirector's constructor hands its blank
   frame to - reconstructed in src/kgraphics2.cpp. */

/* Obj40B2E0's constructor and the four methods ModeSelectDirector's ctor drives
   it with (load a .spv, arm it, stamp the time, and a two-argument setter). */
/* Find a texture by name (or key) in the graphics' list, else create a
   256x256 one under that name; single caller, the video player's load. */
/* findOrCreateTexture @0x409200 is reconstructed in src/loadtexture.cpp. */

/* The graphics call the mode-select teardown makes with -1. */
/* CatchMainGameDirector's own methods and the eight per-note handlers its
   constructor's mode ladder drives. */






/* GFMainGameDirector's own methods and its four ladder handlers. */









/* The two director hooks the third sink family drives. */



/* Obj405F80's two unreconstructed slots, its release sweep, and the free
   table-walker its destructor calls after it. */
/* v2 @0x406170, the graphics bring-up, is reconstructed in
   src/obj405f80.cpp. */



/* EzSubsystem's constructor and its backend's teardown sweep, plus
   Obj40EAC0's six unreconstructed slots. */
/* All four reconstructed in src/ksoundds.cpp: the EzSubsystem constructor
   @0x40e670, the DirectSound bring-up @0x40e7f0, and the two sample
   factories @0x40e910 / @0x40eb80. */


/* The mode name the song-table lookup is keyed by, and the two .gds sections
   loadGds @0x423790 hands off whole. */
/* The song table: 0x56-byte entries with the count at +0x9278, searched by
   name. Only the search is reached from here. */

/* GFMainGameDirector's mode-name getter and its GaugeUpDownRate section. */
/* CatchMainGameDirector's mode-name getter and its GaugeUpDownRate section. */
/* Judge's only non-trivial vtable slot: a two-argument forwarder onto
   @0x426ab0, the class's real work function. */

/* TestGameOption's remaining slots, the same three shared leaves plus its own
   sink override and menu builder. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
void TestGameOption::update()
{
}

// FUNCTION: EZ2AC_UNPACKED 0x00475dc0 folded
void TestGameOption::update2(int /*a*/, int /*b*/)
{
}





/* CatchPanel::LoadPVI's remaining section loaders (src/catchpanelgds.cpp).
   Each reads one .gds section through the parser it is handed. */

/* The .txg atlas loader - reconstructed in wip/loadtxg.cpp, parked on a
   register choice, so it is a stub here for m460ac0's sake. */

/* GFPanel::LoadPVI's remaining section loaders (src/gfpanelgds.cpp). */

/* Panel::LoadPVI's remaining section loaders (src/panelgds.cpp). */


/* CatchMainGameDirector::m46ca30's three section loaders. */



/* GFMainGameDirector::m45e190's three section loaders. */

/* The two "General" .gds handlers are matched in src/maingame.cpp. */


/* The kind-4 draws of the background draw list; the kind-5 pair is matched in
   src/backgrounds.cpp. */

/* The audio loaders' intermediate base destructor: empty, so the original's
   linker folded it onto the shared `ret` @0x402880 that src/applog.cpp's Log
   owns. It has to live here rather than in src/loaders.cpp - visible to the
   two derived destructors, cl would inline the empty body and their tail
   `jmp` would disappear. */
// FUNCTION: EZ2AC_UNPACKED 0x00402880 folded
Obj40DA10::~Obj40DA10()
{
}

/* The two loaders' seek, PARKED at 71.43% - see wip/loaders-seek.cpp. Decoded
   and correct; cl 13.10 places a switch's default arm before the cases and the
   original places it after them, and no arm ordering moves it. */


/* m40fbf0 @0x40fbf0, the channel rebind, is reconstructed in src/sound.cpp;
   the sample lookup it calls stays a stub (and must stay OUT of that TU: the
   rebind reuses the incoming ecx across the call). */
/* m40f650, the round-robin sample pick, is reconstructed in
   src/kezplayertable.cpp. */



/* m421510 @0x421510 is reconstructed in src/maingameslots.cpp - it has to
   live beside its callers so cl can see its clobber set. */







/* The scratch panel's timer-expiry report - GFPanel::m4626d0 @0x4626d0. */


/* The director's "this run is over" notifier. Every call site in the sink
   @0x42e620 reaches it as `mov ecx,[slot+0x3d0]; push slot; call` - a thiscall
   on the director taking the slot that raised it.
   Reconstructed in src/ez2djfinish.cpp. */

/* The scratch director's game-over raise, the loose twin of m420570.
   Reconstructed in src/ez2djfinish.cpp. */


/* The keys modes' grade ladder, called by the stage teardown @0x42e360 to
   grade the course total. Decoded in wip/keysrank.cpp and parked one
   cross-jump direction short of a match - see that file. */



/* One of the two the background script walker @0x41b370 dispatches to. */
/* The catch twins of the two the background script walker dispatches to. */

/* What each background's update2 runs while its alternate draw is off. */


/* m40b9f0 @0x40b9f0, the event pump, is reconstructed in src/videoplayer.cpp. */


/* GFResultDirector's four private helpers, driven by update2 @0x467b90. */


/* Two of CatchMainGameDirector's private helpers, driven by update2. */


/* What update2 advances once the overlay's own tick has been taken -
   reconstructed in src/titlector.cpp. */



/* PlayerRecord's judged-hit commit - PlayerSlot::m42fd90 @0x42fd90's GF twin,
   which handleEvent @0x466da0 ends in. */

/* What BackGround::m41b900 @0x41b900 hands the parsed script to - returning 0
   makes the load fail. */

/* What CatchBackGround::m46bc30 @0x46bc30 hands the parsed script to - the
   catch twin of BackGround::m41b680. */

/* The four the two background script pumps @0x45c6a0 / @0x46ae40 drive: one
   retires the record that just expired, the other arms the next. GFBackGround
   reaches the first through BackGround's copy, as it does the parser hook. */




/* The reject path of the .scr parser @0x41b680 - what it returns when the
   buffer does not begin "SCR0". */

/* The catch parser's reject path @0x46b9b0's twin of BackGround::m41b0c0. */

/* What KGraphics::loadStr @0x402de0 hands the raw .str payload to - it turns
   the file into the cell table the clip's header points at. */
