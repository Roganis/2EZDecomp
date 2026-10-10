#include "ez2acapp.h" /* KGraphics, Sprite */
#include "ksongplayer.h"
#include "bigbase.h"
#include "directors.h"   /* GameOverDirector, and m450420 NOT defined here */
/* Small standalone leaf functions. Placeholder struct/class identities (TBD);
   the offsets are hard facts from the binary. */

/* Saturating unsigned subtract @0x004145d0 (__cdecl):
     if (a < b) return 0; else return a - b; */

// FUNCTION: EZ2AC_UNPACKED 0x004145d0
unsigned int SatSub(unsigned int a, unsigned int b)
{
    if (a >= b)
        return a - b;
    return 0;
}

/* Indexed accessor @0x0040fb50 (__thiscall): element stride 0x30, field @0x2c. */
struct Elem30 { char _pad[0x2c]; int field_0x2c; };
struct Obj40FB50
{
    Elem30 arr[1];
    int getElem2c(int i);
};

__forceinline int Obj40FB50::getElem2c(int i)
{
    return arr[i].field_0x2c;
}

/* KEZPlayer's vftable slot (include/ksongplayer.h): the body above, first
   written against the Obj40FB50 stand-in, forced inline into the slot. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fb50
int KEZPlayer::getEntryVolume(int i)
{
    return ((Obj40FB50 *)this)->getElem2c(i);
}

/* Bounds-checked array setter @0x004264d0 (__thiscall): element stride 0x5c,
   field @0x18, count @0x2294. Writes arr[idx].field = val iff 0 <= idx < count. */
struct Elem5C { char _pad[0x18]; int field_0x18; char _tail[0x5c - 0x18 - 4]; };
struct Obj4264D0
{
    Elem5C arr[1];
    char    _pad[0x2294 - 0x5c];
    int     count;   /* 0x2294 */
    void setElem(int idx, int val);
    /* setElem on every record - the bounds check survives inline. */
    void setAllElem(int val);   /* 0x426e60 */
};

// FUNCTION: EZ2AC_UNPACKED 0x004264d0
void Obj4264D0::setElem(int idx, int val)
{
    if (idx >= 0 && idx < count)
        arr[idx].field_0x18 = val;
}

// FUNCTION: EZ2AC_UNPACKED 0x00426e60
void Obj4264D0::setAllElem(int val)
{
    int i;
    for (i = 0; i < count; i++)
        setElem(i, val);
}

/* The sprite measurements @0x408110 / 0x408120. Both are KGraphics members
   whose body only touches the ARGUMENT (this is unused, but the call sites load
   the graphics singleton into ecx - ShowCredit::setPos @0x44fa50 is one), and
   both return through st(0). What they read is the sprite's own extent: setPos
   centres with `(640 - widthOf(s)) * 0.5f`, which is what identifies +0x80 and
   +0x84 as its width and height. */
// FUNCTION: EZ2AC_UNPACKED 0x00408110
float KGraphics::widthOf(Sprite *s)
{
    return s->width;
}

// FUNCTION: EZ2AC_UNPACKED 0x00408120
float KGraphics::heightOf(Sprite *s)
{
    return s->height;
}

/* __thiscall field setter @0x00406c00: this+0x12c = v. */
struct Obj406C00 { char _pad[0x12c]; int field_0x12c; void setField12c(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x00406c00
void Obj406C00::setField12c(int v) { field_0x12c = v; }

/* __thiscall field setter @0x00407350: this+0xd4 = v. */
struct Obj407350 { char _pad[0xd4]; int field_0xd4; void setFieldD4(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x00407350
void Obj407350::setFieldD4(int v) { field_0xd4 = v; }

/* __thiscall field setter @0x0040b9e0: this+0x43c = v. */
struct Obj40B9E0 { char _pad[0x43c]; int field_0x43c; void setField43c(int v); };

// FUNCTION: EZ2AC_UNPACKED 0x0040b9e0
void Obj40B9E0::setField43c(int v) { field_0x43c = v; }

/* The press state of one first-table slot. `this + 0x10 + i*0x14` is exactly
   slotsA[i].state (the table starts at +8 and state is its third word), which
   is what identifies the object this used to be modelled as - Obj40C1C0 - as
   the engine singleton itself. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c1c0
int Obj40C180::getState(int i) { return slotsA[i].state; }

/* Flag the global instance, then tail-jump Obj401780::setC (@0x401780, defined
   in src/accessors.cpp). Kept in a DIFFERENT TU from setC: with the callee
   visible cl inlines it and the original's tail `jmp` disappears. */
// FUNCTION: EZ2AC_UNPACKED 0x004199d0
void Obj401780::armAndSetC(int v)
{
    Obj40C180 *g = g_bigInstance;

    g->f4200ec = 1;
    g->f4200f0 = 0;

    setC(v);
}

ScrFader *m450570();    /* 0x450570 (STUB, src/stubs.cpp) */

/* GameOverDirector::m44d9d0 @0x0044d9d0 - three lines, and the null test on
   `this` is not a null test: converting to the SECONDARY base at +0x10 is
   what emits `test esi,esi; je; lea edx,[esi+0x10]` against `xor edx,edx`,
   because C++ requires the adjustment to leave a null pointer null.

   It lives HERE rather than in src/directors.cpp with the rest of its class
   because that file defines ScrFader::m450420, and cl inlines a body it can
   see - the original calls it. */
// FUNCTION: EZ2AC_UNPACKED 0x0044d9d0
int GameOverDirector::m44d9d0()
{
    ScrFader *o = m450570();

    o->m450420(2, 7, (GameOverSink *)this);
    return 1;
}

extern EzObjectInfo g_info_4adfa8;   /* 0x4adfa8 (src/classinfo.cpp) */

/* GameOverDirector's new-plus-ctor factory. It lives here rather than in
   src/ezscreens.cpp with its caller, or src/directors.cpp with the ctor -
   cl would inline whichever it could see. */
// FUNCTION: EZ2AC_UNPACKED 0x0044dbf0
void *createGameOverDirector()
{
    g_objectRecord = &g_info_4adfa8;
    return new GameOverDirector;
}

extern EzObjectInfo g_info_4aea2c;   /* 0x4aea2c (src/classinfo.cpp) */
extern EzObjectInfo g_info_4ae118;   /* 0x4ae118 (src/classinfo.cpp) */
extern EzObjectInfo g_info_4a8f58;   /* 0x4a8f58 (src/classinfo.cpp) */

/* DrawModeIntroDirector's new-plus-ctor factory. */
// FUNCTION: EZ2AC_UNPACKED 0x0046a440
DrawModeIntroDirector *createDrawModeIntroDirector()
{
    g_objectRecord = &g_info_4aea2c;
    return new DrawModeIntroDirector;
}

/* ShowCredit's new-plus-ctor factory. */
// FUNCTION: EZ2AC_UNPACKED 0x00450390
ShowCredit *createShowCredit()
{
    g_objectRecord = &g_info_4ae118;
    return new ShowCredit;
}

/* BattleMode's new-plus-ctor factory. */
// FUNCTION: EZ2AC_UNPACKED 0x0041c9a0
BattleMode *createBattleMode()
{
    g_objectRecord = &g_info_4a8f58;
    return new BattleMode;
}

extern EzObjectInfo g_info_4a9040;   /* 0x4a9040 (src/classinfo.cpp) */
extern EzObjectInfo g_info_4ae428;   /* 0x4ae428 (src/classinfo.cpp) */

/* The keys game's director factory - 0xdb0 out of its own record. */
// FUNCTION: EZ2AC_UNPACKED 0x00424980
EZ2DJMainGameDirector *createMainGameDirector()
{
    g_objectRecord = &g_info_4a9040;
    return new EZ2DJMainGameDirector;
}

/* And the ranking screen's - 0x109de8, the second biggest the pool serves. */
// FUNCTION: EZ2AC_UNPACKED 0x0045bbd0
RankingDirector *createRankingDirector()
{
    g_objectRecord = &g_info_4ae428;
    return new RankingDirector;
}

/* NOT in src/accessors.cpp, which is where it belongs by shape: that file
   DEFINES Obj402860::clear, and cl inlines a body it can see - the loop then
   comes out as three stores per iteration instead of a call and scores
   2.35%. */
/* @0x0041b4a0: reset ten chart heads and their ten parallel records. The
   charts are Obj402860 (stride 0x21c) at this+0x1006a0; each record keeps a
   value at +0 that the reset copies to +8 while clearing +4, and the flag
   right past the record array goes to zero first. Both arrays become walking
   pointers, which is why the original counts DOWN. */
struct Rec10e { int f0; int f4; int f8; int fc; };

struct Obj41B4A0
{
    char      _pad0[0x1006a0];
    Obj402860 charts[10];                        /* 0x1006a0, stride 0x21c */
    char      _pad101bb8[0x102078 - 0x1006a0 - 10 * 0x21c];
    Rec10e    recs[10];                          /* 0x102078 */
    int       f102118;                           /* 0x102118 */
    void reset();
};

// FUNCTION: EZ2AC_UNPACKED 0x0041b4a0
void Obj41B4A0::reset()
{
    int i;

    f102118 = 0;

    for (i = 0; i < 10; i++)
    {
        charts[i].clear();

        {
            int v = recs[i].f0;

            recs[i].f4 = 0;
            recs[i].f8 = v;
        }
    }
}

/* @0x00451200 - decide the battle winner, which is what include/directors.h
   already said the two fields at +0x2c94 were for. The same shape as
   Obj467930::rankPair @0x467930 on the other list: collect the two list-A
   nodes, compare the per-player column at `g_elemCount * 64 + 0x254` inside
   each, and record the winner's id then the loser's - or -1 and -2 on a tie.

   Not in src/resultctor.cpp, where its caller lives, because a callee cl can
   see would be inlined into it. */
extern int nextListA(ListNode **cur);   /* 0x4182e0 */
extern int g_elemCount;                 /* 0x1b2e718 */

// FUNCTION: EZ2AC_UNPACKED 0x00451200
void ResultDirector::m451200()
{
    ListNode *cur = 0;
    ListNode *slots[2];
    int off;

    if (nextListA(&cur) != 0)
    {
        ListNode **p = slots;

        do
        {
            *p = cur;
            p++;
        }
        while (nextListA(&cur) != 0);
    }

    /* the same load order as rankPair @0x467930: slots[1] first, then
       the element index, then slots[0] */
    char *pb = (char *)slots[1];
    int b;
    int n = g_elemCount;
    int a;
    char *pa = (char *)slots[0];

    off = n * 64;
    a = *(int *)(pa + off + 0x254);
    b = *(int *)(pb + off + 0x254);

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
