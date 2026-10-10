#include <windows.h>
#include <string.h>
#include <conio.h>
#include "bigbase.h"
#include "kprofile.h"
#include "kengineapp.h"   /* g_iniProfile */

/* Two more pieces of coin state the reset clears; nothing else reconstructed
   reads them yet. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2efac
int g_1b2efac;
// GLOBAL: EZ2AC_UNPACKED 0x01b2ef18
int g_1b2ef18;

/* Obj40C180::shutdown @0x40cc80 - two chained thiscalls with NO `this` saved
   anywhere, which is only possible because cl can SEE the first callee's body and
   therefore knows it does not clobber ecx. Reproducing that needs resetFlags
   defined in THIS TU (a stub body, marked __declspec(noinline) to stand in for
   the real function's size - the original's 0x40c8f0 is simply too big for cl's
   inliner). The tail-jump target releaseAll must stay OUT of this TU, or cl
   inlines it and the `jmp` disappears. */

/* Clear the four flag words and push them straight back out to the I/O board
   (ports 0x100..0x103), so the cabinet's lamps go dark with them. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c8f0
void Obj40C180::resetFlags()
{
    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = 0;

    if (f4200f4 != 0)
    {
        _outp(0x100, (unsigned char)flags[0]);
        _outp(0x101, (unsigned char)flags[1]);
        _outp(0x102, (unsigned char)flags[2]);
        _outp(0x103, (unsigned char)flags[3]);
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0040cc80
void Obj40C180::shutdown()
{
    resetFlags();
    releaseAll();
}

/* The abstract base's destructor - shutdown()'s body under a vptr restore,
   and for the same reason in this TU (resetFlags visible, releaseAll not).
   Non-virtual: the vftable has no slot for it, and its only caller is the
   unreferenced ~CabinetInput below. */
// FUNCTION: EZ2AC_UNPACKED 0x0040cdc0
Obj40C180::~Obj40C180()
{
    resetFlags();
    releaseAll();
}

/* init @0x40cc30 - shutdown's counterpart, called by EZ2AC::v2 @0x415cb0: run
   slot 0, clear f4 and the two entry tables, bring up the three subsystems and
   tail-jump slot 1. Same ecx-preservation shape as shutdown, so resetInputState
   also has to be defined HERE (see the note above) - and with a non-empty body,
   because cl deletes a call to a side-effect-free empty function even when it is
   marked noinline. */

/* Raise one bit of one output word. Taking the bit as a PARAMETER is what the
   original's codegen shows: cl materializes the constant in a register and, once
   the OR has come through an inlined parameter, no longer merges the two writes
   to one word into a single `|= 0xc0` (nor folds them into the store that just
   cleared it). Written as four plain `flags[i] |= 0x40` statements it collapses
   to two stores of 0xc0. */
static void setBit(int *w, int bit)
{
    *w |= bit;
}

/* Take the cabinet as it stands right now to be the rest state: latch both
   button ports into the current AND the previous copy so nothing reads as an
   edge, drop both encoders' movement history, point the event cursor at the board's own
   counter so nothing is pending, and light the two lamps bits 6 and 7 select. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c820
void Obj40C180::resetInputState()
{
    int a;
    int b;
    int c;

    if (f4200f4 == 0)
    {
        return;
    }

    a = _inp(0x103);
    b = _inp(0x104);
    c = _inp(0x105);

    port103b = a;
    port104b = b;
    port103 = a;
    port104 = b;

    hist[0][4] = 0;
    hist[0][3] = 0;
    hist[0][2] = 0;
    hist[0][1] = 0;
    hist[0][0] = 0;
    hist[1][4] = 0;
    hist[1][3] = 0;
    hist[1][2] = 0;
    hist[1][1] = 0;
    hist[1][0] = 0;

    tail = c;
    head = c;

    flags[0] = 0;
    flags[1] = 0;
    flags[2] = 0;
    flags[3] = 0;

    setBit(&flags[2], 0x40);
    setBit(&flags[2], 0x80);
    setBit(&flags[3], 0x40);
    setBit(&flags[3], 0x80);
}

// FUNCTION: EZ2AC_UNPACKED 0x0040cc30
void Obj40C180::init()
{
    resetCoins();

    f4 = 0;
    memset(slotsA, 0, sizeof(slotsA));
    memset(slotsB, 0, sizeof(slotsB));

    clearBuffers();
    createInput();
    resetInputState();

    bindButtons();
}

/* Hook the five slots 0x12..0x16 up to output channels 0x18..0x1c, or unhook
   them by handing each the "no output" channel. Each pick between two constants
   goes branchless (neg/sbb/and/add), which is the ternary's own shape - written
   as two calls cl would cross-jump them into one instead. setChannel has to
   stay in src/accessors.cpp or cl inlines the single store and the five calls
   the original makes disappear. */
// FUNCTION: EZ2AC_UNPACKED 0x00418c10
void Obj40C180::routeOutputs(int on)
{
    setChannel(0x12, on ? 0x18 : -1);
    setChannel(0x13, on ? 0x19 : -1);
    setChannel(0x14, on ? 0x1a : -1);
    setChannel(0x15, on ? 0x1b : -1);
    setChannel(0x16, on ? 0x1c : -1);
}

/* The same routing over the LOWER bank - channels 0xa..0xe onto outputs
   0x10..0x14, where routeOutputs above maps 0x12..0x16 onto 0x18..0x1c.
   Found by scratchpad/twins.py at 80%; the two differ in nothing but the
   ten constants. */
// FUNCTION: EZ2AC_UNPACKED 0x00418b90
void Obj40C180::m418b90(int on)
{
    setChannel(0xa, on ? 0x10 : -1);
    setChannel(0xb, on ? 0x11 : -1);
    setChannel(0xc, on ? 0x12 : -1);
    setChannel(0xd, on ? 0x13 : -1);
    setChannel(0xe, on ? 0x14 : -1);
}

/* The 32 output bits live in four words, so a channel is a word index and a bit
   within it - both by SIGNED division, which is what the cdq/and/add/sar and the
   0x80000007 dance are. The bit itself comes out of a table rather than a shift.

   Writing the table read out at BOTH use sites is load-bearing: cl common-
   subexpression-eliminates the two reads into one load above the branch (which
   is what the original has) AND gives the result a fresh register, edx. Hoisting
   it into a named local instead makes cl reuse the dying index register esi, and
   nothing else about the function changes. Declaring the word index first is
   equally load-bearing - declaration order is what decides whether the division
   or the modulo is emitted first. */
/* Defined in src/engine.cpp, where applyOutputs @0x40c940 uses it too. */
extern const unsigned int g_bit[8];

// FUNCTION: EZ2AC_UNPACKED 0x0040c780
void Obj40C180::setOutput(int channel, int on)
{
    int w = channel / 8, b = channel % 8;

    if (on)
        flags[w] |= g_bit[b];
    else
        flags[w] &= ~g_bit[b];
}

/* WINMM's timeGetTime through the import thunk, and the saturating subtract in
   src/misc.cpp - both really called, so both must stay in other TUs. */
extern "C" DWORD __stdcall timeGetTime(void);
extern unsigned int SatSub(unsigned int a, unsigned int b);

/* How long slot i has been in the state it is in: now minus the timestamp the
   press tracker wrote when it entered state 2, saturating at zero. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c1d0
unsigned int Obj40C180::heldFor(int i)
{
    return SatSub(timeGetTime(), slotsA[i].since);
}

/* The default button map, straight off the cabinet's harness: 26 slots, all of
   them unrouted (channel -1). Source 3 is the I/O board's own handler and
   covers keyState bits 0x00..0x17 in order; the two exceptions are slot 0 and
   slot 3, which watch bits 0x1b and 0x72 through source 0. */
// FUNCTION: EZ2AC_UNPACKED 0x00418c90
void CabinetInput::bindButtons()
{
    if (f4200f4 != 0)
    {
    addButton(0x00, 0, 0x1b, -1);
    addButton(0x01, 3, 0x07, -1);
    addButton(0x02, 3, 0x06, -1);
    addButton(0x03, 0, 0x72, -1);
    addButton(0x04, 3, 0x00, -1);
    addButton(0x05, 3, 0x01, -1);
    addButton(0x06, 3, 0x02, -1);
    addButton(0x07, 3, 0x03, -1);
    addButton(0x08, 3, 0x04, -1);
    addButton(0x09, 3, 0x05, -1);
    addButton(0x0a, 3, 0x08, -1);
    addButton(0x0b, 3, 0x09, -1);
    addButton(0x0c, 3, 0x0a, -1);
    addButton(0x0d, 3, 0x0b, -1);
    addButton(0x0e, 3, 0x0c, -1);
    addButton(0x0f, 3, 0x0d, -1);
    addButton(0x10, 3, 0x0e, -1);
    addButton(0x11, 3, 0x0f, -1);
    addButton(0x12, 3, 0x10, -1);
    addButton(0x13, 3, 0x11, -1);
    addButton(0x14, 3, 0x12, -1);
    addButton(0x15, 3, 0x13, -1);
    addButton(0x16, 3, 0x14, -1);
    addButton(0x17, 3, 0x15, -1);
    addButton(0x18, 3, 0x16, -1);
    addButton(0x19, 3, 0x17, -1);
    }
}

/* The abstract base's own constructor: its vftable (three _purecall slots) and
   the seven fields nothing else initialises - the tick counter, the three
   DirectInput pointers, the no-board gate and the event ring's two ends. */
// FUNCTION: EZ2AC_UNPACKED 0x0040c180
Obj40C180::Obj40C180()
{
    f4 = 0;
    dinput = 0;
    keyboard = 0;
    mouse = 0;
    f4200f4 = 0;
    head = 0;
    tail = 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00488254
struct Obj40C180;

/* CabinetInput's constructor, destructor and vftable 0x48cf50 are in
   src/cabinetinput.cpp, away from the base bodies they must not inline. */

/* Put the coin state back and re-read how many coins a credit costs. The
   no-board gate goes to 1 first, which is what lets the rest of the class run
   without an I/O board present. */
// FUNCTION: EZ2AC_UNPACKED 0x00418700
void CabinetInput::resetCoins()
{
    f4200f4 = 1;
    have = 0;
    g_1b2efac = 0;
    g_1b2ef18 = 0;
    f42028c = 0;
    need = 0;
    f42029c = 0;
    testMode1 = 1;
    testMode2 = 1;

    // STRING: EZ2AC_UNPACKED 0x0048d848
    g_iniProfile->getInt("CreditCoins", &need);
}
