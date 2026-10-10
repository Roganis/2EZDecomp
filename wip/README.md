# wip/ — parked reconstructions, triaged

Every file here is a function that is **decoded and believed correct** but not
byte-matched. Each carries its own analysis and, importantly, the list of source
forms already tried — **read the file before re-attempting one**.

Audited 2026-07-29, re-audited 2026-08-23; table reconciled against the tree
2026-08-26 and again 2026-10-05 (the promotion commits of 08-24..08-26 had emptied most of it —
wip/ is down to 7 park/transcription files plus analysis notes). Re-audit when
the class model changes substantially; that is what retired `entrykind.cpp`
and `resetscreen-ctor.cpp`.

## CHECK THE LIVE MATCH TABLE BEFORE READING A PARK

**A park file does not delete itself when its function gets matched.** The
2026-08-23 audit found SIX of them describing work that was already finished in
`src/` — `catchpanel-drawfading` (100%), `ezchart-findnext` (100%),
`panel-drawitems` (all four at 100%), `reversescratch` (100%),
`testvideo-update` (live at 96.43%, and the park was a second copy of the body),
and five of `smallleaves`'s six functions. One of them was re-derived from
scratch in a loop tick before the duplicate was noticed, so this is not a
tidiness point.

The audit is mechanical: pull every `0x00xxxxxx` out of each park, keep the ones
in `0x401000..0x478000`, and check them against the `// FUNCTION:` markers in
`src/` and against a full `./match.sh` listing. Anything already annotated is
either done or superseded. Do this before opening any park.

**And when a park's function IS live but below 100%, the park is the wrong
place for it** — fold the analysis into the source file's own comment and
delete the file. Two copies of a body is a maintenance hazard, and the analysis
is what the next reader needs to see next to the code, not beside it.

## The rule that retires a park cheaply

**A park whose stated outcome is "100%\* EFFECTIVE" is not blocked, it is DONE.**
reccmp's Accuracy counts those, and the tree already carries several. Only parks
below that grade are real walls. `entrykind.cpp` sat here for no reason but that
convention not being settled when it was written.

**A park whose blocker is a CLASS-SHAPE guess is worth re-testing** whenever the
surrounding model improves. `resetscreen-ctor.cpp` was at 27.78% because its
class was modelled with no base; giving it the right one produced five matches
and the parked body dropped in unchanged.

## For a PORT, read src/, not these files

Every function a file here is about now has its C in `src/`, and that copy is
the one that gets fixed. Two behaviour bugs were found in reconstructed C on
2026-10-05 alone (docs/PORT-DELTAS.md), so an older draft in this directory
can be WRONG where src/ is right. Use these files for their analysis only.

## Triage (re-audited 2026-10-05 against the live table)

**There are no real parks left.** The three the 08-26 table listed
(`kgraphics-colorfill.cpp`, `analogread.cpp`, and `rotdraw.cpp`'s body) are
live in src/ (`src/colorfill.cpp` 58.54%, `src/analogread.cpp` 42.90%,
`src/kgraphicsquad.cpp` 37.09%), and `createsample-40e3c0.md` is gone with
0x40e3c0 at 100% in `src/createsample.cpp`.

### Still useful - analysis that the src/ header does not carry

| file | addr | live in | live score |
| --- | --- | --- | --- |
| mesh-tu-409000.md | 0x409330..0x40a46c | src/kgraphicsworm.cpp | createWorm 99.36, evalCurve 95.88, m40a300 87.65, lookAt 71.15; the rest 100 |
| modehandler-43b8d0.md | 0x43b8d0 | src/songselectinput.cpp | 100; the note records the constant-register wall holding the two walled siblings |
| blit-402f70.cpp | 0x402f70 | src/kgraphicsblit2.cpp | 59.81 - the file is a map of the function, not a body |
| rotdraw.cpp | 0x404250 | src/kgraphicsquad.cpp | 37.09 |
| songselect-ctor.cpp | 0x435390 | src/songselectctor.cpp | 81.81 (was "still a STUB" here) |
| songselect-update2-446540.md | 0x446540 | src/songselectupdate2.cpp | 83.69 |
| songselect-remaining.md | various | see list | its "still open" list: 0x431ae0 66.83, 0x4394e0 99.03, 0x435070 86.84, 0x430970 100, ctor 81.81, update 100, update2 83.69 |
| effector-update2.cpp | 0x41e620 | src/effectorinput.cpp | 95.48 |
| ranking-nameentry.cpp | 0x45aaf0 | src/rankingname.cpp | 73.44 |

### Superseded notes - kept as history only

Candidates for folding into the source header and deleting. None contains a
body that src/ lacks.

| file | addr | now lives in |
| --- | --- | --- |
| rankingdir-ctor.cpp | 0x457a90 | src/rankingctor.cpp at 100% |
| totalresult-ctor.cpp | 0x454990 | src/totalresultctor.cpp (82.85) |
| judge-dispatch-466da0.md | 0x466da0 | src/playerslot.cpp at 100% |
| ez2dj-chartload-423a90.md | 0x423a90 | src/ez2djload.cpp at 100%* |
| radiomix-43f390.md | 0x43f390 | src/songselectinput.cpp at 100% |
| obj402860-refresh-403940.md | 0x403940 | src/accessors.cpp at 100% |
| subsystem-scan.md | 0x40df20 / 0x40df90 | src/subsystembase.cpp at 100% |
| modeselect-ctor-calltrace.txt | 0x44b8f0 | src/modeselectctor.cpp at 100%* |

## Promoted out of wip/ — the ledger

Everything below was deliberately moved into the build (2026-08-24..26, the
port-track integration sweep) and the wip file deleted; each src file's header
carries the park's analysis. Scores drift - the live match table is the
authority, so grep the address there rather than trusting any number written
here.

| was | addr | now |
| --- | --- | --- |
| loadtexture.cpp | 0x408d00 | src/loadtexture.cpp |
| ezchart-findnext.cpp | 0x411d90 | src/ezchart.cpp (retired by the 08-23 audit - already finished) |
| extractdirname.cpp | 0x412e90 | src/strutil.cpp |
| loadtracks.cpp | 0x411920 | src/loadtracks.cpp |
| testvideo-update.cpp | 0x477260 | src/testinput.cpp (audit retiree - the park was a second copy) |
| gfpanel-ctor.cpp | 0x462870 | src/gfpanelctor.cpp |
| mergetracks.cpp | 0x411310 | src/mergetracks.cpp |
| screenrun.cpp | 0x401930 | src/screenrun.cpp |
| playerreset.cpp | 0x466690 | src/playerreset.cpp |
| loadtxg.cpp | 0x408f70 | src/loadtxg.cpp |
| getbit.cpp | 0x40fbd0 | src/getbit.cpp |
| testgameoption-onmenu.cpp | 0x474ee0 | src/testgameoption.cpp |
| scorekeeper.cpp | 0x472d10 | src/scorerank.cpp (100%* effective) |
| judge-release-426700.cpp | 0x426700 | src/judge.cpp |
| judge-grade.cpp | 0x426500 | src/judgegrade.cpp |
| loadsample.cpp | 0x40e590 | src/loadsample.cpp |
| runclubmix.cpp | 0x417670 | src/moderunner.cpp |
| bgupdate-45c930.cpp | 0x45c930 | src/bgupdate.cpp |
| modeselect-buttons-44b4b0.cpp | 0x44b4b0 | src/modeselectbuttons.cpp |
| concatmatrix-406ad0.cpp | 0x406ad0 | src/concatmatrix.cpp |
| setcamera-40bd40.cpp | 0x40bd40 | src/setcamera.cpp |
| crypt.cpp | 0x410a40 | src/chartcrypt.cpp (the `be`-spill wall stands; 50+ forms, 3 cl builds) |
| crypt2.cpp | 0x410c90 | src/chartcrypt.cpp (same family) |
| slurpdecrypt.cpp | 0x4135f0 | src/parsercrypt.cpp (cipher wall) |
| kprofile-load.cpp | 0x413fe0 | src/kprofileload.cpp (the frame-alignment wall stands; 35 experiments, 20 flags) |
| parsetoken.cpp | 0x413940 | src/parsetoken.cpp |
| loadez-411460.cpp | 0x411460 | src/loadez.cpp |
| library-decrypt.cpp | - | its CRT annotations moved into src/library.cpp |
