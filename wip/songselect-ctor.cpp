/* SongSelectDirector::SongSelectDirector @0x435390 - DECODED TRANSCRIPTION IN
   PROGRESS (loop work, 2026-08-13). NOT yet a match candidate; this file is
   the faithful reading the match will be built from, section by section.

   Sources:
     - Ghidra decompile: scratchpad/ssdir-keep.log (FUN_00435390, ~1500 lines)
     - member map:       scratchpad/ssdir-members.txt (the dtor's 40 players)
     - class model:      include/songselect.h (dtor-verified, 100% matched)
     - sizeof:           0x20eb18 (the pooled factory @0x4463e0's push)

   Conventions used while transcribing:
     - Ghidra's `param_1 + N` on an undefined4* is a DWORD index: byte offset
       is N*4. Offsets below are BYTE offsets, matching include/songselect.h.
     - `FUN_00402860()` with a lost `this` is the Obj402860 clip-player ctor;
       the 40 targets and their EH states are the dtor's map in reverse.
     - `FUN_00408d00` is KGraphics::loadTexture(path, &member, key).
     - `FUN_00418e30` is the per-player accessor the entry-name reset below
       calls once per touched field (the original really does re-call it).

   WHAT THE CTOR IS, from the first pass over the decompile: not a flat
   texture table. In order:
     1. EH head: base ctor @0x401690, vptr 0x49193c, the 40 player ctors
        (states 0..0x27).
     2. A settings read (@0x413eb0) and a block of field zeroing.
     3. When g_elemCount == 0: reset the per-player entry-name buffers -
        eight bytes at player+0x220..0x227 to ' ', score at +0x228/+0x22c
        cleared - for one or both players by credit count (@0x418310 x2,
        @0x418340, accessor @0x418e30 re-called PER FIELD).
     4. THE CHANNEL SYSTEM: parse `system\%s\Stage.ini` (and radiomix's two
        variants) into channel records of 0x374 bytes - list A at +0x238,
        list B at +0x5e3c8 - with `ALLSONG`, `HID%d`, `HID-HD%d`,
        `channel_secret%d` keys handled distinctly. This is where song
        select's channel list comes from.
     5. Song-list fill per channel out of the song table / stage lists.
     6. The texture and clip loads (songname/discsmall/SongRank fonts,
        CV2 list bars, DiscCur.str etc.) into the 40 players and texture
        members.
     7. Trailing field defaults.

   The port consumes section 4-5's semantics (channels) and section 6's
   layout (which asset lands on which screen element) long before the byte
   match closes. */

#include "songselect.h"
#include "ez2acapp.h"
#include "bigbase.h"

/* ---- section 1: the head -------------------------------------------------
   (transcribed; offsets from scratchpad/ssdir-members.txt, declaration order
   = the dtor's states 0..0x27 reversed)

   m_bc674 m_bc890 m_bcaac m_bccc8 m_bcee4 m_bd100        // bank A, 0x21c
   m_bd3c0 m_bd5dc m_bd7f8 m_bda14 m_bdc30 m_bde4c        //   (gap 0xa4
   m_be068 m_be284 m_be4a0 m_be6bc m_be8d8 m_beaf4        //   after m_bd100)
   m_bed10                                                //   (gap 0x130)
   m_bf05c m_bf278 m_bf494 m_bf6b0 m_bf8cc m_bfae8        //
   m_c00ac                                                //   (gap 0x3a8)
   m_c05a8 m_c07c4                                        //   (gap 0x2e0)
   m_c09e8                                                //   (gap 0x8)
   m_c0c04                                                //
   m_20c6d8 m_20c8f4 m_20cb10 m_20cd2c m_20cf48 m_20d164  // bank B
   m_20d380 m_20d59c m_20d7b8                             //
   m_20e898                                               //   (gap 0xec4)
*/

/* ---- section 2/3: zero block and the entry-name reset --------------------
   Transcription of decompile lines ~185-300. Byte offsets:

     f_bfde8 = 0; f_bfde4 = 0;            // param_1[0x2ff7a/0x2ff79]
     f_20c6d4 = 0;                        // param_1[0x831b5] - just before bank B
     f_6c = 0;                            // param_1[0x1b]
     f_20eb0c = f_20eb10 = f_20eb14 = 0;  // param_1[0x83ac3..5] - the tail
     memset(&f_20eab4, 0, 0x50);          // param_1[0x83aad], 0x14 dwords
     f_20eb04 = f_20eb08 = 0;             // param_1[0x83ac1/2]
     f_c0e30..f_c0e3c = 0 (4);            // param_1[0x3038c..f]
     f_c0e28 = f_c0e2c = 0;               // param_1[0x3038a/b]
     f_c04b4 = 0;                         // param_1[0x3012d]
     f_bfdc0 = f_bfdc4 = f_bfdc8 = 0;     // param_1[0x2ff70..2]
     f_bfdd4 = g_1b2efd4;                 // param_1[0x2ff75]

   Then, when g_elemCount == 0 (first player of the credit):
     if (credits @0x418310 < 2)  reset ONE player's entry name (the one
                                 @0x418340 names);
     else                        reset BOTH (i = 0, 1).
   Each reset: p->f22c = 0; if (p->f228 > 0) { p->f228 = 0;
   p->name[0..7] = ' '; } with the accessor @0x418e30 RE-CALLED for every
   single field - the original's own shape, worth keeping for the match. */

/* ---- section 4: the channel system, first half (read 2026-08-13) ---------
   Applies to the NON-Radio modes (mode index not 6/7/8/9; the Radio modes
   take the radiomix Stage.ini variants instead, next pass). The file is
   `system\%s\Stage.ini` under the ordinary cipher, loaded through the
   profile machinery (@0x413dc0 open, @0x413fe0 load = the parked
   wip/kprofile-load.cpp, @0x413f70 getString) into a 0x2000-byte value
   buffer.

   4a. THE HIDDEN-SONG TABLE - keys `HID-HD%d`, d = 0 upward, stopping when
   a missing key follows count 0x1d or the table fills:

     for (d = 0; table count < 100; d++) {
         sprintf(key, "HID-HD%d", d);
         if (!getString(key, buf)) { if (d + 1 > 0x1d) break; continue; }
         for each ';'-separated name in buf:
             rec = &m_hidden[count];        // 36-byte records at +0x20da80
             strcpy(rec->name, name);       //   char name[32] at +0
             rec->group = d;                //   int at +0x20 - the HD tier
             count++;                       // count at +0x20e890 (dword)
     }

   4b. THE FLATTENED SONG LIST - key `ALLSONG`, one ';'-separated value:

     getString("ALLSONG", buf);
     for each name:
         strcpy(m_songs[f_80].name, name);  // 0x374-byte records at +0x5e3c8
         f_80++;                            // count at +0x80

   The song records' other 0x354 bytes are filled by the later sections
   (levels, per-tier availability, the per-song ranking peek). The CHANNEL
   list proper is the sibling bank at +0x238, same 0x374 stride, count at
   +0x7c, filled from the channel keys (next pass: `channel_secret%d`, the
   radiomix variants, and the per-channel song membership).

   PORT NOTE: ez2/stageini.c already parses stage.ini channels for the
   COURSE modes; this section is the same file feeding SONG SELECT's
   channel wheel in the normal modes - `ALLSONG` duplicating song.bin is
   what the port's reader already observed ("a normal mode's holds a single
   ALLSONG key"). The hidden table (HID-HD%d) is NEW semantics no port code
   models yet. */

/* ---- section 4c: the caps, the mirror, and the UNLOCKED hidden songs -----

   The flattened list caps at 0x1b4 = 436 songs - exactly the library the
   sweeps counted - and the count is MIRRORED to the global g_1b2ebf4 after
   every append.

   `HID%d` then APPENDS to the wheel, gated per index on the UNLOCK FLAGS:
   the dword array at g_1b2e848 (bounds-walked to 0x1b2e89f, ~22 entries;
   these are what the mode-select hidden commands set - hiddencmd.ssf's
   family). For each set flag whose HID%d key exists, its ';'-separated
   names append to the song records at +0x5e3c8, count and mirror updated.
   So a hidden song reaches the wheel only when its flag was raised this
   credit. State returns to 0x28 and the profile closes (@0x413e20).

   ---- section 4d: the Radio-select branch, and a HARD-OPTION cross-link --

   The Radio modes (6/7/8/9) take a different path with its own surprises:
   in mode 6, three CARRY-OVER TOGGLES from mode select (globals g_37dcf0c/
   g_37dcf14/g_37dcf18, each guarded by a once-flag in the director at
   +0x20eb0c/+0x20eb10/+0x20eb14) are applied here, with a click sound via
   the object at +0xbfd28 (slots 0x14/0x10):

     - g_37dcf0c toggles g_1b2e89c  <- THE GAUGE-DOUBLING HALF OF THE HARD
       OPTION (the fourth scoring state's parse-time flag) is flipped from
       the SELECT ctor, not only from @0x44d802;
     - g_37dcf14 toggles g_1b2e568 (with a log line each way);
     - g_37dcf18 toggles g_1b2e58c.

   Then the Radio channel file is `system\radiomix\Stage.ini`, or
   `Stage_0.ini` when g_1b2e7e0 is set - a variant selector nothing in the
   port knows about yet. */

/* ---- section 4e: the Radio channel parse (read 2026-08-13) ---------------

   Modes 8/9 and 7 first apply their carry-over toggles through the helper
   @0x434f70 (dumped in ssdir-keep.log, to transcribe), then all Radio
   modes load their channel file (4d chose which) under EH state 0x2a:

     - the channel-list key's value (';'-separated - `channel_normal`'s
       shape, the same key ez2/stageini.c reads) fills CHANNEL records at
       +0x238, stride 0x374, count at +0x7c. For Radio, a channel IS a
       course.

     - `channel_secret%d`, d = 0..2: THREE SECRET CHANNEL BANKS, each
       gated on its own unlock flag at +0x20eb0c + d*4 (the once-flags
       section 2 zeroes and 4d's toggles feed). When unlocked, the bank's
       courses APPEND to the channel list; each bank's own count persists
       at +0x20da3c + d*4.

     - then, for every channel, ITS NAME IS READ BACK AS A KEY and the
       value's ';'-separated stage list fills the per-channel song tables
       (bases +0xc0e44 and +0xdb104, walked in step), with the channel
       name "random" (stricmp) special-cased - the 51-90 entry POOLS the
       course reader already knows, handled at select time here.

   The count mirror g_1b2ebf4 tracks the CHANNEL count on this path (it
   tracked the SONG count on the normal path) - one global, two meanings,
   keyed by mode family. */

/* ---- section 4f: EIGHT special channels, five pool tables ----------------

   The per-channel fill stricmp's the channel NAME against eight specials
   before treating it as an ordinary stage list:

     "random"   -> pool table at +0xdb294,  count +0x20bd94
     "random2"  -> pool table at +0xf48d4,  count +0x20bd98
     "random3"  -> pool table at +0x10df14, count +0x20bd9c
     "7znpfg"   -> pool table at +0x127554, count +0x20bda0
     "random4"  -> pool table at +0x140b94, count +0x20bda4
     "11ptgc", "11lab", "11waterflame" - handled after random4 (next pass
     for their exact arms; the shape repeats).

   Each pool table is 400 entries x 260 bytes (0x41 dwords) of stage
   names - five tables is ~508KB, which is most of why sizeof(director)
   is 0x20eb18. The port's course reader (ez2/stageini.c) models only the
   four `random*` pools; `7znpfg` / `11ptgc` / `11lab` / `11waterflame`
   are special channels it has never seen - artist/entry courses with
   their own draw pools, selected here rather than in the stage.ini
   reader. PORT WORK once the arms are transcribed.

   An ordinary channel's stage list goes to the per-channel tables of
   section 4e instead. */

/* ---- section 4g: the last three arms, the ordinary fill, channel art -----
   (and a CORRECTION to 4f: there are EIGHT pool tables, not five)

     "11ptgc"       -> pool at +0x159dd4, count +0x20bda8
     "11waterflame" -> pool at +0x173814, count +0x20bdac
     "11lab"        -> pool at +0x18ce54, count +0x20bdb0

   Eight 400x260 pool tables is ~812KB of the object. An ORDINARY channel
   takes the else arm: at most FIVE stages (a 5-countdown copy loop) into
   the per-channel stage table at +0xc0e44 (260 bytes per stage, five per
   channel), then `%s_level` is read per channel, atol'd into the level
   table at +0xdb104, and logged twice.

   Then per channel the SELECT ART loads - radio-family directories keyed
   by mode index:

     mode 7: system\channel_disc\radio\%s.bmp   + system\channel_info\radio\%s.bmp
     mode 6:                    5radio\%s.bmp   +                    5radio\%s.bmp
     mode 8:                    10radio\%s.bmp  +                    10radio\%s.bmp
     mode 9:                    14radio\%s.bmp  +                    14radio\%s.bmp

   - the channel wheel's DISC and INFO PANEL per course, straight port
   material for course select. One more texture load follows, the state
   returns to 0x28 and the profile closes.

   The NORMAL modes' branch then walks the flattened song list (count at
   +0x80) building `sound\%s\` paths per song - the sound-folder
   resolution - which is where the next pass picks up. */

/* ---- section 5: the per-song walk and the select audio (read 2026-08-13) -

   Normal modes walk the flattened song list; per song:
     - `sound\%s\` resolution, then TIER PROBES (@0x431900 twice, @0x4319f0)
       whose results 1/2/3 clear the song record's flags at +0x114/+0x120/
       +0x12c - the per-tier availability the wheel greys out;
     - disc art: `system\discsmall\%s.bmp` normally, but CV2 takes
       `system\CV2Mix\Disc\cv2disc_%02d.bmp` - a NUMBERED generic disc
       (the Discsmall tglv mystery's sibling; the index is the record's,
       to pin when the asm slice is read);
     - the title picture exactly as the port already ships it: CV2 through
       the variant-stripper @0x430f50 into CV2Mix\SongName, everyone else
       system\songname\%s.bmp;
   then `system\disc\initdisc.bmp` (with @0x408e80 first) and two more
   loads close the walk.

   THE SELECT AUDIO: eleven samples via @0x40e3c0 into the handle block at
   +0xbfd28.. (the click object section 4d uses), then the PREVIEW path
   per mode family:

     mode 6: system\preview_5radio\%s.wav     mode 8: preview_10radio
     mode 9: preview_14radio                  mode 7: preview_radio,
     or preview_legacy when g_1b2e7e0 is set.

   g_1b2e7e0 IDENTIFIED: the RadioMix LEGACY VARIANT toggle - it selects
   Stage_0.ini (4d) and preview_legacy together. The wheel's per-song
   preview music is a per-mode wav directory keyed by the song name -
   straight port material nothing in ez2play models yet. */

/* ---- section 6 (shape pass, 2026-08-13): the clip/texture loads ----------

   The pattern is resource-container pushes (@0x412dd0 open, @0x412e60
   close - they set the base directory, e.g. System\SongSelect\VF) around
   runs of loadStr @0x402de0 + attach @0x4028b0 pairs feeding the forty
   players, with setCell @0x402910 overrides after some. Named so far:

     System\SongSelect\VF\Selectcursor.str        (the wheel cursor)
     System\CV2Mix\ModeSelect\tgselect.str        (CV2's selector)
     System\CV2Mix\disc\tglevelicon.str           (the tg level icons -
                                                   the Discsmall tglv
                                                   system's clip)
     VF_ModeInOut\modetext%02d(+_m).bmp           (the mode plates)
     VF_ModeInout\Mode_Outro.str                  (PRELOADED BY SELECT -
                                                   the outro the port
                                                   plays at session end)
     System\SongSelect\Version\Version_%02d.bmp
     System\SongSelect\V_View\version\Version_%02d.bmp
     System\SongSelect\V_View\sortstyle\sortstyle_%02d.bmp
     System\SongSelect\Effector\Effector_catch_skin%d/player%d/turn_on.bmp
                                                   (mode 10 = catch only)

   A long anonymous run of ~15 loadStr+attach pairs sits behind
   `g_1b2eb6c == 0` (the non-CV2 screen's clips). The exact clip -> player
   pairing is NOT recoverable from the decompile (the `this` args are
   lost); the next pass scripts it out of the raw listing - every
   `push str; lea player; call` triple in 0x435390's range. That mapping
   is the last piece of the manifest and the port's native select layout. */

/* ---- section 6 manifest (extracted 2026-08-13, 241 asset calls) ----------
   Full listing in scratchpad/ssdir-manifest.txt (regenerate with the walk
   in the loop's iteration-12 script); the ctor ends at 0x438d2b. The
   headline clip -> player pairs (CV2 and VF branches alternate some
   players; the manifest keeps the raw order):

     songselect_bg.str (bg\default)      -> player +0xbc890   the backdrop
     TT_DiscCloseUp.str                  -> player +0xc09e8
     caution.str                         -> player +0xc0c04
     tgselect.str / songselect_layout.str-> player +0xbccc8   (CV2 / VF)
     tglevelicon.str                     -> player +0xbf05c   (+setCell)
     Selectcursor.str                    -> player +0xbcee4
     CourseLayout.str                    -> player +0xbcaac
     Mode_Outro.str                      -> player +0xbc674   the FIRST
                                            player, with two setCell plate
                                            overrides (modetext%02d into
                                            +0xbc488's array)
     ani_Version.str                     -> player +0xbe8d8
     Version_View.str                    -> player +0xbeaf4
     info_Panel.str / info_u / info_d    -> players +0x20d7b8/+0x20d164/
                                            +0x20d380 (Channel_Info\common)
     effector.str (cv2 modeselect)       -> player +0xbf278

   Texture families with member offsets: rank panels +0xbfecc/+0xbfed0,
   UseKey panels +0xc04a4..b0, video blank +0xbc670, disc/shape masks
   +0xbc44c/+0xbc450, time fonts +0xbd344 (glow) / +0xbd31c (solid) -
   AGREEING with the matched m439dc0's members, which validates the whole
   extraction - category_%02d +0xbc4b8, channel numerals +0xbc58c, list
   bg/cursor +0xbc5c8/+0xbc5cc, CV2 target panels +0xbc5d0/+0xbc5d4,
   Version_%02d +0xbffc0, sortstyle +0xc000c, modetext array +0xbc488.

   With this the READING of the ctor is substantively complete: data
   phase (sections 2-5), asset phase (6), and only the trailing defaults
   (7, between the last load and 0x438d2b) remain to skim. The two
   consumers now split:
     - the BYTE MATCH: grow include/songselect.h to the full member set
       and reconstruct section by section against the zoo;
     - the PORT: the channel wheel, hidden songs, pools, previews and
       this native layout - the "closer experience" half. */
