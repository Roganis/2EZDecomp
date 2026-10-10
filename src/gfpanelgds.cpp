/* GFPanel::LoadPVI @0x4654b0 - the GF note field's .gds skin reader, and the
   second of the three (CatchPanel's is src/catchpanelgds.cpp @0x472650, and
   Panel's @0x42d700 is still open). The three name themselves in their own
   error logs, which is where LoadPVI comes from.

   Structurally it is CatchPanel's twin down to the cross-jumped return-0 tail:
   a KParser over a 16KB buffer, an early return if the file will not open, a
   `TrackN` arm that splits the digits off the section name, twenty-two
   _stricmp arms, and an unknown section that logs and ABORTS the load.

   Two things it does that CatchPanel's does not: it has three sections of its
   own (RubyKeyPanel, PuzzleNote, SpecialNote) plus MeasureLine, the two
   audience-rating sections and SongSelectPane; and it pops the directory the
   load pushed before returning success. */

#include "gamepanels.h"
#include "kparser.h"
#include <string.h>
#include <stdlib.h>

extern int readGdsInt(int *out, KParser *p);            /* 0x456270 */
extern int readGdsSection(char *out, KParser *p);       /* 0x4561b0 */
extern void popDirectory();                             /* 0x412e60 */
extern void Log(const char *fmt, ...);                  /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x004654b0
int GFPanel::LoadPVI(char *name)
{
    char    sec[0x100];
    char    digits[0x80];
    KParser parser;
    char    buf[0x4000];
    int     n;

    parser.setBuffer(buf, 0x4000);

    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f1af0 = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        if (_strnicmp("Track", sec, 5) == 0)
        {
            strcpy(digits, sec + 5);
            n = atoi(digits);
            loadGdsItem(&parser, n - 1);
        }
        else if (_stricmp("GroupTexture", sec) == 0)
        {
            m460ac0(&parser);
        }
        else if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&ff8, &parser);
        }
        else if (_stricmp("Judgment", sec) == 0)
        {
            m460fa0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490684
        else if (_stricmp("MeasureLine", sec) == 0)
        {
            m460f00(&parser);
        }
        else if (_stricmp("CoolBomb", sec) == 0)
        {
            m461060(&parser);
        }
        else if (_stricmp("LongNoteBomb", sec) == 0)
        {
            m461670(&parser);
        }
        else if (_stricmp("CoolCombo", sec) == 0)
        {
            m463ff0(&parser);
        }
        else if (_stricmp("KeyPanel", sec) == 0)
        {
            m461c00(&parser);
        }
        else if (_stricmp("TargetBar", sec) == 0)
        {
            m461c60(&parser);
        }
        else if (_stricmp("MaxCoolCombo", sec) == 0)
        {
            m461ee0(&parser);
        }
        else if (_stricmp("Score", sec) == 0)
        {
            m461f70(&parser);
        }
        else if (_stricmp("ComboGauge", sec) == 0)
        {
            m462100(&parser);
        }
        else if (_stricmp("GrooveGauge", sec) == 0)
        {
            m462190(&parser);
        }
        else if (_stricmp("Effector", sec) == 0)
        {
            m4622c0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905f8
        else if (_stricmp("AudienceRating", sec) == 0)
        {
            m462000(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905e4
        else if (_stricmp("GoalAudienceRating", sec) == 0)
        {
            m462080(&parser);
        }
        else if (_stricmp("GrooveLight", sec) == 0)
        {
            m462220(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905c8
        else if (_stricmp("SongSelectPane", sec) == 0)
        {
            m4623d0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905ac
        else if (_stricmp("RubyKeyPanel", sec) == 0)
        {
            m4624f0(&parser);
        }
        else if (_stricmp("GoodBomb", sec) == 0)
        {
            m461360(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490594
        else if (_stricmp("PuzzleNote", sec) == 0)
        {
            m462580(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490588
        else if (_stricmp("SpecialNote", sec) == 0)
        {
            m4626a0(&parser);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0049a4b4
            Log("GFPanel::LoadPVI: '%s' Unknown Section", sec);
            return 0;
        }
    }

    popDirectory();
    return 1;
}
