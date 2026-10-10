#ifndef LOADERS_H
#define LOADERS_H

#include <stdio.h>
#include "kbase.h"
#include "ksound.h"   /* KRuntimeClass */

/* THE TWO AUDIO FORMAT LOADERS.

   The sound subsystem's plugin pair, and both name themselves: slot 0 of each
   returns a KRuntimeClass whose name string is "KEzwLoader" / "KWavLoader",
   and slot 2 returns a descriptor holding the human-readable format name, the
   file extension and the sample class it produces:

     vtable    class        rtc       info      name                 ext    sample
     0x4882c0  KEzwLoader   0x4a72a8  0x4a72b8  "EZW Format Loader"  .ssf   KSample
     0x4885c4  KWavLoader   0x4a7320  0x4a7330  "WAV Format Loader"  .wav   KEZPlayer

   They are the same class twice over, with ONE number different: the size of
   the header the reader skips past, 0x12 for .ssf and 0x2c for .wav - and 0x2c
   is exactly the standard 44-byte WAV header, which is what confirms the
   reading. Both are plain stdio: fopen/fread/fseek/ftell/fclose, with the file
   position kept as a payload offset (absolute position minus the header).

   Neither is pooled - each `??_G` ends in `push this; call free`, so they use
   the CRT allocator, and the factory is a plain `new`. */

/* The abstract intermediate both derive from (its ctor is matched in
   src/accessors.cpp, which must stay a separate TU so the call survives). It
   contributes the 0x100-byte buffer the open path strcpy's the file name into,
   which is why the object is 0x110 bytes. */
struct __declspec(novtable) Obj40DA10 : KBase
{
    int buf[0x40];   /* 0x004 */
    Obj40DA10();     /* 0x40da10 (src/accessors.cpp) */
    /* Declared so the derived destructors tail-jump to ITS empty body rather
       than to KBase's. The original does exactly that - it jumps straight to
       the shared `ret` @0x402880 instead of the thunk @0x426e40 every
       KBase-derived destructor uses - which is what says this intermediate has
       a destructor of its own. It is ICF-folded onto that `ret`; the alias is
       in src/stubs.cpp. */
    ~Obj40DA10();    /* 0x402880 (folded) */
};

/* WAVEFORMATEX, spelled out so a TU that models a loader need not pull in
   <mmsystem.h>. Packed, exactly as the SDK declares it: 18 bytes, which is
   also the whole .ssf header. */
#pragma pack(push, 1)
struct WaveFormat
{
    unsigned short wFormatTag;       /* 0x00 */
    unsigned short nChannels;        /* 0x02 */
    unsigned int   nSamplesPerSec;   /* 0x04 */
    unsigned int   nAvgBytesPerSec;  /* 0x08 */
    unsigned short nBlockAlign;      /* 0x0c */
    unsigned short wBitsPerSample;   /* 0x0e */
    unsigned short cbSize;           /* 0x10 */
};
#pragma pack(pop)

/* What slot 2 hands back: the loader's own runtime class, then the three
   strings the subsystem matches a file against. Placeholder storage - only the
   address has to line up. */
struct LoaderInfo
{
    KRuntimeClass *rtc;          /* 0x00 */
    const char    *formatName;   /* 0x04 - "EZW Format Loader" */
    const char    *extension;    /* 0x08 - ".ssf" */
    const char    *sampleClass;  /* 0x0c - "KSample" */
    void          *f10;          /* 0x10 */
    void          *f14;          /* 0x14 */
};

struct KEzwLoader : Obj40DA10
{
    virtual void *getClassDesc();                   /* 0x40cdd0 - slot 0 */
    virtual ~KEzwLoader();                          /* 0x40cff0; ??_G 0x40d0b0 */
    virtual void *getObjectInfo();                  /* 0x40cde0 - slot 2 */
    /* Open `name`, fill `fmt` from the format header and publish the payload
       length through `outLen`. */
    virtual int   open(char *name, int *outLen, WaveFormat *fmt); /* 0x40ce80 */
    virtual int   read(void *dst, int count);       /* 0x40cf00 - slot 4 */
    virtual int   close();                          /* 0x40f160 - slot 5 */
    virtual int   seek(int offset, int whence);     /* 0x40cf60 */
    virtual int   tell();                           /* 0x40f230 - slot 7 */

    int   m_pos;     /* 0x104 - the payload offset, header already taken off */
    FILE *m_file;    /* 0x108 */
    int   m_total;   /* 0x10c - the payload length */

    /* INLINE: the original has no standalone constructor - the factory
       @0x40d030 carries the base call, the three stores and the vptr store. */
    KEzwLoader()
    {
        m_pos = 0;
        m_file = 0;
        m_total = 0;
    }
};

struct KWavLoader : Obj40DA10
{
    virtual void *getClassDesc();                   /* 0x40efa0 - slot 0 */
    virtual ~KWavLoader();                          /* 0x40f240; ??_G 0x40f300 */
    virtual void *getObjectInfo();                  /* 0x40efb0 - slot 2 */
    virtual int   open(char *name, int *outLen, WaveFormat *fmt); /* 0x40f070 */
    virtual int   read(void *dst, int count);       /* 0x40f100 - slot 4 */
    /* Slots 5 and 7 are byte-identical to KEzwLoader's and the original's
       linker folded them; both copies are written out in full. */
    virtual int   close();                          /* 0x40f160 (folded) */
    virtual int   seek(int offset, int whence);     /* 0x40f1a0 */
    virtual int   tell();                           /* 0x40f230 (folded) */

    int   m_pos;     /* 0x104 */
    FILE *m_file;    /* 0x108 */
    int   m_total;   /* 0x10c */

    KWavLoader()
    {
        m_pos = 0;
        m_file = 0;
        m_total = 0;
    }
};

KEzwLoader *createKEzwLoader();   /* 0x40d030 */
KWavLoader *createKWavLoader();   /* 0x40f280 */

#endif /* LOADERS_H */
