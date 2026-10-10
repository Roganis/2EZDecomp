# EZ2DECOMP — private package overview

A **byte-matching decompilation** of the arcade rhythm game **EZ2AC**: reconstructed
C/C++ source that recompiles into a binary **byte-identical** to the original
executable, verified function-by-function with `reccmp` (the same method as the
`isle` LEGO Island decomp).

This document is the map for reading the package. It covers the shipping
executable's structure, why the decomp targets a decrypted image of it, the
repository layout, the build/verify loop, and a role for every source file.

> **Distribution note.** This package contains only reconstructed source we
> wrote, plus analysis (addresses, struct layouts, disassembly-derived notes).
> It contains **no bytes from the original executable, DLLs, or game assets** —
> those are gitignored and never leave the author's machine. Keep it that way:
> the project is shared privately precisely because the original is DRM-locked
> (see "Why this is private" below).

---

## 1. Status

* **Implemented: 100.00%** — every function reachable in the game's `.text` is
  reconstructed and annotated. Machine-checked by two tools in the package:
  `scratchpad/census.py` (0 referenced-but-unannotated functions) and
  `scratchpad/vtaudit.py` (0 pinned vtables with unannotated slots).
* **Accuracy ~96%** — 1,351 of 1,563 compared entities are byte-exact (100%).
  The remainder are documented compiler-codegen residues, not missing work; the
  full standard and the frozen "wall" families are in `docs/DONE-CRITERIA.md`.
* **Toolchain confirmed period-correct**: Microsoft Visual C++ 7.1
  (`cl.exe` 13.10.3077, `_MSC_VER == 1310`, VS.NET 2003), static CRT `/MT`,
  `/GS`, DirectDraw7 + Direct3D7 + DirectSound + DirectInput, built under Wine.

The full standard of "done" for this project — what counts as matched, how the
walls are recorded, the two audit tools — is `docs/DONE-CRITERIA.md`. Read it
first if you want to judge completeness.

---

## 2. The shipping executable's structure

The retail `EZ2AC.exe` (864 KB) is **not** a plain PE — it is a **multi-stage,
DRM-bound AES protector**. The decomp does **not** reconstruct the protector; it
targets the game code the protector unpacks. The layers, established by
reversing the loader:

1. **Stage-1 dropper (the tail stub).** The real PE entry point lands in a small
   stub, not in the game. It resolves `CreateFileA` / `WriteFile` /
   `GetTempPathA` / `WinExec` by walking `kernel32`'s export table, scans forward
   for an embedded `MZ`, writes a **15,872-byte stage-2 loader** to
   `%TEMP%\PWfxzl.exe`, and `WinExec`s it.

2. **Stage-2 loader (`PWfxzl.exe`).** Itself **ASPack-packed**, and its imports
   are pure licensing: `URLDownloadToFileA`, `OpenProcessToken`,
   `SHGetSpecialFolderPathA`, plus the protector's `setupapi.dll` / `hid.dll`
   (hardware probe). This stage performs the online / hardware license check.

3. **The payload cipher: AES-128-ECB.** The protector's `.lcsh` section carries a
   full AES engine — both the standard forward S-box and the **inverse** S-box
   are present. The game's `.text` is encrypted as a **16-byte-block ECB**
   transform (proven: a both-direction bijection with **0 conflicts** across
   31,983 distinct blocks). `.data` ships in the clear; `.rdata` differs from the
   decrypted image only in the import table, which the loader rebuilds at
   runtime.

4. **The key is external.** No AES key of any length is stored anywhere in the
   file. Combined with the stage-2 DRM imports, the key is **derived at runtime
   from the machine's hardware / online license**. Consequence: **the game
   `.text` cannot be decrypted offline** — the plaintext only exists after a
   licensed runtime decrypt.

### What the decomp actually matches against

Because offline decryption is impossible, the `reccmp` target must be a *runtime
decryption* of the genuine `EZ2AC.exe`. The image used is such a decryption. Its
fidelity to the genuine build is **cryptographically verifiable without the key**,
because AES-ECB is a fixed permutation: every 16-byte ciphertext block has exactly
one plaintext. Cross-checking the decrypted `.text` against the genuine
`EZ2AC.exe` ciphertext yields a **consistent AES-ECB bijection, 0 forward and 0
reverse conflicts**, with `.data` byte-identical and `.rdata` differing only in
the (runtime-rebuilt) imports. That is strong proof the target is a faithful,
untampered decryption of *this exact* build.

PE section layout (identical VAs in the packed file and the decrypted image):

| section | VA | role |
| --- | --- | --- |
| `.text`   | `0x401000` | game code — **AES-128-ECB encrypted** in the retail file |
| `.rdata`  | `0x487000` | read-only data + import tables (imports zeroed on disk, rebuilt at runtime) |
| `.data`   | `0x4a5000` | initialized data (ships in the clear) + BSS |
| `.lcsh`   | `0x33e3000`| the AES protector engine (S-box, inverse S-box, loader) |
| tail stub | `0x3800000`| the stage-1 dropper; the PE entry point |

### Why this is private

The genuine plaintext only exists behind a hardware/online license the author
does not hold a dongle for. A public release cannot ship a runnable or
byte-verifiable original, and the decrypted image is the licensee's to hold — so
the work is shared privately, source-only, among trusted people who can carry it
forward.

---

## 3. Repository layout

| path | contents |
| --- | --- |
| `src/*.cpp` | the reconstructed game — 247 translation units |
| `include/*.h` | the reconstructed types, classes and vtables — 44 headers |
| `harness/main.cpp` | a stub `main` so the reconstructed TUs link into a testable image |
| `docs/` | the standard, the matching cookbook, the port plan, format notes |
| `wip/` | decoded-but-not-yet-byte-matched functions and analysis notes (see `wip/README.md`) |
| `scratchpad/*.py *.sh` | the analysis & diffing tools (census, vtable audit, zoo compiler, twin finders) |
| `reccmp-patches/` | patches to `reccmp` this project needed, one concern each (see `docs/reccmp-patches.md`) |
| `match.sh` | the build + `reccmp` loop |
| `CLAUDE.md` | the project's own working rules and confirmed findings (read this to understand conventions) |

The native port and the card-reader project are separate repos under
`../EZ2-projects/` (`port/`, `cardreader/`) as of 2026-09-28; `port/...`
paths elsewhere in these docs refer to that repo.

`original/` (the binaries, DLLs and assets) is **gitignored and absent from this
package** by design.

---

## 4. The build & verify loop

`./match.sh` compiles every `src/*.cpp` + `harness/*.cpp` with the VC7.1
toolchain under Wine and runs `reccmp` against the target:

* `./match.sh` — the overall match table (per-function % + Implemented / Accuracy).
* `./match.sh 0xADDR` — the full instruction diff for one function's address.

A function is *done* only when this prints **100%** for it. Every reconstructed
function carries a `// FUNCTION: EZ2AC_UNPACKED 0xADDR` marker on the line above
its definition; `reccmp` maps that symbol to the target's VA and compares the
bytes. The two coverage audits (`census.py`, `vtaudit.py`) and the duplicate
check (`dupes.py`) are what prove nothing is missing.

Everything needed to reproduce is in the package **except** the toolchain
(`tools/`, gitignored: VC7.1, the Platform SDK, DX8 SDK, the Wine prefix) and
the target image. Rebuilding the toolchain and pointing `match.sh` at a
license-decrypted target reproduces the whole result.

---

## 5. File-by-file role map

### src/ (247 translation units)

| file | role |
| --- | --- |
| `accessors.cpp` | KGraphics |
| `analogread.cpp` | Obj40C180::m40c4b0 @0x40c4b0 - promoted from wip/analogread |
| `applog.cpp` | The engine's logger |
| `backgrounds.cpp` | GFBackGround and CatchBackGround - the two siblings of BackGround (src/panels |
| `battleupdate.cpp` | BattleMode::update2 @0x41c660 - the battle overlay's tick |
| `bgdrawitem4.cpp` | BackGround::drawItem4 @0x45c730 and CatchBackGround::drawItem4 @0x46aed0 - the kind-4 draw of the two backgrounds' draw  |
| `bgjudgenotice.cpp` | GFBackGround::m45cb20 @0x45cb20 and CatchBackGround::m46b7a0 @0x46b7a0 - flash the background's judgement notice |
| `bgload.cpp` | EZ2DJMainGameDirector::m420a60 @0x420a60 - build the keys game's background, and CatchMainGameDirector::m46c4d0 @0x46c4d |
| `bgparse.cpp` | BackGround::m41b680 @0x41b680 and CatchBackGround::m46b9b0 @0x46b9b0 - the |
| `bgparse1st.cpp` | BackGround::m41b0c0 @0x41b0c0 and CatchBackGround::m46b0e0 @0x46b0e0 - the EZ2DJ 1st BACKGROUND FORMAT, which is what th |
| `bgpump.cpp` | GFBackGround::m45c6a0 @0x45c6a0 and CatchBackGround::m46ae40 @0x46ae40 - the background script PUMP, one of kin |
| `bgscript.cpp` | BackGround::m41b900 @0x41b900 - read a background |
| `bgscripttick.cpp` | BackGround::m45c890 @0x45c890 and CatchBackGround::m46b4d0 @0x46b4d0 - one tick of the three payload charts belonging to |
| `bgupdate.cpp` | The three backgrounds' per-frame draw, integrated from wip/bgupdate-45c930 |
| `bgupdate2.cpp` | The three backgrounds' update2 - BackGround / GFBackGround @0x45ca90 (ICF-folded onto one address) and CatchBackGround @ |
| `catchbgload.cpp` | CatchMainGameDirector::m46c4d0 @0x46c4d0 - build the Ez2Catch background |
| `catchdirector.cpp` | timeGetTime WITHOUT dllimport, so the call goes through the thunk @0x4145f0 - the form this function uses |
| `catchfade.cpp` | CatchPanel::m46f3d0 @0x46f3d0 - the three-mode fade colour, the catch twin of Panel::m4270a0 @0x4270a0 (src/gamepanels |
| `catchhold.cpp` | CatchPanel's note drawers @0x46fbb0 / @0x46fe30 - the family's simplest pair: Ez2Catch has no sway offset, no edge lines |
| `catchkeys.cpp` | CatchMainGameDirector::m46e750 @0x46e750 - the Ez2Catch game screen's service-key handler, and the smallest of the famil |
| `catchload.cpp` | CatchMainGameDirector::m46dba0 @0x46dba0 - the Ez2Catch chart loader, and the twin of EZ2DJMainGameDirector::m423a90 @0x |
| `catchpaddle.cpp` | CatchMainGameDirector::m46d360 @0x46d360 - move each side's paddle and open the lanes under it |
| `catchpanelgds.cpp` | CatchPanel::LoadPVI @0x472650 - the Catch note field's |
| `catchstagesetup.cpp` | CatchPlayer::beginStage @0x473fd0 - bind a catch-mode player to its owner for a stage: the keys PlayerSlot::beginStage ( |
| `catchupdate2.cpp` | CatchMainGameDirector::update2 @0x46e550 - the Ez2Catch director's tick |
| `chartcrypt.cpp` | EzChart::decryptFile @0x410a40 - THE CHART FILE CIPHER, integrated from wip/crypt |
| `classinfo.cpp` | Descriptor getters for the game's Director/screen classes |
| `classnames.cpp` | Class-name getters: each returns a pointer to the class's name literal |
| `coinlamp.cpp` | TitleDirector::tickCoinLamp @0x44e880 - one frame of the title screen's coin lamp |
| `colorfill.cpp` | Two small KGraphics leaves, promoted and derived together |
| `com_wrappers.cpp` | Thin wrappers around COM-style (__stdcall) virtual calls on an engine object held in member field 0xb8 |
| `comrelease.cpp` | Obj405F80::releaseAll @0x405580 - drop every DirectDraw/Direct3D interface the object holds |
| `concatmatrix.cpp` | concatMatrix @0x406ad0 - a spine stub, integrated from wip/concatmatrix-406ad0 |
| `constants.cpp` | Constant-return leaf functions |
| `createsample.cpp` | EzSubsystem::createSample @0x40e3c0 - the sound entry point every sample in the game comes through |
| `directors.cpp` | RankingDirector, ResultDirector and GFResultDirector - three more directors whose destructors are their member maps (see |
| `drawline.cpp` | EzObject::drawLine @0x467670 - print a number into a chart row: only while the chart is idle, sprintf the value through  |
| `drawtext.cpp` | KGraphics::drawText @0x406f50 - lock the back buffer and write a whole string into it |
| `effectordraw.cpp` | The effector readout's draw @0x41cb90: three sprite layers, then - for the two readout kinds that show a number - one mo |
| `effectorinput.cpp` | Effector6thStyle::update2 @0x41e620 - the effector input tick |
| `effectorsetup.cpp` | GFMainGameDirector::m45de70 @0x45de70 and CatchMainGameDirector::m46c760 @0x46c760 - build the two effector objects for  |
| `effectorupdate.cpp` | Effector6thStyle::update @0x41d920 - the effector panel's per-frame draw: pick the chart this mode uses, stamp the four  |
| `endingupdate2.cpp` | EndingDirector::update2 @0x455ce0 - the ending screen's tick |
| `engine.cpp` | Log |
| `enginetick.cpp` | One engine tick |
| `ez2ac.cpp` | First byte-match target: a __thiscall accessor returning this->field_0x438 |
| `ez2acapp.cpp` | operator delete == an inlined free |
| `ez2djfinish.cpp` | EZ2DJMainGameDirector::m424830 - step past a finished song |
| `ez2djgameover.cpp` | EZ2DJMainGameDirector::m4204b0 @0x4204b0 - is the song over for everyone? Only while the fade phase f20 is still below 3 |
| `ez2djinput.cpp` | EZ2DJMainGameDirector::m41ffa0 @0x41ffa0 - the per-frame LANE INPUT SWEEP |
| `ez2djkeys.cpp` | EZ2DJMainGameDirector::m422f20 @0x422f20 - the EZ2DJ game screen's service-key handler, and the last of the family of th |
| `ez2djload.cpp` | EZ2DJMainGameDirector::m423a90 @0x423a90 - the EZ2DJ chart loader, written as an EDIT of its twin src/catchload |
| `ez2djsink.cpp` | EZ2DJSink2::v0 @0x4202f0 - the lane-expiry sink |
| `ez2djstage.cpp` | EZ2DJMainGameDirector::m4246d0 @0x4246d0 - begin a stage |
| `ez2djupdate2.cpp` | EZ2DJMainGameDirector::update2 @0x4249e0 - the keys game's per-frame tick |
| `ezbase.cpp` | EzChart's base destructor, alone in its own TU |
| `ezchart.cpp` | operator delete = free, for the deleting destructor |
| `ezcmp.cpp` | Three-key comparator for EzChart records (insertion-sort order): d0 ascending (UNSIGNED compare), byte5 ascending, byte4 |
| `ezload.cpp` | Top-level chart load-by-basename (vtable slot 4) |
| `ezscreens.cpp` | GFPanel, for its factory |
| `ezsubsystem.cpp` | The subsystem's two backend dispatchers |
| `flagwave.cpp` | KGraphics::m406900 @0x406900 - one frame of the FLAG's wave: lock the 11x11 vertex mesh createFlag built, and set every  |
| `gamepanels.cpp` | The three game panels' teardown (see include/gamepanels |
| `gamestate.cpp` | resetGlobals430b50 @0x430b50 - the run-of-globals reset engineTick418e60 @0x418e60 performs before it walks the lists |
| `gaugerates.cpp` | EZ2DJMainGameDirector::m4206e0 @0x4206e0 - the |
| `gdsitem.cpp` | CatchPanel::loadItem - read one |
| `getbit.cpp` | Obj40FBD0::getBit @0x0040fbd0 - read bit 0 of the byte at arr[i] |
| `gfgameover.cpp` | GFMainGameDirector::m45d740 @0x45d740 - is the song over for everyone, the GF twin of EZ2DJMainGameDirector::m4204b0: un |
| `gfinput.cpp` | GFMainGameDirector::m45d200 @0x45d200 - the GF input scanner: per side and per lane-key, route the effector chord, the s |
| `gfkeys.cpp` | GFMainGameDirector::m45f640 @0x45f640 - the GF game screen's service-key handler, and the second of the family of three  |
| `gfload.cpp` | GFMainGameDirector::m45fd50 @0x45fd50 - the scratch chart loader, and the twin of CatchMainGameDirector::m46dba0 @0x46db |
| `gfpanelctor.cpp` | GFPanel::GFPanel @0x462870 - the scratch modes' panel constructor, and the sibling of Panel::Panel @0x429b70 (src/gamepa |
| `gfpanelgds.cpp` | GFPanel::LoadPVI @0x4654b0 - the GF note field's |
| `gfpanelhold.cpp` | GFPanel::m463210 @0x463210 - draw one hold note: catch the slot's row up to the panel's cursor, place the bar, then the  |
| `gfpaneltap.cpp` | GFPanel::m4637f0 @0x4637f0 - draw one tap note: the same row catch-up and placement as the hold drawer beside it (src/gf |
| `gfrank.cpp` | PlayerRecord::m465b30 @0x465b30 - the GF grade ladder, and the twin of PlayerSlot::m42e190 already in src/keysrank |
| `gfresultctor.cpp` | GFResultDirector::GFResultDirector @0x4682e0 TRANSCRIPTION (5,776 bytes) |
| `gfresultrank.cpp` | GFResultDirector::m466ee0 @0x466ee0 - the per-song top-five table, the GF twin of ResultDirector::m450690 (src/resultran |
| `gfresultupdate.cpp` | GFResultDirector::update @0x4680d0 - the GF result screen's per-frame draw |
| `gfsink.cpp` | PlayerRecord::m465f60 @0x465f60 - the scratch game's score commit, the keys PlayerSlot::m42e620 (src/sink |
| `gfstagesetup.cpp` | PlayerRecord::beginStage @0x466700 - the scratch-mode player's stage binding, the third dialect of PlayerSlot::beginStag |
| `gfupdate2.cpp` | GFMainGameDirector::update2 @0x45f350 - the scratch director's tick, and CatchMainGameDirector::update2 @0x46e550's twin |
| `gfxtext.cpp` | The two sprite-font number draws @0x40b100 and @0x40b140 - both pure forwarders into the family's wider base, and both d |
| `gfxtextbase.cpp` | GfxText::m40aaa0 @0x40aaa0 - the sprite-font number drawer the whole family funnels into: sprintf the float through the  |
| `globals.cpp` | KRuntimeClass |
| `holdtick.cpp` | THE KEYS MODES' HOLD STATE MACHINE - three of the four functions the * original lays out back to back at 0x42f4e0, 0x42f |
| `ingame.cpp` | Draw eight DISTINCT numbers below m_randRange[kind] into `out` |
| `ingamestep.cpp` | InGameSongSelect::m425ea0 @0x425ea0 - advance the wheel: the dial or the turntable keys step the selection (1P forward o |
| `ingameupdate.cpp` | InGameSongSelect::update @0x4261d0 - the in-game song-select tick |
| `ingamewheel.cpp` | InGameSongSelect::m425b40 @0x425b40 - lay the current slot's song wheel out: for every entry, wrap its 100px-spaced posi |
| `inputscan.cpp` | Obj40C180::anyPlayHeld @0x4188c0 - is any play button currently held? |
| `judge.cpp` | "Judge" @0x48ffb0 - a sixth class with the panel family's six-slot vtable, reached only from the screen constructor @0x4 |
| `judgegrade.cpp` | Judge::grade @0x426500 - THE GRADING CORE, integrated from wip/judge-grade |
| `judgewalk.cpp` | Obj426CA0::m4265b0 @0x4265b0 - the judge's "is another hit in the way" walker, in a TU of its own so that m4266c0 (src/a |
| `kcontainers.cpp` | Two more members of the KResourceContainer family |
| `kengineapp.cpp` | operator delete == an inlined free |
| `keybroadcast.cpp` | PlayerSlot::m42ee40 @0x42ee40, PlayerRecord::m466550 @0x466550 and CatchPlayer::m473260 @0x473260 - what the third sink  |
| `keychannels.cpp` | Obj40C180::m4189e0 / m418a50 / m418af0 @0x4189e0 / 0x418a50 / 0x418af0 - bind or unbind three consecutive ranges of inpu |
| `keysrank.cpp` | PlayerSlot::m42e190 @0x42e190 - THE KEYS MODES' GRADE LADDER |
| `kezplayer.cpp` | KEZPlayer - see include/ksongplayer |
| `kezplayerreset.cpp` | KEZPlayer::m410110 @0x410110 - put the player back to its just-constructed state |
| `kezplayerslots.cpp` | The eight unreconstructed slots of KEZPlayer's 35-slot vftable @0x4887d0 - the last referenced-but-unannotated functions |
| `kezplayertable.cpp` | KEZPlayer::m40f690 @0x40f690 - rebuild the 96-entry tick table, the tail call of the reset @0x410110 |
| `kfile.cpp` | Class definition in include/kfile |
| `kgraphics.cpp` | the array operator new is a real call |
| `kgraphics2.cpp` | KGraphics, part 2: the methods that src/kgraphics |
| `kgraphicsblit.cpp` | WIP: the corners are COMPILER TEMPORARIES, not named locals - the whole register cascade turned on that |
| `kgraphicsblit2.cpp` | KGraphics::m402f70 @0x402f70 - the single-texture video blit, the two-texture m401d80's sibling (src/kgraphicsblit |
| `kgraphicsquad.cpp` | KGraphics::drawTex @0x00403f80 - the family's plainest member and its * most-called: no crop fractions, no gradient, one |
| `kgraphicsworm.cpp` | KGraphics' spline tube - "the worm" - the 0x409330 |
| `kpalloc.cpp` | Bump-allocate a node (0x50 header + `size` value bytes) from the arena |
| `kparser.cpp` | Log |
| `kprofile.cpp` | KTextureNode, KGraphics |
| `kprofileload.cpp` | KProfile::Load @0x413fe0 - the profile-file record loader, integrated from wip/kprofile-load |
| `krescontainer.cpp` | KResourceContainer: the base class of KProfile (vtable @0x488c24) |
| `ksong.cpp` | Log |
| `ksongdtor.cpp` | ~KSong lives in its own TU on purpose: the original's two virtual calls are genuine dynamic dispatches â the first thr |
| `ksongplayer.cpp` | operator new = malloc |
| `ksongplayerbase.cpp` | KSongPlayerBase ctor/dtor in their own TU: the derived (matched) KSongPlayer ctor calls the base ctor as a real `call` - |
| `ksoundds.cpp` | KSoundDS - the DirectSound half of the sound subsystem, named by its own allocation-failure logs ("KSoundDS::CreateSampl |
| `ktexture.cpp` | KTextureContainer: the minimal KResourceContainer subclass |
| `lamps.cpp` | updateStartLamps is a SongSelectDirector member |
| `library.cpp` | Address annotations for statically-linked CRT symbols in the original binary, resolved against our libcmt by name (reccm |
| `loadaniclip.cpp` | KGraphics::m401b80 @0x401b80 - turn a raw |
| `loader.cpp` | Unload all samples then clear the table @0x00411050 (__thiscall) |
| `loaders.cpp` | KEzwLoader and KWavLoader - the sound subsystem's two audio format loaders |
| `loaderseek.cpp` | KEzwLoader::seek @0x40cf60 and KWavLoader::seek @0x40f1a0 - the two audio loaders' seek, integrated from wip/loaders-see |
| `loadez.cpp` | EzChart::loadEZ @0x411460 - THE CHART LOADER, integrated from wip/loadez-411460 |
| `loadezi.cpp` | EziLoader::loadEZI @0x00411090: parse a |
| `loadsample.cpp` | 0x402880 |
| `loadstr.cpp` | KGraphics::loadStr @0x402de0 - turn a |
| `loadtexture.cpp` | src/strutil |
| `loadtracks.cpp` | EzChart::loadTracks @0x411920 - vtable slot 5, the sibling of loadEZ |
| `loadtxg.cpp` | KGraphics::loadTXG @0x408f70 - the texture ATLAS loader, integrated from wip/loadtxg |
| `loops.cpp` | Accumulation loops over game arrays |
| `maingame.cpp` | The three main-game directors' teardown (see include/ezscreen |
| `maingameslots.cpp` | The two main-game directors' slot-release pass, @0x420ec0 and @0x45def0 - * twins with different bases |
| `mergetracks.cpp` | EzChart::mergeTracks @0x411310 - sort-merge the staged tracks back into the chart's own record list |
| `misc.cpp` | KGraphics, Sprite |
| `mixer.cpp` | Per-control WINMM wrapper view: { HMIXEROBJ @0x00; control id @0x04 } |
| `modename.cpp` | Game-mode name/index globals and the rolling text-slot table |
| `moderunner.cpp` | operator new/delete == an inlined malloc/free |
| `modeselect.cpp` | ModeSelectDirector - the mode-select screen's director (see include/modeselect |
| `modeselectbuttons.cpp` | ModeSelectDirector::m44b4b0 @0x44b4b0 - the button-driven twin of m44b750 @0x44b750, which is matched at 100% in src/mod |
| `modeselectctor.cpp` | ModeSelectDirector's constructor (see include/modeselect |
| `modeselecttimer.cpp` | ModeSelectDirector::m44b750 @0x44b750 - the mode-select countdown, and the confirm it turns into |
| `modeselectupdate.cpp` | ModeSelectDirector::update @0x4491b0 - the mode wheel's 8 |
| `modeselectupdate2.cpp` | ModeSelectDirector::update2 @0x44c860 - the mode wheel's input/state tick, transcribed end to end (the draw half is src/ |
| `newarray.cpp` | The global array `operator new[]`, which the original keeps as a real function (its scalar twin in include/kcrt |
| `nodelists.cpp` | Three singly-linked global lists with an identical helper trio each |
| `note.cpp` | Batch converter, defined BEFORE NoteConvert so cl cannot inline the call |
| `obj405f80.cpp` | Obj405F80 - see include/obj405f80 |
| `obj40c180.cpp` | g_iniProfile |
| `obj40d4e0.cpp` | Obj40D4E0 and its shared base - see include/obj40d4e0 |
| `obj40eac0.cpp` | Obj40EAC0 - see include/obj40eac0 |
| `obj414750.cpp` | Obj414750 - the song-player pump (see include/obj414750 |
| `opensample.cpp` | EzSubsystem::openSample @0x40e2a0 - resolve a sample name and open its stream |
| `panelgauge.cpp` | Panel::m429170 @0x429170 - draw the shared life gauge |
| `panelgds.cpp` | Panel::LoadPVI @0x42d700 - the main note field's |
| `panelhold.cpp` | Panel::m42af30 @0x42af30 - the EZ2DJ panel's tap-note drawer, the Panel-family cousin of GFPanel::m4637f0 (src/gfpanelta |
| `panelholdbar.cpp` | Panel::m42a630 @0x42a630 - the EZ2DJ panel's hold-note drawer: the catch-up (unconditional advance, with the width debit |
| `panelpress.cpp` | TrackSink::m42d370 @0x42d370 - the keys panel's PRESS flash, reached through the slot's sink pointer so it keeps the Tra |
| `panels.cpp` | KGraphics, EzSubsystem, the subsystem singletons |
| `panlevel.cpp` | GFMainGameDirector::m45d5f0 @0x45d5f0 and CatchMainGameDirector::m46c0f0 @0x46c0f0 - the GF and Catch copies of the pan- |
| `panstep.cpp` | The three modes' pan stepper - EZ2DJMainGameDirector::stepPan @0x420400, GFMainGameDirector::stepPan @0x45d690 and Catch |
| `parsercrypt.cpp` | KParser::slurpFile @0x4135f0 - the profile-file reader, and the THIRD instance of the chart cipher |
| `parsetoken.cpp` | KParser::parseToken @0x413940 and its static helper classifyToken @0x413830 - THE CONFIG TOKENIZER every |
| `picksample.cpp` | pickSample42de50 @0x42de50 - the sample m42f080 stamps into a claimed hold note: a twelve-entry {sound, weight} ladder w |
| `playeroption.cpp` | EZ2DJMainGameDirector - what the slot's owner is |
| `playerreset.cpp` | PlayerRecord::reset @0x466690 - put one list-C player back to its starting state under a new id: drop its list link, cle |
| `playersink.cpp` | Log |
| `playerslot.cpp` | The two globals PlayerSlot::reset clears along with the slot itself |
| `rankingchannel.cpp` | RankingDirector::m4570a0 @0x4570a0 - push the finished play into the CHANNEL ranking: ninety-nine places in system\ranki |
| `rankingctor.cpp` | RankingDirector's constructor @0x457a90 - the ranking / name-entry screen's 7 |
| `rankingdefault.cpp` | RankingDirector::m4577b0 @0x4577b0 - the standalone default-name rewrite: when the slot-0 entry's eight characters are a |
| `rankingentry.cpp` | RankingDirector::m456870 @0x456870 - the constructor's go/no-go for name entry |
| `rankinginsert.cpp` | RankingDirector::m456b20 @0x456b20 - push the finished play into the song's top five |
| `rankingname.cpp` | RankingDirector::m45aaf0 @0x45aaf0 - the ranking name-entry stepper: the wheel keys/dial, the glyph wheel with its eased |
| `rankingrow.cpp` | RankingDirector::m4574c0 @0x4574c0 - draw one ranking row: the four-digit place number (c+1, leading zeros suppressed bu |
| `rankingtick.cpp` | RankingDirector::update2 @0x45bc30 - the ranking screen's state machine, dispatching f10 through a seven-entry table: 0  |
| `rankingupdate.cpp` | RankingDirector::update @0x45a290 - the ranking screen's per-frame draw, and a seven-arm state machine on the director's |
| `rankingwalk.cpp` | RankingDirector::m459870 @0x459870 - draw one column of the ranking wheel |
| `rankingwheel.cpp` | RankingDirector::m459bc0 @0x459bc0 - the name-entry overlay |
| `resetscreen.cpp` | KRuntimeClass |
| `resultctor.cpp` | ResultDirector's constructor @0x451f10 - the result screen's 9 |
| `resultdraw.cpp` | ResultDirector::update @0x451b80 - the keys result screen's per-frame draw |
| `resultoptions.cpp` | ResultDirector::m4512b0 @0x4512b0 - the option-icon readout, called at the top of ResultDirector::update2 @0x451550 ever |
| `resultrank.cpp` | ResultDirector::m450690 @0x450690 - the per-song top-five table: read it, sort it, work out where this play's score land |
| `resulttween.cpp` | ResultDirector::m450b50 @0x450b50 and GFResultDirector::m4672f0 @0x4672f0 - tick the two result charts, by phase |
| `resultupdate.cpp` | ResultDirector::update2 @0x451550 - the keys result screen's tick, and the sibling of GFResultDirector::update2 @0x467b9 |
| `runcv2mix.cpp` | EZ2AC::runCv2Mix @0x417a30 - ITS OWN TU: SongTable::loadSongBin is defined in src/moderunner |
| `runspacemix.cpp` | EZ2AC::runSpaceMix @0x417850 - the SpaceMix campaign driver, the family's mid-size dialect: load the mode |
| `scorerank.cpp` | ScoreKeeper::rank @0x472b40 - the grade the round is awarded |
| `scoretransfer.cpp` | ResultDirector::m450fa0 @0x450fa0 and GFResultDirector::m467720 @0x467720 - the battle result's score transfer, one tick |
| `screenrun.cpp` | Obj401780::run @0x401930 - THE SCREEN DRIVER every screen goes through |
| `scrfader.cpp` | ScrFader - the screen fader the three main-game directors build at the end of construction and hand their own first seco |
| `seq.cpp` | Per-lane playback queue filled by LoadEZ @0x411460: 0x800 records + play cursor + count = 0x14008 bytes per track |
| `setcamera.cpp` | KGraphics::setCamera @0x40bd40 - the #2 stub on the port spine, integrated at 16 |
| `settings.cpp` | The settings reader |
| `showcredit.cpp` | g_bigInstance |
| `showcreditsetup.cpp` | EZ2DJMainGameDirector::m420d90 @0x420d90 - build the credit readout, place it by mode, and mark it for the single-player |
| `showcreditupdate.cpp` | ShowCredit::update @0x450040 - draw the credit overlay: the mask additively, the plate over it, then the three readouts  |
| `sink.cpp` | PlayerSlot::commit @0x42e620 - THE KEYS MODES' JUDGEMENT SINK |
| `sink3.cpp` | EZ2DJSink3::v0 @0x4236f0, GFSink3::v0 @0x45fa50 and CatchSink3::v0 @0x46ec00 - broadcast one key event to every node of  |
| `songbin.cpp` | SongTable::loadBinFile @0x469c30 - THE SONG DATABASE LOADER, and the fourth cipher in this binary |
| `songinfo.cpp` | The song-table unpacker @0x431660: look a mode's entry up in g_songTable and spread it into the caller's display block |
| `songselect.cpp` | SongSelectDirector - the song-select screen's director (class named by its own KRuntimeClass @0x4aaed8; see include/song |
| `songselectcode.cpp` | SongSelectDirector::m4462e0 @0x4462e0 - THE COMMAND-CODE MATCHER |
| `songselectctor.cpp` | SongSelectDirector's constructor @0x435390 - 14 |
| `songselectcv2page.cpp` | SongSelectDirector::m4391b0 @0x4391b0 - the CV2 wheel page: a +-5 window over the f80 entry table (positions in 100s, ri |
| `songselectdiff.cpp` | SongSelectDirector::drawDifficulty @0x432930 - one difficulty readout |
| `songselectdiffsel.cpp` | SongSelectDirector::m444ae0 @0x444ae0 - the difficulty-column stepper for the keys modes, run every tick while the selec |
| `songselectinput.cpp` | SongSelectDirector's mode-input handlers - the per-mode arms of the tick @0x446440's thirteen-way switch (see include/mo |
| `songselectpage.cpp` | SongSelectDirector::m439f00 @0x439f00 - the plain list page, the draw update @0x443380 dispatches to for every non-wheel |
| `songselectquad.cpp` | Obj4323C0::drawOne @0x4323c0 - integrated at 69 |
| `songselectrepeat.cpp` | SongSelectDirector::m446440 @0x446440 - the two hold-repeat scans |
| `songselectseed.cpp` | SongSelectDirector::m435070 @0x435070 - seed the select page from one category |
| `songselecttier.cpp` | The three tier probes @0x431810 / 0x431900 / 0x4319f0 - does `name`'s -hd / -shd / -ex variant appear in the hidden-song |
| `songselecttoggle.cpp` | SongSelectDirector::toggleOption @0x434f70 - one of the three option toggles the song-select screen offers |
| `songselectupdate.cpp` | SongSelectDirector::update @0x443380 - the song-select screen's per-frame draw, 4,672 bytes / 1,332 instructions / 221 c |
| `songselectupdate2.cpp` | SongSelectDirector::update2 @0x446540 - the song-select input tick, 10 |
| `sound.cpp` | Volume setter @0x0040f4a0 |
| `stagesetup.cpp` | PlayerSlot::beginStage @0x4305a0 - bind a keys-mode player to its director for a stage |
| `stageteardown.cpp` | EZ2DJMainGameDirector::m4209d0 @0x4209d0, GFMainGameDirector::m45dae0 @0x45dae0 and CatchMainGameDirector::m46c440 @0x46 |
| `strutil.cpp` | @0x00412f60 (__cdecl): copy the substring of `p` starting at the first ' |
| `stubs.cpp` | Definitions of not-yet-matched callees, kept in this separate TU so the matched callers that call them cannot inline/opt |
| `subobj402860.cpp` | Only the destructor lives here â see include/obj402860 |
| `subsystembase.cpp` | SubsystemBase's constructor, alone in its own TU |
| `subsystemload.cpp` | EzSubsystem::m40e520 @0x40e520 - load one sample for `owner` |
| `subsystemreset.cpp` | EzSubsystem::m40e6d0 @0x40e6d0 - hand every live sample to slot 7, then wipe the backend's table under its lock |
| `systemmenu.cpp` | SystemMenu - the operator test menu (see include/systemmenu |
| `tables.cpp` | Append a record to the chart's bounded table (called by insert @0x4124b0 when no sorted predecessor exists; formerly mod |
| `testgameoption.cpp` | 0x16963f8 (src/settings |
| `testinput.cpp` | TestInput - the operator test menu's INPUT TEST page (see include/testinput |
| `texfmtpick.cpp` | pickTextureFormat @0x004072c0 - the texture-format enumeration callback KGraphics::m407360 @0x407360 hands to IDirect3DD |
| `tinyclasses.cpp` | See include/tinyclasses |
| `tinyleaves.cpp` | Small leaves whose callers are already reconstructed |
| `titlector.cpp` | TitleDirector::TitleDirector @0x44e9b0 - the title screen's constructor, and the cheapest of the port queue's big holes: |
| `titledirector.cpp` | The title screen's lamp animation, one frame per call |
| `titleseat.cpp` | TitleDirector::m44e680 @0x44e680 - seat the player in `slot` |
| `titleupdate.cpp` | TitleDirector::update2 @0x44f4d0 - the title screen's per-frame tick |
| `totalresultctor.cpp` | TotalResultDirector's constructor @0x454990 - the course-total screen's 4 |
| `totalresultupdate.cpp` | TotalResultDirector::update2 @0x454800 - the total-result screen's tick, a five-arm state machine on +0x142c whose arms  |
| `tracksinkfx.cpp` | TrackSink::m4299e0 @0x4299e0 - retire one lane's effect state |
| `trackswap.cpp` | PlayerSlot::m430500 @0x430500 - point the in-game song-select overlay at one of this player's two keyed track entries |
| `videoplayer.cpp` | Obj40B2E0 - the DirectShow video player the backgrounds and directors embed |
| `writetree.cpp` | KProfile::writeTree @0x414200 - the recursive in-order dump behind writeFile: left subtree, then `"key" = ` and the valu |

### include/ (44 headers)

| file | role |
| --- | --- |
| `backgrounds.h` | EzObject, Obj40B2E0, BackGround itself |
| `bigbase.h` | One of the two record kinds in |
| `directors.h` | EzObject, Obj40B2E0, EzSoundObject |
| `ez2ac.h` | Reconstructed EZ2AC types |
| `ez2acapp.h` | The game's own application class: KEngineApp's only derived class (its ctor @0x414ea0 calls the base ctor @0x401000, its |
| `ezchart.h` | The main chart/sequence object (original class name likely "EZModule" per the "EZModule(LoadEZ)"/"EZModule(LoadTracks)"  |
| `ezscreen.h` | EzObject |
| `gamepanels.h` | EzObject, Obj402860 |
| `gdsitem.h` | KTexture, KGraphics, Sprite |
| `gfxtext.h` | The graphics singleton's two sprite-font number draws, as their callers see it |
| `icom.h` | The COM-style (__stdcall) interfaces the engine objects hold |
| `ingame.h` | The in-game object: the ~2 MB thing the thirteen per-mode input handlers (include/modeinput |
| `judge.h` | EzObject |
| `kbase.h` | The engine's common polymorphic base (KSong, EzChart, KSNDMixer |
| `kcontainers.h` | KResourceContainer |
| `kcrt.h` | The original's global `operator delete` is just `free`, and it is inlined into every scalar-deleting destructor (so `del |
| `kengineapp.h` | The engine's application base class (vtable @0x487418) |
| `kfile.h` | KFile: a thin Win32 file wrapper |
| `kparser.h` | KParser: the config/token parser (1-entry vtable @0x488b28, pinned in src/kparser |
| `kprofile.h` | KProfile: the config/profile parser ("KProfile::Load - STR/FLOAT/INT %s:%s") |
| `ksong.h` | KRuntimeClass |
| `ksongplayer.h` | Log |
| `ksound.h` | MFC-style runtime class descriptor: {name, base, createFn, pad} |
| `ktexture.h` | KResourceContainer |
| `loader.h` | The sample owner IS the sound subsystem: freeSample @0x40e620 dispatches on the same +0x50 selector and the same +0x54 b |
| `loaders.h` | KRuntimeClass |
| `modeinput.h` | THE PER-MODE INPUT HANDLERS |
| `modeselect.h` | EzObject, KRuntimeClass, DrawGroup |
| `note.h` | Raw 13-byte on-disk |
| `obj402860.h` | A 0x21c-byte record with a head pointer at +0, a 0x200-byte payload buffer at +0xc and a float pair at +0x214/0x218 |
| `obj405f80.h` | The singleton the engine publishes into 0xd36cac as it is built - a KBase derivative with a FOUR-slot vtable @0x487b80 ( |
| `obj40d4e0.h` | IThing |
| `obj40eac0.h` | The subsystem behind the eight-slot vtable @0x4884e8: EzSubsystem plus the backend and two COM objects |
| `obj414750.h` | KLock, KSongPlayerBase |
| `panels.h` | KRuntimeClass |
| `players.h` | The three per-player object kinds the engine keeps on its three global lists (src/nodelists |
| `scorekeeper.h` | The round's score bookkeeping |
| `settings.h` | The machine's configuration, one variable per |
| `sink-view.h` | The field views PlayerSlot::commit @0x42e620 needs - see wip/sink |
| `songselect.h` | EzObject, the family's shared six-slot vtable |
| `sound.h` | Virtual interface of the object receiving the scaled level |
| `systemmenu.h` | EzObject, KRuntimeClass, DrawGroup |
| `testinput.h` | EzObject, KRuntimeClass, DrawGroup |
| `tinyclasses.h` | Three classes with the same minimal shape: a single-slot vtable holding only the scalar deleting destructor, an empty de |

---

## 6. Tooling reference (`scratchpad/`)

The analysis tools, grouped by what they answer:

* **Coverage / done-ness** — `census.py` (unannotated referenced functions),
  `vtaudit.py` (pinned-vtable slot coverage), `dupes.py` (doubly-annotated
  addresses), `spine.py` (reachability from the app entry points),
  `coverage.py`, `wipaudit.py`.
* **Fast per-function diffing (the "zoo")** — `zoo.sh` compiles one TU with the
  project flags in ~2 s; `za.py 0xADDR` / `zdiff.py` / `zoocmp.py` render the
  instruction diff so a source change can be judged without a full `match.sh`.
* **Finding the next target** — `scan.py` / `ready.py` (cheapest to match),
  `twins.py` / `twins2.py` / `twindelta.py` (near-duplicate "family" functions),
  `stubrank.py`, `screen.py`.
* **Semantic checks** — `callcmp.py` (compares the *call multiset & sequence* of
  a reconstruction against the original — the oracle when byte-identity is not the
  goal), `frame.py` (derive a stack frame), `assetmap.py` (read a screen
  constructor as an asset table), `decode.py` / `dis.py`.
* **Build/match** — `match.sh` (root), `zoo.sh`, `d.sh` / `try.sh` helpers.

Note: some tools read the original binary and print disassembly — their **output**
is a hexdump and must never be committed or shared; only the scripts are in the
package.

---

## 7. What remains, and how to continue

1. **The sub-100 matching pass** — ~150 functions integrated at
   "call-sequence-identical" during the port work but never byte-polished. Run
   `./match.sh`, read each sub-100 diff against `docs/cookbook.md`; most are one
   documented lever from 100%. `docs/DONE-CRITERIA.md` freezes the genuine
   compiler-codegen walls (with exemplars) so effort is not wasted on them.
2. ~~**The two truncated vtable models**~~ — done 2026-10-01: `Obj40D4E0` and
   `KEZPlayer` declare their full 19- and 35-slot tables.
3. **The native port** (its own repo at `../EZ2-projects/port` since 2026-09-28) — the runnable reimplementation; its roadmap is
   `docs/PORT-ROADMAP.md` / `PORT-WORKQUEUE.md`.

The one thing that cannot be done without a licensed machine/dongle is producing
an independent offline decryption of `EZ2AC.exe` (the AES key is external). The
algorithm is known (AES-128-ECB); only the key is missing.

---

## Appendix — provenance & the AES-ECB validation

To anchor the target to the genuine original without the key, treat `EZ2AC.exe`
as the ground-truth anchor and accept a decrypted target only if it passes the
AES-ECB bijection check against it:

* build the block codebook (decrypted 16-byte plaintext block <-> genuine
  ciphertext block) over `.text`;
* require it to be a **consistent bijection** — no plaintext block mapping to two
  ciphertexts, no ciphertext to two plaintexts — across all blocks;
* require `.data` byte-identical and `.rdata` to differ only within the import
  directory.

The current target passes all three (0/0 conflicts, 31,983<->31,983 distinct
blocks, `.data` identical). Any future dump can be validated the same way.
