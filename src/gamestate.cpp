/* resetGlobals430b50 @0x430b50 - the run-of-globals reset engineTick418e60
   @0x418e60 performs before it walks the lists. Nothing here is understood
   beyond its shape: one flag set to 1, everything else cleared, with nine
   five-element arrays cleared element by element (which is what fixes them as
   arrays - the stores go array-major, one round per index).

   All of it is placeholder storage, pinned to the original's addresses so the
   stores compare equal; the real data lives in the binary. */

// GLOBAL: EZ2AC_UNPACKED 0x037dceac
int g_37dceac;

// GLOBAL: EZ2AC_UNPACKED 0x037dceb0
int g_37dceb0;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e7d0
int g_1b2e7d0;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ef44
int g_1b2ef44[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2f058
int g_1b2f058[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2ebe0
int g_1b2ebe0[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b5f15c
int g_1b5f15c[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2f120
int g_1b2f120[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2e650
int g_1b2e650[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2ea38
int g_1b2ea38[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2e9fc
int g_1b2e9fc[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b2e668
int g_1b2e668[5];

// GLOBAL: EZ2AC_UNPACKED 0x01b5f2ec
int g_1b5f2ec;

// GLOBAL: EZ2AC_UNPACKED 0x01b2eb28
int g_1b2eb28;

/* Option counter: BGA display, 0..2 = BG_ON / BLACK / BG_OFF. Readout table
   SongSelectDirector+0xbff58 (EFFECTOR_BG_*.bmp); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dced0
int g_37dced0;

// GLOBAL: EZ2AC_UNPACKED 0x037dceb4
int g_37dceb4;

// GLOBAL: EZ2AC_UNPACKED 0x037dceb8
int g_37dceb8;

// GLOBAL: EZ2AC_UNPACKED 0x037dcebc
int g_37dcebc;

// GLOBAL: EZ2AC_UNPACKED 0x037dcec0
int g_37dcec0;

// GLOBAL: EZ2AC_UNPACKED 0x037dcec4
int g_37dcec4;

/* Option counter: FADER, 0..3 = OFF / FADE OUT / FADE IN / BLINK. Readout
   table SongSelectDirector+0xbfed4 (EFFECTOR_FADE*.bmp); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcec8
int g_37dcec8;

/* Option counter: note arrangement, 0..10 = OFF / RANDOM / SRANDOM / PS / FR /
   MRANDOM / HRANDOM / MIRROR / MIRROR_A / KEY / SP. Readout table
   SongSelectDirector+0xbfee8 (EFFECTOR_RANDOM*.bmp etc); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcecc
int g_37dcecc;

// GLOBAL: EZ2AC_UNPACKED 0x037dcee0
int g_37dcee0;

/* Option counter into the mixed panel-effect table at
   SongSelectDirector+0xbfe5c: 0/1 = REVERSE off/on, 2 = 4D, 3 = 5D,
   9/10 = SCAN off/on (4..8 not loaded by the ctor); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcee4
int g_37dcee4;

// GLOBAL: EZ2AC_UNPACKED 0x037dcea8
int g_37dcea8;

/* Option counter: NOTE skin, 0..12. Readout table SongSelectDirector+0xbfe9c
   (EFFECTOR_NOTE_%02d.bmp); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcea4
int g_37dcea4;

/* The side the result screen is showing alone: 0 or 1 excludes that player's
   chart from the tick, 2 (its value in a two-player result) excludes the
   post-phase ticks instead (m450b50 @0x450b50). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb60
int g_1b2eb60;

// GLOBAL: EZ2AC_UNPACKED 0x01b2eb44
int g_1b2eb44;

// GLOBAL: EZ2AC_UNPACKED 0x01b2eb7c
int g_1b2eb7c;

// GLOBAL: EZ2AC_UNPACKED 0x037dced4
int g_37dced4;

/* The difficulty cursor's previous position - the stepper @0x444ae0
   publishes it on every move. */
// GLOBAL: EZ2AC_UNPACKED 0x037dced8
int g_37dced8;

/* The one global this function SETS rather than clears. */
// GLOBAL: EZ2AC_UNPACKED 0x004abd14
int g_4abd14;

// GLOBAL: EZ2AC_UNPACKED 0x037dcedc
int g_37dcedc;

/* Option counter: AUTO assist. Keys modes read it through the table at
   SongSelectDirector+0xbff64 (0..3 = SPEDAL_OFF / SCRATCH_ON / PEDAL_ON /
   SPEDAL_ON), the street/scratch family through +0xbff34 (0..3 = AUTOS off /
   L / R / on); see cardreader/03. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcee8
int g_37dcee8;

/* CV2Mix's SPEED option, 0..0x14 (efSpeed_%02d readout at
   SongSelectDirector+0xbef34), with per-song skip rules: the 11ambit chart
   pins it >= 2, and g_mixStyle == 7 restricts it to {2, 6, 0xa} plus its
   own wrap (inputCV2Mix @0x441fa0). */
// GLOBAL: EZ2AC_UNPACKED 0x037dcef8
int g_37dcef8;

/* Passed as setOptionLamps' third argument by inputCV2Mix; never stepped by
   it. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcf00
int g_37dcf00;

/* The song disc's pulse: an x/y centre offset and a size addend the disc
   draw adds to its quad every tick (m431ae0 @0x431ae0). */
// GLOBAL: EZ2AC_UNPACKED 0x037dcf30
int g_37dcf30;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf34
int g_37dcf34;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf38
int g_37dcf38;

// GLOBAL: EZ2AC_UNPACKED 0x037dceec
int g_37dceec;

// GLOBAL: EZ2AC_UNPACKED 0x037dcef0
int g_37dcef0;

// GLOBAL: EZ2AC_UNPACKED 0x037dcef4
int g_37dcef4;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf0c
int g_37dcf0c;

/* Three the song-select handler @0x43b8d0 reads (src/songselectinput.cpp):
   0x37dcf08 gates the confirm buttons, 0x37dce9c picks the idle lamp shape, and
   0x37dcf28 is added to each option counter for the option lamps. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcf08
int g_37dcf08;

// GLOBAL: EZ2AC_UNPACKED 0x037dce9c
int g_37dce9c;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf28
int g_37dcf28;

/* Set with the hard (-hd) chart page; picks the -hd ranking file
   (SongSelectDirector::loadRankingFile). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e89c
int g_1b2e89c;

/* Set to the channel count while a course loads (loadStageCourse @0x433380). */
/* Bumped by the GF run's beat nudge @0x45d7c0 by (v % 10) % 3 - 1, i.e. by
   -1, 0 or +1 depending on the two counters' low digit. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebd0
int g_1b2ebd0;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ebf4
int g_1b2ebf4;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf10
int g_37dcf10;

// GLOBAL: EZ2AC_UNPACKED 0x01b5f2f8
int g_1b5f2f8;

// GLOBAL: EZ2AC_UNPACKED 0x01b5f2fc
int g_1b5f2fc;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ebdc
int g_1b2ebdc;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ebb8
int g_1b2ebb8;

// GLOBAL: EZ2AC_UNPACKED 0x01b2ebbc
int g_1b2ebbc;

// GLOBAL: EZ2AC_UNPACKED 0x01b5f138
int g_1b5f138;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf14
int g_37dcf14;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf18
int g_37dcf18;

// GLOBAL: EZ2AC_UNPACKED 0x037dcf1c
int g_37dcf1c;

// FUNCTION: EZ2AC_UNPACKED 0x00430b50
void resetGlobals430b50()
{
    int i;

    g_37dceac = 0;
    g_37dceb0 = 0;
    g_1b2e7d0 = 0;

    for (i = 0; i < 5; i++)
    {
        g_1b2ef44[i] = 0;
        g_1b2f058[i] = 0;
        g_1b2ebe0[i] = 0;
        g_1b5f15c[i] = 0;
        g_1b2f120[i] = 0;
        g_1b2e650[i] = 0;
        g_1b2ea38[i] = 0;
        g_1b2e9fc[i] = 0;
        g_1b2e668[i] = 0;
    }

    g_1b5f2ec = 0;
    g_1b2eb28 = 0;
    g_37dced0 = 0;
    g_37dceb4 = 0;
    g_37dceb8 = 0;
    g_37dcebc = 0;
    g_37dcec0 = 0;
    g_37dcec4 = 0;
    g_37dcec8 = 0;
    g_37dcecc = 0;
    g_37dcee0 = 0;
    g_37dcee4 = 0;
    g_37dcea8 = 0;
    g_37dcea4 = 0;
    g_1b2eb44 = 0;
    g_1b2eb7c = 0;
    g_37dced4 = 0;

    g_4abd14 = 1;

    g_37dcedc = 0;
    g_37dcee8 = 0;
    g_37dceec = 0;
    g_37dcef0 = 0;
    g_37dcef4 = 0;
    g_37dcf0c = 0;
    g_37dcf10 = 0;
    g_1b5f2f8 = 0;
    g_1b5f2fc = 0;
    g_1b2ebdc = 0;
    g_1b2ebb8 = 0;
    g_1b5f138 = 0;
    g_37dcf14 = 0;
    g_37dcf18 = 0;
    g_37dcf1c = 0;
}

/* Up when the place plate should be the shared PLACE_c rather than the
   per-place number - read by ResultDirector::m450690 @0x450690 on both of its
   two-chart walks. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f178
int g_1b5f178;

/* The two boost enables the Ez2Catch key handler @0x46e750 gates its mixer
   writes on - each is checked alongside the mixer pointer itself. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e980
int g_bassBoostOn;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e988
int g_trebleBoostOn;

/* The two lamp-sweep enables EZ2DJMainGameDirector::m422f20 @0x422f20 reads
   for keys 0x26 and 0x27. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e9d8
int g_bgBoostOn;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e9e0
int g_playBoostOn;
