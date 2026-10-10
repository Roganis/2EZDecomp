#include "ksound.h"   /* KRuntimeClass */

/* Singleton/global-instance getters: each returns the address of a global
   object. `mov eax, offset g; ret`. The GLOBAL marker pins each reconstructed
   global to the original's address so the operand matches. Storage is a dummy
   placeholder (real layout TBD when the referencing code is reconstructed). */

/* (The getter formerly here at 0x401a90 returning &0x4a50f4 turned out to be
   KSNDMixer vtable slot 0 — moved to src/mixer.cpp as getClassDesc.) */

/* Not static, and its getter is NOT here: 0x405570 is slot 0 of the vtable
   @0x487b80, so it has to BE the slot (src/obj405f80.cpp). */
// GLOBAL: EZ2AC_UNPACKED 0x004a5218
int g_4a5218;

/* (0x4a72b8 and its getter @0x40cde0 moved to src/loaders.cpp: the getter is
   vtable slot 2 of KEzwLoader, so it has to BE the slot, and the storage is
   that loader's descriptor record. What EZ2AC::v2 @0x415cb0 hands to a
   subsystem is exactly that - the loader registering itself.) */

/* Its getter is slot 0 of the vtable @0x488320 - src/obj40d4e0.cpp. */
// GLOBAL: EZ2AC_UNPACKED 0x004a72d4
int g_4a72d4;

/* Its getter is slot 0 of the vtable @0x4884e8 - src/obj40eac0.cpp. */
// GLOBAL: EZ2AC_UNPACKED 0x004a7310
int g_4a7310;

/* (0x4a7320 / 0x4a7330 and their getters @0x40efa0 / @0x40efb0 moved to
   src/loaders.cpp for the same reason - they are KWavLoader's slots 0 and 2.) */

/* Its getter is slot 0 of the vtable @0x4887d0 - src/obj4102c0.cpp. */
// GLOBAL: EZ2AC_UNPACKED 0x004a733c
KRuntimeClass classKEZPlayer;

/* (The getter formerly here at 0x4104c0 returning &0x4a734c turned out to be
   KSong vtable slot 0 — moved to src/ksong.cpp as KSong::getClassDesc.) */

/* __cdecl setter into a stride-8 global array in the large .data section:
   `*(int*)(0x1b2e8a8 + idx*8) = val`. (Formerly parked as wip/setglobal.cpp:
   the annotation resolved as a phantom `identities::` scope until the
   CurlyManager block-comment patch in tools/reccmp.) */
struct Pair4184 { int a; int b; };

// GLOBAL: EZ2AC_UNPACKED 0x01b2e8a8
Pair4184 g_table4184[1];

// FUNCTION: EZ2AC_UNPACKED 0x004184e0
void set_4184e0(int idx, int val) { g_table4184[idx].a = val; }

/* KParser's delimiter-char table and int run-table (src/kparser.cpp buffer ctor
   @0x4138c0 stores these pointers at this+0x10/+0x14). Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x004a8520
char g_parserDelims[16];

// GLOBAL: EZ2AC_UNPACKED 0x004a8530
int g_parserTable[14];   /* 0x38 bytes: one type per delimiter, up to g_parserKey @0x4a8568 */

/* The engine's own sine and cosine, sampled 1024 times over a turn and stored
   adjacent - 0x4a5228 and 0x4a6228 are exactly 0x1000 apart. Only the
   magnitude is tabulated: Obj40BBB0::rotateZ @0x40bc70 looks a negative angle
   up with its absolute value and negates the sine afterwards. Placeholder
   storage. */
// GLOBAL: EZ2AC_UNPACKED 0x004a5228
float g_sinTable[1024];
// GLOBAL: EZ2AC_UNPACKED 0x004a6228
float g_cosTable[1024];

/* The 16x16 1bpp CJK glyph bitmap the direct-to-back-buffer text path reads:
   0x92e glyphs of two bytes per row and sixteen rows. Both the address and the
   size are pinned by loadGlyphFont @0x407080, which reads exactly 0x125c0
   bytes into it. */
// GLOBAL: EZ2AC_UNPACKED 0x00d376b8
unsigned char g_glyphBits[0x92e * 32];

/* The half-width companion of the glyph bitmap: 128 glyphs of sixteen rows,
   one byte per row, plus a per-glyph width. loadAsciiFont @0x407120 reads
   exactly 0x800 and 0x200 bytes into them, and the draw loop @0x406e40 indexes
   the widths as `[glyph * 4]` and runs its column loop to that value - so the
   font is 8x16 but proportional. */
// GLOBAL: EZ2AC_UNPACKED 0x00d36cb8
int g_asciiWidth[128];
// GLOBAL: EZ2AC_UNPACKED 0x00d36eb8
unsigned char g_asciiBits[128 * 16];

/* ---- PlayerSlot::commit @0x42e620's own state (src/sink.cpp) ------------- */

/* The two per-player "this side has bottomed out" latches. Game over is only
   raised once BOTH are 1 - which is what lets one player fail and the other
   keep playing in a two-slot cabinet. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef90
int g_player0Down;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef64
int g_player1Down;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e7d8
int g_1b2e7d8;

/* Per-player audience base, read as `g_1b2eaec[g_elemCount]` by the sink's
   f794 == 1 arm. Two entries are all that is evidenced. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eaec
float g_1b2eaec[2];

/* THE COMBO MULTIPLIER, 1,500 floats indexed by the combo (clamped to 1499).
   The values are `1 + log10(c) / 3`, exact to one ulp of float32 across the
   whole table with mult[0] = mult[1] = 1.0 - established by computing them,
   not by copying them: the table is the game's data and does not belong in
   this repository. Placeholder storage, so the reference resolves. */
// GLOBAL: EZ2AC_UNPACKED 0x004a9160
float g_comboMult[1500];

/* Set @0x429f2a, cleared during slot setup @0x4305c5. While it is set, the
   stage teardown @0x42e360 tops the course block up as if every note had been
   a KOOL and forces the grade to 10. What selects it is not read. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea2c
int g_1b2ea2c;

/* The catch gauge-rate ladder @0x49e3c8 - six per-judge floats per game
   level, read by the catch commit @0x472fc0. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x0049e3c8
float g_catchGaugeRates[16][6];

/* GFMainGameDirector's video-name trim buffer. It lives HERE rather than
   beside its function because in maingame.cpp our own g_1b2ef60 landed four
   bytes below it, and videoName's biased `buf[len - 4]` operand then
   resolved to that global on our side against `buf-4` on the original's -
   the documented hazard of a big placeholder array in a TU whose data is
   referenced with a bias. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd388
char g_gfMainVideoName[260];

/* The song table, looked up by mode name on the way into the .gds load. It
   is 0x927c of placeholder, which is why it lives here rather than beside
   its users. */
#include "ezscreen.h"
// GLOBAL: EZ2AC_UNPACKED 0x03780958
SongTable g_songTable;

/* The selected song's key, republished out of SongSelectDirector::m4332c0
   @0x4332c0 so the mode-input handlers can read it without the director. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efd4
int g_selectedKey;

/* The KProfile KGraphics::m407360 @0x407360 points its +0x11c at, and the
   KGraphics singleton the same function publishes. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x00d49c98
int g_profileD49C98[0x6];
// GLOBAL: EZ2AC_UNPACKED 0x00d49cb0
void *g_graphicsSingleton;

/* IID_IDirect3D7, which KGraphics::m405a90 @0x405a90 queries with. Placeholder
   storage - only the ADDRESS has to map, and the value is the SDK's. */
// GLOBAL: EZ2AC_UNPACKED 0x004a01b0
int g_iidDirect3D7[4];

/* IID_IDirectDraw7, which the device probe @0x405310 creates each adapter
   with. Placeholder storage like the IID above it. */
// GLOBAL: EZ2AC_UNPACKED 0x004a01c0
int g_iidDirectDraw7[4];

/* The texture loader's file buffer @0x530c48: a whole .bmp is read into it
   (KGraphics::loadBmpTexture @0x407c60), header first. Split at the
   BITMAPINFOHEADER so that `&info` is an entity of its own (the loader
   pushes its address): the 14-byte BITMAPFILEHEADER (+0xa = bfOffBits), then
   the info header and pixels - 8 MB in all, the next global sits at
   0xd30c48. */
// GLOBAL: EZ2AC_UNPACKED 0x00530c48
unsigned char g_bmpFileHeader[14];
// GLOBAL: EZ2AC_UNPACKED 0x00530c56
unsigned char g_bmpInfo[0x800000 - 14];

/* The per-HID song-list counts the loader @0x425250 walks: HID<k>-<n> lists
   are read while g_hidCounts[k-1] > 0. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e848
int g_hidCounts[22];

/* The fade level TotalResultDirector::update2 @0x454800 publishes every
   frame. Its storage lives here rather than beside its user: declared in
   that TU it shifted .data and cost ResultDirector::m450c00 @0x450c00 its
   match through reccmp patch 12's biased-array naming. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd06c
int g_37dd06c;

/* The three hidden key sequences the mode-select screen watches for, twenty
   entries each and -1 terminated (m44c690 @0x44c690 walks them). Placeholder
   storage - only the address matters. */
// GLOBAL: EZ2AC_UNPACKED 0x004adde8
int g_hiddenCodes[3][20];

/* The three hidden key sequences the TITLE screen watches for - the same
   twenty-entry, -1 terminated shape as the mode-select screen's at 0x4adde8
   (m44f300 @0x44f300 walks them by pointer). Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x004ae018
int g_titleCodes[3][20];

/* The coin count the credit readout publishes for everyone else, cleared
   along with the engine's own once twenty credits are in (ShowCredit::update2
   @0x44fb40). */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2e8
int g_shownCredits;

/* The two device GUIDs the picker @0x405e50 recognises. Identified by value
   against the published DirectX interface identifiers; placeholder storage,
   since only the address is needed to match. */
// GLOBAL: EZ2AC_UNPACKED 0x004a0180
int g_iidTnLHalDevice[4];   /* IID_IDirect3DTnLHalDevice */
// GLOBAL: EZ2AC_UNPACKED 0x004a0190
int g_iidRefDevice[4];      /* IID_IDirect3DRefDevice */
// GLOBAL: EZ2AC_UNPACKED 0x004a01a0
int g_iidHALDevice[4];      /* IID_IDirect3DHALDevice - m406050 checks it first */

/* ---------------------------------------------------------------------------
   Globals the TITLE screen's constructor @0x44e9b0 resets, and nothing else in
   the tree had named yet. Placeholder storage; only the addresses matter.

   The ctor clears about sixty globals in one block - it is where a run's
   worth of state goes back to zero - so most of what it touches was already
   modelled by the screens that USE those flags. These ten are the remainder.
   --------------------------------------------------------------------------- */

// GLOBAL: EZ2AC_UNPACKED 0x016963c4
int g_16963c4;

/* NOT g_bonusScore[4]'s fifth element, though it sits directly above it at
   0x1b2e54c: that array is summed as [0]+[1]+[2]+[3] by the ranking ctor
   @0x457a90's own log line, so four is its real width and this is a separate
   word the title screen happens to clear in the same run. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e55c
int g_1b2e55c;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e7e0
int g_1b2e7e0;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea9c
int g_1b2ea9c;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ead4
int g_1b2ead4;

/* Eight consecutive words, cleared as a group. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eb04
int g_1b2eb04[8];

// GLOBAL: EZ2AC_UNPACKED 0x01b2eb74
int g_1b2eb74;

/* A FLOAT, not an int - the ctor writes it with `movss` out of a register it
   zeroed with `xorps`, the same way it writes g_rate1p/g_rate2p beside it. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebc4
float g_1b2ebc4;

// GLOBAL: EZ2AC_UNPACKED 0x01b2efb8
int g_1b2efb8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2f034
int g_1b2f034;

/* Two more the title tick @0x44f4d0 clears every frame, alongside g_1b2ea78
   and g_1b2efd4 - the group that gets wiped while the attract loop owns the
   screen. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eba8
int g_1b2eba8;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef94
int g_1b2ef94;
/* Two the effector panel's draw @0x41d920 reads and writes: the first gates the
   audience chart in a two-player run, the second is set when that chart's
   animation finishes. */
// GLOBAL: EZ2AC_UNPACKED 0x01b5f2ac
int g_1b5f2ac;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e714
int g_1b2e714;

/* Four more of the stride-8 option block that begins at g_table4184 @0x1b2e8a8
   - elements 1, 2, 6 and 7 of it. They are declared separately, as the three
   already at 0x1b2e8c0/0x1b2e8c8/0x1b2e8d0 are, because the region is modelled
   as one short array plus individually-named members rather than one long one.
   ResultDirector::m4512b0 @0x4512b0 reads all of them. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e8b0
int g_kind8b0;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e8b8
int g_kind8b8;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e8d8
int g_kind8d8;

// GLOBAL: EZ2AC_UNPACKED 0x01b2e8e0
int g_kind8e0;

/* The raw-note scratch buffer EzChart::loadEZ @0x411460 freads each lane's
   records into before NoteConvertAll turns them into the engine's own form.
   0x5000 thirteen-byte records - the same 0x5000 the chart's own record array
   holds. */
#include "note.h"
// GLOBAL: EZ2AC_UNPACKED 0x00d49e38
EzNoteRaw g_noteBuf[0x5000];

/* The per-category song lists the loader builds: a count per category and
   436 16-byte entry names per bank. m435070 walks them to seed a page. */
// GLOBAL: EZ2AC_UNPACKED 0x03789bd4
int g_catCounts[(0x3789c90 - 0x3789bd4) / 4];

// GLOBAL: EZ2AC_UNPACKED 0x03789c90
char g_catNames[47][436][16];

/* The result screen's rank/clear texture-path pointer tables - .rdata
   pointer arrays the GFResult ctor indexes by the rank word and the
   clear flag. Placeholder storage; the contents are string pointers. */
// GLOBAL: EZ2AC_UNPACKED 0x004ae7c0
char *g_rankTexBig[11];

// GLOBAL: EZ2AC_UNPACKED 0x004ae7ec
char *g_rankTexSmall[11];

// GLOBAL: EZ2AC_UNPACKED 0x004ae818
char *g_clearPlate[5];

// GLOBAL: EZ2AC_UNPACKED 0x004ae82c
char *g_clearPlateM[5];

/* The radio modes' free-play flag - update2 @0x41e620 runs the ungated
   effector ladder only while it is 1. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efa8
int g_1b2efa8;

/* The catch lane grace timer - m46d360 zeroes the paddle window while it
   counts down. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ebb4
int g_1b2ebb4;

/* The credit count ShowCredit::update publishes each frame. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e7e4
int g_1b2e7e4;

/* The ranking name-entry's key tables - placeholder storage, contents
   not reproduced: the per-side turntable codes {0xf,0x17}/{0x10,0x18}
   and the five-per-side effector rows. (The glyph wheel lives in
   src/rankingctor.cpp as kEntryChars; the saved name is its
   g_savedName.) */
// GLOBAL: EZ2AC_UNPACKED 0x004ae488
int g_rankKeyDown[2];
// GLOBAL: EZ2AC_UNPACKED 0x004ae490
int g_rankKeyUp[2];
// GLOBAL: EZ2AC_UNPACKED 0x004ae498
int g_rankKeyRows[2][5];

/* The pause-menu latch the EZ2DJ tick counts under, and the tick budget
   its notify slot 8 is released at (update2 @0x4249e0). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef88
int g_1b2ef88;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ea10
int g_1b2ea10;

/* The press lamps m42d370 lights: six kinds for each of two players.
   Placeholder storage - the numbers stay in the original. */
// GLOBAL: EZ2AC_UNPACKED 0x00490420
int kPressLamps[12];

/* The catch press lamps, m472300's copy of kPressLamps. Placeholder
   storage - the numbers stay in the original. */
// GLOBAL: EZ2AC_UNPACKED 0x0049e138
int kCatchPressLamps[12];

/* The scratch press lamps, m4651a0's copy. Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x0049a460
int kGFPressLamps[12];
