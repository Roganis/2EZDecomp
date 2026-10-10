/* EzSubsystem::openSample @0x40e2a0 - resolve a sample name and open
   its stream. The record (arg 2, or looked up by name through m40e200
   when null) carries the loader's runtime class in its first word; a
   fresh loader is created from it, the name - run through the
   subsystem's field-extension rewrite against the record - is opened
   on the loader's slot 3 (which may rewrite the caller's pointer
   through `out`), and on success the loader lands in *in, or is
   released at once when the caller wants only the name resolved. Both
   failure arms log and fall into one shared release.

   Its own TU: m40e200, createObject and applyFieldExtension live
   elsewhere, and createSample - this function's caller - must not see
   this body. */
#include <string.h>
#include "ez2acapp.h"
#include "kengineapp.h"
#include "ksound.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (folded) */

struct Obj40E250 { void applyFieldExtension(char *name, struct KParser *p); };

// FUNCTION: EZ2AC_UNPACKED 0x0040e2a0
int EzSubsystem::openSample(char *name, int a, char **out, int b,
                            EzStream **in)
{
    char      buf[0x104];
    void     *rec;
    EzStream *s;

    if (name == 0)
        return 0;
    rec = (void *)a;
    if (rec == 0)
    {
        rec = m40e200(name);
        if (rec == 0)
            goto ret0;
    }
    s = (EzStream *)((KRuntimeClass *)*(void **)rec)->createObject();
    if (s == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004883b0
        Log("Allocation Error : OpenSample  - new sample loader");
    }
    else
    {
        strcpy(buf, name);
        ((Obj40E250 *)this)->applyFieldExtension(buf, (struct KParser *)rec);
        if (s->open(buf, out, b) == 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00488384
            Log("Open Error : OpenSample - Open sample %s", buf);
        }
        else
        {
            if (in != 0)
                *in = s;
            else
                s->release(1);
            return 1;
        }
    }
    if (s != 0)
        s->release(1);
ret0:
    return 0;
}
