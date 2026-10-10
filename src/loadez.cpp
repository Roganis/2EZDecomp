/* EzChart::loadEZ @0x411460 - THE CHART LOADER, integrated from
   wip/loadez-411460.cpp. It is called by EzChart::load @0x411720 and is the
   last big loader in the .ez path.

   WHAT IT READS: a decrypted .es chart - fopen "rb", fread a 0x96-byte header,
   check the magic, copy the two 0x40-byte name buffers into this+8 and
   this+0x48, store the four header scalars, then run the per-lane
   record-conversion loops that fill g_tracks through NoteConvertAll @0x410fd0.

   IT SCORES 94.71%, NOT THE "~14 DIFFS" ITS NOTE CLAIMED - 189 instructions
   against 189, with six differing. The park's estimate was pessimistic, which
   is the fourth stale park number this run and the second in the encouraging
   direction. The note is still right about WHERE the remainder is: the
   call and string skeleton is right, but the Group4C header field STORE ORDER
   and the register allocation around it are not pinned. Roughly ninety
   field-ordering permutations were swept without landing it.

   Integrated at that rather than left parked, because it completes the .ez
   load path for a port and a decoded loader in wip/ is invisible to a build.
   The remaining work is the header struct's layout (Group4C w0/name/f44/f48
   against the matched ezchart.h) and the store scheduling. */

#include <stdio.h>
#include <string.h>
#include "note.h"

/* ---- standalone mirror of include/ezchart.h layout (zoo only; the real
   integration uses ezchart.h with `int loadEZ(char *name);` added) ---- */

struct Group4C
{
    unsigned short w0;
    char name[0x42];
    int  f44;
    int  f48;
};

struct Record28
{
    int            d0;
    unsigned char  byte4;
    unsigned char  byte5;
    unsigned short _w6;
    int            _pad1[3];
    unsigned short word14;
    unsigned short _w16;
    int            _pad2[4];
};

struct EzChart
{
    int      f4;
    char     b8;
    char     _pad1[0x48 - 0x09];
    char     b48;
    char     _pad2[0x88 - 0x49];
    int      i88;
    float    f8c;
    float    f90;
    int      i94;
    int      i98;
    Group4C  groups[96];
    Record28 records[0x5000];
    int      sampleTable[0x800];

    virtual void *getClassInfo();

    void reset();
    void mergeTracks(int first, int count);
    int  loadEZ(char *name);
};

void Log(const char *fmt, ...);
void extractBaseName(char *name, char *out);

struct Track
{
    Record28 records[0x800];
    int      cur;
    int      count;
};

extern Track g_tracks[96];
extern EzNoteRaw g_noteBuf[0x5000];

/* ---- on-disk headers ---- */

#pragma pack(push, 1)
struct EzHeader
{
    int            magic;      /* 0x00 */
    unsigned char  verHi;      /* 0x04 */
    unsigned char  verLo;      /* 0x05 */
    char           name1[0x40];/* 0x06 */
    char           name2[0x40];/* 0x46 */
    unsigned short w86;        /* 0x86 */
    float          f88;        /* 0x88 */
    unsigned short lanes;      /* 0x8c */
    int            d8e;        /* 0x8e */
    float          f92;        /* 0x92 */
};                             /* 0x96 */

struct EzLaneHeader
{
    int            d0;         /* 0x00 */
    unsigned short w4;         /* 0x04 */
    char           name[0x40]; /* 0x06 */
    int            d46;        /* 0x46 */
    unsigned int   size;       /* 0x4a */
};                             /* 0x4e */
#pragma pack(pop)

// FUNCTION: EZ2AC_UNPACKED 0x00411460
int EzChart::loadEZ(char *fname)
{
    EzLaneHeader lhdr;
    EzHeader hdr;
    char buf[256];

    FILE *f = fopen(fname, "rb");
    if (f == 0)
    {
        Log("File not found %s", fname);
    }
    else
    {
        memset(&hdr, 0, sizeof(hdr));
        memset(&lhdr, 0, sizeof(lhdr));
        fread(&hdr, 0x96, 1, f);
        if (memcmp(&hdr, "EZFF", 4) != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x004889a8
            Log("Illegal EZ file %s", fname);
        }
        else
        {
            char *str1 = &b8;
            memcpy(str1, hdr.name1, 0x40);
            memcpy(&b48, hdr.name2, 0x40);
            i98 = hdr.lanes > 0x60 ? 0x60 : hdr.lanes;
            f90 = hdr.f92;
            f8c = hdr.f88;
            i88 = hdr.w86;
            i94 = 0;
            unsigned short ver = (unsigned short)((hdr.verHi << 8) | hdr.verLo);
            if (ver == 7)
                goto load;
            if (ver != 8)
            {
                // STRING: EZ2AC_UNPACKED 0x00488990
                Log("%s Version incorrect", fname);
            }
            else
            {
                extractBaseName(fname, buf);
                if (_stricmp(str1, buf) != 0)
                {
                    // STRING: EZ2AC_UNPACKED 0x00488974
                    Log("Title Miss Match!! %s - %s", str1, buf);
                }
                else
                    goto load;
            }
        }
        fclose(f);
    }
    reset();
    // STRING: EZ2AC_UNPACKED 0x00488948
    Log("EZModule(LoadEZ) : Error on loading %s (%s)", fname);
    return 0;

load:
    int i;
    Group4C *g;
    for (i = 0; i < i98; i++)
    {
        fread(&lhdr, 0x4e, 1, f);
        g = &groups[i];
        g->w0 = lhdr.w4;
        g->f44 = lhdr.d46;
        memcpy(g->name, lhdr.name, 0x40);
        g->f48 = *(volatile unsigned int *)&lhdr.size / 13;
        fread(g_noteBuf, lhdr.size, 1, f);
        g_tracks[i].cur = 0;
        g_tracks[i].count = g->f48;
        NoteConvertAll(g_noteBuf, (EzNote *)g_tracks[i].records,
                       (unsigned char)i, g->f48);
    }
    fclose(f);
    mergeTracks(0, i98);
    return 1;
}
