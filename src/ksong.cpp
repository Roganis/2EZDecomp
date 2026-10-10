#include <string.h>
#include "kcrt.h"
#include "ksong.h"
#include "loader.h"   /* Log */

/* KSong: the per-song object-registry class. Class definition in
   include/ksong.h; the vtable entity is pinned from this TU (markers in
   headers are ignored — source_root is src/). ~KSong lives in
   src/ksongdtor.cpp so its two virtual calls stay dynamic dispatches. */

// VTABLE: EZ2AC_UNPACKED 0x00488874
struct KSong;

/* Find the slot holding `key` (0 = a free slot) in an 8-entry registry; -1
   when absent. Kept a real static helper: its inlined copies keep both
   return-0 tails in the callers (the dead i==-1 two-stage codegen). */
static int findSlot(IReleasable **arr, IReleasable *key)
{
    int i = 0;
    IReleasable **p = arr;
    do
    {
        if (*p == key)
            return i;
        i++;
        p++;
    } while (i < 8);
    return -1;
}

/* KSong's class-registry descriptor (vtable slot 0 returns it; placeholder
   storage, layout is the 16-byte KRuntimeClass/EzClassInfo shape). */
// GLOBAL: EZ2AC_UNPACKED 0x004a734c
static int g_4a734c;

// FUNCTION: EZ2AC_UNPACKED 0x004104c0
void *KSong::getClassDesc() { return &g_4a734c; }

/* 0x410900 is the compiler-generated scalar deleting destructor (vtable slot
   1: call ~KSong @0x410840 — src/ksongdtor.cpp — then conditional operator
   delete, inlined to a direct `call free` via kcrt.h). Emitted in this TU
   because the ctor below forces the vftable; target it by name. */
// FUNCTION: EZ2AC_UNPACKED 0x00410900
// KSong::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x004107f0
KSong::KSong()
{
    count1 = 0;
    memset(arr1, 0, sizeof(arr1));
    count2 = 0;
    memset(arr2, 0, sizeof(arr2));
}

// FUNCTION: EZ2AC_UNPACKED 0x004104d0
int KSong::v2()
{
    count1 = 0;
    memset(arr1, 0, sizeof(arr1));
    count2 = 0;
    memset(arr2, 0, sizeof(arr2));
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00410520
int KSong::v3()
{
    v6();
    return v9();
}

/* Create an engine object by class name and store it in registry 1. The
   `int i = -1` + guarded search is load-bearing: it keeps i live across the
   Log call (esi saved in the prologue) and jump-threads the count-full path
   into the shared return-0 tail. The `ok` flag is equally load-bearing: it
   lays the release arm inline and the success block after the return-0. */
// FUNCTION: EZ2AC_UNPACKED 0x00410530
IReleasable *KSong::v4(char *name, KRuntimeClass *cls, void *c)
{
    int i = -1;

    // STRING: EZ2AC_UNPACKED 0x00488864
    Log("CreateSong : %s", name);
    if (count1 < 8)
        i = findSlot(arr1, 0);
    if (i == -1)
        return 0;
    IReleasable *obj = (IReleasable *)cls->createObject();
    if (obj == 0)
        return 0;
    int ok = obj->v2(c) != 0 && obj->v4(name) != 0;
    if (ok == 0)
    {
        obj->v1(1);
        return 0;
    }
    arr1[i] = obj;
    count1++;
    return obj;
}

// FUNCTION: EZ2AC_UNPACKED 0x004105d0
int KSong::release1(IReleasable *key)
{
    if (key == 0)
        return 0;

    int i = findSlot(arr1, key);
    if (i == -1)
        return 0;

    key->v1(1);
    arr1[i] = 0;
    count1--;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00410630
int KSong::v6()
{
    for (int i = 0; i < 8; i++)
    {
        IReleasable *o = arr1[i];
        if (o != 0)
            o->v1(1);
    }
    count1 = 0;
    memset(arr1, 0, sizeof(arr1));
    return 1;
}

/* Registry-2 creator: like v4 but keyed off a source object whose slot-3
   value seeds the new object's slot 3. */
// FUNCTION: EZ2AC_UNPACKED 0x00410680
IReleasable *KSong::v7(ISongSource *src, KRuntimeClass *cls, void *c)
{
    int i = -1;

    if (src == 0 || cls == 0)
        return 0;
    if (count2 < 8)
        i = findSlot(arr2, 0);
    if (i == -1)
        return 0;
    IReleasable *obj = (IReleasable *)cls->createObject();
    if (obj == 0)
        return 0;
    int ok = obj->v3(src->s3()) != 0 && obj->v2(src) != 0 && obj->v4(c) != 0;
    if (ok == 0)
    {
        obj->v1(1);
        return 0;
    }
    arr2[i] = obj;
    count2++;
    return obj;
}

// FUNCTION: EZ2AC_UNPACKED 0x00410740
int KSong::release2(IReleasable *key)
{
    if (key == 0)
        return 0;

    int i = findSlot(arr2, key);
    if (i == -1)
        return 0;

    key->v1(1);
    arr2[i] = 0;
    count2--;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004107a0
int KSong::v9()
{
    for (int i = 0; i < 8; i++)
    {
        IReleasable *o = arr2[i];
        if (o != 0)
            o->v1(1);
    }
    count2 = 0;
    memset(arr2, 0, sizeof(arr2));
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x004108a0
KSong *createKSong()
{
    return new KSong;
}
