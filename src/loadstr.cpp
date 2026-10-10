/* KGraphics::loadStr @0x402de0 - turn a .str resource name into a cell table,
   and CACHE it. Both background parsers feed this: the SCR0 one for kinds 2
   and 3, the EZ2DJ 1st one for kinds 1 and 2.

   THE CACHE IS A KProfile keyed by the name, at KGraphics+0x1d0. A hit returns
   the node's payload at +0x50 and reports success without touching the disk,
   which is why a background that names the same clip in several slots pays for
   it once.

   ON A MISS the node is created with `size + 0x2b210` bytes: a 0x2b210-byte
   cell table followed by the raw file, both inside one allocation, with the
   file read in ABOVE the table. So the clip's parsed form and its source share
   a node and are freed together.

   The path handling is the odd part and is reproduced as-is: it calls
   GetFullPathNameA on the name, then walks to the terminator and scans
   BACKWARD for a backslash, truncating there - so the buffer ends up holding
   the resource's DIRECTORY, which is what it hands the cell-table builder as
   its third argument. The file itself is opened by the ORIGINAL name, not the
   expanded one.

   Failure to open logs "LoadAniClip: Not Found '%s'" - which is the name this
   loader goes by inside the engine, and worth knowing when reading the logs a
   cabinet produces. */

#include <string.h>
#include <windows.h>
#include "ez2acapp.h"
#include "kfile.h"
#include "kparser.h"

extern void Log(const char *fmt, ...);            /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x00402de0
int KGraphics::loadStr(char *name, Obj402860Head **out)
{
    KFile         f;
    char          full[260];
    char         *part;
    KProfileNode *node;
    int           size;
    int           r;

    node = m_clips->find(name);
    if (node != 0)
    {
        *out = (Obj402860Head *)((char *)node + 0x50);
        return 1;
    }

    /* `part` is seeded with the name right before the call - the
       original's `mov [esp+0x20],edi` between the argument pushes. */
    part = name;
    GetFullPathNameA(name, 0x104, full, &part);

    {
        int i = (int)strlen(full);
        while (i >= 0)
        {
            if (full[i] == '\\')
            {
                full[i] = 0;
                break;
            }
            i--;
        }
    }

    if (f.open(name, 1) == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00487648
        Log("LoadAniClip: Not Found '%s'", name);
        return 0;
    }

    size = f.size();
    {
        /* Named: the original loads m_clips right after f.size() returns,
           ahead of the argument arithmetic. */
        KProfile *clips = m_clips;
        node = clips->create(name, 0, size + 0x2b210);
    }
    {
        char *clip = (char *)node + 0x50;
        f.read(clip + 0x2b210, size);
        f.close();
        r = m401b80(clip, out, full);
    }
    return r;
}
