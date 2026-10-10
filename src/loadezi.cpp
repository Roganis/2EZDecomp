#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

/* EziLoader::loadEZI @0x00411090: parse a .ezi text file mapping note numbers
   to sample names; creates a sample per entry via the SampleOwner. */

// FUNCTION: EZ2AC_UNPACKED 0x00411090
int EziLoader::loadEZI(char *name)
{
    char token[8];
    int  mode;
    char sampName[260];

    // STRING: EZ2AC_UNPACKED 0x00488944
    FILE *f = fopen(name, "rt");
    if (f == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00488930
        Log("File not found %s", name);
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x00488928
        while (fscanf(f, "%s %d", token, &mode) != -1)
        {
            if (token[0] != ';')
            {
                int note = atoi(token);
                if (note >= 0 && note < 0x800)
                {
                    int two = 0;
                    switch (mode)
                    {
                    case 1:
                        goto have_mode;
                    case 2:
                        two = 1;
                        break;
                    }
                have_mode:
                    // STRING: EZ2AC_UNPACKED 0x00488924
                    fscanf(f, "%s", sampName);
                    int samp = field_0x4->createSample(sampName, 0, 0);
                    if (samp != 0)
                    {
                        sampleTable[note] = (void *)samp;
                        if (two)
                            fscanf(f, "%s", sampName);
                    }
                    else
                    {
                        // STRING: EZ2AC_UNPACKED 0x004888f8
                        Log("Error on LoadEZI(%s) - CreateSample(%s)", name, sampName);
                        goto load_err;
                    }
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x004888c8
                    Log("Error on LoadEZI(%s) - note number exception %d", name, note);
                    goto load_err;
                }
                // STRING: EZ2AC_UNPACKED 0x00488920
                fscanf(f, "\n");
            }
            else
            {
                fscanf(f, "\n");
            }
        }
        fclose(f);
        return 1;
    }
load_err:
    // STRING: EZ2AC_UNPACKED 0x004888b4
    Log("Loading error %s", name);
    if (f != 0)
        fclose(f);
    unloadSamples();
    return 0;
}
