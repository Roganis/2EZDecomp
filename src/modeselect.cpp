/* ModeSelectDirector - the mode-select screen's director (see
   include/modeselect.h for the layout its destructor writes down).

   Only the teardown path so far. It is the same shape as
   ~SongSelectDirector @0x431090 with two extra graphics calls in front: put the
   world transform back to the identity and push it to the device, then reset the
   camera, drop the cabinet's outputs and log one line. */

#include "modeselect.h"
#include "ez2acapp.h"
#include "bigbase.h"
#include "players.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */

/* Two already-matched methods of the graphics singleton, each modeled under its
   own placeholder class name (src/accessors.cpp @0x40bbb0 reloads the identity
   world matrix into +0x140, src/com_wrappers.cpp @0x40bb00 hands it to the
   device as D3DTRANSFORMSTATE_WORLD). Both are almost certainly KGraphics; they
   are declared here rather than retyped there so the already-matched bodies'
   addressing is not perturbed. */
struct Obj40BBB0 { void loadIdentity(); };
struct Obj40BB00 { void doSub(); };

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classModeSelectDirector;  /* 0x4ac748 */
extern EzObjectInfo  g_info_4ac758;            /* 0x4ac758 */

// FUNCTION: EZ2AC_UNPACKED 0x00448f50
KRuntimeClass *ModeSelectDirector::getRuntimeClass()
{
    return &classModeSelectDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00448f60
DrawGroup *ModeSelectDirector::getObjectInfo()
{
    return &g_info_4ac758;
}

// FUNCTION: EZ2AC_UNPACKED 0x00448f70
ModeSelectDirector::~ModeSelectDirector()
{
    GVec3 eye;
    GVec3 at;
    GVec3 up;

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    ((Obj40BBB0 *)g_sub1b2f054)->loadIdentity();
    ((Obj40BB00 *)g_sub1b2f054)->doSub();
    ((KGraphics *)g_sub1b2f054)->setCamera(&eye, &at, &up);
    ((KGraphics *)g_sub1b2f054)->setProjection(1.0f, 10000.0f, 0.785398185f);
    g_bigInstance->setFlags(0);
    // STRING: EZ2AC_UNPACKED 0x00493fe8
    Log("ModeSelectDirector::Delete");
}

// FUNCTION: EZ2AC_UNPACKED 0x0044c670
// ModeSelectDirector::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`, ICF-folded across every class
   that has one; src/constants.cpp owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int ModeSelectDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00494004
struct ModeSelectDirector;

/* The panel's button map, terminated by -1: the three banks the mode-select and
   title screens scan. Two identical copies exist in the original, one per
   screen - the linker did not fold them because each is its own array. */
// GLOBAL: EZ2AC_UNPACKED 0x00493f98
const int g_modeSelectKeys[15] =
{
    10, 11, 12, 13, 14,
    18, 19, 20, 21, 22,
    6, 7, 8, 9,
    -1
};

extern ListNode *addToListA(int id);   /* 0x418260 (src/nodelists.cpp) */

/* Claim a player slot for `id`, but only if the input board has an event
   pending: update2 @0x44c860 calls this twice, once per side, and follows
   each call by clearing the same f18 it clears here. Nothing in the tree
   referenced it - the owner was found from those two call sites, which pass
   `this` in ebp. */
// FUNCTION: EZ2AC_UNPACKED 0x0044b290
void ModeSelectDirector::m44b290(int id)
{
    if (g_bigInstance->consume() != 0)
    {
        addToListA(id);
        f18 = 0;
    }
}

/* Walk the button map and, for every button the panel reports as held, run the
   two-step latch and log it. The loop reads its NEXT key at the bottom, so the
   first one is a constant cl folds out of the table. */
// FUNCTION: EZ2AC_UNPACKED 0x0044c7f0
void ModeSelectDirector::clearCommand()
{
    int i;
    int key;

    fe8 = 1;

    for (i = 0; (key = g_modeSelectKeys[i]) != -1; i++)
    {
        if (g_bigInstance->getState(key) == 2)
        {
            m44c690(0, key);
            m44c690(1, key);
            // STRING: EZ2AC_UNPACKED 0x00494548
            Log("Check Command = %d", key);
        }
    }

    cmdLen[0] = 0;
}

/* ModeSelectDirector's new-plus-ctor factory: 0x9630 bytes out of its own
   record, which is what audits the class's size. */
// FUNCTION: EZ2AC_UNPACKED 0x0044c790
void *createModeSelectDirector()
{
    g_objectRecord = &g_info_4ac758;
    return new ModeSelectDirector;
}
