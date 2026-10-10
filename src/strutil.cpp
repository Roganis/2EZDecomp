#include <string.h>
#include <stdio.h>
#include <windows.h>

/* @0x00412f60 (__cdecl): copy the substring of `p` starting at the LAST '.'
   into `out`; if there's no '.', empty `out`. Then truncate `out` at the first
   '"'. The middle copy is an inlined strcpy(out, r). */

// FUNCTION: EZ2AC_UNPACKED 0x00412f60
void extractDotName(char *p, char *out)
{
    char *r = strrchr(p, '.');
    if (r == 0)
    {
        out[0] = 0;
        return;
    }

    char *s = r;
    char c;
    do
    {
        c = *s;
        out[s - r] = c;
        s++;
    } while (c != 0);

    char *q = strchr(out, '"');
    if (q != 0)
        *q = 0;
}

/* The game's base-path prefix + its was-set flag. Neighbor 0x412dd0 (a chdir
   helper, unmatched) prefixes '\'-rooted paths with g_basePath; buffer is
   MAX_PATH, bounded by the next global at +0x104. */
// GLOBAL: EZ2AC_UNPACKED 0x015d4168
char g_basePath[260];

// GLOBAL: EZ2AC_UNPACKED 0x015d62f0
char g_basePathSet;

/* strcpy is cl's intrinsic here (base-difference loop, no CRT call). */
// FUNCTION: EZ2AC_UNPACKED 0x00412db0
void setBasePath(const char *path)
{
    g_basePathSet = 1;
    strcpy(g_basePath, path);
}

/* Stack of saved working directories (260-byte slots); g_savedCwdPtr is the
   push cursor, lazily initialized to the stack base. */
// GLOBAL: EZ2AC_UNPACKED 0x015d426c
char *g_savedCwdPtr;

// GLOBAL: EZ2AC_UNPACKED 0x015d4270
char g_savedCwdStack[32][260];

/* Change directory, optionally pushing the current one; '\'-rooted paths are
   prefixed with g_basePath. The path push before the branch is shared: it is
   both sprintf's last-pushed arg and SetCurrentDirectoryA's argument. */
// FUNCTION: EZ2AC_UNPACKED 0x00412dd0
void changeDir(char *path, int savePrev)
{
    char buf[260];

    if (savePrev)
    {
        if (g_savedCwdPtr == 0)
            g_savedCwdPtr = g_savedCwdStack[0];
        GetCurrentDirectoryA(260, g_savedCwdPtr);
        g_savedCwdPtr += 260;
    }

    if (*path == '\\')
    {
        // STRING: EZ2AC_UNPACKED 0x00488b04
        sprintf(buf, "%s%s", g_basePath, path);
        SetCurrentDirectoryA(buf);
    }
    else
    {
        SetCurrentDirectoryA(path);
    }
}

/* Pop the directory stack (clamped at the base) and restore it. */
// FUNCTION: EZ2AC_UNPACKED 0x00412e60
void popDirectory()
{
    g_savedCwdPtr -= 260;
    if (g_savedCwdPtr < g_savedCwdStack[0])
        g_savedCwdPtr = g_savedCwdStack[0];
    SetCurrentDirectoryA(g_savedCwdPtr);
}

/* Copy the basename of `name` (after the LAST '\', up to the LAST '.')
   into out. The memcpy is the /Oi rep-movs intrinsic. */
// FUNCTION: EZ2AC_UNPACKED 0x00412f00
void extractBaseName(char *name, char *out)
{
    char *tail = strrchr(name, '\\');
    if (tail == 0)
        tail = name;
    else
        tail = tail + 1;
    char *end = strrchr(name, '.');
    if (end == 0)
        end = name + strlen(name);
    int len = end - tail;
    memcpy(out, tail, len);
    out[len] = 0;
}

/* Case-insensitive extension test via extractDotName (defined above —
   callee-first here is authentic: the original TU had the same order). */
// FUNCTION: EZ2AC_UNPACKED 0x00412fb0
int hasExtension(const char *ext, char *name)
{
    char buf[260];
    extractDotName(name, buf);
    return _stricmp(buf, ext) == 0;
}

/* Replace name's extension (from the LAST '.', or append at the end,
   staying inside a trailing '"'). len is computed unconditionally; the
   name[len+1]=0 store reuses cl, which the strlen intrinsic left at 0. */
// FUNCTION: EZ2AC_UNPACKED 0x00413000
void replaceExtension(char *name, char *ext)
{
    char *end = name + strlen(name);
    int quoted = end[-1] == '"';
    char *p = strrchr(name, '.');
    if (p == 0)
        p = quoted ? end - 1 : end;
    strcpy(p, ext);
    int len = strlen(name);
    if (quoted)
    {
        name[len] = '"';
        name[len + 1] = 0;
    }
}

/* extractDirName @0x412e90 - getExeDir's callee. Copy src to out, then
   truncate at the LAST '\\' or, failing that, the first '/'; if the path has
   neither, empty the string. After truncating, a leading '"' is stripped with
   memmove(out, out + 1, strlen(out)).

   Integrated from wip/ at 97%: 30 of 31 instructions are byte-identical,
   including both p-test displacements, and the one that is not is a branch
   TARGET, not a branch. The quote-skip test aims at the pop/ret pair right
   after the memmove in the original (75 1e) and at the identical pop/ret of
   the trailing `*out = 0` block here (75 23). cl's tail merger canonicalises
   equivalent epilogues to the textually LAST one and the original takes the
   FIRST; if/else in both orders, early-return arms, an explicit double
   return, a goto-pinned join, flat against nested p-tests and an inlined
   static helper all give the same 75 23. Three more were swept on integration
   - putting the `*out = 0` path FIRST so that cl's "textually last" epilogue
   would BE the memmove path's, as an early return, as an else, and with the
   two p-tests nested - and all three are WORSE (86.4%, and two instructions
   longer), so the canonicalisation is not simply positional. Same class as
   the shared-block attachment rule in CLAUDE.md, on the half of it that is
   not steerable. */
// FUNCTION: EZ2AC_UNPACKED 0x00412e90
void extractDirName(char *src, char *out)
{
    char *p;

    strcpy(out, src);
    p = strrchr(out, '\\');
    if (p == 0)
        p = strchr(out, '/');
    if (p != 0)
    {
        *p = 0;
        if (*out == '"')
            memmove(out, out + 1, strlen(out));
        return;
    }
    *out = 0;
}
