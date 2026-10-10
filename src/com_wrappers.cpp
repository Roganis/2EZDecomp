#include "icom.h"
#include "obj40d4e0.h"

/* Thin wrappers around COM-style (__stdcall) virtual calls on an engine object
   held in member field 0xb8. Interface vtable layout is provisional (only the
   called slots matter); identities TBD. */

/* (IThing moved to include/icom.h.) */


/* Slots 8..11 and 13 of Obj40D4E0's vftable @0x488320 (include/obj40d4e0.h):
   the guarded wrappers around the sound buffer at +0xb8. They were written
   against a placeholder `EngineObj` view until 2026-10-01, when the
   vftable model was extended to all 19 slots. */

// FUNCTION: EZ2AC_UNPACKED 0x0040d350
int Obj40D4E0::get8()
{
    if (fb8 != 0)
    {
        int out;
        fb8->query8(&out);
        return out;
    }
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d3f0
int Obj40D4E0::get4()
{
    if (fb8 != 0)
    {
        int out;
        fb8->query4(&out, 0);
        return out;
    }
    return 0;
}

/* Three write-side wrappers on the same interface: guard the field, then a
   COM-style virtual with `this` pushed as the first stack argument. Unlike the
   get* trio above they discard the result, so the whole call is the if-body. */

// FUNCTION: EZ2AC_UNPACKED 0x0040d2e0
void Obj40D4E0::put15(int v)
{
    if (fb8 != 0)
        fb8->set15(v);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d330
void Obj40D4E0::put17(int v)
{
    if (fb8 != 0)
        fb8->set17(v);
}

/* Release-and-reset @0x0040b8f0 (__thiscall): release the field-0x458 interface
   (if any), then set field 0 and field 0x440 to 1. */
struct ObjB8F0
{
    int      field_0x0;                    /* 0x000 */
    char     _pad0[0x440 - 4];
    int      field_0x440;                  /* 0x440 */
    char     _pad1[0x458 - 0x440 - 4];
    IThing  *field_0x458;                  /* 0x458 */
    void reset();
};

// FUNCTION: EZ2AC_UNPACKED 0x0040b8f0
void ObjB8F0::reset()
{
    if (field_0x458 != 0)
        field_0x458->release();
    field_0x0 = 1;
    field_0x440 = 1;
}

/* Same guarded-query shape as get4/get7/get8, but the empty case yields
   -10000 rather than 0. */
// FUNCTION: EZ2AC_UNPACKED 0x0040d300
int Obj40D4E0::get6()
{
    if (fb8 != 0)
    {
        int out;
        fb8->query6(&out);
        return out;
    }
    return -10000;
}

/* A second object holding the same interface kind at +0x30, handing it a
   pointer to its own +0x140 block. */
struct Obj40BB00
{
    char    _pad0[0x30];
    IThing *field_0x30;   /* 0x30 */
    char    _pad1[0x140 - 0x34];
    int     block140;     /* 0x140 */

    void doSub();      /* 0x0040bb00 */
    void doOp7(int v);  /* 0x0040a470 */
    void doOp10(unsigned int color); /* 0x0040a730 - device->Clear(0, 0, 3, color, 1.0f, 0) */
};

// FUNCTION: EZ2AC_UNPACKED 0x0040bb00
void Obj40BB00::doSub()
{
    field_0x30->sub11(1, &block140);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040a470
void Obj40BB00::doOp7(int v)
{
    field_0x30->op20(7, v);
}

/* The 1.0f argument is pushed as its raw bit pattern (0x3f800000) - a float
   parameter passed by value, not a pointer. */
// FUNCTION: EZ2AC_UNPACKED 0x0040a730
void Obj40BB00::doOp10(unsigned int v)
{
    field_0x30->op10(0, 0, 3, v, 1.0f, 0);
}

/* A third interface holder, this one at +0x460, handed a pointer to its
   owner's +0x4c8 block. Its slot 7 takes two arguments, unlike IThing's, so
   it is a different interface. */
struct IBlockSink
{
    virtual int __stdcall v0(); virtual int __stdcall v1();
    virtual int __stdcall v2(); virtual int __stdcall v3();
    virtual int __stdcall v4(); virtual int __stdcall v5();
    virtual int __stdcall v6();
    virtual int __stdcall submit(void *block, int arg);   /* slot 7 -> +0x1c */
};

struct Obj40B980
{
    char        _pad0[0x460];
    IBlockSink *field_0x460;   /* 0x460 */
    char        _pad1[0x4c8 - 0x464];
    int         block4c8;      /* 0x4c8 */

    void submit(int arg);
};

// FUNCTION: EZ2AC_UNPACKED 0x0040b980
void Obj40B980::submit(int arg)
{
    field_0x460->submit(&block4c8, arg);
}
