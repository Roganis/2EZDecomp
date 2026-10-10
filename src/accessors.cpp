#include "ez2acapp.h" /* KGraphics */
#include "ksongplayer.h"
#include "obj40d4e0.h"
/* Trivial reconstructed accessors (getters/setters). Offsets are hard facts;
   class identities are provisional (TBD). Each __thiscall member takes `this`
   in ecx. */

#include <windows.h>
#include <string.h>
#include "kprofile.h"
#include "kbase.h"
#include "bigbase.h"
#include "loader.h"   /* Log */
#include "obj402860.h"
#include "kparser.h"   /* KParser::getField @0x413260 */
#include "players.h"   /* the three rebaseSounds and the list walkers */
#include "ezscreen.h"   /* DrawNode */

/* (The three adjacent int getters @0x0040f380/90/a0 are vtable slots 5, 6 and
   7 of the class @0x4887d0, so they had to BE the slots - moved into
   KEZPlayer, src/obj4102c0.cpp. They read the root's own three dwords.) */

/* (The int getter @0x0040f230 was matched here as a free-standing lookalike;
   it is slot 7 of BOTH audio loaders' vtables, so it had to move into the
   class - KEzwLoader::tell, src/loaders.cpp.) */

/* (The two accessors at this+0x7260 / this+0x7274, modelled here as
   Obj40F470, are KEZPlayer's - both offsets fall inside the 0x727c its factory
   allocates, and @0x40f5c0 uses the same pair. Moved to src/kezplayer.cpp.) */

/* Two more of the engine singleton's indexed slot accessors (the third,
   getState, is in src/misc.cpp). `this + 0x18 + i*0x14` is slotsA[i].channel
   and `this + 0xa10 + i*0x14` is slotsB[i].state, which is what merges the
   classes these were modelled as - Obj40C230 and Obj40C800 - into Obj40C180. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c230
void Obj40C180::setChannel(int i, int channel) { slotsA[i].channel = channel; }

// FUNCTION: EZ2AC_UNPACKED 0x0040c800
int Obj40C180::getStateB(int i) { return slotsB[i].state; }

/* An entry's `state` and `since` are left alone - the tick owns those. The
   return value is a constant 1 nobody looks at. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c1f0
int Obj40C180::addButton(int index, int source, int bit, int channel)
{
    IoSlot *e = &slotsA[index];

    e->source = source;
    e->bit = bit;
    e->channel = channel;
    f4++;
    // STRING: EZ2AC_UNPACKED 0x00488260
    Log("Button Count : %d", f4);
    return 1;
}

/* Indexed getter over a stride-0x30 array. @0x0040fbb0 reads field_0 of
   arr[i+1] (addr = this + 0x30 + i*0x30). (Sibling bit-getter @0x0040fbd0 is
   in wip/getbit.cpp — a mov-al-vs-movzx peephole our cl won't reproduce.) */
struct Elem30b { char b[0x30]; };

struct Obj40FBB0
{
    char    _pad0[0x30];
    Elem30b arr[1];   /* first element at this+0x30 */
    int getElem0(int i);
};

__forceinline int Obj40FBB0::getElem0(int i) { return *(int *)&arr[i].b[0]; }

/* KEZPlayer's vftable slot (include/ksongplayer.h): the body above, first
   written against the Obj40FBB0 stand-in, forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fbb0
int KEZPlayer::getEntryState(int i)
{
    return ((Obj40FBB0 *)this)->getElem0(i);
}

/* (The setter formerly here at 0x412990 turned out to be KSongPlayer's
   vtable slot 1 — moved to src/ksongplayer.cpp as KSongPlayer::setTarget.) */

struct Obj41D1C0 { char _pad[0x10]; int f; void set(int v); };
// FUNCTION: EZ2AC_UNPACKED 0x0041d1c0
void Obj41D1C0::set(int v) { f = v; }

struct Obj44FA40 { char _pad[0x14]; int f; void set(int v); };
// FUNCTION: EZ2AC_UNPACKED 0x0044fa40
void Obj44FA40::set(int v) { f = v; }

/* Two-dimensional __thiscall setters: arr[a][b] = c, row stride 18 ints.
   @0x00422d10 array at this+0xc80; @0x0045f180 array at this+0xc14. */
struct Obj422D10 { char _pad[0xc80]; int arr[1][18]; void set(int a, int b, int c); };
// FUNCTION: EZ2AC_UNPACKED 0x00422d10
void Obj422D10::set(int a, int b, int c) { arr[a][b] = c; }

struct Obj45F180 { char _pad[0xc14]; int arr[1][18]; void set(int a, int b, int c); };
// FUNCTION: EZ2AC_UNPACKED 0x0045f180
void Obj45F180::set(int a, int b, int c) { arr[a][b] = c; }

/* Storage for the pointer declared in include/bigbase.h. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e710
Obj40C180 *g_bigInstance;

// FUNCTION: EZ2AC_UNPACKED 0x00401780
void Obj401780::setC(int v)
{
    f8 = 0;
    fc = v;
}


/* (The constant setter @0x44deb0 is TitleSong::update - the title screen's
   song base firing the coin lamp; src/titledirector.cpp.) */

/* The 0x418340/0x4184d0 global-list accessors moved to src/nodelists.cpp,
   where the rest of that family (iterate + count, three lists) lives. */

/* Two members of one class, tied together by the shared this+0x1d0 slot:
   @0x00401b70 points it at the global config object and @0x00402dd0 calls
   KProfile::init (matched, @0x4143e0) through it - which is what identifies
   0x530c2c as the KProfile instance. The storage is a placeholder: a real
   KProfile instance would emit a dynamic initializer we do not want. */
// GLOBAL: EZ2AC_UNPACKED 0x00530c2c
static char g_profileStorage[4];

struct ObjWithProfile
{
    char      _pad[0x1d0];
    KProfile *profile;   /* 0x1d0 */
    void setProfile();
    void initProfile();
};

// FUNCTION: EZ2AC_UNPACKED 0x00401b70
void ObjWithProfile::setProfile()
{
    profile = (KProfile *)g_profileStorage;
}

// FUNCTION: EZ2AC_UNPACKED 0x00402dd0
void ObjWithProfile::initProfile()
{
    profile->init();
}

/* @0x00403ef0: points the graphics singleton's +0x128 at the static KProfile
   that lives at 0xd30c48 (placeholder storage; m403f60 @0x403f60 is what shows
   it is a KProfile). */
// GLOBAL: EZ2AC_UNPACKED 0x00d30c48
static char g_objD30C48[4];

// FUNCTION: EZ2AC_UNPACKED 0x00403ef0
void KGraphics::setF128() { m_profile = (KProfile *)g_objD30C48; }

/* Float constant getter @0x0040f440 - loads 0.0f from the literal pool
   @0x488798 (cl does NOT fold this to fldz). */
// FUNCTION: EZ2AC_UNPACKED 0x0040f440
float getZero() { return 0.0f; }

/* Float-pair setter @0x0040b9c0: this+0x428 and this+0x42c. */
struct Obj40B9C0
{
    char  _pad[0x428];
    float f428;   /* 0x428 */
    float f42c;   /* 0x42c */
    void  setXY(float x, float y);
};

// FUNCTION: EZ2AC_UNPACKED 0x0040b9c0
void Obj40B9C0::setXY(float x, float y)
{
    f428 = x;
    f42c = y;
}

/* A 0x21c-byte record with a head pointer at +0, a 0x200-byte payload buffer
   at +0xc and a float pair at +0x214/0x218. Constructed @0x402860 (the EZ2AC
   ctor builds one at +0x6d93c of itself); clear() @0x402890 is the same body
   without the return-this. */
// FUNCTION: EZ2AC_UNPACKED 0x00402860
Obj402860::Obj402860()
{
    head = 0;
    f214 = 0.0f;
    f218 = 0.0f;
}

// FUNCTION: EZ2AC_UNPACKED 0x00402890
void Obj402860::clear()
{
    head = 0;
    f214 = 0.0f;
    f218 = 0.0f;
}

/* `f20c >= head->fc` - written in that order so the member lands in the
   register and the head field stays the memory operand (the reversed form
   compiles to setle against the same cmp encoding). */
// FUNCTION: EZ2AC_UNPACKED 0x00402970
int Obj402860::isDone()
{
    Obj402860Head *p = head;
    if (p == 0)
        return 0;

    return f20c >= p->fc;
}

/* Write one cell of the head's table, with every index checked: no table, a row
   out of range, or a column past that row's own count all report failure and
   change nothing.

   The guards must be GROUPED into two `||` chains, not written as five separate
   `if (bad) return 0;` statements. Five statements make cl place each merged
   return-0 at its first use, inline, ahead of the store; two chains give the
   original's shape - both tails out of line AFTER the success block, and split
   in two because the esi save happens between them, so the row guards return
   without popping it and the column guards with. */
// FUNCTION: EZ2AC_UNPACKED 0x00402910
int Obj402860::setCell(int r, int c, int v)
{
    Obj402860Head *t = head;

    if (t == 0 || r < 0 || r >= t->rowCount)
        return 0;
    if (c < 0 || c >= t->rows[r].count)
        return 0;

    t->rows[r].data[c] = v;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00402990
void Obj402860::reset()
{
    f8 = 0;
    f20c = 0;
    memset(cursor, 0, sizeof(cursor));
}

/* THE CELL-TABLE REFRESH @0x403940 - what every writer of an Obj402860 tail-
   calls, 47 call sites: one tick of the table's tween tracks. Each row of the
   head is a strip of cells (KTexture pointers in `data`) with a track of
   TweenKeys; the row's cursor `buf[r]` points at the key in force. Per row
   with a track: skip when past the last key or before the first (dt < 0);
   when a later key starts THIS frame, advance the cursor (-1 and f20c++ once
   the last one is reached - that is what isDone counts). A jump key
   (flags 0) is copied into the row's output VideoBlit at head+0x27810 (and
   the cursor bumped once more when the next key shares its time); a delta
   key is ADDED channel by channel, and its frameMode says how `frame` moves:
   1 add the key's frame, 2 add speed and clamp to count-1, 3 add speed and
   wrap, 4 subtract speed and wrap upward, 5 ping-pong (dt*speed+frame folded
   over count-1), else leave it. Then, when f210 is set, row 0's colour is
   the clear colour (device Clear through doOp10) and every other row is
   blitted through m4038f0 with (f214, f218). Advances f8 and returns 1
   (0 when there is no head, or the table is finished and not looping).

   Shape notes: the cursor is NOT a named local (`buf[r]` re-read each time -
   naming it swaps esi/ecx with dt), the delta adds are `out = key + out`,
   the clamp/wrap arms store through a named `f`, and the clear colour is
   `unsigned` with `(int)(double)` casts, which is what keeps cl's x87
   truncation. isDone/reset are inlined because this TU defines them. */
struct Obj405F80;
extern Obj405F80 *g_d36cac;      /* 0xd36cac (src/obj405f80.cpp) - KGraphics */
struct Obj40BB00 { void doOp10(unsigned int color); };   /* src/com_wrappers.cpp @0x40a730 */
/* The head's output blocks, one VideoBlit per row, at +0x27810. */
struct TweenOut { char _pad0[0x27810]; VideoBlit outs[1]; };
#define KB ((VideoBlit *)key->blit)

// FUNCTION: EZ2AC_UNPACKED 0x00403940
int Obj402860::m403940()
{
    VideoBlit cp;
    int r;

    if (head == 0)
        return 0;
    if (isDone())
    {
        if (f4 == 0)
            return 0;
        reset();
    }
    for (r = 0; r < head->rowCount; r++)
    {
        Obj402860Row *row = &head->rows[r];
        int dt;
        TweenKey *key;
        VideoBlit *out;

        if (row->keyCount == 0)
            continue;
        if (cursor[r] == -1)
            continue;
        key = &row->keys[cursor[r]];
        dt = f8 - key->time;
        if (dt < 0)
            continue;
        out = &((TweenOut *)head)->outs[r];
        if (dt != 0 && key[1].time == f8)
        {
            cursor[r]++;
            if (cursor[r] >= row->keyCount - 1)
            {
                cursor[r] = -1;
                f20c++;
            }
            key++;
        }
        if (key->flags == 0)
        {
            *out = *KB;
            cp = *KB;
            if (cursor[r] != -1 && key[1].time == key->time)
                cursor[r]++;
        }
        else
        {
            if (KB->frameSpeed > 100000.0f)
                KB->frameSpeed = 0.0f;
            out->x = KB->x + out->x;
            out->y = KB->y + out->y;
            out->u0 = KB->u0 + out->u0;
            out->v0 = KB->v0 + out->v0;
            out->uw0 = KB->uw0 + out->uw0;
            out->vh0 = KB->vh0 + out->vh0;
            out->u1 = KB->u1 + out->u1;
            out->v1 = KB->v1 + out->v1;
            out->uw1 = KB->uw1 + out->uw1;
            out->vh1 = KB->vh1 + out->vh1;
            out->xa = KB->xa + out->xa;
            out->xb = KB->xb + out->xb;
            out->xc = KB->xc + out->xc;
            out->xd = KB->xd + out->xd;
            out->ya = KB->ya + out->ya;
            out->yb = KB->yb + out->yb;
            out->yc = KB->yc + out->yc;
            out->yd = KB->yd + out->yd;
            out->cr = KB->cr + out->cr;
            out->cg = KB->cg + out->cg;
            out->cb = KB->cb + out->cb;
            out->ca = KB->ca + out->ca;
            out->angle = KB->angle + out->angle;
            cp = *out;
            switch (KB->frameMode)
            {
            case 1:
                out->frame = KB->frame + out->frame;
                cp.frame = out->frame;
                break;
            case 2:
                out->frame = KB->frameSpeed + out->frame;
                if (out->frame >= (float)row->count)
                {
                    float f = (float)row->count - 1.0f;
                    out->frame = f;
                }
                cp.frame = out->frame;
                break;
            case 3:
                out->frame = KB->frameSpeed + out->frame;
                if (out->frame >= (float)row->count)
                    out->frame -= (float)(int)(out->frame / (float)row->count) * (float)row->count;
                cp.frame = out->frame;
                break;
            case 4:
                out->frame -= KB->frameSpeed;
                if (0.0f > out->frame)
                {
                    float f = out->frame - (float)(int)(out->frame / (float)row->count) * (float)row->count;
                    if (0.0f > f)
                        f = (float)row->count + f;
                    out->frame = f;
                }
                cp.frame = out->frame;
                break;
            case 5:
            {
                int n = (int)((float)dt * KB->frameSpeed + out->frame);
                int per = row->count - 1;
                int q = n / per;
                if (q & 1)
                    n = (q + 1) * per - n;
                else
                    n = n - q * per;
                cp.frame = (float)n;
                break;
            }
            }
        }
        if (f210 != 0)
        {
            if (r == 0)
            {
                unsigned int c;
                c = (int)(double)cp.cr;
                c = c << 8 | (int)(double)cp.cg;
                c = c << 8 | (int)(double)cp.cb;
                ((Obj40BB00 *)g_d36cac)->doOp10(c);
            }
            else
            {
                ((KGraphics *)g_d36cac)->m4038f0((KTexture **)row->data, row->count, &cp, f214, f218);
            }
        }
    }
    f8++;
    return 1;
}

/* ONE STEP of the tween tracks - what seek @0x402d60 loops on to fast-forward
   a table to a row. The refresh above without the stack copy, the speed
   clamp, ping-pong or the draw tail, and with two differences of its own: a
   finished non-looping table DROPS ITS HEAD (`head = 0`) and returns 0, and
   when the cursor runs off the end the key in hand is not advanced. ~70% by
   masked diff: the original spills all four loop-carried values (r, the
   cursor pointer, the row and output offsets) to the frame and keeps head /
   f8 in eax / ebp; cl keeps two of them in registers here and reloads head
   and f8 instead. Same instructions, different homes; semantically complete
   (scratchpad/zoo/step*.cpp hold six forms). */
/* 62.58%, and the row loop is FOUR memory-resident induction variables
   where this reconstruction indexes everything by `r`. The frame says so
   outright - the original reserves 0x10 and we reserve 0xc, exactly one
   dword short - and the preamble and the increment block name all four:

     r          E+0x14   += 1        tested against head->rowCount
     rowOff     E+0x0c   += 0x4f0    byte offset, rows base is head+0x4f8
                                     (keyCount at +0x4f8, keys at +0x4fc)
     cursor ptr E+0x10   += 4        initialised to `this + 0xc`
     outOff     E+0x18   += 0x74     byte offset, outs base is head+0x7810

   TRIED AND MEASURED, so do not redo these:

     writing the cursor out as an explicit `int *cur = &cursor[0]` walk with
     a guarded do-while EQUALIZES THE INSTRUCTION COUNT - 104 against 104,
     where the indexed form is 104 against 105 - so the reading is right as
     far as it goes. It scores 62.24%, marginally BELOW the indexed form, so
     it is not kept: what remains after it is register allocation, not shape.

     forcing the `head` re-read the original does at the top of every
     iteration (`mov eax,[ebx]`, where cl hoists it into edi for the whole
     loop) with a volatile alias is 57.26% - the alias costs more than the
     hoist does.

   The two register differences left are that hoist, and that the original
   keeps the row offset in ecx with `head` in eax while cl does the reverse,
   so every `[off + head + 0x4f8]` operand has its two registers swapped.

   The 4-byte frame shortfall survives even the cursor walk, so whichever
   fourth value the original homes is still the thing to find; the outs
   offset is the candidate, being the one index expression still recomputed
   from `r`.

   Checked and ruled out first: this is NOT the same-TU inlining that cost
   laneMovable 77 points - our frame is SMALLER than the original's here, not
   larger. */
// FUNCTION: EZ2AC_UNPACKED 0x004029b0
int Obj402860::step()
{
    int r;

    if (head == 0)
        return 0;
    if (isDone())
    {
        if (f4 != 0)
            reset();
        else
        {
            head = 0;
            return 0;
        }
    }
    for (r = 0; r < head->rowCount; r++)
    {
        int cur;
        int dt;
        int now;
        TweenKey *key;
        VideoBlit *out;

        if (head->rows[r].keyCount == 0)
            continue;
        cur = cursor[r];
        if (cur == -1)
            continue;
        now = f8;
        key = &head->rows[r].keys[cur];
        dt = now - key->time;
        if (dt < 0)
            continue;
        out = &((TweenOut *)head)->outs[r];
        if (dt != 0 && key[1].time == now)
        {
            cursor[r] = cur + 1;
            if (cur + 1 >= head->rows[r].keyCount - 1)
            {
                cursor[r] = -1;
                f20c++;
            }
            else
                key++;
        }
        if (key->flags == 0)
        {
            *out = *KB;
            if (key[1].time == key->time)
                cursor[r]++;
        }
        else
        {
            out->x = KB->x + out->x;
            out->y = KB->y + out->y;
            out->u0 = KB->u0 + out->u0;
            out->v0 = KB->v0 + out->v0;
            out->uw0 = KB->uw0 + out->uw0;
            out->vh0 = KB->vh0 + out->vh0;
            out->u1 = KB->u1 + out->u1;
            out->v1 = KB->v1 + out->v1;
            out->uw1 = KB->uw1 + out->uw1;
            out->vh1 = KB->vh1 + out->vh1;
            out->xa = KB->xa + out->xa;
            out->xb = KB->xb + out->xb;
            out->xc = KB->xc + out->xc;
            out->xd = KB->xd + out->xd;
            out->ya = KB->ya + out->ya;
            out->yb = KB->yb + out->yb;
            out->yc = KB->yc + out->yc;
            out->yd = KB->yd + out->yd;
            out->cr = KB->cr + out->cr;
            out->cg = KB->cg + out->cg;
            out->cb = KB->cb + out->cb;
            out->ca = KB->ca + out->ca;
            out->angle = KB->angle + out->angle;
            switch (KB->frameMode)
            {
            case 1:
                out->frame = KB->frame + out->frame;
                break;
            case 2:
                out->frame = KB->frameSpeed + out->frame;
                if (out->frame >= (float)head->rows[r].count)
                    out->frame = (float)head->rows[r].count - 1.0f;
                break;
            case 3:
                out->frame = KB->frameSpeed + out->frame;
                if (out->frame >= (float)head->rows[r].count)
                    out->frame -= (float)(int)(out->frame / (float)head->rows[r].count) * (float)head->rows[r].count;
                break;
            case 4:
                out->frame -= KB->frameSpeed;
                if (0.0f > out->frame)
                {
                    float f = out->frame - (float)(int)(out->frame / (float)head->rows[r].count) * (float)head->rows[r].count;
                    if (0.0f > f)
                        f = (float)head->rows[r].count + f;
                    out->frame = f;
                }
                break;
            case 5:
                goto NEXT;
            }
        }
NEXT:;
    }
    f8++;
    return 1;
}

/* Advance the cursor to row `n`: start over first when it is already past it,
   then step until it arrives. reset() is inlined here because it is defined
   above in this TU. */
// FUNCTION: EZ2AC_UNPACKED 0x00402d60
void Obj402860::seek(int n)
{
    if (f8 != n)
    {
        if (f8 > n)
            reset();
        while (f8 < n)
            step();
    }
}

/* Ring-buffer occupancy over a 0x100-entry window. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c760
int Obj40C180::pending()
{
    int n = head - tail;
    if (n < 0)
        n += 0x100;

    return n;
}

/* Set all four flag words to 7 or 0. The neg/sbb/and idiom is cl's expansion
   of the ternary, not a hand-rolled bit trick. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c7d0
void Obj40C180::setFlags(int on)
{
    int v = (on != 0) ? 7 : 0;

    flags[0] = v;
    flags[1] = v;
    flags[2] = v;
    flags[3] = v;
}

/* Deduct `need` from `have` if there is enough, reporting success; a zero
   need always succeeds. The shortfall must be the ELSE arm - written as
   `if (h < n) return 0;` cl lays the return-0 inline, where the original has
   it as the out-of-line tail after the return-1 block. */
// FUNCTION: EZ2AC_UNPACKED 0x00418890
int Obj40C180::consume()
{
    int n = need;
    if (n != 0)
    {
        int h = have;
        if (h >= n)
            have = h - n;
        else
            return 0;
    }

    return 1;
}


/* Handle closer @0x0040d4c0 - the standalone twin of KLock's inlined dtor. */
struct Obj40D4C0 { HANDLE h; void close(); };

// FUNCTION: EZ2AC_UNPACKED 0x0040d4c0
void Obj40D4C0::close()
{
    if (h != 0)
    {
        CloseHandle(h);
        h = 0;
    }
}

/* KBase-derived record @0x0040da10 whose ctor just zeroes a 0x100-byte block
   at +4. novtable is load-bearing exactly as on KBase itself: the original
   stores NO vftable here, so this is another abstract intermediate. */
struct __declspec(novtable) Obj40DA10 : KBase
{
    int buf[0x40];   /* 0x004 */
    Obj40DA10();
    ~Obj40DA10();    /* 0x402880, folded - see include/loaders.h */
};

// FUNCTION: EZ2AC_UNPACKED 0x0040da10
Obj40DA10::Obj40DA10()
{
    memset(buf, 0, sizeof(buf));
}

/* 0xa008-byte scratch record cleared @0x0040da30: the 0xa000-byte body plus
   two trailing counters. */
struct Obj40DA30
{
    int buf[0x2800];   /* 0x0000 */
    int fa000;         /* 0xa000 */
    int fa004;         /* 0xa004 */
    void clear();
};

// FUNCTION: EZ2AC_UNPACKED 0x0040da30
void Obj40DA30::clear()
{
    memset(buf, 0, sizeof(buf));
    fa000 = 0;
    fa004 = 0;
}

/* Free __stdcall 2D setter @0x00406600 (ret 0xc = three stack args). */
struct Vec2 { float x; float y; };

// FUNCTION: EZ2AC_UNPACKED 0x00406600
void KGraphics::setVec2(Vec2 *v, float x, float y)
{
    v->x = x;
    v->y = y;
}

/* @0x00422dd0: fill a 0x20-byte stack buffer through this's own virtual slot
   8 and return its third dword. */
struct Obj422DD0
{
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void query8(int *buf, int n);   /* slot 8 -> vtable+0x20 */
    int get();
};

// FUNCTION: EZ2AC_UNPACKED 0x00422dd0
int Obj422DD0::get()
{
    int buf[8];

    query8(buf, 4);
    return buf[2];
}

/* (The guarded mode setter @0x0045c300 moved to src/directors.cpp: it is slot 0
   of WarningDirector's SECOND vtable, and a vtable slot has to BE the entity
   matched to it.) */

/* Bounds-checked indexed store @0x00426ca0. The lea/add/add/add/sub chain is
   cl's x23 multiply, so the element stride is 23 dwords = 0x5c - and
   0x2294 (the count) minus 0x14 (the array base) is exactly 96 * 0x5c, which
   fixes the array at 96 entries. */
struct Rec426CA0 { int f0; int rest[22]; };   /* 0x5c */

struct Obj426CA0
{
    char      _pad[0x14];
    Rec426CA0 arr[96];   /* 0x0014 .. 0x2294 */
    int       count;     /* 0x2294 */
    void setF0(int i, int v);   /* 0x426ca0 */
    /* setF0 on every record - the bounds check survives inline. */
    void setAllF0(int v);       /* 0x426cd0 */
    int  getF0(int i);          /* 0x426d10 */
    int  m4266c0(int a, int i); /* 0x4266c0 */
    int  m4266c0b(int a, int i); /* 0x4266c0, folded */
    int  m4266c0c(int a, int i); /* 0x4266c0, folded */
    int  m4265b0(unsigned int now, int i, int lane);  /* 0x4265b0 (src/judgewalk.cpp) */
};

/* @0x004266c0: the record's +0x50 handle, or nothing. Two arms of a
   branchless select - `sub/neg/sbb/and/add` is cl picking between 2 and 0
   on a ternary, not arithmetic. */
// FUNCTION: EZ2AC_UNPACKED 0x004266c0
int Obj426CA0::m4266c0(int a, int i)
{
    Rec426CA0 *r = &arr[i];

    if (r->rest[19] == -1)
        return 0;

    return m4265b0(a, i, r->rest[19]) == 1 ? 2 : 0;
}

/* Two more copies of the same body, ICF-folded onto 0x4266c0 by the
   original's linker. Judge::judgePress @0x426700 keeps THREE separate
   `push; push; mov ecx,edi; call 0x4266c0` return blocks, two of them
   byte-identical; cl only keeps those apart when they called different
   symbols (docs/cookbook.md, the ICF-fold rule) - with one name its tails
   merge into a single call. */
// FUNCTION: EZ2AC_UNPACKED 0x004266c0 folded
int Obj426CA0::m4266c0b(int a, int i)
{
    Rec426CA0 *r = &arr[i];

    if (r->rest[19] == -1)
        return 0;

    return m4265b0(a, i, r->rest[19]) == 1 ? 2 : 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004266c0 folded
int Obj426CA0::m4266c0c(int a, int i)
{
    Rec426CA0 *r = &arr[i];

    if (r->rest[19] == -1)
        return 0;

    return m4265b0(a, i, r->rest[19]) == 1 ? 2 : 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426ca0
void Obj426CA0::setF0(int i, int v)
{
    if (i >= 0 && i < count)
        arr[i].f0 = v;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426cd0
void Obj426CA0::setAllF0(int v)
{
    int i;
    for (i = 0; i < count; i++)
        setF0(i, v);
}

// FUNCTION: EZ2AC_UNPACKED 0x00426d10
int Obj426CA0::getF0(int i)
{
    if (i >= 0 && i < count)
        return arr[i].f0;

    return 0;
}

/* Copy a 40-byte block out of the object at this+0x4bc @0x0040b9a0. */
struct Obj40B9A0Block { char _pad[0x30]; int block[10]; };

struct Obj40B9A0
{
    char             _pad[0x4bc];
    Obj40B9A0Block  *f4bc;   /* 0x4bc */
    void copyOut(void *dst);
};

// FUNCTION: EZ2AC_UNPACKED 0x0040b9a0
void Obj40B9A0::copyOut(void *dst)
{
    memcpy(dst, f4bc->block, 40);
}

/* Free __stdcall record copy @0x00405650: copy 32 bytes src -> dst, but only
   when their tag words at +0xc agree; returns 0 on copy, 1 on mismatch. The
   copy is the if-body (inline), the mismatch the out-of-line tail. */
struct TaggedRec { int f0; int f4; int f8; int tag; int rest[4]; };

// FUNCTION: EZ2AC_UNPACKED 0x00405650
int __stdcall copyIfSameTag(TaggedRec *src, TaggedRec *dst)
{
    if (dst->tag == src->tag)
    {
        memcpy(dst, src, 32);
        return 0;
    }

    return 1;
}

/* List step @0x00401900: advance *cur to the group's head (on a null cursor)
   or to the current node's +8 link, and report whether the result is non-null.
   The store MUST be duplicated into both arms, with the null case as the
   if-body - the natural `*cur = p; return p != 0;` makes cl emit `setne` and
   clone the whole tail instead (same lever as nextListA in src/nodelists.cpp).

   It never reads `this`, so its body is byte-identical to the __stdcall free
   function it was first modelled as - but all six CALLERS load ecx from the
   screen singleton @0x1b2e844 before calling it, and cl only emits that load
   when the declaration is a member. */
// FUNCTION: EZ2AC_UNPACKED 0x00401900
int Obj401780::nextInstance(DrawGroup *group, DrawNode **cur)
{
    DrawNode *p = *cur;
    if (p == 0)
        p = group->nodes;
    else
        p = p->next;

    if (p == 0)
    {
        *cur = p;
        return 0;
    }

    *cur = p;
    return 1;
}

/* The 4x4 identity matrix in .rdata, memcpy'd into this+0x140 @0x0040bbb0.
   (The +0x140 block and the +0x30 interface make this very likely the same
   class as Obj40BB00 in src/com_wrappers.cpp; kept as a separate model here
   so the already-matched doSub's `add ecx,0x140` addressing is not perturbed
   by retyping its member.) */
// GLOBAL: EZ2AC_UNPACKED 0x00487c20
const float kIdentityMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

struct Obj40BBB0
{
    char  _pad[0x140];
    float m140[16];   /* 0x140 */
    void  loadIdentity();
    void  translate(float x, float y, float z);   /* 0x40bbd0 */
    void  rotateZ(int angle);                     /* 0x40bc70 */
};

/* Multiply the matrix at `dst` by `m`, in place. __cdecl - the caller cleans
   both arguments. (Real body in src/concatmatrix.cpp, at 45.71%.) */
extern void concatMatrix(float *dst, const float *m);   /* 0x406ad0 */

/* The engine's sine/cosine tables; storage in src/globals.cpp, which is where
   the placeholder blocks live so a big one does not shift this TU's own data
   (8 KB of it here moved g_bigInstance onto the biased address
   CatchBackGround::videoName reads, and cost that function 6%). */
extern float g_sinTable[1024];   /* 0x4a5228 */
extern float g_cosTable[1024];   /* 0x4a6228 */

// FUNCTION: EZ2AC_UNPACKED 0x0040bbb0
void Obj40BBB0::loadIdentity()
{
    memcpy(m140, kIdentityMatrix, sizeof(kIdentityMatrix));
}

/* Post-multiply the matrix by a translation. The 4x4 is built on the stack -
   an identity with the offset in the last ROW, which is what makes this a
   row-vector convention - and handed to the concatenation with the object's
   own matrix. */
// FUNCTION: EZ2AC_UNPACKED 0x0040bbd0
void Obj40BBB0::translate(float x, float y, float z)
{
    float m[16];

    m[0]  = 1.0f; m[1]  = 0.0f; m[2]  = 0.0f; m[3]  = 0.0f;
    m[4]  = 0.0f; m[5]  = 1.0f; m[6]  = 0.0f; m[7]  = 0.0f;
    m[8]  = 0.0f; m[9]  = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = x;    m[13] = y;    m[14] = z;    m[15] = 1.0f;

    concatMatrix(m140, m);
}

/* Post-multiply the matrix by a rotation about Z. The angle is an index into
   the engine's 1024-sample tables rather than radians, and only its magnitude
   is looked up - a negative angle negates the sine afterwards, which is the
   same thing and saves a second table. */
// FUNCTION: EZ2AC_UNPACKED 0x0040bc70
void Obj40BBB0::rotateZ(int angle)
{
    float m[16];
    float s;
    float c;
    int   i = angle;

    if (i < 0)
    {
        i = -i;
    }
    c = g_cosTable[i & 0x3ff];

    if (angle < 0)
    {
        s = 0.0f - g_sinTable[(-angle) & 0x3ff];
    }
    else
    {
        s = g_sinTable[angle & 0x3ff];
    }

    m[0]  = c;         m[1]  = s;    m[2]  = 0.0f; m[3]  = 0.0f;
    m[4]  = 0.0f - s;  m[5]  = c;    m[6]  = 0.0f; m[7]  = 0.0f;
    m[8]  = 0.0f;      m[9]  = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
    m[12] = 0.0f;      m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;

    concatMatrix(m140, m);
}

/* Classify one entry of a stride-0x30 array. Reconstructed from
   wip/entrykind.cpp; it lands as a 100%* EFFECTIVE match, differing only in
   which dead register the entry kind is loaded into (the original takes ecx,
   cl takes eax) - one field of one instruction, repeated in the following cmp
   and the jump-table index. Every source shape that file lists was tried; this
   is the accepted grade for that class of difference.

   The two `mov eax,C; ret 4` blocks at 0x40ff79/0x40ff81 and the `xor eax,eax;
   ret 4` at 0x40ff89 are this function's switch ARMS, not functions. */
struct Obj40FF50Entry
{
    int  kind;          /* 0x00 */
    char _pad[0x2c];    /* stride 0x30 */
};

struct Obj40FF50Owner
{
    char _pad[0x98];
    int  count;         /* 0x98 */
};

struct Obj40FF50
{
    char             _pad0[4];
    Obj40FF50Owner  *owner;      /* 0x04 */
    char             _pad8[8];
    Obj40FF50Entry   entries[1]; /* 0x10 */

    int entryKind(int index);
};

__forceinline int Obj40FF50::entryKind(int index)
{
    if (index < 0)
    {
        return 0;
    }

    if (index >= owner->count)
    {
        return 0;
    }

    switch (entries[index].kind)
    {
    case 0:
        return 0;
    case 1:
        return 1;
    case 2:
    case 3:
        return 2;
    }

    return 0;
}

/* KEZPlayer's vftable slot (include/ksongplayer.h): the body above, first
   written against the Obj40FF50 stand-in, forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ff50
int KEZPlayer::entryKind(int i)
{
    return ((Obj40FF50 *)this)->entryKind(i);
}

/* ---- three more leaves from the small-function scan (2026-08-17) --------- */

/* @0x004100d0: over the SAME stride-0x30 array as Obj40FBB0 above, but based
   at this+0x10 - forward slot 5 of the pointer each element keeps at +0x24,
   when it has one. */
struct Obj4100D0Target
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4();
    virtual void q5();                     /* slot 5 -> vtable+0x14 */
};

struct Elem30d
{
    char              _pad0[0x24];
    Obj4100D0Target  *p;      /* 0x24 */
    char              _pad28[0x30 - 0x28];
};

struct Obj4100D0
{
    char    _pad0[0x10];
    Elem30d arr[1];           /* first element at this+0x10 */
    void run(int i);
};

__forceinline void Obj4100D0::run(int i)
{
    Elem30d *e = &arr[i];

    if (e->p != 0)
        e->p->q5();
}

/* KEZPlayer's vftable slot (include/ksongplayer.h): the body above, first
   written against the Obj4100D0 stand-in, forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x004100d0
void KEZPlayer::runEntry(int i)
{
    ((Obj4100D0 *)this)->run(i);
}

/* @0x0040f610: run every lane of the chart at this+0x4 through this object's
   OWN slot 30, passing the lane and the caller's argument. The count is the
   chart's lane total at +0x98 - the same field EzChart::findFrom guards on -
   and it is re-read from the chart each iteration. */
struct Obj40F610Chart { char _pad0[0x98]; int laneCount; /* 0x98 */ };

struct Obj40F610
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09(); virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(); virtual void q19(); virtual void q20();
    virtual void q21(); virtual void q22(); virtual void q23();
    virtual void q24(); virtual void q25(); virtual void q26();
    virtual void q27(); virtual void q28(); virtual void q29();
    virtual void q30(int lane, int a);     /* slot 30 -> vtable+0x78 */

    Obj40F610Chart *chart;    /* 0x04 */
    void runLanes(int a);
};

__forceinline void Obj40F610::runLanes(int a)
{
    int i;

    for (i = 0; i < chart->laneCount; i++)
        q30(i, a);
}

/* KEZPlayer's vftable slot (include/ksongplayer.h): the body above, first
   written against the Obj40F610 stand-in, forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f610
void KEZPlayer::runLanes(int a)
{
    ((Obj40F610 *)this)->runLanes(a);
}

/* @0x00426d80: claim a slot for `id` on `lane`. The lane table is 72-byte
   entries at this+0x2298 - a kind at +0, a count at +4 and sixteen ids after
   it, which is exactly the 72 - and the reverse map is a 92-byte-stride array
   at this+0x64 whose first word records which lane took the id. Fails, with
   0, on a lane that is not kind 1 or already full. */
struct LaneSlot72
{
    int kind;      /* 0x00 */
    int count;     /* 0x04 */
    int ids[16];   /* 0x08 .. 0x48 */
};

/* The 24-byte block @0x48ff98 that every entry gets twice. */
struct Tmpl24 { int w[6]; };
extern const Tmpl24 g_tmpl48ff98;

/* 92 bytes. The lane an entry was claimed for sits at +0x50, which is why
   claim @0x426d80 addresses it as this+0x64 + id*92 - the array itself starts
   at this+0x14. */
struct IdRec92
{
    int    f00;      /* 0x00 */
    int    f04;      /* 0x04 */
    char   _pad08[0x20 - 8];
    Tmpl24 a;        /* 0x20 */
    Tmpl24 b;        /* 0x38 */
    int    lane;     /* 0x50 */
    int    index;    /* 0x54 */
    int    f58;      /* 0x58 */
};

struct Obj426D80
{
    char       _pad0[0x14];
    IdRec92    ids[0x60];     /* 0x0014 .. 0x2294 */
    int        count;         /* 0x2294 */
    LaneSlot72 lanes[8];      /* 0x2298 .. 0x24d8 */
    int        f24d8;         /* 0x24d8 */
    SlotSampleSink *sink;     /* 0x24dc */
    int        f24e0;         /* 0x24e0 */
    int        f24e4;         /* 0x24e4 */
    int        f24e8;         /* 0x24e8 */

    int  claim(int lane, int id);
    int  setup(SlotSampleSink *s, int a, int b);
    /* Open the first free lane slot (kind 0 -> 1, count 0) and return its
       index, -1 when all eight are taken. */
    int  openLane();                                   /* 0x426d40 */
    /* Set windows [2..5] of both tables of every entry, widest first. */
    void setWindows(int w5, int w4, int w3, int w2);   /* 0x426ea0 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00426d80
int Obj426D80::claim(int lane, int id)
{
    LaneSlot72 *s = &lanes[lane];

    if (s->kind == 1 && s->count < 16)
    {
        s->ids[s->count] = id;
        s->count++;
        ids[id].lane = lane;
        return 1;
    }

    return 0;
}

/* The search is its own helper returning -1, which is what keeps the
   original's dead `cmp eax,-1` on the found path. */
static int findFreeLane(Obj426D80 *j)
{
    int i;
    for (i = 0; i < 8; i++)
        if (j->lanes[i].kind == 0)
            return i;
    return -1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426d40
int Obj426D80::openLane()
{
    int i = findFreeLane(this);
    if (i != -1)
    {
        LaneSlot72 *s = &lanes[i];
        s->kind = 1;
        s->count = 0;
        f24d8++;
    }
    return i;
}

/* Four loops, one per window, each anchored on the second table's word with
   the first table's at -0x18. */
// FUNCTION: EZ2AC_UNPACKED 0x00426ea0
void Obj426D80::setWindows(int w5, int w4, int w3, int w2)
{
    int i;
    for (i = 0; i < count; i++)
    {
        ids[i].a.w[5] = w5;
        ids[i].b.w[5] = w5;
    }
    for (i = 0; i < count; i++)
    {
        ids[i].a.w[4] = w4;
        ids[i].b.w[4] = w4;
    }
    for (i = 0; i < count; i++)
    {
        ids[i].a.w[3] = w3;
        ids[i].b.w[3] = w3;
    }
    for (i = 0; i < count; i++)
    {
        ids[i].a.w[2] = w2;
        ids[i].b.w[2] = w2;
    }
}

/* @0x0040dca0: tear one record's object down. It is a MEMBER of
   EzSubsystem::SubsystemBackend even though it never touches `this` - with a
   single stack argument a thiscall and a __stdcall free function compile to
   the same body, and what settles it is the CALL SITE: m40e070 @0x40e070
   loads ecx before calling, which a free function would not need.

   Only a record whose kind is 1
   is live; the kind is cleared either way, and the object - if it has one -
   is stopped through four of its virtuals, has the value it kept at +0x18
   written back to +0xa8, and lets go of the sub-object at +0x8 after telling
   it. Every one of the six touches re-reads the object pointer out of the
   record, which is what the original does.  */
struct Obj40DCA0Sub
{
    virtual void s0();
    virtual void s1(int a);        /* slot 1 -> vtable+0x04 */
};

struct Obj40DCA0Target
{
    virtual void q0(); virtual void q1(); virtual void q2();
    virtual void q3(); virtual void q4();
    virtual void q5();             /* slot 5  -> vtable+0x14 */
    virtual void q6(int a);        /* slot 6  -> vtable+0x18 */
    virtual void q7();
    virtual void q8(int a);        /* slot 8  -> vtable+0x20 */
    virtual void q9();
    virtual void q10(int a);       /* slot 10 -> vtable+0x28 */

    char          _pad4[8 - 4];
    Obj40DCA0Sub *f8;              /* 0x08 */
    char          _padc[0x18 - 0xc];
    int           f18;             /* 0x18 */
    char          _pad1c[0xa8 - 0x1c];
    int           fa8;             /* 0xa8 */
};

struct Rec40DCA0
{
    Obj40DCA0Target *obj;          /* 0x00 */
    int              kind;         /* 0x04 */
};

// FUNCTION: EZ2AC_UNPACKED 0x0040dca0
int EzSubsystem::SubsystemBackend::release(Rec40DCA0 *r)
{
    if (r->kind == 1)
    {
        Obj40DCA0Target *p = r->obj;

        r->kind = 0;

        if (p != 0)
        {
            int keep = p->f18;

            p->q5();
            r->obj->q6(0);
            r->obj->q8(0);
            r->obj->q10(keep);
            r->obj->fa8 = keep;

            {
                Obj40DCA0Sub *sub = r->obj->f8;

                if (sub != 0)
                {
                    sub->s1(1);
                    r->obj->f8 = 0;
                }
            }
        }

        return 1;
    }

    return 0;
}

/* @0x00431040: find the entry whose name matches, over the 0x374-stride
   array at this+0x238 with its count at this+0x7c. Returns the index, or 0
   when nothing matches - so index 0 and "not found" are the same answer,
   which is the original's own choice.

   The loop is the documented three-part shape: the counter is zeroed BEFORE
   the guard so the guard compares the count against that register, the
   walking pointer is anchored explicitly and set up INSIDE the guard, and
   the body is a do-while. */
struct Rec374 { char name[0x374]; };

struct Obj431040
{
    char    _pad0[0x7c];
    int     count;            /* 0x7c */
    char    _pad80[0x238 - 0x80];
    Rec374  entries[1];       /* 0x238, stride 0x374 */
    int findByName(const char *name);
};

// FUNCTION: EZ2AC_UNPACKED 0x00431040
int Obj431040::findByName(const char *name)
{
    int i = 0;

    if (count > 0)
    {
        const Rec374 *e = &entries[0];

        do
        {
            if (_stricmp(name, e->name) == 0)
                return i;

            i++;
            e++;
        }
        while (i < count);
    }

    return 0;
}

/* @0x00434d70: a screen Y from an angle. Truncate the argument, take the
   COSINE of it out of the engine's sine table - the table is quarter-turn
   indexed, so cos comes from sin at +180 with the 1024 wrap - and map the
   -1..1 result onto the screen with 1074 - 360*c - 600 - 48.

   Below 0x34c the lookup is mirrored: the index counts DOWN from 180 and the
   sample is negated, which is the other half of the turn. The comparison is
   written as a SUBTRACTION because the original computes `i - 0x34c` into a
   register and tests its sign rather than comparing. */
extern float g_sinTable[1024];   /* 0x4a5228 (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00434d70
float __stdcall angleToY(float a)
{
    int i = (int)a;
    float c;

    if (i - 0x34c < 0)
        c = -g_sinTable[(0xb4 - i) & 0x3ff];
    else
        c = g_sinTable[(i + 0xb4) & 0x3ff];

    return 1074.0f - c * 360.0f - 600.0f - 48.0f;
}

/* @0x0040e250: give `name` the extension named by the parser's FIRST field.
   Gated on this+0x4c. The empty buffer is what asks KParser::getField for
   the first token rather than a keyed one - it writes the token back through
   the same pointer - and the token then goes to replaceExtension as the new
   extension. The 0x24 frame is the 0x20 buffer plus its cookie, which is the
   /GS shape for a function whose only local is the buffer. */
extern void replaceExtension(char *name, char *ext);   /* 0x413000 */

struct Obj40E250
{
    char _pad0[0x4c];
    int  f4c;             /* 0x4c */
    void applyFieldExtension(char *name, KParser *p);
};

// FUNCTION: EZ2AC_UNPACKED 0x0040e250
void Obj40E250::applyFieldExtension(char *name, KParser *p)
{
    char buf[0x20];

    if (f4c != 0)
    {
        buf[0] = 0;

        if (p->getField(buf) == 1)
            replaceExtension(name, buf);
    }
}



/* Re-base every live node's sounds, once per game. Three copies, one per
   list: A holds the keys slots, C the scratch records, B the catch players,
   and each walks its own list calling that class's rebaseSounds. Written as
   a plain `while` - cl rotates it into the original's guarded do-while.

   They live here rather than beside either callee: src/nodelists.cpp defines
   the walkers and src/playerslot.cpp the three rebaseSounds, and cl inlines
   whichever it can see. */
// FUNCTION: EZ2AC_UNPACKED 0x0041fae0
void rebaseListA()
{
    ListNode *cur = 0;

    while (nextListA(&cur) != 0)
        ((PlayerSlot *)cur)->rebaseSounds();
}

/* A MEMBER, not a free function, even though its body never touches `this`:
   update2 @0x45f350 sets ecx before calling it, which only a member call
   does. The body is identical either way. */
// FUNCTION: EZ2AC_UNPACKED 0x0045d1c0
void GFMainGameDirector::rebaseListC()
{
    ListNode *cur = 0;

    while (nextListC(&cur) != 0)
        ((PlayerRecord *)cur)->rebaseSounds();
}

// FUNCTION: EZ2AC_UNPACKED 0x0046c020
void CatchMainGameDirector::rebaseListB()
{
    ListNode *cur = 0;

    while (nextListB(&cur) != 0)
        ((CatchPlayer *)cur)->rebaseSounds();
}

/* Three leaves of the object whose stopTarget is parked in
   wip/smallleaves.cpp - same +0x1c counter, same target at +0xb8.

   @0x0040d420 scales the counter by the caller's float and tail-jumps into
   slot 12 with the rounded result: cl reuses the argument's own slot for the
   int, which is what makes the call a `jmp`.
   @0x0040d460 divides slot 13's answer by that counter.
   @0x0040d490 asks the target for its status through slot 9 - COM-style,
   `this` pushed - and reports whether either of bits 0 and 2 came back.

   Both conversions are UNSIGNED: `fild` plus a conditional add of 2^32
   @0x487c18 is what `(float)(unsigned)` compiles to. */
struct Obj40D420Target
{
    virtual void __stdcall u00(); virtual void __stdcall u01();
    virtual void __stdcall u02(); virtual void __stdcall u03();
    virtual void __stdcall u04(); virtual void __stdcall u05();
    virtual void __stdcall u06(); virtual void __stdcall u07();
    virtual void __stdcall u08();
    virtual void __stdcall status(int *out);   /* slot 9 -> vtable+0x24 */
};

struct Obj40D420
{
    virtual void q00(); virtual void q01(); virtual void q02();
    virtual void q03(); virtual void q04(); virtual void q05();
    virtual void q06(); virtual void q07(); virtual void q08();
    virtual void q09(); virtual void q10(); virtual void q11();
    virtual int  q12(int a);       /* slot 12 -> vtable+0x30 */
    virtual int  q13();            /* slot 13 -> vtable+0x34 */

    char             _pad4[0x1c - 4];
    unsigned int     f1c;          /* 0x1c */
    char             _pad20[0xb8 - 0x20];
    Obj40D420Target *target;       /* 0xb8 */

    int   scaleTo(float k);
    float ratio();
    int   targetBusy();            /* 0x40d490 */
};

/* Slots 14 and 15 of Obj40D4E0's vftable: the bodies stay on the view they
   were written against, forced inline into the slots that carry the
   markers. */
__forceinline int Obj40D420::scaleTo(float k)
{
    return q12((int)((float)f1c * k));
}

__forceinline float Obj40D420::ratio()
{
    return (float)(unsigned int)q13() / (float)f1c;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d420
int Obj40D4E0::scaleTo(float k)
{
    return ((Obj40D420 *)this)->scaleTo(k);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d460
float Obj40D4E0::ratio()
{
    return ((Obj40D420 *)this)->ratio();
}

/* @0x00467930: rank the two list-C records against each other and record the
   winner's id at this+0x294c, the loser's at +0x2950 - or -1 and -2 when
   they tie. The field compared is at `(g_elemCount + 2) * 64` INTO the
   record, computed rather than named, so it is a per-player column of a
   table inside the node.

   The nodes are collected into a two-entry local array first, exactly as the
   list walks elsewhere do; the same class owns the addPending parked in
   wip/smallleaves.cpp at this+0x2934. */
extern int g_elemCount;   /* 0x1b2e718 (src/loops.cpp) */

struct Obj467930
{
    char _pad0[0x294c];
    int  winner;      /* 0x294c */
    int  loser;       /* 0x2950 */
    void rankPair();
};

// FUNCTION: EZ2AC_UNPACKED 0x00467930
void Obj467930::rankPair()
{
    ListNode *cur = 0;
    ListNode *slots[2];
    int off;

    if (nextListC(&cur) != 0)
    {
        ListNode **p = slots;

        do
        {
            *p = cur;
            p++;
        }
        while (nextListC(&cur) != 0);
    }

    /* slots[1] taken first, then the element index, then slots[0]:
       the original's load order (found by scratchpad/perm) */
    char *pb = (char *)slots[1];
    int b;
    int n = g_elemCount;
    int a;
    char *pa = (char *)slots[0];

    off = (n + 2) * 64;
    a = *(int *)(pa + off);
    b = *(int *)(pb + off);

    if (a > b)
    {
        winner = slots[0]->f8;
        loser = slots[1]->f8;
    }
    else if (a < b)
    {
        winner = slots[1]->f8;
        loser = slots[0]->f8;
    }
    else
    {
        winner = -1;
        loser = -2;
    }
}

/* @0x00426390 - arm the whole table. The four arguments are stashed at
   +0x24dc..+0x24e4, the lane table and the id table are cleared whole (the
   two `rep stos` bounds land exactly on the count and on f24d8, which is what
   fixes both arrays' lengths), the sink is asked for its 0x40-word summary,
   and then every one of `count` entries is seeded: zeroed head, the 24-byte
   block at 0x48ff98 copied into both of its halves, lane and f58 to -1, and
   its own index recorded. */
// GLOBAL: EZ2AC_UNPACKED 0x0048ff98
const Tmpl24 g_tmpl48ff98 = { { 0, 0, 0, 0, 0, 0 } };

// FUNCTION: EZ2AC_UNPACKED 0x00426390
int Obj426D80::setup(SlotSampleSink *s, int a, int b)
{
    int buf[8];
    int other[3];
    int i;

    sink = s;
    f24e0 = a;
    f24e4 = b;
    f24e8 = 0;
    f24d8 = 0;

    memset(lanes, 0, sizeof(lanes));
    sink->query(buf, 0x40);
    count = buf[6];
    memset(ids, 0, sizeof(ids));

    i = 0;

    if (i < count)
    {
        /* Anchored on the LANE field, not on the entry: the original walks
           from this+0x64 with a 92 stride, which is &ids[0].lane. */
        int *p = &ids[0].lane;

        do
        {
            sink->q10(i, other, 0xffff);
            p[1] = i;
            p[-20] = 0;
            p[-19] = 0;
            p[0] = -1;
            p[2] = -1;
            *(Tmpl24 *)(p - 12) = g_tmpl48ff98;
            *(Tmpl24 *)(p - 6) = g_tmpl48ff98;
            i++;
            p += 92 / 4;
        }
        while (i < count);
    }

    return 1;
}

/* EzSubsystem::m40e1c0 @0x40e1c0 - find which of the sixteen bound parsers
   holds `key`, and hand that parser back; 0 when none does. m40e200 @0x40e200
   in src/ezsubsystem.cpp is the only caller, which is why this lives here and
   not beside it - cl would inline it into that caller and the original does
   not.

   The array is EzSubsystem::slots, already modelled, and the entries are
   KParsers: the per-slot call is KParser::seek @0x413350, which scans the
   parse buffer field by field. A null slot is skipped rather than ending the
   walk, so the sixteen are a sparse set and not a packed prefix.

   Both induction variables are the original's: it walks a pointer AND tests
   the count (`add ebx,4` with `cmp esi,0x10`), which is the shape an indexed
   `for` gives while the index is still live. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e1c0
void *EzSubsystem::m40e1c0(char *key)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        KParser *p = (KParser *)slots[i];

        if (p != 0 && p->seek(key) != 0)
            return p;
    }

    return 0;
}

/* @0x004665f0: the run's total width, or - in ClubMix and SpaceMix - just the
   current element's.

   The elements are 0x40 apart from this+0x80, each leading with two ints, and
   `g_elemCount` is the index of the last one rather than a count, so the sum
   runs 0..g inclusive. The two mode names are compared case-insensitively;
   either match takes the short path, which is laid out AFTER the summing
   return because cl keeps the if-body inline.

   The walking pointer is anchored at the SECOND int (this+0x84) and reads
   `p[-1]` and `p[0]`, which is what the original's `mov edx,[eax-4]; add edx,
   [eax]` says - indexing the element would put the base at +0x80 and shift
   both displacements. */
extern char g_modeName[256];   /* 0x1b2e680 (src/modename.cpp) */
extern int  g_elemCount;       /* 0x1b2e718 (src/loops.cpp) */

struct Obj4665F0
{
    char _pad0[0x80];
    struct Elem { int f0; int f4; char _pad8[0x40 - 8]; } elems[1];  /* 0x80 */

    int totalWidth();
};

// FUNCTION: EZ2AC_UNPACKED 0x004665f0
int Obj4665F0::totalWidth()
{
    int sum = 0;

    /* "ClubMix" carries no STRING marker here - src/maingame.cpp already
       annotates 0x48cf0c, and the entity is keyed by address. */
    if (_stricmp(g_modeName, "ClubMix") != 0 &&
        // STRING: EZ2AC_UNPACKED 0x0048cf00
        _stricmp(g_modeName, "SpaceMix") != 0)
    {
        int n = g_elemCount;

        if (n >= 0)
        {
            const int *p = &elems[0].f4;

            n = n + 1;
            do
            {
                sum += p[-1] + p[0];
                p += 0x40 / 4;
                n--;
            }
            while (n != 0);
        }

        return sum;
    }

    return elems[g_elemCount].f0;
}

/* Obj40D420::targetBusy @0x40d490 - is the target doing anything?

   Asks it for a status word through slot 9 and answers on bits 0 and 2. The
   answer is SEEDED to 1 and overwritten with 0, which is what the original's
   unconditional `mov eax,1` ahead of the test says, and the 0 it overwrites
   with is the same materialized zero the null check compares against - one
   register serving both. */
__forceinline int Obj40D420::targetBusy()
{
    int st;
    int r = 0;

    if (target != 0)
    {
        target->status(&st);

        if ((st & 5) != 0)
            r = 1;
    }

    return r;
}

/* Slot 16 of Obj40D4E0's vftable (see scaleTo above). */
// FUNCTION: EZ2AC_UNPACKED 0x0040d490
int Obj40D4E0::targetBusy()
{
    return ((Obj40D420 *)this)->targetBusy();
}

/* SoundOwnerBase::m422b60 @0x422b60 - STAMP EVERY HIT OF ONE SOUND with a
   pair of grades, and answer how many there were.

   The sink at the owner's +0x148 is opened on the handle at `start`
   (slot 11), and then walked forward (slot 12) for as long as the state it
   hands back is still at or before `limit`. Each hit gets the two grade
   bytes written into the state - +0x07, the grade a hold's TAIL is judged
   with, and +0x08, the grade a PRESS is judged with - and is handed back
   through slot 18. Both owners call it with 5 for both, i.e. give the whole
   track a KOOL, which is what applyTracks does at the end of a stage.

   `this` is the empty base both owners share, so the sink is reached
   through a cast; the base costs neither of them any layout. Its own TU
   because both callers - SlotOwner::applyTracks and GFSlotOwner::applyTracks
   - live in src/playerslot.cpp and cl would inline it into them. */
// FUNCTION: EZ2AC_UNPACKED 0x00422b60
int SoundOwnerBase::m422b60(int h, int tail, int press, int start,
                            unsigned int limit)
{
    SlotSound s;
    SlotState st;
    int n = 0;

    if (((SlotOwner *)this)->sink->begin(h, start, 0, 1, &st, &s) != 0)
    {
        do
        {
            if ((unsigned int)st.start > limit)
                break;

            st.f7 = (unsigned char)tail;
            st.f8 = (unsigned char)press;
            ((SlotOwner *)this)->sink->setState(&s, &st);
            n++;
        }
        while (((SlotOwner *)this)->sink->next(1, &st, &s) != 0);
    }

    return n;
}

/* PlayerSlot::laneMovable @0x42df20 - moved OUT of src/playerslot.cpp,
   which is load-bearing: m4215a0 @0x4215a0 is defined there, so cl
   inlined it and the function opened a `sub esp,0x38` frame the original
   has no equivalent of - twenty-three surplus instructions against an
   original with no locals at all. */

/* Can this state's lane be moved? Two passes over the track list: the entry
   that currently owns the lane has to be one of the ten playable keys, and no
   OTHER entry may object to the move. */
/* 22.80%, and the LEAD is the frame, not the body: our build opens
   `sub esp,0x38` and closes it at both exits where the original has NO
   locals at all and only its four register saves. 108 instructions against
   85, and the surplus is that frame plus the saves it forces. Find what
   needs 56 bytes before touching the control flow again - the ten key tests
   and the two loops are already right, and rewriting them as the original's
   positive `||` chain with a break (below) changed the score by nothing. */
// FUNCTION: EZ2AC_UNPACKED 0x0042df20
int PlayerSlot::laneMovable(SlotState *st)
{
    TrackList *t = &f3d0->tracks[id];
    int i;
    int j;
    int lane;

    /* The lane is read ONCE for this loop and re-read inside the next - and
       the read belongs INSIDE the guard, beside the walking pointer, which
       is where the original puts it. */
    i = 0;
    if (i < t->count)
    {
    lane = st->lane;
    do
    {
        if (lane == t->items[i].handle)
        {
            int k = t->items[i].key;

            /* A positive || chain with a break, not a negated guard: the ten
               tests each jump to the second loop and the FALL-THROUGH is the
               refusal. */
            if (k == 10 || k == 11 || k == 12 || k == 13 || k == 14 ||
                k == 18 || k == 19 || k == 20 || k == 21 || k == 22)
                break;

            return 0;
        }
        i++;
    }
    while (i < t->count);
    }

    for (j = 0; j < t->count; j++)
    {
        int h = t->items[j].handle;

        if (st->lane != h)
        {
            if (f3d0->m4215a0(h, st->start) != 0)
                return 0;
        }
    }

    return 1;
}
