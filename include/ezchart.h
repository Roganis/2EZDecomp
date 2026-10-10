#ifndef EZCHART_H
#define EZCHART_H

#include <string.h>
#include "ksound.h"

/* The main chart/sequence object (original class name likely "EZModule" per
   the "EZModule(LoadEZ)"/"EZModule(LoadTracks)" log strings; the "KSongPlayer"
   string @0x488a70 belongs to the separate 2-slot class @0x488a7c, see
   src/ksongplayer.cpp). Object size 0xcbd1c (factory @0x412860 news 0xcbd1c).
   Derives from a __declspec(novtable) base whose empty dtor was ICF-folded to
   the shared `ret` @0x402880 (see wip/ezchart-dtor.cpp). Layout:
   - vtable pointer at 0x0 (24-slot vtable @0x488a10)
   - attached KSound* at 0x4, name buffer from 0x8
   - per-lane groups[96] at 0x9c (stride 0x4c), filling 0x9c..0x1d1c
   - record array at 0x1d1c (stride 0x28), count at 0x94
   - group count at 0x98
   - 0x800-entry sample table at 0xc9d1c (same as EziLoader). */

struct Group4C
{
    char _pad0[2];
    char name[0x42];   /* 0x02 (address taken by getGroupInfo flag 1) */
    int  f44;          /* 0x44 last record d0 (rebuildIndex) */
    int  f48;          /* 0x48 per-lane record count (rebuildIndex) */
};

/* Pads are short/int elements ON PURPOSE: VC7.1 /GS treats char (and even
   short-ARRAY) members of a struct-typed LOCAL as string buffers and adds a
   cookie; EzChart::insert has a Record28 local and the original has no cookie
   (proven by its 0x30 frame). Offsets/size unchanged. */
struct Record28
{
    int            d0;       /* 0x00 tick/time (compared UNSIGNED) */
    unsigned char  byte4;    /* 0x04 type */
    unsigned char  byte5;    /* 0x05 lane */
    unsigned short _w6;      /* 0x06 */
    int            _pad1[3]; /* 0x08 */
    unsigned short word14;   /* 0x14 duration (rescaled for type-1 records) */
    unsigned short _w16;     /* 0x16 */
    int            _pad2[4]; /* 0x18 .. 0x28 */
};

struct RecInfo { int lane; int index; };

/* Class-registry record @0x4a7370 ({?, link -> 0x4a735c, factory 0x412860, 0});
   returned by vtable slot 0. Defined in src/ezchart.cpp with its GLOBAL marker. */
struct EzClassInfo
{
    void *f0;       /* 0x00 */
    void *f4;       /* 0x04 -> 0x4a735c (sibling descriptor) */
    void *factory;  /* 0x08 -> 0x412860 (creates a new EzChart) */
    void *fC;       /* 0x0c */
};
extern EzClassInfo g_ezChartClassInfo;

/* Three-key record comparator @0x00410920 (d0 unsigned asc, byte5 asc,
   byte4 desc). Lives in src/ezcmp.cpp — its own TU so callers cannot inline it. */
int recordCompare(const Record28 *a, const Record28 *b);

/* vtable @0x488a10, pinned from src/ezchart.cpp now that all 24 slots
   are matched. Slot order below is load-bearing (indirect calls use byte
   offsets, e.g. search = [eax+0x2c], rebuildIndex = [eax+0x5c]). The class is
   never constructed in our code, so unmatched slots may stay declared-only. */
/* EzChart's base: a __declspec(novtable) root with a runtime-class getter at
   slot 0 and a virtual destructor at slot 1. Its destructor is an EMPTY
   out-of-line function, which the original's linker ICF-folded onto the shared
   bare `ret` @0x402880 - so ~EzChart's `call 0x402880` at EH state -1 is this
   base's destructor, not the logger that owns the address. It must be DEFINED
   IN A SEPARATE TU (src/ezbase.cpp): a visible empty body gets inlined into
   ~EzChart and takes the whole EH frame with it. */
struct __declspec(novtable) EzBase
{
    EzBase();                              /* 0x401690 (folded onto KBase's) */
    virtual EzClassInfo *getClassInfo();   /* slot 0 */
    virtual ~EzBase();                     /* slot 1; 0x402880 (folded) */
};

struct EzChart : EzBase
{
    /* Defined INLINE on purpose: the original has no out-of-line EzChart
       constructor at all - its whole body is expanded into the factory
       @0x412860 (base ctor call, vptr store, f4 = 0, reset(), then a second
       clear of the sample table). */
    EzChart();

    int      f4;                  /* 0x04 attached KSound* (setSound) */
    char     b8;                  /* 0x08 name buffer start */
    char     _pad1[0x48 - 0x09];
    char     b48;                 /* 0x48 */
    char     _pad2[0x88 - 0x49];
    int      i88;                 /* 0x88 rate*4 */
    float    f8c;                 /* 0x8c */
    float    f90;                 /* 0x90 */
    int      i94;                 /* 0x94 record count */
    int      i98;                 /* 0x98 group count */
    Group4C  groups[96];          /* 0x9c .. 0x1d1c */
    Record28 records[0x5000];     /* 0x1d1c .. 0xc9d1c */
    int      sampleTable[0x800];  /* 0xc9d1c */

    virtual EzClassInfo *getClassInfo();                     /* 00 0x410960 */
    virtual ~EzChart();                    /* 01 ??_G 0x412950, ??1 0x4128f0 */
    virtual int  setSound(KSound *snd);                      /* 02 0x410970 */
    /* 03: `mov eax,[ecx+4]; ret` - hands back the attached sound. Empty
       enough that the original's linker ICF-folded it onto Obj40F380::getA
       (src/accessors.cpp), which owns the address. */
    virtual int  getSound();                                 /* 03 0x40f380 (folded) */
    virtual int  load(char *name);                           /* 04 0x411720 */
    virtual int  loadTracks(int lane, char *name, int start, int count); /* 05 0x411920 */
    virtual void copyLane(int dstLane, int srcLane);         /* 06 0x412790 */
    virtual void copyLanes(int dstLane, int srcLane, int count); /* 07 0x4127e0
        (distinct name, not an overload: MSVC reverses the vtable order of
        adjacent same-name virtual overloads) */
    virtual void getState(int *out, int flags);              /* 08 0x411b80 */
    virtual void setState(int *in, int flags);               /* 09 0x411cd0 */
    virtual int  getGroupInfo(int idx, int *out, int flags); /* 10 0x411d30 */
    virtual int  search(int lane, int value, int mode, int filter,
                        Record28 *outRec, RecInfo *outInfo); /* 11 0x412020 */
    virtual int  findNextRecord(int filter, Record28 *out,
                                RecInfo *info);              /* 12 0x412100 */
    virtual int  findPrevRecord(int filter, Record28 *out,
                                RecInfo *info);              /* 13 0x4121b0 */
    virtual int  find(int lane, int nth, Record28 *outRec, int *outInfo); /* 14 0x412250 */
    virtual int  findFirst(int lane, Record28 *outRec, RecInfo *outInfo); /* 15 0x411010 */
    virtual int  findLast(int lane, Record28 *outRec, RecInfo *outInfo);  /* 16 0x411030 */
    virtual int  getRecord(RecInfo *info, Record28 *out);    /* 17 0x4122f0 */
    virtual int  setRecord(RecInfo *info, Record28 *src);    /* 18 0x412340 */
    virtual int  insert(int lane, int tick, Record28 *src,
                        RecInfo *out);                       /* 19 0x4124b0 */
    virtual int  insertAt(Record28 *rec, RecInfo *pos);      /* 20 0x412560 */
    virtual int  insertBefore(Record28 *rec, RecInfo *info); /* 21 0x4125d0 */
    virtual int  removeRecord(RecInfo *info);                /* 22 0x412630 */
    virtual void rebuildIndex();                             /* 23 0x4112a0 */

    /* non-vtable members */
    void reset();                                             /* 0x4109d0 */
    int  findPrev(int lane, int before, int filter);          /* 0x411e00 */
    void rescale(int newRate, int oldRate);                   /* 0x411c10 */
    int  findFrom(int lane, unsigned int pos, int filter);    /* 0x411e60 mode 0 */
    /* The index-keyed twin: first match strictly after `from`. */
    int  findAfter(int lane, int from, int filter);           /* 0x411d90 */
    int  findNextAfter(int lane, unsigned int time, int filter); /* 0x411f40 mode 1 */
    int  findPrevAt(int lane, unsigned int time, int filter); /* 0x411ed0 mode 2 */
    int  findBefore(int lane, unsigned int time, int filter); /* 0x411fb0 mode 3 */
    void copyRecords(int dst, int src, int count);            /* 0x412390 */
    int  locate(Record28 *key, RecInfo *out);                 /* 0x4123e0 */
    int  append(Record28 *src, RecInfo *out);                 /* 0x412450, in tables.cpp */
    void extractLane(int lane, int track);                    /* 0x412690, stubbed */
    void mergeTracks(int first, int count);                   /* 0x411310, stubbed */
    /* names dodge winbase.h's `#define DecryptFile DecryptFileA` */
    int  decryptFile2(char *name);                            /* 0x410c90, stubbed (wip/crypt2.cpp) */
    int  decryptFile(char *name);                             /* 0x410a40, stubbed (wip/crypt.cpp) */
    int  loadEZ(char *name);                                  /* 0x411460, stubbed */
};

inline EzChart::EzChart()
{
    f4 = 0;
    reset();
    memset(sampleTable, 0, sizeof(sampleTable));
}

EzChart *createEzChart();                                     /* 0x412860 */

#endif /* EZCHART_H */
