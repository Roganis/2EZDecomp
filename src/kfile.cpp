#include <windows.h>
#include <stdio.h>
#include "kcrt.h"
#include "kfile.h"

/* Class definition in include/kfile.h; the vtable entity is pinned from this
   TU (markers in headers are ignored — source_root is src/). */

// VTABLE: EZ2AC_UNPACKED 0x00488b0c
struct KFile;

// FUNCTION: EZ2AC_UNPACKED 0x00413080
KFile::KFile()
{
    handle = 0;
}

/* One virtual ~KFile() definition emits two PDB symbols reccmp maps separately:
   the real destructor ??1KFile -> 0x413200 (the plain FUNCTION marker on the
   definition), and the scalar deleting destructor ??_GKFile -> 0x413220
   (inlines this body + conditional operator delete; targeted by-name via the
   standalone offset comment, kept blank-line-separated so it does not bind to
   the definition). */
// FUNCTION: EZ2AC_UNPACKED 0x00413220
// KFile::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00413200
KFile::~KFile()
{
    if (handle != 0)
    {
        CloseHandle(handle);
        handle = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00413090
int KFile::open(char *name, int flags)
{
    if (handle != 0)
        close();

    /* The mode is tested a byte at a time (dl) while the raw flags value is
       re-read for the default; the volatile byte read reproduces cl's split
       load instead of caching the whole dword in a register. */
    int access, disp;
    unsigned char mode = *(volatile unsigned char *)&flags;
    if (mode & 1)
    {
        access = GENERIC_READ;
        disp = OPEN_EXISTING;
    }
    else
    {
        access = flags;
        disp = *(volatile int *)&flags;
    }
    if (mode & 2)
    {
        access = GENERIC_WRITE;
        disp = CREATE_ALWAYS;
    }
    handle = CreateFileA(name, access, 0, 0, disp, FILE_ATTRIBUTE_NORMAL, 0);
    return handle != INVALID_HANDLE_VALUE;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413100
void KFile::close()
{
    if (handle != 0)
    {
        CloseHandle(handle);
        handle = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x00413120
int KFile::read(void *buf, int len)
{
    DWORD n;
    ReadFile(handle, buf, len, &n, 0);
    return (int)n == len;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413150
int KFile::write(void *buf, int len)
{
    DWORD n;
    WriteFile(handle, buf, len, &n, 0);
    return (int)n == len;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413180
unsigned KFile::size()
{
    return GetFileSize(handle, 0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00413190
int KFile::printf(const char *fmt, ...)
{
    char buf[0x100];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    return write(buf, (int)strlen(buf));
}
