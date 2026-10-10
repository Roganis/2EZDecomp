/* Game-mode name/index globals and the rolling text-slot table.

   Both functions copy into a fixed global with cl's *intrinsic* strcpy - the
   base-difference loop (`edx = dst - src; mov cl,[eax]; mov [edx+eax],cl`),
   not a CRT call - the same shape already proven by setBasePath @0x412db0. */

#include <string.h>

/* Rolling text slots, 128 bytes each. Every call site passes the global
   cursor at 0x1b2e718 as the slot index (e.g. 0x434f06, 0x43d8e4), so this
   is an indexed message/line store. The slot COUNT is not evidenced by any
   call site - 32 is a placeholder; only the 128-byte stride is proven (the
   `shl edx,7` in the address computation). */
// GLOBAL: EZ2AC_UNPACKED 0x01b2ec18
char g_textSlots[32][128];

// FUNCTION: EZ2AC_UNPACKED 0x00414e30
void setTextSlot(int slot, const char *text)
{
    strcpy(g_textSlots[slot], text);
}

/* The mode-name table: 13 entries of 0x20 bytes at 0x488ce0. The count is
   proven by the loop bound - it walks a pointer from 0x488ce0 while it stays
   `< 0x488e80`, i.e. exactly (0x488e80 - 0x488ce0) / 0x20 = 13 entries. */
// GLOBAL: EZ2AC_UNPACKED 0x00488ce0
const char kModeNames[13][32] =
{
    "5KeyMix",    "RubyMix",   "StreetMix",  "7StreetMix",
    "ClubMix",    "SpaceMix",  "5RadioMix",  "RadioMix",
    "10RadioMix", "14RadioMix", "EZ2CATCH",  "ScratchMix",
    "CV2Mix"
};

/* The next .rdata object after the table. The loop's one-past-the-end bound
   is `&kModeNames[13]` == 0x488e80, so that address has to resolve to a named
   entity on BOTH sides or the `cmp esi, ...` operand reads as a mismatch.
   Contents are a descending numeric ladder in the original; deliberately NOT
   reproduced (unknown meaning, and we don't copy data out of the binary) -
   this is placeholder storage, same convention as src/globals.cpp. */
// GLOBAL: EZ2AC_UNPACKED 0x00488e80
extern const unsigned int kAfterModeNames[1] = { 1 };

/* Current mode name + its index into kModeNames. The name buffer's size is
   not evidenced (nothing bounds it before the next known global 0x598 later);
   256 is a placeholder and does not affect this function's codegen. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e680
char g_modeName[256];

// GLOBAL: EZ2AC_UNPACKED 0x01b2e548
int g_modeIndex;

/* Note the miss path leaves g_modeIndex untouched - the loop just falls out. */
// FUNCTION: EZ2AC_UNPACKED 0x00414e50
void setModeName(const char *name)
{
    strcpy(g_modeName, name);

    for (int i = 0; i < 13; i++)
    {
        if (_stricmp(name, kModeNames[i]) == 0)
        {
            g_modeIndex = i;
            return;
        }
    }
}
