# Porting EZ2AC — the analysis, the seams, and what has to be true first

This document is the groundwork for a **recompilation**: building the
reconstructed source into a native, portable executable with modern APIs,
rather than a byte-identical Windows binary.

It is written to be read cold by a session that has not done the decomp work.

---

## 1. The one idea that changes everything

**A port does not need matching code.**

The decomp's hardest currency is byte-identity. A recompilation only needs
*functional* equivalence. That single change of goal converts the project's most
frustrating category of work into finished work:

`wip/` holds **27 fully decoded functions** that are structurally correct and
stalled on a single register-allocation or instruction-scheduling choice cl 13.10
will not reproduce. Several sit squarely on the runtime spine — the screen driver
`Obj401780::run` (88.5%), the mode-runner template, `KGraphics::loadTexture`
(99.07%), `DecryptFile`, `KProfile::Load`.

For the decomp those are blockers. **For the port they are done.** Read `wip/`
first; do not re-derive what is already sitting there.

---

## 2. How far along the thing actually is

Two different numbers, and the second is the one a port cares about.

`scratchpad/census.py` — whole-binary volume:

```
matched 89,933 / 477,046 bytes = 18.85%
```

`scratchpad/spine.py` — reachability from the application entry points
(`KEngineApp::create`, `runApp`, `EZ2AC::runGame`, `Obj401780::run`,
`EZ2AC::v4`, `EZ2AC::v2`), following call-rel32 plus every slot of any vtable
that reachable code stores:

```
spine total       2059 funcs   328,203 bytes
  matched          477 funcs    62,561 bytes   (19.1% of the spine)
  stubbed          198 funcs    31,063 bytes
  unreconstructed 1384 funcs   234,579 bytes
```

Two things fall out of that:

* **31% of the binary is not on the spine at all** (477 KB total vs 328 KB
  reachable). That is dead code, unused modes and unreferenced helpers the port
  never has to touch. The job is smaller than the headline number suggests.
* **198 of the 225 stubs are on the spine**, only 27 are not. Stubs are not
  filler — they are the work queue, and almost all of it counts.

Caveat on the scan: function-start detection is heuristic (call targets, vtable
slots, annotations), so a few "functions" in the unreconstructed list are
actually mid-function addresses — an unaligned start like `0x45884e` is the
tell. Aggregates are sound; individual entries near the top deserve a look
before being trusted.

---

## 3. Where the platform boundary already is

The single best piece of news: **you are not inventing a HAL. The original
architecture already drew one.** Measured over the whole 477 KB of game code:

| boundary | call sites | where it lives |
| --- | --- | --- |
| Win32 API (IAT) | 192 | scattered, but thin |
| cabinet I/O (`in`/`out`, ports 0x100–0x105) | 24 | behind a flag at `0x4200f4` |
| DirectDraw7 / Direct3D7 | concentrated | inside `KGraphics` |
| DirectSound | concentrated | inside `EzSubsystem` |
| DirectInput (polled) | concentrated | `Obj40C180` |

The COM interfaces are *members of engine objects*, not sprayed through game
code:

* `KGraphics+0x28` → `IDirectDraw7`
* `KGraphics+0x30` → `IDirect3DDevice7`
* `KGraphics+0x34` → front surface, `+0x38` → back surface
* the sound backend is a sub-object at `EzSubsystem+0x54`

So the seams to reimplement are `KGraphics`, `EzSubsystem`, `Obj40C180`'s input
half, `KFile`, and the timing calls. Game code calls *those*, and game code is
what the decomp is reconstructing.

Do not read the 1,397 indirect-vtable call sites in the binary as COM leakage —
the overwhelming majority are the game's **own** virtuals (screens, panels,
directors). The two cannot be separated by grep; they can be separated by
knowing which classes are which, which is what `include/` records.

### The cabinet is already optional

The 24 port-I/O instructions are gated on a flag at `0x4200f4`. The game
therefore already has a no-I/O-board code path. Keyboard/gamepad mapping should
hang off that existing branch rather than being surgery.

---

## 4. What has to be true before the port starts

**a. Functional coverage of the spine, not a percentage.**
19.1% of the spine is matched. The gating question is not "what percent" but
"is `WinMain → create → runApp → runGame → one mode's loop → chart playback →
scoring → result` complete end to end". `docs/PORT-WORKQUEUE.md` is that list,
generated from the scan.

**b. `Obj40C180`'s fields need names.**
It is ~0x420000 bytes and most of it is still `char _pad[...]`. You can *match*
code against opaque offsets; you cannot *refactor* against them. Anything that
reshapes input handling or run state needs those fields identified first.

**c. File formats must become parsers, not struct overlays.**
The decomp reads records by direct overlay with `#pragma pack(1)` — see
`SongRecord` in `src/songinfo.cpp`. That is correct for matching and fragile for
a port: it bakes in x86 alignment, endianness and padding. Formats are the most
portable knowledge this project has produced and deserve real serialisation code
on the port side.

**d. Decide what the port is allowed to differ on. — SETTLED, 2026-07-29.**

The project owner has ruled. The boundary is:

| must be exact | free to differ |
| --- | --- |
| chart parsing and interpretation | screen layout code and draw order |
| judgement timing | texture upload and atlas strategy |
| scoring and grading | the audio mixer's topology |
| the RNG and anything it feeds | frame pacing |

**Screens are REIMPLEMENTED, not ported.** This is the consequential half of
the ruling. The port does not need `GFMainGameDirector`'s 14.7 KB constructor
or the hundreds of panel/director methods that place and animate widgets; it
needs to know *what they draw and when*, and may then draw it any way it likes.

What that changes:

* The bulk vein described in section 6 — "constructors and destructors are the
  other bulk vein" — is **no longer port work**. It stays valuable to the
  decomp as decomp, but it is off the port's critical path. Most of the 234 KB
  of unreconstructed spine is exactly this.
* What the port still needs from those classes is their **data**: which assets
  a screen loads, which fields drive its state machine, and which inputs move
  it. A constructor read for its `loadTexture` / `loadStr` calls answers that in
  minutes without being reconstructed at all.
* The functions that stay mandatory are the ones behind the "exact" column:
  chart loading, `EzChart`, `Judge`, `ScoreKeeper`, the mode runners' state
  machines, and the timing sources they read.

The rule of thumb for anything ambiguous: **if a player could notice it in a
replay of the same chart with the same inputs, it is exact; if they could only
notice it in a screenshot, it is free.**

**e. Two more decisions the owner has settled (2026-07-29).**

* **Renderer: OpenGL 3.3 core, as the first target.** Section 7 established
  that eleven of `m408130`'s sixteen combiner recipes need a programmable
  fragment stage, so `SDL_Renderer` cannot be the endgame. GL 3.3 was chosen
  over `SDL_GPU` because sixteen small combiner shaders in GLSL source beat a
  per-backend SPIR-V/DXIL/MSL build pipeline for this workload. "First target"
  is deliberate — it does not preclude a second backend later, and the seam in
  `port/platform/platform.h` is what keeps that option open.
* **First playable mode: 5-key standard.** That fixes what "playable" has to
  mean for the next few milestones, and therefore which mode runner, which
  chart lanes and which screens matter. Note that `wip/runclubmix.cpp` is the
  decoded mode-runner *template*; the 5-key runner is one of its twelve
  siblings, and under the ruling above its screens are reimplemented anyway.

---

## 5. Strategy: one codebase, two targets

**Do not fork.** The moment there are two source trees the decomp stops feeding
the port and both rot.

The port builds from the *same* `src/` and `include/`, with the platform layer
swapped:

```
PLATFORM_WIN32_ORIGINAL   → the DDraw7/DSound/DInput implementation (reference)
PLATFORM_PORTABLE         → SDL + a modern renderer
```

`port/platform/platform.h` is that seam. `port/shims/` holds the COM-shaped
adapters so existing reconstructed code compiles unchanged against it.

### Verification — THE ORACLE EXISTS. It is at `<your game install>/`

The full, runnable game is on this machine. It is **not** in this repo and must
never be committed (same rule as `original/`: no bytes, no assets, no dumps —
addresses and analysis only).

```
<your game install>/
  EZ2AC.exe                     the packed original (runnable)
  EZ2AC_Unpacked.exe            byte-identical to original/ here (md5 verified)
  EZ2AC_NO_ANTIDEBUG*.exe       the variants CLAUDE.md's address transfer notes
  EZ2AC.ini                     ENCRYPTED - see below, this is a free test case
  2EZConfig.exe + 2EZ.dll       the launcher/loader it boots through
  2EZConfig-V2-master/          that launcher's SOURCE (GPL-3.0)
  ddraw.dll, dxvk.conf          DXVK's DirectDraw - so DDraw7 already runs on
                                a modern GPU via Vulkan translation
  launch-ez2ac.sh               GE-Proton11-1-pwasio + ASIO-over-PipeWire
  bg/ 283   sound/ 487   system/ 58   video/ 152      the game data
```

It boots under `GE-Proton11-1-pwasio` with `WINEPREFIX=<a test prefix>`,
via `2EZConfig.exe`. Read `launch-ez2ac.sh` before trying anything: the 32-bit
pwasio ASIO driver only exists inside that Proton build, and system Wine crashes
at sound init.

**Three things this unlocks:**

1. **A differential oracle is now real.** Chart parsing, judgement timing and
   scoring can be diffed against the running original rather than eyeballed.
2. **Golden files for every format**, for free — 283 backgrounds, 487 sounds,
   58 system trees, 152 videos.
3. **`EZ2AC.ini` is encrypted with the game's own cipher**, and the decomp has
   that cipher (`DecryptFile` @0x410a40, parked in `wip/crypt.cpp`; `KProfile::Load`
   in `wip/kprofile-load.cpp`). Running our decrypt over this real file and
   getting readable INI back is the cheapest end-to-end validation available,
   and it needs no window, no audio and no graphics. **Do this first.**

### `2EZConfig-V2-master` — read it, but mind the licence

An open-source launcher/loader for the whole EZ2 family, and it has already
solved problems this port faces:

* `src/2ez-dll/ez2dj-io/` — **full cabinet I/O emulation**, touching ports
  0x100–0x106. That is the same range the decomp found (`docs` §3), so it is a
  working reference for what the board is expected to return.
* `src/2ez-dll/ddraw/` — DirectDraw fixes for modern GPUs.
* `src/2ez-dll/audio-eq/` — mixer and Hypersonik hooks; the game is 44.1 kHz.
* a JSON patch library keyed by RVA and pattern scan, plus `game_defs.h` /
  `game_md5.h` identifying every release. **Our target is "Final:EX"
  (`ez2ac_fn_ex`).**

**It is GPL-3.0, and the project owner has decided the port will be GPL-3.0
too — so this code CAN be reused.** `port/LICENSE` carries the licence and
`port/ATTRIBUTION.md` the obligations: keep kasaski's notices intact, mark every
file you modify, and log each derived file in that file's table.

Two boundaries that still hold:

* **GPL code goes in `port/` only.** `src/` and `include/` are the byte-matching
  decomp and must stay clean — they have no licence of their own yet.
* **Do not distribute port binaries until the repo root has a LICENSE.** The
  port links the decomp sources, so a shipped binary is a combined work. There
  is currently no licence file at the repository root. Building and running
  locally is fine; distribution needs the owner to settle it.

---

## 6. Priority shift for the decomp side

If the goal is a playable port rather than a byte-perfect decomp, the priority
order changes:

* **Chase** the game-logic spine and the remaining file formats — `EzChart`,
  scoring, judgement, chart loading. Highest value to a port, and partly done.
* **Deprioritise** register walls in draw routines. Several of the parks in
  `wip/` would not have been worth attempting under a port goal.
* **Deprioritise** the platform shell entirely. `../EZ2REWRITE` already has a
  loose version, and none of it needs to be byte-exact.

See `docs/PORT-WORKQUEUE.md` for the concrete ordered list.

---

## 7. The seam, after compiling it: what revision 1 got wrong

`port/platform/platform.h` was designed from the engine's *behaviour* before
anything had been built against it. It has now been read against the matched
`KGraphics` (`include/ez2acapp.h`, `src/kgraphics.cpp`, `src/kgraphics2.cpp`),
the decoded draw primitives (`wip/drawrect.cpp`, `wip/kgraphics-colorfill.cpp`,
`wip/loadtexture.cpp`) and a working SDL3 implementation
(`port/platform/sdl/` at the time; retired 2026-09-06 in favour of `port/platform/gl/`).

**Verdict: the boundary is in the right place, the vocabulary was wrong.**

Section 3 stands - the COM objects really are members of `KGraphics`,
`EzSubsystem` and `Obj40C180`, and game code really does not touch them. Nothing
about the *location* of the seam needs to move. But revision 1 described a 2D
sprite blitter, and `KGraphics` is a fixed-function **multitexture Direct3D7
device**. Five specific gaps, each now closed in revision 2:

**a. Two texture stages with independent UV sets.** `KGraphics::m408130`
@0x408130 (matched, 190 `SetTextureStageState` calls) is a table of sixteen
combiner recipes. Modes 0-4 are single-stage. Modes 5-15 program stage 1 with
`D3DTSS_TEXCOORDINDEX = 1` - a *second* UV set - and ops with no 2D-API
equivalent: `MODULATE2X`, `ADDSIGNED`, `BLENDTEXTUREALPHA`,
`BLENDDIFFUSEALPHA`, `D3DTA_COMPLEMENT`, `D3DTA_ALPHAREPLICATE`. A one-texture
`ezDrawQuad` cannot express any of it. Added: `ezBindTexture(stage, t)` and
`ezSetCombiner(mode)`.

**This is the single biggest technical decision the port still faces.** Eleven
of those sixteen recipes need a programmable fragment stage. SDL's 2D renderer
covers modes 0-4 and nothing above; the endgame is `SDL_GPU` or GL with one
shader per recipe. The current SDL layer implements the seam honestly and logs
once when a mode it cannot express is selected.

**b. Two different fill primitives, not one.** `drawRect` @0x40a4c0 is a 3D
screen-space quad taking absolute **edges**; `m40a6a0` @0x40a6a0 is a
DirectDraw `COLORFILL` blit straight to the back surface taking an integer
**origin and size**, where zero means "the whole screen". Revision 1's single
`ezFillRect` conflated them. Both now exist, with the right signatures.

`wip/drawrect.cpp` also pins the vertex format outright: `D3DPT_TRIANGLESTRIP`,
4 vertices, FVF `0x1c4` = `XYZRHW|DIFFUSE|SPECULAR|TEX1`, stride `0x20`.
`XYZRHW` means the 2D path is **pre-transformed** and ignores the camera - so
`setCamera`/`setProjection` matter only for the background meshes.

**c. A global vertical flip.** The flag at `0xd36cb0` is honoured by both fill
primitives - and they disagree, mirroring about y=479 and y=480 respectively.
That is the original's inconsistency, and `m40a6a0`'s version produces a
rectangle whose bottom is above its top, which DirectDraw treats as empty.
Reproduced deliberately; a port that "fixes" it diverges. Added `ezSetFlipY`.

**d. The present has a destination rectangle.** `KGraphics::m405d80` @0x405d80
either `Flip()`s the chain or `Blt()`s the back buffer into `m_destRect`
(+0x18). That rectangle is exactly the 640x480-to-modern-display scaling seam,
so it must be explicit. Added `ezSetOutputRect` (null = letterbox).

**e. Text, atlases and screenshots were missing.** `drawTextf(KGraphics*, x, y,
fmt, ...)` is used by every operator test page (`src/testinput.cpp`,
`src/systemmenu.cpp`) and its strings are **CP949 Korean** as well as ASCII -
that needs a real font, not a bitmap strip. `loadTXG` @0x408f70 registers atlas
sub-rectangles sharing one surface, so a sub-texture must be a first-class
handle. `screenShot` @0x40a7b0 is mostly Win32 but the port owes the pixels.

### Three things the bring-up turned up that are not seam questions

* **There is not a single `.bmp` in the asset tree.** Every texture is `.abm`.
  `KGraphics::loadTexture` @0x408d00 redirects `.bmp` to the `.abm` of the same
  name when `KGraphics+0xd4` is set, and anything not `.bmp` takes a "TGA"
  path. **The redirect is engine-side, above the seam, and it is not
  optional.** Decoder now written - see section 8.
* **Case-insensitive path resolution is required, not a nicety.**
  `src/modeselectctor.cpp` asks for `System\PatchNote\...` and
  `system\modeselect\...` four lines apart, and the prior RE records asset
  folders differing only in case between songs. Implemented and verified
  against the real tree.
* **Texture caching is the engine's job, not the platform's.** `loadTexture`
  keeps its own registry in the `KProfile` at `+0x11c` with a byte total and
  count at `+0x120`/`+0x124`. A cache in the platform layer would be a second
  one. Written into `platform.h` so it does not get added by reflex.

### What is actually built

(As of 2026-08-05. The SDL_Renderer backend described here was retired on
2026-09-06; `port/platform/gl/` is the only one now.)

`port/platform/sdl/` on SDL3: window, frame loop, both fill primitives,
textured quads, blend pairs, texture load/free/sub-rect, path resolution,
screenshots. Audio, input, the cabinet and the 3D camera are stubs marked TODO
with the address each owes. `ez2gfxdemo` drives it, and `ctest`'s `gfx_smoke`
runs it headless (`SDL_VIDEODRIVER=dummy`) and asserts the captured frame is
not one flat colour - which caught a silently-rejected blend mode that made
every frame solid black while every call still reported success.

The cabinet stub deserves a note: **reporting "no board" is a complete and
correct implementation**, not a placeholder. The original already gates all 24
port instructions on the flag at `0x4200f4`, so the no-board path is the game's
own code.

---

## 8. M1 — the asset pipeline, and what 62,000 files taught it

The first milestone was chosen because it needs nothing from the decomp
session and nothing from the platform: formats are pure data, and the game
ships its own regression suite in the form of 290,000 files.

`port/ez2/abm.c` decodes `.abm` textures; `port/tools/ez2asset` converts them
to PNG and sweeps whole trees. Current state, whole game:

| tree | files | `.abm` decoded | encrypted decoded | failures |
| --- | --- | --- | --- | --- |
| `system/` | 39,383 | 34,210 | 26 | 0 |
| `bg/` | 35,595 | 27,991 | 0 | 0 |
| `video/` | 455 | 0 | 151 | 0 |
| `sound/` | 215,552 | 0 | 37,061 | 1 |

### Why a bulk sweep, and not three sample files

Every one of the format rules below was **wrong in the first draft** and was
caught by the sweep, not by reasoning. A decoder that handles one file proves
nothing; the tail is where formats actually live.

* **`.abm` is a BMP with an `AW` magic and four XOR-masked header fields**
  (data offset, width, height, depth), with a per-game-era mask table. Ours is
  Final EX. Auto-detected by *consistency* — the decoded depth must be legal
  **and** the decoded offset must be possible for it — rather than by a single
  magic value, which is what makes the detection safe.
* **The palette is what lies between the header and `bfOffBits`**, exactly as
  BMP defines it. Assuming "no palette" rejected 239 files; then assuming a
  full `4 × 2^bpp` palette still rejected 11, because `bg/5lucid/vio*.abm`
  carry **255** entries, not 256.
* **Rows are not always padded to 4 bytes.** 31 files store them packed tight
  (`bg/5hyper/hyperman02.abm` is 498 px of 24-bit = 1494 bytes a row where BMP
  would use 1496). Derived from the body size, which is exact.
* **The declared depth is sometimes a lie.** 25 files claim a paletted depth,
  carry no palette, and hold 24-bit pixels. Only a depth that *cannot* be right
  is overridden, and only when exactly one alternative fits the body size.
* **The 2 spare bytes that 35,376 files carry are at the END, not the front.**
  This one was genuinely ambiguous from sizes alone, and getting it wrong would
  have shifted every pixel of every texture by two bytes — an error that still
  produces a recognisable picture, which is the dangerous kind. Settled
  objectively: decoding a greyscale mask at the declared offset gives 100%
  grey pixels, two bytes later 93%.
* **Extension does not determine the cipher table.** The engine picks by call
  site (`DecryptFile` @0x410a40 / `DecryptFile2` @0x410c90 / the profile
  slurps), not by name — and `sound/10harpe/5radiomix1p-10harpe-rd3.ezi` proves
  it: `.ezi` table, `EZFF` v7 chart inside.

The one remaining failure, `sound/7rdc/5radiomix1p-7rdc-r2.ezi`, decodes to
`"NaN "` plus noise under every table and is twice the size of its siblings. It
looks like a corrupt asset in the shipped game rather than a format we do not
understand. Left failing rather than special-cased, so it stays visible.

### Sprite transparency is synthesised, not stored

16- and 24-bit `.abm` have no alpha. Sprites are cut out against a black or
white border, which is what `loadTexture`'s `SetColorKey(DDCKEY_SRCBLT, 0)`
sets up. `ez2_image_punch_key` decides the key from eight border samples,
leaves near-uniform images alone (they are fills, not cutouts —
`system/modeselect/0_black.abm` is exactly that), and ramps alpha with distance
from the key so edges do not alias. 43,819 of the 62,201 textures are keyed.

A 32-bit `.abm` whose alpha never varies is 24-bit art padded to 32; its stored
alpha is usually all-zero and **must be ignored** or the sprite vanishes. Only
a varying alpha is a real channel.

### Provenance

The format and the XOR tables are RE knowledge from the sibling project
`../EZ2REWRITE` (`reverse-engineering/file-formats.md`, `src/bga/abm.lua`),
which credits freem's `ezabm.c`. `port/ez2/abm.c` is written from that
specification rather than translated from it; see `port/ATTRIBUTION.md`.

### Still owed in M1

`.txg` atlas descriptions. `.str` and `.scr` are done - see section 9.

---

## 9. M1 continued — `.str` clips and `.scr` timelines

`port/ez2/str.c` and `port/ez2/scr.c`. Same method as section 8: build it, then
run it over everything the game ships and let the tail correct the spec.

| format | files | parsed | notes |
| --- | --- | --- | --- |
| `.str` | 10,593 | **10,593** | three variants, all handled |
| `.scr` | 314 | 311 | the 3 holdouts are developer leftovers |

### `.str` — three variants, not one

The prior RE describes the modern one. The game ships two more, both found by
parsing every file to EOF and demanding the body land exactly on the end:

* **v148** — `"STRM"` + `u32 148`, 36-byte header, 124-byte keyframes. 10,579
  files. The header's trailing 16 bytes of padding are easy to over-count:
  `system/Warning_Short.str` looks like it has a 24-byte pad, but what it
  actually has is a **first layer with no textures and no keyframes**. Parsing
  to EOF settles it — 16 bytes fits 10,579 files, 24 fits none.
* **ascii** — a 12-byte `"STRM 1.0.0 \0"` in place of magic + version. One
  file, `bg/2ndjewel/765.str`.
* **legacy** — no magic at all, a 12-byte header, and **88-byte keyframes**.
  13 files. This is not a truncated v148: it drops uv1, textureId, animType,
  animDelta, rotation and the blend factor, *and it stores the draw quad
  BEFORE the uv rect*, where v148 has them the other way round.

  The 88-byte layout was derived from `bg/Reggaerm/noname0004.str`, whose
  single keyframe then reads as position (320, 240) — the centre of a 640×480
  screen — a quad of (−256, 256, 256, −256, −256, −256, 256, 256) — a centred
  512×512 square — and a colour of (255, 255, 255, 255). Every field lands on
  a value that means something, which no other alignment does. Confirmed
  independently two ways: all 13 files parse to EOF at that size, **and** the
  header's first word equals the layer count plus one in all 13.

Two gotchas worth repeating because they are silent when wrong: **the uv rect
is `[u, v, WIDTH, HEIGHT]`** — an offset and a size, not corners — and
**rotation is an index into a 1024-entry sin/cos table** (@0x4a5228 /
@0x4a6228), not radians.

### `.scr` — scanned, not walked, and deliberately so

A record carrying a clip name is 136 bytes (`[u32 tick][u32 type][char[128]]`),
but those are interleaved with 8-byte control records whose grammar did not
fall out: assuming "types 0 and 1 are control records" got 52 of 314 files to
EOF, and the type histogram of the rest was obvious misalignment noise.

So the parser **scans for names and reads the two words in front of each** —
which is what the prior RE settled on, and which is exactly what a player
needs. The control records are not reported; nothing reads them yet. If a
re-serialiser is ever wanted, that grammar is the missing piece.

Three things the sweep added to the prior RE's description:

* a record's name may be a **`.bmp`**, not only a `.str`
  (`bg/frantic/sdfdsfsdf.scr` names `back-tile.bmp`);
* a name may be a **bare stem with no extension**, and not only in the 12
  legacy headerless files — `bg/rdm/old.error/rdm.scr` is a SCR0 file that
  names `BACK`;
* so the extension cannot carry the check. **The type word does**: a real
  record's type is a small enum, while four bytes of text read as a `u32`
  lands in the millions.

Relaxing the name rule that far was verified not to invent records: every file
that already parsed produced the identical event list, three more files began
parsing, and the count of clip references that fail to resolve on disk **did
not move** (19). New matches that resolve to real files are not false
positives.

`tick` is a **frame count at 60 Hz** (`seconds = tick / 60`), not a musical
tick — do not route it through the chart's BPM clock. `type` 3 is the normal
scene and 2 a parallel "COOL"/high-gauge timeline whose ticks restart at zero;
which is which is decided **by name** (a `cool` or `iba` token).

### A finding the port must design for: dangling references

**6% of `.str` texture references point at files that do not exist.** In `bg/`
that is 6,704 of 108,159, covering 457 unique names whose basename appears
nowhere in the tree under any extension — `bg/6dancewith/peep_city_bk.str`
wants `pinetree_m.bmp` where only `pinetree.abm` and `pinetree_dark.abm` ship,
even though `_m` masks are a real convention in that same folder
(`circle_01_m.abm` exists). `system/` adds another 2,170.

These are the game's own dangling references, not a lookup rule we are missing.
**The port's texture loader must tolerate them and carry on**, because the
original evidently did. The sweep reports them and does not treat them as a
failure.

### The three `.scr` left failing

`bg/{frantic,6frantic,7frantic}/sdfdsfsdf.scr` — a literal keyboard-mash
filename, sitting beside the real `frantic.scr` and a `test.scr`. Their one
record has a type of `0xffffffff`. Left failing rather than widening the type
check for three unused files: an accurate "no records found" beats a rule
loosened until nothing can fail.

---

## 10. The chart parser — the first "must be exact" format

`port/ez2/chart.c`. Everything before this was in the *free* column; a chart is
not. A port may draw a note anywhere it likes but must read it identically, so
this one follows the decomp's **matched** code in preference to the prior RE
wherever they disagree — and they do.

**12,359 of 12,361 charts parse; 27,473,420 notes, 557,431 of them holds.**
(v8 = 12,259, v7 = 100.) The two failures are one song, `10harpe`'s `rd3` pair,
whose `.ez` *and* `.ezi` are both mangled — the `.ezi` is the same file that
turned out to hold an EZFF v7 chart instead of a sample list.

That it is corruption rather than a format variant is checkable, and was
checked: a healthy chart contains the string `EZTR` exactly 64 times, once per
declared track, and walking the track table lands **exactly on the end of the
file**. The broken one contains it 82 times and the walk stops at track 26.

### What the matched code adds to the prior RE

* **There are two 64-byte names in the header, not one.** The prior RE
  describes a single name at 0x06 and then jumps to 0x86, leaving 0x46..0x85
  unaccounted for. `wip/loadez-411460.cpp` shows the loader copying *two*
  0x40-byte buffers into two fields of the chart object.
* **There are eight note types plus an open-ended tail, not four.** The prior
  RE documents 1 Note, 2 Volume, 3 BPM, 4 BeatsPerMeasure. The switch in
  `NoteConvert` @0x410ef0 (matched, `src/note.cpp`) also has 5 (no payload),
  6 and 7 (one raw word each), and a default arm for everything from 8 up that
  reads **two** words. Records are fixed width, so stopping at 4 does not
  mis-stride — it silently drops events, which is worse.
* **Byte +0x0c of a v8 record is unused.** The converter reads a note only up
  to +0x0b, and the record is 13 bytes. Worth knowing before writing a test
  fixture, which is exactly where it bit.

### The v7 widening is the format's one real footgun

Records are `5 + payload`, where the payload is 5 bytes for v4, 6 for v5/v6 and
8 for v7/v8. v7 widened the key index from `u8` to `u16` — songs outgrew 256
samples — which pushed velocity, pan and the unknown byte along by one and the
length from +9 to +10.

Read a v8 chart at the v6 offsets and the length picks up the byte above it:
6 becomes `0x0600` = 1536, so **every note in the game becomes an endless
hold**. It parses, it looks plausible, and it is completely wrong. Branch on
the version. `tests/test_chart.c` pins both widths so a single-branch mistake
cannot pass.

### Note semantics worth writing down

* `length` is **biased by 6**: 0 or 6 is a tap, anything more is a hold lasting
  `length - 6` ticks.
* `key_index` selects a sample from the sibling `.ezi`'s list.
* Defaults are **velocity 127** (full) and **pan 64** (centre), and both are
  applied, not merely stored — the note dispatcher sets DirectSound volume and
  pan from them before playing. Pan runs 0 = hard left to 127 = hard right.
* Every modern chart declares 192 ticks per measure and 64 tracks, of which a
  given mode uses a subset as its lanes. The busiest chart in the game has
  9,372 notes across 52 used tracks.

---

## 11. Audio — samples, the keysound index, and the first thing you can listen to

`port/ez2/ssf.c` and `port/ez2/ezi.c`, plus `port/tools/ez2render`.

| format | files | parsed |
| --- | --- | --- |
| `.ssf` samples | 178,474 | **178,470** (38.7 hours of audio) |
| `.ezi` indexes | 12,359 | **12,359** (4,202,081 sample references) |

### `.ssf` has no magic number, so the header validates itself

18 bytes that are exactly a `WAVEFORMATEX` tail plus a size — channels, sample
rate, byte rate, block align, bits, data bytes — then raw PCM. Nothing
identifies it as an `.ssf` at all.

What makes validation possible is that **two of the six fields are redundant**:
block align must be `channels × bits/8`, and byte rate must be `sample rate ×
block align`. Requiring both to agree turns a range check into a real one.

The library is not uniform: of 178,470 samples, 166,846 are at 44.1 kHz (so
about 7% are not), 163,139 are stereo, and 178,466 are 16-bit. **A player has
to resample**, which is not obvious from the "the game is 44.1 kHz" note in
section 5.

Four files fail, and they are two files duplicated across `7dolls` and
`7dolls2`: they declare 3 channels at 24 bits — implying an align of 9 — while
storing 6, and their data size is odd, so it is not a whole number of frames at
any alignment. Rejected rather than guessed at.

### `.ezi` — three parse behaviours worth copying exactly

Plain text once decrypted, `<note> <mode> <filename>` per line. Following the
matched `EziLoader::loadEZI` @0x411090 rather than the obvious reading:

* **Mode 2 lines carry a SECOND filename**, which the original reads and throws
  away. Miss it and every following line is shifted by one token — silently,
  because the shifted tokens still parse as a plausible entry.
* **Any mode that is neither 1 nor 2 reads one filename.** There is no default
  branch in the switch; mode 0 falls through to the same place as mode 1, and
  mode 0 is common in real files.
* **A note number outside [0, 0x800) abandons the whole file.** The original
  logs, unloads every sample it had already created and returns failure. So
  does this — it does not skip the line.

The `;` comment branch exists in the matched loader and is **dead code**: not
one of the 12,359 files contains a comment.

### The finding that would have broken playback

**A `.ezi` may name a sample in another song's folder, with a Windows relative
path** — `..\..\sound\stay\p_MR.wav`. 4,703 references do. A loader that treats
the name as a filename in the `.ezi`'s own directory fails on every one of
them, and the failure looks exactly like a missing asset.

With paths resolved (both separators, `..`, case-insensitive, and the
`.wav` → `.ssf` swap the disk requires), 4,197,115 of 4,202,081 references
resolve. The remaining 4,966 — 0.118% — are dangling in the game's own data,
the same tolerance requirement the textures already established.

### `ez2render` — the chain, end to end, with no window and no audio device

    ez2render --exe UNPACKED.exe CHART.ez OUT.wav [--seconds N]

Decrypt → chart → keysound index → samples → tempo map → velocity and pan →
a stereo WAV. Every format before this was validated in isolation; this is the
first thing that puts them together, and the artefact is one a human can check
by ear.

It is not a player — no judgement, no input, no BGA. But for EZ2AC the notes
*are* the music, so a correct render is strong evidence the data layer is right.

Checked on `5keymix1p-6dancewith` (125 BPM, 14 tempo changes, 400 notes in the
first 30 s): detected onsets land on the beat grid at differences of exactly
1.0, 1.5, 2.5 and 4.0 beats. **That is the tick-to-seconds conversion
verified**, which is the part most likely to be quietly wrong. A full chart
renders in one pass — 1,947 notes over 107.4 s, every sample present.

Two simplifications, stated rather than hidden: type-4 (beats per measure)
events are ignored, so a chart in anything but 4/4 would drift; and off-rate
samples are resampled nearest-neighbour, which is fine for a check by ear and
not fine for a player.

---

## 12. The OpenGL 3.3 renderer

`port/platform/gl/`. The backend the project committed to in section 4e, and
now the only one — the SDL_Renderer backend was removed on 2026-09-06, with
`EZ2_RENDERER` along with it.

### What GL buys, concretely

Two things sat outside `SDL_Renderer`'s model, and both were load-bearing:

* **Arbitrary blend pairs.** `setBlend` @0x40a490 takes any D3DBLEND
  source/destination pair; SDL accepts a fixed set and *rejects* most composed
  modes, which is how the screen fade came out inverted (section 7).
  `glBlendFuncSeparate` takes any pair, so the mapping is now a plain table
  with no special cases and no fallbacks.
* **Two texture stages.** Eleven of the sixteen combiner recipes need one.

### The shader is parameterised, not specialised

The obvious approach — sixteen fragment shaders, one per recipe — was not
taken. `combiner.h` holds each mode as **the ordered list of
`SetTextureStageState` calls it makes**, and one shader evaluates whatever
state those writes accumulate into.

That shape is forced by the format, not chosen for elegance: **D3D's stage
state is persistent**, and the recipes rely on it. Mode 3 never sets stage 0's
`ALPHAARG1`, so whatever the previous recipe left there is still in force.
Modelling each mode as a complete state block would be a *different program*
from the one the game runs. Writing the recipes as call lists also makes them
diffable against `src/kgraphics2.cpp` line by line — sixteen hand-written
shaders would have been sixteen chances to transcribe one wrong.

The shader implements the pieces those recipes actually use: `SELECTARG1/2`,
`MODULATE`, `MODULATE2X`, `ADD`, `ADDSIGNED`, `BLENDTEXTUREALPHA`,
`BLENDDIFFUSEALPHA`, `DISABLE`, and the `D3DTA_COMPLEMENT` /
`D3DTA_ALPHAREPLICATE` argument modifiers — applied in D3D's order, replicate
then complement, which mode 7 depends on.

### Something the first frame taught us

With the demo drawing untextured quads *before* selecting a combiner, they came
out fully opaque despite carrying alpha. That is **correct**: D3D's default
stage state takes alpha from the texture, and an unbound stage reads as opaque
white. It is also exactly why `setFade` @0x40b080 calls `m408130(0)` before it
draws anything.

Worth recording because it is the kind of thing that looks like a renderer bug
and is not — and because it was the first evidence the emulation was live
rather than being quietly ignored.

### State

Implemented: window and GL 3.3 core context, frame loop, letterboxing,
`ezDrawRect`, `ezDrawTex`, `ezFillRect` (a scissored clear — the COLORFILL path
bypasses the 3D pipeline, so no blend and no combiner), blending, the sixteen
combiners, texture upload **straight from `.abm`** including the colour key,
atlas sub-rectangles, screenshots, path resolution.

Not yet: `ezDrawText` draws nothing (the strings are CP949 Korean as well as
ASCII, so a bitmap strip will not do); the 3D camera; audio; input. Each is
marked TODO with the address it owes.

No GLEW, no glad: the ~40 entry points used are listed once in `glfuncs.h` and
expanded three times, so adding one is a one-line change.

### Verified

`ez2gfxdemo` renders all sixteen combiner modes over the same real game texture
and they visibly differ; `ctest`'s `gfx_smoke` runs it headless
(`SDL_VIDEODRIVER=offscreen`) and asserts the frame is not one flat colour.
Both backends pass that test.

---

## 13. Judgement, scoring and the groove gauge

`port/ez2/songini.c`, `port/ez2/score.c`, `port/tools/ez2judge`. The heart of
the "must be exact" column.

### The judgement windows are DATA, and they vary enormously

Every chart ships its own windows and gauge rates in a sibling `.ini`,
encrypted with the same table as `EZ2AC.ini`:

```
[General]          Level, MeasureScale
[JudgmentDelta]    Kool, Cool, Good, Miss   (CHART TICKS — see below)
[GaugeUpDownRate]  Cool, Good, Miss, Fail
```

Measured over all 12,342 chart `.ini` files: **327 distinct window sets and 498
distinct gauge sets**, and **every single file has a `[JudgmentDelta]`
section**. The commonest windows (6/24/50/70) cover only 45% of charts.

That settles a question the prior RE raises but could not quantify: a clone
that hardcodes one global set — as the Rizu fork does — is not playing the same
game as the cabinet on 55% of the library, and is off by a different amount
each time. The port reads them per chart.

Code defaults, for a *missing* file, are 72/36/24/6 in Miss/Good/Cool/Kool
order with Level 99 and gauge +0.2/+0.1/−1.8/−4.8. There is no `Kool` key in
`[GaugeUpDownRate]`: the section parser @0x4206e0 mirrors COOL into the KOOL
slot, so **KOOL's gauge rate is COOL's**.

**And the two negative rates are applied to the OPPOSITE grades from their key
names** — read 2026-08-10 off the keys modes' judgement sink @0x42e620, which
indexes `director + 0x308 + grade*4`:

| game grade | slot | the `.ini` value that filled it |
| --- | --- | --- |
| 1 — a note never pressed | +0x30c | `Fail` |
| 2 — a press in the outermost band | +0x310 | `Miss` |
| 3 — Good | +0x314 | `Good` |
| 4 — Cool | +0x318 | `Cool` |
| 5 — Kool | +0x31c | `Cool` again |

@0x4206e0 reads the four entries **positionally** — the section name is matched,
the keys inside it are not — filling +0x318, +0x314, +0x310, +0x30c in the
file's own order, which is `Cool, Good, Miss, Fail`. Grades 3 and 4 landing
exactly on `Good` and `Cool` is what pins the alignment; the other two follow.

Read that way the `.ini`'s names are self-consistent: `[JudgmentDelta] Miss` is
the outermost WINDOW and `[GaugeUpDownRate] Miss` is what a press inside it
costs. It is the *port's* grade names that were crossed, and `ez2/score.c` had
the two drains swapped until this was read — so mistiming a note cost more than
ignoring one, which is backwards.

The gauge is also **frozen once it reaches zero**: @0x42e7e6 tests `gauge > 0`
before adding anything, so no later note lifts it. Ceiling 100.0 (@0x48dbfc),
which is also what the reset seeds it with.

### And the unit is the TICK, not the millisecond

Corrected after `../docs/judge-timing.md`, which read the press path out of the
binary. The judge's clock is `KEZPlayer.f7258`, and it counts **chart ticks of
1/192 beat** — not the `.ez` file's 1/48 grid: the director's chart-load
@0x423c18 reads the file's rate, multiplies by 4, rescales every record time to
it and logs `m_nTPB = 192`. `Judge::grade` @0x426500 then compares
`dt = pressTime − noteTime` against the lane's window table in that same unit.

So a window is **beat-relative**, and

    1 tick = 312.5 / BPM  ms        (2.60 ms at 120 BPM, 1.56 at 200)

The same `.ini` numbers are three times tighter in milliseconds on a 200 BPM
chart than on a 66 BPM one. A port that judges in milliseconds is playing a
different game on every chart but the one whose tempo it calibrated against —
which is what `test_score.c`'s `test_tick_unit` now pins: a 90 ms press is a
GOOD at 120 BPM and a MISS at 200. Prior RE (EZ2REWRITE's
`bible/behavior/judgment-scoring.md`) reads these values as milliseconds; that
is the one claim in it to distrust.

**And the game never judges with the file's numbers.** The call site @0x430793
widens all four by **+3 ticks** before `Judge::setWindows` @0x426ea0, or by +1
when the alternate-mode flag at 0x1b2eb6c is set. The 6/24/36/72 defaults are
really 9/27/39/75. `ez2_song_ini_apply_judge_widening()` does it, and it is
called once at chart setup like the mode bonus and the hard gauge.

Two things the original does that the port deliberately does not. Its press
timestamp is synthesized when the *frame processes* the press — vtable slot 22
@0x40f450 returns `max(clock − 5, 1)` — so all 0..1 frame of sampling delay
lands in `dt`, and the fixed −5 ticks (~10 ms at 150 BPM) exists to cancel the
mean of it. The port timestamps the real key edge (section 15) and applies no
compensation: doing both would bias every press early by the full 10 ms, and
doing neither reproduces the original's *net* timing to within a couple of
milliseconds. `EZ2_PRESS_COMP_TICKS` in `score.h` keeps the number written down
for a port that ever goes back to frame-sampled input.

The original also keeps **two** six-entry window tables per lane, one for early
presses and one for late, so asymmetric windows are expressible. The shipped
call fills both from the same four numbers, so one set is modelled.

### The score formula, transcribed rather than paraphrased

`ScoreKeeper::rank` @0x472b40 is matched, and it hands over the formula:

```c
total += 250 - (int)((log10(i + 1.0f) * 0.2f) * -250.0f);
```

Two details are load-bearing and both were transcribed, not rewritten:

* **The cast truncates toward zero; it is not a round.** The prior RE describes
  the formula as `round(50·log10(combo) + 250)`. They diverge from the *third*
  note — `log10(3)·50 = 23.856`, so truncation gives 273 and rounding 274 — and
  keep diverging. A play scored with `round()` would exceed the maximum
  `rank()` divides by.
* **`0.2f` is a float literal promoted to double**, not `0.2`. The difference
  is in the last places and is exactly enough to move a note that lands on an
  integer boundary.

### Two combo counters, and the matched code is what proves it

The first full-combo run came out at **101.02%**. Hold ticks were advancing the
same combo the score reads.

`rank()` divides the score by the log-sum over the **note count**, and its top
grade is `rate >= 100`. For a perfect play to land on exactly 100 — rather than
over it, which would make S4 reachable *with misses* on any chart containing
holds — the score must accrue `value(1..noteCount)` and nothing else. So hold
ticks cannot advance the scoring combo. `ez2_score` carries `combo` (scoring,
notes only) and `display_combo` (what the player sees, plus ticks); `max_combo`
follows the latter. A perfect play now scores exactly `ez2_score_max(notes)`,
which is the invariant the tests assert.

### The score ignores which tier you hit

A GOOD-only full combo scores identically to a KOOL-only one — both are 100%
and S4. That is not a bug and not an approximation: per-note score depends only
on the combo, and the tiers differentiate the **gauge**. It is also why the
second ladder exists. `rank()` has two: the 0..10 score ladder, and a 0..6
ladder on `(KOOL+COOL)/notes` selected by the mode flag at `0x1b2eb6c` — and
that second one is what actually distinguishes a sloppy full combo from a
clean one.

Which counter is which fell out of `commit` @0x472d10 and `rank()` together:
`rank()` reads `f18 + f14`, and `commit` folds `m_1d8` into `f18` and `m_1d4`
into `f14`, so the counter block at `+0x1c8..+0x1d8` runs **worst-first** —
MISS, FAIL, GOOD, COOL, KOOL. Derived rather than proven, from the ladder
having to improve as play improves plus the prior RE independently calling it
`(kool+cool)/notes`.

### A correction to the prior RE

It states that every note in every shipped chart carries long-note sub-type 0,
and concludes a single fixed tick grid is faithful. **155 notes across 19
charts carry a non-zero sub-type** — 6 (×132), 20 (×22) and 3 (×1). Only 1..5
select a non-default divisor, so the conclusion survives almost intact: exactly
**one note in the entire game** gets a different tick grid. The premise is
wrong, the table is honoured anyway.

### `ez2judge` — the harness the exactness claim will be checked with

    ez2judge --exe UNPACKED.exe CHART.ez [--offset MS] [--jitter MS] [--seed N]

Presses every note at its own time plus an offset and a deterministic jitter,
then reports the tally, combo, gauge, score, rate and both grades. No window,
no audio, no input device.

On its own it proves *internal consistency* — a perfect play scores exactly the
theoretical maximum and grades S4; a 30 ms offset turns a real chart into 1,939
GOODs; 45 ms turns it into 1,939 MISSes and an empty gauge. Against a recorded
play on the real cabinet it would prove *equivalence*, and that is the next
step for this path.

### Open, and deliberately not guessed

* Whether the window comparison is `<=` or `<`. Nothing seen distinguishes
  them; `<=` is a choice, marked as such in the code. It can only matter for a
  press landing on an exact millisecond.
* `EZ2_J_FAIL` is a defined tier that the shipped configuration cannot produce,
  because the graded zone ends exactly where the GOOD window does. Its counter
  and gauge rate exist in the data, so it is modelled and never fires.
* The difficulty scale the original multiplies each gauge delta by
  (`difficulty_scale[GameLevel][judgment]`, @0x49e3c8) is all `1.0` in Final EX
  — present in the binary, neutral in effect. Not modelled; noted in case
  another build ships a real table.

---

## 14. Modes and lane mapping

`port/ez2/mode.c`, `port/tools/ez2lanes`. The last dependency between the data
layer and a playable 5-key chart.

### Why this is worth verifying rather than trusting

A wrong lane set does not crash and does not look obviously wrong: notes on a
track the mode does not claim simply become backing audio, so the chart plays
with a lane missing and everything still "works". The prior RE caught the
ClubMix case exactly that way and warns about it in bold.

So `ez2lanes` walks every chart, groups by the mode in its filename, and
measures which tracks actually carry playable notes. The prior RE derived its
sets from about sixty charts per mode; this reads all 12,361.

**Result: 12 of the 13 modes verify clean** — every declared lane is used by a
majority of that mode's charts, and no unclaimed track is.

### The roster is the binary's own

`@0x414e50` scans a 0x20-stride name table at `0x487278` and stores the matched
index at `0x1b2e548`; that index is what every per-mode switch in the binary
uses. Read out: exactly 13 entries, 0..12, ending at `CV2Mix`. AndromedaMix is
genuinely *not* in the table, confirming the prior RE's asterisk.

The mode is the **filename prefix**, never inferred from lane count. The game
builds paths with `"%s%dp-%s.ez"`, so `ez2_chart_id_parse` splits on the first
`<digit>p-` rather than matching a prefix list — which is what stops
`RadioMix` swallowing `5RadioMix`, `10RadioMix` and `14RadioMix`.

**`RadioMix` with no number is 7-key**, not 5. The number in the name is the
layout the channel runs on, and the plain one has none.

### The ClubMix gotcha, now confirmed at scale

ClubMix's right hand is tracks **12–16** — the two 2P effectors and 2P keys 1–3
— not 14–18. Measured over 1,150 ClubMix charts:

```
track  3   4   5   6   7   10  11  12  13  14  15  16  17  18  19
       99% 99% 100 99% 99% 92% 96% 99% 99% 100 99% 100  0%  0% 83%
```

Tracks 17 and 18 carry notes in **zero** charts. A 14–18 mapping would have
silently dropped every effector note into the backing audio on every club
chart in the game.

### CV2Mix has no single lane set

Left unmodelled, and that is a finding rather than a gap. Its 112 charts
measure:

```
track  3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20
       93% 91% 92% 92% 92% 35% 33% 76% 66% 43% 44% 43% 43% 44% 19% 20% 35%  9%
```

Nothing clears a threshold while its neighbours sit near zero — that is a
*superposition* of several layouts, not one. Consistent with CV2Mix being a
special compilation that "needs its own playfield". `ez2lanes` reports it as
unmodelled and prints the measurement rather than inventing a set.

### Five dead files

`5keymode1p-jmj*`, `scrachmix1p-5Energy` and `10stmix1p-10lovesong` carry mode
names the game's own table does not contain, and each sits beside a correctly
named equivalent (`5keymix1p-jmj*` and so on). Since the game builds its paths
*from* the table it can never request them. They are leftovers, and are
reported rather than aliased.

### The correction this immediately produced

`ez2judge` was judging every note in the chart. With the lane set applied, the
same 5-key chart goes from **1,939 notes judged to 296** — the other 1,643 are
backing audio. Note count feeds the score maximum and therefore the rate, so
that was a 6.5x error in the denominator of the one number the "must be exact"
column is about.

The cross-check that it is now right: judged + backing is 1,939 for that
chart in *every* mode, while judged alone scales with the lane count — 296 for
5 lanes, 394 for 9, 681 for 13.

### What is verified and what is not

The lane **sets** are verified against the library. The lane **order** — which
track sits in which visual column — is not: it lives in `KEZPlayer`'s note
spawn, which is not reconstructed. The order in `mode.c` is the sensible one
(left to right, 1P before 2P) and is enough to play a chart, but a lane
*index* should not be treated as authoritative until that function is decoded.

---

## 15. `ez2play` — the loop, and the first thing that is a game

`port/tools/ez2play.c`, plus `platform/gl/gl_audio.c` and `gl_input.c`.

Everything before this was a tool. This is a chart you can play: notes scroll,
keysounds fire, presses are judged, the gauge moves and a grade comes out. It
adds no new knowledge — it is the loop between the pieces the earlier
milestones built.

    ez2play --exe UNPACKED.exe CHART.ez --root ASSETS [--auto] [--fast]

Keys are S D F J K (plus L and `;` on 7-lane layouts), A/Q for the scratch and
space for the pedal.

### Time comes from the audio device

A rhythm game's "now" has to be what the player *hears*. `ezAudioTimeMs` counts
frames the device has actually consumed — mixed, minus what is still queued —
so a stall in the render loop moves the picture and never the music. Measured
against the wall clock it tracks to within 20 ms over 5 seconds.

That subtraction of the queue is not cosmetic: it is the difference between
"the game asked for this sound" and "the player heard it", and at a 20 ms
buffer, skipping it is 20 ms of judgement error on every note.

There is a fallback, and it exists because of a real failure. With no audio
device the clock sits at zero forever, no note ever arrives, and the first
headless run scored nothing while reporting success. So: the audio clock when
it is moving, the wall clock when it is not, and a virtual clock under `--fast`
so a whole chart is checkable in a second instead of two minutes.

### Input is deliberately more precise than the original

Polling a level once a frame quantises every press to 16.7 ms at 60 Hz. The
KOOL window on a real chart is 6 ms, and on some charts 4. A frame-quantised
port cannot award a judgement it is unable to measure — timing would be decided
by where a press landed relative to a vsync rather than by the player.

So the seam gained `ezInputEvents`: timestamped press/release events alongside
the engine's polled level view, which is unchanged. **This makes the port more
precise than the cabinet rather than equal to it, which is a divergence inside
the "must be exact" column**, and it is recorded here rather than buried. If
matching the original's quantisation ever matters more than matching the
player's intent, this is the knob.

### Keysounds

`EzSound` is one voice, and playing it again RESTARTS it rather than layering a
second copy — DirectSound's behaviour for a single buffer, and what a chart
that retriggers a sample expects. Mixing is a callback over the active voices
with soft clipping, since a dense chart can put dozens on one frame and a
wrapped sample is an audible click.

A judged note still plays its keysound even when missed: in EZ2AC the notes
*are* the music, so a missed note must not punch a hole in the song.

### What it verifies

`--auto` plays perfectly, which makes the whole loop testable without a person.
`ctest`'s `play_autoplay` runs a full chart headless and requires the run to
end with every note KOOL and the score exactly equal to `ez2_score_max(notes)` —
the same invariant `test_score` asserts on synthetic input, now driven through
chart loading, lane mapping, the tempo map, the clock and the judge. It skips
itself when the game is absent.

Verified interactively too: 343 notes on a real StreetMix chart, 100.00%, S4,
gauge full; and with a real audio device the clock tracks the chart correctly
past the first note at 12.6 s.

### Honest gaps

* **The HUD font is a 3x5 bitmap** covering digits, upper case and a few
  symbols. The real `drawTextf` strings are CP949 Korean as well as ASCII and
  will need a real font with a real encoding; this covers a HUD and says so.
* ~~**Lane order is still the unverified part.**~~ **Closed 2026-08-09**, and
  it was never in `KEZPlayer` at all: lane order is DATA, in each mode's
  plain-text `.gds` descriptor, read by `loadSlotSection` @0x420f60 into the
  per-player TrackList where the array index is the lane. `ez2/gds.c` parses
  it and both `ez2play` and `ez2judge` now use it — including the key→lane map,
  which the descriptor's `Key=` column gives away for free. See
  `docs/gds-slots.md`.
* **Holds are judged at the head only**, which is the documented model, but
  their tick combo is credited at the head rather than accruing over the hold.
  The total is the same; the combo curve during a hold is not.
* No BGA, no song select, no result screen, no scratch direction handling
  beyond mapping both keys to one lane.

---

## 16. Text — the game ships its own font, and the decomp says how to read it

The port's text was a 3x5 built-in with no lower case. It did not have to be:
`system/common/fontEn.dat` and `system/common/fontkr.dat` are in every install,
and `KGraphics::drawText` @0x406f50 plus the four glyph blitters under it
describe them completely. `ez2/font.c` reads both; `port/ez2/font.h` carries the
full derivation. The short version:

| | fontEn.dat | fontkr.dat |
| --- | --- | --- |
| size | 2,560 | 75,200 |
| glyphs | 128 | 2,350 |
| cell | 8x16, stride 1 | 16x16, stride 2 |
| advance | per glyph, 128 ints after the bitmap | fixed **15** |

None of that is measured off the files. The sizes are the two `read` calls of
`loadAsciiFont` @0x407120 and the single `0x125c0` of the Hangul loader
@0x407080; the files then happen to be exactly those sizes, which is the check
rather than the source.

**2,350 is the KS X 1001 precomposed Hangul syllable count**, and the index
arithmetic at @0x406f9c confirms it — cl folds the whole thing into one `lea`:

    lea -0x4141(%edx,%ecx,2)      ecx = 47*lead, edx = trail
    => glyph = (lead - 0xB0) * 94 + (trail - 0xA1)     since 0xB0*94 + 0xA1 == 0x4141

So the font covers **syllables only**. No Hanja, no symbol rows, no Latin
supplement — which also means a port cannot render a Korean string the game
could not.

**The column order is LSB first.** The blitters test `bits & tbl[col & 7]`
against the table at 0x487c64, which is `01 02 04 08 10 20 40 80`. Getting this
backwards mirrors every glyph and still looks like text, so it has its own test
rather than an assertion in a comment.

Two of the original's bugs are reproduced deliberately, because a port that
renders differently from the game is wrong even when it renders better:

1. A lead byte **above 0xCA** jumps straight to the advance at @0x406f8a,
   past the `add $1,%ebx` that eats the trail — so Hanja does not blank, it
   desynchronises the rest of the line.
2. A lead byte **below 0xB0** yields a negative index the blitters reject
   (`test ebx,ebx; jl`), but the trail is already eaten and the pen still moves
   15px. KS X 1001's symbol rows come out as correctly-sized blanks.

### The seam takes a decoder, not a path

Reading game data is not the platform's job, so `platform.h` grew
`ezSetTextFont(EzTextDecode, void *ctx)` rather than a font-loading call. The
application supplies a function that decodes one character and hands back a
bitmap, a stride and an advance; both backends walk that. `ez2/font.c` already
had exactly that shape, because it and the seam hook were written off the same
four blitters. Without an installed font the 3x5 fallback stands, which is what
keeps `ez2gfxdemo` working with no game tree at all.

### Song titles are artwork, not text

They were never going to come out of this. Song select's preload loop
@0x4361a0 — records at `this+0x5e3c8`, stride `0x374`, count at `this+0x80` —
does five things per table entry, and only one of them is text:

| what | address |
| --- | --- |
| `sound\%s\` — the song folder | 0x48fd0c |
| `loadSongInfo` | 0x431660 |
| `readRanking` x5, one per tier | 0x430cf0 |
| `system\discsmall\%s.bmp` — the disc art | 0x48cfd0 |
| `system\songname\%s.bmp` — **the title** | 0x48cfb8 |

The `%s` is one string in all five, so the title asset, the disc art and the
ranking file are keyed alike — which is also why `ez2/ranking.c` keying by the
song table's key was right. The pictures are 256x32, and every one of the 436
keys on StreetMix, 5keyMix and ClubMix resolves to one.

**This is why twelve of the thirteen `song.bin` tables leave the name field
empty.** Nothing reads it: the key names a file, and the file has the title
drawn in it. The empty field was the clue, not the obstacle.

CV2Mix is the exception in both halves, gated on `g_1b2eb6c` @0x43625d. Its
directory is `system\CV2Mix\SongName\` (@0x4937a8) and its `%s` is the
record's *other* string, passed through @0x430f50 first — which tests for a
dash at len-3, then len-4, then len-5, and truncates at the first hit. That is
consistent with the table: CV2 is the one mode whose key is a number (`92`)
and whose name field carries the chart (`11ambit-5o1`). All 54 of its names,
stripped, hit one of the 58 shipped assets.

A second family, `system\SongName\%s.bmp` (capital S) @0x496804, belongs to
three other screens — 0x452676, 0x454b4b, 0x4685d2 — result and ranking rather
than select.

### One thing the port needed for this

The game asks for `.bmp` and the tree ships `.abm`. `ezTextureLoad` already
decoded whatever it opened as `.abm`, but `ezPathResolve` walked the literal
name, and it had never mattered because every texture the port loaded until now
was named `.abm` by our own code. The moment a path comes from the *original's*
own format string it does. `ezPathResolve` now retries with `.abm` when a
`.bmp` fails to resolve — a fallback, so a real `.bmp` would still win.

---

## 17. The BGA — one half is layer animation, the other is Windows Media

The roadmap had this block as "152 `.ogv` (Ogg Theora, a solved problem) plus
`.spv`, format unknown". Both halves of that were wrong, and the correction
went in opposite directions.

### `.spv` is ASF/WMV, and it is the only one the game asks for

A `.spv` opens with `30 26 B2 75 8E 66 CF 11 A6 D9 00 AA 00 62 CE 6C` — the ASF
header GUID. Every `.spv` in the tree probed:

| | |
| --- | --- |
| container | ASF |
| codec | wmv3 ×112, wmv2 ×30, wmv1 ×20 |
| size | 256×256 — **all 162** |
| frame rate | 30 ×86, 24 ×41, 60 ×34, 29.97 ×1 |
| audio | none, in any of them |

256×256 is corroborated from the other side: the binary carries
`video_blank_256.bmp` @0x4881a8 and `System\Common\Video_Blank_256.bmp`
@0x4937dc as the placeholder texture (and a `_512` sibling for a path nothing
has been seen to take).

**The binary references `.spv` ten times and `.ogv` zero times** —
`%s\Video\%s\%s.spv` @0x48e27c for a song's BGA, and eight `bg\*.spv` for the
screens. The 162 `.ogv` files in the tree are not the game's; something else
put them there. So a port cannot reach the video by taking the easy Theora
path: the format is WMV1/2/3, which in practice means libavcodec.

That decision has been taken: **the project depends on ffmpeg**, and
`port/media/video.c` decodes `.spv` through libavformat/libavcodec/libswscale.
Verified against the shipped tree — `11alegend.spv` reports `wmv3 256x256,
30.00 fps, 131.7 s` and puts a frame on the screen at any requested time.

Three things about how it is wired:

* **It is a separate target, `ez2media`.** `ez2core` has no dependencies on
  purpose — that is what keeps the cipher, the chart parser and the scoring
  testable on a machine with no graphics and no libraries — so the one piece
  that cannot be written that way does not live there. Only the tools that
  show video link it, and the build says `no ffmpeg - building without video`
  rather than failing when it is absent.
* **The API advances, it does not seek.** A BGA is played forward against the
  song clock and never scrubbed, so `ez2_video_advance(v, t)` decodes forward
  to a time and `ez2_video_rewind` exists for the screens that loop. Seeking a
  WMV backwards is exactly what the codec is worst at, and nothing in the game
  asks for it.
* **The seam grew two calls**, `ezTextureFromPixels` and `ezTextureUpdate`.
  A decoded frame corresponds to nothing on disk, and `ezTextureUpdate` is the
  per-frame call, so it is `glTexSubImage2D` — it must not reallocate.

**Licence.** ffmpeg is LGPL-2.1-or-later, or GPL-2.0-or-later when built
`--enable-gpl`. `port/` is GPL-3.0-or-later, which is compatible with either,
so no distribution question arises from the dependency itself.

### `.str` layer animation renders, and it needed no dependency at all

The other half was already parsed and simply never drawn. `ez2bga` draws it:
load a clip, resolve its textures beside it, tick the keys, composite the
layers. Three things had to be right, and two of them were wrong first:

1. **`frametype` 0 is a pose and `frametype` 1 is a per-frame DELTA.** Every
   layer interleaves type-0 keys with type-1 records carrying a zero quad, a
   zero position, an alpha of zero and sometimes *negative* colour components
   — the tell that they are not poses. Take the nearest key without telling
   them apart and you draw a type-1 record, whose zero alpha renders the whole
   clip black. That is exactly what the first build did. The second build
   drew the poses and lerped the position between them; the engine's tween
   tick @0x403940 (matched, `src/accessors.cpp`) does something else: a pose
   is copied into an accumulator and the delta that shares its frame is then
   ADDED to it once per frame, every channel — position, uv offset, quad,
   angle, colour and alpha. Position lerp happened to agree because the
   authoring tool writes the delta as the difference over the span; every
   fade, pan and zoom in the library is a delta and was not drawn (the
   owner saw Feel's fades snap, 2026-09-04). `scene/bga.c` now keeps the
   engine's state per row and ticks it — see the header comment there.
2. **`quad[8]` goes round the quad, not across it.** It is all four x then all
   four y (which `str.h` already flagged), and a real clip reads
   `x = -64, 64, 64, -64` against `y = -64, -64, 64, 64` — TL, TR, BR, BL. A
   triangle strip wants TL, TR, BL, BR. `ezDrawQuad` takes the loop and does
   the permutation, so no caller repeats it.
3. The UV rect is an offset and a **size**, as `str.h` says.

`ezDrawQuad` is a new seam call, and it earns its place: the format carries
four explicit corners and a rotation (an index into the engine's 1024-entry
table, not radians), and an axis-aligned blit cannot express either.

### One more thing this fixed

`ezTextureLoad` applies the `.bmp` → `.abm` rule to **literal** paths now, not
only to ones resolved against the asset root. A `.str` names its textures
`drum0000.bmp`, and a tool handed a full filename resolves nothing against the
root — so the rule has to sit behind both doors or it only works for paths the
game itself built.

## 18. The note order — ten options, and the value is the icon index

Read 2026-08-10, closing the last correctness item in Block 7. The full
working (the addresses, the two wrong guesses, and what is still unread) is in
`PORT-ROADMAP.md` under "The note-order options"; this section is the summary
a reader of the port needs.

**Why it is in the *must be exact* column.** A note-order option decides which
lane a note is in. Replay the same chart with the same inputs under a wrong
shuffle and the run diverges — the same standard as judgement timing and
scoring, and for the same reason.

**The chain, and where it was found.** The menu value is not what gameplay
sees:

    g_37dcecc (0..10)                  the value the player selects
      -> icon at songSelect+0xbfee8+4v @0x43858e, which NAMES it
      -> one effect id per arm         @0x448b52, `g_effector[id] = 1`
      -> the replay @0x424711          walks the flags in order
      -> the dispatcher @0x422f20      switch (id) -> a transform

Reading the icon table is what named the values, because the value indexes it
directly: `RANDOM_OFF RANDOM SRANDOM PS FR MRANDOM HRANDOM MIRROR MIRROR_A KEY
SP`. Reading the *effect ids* is what tied the names to behaviour, because
three pairs of options are the same function with one flag flipped.

| option | id | what it does |
| --- | --- | --- |
| RANDOM | 3 | sixteen lane swaps over the whole chart |
| MRANDOM | 5 | the same, with scratch and pedal **included** |
| FR | 3 | RANDOM constrained to one half (ClubMix, 10RadioMix) or to the inner lanes (7StreetMix, RadioMix) |
| SRANDOM | 4 | re-shuffled every **beat** (`ticks_per_measure / 4`) |
| HRANDOM | 23 | the same, with scratch and pedal included |
| PS | 29 | re-shuffled every **eight measures** |
| MIRROR | 22 | reverse the row; on 7-key modes keys and scratch/pedal separately; on 14-key modes the whole row |
| MIRROR_A | 31 | reverse keys, scratch and pedal as one list |
| KEY | 25 | reverse the keys only |
| SP | 26 | swap the scratch with the pedal |

**The lane record is `.gds` data the port already had.** Each transform selects
lanes by **control id** and moves their **track** — the two fields of the same
16-byte record the mode descriptor fills (`docs/gds-slots.md`). So
`ez2_lane_map_from_gds` is the whole of the extraction: `Key=` is the id the
veto and the mirrors test, `SongTrack=` is what moves. With no descriptor the
ids are absent and the four mirrors correctly do nothing rather than mirroring
a guess; `MIRROR` still works, because it alone goes by lane position.

**And the menu is per mode.** Each mode's effector key handler (the arms of
the jump table at 0x448e00) cycles the value through its own skip list, so the
options a mode offers are whatever its ring visits: CV2Mix has two, EZ2CATCH
and ScratchMix four, 5KeyMix five, and only the two 7-key modes have all ten.
`ez2_order_option_available` / `_next` carry it, computed by walking the ring
so it cannot drift from the cycler. `FR` is offered by exactly the four modes
whose reader gives it a real strategy, which is what makes the setter listing
two more of them a dead branch rather than a bug.

**Two details worth keeping.** The windowed options stretch a window rather
than cut a **hold** in half (@0x42184c), and the swap is *wholesale* — a note
whose counterpart track is empty at that tick still moves, which is why
`ez2_note_order_swap` rebuilds both tracks instead of exchanging matched
pairs. Both are the kind of thing that reads as a chart bug rather than a port
bug when it is wrong.

**Try it:**

    ez2play  --exe … --root … CHART.ez --order MIRROR
    ez2judge --exe … --root … CHART.ez --order SRANDOM --order-seed 7

Swept across the library: a perfect play scores identically under all ten,
which is the invariant that matters — a shuffle relocates notes and must
neither create nor destroy one.
