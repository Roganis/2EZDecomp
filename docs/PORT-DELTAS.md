# Decomp → port deltas — findings the port has not absorbed yet

`port/REQUESTS.md` carries traffic the other way (port asks decomp for a layout
change). This file is the mirror: **things the decomp has established that the
port's code contradicts, does not implement, or can now safely delete.**

Same discipline as REQUESTS.md — every entry names an address. Nothing here is
a style preference; each one is a place where the port and the binary disagree
about behaviour, or where the binary settles a question the port left open.

Written 2026-08-16 from the keys hold state machine (`src/holdtick.cpp`), the
last of the judgement sink's four callers to be read.

*(The decomp session does not edit `port/`. Mark an entry **done** here, or
delete it, once the port side has absorbed it.)*

**Port pass, 2026-08-16:** findings 1–8 and 10 absorbed in `port/ez2/score.[ch]`
(commit "port: the keys hold state machine ..."), finding 9 verified from the
disassembly (dead — see below), the unit question settled from the live trace.
What the trace added is recorded under each entry as *port note*.

---

## Read this first: the port's hold model came from the *catch* tick

`port/ez2/score.c`'s `ez2_long_note_divisor()` returns `{1:2, 2:6, 3:8, 4:16,
5:24, default:4}` and cites **@0x473660**. That citation is right, and so is the
table — for **catch**. 0x473660 is the *catch* hold tick; its kind-2 and kind-5
arms use magic-multiply division, i.e. genuinely ÷6 and ÷24.

The **keys** family is a different set of four functions with different
constants, now reconstructed in `src/holdtick.cpp`:

| address | what it is |
| --- | --- |
| 0x42fbf0 | `PlayerSlot::m42fbf0` — start a hold (latches the kind, sets the debt) |
| 0x42f4e0 | `PlayerSlot::m42f4e0` — the sustain tick |
| 0x42f880 | `PlayerSlot::m42f880` — the pump, run while the key is held |
| 0x42fd90 | `PlayerSlot::m42fd90` — the keys commit (press *and* release) |

Everything below is keys unless it says otherwise. The two families share a
skeleton and disagree on every constant, so the port needs **two** ladders, not
one parameterised by mode.

---

### 1. The keys instalment ladder is not the catch one

- **why**: the port applies catch divisors to keys holds. Wrong instalment
  count ⇒ wrong note total ⇒ wrong combo curve ⇒ wrong score.
- **what the keys ladder is** (all three functions agree; the divisor applies
  to what the chart object at `PlayerSlot+0x3d4` reports through its slot-8
  query):

  | kind | step |
  | --- | --- |
  | 1 | period ÷ 2 |
  | 2 | period ÷ 8 |
  | 3 | period ÷ 16 |
  | 0, 6, ≥13 | period ÷ 4 |
  | 4, 5 | the note's **own length** — exactly one instalment |
  | 7 … 12 | the kind is latched and **nothing else happens at all** |

- **evidence**: 0x42fbf0 (the ladder written out as a five-way chain),
  0x42f4e0 and 0x42f880 (the same split, reached per tick).
- **note on impact**: `port/tools/ez2play.c` passes sub-type **0** at both
  `ez2_score_hold_begin` sites, so today every hold takes the default arm and
  the tables agree by accident. This bites the moment the port starts honouring
  the note record's kind byte.
- **status**: done (port, 2026-08-16) — `ez2_hold_step()`; the catch table survives as `ez2_catch_hold_divisor()` for counting only.
- **port note**: the trace pins the ladder's unit. `oracle-30912708` is *theboy* 5KeyMix nm (31 holds, record lengths 48/96/144/192, four/nine/nine/nine of them) and it carries **309** hold-pump sink calls = 4·3 + 9·7 + 9·11 + 9·15 — i.e. `raw/12 − 1` per hold with `buf[2] = 48`, the beat, in a 192-per-measure chart. Every pump event is `grade 5, count 1, flag 1`; not one sink call comes from the tick @0x42f4e0 in 552 events, so in practice the pump owns the held frames.

### 2. `kind` is a byte of the note record, not the game mode

- **why**: the name in the port reads like a mode selector. It is not.
- **evidence**: the commit at 0x42fd90 reads it from the note record at **+0x32**
  and passes it to 0x42fbf0, which latches it into `PlayerSlot+0x210`; the tick
  and the pump read it back from there.
- **status**: done (port, 2026-08-16) — `ez2play` passes the record's byte.

### 3. The instalment count is `length / step − 1`, clamped up to 1

- **why**: `ez2_long_note_ticks()` returns a plain `hold_ticks / grid`. The game
  pays **one fewer**. This is general — it applies to every hold at every
  sub-type — and is independent of finding 1.
- **the exact rule** (0x42fbf0):

      if (g_mixStyle == 7)              count = length / step;          // no −1
      else if (g_1b5f300[player] >= 0)  count = length/step - (kind != 6);
      else if (kind == 6)               count = length / step;
      if (count <= 0) count = 1;

  `g_1b5f300` is seeded to 7 for all five players by `ModeSelectDirector`'s ctor
  @0x44b8f0, so the −1 is the normal path.
- **evidence**: 0x42fbf0.
- **status**: done (port, 2026-08-16) — `ez2_hold_instalments()`; the game's note total 551 = 242 heads + 309 instalments confirms both the −1 and that instalments count as notes.

### 4. Mix style 7 takes the plain quotient

- **why**: covered by the snippet above; recorded separately because the port
  has no mix-style hook in the hold path at all. The whole commit @0x42fd90
  exists **twice** for the same reason — `g_mixStyle == 7` picks between two
  near-identical halves, exactly as the sink @0x42e620 does.
- **evidence**: 0x42fbf0, 0x42fd90 (`g_mixStyle` is `0x1b2ebc8`).
- **status**: done (port, 2026-08-16) — `EZ2_HOLD_MIXSTYLE7` flag; no caller sets it yet.

### 5. Kind 6 pays only its final instalment, and only if still KOOL

- **why**: unimplemented. Every earlier tick just advances the due time and
  counts the debt down — no sink, no effect.
- **also**: in the tick @0x42f4e0 the *last two* instalments of a kind-6 hold
  are **graded** rather than paid flat, on how far past the due time the tick
  lands (a sixty-fourth of a measure is the window), and paying them ends the
  hold outright instead of decrementing.
- **evidence**: 0x42f880 (the last-instalment gate), 0x42f4e0 (the grading).
- **status**: done (port, 2026-08-16) — pump and tick arms both modelled.

### 6. The pump *restores* a failing lane instead of dropping it

- **why**: this is the single biggest behavioural difference in the hold path,
  and the port has no equivalent. A lane whose grade has fallen **below GOOD**
  is not paid — it is **set back to GOOD**, and that is *all* the pump does on
  that path: no sink, no effect, not even a decrement. Gated on the same
  `g_1b5f300[player] >= 0` option as finding 3.
- **evidence**: 0x42f880.
- **status**: done (port, 2026-08-16) — and it is the recovery path after an early release: the tick knocks the lane to FAIL, the next pump call restores GOOD.

### 7. COOL/KOOL bump a counter GOOD does not

- **why**: `grade > 3` additionally increments `PlayerSlot+0xc`; GOOD pays the
  sink and draws the effect but does not touch it. That asymmetry is why the
  original has *two* sink call sites in the pump rather than one. What consumes
  `+0xc` has not been traced.
- **evidence**: 0x42f880.
- **status**: done (port, 2026-08-16) — deliberately NOT modelled (no consumer); noted in score.h.

### 8. Two KOOL-override globals

- **why**: unmodelled in the port. `0x1b5f2e0` is a *state* in which every
  instalment pays KOOL outright and the release suppresses its effect draw;
  `0x1b2efa4` is the same thing as an *option*, and only the quarter division
  consults it.
- **evidence**: 0x42f4e0, 0x42f880, 0x42fd90.
- **status**: done (port, 2026-08-16) — deliberately NOT modelled (autoplay-flavoured); noted in score.h.

### 9. The release average may be dead code — verify before relying on either reading

- **the conflict**: `docs/PORT-ROADMAP.md`'s "Hold release" row says the commit
  averages the head and tail grades into one judgement. The reconstruction says
  the average is computed and then **discarded**:

      if (snd[lane].id != -1) {                 // closing a live hold
          if (grade >= f3cc->f38) {
              getState(&snd[lane], &tail);
              grade = (tail.f7 + grade + 1) / 2;   // the average
          }
          snd[lane].f0 = -1; snd[lane].id = -1;
          pay = 0;                                  // <-- same branch
      }
      ...
      shown = grade;
      if (pay == 0) return;                         // <-- before the sink AND
                                                    //     before m42d370
  Both the sink call and the effect draw live *after* that gate, so on a release
  that closes a live hold neither runs and the averaged grade goes nowhere.
- **why it matters either way**: if the average really is dead, the port's
  current behaviour (head grade applied at press, nothing at release) is
  **correct** and the roadmap row should be struck. If it is not dead, every
  hold whose tail judges differently from its head scores wrong.
- **caveat**: 0x42fd90 is at **87.19%**, so its branch structure is mostly but
  not certifiably right. This deserves one deliberate pass over the original's
  flow at 0x43009f … 0x430155 before anyone acts on it.
- **evidence**: 0x42fd90.
- **status**: done (port, 2026-08-16) — **verified dead** from the disassembly: `pay` is cleared at 0x4300f1 on the branch that averages (the average lands in ebp at 0x4300de and goes nowhere), and the gate `cmpl $0,0x14(%esp); je` at 0x43014e sits before the sink; the trace confirms it — 552 + 226 live events, no `flag = 0` commit anywhere. The roadmap row is rewritten. One correction to the reconstruction on the same function: the BELOW-threshold press branch (0x42ff5a) never clears `pay` — only 0x42fe73 does, on the above-threshold branch — so a hold head under the panel's threshold still commits like a tap; the recon's `pay = 0;` in that `else` is not in the binary.

### 10. The chord table is display flavour — the port can skip it

- **why**: this one *removes* work. `0x37dcdc0` is a 128-entry, 12-byte-stride
  table (start time, previous start, grade) that the commit maintains, and a
  second hit on the same start time adds `4 − |Δgrade|` to the counter at
  `0x37dce98`.
- **the finding**: **nothing in the entire `.text` reads `0x37dce98`.** Its only
  three references are the two adds in the commit and the per-stage clear at
  0x42e000. Its companion flag `PlayerSlot+0x5b4` (set when both grades are ≥
  GOOD and equal) is read exactly once, at 0x42d526, inside a panel function
  that gates a visual.
- **evidence**: 0x42fd90, 0x42e000, 0x42d526.
- **status**: done (port, 2026-08-16) — not implemented, on purpose.

### 11. Renderer curiosity: the half-texel bias goes on the wrong quad

- **why**: not actionable for a port with its own renderer, but it explains a
  half-texel misalignment if anyone tries to reproduce the original's output
  pixel-for-pixel. `KGraphics::m404660`, `m404950` and `m404c50` apply the
  half-texel bias to the **global** vertex quad at `0x4a5198` while drawing from
  a stack copy — the original's own bug.
- **evidence**: 0x404660, 0x404950, 0x404c50 (`src/kgraphicsquad.cpp`).
- **status**: done (port, 2026-08-16) — informational; nothing to do.

### 12. The note COUNTER and the hold STARTER use different ladders — confirmed, and it makes the maximum unreachable

- **status**: confirmed from the disassembly, 2026-08-16. `PlayerSlot::m42f2d0`
  @0x42f2d0 is now reconstructed in `src/playerslot.cpp` at **98.25%**, and it
  agrees with the port session's reading (`REQUESTS.md`, commit 3b40a73)
  instruction for instruction. Nothing here asks the port to change — the port's
  `ez2_note_counted` already implements it.
- **the consequence worth knowing**: the counter's ladder is 1:÷2, 2:÷8, 3:÷16,
  **4:÷32** (0x42f402), else ÷4. The starter's @0x42fbf0 is 1:÷2, 2:÷8, 3:÷16,
  **4 and 5: one instalment of the whole note**, else ÷4. They agree everywhere
  except kinds 4 and 5 — and the counter is what sets the maximum (`count * 300`
  at 0x430940) while the starter is what the pump actually pays out. So for a
  kind-4 hold the game **counts `width/32 − 1` notes toward the maximum and can
  only ever pay one**. A port that fixes the inconsistency would make a score
  achievable that the arcade cabinet cannot reach; reproduce both ladders
  separately, as the binary has them.
- **evidence**: 0x42f2d0 (counter), 0x42fbf0 (starter), 0x430940 (the maximum).
- **port note (2026-08-16)**: already the case — `ez2_note_counted` (the
  counter, in song ticks so kind 4's ÷32 divides where the game does) and
  `ez2_hold_step`/`ez2_hold_instalments` (the starter) are separate functions
  and `test_score.c` pins the disagreement (`kind 4 counts beat/32 steps here
  even though the machine pays ONE`). `ez2judge` totals with the counter and
  pays with the machine, so its "max" is the cabinet's unreachable one. Moot in
  the shipped data: no chart carries a kind-4 or kind-5 note (the non-zero
  kinds present are 6, 20 and 3). **Done — nothing to change.**

---

**Units for 13 and 14 (added 2026-09-28, port REQUESTS.md).** Every tick
below is a GAME tick, 192 to the beat, not a chart tick. `EzChart::rescale`
@0x411c10 multiplies each record's time by 192/48 at load and a record's
length by the same ratio *only when it is a hold* (`byte4 == 1` and length
`> 6`), and the director's chart load @0x423c18 logs `m_nTPB = 192`. So
`s`, `w`, `start`, the `+ 8` slack and the trim's `0x1e` / `0x12` are all in
game ticks - two chart ticks of slack, not eight - while a tap's width (0 or
6) is the one unscaled number in the table. The port read them as chart ticks
and its windowed shuffle, veto and trim came out four times too wide (port
commit 36b01df); its restatement is `ez2/noteorder.h`, above
`ez2_order_hold_veto`.

### 13. The scratch shuffle has no eight-tick hold slack; the EZ2DJ one does

Both time-sliced shuffles refuse to swap a lane whose hold covers the slice's
start. The EZ2DJ director (`m421750` @0x421750, `m4219c0` @0x4219c0) computes
that bound as `hold.start + hold.width + 8` — so a lane stays locked for eight
ticks *after* its hold ends. The scratch director's copies (`m45e770`
@0x45e770, `m45e9f0` @0x45e9f0) use `hold.start + hold.width` with no slack, so
a scratch lane becomes swappable the moment its hold finishes. One instruction
apart in the original (`lea edx,[ecx+eax*1+0x8]` against `add ecx,eax`), and
the two functions are otherwise the same source.

This is the same family as finding 7's `+0xc` asymmetry: a constant that exists
on one mode's path and not the other's, with nothing in the surrounding code
suggesting it was deliberate. **The port should keep both as they are** rather
than unify them — a shuffle that releases lanes eight ticks earlier changes
which swaps are legal, and that is observable in a replay.

Two smaller notes from the same pair, both port-relevant:

* The scratch versions call `m45e4e0` @0x45e4e0 (still a stub here) after their
  slicing loop and before the sink commit, passing the track list and the
  **original, un-advanced** window. The EZ2DJ versions have no such call. The
  early bail — an empty window — jumps straight into it, so it runs
  unconditionally.
* Because that call needs the original window, the scratch versions walk a
  separate cursor and leave the `start` parameter intact, where the EZ2DJ ones
  advance `start` itself. Behaviourally identical for the loop; it matters only
  if a port shares one implementation between the two, in which case the shared
  version must not clobber `start`.

Both scratch copies are reconstructed in `src/maingameslots.cpp` beside their
EZ2DJ templates. All four are structurally complete and stuck at 81–82% on one
stack-packing permutation, so for a port they are finished — see the note above
`m45e770` for why that permutation is not a source-level problem.

- **status**: done (port, 2026-08-29) — `ez2_order_hold_veto()` implements the
  per-mode rejection (kept apart, as asked), `apply_windowed` reproduces the
  stretch exactly (reject past `limit` instead of clamping; the mutated slice
  end persists), and the tail call is read and implemented: `m45e4e0`/`m46cdc0`
  SHORTEN every hold of raw width >= 0x1e by 0x12 after the slicing, ScratchMix
  and Catch only. `test_noteorder.c` pins the +8 boundary cell and the trim.

### 14. All THREE modes reject a swap differently — and Catch's test may be vacuous

Finding 13 compared two of the three time-sliced shuffles. Reading the third
(Catch, `m46df00` @0x46df00 and `m46e170` @0x46e170) shows the rejection rule
is different in all three, not two. Verified against the listings, not
inferred from a diff:

| mode | functions | rejects the lane when |
| --- | --- | --- |
| EZ2DJ | 0x421750 / 0x4219c0 | `w >= 6 && start >= s && start <= s + w + 8` |
| ScratchMix | 0x45e770 / 0x45e9f0 | `w >= 6 && start >= s && start <= s + w` |
| Ez2Catch | 0x46df00 / 0x46e170 | `w > 6 && start < s + w` |

(`s` and `w` are the hold's start and width as the sink reports them.)

So Catch differs on the width threshold too — **`> 6` where the other two use
`>= 6`**, a hold of exactly 6 being swappable in Catch and not elsewhere — and
it drops the lower bound entirely.

**The lower bound's absence may make Catch's position test dead.** The sink's
`begin(h, start, ...)` reports the first hit at or after `start`, so `s >=
start`, and then `start < s + w` is true for any `w > 0`. If that reading of
`begin` is right, Catch's rule collapses to "the lane has a hold wider than
6" with the position playing no part — which would explain why the lower bound
is missing rather than it being an oversight. **The port should verify what
`begin` guarantees before relying on this**; if `s` can precede `start` the
test is live and the collapse does not happen. Either way, implement the three
tests as written above rather than sharing one.

The rest of the Catch pair matches the ScratchMix shape, including the tail:
it calls `m46cdc0` @0x46cdc0 (already matched, `src/playerslot.cpp`) with the
original un-advanced window and then commits, exactly as ScratchMix calls
`m45e4e0`. So the "shared implementation must not clobber `start`" note in
finding 13 applies to all three.

- **status**: done (port, 2026-08-29) — all three tests implemented as
  written, and the open question is SETTLED from the reconstruction: the
  veto's `begin(h, start, 3, 1, ...)` dispatches to `EzChart::findBefore`
  @0x411d90 (`src/ezchart.cpp`, strictly `tick < start`), so the record
  PRECEDES the slice and Catch's lower-bound-free test is live, not vacuous.
  `test_noteorder.c` pins all three rules' boundary cells, including the
  width-6 record only Catch lets swap.

### 15. The gauge deltas are adjusted TWICE after the .gds says them

`EZ2DJMainGameDirector::m4206e0` @0x4206e0 (src/gaugerates.cpp, 100%) is the
JudgmentDelta reader, and it does not just store what the file says. Between
reading and logging it applies two adjustments the port has no equivalent for:

* **modes 5 and 9 make the positive deltas more generous** — COOL gains
  `0.05` and GOOD gains `0.02`, before anything else sees them;
* **with the flag at `0x1b2e89c` set, the two NEGATIVE deltas are DOUBLED**,
  from copies saved at +0x360/+0x364 before the mode adjustment. So a miss
  and a fail drain the gauge twice as fast.

That flag is the one `SongSelectDirector::toggleOption` @0x434f70
(src/songselecttoggle.cpp, 100%) flips, and it only flips in the four course
modes (`g_modeIndex` 6..9). So the doubling is a **course-mode gauge rule**
that lives in the .gds reader rather than in the scorer, which is why reading
the scoring path alone would never have shown it.

The un-doubled COOL delta is also copied to +0x31c before the doubling, so
whatever reads that field gets the file's value rather than the adjusted one.

- **status**: done (port, 2026-08-29) — the 14-key bonus and the doubling were
  already in `port/ez2/songini.c` (`ez2_song_ini_apply_mode_bonus` /
  `_apply_hard_gauge`); what was missing was Catch's GOOD zeroing (added to
  the mode-bonus function) and any CALLER — neither `ez2play` nor `ez2judge`
  applied the mode adjustments at all. Both wired; `test_score.c` pins all
  three rules. The doubling flag stays an unexposed option (the game offers it
  only via option 0 in modes 6..9).

**All three modes were then read and they agree on the doubling**, which is
what makes it a rule of the game rather than a quirk of one director:
`GFMainGameDirector::m45d960` @0x45d960 and
`CatchMainGameDirector::m46c2b0` @0x46c2b0 (both 100%, src/gaugerates.cpp)
do the same thing without the logging, and both double from the saved copies
rather than from the fields - which is also the only spelling that matches.

**And Ez2Catch has one more of its own: it ZEROES the GOOD delta outright.**
`m46c2b0` stores 0.0f over the GOOD field after reading it, so in Ez2Catch a
GOOD earns no gauge at all whatever the .gds says. The port should not read
that value out of the file for this mode.

### 16. MeasureScale from the .gds is thrown away outside the course modes

`EZ2DJMainGameDirector::m420640` @0x420640 (src/gaugerates.cpp, 100%) reads
the MeasureScale float and then **forces it to 1.6 unless the mode is 6, 7,
8, 9 or 12**. So outside the four course modes and mode 12 the file's value
never survives, and the field's own default of 2.0 is never seen either -
which is what the "then always 1.6 from the .gds" note on that field was
observing from the other side without knowing the cause.

The global at 0x1b2eae0 is then published from the scale in modes 6-9 and
from the constant everywhere else - **including mode 12, which keeps its own
file value in the field but still publishes 1.6**. The two ladders really do
test different sets (6/7/8/9/12 then 6/7/8/9); it is not a transcription
slip.

A port that reads MeasureScale out of the .gds and uses it will be wrong for
every mode except the courses.

- **status**: done (port, 2026-08-29) —
  `ez2_song_ini_apply_measure_scale(ini, mode)` reproduces both ladders (field
  forced to 1.6 outside 6/7/8/9/12; published value is the file's only in
  6..9), tested including the mode-12 asymmetry. The port's old "parsed but
  unused" note on the field — reasoned from the key string's absence in the
  binary — is corrected: @0x420640 reads the section POSITIONALLY, which is
  exactly why the string is absent. Nothing in the port consumes the scale
  yet; when the playfield does, it takes it from here.

## Unit question the port should settle while it is in there

The keys ladder divides "what the chart object reports" — and the two sides
disagree about what that number *is*:

* `port/ez2/chart.h`: `ticks_per_measure /* 192 in every modern chart */`, and
  `ez2_long_note_ticks()` derives `per_beat = ticks_per_measure / 4` before
  dividing by the sub-type divisor.
* `docs/judge-timing.md`: ticks are **1/192 of a beat** (the `.ez`'s 48/beat
  rescaled ×4 at load, `m_nTPB = 192`).

If the decomp's reading is the right one, the port's extra `/4` is exactly the
beat↔measure conversion and its grid already equals the game's step; if the
port's naming is right, the port's grid is 4× too fine. **The two readings
cannot both be true**, and only one of them makes the combo curve right. One
number — what `Obj422DD0`'s slot-8 query writes into `buf[2]` at run time —
settles it. This is worth resolving before finding 3's −1 is applied, because
the two compose.

**Settled (port, 2026-08-16):** `buf[2]` is the **beat** — 48 in a chart the
port calls 192 to the measure — so the port's `per_beat = ticks_per_measure/4`
naming was the right one and its extra `/4` is exactly the game's quarter. Proof
is the trace count above: only `step = 12` chart ticks against the *record*
length (biased by 6, i.e. 48/96/144/192, not 42/90/138/186) makes 309. In the
game's own song ticks (the .ez's 48 a beat rescaled ×4 at load) that is
`192 / 4 = 48` — the two readings were the same number in two units.

---

## Two observations from `port/REQUESTS.md` are now settled (2026-08-23)

That file's "Observations for the decomp side" section carries findings the
port made from the disassembly and the live traces. Two of them were aimed at
`src/` and both are now resolved from this side:

- **`PlayerSlot::m42f2d0` @0x42f2d0 is reconstructed** (`src/playerslot.cpp`,
  98.25%) and the two readings agree, including the part the port flagged as
  odd: the COUNTER's instalment ladder really is a different ladder from the
  hold STARTER's at @0x42fbf0. 1/2/3 agree at /2, /8, /16 and the default /4
  agrees, but kind 4 divides by 32 here (0x42f402) where @0x42fbf0 gives kinds
  4 and 5 a single instalment of the whole length. Nothing reconciles them;
  they are two ladders and a port needs both.

- **The spurious `pay = 0` in the keys commit is gone.** The port's reading
  was right: the below-threshold press branch at **0x42ff5a** opens directly
  on the `g_1b5f300` test, and only the above-threshold branch clears `pay`.
  `src/holdtick.cpp` had the clear at the top of that `else`; removing it took
  `PlayerSlot::m42fd90` @0x42fd90 from 87.19% to 87.57%, which is the
  independent confirmation. **A hold head under `Panel+0x38` commits like a
  tap.**

**A third one is settled, and the PORT WAS RIGHT.** `JudgeLane`'s field at
+0x00 (the `[-0x58]` flag in the port's note) makes `Judge::tick` grade a hit
**5 at its `start`** with no input, instead of 1 when the window runs out.
This tree called it "a hold-only lane"; `port/REQUESTS.md` called it
**autoplay**, and that is what it is. The chain, found 2026-08-23:

- `Judge::setup` @0x426390 zeroes it for all 96 lanes (0x426429).
- The only non-zero writer is the stage setup, per track handle:
  `setF0(h, g_1b5f2e0 != 0)` @0x426ca0 (`src/stagesetup.cpp:134`,
  `src/gfstagesetup.cpp:120`), right beside
  `setKind(h, (g_1b5f2e0 != 0) + 1)` on the sound clock. The catch setup
  passes a hard 0 (`src/catchstagesetup.cpp:116`).
- There is no random-access lane indexing anywhere in `.text` (no
  `imul reg,reg,0x5c`), so those two setters and `Judge::setup` are the
  complete set of writers.

So `kind == 1` iff the `g_1b5f2e0` option is up, and everything that option
does lines up with autoplay: auto-KOOL at note start, sound kind 2, a panel
effect from the keys commit, and a draw gate in `src/gamepanels.cpp`.
`include/judge.h` now records the chain. **A port may wire this to an
autoplay toggle.**

The other observations in that section are port-side notes rather than
requests against `src/`, and the two decodes it records for functions we have
not reconstructed - the Judge's expiry scan @0x426ab0 and the meaning of
`+0xe105b8` - are still the best starting point if either is ever taken up.

---

## Song-select input: seven per-mode behaviours, from 2026-08-23

Six of the thirteen per-mode input handlers were reconstructed on 2026-08-23
(`src/songselectinput.cpp`), joining the two that were already there. They are
the arms of the tick @0x446440's switch, and they are **not** interchangeable -
the differences below are behaviour a port has to get per-mode, not cosmetics.
Everything here is read off the original; where a handler is not byte-matched
that is a register-allocation gap, not a doubt about the logic.

| mode | address | grade |
| --- | --- | --- |
| 5KeyMix | 0x43ff00 | 88.39% (register wall) |
| RubyMix | 0x43b8d0 | 100% |
| StreetMix | 0x43c2e0 | 94.03% |
| 7StreetMix | 0x442770 | 93.91% |
| ClubMix | 0x440af0 | 99.70% |
| SpaceMix | 0x441540 | 100% |
| ScratchMix | 0x43ad10 | 100% |
| EZ2CATCH | 0x43a150 | 88.56% (register wall) |
| 14RadioMix | 0x43f390 | 55% (decoded, register wall) |
| 5RadioMix | 0x43dc00 | 54% (decoded, register wall) |

**1. The first option's stop list is different in every mode.** All of them
step `g_37dcecc` on button 7 (button 8 in ScratchMix), then:

| mode | stop list |
| --- | --- |
| 5KeyMix | 4/5/6 -> 7; 8/9/a/b -> 0 (two independent `if`s) |
| 7StreetMix | wraps at 0xb only |
| StreetMix | 4 -> 5, else 8 -> 9, else 0xb -> 0 (an else-if CHAIN) |
| ClubMix | 8 -> 9, else 0xa or 0xb -> 0 |
| SpaceMix | 4 -> 5, else 8 -> 9, else 0xa/0xb -> 0 |
| ScratchMix | 3/4/5/6 -> 7; 8/9/a and **>= 0xb** -> 0 |

The `>= 0xb` in ScratchMix and the `== 0xb` elsewhere are a real difference,
not a transcription choice.

**2. A FOURTH first-page option exists in some modes and not others.**
7StreetMix, StreetMix, ClubMix and SpaceMix step `g_37dcee8` on button 9
(wrapping at 4, or `>= 4` in SpaceMix), show it in readout cell 6, and light
**lamp 0xd** from it. 5KeyMix has none of that and leaves lamp 0xd dark. The
readout table differs too: `f_bff64` for 7StreetMix/StreetMix/ClubMix,
`f_bff34` for SpaceMix.

**3. ClubMix has NO per-player split.** Every other handler gates the wheel on
`(getListAF8() == 0 && countListA() == 1) || countListA() == 2` and then reads
only that player's accumulator. ClubMix reads BOTH accumulators and both
sides' buttons in one chain per direction (0x0f/0x17/0x1e forward,
0x10/0x18/0x1d back) with no list query at all, and its mode change does the
same with all six buttons. A port that applies the split uniformly will get
ClubMix wrong for a second player.

**4. The RadioMix modes do not ease the wheel.** 14RadioMix @0x43f390 and
5RadioMix @0x43dc00 have no `f_bfdd8` latch, no chart reset, no
`f_20c0d8 += k_twoPi / f7c` and no `f_20c0d4` follow - they step the cursor
directly and set `f_20c0e0`. Note the polarity: the POSITIVE accumulator chain
steps the cursor **down**, and each player side clears only its own hold
counter (in 14RadioMix player 1 clears neither).

**5. Those two modes publish a channel level and park the wheel at 7.** Their
mode change plays only `q4(0)` (no `q5`), reads an int per entry at
`this+0xdb104`, stores it to `g_1b2efd0`, logs `"Select Channel Level : %02d"`,
and sets `f_bfdb4 = 7` where every other handler sets 5.

**6. The course picker.** Both RadioMix handlers copy the current course's name
to `g_channelName` and then compare it against channel names, each match
drawing four distinct entries through `InGame::pickDistinct` @0x433cf0 and
publishing their names:

| gate | pickDistinct arg | 14RadioMix table | 5RadioMix table |
| --- | ---: | --- | --- |
| `"random"` | 0 | this+0xdb294 | same |
| `"random2"` | 1 | this+0xf48d4 | same |
| `"random3"` | 2 | this+0x10df14 | same |
| `"7znpfg"` | 3 | this+0x127554 | same |
| `"random4"` | 4 | *(absent)* | this+0x140b94 |
| `"11ptgc"` | 4 | this+0x15a1d4 | same |
| *(default)* | - | this+0xc0e44, indexed `f84*4 + k` | same |

5RadioMix has the extra `"random4"` arm; 14RadioMix does not. The first three
arms `strcpy` into `g_textSlots` directly while the rest call `setTextSlot`
@0x414e30 - the asymmetry is the original's, since setTextSlot is in another
TU and cannot have been inlined selectively.

**7. `pickDistinct` overruns its caller's array, and the frame proves it.**
The pick array in both RadioMix handlers is **four** ints - `buf` plus the
cookie account for 0x108 of the 0x118 frame, leaving exactly 16 bytes, and
only `int pick[4]` reproduces `sub esp,0x118`. `InGame::pickDistinct` writes
EIGHT, so the upper four land in `buf`. Harmless in the original (buf is 260
bytes and is written afterwards), but a port that declares eight will not
match the frame, and one that reads `buf` before the next `sprintf` would see
the spill.

**8. Preview directories are per-mode.** Most handlers load previews from
`System\Preview\%s.wav`; 14RadioMix uses `System\Preview_14radio\%s.wav`
@0x493a64 and 5RadioMix `System\Preview_5radio\%s.wav` @0x493a24. (This was
a live bug in the decomp for one tick - inherited from the template the
function was built on - which is exactly how a port would acquire it.)

**9. 5RadioMix loads the ranking file twice on a move.** Where the other
handlers call `m4332c0()` when the cursor moves, 5RadioMix calls
`loadRankingFile(rows[f84].name)` @0x431420 and then, only while `f3c` is
clear, calls it AGAIN and sets `f3c = 1`. `f3c` is cleared by a latch at the
top of the function whenever `f38` changes.

---

## Port REQUESTS of 2026-09-03 answered (2026-09-30 / 2026-10-01)

All nine open questions of the port's ten (PuzzleNote was answered
2026-09-15) from `SCREEN-AUDIT.md`, each from the
original's bytes (a whole-`.text` scan where the question was "who writes
this"). Details are at the declarations named.

* **Wheel chase, third arm** (`SongSelectDirector::update2` @0x446540): the
  bound is a FLAT `d > -50.0f`; the port was right, the decomp's C was not
  (a370c23).
* **`KEZPlayer` slot 27**: the play state `f7274` - 0 never started / walk
  ended, 1 playing, 2 stopped. The stage ends on 0; the judge also gates on
  `== 0` only, so a stopped player is still judged (a370c23,
  `include/ezscreen.h`).
* **The title's lamp walk and its period**: `TitleSong::step` @0x44deb0
  raises `m_strobe` once a beat at 405.4054 ms (60000/148); two beats per
  lamp group, eight per cycle (dbfae60, `include/ezscreen.h`).
* **`g_1b2efb4`**: set on a death by the three panels, cleared ONLY by the
  TitleDirector ctor. The port's "held for the rest of the session" is
  right; it must drop when the title screen is rebuilt
  (`src/moderunner.cpp`).
* **`ResultDirector::f2c40` phases 2/3/5**: unreachable - update2 @0x451550
  writes only 1, 4 and 7. **`g_1b2ef70`**: never raised - both writers
  store 0. The port's deletions stand (`include/directors.h`,
  `src/moderunner.cpp`).
* **Input slots 0x1d / 0x1e** (the menus' hold-to-repeat): no physical
  control. `bindButtons` @0x418c90 is the only button map and fills 0x00..0x19;
  nothing writes those two slots, so their state is always 0 and every
  long-hold arm reading them is dead. Nothing to bind (`include/bigbase.h`).
* **Exit code 5**: unreachable. `EZ2DJSink1::v0` raises it on run state 4,
  and the run state is only ever 0, 1 or 3; the 3 itself needs a ScrFader
  dim (mode 3) that no caller starts. Do not build it (`src/ez2djsink.cpp`).
* **The version badge** (`ani_Version` / `Version_View`): both take their
  cell from byte **+0x30 of the song's record in `system\<mode>\song.bin`**
  (0..0x12, `Version_%02d.bmp`), unpacked by `loadSongInfo` @0x431660. The
  same byte is `g_1b2eba0`, the background loader's `BG\commonbg*` switch.
  The port gets it by reading `song.bin` (`src/songbin.cpp` has the cipher).

## Behaviour fixes from the matching passes (2026-10-01)

Matching work found places where the decomp's C did something the binary
does not. The port should check whether it inherited any of them.

* **Song-select input, with the options page open** (`f_20c6bc != 0`): the
  selection index is NOT wrapped and the wheel is NOT eased that frame, and
  in ClubMix / EZ2CATCH the mode buttons are not polled either. In
  10/14Radio, 5Key, Street, 7Street the wrap/ease are also skipped while
  `f_bfdb4 != 4`; in 5Radio also while getState(4/5) == 3. StreetMix's
  option-page button needs `f_20c6bc >= 1` even when `f_bfdc4 == 1`.
  (dc48952, b839f9a, 84ec744; `src/songselectinput.cpp`.)
* **Title screen, credits reached:** the chart switches to PRESS START
  (`m_c68`) whatever `need` is - the C showed INSERT COIN (`m_c64`) for a
  one-coin credit. (0d98029, `TitleDirector::update2` @0x44f4d0.)
* **Result screen, the skip button** (`ResultDirector::update2` @0x451550,
  phase 1, one player, button 0x26): the block has NO `break`. After it
  moves `f2c48` to `f3108 - 1` and (unless the CV2 all-combo latch is up)
  sets phase 7, it falls straight into the next check - `m419780() != 0 &&
  f2c48 >= 0x9e` - which, with `f2c48` just moved to the end, usually fires
  too and runs `m467900()` twice more. The C had a `break` there (5d3b23b).
* **GF input, the effector keys outside race mode**
  (`GFMainGameDirector::m45d200` @0x45d200): keys 0x11 / 0x19 are NOT
  ignored when `g_1b2e7d0 <= 0`. The original's chord test fails, the strum
  test fails, and they land in the FRET arm - the tracker gets the state
  (`m462760`), the per-key state is latched, and only the fret-specific
  rewrite skips them. The C `continue`d past them.
* **The .ini cipher** (`KParser::slurpFile` @0x4135f0 decrypt,
  `m4133d0` @0x4133d0 encrypt): the C addressed `data` as
  `path + ... + 0x7f`, which only works if the stack puts `data` directly
  after `path`; in our build it did not, and the stages read the wrong
  buffer. Now written against `data` directly (fc8f209, a8739ac). A port
  that transcribed the old C should re-take it.

* **strchr and strrchr were swapped** - the two CRT addresses carried each
  other's names in `src/library.cpp`, and every caller was written to the
  wrong one so the bytes still matched. What the binary really does:
  - `getExeDir` @0x401080: argv[0] ends at the FIRST `"` after the opening
    quote, or the FIRST space (the C took the last - wrong once the command
    line has arguments).
  - `extractDirName` @0x412e90: cut at the LAST `\`, else the first `/`.
  - `extractDotName` @0x412f60, `extractBaseName` @0x412f00,
    `replaceExtension` @0x413000, `EzChart::load` @0x411720: the extension
    starts at the LAST `.` (and the base name after the LAST `\`), so
    `a.b.ez` loads `a.b.ezi`, not `a.ezi`; `extractDotName` then truncates
    at the FIRST `"`.
  - `KParser::getField` @0x413260: both `;` searches find the NEXT
    separator, not the last one in the string.
  - the ani-clip loader (`m401b80`): appends `.bmp` only when the name has
    no `.` at all (either search gives that - no change).
* **The gds section tests are case-insensitive** - `"Track"` (Panel,
  GFPanel, CatchPanel gds loaders) and `"Slot"` (the three EZ2DJ gds
  loaders) are `_strnicmp`, not `strncmp`; a `[track1]` section counts.
* **`atoi`, not `atol`** - every numeric field goes through libcmt's atoi
  (a `jmp atol`); no behaviour change on a 32-bit build, but a port on an
  LP64 target should use `int`.

* **The hold-note sample ladder** (`pickSample42de50` @0x42de50, called by
  `PlayerSlot::m42f080`): a rung answers when `rand() % 100 <= weight`,
  so each of the eleven rungs has a ~4-6% chance and the flat
  `rand() % 12` fallback is the common case. The C had `weight <= roll`,
  which picks rung 0 (sample 0x13) ~95% of the time.

* **The combo chart's head** (`TrackSink::m42d370` @0x42d370 keys,
  `CatchPanel::m472300` @0x472300 catch): the chart still fires on every
  fifth combo (`c >= 5 && c % 5 == 0`), but the head it attaches is
  `(c / 10) % 3` - combos 5, 10, 15, 20, 25, 30 give heads 0,1,1,2,2,0 - not
  `(c / 5) % 3`. The GF press flash has no combo chart.

* **The credit overlay's remainder digit** (`ShowCredit::update`
  @0x450040): drawn at `x150 - 4.0` on the CV2 skin and `x150 - 2.0` on
  the classic skin. The C had 1.5 and 2.5.

* **The total-result rank art** (`TotalResultDirector` ctor @0x454990):
  the percentage ladder is 100 -> 10, 98, 95, 93, 90, 85, 80, 70, 60,
  50 -> 1, else 0 - the same steps as the per-stage rankers
  (src/scorerank.cpp). The C had 95, 90, 85, 80, 75, 70, 65, 60, 55.

* **Background script, kind-4 records** (`BackGround::m41b680` @0x41b680,
  `CatchBackGround::m46b9b0` @0x46b9b0): at load the parser resets the
  three words `drawItem4` animates - item +0x28 (accumulator) = 0, +0x2c
  (the wave phase, which swings between 1.5 and 3.5) = **2.6f**, +0x30
  (direction) = 0. The C wrote 0 / 1.0f / 0 at +0x20/+0x24/+0x28, i.e.
  over the item's MATERIAL (+0x20, what drawItem4 hands setMaterial).
* **Mode select, the countdown digits** (`ModeSelectDirector::update`
  @0x4491b0): drawn at x = 569 and 591, not 568 and 590.

* **The song top-five file's name swap** (`ResultDirector::m450690`
  @0x450690, `GFResultDirector::m466ee0` @0x466ee0): when the sort swaps
  two records it exchanges SEVEN DWORDS through a `char` temporary - the
  far record ends up with the near dword's low byte sign-extended - at
  record offsets 4..28: the swap pointer is the row + 4 (0x450776 `lea
  ecx,[esp+0x30]` against the fread's record base +0x2c), so the first
  dword never moves. (Corrected 2026-10-01 night: an earlier entry here
  said offsets 0..24.) The GF copy (`GFResultDirector::m466ee0` @0x466ee0,
  0x466fc6) swaps only FOUR dwords, record offsets 4..16. Only the bytes
  written back to `sound\rank_*.bin` are affected; a port that must
  read/write cabinet files byte-compatibly should reproduce it.

* **The measure-line grid, the first line** (`Panel::m42b550` @0x42b550 and
  `GFPanel::m463d30` @0x463d30): the measure length is `beats * f2c` with
  `f2c` unsigned, so the first grid line `((cur - start) / mlen + 1) * mlen
  + start` is an UNSIGNED divide (`div`, 0x42b680). When the visible start
  `cur` is before the measure's start the difference wraps to a huge
  quotient and the grid starts far off - the original's behaviour. The C
  divided signed.

* **The EZ2DJ beat lamp** (`EZ2DJMainGameDirector::update2` @0x4249e0,
  output 4): lit for the first THIRD of the measure, `now % f34c < f34c / 3`
  (0x424ecc multiplies by 0xaaaaaaab and shifts 33) - the same as the GF
  copy. The C lit it for two thirds (`f34c * 2 / 3`).

* **The GF gauge at full** (`GFPanel::m4649c0` @0x4649c0): when the gauge
  is at 100 the original draws the GAUGE texture (`s1ad0.f18`) twice more
  over the bar through `m404660`, with the same quad and UVs as the bar
  itself (`v0 = (h - fnd) / h`, `v1 = fnd / h`) and a fresh random colour
  each time (0x464dd3 / 0x464ea6) - exactly what the EZ2DJ panel
  @0x42c0f0 does. The C drew `s1ad0.f1c` with `drawTex` at its natural
  size. Also both bar arms compute `frac = (1/h) * fnd`; the C's alternate
  arm divided directly.

* **The ranking files' sort** (`RankingDirector::m4570a0` @0x4570a0,
  `system\ranking\ranking_<mode>_<channel>.bin`, and the song top-five
  `RankingDirector::m456b20` @0x456b20, `sound\rank_*.bin`): when two
  entries swap, the bytes swapped are 1..8 of each 8-byte name - byte 0
  stays put and byte 8 is the following record's first byte. A port that
  reads/writes these files byte-compatibly has to reproduce it.

* **Ranking name entry, the confirm** (`RankingDirector::m45aaf0`
  @0x45aaf0): once a name is confirmed, it is copied to the player node
  (+0x1f8) and the saved name (0x4a8e8c) from the SECOND table when the
  f85df8 selector is set, and from the FIRST table otherwise. The C only
  had the first case, so with the selector clear the typed name never
  reached the node.

* **Ranking name entry, the keys** (same function): of the five keys per
  side at 0x4ae498 (`g_rankKeyRows[side][0..4]`), rows **1 and 3** are
  backspace and rows **0, 2 and 4** type the selected glyph (or act on
  END/DEL). In the two-side modes (4/5/8/9) the backspace keys are both
  sides' rows 1 and 3 and the type keys both sides' rows 0, 2 and 4. The
  C had the roles the other way round (backspace on rows 0/2, and in the
  two-side modes a mixed set). The "all 'A' or all blank -> EZ2AC_FN"
  test is two complete eight-byte `&&` chains, unchanged in meaning.

* **Course total, the per-stage result art** (`TotalResultDirector`'s
  constructor @0x454990, the plain-course arm - not modes 6..11): a stage
  without the new-record flag (`g_1b2efc0[i] != 1`) loads
  `g_tresStageArtB[g_stageSongIds[i]]` (0x4ae318), the same table the
  10/11 arm uses. The C passed `(char *)g_stageSongIds[i]` - the integer
  itself - as the path.

* **The effector panel's chart7 clip** (`Effector6thStyle::update`
  @0x41d920, the shared tail): with ONE player and the mode not 1, chart7
  (+0x10a8) is stepped when the audience panel is off (`g_useAudiencePanel
  == 0`) and `g_1b2eba4 == 1`. The C also required `g_modeIndex == 1` on
  that arm - impossible there - so the clip never ran in that case.

* **EZ2DJ hold notes, the mirrored bar** (`Panel::m42a630` @0x42a630,
  the `g_d36cb0 != 0` arm): the note-kind colour ladder applies to the
  HEAD cap only, and only for kinds 0xd (biased fade), 0xe (fade) and 0xf
  (period step) - those kinds draw the head cap twice, ladder colour first
  and then the plain ramp colour over it. Kind 0x10 gets no fixed
  0xffff6464 in this arm, and the body and tail caps always use the two
  ramps (cEnd/cStart). The unmirrored arm runs the full ladder on all
  three segments. The C ran the full ladder in both arms.

* **The .scr script's image records, the name offset** (same two
  parsers): a kind 4/5 record's file name starts at record+0x24 (the
  BgItem's `tex` slot - payload+0x1c), and the loaded texture handle is
  written back over it there. The C read the name 8 bytes early, out of
  the blend-mode words, and wrote the texture over `blendDst` (+0x1c),
  so every image layer of a background script loaded the wrong file and
  drew with a pointer for a blend mode. A port reading .scr files must use
  +0x24.

* **The .scr script parse resets the judgement-notice gate**
  (`BackGround::m41b680` @0x41b680 / `CatchBackGround::m46b9b0`
  @0x46b9b0): after a successful parse the gate the notice flasher
  m45cb20 tests (BackGround +0xac, CatchBackGround +0xa4) is set to 0.
  The C left it untouched.

* **Song select's carry-over toggles, the click sound**
  (`SongSelectDirector`'s constructor @0x435390): each toggle carried over
  from mode select plays the toggle sample as stop() (vtable slot 5,
  +0x14) then play(0) (slot 4, +0x10), like every other sample view. The
  C's view was a slot short and called slots 4 and 3.

* **The tutorial runs' style string** (`EZ2AC::runRubyMix` @0x416600,
  `runStreetMix` @0x4168c0, the `g_1b2eb44 == 1` tutorial path):
  `"STYLE_<mode>1_"` is written into the style PREFIX (0x1b2e5d0, what the
  song select and the player sink read), not the style tag (0x1b2e5ac).

* **The shared life gauge** (`Panel::m429170` @0x429170): the step width
  is the FULL span over the segment count (`players*100 / f1c48`) and the
  lit count is the SUMMED life over that step (`(int)sum / (int)step + 1`,
  less one on an exact multiple, clamped to f1c48); the remainder that
  scales the last segment is `(int)sum % (int)step`. The C had span and
  sum the other way round, which lit a count that grew as life FELL. The
  lit count is what g_1b5f2ac publishes and what latches gauge-over.

* **Graphics bring-up, a missing bitmap font** (`Obj405F80::v2` @0x406170):
  when `fontkr.dat` or `fontEn.dat` cannot be loaded from either path, the
  function returns 0 WITHOUT freeing the device table - both failures jump
  past `freeTable405530` to the bare `return 0`. Every earlier failure does
  free it. The C freed it on all paths. (Harmless for a port; listed because
  it is behaviour.)

* **EZ2DJ chart load, the key-range chaining** (`EZ2DJMainGameDirector::
  m423a90` @0x423a90): the three `m421600` calls on the first listed node
  run when the mode index (0x1b2e548) is 7 OR 3 OR the mix style is 2 - an
  `||` chain, every test of which jumps INTO the block on a hit. The C had
  the negation (`!= 7 && != 3 && == 2`). And the sink's slot-23 commit sits
  INSIDE that block, so outside those modes it never runs; the C committed
  unconditionally. (The GF loader @0x45fd50 has no mode test and already
  commits inside its block.)

* **Title screen, the start buttons with no coin in** (`TitleDirector::
  update2` @0x44f4d0): the ~46 s dim-to-attract test is ONE condition,
  `have == 0 && elapsed >= 0xb3b0 && need != 0`, and its `else` is the 1P/2P
  start poll (`m_104 >= 0x14`). So the start buttons are polled whenever the
  dim does not fire - including with the slot empty, for the first 46 s. The
  C nested the elapsed test inside `have == 0`, which polled the buttons only
  once a coin was in. Also: the coin-count reset logs "Reset Coin Count!!"
  (the same literal as the Test menu's), not "Coin Clear" - a string the
  binary does not contain.

* **The keys commit, an undrawn RELEASE** (`PlayerSlot::m42fd90` @0x42fd90,
  the non-style-7 half): when the release's effect is not drawn
  (`getF0(handle) != 1` or g_1b5f2e0 set) the original jumps straight to the
  `pay` test (0x430123 / 0x43012c -> 0x43014e), exactly as the press path
  already did - skipping `shown = grade` AND the `+0xe105b8 == 0 -> grade = 0`
  zeroing. So an undrawn release keeps its grade for the sink (`m42e620`)
  and for the display even when the e105b8 byte is clear. The C ran both on
  every release. (Only matters when `pay` survives the release, i.e. the
  `snd[lane]` hold was not open.)

* **Song select, state 4's page per mode** (`SongSelectDirector::update`
  @0x443380, the `switch (f78)` under `f_bfdb4 == 4`, table 0x4444b8): the
  arms were in the right ORDER but carried the wrong case labels. By the
  original's jump table: modes 0-3 and 11 draw the list page and then the
  side panels only when `countListA() != 2`; modes 4, 5 and 10 draw the list
  page and the panels unconditionally; mode 6 draws the wheel and mode 7 the
  wheel-with-list, both behind the `countListA` guard; modes 8 and 9 draw the
  wheel unguarded; mode 12 draws m439dc0 unguarded. The C drew wheels in
  modes 4/5 and the list in 6/7/12, and m439dc0 in mode 10. Every other
  `switch (f78)` in update already agreed with this mapping.

* **GF input, a fret key that changes state ENDS that lane's key scan**
  (`GFMainGameDirector::m45d200` @0x45d200, the fret arm): after
  `handleEvent(i, st)` and the keyStates store the original jumps to the
  NEXT LANE (0x45d539 -> 0x45d53b), so the lane's second key code is not
  read that frame. The pick latch is also NOT cleared after a fret is
  processed - the C ran `pick = 0` after every fret, the binary never does,
  so a strum's pick stays live for every later fret of the same scan. And
  in the strum arm the up-pick's `> 6` reset is reached for every non-2
  state (the C nested it under `else`, the same thing given `stA` is 2 or 3).

* **In-game song select, the hidden-song lists** (`InGameSongSelect::m425250`
  @0x425250, the per-stage `HID%d-%d` walk over `Stage.ini`): when a HID key
  is present, the song list that gets appended is read from the HID key
  ITSELF (`getStr(key, buf)` - both calls take the `HIDk-n` key buffer), not
  from the stage's `STAGEn` entry a second time. The C re-read `STAGEn`, so
  every unlocked hidden group duplicated the stage's normal list instead of
  adding the hidden songs. (Unchanged and still the original's: the walk
  over `g_hidCounts` only advances when the current count is > 0, so a zero
  count stops the scan at that entry for the remaining iterations.)

* **Catch paddle, which key drives which curve** (`CatchMainGameDirector::m46d360`
  @0x46d360, keys `side*8 + 0xf` / `side*8 + 0x10`): the original tests the
  `+0xf` key FIRST (it wins when both are held) and gives it the
  `5 > t ? t*t*6 : t*30` curve; the `+0x10` key gets `t > -5 ? t*t*-6 :
  t*30`. The C tested `+0x10` first and had the two curves on the opposite
  keys. (`t` is `delta[side]` times the 0.8/1.2/1.5 speed keys as before.)
* **Catch paddle, the hit window half-width is zeroed for good**
  (same function, the `g_1b2ef1c > 0` lane test): when `g_1b2ebb4 > 0` the
  original writes 0 into `half` ITSELF, not into a per-lane copy, so the
  rest of that call - the remaining lanes AND the second side's playfield
  clamp `(laneCount - 1) * width - half` - run with half = 0. The C reset it
  per lane.

* **Result screen constructor** (`ResultDirector::ResultDirector` @0x451f10),
  four corrections the C had wrong:
  - mode 6 loads its channel tag from `System\Channel_Tag\5Radio\`, not
    `Radio\` (mode 7 is the plain `Radio` one);
  - the stage panel's cell 2 (`m_197c.setCell(2, 0, ...)`) is the MODE ICON
    texture (+0x2ad8, `system\...\%s` mode art), not `ttStage[0]` - and the
    2P (countListA() == 2) branch sets it too, unconditionally; the 1P
    branch only outside CV2;
  - in CV2 the per-stage song-name texture comes from the CV2 name table at
    0x4ae224 (the 7 entries after the 11 normal ones), not from the normal
    table;
  - in Catch (mode 10) the eighth readout row draws `(int)(ra2 * 10)` - the
    score against the running maximum - where the C drew a constant 0.

* **Total result, the per-stage score rows** (`TotalResultDirector::TotalResultDirector`
  @0x454990, the rows 0x15/0x1c/0x23/0x3f on the score clip): every mode draws
  THREE stage rows (0x15, 0x1c, 0x23 from stages 0..2), and only the course
  modes 6..9 draw a FOURTH, row 0x3f, from `g_1b2e590[3]`. The C drew row
  0x3f in every mode, fed it `g_1b2e590[2]` (the third stage again), and
  left row 0x23 out of the plain (non-course, non-Catch) arm.

* **Song select, the STYLE name and its cell** (`SongSelectDirector::update2`
  @0x446540, after the cursor wrap): with `g_1b2eb7c != 1` the name is
  `STYLE_%s%d_black_` ALSO when `g_37dced0 == 1` or `g_1b2ef40 == 0`, and
  only otherwise the plain `STYLE_%s%d_`; the Catch name passes
  `g_37dceac + 1`, not `g_37dceac`. The cell published to `m_c05a8` row 2 is
  the style TEXTURE `f_c04b8[g_37dcf1c * 2]` (the pair table the ctor
  loads), not the raw index.
* **Song select, the idle blink** (same function, after the mode inputs):
  the three arms test `g_37dce9c` (the blink-request flag), not
  `g_37dcea0` (its counter) - `f_20c6bc == 0 && g_37dce9c == 0`,
  `f_20c6bc >= 1 && g_37dce9c == 0`, `g_37dce9c == 1` (which then steps
  `g_37dcea0`).

* **Renderer, every plain quad unbinds texture stage 1** (`KGraphics::drawTex`
  @0x403f80): after binding the texture to stage 0 (`m4080c0(0, tex)`) the
  original also calls `m4080c0(1, 0)` before the draw. The C left stage 1
  as it was, so a draw after a two-stage blit could still sample stage 1.

* **Song select, the per-frame voice pump** (`SongSelectDirector::update2`
  @0x446540, right after the START lamps): the voice's slot-17 call
  (`q17`, +0x44) runs when slot 16 (`q16`, +0x40) reports bit 0 SET
  (`test al,1; je skip`), not clear. The C had the test inverted in both the
  normal (`m_sndPreview`) and CV2 (`m_bfd44`) arms. The other screens' pumps
  (`!(q16() & 1)` in songselectinput/diffsel, and update2's own
  `m_sndMove2` pump @0x4482e2) are the other polarity - each checked against
  its `test al,1; jne` in the listing - so only this pump differs.

* **Stage setup, the all-KOOL flag is reset every stage** (`PlayerSlot::m42e000`
  @0x42e000, the last statement): `koolAll` (+0xe105b8) is written EVERY
  time - 1 in RadioMix with the audience panel, otherwise 0. The C only
  ever set it to 1, so once set it stayed set into a later mode/stage.

* **Applied from the port's CATCH-REVIEW.md "Decomp slips" (2026-10-03)**,
  each re-checked against the listing: S1-S3 were already fixed above; S4
  the catch hold's q3 gate is `halfvh` (`vh * 0.5`, stored at 0x46fd0f), not
  `capY`; S6 the `commonbg.scr` early return leaves `g_d36cb0` (the REVERSE
  flag) at 0; S5 and the naming notes are comment/name changes only
  (`ResStage.rank`, `g_stageRanks`, `g_rankArt*`/`g_bonusArt*`,
  `g_catchOverlay` = the BLACK option, `g_1b2eb6c` = CV2, the catch mirrors
  filter by gds KEY id, CatchNotify slots = KEZPlayer teardown /
  setMasterVolume).

* **Song select ctor, the speed index is NOT forced odd** (`SongSelectDirector`
  ctor @0x435390, at 0x435883): `g_37dcef8` (the SPEED option index, 0..0x14)
  is copied from `g_1b2f06c` and that is all. The C followed it with
  `*(char *)&g_37dcef8 = 1`, a byte store the binary does not have (the
  comment cited a `mov $1,%dl` that is not in the function), so every visit
  reset the low byte of the speed index to 1. (semscan sweep, 2026-10-05.)
* **GF result ctor, the clear mark reads entry 0 or 2** (`GFResultDirector`
  ctor @0x4682e0, at 0x469247): the index is `(notes == judged ? 0 : -2) + 2`,
  i.e. `g_clearMark[2]` on a full pass and `g_clearMark[0]` otherwise. The C
  read `[miss / 2]` = `[0]` or `[-1]`, and the slot below the table
  (0x49b1e4) holds 42, so a non-full pass got a non-null mark. The table
  (0x49b1e8, 20 bytes) is all zero in the binary and nothing writes it, so
  the original's value is always 0. (semscan sweep, 2026-10-05.)
* **The chart ciphers read the wrong buffer in the C** (`EzChart::decryptFile`
  @0x410a40, `decryptFile2` @0x410c90 - the .ez and .ezi decrypters). The C
  reached the file bytes through `header + size + 0x7f`, which is only `data`
  when the compiler puts `data` directly above `header`. In our VC7.1 build
  (and under g++) `out` is there instead, so the reverse stage copied the
  memset fill and the output was noise. Fixed to address `data` directly;
  verified natively on the shipped charts (rubymix1p-music.ez -> `EZFF`,
  5keymix1p-music.ezi -> its sample list). Any port code derived from the
  old C has the same layout dependence. (2026-10-05.)
* **The tokenizer's delimiter-type table was declared 6 entries short**
  (`g_parserTable` @0x4a8530, read by `KParser::parseToken` @0x413940 as
  `table[p - delims]`). The delimiter set has 14 characters and the table is
  14 ints (0x38 bytes, up to `g_parserKey` @0x4a8568); the C declared `[8]`,
  so `]` `(` `)` `<` `>` `/` (types 14..19) read past the array. `]` is the
  token `.gds` section headers close with (`engine.cpp`'s `parseToken(0xe)`),
  so the gds reader depended on whatever followed the array. Both tables are
  placeholders in src/ (data is not copied out of the binary) - a port must
  fill all 16 delimiter bytes and all 14 types. Found by running
  `KProfile::Load` natively: with the real tables loaded, all 18 of the
  game's quoted-token .ini files (Ez2ac.ini, patch.ini, version.ini,
  GameData.ini, every stage.ini) parse identically to an independent
  parser. (2026-10-05.)
* **The four quarter-step speed ladders were declared 18 entries short.**
  `g_scaleLadder` @0x48e6c8 (effector readout), `g_panelSpeedLadder`
  @0x4901a0, `g_gfScaleLadder` @0x49a1e0 and `g_catchScaleLadder` @0x49deb8
  are 34 floats each in the binary (0.25 .. 7.0 in quarter steps, then 7.5,
  8, 8.5, 9, 10, 99). The C had `[16]`, but they are indexed by the 0..0x14
  speed options - `g_panelSpeedLadder` shares its index with the 21-entry
  `g_panelRateLadder` - so options past 4.0x read off the end. Found by a
  size sweep (declared size vs the next global, with the original's data
  continuing past the declared end); `g_parserTable` above was the first hit.
  The other hits were checked and are fine (`g_commandCodes` reads <= 30 of
  40, `g_comboMult` is clamped to 1499, the grade cells to 0..3).
  (2026-10-05.)
  A second, code-based sweep (`scratchpad/sizesweep.py`: every address the
  original's code touches inside each global, against our declared size)
  found no further undersized arrays. The one remaining class is deliberate:
  the five subsystem singletons in `src/ez2acapp.cpp` (`g_obj1b2e320`, the
  graphics subsystem; `g_obj170dff0`, the 0x420290-byte input object; ...) are
  1-byte placeholders, never constructed in the matching build - any build
  that runs that code needs real storage for them.

## Port REQUESTS of 2026-10-03 answered (2026-10-05)

From `REQUESTS.md` "Filed 2026-10-03" (the EZ2Catch review). The decomp
cannot edit that file; the port should flip these to done.

* **S1, S2** (`m46d360` paddle: arms swapped, grace arm zeroing the wrong
  half): fixed in c9183c8. **S3** (catch style name `+ 1`): fixed in 3a21b41.
  **S4, S6** and the naming notes: c0ee7c0. **S5**'s comment: c0ee7c0.
* **"What makes EZ2CATCH scroll at 200%?"** The SongSelect ctor
  (`SongSelectDirector::SongSelectDirector` @0x435390, at 0x435853..0x43586d,
  [`src/songselectctor.cpp:213`](../src/songselectctor.cpp)):

      g_speedPercent = (g_modeIndex == 10) ? 200 : 250;

  (`sub eax,0xa; neg; sbb; and 0x32; add 0xc8`.) Every write to
  `g_speedPercent` (0x1b2e890) in the binary was checked; there are six
  writers. This ctor; the three directors' speed-key handlers
  (EZ2DJ `m422f20`, GF `m45f640`, catch `m46e750` - steps of 25, 1 or
  10 by key, clamped to 50..999; see `src/catchkeys.cpp`'s header); and `EZ2DJMainGameDirector`'s ctor @0x423f40, which
  sets 250 at 0x4241bd but is never constructed in mode 10. So catch enters
  the game at 200 and only its own speed keys move it, which is the
  measured 0.3195 px/ms. The port's default can cite the rule.

## Part 2 — what the port still needs from code that is not decomped

**Nothing is left undecomped.** Re-measured 2026-10-05 with
`scratchpad/coverage.py`:

    functions found              1476
      reconstructed (FUNCTION)   1474    474129 bytes   99.4%
      stubbed (address only)        0
      untouched                     2      2936 bytes    0.6%

The two "untouched" entries are import thunks (`jmp [IAT]` at 0x4145f0 -
timeGetTime - and 0x47748e), not game code. Every function the port could
need has C in `src/`; nothing here is "wip-only" any more.

**What this section used to say is obsolete - do not act on it.** The
2026-08-16 version listed "spine stubs" (0x435390, 0x40bd40, 0x403940, the
0x419a70/0x419e70/0x41a270 triplet, 0x425250, 0x408f70, ...) and twenty
`wip/` drafts (`wip/runclubmix.cpp`, `wip/slurpdecrypt.cpp`,
`wip/parsetoken.cpp`, ...). All of those are FUNCTIONs in `src/` now, and the
wip files are gone. Its nine "genuinely unread" addresses (0x45884e,
0x41e89c, 0x4191c0, 0x437f14, 0x452b24, 0x4367e0, 0x42a672, 0x41fb90,
0x4467fc) were never function starts: the old spine script took every
`jmp rel32` target for one, and each of them lies inside a function that is
reconstructed (the ranking, SongSelect, result ctors, update2, ...).

**A low match score is not a logic gap - but it is not a guarantee either.**
Most functions below 100% differ only in register allocation and
scheduling. Two kinds of real behaviour bug have been found anyway, both
listed above: count mismatches the semscan sweep surfaces (2026-10-03 and
2026-10-05), and C that only works for one stack layout (the chart ciphers,
which decrypted to noise until 2026-10-05). Prefer the current `src/` over
any copy made earlier, and check the "Behaviour fixes" lists above when
something the port derived disagrees with the cabinet.

The roadmap note that stood here ("open flavour is Block 7 / Block 8") is
from 2026-08-16 and has not been re-checked against the port's roadmap.

One thread that is now **closed**: all four callers of the judgement sink are
reconstructed (`src/sink.cpp` plus the three in `src/holdtick.cpp`), so the
scoring seam `docs/oracle-trace.md` observes indirectly is readable end to end.
That is what made findings 1–10 findable.
