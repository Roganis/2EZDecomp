/* Obj405F80::releaseAll @0x405580 - drop every DirectDraw/Direct3D interface
   the object holds.

   The cooperative level is put back to DDSCL_NORMAL on the DirectDraw object
   first, then the seven interfaces go in the original's order: the device at
   +0x30, then the three surfaces at +0x38, +0x3c and +0x40, then the one at
   +0x34, then Direct3D at +0x2c, and DirectDraw itself last. Each slot is
   cleared as it goes.

   Two of the releases are checked: the device's and DirectDraw's own. A
   non-zero refcount from either is logged and makes the function answer
   0x8200000c instead of zero - and note the DirectDraw check is SKIPPED
   entirely when that slot was already null, rather than treated as zero,
   which is why the two tests are not symmetrical. */

#include "ez2acapp.h"
#include "obj405f80.h"

/* The interfaces, viewed for the two slots this uses. Both are COM-style -
   `this` is pushed rather than passed in ecx. */
struct IComObj
{
    virtual int __stdcall v0(); virtual int __stdcall v1();
    virtual int __stdcall release();                   /* slot 2  -> +0x08 */
    virtual int __stdcall v3();  virtual int __stdcall v4();
    virtual int __stdcall v5();  virtual int __stdcall v6();
    virtual int __stdcall v7();  virtual int __stdcall v8();
    virtual int __stdcall v9();  virtual int __stdcall v10();
    virtual int __stdcall v11(); virtual int __stdcall v12();
    virtual int __stdcall v13(); virtual int __stdcall v14();
    virtual int __stdcall v15(); virtual int __stdcall v16();
    virtual int __stdcall v17(); virtual int __stdcall v18();
    virtual int __stdcall v19();
    /* slot 20 -> +0x50: SetCooperativeLevel(hwnd, flags). */
    virtual int __stdcall setCoopLevel(int hwnd, int flags);
};

extern void Log(const char *fmt, ...);   /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x00405580
int Obj405F80::releaseAll()
{
    int devRefs;
    int ddRefs;

    devRefs = 0;
    ddRefs = 0;

    if (f28[0] != 0)
        ((IComObj *)f28[0])->setCoopLevel(f04[0], 8);

    if (f28[2] != 0)
    {
        devRefs = ((IComObj *)f28[2])->release();
        if (devRefs > 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00487804
            Log("KGraphics: Error: D3DDevice object is still referenced!");
        }
    }
    f28[2] = 0;

    if (f28[4] != 0)
    {
        ((IComObj *)f28[4])->release();
        f28[4] = 0;
    }
    if (f28[5] != 0)
    {
        ((IComObj *)f28[5])->release();
        f28[5] = 0;
    }
    if (f28[6] != 0)
    {
        ((IComObj *)f28[6])->release();
        f28[6] = 0;
    }
    if (f28[3] != 0)
    {
        ((IComObj *)f28[3])->release();
        f28[3] = 0;
    }
    if (f28[1] != 0)
    {
        ((IComObj *)f28[1])->release();
        f28[1] = 0;
    }

    if (f28[0] != 0)
    {
        ddRefs = ((IComObj *)f28[0])->release();
        if (ddRefs > 0)
        {
            // STRING: EZ2AC_UNPACKED 0x004877d0
            Log("KGraphics: Error: DDraw object is still referenced!");
        }
        f28[0] = 0;
    }

    /* One conjunction, not two gotos: separate tests put the error tail
       inline and the zero return after it, which is the wrong way round. */
    if (ddRefs != 0 || devRefs != 0)
        return 0x8200000c;
    return 0;
}

/* Obj405F80::m409320 @0x409320 - a bare `jmp 0x408c80`, i.e. the one-line
   forwarder shape: the body is `m408c80();` in tail position, and the target
   must live in another TU or cl inlines it and the jmp disappears.

   The cast is the honest part. m408c80 @0x408c80 is modelled as a KGraphics
   method (it unbinds the four texture stages through the D3D device), and the
   original passes `this` straight through in ecx - so THE TWO NAMES ARE ONE
   CLASS. Obj405F80 is the graphics singleton: it is published into 0xd36cac,
   its ctor writes the interface slots at +0x28..+0x48, and CLAUDE.md's own
   reading of the vtable indices puts IDirectDraw7 at +0x28 and
   IDirect3DDevice7 at +0x30 of "the graphics singleton". Merging the two
   models is a real change that would touch every KGraphics method, so it is
   left for a sitting that can run the full match against it; the cast keeps
   this function honest in the meantime. */
// FUNCTION: EZ2AC_UNPACKED 0x00409320
void Obj405F80::m409320()
{
    ((KGraphics *)this)->m408c80();
}
