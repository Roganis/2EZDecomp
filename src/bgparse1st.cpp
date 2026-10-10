/* BackGround::m41b0c0 @0x41b0c0 and CatchBackGround::m46b0e0 @0x46b0e0 - the
   EZ2DJ 1st BACKGROUND FORMAT, which is what the .scr parser falls back to
   when the buffer does not begin "SCR0". Its own error string names it.

   A SECOND, OLDER FORMAT the port needs, and structurally unlike the SCR0 one:

     int frames;                       the count, published to +0x10211c
     then per frame:  int a; int b;    two words
                      then EXACTLY THREE sub-records

   A sub-record is `int kind` followed by a kind-sized payload, and the sizes
   do not follow one rule:

     kind 0, 5     nothing at all - just the 4-byte kind
     kind 1        0x84 payload; a name loaded through loadStr, handle stored
                   back over the name
     kind 2        0x80 payload; the same load
     kind 3, 4, 6  8 bytes skipped, then a 0x80 payload whose name is copied
                   but NOT loaded
     anything else logs "Unsuported Type" (sic) and fails the parse

   Each frame's entry in the table at +0x102138 is 0x14 bytes - the two words
   plus a pointer to each of the three sub-records - and the walk ends by
   publishing +0x102134 to the cursor at +0x103538 and setting the +0xac flag.

   THE EXTENSION IS APPENDED, NOT REPLACED. This format's names arrive without
   one, so the loader walks to the terminator and writes ".str\0" there - which
   cl emits as a dword store plus a byte store out of the literal. The SCR0
   parser calls replaceExtension instead. That difference is a real property of
   the two formats, not a codegen accident. */

#include <string.h>
#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

extern void Log(const char *fmt, ...);            /* 0x402880 */

// FUNCTION: EZ2AC_UNPACKED 0x0041b0c0
int BackGround::m41b0c0()
{
    char  path[260];
    char *p;
    int  *ent;
    int   frames;
    int   i, j;

    p = m_scrText;
    frames = *(int *)p;
    p += 4;
    *(int *)((char *)this + 0x10211c) = frames;

    i = 0;
    if (frames > 0)
    {
        ent = (int *)((char *)this + 0x102138);
        do
        {
            ent[-1] = *(int *)p;
            p += 4;
            ent[0] = *(int *)p;
            p += 4;

            j = 0;
            do
            {
                int kind;

                ent++;
                *ent = (int)p;
                kind = *(int *)p;
                p += 4;

                if (kind == 0)
                {
                }
                else if (kind == 1)
                {
                    strcpy(path, p);
                    strcat(path, ".str");
                    {
                        struct Obj402860Head *h;

                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)p = h;
                    }
                    p += 0x84;
                }
                else if (kind == 2)
                {
                    strcpy(path, p);
                    strcat(path, ".str");
                    {
                        struct Obj402860Head *h;

                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)p = h;
                    }
                    p += 0x80;
                }
                else if (kind == 5)
                {
                }
                else if (kind == 3)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else if (kind == 4)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else if (kind == 6)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else
                {
                    Log("EZ2DJ 1st Background:  Unsuported Type: %d. Please Report to To", kind);
                    return 0;
                }

                j++;
            } while (j < 3);

            i++;
            ent = (int *)((char *)this + 0x102138) + i * 5;
        } while (i < *(int *)((char *)this + 0x10211c));
    }

    *(int *)((char *)this + 0x103538) = (int)((char *)this + 0x102134);
    fac = 1;
    return 1;
}

/* The catch copy - its own offsets throughout. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b0e0
int CatchBackGround::m46b0e0()
{
    char  path[260];
    char *p;
    int  *ent;
    int   frames;
    int   i, j;

    p = m_scrText;
    frames = *(int *)p;
    p += 4;
    *(int *)((char *)this + 0x102110) = frames;

    i = 0;
    if (frames > 0)
    {
        ent = (int *)((char *)this + 0x10212c);
        do
        {
            ent[-1] = *(int *)p;
            p += 4;
            ent[0] = *(int *)p;
            p += 4;

            j = 0;
            do
            {
                int kind;

                ent++;
                *ent = (int)p;
                kind = *(int *)p;
                p += 4;

                if (kind == 0)
                {
                }
                else if (kind == 1)
                {
                    strcpy(path, p);
                    strcat(path, ".str");
                    {
                        struct Obj402860Head *h;

                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)p = h;
                    }
                    p += 0x84;
                }
                else if (kind == 2)
                {
                    strcpy(path, p);
                    strcat(path, ".str");
                    {
                        struct Obj402860Head *h;

                        ((KGraphics *)g_sub1b2f054)->loadStr(path, &h);
                        *(struct Obj402860Head **)p = h;
                    }
                    p += 0x80;
                }
                else if (kind == 5)
                {
                }
                else if (kind == 3)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else if (kind == 4)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else if (kind == 6)
                {
                    p += 8;
                    strcpy(path, p);
                    p += 0x80;
                }
                else
                {
                    // STRING: EZ2AC_UNPACKED 0x0048e1d8
                    Log("EZ2DJ 1st Background:  Unsuported Type: %d. Please Report to To", kind);
                    return 0;
                }

                j++;
            } while (j < 3);

            i++;
            ent = (int *)((char *)this + 0x10212c) + i * 5;
        } while (i < *(int *)((char *)this + 0x102110));
    }

    *(int *)((char *)this + 0x10352c) = (int)((char *)this + 0x102128);
    fa4 = 1;
    return 1;
}
