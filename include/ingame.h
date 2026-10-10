#ifndef INGAME_H
#define INGAME_H

/* The in-game object: the ~2 MB thing the thirteen per-mode input handlers
   (include/modeinput.h) are methods of, and that the tick @0x446440 dispatches
   through. Only what is reconstructed so far is modeled; everything else is
   padding, so the offsets that ARE named stay honest. */
struct InGame
{
    char _pad0[0x20bd94];
    /* 0x20bd94: the modulus pickDistinct draws each of its eight numbers with,
       one per kind of draw. */
    int  m_randRange[16];

    void pickDistinct(int *out, int kind);   /* 0x433cf0 */
};

#endif /* INGAME_H */
