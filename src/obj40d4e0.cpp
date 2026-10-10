/* Obj40D4E0 and its shared base - see include/obj40d4e0.h. */

#include <string.h>
#include "kcrt.h"   /* operator delete = free, for the deleting destructor */
#include "obj40d4e0.h"
#include "obj40eac0.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */
extern int  g_4a72d4;                    /* src/globals.cpp */

// FUNCTION: EZ2AC_UNPACKED 0x0040d130
void *Obj40D4E0::getClassDesc()
{
    return &g_4a72d4;
}

/* The two CreateEventA calls and their failure logs are the two KLock members'
   inlined constructors; only the plain field resets are this body. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d4e0
Obj40D4E0::Obj40D4E0() : ev0(0, 1), ev1(1, 0)
{
    fac = 0;
    fb8 = 0;
    fbc = 0;
    fc4 = 0;
    fcc = 0;
    fc0 = 0;
    fc8 = -1;
}

/* The two CloseHandle blocks after this are the KLocks going out of scope. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d5a0
Obj40D4E0::~Obj40D4E0()
{
    ((Obj40D4E0Mirror *)this)->q5();
    if (fb8 != 0)
    {
        fb8->v2();
        fb8 = 0;
    }
}

/* Slots 2, 6 and 7. All three were matched as free-standing lookalikes before
   (Obj40D0D0::setPair in src/ez2ac.cpp, EngineObj::put16 and ::get7 in
   src/com_wrappers.cpp); a vtable slot is scored on the recomp address of the
   entity matched to it, so they have to BE the slots. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d0d0
void Obj40D4E0::setPair(int a, int b)
{
    f08 = a;
    f04 = b;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d290
void Obj40D4E0::put16(int v)
{
    if (fb8 != 0)
        fb8->set16(v);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d2b0
int Obj40D4E0::get7()
{
    if (fb8 != 0)
    {
        int out;
        fb8->query7(&out);
        return out;
    }
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d9f0
// Obj40D4E0::`scalar deleting destructor'

// VTABLE: EZ2AC_UNPACKED 0x00488320
struct Obj40D4E0;

/* The sound buffer at +0xb8: IDirectSoundBuffer, of which only Lock (slot 11)
   and Unlock (slot 19) are reached. */
struct IDSoundBuffer
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall GetCaps(void *caps);
    virtual int __stdcall GetCurrentPosition(void *play, void *write);
    virtual int __stdcall GetFormat(void *fmt, int size, void *written);
    virtual int __stdcall GetVolume(int *v);
    virtual int __stdcall GetPan(int *p);
    virtual int __stdcall GetFrequency(int *f);
    virtual int __stdcall GetStatus(int *s);
    virtual int __stdcall Initialize(void *ds, void *desc);
    /* slot 11 -> +0x2c */
    virtual int __stdcall Lock(int offset, int bytes, void **p1, int *n1,
                               void **p2, int *n2, int flags);
    virtual int __stdcall Play(int r1, int r2, int flags);
    virtual int __stdcall SetCurrentPosition(int pos);
    virtual int __stdcall SetFormat(void *fmt);
    virtual int __stdcall SetVolume(int v);
    virtual int __stdcall SetPan(int p);
    virtual int __stdcall SetFrequency(int f);
    virtual int __stdcall Stop();
    /* slot 19 -> +0x4c */
    virtual int __stdcall Unlock(void *p1, int n1, void *p2, int n2);
};

/* The decoder at +0x08; only the two slots the refill drives. */
struct SoundFeed
{
    virtual void v0();
    virtual void v1(int del);                  /* slot 1 -> the deleting dtor */
    virtual void v2(); virtual void v3();
    virtual void fill(void *dst, int bytes);   /* slot 4 -> +0x10 */
    virtual void v5();
    virtual void seek(int pos, int whence);    /* slot 6 -> +0x18 */
};

struct Obj40D230
{
    /* It is polymorphic: stopTarget @0x40d230 calls three of its own slots.
       Only those three are named. */
    virtual void *q0();                   /* slot 0 -> the class descriptor */
    virtual void q1(); virtual void q2(void *in, int flags);
    virtual void q3();
    virtual void q4(int on);              /* slot 4 -> +0x10 - the run flag */
    virtual void q5();                    /* slot 5 -> +0x14 */
    virtual void q6(int a);               /* slot 6 -> +0x18 */
    virtual void q7();
    virtual void q8(int a);               /* slot 8 -> +0x20 */
    virtual void q9();  virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual void q18(int a);              /* slot 18 -> +0x48 */

    int        f4;                /* 0x04 - bit 0 picks the seek target */
    SoundFeed *feed;              /* 0x08 */
    int        fc;                /* 0x0c - how many bytes were loaded */
    char       _pad10[0x22 - 0x10];
    short      bits;              /* 0x22 - bits per sample */
    char       _pad24[0xac - 0x24];
    int        flags;             /* 0xac - bits 1 and 2 are the play state */
    void      *ev0h;              /* 0xb0 - the KLock handles, seen raw */
    void      *ev1h;              /* 0xb4 */
    IDSoundBuffer *buffer;        /* 0xb8 */
    unsigned int streamPos;       /* 0xbc - where the next refill reads from */
    unsigned int posInBlock;      /* 0xc0 */
    int        fc4;               /* 0xc4 */
    int        fc8;               /* 0xc8 */
    unsigned int block;           /* 0xcc */

    int m40d170(int pos);         /* 0x40d170 */
    int stopTarget(int pos);      /* 0x40d230 */
    void m40d380(int pos);        /* 0x40d380 - vftable slot 12 */
    int  m40d700();               /* 0x40d700 - vftable slot 17 */
};

/* Obj40D230::m40d170 @0x40d170 - seek the stream and refill the whole buffer.

   The byte position is split into a BLOCK index and an offset within it by
   an unsigned divide by 0x58000 - 360 KB, the buffer's own span - and the two
   halves are latched at +0xcc and +0xc0. The play-state bits at +0xac are
   cleared and the two markers reset, so this is a hard seek rather than a
   nudge.
   
   Then, if both the buffer and the decoder exist, the whole buffer is locked
   (DSBLOCK_ENTIREBUFFER), the decoder seeked to the requested position and
   asked to fill what the lock handed back, and the buffer unlocked. Any COM
   failure answers 0 and leaves the latched position in place. Note that a
   MISSING buffer or decoder is not a failure - the latch has been done and
   there was nothing to refill, so that path answers 1 like a clean refill
   does; only a COM error answers 0.
   
   Note the lock's byte-count output goes into the ARGUMENT'S OWN home slot,
   which is dead by then - the documented dead-parameter-slot reuse - and the
   pointer into the one local. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d170
int Obj40D230::m40d170(int pos)
{
    void *audio;
    int   bytes;

    block = (unsigned int)pos / 0x58000;
    flags &= ~6;
    fc4 = 0;
    fc8 = -1;
    posInBlock = (unsigned int)pos % 0x58000;

    if (buffer != 0 && feed != 0)
    {
        if (buffer->Lock(0, 0, &audio, &bytes, 0, 0, 2) != 0)
            return 0;

        feed->seek(pos, 0);
        feed->fill(audio, bytes);

        if (buffer->Unlock(audio, bytes, 0, 0) != 0)
            return 0;
    }

    return 1;
}

/* Obj40D230::stopTarget @0x40d230 - move the play cursor and refill.

   Slot 5 is told first and the flag word at +0xac cleared, whatever
   happens. Then, if there is a buffer, its play cursor is moved - to the
   requested position, or to ZERO when bit 0 of +0x04 is set - the whole
   buffer is refilled from there through m40d170 @0x40d170, and slots 6 and
   8 are handed a zero. The answer is always 1.

   Bit 0 of +0x04 does NOT merely pick the seek target: with the bit CLEAR
   the cursor moves to `pos` and that is all, and only the set arm rewinds to
   zero and refills. Reading it as one seek followed by a shared refill costs
   the arm its own `call [ecx+0x34]` - cl cross-jumps the two seeks into one
   - and every branch displacement with it (84.38%). */
__forceinline int Obj40D230::stopTarget(int pos)
{
    IDSoundBuffer *o;

    q5();
    o = buffer;
    flags = 0;

    if (o != 0)
    {
        if ((f4 & 1) == 0)
        {
            o->SetCurrentPosition(pos);
        }
        else
        {
            o->SetCurrentPosition(0);
            m40d170(pos);
        }

        q6(0);
        q8(0);
    }

    return 1;
}

/* Slot 18 of the vftable (include/obj40d4e0.h): the body above, forced
   inline into the slot that carries the marker. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d230
int Obj40D4E0::stopTarget(int pos)
{
    return ((Obj40D230 *)this)->stopTarget(pos);
}

/* Obj40EAC0::s4 @0x40eda0 - LOAD a sound target from an opened stream, the
   one-shot sibling of Obj40D230::m40d170 @0x40d170 above.

   It refuses anything that is not an Obj40D230 (slot 0 has to hand back the
   same class descriptor this file's getClassDesc returns) and anything with
   no stream, then tells the target to stop through slot 5.

   Bit 0 of the target's +0x04 - the same bit that decides whether stopTarget
   rewinds - splits the work. Set, the target is STREAMED: there is nothing to
   preload, so an existing decoder is simply deleted and the stream is handed
   on live. Clear, the whole buffer is locked, the stream rewound and read
   into it, and any shortfall padded with SILENCE - 0x80 for 8-bit samples and
   zero for 16-bit, which is what the branchless `setne`/`sub 1`/`and 0x80`
   computes - after which the stream is closed, deleted, and handed on as
   null. Either way the byte count reaches +0x0c and slots 2 and 18 close it
   out. A COM failure at either end answers 0 with the stream still open.

   The lock writes its two outputs into the DEAD PARAMETER SLOTS - the byte
   count into arg1's home and the pointer into arg3's - which is why the
   function reserves no frame of its own, and both `lea`s come out byte-exact
   on the first build.

   71.32%, AND EVERY DEFECT IS ONE THING: WHICH PREDECESSOR cl ATTACHES A
   SHARED BLOCK TO. Three shared blocks here, and the original attaches each
   to its LAST predecessor while cl 13.10 attaches it to its FIRST:

     the guard's `return 0`   both agree (fixed by writing the three tests as
                              one `||` chain rather than three early returns)
     the COM `return 0`       fixed here, by writing the success as the nested
                              `if (Lock(..) == 0) { ... }` with the single
                              `return 0` textually LAST
     the join                 STILL WRONG. The original lays out
                              [small arm][jmp forward][lock body][join];
                              cl lays out [small arm][join][ret][lock
                              body][jmp back]. That also costs the
                              `mov edi,[esp+0x14]` at 0x40edce - the original
                              loads `bytes` before the branch precisely
                              BECAUSE the join is shared and placed after
                              both arms.

   Six shapes tried for the join, none of which moves it: if/else with the
   tail after; the same with an explicit `goto done`; the whole CFG written
   with gotos in the original's order (72.87%, but it moves the ARMS instead
   and regresses the memset fill); the arms swapped; and the join written out
   in BOTH arms so cl could merge and keep the second - cl keeps both copies
   instead (143 instructions against 129, and the if-arm then does not even
   push edi, which is the proof the original really does share it). */
// FUNCTION: EZ2AC_UNPACKED 0x0040eda0
int Obj40EAC0::s4(Obj40D230 *t, unsigned int bytes, SoundFeed *in)
{
    unsigned int total;
    void        *audio;

    if (t == 0 || in == 0 || t->q0() != &g_4a72d4)
        return 0;

    t->q5();

    if ((t->f4 & 1) != 0)
    {
        if (t->feed != 0)
        {
            t->feed->v1(1);
            t->feed = 0;
        }
    }
    else
    {
        if (t->buffer->Lock(0, 0, &audio, (int *)&total, 0, 0, 2) == 0)
        {
            if (bytes > total)
                bytes = total;

            in->seek(0, 0);
            in->fill(audio, bytes);

            if (bytes < total)
                memset((char *)audio + bytes, t->bits == 8 ? 0x80 : 0,
                       total - bytes);

            if (t->buffer->Unlock(audio, total, 0, 0) == 0)
            {
                in->v5();
                in->v1(1);
                in = 0;
                goto done;
            }
        }
        return 0;
    }

done:
    t->fc = bytes;
    t->q2(in, t->f4);
    t->q18(0);
    return 1;
}

/* Vftable slot 12: seek. A static buffer just moves the DirectSound play
   cursor; a streamed one runs the hard seek through its own slot 18
   (stopTarget) and - when it was playing - starts again with the loop bit
   as the run argument. */
__forceinline void Obj40D230::m40d380(int pos)
{
    if (buffer == 0)
        return;
    if ((f4 & 1) == 0)
    {
        buffer->SetCurrentPosition(pos);
        return;
    }
    /* The original loads the WHOLE flags word and tests bit 0 as a byte
       (`and dl,1; sub dl,1; neg dl; sbb; add 1` - the `== 1` of a char);
       a plain read lets cl narrow the load to `mov dl,byte`. A volatile
       read of this one use keeps the dword load - declaring the member
       volatile instead costs m40d700 22 diff lines. */
    int playing = (unsigned char)(*(volatile int *)&flags & 1) == 1;
    int looping = (f4 & 2) == 2;
    q18(pos);
    if (playing == 1)
        q4(looping);
}

/* Slot 12 of the vftable (include/obj40d4e0.h): the body above, forced
   inline into the slot that carries the marker. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d380
void Obj40D4E0::seek(int pos)
{
    ((Obj40D230 *)this)->m40d380(pos);
}

/* Vftable slot 17: one turn of the streaming ring. The 0x58000-byte buffer
   is sixteen 0x5800 chunks; the chunk four behind the play cursor is the
   one refilled. Bit 2 of the flags means the end of the data is already in
   the ring (everything after it is silence until the marked end chunk comes
   around - then a looping sample flips to bit 4, a one-shot stops); bit 4
   means the wrap back to the start is in flight. The whole ring is locked
   (DSBLOCK_ENTIREBUFFER), a short final chunk is silence-padded, and the
   `fill` flag is genuinely left unset on the plain data paths - the
   original reads it uninitialized there, C4700 and all.

   PORTING LANDMINE: that uninitialized read is faithful HERE, where the
   goal is the original's bytes, and it is undefined behaviour anywhere
   else. The `else` arm where `streamPos + 0x5800 < fc` never assigns
   `fill`, and the test on it below decides whether the chunk is
   silence-padded. A port of this ring (sixteen 0x5800 chunks, refill four
   behind the play cursor, 0x80 silence at 8-bit) must initialise `fill`
   to 0 on that path - on the original's stack it is whatever an earlier
   call left in the slot. Do not reproduce the read. */
__forceinline int Obj40D230::m40d700()
{
    unsigned int pos;
    void *p1;
    unsigned int n1;
    unsigned int ofs;
    int cursor;
    int fill;

    if ((f4 & 1) == 0 || (flags & 1) == 0)
        return 0;
    unsigned int size = 0;
    if (buffer == 0 || feed == 0)
        return 0;

    WaitForSingleObject(ev0h, -1);
    if (WaitForSingleObject(ev1h, 0) != 0)
        goto out;

    pos = 0;
    buffer->GetCurrentPosition(&pos, 0);
    cursor = pos / 0x5800 - 4;
    if (cursor < 0)
        cursor += 16;
    else if (cursor >= 16)
        cursor -= 16;
    if (cursor == fc4)
        goto out;

    ofs = cursor * 0x5800;
    size = 0x5800;
    if (fc4 > cursor)
        block += 1;
    streamPos = (block * 16 + cursor) * 0x5800 + posInBlock;

    /* The flags are read ONCE here and every arm stores its reworked copy
       back last - the value rides a register through the whole ladder. */
    int fl = flags;
    if ((fl & 4) != 0)
    {
        if (cursor == 0)
        {
            fill = 0;
            block = 0;
            streamPos = 0;
            flags = fl & ~4;
        }
        else
        {
            fill = 1;
        }
    }
    else if ((fl & 2) != 0)
    {
        fill = 1;
        if (cursor == fc8)
        {
            if ((f4 & 2) == 0)
                goto stop;
            flags = (fl & ~2) | 4;
        }
    }
    else
    {
        if (streamPos + 0x5800 >= (unsigned int)fc)
        {
            if (streamPos >= (unsigned int)fc)
            {
                fill = 1;
                fc8 = cursor;
                flags = fl | 2;
            }
            else
            {
                size = fc - streamPos;
                fc8 = cursor;
                flags = fl | 2;
            }
        }
    }

    if (buffer->Lock(0, 0, &p1, (int *)&n1, 0, 0, 2) != 0)
        goto stop;
    if (size < 0x5800)
    {
        unsigned char sil = bits == 8 ? 0x80 : 0;
        memset((char *)p1 + ofs + size, sil, 0x5800 - size);
    }
    if (fill != 0)
    {
        unsigned char sil = bits == 8 ? 0x80 : 0;
        memset((char *)p1 + ofs, sil, size);
    }
    else
    {
        feed->seek(streamPos, 0);
        feed->fill((char *)p1 + ofs, size);
    }
    if (buffer->Unlock(p1, n1, 0, 0) != 0)
        goto stop;
    fc4 = cursor;
out:
    SetEvent(ev0h);
    return size;

stop:
    ResetEvent(ev1h);
    flags &= ~7;
    if (buffer != 0)
    {
        buffer->Stop();
        buffer->SetCurrentPosition(0);
    }
    SetEvent(ev0h);
    return 0;
}

/* Slot 17 of the vftable (include/obj40d4e0.h): the body above, forced
   inline into the slot that carries the marker. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d700
int Obj40D4E0::refill()
{
    return ((Obj40D230 *)this)->m40d700();
}
