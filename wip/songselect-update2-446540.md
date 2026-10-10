# SongSelectDirector::update2 @0x446540 - DECODED END TO END, not yet written

Read 2026-08-26 in one sitting: 2,680 instructions, six jump tables, every
block resolved. The write is a fresh-sitting job; this note is the map.
Regenerate the pseudo-source: objdump 0x446540..0x448dd0 then
`scratchpad/decode.py <asm> <names.txt>` (names from the FUNCTION/STUB
annotations; scratchpad/ssud2-decoded.txt is the cached copy if it
survives).

Signature: `void update2(int a, int b)` (ret 8). ebp = this throughout;
the two args at F+4/F+8 are stored into f8c/f90 near the phase dispatch
(`mov %ecx,0x8c(%ebp)` - arg1 arrives in... CHECK: the stores are
`mov 0x88+arg`-shaped; re-derive at write time). Frame 0x108 + GS cookie:
one 260-byte name buffer.

## Shape

1. HEAD: lamp gating - if g_modeName is none of ClubMix / SpaceMix /
   10RadioMix / 14RadioMix (4x stricmp): with findListA(0) light outputs
   0x10/0x12/0x14, always 0x18/0x1a (else-arm lights all five), plus 0x1c;
   preview pump: q16&1 on m_sndPreview (or m_bfd44 in hit-percent mode) ->
   q17? (`*0x44` slot) restart; f_bfdb0++ == 1 -> clear 0x1b2e704 and
   0x1b5f19c; hit-percent q4(1) on m_bfd44.
2. Unless phase >= 7: if mode != 0xc call 0x4445c0 (STUB - per-tick helper);
   then unless EZ2CATCH (inlined strcmp/repz cmpsb against g_modeName!):
   OPTION-PAGE OPEN per player (button 0x11 / 0x19 pressed, getState(6)!=3,
   f_20c6bc>=1 -> m_bfd40 q5+q4(0), g_37dcf1c++, m_c05a8.reset()).
3. The DIFFICULTY CURSOR steppers: per player (or both together for modes
   4/5/8/9), buttons 0xb/0xd (P1) and 0x13/0x15 (P2), hold counters
   f_c0e30/34/38/3c (>= 0x96 held, > 5 fires), stepping g_37dcf1c down/up
   with bound f_c09e0 - 1 (count at +0xc09e0, carve), prev-latch
   g_37dcf20, blip m_bfd40 + m_c05a8.reset() when changed. (The earlier
   "down arms latch the input singleton" reading was WRONG - every arm's
   `mov %ecx` follows a fresh `mov 0x37dcf1c,%ecx`; all four latch the
   cursor properly.)
   EZ2CATCH instead steps g_37dced0 (0..2) on 0x11/0x19 with publishes:
   >2 -> 0 + g_1b2ef40=1; ==1 -> g_1b2e5a8=1; ==2 -PKD g_1b2ef40=0.
4. Cursor wrap: mode != 0xc -> g_37dcf1c %= f_c09e0; the NAME publish:
   if g_1b2eb7c == 1 -> sprintf(buf, "STYLE_%s%d_black_"?? NO:
   fmt 0x493bc8 = "STYLE_%s%d_black_"... the build: g_1b2eb7c==1 ->
   sprintf(buf, "STYLE_%s%d_black_"-family, g_modeName, cursor+1) and
   g_mixStyle==6 -> g_1b2f080=1; else mode 10 uses g_37dceb0/g_37dceac and
   fmt "STYLE_ez2catch%d_%d"; others "STYLE_%s%d_". Copy buf into
   0x1b2e5d0 (inlined strcpy loop). g_1b2e7c8 = cursor. mode 10:
   value from f_c0498[g_37dced0] (carve +0xc0498 array). setCell(2,0,v)
   into m_c05a8.
5. COIN: getState(0) == 2 -> call through [0x1b2e844] slot 0 with 1;
   getState(0x1c)==2 || getState(1)==2 -> same with 2 (the coin sink).
6. PHASE SWITCH t1 @0x448ddc on f_bfdb4-1, phases 1..9 (3 = empty):
   - P1 @447190: q4(0) on m_bfd2c (null-checked), 0x1b2ea78=0, q4(1) on
     m_sndPreview (null-checked); hit-percent: f84 = f8c =
     Obj431040::findByName(0x1b2eb04)  (STUB 0x431040 exists? it's named -
     check; args (this=ebp? ecx=ebp!, 0x1b2eb04)) with f_c4 = f_cc =
     that * 100.0f; f90 = f88; phase = 2. Non-hit-percent: same minus the
     bfd2c/preview lines, plus f_bfdd4 = ecx?? (re-derive).
   - P2 @44726a: g_1b2ebdc = 0; f_bfdb8 += 30 (or 5 hit-percent);
     > 0xfe -> = 0xff, phase = 4, f20 = 0; then the START-anywhere
     open (getState(6)==2 && no option button) or m_bfd18-held ->
     f_ec ^= 1, q5 m_bfd18/q4(0) m_bfd1c; option-button (7/8/9) with
     f_20c6bc >= 1 -> f_ec ^= 1 + blip; then the RANKING SLIDE
     (f_20c6d0 == 1: f_20c6a0 += <0x48e928> capped <0x48dbec>,
     f_20c698 += 0x19 capped 0xfe->0xff... the k values 0x48f740/0x487f5c/
     0x491b0c; else the -=/shrink form).
   - P3 @447541: f_bfdb0 == 0x50 -> phase = 4.
   - P4 @44755d: t2 @0x448e00 13-mode dispatch -> the INPUT HANDLERS
     (case order: 0->CV2Mix(4475cf), 1->RubyMix, 2->StreetMix, 3->RadioMix,
     4->ClubMix... map = table addr order: 44756c=1 RubyMix, 443575=2
     StreetMix, 44757e=7? CHECK against t2[] = [4475cf,44756c,447575,
     447599,447587,447590,4475a2,44757e,4475ab,4475b4,4475bd,4475c6,
     4475d8]; blocks in addr order: RubyMix, StreetMix, RadioMix, ClubMix,
     SpaceMix, 7StreetMix, 5RadioMix, 10RadioMix, 14RadioMix, EZ2Catch,
     ScratchMix, 5KeyMix, CV2Mix. So case1=RubyMix, 2=StreetMix,
     7=RadioMix?? NO - match each case index to its block address).
     Then modes !6..9 -> 0x4458d0(1) (STUB); the IDLE/ATTRACT counters
     (g_37dcf24 blink to 0x1e -> g_37dcf28, >= 0x3c reset; option-page
     open: the g_37dcea0 ladder 3/6/9/12/15 flipping g_37dcf28, at 15
     zeroing g_37dcea0/g_37dce9c/f28/f24 and 0x37dd044 = 0x1e); the
     option-page CLOSE mirror (buttons 0x11/0x19 with getState(6)); the
     page-count advance (mode 1: f_20c6bc 1->0 wraps setting g_37dce9c=1,
     else count 2: 2->0); the CONFIRM (mode-change buttons 0xa/0xc/0xe
     findListA(0) + 0x12/0x14/0x16 findListA(1) -> g_37dce9c=1,
     f_20c6bc=0, g_37dcf08=1, g_37dced4=0, m_bfd18 q5/q4(0)); the SLIDE
     (f_20c6bc >= 1 - wait > 0: f_20c6c0 += <0x48fd08> toward 511
     (0x493a90), f_20c6c4=0, f_20c6c8 -= 2 floor 0xe6, g_37dcf08=0; == 0:
     the mirror shrink toward 0 with g_37dcf08=1 at the floor, f_20c6c8
     += 2 cap 0xff); TARGETBAR toggle (button 9 + not-option-buttons +
     f_20c6bc==0 -> g_1b2eb74 ^= 1); m446440(); FIX-TARGETBAR brightness
     f_c0488 +/- 0x37 clamp 0..0xff on g_1b2e568==1; the COUNTDOWN
     (f_bfd5c==0x3c -> f_bfd48--, < 0xa && f_bfda4==0 -> tick m_bfd34
     q5/q4(0); <= 0 -> f_bfda4=1, f_bfd48=0, f_bfd58=0xa; f_bfd50==0x3c ->
     publish rows[f84].f49c -> 0x1b2eba0); f_bfda4==1 && f_bfd48==0 ->
     f_20c6bc=0, q4(0) m_bfd14, phase = 5 (7 for modes 6..9,c);
     publish rows[f84].f49c -> 0x1b2eba0.
   - P5 @447dca: t3 byte-table (bytetbl [0]*6,1,2,3,4,0,0,5 @0x448e4c):
     arm0 (modes 0-5,10,11) -> 0x444ae0(1) (STUB); arms 1-4 = modes 6,7,8,9
     -> input5RadioMix/inputRadioMix/input10RadioMix/input14RadioMix; arm5
     (mode 12) -> inputCV2Mix. Then the countdown continued (bfd5c on
     bfda4 gate stepping bfd48 or bfd58 with the m_bfd34 tick at < 6),
     f_bfd50 name publish, <= 0 -> q4(0) m_bfd14 + phase = 7; the shrink
     slide; publish rows[f84].f49c.
   - P6 @447fcc: m_be4a0.isDone -> q5/q4(0) m_bfd14, phase = 7.
   - P7 @448006: f_bfdb8 -= 5; g_37dced4 = 0; f94 = 0; f_bfd48 = 0;
     < 2 -> (g_1b2e568==1 ? <0x48e4b8> : 0) -> ...changeDir("data", 1)
     (str 0x493ba4 - CHECK "data"?), loadRankFile(), Obj44FA40::set(0) on
     +0x20bdc4 (carve), f_bfdb8 = 1, phase = 8, and the RESULT BANNER
     loads: mode 7 or 6/8/9 -> fmts 0x493acc/0x493af4/0x493b1c/0x493b44
     (%s-based on g_channelName 0x1b2e728) + loadTexture into ? (4 sites:
     one with 0x48d548 lit tex, one 0x493aa0); hit-percent fmt 0x493b84 on
     g_songInfoName; else "STYLE-select" fmt 0x493b6c on
     g_textSlots[g_elemCount]. THEN the slide-shrink + ranking shrink;
     f_bfdb8 == 1 && g_elemCount < 2 -> 0x1b2efb8 = ecx?? and strcpy
     rows[f84].name -> 0x1b2eb04.
   - P8 @448280: 0x1b2ec0c = f70; 0x1b2ea50[g_elemCount] = ecx??;
     f_bfdb8 += 10; > 0xfe -> = 0xff, phase = 9.
   - P9 @4482b9: f_a0++; g_37dcf30/34/38 = 0; >= 0x3c && !(q16&1 on
     m_bfd14) -> coin-sink [0x1b2e844] slot0(0).
7. TAIL (all phases): if phase 4 or 5: the RANKING slide (g_1b2eb74==1
   grow with 0x25-step/caps else shrink - same ks as P2's block). The
   WHEEL CHASE per family: mode 0xc uses f80, modes 0-5,10,11 use f7c -
   both are the huge banded stepper (target = f84*100; d = target - f_c4;
   bands at f7c*<0.48076924? no - the +-2000/1500/1000/500/200/100 style
   thresholds vs f7c*100*k> stepping f_c4 by +-5/+-10/+-2/<0x487f8c=?>/
   <0x49046c=30>; then snap |d| < 10 && past <0x493a9c>; the modulo fixups
   +-f7c*100); modes 6-9 (and 7) use the EASED chase: d wrapped +-50/100,
   f_c4 +-= (wrapexcess)*(1/3), snap when |d| < 1 (0x487bec), the
   +-(f7c*100+1) sanity resets, and the 0..f7c*100 modulo fixups.
8. KEY-RANGE PUBLISH (set_4184e0 = the effector key router):
   g_37dcebc==1 -> 0x1b2e91c=0x80, 0x1b2e914=7 always first; set(0xf,
   g_37dcebc==1), set(0x10, g_37dcec0==1), set(8, g_37dcec4==1), set(0,0),
   set(1,0), set(2?? %ebx,0), set(0x18,0); t4 @0x448e5c on g_37dcec8-1:
   arms set(0x18,1)+extra pattern - 4 arms for the FADER values; then the
   run of set(3/4/0x1d/5/0x16/0x1f/0x15/0x17/0x1e/0x19/0x1a, 0);
   0x1b2e894 = g_37dceb4; 0x1b2ef38 = 0, 0x1b2ebbc = 0; t5 @0x448e6c on
   g_37dcecc-1 (the NOTE-ARRANGE lamp arms - 10 arms setting the run to 1
   in per-value patterns, table addr order 448bbd(0? no: [448bbd,448b7f,
   448b84,448b89,448bc2,448bc7,448bcc,448bd1,448bd6,448bdb]); set(6,0),
   set(7,0); g_1b2eb6c!=0 -> 0x1b2e914 = g_37dcef8; 0x37dcee0 = ecx??;
   t6 @0x448e94 on g_37dcee4: case0 -> g_1b2e588=1; 1 -> set(6,1)
   [ecx=0x1b2e720!! not the usual - CHECK]; 2 -> set(7,1); 3 ->
   set(0x20,0)+set(0x21,0)+set(6,0)+set(7,0); set(0x11,0).
9. The FINAL option publishes: 0x1b2ea30 = ecx??; 0x1b2efa0 = g_37dceb0;
   the FADER ladder on ecx?? (1/%ebx=?/3/else) writing 0x1b2ef80,
   0x1b2f050, 0x1b2efbc, 0x1b2ea4c per mode==5/9 splits; 0x1b5f178 = it;
   0x1b2e700 = countListA(); mode 10 vs not: 0x1b2ef40 =
   (g_37dced0 != %ebx?) and 0x37dd06c = f_bfdb8. RET 8.

## The ecx?? holes

decode.py loses ecx-held values at several publish sites (marked ecx??
above): each needs one raw read at write time - they are option counters
kept in registers across the tail (likely g_37dcec8 for the fader ladder
and g_37dcee0's source).

## New STUBs needed

0x4445c0, 0x4458d0(int), 0x444ae0(int) - all SongSelectDirector methods;
0x431040 Obj431040::findByName(char*) already annotated somewhere - check;
Obj44FA40::set exists. Carves: f_c09e0 (difficulty count), f_c0498[]
(EZ2CATCH cell table), f8c/f90/f94/f_a0/f20/f_ec/f_bfd50/f_bfd5c members,
m_bfd14/18/1c/2c/34/40 sound slots (bfd2c/bfd34 in pads), f_20c6d0,
f_20c698/9c/a0 (ranking slide - 20c698/9c exist), 0x20bdc4 (Obj44FA40),
f_c0488 exists. Strings: 0x493bc8/ba8/bb4 (STYLE fmts), 0x493ba4,
0x493b84/b6c/b44/b1c/af4/acc, 0x493aa0, 0x48d548. Consts: 0x48fd08,
0x48e928, 0x48dbec, 0x48f740, 0x487f5c, 0x491b0c, 0x48e4b8, 0x48fddc,
0x487f8c, 0x487bec, 0x493a9c, 0x48dbec.

## Verification plan

callcmp with LO=0x446540 HI=0x448dd0 (the code ends at the ret 8 @448dcf;
tables follow). Expect the mode-dispatch arms to fold like update()'s
(the ICF signature) - counts for the 13 handler calls + 0x4184e0 x31 are
the bar.
