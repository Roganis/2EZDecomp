#ifndef KSONG_H
#define KSONG_H

#include "ksound.h"   /* KRuntimeClass */

#include "kbase.h"

/* The objects handed to KSong::v7 as the source argument: only virtual slot 3
   (an argument-less getter) is called. Declared, never instantiated. */
struct ISongSource
{
    virtual void s0();
    virtual void s1(int);
    virtual void s2();
    virtual int s3();
};

/* The engine objects stored in the registries: slot 1 is a Release-style
   call; slots 2..4 are the init protocol used by the two creators. */
struct IReleasable
{
    virtual void v0();
    virtual void v1(int);
    virtual int v2(void *a);
    virtual int v3(int a);
    virtual int v4(void *a);
};

/* The engine's create-object-by-class-name factory: takes the name in ecx
   with no stack args (modeled as __fastcall). STUB @0x401680 (stubs.cpp). */
/* (The factory these two hand their class descriptor to is
   KRuntimeClass::createObject @0x401680 - see src/classinfo.cpp. It reads the
   creator function out of the descriptor at +8 and tail-jumps it, which is what
   identifies the `cls` argument below as a descriptor rather than a name.) */

/* KSong: 0x4c-byte object (factory @0x4108a0 news 0x4c), vtable @0x488874
   with 10 slots. Name evidence: "KSong"/"KSongEZ" strings follow the vtable
   in .rdata; v4 @0x410530 logs "CreateSong : %s". Derives from KBase (ctor
   @0x4107f0 calls the base ctor @0x401690; ~KSong @0x410840 calls the dtor
   thunk @0x426e40). Two 8-slot object registries. */
struct KSong : KBase
{
    IReleasable *arr1[8];   /* 0x04 .. 0x24 */
    int          count1;    /* 0x24 */
    IReleasable *arr2[8];   /* 0x28 .. 0x48 */
    int          count2;    /* 0x48 */

    KSong();                              /* 0x4107f0 */
    virtual void *getClassDesc();         /* 0 @0x4104c0 -> descriptor @0x4a734c */
    virtual ~KSong();                     /* 1 ??1 @0x410840 (src/ksongdtor.cpp),
                                               ??_G @0x410900 */
    virtual int v2();                     /* 2 @0x4104d0 clear both registries */
    virtual int v3();                     /* 3 @0x410520 = v6() + v9() */
    virtual IReleasable *v4(char *name, KRuntimeClass *cls, void *c); /* 4 @0x410530 */
    virtual int  release1(IReleasable *); /* 5 @0x4105d0 */
    virtual int v6();                     /* 6 @0x410630 release+clear arr1 */
    virtual IReleasable *v7(ISongSource *src, KRuntimeClass *cls, void *c); /* 7 @0x410680 */
    virtual int  release2(IReleasable *); /* 8 @0x410740 */
    virtual int v9();                     /* 9 @0x4107a0 release+clear arr2 */
};

KSong *createKSong();                     /* 0x4108a0 */

#endif /* KSONG_H */
