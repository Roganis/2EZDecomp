/* RankingDirector::m45aaf0 @0x45aaf0 - IN-PROGRESS wip transcription.
   3,907 bytes, 0x45aaf0..0x45ba36, thiscall + two stack args (ret 8).
   34 calls: 28 getState, findListA x3, getListAF8 x2, and one call to
   the (also unannotated) 0x4577b0 - stubbed as m4577b0.

   THE MAP (by original label):
     head       - args (a, b); b==0 picks -1/+1 (the step direction
                  sign trick: lea -0x1(%eax,%eax,1) off sete).
                  Per-slot gates on the giant per-player blocks:
                  +0x85dfc[b] (the 99-cap), +0x109dd0[b], +0x85df8
                  (a byte flag), +0x109b48[b].
     45ab54..   - the key ladders: each arm getState(code), then the
                  click sample at this+0x109d8c: ->stop() (+0x14) and
                  ->play(0) (+0x10); cursor steps by the +/-1 from the
                  head. Six ladder blocks (45ab9f, 45ac0a, 45ac76,
                  45ad4e, 45add6) with getListAF8 gates on two.
     45ae1b..   - the call-free middle: the character-wheel arithmetic
                  (the cursor wraps over the glyph set, the name buffer
                  writes) - to be walked with the slot tracker.
     45af80..   - per-side start handlers: findListA(0)/getState(4)
                  and findListA(1)/getState(5), the confirm sample at
                  +0x109d94, then the commit call into m4577b0
                  (0x4577b0) and the tail state writes.

   DERIVED SO FAR (2026-08-29 tick 2):
   - step = (b == 0) ? +1 : -1 (sete + lea).
   - The entry gates: active when f85dfc[a] < 99 && f109dd0[a] == 0 &&
     b85df8 == 1, or f109b48[a] < 99 && f109dd0[a] == 0 && b85df8 == 0.
   - Key tables in .data: 0x4ae488 = {0xf, 0x17} (down per side),
     0x4ae490 = {0x10, 0x18} (up), and the five-entry-per-side group at
     0x4ae498 + b*20 for backspace/confirm rows; the GLYPH TABLE at
     0x4ae454, 48 chars, wheel index 0..0x31 (0x30/0x31 the two
     specials, 0x31 resetting to 0 on backspace).
   - Dial: big->accum[b] against +/-5; the wheel row f109da8[a] steps
     by `step` (gated to 0x30/0x31 when the position f109db0[a] == 8),
     the down arm at 45adee steps -step.
   - The wheel ease at 45ae1b: idx = (row + 50) % 50 stored back;
     target = idx * 100; f109dc0[a] eases a third of the gap toward it
     around the 5000 ring (thresholds +/-2500, the -4999..-5001 seam
     snap, +/-1 snap, wrap into [0, 5000)) - the m425ea0 shape.
   - Click samples: +0x109d8c (wheel), +0x109d90 (type/backspace),
     +0x109d94 (confirm); each ->stop() then ->play(0).
   - The name tables: 50-byte records; the visible one at this+0x20cc
     keyed by f109b48[a], the second at this+0x85e04 keyed by
     f85dfc[a], written only when b85df8 != 0. Position f109db0[a],
     capped 8, 0x20 (space) written on backspace, g_glyphs[row] on
     type; typing past position 7 resets the wheel row to 0.
   - Confirm half (45af80): one-side modes findListA(0)+getState(4) /
     findListA(1)+getState(5) latch f109dd0[a] = 1 with the confirm
     click; modes 4/5/8/9 at 45b2de (unread); the commit tail from
     45b57d (unread - m4577b0 lives there).

   Transcription continues: 45b2de (two-side confirm), 45b57d..45ba36
   (the commit tail). */

#if 0
// FUNCTION: EZ2AC_UNPACKED 0x0045aaf0
void RankingDirector::m45aaf0(int a, int b)
{
}
#endif
