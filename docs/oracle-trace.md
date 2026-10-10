# The capture hook — scoping a differential trace against the running game

Scope for the next port task (`PORT-ROADMAP.md` Block 1's last open item).
Written 2026-08-10 by the decomp session at the owner's request; the work
itself belongs to the port session and lands in `port/`.

Revised after two findings, both in §0 — **that work is now committed**
(`8a056fb`: run.sh repaired, `ez2_score_commit` added, the sinkev sweep).
**The hook and the replay tool (§§2–7) remain a design, not an
implementation.** Corrected 2026-08-10 a second time: the sink's call sites
were misattributed (§0b/§4 below now carry the disassembly-verified split).

---

## 0. Two things to do before writing any hook — BOTH NOW DONE

Done 2026-08-10, in `port/oracle/` and `port/ez2/score.[ch]` (commit
`8a056fb`). Those paths are port-owned (`port/OWNERSHIP.md`: decomp never
edits); the edits were made from the decomp side **at the owner's explicit
direction**, which is recorded here in place of the usual `REQUESTS.md` entry.
The port session should treat `8a056fb` as an upstream change to its tree, not
schedule the §0 work again. Kept below because the reasoning is what shapes
the rest of the document, and §0c records what the work changed about the
hook's job.

### a. The sink oracle arm is BROKEN, and nobody has noticed

`port/oracle/run.sh` line 78 compiles `$ROOT/wip/sink.cpp`. That file no longer
exists: it became `src/sink.cpp` in `2cbd88e` when the sink matched. The arm
exits with `FAIL: could not compile wip/sink.cpp`.

This is the decomp session breaking the port's oracle, which is the failure
`OWNERSHIP.md` warns about in the other direction — and the whole run is skipped
where the VC7.1 toolchain is absent, so it fails quietly.

**It is a one-line fix, and the arm comes back better than it left.** Its own
header says

> `PlayerSlot::commit` @0x42e620 is not matched … so this arm compares the port
> against the decomp's READING of the sink, not against the original.

That caveat is now false. `@0x42e620` is **matched, 100%**. Repoint the path,
delete the caveat from `oracle_sink.cpp` and `run.sh`, and the arm is
**authoritative** — it is the original's own behaviour, not a reading of it.
That is a promotion of the single most important arm in the oracle for the cost
of one path.

### b. The reframing that follows from it

The existing arm already drives the matched sink and diffs `score.c`. **So the
sink's state machine is not what needs a live trace.** What the arm cannot
supply — and what reading has not supplied either — is:

* **the sink's CALLERS.** The twelve call sites sit in FOUR functions, three
  of them **unreconstructed** (attribution corrected 2026-08-10 against the
  disassembly — an earlier revision credited nine sites to the keys commit,
  which holds two): the **hold-sustain tick** `@0x42f4e0` (five sites — 0x42f66e,
  0x42f74c, 0x42f7e1, 0x42f808, 0x42f834; READ 2026-08-15 and reconstructed
  in `src/holdtick.cpp` at 67.40% — one call pays one lane's next instalment
  of a long note, always `count` 1 and `flag` 1, on a half/eighth/sixteenth/
  quarter of the measure by the slot's +0x210 latch, and its `kind` 6 arm
  GRADES the last two instalments instead of paying them flat), the hold pump `@0x42f880` (four — 0x42f94c, 0x42fae7, 0x42fb85,
  0x42fba1, the last with a variable `count`: the tick batch), and the keys
  commit `@0x42fd90` (two — 0x43026c, 0x4304d8, the only sites with a
  variable `flag`). The twelfth is `applyTracks`' site 0x422c65, already
  matched. Nothing anywhere establishes what sequence of
  `(grade, count, flag)` a real play actually produces.
* **`count` and `flag` at all.** The arm calls `m42e620(grade, 1, 1)` — count
  always 1, flag always 1. It sweeps the grade dimension and *only* that. Hold
  tick payout (`count > 1`) and the score-without-combo path (`flag == 0`) are
  completely unexercised, and `flag` is a parameter the port cannot currently
  express.
* **the director block**, i.e. the gauge rates the game actually parsed, which
  is the live check on `songini.c`'s positional reading.

So this task is not "verify the sink". It is **"verify what drives the sink"**,
which is a smaller and better-aimed job than the first draft of this document
described.

### c. What §0b's work then closed, and what it left

`count` and `flag` are no longer blind. `port/ez2/score.c` gained
`ez2_score_commit(s, j, count, flag, ini)` — the sink's own shape, with
`ez2_score_apply` as the pinned-at-1 wrapper — and the arm gained an additive
event sweep (`sinkev`) covering batch commits, a zero flag, a non-positive
count and CV2's latch under a batch. **6,087 cases pass**, and both new
semantics were confirmed to bite: reverting `combo += count` to `combo++`, or
gating the tally on `flag`, each produces a mismatch.

Two things that read as compiler-level trivia and are not:

* **A batch commit advances the combo by `count` while the tally and the score
  move by ONE.** Not a loop. `SlotOwner::applyTracks` @0x422bf0 (matched,
  `src/playerslot.cpp`) sums a helper over the player's whole track list and
  calls `m42e620(5, total, 1)` in a single call, so this is live gameplay.
* **`flag` gates the combo and nothing else.** A zero-flag MISS drains the
  gauge and tallies without breaking the combo — visible in the sweep's output
  as gauge 100 → 95.8 with the combo sitting still.

**What that leaves for the hook** is narrower than §2 implies, and better for
it. The sink's *parameter space* is now covered synthetically; what a trace
still uniquely supplies is:

1. **which sequences actually occur** — which of the twelve call sites fire, in
   what order, with what counts. Synthetic sweeps cannot invent the real
   distribution, and the callers are still unreconstructed.
2. **the director block as the game parsed it** — the live check on
   `songini.c`'s positional `.ini` reading.
3. **two players** (§5), which no synthetic arm can reach while the port has no
   second player to compare against.

## 1. Why it is worth doing at all

Reading is the method that has failed in this subsystem, four times — the
crossed gauge drains, the wrong director's gauge, CV2's combo latch, holds paid
at the head. Three of the four were caught by re-reading the same disassembly a
second time, which is luck with a delay. The one caught mechanically was caught
the moment an arm pointed at it.

The unreconstructed callers are the last part with neither a reading that has
been checked nor an arm that could check it.

## 2. The idea that makes it small

**Trace state transitions, not gameplay.** Hook the sink, record the inputs
`(grade, count, flag)` and the resulting state, then replay the recorded input
sequence through the port's `score.c` and diff.

Four consequences, each deleting a subsystem from the job:

* **No input capture.** Grades are inputs to this tier, not outputs.
* **No determinism, no seed forcing.** `srand(time(0))` in `pickDistinct`
  @0x433cf0 makes a random-option run unreplayable, and it stops mattering: the
  trace is self-describing.
* **No window, no audio device, no chart parsing** on the replay side.
* **Playing badly is the test.** Misses, fails, broken combos, released holds, a
  drained gauge. A perfect play exercises one branch.

## 3. What to hook — two functions, both matched

| addr | what | why |
| --- | --- | --- |
| `0x42e620` | `PlayerSlot::commit` — the judgement sink | the event stream, and through it the behaviour of its unreconstructed callers |
| `0x42e360` | `EZ2DJSlotObject::release` — stage teardown | the per-stage summary block, and the first mechanical check the grade ladder `@0x42e190` has had (decoded but parked in `wip/keysrank.cpp`) |

Both are matched and documented in `src/sink.cpp`, so signatures, field offsets
and globals are known rather than guessed. That is the reason to start here and
not at `@0x42fd90` itself.

**Do not hook the section parser `@0x4206e0`** for the chart's gauge rates. It
is a stub, so its signature is not established — and the rates are free from
state already being read: `dir->gaugeRate[1..5]` at `dir + 0x308`, via
`self->dir` at `+0x3d0`.

## 4. The record

Per sink call:

* sequence number, stage id, `this`
* **the call site**, from `_ReturnAddress()`. Free, and it says what kind of
  event this is. Two rules for reading it (corrected 2026-08-10): the captured
  value is the address AFTER the 5-byte `call` (site + 5, so site 0x42f66e
  records as 0x42f673), and attribution goes by **enclosing-function range**,
  not by comparing against call-site constants. The map:
  `0x42f4e0..0x42f873` = the hold-sustain tick `@0x42f4e0`;
  `0x42f880..0x42fbe2` = the hold pump `@0x42f880` (src/holdpump.cpp);
  `0x42fd90..0x4304fc` = the keys commit `@0x42fd90` (return address 0x430271
  = the normal path, 0x4304dd = the lane-swap variant);
  `0x422c6a` exactly = `applyTracks` `@0x422bf0`. The port models those
  separately, so this is how a divergence gets attributed rather than merely
  detected. **This field is the single most valuable thing in the trace**, since
  the callers are the thing under test.
* inputs: `grade`, `count`, `flag`
* state before and after: `combo`, `maxCombo`, `score`, `gauge`,
  `counts[1..5]`, `f1c0`, `audience`

Record **both** sides. After-only diffs a chain fine, but before-state catches
state changing *between* two sink calls — which means something the port does
not model touched the slot, and after-only would report that as a scoring bug.
It costs reading the same fields twice.

Per stage, at the first event: the director block (`gaugeRate[1..5]`,
`audienceRate[1..5]`, `f794`, `fd18`), `noteCount` at `+0xe105bc`, and the model
globals named in `src/sink.cpp` — `g_1b2eb6c`, `g_modeIndex`, `g_1b2ea20`,
`g_useAudiencePanel`, `g_1b2e7d4`, `g_elemCount`. A global is a plain read at a
fixed VA.

## 5. What the "player can notice it" ruling cuts

Applied concretely, per the owner's ruling of 2026-08-10.

**TWO-PLAYER IS NOT A CUT.** An earlier revision of this document cut it as
unnoticeable; the owner corrected that on 2026-08-10 — **a cabinet really does
run two players**, so 2P is ordinary gameplay and squarely inside the exact
column. Everything below is scoped with 2P in.

That makes it a **port gap rather than an oracle gap**, and the distinction
matters for sequencing: `ez2_score` has no second player, so there is nothing
for a 2P arm to compare against yet. Sweeping `checkBothPlayers` @0x42e83e
today would test the original against itself. The order is therefore: model a
second player in the port, *then* sweep it, *then* trace it. `oracle_sink.cpp`
now carries a real two-entry slot list (`sink_set_slots`) so the arm is ready
when the port side is; with one slot registered it behaves exactly as before.

What the 2P path actually does, for whoever picks it up: each player's gauge is
clamped independently, and game over is raised only when **both** are down —
with a `countListA() != 2` test inside that lets a single player in a two-slot
cabinet end immediately.

**Cut:**

* **`PlayerSlot::m42e2e0 @0x42e2e0`** as a separate hook. It is the mean of the
  grades the teardown already stored — arithmetic over data the teardown hook
  captures. (With 2P back in it is a real mean rather than a one-element one,
  but it is still arithmetic over captured data.)
* **`g_1b2ea2c`, the perfect-stage force.** Service/debug; no normal play
  reaches it.
* **The audience meter's value: record it, do not assert on it.** In audience
  modes the sink's own game-over check returns early
  (`g_useAudiencePanel != 0 && g_1b2e7d4 != 2`), so nothing in this function
  fails you on it. Whether something *else* reads `+0x21c` as a fail condition
  is unread — so capture the float, and if a trace ever shows the port
  diverging there, that is when it becomes worth the reading. One field, no
  test.
* **The `Log` arm.** Compiled out in the original (`@0x402880` is a folded `ret`).
* **The CV2 multiplier table.** Already established as `1 + log10(c)/3` to
  within 5.96e-7 across all 1,500 entries, and the repo may not carry a copy.
  Take it as given, as the existing arm does and says out loud.

**Do not cut, so this does not drift later:**

* **Two players** — see above.
* **`count` and `flag`.** Now modelled and swept (§0c). A player notices a
  combo curve immediately.
* **The gauge rates' positional mapping.** It decides whether you die.
* **The grade ladder.** It is the letter on the result screen.

## 6. Mechanics — BUILT 2026-08-10

The hook is `port/2EZConfig-V2-master/src/2ez-dll/oracle_trace.{h,cpp}`, gated
on a `game-settings.json` key `"oracle_trace": true` and the `ez2ac_fn_ex`
game id, wired into `dll.cpp`'s EZ2DJ install block beside `ScreenTrace`. It
hooks the sink `@0x42e620` (`__thiscall`, detour `__attribute__((thiscall))`
so the mingw build passes `this` in ecx) and the teardown `@0x42e360`, reads
the slot and director fields through the offsets in EZ2DECOMP
`include/sink-view.h`, and writes `port/ez2/trace.c`'s records to an
`oracle-<tick>.trace` beside the exe. The six model globals are read at their
literal VAs. **Confirmed armed in the running game** — `2ez-logs.txt` prints
`[Oracle] judgement-sink trace armed -> …\oracle-*.trace` right after the
other three EZ2DJ features. Everything below is the reasoning that shaped it,
kept because the next person to touch the hook needs it.

**To capture a trace:** set `"oracle_trace": true` in `game-settings.json`,
launch (`launch-ez2ac.sh`), and **play at least one stage** — insert a coin,
play, let the stage end. The trace buffers in memory and flushes at the
teardown hook, so it appears complete only after a stage finishes (and on
process exit). Attract/demo mode does NOT drive the sink: it never enters the
main-game screen, so a trace taken from an idle attract loop is empty (an
8-byte header still sitting in the stdio buffer). Then replay:
`port/build/ez2trace "…/oracle-<tick>.trace"`.

**Try the unpacked exe first.** `EZ2AC_Unpacked.exe` ships beside the packed
one. Be precise about what is and is not verified: the project's 12-byte
comparison (CLAUDE.md) is between the two **on-disk unpacked dumps**
(`EZ2AC_NO_ANTIDEBUG_Unpacked.exe` vs `EZ2AC_Unpacked.exe`) — it says nothing
about the packed exe's **in-memory post-unpack image**, which may carry
runtime fixups or anti-debug patches. If 2EZConfig can be pointed at the
unpacked exe, the packer leaves the problem entirely: no unpack wait, no
readiness poll, no anti-debug interaction. **Establish whichever identity the
chosen route actually relies on before writing anything else** — and if the
packed exe must be used, the §"readiness check" below is doing real work, not
re-confirming something known.

**No ASLR, so the addresses are literal.** Both exes declare image base
`0x400000` with `DllCharacteristics = 0x0` — no `DYNAMIC_BASE`. `0x42e620` is
`0x42e620` at runtime. No base + RVA arithmetic anywhere.

**If the packed exe must be used**, installation has to wait for the stub.
2EZConfig's answer is `patch_delay_ms` (default 2000) then
suspend-threads-and-patch. Reuse the mechanism but **replace the sleep with a
readiness check**: poll until the bytes at `0x42e620` match the prologue read
out of *the user's own* `EZ2AC_Unpacked.exe` at file offset `VA - 0x400000`.
That is a real check rather than a guess, and it ships no game bytes — the rule
`ez2/keytable.c` already follows for the cipher tables.

**Hooking** is `hook_create(target, detour, &original)` from
`2EZConfig-V2-master/src/libs/hooks/` — a MinHook wrapper that already exists
and already takes absolute addresses. The callee is `__thiscall` (`this` in ecx,
three stack args, `ret 0xC`); the usual MSVC detour shape is
`__fastcall(void *ecx, void *edx_ignored, int, int, int)`.

**Injection is SETTLED — build the hook INTO `2EZ.dll`.** The open question
was whether a second DLL could be injected; the answer made it moot. The hook
is a feature module inside the existing `2EZ.dll`, installed from the same
`InitThread` block as `InstantKeysound`/`FramePacing`/`ScreenTrace`, so it
rides the injection 2EZConfig already does. No `src/libs/injector/` fork was
needed. The `port/` tree vendors the whole 2EZConfig source; cross-build with
`build/linux-cross` (`make 2EZ`, `CMAKE_C_FLAGS` must carry
`-Wno-error=implicit-function-declaration` for the hardlock C, and the sub-build
caches must be wiped after copying the tree — they hard-code their origin path).

**Do not perturb timing.** The sink runs on the game thread. A dense stage is
~10k events at ~64 bytes, so buffer in a static array and flush at the teardown
hook and on process detach. Per-event `fflush` is the wrong trade — the thing
being measured is a rhythm game.

## 7. The replay side — IMPLEMENTED 2026-08-10

`port/tools/ez2trace`: read the trace, and per stage — init `ez2_score` from the
snapshot, feed each event, compare, stop at the first divergence and print the
event index, **the call site** (attributed by the §4 range map), and both
states. In `ctest` as `oracle_trace`, skipping (77) while no capture exists;
point `EZ2_TRACE` at a captured file. A before-state mismatch is reported as
"state changed between sink calls", distinct from a scoring divergence, per §4.

**The wire format is `port/ez2/trace.[ch]`** — that file is the contract the
capture hook must write to (plain C, byte-wise little-endian, so the hook can
lift the writer verbatim under MSVC). `ez2trace --selftest` proves the reader,
writer, replay loop, diff and attribution against the port's own model with no
capture needed (`trace_selftest` in ctest), including that a perturbed
after-state diverges at the right event and a perturbed before-state is
reported as out-of-band. One configuration is declared unmodelled and
skipped per stage rather than diffed: CV2 with the director's `fd18` set.
(The alternate-values option was the second such skip until 2026-08-13; it
is `EZ2_SCORE_KEYS_ALT` now and replays like any other stage, selected by
the sink's own gate — mode 6/7 with the stage's `g_1b2ea20` snapshot set.)

**The API it needs already shipped** (§0c, commit `8a056fb`). The faithful
entry point is

    void ez2_score_commit(ez2_score *s, ez2_judgement j, long count, int flag,
                          const ez2_song_ini *ini);

in `port/ez2/score.h`, with `ez2_score_apply` as the count=1 / flag=1
wrapper. Do NOT re-shape it to take a raw grade int or a bare `float[6]` —
the existing wrapper, `oracle_sink.cpp` and the passing sinkev sweep all sit
on this signature. The replay tool's mapping work is therefore only:
(a) the game's grade numbering 1..5 (MISS, FAIL, GOOD, COOL, KOOL) →
`ez2_judgement`, and (b) loading the trace's director block — the gauge rates
as the game actually parsed them — into the `ez2_song_ini` handed to each
commit, which is itself the live check on `songini.c`'s positional reading.

## 7b. FIRST LIVE CAPTURE REPLAYED CLEAN — 2026-08-13

The loop is closed. A human played one EZ2DJ stage with the hook armed
(`oracle-30912708.trace`, 68,548 bytes, `[Oracle] trace closed` in the log at
stage teardown), and the replay agreed with the game on every event:

    stage 1: slot=01b5fdd0 mode=0 notes=551
      rates by grade: fail=-4.8000 miss=-1.8000 good=0.1000 cool=0.2000 kool=0.2000
    1 stage, 552 events, 0 divergences

What that verifies in one pass: `ez2_score_commit` (all twelve call sites'
count/flag shapes as they occurred live), the director-block gauge rates —
i.e. `songini.c`'s positional read matches what the game actually parsed —
and the wire contract between the MSVC-built hook and the native reader.
No before-state mismatches either: nothing outside the modelled fields
touched the slot during the stage.

The capture lives locally in `port/traces/` (gitignored, like every trace);
`EZ2_TRACE=port/traces/oracle-30912708.trace ctest -R oracle_trace` now
PASSES instead of skipping. Coverage caveats: one stage, mode 0, single
player, no hold-heavy or count>1 batching guarantees — more stages under
different modes/options only add evidence, and the one declared-unmodelled
configuration (§7, CV2 with `fd18`) remains skipped by design.

**And a TWO-PLAYER capture replayed clean the same day**
(`oracle-32735067.trace`, also in `port/traces/`): two slots, two stages
announced up front, 226 interleaved events, 0 divergences. That live-checks
the stage-id demultiplexer on a real capture, both players' scoring at every
event, and — via the before-state consistency check coming up empty — that
the 2P cross-clamp (`checkBothPlayers` @0x42e83e re-clamping BOTH gauges on
every commit) never changes a slot's state between that slot's own sink
calls, which is exactly the argument the replay's independent-scores model
rests on.

## 8. What this does NOT verify

Say it in the tool's own output, the way the `Judge::grade` arm does:

* **the grades themselves** — inputs here, not outputs (§9)
* **timing and the tick conversion** — nothing in this tier has a clock
* **anything in EZ2CATCH.** `ScoreKeeper` is a different director with its own
  commit `@0x472fc0`; `wip/scorekeeper.cpp` has `@0x472d10` decoded if wanted.

## 9. Tier B, sketched and deliberately not started

Make grades outputs: capture `(tick, lane, dt, grade)` and replay presses
through `ez2_judge`. That verifies the window walk, the +3-tick widening and the
tick unit — everything `docs/judge-timing.md` argues for rather than measures.
It needs input capture, and for any random option the seed forced on both sides.
Tier A gives most of the value for a fraction of it.

## 10. Licence and housekeeping

Reusing 2EZConfig is already settled policy, not a new permission:
`port/OWNERSHIP.md` records the GPL-3 decision and `port/ATTRIBUTION.md` the
obligations. Concretely, for this task:

* keep kasaski's notices intact, **mark every file modified**, and log each
  derived file in `port/ATTRIBUTION.md`'s table — the hooks wrapper and anything
  taken from `src/libs/injector/`;
* **MinHook itself is BSD-2** and carries its own notice, separate from
  2EZConfig's GPL-3;
* it all lives in `port/`. Nothing derived from 2EZConfig may enter `src/` or
  `include/` — that flow is one-way for provenance, independent of licence
  compatibility.

**Traces are gitignored, like `original/`** — `port/.gitignore` covers
`*.trace` and `port/traces/` (added 2026-08-10; an earlier revision asserted
this before it was true). A trace holds no code and no assets, but the stage
snapshot carries gauge rates lifted from chart `.ini` data. Generate locally;
do not commit golden traces.

No request to the decomp session is needed for Tier A — both hooked functions
are matched. What would help separately is matching `@0x426500` (turns the weak
`Judge::grade` arm authoritative with no change to its sweep) and the sink's
three unreconstructed callers, the thing this trace observes indirectly — - ALL FOUR ARE
RECONSTRUCTED NOW (the keys commit landed 2026-08-16 at 66.89%). `@0x42f4e0` (src/holdtick.cpp) and the hold pump
`@0x42f880` (src/holdpump.cpp) are both read and reconstructed; only their
codegen is still open, and the pump is where `count > 1` never happens
either - every one of its four sites pays one note at a time.
