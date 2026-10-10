#ifndef KBASE_H
#define KBASE_H

/* The engine's common polymorphic base (KSong, EzChart, KSNDMixer... all call
   its ctor/dtor). __declspec(novtable) is load-bearing: the class owns the
   vptr slot but never stores a vftable, so its empty out-of-line ctor
   @0x401690 compiles to `mov eax,ecx; ret` and its dtor body to a bare `ret`
   (derived dtors reach it at @0x426e40, a tail call - see below). Both must be DEFINED
   IN A SEPARATE TU from any derived class or cl inlines them away:
   the ctor lives in src/constants.cpp, the dtor stub in src/stubs.cpp. */
struct __declspec(novtable) KBase
{
    KBase();                      /* 0x401690 */
    virtual void *getClassDesc(); /* slot 0 (each derived returns its own) */
    /* slot 1 @0x426e40: a bare `jmp 0x402880` - a tail call of an empty
       function the linker folded onto the shared `ret`, not a linker thunk
       as this header used to say (src/stubs.cpp). The callee is declared
       only there: adding it to this class changes other TUs' codegen. */
    virtual ~KBase();
};

#endif /* KBASE_H */
