/* KEzwLoader and KWavLoader - the sound subsystem's two audio format loaders.
   See include/loaders.h for how they name themselves and for the layout.

   The two classes are the same code twice with ONE constant different: the
   header size the payload offset is measured from, 0x12 for .ssf and 0x2c for
   .wav. Everything else - the fopen/fread/fseek/ftell/fclose sequence, the
   three-way seek, the clamp, the "already there" shortcut - is identical, and
   the original's linker ICF-folded the two slots that carry no constant at all
   (close and tell). */

#include "kcrt.h"      /* operator new = malloc, operator delete = free */
#include "loaders.h"
#include <string.h>

/* The two format headers, packed because every field after the first is
   unaligned in the file. The .ssf one IS 18 bytes - the same 18 the loader
   measures its payload offset from - and the .wav one is the canonical 44-byte
   RIFF/WAVE header, read whole and picked apart at its standard offsets. */
#pragma pack(push, 1)
struct SsfHeader
{
    unsigned short nChannels;        /* 0x00 */
    unsigned int   nSamplesPerSec;   /* 0x02 */
    unsigned int   nAvgBytesPerSec;  /* 0x06 */
    unsigned short nBlockAlign;      /* 0x0a */
    unsigned short wBitsPerSample;   /* 0x0c */
    unsigned int   dataLength;       /* 0x0e */
};

struct WavHeader
{
    char           _riff[0x16];      /* 0x00 - "RIFF" .. wFormatTag */
    unsigned short nChannels;        /* 0x16 */
    unsigned int   nSamplesPerSec;   /* 0x18 */
    unsigned int   nAvgBytesPerSec;  /* 0x1c */
    unsigned short nBlockAlign;      /* 0x20 */
    unsigned short wBitsPerSample;   /* 0x22 */
    char           _data[4];         /* 0x24 - "data" */
    unsigned int   dataLength;       /* 0x28 */
};
#pragma pack(pop)

/* The header readers. Both are `static`, which is what lets cl give them their
   own register convention - the original passes the FILE* in eax (.ssf) and
   the two out-pointers in esi/edi with no stack slots at all. By CLAUDE.md's
   static-convention rule they have to live in the same TU as their callers,
   which is why they are here rather than in a header. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cdf0
static int readSsfHeader(FILE *f, WaveFormat *fmt, int *outLen)
{
    SsfHeader hdr;

    if (f == 0)
    {
        return 0;
    }
    if (fread(&hdr, sizeof(hdr), 1, f) != 1)
    {
        return 0;
    }

    if (fmt != 0)
    {
        memset(fmt, 0, sizeof(*fmt));
        fmt->wFormatTag = 1;
        fmt->nChannels = hdr.nChannels;
        fmt->nSamplesPerSec = hdr.nSamplesPerSec;
        fmt->nAvgBytesPerSec = hdr.nAvgBytesPerSec;
        fmt->nBlockAlign = hdr.nBlockAlign;
        fmt->wBitsPerSample = hdr.wBitsPerSample;
        fmt->cbSize = 0;
    }

    if (outLen != 0)
    {
        *outLen = hdr.dataLength;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040efc0
static int readWavHeader(FILE *f, WaveFormat *fmt, int *outLen)
{
    WavHeader hdr;

    if (f == 0)
    {
        return 0;
    }
    if (fread(&hdr, sizeof(hdr), 1, f) != 1)
    {
        return 0;
    }

    if (fmt != 0)
    {
        memset(fmt, 0, sizeof(*fmt));
        fmt->wFormatTag = 1;
        fmt->nChannels = hdr.nChannels;
        fmt->nSamplesPerSec = hdr.nSamplesPerSec;
        fmt->nAvgBytesPerSec = hdr.nAvgBytesPerSec;
        fmt->nBlockAlign = hdr.nBlockAlign;
        fmt->wBitsPerSample = hdr.wBitsPerSample;
        fmt->cbSize = 0;
    }

    if (outLen != 0)
    {
        *outLen = hdr.dataLength;
    }

    return 1;
}

/* The two runtime classes and their descriptors. Placeholder storage: only the
   addresses have to line up, and the contents are data the reconstruction does
   not build. */
// GLOBAL: EZ2AC_UNPACKED 0x004a72a8
KRuntimeClass classKEzwLoader;
// GLOBAL: EZ2AC_UNPACKED 0x004a72b8
LoaderInfo g_4a72b8;
// GLOBAL: EZ2AC_UNPACKED 0x004a7320
KRuntimeClass classKWavLoader;
// GLOBAL: EZ2AC_UNPACKED 0x004a7330
LoaderInfo g_4a7330;

/* ---- KEzwLoader -------------------------------------------------------- */

// FUNCTION: EZ2AC_UNPACKED 0x0040cdd0
void *KEzwLoader::getClassDesc()
{
    return &classKEzwLoader;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040cde0
void *KEzwLoader::getObjectInfo()
{
    return &g_4a72b8;
}

/* Close the file by hand rather than through slot 5: the destructor restores
   its own vptr first, so a virtual call here would go to this class anyway,
   and the original writes the three statements out. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cff0
KEzwLoader::~KEzwLoader()
{
    if (m_file != 0)
    {
        fclose(m_file);
        m_file = 0;
        m_pos = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0040d0b0
// KEzwLoader::`scalar deleting destructor'

/* Close whatever is open (through the vtable), then open `name`, remember it
   in the base's buffer, read the format header and publish the payload length
   both into the member and through the caller's pointer. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ce80
int KEzwLoader::open(char *name, int *outLen, WaveFormat *fmt)
{
    if (m_file != 0)
    {
        close();
    }

    if (name == 0)
    {
        return 0;
    }

    strcpy((char *)buf, name);
    // STRING: EZ2AC_UNPACKED 0x00487f50
    m_file = fopen(name, "rb");
    if (m_file == 0)
    {
        return 0;
    }

    m_pos = 0;
    if (readSsfHeader(m_file, fmt, &m_total) == 0)
    {
        return 0;
    }

    if (outLen != 0)
    {
        *outLen = m_total;
    }

    return 1;
}

/* Read `count` payload bytes, or the whole payload when count is -1, and
   re-derive the payload offset from where the file ended up. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cf00
int KEzwLoader::read(void *dst, int count)
{
    if (m_file != 0 && dst != 0)
    {
        if (count == -1)
        {
            count = m_total;
        }
        if (count != 0)
        {
            fread(dst, count, 1, m_file);
        }
        m_pos = ftell(m_file) - 0x12;
        return 1;
    }

    return 0;
}

/* Slot 5, shared with KWavLoader (the alias is in src/stubs.cpp). */
// FUNCTION: EZ2AC_UNPACKED 0x0040f160 folded
int KEzwLoader::close()
{
    if (m_file != 0)
    {
        fclose(m_file);
        m_file = 0;
    }
    m_pos = 0;
    return 1;
}

/* Slot 7, shared with KWavLoader. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f230 folded
int KEzwLoader::tell()
{
    return m_pos;
}

// VTABLE: EZ2AC_UNPACKED 0x004882c0
struct KEzwLoader;

// FUNCTION: EZ2AC_UNPACKED 0x0040d030
KEzwLoader *createKEzwLoader()
{
    return new KEzwLoader;
}

/* ---- KWavLoader -------------------------------------------------------- */

// FUNCTION: EZ2AC_UNPACKED 0x0040efa0
void *KWavLoader::getClassDesc()
{
    return &classKWavLoader;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040efb0
void *KWavLoader::getObjectInfo()
{
    return &g_4a7330;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f240
KWavLoader::~KWavLoader()
{
    if (m_file != 0)
    {
        fclose(m_file);
        m_file = 0;
        m_pos = 0;
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f300
// KWavLoader::`scalar deleting destructor'

/* Slots 5 and 7, the two the original's linker folded across the pair. Both
   copies carry a `folded` annotation and both carry the REAL body: an alias
   whose body is a stub would turn the shared address into a stub entity and
   the winner would stop being scored. */
// FUNCTION: EZ2AC_UNPACKED 0x0040f160 folded
int KWavLoader::close()
{
    if (m_file != 0)
    {
        fclose(m_file);
        m_file = 0;
    }
    m_pos = 0;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f230 folded
int KWavLoader::tell()
{
    return m_pos;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f070
int KWavLoader::open(char *name, int *outLen, WaveFormat *fmt)
{
    if (m_file != 0)
    {
        close();
    }

    if (name == 0)
    {
        return 0;
    }

    strcpy((char *)buf, name);
    m_file = fopen(name, "rb");
    if (m_file == 0)
    {
        return 0;
    }

    m_pos = 0;
    if (readWavHeader(m_file, fmt, &m_total) == 0)
    {
        return 0;
    }

    if (outLen != 0)
    {
        *outLen = m_total;
    }

    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x0040f100
int KWavLoader::read(void *dst, int count)
{
    if (m_file != 0 && dst != 0)
    {
        if (count == -1)
        {
            count = m_total;
        }
        if (count != 0)
        {
            fread(dst, count, 1, m_file);
        }
        m_pos = ftell(m_file) - 0x2c;
        return 1;
    }

    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x004885c4
struct KWavLoader;

// FUNCTION: EZ2AC_UNPACKED 0x0040f280
KWavLoader *createKWavLoader()
{
    return new KWavLoader;
}
