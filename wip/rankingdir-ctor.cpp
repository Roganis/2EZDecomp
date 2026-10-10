/* RankingDirector::RankingDirector @0x457a90 - MATCHED (src/rankingctor.cpp,
   100% effective; the sole residue is the bonus-sum's accumulator register,
   edx for the original's eax - all eight chain permutations were compiled and
   none reaches it, the four-buildMenus family).

   THREE CORRECTIONS the match pass made to the reading below - do not trust
   the older text where they conflict:
   - the charset @0x4ae454 is 50 chars, not 48: it ends "...;. eb" - the 'e'
     and 'b' tails are real glyphs (the 0x32 loop bound proves it). THE PORT'S
     PICKER (ported from the 48-char reading) IS MISSING THE LAST TWO.
   - the VRanking arm attaches "1p_arrow.str" TWICE (m_cbc[0] then m_cbc[1]) -
     0x459624 pushes 0x4987cc again, not the 2p2 arrow.
   - the solo arm's last attach is m_cbc[0] (it jumps past the shared
     `lea ecx,[ebp+0xed8]` with its own ecx), not m_cbc[1].
   Also: the replay flag @0x37dd27c abuts g_gfVideoName @0x37dd280, which is
   why pinning it needed reccmp patch 13 (see reccmp-patches/).

   ---- original reading notes kept below for the record -------------------- */

/* RankingDirector::RankingDirector @0x457a90 - DECODED MANIFEST (loop work,
   2026-08-13). The class's teardown is already matched (src/directors.cpp:
   trivial dtor @0x456750, deleting dtor @0x459850, vtable pin 0x498680), and
   the dtor destroys NO members - so this ctor is a flat asset/field table
   and the family's best first byte-match candidate.

   sizeof = 0x109de8 (record @0x4ae428); ctor ends @0x45984f. Full 61-call
   manifest in scratchpad/rankdir-manifest.txt (regenerate with the loop's
   manifest walker over 0x457a90..). Headlines:

     - System\Common\Video_Blank_256.bmp at +0x1ec4 (the video-blit blank)
       and System\Ranking\VRanking\Modeicon\icon_%s.bmp at +0x1ec0;
     - FIVE sounds via @0x40e3c0: ranking_bgm, rankingfall, rankingmove,
       rankingnext, rank (all .wav names resolving to shipped .ssf), plus
       ModeIntro.wav - the name-entry screen's audio set;
     - the MODE-INTRO clip with its plates (modetext%02d(+_m) at +0x1e8c's
       array, Mode_Intro.str attached + two setCell overrides) - the
       ranking screen composites the mode plate exactly like the intro;
     - the NAME-ENTRY fonts: r_fon_%02d at +0x1ce8's run, nrf_%02d,
       scfont_%1d at +0x1e00's, white_entry_%1d + white_entry_10 - the
       letter picker's own glyphs;
     - the timer font time_%d at +0x1e2c, and BG\rankingbg.str as the
       backdrop clip;
     - and the MODE-WIDE TABLE read @0x4580e0 (ranking_%s.bin and the
       %s_%s battle/2P variants) lives IN THIS CTOR - the layout
       ez2_mode_ranking_load already mirrors, including the
       fwrite-8-from-stride-50 quirk.

   PORT NOTES: run_name_entry can adopt rankingbg.str + the white_entry /
   scfont glyphs + the five sounds for a native name-entry screen; the
   mode-icon and mode-plate composite complete it. (DONE in the port,
   commit eadcc2f.)

   ---- ctor reading, first pass (decompile in scratchpad/rankctor.log) ----

   CORRECTION to the first note: the class DOES have members with dtors -
   include/directors.h models all eleven, and the matched ~RankingDirector
   @0x456750's member destructions are COMPILER-GENERATED from that model
   (the source body is just setFlags(0)). The ctor's member phase mirrors
   it: states 0..10 = four players, two Obj402860[2] ehvec arrays
   (+0x884, +0xcbc), three more players, the Obj40B2E0 at +0x1748 (its
   ctor @0x40b180), and the far player at +0x109b68.

   Body after the members:
     - THE RADIO-FAMILY TEST: the mode name at g_1b2e680 is compared
       (inline counted char loops - strncmp-shaped, a matching fight to
       come) against "RadioMix"/"5RadioMix"/"10RadioMix"/"14RadioMix";
       any hit sets the byte at +0x85df8 - the flag the mode-table reader
       @0x4580be branches on (radio modes read ranking_<mode>.bin's
       RADIO variant path).
     - an EARLY OUT when g_1b2e7ac > 0x8234 (it is clamped to 0x8235,
       a container closes, and a vcall on g_1b2e844 leaves the ctor) -
       a coin/timer guard to read properly later.
     - Video_Blank_256 + the video blit @0x408040 into +0x1ec4,
       icon_%s.bmp -> +0x1ec0, then SIX samples into +0x109d84.. (the
       manifest's five ranking sounds + ModeIntro), then the container
       opens and the 0xc-iteration font loops begin. */

/* ---- ctor reading, second pass ------------------------------------------
   After the plates/Mode_Intro composite, the ctor SEEDS THE PHASE at +0x10:
   5 (skip the screen) when g_1b2eb44 == 1, when g_1b2ea28 == 1 (the sink's
   game-over raise!), when credits @0x418310 > 1 or @0x4184a0 > 1, when the
   g_1b2f024 path's @0x456870 returns 0, or when any of g_1b2f050 /
   g_1b2ef80 / g_1b2efbc is set; phase 5 on a Radio mode becomes 4 after a
   vcall on g_1b2e844. The ranking screen SKIPPING ITSELF on game-over/2P
   is the session machine's rule seen from the director's side.

   THE GLYPH MAP: a 0x80-entry ascii->glyph table at +0x1ecc, -1-filled,
   then the charset string @0x4ae454 -
     "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>()*^_/-;. "
   - maps each character to its index while r_fon_%02d (VRanking) and
   nrf_%02d load per glyph. THE PORT'S PICKER NOW USES THIS EXACT CHARSET.
   scfont_%1d and white_entry follow; transcription continues. */

/* ---- THE DRAFT (zoo syntax pass; not yet a match attempt) ----------------
   Unmapped fields go through the view struct so include/directors.h - which
   four matched functions depend on - stays untouched until the match pass
   carves them for real (and then the FULL match.sh runs, per the cookbook's
   struct warning). */

#include "directors.h"
#include "ez2acapp.h"
#include "kcrt.h"

extern void Log(const char *fmt, ...);
extern char g_modeName[];                    /* 0x1b2e680 */
extern int  g_1b2e7ac, g_1b2eb44, g_1b2ea28, g_1b2f024, g_1b2f050;
extern int  g_1b2ef80, g_1b2efbc, g_1b2f028, g_elemCount;
extern void *g_1b2e844;

struct RankView                              /* unmapped ctor fields */
{
    char _pad0[0x10];
    int  phase;                              /* 0x10 */
    char _pad14[0x1ec0 - 0x14];
    struct KTexture *modeIcon;               /* 0x1ec0 */
    struct KTexture *videoBlank;             /* 0x1ec4 */
    char _pad1ec8[0x1ecc - 0x1ec8];
    int  glyphMap[0x80];                     /* 0x1ecc */
    char _pad20cc[0x85df8 - 0x20cc];
    char isRadioFamily;                      /* 0x85df8 */
    char _pad85df9[0x109d84 - 0x85df9];
    void *sample[6];                         /* 0x109d84 */
    char _pad109d9c[0x109de8 - 0x109d9c];
};

/* The charset the glyph map is built from - the string @0x4ae454. */
// STRING: EZ2AC_UNPACKED 0x004ae454
static const char *kEntryChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789<>()*^_/-;. ";

#if 0   /* DRAFT - enable when the match pass begins */
RankingDirector::RankingDirector()
{
    RankView *v = (RankView *)this;
    char path[260];
    int  i;

    /* the radio-family flag the mode-table reader branches on */
    if (strncmp(g_modeName, "RadioMix", 9) == 0 ||
        strncmp(g_modeName, "5RadioMix", 10) == 0 ||
        strncmp(g_modeName, "10RadioMix", 11) == 0 ||
        strncmp(g_modeName, "14RadioMix", 11) == 0)
        v->isRadioFamily = 1;
    else
        v->isRadioFamily = 0;

    /* the free-play/timer early out - semantics still to read */
    if (g_1b2e7ac > 0x8234) {
        g_1b2e7ac = 0x8235;
        /* container close + vcall on g_1b2e844, then return */
        return;
    }

    ((KGraphics *)g_sub1b2f054)->loadTexture(
        "System\\Common\\Video_Blank_256.bmp", &v->videoBlank, 0);
    /* @0x408040 video-blit hookup with g_1b2e868/g_1b2f138 */

    sprintf(path, "System\\Ranking\\VRanking\\Modeicon\\icon_%s.bmp",
            g_modeName);
    ((KGraphics *)g_sub1b2f054)->loadTexture(path, &v->modeIcon, 0);

    /* six samples, the mode plates + Mode_Intro composite, the phase
       seeding, and the glyph map:

       for (i = 0; i < 0x80; i++) v->glyphMap[i] = -1;
       for (i = 0; kEntryChars[i]; i++) {
           v->glyphMap[(unsigned char)kEntryChars[i]] = i;
           ... r_fon_%02d / nrf_%02d per glyph ...
       }
       ... scfont_%1d, white_entry_%1d, time_%d, rankingbg, the table
       read @0x4580e0 ... */
}
#endif

/* ---- ctor reading, final pass: the tables -------------------------------
   TWO mode-wide tables, each with the create-if-absent (99 x 'EZ2AC_FN' at
   stride 0x32 + zeroed score dword) and the fwrite/fread(base,8,99) +
   (scores,4,99) round trip ez2_mode_ranking_load already mirrors:

     normal: names +0x20cc,  scores +0x7c1bc,  counts 99 at +0x109b48/4c
     radio:  names +0x85e04, scores +0xffef4,  counts 99 at +0x85dfc/0x85e00
             (read only when the +0x85df8 radio-family flag is set)

   The ctor reading is COMPLETE end to end. What remains for @0x457a90 is
   the byte-match proper: carve the fields, enable the draft, zoo the
   sections. */
