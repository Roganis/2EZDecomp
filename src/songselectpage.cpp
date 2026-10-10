/* SongSelectDirector::m439f00 @0x439f00 - the plain list page, the draw
   update @0x443380 dispatches to for every non-wheel mode arm (40 call
   sites). Decoded 2026-08-26.

   The entry loop is drawWheel's wrap (i*100 - f_c4 into +-50*f7c) at DOUBLE
   spacing: d is scaled by 2 and only entries with -400 < d < 800 draw. Per
   entry: brightness 255 - |(int)d| inside +-100 and 155 outside; a
   perspective scale 176 / (1 +- d/100) clamped to 112 outside +-200; a
   cosine-arc y = 444 - cos[(int)d + 0x34c & 0x3ff]*500 - 500 + 480 - 56 + 88
   (five constants the original keeps unfolded - written in its order); and
   x = angleToY(d) - 274 (the matched @0x434d70). Each entry goes to
   m431ae0(i, x, y, scale, bright) - the 1.8 KB per-entry draw, stubbed.

   The tail: m438d30 (STUB), the m_bccc8 refresh, and the class cell -
   rows[f84]'s dword at +0x49c (the entry stride 0x374 folded into the
   scaled-index addressing; +0x49c is past the modeled 0x374 row, so it is
   spelled as the raw offset the listing has) indexes the texture table at
   +0xbff74 (= f_bff64 + 4) into row 1 of m_be8d8, which seeks to 0x78 once
   done and is refreshed as the tail call.

   EIGHT FUNCTIONS, ONE ADDRESS. update @0x443380 calls this page from every
   list-mode arm of its five `switch (f78)` blocks, and the original keeps
   every arm - even three consecutive `mov ecx,esi; call 0x439f00; jmp X`
   arms with the same X - as its own block. cl cross-jumps arms that call
   the same function, so at compile time each mode called its OWN function
   and the linker ICF-folded the eight identical bodies onto 0x439f00: mode 0
   is m439f00, modes 1-5, 10 and 11 are pageModeN below. The body is written
   once, in include/songselectpage439f00.inl, and every copy carries the
   `folded` marker. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "songselect.h"

extern float g_cosTable[1024];   /* 0x4a6228 (src/globals.cpp) */
extern float __stdcall angleToY(float a);   /* 0x434d70 (src/accessors.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::m439f00()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode1()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode2()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode3()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode4()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode5()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode10()
{
#include "songselectpage439f00.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00439f00 folded
void SongSelectDirector::pageMode11()
{
#include "songselectpage439f00.inl"
}

/* SongSelectDirector::m434080 @0x434080 - the BPM readout at (x, y). Both
   row floats (+0x344 and +0x348) are printed through a magnitude-picked
   zero-padding format (00%3.2f / 0%3.2f / %3.2f - always six characters,
   "DDD.DD") into two stack buffers, then drawn right-to-left from the digit
   glyphs at f_bc638: 7px step, 5px around the point, leading zeros trimmed
   off the front (stop index) and a trailing ".00" or "0" off the back
   (start index). THE +0x348 TEXT IS DRAWN FIRST - the frame arithmetic is
   what says so (the first walk's base is the second sprintf's buffer) -
   and only when +0x344 is non-zero does the range dash f_bc664 follow at
   wx-3 with the +0x344 text 11px on. The walking coordinate `wx` lives in
   the x parameter's slot and the row pointer in `entry`'s (both reused);
   `course` is unused. The member reads are spelled per-test, as the
   original re-reads them. */
// FUNCTION: EZ2AC_UNPACKED 0x00434080
void SongSelectDirector::m434080(float x, float y, int entry, int course)
{
    int  stop2;
    int  stop1;
    int  start2;
    char bufB[8];
    char bufA[8];
    float wx;
    int  i;
    int  g;

    stop2 = 0;
    start2 = 6;
    stop1 = 0;
    wx = x;
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    /* The row pointer lives in the entry PARAMETER's slot - the original
       overwrites the argument once the index is consumed. */
    entry = (int)((char *)this + entry * 0x374);
    if (*(float *)(entry + 0x344) >= 100.0f)
        // STRING: EZ2AC_UNPACKED 0x00491c4c
        sprintf(bufA, "%3.2f", (double)*(float *)(entry + 0x344));
    else if (*(float *)(entry + 0x344) >= 10.0f)
        // STRING: EZ2AC_UNPACKED 0x00491c44
        sprintf(bufA, "0%3.2f", (double)*(float *)(entry + 0x344));
    else
        // STRING: EZ2AC_UNPACKED 0x00491c3c
        sprintf(bufA, "00%3.2f", (double)*(float *)(entry + 0x344));
    if (*(float *)(entry + 0x348) >= 100.0f)
        sprintf(bufB, "%3.2f", (double)*(float *)(entry + 0x348));
    else if (*(float *)(entry + 0x348) >= 10.0f)
        sprintf(bufB, "0%3.2f", (double)*(float *)(entry + 0x348));
    else
        sprintf(bufB, "00%3.2f", (double)*(float *)(entry + 0x348));

    if (bufB[0] == '0')
        stop1 = 1 + (bufB[1] == '0');
    i = 6;
    if (bufB[4] == '0')
    {
        if (bufB[5] == '0')
            i = 3;
    }
    else if (bufB[5] == '0')
    {
        i = 5;
    }
    for (i = i - 1; i >= stop1; i--)
    {
        g = bufB[i] - '0';
        if (g == -2)
            g = 10;
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc638[g], wx, y, 16.0f, 16.0f, -1);
        if (bufB[i - 1] - '0' == -2 || g == 10)
            wx -= 5.0f;
        else
            wx -= 7.0f;
    }

    if (*(float *)(entry + 0x344) == 0.0f)
        return;
    if (bufA[0] == '0')
        stop2 = 1 + (bufA[1] == '0');
    if (bufA[4] == '0')
    {
        if (bufA[5] == '0')
            start2 = 3;
    }
    else if (bufA[5] == '0')
    {
        start2 = 5;
    }
    /* the separator's x taken BEFORE the step, drawn after it */
    float sx = wx - 3.0f;
    wx -= 11.0f;
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bc664, sx, y, 16.0f, 16.0f, -1);
    for (i = start2 - 1; i >= stop2; i--)
    {
        g = bufA[i] - '0';
        if (g == -2)
            g = 10;
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc638[g], wx, y, 16.0f, 16.0f, -1);
        if (bufA[i - 1] - '0' == -2 || g == 10)
            wx -= 5.0f;
        else
            wx -= 7.0f;
    }
}

/* SongSelectDirector::m4394e0 @0x4394e0 - the select page's difficulty strip,
   scroll needles and info panel (678 instructions). Decoded 2026-08-26.

   - The strip's eased slide chases (g_4abd14 - 1) * 46 a third of the way per
     tick and snaps inside half a pixel (f_c047c).
   - The difficulty count at this+0xa4 (inside the modeled entries[0], so it
     is spelled raw): 4 / 3 / (lv1 != 0) + 1 off the row's level floats.
   - FOUR VARIANTS on f_20c6d4, emitted default-first (the switch's source
     order): each resolves f70 - default takes (int)lv0 outright, cases 1/3/4
     take their own level or resolveLevel(row-by-value, 0/1/3) when it is
     zero, testing through a LOCAL SongLevels copy - and draws the four
     plates at x = 371/417/463/509, y = 362, lighting its own column
     (default lights the first). Every call's first argument is f7c - 7.
   - Then the m_20cf48 refresh with its rest position f_c047c+394 / 387, the
     info panel's fade level f_c04b4 stepped +-30 toward 255/0 on f_20c6bc,
     the (24,292) panel pair with the per-mode second texture (mode 10 ->
     f_c04ac, mode 1 -> f_c04b0, else f_c04a4), the wheel fraction
     f_20c640 = f_c4 * (1 / (float)f7c) capped to 100 at 99.8, and the two
     rotated needles at x=12, y = f_20c640 * 3 + 110 (m404250, STUB park).
   - On the way out it publishes f7c to 0x1b2ebf4 and g_4abd14 to 0x1b2ebb8. */

extern int g_modeIndex;   /* 0x1b2e548 (src/modename.cpp) */
extern int g_4abd14;      /* src/gamestate.cpp */
extern int g_1b2eb6c;     /* src/ez2acapp.cpp - hit-percent mode */
extern int g_37dcf30;     /* src/gamestate.cpp - the disc pulse */
extern int g_37dcf34;
extern int g_37dcf38;
extern int g_1b2ebf4;     /* src/gamestate.cpp */
extern int g_1b2ebb8;     /* src/gamestate.cpp */

/* (Moved beside its only caller, m4394e0 @0x4394e0: the original keeps
   edx and the row pointer live across the call, which cl only does for a
   body it can see.) Resolve the effective chart level for a requested difficulty `mode`. Each
   difficulty's level float being 0 means it is unavailable, so the request
   falls back to the next-easier chart (4->3->1->0); mode 2 has none. The
   `!= 0.0f` guards compile to the ucomiss/lahf/test-ah/jp idiom, and the
   truncation to cvttss2si. `this` is passed (thiscall) but unused. Written as
   a plain `for(;;) switch` with `default: return 0` - case 2 folds into the
   default and the switch's own range check IS the loop-back test, which is
   what puts the shared `xor eax,eax; ret` out of line (an early
   `if (mode > 4)` guard added a redundant loop-top check). */
// FUNCTION: EZ2AC_UNPACKED 0x00432310
__declspec(noinline) int SongSelectDirector::resolveLevel(SongLevels row, int mode)
{
    for (;;)
    {
        switch (mode)
        {
        case 0:
            return (int)row.lv0;
        case 1:
            if (row.lv1 != 0.0f)
                return (int)row.lv1;
            mode = 0;
            break;
        case 3:
            if (row.lv2 != 0.0f)
                return (int)row.lv2;
            mode = 1;
            break;
        case 4:
            if (row.lv3 != 0.0f)
                return (int)row.lv3;
            mode = 3;
            break;
        default:
            return 0;
        }
    }
}

/* The row's SongLevels view: base +0x12c puts lv0..lv3 on the +0x340..+0x364
   displacements the listing has. */
#define SSP_LV(self, e) ((SongLevels *)((char *)(self) + 0x12c + (e) * 0x374))

// FUNCTION: EZ2AC_UNPACKED 0x004394e0
void SongSelectDirector::m4394e0()
{
    float t;

    t = (float)(g_4abd14 - 1) * 46.0f;
    f_c047c += (t - f_c047c) * 0.33333334f;
    if (0.5f > t - f_c047c && t - f_c047c > -0.5f)
        f_c047c = t;

    if ((int)SSP_LV(this, f84)->lv3 != 0)
        *(int *)((char *)this + 0xa4) = 4;
    else if ((int)SSP_LV(this, f84)->lv2 != 0)
        *(int *)((char *)this + 0xa4) = 3;
    else
        *(int *)((char *)this + 0xa4) = ((int)SSP_LV(this, f84)->lv1 != 0) + 1;

    switch (f_20c6d4)
    {
    default:
        f70 = (int)SSP_LV(this, f84)->lv0;
        if (SSP_LV(this, f84)->lv0 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv0, 371.0f, 362.0f, 0, 0, 1);
        if (SSP_LV(this, f84)->lv1 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv1, 417.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv2 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv2, 463.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv3 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv3, 509.0f, 362.0f, 0, 0, 0);
        break;

    case 4:
        {
            /* An int-array copy, not a SongLevels local: the type's char
               pads would grow a /GS cookie the original does not have.
               memcpy inlines to the same rep movsl. */
            int lvw[0x374 / 4];
            memcpy(lvw, SSP_LV(this, f84), 0x374);
            f70 = (*(float *)((char *)lvw + 0x238) != 0.0f)
                ? (int)*(float *)((char *)lvw + 0x238)
                : resolveLevel(*SSP_LV(this, f84), 3);
        }
        if (SSP_LV(this, f84)->lv0 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv0, 371.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv1 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv1, 417.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv2 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv2, 463.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv3 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv3, 509.0f, 362.0f, 0, 0, 1);
        break;

    case 3:
        {
            /* An int-array copy, not a SongLevels local: the type's char
               pads would grow a /GS cookie the original does not have.
               memcpy inlines to the same rep movsl. */
            int lvw[0x374 / 4];
            memcpy(lvw, SSP_LV(this, f84), 0x374);
            f70 = (*(float *)((char *)lvw + 0x22c) != 0.0f)
                ? (int)*(float *)((char *)lvw + 0x22c)
                : resolveLevel(*SSP_LV(this, f84), 1);
        }
        if (SSP_LV(this, f84)->lv0 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv0, 371.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv1 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv1, 417.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv2 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv2, 463.0f, 362.0f, 0, 0, 1);
        if (SSP_LV(this, f84)->lv3 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv3, 509.0f, 362.0f, 0, 0, 0);
        break;

    case 1:
        {
            /* An int-array copy, not a SongLevels local: the type's char
               pads would grow a /GS cookie the original does not have.
               memcpy inlines to the same rep movsl. */
            int lvw[0x374 / 4];
            memcpy(lvw, SSP_LV(this, f84), 0x374);
            f70 = (*(float *)((char *)lvw + 0x220) != 0.0f)
                ? (int)*(float *)((char *)lvw + 0x220)
                : resolveLevel(*SSP_LV(this, f84), 0);
        }
        if (SSP_LV(this, f84)->lv0 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv0, 371.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv1 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv1, 417.0f, 362.0f, 0, 0, 1);
        if (SSP_LV(this, f84)->lv2 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv2, 463.0f, 362.0f, 0, 0, 0);
        if (SSP_LV(this, f84)->lv3 != 0.0f)
            drawDifficulty(f7c - 7, (int)SSP_LV(this, f84)->lv3, 509.0f, 362.0f, 0, 0, 0);
        break;
    }

    {
        Obj402860 *o = &m_20cf48;

        o->m403940();
        o->f214 = f_c047c + 394.0f;
        o->f218 = 387.0f;
    }

    if (f_20c6bc >= 1)
    {
        f_c04b4 += 30;
        if (f_c04b4 >= 255)
            f_c04b4 = 255;
    }
    else if (f_20c6bc == 0)
    {
        f_c04b4 += -30;
        if (f_c04b4 <= 0)
            f_c04b4 = 0;
    }

    {
        if (g_modeIndex != 0xa)
        {
            if (g_modeIndex != 1)
            {
                    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
                    ((KGraphics *)g_sub1b2f054)->drawTex(f_c04a8, 24.0f, 292.0f, 0.0f, 0.0f,
                        f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
                    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                    ((KGraphics *)g_sub1b2f054)->drawTex(f_c04a4, 24.0f, 292.0f, 0.0f, 0.0f,
                        f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
            }
            else
            {
                    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
                    ((KGraphics *)g_sub1b2f054)->drawTex(f_c04a8, 24.0f, 292.0f, 0.0f, 0.0f,
                        f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
                    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                    ((KGraphics *)g_sub1b2f054)->drawTex(f_c04b0, 24.0f, 292.0f, 0.0f, 0.0f,
                        f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
            }
        }
        else
        {
                ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
                ((KGraphics *)g_sub1b2f054)->drawTex(f_c04a8, 24.0f, 292.0f, 0.0f, 0.0f,
                    f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                ((KGraphics *)g_sub1b2f054)->drawTex(f_c04ac, 24.0f, 292.0f, 0.0f, 0.0f,
                f_c04b4 << 24 | f_c04b4 << 16 | f_c04b4 << 8 | f_c04b4);
        }
    }

    g_1b2ebf4 = f7c;
    {
        float q = 1.0f / (float)f7c;

        f_20c640 = *(volatile float *)&f_c4 * q;
    }
    if (f_20c640 >= 99.8f)
        f_20c640 = 100.0f;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->m404250(f_20c0d0, 12.0f,
        f_20c640 * 3.0f + 110.0f, 0.0f, 0.0f, 0.0f, 0xffffffff);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->m404250(f_20c0cc, 12.0f,
        f_20c640 * 3.0f + 110.0f, 0.0f, 0.0f, 0.0f, 0xffffffff);
    g_1b2ebb8 = g_4abd14;
}

/* SongSelectDirector::m431ae0 @0x431ae0 - one list entry, and the song DISC.
   Decoded 2026-08-26 (485 instructions, the workqueue's flagged x87/SSE
   sitting).

   UNFOCUSED (scale < 176, or another entry holds the focus): the entry's
   thumb - the per-row texture at view+0 - under the disc base f_bc44c and
   the ring f_bc450 (3px larger), each at (x - s/2, y - s/2) sized s, the
   brightness splatted into RGB. Reaching full scale LATCHES the focus:
   f_f0 = i, state 1, swing angle 0, step -30.

   FOCUSED: state 1 loads the disc art - system\disc\<name>[-hd/-shd/-ex]
   .bmp into the named hotspot sprites (slot 2 unused; hit-percent mode
   loads cv2disc_%02d instead) - then state 2 retargets the difficulty
   index from f_20c6d4, reversing the swing step when it changes. The disc
   is drawn TWICE additively at the pulse size (float)g_37dcf38 + s, centre
   nudged by g_37dcf30/34, rotated by the swing angle f_fc; the difficulty
   art f_104[idx] shows once the swing passes 180, and the ring draws at
   angle 0 unless the screen is in phase 7. The step ladder aims the swing
   at per-difficulty rest angles (30/90/150 past 180, or spinning down by
   a sixth), mirrored into f_c0e40, the art published to row 2 of m_c09e8,
   and the angle advanced inside (-360, 1080) with a two-arm clamp. At
   exactly 180 the state re-arms. */
// FUNCTION: EZ2AC_UNPACKED 0x00431ae0
void SongSelectDirector::m431ae0(int i, float x, float y, float s, int bright)
{
    char buf[260];
    int  n;
    int  idx;

    if (s >= 176.0f)
    {
        if (f_f0 != i)
            goto latch;
        {
            switch (f_f8)
            {
            case 1:
                if (g_1b2eb6c == 0)
                {
                    char *name = (char *)this + 0x238 + i * 0x374;
                    // STRING: EZ2AC_UNPACKED 0x00491aa0
                    sprintf(buf, "system\\disc\\%s.bmp", name);
                    // STRING: EZ2AC_UNPACKED 0x00491a94
                    ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDisc", buf, &f_104[0]);
                    // STRING: EZ2AC_UNPACKED 0x00491a7c
                    sprintf(buf, "system\\disc\\%s-hd.bmp", name);
                    // STRING: EZ2AC_UNPACKED 0x00491a6c
                    ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDiscHard", buf, &f_104[1]);
                    // STRING: EZ2AC_UNPACKED 0x00491a54
                    sprintf(buf, "system\\disc\\%s-shd.bmp", name);
                    // STRING: EZ2AC_UNPACKED 0x00491a40
                    ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDiscSHard", buf, &f_104[3]);
                    // STRING: EZ2AC_UNPACKED 0x00491a28
                    sprintf(buf, "system\\disc\\%s-ex.bmp", name);
                    // STRING: EZ2AC_UNPACKED 0x00491a18
                    ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDiscEx", buf, &f_104[4]);
                    f68 = i;
                    f_f8 = 2;
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x004919f4
                    sprintf(buf, "system\\CV2Mix\\Disc\\cv2disc_%02d.bmp", i + 1);
                    ((KGraphics *)g_sub1b2f054)->loadSprite("HotSpotDisc", buf, &f_104[0]);
                    f68 = i;
                    f_f8 = 2;
                }
                break;
            default:
                if (f_20c6d4 != f_f4)
                {
                    f_100 = 0.0f - f_100;
                    f_f4 = f_20c6d4;
                }
            }

            if (g_1b2eb6c == 0)
            {
                ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
                ((KGraphics *)g_sub1b2f054)->m404250(f_bc44c,
                    x - s * 0.5f - (float)g_37dcf30, y - s * 0.5f - (float)g_37dcf34,
                    (float)g_37dcf38 + s, (float)g_37dcf38 + s, f_fc,
                    bright << 24 | bright << 16 | bright << 8 | bright);
                ((KGraphics *)g_sub1b2f054)->m404250(f_bc44c,
                    x - s * 0.5f - (float)g_37dcf30, y - s * 0.5f - (float)g_37dcf34,
                    (float)g_37dcf38 + s, (float)g_37dcf38 + s, f_fc,
                    bright << 24 | bright << 16 | bright << 8 | bright);
            }
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            if (180.0f > f_fc)
                idx = 0;
            else
                idx = f_f4;
            ((KGraphics *)g_sub1b2f054)->m404250(f_104[idx],
                x - s * 0.5f - (float)g_37dcf30, y - s * 0.5f - (float)g_37dcf34,
                (float)g_37dcf38 + s, (float)g_37dcf38 + s, f_fc,
                bright << 24 | bright << 16 | bright << 8 | bright);
            if (g_1b2eb6c == 0)
            {
                ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
                if (f_bfdb4 != 7)
                    ((KGraphics *)g_sub1b2f054)->m404250(f_bc450,
                        x - s * 0.5f - (float)g_37dcf30, y - s * 0.5f - (float)g_37dcf34,
                        (float)g_37dcf38 + s, (float)g_37dcf38 + s, 0.0f,
                        bright << 24 | bright << 16 | bright << 8 | bright);
            }

            if (f_f4 == 1)
                f_100 = 30.0f - (f_fc - 180.0f) * 0.16666667f;
            else if (f_f4 == 3)
                f_100 = 90.0f - (f_fc - 180.0f) * 0.16666667f;
            else if (f_f4 == 4)
                f_100 = 150.0f - (f_fc - 180.0f) * 0.16666667f;
            else
                f_100 = f_fc * -0.16666667f;
            f_c0e40 = f_100;

            m_c09e8.setCell(2, 0, (int)f_104[f_f4]);

            if ((f_100 > 0.0f && 1080.0f > f_fc) ||
                (0.0f > f_100 && f_fc > -360.0f))
            {
                f_fc = f_fc + f_100;
                if (f_fc > 1080.0f)
                    f_fc = 1080.0f;
                else if (f_fc <= 0.0f)
                    f_fc = 0.0f;
            }
            if (f_fc == 180.0f)
                f_f8 = 2;
            return;
        }
latch:
        f_fc = 0.0f;
        f_f8 = 1;
        f_f0 = i;
        f_f4 = 0;
        f_100 = -30.0f;
    }

    if (g_1b2eb6c == 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bc44c,
            x - s * 0.5f, y - s * 0.5f, s, s,
            bright << 24 | bright << 16 | bright << 8 | bright);
    }
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    n = (i + f7c) % f7c;
    ((KGraphics *)g_sub1b2f054)->drawTex(
        *(KTexture **)((char *)this + 0x12c + n * 0x374),
        x - s * 0.5f, y - s * 0.5f, s, s,
        bright << 24 | bright << 16 | bright << 8 | bright);
    if (g_1b2eb6c == 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
        ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bc450,
            x - s * 0.5f, y - s * 0.5f, s + 3.0f, s + 3.0f,
            bright << 24 | bright << 16 | bright << 8 | bright);
    }
}

/* The per-entry quad drawer, viewed the way src/songselectquad.cpp declares
   it (the mangled name is what links). */
struct Obj4323C0
{
    void drawOne(int i, int unused, float x, float y, float off, float frac,
                 int bright);   /* 0x4323c0 */
};

/* SongSelectDirector::m438d30 @0x438d30 - the left rail: the strip texture
   f_c046c at (0,64) sized 0x490, the m_bcee4 refresh, then a wrapped walk
   handing every visible entry to Obj4323C0::drawOne at x=76.5 with a banded
   y and brightness. TWO HALVES on f7c < 30: the fine half walks
   i in [-7, f7c*60) with the odd wrap (threshold 1000*f7c, adjust 100*f7c -
   transcribed as read), the coarse half walks [-7, f7c) with the classic
   50/100 wrap. Shared bands: visible only for 85 <= d <= 1500 and a
   non-negative (i + 8*f7c) %% f7c; brightness 255 in the mid bands, (int)d
   up to 250 low, 1455 - (int)d past 1200 - AND UNINITIALIZED in the
   fractional window 699 < d < 701, d != 700, faithful to the original
   (its frame aliases the saved ecx there); y = d*k by band with a cosine
   ripple (cos[|(d-500)*5| & 0x3ff]) in the second band, the halves
   differing only in the band constants (95/., 106/97/216). */
// FUNCTION: EZ2AC_UNPACKED 0x00438d30
void SongSelectDirector::m438d30()
{
    int   i;
    int   pos;
    int   bright;
    int   idx;
    float d;
    float c;
    float yy;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_c046c, 0.0f, 64.0f, 0.0f, 490.0f, -1);
    m_bcee4.m403940();

    if (f7c < 0x1e)
    {
        for (i = -7, pos = 0; i + 7 < f7c * 60; i++)
        {
            d = (float)pos - f_c4;
            if (d > (float)(f7c * 1000))
                d -= (float)(f7c * 100);
            if ((float)(-(f7c * 1000)) > d)
                d = (float)(f7c * 100) + d;
            if (d >= 85.0f && 1500.0f >= d && (i + f7c * 8) % f7c >= 0)
            {
                if (d == 700.0f)
                {
                    bright = 0xff;
                }
                else if (d >= 701.0f)
                {
                    if ((int)d >= 0x4b0)
                        bright = 0x5af - (int)d;
                    else
                        bright = 0xff;
                }
                else if (699.0f >= d)
                {
                    bright = (int)d;
                    if (bright > 0xfa)
                        bright = 0xff;
                }
                idx = (int)((d - 500.0f) * 5.0f);
                if (idx < 0)
                    idx = -idx;
                idx = idx & 0x3ff;
                c = g_cosTable[idx];
                if (599.0f >= d)
                    ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f - 104.0f, 0.0f, 0.0f, bright);
                else if (699.0f >= d)
                    ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f + c * 10.0f - 95.0f, 0.0f, 0.0f, bright);
                else if (d >= 799.0f)
                    ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f - 31.0f, 0.0f, 0.0f, bright);
                else
                    ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.48076925f + 34.0f - c * 22.0f - 214.0f, 0.0f, 0.0f, bright);
            }
            pos += 100;
        }
        return;
    }

    for (i = -7, pos = 0; i + 7 < f7c; i++)
    {
        d = (float)pos - f_c4;
        if (d > (float)(f7c * 50))
            d -= (float)(f7c * 100);
        if ((float)(-f7c * 50) > d)
            d = (float)(f7c * 100) + d;
        if (d >= 85.0f && 1500.0f >= d && (i + f7c * 8) % f7c >= 0)
        {
            if (d == 700.0f)
            {
                bright = 0xff;
            }
            else if (d >= 701.0f)
            {
                if ((int)d >= 0x4b0)
                    bright = 0x5af - (int)d;
                else
                    bright = 0xff;
            }
            else if (699.0f >= d)
            {
                bright = (int)d;
                if (bright > 0xfa)
                    bright = 0xff;
            }
            idx = (int)((d - 500.0f) * 5.0f);
            if (idx < 0)
                idx = -idx;
            idx = idx & 0x3ff;
            c = g_cosTable[idx];
            if (599.0f >= d)
                ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f - 106.0f, 0.0f, 0.0f, bright);
            else if (699.0f >= d)
                ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f + c * 10.0f - 97.0f, 0.0f, 0.0f, bright);
            else if (d >= 799.0f)
                ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.31695721f - 31.0f, 0.0f, 0.0f, bright);
            else
                ((Obj4323C0 *)this)->drawOne(i, f70, 76.5f, d * 0.48076925f + 34.0f - c * 22.0f - 216.0f, 0.0f, 0.0f, bright);
        }
        pos += 100;
    }
}
