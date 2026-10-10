# Judge time semantics — how a press becomes a judgment (FN EX)

Investigated 2026-08-03 to answer: does judgement compare **timestamps** (press time vs
note time) or **frame-sampled state**? The answer decides whether an input-timestamp
backdating patch (2EZConfig) can improve judgement accuracy, and how the port should
model timing. Everything below is read from the original binary; addresses are VAs.

## Headline answer

**Judgement is a scalar time comparison (`dt = pressTime − noteTime`), so timestamps
matter — but the "press time" is synthesized at PROCESSING time, not captured at the
input edge.** When the per-frame judge pump processes a press edge (which the input
layer detected by frame-sampling), it stamps it with the *current* song clock minus a
fixed 5-unit latency compensation. All input-sampling delay (0..1 frame) therefore
lands in `dt` as a "late" bias. A backdating patch that substitutes the true edge time
is both meaningful and cleanly implementable (see "Patch design" below).

**The time unit is the chart tick, not the millisecond — and the tick is 1/192 beat.**
The song clock advances +1 per player step, a step is `m_rate` ms with
`m_rate = 60000 / ((f88/4) * speed)` (KEZPlayer::setSpeed @0x40f5c0 / play @0x40fa00),
f88 = EzChart::i88 (rate·4) and speed = f8c = the chart's BPM (EzChart::reset
@0x4109d0 defaults them to 192 and 120.0). The .ez file itself is 48 ticks/beat, but
the director's chart-load @0x423c18 reads the rate, multiplies by 4, and
`setState(flag 4)` RESCALES every record time to it — then logs `m_nTPB = %d` (= 192,
string @0x48f9b8, log push @0x423c5d). So:

    1 tick = 1/192 beat = 312.5 / BPM  ms      (2.08 ms at 150 BPM)

Judgment windows are therefore **beat-relative**: the same `[JudgmentDelta]` numbers
are tighter in absolute ms on a faster chart. Effective widths with the +3 widening
(normal mode), default windows {kool 6, cool 24, good 36, miss 72}:

    | BPM | tick ms | KOOL ±9t | COOL ±27t | GOOD ±39t | MISS ±75t |
    | 120 |  2.60   |  ±23 ms  |  ±70 ms   |  ±102 ms  |  ±195 ms  |
    | 150 |  2.08   |  ±19 ms  |  ±56 ms   |   ±81 ms  |  ±156 ms  |
    | 180 |  1.74   |  ±16 ms  |  ±47 ms   |   ±68 ms  |  ±130 ms  |
    | 200 |  1.56   |  ±14 ms  |  ±42 ms   |   ±61 ms  |  ±117 ms  |

(Shipped charts often override good=50 / miss=70 in the `.ini`.) The slot-22 `−5
ticks` compensation is ≈ −1562.5/BPM ms (−10.4 ms at 150 BPM), sized like the average
frame-sampling delay (~8 ms) — the devs compensated the input path's mean latency.
The prior RE's reading of `[JudgmentDelta]` values as "ms" (EZ2REWRITE
bible/behavior/judgment-scoring.md) should be corrected to "ticks (1/192 beat after
the ×4 load-time rescale)".

## The full chain

1. **Song clock.** `KEZPlayer.f7258` (+0x7258) is the judge's clock. It is set to 1 at
   `play()` @0x40fa00 and advanced +1 per step in the slot-34 delegate callback
   @0x40fd70 (`add [edi+0x7258], ebx` with ebx=1). Steps are driven by
   `KSongPlayerBase::advance` @0x4146a0 (matched, src/ksongplayerbase.cpp), which the
   sound pump thread `Obj414750::run` @0x414ab0 (matched) calls with `timeGetTime()`
   about every millisecond (`Sleep(m_sleep)`, m_sleep=1 default). So the clock has
   ~1 ms freshness, independent of frame rate — it does NOT freeze between frames.

2. **Press detection** stays frame-sampled: `Obj40C180::tick` @0x40cc90 edge-detects
   the port cache / DirectInput state once per frame (see CLAUDE.md input notes).
   The main-game frame update walks the resulting events and calls the per-lane
   handler @0x430970 with an event code: 2 = press → `Judge::judgePress` @0x426700;
   1 → the second judge path @0x4268c0 (release — see below);
   3 → @0x42f880, which is the HOLD SUSTAIN pump, not the scratch path this
   line originally guessed (see its own section below).

3. **Press timestamp.** Before calling the judge, the handler calls **KEZPlayer vtable
   slot 22 = @0x40f450** (vtable @0x4887d0 — it has 35 slots, not the 8 our
   include/ksongplayer.h currently declares):

       return max(f7258 - 5, 1);

   That is: *current* song tick at processing time, minus a fixed 5-tick input-latency
   compensation the original developers baked in (≈40 ms at 150 BPM if a tick is
   1/48 beat). Slot 22 takes no arguments; the four values pushed before it
   (lane, 1, 0, &out) are consumed by `judgePress` itself (`ret 0x14`, five args
   including the pushed time) — a push-merge across the call.

   Slot 27 = @0x40f490 (`getI7274`, the play state: 0 never started / walk
   ended, 1 playing, 2 stopped) gates the whole judge on `== 0` only: no
   judging before the start or after the chart walk ends, but a stopped (2)
   player is still judged. (Corrected 2026-09-30.)

4. **Judge setup.** `Judge::init` @0x426390 stores the chart reader
   (`director+0x148`) at Judge+0x24dc and the KEZPlayer (`director+0x14c`, created via
   the KRuntimeClass factory @0x4a733c → @0x410440 → ctor @0x4102c0, size 0x727c) at
   Judge+0x24e0. Per lane (stride 0x5c, count at +0x2294) it keeps TWO six-entry
   window tables — one for early presses, one for late — both defaulted from the
   six dwords @0x48ff98 = {-1, -1, 24, 12, 4, 2} (widest→tightest).

   `Judge::setWindows` @0x426ea0(kool, cool, good, miss) overwrites the tight four
   slots of both tables in every lane. The caller @0x430793 passes the per-chart
   `[JudgmentDelta]` values (director +0x304/+0x300/+0x2fc/+0x2f8, loaded by
   @0x423790) **widened by +3** (normal; global 0x1b2eb6c == 0) or **+1** (alternate
   mode). Windows can therefore be asymmetric in principle; the shipped call sets
   early = late.

5. **Grading.** `judgePress` finds the candidate note per lane, tie-breaking two
   candidates by smaller |pressTime − noteTime| (@0x4267f5). The core @0x426500 does:

       dt = pressTime - noteTime
       if dt < 0: early (flag +0x24e8) else late (flag +0x24ec)
       scan |dt| against the lane's early/late window table, tightest first:
         grade 5 = KOOL, 4 = COOL, 3 = GOOD, 2 = inside miss window but worse
         than GOOD (resolves the note as a miss — the "mash penalty" band),
         0 = outside all windows: stray press, note not consumed
       grade 4 (COOL) additionally tallies the global FAST (0x1b2eaa8) / SLOW
       (0x1b5f1a0) counters from the early/late flag.

   Gauge/score commit is downstream (FUN_00472fc0 family — already RE'd in
   EZ2REWRITE bible/behavior/judgment-scoring.md).

## Consequences

- **For feel:** press timing is quantized to *when the frame processes the press*, not
  when the key went down. Worst case ~1 frame (16.7 ms) of late-bias jitter on top of
  the input chain, partially masked by the fixed −5-tick compensation. The windows
  being beat-relative also means high-BPM charts are strictly harder in ms terms.

- **For the 2EZConfig backdating patch — the good case confirmed.** The judge consumes
  a scalar time, so feeding it the true edge time fully de-quantizes grading. And the
  original architecture already provides a perfect single choke point:

  **Patch design (2EZ.dll):** hook KEZPlayer vtable slot 22 (one dword at
  0x4887d0+0x58, or a 5-byte detour at 0x40f450).
  - The DLL's input poll thread (~1 ms) already sees every press edge (it emulates the
    I/O board). At each edge, sample the game's own clock `player->f7258` and push it
    into a FIFO. The player instance pointer is `ecx` on every slot-22 call — cache it
    from the previous call; before the first call no edges need timestamps because the
    judge is gated off while stopped. Sampling f7258 AT the edge sidesteps the
    ms→tick conversion entirely.
  - In the hook: if a recent press-edge sample is pending (sampled within the last
    couple of frames), return `max(sample - 5, 1)` (preserving the original's
    compensation); else fall through to the original behavior. This keeps the
    release/scratch paths (codes 1/3, which also call slot 22) unaffected until their
    semantics are read.
  - Ordering caveat: several presses in one frame pop the FIFO in processing order,
    which may mispair edges across lanes; the error is bounded by the intra-frame
    spread of those edges (a few ms), still far below the 16.7 ms it removes.

- **For the decomp:** include/ksongplayer.h's "eight-slot vtable @0x4887d0" note
  undercounts — the table has 35 slots (slot 22 @0x40f450, 25 @0x40f470, 26
  @0x40f480, 27 @0x40f490, 29 @0x40f610, 30 @0x40ff00, 31 @0x40ff50, 32 @0x40ffa0,
  33 @0x4100d0, 34 @0x40fd70, ...). Slot 22/@0x40f450 and the slot-34 body @0x40fd70
  are small and near-leaf — good next matching targets, and matching them would give
  the Judge cluster (0x426390/0x426500/0x426700/0x426ea0) its callee ground truth.

## The keysound path (button → sound)

The engine plays keysounds **on-hit as individual DirectSound voices**, not from a
software-mixed ring: the sample-buffer creation sites @0x40e9ea / @0x40ec9d build
DX7 `DSBUFFERDESC`s (dwSize 0x24) with flags `GETCURRENTPOSITION2 | GLOBALFOCUS |
CTRLPAN | CTRLVOLUME` (and a `CTRLFREQUENCY | LOCSOFTWARE` variant @0x40ea0a) — 
per-voice secondary buffers, DirectSound does the mixing. Master volume goes through
the WinMM mixer API (the "KSNDMixer(ksndInitMixer)" strings), not the buffers.
The press path triggers the lane's sample in the same frame update that judges it
(the `call [x+0x80]` @0x4309fd after judgePress); the auto/BGM notes are fired by the
slot-34 chart walk @0x40fdf2 when their tick passes. Latency chain on a press:
edge→detection (board latch / 2EZConfig ~1 ms poll), detection→frame processing
(0–16.7 ms, mean ~8 ms — the same frame quantization the judge sees), then
DirectSound `Play()` mixing latency: near-zero–10 ms with period hardware mixing
(the cab), ~30 ms XP kmixer software path, **~30–60 ms on Vista+ where DirectSound
is emulated over shared WASAPI** — which is exactly why 2EZConfig's fork replaces
`DirectSoundCreate` with the hypersonik shim (WASAPI-exclusive/ASIO, ~5–15 ms).
Audio latency does NOT affect judgement (the judge reads the tick clock, not the
sound) — it only affects feel.

## Tier-2 RE: the complete on-hit keysound chain (for the instant-keysound hook)

All of it lives in KEZPlayer (`this` = the object at director+0x14c, vtable 0x4887d0):

    slot 32 @0x40ffa0  __thiscall playLaneKeysound(int lane)   ; ret 4
      entry = this+0x10 + lane*0x30            ; per-lane SoundParam
      if (!entry->f0) return                    ; lane inactive
      cur = entry->f2c                          ; lane's current record index
      if (cur < 0 || cur >= chart->i94) return
      now  = slot22()                           ; f7258 - 5, the judge's clock
      next = EzChart_findNextInLane @0x411d90(lane, cur, 1)
      prev = cur itself if records[cur] is a type-1 note on this lane,
             else EzChart_findPrevInLane @0x411e00(lane, cur, 1)
      rec  = whichever of prev/next is nearer to `now` (midpoint compare)
      bind @0x40fbf0(entry, rec->sampleId, rec->b12)
           ; no-op if entry->fc == sampleId already; else voice =
           ; pickVoice @0x40f650(id): TWO VOICES PER SAMPLE, round-robin -
           ; pointer pairs at this+0x1210 + id*8, toggle ints at this+0x5210;
           ; releases old voice (slot 5), entry->f24 = voice, entry->fc = id
      setVolume @0x40f4a0(entry, rec->vol)      ; -> voice slot 8 (+0x20), DS millibels
      setPan    @0x40f550(entry, rec->pan)      ; -> voice slot 6 (+0x18), DS pan
      play      @0x40f520(entry)                ; voice slot 5 (stop) then slot 4 (play)

    Record28 fields used (stride 0x28 @chart+0x1d1c): +0x00 tick, +0x04 type
    (1 = note), +0x05 lane, +0x0e u16 sampleId, +0x10 vol, +0x11 pan, +0x12 flag.

  The voice objects are thin IDirectSoundBuffer wrappers (per-voice secondary
  buffers — the DSBUFFERDESC sites above); vol/pan arrive already in DS units
  (level−5000 mB, pan ±10000), so the tail is Stop/SetCurrentPosition/Play — safe
  from any thread (dsound COM locks internally). Note the pump thread's slot-34
  auto-BGM dispatcher @0x40fc50 ALREADY drives this same bind/vol/pan/play path
  concurrently with the main thread's slot-32 presses — slot 34 holds the KLock at
  this+0x7250 while doing it; slot 32 takes no lock.

### Hook design ("instant keysound", 2EZ.dll)

Two facts make this nearly trivial:
1. **Slot 32 is fully self-contained** — resolution AND playback in one `__thiscall`
   (this, lane). The DLL never reimplements anything: its poll thread just CALLS
   0x40ffa0 directly the moment it sees the edge.
2. **Slot 32 never consults the judge** — a press always sounds the lane's nearest
   keysound, hit or miss. Early-triggering is therefore behaviorally identical.

Design:
- **Vtable-patch slot 32** (one dword @0x4887d0+0x80). The replacement records
  `this` (giving the DLL the live KEZPlayer), and implements dedupe: swallow the
  game's own call for a lane the DLL early-triggered within the last ~30 ms
  (all game calls go through the slot; the DLL's own early calls go straight to
  0x40ffa0).
- **Button→lane map self-calibrates**: on each button edge, watch which lane the
  game's own slot-32 call passes ~one frame later; after one press per button the
  map is learned, and every subsequent press fires instantly. No per-mode tables.
- Take the KLock at this+0x7250 around the early call (same discipline as the
  pump's slot 34); check f7274 (playing) first.
- Residual imperfection: the early call samples the clock ~8 ms sooner, so a press
  landing within ~8 ms of the exact midpoint between two same-lane notes may pick
  the other neighbor's sample than the game would have — rare and benign.

Expected result: edge→sound = poll (~1 ms) + output (ASIO ~5–10 ms) ≈ **6–12 ms**,
vs ~40–80 ms stock on Win11 and ~10–40 ms on the cab — while judgement, visuals and
frame rate stay completely untouched. Scratch keysounds: the turntable path calls
the same trigger; the calibration approach extends to TT+/TT− edges. v1 scope is
the keys modes (EZ2DJ director); GF/Catch have analogous players at their own
offsets.

## The RELEASE path @0x4268c0 — a hold is judged TWICE

Read 2026-08-09, answering the first half of open item 2.

    int __thiscall Judge::judgeRelease(int time, int lane, int commit,
                                       EzNote *outRec, int outCursor[2])   ; ret 0x14

`lane` indexes the same 0x5c-stride per-lane block at `this+0x14`, and cl reuses
that argument's stack slot as the running `grade` (it is zeroed at the top),
which is why the frame appears to write to its own parameter. Gated on
**player slot 27** @0x40f490 (playing) exactly like the press path — not playing
returns 0 before touching the chart.

It then searches the lane's records through the chart reader's own vtable
(slots 11/12/13 at +0x2c/+0x30/+0x34, the search taking `(lane, time, kind, 1,
&rec, &cursor)` with kind 2 or 1), and tests **`rec.length > 6`** — the same
bias-of-6 hold test the port already implements — to decide whether a record is
a hold at all. What it grades is the tail:

    push rec.tick + rec.length - 1          ; the LAST tick of the hold
    push time                               ; slot-22 time, as for a press
    push cursor
    call 0x426500                           ; THE SAME grading core

So a release runs through the identical `dt` comparison against the identical
per-lane early/late window tables. If the core returns 0 (outside every window)
the grade is forced to **2** — the mash band — rather than being discarded.
`commit` non-zero writes the grade byte into the record (`rec.b[8]`) and calls
chart slot 18; `outRec` receives a 40-byte copy of the record (`rep movsl` of
ten dwords, which independently confirms the 0x28 stride) and `outCursor` two
dwords.

### And the caller commits both, through one function

`@0x430970`'s two arms, which is what makes this a scoring fact rather than a
detail:

    event 2 (press):    t = player->slot22(lane, 1, 0, &out)
                        g = judge->judgePress(t, lane, ...)      @0x426700
                        player->playLaneKeysound(lane)           ; slot 32, always
                        if (g) commit(&rec, g, 1)                @0x42fd90  <- flag 1

    event 1 (release):  t = player->slot22(lane, g, 0, &out)
                        g = judge->judgeRelease(t, lane, ...)    @0x4268c0
                        if (g == 2) player->slot33(lane)         ; @0x84
                        if (g) commit(&rec, g, 0)                @0x42fd90  <- flag 0

**A hold therefore produces TWO judged events** — head and tail — both graded
by the same core and both committed through the same function, distinguished
only by that trailing flag. A port that judges the head and credits the rest is
not playing the same game.

### The commit @0x42fd90 — and a hold is graded twice but TALLIED ONCE

Read 2026-08-09. `int __thiscall commit(int lane, int *cursor, int grade,
int isPress)` (`ret 0x10`; `lane` in the first stack slot, not ecx). It branches
on `isPress` immediately — `cmp dword [esp+0x80],1; jne 0x43009f` — and a
second, whole-hog variant is selected by the global at **0x1b2ebc8 == 7**
(press 0x4302a9 / release 0x430391), which mirrors the pair below.

It keeps a **per-lane hold cursor**, a pair of dwords at
`this + lane*8 + 0x3e4` / `+0x3e8`. The setup @0x4305a0 fills **18** such pairs
with -1 (`mov ecx,0x12` over an 8-byte stride), and -1 means "no hold in
progress on this lane".

    press (isPress = 1), at 0x42fe3a / 0x4302a9:
        if (rec.type == 1 && rec.length > 6 && grade >= panel->f38) {
            chart->slot18(...)                  ; @0x48
            holdCursor[lane] = *cursor          ; remember the hold
            doEffects = 0                       ; <- SUPPRESSED
        }

    release (isPress = 0), at 0x43009f / 0x430391:
        if (holdCursor[lane].second != -1) {
            if (grade >= panel->f38) {
                chart->slot17(&holdCursor[lane], &rec)          ; @0x44
                grade = (grade + rec_byte + 1) / 2              ; AVERAGED, rounding up
            }
            holdCursor[lane] = { -1, -1 };
            doEffects = 0;                       ; <- SUPPRESSED
        }

`doEffects` is a local seeded to 1 at the top of the function (`mov dword
[esp-0x5c], 1`), and both arms test it before doing anything else:
`cmp doEffects, 0; je <epilogue>`. Everything downstream of that test — the
fast/slow bookkeeping at 0x37dcdc0, the judgement sink **@0x42e620**
(`(grade, 1, flag)`, the common "a judgement happened" call reached from twelve
places) and the panel call @0x42d370 — is skipped.

**So neither end of a tracked hold emits the ordinary judgement.** A hold is
*graded* twice, by the same core against the same windows, and the two grades
are averaged into one; it is not tallied twice. The suppression is symmetric and
deliberate: the head registers the hold and stays quiet, the tail closes it and
stays quiet.

This CORRECTS the reading in the section above. "A port that judges the head and
credits the rest is not playing the same game" is right about the *grading* and
wrong about the *counting*: a port must **not** tally a release as an extra
note. A head-only note total — which is what `ez2_score_max` already assumes —
is correct.

Resolved since: the emitter is the **hold sustain pump @0x42f880** (its own
section below), which pays a hold out one tick at a time while it is held. The
candidate named here, @0x426d10, is a red herring — it is already matched, as
`Obj426CA0::getF0` in src/accessors.cpp, a bounds-checked array getter. What
remains genuinely unresolved is only where the *averaged* grade goes: it lands
in a local (`[esp-0x54]`) that the suppressed path does not consume.

## The scratch path @0x42f880 is not a third judge entry

It is a per-lane turntable state machine — arrays at `+0x58`, `+0xa0`, `+0xe8`
and `+0x178` all indexed `[lane*4]` — that reads slot-22 time and calls
@0x422dd0 for a half-range value, rather than a `judgePress` sibling. It is
~0x340 bytes and reaches into the same commit path further down. Left open;
the second half of open item 2.

## The record dispatcher, and what a BPM change actually does

The pump's chart walk @0x40fdf2 hands each record whose tick has passed to
**@0x40fc50**, which switches on the RUNTIME record type (`rec+4`, after
`NoteConvert` @0x410ef0 — matched, src/note.cpp — has remapped the on-disk one):
`movzx eax,[esi+4]; add eax,-1; cmp eax,7; ja default; jmp [0x40fcfc+eax*4]`.

    disk type (NoteConvert)   runtime   arm         what it does
    1  note                →  1        @0x40fc6b   bind + setVolume + setPan + play
    2  volume              →  4        @0x40fcaf   entry->f18 = rec->b[0xc]; entry->f10 = -1
    3  BPM                 →  5        @0x40fcc6   push rec->f0c; call setSpeed @0x40f5c0
    5  mark                →  6        @0x40fcaa   nothing
    6                      →  7        @0x40fcd6   float rec->f0c -> global 0x1b2e708
    7                      →  8        @0x40fce8   Log(@0x4887c8)
    4  beats               →  9        (out of range) nothing
    -                      →  2, 3     @0x40fcaa   unreachable — NoteConvert emits neither

**So a mid-song BPM record does re-drive `setSpeed`, and that is the only way it
is ever called: `setSpeed` @0x40f5c0 has exactly ONE reference in the whole
binary, the `call` at 0x40fccc in this arm.** The step rate is recomputed at the
moment the pump walks past the record, so the tick's millisecond size tracks the
current tempo and the judgement windows stay beat-relative for the whole chart.

## Open items

1. ~~Confirm `f8c` = initial BPM~~ — done: EzChart::reset seeds f8c=120.0/i88=192,
   and the director's ×4 rescale + `m_nTPB = 192` log pin the unit (1/192 beat).
   ~~Confirm mid-song BPM-change records re-drive setSpeed~~ — **done**, see the
   dispatcher section above: disk type 3 → runtime type 5 → `setSpeed`, its only
   caller. Also verified library-wide that the rate really is 192 ticks per
   measure: **12,359 of 12,359 parseable charts**, no other value, so the
   1/192-beat tick is data and not an assumption.
2. ~~Read the event-code-1 path @0x4268c0~~ and ~~@0x43009f, the release arm of
   the commit~~ — both **done**. A hold is *graded* twice (tail against
   `tick + length - 1`, same core, same windows) and *tallied once*, the two
   grades averaged; neither end emits the ordinary judgement sink. The port's
   head-only note total is therefore correct and releases must NOT be counted.
   What is left of this item: `Judge::m426d10` @0x426d10, which is how a
   completed hold's judgement presumably does get emitted, and the scratch path
   @0x42f880 — a per-lane turntable state machine rather than a third judge
   entry. Neither blocks the port.
3. ~~Map grade indices 5..2 onto FUN_00472fc0's 1..5 display/gauge indices.~~ —
   **done**, and there was no remapping to do: @0x472fc0 ends with
   `add dword [this + 0x1c4 + grade*4], 1`, so the judge's own grade IS the
   counter index. The block at +0x1c8..+0x1d8 is grades 1..5 = MISS, FAIL,
   GOOD, COOL, KOOL — the worst-first ordering `wip/scorekeeper.cpp` derived,
   now read directly. The gauge rate is indexed the same way, at
   `director + 0x314 + grade*4`, scaled by the difficulty table @0x49e3c8 and
   accumulated into the float at +0x1ec (ceiling @0x48dbfc = 100.0).
   The consequence for the port was a real bug: **the band between GOOD and
   MISS is grade 2 = FAIL, not MISS**, so it drains gauge_fail (-4.8) rather
   than gauge_miss (-1.8) and does NOT break the combo — @0x472fc0 resets the
   combo at +0x1dc only for grade 1. Grade 1 is only ever a note nobody
   pressed. Also visible there: score at +0x1e8, combo at +0x1dc, max combo at
   +0x1e4 (copied to +0x1e0 on the way out), and a SECOND scoring mode selected
   by the global at 0x1b2ebac — `score += combo / 100` instead of the log10
   formula.
4. ~~The two `-1` sentinel slots in the window tables~~ — **confirmed** by
   reconstructing the grading core @0x426500 (parked at 84.11% in
   wip/judge-grade.cpp, blocked on one register): the walk starts at the
   tightest window with grade 5 and steps DOWN, testing `|dt| <= window`. No
   `|dt|` is ever <= -1, so grades 1 and 0 cannot be produced. The defaults
   @0x48ff98 are {-1, -1, 24, 12, 4, 2} in memory order, tightest LAST.

## @0x42e620 — the judgement sink, decoded (NOT yet matched)

Read 2026-08-09. This is the function every judged event ends at: the commit
@0x42fd90 calls it for a note, the sustain pump @0x42f880 calls it for each
hold tick, and ten other lane handlers call it besides. It is
`void __thiscall PlayerSlot::commit(int grade, int count, int flag)` (`ret
0xc`), 357 instructions, 76 branches, no EH prologue, and — unusually for
something this size — only **four** direct call targets, three of them already
matched (`countListA` @0x418310, `findListA` @0x418e30, @0x420570) plus the
folded empty `Log` @0x402880, and **no indirect calls at all**. So nothing
about it is blocked on stubs; it is simply big and branchy.

### The per-grade counter, confirmed on a second class

Both exits end with

    add dword [this + 0x1c4 + grade*4], 1

which is the same grade-indexed counter block @0x472fc0 uses. Two independent
classes agreeing settles the layout beyond the earlier single reading.

### The score here is NOT rank()'s logarithm

    score(+0x1e4) += (int)(perGrade[grade] * comboMul[min(combo, 1499)])

* `perGrade` is a six-entry array built on the stack at the top of the
  function, `{0, 0, 0, 40, 100, 170}` or `{0, 0, 0, 41, 150, 300}` — the pair
  is chosen by the mode test at 0x42e627 (`g_1b2eb6c`, plus a check on
  `g_1b2e548` being 6 or 7). So grades 0..2 score nothing, and GOOD/COOL/KOOL
  are worth 40/100/170 or 41/150/300.
* `comboMul` is a **1,500-entry float table @0x4a9160**, rising 1.0, 1.0,
  1.1003, 1.159, ... 1.6652 at 100, 1.7663 at 200, 1.9999 at 1000, 2.0586 at
  1499. The combo index is clamped to 1499.

### The rest of the block

    +0x1dc  combo          +0x1e0  max combo (updated by compare-and-store)
    +0x1e4  score          +0x1f4  gauge, float
    +0x21c  a SECOND float gauge

The gauge at +0x1f4 takes its per-grade delta from `director + 0x308 +
grade*4` and is clamped to 0 below and to the ceiling @0x48dbfc (100.0) above;
the second at +0x21c takes `director + 0x32c + grade*4` and clamps against
@0x491264. Two-player play is handled by `findListA(0)` / `findListA(1)` and
`countListA()`, with a large duplicated block per player.

### THE OPEN QUESTION, and it matters to the port

This is a **different scoring model** from `ScoreKeeper::rank` @0x472b40 — the
logarithmic `250 + 50*log10(combo)` accumulation the port implements and
`port/oracle` verifies. Both write "the score" field of the same block layout;
the two classes' headers differ by exactly 4 bytes (ScoreKeeper has combo at
+0x1dc, max combo +0x1e4, score +0x1e8; PlayerSlot has combo +0x1dc, max combo
+0x1e0, score +0x1e4).

### Settled: `rank()` is EZ2CATCH's, and the keys modes have their own

`ScoreKeeper::commit` @0x472d10 — the only thing that calls `rank()` @0x472b40 —
has exactly one caller itself: **`CatchMainGameDirector::m46c7e0`** @0x46c7e0.
Every caller of `PlayerSlot::commit` @0x42e620, by contrast, resolves into the
EZ2DJ player range. So the logarithmic model belongs to Catch, and the keys
modes — every mode the port targets, including the declared first playable one —
use the other.

**The keys modes' own rank function is @0x42e190.** Same shape as Catch's: two
ladders picked by `g_1b2eb6c`, a hit percentage `(f18 + f14) / count * 100` and
a score percentage. Two things fall out of it:

* **Its ladders are IDENTICAL to Catch's** — hit 95/90/80/70/60/50 for grades
  6..1, score 100/98/95/93/90/85/80/70/60/50 for grades 10..1. So the port's
  `ez2_score_rank` and `ez2_score_rank_hits` are correct for both, which is
  worth knowing before anyone "fixes" them.
* **Its maximum is flat: `count * 300`.** The arithmetic is unambiguous —
  `shl eax,4; sub eax,edx` gives count*15, then `lea (eax+eax*4); add; add`
  gives count*300 — and it is divided into the score at +0x1e4. That is NOT
  the logarithmic sum `rank()` divides by, and it is what the port currently
  implements as `ez2_score_max`.

### Settled: the per-note value is FLAT, by tier

The branch at 0x42e793 is `cmp [0x1b2eb6c], ebx; je 0x42e7c8` — it takes the
je when the global is ZERO, which is normal mode, and 0x42e7c8 is

    mov edx, [esp + 0x10 + grade*4]
    add [esi + 0x1e4], edx

with **no combo multiplier at all**. I read that branch backwards on the first
pass; the multiplier at 0x42e79b is the ALTERNATE mode's path.

The two stack tables and their modes therefore pair up as:

    normal    (0x1b2eb6c == 0)   GOOD 41,  COOL 150, KOOL 300   flat
    alternate (0x1b2eb6c != 0)   GOOD 40,  COOL 100, KOOL 170   x comboMul[]

and the check falls out immediately: a full combo of N KOOLs in normal mode
scores N * 300 against a maximum of N * 300 — **exactly 100.00%**, on any
chart length. That is what confirms the reading, and it is why the alternate
model's 96-110% could not have been right.

(The normal-mode branch has one sub-case: if `g_1b2e548` is 6 or 7 and
`g_1b2ea20` is non-zero, it takes the {40,100,170} table but still scores it
flat.)

### Done in the port

`ez2/score.c` now implements the keys modes: `ez2_score_value(j)` returns
300/150/41/0/0 and `ez2_score_max(notes)` is `notes * 300`. Catch's model is
kept beside it as `ez2_catch_note_value`/`ez2_catch_score_max`, because
ez2catch is one of the thirteen modes `ez2/mode.c` maps and because
`port/oracle` checks it against the original's own compiled code — the oracle's
rank arm now divides by Catch's maximum explicitly, since the ladders are
shared but the maximum is not.

The invariant survives the change, which is the point: an autoplayed 1,364-note
chart scores 409,200 of 409,200 and rates 100.00% S4.

The score now depends on the TIER rather than the combo, which inverts a test
this port used to assert — a GOOD-only full combo is worth 41/300 of a
KOOL-only one, not the same.
