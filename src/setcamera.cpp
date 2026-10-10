/* KGraphics::setCamera @0x40bd40 - the #2 stub on the port spine, integrated
   at 16.41% (was the wip/setcamera-40bd40.cpp park). 1082 bytes / 245
   instructions / 4 branches / 1 call. The semantics are certain and every
   instruction is accounted for; the remaining defect is the STACK PACKER
   (see THE FRAME below), so read the sweep list before re-attempting.

   WHAT IT IS
   ----------
   The view transform. Three GVec3 pointers - eye, look-at, up - become a
   D3D row-major view matrix that is handed straight to
   IDirect3DDevice7::SetTransform(D3DTRANSFORMSTATE_VIEW = 2, &m) through
   the device at KGraphics+0x30 (slot 11, +0x2c). `this` stays in ecx for
   all 245 instructions and is only touched by the final
   `mov eax,[ecx+0x30]`, so nothing in the body may clobber ecx.

   The basis is built by Gram-Schmidt with a THREE-LEVEL fallback cascade:

     n = normalize(at - eye)                      ; forward, the Z row
     if (|at - eye| < 1e-6f) return;              ; degenerate, do nothing
     u = up - n * dot(up, n)                      ; up, rejected against n
     if (|u| < 1e-6f) {                           ; up was parallel to n
         u = ey - n * n.y;                        ; retry with world +Y
         if (|u| < 1e-6f) {
             u = ez - n * n.z;                    ; retry with world +Z
             if (|u| < 1e-6f) return;             ; give up
         }
     }
     u = normalize(u);
     x = cross(u, n);                             ; right, the X row

   The two constants are FLOAT pool literals: 1e-6f @0x488230 (the epsilon,
   compared as `fld dword ptr` promoted to double) and 1.0f @0x487644 (the
   reciprocal numerator and m44). Both resolve automatically through
   reccmp's float analysis - do NOT pin either with a GLOBAL.

   The matrix, in the frame at [esp+0x34] as sixteen floats:

       x.x  u.x  n.x  0
       x.y  u.y  n.y  0
       x.z  u.z  n.z  0
       -dot(eye,x)  -dot(eye,u)  -dot(eye,n)  1

   THE COMPONENT ORDER IS NOT CONSISTENT, and this is not a misreading - the
   x87 fld order and the addss chain shape both pin it. The direction's
   length and m43's dot are summed x, z, y; every other length and both other
   dots are summed z, y, x. The source really does spell them differently at
   different sites, so transcribe each one literally rather than factoring a
   single helper out of them.

   THE FLOAT/DOUBLE SPLIT is settled: the vector arithmetic is SSE but every
   SQUARE is x87 with a dword memory operand, the same slot read twice. That
   comes from writing the length as
   `sqrt((double)(a*a) + (double)(b*b) + (double)(c*c))` - cl computes each
   float product in x87 at double precision under MSVC's FLT_EVAL_METHOD
   licence and folds each operand into the memory form. Casting the other way,
   `(double)a * a`, poisons the whole chain into x87 including the
   subtractions; an inlined `static double len3(double,double,double)` keeps
   those in SSE but CSEs the three promotions and leaves three dead `fstp`.

   THE FRAME, and why it is stuck at 16%
   -------------------------------------
   The original's frame is 0x74 = 116 bytes and decomposes exactly:

       [esp+0x00]        float len
       [esp+0x04..0x0f]  GVec3 v   - the vector being measured/normalized
       [esp+0x10..0x1b]  GVec3 e   - the eye copy (e.x NEVER stored: its
                                     slot is reserved and dead)
       [esp+0x1c..0x27]  GVec3 t   - up, then each fallback candidate,
                                     then the cross result
       [esp+0x28..0x33]  GVec3 x   - the cross-product temporary
       [esp+0x34..0x73]  float m[16]

   The integer `mov eax,[esp+0x1c]; mov [esp+0x4],eax` pairs are STRUCT
   ASSIGNMENTS - `v = t` in the two fallbacks and `t = x` after the cross.
   Writing them as struct assignments reproduces them exactly.

   What does NOT reproduce is the RESERVATION. cl packs our four GVec3s into
   nine dwords (frame 0x64, matrix at 0x24) by overlapping `x`'s short live
   range and dropping `e.x`'s dead slot; the original reserves all thirteen.
   That is the address-exposed-aggregate signature from CLAUDE.md, but nothing
   here exposes an address, and the sixteen bytes move every esp-relative
   operand in the function - which is the whole of the remaining deficit.

   Swept, all producing BYTE-IDENTICAL output (frame 0x64, 44.90% masked):
     - helpers taking `const GVec3 &` for the two lengths and the cross
     - the same helpers taking `const GVec3 *`
     - four declaration orders of len / v / e / t / x / m
     - a fifth GVec3 local for the up vector, so `t` is only the candidate
   Earlier sweeps (24 + 12 variants) over the direction assignment order,
   struct-vs-scalar locals and which vectors are hoisted moved the masked
   score between 27% and 45% and never moved the frame.

   Next thing to try, if anyone returns to this: the aggregates may be
   reserved because the original's helpers were NOT inlined at the source
   level but were `__forceinline` free functions in another TU with pointer
   parameters, or because a `D3DMATRIX`/`D3DVECTOR` typedef from the DX
   headers carries an alignment or size cl treats differently. Neither was
   tested. */

#include <math.h>
#include "ez2acapp.h"
#include "icom.h"

// FUNCTION: EZ2AC_UNPACKED 0x0040bd40
void KGraphics::setCamera(GVec3 *eye, GVec3 *at, GVec3 *up)
{
    float len;
    GVec3 v;
    GVec3 e;
    GVec3 t;
    GVec3 x;
    float m[16];
    float nx, ny, nz, inv, dot;

    e.x = eye->x;
    e.y = eye->y;
    e.z = eye->z;

    v.x = at->x - e.x;
    v.y = at->y - e.y;
    v.z = at->z - e.z;
    len = (float)sqrt((double)(v.x * v.x) + (double)(v.z * v.z) + (double)(v.y * v.y));
    if (len < 1e-6f)
        return;

    inv = 1.0f / len;
    nx = inv * v.x;
    ny = inv * v.y;
    nz = inv * v.z;

    t.x = up->x;
    t.y = up->y;
    t.z = up->z;
    dot = t.x * nx + t.z * nz + t.y * ny;
    v.x = t.x - nx * dot;
    v.y = t.y - ny * dot;
    v.z = t.z - nz * dot;
    len = (float)sqrt((double)(v.z * v.z) + (double)(v.y * v.y) + (double)(v.x * v.x));
    if (len < 1e-6f)
    {
        t.x = 0.0f - nx * ny;
        t.y = 1.0f - ny * ny;
        t.z = 0.0f - nz * ny;
        v = t;
        len = (float)sqrt((double)(v.z * v.z) + (double)(v.y * v.y) + (double)(v.x * v.x));
        if (len < 1e-6f)
        {
            t.x = 0.0f - nx * nz;
            t.y = 0.0f - ny * nz;
            t.z = 1.0f - nz * nz;
            v = t;
            len = (float)sqrt((double)(v.z * v.z) + (double)(v.y * v.y) + (double)(v.x * v.x));
            if (len < 1e-6f)
                return;
        }
    }

    inv = 1.0f / len;
    v.x = inv * v.x;
    v.y = inv * v.y;
    v.z = inv * v.z;

    x.x = v.y * nz - v.z * ny;
    x.y = v.z * nx - v.x * nz;
    x.z = v.x * ny - v.y * nx;
    t = x;

    m[0]  = t.x;  m[1]  = v.x;  m[2]  = nx;   m[3]  = 0.0f;
    m[4]  = t.y;  m[5]  = v.y;  m[6]  = ny;   m[7]  = 0.0f;
    m[8]  = t.z;  m[9]  = v.z;  m[10] = nz;   m[11] = 0.0f;
    m[12] = 0.0f - (e.z * t.z + e.y * t.y + e.x * t.x);
    m[13] = 0.0f - (e.z * v.z + e.y * v.y + e.x * v.x);
    m[14] = 0.0f - (e.x * nx + e.z * nz + e.y * ny);
    m[15] = 1.0f;

    device->SetTransform(2, m);
}
