#include "note.h"

/* Batch converter, defined BEFORE NoteConvert so cl cannot inline the call.
   The loop must stay in the indexed form (&src[i], &dst[i]) — cl's own
   strength reduction then assigns count->ebx / p3->ebp as the original does;
   manual pointer bumps flip the ebx/ebp allocation. */
// FUNCTION: EZ2AC_UNPACKED 0x00410fd0
void NoteConvertAll(EzNoteRaw *src, EzNote *dst, unsigned char p3, int count)
{
    int i;
    for (i = 0; i < count; i++)
        NoteConvert(&src[i], &dst[i], p3);
}

/* Per-note record converter: 13-byte raw record -> 40-byte internal note.
   Original @0x00410ef0: copies the u32 header, stamps p3 into dst[5]/dst[6],
   zeroes dst[0x18..0x24], then switch(src[4]) writes the note kind + payload. */

// FUNCTION: EZ2AC_UNPACKED 0x00410ef0
void NoteConvert(EzNoteRaw *src, EzNote *dst, unsigned char p3)
{
    *(unsigned int *)dst->b = *(unsigned int *)src->b;
    dst->b[5] = p3;
    dst->b[6] = p3;
    *(unsigned int *)(dst->b + 0x18) = 0;
    *(unsigned int *)(dst->b + 0x1c) = 0;
    *(unsigned int *)(dst->b + 0x20) = 0;
    *(unsigned int *)(dst->b + 0x24) = 0;

    switch (src->b[4])
    {
    case 0:
        dst->b[4] = 0;
        return;
    case 1:
        dst->b[4] = 1;
        dst->b[0xc] = 0;
        *(unsigned short *)(dst->b + 0xe) = *(unsigned short *)(src->b + 5);
        dst->b[0x10] = src->b[7];
        dst->b[0x11] = src->b[8];
        dst->b[0x12] = src->b[9];
        *(unsigned short *)(dst->b + 0x14) = *(unsigned short *)(src->b + 0xa);
        return;
    case 2:
        dst->b[4] = 4;
        dst->b[0xc] = src->b[5];
        return;
    case 3:
        dst->b[4] = 5;
        *(unsigned int *)(dst->b + 0xc) = *(unsigned int *)(src->b + 5);
        return;
    case 4:
        dst->b[4] = 9;
        dst->b[0xc] = src->b[5];
        return;
    case 5:
        dst->b[4] = 6;
        return;
    case 6:
        dst->b[4] = 7;
        *(unsigned int *)(dst->b + 0xc) = *(unsigned int *)(src->b + 5);
        return;
    case 7:
        dst->b[4] = 8;
        *(unsigned int *)(dst->b + 0xc) = *(unsigned int *)(src->b + 5);
        return;
    default:
        dst->b[4] += 2;
        *(unsigned int *)(dst->b + 0xc) = *(unsigned int *)(src->b + 5);
        *(unsigned int *)(dst->b + 0x10) = *(unsigned int *)(src->b + 9);
        return;
    }
}
