#include <windows.h>
#include <string.h>
#include "kcrt.h"
#include "kfile.h"
#include "kparser.h"
#include "loader.h"   /* Log */

/* Class definition in include/kparser.h; the vtable entity is pinned from
   this TU (markers in headers are ignored — source_root is src/). */

extern char g_parserDelims[16];   // 0x4a8520
extern int  g_parserTable[14];    // 0x4a8530

// VTABLE: EZ2AC_UNPACKED 0x00488b28
struct KParser;

// FUNCTION: EZ2AC_UNPACKED 0x004138f0
// KParser::`scalar deleting destructor'

/* One definition emits both symbols: ??1 (the real dtor, plain marker below,
   original body = reinstall vtable + ret) and ??_G (by-name marker above).
   0x413820 was previously parked in wip/kparser-ctor.cpp as an "empty ctor
   missing return-this" - it is actually the destructor, which has no
   return-this, and matches as-is. */
// FUNCTION: EZ2AC_UNPACKED 0x00413820
KParser::~KParser()
{
}

/* Buffer-parser constructor: point at the embedded buffer, install the two
   character tables. cl materializes `this` in eax (the ctor return value) and
   uses ecx as scratch for `lea ecx,[eax+0x18]`. */
// FUNCTION: EZ2AC_UNPACKED 0x004138c0
KParser::KParser()
{
    base   = buf;
    size   = 0x4000;
    quote  = '\'';
    delims = g_parserDelims;
    table  = g_parserTable;
}

/* Re-point the parser at an external buffer and reset. Store order is
   load-bearing (both args pre-loaded, eax reused for the two same-value
   stores). */
// FUNCTION: EZ2AC_UNPACKED 0x00413910
void KParser::setBuffer(char *p, int n)
{
    base   = p;
    size   = n;
    cursor = p;
    f401c  = 1;
    ttype  = 0;
}

/* Extract a field from a "key;value;key;value;" string held at base(+0x8):
   empty key -> the first token; else the token after the key's separator.
   Both ';' searches are strchr @0x478740 - the NEXT separator (the C had
   strrchr, under a swapped library annotation, until 2026-10-01); strncpy
   (not memcpy) is the real CRT call. */
// FUNCTION: EZ2AC_UNPACKED 0x00413260
int KParser::getField(char *key)
{
    char buf64[64];
    if (base == 0)
        return 0;

    int len = (int)strlen(base);
    char *p;
    if (*key == 0)
    {
        p = base;
    }
    else
    {
        strcpy(buf64, key);
        _strlwr(buf64);
        char *f = strstr(base, buf64);
        if (f == 0)
            return 0;
        p = strchr(f, ';');
        if (p == 0)
            return 0;
        p++;
        if (*p == 0)
            return 0;
    }
    char *e = strchr(p, ';');
    if (e == 0)
        e = base + len;
    int n = (int)(e - p);
    strncpy(key, p, n);
    key[n] = 0;
    return 1;
}

/* Scan the parse buffer field by field via getField(), returning 1 at the
   first that case-insensitively equals `key`, else 0. */
// FUNCTION: EZ2AC_UNPACKED 0x00413350
int KParser::seek(char *key)
{
    char buf64[64];
    buf64[0] = 0;
    while (getField(buf64) == 1)
    {
        if (_stricmp(buf64, key) == 0)
            return 1;
    }
    return 0;
}

/* Load a config file into the parser buffer. If the name ends ".ini" the
   encrypted file is first decrypted to a ".int" sibling via slurpFile()
   (parked, wip/slurpdecrypt.cpp) and that temp file is read and deleted.
   Fails (0) on decrypt error, open error, or a file bigger than buf. */
// FUNCTION: EZ2AC_UNPACKED 0x00413bc0
int KParser::readFile(char *name)
{
    KFile f;
    char tmp[0x200];
    char fin[0x200];
    int isTemp = 0;

    strcpy(fin, name);

    int i = 0;
    char *p = name;
    char *d = tmp;
    do
    {
        char c = *p;
        *d = c;
        if (c == '.')
        {
            if (name[i + 1] == 'i' && name[i + 2] == 'n' && name[i + 3] == 'i')
            {
                isTemp = 1;
                tmp[i + 1] = 'i';
                tmp[i + 2] = 'n';
                tmp[i + 3] = 't';
                tmp[i + 4] = 0;
                if (slurpFile(name) == -1)
                {
                    // STRING: EZ2AC_UNPACKED 0x00488b50
                    Log("KParser %s cannot open", name);
                    return 0;
                }
                strcpy(fin, tmp);
            }
            break;
        }
        i++;
        p++;
        d++;
    } while (i < 0x80);

    if (f.open(fin, 1) == 0)
    {
        Log("KParser %s cannot open", fin);
        return 0;
    }

    unsigned n = f.size();
    if (n > (unsigned)size)
    {
        // STRING: EZ2AC_UNPACKED 0x00488b2c
        Log("KParser %s buffer size exceed %d", name, 0x4000);
        f.close();
        if (isTemp)
            DeleteFileA(fin);
        return 0;
    }

    f.read(base, n);
    base[n] = 0;
    f.close();
    cursor = base;
    f401c = 1;
    ttype = 0;
    if (isTemp)
        DeleteFileA(fin);
    return 1;
}

/* Take the next string token into the caller's buffer, reporting whether there
   was one. The token buffer is emptied first, so a failed read leaves the
   caller's buffer untouched and the parser's empty. `this` survives in ecx
   across the parseToken call because nothing between the two clobbers it. */
// FUNCTION: EZ2AC_UNPACKED 0x00456170
int KParser::readGdsString(char *out)
{
    char *s = tok;

    *s = 0;

    if (parseToken(2) == 0)
    {
        return 0;
    }

    strcpy(out, s);
    return 1;
}
