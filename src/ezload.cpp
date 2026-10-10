#include <string.h>
#include <windows.h>
#include "ezchart.h"
#include "loader.h"

/* Top-level chart load-by-basename (vtable slot 4). Own TU on purpose:
   loadEZI's body must not be visible here or cl inlines it into the caller.
   Flow: strip the extension, decrypt "base.ezi" to the "base.ezz" temp and
   parse it (sample table), then decrypt "base.ez" to "base.es" and parse the
   chart; temps are deleted either way, samples unloaded when the chart parse
   fails. decryptFile/decryptFile2 are thiscall members that ignore `this`
   (call sites still set ecx); loadEZI/unloadSamples are reached through the
   EziLoader view of the same object (the cast emits no code). */
// FUNCTION: EZ2AC_UNPACKED 0x00411720
int EzChart::load(char *name)
{
    char tmpName[260];
    char srcName[260];
    char encName[260];

    strcpy(srcName, name);
    char *dot = strrchr(srcName, '.');
    if (dot != 0)
        *dot = 0;

    strcpy(encName, srcName);
    // STRING: EZ2AC_UNPACKED 0x004889d4
    strcat(encName, ".ezi");
    strcpy(tmpName, srcName);
    // STRING: EZ2AC_UNPACKED 0x004889cc
    strcat(tmpName, ".ezz");

    decryptFile2(encName);
    if (((EziLoader *)this)->loadEZI(tmpName) == 0)
    {
        DeleteFileA(tmpName);
        return 0;
    }
    DeleteFileA(tmpName);

    strcpy(encName, srcName);
    // STRING: EZ2AC_UNPACKED 0x004889c8
    strcat(encName, ".ez");
    strcpy(tmpName, srcName);
    // STRING: EZ2AC_UNPACKED 0x004889c4
    strcat(tmpName, ".es");

    decryptFile(encName);
    if (loadEZ(tmpName) == 0)
    {
        ((EziLoader *)this)->unloadSamples();
        DeleteFileA(tmpName);
        return 0;
    }
    DeleteFileA(tmpName);
    return 1;
}
