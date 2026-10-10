#ifndef GFXTEXT_H
#define GFXTEXT_H

/* The graphics singleton's two sprite-font number draws, as their callers see
   it. Both are __thiscall on the object at 0x1b2f054; the float one takes an
   extra scale, the int one does not.

   THE SIXTH ARGUMENT IS THE VALUE BEING PRINTED, and the seventh is the
   printf format for it - which is what the F/I in the two names means. It was
   modelled as a font POINTER until CatchPanel's readout @0x471170 turned up
   indexing a quarter-step float ladder with it (0.25, 0.5, 0.75, 1.0 ... at
   both 0x48e6c8 and 0x49deb8 - a speed multiplier, printed "%02.1f"), and
   Panel's @0x42bd80 confirms the integer side by comparing its value against
   10, 100, 1000 and 10000 to pick between four format strings. */
struct GfxText
{
    void drawNumF(float x, float y, float w, float h, float step, float scale,
                  const char *fmt, int color, void *out);   /* 0x40b140 */
    void drawNumI(float x, float y, float w, float h, int v,
                  const char *fmt, int color, void *out);   /* 0x40b100 */
    /* The third of the family: the same nine arguments as drawNumF but an
       INTEGER value. Panel's @0x42bd80 settles it - it compares the same dword
       against 10, 100, 1000 and 10000 to choose the format string. */
    void m40a970(float x, float y, float w, float h, float adv, int value,
                 const char *fmt, int color, void *tab);
                                                /* 0x40a970 (src/gfxtextbase.cpp) */
    /* The base of the family - TEN arguments, one more piece of geometry than
       drawNumF, which forwards to it. */
    void m40aaa0(float x, float y, float w, float h, float adv, float step,
                 float value, const char *fmt, int color, void *tab);
                                                /* 0x40aaa0 (src/gfxtextbase.cpp) */
};

/* The quarter-step scale ladder, one copy per user (src/effectordraw.cpp and
   src/catchpanel-readout below). */

#endif /* GFXTEXT_H */
