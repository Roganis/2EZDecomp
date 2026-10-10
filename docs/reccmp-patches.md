# reccmp patches (tools/ is gitignored - reapply if the install is rebuilt)

Moved verbatim from CLAUDE.md. Patch files live in reccmp-patches/.

- **reccmp patches** (tools/ is gitignored — reapply if the install is rebuilt):
  1. `reccmp/formats/pe.py` decodes section names as `latin-1` (was `ascii`) so it tolerates
     the packer's non-ASCII section name. The bogus entry point `mainCRTStartup @0x3800000`
     (packer stub) is in `report.ignore_functions` in `reccmp-project.yml`.
  2. `reccmp/parser/parser.py` `CurlyManager` gets a cross-line `/* */` tracker
     (`_strip_block_comments`): without it, a comment line beginning `class X ...` pushes a
     phantom `X::` scope and every later GLOBAL in the TU resolves as `X::name` (this was the
     `identities::` 75% gotcha that stranded wip/setglobal.cpp — un-parked by this fix).
  3. `reccmp/analysis/float_const.py` skips the "operand must be a relocation" gate when the
     image has NO relocation table (our packer-dumped target): FP pool constants then resolve
     by const-region check + value equality on both sides automatically.
  4. `reccmp/compare/asm/parse.py` gets `hex_replace_displacement`, used for the
     `displace_replace_regex` substitution in place of `hex_replace_relocated`. A
     REGISTER-RELATIVE displacement `[reg + X]` is a struct member offset, not an address;
     naming it is only safe when X is the exact base of an entity, or when the operand has
     no base register (`[esi*8 + 0xd9ee38]`, a genuine interior reference into a global
     array). Otherwise leave it raw so both sides print the same number. Kept as a real
     patch file: **reccmp-patches/04-displacement-not-an-address.patch**.
  5. `reccmp/compare/ingest.py` resolves **ICF aliases**. A `folded` annotation lets several
     source functions share one ORIGINAL address; upstream assumes the recomp folds them too,
     which is false here because we link `/opt:noicf`. The patch gives each unmatched
     duplicate's recomp entity the `computed_name` of whichever entity won the address, so
     calls and vtable slots pointing at it symbolize identically on both sides (the
     duplicates stay unmatched and unscored). Patch + unit test:
     **reccmp-patches/05-icf-aliases.patch**, **05-icf-aliases-test.py**.
  6. `reccmp/compare/core.py` `_compare_vtable` credits an **ICF-aliased slot**. Patch 5 fixes
     symbolization, which is all an asm operand needs; a vtable slot is scored by comparing
     `recomp_addr`, which an alias can never satisfy (its recomp address is its own). A vtable
     with a folded slot was therefore stuck below 100% with every slot byte-identical. The
     patch credits the slot when the recomp entity is unmatched AND carries the matched
     original's name (which after patch 5 happens only by construction), and renders it in the
     winner's terms so the diff does not contradict the score.
     **reccmp-patches/06-icf-aliases-in-vtables.patch**; regression check =
     `KEngineApp::`vftable'` @0x487418 (60% without it).

  7. `reccmp/compare/asm/parse.py` also stops naming the IMMEDIATE of a multiply or shift.
     Same false positive as patch 4, one operand kind on: with a 53MB `.data`, a large
     struct stride passes the address test, so `imul esi,esi,0xe105c4` (a 14MB per-player
     element size) printed as `g_tracks[6].records+55132` on the original side.
     **reccmp-patches/07-imul-immediate-is-not-an-address.patch**; regression check =
     `addToListA` @0x418260 (97.06% without it).

  8. `reccmp/analysis/float_const.py` also scans the **SSE scalar** instructions.
     Upstream only looks at x87 opcodes (D8..DF), but with `/arch:SSE2` cl compiles
     float member arithmetic to `movss`/`addss`/`subss`/`ucomiss` and keeps x87 only
     for return values, so one function mixes both and only half its pool constants
     resolved. Patch + unit test: **reccmp-patches/08-sse-float-constants.patch**,
     **08-sse-float-constants-test.py**; regression check = `ShowCredit::setPos`
     @0x44fa50 (82.69% without it).

  9. `reccmp/compare/asm/parse.py` treats an **arithmetic immediate that lands INSIDE
     an entity as an offset**, not a reference to it — patch 04's rule applied to the
     immediate of add/sub/and/or/xor/adc/sbb/test. `add reg,X` can legitimately be a
     real address, so unlike patch 07 it is not excluded outright: it is named only
     when X is an entity's exact base. **reccmp-patches/09-arithmetic-immediate-offsets.patch**;
     regression check = `Obj40C180::createInput` @0x40c250 (97.75% without it).

  10. `reccmp/compare/asm/parse.py` applies the same rule to a **push immediate**
     (as often a constant as an address): named only at an entity's exact base.
     **reccmp-patches/10-push-immediate-interior.patch**; regression check =
     `KGraphics::screenShot` @0x40a7b0 (99.18% without it, which pushes SRCCOPY).

  11. `reccmp/compare/asm/parse.py` applies patch 09's exact-base rule to the
     immediate of a `mov` whose DESTINATION IS MEMORY. `mov dword ptr [mem],
     imm32` stores a constant far more often than an address - but it does store
     real addresses too, since a vptr store is exactly this shape, so it cannot
     be excluded outright. Naming it only at an entity's exact base keeps the
     vftable stores and drops the false positives (0xffffff, a 24-bit mask,
     was printing as `g_tracks[31].records+37071`). `mov reg, imm` is NOT
     covered - copyLane @0x412790 loads a genuine interior reference that way,
     which is why the blanket rule was reverted when it was tried.
     **reccmp-patches/11-mov-immediate-store.patch**; regression check =
     `Obj405F80::Obj405F80` @0x405f80 (96.55% without it).

  12. `reccmp/compare/asm/parse.py` resolves a displacement that sits a few bytes
     **BELOW** an entity's base as `name-k`. This is the EXACT OPPOSITE failure to
     patches 4/7/9/10/11, which SUPPRESS naming so both sides print the same raw
     number — sound for a member offset, which really is the same number in both
     images, and wrong for an address, which is not. cl folds the constant part of
     `arr[i - k]` into the addressing mode, so `buf[strlen(buf) - 3]` carries
     `buf - 3` in the operand; that constant points at no entity (it is short of the
     array's base), `is_addr` rejects it, and the two raw numbers differ purely
     because the images are based differently. The check runs BEFORE the `is_addr`
     gate, because the biased constant can also land inside whatever entity happens
     to precede the array — which resolves, but to the wrong thing and only on the
     original side. **reccmp-patches/12-biased-array-base.patch**; regression check =
     `BackGround::videoName` @0x41add0 (91.04% without it).

  13. `reccmp/compare/asm/parse.py` SHADOWS a tiny entity that abuts a buffer
     from below (`DISPLACEMENT_SHADOWED`, currently {0x37dd27c} - the ranking
     replay flag 4 below GFBackGround's trim buffer). As a register-relative
     DISPLACEMENT the address resolves through patch 12's biased naming
     (`g_gfVideoName-4` on both sides) and is skipped as a biased target
     (`buf[len-5]` scans past it to the buffer); ABSOLUTE operands still
     resolve by its own name, which is what RankingDirector's ctor stores
     produce. A blanket biased-over-exact precedence was tried first and broke
     three other functions - the shadow set is opt-in per address. Add any new
     flag-below-a-buffer global to the set when its buffer's videoName-style
     user regresses. **reccmp-patches/13-shadowed-displacement-entities.patch**;
     regression check = `GFBackGround::videoName` @0x45c470 (94.03% without it).

  14. `reccmp/compare/asm/parse.py` lets a **`push imm` name an entity's INTERIOR**
     for entities listed in `PUSH_INTERIOR_ENTITIES` (currently {`g_worm`}). Patch 10
     leaves an interior push immediate raw, which is right for a constant that
     happens to land inside a placeholder global and wrong for a struct whose
     members are passed by address (`push &g_worm.center`) - that shape is
     legitimate and the two images then print two different raw numbers.
     Opt in per entity NAME.

  15. `reccmp/compare/asm/parse.py` treats a listed **member offset that collides
     with the ORIGINAL's own `.text`** as never-an-address, in BOTH the
     displacement path (patch 4) and the arithmetic-immediate path (patch 9).
     Patch 4 already leaves a displacement raw when it only lands INSIDE an
     entity - but that test runs after the name lookup, and a FUNCTION interior
     never returns a name (`create_name_lookup`'s `get_name` refuses an offset
     name for anything but DATA/OFFSET). The address still passes `is_addr` on
     the ORIGINAL side, so it falls through to the `<OFFSETn>` placeholder,
     while the RECOMP side - whose `/base` puts every real address far above any
     offset this program can form - prints the raw number.
     **SO ANNOTATING AN UNRELATED FUNCTION CAN BREAK METHODS OF THE BIG BASE
     CLASS.** Obj40C180 runs to ~0x420290, so its `this` offsets sweep straight
     through the original's .text around 0x420000; annotating
     `EZ2DJMainGameDirector::m41ffa0` @0x41ffa0..0x420130 dropped FOURTEEN
     Obj40C180 methods at once with every byte of them still right. If several
     unrelated methods of one class drop together right after you annotate a new
     function, check whether that function's SPAN covers their member offsets
     before touching any source. The blanket fix - run patch 4's base-register
     test ahead of the lookup - was tried and is wrong: a switch's byte-index
     table (`movzx eax, byte ptr [eax + 0x4214ec]`) has the identical operand
     shape, is a genuine address, differs between the images and NEEDS the
     placeholder; it cost six functions. So the set is opt-in per address like
     patch 13 - and expressed as a **.text WINDOW** rather than an address
     list, because the colliding offsets come in whole runs of one class's
     members (m41ffa0 broke fourteen methods, its twin m420130 fifteen more).
     The rule for adding a window: it must be the span of annotated functions
     that contain NO jump table of their own, since a byte-index table inside
     the window would be suppressed along with them.
     **reccmp-patches/15-member-offset-in-original-text.patch**; regression
     checks = `Obj40C180::resetFlags` @0x40c8f0 and `Obj40C180::createInput`
     @0x40c250 (the second covers the immediate path).

  16. `reccmp/compare/asm/parse.py` lets an **arithmetic immediate name an
     entity's INTERIOR** for entities in `ARITH_INTERIOR_ENTITIES` (currently
     {`g_gfrPos`, `g_textSlots`, `_iob`}) - patch 14's opt-in applied to patch 09's
     operand kind. A row base folded into `add reg, imm` (`g_textSlots[1]` as
     `add eax, 0x1b2ec98`) is a real interior address and differs between the
     images. **reccmp-patches/16-arith-immediate-interior.patch**; regression
     check = `GFResultDirector::drawPlayers` @0x468050 (97.62% without it).

  17. `reccmp/compare/asm/parse.py` lets a **register-relative displacement
     name an entity's INTERIOR** for entities in
     `DISPLACEMENT_INTERIOR_ENTITIES` (currently {`g_textSlots`}). Patch 04
     leaves `[reg + X]` raw when X lands inside an entity, since with a base
     register X is nearly always a member offset - but an inlined strcpy into
     a fixed row (`strcpy(g_textSlots[3], def)`) indexes the byte with the
     register and folds the row into X: `mov [eax + 0x1b2ed98], dl`. Opt in
     per entity NAME, only for a global array whose rows are written this way.
     **reccmp-patches/17-displacement-interior-entities.patch**; regression
     check = `InGameSongSelect::m425ea0` @0x425ea0 (99.08% without 16's and
     17's `g_textSlots` entries).

  18. `reccmp/compare/asm/parse.py` normalises the **SEH scope table** push:
     a `push 0x...` immediately followed by `call __SEH_prolog` prints as
     `push <SCOPETABLE>` on both sides. The CRT's SEH frames are `push size;
     push scopetable; call __SEH_prolog`, and the scope table is static,
     compiler-generated .rdata with no symbol in either image (not in our map,
     not in our PDB), so it can never be annotated - two raw numbers for the
     same table. ~38 CRT functions carried exactly this one line.
     The same commit put `_iob` in patch 16's set: `_flsbuf` / `_lock_file` /
     `_unlock_file` compare against `&_iob[1]`, `&_iob[2]`, `&_iob[19]`.
     **reccmp-patches/18-seh-scope-table.patch** (both changes); regression
     check = `fclose` @0x479354 (96.30% without it), `_lock_file` @0x47d1d8
     (94.12% without the `_iob` entry).

  19. `reccmp/compare/asm/parse.py` extends patch 12 with **per-entity wider
     biases** (`BIASED_EXTRA`, currently {`__lookuptable`: 0x20}). `_output`
     indexes the CRT's printf state table as `__lookuptable[ch - ' ']`, a bias
     of 0x20 - four times patch 12's window, which stays small because a wide
     window would catch member offsets. Opt in per entity name with the exact
     bias. **reccmp-patches/19-biased-extra.patch**; regression check =
     `_output` @0x47b1dc (the `movsx eax, byte ptr [eax + __lookuptable-32]`
     line).

  20. `reccmp/compare/asm/parse.py` numbers `<OFFSETn>` placeholders by the
     count of DISTINCT replacement strings, not of replaced addresses. ICF
     `folded` aliases (patch 5) are several addresses on our side printing
     under the winner's name against one address in the original, so every
     later placeholder shifted by the number of extra aliases a function
     calls - SongSelectDirector::update @0x443380 calls nine and its four
     jump tables printed `<OFFSET40>` against `<OFFSET31>`. A name present on
     one side only still counts once per side, which keeps upstream's reason
     for counting names at all. **reccmp-patches/20-placeholder-count-distinct.patch**;
     regression check = `SongSelectDirector::update` @0x443380 (99.69%
     without it).
  21. `reccmp/compare/asm/parse.py` leaves a `mov reg, imm` raw when the
     immediate is a **BYTE-REPLICATED FILL PATTERN** (all four bytes equal,
     e.g. 0x01010101 from `memset(buf, 1, n)`) and is not an entity's exact
     base. The original's .data spans 0x4a5000..0x37e2714, so 0x01010101 ..
     0x03030303 land inside g_tracks and got an OFFSET name on the ORIGINAL
     side only - the mirror image of the /base problem, which /base cannot
     fix. Patch 11 deliberately leaves `mov reg, imm` alone (copyLane
     @0x412790 loads a genuine interior address that way); a byte-replicated
     value is narrow enough to exempt. **reccmp-patches/21-memset-fill-pattern.patch**;
     regression check = `KParser::m4133d0` @0x4133d0 (98.25% without it),
     also decryptFile/decryptFile2 @0x410a40/0x410c90 and slurpFile @0x4135f0.

  **And the structural half of the same problem: `/base:0x10000000` in match.sh.**
  Patches 4/7/9/10 suppress the INTERIOR case of "a member offset that collided with
  one of our own entities"; the EXACT-BASE case cannot be fixed in reccmp at all,
  because from one side a member offset landing on an entity's base is
  indistinguishable from a real reference to it. The fix is to put our entities where
  no member offset can reach: this engine's biggest offsets are ~0x420290 (Obj40C180)
  and 0xe105c4 (a 14 MB per-player element), so basing the recomp image at 256 MB
  leaves an 18x margin and the collision becomes impossible by construction — for
  .text, .rdata and .data alike. **If several unrelated byte-identical functions
  regress at once, check `/base` is still on the link line before suspecting anything
  else.** This superseded `src/aaa_pad.cpp`, a 4 MB unreferenced array linked first
  that only pushed .data clear; it was retired once the base moved, after a CRT string
  landed on 0x420220 in .data — which the pad could not have prevented, because
  zero-init padding goes to .bss and .bss is placed AFTER .data.

