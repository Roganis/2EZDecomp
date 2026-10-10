/* KParser::parseToken @0x413940 and its static helper classifyToken
   @0x413830 - THE CONFIG TOKENIZER every .ini and .gds in the game is read
   through. Integrated from wip/parsetoken.cpp, whose logic was complete and
   verified against the Ghidra decompilation.

   INTEGRATED, NOT GROUND OUT. The park is explicit that closing its remaining
   structural divergences is "a 15-30 zoo-iteration grind", and it enumerates
   them: the original reloads the cursor from [esi+4] every iteration and reads
   the character as `mov cl,[edx]` then `movsx`, its `have` flag lives in a
   memory slot rather than a register, and its goto/label layout has to match
   cl's block ordering. None of that changes what the tokenizer DOES, and a
   port needs the behaviour.

   classifyToken IS static AND MUST STAY IN THIS TU. It was matched standalone
   once, but cl only gives it the eax-argument convention the original uses
   when it can see the real caller - the non-standard-register rule in
   CLAUDE.md. The wip draft carried a `classifyToken_force` forwarder to keep
   it alive without one; that is dropped now that parseToken is here.

   WHAT IT DOES: skip whitespace and comments, then read either a delimiter
   token (matched with strchr against the delimiter set, its type taken from
   the parallel int table at +0x14), a quoted string, or an identifier. The
   token is classified 2=ident, 3=int, 4=float, 5=string, 0=delimiter, and the
   value stored - type 3 runs atoi into lval and a conversion into fval, type 4
   runs atof with an x87 round-to-int into lval and the float into fval, and
   types 2, 5 and 0 copy the text into tok.

   AND IT RETURNS AN UNINITIALIZED LOCAL when `expected` is 0. With a non-zero
   `expected` it returns (type == expected); otherwise the original genuinely
   reads an uninitialised [esp+0x14]. Reproduced rather than tidied - a caller
   passing 0 gets whatever the frame held, and any port that "fixed" it would
   diverge from the original on exactly the paths that do so. */

#include <string.h>
#include <stdlib.h>
#include "kparser.h"

extern "C" int __cdecl isdigit(int c);

// FUNCTION: EZ2AC_UNPACKED 0x00413830
static int classifyToken(char *p)
{
    int st = 0;
    char *q = p;
    while (*(volatile char *)q != 0)
    {
        if (isdigit(*q))
        {
            if (st == 0 || st == 3)
                st = 3;
        }
        else
        {
            char ch = *q;
            if (ch == '.')
            {
                if (st == 3 || st == 0 || st == 4)
                    st = 4;
            }
            else
            {
                int c = ch;
                if (c > 127 || c == '_' ||
                    (c >= 'A' && c <= 'Z') ||
                    (c >= 'a' && c <= 'z'))
                {
                    if (st == 0 || st == 5 || st == 3)
                        st = 2;
                }
            }
        }
        ++q;
    }
    return st;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413940
unsigned KParser::parseToken(int expected)
{
    char lbuf[0x1000];
    int i = 0;
    int type;
    bool have = false;
    unsigned ret = 0;
    unsigned uninit;

    if (ttype == 1)
        return 0;

    memset(lbuf, 0, sizeof(lbuf));

    for (;;)
    {
        char c = *cursor;
        if (c == 0)
        {
            type = have ? classifyToken(lbuf) : 1;
            goto haveType;
        }
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            if (*cursor == '\n')
                f401c++;
            if (have)
            {
                type = classifyToken(lbuf);
                goto haveType;
            }
            cursor++;
        }
        else if (c == quote)
        {
            char *p = ++cursor;
            while (*p != 0)
            {
                if (*p == '\n') { f401c++; cursor++; break; }
                p++;
                cursor = p;
            }
        }
        else
        {
            char *p = strchr(delims, c);
            if (p != 0)
            {
                if (have)
                {
                    type = classifyToken(lbuf);
                    goto haveType;
                }
                type = table[p - delims];
                cursor++;
                if (type == 0)
                {
                    type = classifyToken(lbuf);
                    goto haveType;
                }
                goto haveType;
            }
            c = *cursor;
            if (c == '"')
            {
                p = ++cursor;
                while (*p != 0)
                {
                    c = *p;
                    if (c == '"')
                    {
                        cursor++;
                        type = 5;
                        lbuf[i] = 0;
                        goto haveType;
                    }
                    p++;
                    lbuf[i++] = c;
                    cursor = p;
                }
            }
            else
            {
                lbuf[i++] = c;
                cursor++;
                have = true;
            }
        }
    }

haveType:
    if (expected != 0)
        ret = (type == expected);
    else
        ret = uninit;
    if (type == 1)
        ret = 0;
    else if (type == 3)
    {
        long v = atoi(lbuf);
        lval = v;
        fval = (float)v;
    }
    else if (type == 4)
    {
        double d = atof(lbuf);
        fval = (float)d;
        lval = (int)(float)d;
    }
    else if (type == 2)
        strcpy(tok, lbuf);
    else if (type == 5)
        strcpy(tok, lbuf);
    else if (type == 0)
    {
        lbuf[0] = cursor[-1];
        lbuf[1] = 0;
        strcpy(tok, lbuf);
    }
    ttype = type;
    return ret;
}
