#ifndef MODEINPUT_H
#define MODEINPUT_H

/* THE PER-MODE INPUT HANDLERS.

   The in-game object has one of these per mode, and they are the arms of a
   single switch inside the tick @0x446440:

       mov eax, [ebp+0x78]
       cmp eax, 0xc
       ja  <done>
       jmp [eax*4 + 0x448e00]      <- a 13-entry table

   which maps mode index to handler:

       0  0x43ff00      7  0x43cf30
       1  0x43b8d0      8  0x43e8d0
       2  0x43c2e0      9  0x43f390
       3  0x442770     10  0x43a150
       4  0x440af0     11  0x43ad10
       5  0x441540     12  0x441fa0
       6  0x43dc00

   Thirteen modes, the same count as the mode runners and as the
   Del*Ranking switches readSettings @0x415a40 reads. Each handler is 2.5-7.7KB
   and they fall into two shapes by opcode sequence: 0x43a150 / 0x43ad10 /
   0x43b8d0 / 0x43c2e0 / 0x43ff00 / 0x440af0 / 0x441540 are variations of one
   another (0.4-0.7 similar), and 0x43cf30 / 0x43dc00 / 0x43e8d0 / 0x43f390 are
   a second family (0.4-0.5 among themselves, ~0.12 against the first).

   What they all do: read ~30 slot states through Obj40C180::getState, drive
   ~12 output channels through setOutput, and time held keys through heldFor.
   They share five helpers, which were the enabling work for all of them -
   and as of 2026-08-17 THE ENABLING WORK IS DONE:

       0x402910  NoteTable::setCell            - matched, src/accessors.cpp
       0x40e590  EzSubsystem::loadSample       - reconstructed, wip/loadsample.cpp
       0x4332c0  SongSelectDirector::m4332c0   - matched, src/songselect.cpp
       0x433cf0  InGameSongSelect::pickDistinct- matched, src/ingame.cpp
       0x44b2c0  updateStartLamps              - matched, src/lamps.cpp

   Four matched and the fifth reconstructed, so nothing blocks the handlers
   themselves now.

   STATUS 2026-08-18 (src/songselectinput.cpp; the handlers are
   SongSelectDirector methods, every offset lands in that class):
       0x43b8d0  inputRubyMix   100% (byte-exact)
       0x43ff00  input5KeyMix    88.39%  parked - constant-register wall
       0x43a150  inputEZ2Catch   88.56%  parked - same wall
   Read wip/modehandler-43b8d0.md before writing another one: the family's
   remaining defect is which callee-saved register cl gives the constants
   1 / 5 / 0, and the original's siblings each get a DIFFERENT assignment
   from the same source text. The shared callee 0x434b20 is
   SongSelectDirector::setOptionLamps (matched), and updateStartLamps
   @0x44b2c0 is a member of the same class (every caller loads ecx).

   WHAT THEY COST, measured 2026-08-17. All eleven are 650-880 instructions
   with ~0.15 branches per instruction and NO EH prologue:

       0x43b8d0  650 insns, 100 br, 2565 bytes   <- the smallest
       0x440af0  676, 106, 2626
       0x441540  682, 108, 2654
       0x43e8d0  723, 101, 2742
       0x43a150  770, 117, 3003
       0x43ad10  764, 117, 2997
       0x43ff00  774, 120, 3054
       0x43f390  779, 108, 2919
       0x43c2e0  795, 122, 3138
       0x43cf30  871, 126, 3268
       0x43dc00  877, 126, 3271

   Branch-dense, so these are the opposite of the table-of-calls profile that
   lands first try - CLAUDE.md's rule puts them squarely in "inline, serially,
   one to four iterations each". Budget a tick or more per handler.

   THE PAYOFF IS THE SECOND ONE ONWARD, NOT THE FIRST. The seven-strong family
   is 0.4-0.7 self-similar and the four-strong one 0.4-0.5, so the first
   handler in each family is a reconstruction and the rest should be template
   edits - the pattern that produced five of this session's matches. Start
   with 0x43b8d0 (smallest of the seven-family) and 0x43e8d0 (smallest of the
   four-family); together they unlock roughly 20 KB. */

#endif /* MODEINPUT_H */
