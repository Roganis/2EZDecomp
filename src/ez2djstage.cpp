/* EZ2DJMainGameDirector::m4246d0 @0x4246d0 - begin a stage.

   Release the previous run's slots, bump the element counter, and load the
   chart through m423a90 @0x423a90 (src/ez2djload.cpp); a load that fails
   hands 1 to the object at g_1b2e844 and gives up. Otherwise the timed
   object's ten chart heads are reset and the run is started under whichever
   option kind is set - a ten-arm ladder over the g_kind globals, each arm
   naming the kind it passes to m422f20 and then starting. The last arm and
   the no-kind case share the plain start. */

#include "ezscreen.h"

/* Whatever g_1b2e844 points at; only its slot 0 is reached. */
struct Obj1B2E844 { virtual void q00(int v); };

/* The timed object's reset, under the name src/misc.cpp defines it with. */
struct Obj41B4A0 { void reset(); };   /* 0x41b4a0 */

extern void *g_1b2e844;     /* src/moderunner.cpp */
extern int  g_elemCount;    /* 0x1b2e718 (src/loops.cpp) */

extern int g_kind8c8, g_kind990, g_kind8c0, g_kind8d0, g_kind958;
extern int g_kind960, g_kind970, g_kind978;   /* src/maingame.cpp */
extern int g_1b2e9a0, g_1b2e998;              /* src/maingame.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x004246d0
void EZ2DJMainGameDirector::m4246d0()
{
    m420ec0();
    m4209d0();
    g_elemCount += 1;

    if (m423a90() == 0)
    {
        ((Obj1B2E844 *)g_1b2e844)->q00(1);
        return;
    }

    m423ed0();
    ((Obj41B4A0 *)timed)->reset();

    if (g_kind8c8 != 0)
    {
        m422f20(4, 1);
        start();
        return;
    }
    if (g_kind990 != 0)
    {
        m422f20(0x1d, 1);
        start();
        return;
    }
    if (g_kind8c0 != 0)
    {
        m422f20(3, 1);
        start();
        return;
    }
    if (g_kind8d0 != 0)
    {
        m422f20(5, 1);
        start();
        return;
    }
    if (g_kind958 != 0)
    {
        m422f20(0x16, 1);
        start();
        return;
    }
    if (g_1b2e9a0 != 0)
    {
        m422f20(0x1f, 1);
        start();
        return;
    }
    if (g_kind960 != 0)
    {
        m422f20(0x17, 1);
        start();
        return;
    }
    if (g_1b2e998 != 0)
    {
        m422f20(0x1e, 1);
        start();
        return;
    }
    if (g_kind970 != 0)
    {
        m422f20(0x19, 1);
        start();
        return;
    }
    if (g_kind978 != 0)
        m422f20(0x1a, 1);

    start();
}

#include "players.h"   /* ListNode, SlotSampleSink */

/* EZ2DJMainGameDirector::m422870 @0x422870 - mirror the player's playable
   lanes over a time range: the EZ2DJ twin of CatchMainGameDirector::m46cfc0
   @0x46cfc0 and m46d0f0 @0x46d0f0 (src/catchdirector.cpp).

   Same collect-then-mirror, and the same three moves through scratch lane
   0x15 per pair, over the same six kinds m46d0f0 takes. Two things differ,
   and both are the EZ2DJ record's shape rather than a change of intent:

     - its per-player record is 113 ints where Catch's is 111, and it starts
       at +0x380 rather than +0x34c;
     - it carries an explicit START as well as an end - rec[3] and rec[2],
       against Catch's single count at rec[1] - so the walk begins at entry
       `start` and runs `end - start` times instead of starting at the first
       entry every time. Catch's `count - 1` / `count - 2` are that same
       range with the start folded in.

   The two extra header ints are also why the entries sit two ints further
   in: kind at rec[(start+1)*4], lane two ints after it. */
// FUNCTION: EZ2AC_UNPACKED 0x00422870
void EZ2DJMainGameDirector::m422870(ListNode *n, int a, int b)
{
    int  lanes[16];
    int *rec = &f380[n->f8 * 113];
    int  start = rec[3];
    int  out = 0;
    int  j;

    if (start < rec[2])
    {
        int *p = &rec[(start + 1) * 4];
        int  k = rec[2] - start;

        do
        {
            int kind = *p;

            if (kind == 0xf || kind == 0x10 || kind == 0x17 ||
                kind == 0x18 || kind == 0x11 || kind == 0x19)
            {
                lanes[out] = p[2];
                out++;
            }

            p += 4;
        }
        while (--k != 0);
    }

    j = 0;
    if (out / 2 > 0)
    {
        int *hi = &lanes[out - 1];

        do
        {
            int h = *hi;
            int lo = lanes[j];

            m421510(lo, 0x15, a, b);
            m421510(h, lo, a, b);
            m421510(0x15, h, a, b);

            j++;
            hi--;
        }
        while (j < out / 2);
    }

    sink->commit();
}

extern int nextListA(struct ListNode **cur);   /* 0x4182e0 (src/nodelists.cpp) */

/* EZ2DJMainGameDirector::m422d30 @0x422d30 - step every present player's
   per-lane countdowns.

   For each player in list A, each of its eighteen lanes with a countdown
   still above zero is decremented, and the lane that has just reached zero
   is retired through that player's track sink - the same @0x4299e0 the event
   dispatcher calls, here always with 1, the release code.

   The counters live in this director rather than in the player: `fc80` is
   two players of eighteen, indexed `id * 18 + lane`, which is the array
   @0x422d10's setter writes and which identifies that address-named class as
   this one.

   The index is written out at all THREE sites rather than hoisted - the
   original re-reads the node's id and recomputes `id*9*2 + lane` for the
   test, the decrement and the zero check, and reloads the node itself after
   the sink call. */
// FUNCTION: EZ2AC_UNPACKED 0x00422d30
void EZ2DJMainGameDirector::m422d30()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            int lane;

            for (lane = 0; lane < 18; lane++)
            {
                if (fc80[cur->f8 * 18 + lane] > 0)
                {
                    fc80[cur->f8 * 18 + lane] -= 1;

                    if (fc80[cur->f8 * 18 + lane] == 0)
                        ((PlayerSlot *)cur)->f3cc->m4299e0(lane, 1);
                }
            }
        }
        while (nextListA(&cur) != 0);
    }
}

/* GFMainGameDirector::m45f1a0 @0x45f1a0 - the scratch game's copy of
   m422d30 above, and a structural duplicate of it: the same walk of list A,
   the same eighteen lane countdowns per player, the same decrement and the
   same expiry notice to the player's own sink. Only the three addresses
   differ - the table is at +0xc14 rather than +0xc80, the sink hangs off the
   player's +0x1dc rather than +0x3cc, and it is told through m462760 rather
   than m4299e0 - and it walks list C where the keys copy walks list A. */
// FUNCTION: EZ2AC_UNPACKED 0x0045f1a0
void GFMainGameDirector::m45f1a0()
{
    ListNode *cur = 0;

    if (nextListC(&cur) != 0)
    {
        do
        {
            int lane;

            for (lane = 0; lane < 18; lane++)
            {
                if (fc14[cur->f8 * 18 + lane] > 0)
                {
                    fc14[cur->f8 * 18 + lane] -= 1;

                    if (fc14[cur->f8 * 18 + lane] == 0)
                        ((PlayerRecord *)cur)->f1dc->m462760(lane, 1);
                }
            }
        }
        while (nextListC(&cur) != 0);
    }
}

/* EZ2DJMainGameDirector::m423ed0 @0x423ed0 - BEGIN THE STAGE for every live
   player: re-stamp that player's track list, mark it live, point it back at
   the player, and let the player set itself up.

   AND IT SETTLES A NAMING QUESTION. It calls SlotOwner::claimTracks
   @0x422ca0 and PlayerSlot::beginStage @0x4305a0 with the DIRECTOR's own
   `this`, and reaches the track list at the director's +0x380 with
   SlotOwner's 0x1c4 stride - so the director and the "SlotOwner" this tree
   models separately are ONE OBJECT under two names. The other offsets agree
   outright: the director's f16c[0x60] is 0x16c..0x2ec, which is SlotOwner's
   sounds[48], and both call +0x320 the run's base scroll rate. Merging the
   two models is a bigger change than this function, so it casts; the finding
   is recorded rather than acted on.

   The list element's +0x130 is the player pointer - the same slot the lane
   sweep m41ffa0 @0x41ffa0 reads its PlayerSlot out of, which is what says
   TrackList's items[] runs to eighteen entries and not the twenty-seven
   include/players.h currently gives it. Written through a cast for the same
   reason. */
// FUNCTION: EZ2AC_UNPACKED 0x00423ed0
void EZ2DJMainGameDirector::m423ed0()
{
    ListNode *cur = 0;

    if (nextListA(&cur) != 0)
    {
        do
        {
            ListNode  *n = cur;
            TrackList *t = &((SlotOwner *)this)->tracks[n->f8];

            ((SlotOwner *)this)->claimTracks(n);
            t->f0 = 1;
            *(ListNode **)((char *)t + 0x130) = n;
            ((PlayerSlot *)n)->beginStage((SlotOwner *)this);
        }
        while (nextListA(&cur) != 0);
    }
}

/* The other two families' copies of m423ed0, kept beside it because they are
   one source read three times and the deltas are the whole content.

   scratch: list C, the owner's lists 0x40 lower at +0x340, claimTracks
            @0x45f110, and the player set up through PlayerRecord::beginStage
            @0x466700.
   catch:   list B, lists at +0x34c with the catch stride of 0x1bc rather than
            0x1c4 - so the player pointer sits at +0x128, eight lower, exactly
            as the two strides differ - claimTracks @0x46d1f0 (which takes the
            player, not a bare node), and CatchPlayer::beginStage @0x473fd0. */

// FUNCTION: EZ2AC_UNPACKED 0x004600a0
void GFMainGameDirector::m4600a0()
{
    ListNode *cur = 0;

    if (nextListC(&cur) != 0)
    {
        do
        {
            ListNode  *n = cur;
            TrackList *t = &((GFSlotOwner *)this)->tracks[n->f8];

            ((GFSlotOwner *)this)->claimTracks(n);
            t->f0 = 1;
            *(ListNode **)((char *)t + 0x130) = n;
            ((PlayerRecord *)n)->beginStage((GFSlotOwner *)this);
        }
        while (nextListC(&cur) != 0);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046e4c0
void CatchMainGameDirector::m46e4c0()
{
    ListNode *cur = 0;

    if (nextListB(&cur) != 0)
    {
        do
        {
            ListNode       *n = cur;
            CatchTrackList *t = &((CatchSlotOwner *)this)->tracks[n->f8];

            ((CatchSlotOwner *)this)->claimTracks((CatchPlayer *)n);
            t->f0 = 1;
            *(ListNode **)((char *)t + 0x128) = n;
            ((CatchPlayer *)n)->beginStage((CatchSlotOwner *)this);
        }
        while (nextListB(&cur) != 0);
    }
}
