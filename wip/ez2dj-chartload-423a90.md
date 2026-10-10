# EZ2DJMainGameDirector::m423a90 @0x423a90 — the EZ2DJ chart loader

**Not written yet.** This is the delta map against its twin, which IS written:
`CatchMainGameDirector::m46dba0` @0x46dba0 in `src/catchload.cpp` (87.14%).
The two share 73.9% of their instruction sequence, so this one is a **copy of
that file plus the edits below**, not a reconstruction. Read `src/catchload.cpp`
first — its header explains the registry calls, the volatile `step` and the
frame.

1079 bytes / 312 instructions against the twin's 756 / 218; the extra ~94
instructions are the four new blocks in the second table.

## Warning before starting

The original holds **0 in ebx and 1 in ebp** for the whole function
(`xor ebx,ebx`, `mov ebp,1`, then `cmp [g],ebx` / `push ebp` / `add ebx,ebp`
throughout). That is the constant-register assignment wall in CLAUDE.md.
Expect to land around 80% with every instruction present and a handful of
`push imm` against `push reg`; do not spend a session chasing it.

The frame is 0x26c on both, but this one pushes FOUR registers (ebx, ebp,
esi, edi) against the twin's two, so every `[esp+X]` in the shared body is
8 higher.

## Field and callee mapping

| twin (Catch) | this (EZ2DJ) | what |
| --- | --- | --- |
| +0x40 | +0x24 | the pump (`Obj414750::start(0, 0)`) |
| +0x164 | +0x148 | `sink` — the chart the registry returns |
| +0x168 | +0x14c | `notify` — the KEZPlayer bound to it |
| +0x16c | +0x154 | `judge` |
| +0x334 | +0x34c | the measure length (`hdr[2] * 4`) |
| +0xca4 | +0xd14 | the fourth header value |
| +0x348 | +0x37c | the "too many players" flag |
| +0x350 | +0x384 | the lane count |
| +0x35c / +0x518 | +0x398 / +0x55c | the two lane arrays, stride 0x10 |
| +0x38 | +0x1c | the load-failed flag |
| +0x2f8 | +0x2ec | the value published at the end |
| 0x46c360 | 0x4208f0 | the song-name getter |
| 0x4183e0 (countListB) | 0x418310 (countListA) | the player count |
| 0x46d8d0 | 0x423790 | loadGds |

Also: this one's THIRD header read passes 4 where the twin passes 0x10 —
which is the evidence that the argument is not a byte count (see the twin's
header for the rest of that argument).

## The four blocks that are new here

1. **A prologue gate**, before the path is built:
   `if (g_1b2e548 == 5 && g_elemCount >= 2 && countListA() <= 1)`
   then `m42e2e0(m418e30(m418340()))` — a three-call chain on the result.

2. **A three-way name build** instead of the twin's two-way. It formats with
   0x48fa08 against `g_1b2e680` and the player number, logs the result
   (0x48f9f4 / 0x48f9ec), and when the mix style `g_1b2ebc8` is 6 it takes a
   second arm. It then compares the built name against `g_songShortName` with
   0x47f945 and sets `g_1b2eac8` to 1 (logging 0x48f9c4) or 0.

3. **A log of the measure length** right where the twin just stores it:
   `Log(0x48f9b8, step)` before the store to +0x34c.

4. **A post-loop effector pass**, after the lane loop and before
   `createJudge`:

       h = m418e30(1);
       if (h != 0 && g_1b2e548 != 7 && g_1b2e548 != 3 && g_1b2ebc8 == 2)
       {
           m421600(h, 7, 8, 0, fd14);
           m421600(h, 1, 5, 0, fd14);
           m421600(h, 2, 4, 0, fd14);
       }
       sink->commit();          /* slot 23 -> +0x5c */

   and a trailing `if (m418e30(0)) { ... +0x2ec ... }` after loadGds.

The failure path's string is 0x48f980 here, not the twin's 0x49daf0.
