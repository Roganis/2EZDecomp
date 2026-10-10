# Local reccmp patches

`tools/` is gitignored, so the reccmp install is not version-controlled. These
patches are the durable copy — reapply them if the install is ever rebuilt.
CLAUDE.md describes patches 1-3 in prose; this directory holds the ones large
enough that prose is not a reliable reproduction.

Apply from the reccmp source root:

    cd tools/reccmp-0.1.6
    patch -p1 < ../../reccmp-patches/04-displacement-not-an-address.patch
    patch -p1 < ../../reccmp-patches/05-icf-aliases.patch
    # then append 05-icf-aliases-test.py to tests/test_compare_ingest_code.py
    patch -p1 < ../../reccmp-patches/06-icf-aliases-in-vtables.patch
    patch -p1 < ../../reccmp-patches/07-imul-immediate-is-not-an-address.patch
    patch -p1 < ../../reccmp-patches/08-sse-float-constants.patch
    # then append 08-sse-float-constants-test.py to tests/test_analysis_float_const.py
    patch -p1 < ../../reccmp-patches/09-arithmetic-immediate-offsets.patch
    patch -p1 < ../../reccmp-patches/10-push-immediate-interior.patch
    patch -p1 < ../../reccmp-patches/11-mov-immediate-store.patch
    patch -p1 < ../../reccmp-patches/12-biased-array-base.patch
    patch -p1 < ../../reccmp-patches/13-shadowed-displacement-entities.patch
    patch -p1 < ../../reccmp-patches/14-push-interior-entities.patch
    patch -p1 < ../../reccmp-patches/15-member-offset-in-original-text.patch
    patch -p1 < ../../reccmp-patches/16-arith-immediate-interior.patch
    patch -p1 < ../../reccmp-patches/17-displacement-interior-entities.patch
    patch -p1 < ../../reccmp-patches/18-seh-scope-table.patch
    patch -p1 < ../../reccmp-patches/19-biased-extra.patch
    patch -p1 < ../../reccmp-patches/20-placeholder-count-distinct.patch
    patch -p1 < ../../reccmp-patches/21-memset-fill-pattern.patch

Verify afterwards with `./match.sh` — the whole corpus must stay at
Accuracy 100%, and reccmp's own tests must be unchanged:

    cd tools/reccmp-0.1.6 && ../../.venv/bin/python -m pytest \
        tests/test_sanitize32.py tests/test_name_replacement.py \
        tests/test_match_msvc.py tests/test_fixes.py -q

## 05 — ICF aliases

`folded` already lets several source functions share one ORIGINAL address,
because the original's linker folded them (identical COMDAT folding). Upstream
assumes the recompiled build folds them too, so a single match covers all of
them. That does not hold here: we link with `/opt:noicf` precisely so every
other function keeps a 1:1 address mapping. Each duplicate therefore keeps its
own recomp address, only the first wins the match, and the rest symbolize under
their own PDB names — so a call or a vtable slot pointing at them differs from
the original even when the bytes are identical.

The patch gives each unmatched duplicate's RECOMP entity the `computed_name` of
whichever entity won the address, so both sides print the same string. The
duplicates stay unmatched and are never scored.

`05-icf-aliases-test.py` is the accompanying unit test. It was verified to FAIL
without the patch (the alias reports its own name) and pass with it. Note the
rest of that test module cannot run in this install — it depends on a `binfile`
fixture whose sample binary is not shipped — so the test brings its own stub.

## 06 — ICF aliases in vtables

Patch 05 fixes *symbolization*, which is all an asm operand needs. A VTABLE
comparison is scored differently: `_compare_vtable` credits a slot only when the
two entities agree on `recomp_addr`, and an ICF alias by definition cannot — its
recomp address is its own, not the winner's. So a vtable holding a folded slot
was stuck below 100% with every slot byte-identical (KEngineApp's vftable
@0x487418, whose slots 2 and 3 are both the shared `ret` @0x402880, scored
60%).

The patch credits a slot when the recomp entity is an ICF alias of the matched
original: unmatched, and carrying the original's name — which after patch 05
happens only by construction. Such a slot is also rendered in the winner's terms
on the recomp side, so the diff shows one line for it instead of a red/green
pair that the score then contradicts.

No unit test: `_compare_vtable` needs both images, i.e. the `binfile` fixture
that this source distribution cannot provide. `KEngineApp::`vftable'` @0x487418
is the regression check — it goes back to 60% if the patch is lost.

## 07 — an imul/shift immediate is not an address

Patch 04 stopped reccmp from naming a register-relative *displacement*; this is the
same false positive one operand kind further along. `sanitize` symbolizes any
immediate that passes the address test, and with a 53 MB `.data` section a large
struct stride does: `addToListA` @0x418260 multiplies an index by 0xe105c4 (a 14 MB
per-player element size) and the original side printed
`imul esi, esi, g_tracks[6].records+55132` against our plain `0xe105c4`, i.e. a
mismatch with identical bytes.

The immediate of a multiply, divide or shift can only ever be a scale or a count,
so those mnemonics are excluded from immediate substitution. `cmp` keeps its own
(annotation-only) path, and `push`/`mov` still resolve real address immediates.
Regression check: `addToListA` @0x418260 (97.06% without it).

## 08 — SSE scalar float constants

`float_const.py` only scanned the **x87** instructions (opcodes D8..DF), so a
float constant reached by `movss` / `addss` / `subss` / `ucomiss` was never
detected. In this binary that is not a corner case: cl 13.10 with `/arch:SSE2`
compiles float member arithmetic to SSE and reserves x87 for return values, so a
single function mixes both — `ShowCredit::setPos` @0x44fa50 has four x87
operands (which resolved) and eight SSE ones (which printed as `<OFFSETn>` on
the original side against our named constants, i.e. a mismatch with every byte
identical: 82.69%).

The patch adds a second scan for the SSE/SSE2 scalar instructions that READ a
float through an absolute 32-bit displacement: `F3 0F op` (scalar single),
`F2 0F op` (scalar double) for movss/sqrt/add/mul/cvt/sub/min/div/max, and
`(66) 0F 2E|2F` for u/comiss. The store form (`0x11`) and the integer converts
(`0x2A`, `0x2C`) are excluded, as is `F3 0F 2E`, which is not a compare at all.
Because the operand sits at a different offset in these encodings, the new
record carries the displacement's own address for the relocation check rather
than assuming `instruction + 2`.

`08-sse-float-constants-test.py` is the accompanying unit test — four cases over
`find_sse_float_instructions_in_buffer`, needing no sample binary (so unlike the
two tests already in that module, they actually run here). Regression check:
`ShowCredit::setPos` @0x44fa50 (82.69% without it).

## 09 — an arithmetic immediate that lands inside an entity is an offset

The third instance of the patch-04 false positive. `add reg, X` is usually pointer
arithmetic on a struct — `createInput` @0x40c250 forms `&this->keyboard` as
`add esi, 0x420104` — but unlike a multiply's immediate (patch 07) it CAN be a
real address, so it cannot simply be excluded.

The resolution is patch 04's: name the immediate when it is an entity's exact
base (the genuine `add reg, global` form) and leave it raw when it merely lands
inside one. An interior hit means the value collided with one of OUR recompiled
globals — the other image has nothing there, so one side prints
`g_textSlots[11][12]` against the other's `0x420104` and the operand can never
match even though the bytes are identical. Applies to add/sub/and/or/xor/adc/sbb
and test; `cmp` keeps its own annotation-only path and push/mov still resolve real
address immediates.

Regression check: `Obj40C180::createInput` @0x40c250 (97.75% without it).

## 10 — a push immediate that lands inside an entity is a constant

The fourth and last instance of the patch-04 false positive, on `push imm`. That
operand is as often a plain constant as an address, and this reconstruction's
placeholder globals are megabytes wide, so an ordinary constant lands inside one:
`screenShot` @0x40a7b0 pushes SRCCOPY (0xcc0020), which fell inside a 14 MB
array and printed as `g_tracks[55].records+68616` against the original's raw
number.

Same resolution as 04 and 09: name it when it is an entity's exact base — the
genuine `push &global` / `push "string"` form, which every matched function
relies on — and leave it raw when it merely lands inside one. An address that
resolves to no entity at all keeps upstream's placeholder behaviour, which is
symmetric on both sides.

Regression check: `KGraphics::screenShot` @0x40a7b0 (99.18% without it).

## A note on why these four patches keep recurring

All of 04, 07, 09 and 10 are the same false positive on four operand kinds. The
root cause is that this engine's classes are enormous — `Obj40C180` alone is
0x420290 bytes — so an ordinary `this`-relative member offset is a number in the
same range as an address, and our own placeholder globals used to live in exactly
that range.

`src/aaa_pad.cpp` attacks it from the other end. It is a 4 MB unreferenced array,
linked first, that pushes every later global clear of the member-offset range. The
patches suppress the INTERIOR case; the pad is what makes the EXACT-BASE case
impossible, and that case cannot be fixed in reccmp at all — a member offset
landing exactly on a global's base is indistinguishable, from one side, from a
real reference to it. It cost `Obj40C180::pending` 20% the day a descriptor landed
on 0x42026c. If those regressions ever reappear across unrelated functions at
once, check that the pad is still linked first before suspecting anything else.
