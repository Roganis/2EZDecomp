#include <stdlib.h>
#include <string.h>
#include "players.h"
#include "judge.h"

/* The two globals PlayerSlot::reset clears along with the slot itself.
   Placeholder storage. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e704
int g_1b2e704;
// GLOBAL: EZ2AC_UNPACKED 0x01b5f19c
int g_1b5f19c;

/* Put one list-A player slot back to its starting state under a new id. The
   four 18-entry tables are cleared by a single loop, in the order the original's
   stores give (cl anchors its induction pointer on the second of them). In its
   own TU because src/nodelists.cpp calls this. */
// FUNCTION: EZ2AC_UNPACKED 0x0042efb0
void PlayerSlot::reset(int newId)
{
    int i;

    id = newId;
    next = 0;
    f3cc = 0;
    f3d0 = 0;
    f3d4 = 0;
    f3d8 = 0;
    f3dc = 0;
    fc = 0;
    g_1b2e704 = 0;
    g_1b5f19c = 0;
    f3c8 = 0;
    f210 = 0;

    for (i = 0; i < 18; i++)
    {
        t58[i] = 0;
        t130[i] = 0;
        te8[i] = 0;
        t178[i] = 0;
    }

    f1ec = 0;
    f5b4 = 0;
    memset(block, 0, sizeof(block));
    memset(name, 0, sizeof(name));
    m42e000();
}

/* Re-base every playing sound against the clock's current reading: read each
   live slot's state, set its correction to now-minus-start, and hand it back.
   This is what makes a resumed song pick up where it stopped. */
// FUNCTION: EZ2AC_UNPACKED 0x0042eb80
void PlayerSlot::rebaseSounds()
{
    SlotState st;
    int i;

    for (i = 0; i < 18; i++)
    {
        if (snd[i].id != -1)
        {
            f3d0->sink->getState(&snd[i], &st);
            st.offset = f3d0->clock->now() - st.start;
            f3d0->sink->setState(&snd[i], &st);
        }
    }
}

/* The same resync for the other two games' player nodes: identical bodies over
   their own tables. Written out rather than templated - the original compiled
   three separate functions and the linker did not fold them, because each one
   addresses different member offsets. */
// FUNCTION: EZ2AC_UNPACKED 0x00466260
void PlayerRecord::rebaseSounds()
{
    SlotState st;
    int i;

    for (i = 0; i < 18; i++)
    {
        if (snd[i].id != -1)
        {
            f1e0->sink->getState(&snd[i], &st);
            st.offset = f1e0->clock->now() - st.start;
            f1e0->sink->setState(&snd[i], &st);
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x004731f0
void CatchPlayer::rebaseSounds()
{
    SlotState st;
    int i;

    for (i = 0; i < 18; i++)
    {
        if (snd[i].id != -1)
        {
            f360->sink->getState(&snd[i], &st);
            st.offset = f360->clock->now() - st.start;
            f360->sink->setState(&snd[i], &st);
        }
    }
}

extern int g_elemCount;      /* 0x1b2e718 (src/loops.cpp) - the player cursor */
extern int g_1b5f300[5];     /* 0x1b5f300 (src/modeselectctor.cpp) */
extern int g_mixStyle;       /* 0x1b2ebc8 (src/panels.cpp) */

/* PlayerSlot::m42f2d0 @0x0042f2d0 - COUNT ONE TRACK'S NOTES, which is what
   sumTracks below totals into the stage's note count (+0xe105bc, and the
   caller at 0x430940 turns it into the maximum score as `count * 300`).
   Read from the port side first - see port/REQUESTS.md - and the two
   readings agree.

   It walks the track's records through the sink's begin/next pair and adds,
   per record: the HEAD, unless the record is a type other than 1 or its
   kind is 9..12; and then a hold's INSTALMENTS, by the same
   `width / step - 1` clamped up to 1 that m42fbf0 gives the sustain tick.

   THE COUNTER'S LADDER IS NOT THE STARTER'S. Kind 4 divides the beat by 32
   here (0x42f402) where @0x42fbf0 gives kinds 4 and 5 a single instalment
   of the note's whole length; 1, 2 and 3 agree at /2, /8 and /16, and so
   does the /4 default. Nothing reconciles them - they are two ladders. Kinds
   7..12 add nothing beyond the head either way, and under mix style 7, a
   negative option, or kind 6 the whole instalment calculation collapses to
   "+1 if this is a hold at all, and only for kind 6".

   Five separate query buffers, one per ladder arm, is what the original's
   0xd0 frame holds: cl gives each arm's own `int buf[8]` its own slot.

   98.25%, and the whole residue is WHERE THE ACCUMULATOR'S ZERO LANDS. The
   original materializes it in the prologue beside the loop's zero
   (`xor ebp,ebp; xor edi,edi` before the `lea`); cl emits ours just above
   the begin call, and pays one `jmp` over the loop's alignment padding for
   it, which the original falls straight through. Moving `total = 0` up in
   the source is exactly wrong - both an initializer at the declaration and
   a statement before the clearing loop measure 90.14%, because cl then
   makes edi the loop's zero as well and the whole prologue reshapes. The
   guarded form of the walk is worth 2.7 points on its own: written
   `if (begin(...) == 0) return total;` cl duplicates the epilogue inline
   (95.53%), where the original jumps to the one it shares with the loop's
   exit. */
// FUNCTION: EZ2AC_UNPACKED 0x0042f2d0
int PlayerSlot::m42f2d0(int h)
{
    SlotState st;
    SlotSound snd;
    int total;
    int i;

    for (i = 0; i < 18; i++)
    {
        t58[i] = 0;
        t130[i] = 0;
        te8[i] = 0;
        t178[i] = 0;
    }
    total = 0;
    fc = 0;
    if (((SlotSampleSink *)f3d4)->begin(h, 0, 0, 1, &st, &snd) != 0)
    {
    do
    {
        if (st.f4 == 1 && (st.kind < 9 || st.kind > 12))
            total += 1;
        if (st.kind >= 7 && st.kind <= 12)
            continue;
        if (g_mixStyle != 7 && g_1b5f300[g_elemCount] >= 0 && st.kind != 6)
        {
            int step;
            int n;

            if (st.width <= 6)
                continue;
            if (st.kind == 1)
            {
                int buf[8];

                ((SlotSampleSink *)f3d4)->query(buf, 4);
                step = buf[2] / 2;
            }
            else if (st.kind == 2)
            {
                int buf[8];

                ((SlotSampleSink *)f3d4)->query(buf, 4);
                step = buf[2] / 8;
            }
            else if (st.kind == 3)
            {
                int buf[8];

                ((SlotSampleSink *)f3d4)->query(buf, 4);
                step = buf[2] / 16;
            }
            else if (st.kind == 4)
            {
                int buf[8];

                ((SlotSampleSink *)f3d4)->query(buf, 4);
                step = buf[2] / 32;
            }
            else
            {
                int buf[8];

                ((SlotSampleSink *)f3d4)->query(buf, 4);
                step = buf[2] / 4;
            }
            n = st.width / step - 1;
            if (n <= 0)
                n = 1;
            total += n;
        }
        else if (st.width > 6 && st.kind == 6)
        {
            total += 1;
        }
    }
    while (((SlotSampleSink *)f3d4)->next(1, &st, &snd) != 0);
    }

    return total;
}

/* Total whatever the per-entry helper makes of this player's track list. The
   accumulator stays in memory: with the list pointer, the walking pointer, the
   counter and `this` all live, there is no register left for it. */
// FUNCTION: EZ2AC_UNPACKED 0x0042f480
int PlayerSlot::sumTracks()
{
    int total = 0;
    TrackList *t = &f3d0->tracks[id];
    int i;

    for (i = 0; i < t->count; i++)
        total += m42f2d0(t->items[i].handle);

    return total;
}

/* And the catch owner's: a plain 0..count walk where the keys and scratch
   owners walk their list's cursor to its end, over a 0x1bc list whose
   entries carry the handle in their FIRST word. */
// FUNCTION: EZ2AC_UNPACKED 0x0046d1f0
void CatchSlotOwner::claimTracks(CatchPlayer *p)
{
    CatchTrackList *t = &tracks[p->id];
    int i;

    for (i = 0; i < t->count; i++)
    {
        TrackEntry *e = &t->items[i];

        sounds[e->key].f0 = p->id;
        sounds[e->key].id = i;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00473600
int CatchPlayer::sumTracks()
{
    int total = 0;
    CatchTrackList *t = &f360->tracks[id];
    int i;

    for (i = 0; i < t->count; i++)
        total += m473400(t->items[i].key);

    return total;
}

// FUNCTION: EZ2AC_UNPACKED 0x00465ad0
int PlayerRecord::sumTracks()
{
    int total = 0;
    TrackList *t = &f1e0->tracks[id];
    int i;

    for (i = 0; i < t->count; i++)
        total += m465a70(t->items[i].handle);

    return total;
}

/* Stamp every sound slot this player's track list still owns with the player's
   id and the entry that claimed it, from the list's cursor to its end. The
   entry's handle is re-read for the second store because the first could have
   aliased it. */
// FUNCTION: EZ2AC_UNPACKED 0x00422ca0
void SlotOwner::claimTracks(ListNode *n)
{
    TrackList *t = &tracks[n->f8];
    int i;

    for (i = t->first; i < t->end; i++)
    {
        TrackEntry *e = &t->items[i];

        sounds[e->handle].f0 = n->f8;
        sounds[e->handle].id = i;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0045f110
void GFSlotOwner::claimTracks(ListNode *n)
{
    TrackList *t = &tracks[n->f8];
    int i;

    for (i = t->first; i < t->end; i++)
    {
        TrackEntry *e = &t->items[i];

        sounds[e->handle].f0 = n->f8;
        sounds[e->handle].id = i;
    }
}

/* Total this player's track list through the shared per-entry helper, report
   the total back to the player, then walk the list again notifying its sink.
   Both arguments are re-read from the frame at every call because the helper
   could have written them. */
// FUNCTION: EZ2AC_UNPACKED 0x00422bf0
void SlotOwner::applyTracks(PlayerSlot *p, int a, int b)
{
    TrackList *t = &tracks[p->id];
    int total = 0;
    int i;

    for (i = t->first; i < t->end; i++)
    {
        int h = t->items[i].handle;

        total += m422b60(h, 5, 5, a, b);
    }

    p->m42e620(5, total, 1);

    for (i = t->first; i < t->end; i++)
        p->f3cc->m42d370(i, 5, 1);
}

// FUNCTION: EZ2AC_UNPACKED 0x0045f060
void GFSlotOwner::applyTracks(PlayerRecord *p, int a, int b)
{
    TrackList *t = &tracks[p->id];
    int total = 0;
    int i;

    for (i = t->first; i < t->end; i++)
    {
        int h = t->items[i].handle;

        total += m422b60(h, 5, 5, a, b);
    }

    p->m465f60(5, total, 1);

    for (i = t->first; i < t->end; i++)
        p->f1dc->m4651a0(i, 5, 1);
}

/* Is the first player's track list finished? Take the head of the list - the
   walker's own result is ignored, only the node it writes back is used - and
   ask the clock about every entry from the cursor to the end. The scratch game
   asks list C instead of list A, and everything else is the same.

   The `return 1` is the fall-through and the `return 0` the out-of-line tail,
   which is what an early return inside the loop gives. */
// FUNCTION: EZ2AC_UNPACKED 0x0041f990
int SlotOwner::tracksIdle()
{
    ListNode *cur = 0;
    TrackList *t;
    int i;

    nextListA(&cur);
    t = &tracks[cur->f8];

    for (i = t->first; i < t->end; i++)
    {
        if (clock->busy(t->items[i].handle) != 0)
            return 0;
    }

    return 1;
}

/* The catch owner's copy: list B rather than list A, its own 0x1bc list
   walked by COUNT from zero where the keys owner walks first..end, and the
   clock four words further along. twins.py had it at 69%. */
// FUNCTION: EZ2AC_UNPACKED 0x0046bf30
int CatchSlotOwner::tracksIdle()
{
    ListNode *cur = 0;
    CatchTrackList *t;
    int i;

    nextListB(&cur);
    t = &tracks[cur->f8];

    for (i = 0; i < t->count; i++)
    {
        if (clock->busy(t->items[i].key) != 0)
            return 0;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045d080
int GFSlotOwner::tracksIdle()
{
    ListNode *cur = 0;
    TrackList *t;
    int i;

    nextListC(&cur);
    t = &tracks[cur->f8];

    for (i = t->first; i < t->end; i++)
    {
        if (clock->busy(t->items[i].handle) != 0)
            return 0;
    }

    return 1;
}

/* Walk this player's track list and give every hit of every entry a fresh
   playback rate between 1.0 and 2.0, stopping at the time limit. The sink
   fills both blocks for an entry, then hands back one hit at a time.

   The inner loop's FIRST test is an exit and its SECOND jumps back, which is
   the fingerprint of a real do-while with a break in it rather than a while
   cl rotated. */
// FUNCTION: EZ2AC_UNPACKED 0x00422a60
void SlotOwner::randomizeHits(ListNode *n, int a, unsigned int limit)
{
    SlotSound s;
    SlotState st;
    TrackList *t = &tracks[n->f8];
    int i;

    i = t->first;
    if (i < t->end)
    {
        /* Anchored on the HANDLE, not on the entry: the original walks with a
           zero displacement and a 0x10 stride, which is what indexing the
           entry and reading its +8 would not give. */
        const int *h_p = &t->items[i].handle;

        do
        {
            int h = *h_p;

            if (sink->begin(h, a, 0, 1, &st, &s) != 0)
            {
                int more;

                do
                {
                    st.rate = (float)(rand() % 100) * 0.01f + 1.0f;
                    sink->setState(&s, &st);
                    more = sink->next(1, &st, &s);
                    if ((unsigned int)st.start > limit)
                        break;
                }
                while (more != 0);
            }

            i++;
            h_p += 4;
        }
        while (i < t->end);
    }
}

/* Walk this player's track list and give every hit of every entry a fresh
   playback rate between 1.0 and 2.0, stopping at the time limit. The sink
   fills both blocks for an entry, then hands back one hit at a time.

   The inner loop's FIRST test is an exit and its SECOND jumps back, which is
   the fingerprint of a real do-while with a break in it rather than a while
   cl rotated. */
// FUNCTION: EZ2AC_UNPACKED 0x0045ef60
void GFSlotOwner::randomizeHits(ListNode *n, int a, unsigned int limit)
{
    SlotSound s;
    SlotState st;
    TrackList *t = &tracks[n->f8];
    int i;

    i = t->first;
    if (i < t->end)
    {
        /* Anchored on the HANDLE, not on the entry: the original walks with a
           zero displacement and a 0x10 stride, which is what indexing the
           entry and reading its +8 would not give. */
        const int *h_p = &t->items[i].handle;

        do
        {
            int h = *h_p;

            if (sink->begin(h, a, 0, 1, &st, &s) != 0)
            {
                int more;

                do
                {
                    st.rate = (float)(rand() % 100) * 0.01f + 1.0f;
                    sink->setState(&s, &st);
                    more = sink->next(1, &st, &s);
                    if ((unsigned int)st.start > limit)
                        break;
                }
                while (more != 0);
            }

            i++;
            h_p += 4;
        }
        while (i < t->end);
    }
}

/* Which entry of this player's track list carries a key, or -1. The not-found
   tail is the fall-through and the hit jumps past it, which is what an early
   `return i` from inside the loop gives. */
// FUNCTION: EZ2AC_UNPACKED 0x0042eee0
int PlayerSlot::findTrack(int key)
{
    TrackList *t = &f3d0->tracks[id];
    int n = t->count;
    int i = 0;

    if (n > 0)
    {
        const TrackEntry *e = t->items;

        do
        {
            if (e->key == key)
                return i;

            i++;
            e++;
        }
        while (i < n);
    }

    return -1;
}

/* The chart player the slot holds at +0x3d4, as m42f080 sees it: vtable
   slot 8 fills a note-info block (0x80 = by the index stored just below the
   block, 4 = the current note's timing info), and the hold-note table lives
   at +0x1d1c, 0x28 bytes per note. */
struct ChartPlayerV
{
    virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
    virtual void s4(); virtual void s5(); virtual void s6(); virtual void s7();
    virtual void s8(void *out, int flags);
};

struct HoldNote
{
    unsigned int start;        /* 0x00 */
    char f4;                   /* 0x04 */
    unsigned char lane;        /* 0x05 */
    char _pad6[0x14 - 0x6];
    short type;                /* 0x14 */
    char _pad16[0x1c - 0x16];
    int handle;                /* 0x1c */
    char _pad20[0x28 - 0x20];
};

struct ChartPlayerNotes
{
    char _pad0[0x1d1c];
    HoldNote notes[1];
};

/* The note-info block slot 8 fills: only the count at +0x1c is read here. */
struct HoldQuery
{
    int f0; int f4; int f8; int fc;
    int f10; int f14; int f18;
    int count;                 /* 0x1c */
};

struct HoldTiming
{
    int f0; int f4;
    unsigned int f8;           /* 0x08 - the note's spacing base */
    int fc;
    int f10; int f14; int f18; int f1c;
};

int pickSample42de50();

extern int g_eventMode;   /* 0x1696404 (src/settings.cpp) */

/* Give each hold note a sample handle, skipping notes that start too close
   to the previous claim. The spacing limit is the note's own timing base,
   half again as long - or, in event mode, a randomized 8..15x multiple. */
// FUNCTION: EZ2AC_UNPACKED 0x0042f080
void PlayerSlot::m42f080()
{
    int last;
    int qIdx;
    HoldTiming ht;
    HoldQuery q;
    ChartPlayerV *p = (ChartPlayerV *)f3d4;
    HoldNote *note;

    last = 0;
    note = ((ChartPlayerNotes *)f3d4)->notes;
    qIdx = 0;
    p->s8(&q, 0x80);
    if (q.count > 0)
    {
        do
        {
            TrackList *t = &f3d0->tracks[id];
            int n = t->count;
            int k = 0;

            if (n > 0)
            {
                int lane = note->lane;
                const int *e = &t->items[0].handle;

                do
                {
                    if (lane == *e)
                    {
                        int v;
                        int lim;

                        ((ChartPlayerV *)f3d4)->s8(&ht, 4);
                        v = ht.f8;
                        if (g_eventMode != 0)
                            lim = (rand() % 8 + 8) * v;
                        else
                            lim = v + (int)((unsigned int)v >> 1);
                        if (note->start > (unsigned int)(last + lim))
                        {
                            last = note->start;
                            note->handle = pickSample42de50();
                        }
                        else
                        {
                            note->handle = -1;
                        }
                        break;
                    }
                    k++;
                    e += 4;
                }
                while (k < n);
            }

            note++;
            qIdx = qIdx + 1;
            p->s8(&q, 0x80);
        }
        while (qIdx < q.count);
    }
}

/* The type-6 sibling of m42f080: only word-type-6 notes on movable lanes are
   claimed, always into slot 0x15, and the spacing limit runs eight times as
   long. */
// FUNCTION: EZ2AC_UNPACKED 0x0042f1a0
void PlayerSlot::m42f1a0()
{
    int last;
    int qIdx;
    HoldTiming ht;
    HoldQuery q;
    ChartPlayerV *p = (ChartPlayerV *)f3d4;
    HoldNote *note;

    last = 0;
    note = ((ChartPlayerNotes *)f3d4)->notes;
    qIdx = 0;
    p->s8(&q, 0x80);
    if (q.count > 0)
    {
        do
        {
            TrackList *t = &f3d0->tracks[id];
            int n = t->count;
            int k = 0;

            if (n > 0)
            {
                int lane = note->lane;
                const int *e = &t->items[0].handle;

                do
                {
                    if (lane == *e)
                    {
                        if (note->type == 6)
                        {
                            int v;
                            int lim;

                            ((ChartPlayerV *)f3d4)->s8(&ht, 4);
                            v = ht.f8;
                            if (g_eventMode != 0)
                                lim = (rand() % 8 + 8) * v;
                            else
                                lim = v + (int)((unsigned int)v >> 1);
                            if (note->start > (unsigned int)(last + lim * 8))
                            {
                                if (laneMovable((SlotState *)note) != 0)
                                {
                                    last = note->start;
                                    note->handle = 0x15;
                                    break;
                                }
                            }
                        }
                        note->handle = -1;
                        break;
                    }
                    k++;
                    e += 4;
                }
                while (k < n);
            }

            note++;
            qIdx = qIdx + 1;
            p->s8(&q, 0x80);
        }
        while (qIdx < q.count);
    }
}

/* Walk one catch track list and shorten every hit wider than 0x1e by 0x12,
   stopping each track's walk once the hits pass `limit`. The keys owner's
   randomizeHits @0x422a60 has the same skeleton and reskins instead, and
   takes a list NODE where this takes the list itself.

   The counter is zeroed before the guard and the walking pointer set up
   inside it, and the count is re-read from the list at the bottom - the
   three-part loop shape. */
// FUNCTION: EZ2AC_UNPACKED 0x0046cdc0
void CatchSlotOwner::m46cdc0(CatchTrackList *t, int a, unsigned int limit)
{
    SlotSound snd;
    SlotState st;
    int i;

    i = 0;
    if (i < t->count)
    {
        const TrackEntry *e = t->items;

        do
        {
            int r = sink->begin(e->key, a, 0, 1, &st, &snd);

            while (r == 1)
            {
                if ((unsigned int)st.start > limit)
                    break;

                if (st.width >= 0x1e)
                {
                    st.width = (unsigned short)(st.width - 0x12);
                    sink->setState(&snd, &st);
                }

                r = sink->next(1, &st, &snd);
            }

            i++;
            e++;
        }
        while (i < t->count);
    }
}

/* THE EVENT DISPATCHER - EZ2DJMainGameDirector hands every lane event here
   (ten call sites @0x41ff01..0x42027c): kind 3 is a held key, pumped through
   m42f880 with this player's f210; kind 2 a PRESS and kind 1 a RELEASE, each
   judged against the sound the lane's track entry carries at the clock's
   current tick, and committed through m42fd90 when the verdict is nonzero -
   flag 1 for the press (the arm that starts a hold at its head), flag 0 for
   the release (the arm that closes it). A press always sounds the lane's
   nearest keysound (KEZPlayer slot 32, reseedLane @0x40ffa0), hit or miss; a
   release judged 2 stops the lane's voice (slot 33 @0x4100d0). The C had
   kinds 1 and 2 labelled the other way round until 2026-09-28. The verdict is written into the
   SlotSound's FIRST word (`&s.f0`, not the id). Every kind ends by retiring
   the lane's effect state through the sink. */
// FUNCTION: EZ2AC_UNPACKED 0x00430970
void PlayerSlot::handleEvent(int lane, int kind)
{
    SlotSound s;
    int handle = f3d0->tracks[id].items[lane].handle;
    int r;

    s.f0 = -1;
    s.id = -1;
    if (kind == 3)
    {
        m42f880(lane, f210);
    }
    else if (kind == 2)
    {
        Judge *j = (Judge *)f3dc;
        r = j->judgePress(((SlotClock *)j->f24e0)->now(), handle, 1, 0, &s);
        ((SlotClock *)f3d8)->reseedLane(handle);
        if (r != 0)
            m42fd90(lane, &s, r, 1);
    }
    else if (kind == 1)
    {
        Judge *j = (Judge *)f3dc;
        r = j->judgeRelease(((SlotClock *)j->f24e0)->now(), handle, 1, 0, &s);
        if (r != 0)
        {
            if (r == 2)
                ((SlotClock *)f3d8)->stopLane(handle);
            m42fd90(lane, &s, r, 0);
        }
    }
    f3cc->m4299e0(lane, kind);
}

/* The per-element scores the total-result screen reads. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f0b0
int g_elemScorePlus[5];
extern int g_1b2e590[5];   /* src/totalresultctor.cpp */

/* PlayerSlot::m42ef30 @0x42ef30 - publish each element's block word 9 and
   return their total, over elements 0..g_elemCount inclusive. */
// FUNCTION: EZ2AC_UNPACKED 0x0042ef30
int PlayerSlot::m42ef30()
{
    int total = 0;
    int i;

    i = 0;
    if (i <= g_elemCount)
    {
        const int *p = &block[9];
        do
        {
            int v = *p;
            g_1b2e590[i] = v;
            i++;
            total += v;
            p += 16;
        }
        while (i <= g_elemCount);
    }
    return total;
}

/* PlayerSlot::m42ef70 @0x42ef70 - the same over 0..g_elemCount-1, the
   total taking words 9 and 10 and the table word 9.
   40.91%: every instruction present, four registers renamed - the original
   walks the block in ecx (this, reused), counts in edi and holds a/b in
   esi/ebx; cl gives the walk esi, the count ebx and a/b edi/ecx. Indexed
   and pointer forms, named/unnamed b, a named count and declaration
   orders all give the same assignment. */
// FUNCTION: EZ2AC_UNPACKED 0x0042ef70
int PlayerSlot::m42ef70()
{
    int i;
    int total;
    int n = g_elemCount;

    total = 0;
    i = 0;
    if (i < n)
    {
        /* 40.91%, and every instruction is present - the whole difference is
           that the original strength-reduces `this` ITSELF into the walk
           (`add ecx,0x254`, then `add ecx,0x40` per step), destroying it,
           while cl copies it to esi and keeps ecx alive. That costs one
           register, which rotates the other three: the count is edi in the
           original and ebx here, the value esi against edi.

           Indexing rather than hand-walking a pointer - the documented fix
           for "the walking pointer must come from cl's strength reduction" -
           compiles to byte-identical output, so it is not that. Kept in the
           indexed form anyway, as the more faithful reading.

           TEN MORE FORMS SWEPT 2026-08-25, ALL 40.91% OR WORSE. The whole
           loop is one register permutation - original {ecx=walker, esi=a,
           ebx=b, edi=n}, ours {esi=walker, edi=a, ecx=b, ebx=n} - with the
           same three callee-saved registers pushed in a different order, so
           there is nothing to add or remove, only an assignment to flip.
           Anchoring: an explicit `const int *p = &block[9]` read as p[0]/p[1]
           (the cookbook's re-anchoring fix), with the bump before or after
           the accumulate, is byte-identical to the indexed form - cl anchors
           at +0x258 and reads [-4]/[0] whatever the source says. Reading the
           +10 field FIRST is 50.0% but two instructions long; dropping the
           `b` local is 36.4%. All six declaration orders of i/total/n give
           the identical listing. This is the coin, on 22 instructions. */
        do
        {
            int a = block[i * 16 + 9];
            int b = block[i * 16 + 10] + a;
            g_elemScorePlus[i] = a;
            i++;
            total += b;
        }
        while (i < n);
    }
    return total;
}

// FUNCTION: EZ2AC_UNPACKED 0x00465a70
int PlayerRecord::m465a70(int h)
{
    SlotState st;
    SlotSound sn;
    int n = 0;

    if (((SlotSampleSink *)f1e4)->begin(h, 0, 0, 1, &st, &sn) != 0)
    {
        do
        {
            if (st.f4 == 1)
                n++;
        }
        while (((SlotSampleSink *)f1e4)->next(1, &st, &sn) != 0);
    }
    return n;
}

/* Whether the catch game judges at all this stage (the song select sets it);
   off, every hit counts one and the per-lane tables are left alone. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e58c
int g_catchJudging;
/* The sink's own summary getter (src/accessors.cpp): its slot-8 query,
   third word. */
struct Obj422DD0 { int get(); };

/* CatchPlayer::m473400 @0x473400 - count sound h's judgeable hits, the
   catch dialect of PlayerSlot::m42f2d0: with judging on, clear the four
   per-lane tables and fc first and walk the hits with a state block; a kind
   1 hit outside 9..12 counts one; kinds 7..12 count nothing more; outside
   style 7 with the player's rung set and kind not 6, a wide hit (> 6) counts
   its width over the period's share (/2, /6, /8 by the kind-specific query,
   /16, /12, /4 by the sink's getter) less one, at least one; else a wide
   kind-6 hit counts one. With judging off the hits are walked without a
   state block and each counts one.
   88.17%: the judging-off path's next() is its own call here where the
   original cross-jumps it into the judging path's (its vtable load lands in
   eax instead of edx, so the tails differ); an if/else pair of next() calls
   at the loop end, a ternary of calls, and `continue` forms all make cl
   unswitch the loop instead. */
// FUNCTION: EZ2AC_UNPACKED 0x00473400
int CatchPlayer::m473400(int h)
{
    SlotState st;
    SlotSound snd;
    int total = 0;
    int i;
    int r;

    if (g_catchJudging != 0)
    {
        for (i = 0; i < 18; i++)
        {
            t58[i] = 0;
            t130[i] = 0;
            te8[i] = 0;
            t178[i] = 0;
        }
        fc = 0;
    }
    if (g_catchJudging != 0)
        r = ((SlotSampleSink *)f364)->begin(h, 0, 0, 1, &st, &snd);
    else
        r = ((SlotSampleSink *)f364)->begin(h, 0, 0, 1, 0, &snd);
    if (r != 0)
    {
    do
    {
        if (g_catchJudging != 0)
        {
            if (st.f4 == 1 && (st.kind < 9 || st.kind > 12))
                total += 1;
            if (st.kind < 7 || st.kind > 12)
            {
                if (g_mixStyle != 7 && g_1b5f300[g_elemCount] >= 0 && st.kind != 6)
                {
                    if (st.width > 6)
                    {
                        int step;
                        int n;

                        if (st.kind == 1)
                        {
                            int buf[8];

                            ((SlotSampleSink *)f364)->query(buf, 4);
                            step = buf[2] / 2;
                        }
                        else if (st.kind == 2)
                        {
                            int buf[8];

                            ((SlotSampleSink *)f364)->query(buf, 4);
                            step = buf[2] / 6;
                        }
                        else if (st.kind == 3)
                        {
                            int buf[8];

                            ((SlotSampleSink *)f364)->query(buf, 4);
                            step = buf[2] / 8;
                        }
                        else if (st.kind == 4)
                        {
                            step = ((Obj422DD0 *)f364)->get() / 16;
                        }
                        else if (st.kind == 5)
                        {
                            step = ((Obj422DD0 *)f364)->get() / 24;
                        }
                        else
                        {
                            step = ((Obj422DD0 *)f364)->get() / 4;
                        }
                        n = st.width / step - 1;
                        if (n <= 0)
                            n = 1;
                        total += n;
                    }
                }
                else if (st.width > 6 && st.kind == 6)
                {
                    total += 1;
                }
            }
            r = ((SlotSampleSink *)f364)->next(1, &st, &snd);
        }
        else
        {
            total += 1;
            r = ((SlotSampleSink *)f364)->next(1, 0, &snd);
        }
    }
    while (r != 0);
    }

    return total;
}

/* CatchPlayer::reset @0x473300 - the catch dialect of PlayerSlot::reset:
   id, the totals, the links and the five owner pointers cleared, the four
   per-lane tables, the per-element block (0x1f4..0x35c as one wipe), the
   per-judge counters and the combo/life state - the life to 100 with an
   owner still attached, else 0.
   100%* : cl saves edi in the prologue where the original saves it just
   before the wipe that uses it - one save-slot schedule, no byte moved
   anywhere else; three shapes tried. */
struct CatchResetPlayer
{
    char _pad0[0x1c0];
    int  f1c0;           /* 0x1c0 */
    int  f1c4[6];        /* 0x1c4 .. 0x1dc */
    int  f1dc, f1e0, f1e4, f1e8;
    float life;          /* 0x1ec */
    int  f1f0;
    int  blk[0x5a];      /* 0x1f4 .. 0x35c - the per-element block */
    char _pad35c[0x548 - 0x35c];
    int  f548, f54c, f550, f554, noteCount;   /* 0x548 .. 0x55c */
};

// FUNCTION: EZ2AC_UNPACKED 0x00473300
void CatchPlayer::reset(int newId)
{
    CatchResetPlayer *me = (CatchResetPlayer *)this;
    int i;
    float life;

    id = newId;
    me->f548 = 0;
    next = 0;
    f35c = 0;
    f360 = 0;
    f364 = 0;
    f368 = 0;
    f36c = 0;
    ((int *)this)[0x370 / 4] = 0;

    for (i = 0; i < 18; i++)
    {
        t58[i] = 0;
        t130[i] = 0;
        te8[i] = 0;
        t178[i] = 0;
    }

    life = 100.0f;
    memset(me->blk, 0, sizeof(me->blk));
    me->f1f0 = 0;
    memset(me->f1c4, 0, sizeof(me->f1c4));
    me->noteCount = 0;
    me->f54c = 0;
    me->f550 = 0;
    me->f548 = 0;
    me->f1e8 = 0;
    me->f1e4 = 0;
    me->f1e0 = 0;
    me->f1dc = 0;
    me->f1c0 = 0;
    if (f360 == 0)
        life = 0.0f;
    me->life = life;
}

/* SlotOwner::m4215a0 @0x4215a0 - does time t land inside one of sound h's
   wide hits? The sink's begin with mode 3 walks nothing - only the first
   block is read: a hold (width >= 6) whose start..start+width+8 spans t
   says yes. */
// FUNCTION: EZ2AC_UNPACKED 0x004215a0
int SlotOwner::m4215a0(int h, unsigned int t)
{
    SlotState st;
    SlotSound sn;

    if (sink->begin(h, t, 3, 1, &st, &sn) != 0 &&
        st.width >= 6 &&
        t >= (unsigned int)st.start &&
        t <= (unsigned int)st.start + st.width + 8)
    {
        return 1;
    }
    return 0;
}

/* PlayerRecord::handleEvent @0x466da0 - the GF (ScratchMix) twin of
   PlayerSlot::handleEvent @0x430970 above, on the list-C player record.

   The same two judged kinds (2 = press, 1 = release) against the same Judge, reached through this
   class's own pointers - the track owner at +0x1e0, the clock at +0x1e8, the
   judge at +0x1ec - where the EZ2DJ slot keeps them at +0x3d0/+0x3d8/+0x3dc.

   Two things it does NOT do. There is no kind 3: the scratch game has no
   held-key pump here, so the dispatch is two arms rather than three. And
   there is no trailing sink call - PlayerSlot retires the lane's effect
   state through f3cc->m4299e0 on every kind, this one does not retire
   anything. */
// FUNCTION: EZ2AC_UNPACKED 0x00466da0
void PlayerRecord::handleEvent(int lane, int kind)
{
    SlotSound s;
    int handle = f1e0->tracks[id].items[lane].handle;
    int r;

    s.f0 = -1;
    s.id = -1;
    if (kind == 2)
    {
        Judge *j = (Judge *)f1ec;
        r = j->judgePress(((SlotClock *)j->f24e0)->now(), handle, 1, 0, &s);
        ((SlotClock *)f1e8)->reseedLane(handle);
        if (r != 0)
            m466ac0(lane, &s, r, 1);
    }
    else if (kind == 1)
    {
        Judge *j = (Judge *)f1ec;
        r = j->judgeRelease(((SlotClock *)j->f24e0)->now(), handle, 1, 0, &s);
        if (r != 0)
        {
            if (r == 2)
                ((SlotClock *)f1e8)->stopLane(handle);
            m466ac0(lane, &s, r, 0);
        }
    }
}

extern int       countListA();              /* 0x418310 (src/nodelists.cpp) */
extern ListNode *findListA(int key);        /* 0x418e30 (src/nodelists.cpp) */
extern float     g_playerScale[2];          /* 0x1b2ef20 (src/gamepanels.cpp) */
extern int       g_elemCount;               /* 0x1b2e718 */

/* Is this slot's run over? Normal gauge mode: over when a life has hit
   zero - either list-A player's when two are up, this slot's own
   otherwise. Survival (f3d0's selector at 1): over when the published
   scale has reached the second gauge. */
// FUNCTION: EZ2AC_UNPACKED 0x0042e590
int PlayerSlot::m42e590()
{
    float v;

    /* The two final tests RETURN their comparisons: that is what lays the
       `return 1` inline after the gauge test (jp over it) and the survival
       `return 0` after its jae - goto-into-a-shared-win shapes put both
       arms the other way round. */
    if (f3d0->f794 == 0)
    {
        if (countListA() >= 2)
        {
            if (((PlayerSlot *)findListA(0))->gauge == 0.0f)
                return 1;
            v = ((PlayerSlot *)findListA(1))->gauge;
        }
        else
            v = gauge;
        return v == 0.0f;
    }
    if (f3d0->f794 == 1)
        return g_playerScale[g_elemCount] >= gauge2;
    return 0;
}
