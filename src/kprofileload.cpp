/* KProfile::Load @0x413fe0 - the profile-file record loader, integrated from
   wip/kprofile-load.cpp at 53.14%. LOGIC 100% VERIFIED against the Ghidra
   decompilation and the original disassembly: every instruction of the body
   below corresponds 1:1 with the original. What differs is the whole-frame
   form.

   THE WALL: the original is an FPO-EH chkstk frame - no ebp, no `and esp,-8` -
   and cl 13.10 emits `push ebp; mov ebp,esp; and esp,-8` for it. This is the
   binary's ONLY such function, and CLAUDE.md documents the cause: cl emits
   that prologue for any function with a frame local, a loop, and an 8-byte FP
   push inside the loop, and a float promoted to double for a vararg Log is
   enough to trigger it. Twenty-plus flags, three cl builds and thirty-five
   source shapes were tried, including helper splits and LTCG.

   Integrated at that rather than left parked, because it is a FILE FORMAT
   reader - the profile records every .ini in the game is built from - and a
   decoded loader in wip/ is invisible to a build. The alternate helper-split
   form the park also carried is left behind with it; it reached the unaligned
   prologue but only by not inlining the helper, which is not a shape that
   survives. */

#include <string.h>
#include "kparser.h"
#include "kprofile.h"
#include "loader.h"   /* Log */

/* KProfile::Load lives in its own TU: it calls find/create/setStr, whose
   bodies are defined in src/kprofile.cpp - if cl could see them here it
   would inline them and destroy the caller's codegen. */

/* Parse a whole config file into the profile tree. Records are
   "key = value": token 5 (STR) keys, token 0xc (= separator), then a free
   token whose classified type dispatches INT (3) / FLOAT (4) / STR (5)
   into the KProfileNode tree. EOF (no next key) returns 1; a key without
   its "=" returns 0. */
// FUNCTION: EZ2AC_UNPACKED 0x00413fe0
int KProfile::Load(char *name)
{
    KParser parser;
    char key[0x80];

    if (parser.readFile(name) == 0)
        return 0;

    for (;;)
    {
        parser.tok[0] = 0;
        if (parser.parseToken(5) == 0)
            return 1;
        strcpy(key, parser.tok);
        if (parser.parseToken(0xc) == 0)
            return 0;
        parser.parseToken(0);
        if (parser.ttype == 3)
        {
            // STRING: EZ2AC_UNPACKED 0x00488bd8
            Log("KProfile::Load - INT %s:%d", key, parser.lval);
            int n = parser.lval;
            KProfileNode *node = find(key);
            if (node == 0)
                node = create(key, 1, 4);
            node->value = n;
        }
        else if (parser.ttype == 4)
        {
            /* the original's format string really says %d for a float */
            // STRING: EZ2AC_UNPACKED 0x00488bb8
            Log("KProfile::Load - FLOAT %s:%d", key, parser.fval);
            float f = parser.fval;
            KProfileNode *node = find(key);
            if (node == 0)
                node = create(key, 2, 4);
            *(float *)&node->value = f;
        }
        else if (parser.ttype == 5)
        {
            // STRING: EZ2AC_UNPACKED 0x00488b9c
            Log("KProfile::Load - STR %s:%s", key, parser.tok);
            setStr(key, parser.tok);
        }
    }
}
