/* The song-table unpacker @0x431660: look a mode's entry up in g_songTable and
   spread it into the caller's display block.

   The record's shape falls out of the reads: a name at +0x10, one byte at
   +0x30, and then FOUR nine-byte groups at +0x32 with stride 9 (0x32, 0x3b,
   0x44, 0x4d), each a level byte followed by two unaligned floats - the four
   difficulty steps. 0x4d + 9 = 0x56, which is exactly the record size the
   three loadGds functions already established for g_songTable. */

#include "ez2acapp.h"
#include "ezscreen.h"   /* SongTable, g_songTable */
#include <string.h>

/* Packed on purpose: the two floats of each group sit at odd offsets, which is
   what makes the original's loads unaligned `movss`es. */
#pragma pack(push, 1)
struct SongLevel
{
    unsigned char level;   /* +0x00 */
    float         a;       /* +0x01 */
    float         b;       /* +0x05 */
};

/* Only the fields this function touches; the rest of the 0x56 bytes stay
   opaque. */
struct SongRecord
{
    char          _pad0[0x10];
    char          name[0x20];   /* 0x10 */
    /* The song's ORIGIN VERSION, 0..0x12: the cell index of both version
       badges on the song select (ani_Version from Version\Version_%02d.bmp,
       Version_View from V_View\version\Version_%02d.bmp), and, as
       g_1b2eba0, what the background loader switches on for the
       BG\commonbg* fallbacks. Was named `kind`. */
    unsigned char version;      /* 0x30 */
    char          _pad31;
    SongLevel     levels[4];    /* 0x32 .. 0x56, which IS the record size */
};
#pragma pack(pop)

/* Where the record is unpacked to: a name at +8 and twelve floats from +0x214,
   three per difficulty step, plus the record's kind at +0x370. */
struct SongInfoOut
{
    char  _pad0[8];
    char  name[0x20c];          /* 0x008 */
    float levels[4][3];         /* 0x214 .. 0x244 */
    char  _pad244[0x370 - 0x244];
    int   version;              /* 0x370 - the record's +0x30, see src/songinfo.cpp */
};

/* The owner: only the flag at +0x30 is evidenced, and it gates everything past
   the first difficulty step. */
struct Obj431660
{
    char _pad0[0x30];
    int  f30;                   /* 0x30 */

    void loadSongInfo(char *name, SongInfoOut *out);   /* 0x431660 */
};

/* Republished so the rest of the game can see which version the song is from. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef5c
int g_songKind;

// FUNCTION: EZ2AC_UNPACKED 0x00431660
void Obj431660::loadSongInfo(char *name, SongInfoOut *out)
{
    SongRecord rec;

    if (g_songTable.find(name, &rec) == 0)
    {
        out->version = 0;
        out->levels[0][0] = 0.0f;
        out->levels[0][1] = 0.0f;
        out->levels[0][2] = 0.0f;
        out->levels[1][0] = 0.0f;
        out->levels[1][1] = 0.0f;
        out->levels[1][2] = 0.0f;
        out->levels[2][0] = 0.0f;
        out->levels[2][1] = 0.0f;
        out->levels[2][2] = 0.0f;
        out->levels[3][0] = 0.0f;
        out->levels[3][1] = 0.0f;
        out->levels[3][2] = 0.0f;
        return;
    }

    strcpy(out->name, rec.name);

    out->version = rec.version;
    g_songKind = out->version;

    out->levels[0][0] = rec.levels[0].level;
    out->levels[0][1] = rec.levels[0].a;
    out->levels[0][2] = rec.levels[0].b;

    if (f30 == 0)
    {
        out->levels[1][0] = rec.levels[1].level;
        out->levels[1][1] = rec.levels[1].a;
        out->levels[1][2] = rec.levels[1].b;
        out->levels[2][0] = rec.levels[2].level;
        out->levels[2][1] = rec.levels[2].a;
        out->levels[2][2] = rec.levels[2].b;
        out->levels[3][0] = rec.levels[3].level;
        out->levels[3][1] = rec.levels[3].a;
        out->levels[3][2] = rec.levels[3].b;
    }
}

/* @0x00469f40: clear every entry of the song table - the four bytes each
   record leads with, and the byte-and-two-floats of all four packed tail
   entries. The layout, and why it needs pack(1), is on SongTable in
   include/ezscreen.h.

   Found as an anonymous 0x56-stride array before the header was grepped for
   its shape; SongTable was already there, described in exactly those terms.
   The two models are merged here. */
// FUNCTION: EZ2AC_UNPACKED 0x00469f40
void SongTable::clearAll()
{
    int i = 0;

    if (count > 0)
    {
        SongEntry *e = &entries[0];

        do
        {
            e->b00 = 0;
            e->b10 = 0;
            e->b30 = 0;
            e->b31 = 0;
            e->r[0].b = 0;
            e->r[0].f0 = 0.0f;
            e->r[0].f1 = 0.0f;
            e->r[1].b = 0;
            e->r[1].f0 = 0.0f;
            e->r[1].f1 = 0.0f;
            e->r[2].b = 0;
            e->r[2].f0 = 0.0f;
            e->r[2].f1 = 0.0f;
            e->r[3].b = 0;
            e->r[3].f0 = 0.0f;
            e->r[3].f1 = 0.0f;
            i++;
            e++;
        }
        while (i < count);
    }
}

/* SongTable::find @0x469ed0 - look one song up by name and hand back a COPY.

   A linear case-insensitive scan of the whole table; on a hit the entry's
   whole 0x56 bytes are copied out and the answer is 1, and a miss answers 0
   without touching the destination. Note the copy is addressed from the
   INDEX, not from the walking pointer the scan advanced - the original
   rebuilds `this + 86*i` with a lea chain rather than reusing it - so the
   source indexes entries[i] there.

   Same three-part loop shape as Obj431040::findByName @0x431040: the counter
   is zeroed BEFORE the guard so the guard compares the count against that
   register, the walking pointer is anchored explicitly and set up INSIDE the
   guard, and the body is a do-while. */
// FUNCTION: EZ2AC_UNPACKED 0x00469ed0
int SongTable::find(char *name, void *out)
{
    int i = 0;

    if (count > 0)
    {
        const SongEntry *e = &entries[0];

        do
        {
            if (_stricmp(name, (const char *)e) == 0)
            {
                memcpy(out, &entries[i], sizeof(SongEntry));
                return 1;
            }

            i++;
            e++;
        }
        while (i < count);
    }

    return 0;
}
