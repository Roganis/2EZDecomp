/* concatMatrix @0x406ad0 - a spine stub, integrated from
   wip/concatmatrix-406ad0.cpp. 68 instructions, one branch, no calls.

   TWO MEASURED RESULTS WORTH KEEPING, both recorded in CLAUDE.md from this
   function. The multiply ORDER inside a float sum is not steerable: all
   twenty-four permutations of the four terms compile byte-identically,
   because cl reassociates float adds (/Op is off) and schedules the products
   by descending memory offset of whichever operand it keeps in a register.
   What IS steerable is the operand SPELLING - `a[j] * m[k]` puts m in the
   register and makes a the memory operand, and flipping it also fixes the
   induction anchoring.

   And a FOUR-ITERATION LOOP IS UNROLLED unless the counter is explicit:
   `for (i = 0; i < 4; i++)` over this body gives 204 instructions fully
   unrolled, where `i = 4; do { ... } while (--i != 0);` keeps it rolled at 72,
   which is what the original's `mov esi,4` / `sub esi,1` / `jne` has. */

#include <string.h>

// FUNCTION: EZ2AC_UNPACKED 0x00406ad0
void concatMatrix(float *dst, const float *m)
{
    float tmp[16];
    const float *a = dst;
    float *o = tmp;
    int i = 4;
    do
    {
        o[0] = m[0] * a[0] + m[8] * a[2] + m[12] * a[3] + m[4] * a[1];
        o[1] = m[1] * a[0] + m[9] * a[2] + m[13] * a[3] + m[5] * a[1];
        o[2] = m[2] * a[0] + m[10] * a[2] + m[14] * a[3] + m[6] * a[1];
        /* column 3 sums in the order 1, 3, 0, 2 - not the other three's
           0, 2, 3, 1.  Transcribed as the original spells it. */
        o[3] = m[7] * a[1] + m[15] * a[3] + m[3] * a[0] + m[11] * a[2];
        a += 4;
        o += 4;
    }
    while (--i != 0);

    memcpy(dst, tmp, 64);
}
