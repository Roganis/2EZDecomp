#ifndef PLAYERS_H
#define PLAYERS_H

/* The three per-player object kinds the engine keeps on its three global lists
   (src/nodelists.cpp). All three share the same list plumbing - a vptr at +0, the
   `next` link at +4 and the id at +8 - which is why the walkers in that file can
   treat them all as ListNode; only their sizes, their reset entry points and the
   list they join differ. Each lives in a static array of exactly TWO (1P/2P),
   built at image init by a CRT vector-ctor thunk. */

/* List A's node: the big per-player state object, 0xE105C4 bytes apiece at
   0x01b5fdd0 (thunk 0x486690, ctor 0x42de30 / dtor 0x42de40 - both already
   matched in src/tinyclasses.cpp). Only the reset entry point is modeled; the
   14 MB of state behind it is not. */
/* The three node lists' common head. Whichever kind of node a list holds, the
   walkers only ever touch these fields. */
struct ListNode
{
    void     *f0;     /* 0x00 */
    ListNode *next;   /* 0x04 */
    int       f8;     /* 0x08 - the id */
    char      _padc[0x1ec - 0x0c];
    /* The result screen writes each player's combined score here - see
       ResultDirector::m4510a0 @0x4510a0. */
    int       f1ec;   /* 0x1ec */
    char      _pad1f0[0x220 - 0x1f0];
    /* The entered player name (8 spaces when blank) and its score - the
       select ctor blanks them for the fresh credit. */
    char      name[8];/* 0x220 */
    int       f228;   /* 0x228 - cleared on the way out of list A */
    int       f22c;   /* 0x22c */
};

int nextListA(ListNode **cur);   /* 0x4182e0 */
int nextListB(ListNode **cur);   /* 0x4183b0 */
int nextListC(ListNode **cur);   /* 0x418470 */

/* One entry of the 18-slot sound table a player keeps at +0x3e4: a handle and
   the id of whatever is playing in it, -1 when the slot is idle. */
struct SlotSound
{
    int f0;
    int id;      /* 0x04 - -1 means nothing is playing */
};

/* The block the sink hands back and takes again, 0x28 bytes of which the
   resync only touches the timestamp at the front and the correction at the
   end. */
struct SlotState
{
    int   start;             /* 0x00 */
    char  f4;                /* 0x04 */
    /* Which lane the update stamps in - written as a byte, one at a time. */
    unsigned char lane;      /* 0x05 */
    /* Both read with movzx by the keys commit: f6 keys the chord table, f7
       is the grade a hold's tail was judged with. */
    unsigned char f6;        /* 0x06 */
    unsigned char f7;        /* 0x07 */
    /* Four single chars, not a char ARRAY: an array inside a struct-typed
       LOCAL is a /GS buffer and gets a cookie the original's frame has no
       room for. f8 is the grade a press was judged with (the judge stores a
       byte); the stage setup reads +8 as a whole word out of the sink's
       summary. */
    unsigned char f8;        /* 0x08 */
    char  f9;
    char  fa;
    char  fb;
    /* The segment's beats per measure - the beat grid scales by it. */
    unsigned char beats;     /* 0x0c */
    char  _pad0d;
    char  _pad0e;
    char  _pad0f;
    char  f10;               /* 0x10 */
    char  f11;
    /* The note's KIND, and the whole hold state machine's selector: the
       commit hands it to m42fbf0, which latches it into the slot's +0x210
       for the tick and the pump to read back. */
    unsigned char kind;      /* 0x12 */
    char  f13;
    /* >6 marks a hold; the shuffles measure their windows with it. */
    unsigned short width;    /* 0x14 */
    short f16;               /* 0x16 */
    float rate;              /* 0x18 - the randomized playback rate */
    /* The hit's skin index: -1 for the lane's own skin, else a row of the
       panel's override table. */
    int   skin;              /* 0x1c */
    int   _pad20[(0x24 - 0x20) / 4];
    int   offset;            /* 0x24 */
};

/* The two polymorphic helpers the resync reaches through the player's +0x3d0.
   Neither is ever instantiated; only the slots it calls are named, and the
   dummies before them are what put those at the right vtable offsets. */
struct SlotSampleSink
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07();
    /* slot 8 -> +0x20: fill a block with the run's tempo summary. */
    virtual void query(int *out, int n);
    virtual void q09();
    /* slot 10 -> +0x28: seed one entry from the summary. */
    virtual void q10(int i, int *summary, int mask);
    /* slot 11 -> +0x2c: fill both blocks for one entry, 0 if it has none. */
    virtual int  begin(int h, int a, int b, int c, SlotState *st,
                       SlotSound *s);
    /* slot 12 -> +0x30: advance to the next hit, 0 when there is none. */
    virtual int  next(int a, SlotState *st, SlotSound *s);
    virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16();
    /* Returns a status the keys commit branches on - 1 means the hit it
       describes is a real one. */
    virtual int  getState(SlotSound *s, SlotState *st);  /* slot 17 -> +0x44 */
    virtual void setState(SlotSound *s, SlotState *st);  /* slot 18 -> +0x48 */
    virtual void q19(); virtual void q20(); virtual void q21();
    virtual void q22();
    /* slot 23 -> +0x5c: run once a batch of lane moves is finished. */
    virtual void commit();
};

struct SlotClock
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07();
    /* slot 8 -> +0x20: fill a block with the run's tempo summary. */
    virtual void query(int *out, int n);
    virtual void q09(); virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16();
    /* slot 17 -> +0x44: the stage setup pans each of the second player's
       sounds to 0x7f (the first player's to 0). */
    virtual void setPan(int h, int pan);
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21();
    virtual int  now();                                  /* slot 22 -> +0x58 */
    virtual void q23(); virtual void q24();
    virtual float bpm();                                 /* slot 25 -> +0x64 */
    virtual void q26(); virtual void q27();
    /* slot 28 -> +0x70: nonzero while the entry is still running. */
    virtual int  busy(int h);
    virtual void q29();
    /* slot 30 -> +0x78: 1 normally, 2 under the g_1b5f2e0 option. */
    virtual void setKind(int h, int k);
    virtual void q31();
    /* slots 32/33 -> +0x80/+0x84: the event dispatcher hands each a sound
       handle - after a PRESS is judged (KEZPlayer::reseedLane @0x40ffa0,
       sound the lane's nearest keysound), and after a RELEASE judged 2
       (@0x4100d0, stop the lane's voice). Were onRelease/onPress2. */
    virtual void reseedLane(int h);
    virtual void stopLane(int h);
};

/* One entry of a track list, 0x10 bytes. findTrack @0x42eee0 walks the array
   from the list's +0x10 comparing the FIRST word, which is what fixes the
   entry's base - every other walk reads the handle at +8 and so appeared to
   start at +0x18. */
struct TrackEntry
{
    int key;      /* 0x00 - what the by-key search matches */
    int f4;       /* 0x04 */
    int handle;   /* 0x08 - what every other walk passes on */
    int fc;       /* 0x0c */
};

/* A per-player track list inside the owner: a count and up to 26 entries,
   0x1c4 bytes apiece. */
struct TrackList
{
    int        f0;
    int        count;          /* 0x04 - what sumTracks walks to */
    int        end;            /* 0x08 - and what claimTracks walks to */
    int        first;          /* 0x0c - where claimTracks starts */
    TrackEntry items[27];      /* 0x10 .. 0x1c0 */
    char       _pad1c0[0x1c4 - 0x1c0];
};

/* The per-entry helper both owners call with `this` in ecx, so it hangs off a
   base they share. The base is empty, so it costs neither of them any
   layout. */
struct SoundOwnerBase
{
    /* Stamp every hit of one sound handle, from `start` up to `limit`, with
       the two grades, and answer how many there were (src/accessors.cpp). */
    int m422b60(int h, int tail, int press, int start, unsigned int limit);
                                                    /* 0x422b60 */
};

/* What a player keeps at +0x3cc / +0x1dc: whatever the second pass of
   applyTracks notifies, one call per entry. */
struct TrackSink
{
    char _pad0[0x14];
    /* The PlayerSlot that owns this sink - m4299e0 @0x4299e0 reaches its id,
       its +0x3d0 table and its +0x3dc counter object through it. Typed
       loosely so this header need not know the slot's layout. */
    void *owner;                         /* 0x14 */
    char _pad18[0x38 - 0x18];
    /* The grade a press has to reach before the commit will start a hold
       or average a release - compared against, never written here. */
    int  f38;                            /* 0x38 */

    char _pad3c[0x134 - 0x3c];
    /* One per lane, 0x104 bytes each: an effect timer, its kind, and the
       lane's own on/off. */
    struct LaneFx
    {
        char  _pad0[0xf4];
        float ff4;                       /* +0x0f4 */
        int   ff8;                       /* +0x0f8 */
        char  _padfc[0x100 - 0xfc];
        int   f100;                      /* +0x100 */
    }    lanes[1];                       /* 0x134, stride 0x104 */

    void m42d370(int i, int a, int b);   /* 0x42d370 - the press flash (src/panelpress.cpp) */
    /* Retire one lane's effect state - the keys commit calls it with 1 and
       with 2 around a judged press. */
    void m4299e0(int lane, int what);    /* 0x4299e0 (src/tracksinkfx.cpp) */
};

struct GFTrackSink
{
    char _pad0[0x34];
    int  f34;                            /* 0x34 - what beginStage publishes to the judge */
    char _pad38[0xfc - 0x38];
    /* The same per-lane effect record TrackSink keeps at its own 0x134, with
       the same 0x104 stride - m462760 reads the identical three fields. */
    struct GFLaneFx
    {
        char  _pad0[0xf4];
        float ff4;                       /* +0x0f4 */
        int   ff8;                       /* +0x0f8 */
        char  _padfc[0x100 - 0xfc];
        int   f100;                      /* +0x100 */
    }    lanes[1];                       /* 0xfc, stride 0x104 */

    void m4651a0(int i, int a, int b);   /* 0x4651a0 - the press flash (src/panelpress.cpp) */
    /* The GF twin of TrackSink::m4299e0 - the lane countdown's expiry
       notice, without the autoplay gate the EZ2DJ one carries. */
    void m462760(int lane, int what);    /* 0x462760 (src/tracksinkfx.cpp) */
};

struct SlotOwner : SoundOwnerBase
{
    /* Rotate every pickable lane's hits through the spare slot 21: collect
       the slot's key handles into two banks, pair them up in opposite order,
       and run the (x,21)(y,x)(21,y) chain per pair. */
    void m4226f0(struct ListNode *n, int a, int b);   /* 0x4226f0 */

    char            _pad0[0x148];
    SlotSampleSink *sink;    /* 0x148 */
    SlotClock      *clock;   /* 0x14c */
    char            _pad150[0x16c - 0x150];
    /* Who owns each sound slot, and which entry of their track list put it
       there. */
    /* The count is not evidenced - only the base and the 8-byte stride are. */
    SlotSound       sounds[48];/* 0x16c .. 0x2ec */
    char            _pad2ec[0x320 - 0x2ec];
    float           f320;      /* 0x320 - the run's base scroll rate */
    char            _pad324[0x36c - 0x324];
    char            b36c;      /* 0x36c - up when the bpm comes from the clock */
    char            _pad36d[0x37c - 0x36d];
    int             f37c;      /* 0x37c - 1 puts every player's panel on slot 0 */
    TrackList       tracks[2]; /* 0x380 */
    char            _pad708[0x794 - 0x708];
    /* The gauge/survival selector m42e590 dispatches on - the same word
       the owner director carries as its f794. */
    int             f794;      /* 0x794 */
    char            _pad798[0x7c0 - 0x798];
    /* Which track-list entry the overlay is pointed at - the swap
       @0x430500 leaves the one it did NOT mark on the panel here. */
    int             f7c0;      /* 0x7c0 */
    char            _pad7c4[0x7f0 - 0x7c4];
    int             f7f0;      /* 0x7f0 - holds the rate steady while up */

    int  m4215a0(int handle, unsigned int t); /* 0x4215a0 - is t inside one of the sound's wide hits (src/playerslot.cpp) */
    void claimTracks(ListNode *n);            /* 0x422ca0 */
    /* Total the list through the shared helper, report the total to the
       player, then walk the list a second time notifying its sink. */
    void applyTracks(struct PlayerSlot *p, int a, int b);  /* 0x422bf0 */
    /* 1 once nothing in the first list-A player's track list is still
       running. */
    int  tracksIdle();                        /* 0x41f990 */
    /* Give every hit of every entry a randomized playback rate, up to the
       time limit. */
    void randomizeHits(ListNode *n, int a, unsigned int limit); /* 0x422a60 */
};

/* The scratch game's owner: the same two helpers at the same place, but its
   track lists start 0x40 earlier. */
struct GFSlotOwner : SoundOwnerBase
{
    char            _pad0[0x148];
    SlotSampleSink *sink;    /* 0x148 */
    SlotClock      *clock;   /* 0x14c */
    char            _pad150[0x160 - 0x150];
    /* The count is not evidenced - only the base and the 8-byte stride are. */
    SlotSound       sounds[40];/* 0x160 .. 0x2a0 */
    char            _pad2a0[0x314 - 0x2a0];
    float           f314;      /* 0x314 - the run's base scroll rate */
    char            _pad318[0x33c - 0x318];
    int             f33c;      /* 0x33c - 1 puts every player's panel on slot 0 */
    TrackList       tracks[2]; /* 0x340 */
    char            _pad6c8[0x750 - 0x6c8];
    int             f750;      /* 0x750 - the gauge/survival selector (m465ed0) */

    void claimTracks(ListNode *n);            /* 0x45f110 */
    void applyTracks(struct PlayerRecord *p, int a, int b); /* 0x45f060 */
    int  tracksIdle();                        /* 0x45d080 */
    void randomizeHits(ListNode *n, int a, unsigned int limit); /* 0x45ef60 */
};

/* The catch game's owner keeps the same two helpers 0x1c bytes further along,
   so it is a different class with the same pair - not the same one reached
   through a different member. */
/* The catch owner's track list: the same shape as TrackList but 0x1bc to
   its 0x1c4, and its walkers read the entry's FIRST word where the keys
   modes' read the handle at +8. */
struct CatchTrackList
{
    int        f0;
    int        count;          /* 0x04 */
    int        end;            /* 0x08 */
    int        first;          /* 0x0c */
    TrackEntry items[26];      /* 0x10 .. 0x1b0 */
    char       _pad1b0[0x1bc - 0x1b0];
};

struct CatchSlotOwner
{
    char            _pad0[0x164];
    SlotSampleSink *sink;    /* 0x164 */
    SlotClock      *clock;   /* 0x168 */
    void           *judge;   /* 0x16c */
    char            _pad170[0x178 - 0x170];
    /* Who owns each sound slot and which entry claimed it - the same pair
       the keys owner keeps at +0x16c. */
    SlotSound       sounds[48]; /* 0x178 .. 0x2f8 */
    char            _pad2f8[0x32c - 0x2f8];
    /* The per-note playback rate resetLanes @0x46f590 seeds every item with,
       the catch owner's copy of SlotOwner::f320. */
    float           f32c;       /* 0x32c */
    char            _pad330[0x34c - 0x330];
    CatchTrackList  tracks[2];  /* 0x34c */
    char            _pad6c4[0xcac - 0x6c4];
    char            bcac;       /* 0xcac - up when the bpm comes from the clock, not the timing block */

    void claimTracks(struct CatchPlayer *p);   /* 0x46d1f0 */
    /* Shrink every wide hit in one track list: the catch answer to
       SlotOwner::randomizeHits, which reskins rather than shrinks. */
    void m46cdc0(struct CatchTrackList *t, int a, unsigned int limit);
                                               /* 0x46cdc0 */
    /* The catch copy of SlotOwner::tracksIdle - list B, count-keyed. */
    int  tracksIdle();                         /* 0x46bf30 */
};

struct PlayerSlot
{
    void        *vptr;     /* 0x000 - vftable 0x4911c8 (as Obj42DE30, whose
                              ctor/dtor are matched in src/tinyclasses.cpp) */
    PlayerSlot  *next;     /* 0x004 */
    int          id;       /* 0x008 */
    int          fc;       /* 0x00c */
    /* SIX 18-entry per-lane tables in a row, of which reset clears four in
       one loop over all four. The hold-sustain tick @0x42f4e0 reads or
       writes five of them: t58 is the sustain-active flag, ta0 the ticks
       still owed, te8 the time the next tick falls due, t130 the display
       lane the judgement is drawn on, t178 the grade it will award, and t10
       a per-lane flag that (with the byte at +0xe105b8) forces the grade to
       KOOL. */
    int          t10[18];  /* 0x010 */
    int          t58[18];  /* 0x058 */
    int          ta0[18];  /* 0x0a0 */
    int          te8[18];  /* 0x0e8 */
    int          t130[18]; /* 0x130 */
    int          t178[18]; /* 0x178 */
    int          f1c0;     /* 0x1c0 - the combo latch */
    int          f1c4[6];  /* 0x1c4 .. 0x1dc - the per-judge counters */
    int          f1dc;     /* 0x1dc - the running combo */
    int          f1e0;
    int          f1e4;
    int          f1e8;
    int          f1ec;     /* 0x1ec */
    int          f1f0;
    float        gauge;    /* 0x1f4 - the life */

    char         name[9];  /* 0x1f8 - cleared as two words and a byte */
    char         _pad201[0x210 - 0x201];
    int          f210;     /* 0x210 */
    /* Gates the chord bookkeeping the keys commit does. */
    int          f214;     /* 0x214 */
    int          f218;
    float        gauge2;   /* 0x21c - the second gauge, 5 at a fresh run */
    char         _pad220[0x230 - 0x220];
    int          block[0x60]; /* 0x230 - the same 0x180-byte run PlayerRecord has */
    char         _pad3b0[0x3c8 - 0x3b0];
    int          f3c8;     /* 0x3c8 */
    TrackSink   *f3cc;     /* 0x3cc - and four more words after it */
    SlotOwner   *f3d0;
    int          f3d4;
    int          f3d8;
    int          f3dc;
    int          f3e0;     /* 0x3e0 - the sink summary's third word (beginStage) */
    SlotSound    snd[18];  /* 0x3e4 .. 0x474 */
    char         _pad474[0x4e0 - 0x474];   /* the forty option pairs start here */
    int          f4e0;     /* 0x4e0 - the speed index (option pair 13's .b) */
    char         _pad4e4[0x4f4 - 0x4e4];
    /* Either of these two being set is what lets a judged press retune the
       hit's skin through m42ebf0. */
    int          f4f4;     /* 0x4f4 */
    char         _pad4f8[0x51c - 0x4f8];
    int          f51c;     /* 0x51c */
    char         _pad520[0x5b4 - 0x520];
    char         f5b4;     /* 0x5b4 */
    /* ...and 14 MB more, which nothing here needs. */

    void reset(int id);   /* 0x42efb0 */
    void m42e000();       /* 0x42e000 (STUB) - reset's last act */
    /* Retune one running sound to a lane's rate; the keys commit calls it
       for the head of a hold when the option at +0x4f4 / +0x51c is on. */
    void m42ebf0(int opt, int on);        /* 0x42ebf0 - set an option (src/playeroption.cpp) */
    void rebaseSounds();  /* 0x42eb80 */
    /* Total whatever m42f2d0 makes of every entry of this player's track
       list. */
    int  sumTracks();     /* 0x42f480 */
    int  m42f2d0(int h);  /* 0x42f2d0 (STUB) */
    /* Which entry of this player's track list carries a key, or -1. */
    int  findTrack(int key);   /* 0x42eee0 */
    /* Point the in-game overlay at one of this player's two keyed entries -
       and for anyone but player 1, swap their handles first. */
    void m430500(int isFirst);  /* 0x430500 (src/trackswap.cpp) */
    /* Can this state's lane be moved? */
    int  laneMovable(SlotState *st);   /* 0x42df20 */
    /* THE JUDGEMENT SINK - every judged event in the twelve keys modes ends
       here. src/sink.cpp, and read its header before touching scoring. */
    void m42e620(int grade, int count, int flag);   /* 0x42e620 */
    /* THE HOLD-SUSTAIN TICK - one lane's next sustain payment. Five of the
       sink's twelve call sites are in here; src/holdtick.cpp. */
    void m42f4e0(int lane, int kind);               /* 0x42f4e0 */
    /* THE HOLD PUMP - the tick's twin, run while the key is held; four more
       of the sink's call sites. src/holdpump.cpp. */
    void m42f880(int lane, int kind);               /* 0x42f880 */
    /* Start a hold: the state the tick and the pump then spend. */
    void m42fbf0(int lane, int grade, int length, int start, int kind);
                                                    /* 0x42fbf0 */
    /* THE KEYS COMMIT - every judged press and release in the keys modes
       goes through here, and it is what starts the holds. */
    void handleEvent(int lane, int kind);           /* 0x430970 */
    /* Is this slot's run over - the keys twin of PlayerRecord::m465ed0
       (src/playerslot.cpp). */
    int  m42e590();                                 /* 0x42e590 */
    /* Bind this player to its director for a stage and seed everything -
       options, sounds, judge windows and lanes, effect sink, note count. */
    void beginStage(struct SlotOwner *owner);       /* 0x4305a0 */
    /* Build the track sink (a .str by mode name). */
    int  m42e470();                                 /* 0x42e470 - build the panel (src/playersink.cpp) */
    void m42fd90(int lane, struct SlotSound *s, int grade, int flag);
                                                    /* 0x42fd90 */
    /* This run's grade, 0..10 normally and 0..6 under CV2Mix. Decoded in
       wip/keysrank.cpp, not matched - see that file. */
    /* What the third sink forwards to every node of list A. */
    void m42ee40(int key, int what);                /* 0x42ee40 (src/keybroadcast.cpp) */
    int  m42e190();                                 /* 0x42e190 (unmatched) */
    /* The course result: every player's stored grade, averaged. */
    int  m42e2e0();                                 /* 0x42e2e0 */
    /* Sum the stage records' +0x24 into g_1b2e590 and return the total, and
       the +0x28 sibling. */
    int  m42ef30();                                 /* 0x42ef30 - publish and total the per-element scores */
    int  m42ef70();                                 /* 0x42ef70 - ... and the score-plus-bonus */
    /* Walk the chart player's hold-note table and hand each note a fresh
       sample handle if it starts far enough past the previous claim. */
    void m42f080();                                 /* 0x42f080 */
    /* The type-6 sibling: claim movable lanes with slot 0x15, spacing x8. */
    void m42f1a0();                                 /* 0x42f1a0 */
};

/* List B's node: the Ez2Catch player, 0x55c bytes apiece at 0x01b5f318 (thunk
   0x4866d1, ctor 0x472b20 / dtor 0x472b30, one-slot vftable 0x49e428 - all
   matched in src/tinyclasses.cpp). Named by its own diagnostic string @0x49e42c,
   "CatchPlayer: Can't PVI '%s'", which sits immediately after that vftable. */
struct CatchPlayer
{
    void        *vptr;   /* 0x000 - vftable 0x49e428 */
    CatchPlayer *next;   /* 0x004 */
    int          id;     /* 0x008 */
    int          fc;     /* 0x00c */
    /* The same six per-lane tables PlayerSlot keeps. */
    int          t10[18];  /* 0x010 */
    int          t58[18];  /* 0x058 */
    int          ta0[18];  /* 0x0a0 */
    int          te8[18];  /* 0x0e8 */
    int          t130[18]; /* 0x130 */
    int          t178[18]; /* 0x178 */
    char         _pad1c0[0x35c - 0x1c0];
    struct CatchPanel *f35c; /* 0x35c - the player's panel */
    CatchSlotOwner *f360; /* 0x360 */
    SlotSampleSink *f364; /* 0x364 */
    SlotClock      *f368; /* 0x368 */
    void           *f36c; /* 0x36c - the judge */
    /* The kind of the hold in force, latched by m473a60 @0x473a60 - the
       catch twin of PlayerSlot::f210. */
    int          f370;   /* 0x370 */
    char         _pad374[0x378 - 0x374];
    SlotSound    snd[18];/* 0x378 .. 0x408 */
    char         _pad408[0x548 - 0x408];
    /* Bumped by the lane-expiry sink when the expiring lane's flag is up. */
    int          f548;   /* 0x548 */
    char         _pad54c[0x55c - 0x54c];

    /* START A HOLD - PlayerSlot::m42fbf0's catch twin (src/holdtick.cpp). */
    /* What the third sink forwards to every node of list B. */
    void m473260(int key, int what);          /* 0x473260 (src/keybroadcast.cpp) */
    /* The catch twin of PlayerSlot::m42fd90 / PlayerRecord::m466ac0 - the
       commit the lane-expiry sink runs. */
    void m473c60(int lane, struct SlotSound *s, int grade, int flag);
                                              /* 0x473c60 (STUB) */
    void m473a60(int lane, int grade, int length, int start, int kind);
                                              /* 0x473a60 */
    /* ONE INSTALMENT of a running hold - PlayerSlot::m42f4e0's catch twin
       (src/holdtick.cpp). */
    void m473660(int lane, int kind);         /* 0x473660 */
    void beginStage(CatchSlotOwner *owner);   /* 0x473fd0 (src/catchstagesetup.cpp) */
    int  m472e60();                           /* 0x472e60 - build the panel (src/playersink.cpp) */

    void reset(int id);  /* 0x473300 (src/playerslot.cpp) */
    void rebaseSounds(); /* 0x4731f0 */
    /* Total whatever m473400 makes of every entry of this player's list -
       the catch twin of PlayerSlot::sumTracks, 31 of 35 instructions
       shared with it. */
    int  sumTracks();    /* 0x473600 */
    int  m473400(int key); /* 0x473400 - count a sound's judgeable hits (src/playerslot.cpp) */
};

/* List C's node: 0x538 bytes apiece at 0x037d9d50 (thunk 0x4866f0, ctor 0x4659e0,
   one-slot vftable 0x49a680 - matched in src/tinyclasses.cpp). The log tag its
   own methods print is "GFPlayer" (@0x49a684, used at 0x465e66, i.e. between its
   ctor and dtor), so that is very likely the original's name for it. Modeled: the
   list links, the id, and the five characters of initials plus the score above
   them that runEnding @0x415db0 blanks. */
struct PlayerRecord
{
    void         *f0;         /* 0x00 */
    PlayerRecord *next;       /* 0x04 - the list-C link */
    int           id;         /* 0x08 - the lookup key */
    char          _pad0c[0x34 - 0x0c];
    int           f34;        /* 0x34 */
    float         f38;        /* 0x38 - the life (the GF gauge; m465ed0) */
    int           f3c;        /* 0x3c */
    char          initials[5];/* 0x40..0x44 - blanked to ' ' */
    char          _pad45[3];
    int           score;      /* 0x48 - cleared when it is positive */
    char          _pad4c[0x54 - 0x4c];
    float         f54;        /* 0x54 - the second gauge (m465ed0) */
    char          _pad58[0x5c - 0x58];
    int           block[0x60];/* 0x5c..0x1dc - one 0x180-byte run reset wipes */
    GFTrackSink  *f1dc;       /* 0x1dc - and five more words after it */
    GFSlotOwner  *f1e0;
    int           f1e4;
    int           f1e8;
    int           f1ec;
    int           f1f0;
    SlotSound     snd[18];    /* 0x1f4 .. 0x284 */
    char          _pad284[0x304 - 0x284];
    /* The two option flags the commit's skin retune is gated on - either one
       up is enough (PlayerRecord::m466ac0 @0x466ac0). */
    int           f304;       /* 0x304 */
    char          _pad308[0x32c - 0x308];
    int           f32c;       /* 0x32c */
    char          _pad330[0x538 - 0x330];

    void reset(int id);       /* 0x466690 (STUB; wip/playerreset.cpp) */
    void m4659f0();           /* 0x4659f0 - clear the counters and the gauges (src/gfstagesetup.cpp) */
    void rebaseSounds();      /* 0x466260 */
    /* What the third sink forwards to every node of list C. */
    void m466550(int key, int what); /* 0x466550 (src/keybroadcast.cpp) */
    void m4662d0(int opt, int on);   /* 0x4662d0 - set an option (src/playeroption.cpp) */
    int  sumTracks();         /* 0x465ad0 */
    void beginStage(GFSlotOwner *owner);   /* 0x466700 (src/gfstagesetup.cpp) */
    int  m465d90();           /* 0x465d90 - build the panel (src/playersink.cpp) */
    /* PlayerSlot::handleEvent @0x430970's GF twin (src/playerslot.cpp). */
    void handleEvent(int lane, int kind);  /* 0x466da0 */
    /* And the commit it ends in - PlayerSlot::m42fd90's twin. */
    int  m465b30();           /* 0x465b30 - the GF grade ladder */
    /* The stage-end fold the director's slot-release pass runs
       (src/gfstagesetup.cpp). */
    void m465c70();           /* 0x465c70 */
    void m466ac0(int lane, struct SlotSound *s, int grade, int flag);
                              /* 0x466ac0 (STUB) */
    int  m465a70(int h);      /* 0x465a70 - count the hits of h still at state 1 */
    /* Is this player's run over - gauge dead (either player's, two-up), or
       the survival scale past f54 (src/playersink.cpp). */
    int  m465ed0();           /* 0x465ed0 */
    void m465f60(int grade, int count, int flag);   /* 0x465f60 - the score commit (src/gfsink.cpp) */
};

PlayerRecord *findPlayer(int id);   /* 0x418f30 (STUB) */

#endif /* PLAYERS_H */
