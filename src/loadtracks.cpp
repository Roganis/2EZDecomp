/* EzChart::loadTracks @0x411920 - vtable slot 5, the sibling of loadEZ. Loads
   a .ez track file: clamp lane+count to 0x60, grow the group count, fopen "rb"
   and fread a 0x96-byte header, check the 4-byte magic, fclose on a bad one
   and Log on an open failure, then fill g_tracks' lanes from the file records
   through NoteConvert.

   Structure, all calls, strings, globals and the frame match. What is left is
   two or three cl scheduling and addressing choices - the record-store site
   emits `mov [ebx-0x4],eax` where the original anchors the walking pointer one
   element along - which is the induction-anchoring family in CLAUDE.md and was
   not steerable from the draft. */

#include <stdio.h>
#include <string.h>
#include "ezchart.h"
#include "note.h"
#include "loader.h"

struct Track
{
    Record28 records[0x800];  /* 0x00000 */
    int      cur;             /* 0x14000 play cursor */
    int      count;           /* 0x14004 record count */
};

/* Both defined elsewhere - g_tracks in src/seq.cpp, g_noteBuf in
   src/globals.cpp - and the park defined its own copies. */
extern Track g_tracks[96];

extern EzNoteRaw g_noteBuf[0x5000];

// FUNCTION: EZ2AC_UNPACKED 0x00411920
int EzChart::loadTracks(int lane, char *name, int start, int count)
{
    char rec[0x4e];
    char hdr[0x96];
    int  i;

    if (lane + count > 0x60)
        count = 0x60 - lane;
    if (lane + count > i98)
        i98 = lane + count;

    FILE *f = fopen(name, "rb");
    if (f == 0)
        goto fail;

    memset(hdr, 0, 0x96);
    memset(rec, 0, 0x4e);
    fread(hdr, 0x96, 1, f);
    // STRING: EZ2AC_UNPACKED 0x004889bc
    if (memcmp(hdr, "EZFF", 4) != 0)
    {
        fclose(f);
fail:
        // STRING: EZ2AC_UNPACKED 0x004889dc
        Log("EZModule(LoadTracks) : Error on loading %s (%s)", name);
        return 0;
    }

    if (start + count > (int)*(unsigned short *)&hdr[0x8c])
        count = *(unsigned short *)&hdr[0x8c] - start;

    if (start > 0)
    {
        i = start;
        do
        {
            fread(rec, 0x4e, 1, f);
            fseek(f, *(unsigned int *)&rec[0x4a], SEEK_CUR);
            i--;
        } while (i != 0);
    }

    i = lane;
    int end = lane + count;
    if (i < end)
    {
        int *tp = &g_tracks[lane].count;
        do
        {
            fread(rec, 0x4e, 1, f);
            Group4C *g = &groups[i];
            *(unsigned short *)g->_pad0 = *(unsigned short *)&rec[4];
            g->f44 = *(int *)&rec[0x46];
            g->f48 = *(unsigned int *)&rec[0x4a] / 13;
            memcpy(g->name, &rec[6], 0x40);
            fread(g_noteBuf, *(unsigned int *)&rec[0x4a], 1, f);
            tp[-1] = 0;
            *tp = g->f48;

            int n = g->f48;
            EzNote *dst = (EzNote *)((char *)tp - 0x14004);
            for (int j = 0; j < n; j++)
                NoteConvert(&g_noteBuf[j], &dst[j], (unsigned char)i);

            i++;
            tp += 0x5002;
        } while (i < end);
    }

    fclose(f);
    mergeTracks(lane, count);
    return 1;
}
