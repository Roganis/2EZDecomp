#ifndef SONGSELECT_H
#define SONGSELECT_H

#include "panels.h"      /* EzObject, the family's shared six-slot vtable */
#include "obj402860.h"

/* SongSelectDirector - the song-select screen's director object.

   Named by its own slot-0 KRuntimeClass @0x4aaed8 ("SongSelectDirector", base
   0x4a8e50 = KEZ2DJObject) and sized by its slot-2 object record @0x4aaee8:
   0x20eb18 bytes, count 1, flags 6.

   The layout below is what its destructor @0x431090 writes down: forty
   `lea ecx,[esi+off]; call 0x402880` pairs at EH states 0x27..0x00, i.e. forty
   distinct MEMBERS (a real array would go through __ehvec_dtor_iterator
   @0x47a12b instead). Every one of them is 0x21c bytes apart or more and its
   destructor is the shared folded `ret`, which is exactly Obj402860 - the
   0x21c-byte text/chart record. Thirty-eight of the forty sit in runs at a
   plain 0x21c stride; the gaps are members without destructors, which the
   destructor cannot see, so they stay padding. */
/* A chart row as resolveLevel @0x432310 takes it BY VALUE (0x374 bytes): the
   four difficulty levels sit at 0x214 / 0x220 / 0x22c / 0x238. */
struct SongLevels
{
    char  _pad0[0x214];
    float lv0;                                  /* 0x214 */
    char  _pad218[0x220 - 0x218];
    float lv1;                                  /* 0x220 */
    char  _pad224[0x22c - 0x224];
    float lv2;                                  /* 0x22c */
    char  _pad230[0x238 - 0x230];
    float lv3;                                  /* 0x238 */
    char  _pad23c[0x374 - 0x23c];
};

struct SongSelectDirector : EzObject, EzPooled
{
    virtual KRuntimeClass *getRuntimeClass();  /* 0x430b30 */
    virtual DrawGroup     *getObjectInfo();    /* 0x430b40 */
    virtual void           update();           /* 0x443380 (src/songselectupdate.cpp) */
    /* One of the three option toggles - 0 flips the auto-play flag in the
       four course modes, 1 the FIX TARGETBAR flag, 2 the catch-judging
       flag. Refuses any whose lock is set (src/songselecttoggle.cpp). */
    char toggleOption(int which);              /* 0x434f70 */
    /* Draw one difficulty readout: the plate, then the level bar over it,
       tinted bright when `lit` and grey otherwise (src/songselectdiff.cpp). */
    void drawDifficulty(int a1, int level, float x, float y,
                        int a5, int a6, char lit);   /* 0x432930 */
    /* One key-repeat step: player `who`, input code `code`. */
    /* Append one command to a player's code buffer and see whether it has
       completed a command sequence (src/songselectcode.cpp). */
    void m4462e0(int who, int code);           /* 0x4462e0 */
    /* The two hold-repeat scans, sharing one cursor into the code table
       at 0x491858 (src/songselectrepeat.cpp). */
    void m446440();                            /* 0x446440 */
    virtual void           update2(int, int);  /* 0x446540 (src/songselectupdate2.cpp) */
    virtual int            v5(int, int, int);  /* 0x401770 (folded) */

    SongSelectDirector();                      /* 0x435390 (src/songselectctor.cpp) */
    ~SongSelectDirector();                     /* 0x431090 */

    /* The CV2 wheel page draw + selection publish (src/songselectcv2page.cpp). */
    void m4391b0();                            /* 0x4391b0 */
    /* drawOne's CV2 sibling - panels, entry texture, level digits
       (src/songselectcv2page.cpp). */
    void m4324b0(int i, int lvl, float x, float y, float off, float frac,
                 int grey);                    /* 0x4324b0 */
    /* Trim the trailing "-xx" off entry `i`'s name into a shared buffer. */
    char *m430f50(int i);                      /* 0x430f50 */
    /* Refresh three of the charts and draw a two-digit number twice - once
       additively, once normally. */
    void m439dc0();                            /* 0x439dc0 */
    /* Republish the selected entry's key when the selection moves - shared by
       eight of the mode-input handlers. */
    void m4332c0();                            /* 0x4332c0 */
    /* The four option lamps 0xa..0xd, one flag each. */
    void setOptionLamps(int a, int b, int c, int d); /* 0x434b20 */
    /* The two START lamps - a free function until the handlers showed every
       call site loading ecx first (src/lamps.cpp). */
    void updateStartLamps();                   /* 0x44b2c0 */
    /* The mode-input handlers - the song-select cursor per mode (mode names
       from the table @0x488ce0, src/modename.cpp). */
    /* Draw the selected entry's banner and its four difficulty levels, the
       current one highlighted. */
    void drawLevels();                         /* 0x4343b0 */
    /* The small two-digit counter at the top right. */
    void drawCounter();                        /* 0x434a30 */
    /* Read the selected song's rank file into m_rankNames / m_rankScores,
       creating a default one first if it is missing. */
    void loadRankFile();                       /* 0x434b80 */
    /* Read `song`'s ranking file for this mode into m_rankingNames /
       m_rankingScores, defaulting every entry to EZ2AC_FN / 0. */
    void loadRankingFile(char *song);          /* 0x431420 */
    /* Draw the ranking table (eight names and scores) at (x, y). */
    void drawRankTable(float x, float y);      /* 0x432bb0 */
    /* Draw the song wheel: every entry within +-150 of the scroll position,
       three cell tables, and the ranking table. */
    void drawWheel();                          /* 0x433180 */
    void wheelMode8();                         /* 0x433180 folded */
    void wheelMode9();                         /* 0x433180 folded */
    /* drawWheel, then the list panel when f_dc is set. */
    void drawWheelWithList();                  /* 0x433030 */
    /* The list panel: backdrop, fifteen rows, and the eased cursor bar. */
    void drawListPanel();                      /* 0x432f00 */
    /* The genre strip on the left, and the mode label for g_elemCount. */
    void drawGenreStrip();                     /* 0x432200 */
    /* Draw course `course`'s ranking table for stage `stage` at (x, y): the two
       header textures always, the five names and scores only when `body`. */
    void drawCourseRankTable(float x, float y, int course, int stage, char body); /* 0x433d50 */
    /* Pick the highest available chart level for a requested difficulty,
       falling back 4->3->1->0 when a level is 0 (unavailable). */
    int  resolveLevel(struct SongLevels row, int mode); /* 0x432310 */
    /* Load a course's channel list from system\radiomix\Stage_<n>.ini. */
    void loadStageCourse(int stage);           /* 0x433380 */
    void input5KeyMix();                       /* 0x43ff00 */
    void inputEZ2Catch();                      /* 0x43a150 */
    void inputRubyMix();                       /* 0x43b8d0 */
    void input7StreetMix();                    /* 0x442770 */
    void inputStreetMix();                     /* 0x43c2e0 */
    void inputScratchMix();                    /* 0x43ad10 */
    void inputClubMix();                       /* 0x440af0 */
    void inputSpaceMix();                      /* 0x441540 */
    void input14RadioMix();                    /* 0x43f390 */
    void input5RadioMix();                     /* 0x43dc00 */
    void input10RadioMix();                    /* 0x43e8d0 */
    void inputRadioMix();                      /* 0x43cf30 */
    void inputCV2Mix();                        /* 0x441fa0 */
    /* The plain list page the keys modes share - update's four mode
       switches call it for every non-wheel arm (src/songselectpage.cpp). */
    void m439f00();                            /* 0x439f00 */
    void pageMode1();                           /* 0x439f00 folded */
    void pageMode2();                           /* 0x439f00 folded */
    void pageMode3();                           /* 0x439f00 folded */
    void pageMode4();                           /* 0x439f00 folded */
    void pageMode5();                           /* 0x439f00 folded */
    void pageMode10();                          /* 0x439f00 folded */
    void pageMode11();                          /* 0x439f00 folded */
    /* Something the list page runs after its entry loop. */
    void m438d30();                            /* 0x438d30 (src/songselectpage.cpp) */
    /* One list entry: index, the eased x (angleToY(d) - 274), the cosine-arc
       y, the perspective scale, and the distance brightness. ~1.8 KB with
       rotdraw + sprintf; see the workqueue. */
    void m431ae0(int i, float x, float y, float s, int bright); /* 0x431ae0 (src/songselectpage.cpp) */
    /* The per-mode select page block, ~2.2 KB. */
    void m4394e0();                            /* 0x4394e0 (src/songselectpage.cpp) */
    /* The selected song's title panel at (x, y). */
    void m434080(float x, float y, int entry, int course); /* 0x434080 (src/songselectpage.cpp) */
    /* update2's three page helpers, still stubbed. */
    /* The genre-strip pager - keys 7/8 step f_bc57c, eased tail
       (src/songselectdiffsel.cpp). */
    void m4445c0();                            /* 0x4445c0 */
    /* The stepper's reduced twin - step keys only (src/songselectdiffsel.cpp). */
    void m4458d0(int a);                       /* 0x4458d0 */
    /* The difficulty-column stepper for the keys modes - state 5's input
       (src/songselectdiffsel.cpp). */
    void m444ae0(int a);                       /* 0x444ae0 */
    /* Publish the tier-qualified name into the mode text slot - the
       steppers run it every tick (src/songselecttier.cpp). */
    void m434de0();                            /* 0x434de0 */
    /* The legacy RadioMix stage list: open on keys 4/5, step f_d4 by
       wheel/keys, confirm through loadStageCourse
       (src/songselectcv2page.cpp). */
    void m433980();                            /* 0x433980 */
    /* The ctor's per-song helpers: the five tier entries, and the three
       tier probes whose 1/2/3 results grey a tier out. */
    /* Read one course's rank file into the caller's five name/score
       slots, or write the EZ2AC_FN default (src/songselecttier.cpp). */
    int  m430cf0(char *name, void *a, void *b, int j);  /* 0x430cf0 */
    /* The three tier probes: is name's -hd / -shd / -ex variant present in
       the hidden table with its group unlocked (src/songselecttier.cpp). */
    int  m431810(char *name);                  /* 0x431810 */
    int  m431900(char *name);                  /* 0x431900 */
    int  m4319f0(char *name);                  /* 0x4319f0 */
    /* Seed the page from category `a`, restoring the cursor to the
       remembered song when `keep` (src/songselectseed.cpp). */
    void m435070(int a, int keep, int c);      /* 0x435070 */

    char _pad10[0x14 - 0x10];
    int  f14;                                  /* 0x14 - zeroed by the ctor's tail */
    char _pad18[0x1c - 0x18];
    /* The category the page was last seeded from (m435070's first
       argument). */
    int  f1c;                                  /* 0x1c */
    /* Cleared when phase 2 fires the page-in (update2). */
    int  f20;                                  /* 0x20 */
    int  f24;                                  /* 0x24 */
    char _pad28[0x2c - 0x28];
    /* The remembered cursor row - the ctor restores f84 from it for the
       non-radio modes. */
    int  f2c;                                  /* 0x2c */
    /* Which tier the last probe asked about: 1 for -hd, 0 for -shd/-ex. */
    int  f30;                                  /* 0x30 */
    /* Gates the republish of the selected entry's +0x368 field to the global
       at 0x1b2efd4 - m4332c0 @0x4332c0 tests it before its range check. */
    int  f34;                                  /* 0x34 */
    /* input5RadioMix @0x43dc00's latch: when f38 changes it clears f3c and
       records the new value, and f3c gates a second loadRankingFile. */
    int  f38;                                  /* 0x38 */
    int  f3c;                                  /* 0x3c */
    int  f40;                                  /* 0x40 */
    char _pad44[0x64 - 0x44];
    int  f64;                                  /* 0x64 */
    /* The focused entry's index, republished by the disc draw (m431ae0). */
    int  f68;                                  /* 0x68 */
    /* update @0x443380's page timer: counts to 30 in phases 4 and 5. */
    int  f6c;                                  /* 0x6c */
    /* The resolved level of the current selection - m4394e0 publishes it
       every tick. */
    int  f70;                                  /* 0x70 */
    int  f74;                                  /* 0x74 - 1 at construction */
    /* The director's own mode index - update's four 13-way draw switches
       dispatch on it. */
    int  f78;                                  /* 0x78 */
    /* How many entries the cursor wraps through. */
    int  f7c;                                  /* 0x7c */
    int  f80;                                  /* 0x80 */
    /* The entry index m430f50 trims a name out of - the cursor. */
    int  f84;                                  /* 0x84 */
    int  f88;                                  /* 0x88 */
    /* update2 snapshots the cursor pair here every tick (f8c = f84,
       f90 = f88) before its phase dispatch. */
    int  f8c;                                  /* 0x8c */
    int  f90;                                  /* 0x90 */
    int  f94;                                  /* 0x94 */
    int  f98;                                  /* 0x98 */
    char _pad9c[0xa0 - 0x9c];
    /* Phase 9's tick counter - at 60 the coin sink is poked. */
    int  f_a0;                                 /* 0xa0 */
    int  f_a4;                                 /* 0xa4 */
    char _pada8[0xb0 - 0xa8];
    /* Four counters inputStreetMix @0x43c2e0 clears whenever the cursor
       moves, right after m4332c0() - no other reconstructed function touches
       them yet. Carved out of the pad, same total size. */
    int  f_b0;                                 /* 0xb0 */
    int  f_b4;                                 /* 0xb4 */
    int  f_b8;                                 /* 0xb8 */
    int  f_bc;                                 /* 0xbc */
    int  f_c0;                                 /* 0xc0 */
    /* The wheel's scroll offset in entry units of 100 - the draws position
       entry i at i*100 - f_c4, wrapped into (-50*f7c, 50*f7c]. */
    float f_c4;                                /* 0xc4 */
    float f_c8;                                /* 0xc8 */
    /* The chase target (f84 * 100), republished every tick by update2. */
    float f_cc;                                /* 0xcc */
    float f_d0;                                /* 0xd0 */
    /* The list panel's cursor row (0..14, the input handlers @0x4339xx wrap
       it) and the flag that shows the panel at all. */
    int  f_d4;                                 /* 0xd4 */
    int  f_d8;                                 /* 0xd8 */
    int  f_dc;                                 /* 0xdc */
    int  f_e0;                                 /* 0xe0 */
    int  f_e4;                                 /* 0xe4 */
    /* The list cursor's eased y (row*20), snapping within half a pixel. */
    float f_e8;                                /* 0xe8 */
    /* The option-page toggle update2 flips (f_ec ^= 1). */
    int  f_ec;                                 /* 0xec */
    /* The song disc (m431ae0 @0x431ae0): the focused entry, the difficulty
       index its art shows (0/1/3/4 - slot 2 unused), the state (1 = art
       load pending, 2 = loaded), the swing angle driven toward 180, its
       step, and the five art sprites (base / -hd / (unused) / -shd / -ex). */
    int   f_f0;                                /* 0xf0 */
    int   f_f4;                                /* 0xf4 */
    int   f_f8;                                /* 0xf8 */
    float f_fc;                                /* 0xfc */
    float f_100;                               /* 0x100 */
    struct Sprite *f_104[5];                   /* 0x104 */
    char _pad118[0x238 - 0x118];
    /* A SECOND table of the same 0x374 stride, indexed by f84 exactly as
       `entries` is. The BASE is 0x238, not the 0x49c an earlier reading gave:
       the mode-input handler @0x43b8d0 builds `esi + i*0x374 + 0x238` and
       passes it straight to sprintf as a string, which only makes sense at an
       element's start, so m4332c0's `+0x49c` is this element's field at
       +0x264 rather than a third table. 0x238 + 435 elements ends at 0x5e054,
       leaving 0x270 before `entries` - so it plausibly holds 435, but the
       length is still not established. */
    struct SongSelectRow
    {
        char name[0x108];                      /* +0x000 - a string */
        /* Four difficulty slots, 12 bytes each; the leading float is the
           level, and m4343b0 draws its integer part as a digit texture. */
        struct { float level; float f4; float f8; } diff[4];  /* +0x108 */
        /* The course's ranking tables, one per stage: five names of eight
           and five scores, drawn by drawCourseRankTable @0x433d50. */
        char rankNames[5][5][8];               /* +0x138 */
        int  rankScores[5][5];                 /* +0x200 */
        /* +0x264 - the song's origin version (song.bin record +0x30, via
           loadSongInfo's +0x370 and m435070's copy): both version badges'
           cell index, into f_bffc0 and f_bff64[4..]. */
        int  f264;
        char _pad268[0x374 - 0x268];
    } rows[1];                                 /* 0x238 */
    char _pad5ac[0x5e2c4 - 0x5ac];
    /* The song entries - 0x374 apiece, leading with the name. The same stride
       0x4324b0 and 0x4323c0 walk from their own bases. */
    struct SongSelectEntry
    {
        char name[0x368];
        /* What m4332c0 republishes to 0x1b2efd4 when the selection moves -
           the song's origin version (song.bin +0x30, see src/songinfo.cpp). */
        int  f368;                             /* +0x368 */
        char _pad36c[0x374 - 0x36c];
    } entries[1];                              /* 0x5e2c4 */
    char _pad5e638[0xbc44c - 0x5e638];
    /* The disc base sprite and its overlay ring (m431ae0 draws the ring 3px
       larger unfocused, at angle 0 focused). */
    struct Sprite *f_bc44c;                    /* 0xbc44c */
    struct Sprite *f_bc450;                    /* 0xbc450 */
    /* The full-screen fade quad update draws at (-5,-5) 645x485 with
       f_bfdb8's level in the alpha byte. */
    struct KTexture *f_bc454;                  /* 0xbc454 */
    /* The mode plates (System\ModeSelect\VF_ModeInOut\modetext%02d(_m)):
       the plain plate indexed by g_1b2e548 into m_bc674's row 5, the mask
       into row 4. */
    int  f_bc458[12];                          /* 0xbc458 */
    int  f_bc488[12];                          /* 0xbc488 */
    /* The genre strip: 47 label textures drawn 100 units apart along a
       wheel scrolled by f_bc584, the current one (f_bc574, a copy of
       f_bc57c) tinted differently (drawGenreStrip @0x432200). */
    struct KTexture *f_bc4b8[47];              /* 0xbc4b8 */
    int  f_bc574;                              /* 0xbc574 */
    int  f_bc578;                              /* 0xbc578 */
    /* Gated in [2, 0x12] before m4332c0 will republish anything. */
    int  f_bc57c;                              /* 0xbc57c */
    int  f_bc580;                              /* 0xbc580 */
    float f_bc584;                             /* 0xbc584 */
    int  f_bc588;                              /* 0xbc588 */
    /* The list panel: fifteen row textures, its 645x485 backdrop and the
       cursor bar (drawListPanel @0x432f00; loaded @0x4372f5). */
    struct KTexture *f_bc58c[15];              /* 0xbc58c */
    struct KTexture *f_bc5c8;                  /* 0xbc5c8 */
    struct KTexture *f_bc5cc;                  /* 0xbc5cc */
    /* CV2's target-name back panel pair. */
    struct KTexture *f_bc5d0;                  /* 0xbc5d0 */
    struct KTexture *f_bc5d4;                  /* 0xbc5d4 */
    char _padbc5d8[0xbc638 - 0xbc5d8];
    /* The BPM readout's glyphs: digits 0..9, the point at [10], and the
       range dash right after (m434080 @0x434080). */
    struct KTexture *f_bc638[11];              /* 0xbc638 */
    struct KTexture *f_bc664;                  /* 0xbc664 */
    char _padbc668[0xbc670 - 0xbc668];
    struct KTexture *f_bc670;                  /* 0xbc670 - Video_Blank_256 */
    Obj402860 m_bc674;                         /* 0xbc674 */
    Obj402860 m_bc890;                         /* 0xbc890 */
    Obj402860 m_bcaac;                         /* 0xbcaac */
    Obj402860 m_bccc8;                         /* 0xbccc8 */
    Obj402860 m_bcee4;                         /* 0xbcee4 */
    Obj402860 m_bd100;                         /* 0xbd100 */
    /* Two ten-deep digit runs - m439dc0 @0x439dc0 indexes both by the same
       value's ones and tens digit. */
    struct KTexture *m_bd31c[10];              /* 0xbd31c */
    struct KTexture *m_bd344[10];              /* 0xbd344 */
    char _padbd36c[0xbd3c0 - 0xbd36c];
    Obj402860 m_bd3c0;                         /* 0xbd3c0 */
    Obj402860 m_bd5dc;                         /* 0xbd5dc */
    Obj402860 m_bd7f8;                         /* 0xbd7f8 */
    Obj402860 m_bda14;                         /* 0xbda14 */
    Obj402860 m_bdc30;                         /* 0xbdc30 */
    Obj402860 m_bde4c;                         /* 0xbde4c */
    Obj402860 m_be068;                         /* 0xbe068 */
    Obj402860 m_be284;                         /* 0xbe284 */
    Obj402860 m_be4a0;                         /* 0xbe4a0 */
    Obj402860 m_be6bc;                         /* 0xbe6bc */
    Obj402860 m_be8d8;                         /* 0xbe8d8 */
    Obj402860 m_beaf4;                         /* 0xbeaf4 */
    Obj402860 m_bed10;                         /* 0xbed10 */
    /* CV2Mix's effector readout textures, loaded by the ctor from
       System\cv2mix\modeselect\effector\ (efR_%02d / efSpeed_%02d /
       efnote_%02d / efbp_%02d); extents are the gaps between the evidenced
       bases. inputCV2Mix @0x441fa0 indexes them by g_37dcecc / g_37dcef8 /
       g_37dcea4 / g_1b2eb7c into rows 8/7/6/5 of m_bf278. */
    int  f_bef2c[(0xbef34 - 0xbef2c) / 4];     /* 0xbef2c - efR */
    int  f_bef34[(0xbefe4 - 0xbef34) / 4];     /* 0xbef34 - efSpeed */
    int  f_befe4[(0xbefec - 0xbefe4) / 4];     /* 0xbefe4 - efnote */
    int  f_befec[(0xbf058 - 0xbefec) / 4];     /* 0xbefec - efbp */
    struct KTexture *f_bf058;                  /* 0xbf058 - CV2 o_ListBar */
    Obj402860 m_bf05c;                         /* 0xbf05c */
    Obj402860 m_bf278;                         /* 0xbf278 */
    Obj402860 m_bf494;                         /* 0xbf494 */
    Obj402860 m_bf6b0;                         /* 0xbf6b0 */
    Obj402860 m_bf8cc;                         /* 0xbf8cc */
    Obj402860 m_bfae8;                         /* 0xbfae8 */
    /* The three full-screen layers of phases 8 and 9 (setBlend 2/1, 9/6 and
       2/2 in that order). */
    struct KTexture *f_bfd04;                  /* 0xbfd04 */
    struct KTexture *f_bfd08;                  /* 0xbfd08 */
    struct KTexture *f_bfd0c;                  /* 0xbfd0c */
    /* Three EzSubsystem::createSample results the ctor @0x435390 stores: the two
       cursor-move sounds (select.wav / select2.wav) and the song preview, which
       the input handler @0x43b8d0 reloads with System\Preview\%s.wav. */
    EzSoundObject *m_sndMove;                  /* 0xbfd10 */
    EzSoundObject *m_sndMove2;                 /* 0xbfd14 */
    /* The option-page open/close blips. */
    EzSoundObject *m_bfd18;                    /* 0xbfd18 */
    EzSoundObject *m_bfd1c;                    /* 0xbfd1c */
    EzSoundObject *m_sndPreview;               /* 0xbfd20 */
    char _padbfd24[0xbfd28 - 0xbfd24];
    /* The confirm blip the three toggles of m434f70 @0x434f70 play. */
    EzSoundObject *m_sndToggle;                /* 0xbfd28 */
    /* Phase 1 stops this one; the countdown tick is m_bfd34; the
       difficulty-step blip is m_bfd40. */
    EzSoundObject *m_bfd2c;                    /* 0xbfd2c */
    char _padbfd30[0xbfd34 - 0xbfd30];
    EzSoundObject *m_bfd34;                    /* 0xbfd34 */
    /* LevelChange / NoLevelChange. */
    EzSoundObject *m_bfd38;                    /* 0xbfd38 */
    EzSoundObject *m_bfd3c;                    /* 0xbfd3c */
    EzSoundObject *m_bfd40;                    /* 0xbfd40 */
    /* The second preview voice update ramps in hit-percent mode
       (g_1b2eb6c != 0). */
    EzSoundObject *m_bfd44;                    /* 0xbfd44 */
    int  f_bfd48;                              /* 0xbfd48 - the two-digit number
                                                  m439dc0 draws */
    int  f_bfd4c;                              /* 0xbfd4c - 10 at construction */
    /* The name-republish subdivider (60 ticks). */
    int  f_bfd50;                              /* 0xbfd50 */
    int  f_bfd54;                              /* 0xbfd54 */
    /* The number drawn instead of f_bfd48 once the selection is taken and
       f_bfda4 is set (m434a30). */
    int  f_bfd58;                              /* 0xbfd58 */
    /* The countdown subdivider (60 ticks per step). */
    int  f_bfd5c;                              /* 0xbfd5c */
    int  f_bfd60;                              /* 0xbfd60 - 10 at construction */
    /* The selected song's short name, latched from g_songShortName
       (0x1b2f004) by inputCV2Mix's moved block; 0x40 bytes ending exactly
       on f_bfda4. */
    char f_bfd64[0x40];                        /* 0xbfd64 */
    int  f_bfda4;                              /* 0xbfda4 */
    int  f_bfda8;                              /* 0xbfda8 */
    int  f_bfdac;                              /* 0xbfdac */
    /* update2's tick counter - the first tick clears two player globals. */
    int  f_bfdb0;                              /* 0xbfdb0 */
    /* The screen's phase: 4 while the cursor is live, 5 once a song is taken. */
    int  f_bfdb4;                              /* 0xbfdb4 */
    /* The fade level of phases 6..9 (alpha byte of the fade quad, and the
       preview volume ramp in phase 7). */
    int  f_bfdb8;                              /* 0xbfdb8 */
    char _padbfdbc[0xbfdc0 - 0xbfdbc];
    /* 5KeyMix's hold-to-cycle on button 8: ticks held, the fired flag, and a
       latch the handler sets with it. */
    int  f_bfdc0;                              /* 0xbfdc0 */
    int  f_bfdc4;                              /* 0xbfdc4 */
    int  f_bfdc8;                              /* 0xbfdc8 */
    int  f_bfdcc;                              /* 0xbfdcc */
    char _padbfdd0[0xbfdd4 - 0xbfdd0];
    /* The three the selection tracks: bfdd8 is the PREVIOUS index, bfddc and
       bfde0 the +0x368 fields of the previous and current entries, and bfdd4
       the value latched when they differ. */
    int  f_bfdd4;                              /* 0xbfdd4 */
    int  f_bfdd8;                              /* 0xbfdd8 */
    int  f_bfddc;                              /* 0xbfddc */
    int  f_bfde0;                              /* 0xbfde0 */
    int  f_bfde4;                              /* 0xbfde4 */
    int  f_bfde8;                              /* 0xbfde8 */
    int  f_bfdec;                              /* 0xbfdec - 100 at construction */
    int  f_bfdf0[(0xbfe18 - 0xbfdf0) / 4];     /* 0xbfdf0 */
    int  f_bfe18[(0xbfe34 - 0xbfe18) / 4];     /* 0xbfe18 */
    /* EZ2CATCH's effector toggles (turn / 4D / 2_3, off then on) - the
       ctor loads [0]=off/[1]=on pairs, the handlers index by the counters. */
    int  f_bfe34[(0xbfe3c - 0xbfe34) / 4];     /* 0xbfe34 */
    int  f_bfe3c[(0xbfe50 - 0xbfe3c) / 4];     /* 0xbfe3c */
    /* The option readout values the handlers put in m_20cb10 rows 3..6: fixed
       ones per mode/page and tables indexed by the option counters in
       src/gamestate.cpp (0x37dcecc, 0x37dcee8, 0x37dced0, 0x37dcea4, 0x37dcec8,
       0x37dcee4). The extents are the gaps between the evidenced bases. */
    int  f_bfe50;                              /* 0xbfe50 */
    int  f_bfe54;                              /* 0xbfe54 */
    int  f_bfe58;                              /* 0xbfe58 */
    /* [0] Reverse off, [1] Reverse on, [2] 4D on, [3] 5D on; [9]/[10] the
       Scan pair at 0xbfe80. */
    int  f_bfe5c[(0xbfe9c - 0xbfe5c) / 4];     /* 0xbfe5c */
    int  f_bfe9c[12];                          /* 0xbfe9c - EFFECTOR_NOTE_%02d */
    /* The rank-panel pair (mask, plain; the radio_rank set for the radio
       family). */
    struct KTexture *f_bfecc;                  /* 0xbfecc */
    struct KTexture *f_bfed0;                  /* 0xbfed0 */
    /* [0] FADEOUT_OFF, [1] FADEOUT, [2] FADEIN, [3] BLINK. */
    int  f_bfed4[(0xbfee8 - 0xbfed4) / 4];     /* 0xbfed4 */
    int  f_bfee8[(0xbff18 - 0xbfee8) / 4];     /* 0xbfee8 - indexed by
                                                  0x37dcecc, which reaches
                                                  0xa, so twelve is enough */
    /* inputScratchMix @0x43ad10 reads two more tables out of what used to be
       modelled as the tail of f_bfee8 - same total extent. */
    int  f_bff18[(0xbff20 - 0xbff18) / 4];     /* 0xbff18 - by 0x1b2e7d0 */
    int  f_bff20[(0xbff34 - 0xbff20) / 4];     /* 0xbff20 - by 0x1b2ea18 */
    int  f_bff34[(0xbff54 - 0xbff34) / 4];     /* 0xbff34 - by 0x37dcee8,
                                                  inputSpaceMix @0x441540 */
    int  f_bff54;                              /* 0xbff54 */
    int  f_bff58[3];                           /* 0xbff58 */
    int  f_bff64[(0xbffc0 - 0xbff64) / 4];     /* 0xbff64 */
    /* Indexed by the key table's leading dword. */
    int  f_bffc0[1];                           /* 0xbffc0 */
    char _padbffc4[0xc000c - 0xbffc4];
    /* The thirty V_View sort-style labels (sortstyle_%02d.bmp). */
    int  f_c000c[(0xc0084 - 0xc000c) / 4];     /* 0xc000c */
    /* The mode label textures, indexed by g_elemCount. */
    struct KTexture *f_c0084[(0xc00ac - 0xc0084) / 4];  /* 0xc0084 */
    Obj402860 m_c00ac;                         /* 0xc00ac */
    /* LV%02d (the 21-entry set the difficulty readout comment below names)
       and CV2's lv_%02d. */
    struct KTexture *f_c02c8[21];              /* 0xc02c8 */
    char _padc031c[0xc0320 - 0xc031c];
    struct KTexture *f_c0320[10];              /* 0xc0320 */
    /* The per-level art the difficulty readout draws: twenty-one entries
       each (the bar and the plate), with the pair at +0xc0400 used for
       anything at or past level 0x15 (m432930 @0x432930). */
    struct KTexture *t_c0348[21];              /* 0xc0348 */
    char _padc039c[0xc03a0 - 0xc039c];
    struct KTexture *t_c03a0[21];              /* 0xc03a0 */
    char _padc03f4[0xc03f8 - 0xc03f4];
    struct KTexture *f_c03f8;                  /* 0xc03f8 - alv_na */
    struct KTexture *f_c03fc;                  /* 0xc03fc - alv_na_m */
    struct KTexture *f_c0400;                  /* 0xc0400 */
    struct KTexture *f_c0404;                  /* 0xc0404 */
    char _padc0408[0xc040c - 0xc0408];
    /* The digit textures m4343b0 draws the four levels with, the two
       banner textures under them, and the eased banner offset / difficulty
       cursor position (0 or 512 to slide in and out; (g_4abd14-1)*80). */
    struct KTexture *f_c040c[(0xc0464 - 0xc040c) / 4];  /* 0xc040c */
    struct KTexture *f_c0464;                  /* 0xc0464 */
    struct KTexture *f_c0468;                  /* 0xc0468 */
    int   f_c046c;                             /* 0xc046c */
    float f_c0470;                             /* 0xc0470 */
    float f_c0474;                             /* 0xc0474 */
    int  f_c0478;                              /* 0xc0478 */
    /* The difficulty strip's eased slide (m4394e0's snap-eased chase of
       (g_4abd14 - 1) * step). */
    float f_c047c;                             /* 0xc047c */
    /* The page backdrop pair update draws at (390,420) - f_c0484 with
       setBlend(9,6) then f_c0480 with setBlend(2,2) - and the brightness
       byte splatted into their color. */
    struct KTexture *f_c0480;                  /* 0xc0480 */
    struct KTexture *f_c0484;                  /* 0xc0484 */
    int   f_c0488;                             /* 0xc0488 */
    char _padc048c[0xc0498 - 0xc048c];
    /* EZ2CATCH's three class cells, picked by g_37dced0 (update2). */
    int  f_c0498[3];                           /* 0xc0498 */
    /* m4394e0's (24,292) panel pair - f_c04a8 under blend 9/6, then one of
       f_c04a4 / f_c04b0 (mode 1) / f_c04ac (mode 10) under 2/2 - and the
       fade level f_c04b4 stepped +-30 toward 255/0 on f_20c6bc. */
    struct KTexture *f_c04a4;                  /* 0xc04a4 */
    struct KTexture *f_c04a8;                  /* 0xc04a8 */
    struct KTexture *f_c04ac;                  /* 0xc04ac */
    struct KTexture *f_c04b0;                  /* 0xc04b0 */
    int   f_c04b4;                             /* 0xc04b4 */
    /* The style-select art: (texture, unused) pairs per style rung
       (STYLE_%s%d), f_c09e0 of them. */
    int  f_c04b8[(0xc05a8 - 0xc04b8) / 4];     /* 0xc04b8 */
    Obj402860 m_c05a8;                         /* 0xc05a8 */
    Obj402860 m_c07c4;                         /* 0xc07c4 */
    /* How many difficulty rungs the g_37dcf1c cursor wraps through. */
    int  f_c09e0;                              /* 0xc09e0 */
    char _padc09e4[0xc09e8 - 0xc09e4];
    Obj402860 m_c09e8;                         /* 0xc09e8 */
    Obj402860 m_c0c04;                         /* 0xc0c04 */
    /* How long each of the two hold-to-scroll buttons has been down. */
    int  f_c0e20;                              /* 0xc0e20 */
    int  f_c0e24;                              /* 0xc0e24 */
    int  f_c0e28;                              /* 0xc0e28 - zeroed by the ctor */
    int  f_c0e2c;                              /* 0xc0e2c - zeroed by the ctor */
    /* update2's difficulty-stepper hold counters (P1 up/down, P2 up/down). */
    int  f_c0e30;                              /* 0xc0e30 */
    int  f_c0e34;                              /* 0xc0e34 */
    int  f_c0e38;                              /* 0xc0e38 */
    int  f_c0e3c;                              /* 0xc0e3c */
    /* The disc swing step, mirrored out of f_100 every tick (m431ae0). */
    float f_c0e40;                             /* 0xc0e40 */
    /* THE CHANNEL SYSTEM'S TABLES (the ctor's stage.ini parse).
       Per channel: up to five 260-byte stage names on a 0x410 stride, and
       one course level. */
    char m_stageTable[100][0x410];             /* 0xc0e44 */
    char _padda484[0xdb104 - 0xda484];
    int  m_courseLevels[100];                  /* 0xdb104 */
    /* The eight special-channel draw pools: 400 stage names of 260 each. */
    char m_poolRandom[400][0x104];             /* 0xdb294 */
    char m_poolRandom2[400][0x104];            /* 0xf48d4 */
    char m_poolRandom3[400][0x104];            /* 0x10df14 */
    char m_pool7znpfg[400][0x104];             /* 0x127554 */
    char m_poolRandom4[400][0x104];            /* 0x140b94 */
    char m_pool11ptgc[400][0x104];             /* 0x15a1d4 */
    char m_pool11waterflame[400][0x104];       /* 0x173814 */
    char m_pool11lab[400][0x104];              /* 0x18ce54 */
    char _pad1a6494[0x20bd94 - 0x1a6494];
    /* The pools' entry counts, in table order. */
    int  f_20bd94;                             /* 0x20bd94 */
    int  f_20bd98;                             /* 0x20bd98 */
    int  f_20bd9c;                             /* 0x20bd9c */
    int  f_20bda0;                             /* 0x20bda0 */
    int  f_20bda4;                             /* 0x20bda4 */
    int  f_20bda8;                             /* 0x20bda8 */
    int  f_20bdac;                             /* 0x20bdac */
    int  f_20bdb0;                             /* 0x20bdb0 */
    char _pad20bdb4[0x20bdc4 - 0x20bdb4];
    /* The Obj44FA40 update2 resets at the confirm hand-off. */
    void *f_20bdc4;                            /* 0x20bdc4 */
    struct KTexture *f_20bdc8;                 /* 0x20bdc8 - songrank.bmp */
    char _pad20bdcc[0x20be8c - 0x20bdcc];
    /* Glyph textures for the ranking names (indexed by the char->glyph map at
       0x20c240) and digit textures for the scores (0..9). */
    struct KTexture *f_20be8c[(0x20bef4 - 0x20be8c) / 4];  /* 0x20be8c */
    struct KTexture *f_20bef4[(0x20c0cc - 0x20bef4) / 4];  /* 0x20bef4 */
    /* The two rotated needles m4394e0 draws at x=12 (m404250). */
    struct Sprite *f_20c0cc;                   /* 0x20c0cc */
    struct Sprite *f_20c0d0;                   /* 0x20c0d0 */
    /* The cursor's eased angle and its target - the target moves by
       2*pi/f7c per step and the angle chases it a third of the way per tick. */
    float f_20c0d4;                            /* 0x20c0d4 */
    float f_20c0d8;                            /* 0x20c0d8 */
    char _pad20c0dc[0x20c0e0 - 0x20c0dc];
    /* Which of the two 0x20d164 / 0x20d380 tables the wheel draw refreshes:
       1 the second, 0 the first, anything else neither. */
    int  f_20c0e0;                             /* 0x20c0e0 */
    /* The ranking-name glyph set (schar_%02d). */
    struct KTexture *f_20c0e4[(0x20c1a4 - 0x20c0e4) / 4];  /* 0x20c0e4 */
    /* The per-mode rank file (sound\rank_<mode>_<song>.bin): five 8-char
       names and five scores, created as "EZ2AC_FN" / 0 when missing. */
    char m_rankNames[5][8];                    /* 0x20c1a4 */
    /* The song's ranking file (system\ranking\ranking_<mode>_<song>.bin):
       eight names read as 50-byte records, and eight scores from +0x318. */
    char m_rankingNames[8][8];                 /* 0x20c1cc */
    int  m_rankScores[5];                      /* 0x20c20c */
    int  m_rankingScores[8];                   /* 0x20c220 */
    /* char (0..127) -> glyph index for the ranking-name draw; 0xffffffff
       means no glyph. */
    int  f_20c240[(0x20c640 - 0x20c240) / 4];  /* 0x20c240 */
    /* The wheel position as a fraction of a revolution (f_c4 / f7c), capped
       at 100 - the needles' angle input. */
    float f_20c640;                            /* 0x20c640 */
    char _pad20c644[0x20c698 - 0x20c644];
    /* The ranking table's origin: a brightness byte and a float y. */
    int  f_20c698;                             /* 0x20c698 */
    float f_20c69c;                            /* 0x20c69c */
    /* The ranking panel's second ease (update2's phase 2/4 slide). */
    float f_20c6a0;                            /* 0x20c6a0 */
    float f_20c6a4;                            /* 0x20c6a4 */
    /* RadioMix's mode-change gate: its handler requires this to be exactly
       0.0f where the other handlers test f_20c6bc == 0 (inputRadioMix
       @0x43cf30, ucomiss against the 0.0f pool constant). */
    float f_20c6a8;                            /* 0x20c6a8 */
    float f_20c6ac;                            /* 0x20c6ac */
    int  f_20c6b0;                             /* 0x20c6b0 - 0xff at construction */
    char _pad20c6b4[0x20c6b8 - 0x20c6b4];
    float f_20c6b8;                            /* 0x20c6b8 */
    /* Non-zero once the selection is confirmed; gates the whole cursor. */
    int  f_20c6bc;                             /* 0x20c6bc */
    /* The slide-in offset the readout grids subtract from their rest
       position each tick. */
    float f_20c6c0;                            /* 0x20c6c0 */
    int  f_20c6c4;                             /* 0x20c6c4 */
    /* The preview volume ramp's level, 0..255. */
    int  f_20c6c8;                             /* 0x20c6c8 */
    int  f_20c6cc;                             /* 0x20c6cc */
    /* Up while the ranking panel is shown (update's phase 2 grow). */
    int  f_20c6d0;                             /* 0x20c6d0 */
    /* The selected course/stage the rank table and title panel draw. */
    int  f_20c6d4;                             /* 0x20c6d4 */
    Obj402860 m_20c6d8;                        /* 0x20c6d8 */
    Obj402860 m_20c8f4;                        /* 0x20c8f4 */
    Obj402860 m_20cb10;                        /* 0x20cb10 */
    Obj402860 m_20cd2c;                        /* 0x20cd2c */
    Obj402860 m_20cf48;                        /* 0x20cf48 */
    Obj402860 m_20d164;                        /* 0x20d164 */
    Obj402860 m_20d380;                        /* 0x20d380 */
    Obj402860 m_20d59c;                        /* 0x20d59c */
    Obj402860 m_20d7b8;                        /* 0x20d7b8 */
    /* Per-channel info-panel art (channel_info\<fam>\%s.bmp). */
    int  f_20d9d4[26];                         /* 0x20d9d4 */
    /* The three secret channel banks' course counts - the ctor zeroes only
       the first two, as the original does. */
    int  f_20da3c;                             /* 0x20da3c */
    int  f_20da40;                             /* 0x20da40 */
    int  f_20da44;                             /* 0x20da44 */
    char _pad20da48[0x20da80 - 0x20da48];
    /* The hidden-song table (Stage.ini's HID-HD%d keys): name + HD tier. */
    struct HiddenSong
    {
        char name[0x20];
        int  group;
    } m_hidden[100];                           /* 0x20da80 */
    int  f_20e890;                             /* 0x20e890 - its count */
    /* Ticks since the cursor last moved; the preview loads after 0x1e. */
    int  f_20e894;                             /* 0x20e894 */
    Obj402860 m_20e898;                        /* 0x20e898 */
    /* THE CODE BUFFER: ten entered commands per player, and the two
       lengths below it. m4462e0 @0x4462e0 appends to it and matches the
       result against the command table @0x4aaf18; m446440 @0x446440 clears
       a half's length whenever that half's hold button is not down, which
       is what makes a code have to be entered without letting go. */
    int  m_seq[20];            /* 0x20eab4 - two players x ten */
    int  seqLen[2];            /* 0x20eb04, 0x20eb08 */
    /* One lock per toggle - a set entry makes m434f70 @0x434f70 refuse
       that toggle. Exactly three, which is what the switch has. */
    int  f20eb0c[3];           /* 0x20eb0c */
};

#endif /* SONGSELECT_H */
