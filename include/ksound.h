#ifndef KSOUND_H
#define KSOUND_H

/* MFC-style runtime class descriptor: {name, base, createFn, pad}.
   isDerivedFrom @0x00401660 walks the base chain at +4 comparing against the
   target (stubbed in src/stubs.cpp until matched). */
struct KRuntimeClass
{
    const char    *name;      /* 0x00 */
    KRuntimeClass *base;      /* 0x04 */
    void          *createFn;  /* 0x08 */
    void          *pad;       /* 0x0c */

    int isDerivedFrom(KRuntimeClass *target);
    void *createObject();   /* 0x401680 */
};

/* Engine sound base class; virtual slot 0 returns the runtime class.
   Never instantiated here — only the slot layout matters. */
struct KSound
{
    virtual KRuntimeClass *getRuntimeClass();
};

/* Runtime-class descriptor for KSound @0x4a7300 (name -> "KSound" @0x48837c;
   the "KSoundDS" descriptor @0x4a7310 chains to it as base). Defined in
   src/ezchart.cpp with its GLOBAL marker. */
extern KRuntimeClass classKSound;

/* Global sound-object finder @0x0040f420 (cdecl, `xor eax,eax; ret` in this
   build; stubbed in src/stubs.cpp). */
KSound *findSoundObject(int *index, KRuntimeClass *cls);

#endif /* KSOUND_H */
