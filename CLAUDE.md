# EZ2DECOMP — byte-matching decompilation of EZ2AC

Goal: reconstruct C/C++ source that recompiles into a binary **byte-identical** to the
original EZ2AC executable, verified function-by-function with `reccmp`. This is a
*matching* decomp (like the `isle`/LEGO Island project), not a rewrite. The sibling
`../EZ2REWRITE` project is a loose reimplementation and a source of RE knowledge only —
its code is **not** byte-matching and must never be copied in as-is.

## The original binary — handling rules (NON-NEGOTIABLE)

- The original binary, DLLs, and game assets live in `./original/` and are **gitignored**.
  The current matching target is `original/EZ2AC_Unpacked.exe`.
- **Never commit** anything from `./original/`. Never paste its raw bytes, byte arrays,
  hexdumps, or extracted asset data into source files, commit messages, or docs.
- Referencing an **address** (`0x00410a40`), a decompiled **structure/offset**, or a
  disassembly **listing** in a comment or doc is fine — that is analysis, not distribution
  of the copyrighted bytes. Embedding reconstructed data tables that were copied verbatim
  out of the binary is **not** fine; derive them or leave them as an external-load.
- Do not read the binary into any tool whose output gets committed. Ghidra projects,
  `strings` dumps, and raw extracts stay out of the repo (gitignored).
- **`objdump -D` output IS a hexdump** — it prints each instruction's byte encoding,
  so a per-function listing under `scratchpad/` is raw `.text` and must never be
  committed. `.gitignore` covers `*.asm` and `scratchpad/*.txt`; **never `git add -A`
  after disassembling** — stage the source files by name, or check `git status` first.
  (23 such dumps were swept in by `git add -A` in session 13 and had to be purged
  from history with `git filter-branch`.)

## Definition of done: 100% reccmp match

- A reconstructed function is **not done** until `reccmp` reports **100%** match against the
  original at that function's address. 99% is not done.
- After changing any function, **run the match script and paste the diff** into the working
  log / PR. Never claim a match without showing the reccmp output that proves it.
- Prefer small **leaf functions** first (no or few calls into other game code). Build the
  matched call-graph outward from the leaves.

## Toolchain (period-correct only) — CONFIRMED

Identified from the PE (Rich header build **3077**, linker **7.10**, `/GS` release codegen)
and **verified**: our `cl.exe` reports *"Version 13.10.3077 for 80x86"*, exactly matching
the original's Rich-header build. It compiles + writes a PDB under Wine.

- **Compiler:** Microsoft Visual C++ 7.1 — Visual Studio .NET **2003**, `cl.exe`
  **13.10.3077** (`_MSC_VER == 1310`). Installed at `tools/msvc71/{bin,include,lib}`.
- **Linker:** `link.exe` 7.10 (same dir).
- **CRT:** statically linked, multithreaded — `/MT`. The original imports no CRT DLL. The
  VC7.1 static CRT libs (`libcmt.lib`, `msvcrt.lib`) are present in `tools/msvc71/lib`.
- **Security:** `/GS` stack cookies are present in release code — keep `/GS` on.
- **DirectX:** DX8 SDK installed at `tools/dx8sdk/{include,lib}` (d3d8/ddraw/dsound/dinput).
- **Host:** runs under **Wine** (`wine-11.13`, new-WoW64) in a dedicated prefix at
  `tools/wineprefix`. Invoke `cl.exe`/`link.exe` by absolute path; set Windows-style
  `INCLUDE`/`LIB` (Z: drive maps `/`).
- **THE BUILD MUST NEVER OPEN A WINDOW.** cl 13.10 traps on exit on some TUs
  ("Unhandled illegal instruction at address 00402B0A"), which is harmless -
  the object is written and the exit code is 0 - but Wine's crash handler then
  launches whatever `AeDebug` names, and that process opens a window on the
  user's desktop and STEALS FOCUS, once per trapping TU. Four settings keep the
  toolchain headless; the registry two live in the gitignored prefix, so
  **reapply them if it is ever rebuilt**:

      wine reg add 'HKLM\Software\Microsoft\Windows NT\CurrentVersion\AeDebug' /v Auto     /t REG_SZ /d 0  /f
      wine reg add 'HKLM\Software\Microsoft\Windows NT\CurrentVersion\AeDebug' /v Debugger /t REG_SZ /d '' /f
      # ...and the same two under HKLM\Software\Wow6432Node\...
      wine reg add 'HKCU\Software\Wine\Drivers' /v Graphics /t REG_SZ /d '' /f

  `Auto=0` + an empty `Debugger` means nothing is launched (an earlier fix set
  `Debugger` to `cmd.exe /c exit`, which stopped the debugger HANGING but was
  itself the window that popped up). The empty `Graphics` driver makes a window
  impossible in this prefix at all - it is a build-only prefix, the game runs
  from its own tree. And `match.sh` / `scratchpad/zoo.sh` export **both**
  `DISPLAY=` and `WAYLAND_DISPLAY=` empty: an unset `DISPLAY` alone still let
  the spawned process reach the Wayland compositor (the `err:waylanddrv:`
  lines in the build log were the tell). A clean run prints the `wine:
  ...starting debugger...` line and NOTHING follows it - no `waylanddrv`
  chatter, no lingering `winedbg`/`cmd.exe` (`ps aux | grep winedbg`).

### Graphics API — CONFIRMED DirectDraw7 + Direct3D7

The original imports **`DDRAW` + `DINPUT`**, not `d3d8.dll`. Prior `../EZ2REWRITE` RE that
said "Direct3D 8" only evidenced COM vtables + `SetRenderState(0x13/0x14)` D3DBLEND enums,
which D3D7 and D3D8 share. **Now settled by vtable INDICES** (include/icom.h): the graphics
singleton holds `IDirectDraw7` at +0x28 and `IDirect3DDevice7` at +0x30, and six independent
slots land on the right D3D7/DDraw7 names —
DDraw7 slot 9 `EnumSurfaces` (countSurfaces @0x408af0 passes 0x11 =
`DDENUMSURFACES_ALL|DDENUMSURFACES_DOESEXIST` + a per-surface callback) and slot 25
`RestoreAllSurfaces`; device slot 5/6 `BeginScene`/`EndScene` (m405e10 @0x405e10 /
endFrame @0x405e30), slot 20 `SetRenderState` (setBlend @0x40a490, and setFade @0x40b080
sets ZERO/SRCALPHA), slot 35 `SetTexture` (m408c80 @0x408c80 unbinds stages 0..3). D3D8's
device has none of those at those indices.
Audio = **DirectSound**; input = **DirectInput (polled)** — both match the imports, and
Obj40C180's releaseAll @0x40c320 unacquires (slot 8) + releases (slot 2) two DI devices.
The cabinet's own I/O board is reached with **IN/OUT on ports 0x100..0x105** (`_inp`/`_outp`
from `<conio.h>` give the original's `mov dx,port; in al,dx`), gated on the 0x4200f4 flag.

### Build backend — VERIFIED WORKING

Full pipeline proven: `cl.exe` compiles → `link.exe` links (with `msobj71.dll` from VS2003
Disc 2 + `mspdb71.dll`) → produces `EXE` + `PDB` → the EXE runs under Wine. This is the
complete reccmp loop backend. `msobj71.dll` is installed in `tools/msvc71/bin`.

### Platform SDK — INSTALLED

Win32 SDK is the **Windows Server 2003 SP1 PSDK** (last fully VC7.1-compatible), installed at
`tools/platformsdk/{Include,Lib}` (1277 headers, x86 import libs). `#include <windows.h>`
compiles clean with our cl. `match.sh` adds it to `INCLUDE`/`LIB` automatically.

### Language / style constraints

- **No modern C++.** Target C++98/03 as VC7.1 accepts it. No C++11+ features, no `auto`,
  no range-for, no lambdas, no `nullptr`, no `<cstdint>`.
- **No STL unless the binary demonstrably uses it.** Match the original's own containers /
  the custom `K`-prefixed engine classes. Only introduce `std::` when a symbol/codegen
  pattern proves the original did.
- **A FUNCTION WHOSE ARGUMENTS ARRIVE IN NON-STANDARD REGISTERS IS `static`,
  AND IT MUST BE RECONSTRUCTED IN THE SAME TU AS ITS CALLERS.** cl 13.10 gives
  a `static` function that is not address-taken its OWN register convention -
  verified in the zoo: `static int helper(T*, float, int*, int)` called twice
  from one TU receives the pointer in edx, the out-pointer in edi and the count
  in ecx, with `ret` and no stack cleanup for them. The original goes further
  and passes floats in xmm registers (0x409a30 reads its float argument from
  xmm1, which no documented convention does).
  Consequences: such a function can NEVER match while it is declared normally,
  its exact register assignment is chosen per-TU by cl, and the callers have to
  be reconstructed alongside it. reccmp still resolves it - a static function
  gets an ordinary PDB symbol - so only the TU grouping is the constraint.
  The 0x409660 / 0x409a30 cluster is the known example; do not attempt either
  in isolation. **AND IT IS A WHOLE TU, NOT A PAIR** - see
  wip/mesh-tu-409000.md, which maps 0x409200..0x40a400 as one translation
  unit: two ordinary entry points (0x409d90, a KGraphics method two directors
  call, and the function around 0x40a0ba) over nine or ten `static` helpers
  that take arguments in ecx/edx/esi/edi/xmm and share file-scope data at
  0xd49cb8..0xd49dcc. 0x409330 alone takes a pointer in ecx AND three cdecl
  stack args the CALLER cleans. Roughly 4.6 KB that matches together or not at
  all; do not spend a short session on any single member.
- **A `__stdcall` FREE FUNCTION AND A `__thiscall` MEMBER WITH ONE STACK
  ARGUMENT COMPILE TO THE SAME BODY** - both take the argument at `[esp+4]`
  and both end `ret 4` - so a member that never touches `this` is
  indistinguishable from a free function UNTIL ITS CALL SITE IS
  RECONSTRUCTED. The tell is a `mov ecx,<object>` right before the `call`
  that the reconstruction does not emit: one extra instruction, everything
  else byte-exact. Move the definition into the class (it stays 100%, the
  body does not change) and the caller gains the load.
  (release @0x40dca0, modelled as `__stdcall release40DCA0` until
  m40e070 @0x40e070 was written; 93.75% -> 100% and the callee held.)
  **AND THE MEMBER MAY TAKE NO ARGUMENT AT ALL** - a `__thiscall` member with
  an EMPTY parameter list and a `__cdecl` free function of no arguments are
  also the same bytes, so a list-walking helper that never touches `this`
  matches at 100% either way and only its call site can tell. Two of these
  were sitting in src/accessors.cpp with the tell already written into the
  callers' comments: rebaseListC @0x45d1c0 (GFMainGameDirector) and
  rebaseListB @0x46c020 (CatchMainGameDirector). Declaring each on its class -
  the definition does not move and does not change - gives the caller its
  `mov ecx,esi` and keeps the callee at 100%. It was the LAST instruction
  wrong in CatchMainGameDirector::update2 @0x46e550 (98.53% -> 100%).
- **SWEEP FOR THIS WHOLESALE: `scratchpad/ecxsweep.py`.** The rule above is
  usually met one function at a time, when a call site you are writing wants a
  `mov ecx` your reconstruction does not emit. It is cheaper to ask for the
  whole tree at once: walk the original's .text, find every `call` to an
  address annotated as a FREE function, and report the ones where EVERY site
  loads ecx in the instruction immediately before. Seven of 109 came out, and
  one - `runGameOver` @0x415d70 - had been declared free with a comment
  explicitly asserting its call sites do not set ecx. All five do.
  Two filters or it lies: a `static` helper legitimately takes arguments in ecx
  (the worm-mesh TU's helpers are not defects), and a caller already at 100%
  clears its site, since our build must be emitting that load for its own
  reasons. **BUT RESOLVE THE ENCLOSING FUNCTION BY ITS REAL END, NOT BY THE
  NEXT ANNOTATION.** Bisecting the annotation list put two sites inside matched
  functions and read as two clean false positives; both were wrong -
  `~GFBackGround` ends at 0x45c5ea and its "site" is at 0x45c642, m439dc0 ends
  at 0x439efa and its site is at 0x43a0aa, so both sites are in unannotated
  neighbours and nothing cleared them. An over-long span turns a real finding
  into a dismissal, which is the expensive direction.
- **Calling conventions matter for matching.** Member functions are `__thiscall` (`this`
  in `ecx`). Free functions default to `__cdecl`; use `__stdcall`/`__fastcall` only where
  the disassembly shows it. Getting the convention wrong changes prologue/epilogue and
  argument handling and will never reach 100%.
- Match the compiler's codegen, not "clean code": preserve struct layout/padding, member
  order, global order, literal-pool ordering, and `/O` optimization shape. If a match is
  off by an instruction, the fix is usually a source-level change (order, type, `const`,
  inline hint), not a compiler flag.

## Prior art & reusable analysis

- The sibling `../EZ2REWRITE` project holds extensive RE (functions, globals, structs, the
  fully-solved `.ez`/`.ezi`/`.str`/`.scr` formats and cipher). Its `reverse-engineering/*.md`
  and `bible/` are knowledge sources; its **code is a loose reimplementation, never
  byte-matching — do not copy it in**.
- A ready Ghidra project exists at a local Ghidra project (not included) with decompiled dumps
  (`*_decomp.txt`) + scripts, plus targeted scripts in local Ghidra scripts (not included). Reuse it rather
  than re-analyzing from scratch. (Ghidra 12.1.2 at `tools/ghidra_12.1.2_PUBLIC`.)
- **Address transfer is 1:1.** Prior labels come from `EZ2AC_NO_ANTIDEBUG_Unpacked.exe`; our
  target `original/EZ2AC_Unpacked.exe` differs by only **12 bytes, all inside the packer stub**
  (`.text`/`.rdata`/`.data` are byte-identical), so every prior `FUN_`/`DAT_` address applies
  verbatim at image base `0x400000`.
- The target is a **packer-dumped** PE (residual `.l33t` + stub sections, bogus entry point).
  We match individual functions at their VAs — we do **not** reconstruct the packer or aim for
  whole-file byte identity. "Byte-identical" == identical `.text` per function, which is what
  reccmp checks.
- Good first (leaf/near-leaf) targets from the prior RE: decrypt `FUN_00410a40`, rank ladder
  `FUN_00472b40`, vol/pan setters `FUN_0040f4a0`/`FUN_0040f550`, note converter `FUN_00410ef0`,
  LCG `rand` at `0x0047928f`.

## The build/match loop — WORKING

`./match.sh` builds every `src/*.cpp` + `harness/*.cpp` with the VC7.1 toolchain under Wine
(→ `build/EZ2AC_Unpacked.exe` + `.pdb`), then runs `reccmp-reccmp --target EZ2AC_UNPACKED`:
- `./match.sh` — overall match table (per-function % + Implemented/Accuracy/Progress).
- `./match.sh 0x0040b970` — full asm diff for that original address.

A function is done only when this prints **100%** for it. First proven match:
`Obj40B970::getField438 @0x0040b970` (100%).

### How to add a function
1. Find it in the prior RE / Ghidra dumps (the local Ghidra project's decompiled dumps); get its
   original bytes with `objdump -D -b binary -m i386 --adjust-vma=0x400000
   --start-address=0xVA --stop-address=0xVA original/EZ2AC_Unpacked.exe` (file off = VA−0x400000).
2. Write it in `src/` (types in `include/`), with `// FUNCTION: EZ2AC_UNPACKED 0xADDR` on the
   line above the definition. Member functions are `__thiscall` (default for C++ members).
3. `./match.sh 0xADDR`, read the diff, adjust the C (types/order/`const`/convention) until 100%.
   The recompiled binary's address differs from the original's — reccmp maps via the PDB symbol,
   so only the *bytes* must match, not the location.

### Build details / current flags (provisional, refine per function)
- `cl /nologo /c /O2 /MT /GS /Gy /GF /Gd /G7 /arch:SSE2 /EHsc /Zi /Iinclude` ; `link /debug /opt:noref /opt:noicf
  /incremental:no libcmt.lib`. `/opt:noicf` is required (stops identical functions folding, which
  would break per-address mapping); `/opt:noref` keeps reconstructed functions without referencing
  them from the harness. Flags locked against NoteConvert @0x00410ef0 (`/G7` for
  its partial-register movzx), loadEZI @0x411090 and the first 15 matches.
  `/arch:SSE2` is required (the aligned-frame prologue on stack-buffer
  functions); `/EHsc` proven by the original's 165 EH prologues - and the
  exception model is PER-TU: src/kengineapp.cpp builds /EHs (match.sh's
  eh_model picks it; if a function's only defect is an EH-state store around
  an extern "C" call, flip its TU's model first).
- Toolchain lives in `tools/{msvc71,platformsdk,dx8sdk}`; Wine prefix `tools/wineprefix`;
  reccmp in `.venv` (installed editable from `tools/reccmp-0.1.6`).
- **reccmp patches: see docs/reccmp-patches.md** (tools/ is gitignored -
  reapply if the install is rebuilt; patch files in reccmp-patches/). Twenty-one
  patches, in one line each: (1) latin-1 section names; (2) cross-line
  comment tracking in the parser (the `identities::` 75% gotcha); (3) float
  pool constants without a reloc table; (4) a register-relative displacement
  is a member offset, not an address - name it only at an entity's exact
  base; (5) ICF aliases (`folded`) symbolize under the winner's name;
  (6) ICF-aliased vtable slots score; (7) an imul/shift immediate is never
  an address; (8) SSE scalar float-pool constants resolve; (9) arithmetic
  immediates name entities only at exact base; (10) push immediates
  likewise; (11) mov-to-memory immediates likewise (vptr stores still
  resolve); (12) a displacement a few bytes BELOW an entity resolves as
  `name-k` (the `buf[len-3]` shape); (13) DISPLACEMENT_SHADOWED opt-in set
  for a tiny entity abutting a buffer; (14) PUSH_INTERIOR_ENTITIES opt-in
  (`&g_worm.center`); (15) DISPLACEMENT_TEXT_COLLISIONS .text windows,
  currently (0x41FB20, 0x4202E6) - a member offset landing in the
  original's own .text prints raw, because annotating one function there
  once broke fourteen Obj40C180 methods. (16)/(17) ARITH_ / DISPLACEMENT_INTERIOR_ENTITIES
  opt-ins: an `add reg,imm` / `[reg+X]` landing inside a listed global array
  (`g_gfrPos`, `g_textSlots`, `_iob`) names its interior on both sides;
  (18) a `push imm` right before `call __SEH_prolog` is `<SCOPETABLE>`;
  (19) BIASED_EXTRA: per-entity wider `name-k` biases (`__lookuptable-0x20`);
  (20) placeholder numbers count distinct replacement STRINGS (folded aliases);
  (21) a byte-replicated `mov reg, imm` (0x01010101, a memset fill) is never
  an address.
  If several unrelated byte-identical
  functions regress at once, check these patches are applied and `/base` is
  still on the link line before touching source.
  **And the structural half: `/base:0x11000000` in match.sh** - our image is
  based where no member offset (biggest: 0xe105c4) and no large immediate in
  the original's game code (783 literals >= 16MB were scanned; 0x11000000 has
  an 80MB clear span) can collide with a recomp entity. A colliding immediate
  is fixed by the base, never by a patch (mov-reg immediates are real
  interior refs as often as constants); re-run the literal scan before ever
  moving the base again.

## Matching cookbook - MOVED to docs/cookbook.md

The full cookbook (every recurring codegen pattern, with its war story and
the measured scores), the leaf-accessor quick-win categories, and the
big-repetitive-functions playbook now live in **docs/cookbook.md**. Read the
matching entry there BEFORE working a function whose diff fits a shape below;
add new rules there and mirror the headline here. One line per rule:

- Optimizer flags
- /GS buffer +8 pad under /arch:SSE2
- The exception model is PER-TU: /EHsc everywhere except src/kengineapp.cpp (/EHs)
- Switch arms are emitted in SOURCE order, and a jump table is compared as offsets into the function
- A FUNCTION WITH TWO NEAR-DUPLICATE HALVES DUPLICATES THEIR SHARED TAIL TOO
- A DIRECT jump table means every case had its OWN arm in the source
- Two switch arms that call the SAME address = two functions the original's linker ICF-folded
- AND A SWITCH WHOSE ARMS ARE ALL `call SAME; jmp SAME` YET STAY SEPARATE IS THE SAME THING AT SCALE
- A `// VTABLE:` pin costs nothing but STUBs
- `call A; jmp B` with NO `this` saved = A's body must be VISIBLE in the same TU
- A helper the original inlined at SOME sites lives in its own TU; write the inlined sites out
- PARENTHESES ARE A BARRIER TO cl's FLOAT REWRITES (negative-constant multiply, adjacent-constant fold)
- `return a == b;` lays the arms out differently from if/return 1/return 0
- SWEEP FOR WRONG CONSTANTS BEFORE MATCHING A FUNCTION (fltscan/divscan/cmpscan)
- A function that is nothing but `jmp rel32` is a one-line forwarder
- `volatile` LOCALS, when the original reloads a local at EVERY use
- Keeping a dead counter live forces indexed array stores
- AN IAT SLOT THE PACKER LEFT UNREBUILT STILL RESOLVES - BY OFFSET, FROM THE SURVIVING NAME TABLE
- A `call` to a 5-byte thunk in the CRT range can be a Win32 API called WITHOUT dllimport
- A constant-return "leaf" with NO callers is probably somebody's out-of-line return block
- `mov ecx,[g]; mov esi,ecx` (load into ecx, then copy) = the pointer was a PARAMETER of an inlined helper, not a local
- Frame-layout differences can be a SEMANTIC clue, not a layout problem
- A LOOP OVER RECORDS NEEDS THREE THINGS AT ONCE, and each one alone is worth a few percent
- SCOPE THE LATER LOCALS when an `__asm` helper's argument homes are too high
- A NAMED LOCAL AND A COMPILER TEMPORARY ARE PACKED DIFFERENTLY
- AND A NAMED LOCAL IS SPILL-PREFERRED, A TEMPORARY REGISTER-PREFERRED
- TWO FLOAT STORES THROUGH ONE MEMBER POINTER NEED A NAMED POINTER *AND* A NAMED VALUE
- A BARE `fistp` IS INLINE ASSEMBLY, NOT A CAST
- THE SCAN THAT FINDS A MIS-FOLDED ALIAS IS ONE LINE: any address carrying BOTH a `FUNCTION` and a `STUB` annotation
- A `folded` ALIAS MUST CARRY THE REAL BODY, NOT AN EMPTY ONE
- A BIG NEW GLOBAL CAN BREAK A BIASED REFERENCE IN A FUNCTION YOU DID NOT TOUCH
- A `// LIBRARY` MARKER RESOLVES BY NAME IN *OUR* PDB, so annotating a CRT symbol nothing in `src/` calls costs a point of Implemented
- A LOCAL WITH A CONSTRUCTOR RUNS IT AT ITS DECLARATION POINT, so where you declare it is a matching lever
- cl 13.10 DOES apply the empty-base optimization to a TRAILING empty base
- A run of `addInfo` calls is an OnRegisterClass, and that locates a whole unknown class
- A named local also changes `sub reg,1` into `add reg,-1`
- cl ATTACHES A SHARED BLOCK TO ITS *FIRST* PREDECESSOR; THE ORIGINAL ATTACHES IT TO ITS LAST - AND ONE OF THE TWO CASES IS STEERABLE
- Cross-jumped `return 0` tails want plain EARLY RETURNS, not a wrapped success block
- `x - y` as a subexpression vs. a named local changes which register the subtract targets
- STRING/LIBRARY/GLOBAL-by-name annotations
- Split `add esp` between two back-to-back cdecl calls
- Keeping an empty switch case's `sub/je` test
- do-while vs while
- DERIVE THE WHOLE FRAME AT ONCE, DO NOT GUESS BLOCK SIZES ONE AT A TIME
- `scratchpad/frame.py` RESTORES esp AT EVERY `call`, SO IT DRIFTS ON cdecl
- AND A HIGHER reccmp SCORE CAN BE THE WRONG CODE - THE MULTISET OUTRANKS THE PERCENTAGE
- A CALL-COUNT MISMATCH IS A MISSING BLOCK, AND IT IS OFTEN ONE YOU MIS-DECODED AS DATA
- Deriving a stack buffer's exact size from the frame
- `char` vs typed array picks the /GS buffer order
- A natural register spill can be irreproducible (known-hard)
- A run of calls on one global, with the global read ONCE, is an inlined helper taking the pointer
- AND TWO memcpy CALLS IN AN if/else HOIST THEIR SET-UP ABOVE THE BRANCH
- A ternary picking between two CONSTANTS goes branchless; two calls do not
- Branch layout
- N-way pick between calls: CONTIGUOUS tests then out-of-line arms is a grouped `&&`, not a chain
- Two int-to-float member stores through one pointer need two float LOCALS
- Virtual / vtable calls
- Magic-number division
- cl CONSTANT-FOLDS `a * k1 * k2` WHEN THE TWO CONSTANTS ARE ADJACENT
- Constant multiply
- cl CANONICALISES THE MULTIPLY ORDER INSIDE A FLOAT SUM - do not sweep it
- A FOUR-ITERATION LOOP IS UNROLLED UNLESS THE COUNTER IS EXPLICIT
- THE FLOAT/DOUBLE SPLIT IN AN x87 `sqrt`: put the cast OUTSIDE the product
- Global data references
- Float/double pool constants now auto-match
- A STUB's ARGUMENT COUNT moves every branch displacement in its caller
- A pointer initialized INSIDE a guarded loop's `if` still belongs outside it
- The register pair inside an inlined `strlen` follows what is live AFTER it, not the strlen statement itself
- `volatile` on a local pins it to memory, and a cast reads it back folded
- THE FIRST RELOAD AFTER A STORE-THROUGH-THE-RETURNED-POINTER TAKES edx, NOT eax, AND NOTHING CHANGES IT
- NAMING THE INDEXED LOAD ALSO DECIDES WHETHER IT IS LOADED EARLY, AND CAN SPILL A COUNTER
- A DEAD STORE THAT THE ORIGINAL KEEPS IS AN ADDRESS-EXPOSED AGGREGATE
- AN ACCUMULATOR COMBINED WITH A CALL'S RESULT MUST BE UPDATED IN ITS OWN STATEMENT
- A ZERO REGISTER cl DECLINES COSTS THE WHOLE FRAME, NOT JUST THE ZEROS (and goto-shaped stores of 0 are the usual cause)
- DIFF THE REGISTER-FREE CONTENT, NOT THE LISTING: scratchpad/semscan.py (+ callseq.py, zq2.sh)
- goto-SHAPED CONTROL FLOW IS THE USUAL CAUSE OF A "REGISTER WALL" (positive nested tests, per-arm stores/calls)
- A BYTE-BY-BYTE COLOUR BUILT FROM rand() IS ONE EXPRESSION
- A DEREF FEEDING THE *FIRST* ARGUMENT IS EVALUATED AFTER THE OTHER PUSHES, AND cl WILL NOT DO IT
- AN ADDRESS ARGUMENT MATERIALIZED *BEFORE* THE PUSHES IS READ THROUGH
- NAMING THE INDEXED LOAD DECIDES WHICH REGISTER CARRIES IT
- DECLARING THE ACCUMULATOR BEFORE THE POINTER IS WHAT RESERVES eax FOR THE ZERO
- THE STACK PACKER'S SLOT ASSIGNMENT IS NOT STEERABLE FROM SOURCE, and it now costs SIX functions
- A ZERO REGISTER THAT LIVES FROM A GUARD TO A LATER LOOP IS `i = 0` BETWEEN TWO NESTED TESTS
- A zero register vs. the loop counter is a coin flip cl does not expose
- Hoisting a pointer OUT of a loop is worth trying whenever the frame is 4 bytes too big
- A subclass built by INLINING its ctor into its owner needs an INLINE ctor
- `operator delete` is decided PER CLASS, and the deleting destructor says which
- CHECK A STRUCT CARVE BY *PRINTING* THE OFFSETS, NOT BY ASSERTING THEM
- ADDING FIELDS TO A VERIFIED STRUCT BREAKS MATCHED FUNCTIONS, EVEN INSIDE `_pad`
- Before modelling a class, grep the headers for its address
- A vtable is only emitted where cl generates a ctor/dtor
- `setne` instead of a branch = the two returns must be written as separate blocks
- A huge member offset mis-symbolized as an address — FIXED in reccmp (patch 4)
- A large IMMEDIATE colliding with our image is the SAME problem, and it is fixed by `/base`, NOT by a patch
- VTABLE/FUNCTION markers must live in a `src/` file
- One virtual `~T()` definition emits BOTH the real dtor (`??1`) and the scalar deleting dtor (`??_G`)
- TWO STRING LITERALS ON ONE LINE NEED TWO MARKERS ON SEPARATE LINES
- A SECONDARY BASE MUST BE `__declspec(novtable)` too
- A parameter that receives the CALLER'S OWN secondary-base pointer has to be `void *`
- A LOOP BOUND IS AN ADDRESS, so it must resolve the same way on BOTH sides
- A LOOP COUNTER SPILLED TO THE FRAME is the dead-loop-variable rule under register pressure
- What CREATES a float zero decides which register a nearby constant gets
- A BROKEN `// STRING` MARKER READS AS "Failed to find a match at 0xADDR", not as a low score
- cl CANNOT hoist a call above a store to memory the call might touch
- COMPARE JUMP TARGETS, NOT JUST INSTRUCTIONS - THE ZOO CANNOT SEE A WRONG BRANCH (scratchpad/jt3.py, jt4.py, jctx.py)
- cl THREADS A BRANCH THROUGH A NAMED LOCAL'S KNOWN VALUE, NOT THROUGH A CSE'd RE-READ
- cl HOISTS A LOAD ABOVE A STORE ONLY WHEN BOTH ARE MEMBERS OFF ONE BASE
- Two stores in an if/else beat a ternary when the original schedules the compare next to the branch
- A member addressed off a LIVE POINTER rather than off `this`
- THE FLOAT REGISTER ASSIGNMENT FOLLOWS SOURCE ORDER; THE EMISSION ORDER FOLLOWS THE SCHEDULER
- AN EFFECTIVE-100% WHOSE ONLY DEFECT IS A FLOAT *LOAD* ONE INSTRUCTION EARLY MEANS THE FLOAT STORE CAME FIRST IN THE SOURCE
- WHEN EVERY VALUE IS STORED TWICE, THE GROUPING OF THE STORES DECIDES WHETHER cl SPILLS OR DUPLICATES
- Float PAIRS set through an inlined two-argument helper stay where they are; six plain assignments float to the top
- AND WHEN THE INDEX IS STILL LIVE (a float conversion, an address) BUT A DOWN-COUNTER EXISTS ANYWAY, WRITE THE COUNTER EXPLICITLY
- A SEPARATE DOWN-COUNTER MEANS THE LOOP VARIABLE IS DEAD
- Derive the /GS buffer size from the frame every time - 256 and 260 both occur
- ASCENDING EH states mean an INLINED BASE DESTRUCTOR, not members
- The EH state in a destructor is the INDEX OF THE NEXT MEMBER TO DESTROY
- Pinning a vtable forces every virtual to be DEFINED, which surfaces the ICF-folded slots
- A 24-byte zero block is a memset, not six assignments
- The same string literal used by two functions needs only ONE `// STRING` marker
- A SECOND VPTR STORED AT THE TOP OF A DESTRUCTOR IS MULTIPLE INHERITANCE
- ONCE A CLASS HAS TWO VPTRS, *BOTH* MARKERS NEED THE BASE-CLASS FORM
- A SECONDARY BASE'S OWN VFTABLE FULL OF `_purecall` MEANS IT IS ABSTRACT, and the intermediate that implements it is `novtable`
- The root EzObject/KEZ2DJObject is 0x10 bytes, not 4
- A gap to the next member is an UPPER bound on a member's size, not the size
- `__ehvec_dtor_iterator` @0x47a12b needs a LIBRARY annotation
- Two scans find destructors, and destructors are the cheapest class entry
- A destructor is a free MEMBER MAP, and it says array vs. members
- A vtable slot cannot be pointed at a lookalike beside it
- A `// STRING` marker must sit DIRECTLY above the line holding the literal
- A `// STRING` marker must be DIRECTLY above the literal's line, even inside an argument list
- A `// STRING` literal must be on ONE source line
- memmove vs memcpy
- strcpy must be called, not hand-written
- ICF-folded empty functions: use `folded` (reccmp patch 5)
- A destructor with NO vptr store in front of it is the BASE's, so the class has no destructor of its own
- AND GREP FOR IT BEFORE RECONSTRUCTING, NOT ONLY BEFORE DECLARING A GLOBAL
- RUN `scratchpad/dupes.py` AFTER A BATCH - IT FINDS EVERY DOUBLY-ANNOTATED ADDRESS
- AND GREP FOR THE ADDRESS, NOT THE NAME YOU ARE ABOUT TO INVENT
- AND THE INVERSE IS QUIETER STILL: REUSING AN EXISTING *NAME* FOR A NEW ADDRESS BINDS YOU TO THE WRONG STORAGE, SILENTLY
- AND NEVER PIPE THE DUPLICATE CHECK THROUGH `head`
- GREP FOR AN ADDRESS BOTH WAYS: `0x1b2eb80` and `0x01b2eb80`
- Marker formatting
- A SIGNED `jl` ON WHAT LOOKS LIKE A POINTER COMPARE MEANS THE LOOP VARIABLE IS AN `int`
- ANCHOR A NAMED POINTER AT THE *ROW*, NOT AT THE ELEMENT, WHEN THE ORIGINAL KEEPS THE MEMBER OFFSET IN THE DISPLACEMENT
- `p++; x = *p;` AND `x = p[1]; p++;` ARE NOT THE SAME CODE
- Loop induction anchoring
- The stack probe is `_chkstk`, NOT `__chkstk`
- CRT calls match automatically
- A dead `if (x == -1)` after a search = an inlined helper returning -1
- Whole-class reconstruction (vtable classes)
- Scalar deleting destructor
- cl DUPLICATES A SHARED TAIL *AFTER* CONSTANT FOLDING, so assigning a constant to a variable keeps it unfolded where returning the expression does not
- A constant passed through an INLINED HELPER'S PARAMETER is not folded
- Write a post-increment as a post-increment
- cl merges a `push` across a branch
- A shared value written out at BOTH use sites gets a different register than the same value hoisted into a local
- Name an index in a local to fold it into a read-modify-write
- A clamp must OVERWRITE, not select
- Seed a switch's result before the dispatch to share the default's tail
- A walking pointer must come from cl's strength reduction, not from the source
- A VALUE THE ORIGINAL RE-READS INSIDE AN *INLINED HELPER* NEEDS THE QUALIFIER ON THE HELPER'S PARAMETER, NOT ON THE ARGUMENT
- `volatile` is decided PER MEMBER, and codegen tells you which
- NAME THE PROMOTION, NOT THE PRODUCT, TO SPLIT AN INLINE ftol BLOCK
- A float pool constant must stay an ANONYMOUS literal
- AND SOMETIMES IT HAS TO BE A NON-CONST `static float`
- A VALUE IN A DEAD PARAMETER'S SLOT MAY BE THE PARAMETER ITSELF, REUSED
- A TAIL `jmp` THROUGH A VTABLE SLOT MEANS THE FUNCTION RETURNS `void`
- A RUN OF TESTS THAT ALL JUMP TO ONE `return 1` IS A SINGLE `||` CHAIN
- TWO ARMS THAT SHARE THEIR LAST INSTRUCTION SHARE A *VARIABLE* IN THE SOURCE
- A CALL WRITTEN ONCE AFTER A BRANCH BELONGS INSIDE EVERY ARM WHEN ITS RECEIVER'S ADDRESS IS MATERIALIZED IN EACH
- AND READ WHAT THE ORIGINAL COMPARES *AGAINST*, not what the value is
- BUT A CONSTANT REGISTER cl DECLINES MAY BE A SEMANTIC ERROR IN *YOUR* CONSTANTS, NOT A WALL
- COUNT THE PROLOGUE PUSHES: A CALLEE-SAVED REGISTER THE ORIGINAL DOES NOT HAVE IS A CACHED PARAMETER, AND THE ZERO REGISTER IS ITS SYMPTOM
- AND A BACKWARD `jmp` INTO A SHARED TAIL COMES FROM WRITING THAT TAIL IN BOTH ARMS
- AND BEFORE PARKING ONE, ASK WHAT THE *OTHER* CANDIDATE IS DOING IN THAT REGISTER
- THE MODE-INPUT "CONSTANT-REGISTER WALL" WAS A SCOPE DEFECT - READ THE ORIGINAL'S JUMP TARGETS BEFORE BLAMING cl
- AND A FLOAT *LITERAL* PROMOTES WHERE A FLOAT *VARIABLE* DOES NOT
- `fdivr qword` WHERE THE ORIGINAL HAS `fdivr dword` IS A `(double)` CAST ON THE OTHER OPERAND
- A loop between stores and an inline memset anchors cl's scheduling
- A parameter re-read from memory where cl caches it in a register
- `volatile` MEMBERS, when the original never folds a member as a memory operand
- Ctor/dtor virtual calls that stay DYNAMIC (slot-address form)
- Success-block-after-the-return layout
- Keeping a search-index register live early (prologue save)
- A FACTORY'S `push` IS `sizeof(T)`, so matching one CHECKS the class
- The engine has its own POOL `operator new` @0x4016a0, and it is `__stdcall`
- operator new is the libcpmt operator new, NOT an inlined malloc - check LIBRARY scores
- The frame-alignment wall (KNOWN-HARD, parked)
- Alternate period compilers on hand
- AND WHEN THE ORIGINAL KEEPS A VALUE IN edx ACROSS A CALL, THE CALLEE MUST BE IN THE SAME TU
- A STUB's CLOBBER SET is fictional, and that can be unfixable
- Matching callers before callees (STUB markers)
- A STORE OF A VARIABLE WHOSE VALUE THE BRANCH ALREADY KNOWS = ONE SHARED TAIL IN THE SOURCE
- A BUBBLE SORT THAT INDEXES names[j] DIRECTLY IS `i = 0..n-2, j = i + 1` WITH THE BYTE SWAP WRITTEN OUT
- RUN semscan AND callseq ON EVERY BIG FUNCTION BEFORE TUNING REGISTERS - THE CALL MULTISET FINDS REAL BUGS
- AN ARM THE ORIGINAL PLACES OUT OF LINE AFTER THE OTHERS IS THE `else` OF A NEGATED TEST
- THE ICF RULE COVERS ANY IDENTICAL TAILS, NOT JUST SWITCH ARMS
- `(int)(float)(x87 expression)` COMPILES TO THE INLINE fnstcw/fldcw/fistp SEQUENCE; `(int)(expr)` CALLS _ftol2
- CALL SET-UP HOISTED *BETWEEN* TWO COMPARES = NESTED ifs WITH ONE BODY WRITTEN TWICE
- RUN THE PERMUTER ON A "COIN" BEFORE PARKING IT: scratchpad/perm (six such coins fell in one batch)

## Big repetitive functions & leaf accessors - MOVED to docs/cookbook.md

A big function that is a table of calls lands 100% on the first build; rank
targets by branches-per-instruction ascending, decode with a script, write
every switch arm out. Tiny load/store leaves batch by category (field
get/set, indexed, float, constant-return, class-name, singleton). Full
playbook and the known gotchas: docs/cookbook.md.

## Multi-agent fan-out vs inline — the token cost rule

Parallel Workflow fan-out (one agent per unmatched function, each disassembling +
zoo-compiling in its own scratchpad) is powerful but has a sharp cost cliff. Measured:
a batch of 12 **small** functions matched 12/12 for ~820k tokens; a batch of 9 **big**
EH functions matched only 1/9 for ~530k tokens — the other 8 hit the session token
limit mid-analysis and died having written nothing, and the forced re-run re-burned
tokens re-analyzing from scratch. Doing those same big functions **inline, one at a
time**, matched ~9 of them for a small fraction of the tokens with every one banked as a
commit.

- **Fan out ONLY for many small/leaf functions** — getters, setters, ctors, deleting
  dtors, class-name getters, thin wrappers. Heuristic to qualify: `< ~80` instructions
  **and** no EH prologue (`push -1; push handler; mov eax,fs:[0]`). Cap ~12 agents/batch.
- **Do big functions inline, serially** — disasm → zoo build → byte-diff (mask only
  `call`/`jmp` rel32 operands) → integrate → `./match.sh` → **commit each**. A big
  function lands in 1-4 zoo iterations at a fraction of a fan-out agent's cost, and each
  commit banks progress so a session-limit cut never loses work. EH prologue or `> ~100`
  instructions ⇒ inline, never spend an agent on it.
- **Measured again (2026-07-25): 10 small leaf functions (6-27 instructions, no EH), one
  agent each, ~730k tokens — 10/10 BYTE_MATCH and all ten integrated at 100% first try.**
  What made it work: agents were READ-ONLY on the repo (no edits, no ./match.sh, no git) and
  each verified its candidate in its own zoo TU via scratchpad/zoo.sh, returning paste-ready
  source + the STUB/GLOBAL blocks it needed + the list of addresses it found ALREADY
  annotated. The parent integrates serially and runs reccmp — that is the only ground truth,
  and it also resolves the naming collisions agents inevitably invent for a shared callee.
- **Repeated session-limit hits during a workflow run = you are fanning out too much or
  too big.** Switch to inline immediately; don't keep re-running the workflow. If a
  fanned-out big function did die, salvage its `scratchpad/fn_0xADDR/*.cpp` draft first,
  but expect little — big-function agents often die before writing any candidate.

## THE GOAL HAS A SECOND TRACK: A RECOMPILATION

A native, portable build is planned alongside the matching decomp, and it
changes what is worth working on. Read **docs/PORTING.md** before choosing a
target; **docs/PORT-WORKQUEUE.md** is the ordered list.

- **A port does not need matching code** - but it does need CORRECT code.
  As of 2026-10-05 every game function has C in `src/` (99.4% of .text by
  bytes, `scratchpad/coverage.py`; the rest is two import thunks), no STUBs
  remain, and `wip/` holds analysis notes only (wip/README.md). A low score
  is usually just registers, yet two reconstructed functions were found
  decrypting noise and two more had invented stores - so for a port the
  useful work is behaviour checks (semscan, native runs on real data), not
  more decoding.
- **`scratchpad/fmt/run.sh` RUNS THE FILE-FORMAT C ON THE GAME'S DATA.** It
  links the build's own objects into four small VC7.1 harnesses (ciphers,
  chart load, KProfile::Load, .gds header) and diffs them under Wine against
  ../EZ2REWRITE's decoders. 2026-10-05: .ez/.ezi/.ini decrypt and the .ini
  round trip identical on all ~37k files, all 12354 loadable charts identical
  note for note, 18/18 profile .ini, 23/23 .gds; .str clips 10593/10593 and
  every readable .scr script (318) - where EZ2REWRITE reads kind-4/5 names
  from the wrong offset; the decomp and the port use payload+0x1c. Chart
  .ini through EZ2DJMainGameDirector::loadGds 12339/12344 (the 5 are -0 vs
  0), and the whole mode .gds through m421190 23/23 - the game reads every
  block POSITIONALLY, and no shipped file's key order breaks that. It found the chart ciphers
  decrypting noise and `g_parserTable` 6 entries short - two bugs no match
  score could show. Data tables are zero placeholders in our build; the
  harnesses load the real ones from original/ at run time.
- **Rank by REACHABILITY, not branch density, while the port is the goal.**
  `scratchpad/spine.py` walks the call graph (call-rel32 plus the slots of any
  vtable reachable code stores) from the app entry points; `scratchpad/scan.py`
  still answers "what is cheapest to match" and the two disagree often.
  **CORRECTED 2026-08-23 - the earlier figures here were wrong.** The script
  treated every `jmp rel32` target as a function start, so every loop tail and
  `else` inside an already reconstructed function counted as a separate
  unreconstructed one, and the walk put non-start addresses into the reached
  set where they had no size and no annotation. It reported 988 unreconstructed
  spine functions / 154 KB and a 328 KB spine; the real figures are **74 / 73 KB
  and a 438 KB spine**. So the old claim that "31% of the binary is not
  reachable at all and the port never needs it" is FALSE - the spine is 92% of
  the code, and there is very little the port can skip. (The vtable rule
  over-approximates, so 92% is an upper bound.) Those spine figures are
  historical: the 74 unreconstructed functions have all been written since.
- **Priority under the port goal:** chase the game-logic spine (EzChart,
  scoring, judgement, chart loading) and the remaining file formats.
  Deprioritise register walls in draw routines and the platform shell - the
  shell gets replaced and `../EZ2REWRITE` already has a loose version.
- **The port is its own repo now, at `../EZ2-projects/port`** (moved out of
  this tree on 2026-09-28, history kept via `git subtree split`). It belongs to
  a different session. Do not edit it; see its `OWNERSHIP.md`. Requests from
  that side arrive in its `REQUESTS.md`. Its oracle (`oracle/run.sh`) still
  compiles `src/`/`include/`/`wip/` from here via `../../../EZ2DECOMP` or
  `$EZ2DECOMP`. `port/...` paths in older docs mean that repo.

## Workflow rules for the assistant

- Do not silently guess the toolchain, an address, a struct layout, or a calling
  convention. State the evidence; if unconfirmed, say so and ask.
- Before starting a new function, consult the prior-art RE (from `../EZ2REWRITE`, to be
  captured in `docs/prior-art.md` during scaffold) — it already names many functions,
  globals, and structures by address.
- One function at a time to 100%, then move outward. Paste the reccmp diff every time.
