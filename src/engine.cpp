#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "ez2acapp.h"
#include "ezscreen.h"
#include "kprofile.h"
#include <stdio.h>
#include "kparser.h"
#include "ksongplayer.h"
#include "loader.h" /* Log */

/* Odds and ends whose callees (KProfile::find/create, KParser::parseToken) and
   callers both live elsewhere, so this TU keeps cl from inlining either way. */

/* First free entry, or -1 when all sixteen are taken. A real helper: the
   original's dead `cmp eax,-1` right after the search is what an inlined
   returns-minus-one function leaves behind (CLAUDE.md's rule), and writing the
   loop inline instead merges the two return-0 tails. */
static int findFreeSlot(void **slots)
{
    int i;

    for (i = 0; i < 16; i++)
    {
        if (slots[i] == 0)
        {
            return i;
        }
    }

    return -1;
}

/* Register one object with the subsystem. */
// FUNCTION: EZ2AC_UNPACKED 0x0040e180
int EzSubsystem::bind(void *p)
{
    int i = findFreeSlot(slots);

    if (i == -1)
    {
        return 0;
    }

    slots[i] = p;
    count++;
    return 1;
}

/* Read one integer out of the .gds the parser holds: skip two tokens, clear the
   parser's value and take it only if the third token really was an int. The
   unconditional `return 1` is load-bearing: it gives both paths one shared tail,
   where a void return duplicates the epilogue into the if-body. */
// FUNCTION: EZ2AC_UNPACKED 0x00456270
int readGdsInt(int *out, KParser *p)
{
    p->parseToken(2);
    p->parseToken(0xc);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        *out = p->lval;
    }

    return 1;
}

/* One float. Unlike the int readers there is no guard: whatever the tokenizer
   left in fval is taken, which is why fval is cleared first. */
// FUNCTION: EZ2AC_UNPACKED 0x00456230
int readGdsFloat(float *out, KParser *p)
{
    p->parseToken(2);
    p->parseToken(0xc);
    p->fval = 0.0f;
    p->parseToken(4);
    *out = p->fval;
    return 1;
}

/* Two floats separated by a comma - a position or a size. */
// FUNCTION: EZ2AC_UNPACKED 0x00456590
int readGdsFloat2(float *a, float *b, KParser *p)
{
    p->parseToken(2);
    p->parseToken(0xc);
    p->fval = 0.0f;
    p->parseToken(4);
    *a = p->fval;
    p->parseToken(0xa);
    p->fval = 0.0f;
    p->parseToken(4);
    *b = p->fval;
    return 1;
}

/* Two ints separated by a comma, each left alone when its token is not one. */
// FUNCTION: EZ2AC_UNPACKED 0x00456600
int readGdsInt2(int *a, int *b, KParser *p)
{
    p->parseToken(2);
    p->parseToken(0xc);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        *a = p->lval;
    }

    p->parseToken(0xa);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        *b = p->lval;
    }

    return 1;
}

/* Four comma-separated components packed into one ARGB word: the three later
   ones fall back to the FIRST when their token is not an int, and the first is
   never initialized - cl gives it the dead `p` parameter's stack slot, so a
   colour whose first component is not an int reads back whatever the caller
   pushed.

   `r` has to be volatile AND the final read has to be non-volatile. cl spills
   the second component and keeps the first in ebx here, the exact opposite of
   the original, and neither declaration order nor the shape of the else-arms
   moves it - volatile is what pins the first to memory, giving the original's
   `mov [r],eax` store and `mov reg,[r]` else-loads. But volatile also stops cl
   folding it into the accumulating `or`, which the original does, so the last
   read reaches the same storage through a plain `const int *`. */
// FUNCTION: EZ2AC_UNPACKED 0x004562c0
int readGdsColor(unsigned int *out, KParser *p)
{
    volatile int r;
    int g, b, a;

    p->parseToken(2);
    p->parseToken(0xc);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        r = p->lval;
    }

    p->parseToken(0xa);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        g = p->lval;
    }
    else
    {
        g = r;
    }

    p->parseToken(0xa);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        b = p->lval;
    }
    else
    {
        b = r;
    }

    p->parseToken(0xa);
    p->lval = 0;

    if (p->parseToken(3) != 0)
    {
        a = p->lval;
    }
    else
    {
        a = r;
    }

    *out = (a << 24) | (*(const int *)&r << 16) | (g << 8) | b;
    return 1;
}

/* A texture by file name: an empty name clears the slot instead of loading. */
// FUNCTION: EZ2AC_UNPACKED 0x004563a0
int readGdsTexture(KTexture **out, KGraphics *g, KParser *p)
{
    char name[0x80];

    name[0] = 0;
    readGdsToken(name, p);

    if (name[0] != 0)
    {
        g->loadTexture(name, out, 0);
        return 1;
    }

    *out = 0;
    return 1;
}

/* The string reader the name/resource helpers share: eat the `=` and the name,
   then take the value only when it really was a string token (5). The token
   buffer is emptied first, so a failed read leaves the caller's buffer as it
   was - which is what lets every caller test out[0] afterwards. */
// FUNCTION: EZ2AC_UNPACKED 0x004561e0
int readGdsToken(char *out, KParser *p)
{
    char *s;

    p->parseToken(2);
    p->parseToken(0xc);
    s = p->tok;
    *s = 0;

    if (p->parseToken(5) != 0)
    {
        strcpy(out, s);
    }

    return 1;
}

/* N comma-separated ints straight into an array. The count is one-based here -
   the comma is eaten between values, not after the last - which is why the
   index starts at 1 and the store goes through a walking pointer. */
// FUNCTION: EZ2AC_UNPACKED 0x00456670
int readGdsIntN(int *out, int count, KParser *p)
{
    int i;

    p->parseToken(2);
    p->parseToken(0xc);

    for (i = 1; i <= count; i++)
    {
        p->lval = 0;

        if (p->parseToken(3) != 0)
        {
            *out = p->lval;
        }

        if (i != count)
        {
            p->parseToken(0xa);
        }

        out++;
    }

    return 1;
}

/* A .str resource by name - the loadStr twin of readGdsTexture. */
// FUNCTION: EZ2AC_UNPACKED 0x00456500
int readGdsStr(Obj402860Head **out, KGraphics *g, KParser *p)
{
    char name[0x80];

    name[0] = 0;
    readGdsToken(name, p);

    if (name[0] != 0)
    {
        g->loadStr(name, out);
        return 1;
    }

    *out = 0;
    return 1;
}

/* A numbered run of textures with the base name read from the parser. */
// FUNCTION: EZ2AC_UNPACKED 0x00456430
int readGdsTexSeries(KTexture **dst, int count, KGraphics *g, KParser *p)
{
    char name[0x80];
    char buf[0x80];
    int i;

    buf[0] = 0;
    name[0] = 0;
    readGdsToken(name, p);

    if (name[0] == 0)
    {
        return 0;
    }

    for (i = 0; i < count; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00498320
        sprintf(buf, "%s%d.bmp", name, i);
        g->loadTexture(buf, &dst[i], 0);
    }

    return 1;
}

/* The same run with the base name handed in rather than parsed. */
// FUNCTION: EZ2AC_UNPACKED 0x00456010
int loadTexSeries(KTexture **dst, int count, KGraphics *g, char *base)
{
    char buf[0x80];
    int i;

    buf[0] = 0;

    if (base != 0 && base[0] != 0)
    {
        for (i = 0; i < count; i++)
        {
            sprintf(buf, "%s%d.bmp", base, i);
            g->loadTexture(buf, &dst[i], 0);
        }

        return 1;
    }

    return 0;
}

/* Store an int under `key`, creating the node if it is not there yet. The store
   is duplicated into both arms - that is what keeps the two tails apart, as the
   original has them. */
// FUNCTION: EZ2AC_UNPACKED 0x00413eb0
int KProfile::setInt(char *key, int value)
{
    KProfileNode *n = find(key);

    if (n == 0)
    {
        n = create(key, 1, 4);
        n->value = value;
        return 1;
    }

    n->value = value;
    return 1;
}

/* Hand out the next slot of the record most recently published at 0x530b74 and
   thread it onto that record's list of live objects. The pool is a bump cursor
   with no free list: `last + objectSize` is the next object, and running past
   the end wraps back to the base with a complaint rather than failing. Every
   read of the record is a fresh load because the stores through `p` may alias
   it - which is exactly what the original does. */
// FUNCTION: EZ2AC_UNPACKED 0x004016a0
void *__stdcall EzPooled::operator new(unsigned int)
{
    DrawNode *p;

    if (g_objectRecord->last == 0)
    {
        p = (DrawNode *)g_objectRecord->poolBase;
    }
    else
    {
        p = (DrawNode *)((char *)g_objectRecord->last + g_objectRecord->objectSize);

        if ((char *)p > g_objectRecord->poolEnd)
        {
            p = (DrawNode *)g_objectRecord->poolBase;
            // STRING: EZ2AC_UNPACKED 0x00487534
            Log("Exceed Pool Size");
        }
    }

    p->flags = 1;
    p->prev = g_objectRecord->last;
    p->next = 0;

    if (p->prev != 0)
    {
        p->prev->next = p;
    }

    g_objectRecord->live++;
    g_objectRecord->last = p;

    if (g_objectRecord->nodes == 0)
    {
        g_objectRecord->nodes = p;
    }

    return p;
}

/* Register one object record with the screen: append it to the screen's own
   list, and to the tick and draw lists if the record asks for them. */
// FUNCTION: EZ2AC_UNPACKED 0x00401850
void Obj401780::addInfo(DrawGroup *g)
{
    if (f10 == 0)
    {
        f10 = g;
    }
    else
    {
        f14->nextOwned = g;
    }

    f14 = g;
    g->nextOwned = 0;

    if ((g->recFlags & 2) != 0)
    {
        if (f18 == 0)
        {
            f18 = g;
        }
        else
        {
            f1c->nextTick = g;
        }

        f1c = g;
        g->nextTick = 0;
    }

    if ((g->recFlags & 4) != 0)
    {
        if (f20 == 0)
        {
            f20 = g;
        }
        else
        {
            f24->nextDraw = g;
        }

        f24 = g;
        g->nextDraw = 0;
    }
}

/* Wipe the whole 4 MB tail of the engine object: two 540000-dword buffers, a
   small one after them, and the pair of 20-entry tables at the very end. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ca20
void Obj40C180::clearBuffers()
{
    int i;

    f4200e8 = 0;
    f4200ec = 1;

    for (i = 0; i < 20; i++)
    {
        arrB[i] = 0;
        arrA[i] = 0;
    }

    memset(events, 0, sizeof(events));
    memset(replay, 0, sizeof(replay));
    memset(channels, 0, sizeof(channels));

    f4200f0 = 0;
    f4200ec = 1;
}

/* Give the input hardware back: unacquire each device before releasing it, and
   release the interface they came from last. Every pointer is re-read from the
   object after the first call because the call may have written it. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c320
void Obj40C180::releaseAll()
{
    if (mouse != 0)
    {
        mouse->Unacquire();
        mouse->Release();
        mouse = 0;
    }

    if (keyboard != 0)
    {
        keyboard->Unacquire();
        keyboard->Release();
        keyboard = 0;
    }

    if (dinput != 0)
    {
        dinput->Release();
        dinput = 0;
    }
}

/* One pass over everything the owner drives, under its lock event: if the gate
   is set, run slot 17 of every live entry. Kept out of src/ksongplayer.cpp
   because Obj410100::update calls this and cl would inline it there. */
// FUNCTION: EZ2AC_UNPACKED 0x0040fe90
void KEZPlayer::tick()
{
    int i;

    WaitForSingleObject(ev0.m_h, INFINITE);

    if (WaitForSingleObject(ev1.m_h, 0) == WAIT_OBJECT_0)
    {
        for (i = 0; i < ((Counted *)f04)->count; i++)
        {
            TickTarget *t = entries[i].obj;

            if (t != 0)
            {
                t->v17();
            }
        }
    }

    SetEvent(ev0.m_h);
}

/* Bit i of an output word. The original indexes a 32-byte .rdata table with the
   channel modulo 8; the values are just 1 << i, so this is the formula rather
   than a copy of the table. */
// GLOBAL: EZ2AC_UNPACKED 0x00488234
extern const unsigned int g_bit[8] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3,
                                       1 << 4, 1 << 5, 1 << 6, 1 << 7 };

/* Resolve the first table's entries down onto the 32 output bits: an entry of
   kind 2 or 3 drives its channel and claims it, so a later entry cannot touch
   it; kinds 0 and 1 release the channel, but only while nothing has claimed it.
   An entry with channel -1 is skipped. Note the claim is recorded ONLY on the
   driving path - releasing does not lock the channel.

   The `int w = c / 8` local is load-bearing: written inline as `flags[c / 8] |=
   ...` cl materializes the word's address with a lea and then works through it,
   where the original folds the index straight into the read-modify-write. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c940
void Obj40C180::applyOutputs()
{
    int claimed[32];
    int i;

    memset(claimed, 0, sizeof(claimed));

    for (i = 0; i < f4 + 1; i++)
    {
        int c = slotsA[i].channel;

        if (c != -1)
        {
            int k = slotsA[i].state;

            if (k == 2 || k == 3)
            {
                if (claimed[c] == 0)
                {
                    int w = c / 8;

                    flags[w] |= g_bit[c % 8];
                    claimed[slotsA[i].channel] = 1;
                }
            }
            else if (k == 0 || k == 1)
            {
                if (claimed[c] == 0)
                {
                    int w = c / 8;

                    flags[w] &= ~g_bit[c % 8];
                }
            }
        }
    }
}

/* Read the .gds's braced section name: token 0xd opens it, the name itself is
   the next string, and token 0xe closes it. A missing opener is not an error the
   caller acts on, but it does stop the read - and returning zero there costs
   nothing, since the failed parseToken left zero in place. */
// FUNCTION: EZ2AC_UNPACKED 0x004561b0
int readGdsSection(char *out, KParser *p)
{
    if (p->parseToken(0xd) == 0)
    {
        return 0;
    }

    p->readGdsString(out);
    p->parseToken(0xe);
    return 1;
}

/* DirectInput's own data, referenced by address only - placeholder storage, the
   real bytes stay in the binary. The first dword of each GUID is what names them
   (0x6f1d2b60 / 0x6f1d2b61 are GUID_SysMouse and GUID_SysKeyboard), and each
   device's format is the one pushed with it. */
// GLOBAL: EZ2AC_UNPACKED 0x004a0130
static char g_guidSysMouse[16];
// GLOBAL: EZ2AC_UNPACKED 0x004a0120
static char g_guidSysKeyboard[16];
// GLOBAL: EZ2AC_UNPACKED 0x004a0108
static char g_dfDIMouse[4];
// GLOBAL: EZ2AC_UNPACKED 0x004a00f0
static char g_dfDIKeyboard[4];

/* Not dllimport: the original reaches DirectInputCreateA through the linker's
   thunk @0x478510, which is what a plain declaration produces. */
extern "C" int __stdcall DirectInputCreateA(HINSTANCE hinst, unsigned int version,
                                            IDirectInputA **out, void *unkOuter);

/* Bring DirectInput up on the game window: create the interface for version 7,
   then a mouse and a keyboard device, each with its data format and a
   non-exclusive foreground cooperative level, and acquire both. Any failure
   anywhere just logs - the caller keeps going without input. The instance handle
   comes from the window rather than being remembered. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c250
void Obj40C180::createInput()
{
    HWND hwnd = g_app->m_hWnd;

    if (DirectInputCreateA((HINSTANCE)GetWindowLongA(hwnd, GWL_HINSTANCE),
                           0x700, &dinput, 0) < 0)
    {
        goto fail;
    }

    if (dinput->CreateDevice(g_guidSysMouse, &mouse, 0) < 0)
    {
        goto fail;
    }

    if (mouse->SetDataFormat(g_dfDIMouse) < 0)
    {
        goto fail;
    }

    if (mouse->SetCooperativeLevel(hwnd, 6) < 0)
    {
        goto fail;
    }

    if (dinput->CreateDevice(g_guidSysKeyboard, &keyboard, 0) < 0)
    {
        goto fail;
    }

    if (keyboard->SetDataFormat(g_dfDIKeyboard) < 0)
    {
        goto fail;
    }

    if (keyboard->SetCooperativeLevel(hwnd, 6) < 0)
    {
        goto fail;
    }

    keyboard->Acquire();
    mouse->Acquire();
    return;

fail:
    // STRING: EZ2AC_UNPACKED 0x00488274
    Log("DirectInput Initialize Error");
}

/* Poll the keyboard. A device that has been lost has to be re-acquired before it
   will answer, and re-acquiring can itself report the loss, so both are retried
   until one of them says something else.

   The loop is a DO-WHILE whose continuation test is the re-acquire, with the
   poll's own result breaking out of the middle. Written as a while or a for(;;)
   cl peels the first iteration ahead of the loop; the original has one aligned
   loop head, so the poll cannot be the loop's condition. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c390
void Obj40C180::m40c390()
{
    do
    {
        if (keyboard->GetDeviceState(0x100, keys) != 0x8007001e)
        {
            break;
        }
    }
    while (keyboard->Acquire() == 0x8007001e);
}

/* Poll the mouse, the same way, and then take the cursor position in the game
   window's own coordinates - which is what the game actually uses; the device
   state only contributes its four button bytes. A failed re-acquire skips the
   position entirely. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c430
void Obj40C180::m40c430()
{
    POINT pt;
    int hr;

    do
    {
        if (mouse->GetDeviceState(0x10, &mouseX) != 0x8007001e)
        {
            break;
        }

        hr = mouse->Acquire();

        if (hr < 0)
        {
            return;
        }
    }
    while (hr == 0x8007001e);

    GetCursorPos(&pt);
    ScreenToClient(g_app->m_hWnd, &pt);
    cursorX = pt.x;
    cursorY = pt.y;
}

/* tick's source-0 handler: the entry watches a VIRTUAL KEY rather than a device
   byte, so its press state comes from GetAsyncKeyState. The four-state machine is
   the same one tick runs for source 1. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c3d0
void Obj40C180::m40c3d0(IoSlot *e, unsigned int now)
{
    if ((GetAsyncKeyState(e->bit) & 0x8000) != 0)
    {
        if (e->state == 0 || e->state == 1)
        {
            e->state = 2;
            e->since = now;
        }
        else
        {
            e->state = 3;
        }
    }
    else
    {
        if (e->state == 2 || e->state == 3)
        {
            e->state = 1;
        }
        else
        {
            e->state = 0;
        }
    }
}

/* tick's source-3 handler: this entry watches a single BIT rather than a whole
   byte, and it indexes the I/O board's own three bytes (the array from +4 on,
   i.e. past the mouse's four) with a shift and a mask rather than a divide - the
   original's `sar 3` / `and 7`, not applyOutputs' signed / and %. The state
   machine is the same one again. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c6f0
void Obj40C180::m40c6f0(IoSlot *e, unsigned int now)
{
    if ((keyState[(e->bit >> 3) + 4] & g_bit[e->bit & 7]) != 0)
    {
        if (e->state == 0 || e->state == 1)
        {
            e->state = 2;
            e->since = now;
        }
        else
        {
            e->state = 3;
        }
    }
    else
    {
        if (e->state == 2 || e->state == 3)
        {
            e->state = 1;
        }
        else
        {
            e->state = 0;
        }
    }
}

/* Which recording channel follows this entry, or -1. A real helper: the caller's
   `cmp n,-1` is dead once it is inlined, which is what tells us it was one. */
static int findChannel(Obj40C180 *o, int key)
{
    int count = o->f4200e8;
    int n;

    for (n = 0; n < count; n++)
    {
        if (o->channels[n].key == key)
        {
            return n;
        }
    }

    return -1;
}

/* Record one state change, if any channel is following this entry. Only the two
   PRESSED states are recorded - a release leaves no event - and the time is
   relative to the run's base at +0x4200f0. */
// FUNCTION: EZ2AC_UNPACKED 0x0040ca90
void Obj40C180::m40ca90(int i, char state, unsigned int now)
{
    int n = findChannel(this, i);

    if (n == -1)
    {
        return;
    }

    if (state == 2 || state == 1)
    {
        events[n][arrA[n]].time = now - f4200f0;
        events[n][arrA[n]].state = state;
        arrA[n]++;
    }
}

/* Advance the replay. For each of the twenty channels, walk the events recorded
   for it and apply every one whose time has come to that channel's slot in the
   second table: a press (2) becomes a hold (3) and stamps the time, a hold keeps
   the slot held, and a release (1) first turns the PREVIOUS event's hold back
   into a press. The `consumed` byte makes a press or a release fire once, and
   firing one ends that channel for this tick; a hold does not.

   The previous-event pointer starts null and is dereferenced by the release path
   without a check - the original does that too, so a channel whose first event is
   a release would fault. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cb20
void Obj40C180::m40cb20(unsigned int now)
{
    unsigned int delta = now - f4200f0;
    int ch;

    for (ch = 0; ch < 20; ch++)
    {
        IoEvent *prev = 0;
        IoEvent *e = replay[ch];
        int i;

        for (i = 0; i < arrB[ch]; i++)
        {
            if (e->time <= delta)
            {
                if (e->state == 2)
                {
                    if (e->consumed == 0)
                    {
                        e->state = 3;
                        e->consumed = 1;

                        {
                            /* One load of the slot index feeds both stores here
                               - the only arm with two - and it has to happen
                               AFTER the two stores above. */
                            int s = channels[ch].slot;

                            slotsB[s].state = 2;
                            slotsB[s].since = now;
                        }

                        break;
                    }
                }
                else if (e->state == 3)
                {
                    slotsB[channels[ch].slot].state = 3;
                }
                else if (e->state == 1)
                {
                    if (prev->state == 3)
                    {
                        prev->state = 2;
                    }

                    if (e->consumed == 0)
                    {
                        e->consumed = 1;
                        slotsB[channels[ch].slot].state = 1;
                        break;
                    }
                }
                else
                {
                    slotsB[channels[ch].slot].state = 0;
                }
            }

            prev = e;
            e++;
        }
    }
}

/* The pool's deallocator, and the exact counterpart of operator new above:
   unlink the object from its record's live list and drop the record's count.
   __stdcall for the same reason - it ends `ret 4`, so the caller pushes and
   does not clean up.

   The object's own vtable slot 2 hands back the record (it is getObjectInfo on
   every class in the pooled family), which is why this needs no argument for
   it. Modelled through a local view rather than DrawNode's own slot 2, whose
   declared return type is shared with other callers.

   `live--` really is written into BOTH arms of the second test; the original
   has two separate decrements, not one after the join. */
struct EzPooledNode
{
    virtual void       q0();
    virtual void       q1(int flag);
    virtual DrawGroup *getGroup();     /* slot 2 -> +0x08 */
};

// FUNCTION: EZ2AC_UNPACKED 0x00401720
void __stdcall EzObject::operator delete(void *p)
{
    DrawNode *n = (DrawNode *)p;
    DrawNode *prev;
    DrawNode *next;

    if (n == 0)
    {
        return;
    }

    prev = n->prev;
    next = n->next;

    if ((n->flags & 1) != 0)
    {
        DrawGroup *g;

        n->flags = 0;
        g = ((EzPooledNode *)p)->getGroup();

        if (prev == 0)
        {
            g->nodes = next;
        }
        else
        {
            prev->next = next;
        }

        if (next == 0)
        {
            g->live--;
            g->last = prev;
        }
        else
        {
            next->prev = prev;
            g->live--;
        }
    }
}

/* loadSeries @0x4560c0 - the third member of the family above, and the one
   ShowCredit's constructor uses. Same loop and the same "%s%d.bmp" (already
   annotated at its first use, so no second marker), differing only in what it
   checks and what it takes:

     - it does NOT null-check `base`, only its first byte, where
       loadTexSeries checks both;
     - it does not clear the buffer first, which is why its frame is the
       0x80 buffer plus the cookie and nothing else;
     - the arguments are (dst, base, count, g) rather than
       (dst, count, g, base), and dst is the sprite array ShowCredit keeps.
*/
// FUNCTION: EZ2AC_UNPACKED 0x004560c0
int loadSeries(Sprite **dst, char *base, int count, KGraphics *g)
{
    char buf[0x80];
    int  i;

    /* The early return is written FIRST, unlike loadTexSeries above: the
       original lays the `return 0` inline right after the test and the loop
       after it, which is the opposite arrangement. */
    if (base[0] == 0)
        return 0;

    for (i = 0; i < count; i++)
    {
        sprintf(buf, "%s%d.bmp", base, i);
        g->loadTexture(buf, (KTexture **)&dst[i], 0);
    }

    return 1;
}
