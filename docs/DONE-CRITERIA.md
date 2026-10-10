# What "done" means for this decomp, and where it stands

Stamped 2026-08-30. This is the isle-style completion standard the project
adopted once the spine closed: **every game function annotated and
reconstructed, byte-matched where the compiler permits, with documented
effective matches accepted where it does not.** Whole-file identity was never
the goal (the target is a packer dump; CLAUDE.md defines done as per-function
`.text` identity), and the CRT is link-supplied, not reconstructed.

## The coverage bar - MET

Two tools define it, and both run clean:

* **`scratchpad/census.py`** - every *referenced* function start in the
  game's `.text` (call targets plus data-section pointers into text) that has
  no FUNCTION/STUB annotation. **Result: 0 functions, 0 bytes.** The tool
  carries the hand-resolved artifact list (jump-table interiors, one function
  tail, data-in-text, two import thunks) with the evidence for each.
* **`scratchpad/vtaudit.py`** - every pinned vtable's original slot targets
  against the annotation set (`_purecall` resolves from our own libcmt).
  **Result: 97 pinned tables, 0 with unannotated slots.**

Also met: `Implemented 100.00%` (1466 entities), `dupes.py` clean, and the
spine walker reports 99.7% of spine bytes matched with only the permanent
linker-thunk alias @0x426e40 left as a STUB (a `jmp 0x402880` our
`/opt:noicf` link never emits - by design, forever).

**Update 2026-10-01:** no STUB markers remain in `src/` at all. The folded
empty bodies (0x402880 `ret`, 0x475dc0 `ret 8`) and the 0x426e40 thunk alias
are FUNCTION markers now and score 100% - an empty body compiles to the
same bytes. `Implemented 100.00%` over 1663 entities (the CRT's library
functions are named and counted since this session).

Two honesty notes on the bar itself:

* (RESOLVED 2026-10-01: both tables below are now modelled in full - 19
  and 35 slots. Kept for the record.) reccmp compares a pinned vtable over
  the **modelled prefix**. Two classes were modelled shorter than their originals - Obj40D4E0's table @0x488320
  (8 of 19 slots) and KEZPlayer's @0x4887d0 (a prefix of 35) - because
  emitting the full tables means virtualizing methods that live as
  placeholder views (EngineObj, Obj40FB50, ...) across many matched TUs.
  Every slot TARGET is reconstructed and annotated; only the emitted table
  length is short. Extending them is mechanical but touches dozens of
  matched functions, so it is deferred deliberately.
* The census counts *referenced* starts. Code reachable only through
  never-taken data paths would escape it; the spine walker's own
  over-approximating sweep found none.

## The match bar - score distribution, 2026-10-01

Accuracy 97.14% over 1761 compared entities (reccmp's basis, CRT included):

| bucket | game .text | CRT | data/vtables |
| --- | ---: | ---: | ---: |
| 100% | 1273 | 195 | 78 |
| 90-99% | 65 | 8 | 0 |
| 80-89% | 41 | 4 | 0 |
| 60-79% | 54 | 0 | 0 |
| below 60% | 43 | 0 | 0 |

The late passes that moved it (see PORT-DELTAS for the behaviour fixes
they found): a jump-target comparison (scratchpad/jt3.py) and a mnemonic-
count comparison (scratchpad/mnscan.py) over every sub-100 function - both
see what the zoo diff masks - plus the folded per-mode copies, the reccmp
patches 17-20 and the CRT naming.

## The match bar - score distribution at the stamp

1563 compared entities (functions + vtables):

| bucket | count |
| --- | ---: |
| 100% | 1351 |
| 90-99% | 48 |
| 80-89% | 46 |
| 60-79% | 58 |
| below 60% | 60 |

Headline Accuracy 95.99%. The sub-100 population is two different things:

1. **Port-track integrations not yet match-polished.** Most of the big
   directors' constructors and ticks were declared done at "call sequence
   identical" (the port's oracle) and never got a matching pass. History
   says such passes pay: 10 of 12 re-read parks un-parked once the cookbook
   grew the right rule. This is the remaining ordinary work - roughly 150
   functions, tracked by re-running the gate and reading each diff against
   docs/cookbook.md.

2. **The walls** - residues where the instruction multiset is exact (or
   off by an alignment pad) and the difference is a cl 13.10 internal
   choice that no source spelling flips. Frozen list of families, each with
   its exemplar and its evidence:

   * **The constant-register assignment** - which register (or immediate)
     carries a small constant through a whole function. **NO LONGER A
     FAMILY (2026-10-01):** both exemplars, input5KeyMix and inputEZ2Catch,
     and seven siblings reached 100% - every one was a statement in the
     wrong block, found by diffing branch targets function-relative
     (cookbook). What is left is inputCV2Mix @0x441fa0 at 98.43%: body
     byte-exact, only the ebx/ebp saves sit after the lamp block, because
     `moved` must be initialised after it to win a register at all.
   * **The whole-function register permutation** - identical code, two
     callee-saved registers swapped everywhere. Exemplars: PlayerSlot::
     m42ef70 @0x42ef70 (ten forms swept), KSoundDS::_CreateSample @0x40eb80,
     KEZPlayer::drainQueue @0x40fd70 / reseedLane @0x40ffa0.
   * **The stack packer's slot assignment** - which frame slot a spilled
     temporary gets. Not steerable from source (cookbook); exemplars: the
     six functions the cookbook already lists, plus Obj40D230::m40d700's
     fill/size pair.
   * **Shared-block attachment** - cl attaches a merged tail/join to its
     first predecessor, the original to its last; only the shared-RETURN
     half is steerable (nest the success; a flag materialization worked
     verbatim on m406050 @0x406050 - 87% to 100%). The shared-JOIN half is
     not: KGraphics::loadSprite @0x408e80 (every fall-into-hit spelling
     duplicates the reloadBmpTexture tail), Obj40EAC0::init @0x40e7f0
     (the three-failure fold), enumD3DDevice @0x405060 (the bpp appends).
   * **List-scheduler placement** - one store or lea sunk a slot or two.
     Exemplars: loadTexture @0x408d00 / findOrCreateTexture @0x409200
     (the dwSize store; instruction set identical, 84/84), setProjection's
     rep-stos swap, m405680's one test-width narrowing.
   * **The frame-alignment wall (KNOWN-HARD)** - KProfile::Load @0x413fe0,
     the binary's only unaligned frame of its shape; no flag, no cl build
     (2179/3077/4035 identical), no source form in 35 experiments. See
     wip/kprofile-load.cpp before ever reattempting.

   A function parked on a wall is DONE under this standard when its diff is
   read, its residue named in the source header, and its calls verified
   (callcmp sequence-identical). Every exemplar above meets that.

## What would still move the headline

In order of value per hour, from measured history:

1. The sub-100 matching pass with the current cookbook (many of the 90+
   bucket are one documented lever away).
2. ~~Extending the two truncated vtable models~~ - DONE 2026-10-01:
   Obj40D4E0 (19 slots) and KEZPlayer (35 slots) are modelled in full.
   The slot bodies first written against stand-in classes (EngineObj,
   Obj40D420, Obj40D230, Obj40FB50, Obj40FBB0, Obj40FBD0, Obj40F610,
   Obj40FF50, Obj4100D0) are kept, __forceinline, behind slot methods
   that carry the markers; ICF-shared slots are `folded` aliases.
3. New wall breakthroughs - historically these come from new cookbook rules,
   not from re-sweeping; "the wall was half defect" has happened twice, so a
   park re-read after any new rule is always worth one look.

Nothing else remains: there are no stubs, no unannotated referenced
functions, no unowned vtable slots, and no open port requests.
