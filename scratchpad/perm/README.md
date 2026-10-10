# scratchpad/perm - source permuter (v1)

decomp-permuter's loop rebuilt for this project: the stock tool parses only C
(pycparser) and scores only MIPS/PPC/ARM `.o` files, and our TUs are C++
compiled to COFF by cl 13.10.

    scratchpad/perm/perm.sh 0xADDR [-j 14] [--minutes 20] [--seed N]
                            [--only swap_stmts,if_negate] [--sym MANGLED]
                            [--src scratch_copy.cpp] [--no-guard]
                            [--start earlier_best.cpp]   # seed the pool, e.g. a longer second pass
                            [--restart N]   # back to the source after N candidates without a gain
                            [--region A-B]  # mutate only source lines A..B (see regions.py)

- **locate** - the `// FUNCTION:` marker in `src/`, then the definition under
  it (tree-sitter-cpp, installed into `.venv`). Functions with `__asm` are
  refused.
- **compile** - a copy of the whole TU with the function rewritten, same flags
  and per-TU exception model as `match.sh`, one worker dir per job, a
  persistent `wineserver -p` (headless env, same as `zoo.sh`). About 1.2 s per
  compile, mostly Wine; 14 workers give about 6 candidates/s.
- **score** (`score.py`) - capstone over our COFF function (relocations
  parsed, so a relocated field is an address) and over the original's bytes,
  read live from `original/` and never written anywhere. The original's extent
  is a linear sweep to the REAL end (CLAUDE.md), not the next annotation.
  Weighted edit distance: register-only operand 1, other operand 2, opcode or
  shape 4, insert/delete 3, wrong branch target +2. A member offset in the
  image range (0x420228) is NOT masked on the original's side - only our
  relocated fields are wildcards. 0 = every instruction equal modulo
  addresses; the % printed agrees with reccmp on the functions tried.
- **guard** - semscan's idea: callees (multiset), referenced globals and
  non-stack constants of each candidate must equal the baseline's (constants
  may also come from the original's). Changing those is a behaviour change.
- **mutations** (`mutate.py`) - the cookbook's levers: statement swaps (with a
  conservative dependence test: locals, member fields, constant-index arrays;
  calls never cross memory accesses), moving declarations, `volatile`,
  `i++`/`++i`, negated if/else, ternary <-> if/else, while <-> if+do-while,
  for -> while, parentheses, split/merge initialisers, inlining a one-use
  temporary, commuting operands, early return <-> else, `x == 0` <-> `!x`.
- **search** - hill-climb over a pool of the best 16, ties kept (plateaus),
  1..n mutations per child, deeper as duplicates pile up.

Output in `scratchpad/perm/out/0xADDR/` (gitignored - candidate TUs and
objects): `log.txt`, `best.cpp` / `best_<score>.cpp` (the function alone),
`best.diff` against the source, `compile_fail_*.cpp` samples. A best of 0 is a
CANDIDATE: paste it into `src/`, tidy it, run `./match.sh 0xADDR`, paste the
reccmp output. reccmp stays the only ground truth.

Helpers:

- `batch.py RC.json [--lo 0.85] [--minutes 10] [--resume DIR]` - every
  function reccmp scores in [lo, 1), best first; report-only, summary in
  `out/batch-<stamp>/summary.tsv`. Skips the worm static TU.
- `minimize.py 0xADDR [best.cpp]` - revert a candidate's edits one line-hunk
  at a time, keeping each revert that holds the score; writes minimal.cpp.
- `check.py 0xADDR...` - compile the CURRENT src/ TU and print the score; a
  seconds-long check while hand-tidying a candidate.
- `floatsweep.py RC.json` - every non-100% function's float pool constants
  against the original's, at aligned instructions AND as a multiset (uses
  build/obj/, no compiling). A screen for wrong literals: it found the
  ModeSelectDirector::update `-40`; most other multiset hits are merged arms
  or constant reloads (2026-10-09: 13 functions, read before acting).
- `regions.py 0xADDR` - the source-line spans a function's reccmp diff falls
  on (needs a current build); feed each to `perm.sh --region` for a big
  function.
- `verify.py 0xADDR [cand.cpp] [--keep]` - splice a candidate into src/, run
  `./match.sh 0xADDR`, restore the file (unless --keep and 100%).

Integrating: a candidate drags along neutral edits (`0 == end`, stray
parens, flattened indentation). Re-apply the edits one at a time with
`check.py` to find the minimal set, fix the comments the change makes
wrong, then `./match.sh`.

## Results (2026-10-07)

- Positive control: extractBaseName @0x412f00 (100% in `src/`) with its
  if/else damaged in a scratch copy (`--src`), score 12 -> 0 in 2 s.
- Batch over the 95 functions at 85-99.9%, 10 min each (all 95, in two
  sittings): **7 new 100% matches** - SlotOwner::m4226f0 (85.38%, one
  assignment moved above two declarations), and - m405680 (`0 == (x & 8)`), Panel::m427300
  (parentheses round an unrelated float sub), SubsystemBackend::m40db00 and
  m406900 (`for` -> `while` with the init hoisted), setProjection (a divide
  moved above a memset), ModeSelectDirector::m44b4b0 (if/else arm order).
  Every one had a comment saying its last defect was a coin no spelling
  moved. 13 more improved without reaching 0 (biggest: m438d30 87->45,
  m427d90 22->6, loadStageCourse 164->139, decryptFile2 79->65); 70 no gain;
  4 skipped (worm TU), 1 refused (m439f00, 8 markers on one address).
- Blind spot: TitleDirector::m44f300 scores 0 here at reccmp 98.67% - its
  defect is which global a loop-bound address NAMES, and the scorer masks
  addresses. Not a permuter problem (cookbook: a loop bound is an address).

## Not in v1

- Naming a subexpression (needs types), reordering struct/global
  declarations, macro bodies, inline `__asm` functions.
- Jump tables are cut at the table start on both sides but not compared.
- A score-0 result keeps the neutral mutations it picked up on the way
  (`0 == end`); tidy them by hand before integrating.
- Multi-function TUs (the worm static cluster) - scores one function only.

## Round 2 (2026-10-08): move_far + name_subexpr

- Panel::m427d90 96.30 -> 100 (three double locals for `t` plus a
  parenthesised `(double)` cast; found from a scratch copy with `q` inlined -
  the START point mattered more than the 30 minutes spent from its best).
- The other 11 improved functions, 15 min each from the source with
  `--restart 1500`: no matches. New bests on decryptFile (73 -> 65),
  decryptFile2 (79 -> 59), m435070 (124 -> 118); the rest at or above their
  round-1 bests. Several differ in INSTRUCTION COUNT from the original
  (m4391b0 217/215, m435070 215/218, ModeSelectDirector::update 2080/2072) -
  a missing or extra block, which no reordering can fix: read those by hand
  (semscan / callseq first).

## Round 3 (2026-10-09): the 70 no-gain functions again, new mutations

8 min each, `--restart 1500`. New 100%s: TestSound/TestBookkeeping/
TestModeDirector/TestGameOption/TestLighting::buildMenu (one hit, four
siblings by hand), EZ2DJMainGameDirector::m420130, Obj467930::rankPair (and
its twin ResultDirector::m451200 by hand), Panel::m42b2f0 + GFPanel::m463b40.
100% effective: KParser::m4133d0, KParser::slurpFile (one lea). Improved:
ShowCredit::update. Rejected after reccmp: m4221b0, PlayerSlot::m42f2d0,
m46d360 (all worse by reccmp), CatchPlayer::reset (needed an extra store),
GFResultDirector ctor (minimize.py had half-reverted a move).

The SCORER was wrong in four ways this round, each found by a reccmp
disagreement - always reccmp a 'gain' before believing it:
1. our side decoded switch jump tables as code (every switch function);
2. branch targets compared as indices - one insertion made every later
   jump 'wrong', in distance() and later found again in exact_pct();
3. float pool constants masked as addresses (a wrong 40.0 scored as right);
4. the guard's label/literal numbers ($L, $SG) changed with any reorder.
The objective is now (exact %, distance), exact % being the reccmp-like
measure. Functions 1-31 of this round ran under the old objective and all
70 under a flawed exact %: a re-run is the next step.

## Round 4 (2026-10-09): all 70 again, fixed scorer + reccmp-first objective

7 min each, `--restart 1500`. Every gain >= 1 point went through
minimize.py, a hand rewrite, reccmp, and for the ciphers the fmt harness.
- 100%: pickTextureFormat (int-flag guard + fourCC==0 body), TestSoundSink::
  onMenu (volume read before the path buffer is declared).
- up: decryptFile 87.50 -> 90.22, decryptFile2 87.10 -> 90.86 (both still
  decrypt all 12362 charts identically), m434080 91.43 -> 94.54,
  loadStageCourse 88.36 -> 90.04, GFPanel::m464580 95.04 -> 96.00,
  m444ae0 93.64 -> 94.21, ModeSelectDirector::update 98.23 -> 98.45.
- Two more permuter flaws found and fixed on the way: statement moves could
  cross an early return (m444ae0's candidate moved a store below two
  returns), and minimize.py could drop a rewritten-and-moved call
  (m434080's drawTex). Overall 97.65% -> 97.66%, no regressions.

