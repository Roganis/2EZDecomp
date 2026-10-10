# EZ2DECOMP matching cookbook

Moved verbatim from CLAUDE.md (which holds a one-line index of every rule
here). This file is the FULL text: read the entry before working a function
that matches its shape, and add new rules here, mirroring the headline into
CLAUDE.md's index.

## Leaf-accessor quick wins (batch these)

A scan for tiny functions (load/store + ret, no calls/branches) yields dozens of
trivial matches. Categories, each nearly guaranteed 100%:
- **Field getter/setter** (`mov [ecx+off],eax` / `mov eax,[ecx+off]`): `__thiscall`
  member `T::get(){return f;}` / `set(int v){f=v;}` with a `char _pad[off]` struct.
- **Indexed / 2D** (`lea eax,[eax+eax*4]; mov [ecx+eax*4+base]`): model a stride-N
  element array at `base`; `arr[i].field` or `arr[a][b]` (row stride from the lea).
- **Float getter** (`flds [ecx+off]` / `flds 0xADDR`): `float T::get(){return f;}`;
  a global float needs a `// GLOBAL` on the address.
- **Constant return** (`mov eax,C; ret N` / `xor eax,eax` / `or eax,-1` / `mov eax,ecx`):
  pick the convention that yields the right `ret N` — `__stdcall(k args)` for `ret 4k`,
  plain for `ret`, thiscall `return this` for `mov eax,ecx`. Bytes are identical for
  `__stdcall(k)` and a thiscall with k stack args, so either declaration matches.
- **Class-name getter** (`mov eax,0xADDR; ret`, ADDR in .rdata is a string): return
  the literal with a `// STRING: EZ2AC_UNPACKED 0xADDR` marker above it.
- **Singleton getter** (`mov eax,0xADDR; ret`, ADDR in .data): return `&g` with a
  `// GLOBAL: EZ2AC_UNPACKED 0xADDR` + dummy placeholder storage.
- Skip: SEH funclets (`mov ecx,fs:0x0; add esp,0x10; ret`) and epilogue fragments
  (pop/`add esp` with no matching push) — the scanner mis-flags these as starts.
- Known gotchas that can strand one function at <100%: a byte-load feeding `& 1` may
  emit `mov al` instead of the original's `movzx` (aligned-index peephole, see
  wip/getbit.cpp); a file-scope global DATA ref can show under a synthetic
  `identities::` namespace and score 75% (see wip/setglobal.cpp). Park these.


## Matching cookbook (recurring patterns)

- **Optimizer flags:** `/O2 /MT /GS /Gy /GF /Gd /G7 /arch:SSE2 /EHsc`. `/G7` is required
  (P4 scheduling + partial-register `movzx`). `/arch:SSE2` is required — it is what makes
  cl emit the `push ebp; mov ebp,esp; and esp,-8` aligned-frame prologue on functions with
  stack buffers (35 such prologues across the original's game code; arcade cab = fixed P4).
  `/EHsc` proven by 165 `mov eax,fs:[0]` EH prologues in the original's game .text.
  All flags locked against loadEZI @0x411090 + the 15 prior matches (all stay 100%).
- **/GS buffer +8 pad under /arch:SSE2:** the cookie does NOT sit directly after the top
  GS array — cl inserts 8 bytes of pad between the last buffer and the cookie. When
  deriving a buffer's size from `cookie_offset - buffer_offset`, subtract 8 (loadEZI's
  sampName is 260, not the 268 the raw frame math suggests). But the pad is NOT
  unconditional: a function whose only local is the buffer gets frame = buffer + cookie
  exactly (getExeDir @0x401080 is `char[260]` in a 0x108 frame). Derive the size, then
  confirm it — an 8-byte error shows up immediately as a wrong `sub esp,N`.
- **The exception model is PER-TU: /EHsc everywhere except src/kengineapp.cpp (/EHs).**
  match.sh's `eh_model` picks it. Under /EHsc cl assumes `extern "C"` never throws, so a
  Win32/CRT call is not a throw point; under /EHs it is. Both readings are proven in the
  original: `~KSongPlayerBase`/`~KSNDMixer`/`~Obj414750` have NO EH frame around their
  Win32 cleanup calls (⇒ /EHsc), while `KEngineApp::loadProfiles` @0x401240 resets its EH
  state to -1 after each `new`'s ctor, which cl only emits when the *next* allocation call
  can throw (⇒ /EHs). If a function's only defect is a missing/extra EH-state store around
  an extern "C" call, try flipping its TU's model before touching the source.
- **Switch arms are emitted in SOURCE order, and a jump table is compared as offsets into
  the function** — so the case order in the reconstruction must match the original's *block
  layout* order, which is often NOT numeric. Read the table, resolve each entry to its
  block, and list the cases in that order (runGame's 13-mode dispatch is 0,1,2,3,6,8,9,7,
  4,5,10,11,12).
- **A FUNCTION WITH TWO NEAR-DUPLICATE HALVES DUPLICATES THEIR SHARED TAIL
  TOO.** When one global splits a function in two (`g_mixStyle == 7` in the
  keys commit @0x42fd90, CV2Mix in the sink @0x42e620), the ending both halves
  appear to share - test a flag, call the sink, draw the effect, return - is
  written out ONCE PER HALF in the source. Sharing it through a `goto` costs
  the whole block: 83 instructions and 17 points on that function, because
  every arm inside each half then jumps forward to one copy instead of falling
  into its own. The tell is the same block appearing at two addresses with
  different registers, and the small per-half differences that hide in it
  (the style-7 copy is where its KOOL->COOL knock-down lives, rather than a
  test inside a shared tail). Duplicate first, then look for what differs.
  **BUT A RE-READ IN EVERY COPY MEANS THE OPPOSITE - WRITE THE TAIL ONCE AND
  LET cl DUPLICATE IT.** The two cases look alike in the diff and want
  contrary sources, and there is a clean discriminator: a HAND-written copy
  of `g = member;` forwards the store the same block just made and publishes
  the register, while a copy cl made by duplicating a single tail RE-READS
  the member, because the duplication happens after store-forwarding. So if
  every copy of the shared block reloads what the block above it just wrote,
  the source has ONE of them. Writing the tail out per path also inflates the
  use count of whatever constants it mentions, which was enough here to make
  cl materialize a zero register the original does not have - so the whole
  function read as a register problem when it was a structure one.
  (ScrFader::update @0x450450, 60% -> 100% by deleting four copies.)
  **WHICH arms get a copy is itself the lever.** In a ladder whose last arm
  falls through to the tail, give the copies to the LADDER arms and NOT to the
  out-of-line default: cl merges the copies into the one that falls through,
  which is where the original keeps it. The hold tick @0x42f4e0 measured
  78.90% shared, 84.81% with every arm copied, 86.92% with the three ladder
  arms copied and the default left sharing (67.40% -> 75.41% by reccmp). And
  the converse is real - the hold pump's four copies are already right, and
  dropping any one of them costs sixteen points, so try both directions
  rather than assuming more duplication is better.
- **A DIRECT jump table means every case had its OWN arm in the source.** cl has two
  lowerings: `cmp eax,N; ja default; jmp [eax*4+TBL]` (direct) and
  `add eax,-lo; cmp eax,N; ja default; movzx eax,BYTE PTR [BYTETBL+eax]; jmp [eax*4+TBL]`
  (byte-index). Grouping repeated cases under one label — `case 0: case 2: case 3:
  default: <body>` — makes cl drop the labels that duplicate the default, shrink the range
  to the remaining cases, and pick the byte-index form. Writing all twelve cases out with
  their own (identical) bodies keeps the range 0..11, gets the direct table, and the tail
  merger folds the bodies back to the same handful of blocks afterwards. So the table's
  ENTRY COUNT tells you how many arms the source had, not how many distinct bodies.
  (Effector6thStyle's ctor @0x41dcc0, both of its switches.)
- **Two switch arms that call the SAME address = two functions the original's linker
  ICF-folded.** Written as one function, cl cross-jumps the two arms into one block and the
  table gets a duplicate entry; the original keeps both blocks because at compile time the
  two arms called different symbols. Declare both functions and give each a `folded`
  annotation on the shared address (`// STUB: … 0xADDR folded` works — patch 5 covers stubs
  too). (5KeyMix and 7StreetMix @0x416b90.)
- **AND A SWITCH WHOSE ARMS ARE ALL `call SAME; jmp SAME` YET STAY SEPARATE
  IS THE SAME THING AT SCALE.** SongSelectDirector::update @0x443380 has five
  `switch (f78)` blocks over 13 modes; in the original every arm is its own
  `mov ecx,esi; call X; jmp Y` block - three consecutive identical ones
  included - while cl merged ours down to a handful. Eight modes called eight
  identical page functions (folded onto 0x439f00) and three called identical
  wheels (0x433180). Declaring `pageModeN` / `wheelModeN`, each a `// FUNCTION:
  ... folded` copy, took update from 84.27% to 99.15% in one step. Write the
  shared body ONCE in an `.inl` (include/songselectpage439f00.inl) and
  `#include` it inside each copy's braces - the folded-alias rule needs the
  real body in every copy, not N pasted ones. reccmp patch 20 then fixes the
  placeholder NUMBERING the extra alias addresses shift (99.69% -> 100%).
  The same scan exposed this switch's arms carrying the wrong case labels -
  read each arm's call target against the jump table, not just the order.
- **A `// VTABLE:` pin costs nothing but STUBs.** Every slot only has to be *address-mapped*,
  and a STUB is; the vtable data then compares equal while the slots stay unscored. This is
  what makes a class usable from a matched caller long before its methods are reconstructed
  (runGame constructs five 7-slot screen classes, 14 STUBs total). The pin is REQUIRED for
  any inlined ctor: without an entity at the vftable address the original's vptr store
  prints raw hex while ours prints the vftable name.
- **`call A; jmp B` with NO `this` saved = A's body must be VISIBLE in the same TU.** cl
  will not keep `this` in ecx across a call it cannot see (it emits `push esi; mov esi,ecx;
  call; mov ecx,esi; pop esi; jmp`). When the callee's body is in the same TU — either
  `__declspec(noinline)` or simply too big for the inliner — cl knows ecx survives and emits
  just the two instructions. This INVERTS the usual stub-placement rule for that one callee:
  its STUB has to live in the caller's TU. Two riders: the stub body must be NON-EMPTY (cl
  deletes a call to a side-effect-free empty function even when it is marked noinline), and
  any callee the original DOES reload ecx after must stay in another TU.
  (Obj40C180::shutdown @0x40cc80 and ::init @0x40cc30.)
- **A HELPER THE ORIGINAL INLINED AT SOME SITES AND CALLED AT OTHERS lives in its own TU,
  and the inlined sites are WRITTEN OUT.** cl 13.10 inlines a small same-TU helper at every
  site, so one visible definition can never reproduce a mixed pattern. Tell: a
  call-sequence diff where the original has `call 0x415d70` at some sites and our build has
  `new`/`run`/`free` there, or the reverse. Fix: move the helper out of the callers' TU and
  paste its body (here three statements: `new`, `run`, `delete`) at exactly the sites the
  original inlined. The mode runners in src/moderunner.cpp use five screen helpers now kept in
  src/moderunscreens.cpp; this took four runners to 100% and Ez2Catch from 37 to 67%.
- **PARENTHESES ARE A BARRIER TO cl's FLOAT REWRITES.** `s * -250.0f + 240.0f`
  compiles to `240 - s * 250` (a subtract with the POSITIVE constant) and
  `a * 0.02f * 4.0f` to one multiply by 0.08, and `c * k + 240 + f + 15` folds
  the 240 and 15 into 255 - but `(s * -250.0f) + 240.0f`,
  `(a * 0.02f) * 4.0f` and `((c * k) + 240.0f) + f + 15.0f` keep the
  original's mulss-by-negative / two mulss / three addss. A named local for
  the product works too. Tell: the original multiplies by a NEGATIVE pool
  constant, or references two constants whose product is the one ours uses.
  (InGameSongSelect::m425b40 76.75 -> 100, SongSelectDirector::update2,
  Effector6thStyle::update2; `/Op`, int literals and inlined helpers do not
  do it.)
- **`return a == b;` LAYS THE ARMS OUT DIFFERENTLY FROM `if (a == b) return 1;
  return 0;`** for a float compare: returning the comparison puts the
  `return 1` inline after the test (`jp` over it), where the if/return form and
  goto-into-a-shared-win both put the `return 0` there. (PlayerSlot::m42e590
  69 -> 100.)
- **SWEEP FOR WRONG CONSTANTS BEFORE MATCHING A FUNCTION.** Three scratchpad
  scans over a reccmp --json turn up behaviour bugs a call-sequence check
  cannot: `fltscan.py` (float/double pool constants and float immediates
  pushed, per function, original vs ours), `divscan.py` (magic divisions and
  idiv divisors) and `cmpscan.py` (a `cmp A,B; jcc` whose operands swapped
  without the condition mirroring). Found on 2026-10-01: a reversed weight test
  (pickSample42de50), /10 where the C had /5 (the combo-chart head), a rank
  ladder of 95..55 where the binary has 98..50, a parser writing two words low
  over a material pointer, and five pixel offsets. Most hits are noise (a
  constant that only fell out of the diff window); read each one.
- **A function that is nothing but `jmp rel32` is a one-line forwarder** — `void T::s5() {
  doS5(); }` in tail position. The target must be in another TU or cl inlines it and the jmp
  disappears. (Four of the five screens' slot 5.)
- **`volatile` LOCALS, when the original reloads a local at EVERY use** (not once per basic
  block, which is ordinary memory-resident behaviour): `mov [x],eax` immediately followed by
  `mov edx,[x]` is the tell. Expect it on loop scratch the original keeps in memory because
  its registers are full. CAVEAT: with TWO volatile locals cl's stack packer stops using
  declaration order and permutes their slots, and nothing at source level steers it — that is
  what parks wip/screenrun.cpp at 88.5%.
- **Keeping a dead counter live forces indexed array stores.** A collect loop that fills a
  local array and then never reads the count lets cl strength-reduce `arr[n]` into a walking
  pointer (`mov [ecx],eax; add ecx,4`). The original's `mov [esp+ecx*4],eax; add ecx,1` means
  the count was still live afterwards — reproduce it with the trailing `for (i = 0; i < n;
  i++)` loop whose body optimizes to nothing. (engineTick418e60/418ee0.)
- **AN IAT SLOT THE PACKER LEFT UNREBUILT STILL RESOLVES - BY OFFSET, FROM THE
  SURVIVING NAME TABLE.** Most imports read straight out of the file, but some
  slots in this dump hold name RVAs that point OUTSIDE it (0x33e3208,
  0x33e380e and neighbours against a 0xb7000-byte image), so the usual lookup
  returns nothing. The name table itself survived the dump and kept its
  internal offsets, so the slot's LOW TWELVE BITS index it directly: search
  the file for the candidate API names, take each hit's hint/name entry (the
  string's offset minus 2), and match the low bits. Confirmed on eight imports
  across two functions - WaitForSingleObject/SetEvent for the subsystem lock
  @0x40df20, and FillRect/DrawTextA/CreateSolidBrush/SetTextColor/SetBkMode/
  DeleteObject for the dummy-texture label @0x407b60 - and every one agreed
  with what its call site did with the result. Cross-check the semantics
  anyway, then let the build adjudicate: reccmp resolves IAT calls by name, so
  a wrong guess shows up immediately.
- **A `call` to a 5-byte thunk in the CRT range can be a Win32 API called WITHOUT
  dllimport.** `call 0x478522` where 0x478522 is `jmp [__imp__BlockInput]` is what the
  linker builds when the declaration is NOT `__declspec(dllimport)`; windows.h's own
  declaration is the dllimport one and compiles to a 6-byte `call dword ptr [__imp__...]`
  instead. Redeclare it yourself (`extern "C" BOOL __stdcall BlockInput(BOOL);` after
  windows.h — cl accepts it) and annotate the thunk in library.cpp by its PDB name
  (`_BlockInput@4`). Same for WINMM's timeGetTime @0x4145f0 — and note the original MIXES
  the two forms across TUs (runApp's timeBeginPeriod uses the dllimport indirect call), so
  decide per call site, not per API. A dllimport call inside a loop also lets cl hoist the
  IAT slot into a register (`mov ebx,[__imp__Sleep]` … `call ebx`).
  (EZ2AC::v2 @0x415cb0, runGame @0x417e90, wip/screenrun.cpp.)
- **A constant-return "leaf" with NO callers is probably somebody's out-of-line return
  block.** Before annotating a `mov eax,C; ret N` / `xor eax,eax; ret N` fragment as a
  function, check that something actually reaches it: scan for `call`/`jmp` rel32 to the
  address AND for the raw little-endian address in .data (a vtable slot) or in a switch
  jump table. An unaligned start (`0x40ff81`) is a strong tell. Annotating a tail TRUNCATES
  the enclosing function's size, so when that function is later reconstructed its tail
  instructions read as recomp-only additions and it stalls below 100% with every byte
  right. Two such phantoms were retired: 0x401678 (the `return 1` tail of
  KRuntimeClass::isDerivedFrom) and 0x40ff81/0x40ff89 (switch arms of the function
  @0x40ff50, see wip/entrykind.cpp).
- **`mov ecx,[g]; mov esi,ecx` (load into ecx, then copy) = the pointer was a PARAMETER of
  an inlined helper, not a local.** Three virtual calls on one object have three shapes:
  reading the global at each call reloads it three times (a virtual call may write it); a
  local `T *p = g;` loads straight into esi and copies to ecx per call; passing the pointer
  to a small `static` helper that cl inlines materializes it in ecx for the first call and
  copies it to esi for the rest — which is the original's shape. (runApp @0x4015e0.)
- **Frame-layout differences can be a SEMANTIC clue, not a layout problem.** loadProfiles
  put two `char[260]` path buffers in the swapped order, and no declaration order, rename,
  or dead address-take moved them (cl lays GS buffers out by *use*, not declaration). The
  actual bug was in the reconstruction: tracing each buffer's address through the calls
  showed the original loads the *version* path into the profile whose failure is
  non-fatal and the *patch* path into the fatal one — the opposite of the first reading.
  Fixing the semantics fixed the layout for free. Always resolve `lea` offsets to absolute
  frame addresses and check WHICH buffer each call actually gets.
- **A LOOP OVER RECORDS NEEDS THREE THINGS AT ONCE, and each one alone is
  worth a few percent.** The original's shape is `xor ebp,ebp; cmp
  [this+count],ebp; jle end; push esi; lea esi,[this+base+K]; body...; add
  ebp,1; add esi,STRIDE; cmp ebp,[this+count]; jl body`. To reproduce it:
  (a) walk an EXPLICIT `const int *` anchored where the original anchors it -
  indexing `arr[i].field` lets cl pick its own base and every displacement in
  the body shifts; (b) zero the counter BEFORE the guard, so the guard compares
  the count against that register instead of an immediate; (c) write a GUARDED
  `do`-`while` with the pointer set up INSIDE the guard, so its `push esi`
  lands in the preheader. Measured on one function: a plain `for` with the
  pointer in its init 96%, an `if` around a `for` 93% (the count test is
  emitted twice), a guarded do-while with the counter zeroed inside 95%, all
  three together 100%. (Panel::m42cf40 @0x42cf40, and it lifted the parked
  wip/panel-drawitems.cpp from 83.87% to 96%.)
- **SCOPE THE LATER LOCALS when an `__asm` helper's argument homes are too
  high.** cl gives an inlined `__asm` block its argument homes out of the slots
  not live at that call site, but a local declared at FUNCTION scope counts as
  live throughout. Declaring the ones used after an early return inside their
  own block lets the earlier call's homes overlap their slots, which is what
  the original does. (m42d290 @0x42d290: 84.31% -> 100%, with all three
  destinations already right.) Note the opposite case exists - m42bbe0
  @0x42bbe0 needed its late local turned into a TEMPORARY instead and scoping
  did nothing - so try both.
- **A NAMED LOCAL AND A COMPILER TEMPORARY ARE PACKED DIFFERENTLY**, and with
  an `__asm` helper that shows up as the frame layout. A named local reserves
  its slot for the whole function and every `__asm` argument home goes ABOVE
  it; a temporary overlaps freely with them. So `FloatToInt(&x, v)` into a
  named `int x` and `(float)RoundToInt(v)` as an expression give the same frame
  SIZE but different slot assignments - and which one each call site uses is
  what the original records. Declaration order decides the rest: `int x; int
  y;` puts x in the lower slot. (m42bbe0 @0x42bbe0: 61% -> 90% by making the
  third conversion an expression.)
- **AND A NAMED LOCAL IS SPILL-PREFERRED, A TEMPORARY REGISTER-PREFERRED.**
  The same distinction decides REGISTERS, not just slots, and it is the lever
  for a whole-function xmm cascade. A named local already owns a home slot, so
  spilling it is free; a compiler temporary would need a new one, so cl keeps
  it in a register. A value that is both stored into memory and used later is
  therefore a temporary if the source READS IT BACK from where it was stored -
  cl forwards the store, and the reload is only a rematerialization it never
  has to perform. The video blit @0x401d80 carries four corner y's and an x
  across its branch tree: written as `float y0..y3` two of them went to
  [esp+0x14]/[esp+0x18] and every xmm number after them shifted (66.67%);
  storing each corner into its vertex and reading `g_blitVerts[i].y` back put
  all four in the original's xmm1..xmm4 and the x's in its one shared
  [esp+0x18] temp slot (86.49%). Read the original's spill slots first: N
  DISTINCT values sharing ONE slot is a temporary, one value owning a slot for
  the whole function is a named local.
- **TWO FLOAT STORES THROUGH ONE MEMBER POINTER NEED A NAMED POINTER *AND* A
  NAMED VALUE.** `p->a = f(x); p->b = 0.0f;` written through the member twice
  makes cl reload the pointer between the stores; hoisting it into a local
  fixes that but then its load is scheduled ABOVE the value's computation.
  Computing the value into a local FIRST and naming the pointer second gives
  the original's order - one load, placed after the conversion. (Same
  function: 90% -> 94.5% -> 100%.)
- **A BARE `fistp` IS INLINE ASSEMBLY, NOT A CAST.** `(int)f` compiles to a
  call into the CRT's `_ftol2` (C requires truncation); the original's
  float-to-int rounds with a plain `fistp`, which follows the current rounding
  mode. The fingerprint is that BOTH arguments are spilled to the frame and
  read straight back - `lea eax,[esp+4]` / `movss [esp+0xc],xmm0` /
  `mov [esp+8],eax` / `fld [esp+0xc]` / `mov edx,[esp+8]` / `fistp [edx]` -
  because an `__asm` block cannot see values held in registers. The period's
  FastFloatToInt reproduces it byte for byte:

      __forceinline void FloatToInt(int *i, float f)
      { __asm fld f  __asm mov edx, i  __asm fistp dword ptr [edx] }

  Every panel readout uses it (GFPanel's three @0x464250/0x464300/0x4644c0,
  all first build once it was written this way).
- **THE SCAN THAT FINDS A MIS-FOLDED ALIAS IS ONE LINE: any address carrying
  BOTH a `FUNCTION` and a `STUB` annotation.** The trap in the next rule is
  SILENT - the table prints `X is a stub` for a function whose body is written
  and byte-exact, nothing errors, and the summary rounds the missing entity
  away. `TotalResultDirector::loadDigits` @0x454480 sat that way undetected;
  making its ICF twin a `// FUNCTION: ... folded` too scored it at 100% and
  added an entity. Grep both annotation kinds into two dicts and intersect the
  keys. Across the whole tree that finds exactly two addresses, and the other -
  0x402880, the ICF-folded empty `ret` that src/applog.cpp is meant to own - is
  the intended arrangement.
- **A `folded` ALIAS MUST CARRY THE REAL BODY, NOT AN EMPTY ONE.** The rule
  above says a folded alias has to be a `STUB` when the WINNER is a stub; the
  converse case is the trap. If the winner is a real function and the alias is
  a `// STUB: 0xADDR folded`, the shared address becomes a STUB ENTITY and the
  winner stops being scored - reccmp reports it as "is a stub" and its 100%
  never counts. Annotate BOTH copies `// FUNCTION: 0xADDR folded` and write the
  body out twice. (KEzwLoader/KWavLoader's close and tell @0x40f160/0x40f230;
  the existing EzChart::getSound / Obj4102C0::getA pair @0x40f380 already does
  it this way.)
- **A BIG NEW GLOBAL CAN BREAK A BIASED REFERENCE IN A FUNCTION YOU DID NOT
  TOUCH.** reccmp patch 12 resolves a displacement a few bytes below an
  entity's base as `name-k` (the `buf[len - 4]` shape). That resolution happens
  on BOTH sides independently, so if new placeholder storage shifts our .data
  and lands some other entity exactly on the biased address, our side names
  that entity instead and the function drops with every byte still right. Two
  4 KB tables added beside their user in src/accessors.cpp did this to
  `CatchBackGround::videoName` @0x46aba0 (100% -> 94.03%); moving them to
  src/globals.cpp, where the placeholder blocks belong, fixed it. Keep big
  placeholder arrays out of TUs whose own data is referenced with a bias, and
  run the FULL match after adding one.
- **A `// LIBRARY` MARKER RESOLVES BY NAME IN *OUR* PDB, so annotating a CRT
  symbol nothing in `src/` calls costs a point of Implemented.** The entity is
  created on the original side and has nowhere to match, and it shows up only
  as `Implemented: 99.87% (N / N+1)` with every function still at 100% - there
  is no error message. Keep an annotation for a CRT function whose only caller
  is parked in `wip/` next to that park, not in src/library.cpp. (fseek, when
  the audio loaders' seek was parked.)
- **A LOCAL WITH A CONSTRUCTOR RUNS IT AT ITS DECLARATION POINT, so where you
  declare it is a matching lever.** Declaring `KParser parser;` at the top of a
  function puts its ctor call ahead of every statement; if the original does
  real work first, `this` has to survive that call and takes a callee-saved
  register, which costs the function whatever that register was doing. Moving
  the declaration BELOW the leading statements let the original's shape come
  back: the work runs while `this` is still in ecx, the object is never touched
  again, and esi is free to be the function's zero. VC7.1 accepts mid-block
  declarations. (InGameSongSelect::m425140 @0x425140: 86.44% -> 98.31%.)
- **cl 13.10 DOES apply the empty-base optimization to a TRAILING empty base.**
  Adding `EzPooled` (a class with nothing but a static `operator new`) as the
  fifth base of a class that already has four non-empty ones changes neither
  `sizeof` nor any member offset - zoo-checked with a three-base test whose
  `offsetof` came out unchanged. So giving an existing modelled class the pool
  allocator, which is what its factory needs, is layout-safe. Do check the full
  match anyway; the failure mode would be silent and total.
- **A run of `addInfo` calls is an OnRegisterClass, and that locates a whole
  unknown class.** Scan .rdata for the function's address, and the slot it
  lands in tells you the vftable base (slot 4 = base + 0x10 for the screen
  family, whose slot 6 is the shared OnDestroyGame @0x455b70). Screen48D47C -
  the ScratchMix game screen, a class that was not modelled at all - was found
  entirely that way.
- **A named local also changes `sub reg,1` into `add reg,-1`.** Same lever as the
  rule below, one level down: `f(atol(s) - 1)` written inline emits `sub eax,1`,
  while `n = atol(s); f(n - 1);` emits the original's `add eax,-1`. Both are three
  bytes, so this never moves a displacement and shows up as the *only* defect in an
  otherwise byte-exact function. Zoo-test the two forms rather than reasoning about
  it - `+ -1`, a cast, and a separate `n--` statement all give `sub`.
  (CatchPanel::LoadPVI @0x472650, 99.69% -> 100%.)
  **AND IT MUST BE AN EXPRESSION ON THE LOCAL, NOT A REASSIGNMENT OF IT.**
  `n = m; n = n - 1; m = n;` is an in-place decrement and gives `sub`;
  `n = m; m = n - 1;` - the subtraction feeding a DIFFERENT destination -
  gives the original's `add -1`. Same three bytes either way, so it shows up
  as the only defect in an otherwise byte-exact function. When the following
  test reads the member back (`if (m == 0)`), cl forwards the store and tests
  the arithmetic's own flags, which is what the original's `mov [m],eax; jne`
  has. (TitleDirector::tickCoinLamp @0x44e880, 98.21% -> 100%.)
- **cl ATTACHES A SHARED BLOCK TO ITS *FIRST* PREDECESSOR; THE ORIGINAL
  ATTACHES IT TO ITS LAST - AND ONE OF THE TWO CASES IS STEERABLE.** The
  signature is a function that is structurally exact but whose every defect
  is a jump DIRECTION: the original jumps forward into a block laid out after
  both arms, ours jumps backward into one laid out inside the first arm.
  For a shared `return`, writing the success path as the nested positive test
  with the single `return 0` textually LAST moves it - `if (f() == 0) { work;
  if (g() == 0) { more; goto done; } } return 0;` gives the original's two
  forward jumps where `if (f() != 0) return 0; ... if (g() != 0) return 0;`
  puts the first arm's copy inline and makes the second jump back. For a
  shared JOIN (an if/else whose two arms fall into a common tail) nothing at
  source level moves it: if/else with the tail after, the same plus an
  explicit `goto`, the whole CFG spelled out with gotos, the arms swapped,
  and the join written into BOTH arms were all tried - the last of these does
  not even merge, it keeps both copies. Recognize the two halves separately;
  the return is worth fixing and the join is not.
  (Obj40EAC0::s4 @0x40eda0, 63.81% -> 71.32%, with all three shared blocks
  in one function.)
- **Cross-jumped `return 0` tails want plain EARLY RETURNS, not a wrapped success
  block.** When two failure paths reach one shared cleanup block that is laid out
  mid-function (here a parser destructor + `xor eax,eax` + a jump to the epilogue),
  write each failure as its own `return 0` and let cl merge them. Wrapping the body
  in `if (ok) { ...; return 1; } return 0;` instead attaches the *epilogue* to the
  failure path and makes the success path jump backwards into it - every forward
  branch in the function then sits a few bytes off. (Same function, 89.91% -> 99.69%.)
- **`x - y` as a subexpression vs. a named local changes which register the subtract
  targets.** For a length that must survive a call, `int len = end - start;` lets cl kill
  `end` and subtract in place, then copy (`sub eax,edi; mov esi,eax`); writing `end - start`
  inline at each use site (CSE'd) makes it copy first and subtract into the surviving
  register (`mov esi,eax; sub esi,edi`). Match the original by choosing the form, not by
  reordering statements. (getExeDir @0x401080.)
- **STRING/LIBRARY/GLOBAL-by-name annotations:** CRT calls (`fopen`…) and string literals
  in the original resolve only if annotated. `src/library.cpp` holds
  `// LIBRARY: EZ2AC_UNPACKED 0xaddr` + `// name` pairs (lookup-by-name in our PDB) and the
  `// GLOBAL: … // __security_cookie` pair. String literals get an inline
  `// STRING: EZ2AC_UNPACKED 0xaddr` comment on the line above their first use. Get CRT
  addresses from the original's call sites (objdump), never guess. match.sh passes
  `--nolib` so LIBRARY functions aren't scored.
- **Split `add esp` between two back-to-back cdecl calls** (call A; `add esp,N`; call B
  instead of one merged add): cl only emits this when block A is a **cross-jump tail** —
  i.e. the SOURCE has the same trailing call (e.g. `fscanf(f, "\n")`) at the end of BOTH
  arms of an if/else, and the tail-merger folded them. No flag and no single-copy source
  shape reproduces it; duplicate the trailing call into both arms.
- **Keeping an empty switch case's `sub/je` test:** `case 1: break;` gets folded into the
  default path (tests merge to `sub eax,2`). Write `case 1: goto label_after_switch;` —
  non-empty at parse (both `sub eax,1` tests survive), zero code after optimization, and
  a `two = 0` init before the switch stays hoisted as `xor` above the dispatch.
- **do-while vs while:** a source-level `do{}while` (even guarded) makes cl drop the
  ebp/aligned frame and take `ebp` as a scratch register. The original's rotated loops
  come from plain `while (cond)` — write while, let cl rotate. **The converse tells you
  when it really IS a do-while: if the loop's FIRST test is an exit and its SECOND is
  the one that jumps back to an aligned head, the first test cannot be the loop
  condition.** Written as `while` or `for(;;)` cl peels a copy of the first iteration
  ahead of the loop; `do { if (a()) break; } while (b());` gives the original's single
  loop head with two exits. (Obj40C180::m40c390/m40c430 @0x40c390/0x40c430, the two
  DirectInput polls: poll, and re-acquire only while the device reports it was lost.)
- **DERIVE THE WHOLE FRAME AT ONCE, DO NOT GUESS BLOCK SIZES ONE AT A TIME.**
  When several locals share a /GS frame and the reconstruction's `esp`-relative
  operands are off by a constant, the question is never how big each block is -
  it is where the frame's EDGES are. Resolve every `lea` and every spill to an
  absolute offset from the ENTRY esp (track the pending pushes at each site;
  the depth differs from line to line) and the layout falls out with no slack:
  the blocks' bases, their spacing, and the top buffer ending exactly on the
  cookie. Two things come out of that which guessing cannot reach - the real
  buffer size (a 260 reads as 256 until you see `path` end on the cookie), and
  **a value that has NO SLOT AT ALL**: when the derived layout is full, a local
  the original spills is living in one of the header blocks' words, which the
  reads that follow then overwrite. Writing `hdr[2] = x` where the twin has
  `volatile int step` is what the bytes say. Three size splits were tried on
  one function before the arithmetic was done; the arithmetic took it from
  82.50% to a 100%* effective match in one edit.
  (GFMainGameDirector::m45fd50 @0x45fd50, frame 0x26c = 3 x 0x20 + 2 x 260 + 4.)
- **`scratchpad/frame.py` RESTORES esp AT EVERY `call`, SO IT DRIFTS ON cdecl.**
  It assumes stdcall/thiscall, which is right for engine methods and wrong for
  the CRT. One `sprintf` and every offset it prints below that point is off by
  the argument bytes the callee did NOT pop, and the drift compounds per call.
  The symptom is seductive rather than obvious: it reported `_access` being
  handed F-0xe8 where `sprintf` had just written F-0x108, which reads as two
  buffers 0x20 apart and sent a whole tick looking for the second one. Both
  raw displacements resolve to the SAME buffer once esp is tracked by hand.
  So on any function that calls the CRT, take the raw `objdump` and track the
  pushes yourself; use frame.py only to find the interesting sites.
  **AND THE ARITHMETIC IS THE PROOF, NOT THE GUESS**: the layout is right when
  the blocks tile the frame with NO slack - the last buffer ending exactly on
  the cookie is what fixed m450690's path at 260, and the names array ending
  exactly on that buffer's base is what proved its records are 0x20 and not the
  8 the file holds. (ResultDirector::m450690 @0x450690.)
- **AND A HIGHER reccmp SCORE CAN BE THE WRONG CODE - THE MULTISET OUTRANKS
  THE PERCENTAGE.** In PlayerRecord::m466ac0 @0x466ac0 the skin-retune block is
  reached from two arms. Written out per-arm it scores **79.14%**; shared
  through a `goto`, which is what the original's single copy at 0x466d25 really
  is, it scores **75.00%**. The four-point-better version is the one that calls
  m4662d0 TWICE where the original calls it once - cl emitted both copies and
  declined to merge them, and the extra call bought more matching bytes than it
  cost. Nothing in the byte diff says "you added a call"; `scratchpad/callcmp.py`
  says it in one line. So when a source change moves the score by a few points,
  check the call multiset before believing the score - especially on the port
  track, where the multiset is the actual bar.
- **A CALL-COUNT MISMATCH IS A MISSING BLOCK, AND IT IS OFTEN ONE YOU
  MIS-DECODED AS DATA.** `scratchpad/callcmp.py` compares the call multiset
  and sequence, and on the port track that is the oracle - but its real value
  is catching what reading the listing did not. m450690 came out four calls
  short (-2 fwrite, -1 fopen, -1 fclose) because the block that reopens the
  file "wb" and writes the sorted table back sat in bytes an earlier `objdump`
  had started mid-instruction and rendered as `(bad)`. A misaligned start
  prints plausible-looking garbage; the call counts do not lie. Run callcmp
  BEFORE believing a decode is complete, not only to score it.
- **Deriving a stack buffer's exact size from the frame:** when several char buffers are
  contiguous, the last one's size is `chkstk_frame - buffer_base_off - sum(other buffers)`.
  DecryptFile's `out` is 131068 (0x1fffc), not the 131084 the prior RE guessed:
  `0x4009c(frame) - 0x20(base) - 0x80(header) - 0x20000(data) = 0x1fffc`. Getting a buffer
  size wrong shifts the whole frame and every esp-relative offset; verify against chkstk.
- **`char` vs typed array picks the /GS buffer order.** cl groups char arrays together and
  orders a mixed set (char + `int[]`) so the char buffers sit nearest the cookie, which
  reorders them vs declaration order. If the original keeps buffers in *declaration* order
  and indexes across them as one region, declare them all the SAME type (all `char[]`) —
  then cl lays them out in source order (fixed DecryptFile's header/data/out layout).
- **A natural register spill can be irreproducible (known-hard).** DecryptFile @0x410a40 is
  matched on every structural axis (layout, frame, branches, tail) EXCEPT one inner-loop
  detail: the original spills the wrap counter `be` to memory (all its uses are memory-
  compatible: `cmp [m]`,`sub [m]`), freeing a register for the byte. cl 13.10.3077 keeps the
  loop-carried `be` in a register instead. Forcing it (`volatile`, `int be[1]`, `&be`) works
  but each adds ops or reorders the /GS buffers. 50+ source forms, both cl builds (3077/4035),
  and every alloc flag were tried — see wip/crypt.cpp. When a single allocation choice resists
  every equivalent source, park it in wip/ with the analysis rather than shipping a hack.
- **A run of calls on one global, with the global read ONCE, is an inlined helper taking
  the pointer** — the `mov ecx,[g]; mov esi,ecx` … `mov ecx,esi` shape from runApp, at
  statement scale. Writing `g->m(a); g->m(b); …` reloads the global before every call (a
  call may write it). Group each run into a `static` helper taking the object; cl inlines a
  short one on its own but DECLINES ~7 calls, so those need `__declspec(forceinline)`.
  (Obj401780::s3 @0x418fa0 resets three key ranges that way.)
- **AND TWO memcpy CALLS IN AN if/else HOIST THEIR SET-UP ABOVE THE BRANCH.**
  `memcpy(dst, flag ? &a : &b, 32)` puts the branch first and the `mov ecx,8;
  lea edi` after it; `if (flag) memcpy(dst, &a, 32); else memcpy(dst, &b, 32);`
  gives the original's shape - the common head hoisted, only the two `lea
  esi` differing, one shared `rep movsd`. Same family as the rule below.
  (KGraphics::createOffscreenSurface @0x407450, 99.22% -> 100%.)
- **A ternary picking between two CONSTANTS goes branchless; two calls do not.**
  `f(c ? A : B)` compiles to `neg/sbb` flag arithmetic; writing the two arms as separate
  calls (`if (c) f(A); else f(B);`) lets cl cross-jump them back into ONE call fed by
  `push A; jmp; push B`, which is what the original has. (Obj401780::s2 @0x4184f0.)
- **Branch layout:** cl lays the `if`-body inline (fall-through) and the else/tail
  out-of-line. **This applies to a mid-function GUARD, not just an early return,
  and there it is worth several points**: `if (cold) { small; return; } <big
  ladder>` puts the cold block INLINE and pushes the whole ladder down, where the
  original has it out of line past the epilogue. Invert it - `if (!cold) { <big
  ladder> } else { small }` - and the layout comes back (the hold pump's
  below-GOOD restore @0x42f880, 72.15% -> 77.64%). To match an original whose early-return is the out-of-line tail, write the
  *hot path* as the `if`-body and the early-return as the trailing `return`. (Flipping
  `if (x==0) return 0; <work>` → `if (x!=0) { <work> } return 0;` fixed SatSub and the COM
  wrappers.)
- **N-way pick between calls: CONTIGUOUS tests then out-of-line arms is a grouped `&&`,
  not a chain.** A plain `if/else if/else if/else` interleaves (`cmp; jne next; <body>;
  jmp shared; next: cmp; jne …`), and a nested-negated chain gets the `je` polarity right
  but always places the OUTERMOST else last — so the block order can never match when the
  original's last block is the one tested *second*. What does match is grouping the first
  two tests with `&&` and letting one-step jump threading split them:

      if (m != 4 && m != 8) { if (m != 6) A; else B; }   // tests 4, 8, 6; blocks A,B,C,D
      else if (m == 4) C; else D;

  Threading turns `cmp 4; je ELSE` into `je C` because `==4` is the ELSE block's first
  test and it resolves TAKEN; `cmp 8; je ELSE` stays one step short and lands on D. cl
  threads exactly ONE level — that asymmetry is the fingerprint of this shape, and it is
  also why the outer mode grouping in the same function threads `==5` straight to its arm
  while `==9` goes to the shared test block. Eighteen chain/nest/switch permutations were
  compiled before this was found; brute-force the permutations rather than reasoning, and
  compare the (test order, target block index) signature.
- **Two int-to-float member stores through one pointer need two float LOCALS.**
  `c->f214 = (float)x; c->f218 = (float)y;` emits convert, store, convert, store. The
  original converting BOTH before storing either means the source named the results —
  `oy = (float)y; ox = (float)x; c->f214 = ox; c->f218 = oy;` — and the DECLARATION order
  of the two locals is what decides which `cvtsi2ss` comes first. (Effector6thStyle's ctor,
  three times over.)
- **Virtual / vtable calls:** model the callee as a class with enough dummy virtuals that the
  real one lands at the right slot (slot = byte-offset/4); no definitions needed (never
  instantiated). `call [ecx+off]` with `this` in ecx → a normal `virtual` method (thiscall).
  `push this; call [vt+off]` (this pushed as first stack arg) → a COM-style `virtual __stdcall`
  method. VC7.1 rejects `__thiscall` on a function-pointer type (C4234) — use the virtual-call
  model, not a typedef.
- **Magic-number division:** derive the divisor D from the magic constant + shift with the
  Hacker's Delight `magic_s32` algorithm (see git history), then just write `x / D`; cl
  regenerates the identical magic. (setVolume /16129, setPan /127.)
- **cl CONSTANT-FOLDS `a * k1 * k2` WHEN THE TWO CONSTANTS ARE ADJACENT.**
  `(float)n * 0.00390625f * 100.0f` comes out as one `mulss` against a folded
  0.390625f, where the original has two. Parenthesise the FIRST product -
  `((float)n * 0.00390625f) * 100.0f` - and both survive. The same parentheses
  also fix the multiply ORDER in a mixed constant/memory product: unparenthesised,
  cl schedules the memory operand ahead of the constant
  (`mulss xmm0,[esi+0x40]` before `mulss xmm0,[k]`), which is the opposite of
  what the original does. (EffectorDraw::drawReadout @0x41cb90.)
- **Constant multiply:** write `x * K`; cl picks `lea` chains vs `imul` to match.
- **cl CANONICALISES THE MULTIPLY ORDER INSIDE A FLOAT SUM - do not sweep it.**
  In `w*a + x*b + y*c + z*d` all twenty-four permutations of the four terms
  compile to BYTE-IDENTICAL code (swept twice, in both operand spellings, on
  concatMatrix @0x406ad0). cl 7.1 reassociates float adds - /Op is off by
  default - and schedules the products by DESCENDING memory offset of the
  operand it keeps in a register. So when an FP dot product or matrix kernel
  has every instruction right and the products in the wrong order, that is a
  wall, not a source-order problem. **What IS steerable is the operand
  SPELLING**, and it is worth several points: `a[j] * m[k]` puts `m` in the
  register and makes `a` the memory operand, `m[k] * a[j]` does the reverse -
  and flipping it also fixed the induction anchoring for free. Try the
  spelling; never sweep the order. (src/concatmatrix.cpp.)
- **A FOUR-ITERATION LOOP IS UNROLLED UNLESS THE COUNTER IS EXPLICIT.**
  `for (i = 0; i < 4; i++)` over a 16-instruction body gives 204 instructions
  fully unrolled; `i = 4; do { ... } while (--i != 0);` keeps it rolled at 72,
  which is what the original's `mov esi,4` / `sub esi,1` / `jne` has. A plain
  `while` and `for (i = 4; i != 0; i--)` also stay rolled but score a couple
  of points lower. (Same function.)
- **THE FLOAT/DOUBLE SPLIT IN AN x87 `sqrt`: put the cast OUTSIDE the product.**
  When the vector arithmetic is SSE but every SQUARE is x87 with a dword
  memory operand (`fld dword [x]; fmul dword [x]`, the same slot read twice),
  the source is `sqrt((double)(a*a) + (double)(b*b) + (double)(c*c))` - cl
  computes each float product in x87 at double precision under MSVC's
  FLT_EVAL_METHOD licence and folds each operand into the memory form.
  `(double)a * a` poisons the WHOLE chain into x87, subtractions included; an
  inlined `static double len3(double,double,double)` keeps the subtractions in
  SSE but CSEs the three promotions and leaves three dead `fstp st(0)`.
  (src/setcamera.cpp.)
- **Global data references:** a reconstructed global must carry `// GLOBAL: EZ2AC_UNPACKED
  0xADDR` above its definition, so reccmp equates it to the original's address (otherwise the
  referencing instruction shows a data mismatch even when otherwise identical).
- **Float/double pool constants now auto-match** (float_const.py patches above, x87
  AND SSE), including
  compiler-generated ones like the 2^32 unsigned→double fixup (`__real@41f0000000000000`,
  EzChart::rescale). Do NOT also add a GLOBAL annotation at the constant's address — an
  existing entity blocks the FLOAT analysis (`db.intersects`). A source-level named
  `extern const float k = V;` with a GLOBAL pin (k120 pattern) still works and reads better
  for real named constants, but is no longer required.
- **A STUB's ARGUMENT COUNT moves every branch displacement in its caller.**
  Declaring a stubbed callee with no parameters where the original passes one
  costs the call site a `push` (and, for a cdecl callee, its cleanup), so a
  caller that is otherwise byte-exact reports a long list of `jmp`/`je`/`jne`
  displacement mismatches and nothing else. When a function is structurally
  right but EVERY branch offset is off by a constant, count the arguments of the
  stubs it calls before suspecting the reconstruction.
  (GFMainGameDirector::loadGds @0x45faf0: 91.97% -> 100% by giving one stub its
  `KParser *`.)
- **A pointer initialized INSIDE a guarded loop's `if` still belongs outside it.**
  When the original computes the walking pointer between the guard's `cmp` and its
  `je` - `cmp [buf],dl` / `lea eax,[buf]` / `je skip` - the `lea` is scheduled
  before the branch because the SOURCE assigns it before the `if`. Writing
  `if (buf[0] != 0) { s = buf; do {...} while (*s); }` puts the lea after the je
  and also drags a nearby `xor` forward; `s = buf; if (*s != 0) { do {...} }` is
  what matches. (Panel::m428450 @0x428450: 98.25% -> 100%.)
- **The register pair inside an inlined `strlen` follows what is live AFTER it,
  not the strlen statement itself.** cl's /Oi expansion is `lea ecx,[eax+1]` /
  `mov dl,[eax]` / `add eax,1` / `test dl,dl` / `jne` / `sub eax,ecx`, and which
  of ecx/edx holds the walking base and which holds the byte looks unsteerable:
  five equivalent forms of the strlen statement (`buf[strlen(buf)+1]=0`, a
  pointer built from the result, `+= strlen`, a named length local, moving the
  statement) all give the identical wrong pick. What actually decides it is the
  NEXT statement - hoisting the following loop's walking pointer above its guard
  (the rule above) changed the pair and took @0x46f660 from 97.22% to 100%.
  When a strlen's registers are wrong, look at what comes after it.
- **`volatile` on a local pins it to memory, and a cast reads it back folded.**
  When the original keeps ONE of several same-shaped locals in memory and cl picks
  a different one (declaration order, else-arm shape and expression order all had
  no effect), `volatile int x;` forces cl's hand: the store becomes `mov [x],eax`
  and every use `mov reg,[x]`, which is the original's shape. The rider is that
  volatile ALSO stops cl folding the local into an arithmetic operand - where the
  original has `or esi,[x]` we get a load plus `or esi,reg`. Read that one site
  through a non-volatile alias, `*(const int *)&x`, and the fold comes back.
  (readGdsColor @0x4562c0: 52% -> 93.6% with volatile -> 100% with the alias.)
- **THE FIRST RELOAD AFTER A STORE-THROUGH-THE-RETURNED-POINTER TAKES edx, NOT
  eax, AND NOTHING CHANGES IT.** The shape is `p = make(); p->a = x; p->b = y;`
  where the first store goes through the returned pointer (still in eax) and the
  second must RELOAD `p` because that store could have aliased it. cl 13.10
  picks edx for the reload; the original picks eax. Both encodings are three
  bytes, so nothing shifts and the function scores as a 100%* EFFECTIVE match -
  but it is never byte-identical. Confirmed on FOUR independent functions at
  once (the four test-menu `buildMenu`s @0x474bf0/0x4750e0/0x475f40/0x476e40),
  each differing in exactly that one instruction and nowhere else, and against
  six source permutations (a local for the return value, swapping the two
  stores, extra casts, storing through the local - which deletes the reload the
  original really has). Recognize the shape and accept the 100%*.
- **NAMING THE INDEXED LOAD ALSO DECIDES WHETHER IT IS LOADED EARLY, AND CAN
  SPILL A COUNTER.** The same lever as below, at loop scale: `f(e->f0, ...)`
  written inline makes cl schedule the load AFTER the argument pushes and keep
  the loop counter in a register; `int h = e->f0;` at the top of the body loads
  it first and pushes the counter into its frame slot, which is what turns the
  walking pointer into the memory-resident `add [esp+X],0x10` form the original
  has. Worth trying whenever a loop body is right but its induction variables
  are in the wrong PLACE rather than the wrong register.
  (SlotOwner::randomizeHits @0x422a60, 72.05% -> 100%.)
- **A DEAD STORE THAT THE ORIGINAL KEEPS IS AN ADDRESS-EXPOSED AGGREGATE.**
  When the original computes a value into a frame slot and never reads it back,
  the source did not write a plain local - cl deletes that store at /O2. It also
  did not write a `volatile` local: volatile keeps the STORE but not the SLOT,
  because a value that is never read has an empty live range and cl packs every
  such local onto ONE slot. Only a struct or array whose address is taken
  somewhere (`FloatToInt(&v.field, ...)` is enough) both keeps the store and
  reserves a slot per field. Symptom: the frame is several words too small,
  every slot below it shifts, and the normalized listing shows two or three
  consecutive stores to the SAME slot. Declare the group as one struct in the
  original's slot order - lowest address is the first field - and the
  `__asm` argument homes drop BELOW the block, where the original has them.
  Worth 28 points on each of two functions (CatchPanel::m471c20 @0x471c20,
  m472020 @0x472020).
  **Compare the two frames with `scratchpad/frame.py`, which now takes an
  optional PE path** so it can normalize our build as well as the original -
  raw `[esp+X]` offsets are meaningless across the two because the push depth
  at each call site differs.
- **AN ACCUMULATOR COMBINED WITH A CALL'S RESULT MUST BE UPDATED IN ITS OWN
  STATEMENT.** `c = c << 8 | rand() % 256;` leaves the shift and the call
  unsequenced, and cl evaluates the CALL first: the new byte goes to a second
  register, the accumulator is shifted after it, and the two are ORed the other
  way round - four extra instructions per step and a register that should not
  exist. The original's `shl edi,8; call rand; and eax,..; or edi,eax` is the
  left-to-right reading, and only splitting the statement produces it:
  `c = c << 8;` then `c |= rand() % 256;`. Worth up to fifteen points on one
  function; thirty sites across the four panel families shared the defect
  (GFPanel::m464580 @0x464580 65.66% -> 81.13%).
- **A ZERO REGISTER cl DECLINES COSTS THE WHOLE FRAME, NOT JUST THE ZEROS.**
  When the original materializes 0 in a callee-saved register and cl spells it
  as immediates, the missing `push ebp` shifts EVERY `esp`-relative offset by
  four and the function reads as a near-total mismatch. The diagnostic is the
  instruction count: `@@ -0x...,520 +0x...,514 @@` differing by exactly six is
  `push ebp` + `xor ebp,ebp` + four `pop ebp`, and every other delta is a
  mechanical `0` <-> `ebp` substitution plus the frame shift. Extract both
  sides' push sequences from the diff and compare them before suspecting a
  missing argument - 19 `push ebp` against 18 `push 0` is the register, not a
  dropped parameter. Same unsteerable coin as the counter-vs-zero flip, but at
  50 points instead of five: recognize it and park immediately.
  (Panel::m42c5f0 @0x42c5f0, 55.51% with 514 of 520 instructions right.)
  **CORRECTED 2026-10-03: m42c5f0 WAS NOT A WALL.** Its danger flag was
  written as `goto danger` / `goto safe` labels (`g = 1` / `g = 0` once
  each). The original writes every arm as a structured `if (...) g = 1;
  else g = 0;` and lets cl merge the identical `g = 0` blocks; that is
  what gives the constant 0 enough uses for cl to put it in ebp. It is
  also what gives `jbe else` with the `g = 1` store inline: a `goto` arm
  inverts every branch. The `g_1b2e7d4` dispatch is a `switch` (`sub
  eax,1; je`), and its arms are laid out default first, then case 2, then
  case 1, which is their source order. 55.51% -> 76.48%. So before parking
  a zero register, look for goto-shaped control flow around stores of 0.
- **DIFF THE REGISTER-FREE CONTENT, NOT THE LISTING: scratchpad/semscan.py.**
  `RCJSON_DIR=$S .venv/bin/python scratchpad/semscan.py bNN.json [addr]`
  compares, per sub-100 function, MULTISETS of what survives register
  allocation: call targets by name (both sides through the reccmp JSON),
  globals by annotated name, float constants and string literals by VALUE,
  member displacements and small immediates. Register choice, scheduling
  and layout drop out, so what is left is a wrong field, a wrong constant,
  a missing or extra call, or a dead condition cl deleted. First run
  (2026-10-03) found seven behaviour bugs the aligned listings had hidden:
  swapped name-entry key rows, a song id passed as a path, truncated log
  formats missing their HRESULT, a `mode == 1` test that made an arm dead
  (a global read 1 vs 0 times), a mirrored colour ladder, a .scr name read
  8 bytes early, a missing gate reset. `scratchpad/callseq.py bNN.json addr`
  then shows the ORDERED calls and pushed strings of one function to place
  the difference. The JSON must belong to the binary in build/ - a stale
  JSON maps every recomp call to nothing. And `scratchpad/zq2.sh FILE SYM
  ADDR` is the zoo diff via the COFF section and its relocation table
  (scratchpad/zd2.py): it is not derailed by an inline-asm label inside an
  instruction (FloatToInt's `fistp`), which made zoo.sh stop after 14 lines.
- **`goto`-SHAPED CONTROL FLOW IS THE USUAL CAUSE OF A "REGISTER WALL".**
  Four functions documented as register permutations or unsteerable coins
  turned out to be control-flow shape (2026-10-03):
  - **A `goto next` filter inverts every compare.** `if (85 > d) goto
    next;` gives `comiss; ja next`; the original's `comiss d,[85]; jb next`
    is the POSITIVE test around a nested body: `if (d >= 85.0f && 1500.0f
    >= d && ...) { body }`. cl canonicalises `85 > d` and `d < 85` to the
    same code, so only the positive/nested form moves it. Same for a
    negated else-if (`!(699 < d)` -> `699.0f >= d`). (m438d30 64 -> 93.)
  - **A flag set through `goto danger` / `goto safe` labels** starves the
    constant 0 of uses; per-arm `if (...) g = 1; else g = 0;` gives it a
    register (m42c5f0, entry above).
  - **A loop whose exit block the original lays out LAST** is a `while`
    tested at the bottom with a `goto` from the early-exit straight to the
    code after the loop's own tail - here the song-end path skipping the
    `f725c` store to reach the clock advance (KEZPlayer::drainQueue 40 ->
    100, together with a store reordered per the same-base hoist rule).
  - **Per-band calls:** when each arm of a band ladder pushes its own
    argument and jumps to ONE shared `call`, the source called the
    function in every arm (m438d30's drawOne).
  And the converse: two arms the original keeps as separate copies but cl
  merges are spelled DIFFERENTLY in the source - an inverted `if (f18 !=
  10)` in one arm kept ModeSelectDirector::m44b4b0's two confirm copies
  apart (56 -> 90).
- **A BYTE-BY-BYTE COLOUR BUILT FROM rand() IS ONE EXPRESSION.** The
  original's random ARGB (`and eax,0x800000ff` per byte, `or edi,eax; shl
  edi,8` between calls) accumulates in ONE register; four statements
  (`c = rand() % 256; c <<= 8; c |= rand() % 256; ...`) make cl alternate
  two registers, which costs the function its zero register too. Write
  `c = (((rand() % 256 << 8 | rand() % 256) << 8 | rand() % 256) << 8) |
  rand() % 256;` - the calls still run left to right. Five gauge drawers
  moved at once (2026-10-03): CatchPanel::m471c20 85.82 -> 100,
  Panel::m42c0f0 80.65 -> 97.07, GFPanel::m464580 81.13 -> 95.04,
  GFPanel::m4649c0 68.36 -> 78.4, Panel::m42c5f0 76.48 -> 80.31.
- **A DEREF FEEDING THE *FIRST* ARGUMENT IS EVALUATED AFTER THE OTHER PUSHES,
  AND cl WILL NOT DO IT.** `f(*p, arr[i], c, d)` where `p` is a CSE'd address in
  a frame slot: the original evaluates `arr[i]` into eax, pushes d, c and eax,
  and only then reloads the slot and derefs it into eax again (`mov eax,[esp+X];
  mov eax,[eax]; push eax`) - textbook right-to-left. cl hoists both loads above
  the pushes instead and lands the value in ecx (`mov edx,[esp+X]; mov ecx,[edx]
  ... push ecx`). Same instruction count, one register renamed, the pair moved
  three pushes earlier. Six sites across EZ2DJMainGameDirector::m421ed0,
  m4221b0 and m422410 (98.40%, 97.06%, 97.55%) differ in nothing else, and a
  named `int *hi = &arr[j];` before the calls - the form the indexed-load rules
  below would suggest - changes nothing. Recognize the shape and take the 97-98%.
- **AN ADDRESS ARGUMENT MATERIALIZED *BEFORE* THE PUSHES IS READ THROUGH.**
  `f(&n->tex, 0, 0, n->tex.w, n->tex.h, 0)` evaluates the two sizes up front
  and leaves the `lea` to push time; the original's `mov ecx,[esi+0xd4]` /
  `mov edx,[esi+0xd0]` / `lea eax,[esi+0x50]` - all three registers loaded
  ahead of the first push - means the source read the sizes THROUGH the
  pointer it passes: `t = &n->tex; f(t, 0, 0, t->w, t->h, 0);`. That gives
  the address two uses, so cl computes it first, and it then folds the
  member reads back to node-relative displacements, so the addressing in the
  listing is IDENTICAL either way and only the schedule and the register
  rotation differ. Naming the pointer alone does nothing - it is the second
  use that matters. (KProfile::drawTree408bf0 @0x408bf0, 83.33% -> 100%.)
- **NAMING THE INDEXED LOAD DECIDES WHICH REGISTER CARRIES IT.** A call whose
  arguments are `(arr[i].f, K, K, a, b)` loads the three non-constants in the
  same right-to-left order either way, but written inline the indexed load takes
  the LAST free register and the two reloaded parameters take the first two;
  hoisted into a named local (`int h = arr[i].f;`) the assignment rotates and
  matches the original. Every instruction is identical, so this shows up as
  three renamed registers and nothing else - the same family as the accumulator
  rule below, and just as cheap to try. (SlotOwner::applyTracks @0x422bf0.)
- **DECLARING THE ACCUMULATOR BEFORE THE POINTER IS WHAT RESERVES eax FOR THE
  ZERO.** The cousin of the coin-flip rule below, and this one IS steerable.
  `T *p = &base[i]; int total = 0;` lets cl use eax for the index arithmetic and
  materialize the zero afterwards; `int total = 0; T *p = &base[i];` makes it
  emit `xor eax,eax` in the middle of the address math and compute the index in
  ecx/edx instead - which also decides the register the loop body then uses.
  Every instruction is the same either way, so it shows up as nothing but three
  or four renamed registers. Try the swap before concluding it is the coin flip.
  (PlayerSlot::sumTracks @0x42f480, 74.29% -> 100%.)
- **THE STACK PACKER'S SLOT ASSIGNMENT IS NOT STEERABLE FROM SOURCE, and it
  now costs SIX functions.** The three time-sliced shuffles - EZ2DJ
  @0x421750/0x4219c0, ScratchMix @0x45e770/0x45e9f0, Ez2Catch
  @0x46df00/0x46e170 - are one source in three dialects, and all six sit at
  80-82% with every instruction present and only frame SLOT NUMBERS wrong. The
  evidence that no source shape fixes it is unusually strong, so do not spend
  another session on it:
  (a) the /GS aggregates ABOVE the scalars land at identical offsets on both
      sides, so both frames reserve the same block and only assign it
      differently - it is a permutation, not a size error;
  (b) the ORIGINAL compiler produced three DIFFERENT layouts from what is
      essentially one source (EZ2DJ homes r1 low, ScratchMix homes the track
      list low), so the driver is register pressure from each dialect's own
      differences, not anything the source says;
  (c) declaration order (five permutations on the EZ2DJ pair, plus A/B swapped
      and r2 hoisted to function scope on the scratch pair), inner scoping, and
      reusing locals all leave the assignment untouched;
  (d) reconstructing the stubbed callee did nothing either - see the bound on
      the callee-first rule above.
  Recognize the signature - a big function whose every defect is `[esp+X]`
  against `[esp+Y]` with the same instruction - and take the 80%.
  **RE-CHECKED 2026-08-22 with the frame-derivation method that fixed the
  three chart loaders, and it does not apply.** That method finds a SIZE
  error - a buffer that is 260 rather than 256, or a value with no slot at
  all - and here the frame size is identical (0xd8), the early spill COUNT
  is identical, and only the assignment differs. One correction to (a)
  though: the aggregate bases do NOT land identically, they are 4 apart, so
  the note's "both frames reserve the same block" is too strong. Making
  `step` volatile to buy a slot back is 81.56% -> 73.85%.
- **A ZERO REGISTER THAT LIVES FROM A GUARD TO A LATER LOOP IS `i = 0`
  BETWEEN TWO NESTED TESTS.** `if (countListA() > 1 && tl->count > 0) {...}`
  folds the zero into immediates for the rest of the function; the original's
  `cmp [esi+4],edi` guard and the edi it keeps alive through the option tests
  come from `if (countListA() > 1) { i = 0; if (i < tl->count) { do {...} } }`
  - the zero materialized BETWEEN the outer and inner test. Thirteen other
  forms (named zeros, early `slot = 0`, a zero argument) had been swept as a
  "coin flip" before this was found; try the nesting before parking.
  (PlayerSlot::beginStage @0x4305a0 95.60% -> 100%, and its catch twin
  @0x473fd0 first build.)
- **A zero register vs. the loop counter is a coin flip cl does not expose.** When
  a loop has exactly three long-lived values and a fourth candidate, cl 13.10 puts
  three in callee-saved registers and spills the rest; whether the survivor is the
  materialized zero (giving `cmp reg,ebx` / `push ebx` / `mov [m],ebx` and a spilled
  `mov eax,[i]; add eax,1; cmp; mov [i],eax` tail) or the counter (giving immediate
  zeros and `add ebx,1`) is decided by its own ranking, and no declaration order,
  statement order or scope changes it. Recognize the shape and park rather than
  chase it. (wip/loadtxg.cpp @0x408f70, 78.71%.)
- **Hoisting a pointer OUT of a loop is worth trying whenever the frame is 4 bytes
  too big.** An out-parameter the loop body then reads through (`loadTexture(name,
  &atlas, 0)` … `atlas->w` inside the loop) keeps its stack slot live for the whole
  loop; copying it to a local first (`tex = atlas;`) kills the slot at the copy and
  lets cl's packer reuse it for a later local — which is exactly how the original's
  `sub esp,0x248` comes out one slot smaller than the naive reading. The copy is
  also what puts the pointer in esi for the whole loop. (Same function, +6.7%.)
- **A subclass built by INLINING its ctor into its owner needs an INLINE ctor.** The
  rule below says to declare an out-of-line ctor to force a subclass's vftable out;
  when the owner's constructor shows `lea ecx,[this+off]; call Base::Base; mov
  [that],SubVt` — the base's ctor plus a bare vptr store, with no call of the
  subclass's own — an out-of-line ctor is exactly wrong: cl emits a real
  `call Sub::Sub` instead. Define it `{}` INSIDE the class. Declaring it at all is
  what lays the vftable down; where it is defined only decides whether cl can expand
  it. (Obj410100 @+0x7230 of Obj4102C0, vftable 0x488790.)
- **`operator delete` is decided PER CLASS, and the deleting destructor says which.**
  A class deriving from EzObject inherits the pool's `__stdcall operator delete`
  @0x401720 (`push this; call` with no cleanup); a class whose `??_G` ends in
  `push esi; call 0x4788ca; add esp,4` (the CRT `operator delete`, a `jmp free`)
  is NOT pooled and hides it with its own inline
  `static void operator delete(void *p) { ::operator delete(p); }`. Read the `??_G` before
  assuming the family's allocator. (Obj4102C0 @0x4104a0.)
- **CHECK A STRUCT CARVE BY *PRINTING* THE OFFSETS, NOT BY ASSERTING THEM.**
  The rule below says a `_pad` is not free space and to run the full match
  after touching one. Two seconds of zoo beats four minutes of match for the
  first pass: write one function per offset -

      #define OFF(m) ((int)((char*)&((T*)0)->m - (char*)0))
      int f_size() { return sizeof(T); }
      int f_x()    { return OFF(x); }

  each folds to a single `mov eax,IMM`, so `zoo.sh` plus
  `grep -E '^[0-9a-f]+ <|mov    eax' | paste - -` prints one offset per line
  with no program to run. Printing beats a negative-array static assert
  because an assert says only THAT something moved, where the numbers say by
  HOW MUCH - which is what localises the missing pad. Carving
  TitleDirector's twelve ctor fields lost four bytes on the first try (a
  field was followed by a four-byte gap rather than abutting the next one)
  and every offset from there down was exactly 4 low, which named the culprit
  immediately. Then run the full match, which is the actual verdict.
- **ADDING FIELDS TO A VERIFIED STRUCT BREAKS MATCHED FUNCTIONS, EVEN INSIDE
  `_pad`.** Carving six evidenced fields out of EZ2DJMainGameDirector's padding -
  every offset read off a real function - dropped FOUR already-matched methods of
  that class (80-98%) and one unrelated function in another TU. Reverting
  restored all five. A `_pad` is not free space: the matched functions encode the
  layout as it is, and any size or alignment change downstream of your insertion
  shifts them. **Always run the FULL `./match.sh` after touching a struct, never
  just the function you are working on** - the damage shows up in files you did
  not open. If the fields are only needed by a function you are parking, leave
  them in the wip/ file. (wip/maingame-gds.cpp.)
- **Before modelling a class, grep the headers for its address.** Half-finished
  models accumulate: Obj4102C0 already existed in include/ksongplayer.h, named from
  its matched `tick()`, with the two KLocks under placeholder names (`m_lock`,
  `m_gate`) and the root's dwords as `f0`/`f4`. Merging into the existing struct kept
  tick() at 100% for free; a second `struct Obj4102C0` is a C2011 and an afternoon.
- **A vtable is only emitted where cl generates a ctor/dtor.** If a class has no
  ctor/dtor definition in any TU (e.g. the original's subclass ctor exists only INLINED
  into its owner's ctor - `call Base::Base; mov [this],SubVt`), cl never lays down
  `??_7Sub@@6B@`, so the `// VTABLE:` marker silently creates nothing and the vtable entity
  never appears in the table. Fix: declare an out-of-line ctor for the subclass and define
  it (empty) in the .cpp purely to force the vftable out. The standalone ctor our build
  then emits is unannotated and therefore unscored. (Obj410100 / vtable 0x488790.)
- **`setne` instead of a branch = the two returns must be written as separate blocks.**
  When the original ends a predicate with a real branch to a shared `mov eax,1; ret` (or
  duplicates a store into both arms), the natural `x = ...; return x != 0;` makes cl compute
  the result with `xor ecx,ecx; test; setne cl; mov eax,ecx` and clone the whole tail.
  Rewrite so each return is its own block and the SHARED one is the fall-through:
  `if (p == 0) { *cur = p; return 0; } *cur = p; return 1;` — duplicating the store in the
  source is what reproduces the original's two stores. Confirmed twice: nextListA
  @0x4182e0 (shared `return 0` tail) and nextOwnedNode @0x401900 (duplicated store).
- **A huge member offset mis-symbolized as an address — FIXED in reccmp (patch 4).** This
  engine has a ~0x420000-byte base class, so ordinary struct offsets are larger than the
  image base and used to collide with our own reconstructed globals: the recomp side printed
  `g_tracks+16252` where the original printed the raw displacement, and the function scored
  <100% with identical bytes. Patch 4 above fixes it by only naming a register-relative
  displacement when it is an entity's exact base, or when the operand has no base register.
  If you see this symptom again, check the patch is still applied rather than parking the
  function. (Obj40C760::pending @0x40c760 was parked on this and is now matched.)
- **A large IMMEDIATE colliding with our image is the SAME problem, and it is fixed by
  `/base`, NOT by a patch.** `mov eax,0x10624dd3` is the magic multiplier for a signed
  divide by 1000; at the old `/base:0x10000000` it landed 6 MB into our own .data and our
  side symbolized it against the original's raw number. Patch 4's rule cannot be extended
  to `mov reg,imm`: unlike a displacement, `mov` is as often a REAL interior data reference
  (copyLane @0x412790 loads `&g_tracks[0].records[n]`) as a constant, so an exact-base-only
  rule breaks one to fix the other — it was tried and reverted. Fix it structurally: pick a
  base no constant in the original's game code can reach. Scan every `0x[0-9a-f]{7,8}`
  literal in 0x401000..0x478000 (783 of them >= 16 MB), then choose a 16 MB-aligned base
  whose clear span above it dwarfs our image. **0x11000000 has 80 MB of clear span, ten
  times the current image**; re-run that scan before moving the base again.
- **VTABLE/FUNCTION markers must live in a `src/` file** — reccmp-project.yml sets
  `source_root: src`, so markers in `include/*.h` are silently ignored (the vtable entity is
  never created and the `mov [this],vtable` operand stays unresolved). When the class/struct is
  defined in a header, pin the vtable from the `.cpp` with `// VTABLE: EZ2AC_UNPACKED 0xADDR`
  followed by a `struct X;` forward-declaration (reccmp's `get_class_name` reads the name off
  that next line). Add the marker only once every vtable slot is a matched function. (KProfile.)
- **One virtual `~T()` definition emits BOTH the real dtor (`??1`) and the scalar deleting
  dtor (`??_G`)** at two addresses. Map them with two markers on the single definition: a plain
  `// FUNCTION: 0xADDR` directly on `T::~T()` binds `??1`, and a separate blank-line-separated
  `// FUNCTION: 0xADDR` + `// T::`scalar deleting destructor'` comment binds `??_G`. (KFile
  @0x413200/0x413220.) If `~T()` is a big EH function the deleting dtor *calls* it instead of
  inlining — same two-marker setup still works.
- **TWO STRING LITERALS ON ONE LINE NEED TWO MARKERS ON SEPARATE LINES.**
  Stacking both `// STRING` markers above the call breaks BOTH. Split the call
  so each literal begins its own line and put its marker directly above that
  line. Without a marker, OUR side symbolizes the literal while the original's
  operand stays raw hex - the mirror image of the usual failure, and it reads
  as `-push 0x48fac0` / `+push "..." (STRING)`.
  (EZ2DJMainGameDirector's ctor @0x423f40.)
- **A SECONDARY BASE MUST BE `__declspec(novtable)` too.** Otherwise cl stores
  each base's OWN vftable into its slot before storing the derived ones, and a
  constructor comes out with one extra store per secondary base that the
  original does not have. (The three MainGameDirectors, three sinks each.)
- **A parameter that receives the CALLER'S OWN secondary-base pointer has to be
  `void *`.** Typing it as one class's sink makes the other caller's cast a
  no-op - `push esi` instead of `lea eax,[esi+0x10]` - because the conversion
  is only an adjustment when the type really is a base of that class. (Same
  family: Obj450570::m450420's third argument.)
- **A LOOP BOUND IS AN ADDRESS, so it must resolve the same way on BOTH sides.**
  A loop that walks a table by pointer ends on `cmp reg, <one-past-the-end>`.
  Modelling three adjacent tables as three separate globals let our linker put
  unrelated entities at those boundaries, and every bound mismatched even
  though the loops were right. Spanning the whole region with ONE array and
  indexing at named offsets makes each bound an interior reference to that one
  entity - identical by construction, and immune to how the linker orders .bss.
  (ModeSelectDirector's ctor @0x44b8f0, four bounds at once.)
- **A LOOP COUNTER SPILLED TO THE FRAME is the dead-loop-variable rule under
  register pressure** - and it costs four bytes of frame, which moves every
  stack local and every `esp`-relative offset with it. If a big function is
  structurally right but every local is off by a constant, count the induction
  variables in the original's loops before suspecting the layout. (Same
  function: fixing one loop took it 88.69% -> 98.60%.)
- **What CREATES a float zero decides which register a nearby constant gets.**
  With nothing zeroing xmm0 between two constant stores, cl puts the second
  constant in xmm0 and defers the `xorps`; the original had it in xmm1 with the
  `xorps` ahead. Only a real zero STORE moves it - a named `float zero = 0.0f`
  with no use just sinks to its first use, and naming the constant (even with a
  GLOBAL pin) changes nothing. Moving an unrelated zero store up is what buys
  it, at the cost of that store's own position. (Same function; this is what
  leaves it a 100%* EFFECTIVE match rather than byte-identical.)
- **A BROKEN `// STRING` MARKER READS AS "Failed to find a match at 0xADDR",
  not as a low score.** Wrapping a call across two lines with the literal on the
  second line breaks the marker, and a broken marker takes the WHOLE function's
  annotation down with it - the function disappears from the table entirely.
  When reccmp says it cannot find a function you just annotated, check every
  STRING marker in it before suspecting the reconstruction.
  (AudienceRatePanel::AudienceRatePanel @0x41aa30.)
- **cl CANNOT hoist a call above a store to memory the call might touch.** So if
  the original calls something BEFORE a member store, the source called it
  before that store too - read the count into a local first. This is a rare
  case of the original's instruction order proving the source's statement order
  outright rather than merely suggesting it. (Same function.)
- **COMPARE JUMP TARGETS, NOT JUST INSTRUCTIONS - THE ZOO CANNOT SEE A WRONG
  BRANCH.** scratchpad/zoo.sh's diff masks every `call`/`j*` operand, so a
  branch to the wrong place is invisible there, and reccmp shows it only as a
  displacement that differs by a few bytes. scratchpad/jt3.py disassembles
  both images (capstone), aligns them (difflib) and reports every jump whose
  target, mapped through the alignment, is a different instruction;
  scratchpad/jt4.py prints the original's target, ours and the aligned spot
  for triage, scratchpad/jctx.py the whole function side by side (all three:
  `RCJSON_DIR=<dir of the --json> .venv/bin/python scratchpad/jt4.py b.json
  [0xADDR]`). Most hits are block order or register choice; the rest were
  real: an `if` whose conditions were negated, a `commit()` outside the block
  it belongs to, a free on a path that skips it, a release path missing its
  `goto`, a switch whose arms carried the wrong case labels. Run it after
  every batch of reconstructions.
  Its companion scratchpad/mnscan.py compares per-function COUNTS of rarely
  noisy mnemonics (movsx/movzx, div/idiv, shifts, float ops, calls): a
  signed divide where the original is unsigned, a swap loop of the wrong
  length, a draw through the wrong primitive each showed up as one count
  that differed. Discount `call` counts (tail duplication) and judge the
  rest by reading the two blocks.
- **cl THREADS A BRANCH THROUGH A NAMED LOCAL'S KNOWN VALUE, NOT THROUGH A
  CSE'd RE-READ.** RankingDirector::m4577b0 @0x4577b0 tests a name for
  "all 'A'" || "all ' '". With the first character in a named local `c0`,
  cl knows `c0 == 'A'` after the first test and sends every later 'A'
  mismatch straight to the exit, skipping the `c0 == ' '` test. The
  original's misses all land on that test (`cmp bl,dl` with bl = the cached
  byte): written as `NB(0) == 'A' ... || NB(0) == ' ' ...`, cl still caches
  the byte in bl by CSE but does not thread on it. 69.33% -> 100%. So when
  the original re-tests a value whose outcome is already implied, delete
  the local you cached it in.
- **cl HOISTS A LOAD ABOVE A STORE ONLY WHEN BOTH ARE MEMBERS OFF ONE BASE.**
  `Panel::loadGdsItem` @0x427820 (and the GF twin @0x460b50) read the second
  colour's byte 1 BEFORE storing the first colour, mid-way through composing
  it. Written through two pointer locals (`*pa = ...; g = (*pb >> 8)...`) cl
  must assume they alias and keeps the load after the store - 98.96%. Writing
  the same reads and stores as `e->f1c` / `e->f20` (one base, offsets 0x1c and
  0x20, provably disjoint) frees the scheduler and both go 100%. The pointer
  locals still exist for the register they hold (`a = ((unsigned char *)pa)[3]`
  keeps `[ebp+3]`); only the accesses that cross the store change. So a load
  the original issues EARLIER than your store is a statement that the source
  named both locations off a common base, not a statement order.
- **Two stores in an if/else beat a ternary when the original schedules the
  compare next to the branch.** `p->f = (n == 1) ? A : B;` evaluates the compare
  at the call's result and pushes the constructor's `mov eax,ebx` into the
  epilogue; writing `if (n == 1) p->f = A; else p->f = B;` lets cl cross-jump
  the two stores back into one AND sink the compare down beside the `je`, with
  the return value filling the slot between them. Same family as the
  "two calls do not go branchless" rule, one level down. (Same function.)
- **A member addressed off a LIVE POINTER rather than off `this`.** When a
  constructor's later stores read `[ebp+0x214]` where ebp was set by
  `lea ebp,[this+off]` for the member's own constructor call, the source held
  the member's address in a local across the whole body: `Obj402860 *c =
  &chart;` declared at the top and used for every later touch. Writing
  `chart.f214 = ...` addresses off `this` and never matches. (Same function.)
- **THE FLOAT REGISTER ASSIGNMENT FOLLOWS SOURCE ORDER; THE EMISSION ORDER
  FOLLOWS THE SCHEDULER.** Panel's constructor emits its 512.0 store first and
  its six zero stores after, but puts the ZERO in xmm0 and the constant in
  xmm1 - because in the source the zeros come first. Write the constant first
  and it takes xmm0 and never matches, however the stores end up ordered.
  (Panel::Panel @0x429b70.)
- **AN EFFECTIVE-100% WHOSE ONLY DEFECT IS A FLOAT *LOAD* ONE INSTRUCTION
  EARLY MEANS THE FLOAT STORE CAME FIRST IN THE SOURCE.** Write two integer
  stores and then a float member store and cl emits load/store/store/store in
  source order; write the FLOAT store first and the P4 scheduler hoists the
  two independent integer stores above it to cover the load's latency, giving
  `movss xmm0,[k]` / `mov [a],edi` / `mov [b],edi` / `movss [c],xmm0` - the
  original's shape. Same instructions, one moved, so it scores as a 100%*
  effective match and is easy to accept; the fix is a two-line reorder.
  (KEZPlayer::m410110 @0x410110.)
  **BUT CHECK WHICH OF THE TWO IS ACTUALLY WRONG FIRST - the load and the
  store are separately observable.** When the constant load already sits at
  the head of the block (typically because the block is a branch arm and
  nothing precedes it), it is hoisted whatever the source order says, and
  then the STORE order follows the source directly - so the float store goes
  where the original's is, i.e. AFTER the int stores. The two functions look
  identical in the diff (`movss [k]` / `mov [m],reg` / `movss [f],xmm0`) and
  want opposite source orders, so read whether the misplaced instruction is
  the load or the store and flip only that one.
  (TrackSink::m4299e0 @0x4299e0, the counter-example.)
- **WHEN EVERY VALUE IS STORED TWICE, THE GROUPING OF THE STORES DECIDES
  WHETHER cl SPILLS OR DUPLICATES.** Filling two same-shaped structs from
  four converted values, the four conversions happen first either way - so
  this is NOT evaluation order, it is how long each value stays live.
  Grouped BY STRUCT (all of A, then all of B) cl keeps all four on the x87
  stack and duplicates with `fld st(n)`; grouped BY CHANNEL (each value into
  A and B back to back) it spills the later two to scratch and copies their
  BITS into both with integer moves, keeping only the first two in registers.
  The spread is wide, so it is a real lever and not noise: by channel 100%,
  A-then-B 78.95%, B-then-A 76.32%, a chained `a = b = v` 60.15%, and B read
  back out of A 58.99%. Worth trying on any function parked on "cl keeps a
  float in a register where the original spilled it".
  (KGraphics::setMaterial @0x4069d0.)
- **Float PAIRS set through an inlined two-argument helper stay where they are;
  six plain assignments float to the top.** `setPair(&a, x, y)` three times
  keeps the `xorps` block after the integer stores where the original has it;
  written out as six assignments cl hoists the whole block to the head of the
  function. (wip/gfpanel-ctor.cpp, which is 96.18% for want of the same
  block's register choice - read it before touching the sibling ctors.)
- **AND WHEN THE INDEX IS STILL LIVE (a float conversion, an address) BUT
  A DOWN-COUNTER EXISTS ANYWAY, WRITE THE COUNTER EXPLICITLY.** cl will not
  invent a second induction variable for `for (i = 0; i < 11; i++)` when i
  is converted in the body - it compares i. `for (i = 0, n = 11; n != 0;
  n--, i++)` gives the original's `mov [esp+X],0xb ... sub [esp+X],1` beside
  the live i. The converse trap is a `short` loop variable: `for (r = 0;
  r < 10; r++)` on an `unsigned short` REVERSES into a counter; an `int r`
  in a do-while compared as `(unsigned short)r < 10` keeps the original's
  memory-resident r and its `cmp cx,0xa; jb`. (KGraphics::createFlag
  @0x406620, 55% -> 100% across those two loops.)
- **A SEPARATE DOWN-COUNTER MEANS THE LOOP VARIABLE IS DEAD.** A loop ending
  `add esi,4; add edi,1; sub ebp,1; jne` has three induction variables where
  two would do: `i` itself is never used, because both of its uses (`i + 1` for
  a printed number, `&arr[i]` for the walk) became their own IVs, leaving
  nothing to test. Writing `for (i = 1; i <= 12; i++)` and indexing `[i - 1]`
  keeps `i` live and gets `cmp esi,0xc; jle` instead.
  (DrawModeIntroDirector's ctor @0x46a260.)
- **Derive the /GS buffer size from the frame every time - 256 and 260 both
  occur.** Two constructors in the same family differ: DrawModeIntro's is
  char[260] (frame 0x110 = res + this + 260 + cookie) and GFPanel's is
  char[256] (frame 0x108 = this + 256 + cookie) even though the original passes
  260 to GetCurrentDirectoryA out of it. Four bytes wrong shows up as nothing
  but the frame size.
- **ASCENDING EH states mean an INLINED BASE DESTRUCTOR, not members.** The rule
  below (state = index of the next member, so the sequence DESCENDS) covers a
  class's own members. A base sub-object destroyed by an INLINE base destructor
  is a different region kind and numbers the other way: `~Obj40EAC0` @0x40eae0
  runs 0 during the body, 1 while it destroys the base's member, then -1 for the
  base proper. It is the ONLY ascending destructor in the binary
  (scratchpad/scan4.py finds them), and the shape is unreachable by adding
  members - one member gives [1, 0, -1], two give [2, 1, 0, -1]. What produces
  it is `~EzSubsystem() {}` written INLINE IN THE HEADER, so cl inlines it into
  every derived destructor and the member it owns is destroyed in a base region.
  When a destructor's states ascend, look for a base whose destructor is inline
  and owns the member being destroyed - do not move the member into the derived
  class, which is what the operands appear to say and is wrong.
- **The EH state in a destructor is the INDEX OF THE NEXT MEMBER TO DESTROY** -
  equivalently, how many are still alive; -1 means only the base is left. A
  destructor whose body can throw opens by storing the member COUNT; one with an
  empty body has no opening store and just sets count-1 right before the last
  member goes. So `mov [esp+X],0` around a body with NO member destruction
  anywhere means ZERO members, not "a member whose destructor was inlined
  away" - and modelling it as a second base with an empty inline destructor
  makes cl restore that base's vptr and grow the frame (75.41% on
  ~TestModeDirector; removing it is 100%).
- **Pinning a vtable forces every virtual to be DEFINED, which surfaces the
  ICF-folded slots.** While a class is never instantiated cl emits no vftable
  and declared-only virtuals link fine; the moment a `// VTABLE:` marker exists
  the vftable is emitted and every slot needs a definition. Expect one or two
  of them to be leaves already matched under another class's name at a shared
  address - give those a `folded` annotation on the class's own method (EzChart
  slot 3 @0x40f380 = `mov eax,[ecx+4]; ret`, folded onto Obj40F380::getA).
  A deleting destructor emitted at the same time needs kcrt.h's inline
  `operator delete` or it calls `operator delete` where the original calls
  `free`.
- **A 24-byte zero block is a memset, not six assignments.** Six assignments
  reuse whatever zero register is already live and address off the object;
  memset's /Oi expansion materializes its own `xor` + `lea` and works through
  the pointer. (Bigger blocks become `rep stosd` and are unambiguous.) The
  same function's forty `= -1` stores ARE a plain `for` loop - cl unrolls it
  and hoists the constant into a register, giving the original's six-byte
  `mov [..],eax` instead of a ten-byte immediate store.
  (CatchPanel::CatchPanel @0x471380.)
- **The same string literal used by two functions needs only ONE `// STRING`
  marker.** The entity is keyed by address; a second marker elsewhere is
  redundant (DrawModeIntroDirector logs "ModeSelectDirector::Delete" verbatim).
- **A SECOND VPTR STORED AT THE TOP OF A DESTRUCTOR IS MULTIPLE INHERITANCE.** A
  member's vptr is never restored in its owner's destructor, so `mov [esi],VT1;
  mov [esi+N],VT2` at the top can only be the object's own two vptrs. reccmp
  handles it with the base-class form of the marker, one per vtable:
  `// VTABLE: EZ2AC_UNPACKED 0xADDR BaseName` resolves against
  ``X::`vftable'{for `BaseName'}``, and the bare form only against ``X::`vftable'``.
  The two tables sit adjacent in .rdata (secondary first), so a naive dump of the
  primary's address looks like one long table - count the slots from the class
  shape, not from the dump. **The secondary slot pointing STRAIGHT at the override
  (not at a 5-byte `sub ecx,N; jmp` thunk) means the override is compiled for the
  SECOND base's `this`, so it must be declared ON that base, not on the derived
  class** - declaring it on the derived class makes cl emit the adjustor thunk and
  the slot then points at the thunk. Fields of the derived object that such an
  override touches appear at (offset - N) from its `this`; model them as a
  separate view struct and cast, which costs no code. (GameOverDirector @0x44d960,
  WarningDirector @0x45c1e0, TestSound @0x4768b0.)
- **ONCE A CLASS HAS TWO VPTRS, *BOTH* MARKERS NEED THE BASE-CLASS FORM** - the
  PRIMARY one included. cl names it ``X::`vftable'{for `EzObject'}``, not
  ``X::`vftable'``, so the bare marker resolves to nothing and the constructor's
  first vptr store prints `X (VTABLE)` against the original's real name. Symptom:
  a two-store constructor that is otherwise byte-exact reports exactly two
  mismatched operands. (TestLighting's ctor @0x476020, 94.74% -> 100%.)
- **A SECONDARY BASE'S OWN VFTABLE FULL OF `_purecall` MEANS IT IS ABSTRACT, and
  the intermediate that implements it is `novtable`.** When the constructor stores
  THREE vftables - the sink's own first, then the class's two - the first is the
  sink base's, and if its slots point at `_purecall` @0x479a20 the base declared
  the virtual pure. But the derived's secondary slot points STRAIGHT at the
  override (rule above), which a pure virtual cannot give you and a plain override
  on the derived class turns into a thunk. What produces both at once is a
  per-class `__declspec(novtable)` sink deriving from the abstract one and
  declaring the override: novtable is what stops the intermediate storing a vptr
  of its own, so the count stays at the original's three.
  `_purecall` needs a LIBRARY annotation in src/library.cpp or the abstract base's
  vftable scores 0% with its one slot already right.
  (TestPageSink @0x49ec9c; TestLighting @0x476020, TestBookkeeping @0x474d80.)
- **The root EzObject/KEZ2DJObject is 0x10 bytes, not 4.** Its own object record
  @0x4a8e60 says {0x10, 1, 0}, and the multiple-inheritance classes above prove it
  by putting the second base's vptr at +0x10 exactly. Derived classes pad from
  0x10.
- **A gap to the next member is an UPPER bound on a member's size, not the size.**
  Obj40B2E0 read as 0x554 from ModeSelectDirector's gap and as 0x524 from
  GFResultDirector's; its destructor's highest access is +0x470, so 0x474 is all
  that is evidenced. Pad explicitly to the following member at every use site
  rather than letting one guessed size decide the layout.
- **`__ehvec_dtor_iterator` @0x47a12b needs a LIBRARY annotation** (name
  `` `eh vector destructor iterator' ``) or every array-member destruction reads as
  `call <OFFSETn>` against our CRT's name, with every byte already right.
- **Two scans find destructors, and destructors are the cheapest class entry.**
  (a) unmatched functions that CALL 0x47a12b - those are the ones with array
  members; (b) unmatched functions that store a `0x4xxxxx` constant into `[reg]`
  in their first dozen instructions AND whose LAST call is the folded empty
  `ret` @0x402880 (or the thunk @0x426e40) - that is every class destructor in
  the binary, about 33 of them, with the vtable address handed to you in the
  same line. Both were single-digit-line additions to the function-start scan
  and each yielded five to ten 100% matches per pass.
- **A destructor is a free MEMBER MAP, and it says array vs. members.** N separate
  `lea ecx,[esi+off]; mov [esp+X],state; call ~M` pairs at a constant stride are N distinct
  MEMBERS; a real array member goes through `__ehvec_dtor_iterator` @0x47a12b instead
  (`push ~M; push count; push size; push base`), and the ctor's is @0x47a06b. The EH state
  numbering is the index: entering the dtor at state N-1 means N members have destructors,
  the state is set to K immediately BEFORE destroying member K, and -1 before the base's
  dtor. So one 200-byte destructor hands you the whole layout — offsets, count, order, and
  the base — which is what makes it the cheapest way into an unmodeled class. Write the
  members in declaration order and cl reproduces the reverse order for free.
  (~Effector6thStyle @0x41d0e0 = nine members; ~BackGround @0x41aeb0 = a real array of ten.)
- **A vtable slot cannot be pointed at a lookalike beside it.** A slot is scored on the
  RECOMP ADDRESS of the entity matched to the original slot's target, so a function already
  matched as a free function does not make the slot compare — it has to BE the slot. Four of
  the panel family's `getObjectInfo` leaves were matched as free `getInfo_*` functions in
  src/classinfo.cpp and had to move into their classes; the descriptor storage stayed put.
  The same rule is why slot 1 of a class with a virtual destructor can never be a STUB: it
  is cl's own `??_G`, so pinning that vtable means reconstructing the destructor first.
- **A `// STRING` marker must sit DIRECTLY above the line holding the literal.** Wrapping
  the statement in a block afterwards (or any reformat that puts `{` between the marker and
  the literal) silently breaks it — and a broken STRING marker takes the whole function's
  annotation down with it, so the function vanishes from the table and its CALLERS regress
  to an unmapped operand. Symptom: a caller that was 100% drops with `call <OFFSETn>`.
- **A `// STRING` marker must be DIRECTLY above the literal's line, even inside
  an argument list.** Putting it above the whole CALL when the literal is the
  seventh argument four lines down breaks it, and a broken marker takes the
  whole function's annotation down: reccmp says "Failed to find a match at
  0xADDR" rather than reporting a low score. Split the argument list so the
  literal begins its own line and put the marker immediately above THAT line,
  mid-call if necessary. (EffectorDraw::drawReadout @0x41cb90.)
- **A `// STRING` literal must be on ONE source line.** reccmp's parser reads the string off
  the single line following the marker, so a C adjacent-literal split across two lines
  (`"part1 - "` newline `"part2"`) makes it compare only `part1` and fail with "Data at 0xADDR
  does not match string". Keep the whole literal on the marker's next line however long.
- **memmove vs memcpy:** a real `call` to the CRT copy routine (0x479630) means the source
  used `memmove` — `memcpy` is a /Oi intrinsic and expands inline as `rep movsd`. Annotate
  0x479630 as `memmove` in library.cpp. (EzChart::copyRecords.)
- **strcpy must be called, not hand-written:** the /Oi intrinsic expansion (`mov al,[ecx];
  mov [edx],al; ...`) has src=ecx/dst=edx; a manual `while((*d++=*s++))` loop emits the same
  instructions with the registers role-swapped and never matches. (EzChart::setState.)
- **ICF-folded empty functions: use `folded` (reccmp patch 5).** The original was linked with
  ICF, so distinct empty/identical functions share one address (0x402880 `ret` = compiled-out
  `Log` AND the novtable base dtor `??1EzBase`). Annotate EVERY function that shares the
  address with `// FUNCTION: EZ2AC_UNPACKED 0xADDR folded`; the first wins the match and the
  rest become aliases that symbolize under its name, so their callers and vtable slots
  resolve. Only one is scored, which is correct - there is only one original function there.
  Vtable slots additionally need patch 6 (see above). **Which annotation wins is decided by
  reccmp's source-file order** (`source_code_search` sorts by lower-cased path), and any
  NON-folded annotation at the address that loses is *dropped entirely* — its callers then
  mismatch. So the one you want to own the address must be `folded` too, or live in the
  alphabetically first file: 0x402880's `Log` STUB was moved to **src/applog.cpp** so it keeps
  the address ahead of KEngineApp's two folded empty virtuals in src/kengineapp.cpp.
  First consumer, done: the KEngineApp vtable @0x487418 (slots 2+3 = 0x402880). Still waiting:
  wip/ezchart-dtor.cpp (~EzChart @0x4128f0, blocked on its 24-slot vtable).
  **A `folded` alias for a shared address must be a STUB, not a FUNCTION.** A
  `// FUNCTION: 0xADDR folded` UPGRADES the entity from stub to function, so the
  winner's body is then SCORED — and the winner is usually an empty STUB, which
  reads as 0.00% against the original's real bytes. `// STUB: 0xADDR folded` adds the
  alias without changing what the address is. And put the alias in a file that sorts
  AFTER the intended owner's: this is the same source-file-order rule as above, seen
  from the other end. (Judge's implicit `??1Judge` @0x426e40, whose owner is
  KBase::~KBase in src/stubs.cpp, so the alias lives in src/tinyclasses.cpp.)
- **A destructor with NO vptr store in front of it is the BASE's, so the class has
  no destructor of its own.** cl restores the derived vptr at the top of any
  destructor it generates for a class with a vftable, so a deleting destructor whose
  `call` lands on a bare `jmp <base dtor>` proves the derived class declared none —
  give it none, and let the implicit `??1` be a folded alias. Modelling it with one
  costs exactly that store. (Judge @0x426e20/0x426e40.)
- **AND GREP FOR IT BEFORE RECONSTRUCTING, NOT ONLY BEFORE DECLARING A GLOBAL.**
  The same collision happens when the function you are about to build already
  exists as a `// STUB:` someone added to satisfy a caller - often with a comment
  that describes it exactly. Two entities then claim one address: the FUNCTION
  wins on the ORIGINAL side while every call site still calls the stub, so the
  CALLERS drop (95-97%) with every byte right and the new function reports 100%.
  Retire the stub and keep ITS name and signature, so no call site moves.
  (drawTextf @0x4071d0, which cost SystemMenu::update and TestInput::update one
  build.)
- **RUN `scratchpad/dupes.py` AFTER A BATCH - IT FINDS EVERY DOUBLY-ANNOTATED
  ADDRESS.** Two entities on one address is the quietest failure in this tree:
  reccmp drops one, the callers keep symbolizing the other, and they lose a
  fraction of a percent with every byte right. One sweep found ten, nine of
  them harmless duplicate STRING markers and the tenth a real defect the other
  nine had been hiding - the blit's trig tables declared in both globals.cpp
  and kgraphicsblit.cpp, worth 1.1% on that function. The "Dropped duplicate
  address" lines in the build log are the same signal; keep them at ZERO so a
  new one is visible.
- **AND GREP FOR THE ADDRESS, NOT THE NAME YOU ARE ABOUT TO INVENT.** Checking
  `g_1b2ebc8` finds nothing when the global already exists as `g_mixStyle`; the
  duplicate then wins on the ORIGINAL side for six unrelated functions at once
  (BattleMode, Effector6thStyle and EZ2DJMainGameDirector's ctors, two Panel
  draws, RankingDirector) and each drops ~1% with one operand renamed. The
  address is the identity - the name is yours to guess wrong.
- **AND THE INVERSE IS QUIETER STILL: REUSING AN EXISTING *NAME* FOR A NEW
  ADDRESS BINDS YOU TO THE WRONG STORAGE, SILENTLY.** The rule above is about
  inventing a second entity at one address, which at least produces a
  "Dropped duplicate address" line. Writing `extern char g_songInfoName[];`
  with a comment saying 0x1b2e680 produces NOTHING: that name already has a
  real definition at 0x1b2eb80, so the linker binds the reference to it, the
  build is clean, no duplicate is reported, and the emitted code reads the
  wrong global. reccmp cannot see it either - the operand resolves to a
  legitimate entity on both sides. Two files had it, and in one the format
  string had no %s so the argument was ignored and nothing was wrong, which
  is exactly why it survived. Before declaring an `extern` for a global,
  grep the NAME as well as the address: if the name already exists anywhere
  in src/, confirm it carries the same address.
- **AND NEVER PIPE THE DUPLICATE CHECK THROUGH `head`.** The grep that
  protects you is worthless truncated. Checking `1b2e58c` with `| head -2`
  printed two hits that were both my own lines from the edit before it, and
  the real definition - `g_catchJudging` in src/playerslot.cpp - was the third.
  A duplicate GLOBAL went in, and it cost `SongSelectDirector::toggleOption`
  @0x434f70 its 100% while every byte of it stayed right. Moving the duplicate
  to src/globals.cpp did NOT help, which is the tell that it is a collision and
  not the .data-shift hazard: a shift moves a biased reference, a duplicate
  follows the address wherever you put it. Run the grep unfiltered and read all
  of it.
- **GREP FOR AN ADDRESS BOTH WAYS: `0x1b2eb80` and `0x01b2eb80`.** The GLOBAL marker
  convention pads .data addresses to eight digits, so a `grep 0x1b2eb80` misses every
  existing annotation and you declare a second entity at the same address. The
  duplicate silently WINS for callers declared earlier in the file and the original
  side then prints your name where the recomp side prints the established one — a
  100% function drops with a single `push` operand wrong. `grep 1b2eb80` (no prefix)
  is the habit that works. (Cost BattleMode::BattleMode 99.67% for one build.)
- **Marker formatting:** a one-line function definition whose parameter list contains inline
  `/*name*/` comments breaks the annotation parser (the function is silently dropped AND the
  next marker errors). Multi-line definitions with `{` on its own line always parse.
  **AND THE HEADLINE ACCURACY NUMBER WILL NOT TELL YOU.** Five compact one-line
  STUBs dropped the marker after them, which cost `KProfile::writeFile` its
  callee symbolization and put it at 98.39% - while `Accuracy` still printed
  100.00%, because one function at 98% across 653 entities rounds away. So
  after any batch of work, grep the table for entities that are NOT 100%:
      ./match.sh 2>&1 | grep similar | grep -v "100.00%"
  The summary line is not a regression check.
- **A SIGNED `jl` ON WHAT LOOKS LIKE A POINTER COMPARE MEANS THE LOOP
  VARIABLE IS AN `int`.** A hand-walked pointer (`for (e = tbl; e < tbl + N;
  e += K)`) compares UNSIGNED and ends `jb`; when the original ends `jl`, the
  source counted with a signed int and cl strength-reduced it - into the
  pointer memcpy/memcmp gets AND into the dword index a second read uses,
  while keeping the source's signed test. Writing the reduced forms by hand
  (a walking pointer plus one counter per derived index) reproduces every
  instruction but leaves that one `jb`, three extra locals and a permuted
  frame. Collapsing them back to `for (k = 0; k < N; k++)` and spelling the
  uses `tbl[k * K]` / `tbl[k * K + j]` is what matches - cl re-derives both.
  (SongSelectDirector::m4462e0 @0x4462e0, 83.56% -> 91.78%.)
- **ANCHOR A NAMED POINTER AT THE *ROW*, NOT AT THE ELEMENT, WHEN THE
  ORIGINAL KEEPS THE MEMBER OFFSET IN THE DISPLACEMENT.** For a member array
  that does not start at offset 0, C offers exactly two spellings and the
  original is often NEITHER: subscripting (`arr[i].f`) gives the original's
  addressing - base `this`, index i*stride, the array base AND the member
  folded into one displacement - but lets cl choose its own evaluation order;
  naming the element (`T *p = &arr[i];`) fixes the order but folds the array's
  base into the POINTER, so the fields drop to +0 and +offsetof. The array's
  base offset belongs to whichever of the two the source names. Spell the
  third form directly - anchor at the row and reach the fields by their
  absolute offsets:

      int *pa = (int *)this + STRIDE/4 * a;   /* the ROW, 0x134 below arr[a] */
      ... pa[0x220 / 4] ... pa[0x134 / 4] ...

  and DECLARE EVERY SUCH POINTER BEFORE THE FIRST TEMPORARY that reads
  through one: with the second pointer declared after the temporary, cl
  schedules the second element's read ahead of the first's and takes an extra
  callee-saved register for it (61% against 89% on one function). Six
  ordinary forms scored 59-73% before this; the row anchor with both pointers
  first is 100%. (Panel::m429950 @0x429950.)
  THE OPPOSITE CASE IS THE ELEMENT POINTER: when the original addresses one
  field OFF ANOTHER FIELD'S ADDRESS (`lea edi,[ebx-0x46]; mov [edi+0x42],eax`
  - f44 written through the memcpy destination `name`), the source named the
  element, `Group4C *g = &groups[i];`, freshly per iteration. Subscripting
  every use folds the row into each displacement (`mov [ebx-4],eax`), and so
  does walking an `int *` at one field. (EzChart::loadEZ @0x411460 98.94 ->
  100, loadTracks @0x411920 98.29 -> 100* - one defect, two functions.)
- **`p++; x = *p;` AND `x = p[1]; p++;` ARE NOT THE SAME CODE.** The same two
  statements in the other order, and the difference is the loop's ANCHOR:
  reading `p[1]` lets cl fold the +1 into the pointer, so it anchors one
  element ALONG and reads `[ebx]` where the original reads `[ebx+4]` - and the
  `lea` that materializes the pointer then lands AFTER the guard's `je`
  instead of before it, which moves every branch in the loop. Bumping first
  and dereferencing keeps the anchor on the entry itself. This is the
  source-level lever for the anchoring rule below, which otherwise reads as
  something only cl decides. (SongSelectDirector::m446440 @0x446440, 91.02%
  -> 100%; the plain index form `x = arr[i+1]; i++;` and guarding on `*p`
  both leave it at 91.02%, so it is specifically the p[1]-versus-++p choice.)
- **Loop induction anchoring:** cl may anchor an induction pointer at a nonzero field offset
  (e.g. `[ecx-4]`,`[ecx]` with the pointer at `elem+4`). If the loop body matches but the
  base/displacements are shifted, re-anchor the C pointer (e.g. `int* p = &e[0].b; p[-1]+p[0]`).
- **The stack probe is `_chkstk`, NOT `__chkstk`.** Our libcmt exports both names at
  DIFFERENT addresses and cl's prologue calls `_chkstk`; annotating 0x479240 as the alias
  makes any function with a >4KB frame read as a call mismatch with every byte already
  correct. (First hit by loadModeGds @0x4150d0, whose 0x502c-byte KParser local forces the
  probe. Nothing with a smaller frame ever noticed.)
- **CRT calls match automatically:** reccmp recognizes statically-linked CRT functions
  (`fopen`/`fread`/`memset`→`rep stosd`/`__security_check_cookie`/`_chkstk`…) by name, so
  functions that call the CRT are matchable without annotating the CRT. `#include <stdio.h>`
  etc. and call them normally.
- **A dead `if (x == -1)` after a search = an inlined helper returning -1.** When the
  original has a search loop that returns 0 on not-found AND a provably-dead `cmp i,-1;
  jne` before the hit path (two separate identical `return 0` tails that cl would normally
  cross-jump/merge), the source was a separate `static int findSlot(...){ ...; return -1; }`
  that got inlined. Writing the loop inline in the caller makes cl merge the two `return 0`
  blocks (wrong); pulling the search into a real `static` helper that `return -1`s on miss
  reproduces the two-stage codegen with both tails kept. (Matched Obj4105D0/Obj410740::release.)
- **Whole-class reconstruction (vtable classes).** A `mov [ecx], 0xVT; ...` constructor
  is a C++ class with a vtable at `0xVT`. Model the class with its N virtuals *in slot
  order* (vtable entries `0xVT`, `0xVT+4`, …), put `// VTABLE: EZ2AC_UNPACKED 0xVT` above
  the struct, and reccmp matches the vtable-data entity once every slot points to a matched
  method. Win32 IAT calls (`call *0xADDR`) resolve automatically — just call the API
  (CreateFileA/ReadFile/…) via `<windows.h>`. Matched all of KFile (ctor, 6 methods, vtable).
- **Scalar deleting destructor** (vtable slot 0, `…; if(flag&1) op-delete(this); ret 4`):
  the address is the *deleting* destructor, not `~T()`. Target it by name — put
  `// FUNCTION: EZ2AC_UNPACKED 0xADDR` then a `// T::`scalar deleting destructor'` comment
  line (reccmp by-name lookup). (The original's `call 0x4788ca` IS the global CRT
  `operator delete` - not free, see the operator-new entry.)
- **cl DUPLICATES A SHARED TAIL *AFTER* CONSTANT FOLDING, so assigning a
  constant to a variable keeps it unfolded where returning the expression does
  not.** `if (a) return f(0xff); if (b) return f(0); return f(x);` folds each
  constant arm to a literal - `or eax,0xffffffff` - because the fold happens
  at the call site. Writing `int v; if (a) v = 0xff; else if (b) v = 0; else
  v = x; return f(v);` gives ONE tail that cl then duplicates into all three
  arms, and each copy keeps the arithmetic: `mov eax,0xff; shl eax,0x18;
  or eax,0xffffff`. Same three arms, same instruction count, but only the
  second form matches an original that spells the constant out. Note this is
  NOT the inlined-helper-parameter rule below - routing the constant through
  an inlined `f` changes nothing on its own, because cl folds after inlining;
  what matters is that the value is a VARIABLE at the point the tail is
  written. (Panel::m4270a0 @0x4270a0, 88.20% -> 100%.)
  **AND THE RETURN VALUE ITSELF IS THE COMMONEST CASE.** A function whose
  paths all end `cleanup(); return r;` must write that cleanup and return
  ONCE, not once per path: `if (miss) { unlock(); return r; }` lets cl prove
  `r` is still its initializer at that return and emit the constant, so the
  value never reaches a register and every argument offset downstream shifts
  by four. `if (hit) r = f(...); unlock(); return r;` gives cl one tail to
  duplicate into both paths, and each copy reads the variable - the
  original's `mov eax,edi` at both exits, against an edi zeroed before the
  lock was even taken. Worth 55 points, and it un-parked a file that had
  been shipped to wip/ as a register wall.
  (EzSubsystem::SubsystemBackend::m40de30 @0x40de30, 42.35% -> 97.50%, then
  100% by moving `r = 0;` ABOVE the lock call so the zero is scheduled into
  the call set-up where the original has it.)
- **A constant passed through an INLINED HELPER'S PARAMETER is not folded.**
  `flags[2] |= 0x40; flags[2] |= 0x80;` collapses first to `|= 0xc0` and then into the
  store that just cleared the word. Routing both through
  `static void setBit(int *w, int bit) { *w |= bit; }` reproduces the original's four
  separate `or [mem],reg` with the two constants hoisted into registers — the parameter is
  what breaks the read-modify-write merge. No loop form, memset, pointer alias or statement
  order does this. (Obj40C180::resetInputState @0x40c820.)
- **Write a post-increment as a post-increment.** `if (g++ % 30 < 15)` keeps the
  counter live across the divide, so cl takes a callee-saved register for the
  divisor and stores the bumped value AFTER the `idiv`. Split into the obvious
  `int c = g; g = c + 1; if (c % 30 < 15)`, cl stores first, frees the register
  holding `c` and uses it for the divisor instead - the same instructions in a
  different order, and no statement ordering recovers it.
  (blinkAlternateLamps @0x44b330.)
- **cl merges a `push` across a branch.** One `push 0` before a conditional can
  serve as a `__cdecl` callee's argument on one side and as the last argument of
  a completely different call on the other, with only one side doing the
  `add esp,4`. It is the optimizer's doing, not something the source expresses -
  don't go looking for a source shape that "produces" it. (Same function.)
- **A shared value written out at BOTH use sites gets a different register than
  the same value hoisted into a local.** `if (on) f[w] |= tbl[b]; else f[w] &=
  ~tbl[b];` makes cl common-subexpression-eliminate the two reads into ONE load
  above the branch *and* give the result a fresh register (edx); writing `int m =
  tbl[b];` first produces the same single load but lets cl reuse the dying index
  register (esi) for it. Every other instruction is identical either way, so this
  is the lever when a function differs only by one register across its arms. The
  companion rule still holds: declaration order decides which of `c / 8` and
  `c % 8` is emitted first. (Obj40C180::setOutput @0x40c780.)
- **Name an index in a local to fold it into a read-modify-write.** `flags[c / 8] |= m;`
  makes cl materialize the word's address with a `lea` and work through the pointer;
  `int w = c / 8; flags[w] |= m;` folds `[esi+eax*4+off]` straight into the `or`. Same class
  as the `x - y` subexpression rule. (Obj40C180::applyOutputs @0x40c940.)
- **A clamp must OVERWRITE, not select.** `if (a >= hi) v = hi; else if (a < lo) v = lo;
  else v = a;` makes cl if-convert the second arm (`xor/setl/sub/and`). Assigning the value
  first and conditionally overwriting it — `if (level >= 255) level = 255; else if (level <
  0) level = 0;`, i.e. two assignments to ONE variable — keeps the original's `test/jge`
  branch. (KGraphics::setFade @0x40b080.)
- **Seed a switch's result before the dispatch to share the default's tail.** When the
  default and one arm both end in the same `return`, `int r = <default>; switch (...) { ...
  break; } return r;` hoists `mov eax,<default>` above the dispatch, leaves the selector in
  ecx and gives the arm and the default ONE epilogue. A trailing `return <default>` after
  the switch gives the default its own. (EzSubsystem::freeSample @0x40e620.)
- **A walking pointer must come from cl's strength reduction, not from the source.** Writing
  `e = entries;` before a `for` hoists the pointer set-up ABOVE the loop's count test, where
  the original builds it inside the guard; index with `entries[i]` and let cl reduce it.
  Load the element into a local first, or cl tests memory and reloads it for the call.
  (Obj4102C0::tick @0x40fe90.)
- **A VALUE THE ORIGINAL RE-READS INSIDE AN *INLINED HELPER* NEEDS THE
  QUALIFIER ON THE HELPER'S PARAMETER, NOT ON THE ARGUMENT.** The documented
  lever for a memory-resident parameter is a `volatile` alias at each use -
  but when the use is inside a helper cl inlines, "each use" is the CALL
  SITE, and an argument is evaluated before the callee's branches. So the
  load lands ABOVE the inlined body where the original has it inside one of
  its blocks, and the score goes DOWN. Pass the address and dereference it in
  the helper instead (`cmp3(pan, 0x80, &dir)` against `return *v;`): the read
  is then emitted in the block that uses it, and the address-take is also
  what keeps the value in its home slot for the other uses. The three pan
  steppers @0x420400/0x45d690/0x46c190 sat at 79.21% for a session on this,
  logged as the register-allocation coin because the call-site alias measured
  76.60%; the helper-side qualifier is 100% on all three. When a `volatile`
  alias makes things WORSE, ask where the original's load actually sits
  before concluding it is a coin.
- **`volatile` is decided PER MEMBER, and codegen tells you which.** In one class the two
  latched input ports must be volatile (without it cl groups their four stores by register
  instead of keeping source order) while the ring cursor next to them must NOT be (pending
  @0x40c760 folds it as a memory operand, which volatile forbids). Read each member's uses
  before qualifying the group. (Obj40C180.)
- **NAME THE PROMOTION, NOT THE PRODUCT, TO SPLIT AN INLINE ftol BLOCK.**
  `m = (unsigned int)(f(x) * K);` makes cl emit the fmul and then hoist the
  NEXT statement's argument push into the FP shadow, leaving the control-word
  save (`fnstcw`/`movzx`) after both. An original that saves the control word
  BEFORE the fmul and wedges the push between `or ah,0xc` and the `fldcw` -
  i.e. that splits the ftol expansion around the push - wrote the
  integer-to-double promotion as its own statement: `double d = f(x); m =
  (unsigned int)(d * K);`. It must be the PROMOTION that is named; naming the
  PRODUCT (`double d = f(x) * K;`) changes nothing, and neither does a uint
  local, an `__int64` local, an explicit `(__int64)` cast, `unsigned long`,
  reciprocal division, or touching the next call's argument. Eight zoo forms,
  one works. (The three backgrounds' update2 @0x45ca90/0x46b6d0,
  93.18% -> 100%.)
- **A float pool constant must stay an ANONYMOUS literal.** Giving it a name
  (`static const float k = -5.0f;`) puts a symbol on it, and the operand then prints as data
  on our side against the original's float — a mismatch with identical bytes. Also: cl does
  NOT fold `x - -5.0f` into `x + 5.0f`, so the pool records which form the source used.
  (ShowCredit::setPos @0x44fa50.)
  **THE CONVERSE: a float constant used in MANY blocks that the last block
  gets WRONG is a `static const float`.** A literal is a rematerializable pool
  constant and cl treats all its uses as one value: in the block holding its
  LAST use on a path the constant takes xmm0 and the other operand xmm1, the
  reverse of every earlier block, and nothing in that block's own text moves
  it (forty forms swept - named quotient, statement order, `-=` vs `= a - b`,
  cast forms). A file-scope `static const float k = V;` (or a plain
  `const float` at file scope, internal linkage) with a `// GLOBAL` pin at the
  constant's address is a real load from its own symbol: every block then
  comes out `cvtsi2ss xmm0 / movss xmm1,[k]`, including the original's one
  load-first block. Do NOT make it `extern const`: that schedules the load
  ahead of the conversion in every block and lands a 100%* effective match,
  not a byte-exact one - the two declarations differ only in linkage and cl
  schedules them differently. Grep the constant's other users first: pinning
  it blocks the FLOAT analysis for anyone else who reads it as a literal
  (all forty-six users of 0x4916b4 are one TU, so it was safe).
  (SongSelectDirector::inputRubyMix @0x43b8d0, 99.08% -> 100%* -> 100%.)
- **AND SOMETIMES IT HAS TO BE A NON-CONST `static float`.** The same
  last-use-block reversal in drawRankTable @0x432bb0 (the digit block took
  `wx - 55` in xmm0 and the 213.0 origin in xmm1) was NOT fixed by
  `static const float k = 213.0f;` - cl folded the constant initializer back
  into a pool literal and the order stayed wrong - nor by `extern const`;
  `static float k_tableOriginX = 213.0f;` (mutable, internal linkage, GLOBAL
  pin at the pool address) is a real symbol load and gave the original's
  order in every block. Try const first (RubyMix's case), then drop the const.
- **A VALUE IN A DEAD PARAMETER'S SLOT MAY BE THE PARAMETER ITSELF, REUSED.**
  When two values land in the two dead argument homes and cl assigns them the
  other way round to the original (nine forms swept: named/unnamed, declaration
  order, do-while pointer walks, separate counters), the source wrote one of
  them INTO the parameter: `x = ry + 14.0f; drawTex(..., x, ...)` put the row's
  y in x's slot and left y's for the counter, and nothing else did.
  (drawCourseRankTable @0x433d50, 98.01% -> 100%.)
- **A TAIL `jmp` THROUGH A VTABLE SLOT MEANS THE FUNCTION RETURNS `void`.**
  cl only turns a trailing virtual call into `pop; pop; mov [esp+4],0; jmp
  [reg+off]` - overwriting its own argument slot rather than pushing - when
  caller and callee agree on returning nothing. Declare the function `char`
  or `int` and cl must materialize a value instead, which costs an
  `xor al,al` and puts EVERY branch displacement in the function off by
  exactly one byte: ten defects, all of them `-jne 0x68` against `+jne
  0x67`. The tell is in the original's OTHER exits - if they are bare
  `pop; pop; ret N` with no `xor al,al` anywhere, nothing is returned.
  Worth 12 points and a one-word edit. (TitleDirector::m44e680 @0x44e680.)
- **A RUN OF TESTS THAT ALL JUMP TO ONE `return 1` IS A SINGLE `||` CHAIN.**
  Not separate early returns, and not gotos to a label. The three forms are
  distinguishable and only one matches: seventeen `if (t) return 1;`
  statements put the `mov eax,1; ret` INLINE after the FIRST test and make
  the rest jump backwards to it; seventeen `goto held;` with a trailing
  `held: return 1;` gets the direction right but cl then SWAPS the two tail
  blocks, inverting the last test to `jne` and putting every displacement
  eight out; `if (a || b || ... || q) return 1; return 0;` gives the
  original exactly - every test `je` forward, `xor eax,eax; ret` first and
  `mov eax,1; ret` last. Measured 77.17% -> 79.56% -> 100% on one function.
  (Obj40C180::anyPlayHeld @0x4188c0.)
  **AND THE PROLOGUE'S CALLEE-SAVED PUSHES COUNT THE BLOCKS FOR YOU.** The
  cheapest tell that a run of tests is one `||` and not several `if`s is where
  the pushes sit: the original saving BOTH registers before the first test,
  against a reconstruction that defers the second push until after it, means
  the two tests are one basic block - cl allocates a callee-saved register at
  entry only for a value the entry block already needs. The same reading
  applies to a group after a call. Worth 28 points on a function whose body
  was already instruction-for-instruction correct, and it beat four earlier
  forms (seven plain returns, the same with the result seeded, one nested
  `&&` chain, and moving a store) that had left the park looking like a
  push-placement coin. (pickTextureFormat @0x4072c0, 67.83% -> 100%*, whose
  seven exits are two chains either side of a Log call.)
  **AND IT IS NOT ABOUT THE COUNT - TWO TESTS BEHAVE THE SAME WAY.**
  `if (a) goto err; ... if (b) goto err; return 0; err: return E;` puts the
  error tail INLINE and the zero return after it; `if (a || b) return E;
  return 0;` puts them the original's way round. It also stops mattering
  that one of the two values is only assigned inside an earlier `if` -
  initialize it to zero at the top and cl still skips the test on the path
  that skipped the assignment. Measured 89.73% -> 92.39% -> 100%.
  (Obj405F80::releaseAll @0x405580.)
- **TWO ARMS THAT SHARE THEIR LAST INSTRUCTION SHARE A *VARIABLE* IN THE
  SOURCE.** The tell is an if/else whose arms both end `mov edx,<something>`
  and fall into ONE `add [mem],edx` - and the reconstruction, written with the
  add spelled out in each arm, has one instruction too many because cl
  duplicated it. It is not a cross-jump cl declined: written per-arm the two
  arms allocate their own registers (the eax/edx alternation restarts at eax
  in each) so the two adds are not identical code and there is nothing to
  merge. Assign to a named local in each arm and do the operation after the
  join - one variable means one allocation, both arms end in the same
  register, and cl merges. Same family as the constant-folding rule above:
  what is shared has to be shared in the SOURCE. The rider is that the local's
  register is then a coin (ours eax, the original edx) and the whole tail's
  alternation follows it, which is a 100%* effective match rather than a
  byte-identical one - declaration order, swapped header stores, moving the
  shared operation later and ten orderings of the arms' own statements all
  leave it. (ScoreKeeper::commit @0x472d10, 56.93% -> 100%*.)
- **A CALL WRITTEN ONCE AFTER A BRANCH BELONGS INSIDE EVERY ARM WHEN ITS
  RECEIVER'S ADDRESS IS MATERIALIZED IN EACH.** The tell is one `lea
  reg,[this+off]` per BRANCH in the original where the reconstruction has a
  single one after the merge. Writing `if (a) v = x; else v = y;
  m.call(v);` lets cl compute `&m` once past the join; writing the call out
  in each arm - `if (a) m.call(x); else m.call(y);` - puts the `lea` in both
  branches where the original has it, and cl still merges the identical
  argument pushes and the call itself. Worth 22 points on one function
  because every block after the join shifts otherwise.
  (GFResultDirector::update @0x4680d0, 72.73% -> 94.67%.)
- **AND READ WHAT THE ORIGINAL COMPARES *AGAINST*, not what the value is.**
  `cmp [g_flag],eax` where eax still holds a count just returned is
  `if (g_flag < n)` with a named `n`, not `if (g_flag < 2)` - even when the
  surrounding test proves n == 2. The same reading error that hides a
  constant register (rule below) also hides this one. (Same function.)
- **BUT A CONSTANT REGISTER cl DECLINES MAY BE A SEMANTIC ERROR IN *YOUR*
  CONSTANTS, NOT A WALL.** The wall below is real for the mode-input family,
  and the shape is seductive - the original materializes 1 in ebx and spells
  five or six sites as `cmp reg,ebx` / `mov [m],ebx` / `push ebx` while cl
  emits immediates. Before parking, CHECK WHAT THE ORIGINAL IS COMPARING
  AGAINST. TotalResultDirector::update2 @0x454800 sat at 76.09% with exactly
  that signature, and the cause was that two of its arms had been read as
  `if (x < 0) x = 0;` and `if (f() == 0)` when the original's `cmp
  [m],ebx` / `mov [m],ebx` / `cmp eax,ebx` are all against **1**. Fixing the
  two comparisons took it to **100% on the next build** - cl materialized the
  register by itself once the constant had enough uses. The register is a
  CONSEQUENCE of the use count, so a missing one is evidence your reading is
  wrong before it is evidence of a coin. Read every `,ebx` in the listing as
  a value, not as a register name.
- **COUNT THE PROLOGUE PUSHES: A CALLEE-SAVED REGISTER THE ORIGINAL DOES NOT
  HAVE IS A CACHED PARAMETER, AND THE ZERO REGISTER IS ITS SYMPTOM.** The
  shape reads exactly like the constant-register wall - cl materializes 0 in
  ebp and spells every zero test and push against it (`cmp eax,ebp`,
  `push ebp`, `mov [g],ebp`) where the original uses immediates - and it is
  not that. Read the prologue instead: if the original saves TWO registers and
  we save FOUR, the question is not "why does cl want a zero register" but
  "what is in the two registers the original does not have". Here it was the
  `a` parameter, cached in edi for the whole function where the original
  re-reads it from its home at each use (`[esp+0x10]` at push depth 3,
  `[esp+0xc]` at depth 2 after a mid-function `pop edi`). A `volatile` alias
  at each use frees edi, cl then uses edi for what the original uses it for,
  and the zero register goes with it - one edit, 76.96% -> 92.7%, with the
  instruction counts already equal beforehand. The two clues that say "cached
  parameter" rather than "coin": the original reading the SAME `[esp+X]` at
  two different push depths, and a `pop` in the middle of the function.
  (GFMainGameDirector::update2 @0x45f350, which had been written up as the
  wall and was one line from 100%.)
- **AND A BACKWARD `jmp` INTO A SHARED TAIL COMES FROM WRITING THAT TAIL IN
  BOTH ARMS.** The companion half of the same function. When an if/else's two
  arms end in the same calls and the original lays one arm inline, jumps over
  the other, and has the SECOND arm jump BACKWARD into the first arm's copy of
  the tail, that backward jump is cl's tail merger at work - so the source has
  the tail written out per arm and lets the merger fold it. Swapping the arms
  alone does nothing when one of them holds an early exit (cl re-inverts);
  hoisting the early exit into an `&&` guard is worse. And the ORDER of the
  duplicated arms is the lever: the arm cl keeps is the one written FIRST, so
  put the arm the original lays inline first - the other way round, and the
  `&&` form, both cost two or three instructions and eight points.
  (Same function's demo-play branch, 92.7% -> 100%.)
- **AND BEFORE PARKING ONE, ASK WHAT THE *OTHER* CANDIDATE IS DOING IN THAT
  REGISTER.** A constant-register difference looks like two values competing
  for one register by chance, and it usually is not. m42fbf0 @0x42fbf0 was
  parked at 66.67% on "the original materializes 1 in ebp, cl caches the
  `length` parameter there instead, nothing at source level exposes the
  ranking". The real reading is that the original's `length` is
  MEMORY-RESIDENT - reloaded from its home slot at each of its four uses -
  and that is WHY ebp is free for the constant. Reading the parameter through
  a volatile alias at each use (the lever already documented two rules below)
  frees the register and brings `mov ebp,1` / `cmp ebx,ebp` / `sub eax,ebp`
  out together: 66.67% -> 78.05%, and the "coin" is gone. So identify the
  competitor and check whether the original keeps IT in memory, before
  believing the flip.
- **THE MODE-INPUT "CONSTANT-REGISTER WALL" WAS A SCOPE DEFECT - READ THE
  ORIGINAL'S JUMP TARGETS BEFORE BLAMING cl.** (2026-10-01.) Seven song-select
  handlers parked at 83-95% on "which register holds 0 and 1" all went to
  100% once the `f84 = (f84 + f7c) % f7c` wrap (and the f_20c0d4 lerp) moved
  INSIDE the block the original has it in. The tell was in plain sight: the
  guard tests (`f_20c6bc`, `f_dc`, `f_bfdb4`, sometimes the two getState(4/5)
  tests) `jne` to the instruction JUST AFTER `mov [esi+0x84],edx` - past the
  wrap - which only happens if the wrap is inside what they guard. Moved out
  of its block, the wrap runs on every path, the 0/1 use-counts per region
  change, and cl's global constant-register weighing flips - so every defect
  LOOKS like a register coin. Which block varies per handler (ClubMix: inside
  `f_20c6bc == 0` only; 10/14Radio: inside `f_bfdb4 == 4`, getState outside;
  5Radio: inside the whole combined guard), and the jump targets say which.
  Tool: diff branch targets as FUNCTION-RELATIVE offsets, not just the
  instruction text - a masked-target diff reads 0 while a branch lands on the
  wrong block. Also in the family: `moved = 1` after the two sound calls (it
  is the register the f_20c0e0 store uses), `f_20e894 = 0` before the preview
  resets, `g_1b2e588 = 0` before `g_37dcee0++`, and StreetMix's
  `(f_bfdc4 == 1 || getState(8) == 2) && f_20c6bc >= 1`. These are behaviour
  fixes, not only codegen: with the options page open the C wrapped and eased
  where the game does not. EZ2CATCH went 88.56 -> 100 the same way (its wrap,
  lerp AND getState block inside `f_20c6bc == 0`, ClubMix's shape). CV2Mix
  85.01 -> 98.43 on a different lever: `int moved = 0;` at the declaration
  made cl spill `moved` and give the constant 1 its register; initialising
  it after the lamp block puts it in ebx as the original has it. The last
  1.6% is the ebx/ebp saves moving below the lamp block (cl sinks them when
  nothing above uses them; the original's lamp block reads moved's zero).
  **The original entry, kept for its measurements:** cl
  gives the small constants a function uses often (1, 5, 0) callee-saved
  registers, and WHICH register - and from where - is decided by a global
  weighing that three siblings compiled from the same source text resolve
  three different ways: RubyMix @0x43b8d0 5->ebp then 1->edi from the confirm
  run; 5KeyMix @0x43ff00 1->ebp for the WHOLE function (the lamp pushes at the
  top are `push ebp`), 5->edi, then edi as a zero; EZ2CATCH @0x43a150 5->edi
  briefly, 1->edi, 0->ebp. cl reproduces only RubyMix's. The use counts of
  1/5/0 in our 5KeyMix are IDENTICAL to the original's (45/8/48) so it is not
  a missing use; probes flip the choice on any single 5-use, and the top
  hoist of 1 needs sixteen extra stores. Recognise the shape - every
  instruction present, every defect a callee-saved register name or a
  `push imm` against `push reg` - and park at 88%; the two written siblings
  (src/songselectinput.cpp) and the note in wip/modehandler-43b8d0.md hold
  what was tried.
  **BUT IT IS THOSE TWO SIBLINGS, NOT THE FAMILY.** Four more members written
  2026-08-23 from the same template came out 100% (inputScratchMix @0x43ad10),
  100% (inputRubyMix @0x43b8d0), 93.53% (inputStreetMix @0x43c2e0) and 93.40%
  (input7StreetMix @0x442770), all on the first or second build. So do NOT
  read a sibling's 88% as evidence the next one is hopeless - write it, and
  investigate only if it lands low. ScratchMix went 98.69% -> 100% on ONE
  edit that had nothing to do with the constant registers: the lamp block's
  `int b` and `int c` were declared in the wrong order, so cl put each sum in
  the other register. Both of its option pages needed the same swap.
  Two forms that ARE load-bearing for two-page handlers:
  `switch (g) { default: A; break; case 1: B; break; }` emits `sub eax,1; je`
  where any `if` form emits `cmp`; and a stop list is TWO ifs, not if/else-if
  (the first arm sits inline, the second's tests after it).
- **AND A FLOAT *LITERAL* PROMOTES WHERE A FLOAT *VARIABLE* DOES NOT.** The
  rule below covers a `(double)` cast you can delete; this is the case with no
  cast to delete. In `sin(x) * 100.0 + member - 100.0f` the member is read with
  `fadd dword ptr` - x87 converts a 4-byte operand for free - but the LITERAL
  is folded to an 8-byte pool constant at compile time and comes out
  `fsub qword ptr`, however it is spelled (`100.0f`, `(float)100`, a cast on
  the operand). What produces the original's `fsub dword ptr` is making the
  SUBTRACTION itself float-typed: cast the accumulator down first,
  `float y = (float)(sin(x) * 100.0 + member) - 100.0f;`. Worth six points on
  one function and six on its twin (Panel::m428c60 @0x428c60,
  GFPanel::m461de0 @0x461de0, both in wip/smallleaves.cpp).
- **`fdivr qword` WHERE THE ORIGINAL HAS `fdivr dword` IS A `(double)` CAST ON THE
  OTHER OPERAND.** The x87 operand size is the C type of the pool constant, and a
  float constant divided by a `(double)`-cast integer gets promoted, so the whole
  divide moves to a double constant: `48.0f / (double)(int)n` emits
  `fdivr qword ptr [48.0]`, `48.0f / (int)n` emits the original's
  `fdivr dword ptr [48.0]`. The FLOAT entity resolves to the same name on both
  sides, so the diff shows one word of difference and nothing else - easy to read
  past. **And the fix has two halves that must go together**: keep the quotient in
  a NAMED `double` local. Written as one inlined expression, dropping the cast
  reschedules the entire x87 chain around the divide and costs six to twenty-six
  points; the named local anchors the schedule while the undecorated constant
  fixes the operand. Grep `(double)` across src/ after finding one - all six panel
  readouts that divide the 48-tick beat had the identical cast.
  (Panel::m427d90 @0x427d90 95.37% -> 96.30%, and five siblings.)
- **A loop between stores and an inline memset anchors cl's scheduling.** Without one, cl
  hoists the `rep stosd` set-up (`push edi; xor eax,eax; lea edi`) to the top of the
  function; with one it stays put. This is the whole difference between PlayerSlot::reset
  @0x42efb0 (matched) and wip/playerreset.cpp (parked at 86.96%) — same function on the
  sibling class. Writing a short run of stores as a `for` loop cl unrolls is the cheapest way
  to get the anchor.
- **A parameter re-read from memory where cl caches it in a register** (e.g. the original
  does `mov dl,[esp+x]` for byte tests then `mov eax,[esp+x]` again for a full-width use,
  while cl loads the dword once and reuses it): force the split with a `volatile` read at
  each use — `unsigned char b = *(volatile unsigned char*)&p;` for the byte test,
  `x = *(volatile int*)&p;` for the re-read. (This is the *matchable* cousin of the
  DecryptFile `be` spill; here it works because the value is a param, not an induction.)
- **`volatile` MEMBERS, when the original never folds a member as a memory operand.** If a
  function shows a systematic "original loads into a register, cl folds" split across
  *several* sites at once — `fld [m]` + register `fsubp` where cl emits `fsubr [m]`,
  `mov ecx,[m]` + register `sub` where cl emits `sub ecx,[m]`, `movss xmm0,[m]` +
  `comiss xmm1,xmm0` where cl emits a folded `comiss` — the member itself is `volatile`
  in the original, not a source-shape or flag problem (/Op, /fp:precise, /G6 all swept and
  none of them do this). Declaring the member volatile fixes every site at once. Expect
  thread-shared state to be volatile: KSongPlayerBase::m_pos/m_time are, and they exist to
  pass state between the pump thread and the main thread. Two follow-on constraints once
  volatile: keep the whole update as ONE expression (splitting through a local costs the
  x87 pop — `fsubr st,st(1)` instead of `fsubp`), and write it so the reads happen in the
  original's order (`m_pos - (float)(now - m_time)` reads m_pos first; hoisting the delta
  into a local swaps the two loads). (KSongPlayerBase::advance @0x4146a0.)
- **Ctor/dtor virtual calls that stay DYNAMIC (slot-address form):** a dtor's
  `v6();` devirtualizes to a direct call in every TU arrangement (defined,
  extern, even pure-virtual). When the original shows `call [0xVT+off]` (the
  through-the-vtable-slot form) + a second call via a reloaded vptr, the calls
  were made through a pointer of a DIFFERENT class type: declare a mirror
  struct with the same slot shape, `Mirror *p = (Mirror *)this; p->q6();` —
  the devirtualizer is defeated but the just-stored vtable constant still
  folds into the first call's operand. (KSong::~KSong @0x410840.)
- **Success-block-after-the-return layout:** cl normalizes `if (A && B)
  {win} lose;`, the `||`/else form, and goto-into-tail all to win-inline.
  When the original lays the lose arm inline and the win block AFTER the
  common `return 0`, materialize the conjunction: `int ok = A && B; if (ok
  == 0) { lose; return 0; } win;` — jump-threading of the flag reproduces
  the original's jne-forward + trailing win block. (KSong::v4/v7.)
- **Keeping a search-index register live early (prologue save):** `int i =
  -1; if (count < 8) i = findSlot(...); if (i == -1) return 0;` — the -1
  init is never materialized (jump-threaded away) but keeps i's lifetime
  starting before earlier calls, promoting its register to a prologue save
  and letting the count-full branch cross-jump into the -1 tail. The
  equivalent early-return chain saves the register just-in-time instead.
- **A FACTORY'S `push` IS `sizeof(T)`, so matching one CHECKS the class.** The pooled
  factories are four lines each (`g_objectRecord = &rec; return new T;`) and land 100%
  on the first build - but only if the modeled struct is exactly the size the original
  pushes, which makes them the cheapest audit of a layout there is. Effector6thStyle
  came out 0x14f0 against 0x14e0 and the extra sixteen bytes were a spurious trailing
  pad; the other seven agreed to the byte and so confirmed seven layouts for free.
  Find them all with one grep for `mov [0x530b74],0x4......` and read the `push`
  above it and the `call` below it. Adding `, EzPooled` to a class costs no layout
  (trailing empty base, EBO).
- **The engine has its own POOL `operator new` @0x4016a0, and it is `__stdcall`.** The
  shape is `mov [0x530b74],&record; push size; call 0x4016a0` with NO `add esp,4` after —
  a class-specific `static void *__stdcall operator new(unsigned int)` on a shared empty
  base reproduces it (cl accepts a calling convention there). Objects allocated this way
  come out of the pool their record owns, which m4017a0 @0x4017a0 carved. Writing it
  __cdecl leaves an extra `add esp,4`. (The four screens' director factories.)
- **operator new is the libcpmt `operator new`, NOT an inlined malloc**
  (corrected 2026-10-01). `new T` emits `push size; call 0x478528`, and that
  address is newop.obj's `_callnewh` retry loop around the real malloc
  @0x47a5e8; `delete` calls 0x4788ca, libcmt's `jmp free` (free @0x47afc4).
  Game code never calls malloc/free directly. The old model (inline
  malloc/free in kcrt.h, and LIBRARY names `malloc`/`free` on the operator
  addresses) matched every caller byte for byte, because a call is a call -
  CHECK A LIBRARY ANNOTATION'S OWN SCORE: `malloc` 14%, `free` 0%, `strchr`
  5% and `strrchr` 7% were each a mis-named symbol, and the strchr/strrchr
  swap hid a dozen behaviour bugs in the callers (docs/PORT-DELTAS.md).
  Two riders: match.sh links libcpmt.lib AHEAD of libcmt.lib (libcmt's own
  new.obj is the non-looping `_nh_malloc` form), and kcrt.h includes `<new>`
  so `operator delete` is `throw()` - without that, a destructor whose TU
  calls `operator delete` explicitly grows an EH frame (~KSNDMixer, 100 ->
  61%).
- **The frame-alignment wall (KNOWN-HARD, parked):** cl 13.10 emits `push
  ebp; mov ebp,esp; and esp,-8` for ANY function with (a) a frame local +
  (b) a loop + (c) an 8-byte FP push inside that loop (a float promoted to
  double for a vararg Log is enough). KProfile::Load @0x413fe0 is the
  binary's only such function and it is UNALIGNED — no flag (20+ swept), no
  cl build (2179/3077/4035 identical here), and no source shape (35
  experiments incl. helper splits, LTCG, forceinline) reproduces it; see
  wip/kprofile-load.cpp for the full analysis before attempting.
- **Alternate period compilers on hand:** tools/cl2179 (13.10.2179, DDK
  3790) and tools/cl4035 (13.10.4035, DDK 3790.1830) — byte-identical to
  3077 on everything tested so far; useful for ruling compiler-build
  theories in or out. Rich-header calibration: decode the original's Rich
  entries AND our build/EZ2AC_Unpacked.exe's with the same script — matching
  prodids identify which entries are compiled code vs import libs.
- **AND WHEN THE ORIGINAL KEEPS A VALUE IN edx ACROSS A CALL, THE CALLEE MUST
  BE IN THE SAME TU.** cl narrows a callee's clobber set only for a function
  whose body it can see, so a value living in a caller-saved register across a
  `call` is proof the original compiled both in one TU - the same constraint as
  the non-standard-register rule, one notch weaker. Symptom: the caller is
  structurally exact but moves that value into a callee-saved register and pays
  a handful of extra instructions for it. Reconstructing the callee elsewhere
  does NOT help; it has to land beside its caller.
  (EZ2DJMainGameDirector::pickTwoLanes @0x421680, 54.55%, blocked on @0x421490
  which touches only eax and ecx.)
  **AND THE MOVE IS CHEAP WHEN THE CALLEE'S ONLY CALLERS ARE THE ONES YOU
  ARE MATCHING - but mark it `__declspec(noinline)`.** resolveLevel @0x432310
  lived in src/songselect.cpp; its three call sites are all in
  m4394e0 @0x4394e0 (src/songselectpage.cpp, 46.07%). The original reuses the
  row pointer in edx after the call AND does not reload `f84`, i.e. cl knew
  the callee writes no memory. Moving the definition above the caller first
  INLINED it (diff grew 812 -> 1004 lines); with `__declspec(noinline)` the
  caller went 46.07 -> 99.03 and the callee stayed 100. Check its callers
  with `grep "call.*0xADDR" text.txt` before moving.
- **A STUB's CLOBBER SET is fictional, and that can be unfixable.** The
  visible-body rule ("`call A` with no `this` saved ⇒ A's body must be in the
  same TU") and the stub's own register usage pull against each other: cl decides
  what survives a call by looking at the callee it can see, so a trivial stub
  makes cl believe *everything* survives and it puts long-lived values in
  caller-saved registers where the original — calling a real function that
  clobbers them — used callee-saved ones. Making the stub heavier does not thread
  the needle: enough clobbering to push the value into esi also makes cl stop
  trusting ecx, so `this` gets spilled and the score drops further. There is no
  way to declare "preserves ecx, clobbers edx". When a caller is structurally
  exact and differs ONLY in register allocation around a stubbed call, stop and
  reconstruct the CALLEE first — its real clobber set is the missing input.
  (wip/scorekeeper.cpp: commit @0x472d10 is 68/68 instructions and 33.82%,
  blocked on rank @0x472b40.)
  **BUT IT DOES NOT REACH THE STACK PACKER.** The rule is about register
  allocation and SCHEDULING around the call. When a caller's only defect is
  which frame SLOT each local got, reconstructing the stubbed callee buys
  nothing: the two scratch shuffles @0x45e770/0x45e9f0 were measured at 82.12%
  while their tail callee m45e4e0 @0x45e4e0 was a STUB, and taking that callee
  to 100% moved neither of them by a hundredth of a percent. Read WHICH kind of
  mismatch you have before spending a function on it - renamed registers or a
  shifted schedule, yes; a permuted frame, no.
  **AND IT COSTS SCHEDULING, NOT JUST REGISTERS.** TestLightingSink::onMenu
  @0x475e60 was 91.04% with every instruction present and only four stores
  placed one push earlier than the original - six source forms of the array
  they initialize (initializer, element assignments, either order, a named
  index local, a named pointer local) all gave the identical wrong schedule.
  Reconstructing the stubbed callee it passes the array to (setLamps @0x475dd0,
  itself a first-build 100%) took the CALLER to 100% with no change to its
  source at all. When a caller is exact but mis-SCHEDULED around a stubbed
  call, that is the same signal as a register mismatch: do the callee.
- **Matching callers before callees (STUB markers):** to match a function that calls an
  unmatched game function, reccmp needs the callee's *address* mapped or the call reads as a
  mismatch. Declare the callee and give it a `// STUB: EZ2AC_UNPACKED 0xADDR` marker — the
  address is mapped for call-resolution and the stub body is NOT compared (no accuracy penalty).
  **The stub definition MUST live in a separate TU** (e.g. `src/stubs.cpp`): if cl can see an
  empty stub body it inlines it and deletes the surrounding code, changing the caller's codegen.
  **AND THE SAME IS TRUE OF A MATCHED CALLEE.** A new function placed in the TU
  that DEFINES its callee gets that callee inlined - GameOverDirector::m44d9d0
  @0x44d9d0 came out as four stores into Obj450570's fields instead of the
  original's `call 0x450420`, purely because src/directors.cpp defines both.
  Moving the new function to another TU took it from 21.74% to 100%. When a
  reconstruction's calls turn into their callees' bodies, look at which TU you
  put it in before touching the source.


- **CALL SET-UP HOISTED *BETWEEN* TWO COMPARES = NESTED ifs WITH ONE BODY
  WRITTEN TWICE.** `cmp a,5; mov ecx,[g]; push 2; push 2; jne B; cmp b,0;
  je B; call; ...A...; B: call; ...` - the common head of both arms sits
  inside the FIRST test's block. A flat `if (a != 5 || b == 0) B else A`
  cannot produce it (B is reached from two tests, so nothing hoists).
  `if (a == 5) { if (b != 0) A else B } else B` does: the inner if/else
  head-merges into its test block, the outer if/else then sees both
  successors open with the same set-up and hoists it again past its own
  compare, and cross-jumping folds the two copies of B back into one.
  (SongSelectDirector::drawCounter @0x434a30, 91.06% -> 100%; the source
  comment had called it "a head-merge cl declines".)
- **RUN THE PERMUTER ON A "COIN" BEFORE PARKING IT: scratchpad/perm.** A
  source permuter (decomp-permuter's loop, rebuilt for C++/COFF) mutates one
  function with the levers in this file - statement and declaration order,
  `volatile`, negated if/else, ternary, `for`/`while`, parentheses,
  early-return/else, `== 0` vs `!`, operand order - compiles each candidate
  with the real cl, and scores it against the original's bytes (addresses
  masked, a semscan-style guard rejecting behaviour changes). 2026-10-07, one
  10-minute-per-function batch over the 85-99.9% list: SIX new 100%s, and
  every one carried a comment saying its last defect was a register/layout
  coin no spelling moved - m405680 (`0 == (flags & 8)` keeps the test a
  dword), Panel::m427300 (parentheses round a float sub in the OTHER arm
  fix a load pair), SubsystemBackend::m40db00 and m406900 (`for` -> `while`
  with `i = 0` hoisted above the statement before it), setProjection (a
  divide moved above the memset), m44b4b0 (the two arms' picks in different
  orders). The candidate drags neutral edits along; minimise with
  `scratchpad/perm/check.py` before integrating. Blind spot: it masks
  addresses, so a wrong-NAME defect (a loop bound resolving to the wrong
  global) scores 0. README: scratchpad/perm/README.md.

## Big REPETITIVE functions are the cheapest bytes in the binary

The fan-out rule below says "big ⇒ inline, expect 1-4 iterations". There is a
sub-class that is better than that: a big function that is a **table of calls**
lands 100% on the FIRST build, because there is no register allocation or
scheduling to reproduce — every instruction is `mov reg,[this+off]; push; push;
push; call [reg+off]`. Measured: `KGraphics::m408130` @0x408130 (2,416 bytes, 190
`SetTextureStageState` calls, a 16-arm switch) and the four settings readers
(`loadCounters` @0x4156c0, `readSettings` @0x415a40, `readPatchSettings`
@0x415900, `readBookkeeping` @0x415070 — 1,600 bytes, ~80 `getInt` calls) all
matched first try. That is 4KB for roughly the effort of one hard 60-byte leaf.

**A better scan than repetitiveness: rank by BRANCHES PER INSTRUCTION, ascending.**
`(push-imm + indirect-call) / insns` only finds COM tables; branch density finds every
kind of straight-line function, and straight-line is what actually makes a function
land first try. `Effector6thStyle::updateSpeedDigits` @0x41d1d0 (1,861 bytes, 590
instructions, **6 branches**) is 40 `setCell` calls fed by `table[value / place % 10]`
— it matched on the first build and would never have shown up in a push-immediate
scan. Still open at the top of that ranking: 0x44b8f0 (3,449 bytes, 12 branches — the
biggest untouched function in the binary), 0x431090 (906, 0), 0x4324b0 (1,137, 7),
0x4343b0 (1,897, 13), 0x44e9b0 (2,347, 20).

**Constructors and destructors are the other bulk vein, and the dtor comes first.**
The engine's classes are built by huge EH constructors — 0x435390 is 14.7KB, and
0x451f10 / 0x448f70 / 0x457a90 / 0x4682e0 / 0x454990 are 4-9KB each — that are almost
entirely member-ctor calls plus `KGraphics::loadTexture(path, &member, 0)` with a
string-literal path. They are mechanical, but each needs the class's layout first, and
the class's DESTRUCTOR is where the layout is written down for free (see the
destructor-as-member-map rule in the cookbook). Do the dtor, then the vtable, then the
ctor. Naming comes free too: slot 0 of these classes returns a KRuntimeClass whose
first field is the class-name string.

- **ITERATE WITH `scratchpad/zdiff.py`, NOT WITH `./match.sh` - 2 SECONDS
  AGAINST EIGHT MINUTES.** `zdiff.py SRC.cpp SYMBOL LO HI` zoo-compiles one TU,
  objdumps the original over [LO,HI), masks the branch/call targets and the
  address-sized operands, and prints the instruction-shape diff plus a percent.
  A full `./match.sh` rebuilds every TU AND re-parses the PDB; even
  `./match.sh 0xADDR` pays both, so a single diff inspection costs about eight
  minutes and reccmp alone about the same. That is affordable for the FINAL
  check and ruinous for the sweep that finds the answer - the three background
  update2s took EIGHT source forms to crack, which is a whole session at
  match.sh speed and four minutes at zoo speed. So: sweep in the zoo, drive it
  from a python loop that prints one masked mnemonic line per variant, and
  spend the full match only to confirm and to catch collateral. Note the
  masking must accept a BARE hex branch target - objdump prints `call 1b`
  inside a .obj and `call 0x4145d0` in the image, and a mask that only knows
  the second form reports every branch as a difference. It must also keep the
  local labels: objdump prints cl's `$L60598` (a switch arm) in exactly the
  symbol form, and treating it as a new symbol truncates the capture at the
  jump table.
  **AND A ZOO DELTA ON A SHORT FUNCTION IS NOT A reccmp DELTA.** The compass
  rule above is about absolute scores; this is about CHANGES. Re-measuring the
  wip/ parks, the zoo said `Obj40FBD0::getBit` @0x40fbd0 had gone 72.7 -> 83.3
  while reccmp said it had not moved at all - on an eleven-instruction function
  a single masked operand is nine points, so the two metrics can disagree in
  the IMPROVING direction and the zoo is the one that lies. In the same sweep
  `PlayerSlot::m42e190` @0x42e190 really had gone 56.67% -> 72.15%. Screen with
  zdiff, decide with match.sh, and never bank an improvement the zoo alone
  reports.
  **AND A PARK'S RECORDED SCORE IS A SNAPSHOT, NOT A STANDING FACT.** Two of
  the parks are worth more today than their notes say - the audio loaders' seek
  by 14 points and m42e190 by 15 - with no change to either source, because the
  numbers predate some of the reccmp operand patches. Re-measure before
  deciding a park is still where it was left. The catch is that only 4 of 24
  parked files build standalone; the rest need their TU's context, so LEAVE A
  PARK SELF-CONTAINED if you want it re-checkable.
  **AND IT MUST SCORE THE BRANCH DISPLACEMENT, NOT MASK THE TARGET.** Masking
  a branch target outright is the obvious thing and it LIES on a branch-heavy
  function: it reports the block CONTENTS as matching while the layout is
  still wrong, so a change that grows the function reads as an improvement.
  Render each branch as its displacement from the next instruction - the same
  number the encoding holds, comparable across the two images, and what
  reccmp actually scores. On GFResultDirector::update2 the masked metric
  called a 22-byte-larger version a 7-point WIN where reccmp scored it a
  0.8-point loss; the displacement metric agreed with reccmp both times.
  Even so, treat zdiff as a compass and `./match.sh` as the verdict.
  (This generalizes the dozen one-off `*diff.py` scripts already in
  scratchpad/, each of which was this same tool hard-coded to one function.)

- **AND SWEEP FOR SIBLINGS WHOLESALE: `scratchpad/twins.py`.** It compares
  every unmatched function against every matched one (mnemonic + operand-shape
  sequences, addresses masked, length-bucketed so it runs in a minute) and
  ranks the near-copies. One run found seventeen candidates at >=85%, five of
  them EXACT copies of BackGround::videoName - one per director class, each
  with its own trim buffer, which is why the original's ICF left them
  unfolded. Run it after every batch of matches: each new match is a new
  template, so the list grows as the tree does.
  **AND LOWER THE THRESHOLD ONCE IT GOES QUIET - 0.50, NOT 0.85.** The >=85%
  vein is harvested (zero candidates at 85% AND zero at 70% as of 2026-08-17),
  but 0.50 still finds real work: the CROSS-DIRECTOR twins score only 65-68%
  because the three director families assign their registers differently -
  the scratch shuffles keep `this` in esi and the window in edi where the EZ2DJ
  pair does the reverse - and a whole-function register swap wrecks the ratio
  while leaving the source an EDIT of the template. Read the low-scoring pairs
  by hand before dismissing them; the two scratch shuffles @0x45e770/0x45e9f0
  cost a copy of their EZ2DJ template plus three edits.
- **DIFF TWO SIBLINGS' DISASSEMBLY BEFORE RECONSTRUCTING THE SECOND ONE.**
  0x404660 and 0x404950 share 146 of their 155 instructions - the whole
  difference is one colour against two - so the second function cost a copy of
  the first's source and a rename. Mask the address-sized literals, run the two
  listings through difflib, and read the opcode list: if the common run is most
  of the function, the sibling is an EDIT of the one you have, not a
  reconstruction. It fixes the signature for free too - m404660 had been
  stubbed with two `int` parameters guessed from call sites that only ever pass
  a literal zero there, and the shared body proves they are floats.
- **Find them with a repetitiveness scan**, not by reading: disassemble
  0x401000..0x478000, take call-rel32 targets plus the annotated addresses as
  function starts, and rank the unmatched ones by
  `(push-immediate + indirect-call) / instruction-count` with `size >= 400`.
  The winners have few branches and no EH prologue.
- **Decode them with a script, not by hand.** Walk the instructions, collect the
  pending `push`es, and emit one source line per `call` — following `jmp`s so
  cross-jumped tails are expanded back into every arm that reaches them.
- **Do not hand-write the cross-jumping back.** cl's tail merger reproduces it
  from the straightforward source: m408130's shared 4-call tail at 0x408a54,
  which each arm reaches by pushing its own `D3DTSS_COLOROP` value first, came
  out byte-identical from sixteen fully written-out cases.
- **Read the jump table for the case order** (see the switch rule below), but
  expect it to be plain 0..N here — a dispatch table this regular usually is.
- The payoff is not only bytes: a table of API calls names things. m408130's 190
  triples pinned `IDirect3DDevice7` slot 37, and the settings readers named 26
  globals from the .ini keys they read them with.


- **A STORE OF A VARIABLE WHOSE VALUE THE BRANCH ALREADY KNOWS = ONE SHARED
  TAIL IN THE SOURCE.** In the GF strum arm (m45d200 @0x45d200) the
  `stA == 0 && stB == 0` arm stores `bx[-4] = stA` as a REGISTER, not as the
  constant 0 cl would propagate. cl duplicates a shared tail into the arms
  after constant propagation has run, so a tail written ONCE after an
  if/else-if chain keeps the variable, while the same store written in every
  arm folds to the constant. Write per-arm stores only when the original
  stores constants. (24.18 -> 100 with the other m45d200 fixes.)
- **A BUBBLE SORT THAT INDEXES `names[j]` DIRECTLY (`[esp+ecx*8+k]`) AND HAS
  cl's DOWN-COUNTER ON THE OUTER LOOP IS `for (i = 0; i < n - 1; i++) for
  (j = i + 1; ...)` WITH THE BYTE SWAP WRITTEN OUT.** The `i = 1.., j = i,
  [i - 1]` form and a `for (k ...)` swap loop both let cl strength-reduce the
  j row into a pointer and spill j. Read the swap direction off the loads
  (`b = names[i][k]; names[i][k] = names[j][k]; names[j][k] = b;`) and put
  `scores[j] = t` where the original stores it - inside the byte run when
  that is where it sits. (RankingDirector m456b20 37.93 -> 94.61, m4570a0
  41.58 -> 75.98, m456870 56.62 -> 68.24.)

- **RUN semscan AND callseq ON EVERY BIG FUNCTION BEFORE TUNING REGISTERS -
  THE CALL MULTISET FINDS REAL BUGS.** 2026-10-03, one pass over the
  directors: a missing `m197c.setCell` and a wrong texture in the result
  ctor, a mode-6 `Radio` tag that should be `5Radio`, a fourth stage row drawn
  in every mode, `g_37dcea0` where the blink logic tests `g_37dce9c`, a
  Catch style name missing its `+ 1`, the paddle's two keys on swapped
  curves, a fret loop missing its `break`, and drawTex missing
  `m4080c0(1, 0)`. Each showed as a call/immediate/global COUNT mismatch
  (`imm "STYLE_%s%d_black_" 3 1`, `g g_1b2e590+c 1 0`, `call ... 25 23`).
  `callseq.py JSON ADDR` then gives the ordered position of the missing call.
  The JSON must be from the build currently in build/, or the recomp side
  resolves to raw addresses and every call reads as a mismatch.
- **AN ARM THE ORIGINAL PLACES OUT OF LINE AFTER THE OTHERS IS THE `else` OF
  A NEGATED TEST.** `cmp 7; je R; cmp 6; je R; ...; [plain arm inline]; ...
  R: [course arm]` is `if (m != 7 && m != 6 && m != 8 && m != 9) {plain}
  else {course}`, not the positive `||` chain with the arms swapped - cl
  lays the then-block first. (ResultDirector / GFResultDirector panel and
  ResultDisc picks.)
- **THE ICF RULE COVERS ANY IDENTICAL TAILS, NOT JUST SWITCH ARMS.** Two
  byte-identical `push str; mov [g],1; call Log; add esp,4; jmp X` blocks
  (SongSelect ctor) and three identical `push; push; call m4266c0; epilogue`
  returns (Judge::judgePress) stay separate in the original and merge in
  ours: the original called DIFFERENT symbols that the linker folded. Declare
  a second (third) copy with the same body and a `folded` annotation
  (LogAlt in src/applog.cpp; m4266c0b/c in src/accessors.cpp). judgePress
  68.9 -> 77.84.
- **`(int)(float)(x87 expression)` COMPILES TO THE INLINE fnstcw/fldcw/fistp
  SEQUENCE; `(int)(expr)` CALLS _ftol2.** An original with the cw-change
  block around a `fistp dword` where ours has `call __ftol2` wants the
  `(float)` cast in the middle (ResultDirector's log-score loop; the same
  shape as Panel::m42b2f0's window).
