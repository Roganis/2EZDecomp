/* KSoundDS - the DirectSound half of the sound subsystem, named by its own
   allocation-failure logs ("KSoundDS::CreateSample", "KSoundDS::_CreateSample").
   Four members of the EzSubsystem / Obj40EAC0 pair that were the tree's last
   off-spine stubs: the base constructor, the DirectSound bring-up, and the
   two sample factories.

   Its own TU: the EzSubsystem constructor must stay out of ezsubsystem.cpp
   (its member's constructor @0x40da50 lives there and has to stay a call)
   and out of obj40eac0.cpp (whose constructor calls it). */

#include <windows.h>
#include <string.h>
#include "kcrt.h"       /* inline operator new = malloc, delete = free */
#include "ez2acapp.h"
#include "obj40eac0.h"
#include "obj40d4e0.h"

extern void Log(const char *fmt, ...);        /* 0x402880 (src/applog.cpp) */
extern int  g_4a72d4;                         /* src/globals.cpp */

/* The five-byte-thunk form - dsound.h would give the dllimport one. It is
   DirectSoundCreate8, not DirectSoundCreate: the thunk @0x47748e jumps
   through DSOUND's IAT slot 0x487024, imported BY ORDINAL 11, which is
   DirectSoundCreate8 (ordinal 1 is the DX7 entry). Corrected 2026-10-01. */
extern "C" long __stdcall DirectSoundCreate8(void *guid, void **out, void *unk);

/* A DSBUFFERDESC (the DX7 shape, 0x24 bytes with the 3D-algorithm GUID).
   The GUID keeps its real char[8] tail: that is what makes a local of this
   type a /GS buffer, and every function here carries the cookie. */
struct DSGuid
{
    unsigned int   a;
    unsigned short b, c;
    unsigned char  d[8];
};
struct DSBufferDesc
{
    unsigned int dwSize;          /* 0x00 - always 0x24 */
    unsigned int dwFlags;         /* 0x04 */
    unsigned int dwBufferBytes;   /* 0x08 */
    unsigned int dwReserved;      /* 0x0c */
    void        *lpwfxFormat;     /* 0x10 */
    DSGuid       guidAlgorithm;   /* 0x14 .. 0x24 */
};

/* IDirectSound, only the slots the bring-up and the factories reach. */
struct IDSound
{
    virtual int __stdcall QueryInterface(void *iid, void **out);
    virtual int __stdcall AddRef();
    virtual int __stdcall Release();
    virtual int __stdcall CreateSoundBuffer(DSBufferDesc *desc, void **out,
                                            void *unk);   /* slot 3 -> +0x0c */
    virtual int __stdcall GetCaps(void *caps);
    virtual int __stdcall DuplicateSoundBuffer(void *src, void **out);
                                                          /* slot 5 -> +0x14 */
    virtual int __stdcall SetCooperativeLevel(void *hwnd, int level);
                                                          /* slot 6 -> +0x18 */
};

/* The sample seen with its whole 19-slot vtable, for the two slots the clone
   drives that the Obj40D4E0 model does not carry (its pinned vftable is
   compared over the modelled prefix; the original's table runs to slot 18 =
   Obj40D230::stopTarget @0x40d230). */
struct KSampleView
{
    virtual void q0();  virtual void q1(int);  virtual void q2(int, int);
    virtual void q3();  virtual void q4();  virtual void q5();
    virtual void q6();  virtual void q7();  virtual void q8();
    virtual void q9();  virtual void q10(); virtual void q11();
    virtual void q12(); virtual void q13(); virtual void q14();
    virtual void q15(); virtual void q16(); virtual void q17();
    virtual int  q18(int pos);    /* slot 18 -> +0x48 = stopTarget */
};

/* The stream object a sample plays from, seen as the clone uses it: slot 0
   is the runtime-class getter and slot 3 opens the named file. */
struct KStreamView
{
    virtual struct KRuntimeClass *getRuntimeClass();
    virtual ~KStreamView();      /* slot 1 - what `delete` reaches */
    virtual void r2();
    virtual int  open(void *name, int a, int b);   /* slot 3 -> +0x0c */
};

/* Only what the two factories read of the source sample beyond the
   SubsystemBase block: the stream handle at +0x08 and the rate at +0xa8. */
struct KSampleFields
{
    void *vptr;                /* 0x00 */
    int  f04;                  /* 0x04 - bit 0 = streamed */
    KStreamView *stream;       /* 0x08 */
    int  f0c;                  /* 0x0c */
    int  f10;                  /* 0x10 */
    char wfx[18];              /* 0x14 - the WAVEFORMATEX copy */
    char name[0x80];           /* 0x26 */
    char _pada6[2];
    int  rate;                 /* 0xa8 - the format's nSamplesPerSec */
};

/* Zero the slot table and the counters; the backend member's constructor
   (src/ezsubsystem.cpp) runs first, which is where the EH state comes from. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e670
EzSubsystem::EzSubsystem()
{
    memset(slots, 0, sizeof(slots));
    count = 0;
    f48 = 0;
    f4c = 0;
    kind = 0;
}

/* The DirectSound bring-up: create the device against the foreground (or
   desktop) window at DSSCL_PRIORITY and take the primary buffer; any failure
   runs the subsystem's own shutdown and reports 0. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e7f0
int Obj40EAC0::init()
{
    DSBufferDesc desc;
    void *wnd;

    wnd = GetForegroundWindow();
    if (wnd == 0)
        wnd = GetDesktopWindow();

    /* Nested successes over ONE shared shutdown tail. The original writes
       that tail inline after the LAST test with the outer failures folded
       onto it; cl will not reproduce that fold from any spelling tried
       (three explicit arms and an inline-last hybrid both keep 11 extra
       instructions), so this form - byte-count exact, the last arm's
       branch inverted - is the closest. */
    if (DirectSoundCreate8(0, (void **)&fa060, 0) == 0)
    {
        if (((IDSound *)fa060)->SetCooperativeLevel(wnd, 3) == 0)
        {
            memset(&desc, 0, sizeof(desc));
            desc.dwSize = 0x24;
            desc.dwFlags = 1;
            if (((IDSound *)fa060)->CreateSoundBuffer(&desc,
                                                      (void **)&fa064, 0) == 0)
                return 1;
        }
    }
    shutdown();
    return 0;
}

/* KSoundDS::CreateSample - make a sample from scratch: a fresh KSample, the
   caller's format copied in (and its rate cached), then a DirectSound buffer
   - a streamed sample (bit 0 of `flags`) gets the fixed 0x58000-byte ring
   with position notification, a static one gets `bytes` plain. The flags
   word is handed on through setPair. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e910
EzSample *Obj40EAC0::s6(int bytes, void *wfx, int flags)
{
    DSBufferDesc desc;
    int hr;

    Obj40D4E0 *s = new Obj40D4E0;
    if (s == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004884b0
        Log("Allocation Error : KSoundDS::CreateSample - new KSample");
        return 0;
    }

    KSampleFields *f = (KSampleFields *)s;
    f->f0c = bytes;
    memcpy(f->wfx, wfx, 18);
    f->rate = *(int *)((char *)wfx + 4);

    unsigned int size;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = 0x24;
    desc.lpwfxFormat = f->wfx;
    if ((flags & 1) != 0)
    {
        size = 0x58000;
        desc.dwFlags = 0x180c0;
    }
    else
    {
        size = (unsigned int)bytes;
        desc.dwFlags = 0x80e8;
    }
    desc.dwBufferBytes = size;
    f->f10 = size;

    if ((hr = ((IDSound *)fa060)->CreateSoundBuffer(&desc, (void **)&s->fb8, 0)) != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00488468
        Log("Allocation Error : KSoundDS::CreateSample - Alloc. DSBuffer, ErrCode %X", hr);
        delete s;
        return 0;
    }

    s->setPair(0, flags);
    return (EzSample *)s;
}

/* KSoundDS::_CreateSample - clone `src` onto a fresh sample: refuse anything
   that is not a KSample, copy the plain fields, the format and the name,
   then either rebuild a streamed source (a fresh notification ring, a fresh
   stream object made through the runtime class and opened on the source
   stream's own name, rewound to 0) or duplicate the static buffer. */
// FUNCTION: EZ2AC_UNPACKED 0x0040eb80
EzSample *Obj40EAC0::s5(EzSample *src)
{
    DSBufferDesc desc;
    int hr;
    /* At 55% on a whole-function register permutation: the original keeps a
       zero in ebx from the prologue (the null the tests compare against AND
       the st phi), spills `this` to the frame, and shrink-wraps ebp/ebx
       around the field copies; cl here caches `this` in ebp and folds the
       zero into immediates - hoisting `st = 0` above the allocation changes
       nothing, cl propagates the constant. Calls, order and multiset are
       exact (177 of its 179 instructions; the two missing are a nop pad and
       a pop-interleaved duplicate store). */
    KStreamView *st = 0;

    if (((KStreamView *)src)->getRuntimeClass() != (KRuntimeClass *)&g_4a72d4)
        return 0;

    Obj40D4E0 *s = new Obj40D4E0;
    if (s == (Obj40D4E0 *)st)
        return 0;

    KSampleFields *a = (KSampleFields *)src;
    KSampleFields *f = (KSampleFields *)s;
    f->f0c = a->f0c;
    f->f10 = a->f10;
    memcpy(f->wfx, a->wfx, 18);
    f->rate = a->rate;
    strcpy(f->name, a->name);

    if ((a->f04 & 1) != 0)
    {
        if (a->stream == 0)
        {
            delete s;
            return 0;
        }

        memset(&desc, 0, sizeof(desc));
        desc.dwSize = 0x24;
        desc.lpwfxFormat = f->wfx;
        desc.dwBufferBytes = 0x58000;
        desc.dwFlags = 0x140c0;
        if ((hr = ((IDSound *)fa060)->CreateSoundBuffer(&desc, (void **)&s->fb8, 0)) != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00488550
            Log("Allocation Error : KSoundDS::_CreateSample - Alloc. DSBuffer, ErrCode %X", hr);
            delete s;
            return 0;
        }

        st = (KStreamView *)a->stream->getRuntimeClass()->createObject();
        if (st == 0)
        {
            delete s;
            return 0;
        }
        if (st->open((char *)a->stream + 4, 0, 0) == 0)
            goto fail;
    }
    else if ((hr = ((IDSound *)fa060)->DuplicateSoundBuffer(((Obj40D4E0 *)src)->fb8,
                                                            (void **)&s->fb8)) != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00488508
        Log("Allocation Error : KSoundDS::_CreateSample - Dup. DSBuffer, ErrCode %X", hr);
        delete s;
        return 0;
    }
    s->setPair((int)st, a->f04);
    if (((KSampleView *)s)->q18(0) != 0)
        return (EzSample *)s;
fail:
    if (st != 0)
        delete st;
    delete s;
    return 0;
}
