#ifndef OBJ402860_H
#define OBJ402860_H

/* A 0x21c-byte record with a head pointer at +0, a 0x200-byte payload buffer at
   +0xc and a float pair at +0x214/0x218. Constructed @0x402860; clear()
   @0x402890 is the same body without the return-this. EZ2AC embeds one at
   +0x6d93c of itself (include/ez2acapp.h), which is what makes its destructor
   load-bearing: empty, so the original's linker ICF-folded it onto the shared
   bare `ret` @0x402880, and ~EZ2AC calls it there. Defined in
   src/subobj402860.cpp — a TU of its own so cl cannot inline it into ~EZ2AC and
   delete that call. The other methods live in src/accessors.cpp. */
/* One row of the head's table: a count and the cells, 0x4f0 bytes apart -
   the stride setCell @0x402910 builds with `r*79*16`. */
/* One keyframe of a row's tween track, 0x7c bytes: when it starts, whether
   it is a jump (flags == 0: the blit is copied) or a delta (added), and a
   whole VideoBlit whose +0x4c/+0x50 carry the frame-cycling mode and speed
   (see the refresh @0x403940). */
struct TweenKey
{
    int   time;       /* 0x00 */
    int   flags;      /* 0x04 */
    char  blit[0x74]; /* 0x08 - a VideoBlit (include/ez2acapp.h) */
};

struct Obj402860Row
{
    int count;        /* 0x00 - how many cells of this row are in use (the frame count) */
    int f4;
    int data[312];    /* 0x08 .. 0x4e8 - the cells (KTexture pointers) */
    int keyCount;     /* 0x4e8 - tween keys in this row, 0 = no track */
    TweenKey *keys;   /* 0x4ec */
};

struct Obj402860Head
{
    int  f0;          /* 0x00 */
    int  f4;          /* 0x04 */
    int  rowCount;    /* 0x08 - how many rows the table has */
    int  fc;          /* 0x0c */
    Obj402860Row rows[1];  /* 0x10 */
};

struct Obj402860
{
    Obj402860Head *head;   /* 0x000 */
    int   f4;              /* 0x004 */
    int   f8;              /* 0x008 */
    int   cursor[0x80];    /* 0x00c - per-row index of the tween key in force, -1 = past the last */
    int   f20c;            /* 0x20c */
    int   f210;            /* 0x210 */
    float f214;            /* 0x214 */
    float f218;            /* 0x218 */

    Obj402860();           /* 0x402860 */
    ~Obj402860();          /* 0x402880 (ICF-folded onto the shared ret) */
    void clear();
    int  isDone();
    void reset();
    int  setCell(int r, int c, int v);  /* 0x402910 */
    /* Point the record at a table and reset the cursor: stores the head and the
       flag, clears the 0x200-byte buffer and copies head->f8 into head->fc. */
    void attach(Obj402860Head *h, int flag);  /* 0x4028b0 */
    /* The refresh every writer of the table tail-calls once it has finished
       with it. It RETURNS a value - Panel's @0x42d290 branches on it and then
       reuses the zero for two of the draw's arguments. */
    int  m403940();        /* 0x403940 - the tween tick (src/accessors.cpp) */
    /* One step of the cell-table cursor (returns 0 when there is no head). */
    int  step();           /* 0x4029b0 (STUB) */
    /* Advance the cursor to row n, starting over if it is already past it. */
    void seek(int n);      /* 0x402d60 */
};

#endif /* OBJ402860_H */
