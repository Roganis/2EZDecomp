/* The three tier probes @0x431810 / 0x431900 / 0x4319f0 - does `name`'s
   -hd / -shd / -ex variant appear in the hidden-song table with its
   group's unlock counter run out (<= 0)? Returns the TIER on a hit -
   1 for -hd, 2 for -shd, 3 for -ex - and 0 otherwise. One source, three
   suffixes; each also
   records which tier it asked about in f30 (1 for -hd, 0 otherwise - the
   -ex probe stores it before building the name). The ctor runs them per
   song to decide which difficulty tiers exist. */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "songselect.h"

extern "C" int __cdecl _stricmp(const char *, const char *);

extern int g_hidCounts[22];     /* 0x1b2e848 (src/globals.cpp) */

// FUNCTION: EZ2AC_UNPACKED 0x00431810
int SongSelectDirector::m431810(char *name)
{
    char buf[256];
    int  i;

    memset(buf, 0, sizeof(buf));
    strcat(buf, name);
    // STRING: EZ2AC_UNPACKED 0x004919cc
    strcat(buf, "-hd");
    f30 = 1;
    for (i = 0; i < f_20e890; i++)
    {
        if (_stricmp(m_hidden[i].name, buf) == 0 &&
            g_hidCounts[m_hidden[i].group] <= 0)
            return 1;
    }
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x00431900
int SongSelectDirector::m431900(char *name)
{
    char buf[256];
    int  i;

    memset(buf, 0, sizeof(buf));
    strcat(buf, name);
    // STRING: EZ2AC_UNPACKED 0x004919d0
    strcat(buf, "-shd");
    f30 = 0;
    for (i = 0; i < f_20e890; i++)
    {
        if (_stricmp(m_hidden[i].name, buf) == 0 &&
            g_hidCounts[m_hidden[i].group] <= 0)
            return 2;
    }
    return 0;
}

// FUNCTION: EZ2AC_UNPACKED 0x004319f0
int SongSelectDirector::m4319f0(char *name)
{
    char buf[256];
    int  i;

    memset(buf, 0, sizeof(buf));
    f30 = 0;
    strcat(buf, name);
    // STRING: EZ2AC_UNPACKED 0x004919d8
    strcat(buf, "-ex");
    for (i = 0; i < f_20e890; i++)
    {
        if (_stricmp(m_hidden[i].name, buf) == 0 &&
            g_hidCounts[m_hidden[i].group] <= 0)
            return 3;
    }
    return 0;
}

extern char g_modeName[256];    /* 0x1b2e680 (src/modename.cpp) */
extern int  g_1b2eb70;          /* src/resultctor.cpp */
extern int  g_modeIndex;        /* 0x1b2e548 (src/modename.cpp) */

/* The per-course rank-file reader @0x430cf0 - the ctor runs it five times
   per song (courses 0/1/3/4 map to the .bin / -hd.bin / -shd.bin / -ex.bin
   files; course 2 probes an unbuilt path and always defaults). The file
   holds five 8-byte names then five scores; a missing file writes the
   EZ2AC_FN / 0 default into the caller's slots instead. The e_rank_
   variant is used when g_1b2eb70 is set in mode 10. Returns whether the
   file was read. */

// FUNCTION: EZ2AC_UNPACKED 0x00430cf0
int SongSelectDirector::m430cf0(char *name, void *names_, void *scores_,
                                int course)
{
    char  path[260];
    FILE *f;
    int   i;

    if (g_1b2eb70 == 0)
    {
        if (course == 0)
            sprintf(path,
                // STRING: EZ2AC_UNPACKED 0x004918a8
                "sound\\rank_%s_%s%s", g_modeName, name,
                // STRING: EZ2AC_UNPACKED 0x004918f0
                ".bin");
        else if (course == 1)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                // STRING: EZ2AC_UNPACKED 0x004918e8
                "-hd.bin");
        else if (course == 3)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                // STRING: EZ2AC_UNPACKED 0x004918dc
                "-shd.bin");
        else if (course == 4)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                // STRING: EZ2AC_UNPACKED 0x004918d4
                "-ex.bin");
    }
    else if (g_modeIndex == 0xa)
    {
        if (course == 0)
            sprintf(path,
                // STRING: EZ2AC_UNPACKED 0x004918bc
                "sound\\e_rank_%s_%s%s", g_modeName, name, ".bin");
        else if (course == 1)
            sprintf(path, "sound\\e_rank_%s_%s%s", g_modeName, name,
                    "-hd.bin");
        else if (course == 3)
            sprintf(path, "sound\\e_rank_%s_%s%s", g_modeName, name,
                    "-shd.bin");
        else if (course == 4)
            sprintf(path, "sound\\e_rank_%s_%s%s", g_modeName, name,
                    "-ex.bin");
    }
    else
    {
        if (course == 0)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name, ".bin");
        else if (course == 1)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                    "-hd.bin");
        else if (course == 3)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                    "-shd.bin");
        else if (course == 4)
            sprintf(path, "sound\\rank_%s_%s%s", g_modeName, name,
                    "-ex.bin");
    }

    if (_access(path, 0) == -1)
    {
        for (i = 0; i < 5; i++)
        {
            int j;
            for (j = 0; j < 8; j++)
                ((char *)names_)[i * 8 + j] = "EZ2AC_FN"[j];
            ((int *)scores_)[i] = 0;
        }
        return 0;
    }
    f = fopen(path, "rb");
    for (i = 0; i < 5; i++)
        fread((char *)names_ + i * 8, 8, 1, f);
    for (i = 0; i < 5; i++)
        fread((int *)scores_ + i, 4, 1, f);
    fclose(f);
    return 1;
}

extern char g_textSlots[32][128];   /* 0x1b2ec18 (src/modename.cpp) */
extern int  g_elemCount;            /* 0x1b2e718 (src/loops.cpp) */
extern void setTextSlot(int slot, const char *text);   /* src/modename.cpp */

/* Publish the selected song's tier-qualified name into the mode's text
   slot - the difficulty steppers run it every tick. Columns 4/3 build
   %s-ex / %s-shd and copy the slot directly; column 1 builds %s-hd and
   the rest hand the plain name through setTextSlot. */

// FUNCTION: EZ2AC_UNPACKED 0x00434de0
void SongSelectDirector::m434de0()
{
    char buf[260];

    if (f_20c6d4 == 4)
    {
        // STRING: EZ2AC_UNPACKED 0x00491cc0
        sprintf(buf, "%s-ex", rows[f84].name);
        strcpy(g_textSlots[g_elemCount], buf);
    }
    else if (f_20c6d4 == 3)
    {
        // STRING: EZ2AC_UNPACKED 0x00491cb8
        sprintf(buf, "%s-shd", rows[f84].name);
        strcpy(g_textSlots[g_elemCount], buf);
    }
    else if (f_20c6d4 == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x00491cb0
        sprintf(buf, "%s-hd", rows[f84].name);
        setTextSlot(g_elemCount, buf);
    }
    else
    {
        setTextSlot(g_elemCount, rows[f84].name);
    }
}
