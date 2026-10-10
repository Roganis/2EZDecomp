#include <windows.h>
#include <string.h>
#include "kcrt.h"
#include "kfile.h"
#include "icom.h"
#include "kprofile.h"
#include "ez2acapp.h"  /* KTextureNode, KGraphics */
#include "loader.h"   /* Log */

/* The VTABLE marker must live in a scanned src/ file (reccmp source_root=src),
   so it is pinned here rather than in the header where the struct is defined. */
// VTABLE: EZ2AC_UNPACKED 0x00488b68
struct KProfile;

/* 0x413e90 is the compiler-generated scalar deleting destructor (calls the
   out-of-line ~KProfile below then conditionally frees via kcrt.h's inline
   operator delete). Target it by name; the real ~KProfile @0x413e20 is the
   definition further down. This TU sorts before stubs.cpp so its ??_G COMDAT
   wins (same device as KSong). */
// FUNCTION: EZ2AC_UNPACKED 0x00413e90
// KProfile::`scalar deleting destructor'

/* Real (non-deleting) destructor: installs the vtable, logs, and the compiler
   emits the base-dtor call + EH frame because KResourceContainer's dtor is
   non-trivial. Log must stay a throwing `void Log(const char*,...)` or the EH
   prologue vanishes. */
/* Constructor: base ctor (auto), log, install vtable (auto), stash the arena
   size at the base's f8, then init() allocates the arena. The 2nd param is
   unused by the ctor body. */
// FUNCTION: EZ2AC_UNPACKED 0x00413dc0
KProfile::KProfile(int size, void * /*p*/)
{
    // STRING: EZ2AC_UNPACKED 0x00488b70
    Log("New KProfile");
    f8 = size;
    init();
}

/* Allocate and zero the key-pool arena (VirtualAlloc size = f8), publish it at
   +0x10 and clear the tree root. Called by the ctor. */
// FUNCTION: EZ2AC_UNPACKED 0x004143e0
void KProfile::init()
{
    f4 = 0;
    if (arena == 0)
    {
        // STRING: EZ2AC_UNPACKED 0x00488c94
        Log("Create %s", getName());
        arena = VirtualAlloc(0, f8, MEM_COMMIT, PAGE_READWRITE);
        if (arena == 0)
        {
            // STRING: EZ2AC_UNPACKED 0x00488c30
            Log("Memory error : %s : KResourceContainer::ResetSearchTree() - m_pKeyPool = NULL, m_nKeyPoolSize = %d", getName(), f8);
        }
    }
    memset(arena, 0, f8);
    pad10 = arena;
    root = 0;
}

/* Tear the whole key tree down, releasing what each node owns. The tree is
   walked with recursion down the left link and iteration along the right one,
   and every node's value is a COM interface (the graphics profile is the
   texture cache): drop references until the object reports none left, then
   clear the slot. Its caller m403f60 lives in another TU on purpose - it
   reloads ecx after this call, which cl only does for a body it cannot see. */
// FUNCTION: EZ2AC_UNPACKED 0x00403f20
void KProfile::freeNodes(KProfileNode *n)
{
    while (n != 0)
    {
        if (n->left != 0)
            freeNodes(n->left);
        if (n->value != 0)
        {
            while (((IDirectDrawSurface7 *)n->value)->Release() != 0)
                ;
            n->value = 0;
        }
        n = n->right;
    }
}

/* The same teardown over the OTHER profile, the texture cache: its nodes carry
   a whole KTexture at the generic node's payload offset, so the release path
   also has to put a lost surface back before dropping it and to take the
   texture out of the owner's running byte/count totals. A node whose f88 is
   set was never counted (it borrowed its surface), so it is only unhooked.
   Returns 1 for a node it emptied - or for a null one, which is what makes the
   caller clear the link it just recursed down. */
/* The drawing half of the texture-cache walk: recurse down the left link,
   draw this node's texture, then loop along the right one. Every cached
   texture is drawn at the origin at its own recorded size, so this is a
   debug overlay of the whole cache. */
// FUNCTION: EZ2AC_UNPACKED 0x00408bf0
void KProfile::drawTree408bf0(KTextureNode *n)
{
    KTexture *t;

    if (n == 0)
        return;

    do
    {
        if (n->left != 0)
            drawTree408bf0(n->left);

        t = &n->tex;
        m_owner->drawTex(t, 0, 0, t->w, t->h, 0);
        n = n->right;
    }
    while (n != 0);
}

// FUNCTION: EZ2AC_UNPACKED 0x00408b40
int KProfile::freeNodes408b40(KTextureNode *n)
{
    int freed;

    if (n == 0)
        return 1;

    freed = 0;
    if (n->left != 0)
    {
        if (freeNodes408b40(n->left) != 0)
            n->left = 0;
    }
    if (n->tex.surface != 0)
    {
        if (n->tex.f88 == 0)
        {
            if (n->tex.surface->IsLost() != 0)
                n->tex.surface->Restore();
            while (n->tex.surface->Release() != 0)
                ;
            m_owner->m_texBytes -= n->tex.bytes;
            m_owner->m_texCount--;
        }
        n->tex.surface = 0;
        n->tex.bytes = 0;
        freed = 1;
    }
    if (n->right != 0)
    {
        if (freeNodes408b40(n->right) != 0)
            n->right = 0;
    }
    return freed;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413e20
KProfile::~KProfile()
{
    // STRING: EZ2AC_UNPACKED 0x00488b80
    Log("Free KProfile");
}

// FUNCTION: EZ2AC_UNPACKED 0x00413e80
const char *KProfile::getName()
{
    // STRING: EZ2AC_UNPACKED 0x00488b90
    return "KProfile";
}

/* Value getters: look the key up in the parsed tree (find, a same-class
   thiscall STUB) and, on a hit, read the +0x50 payload. Write the guard first
   (`if (n == 0) return 0;`) so the early return is the inline fall-through and
   reuses find()'s eax==0 with no `xor`. */
/* Insert-or-find a node for `key` in the tree, allocating from the arena via
   allocNode (kept in a separate TU so cl can't inline it). `target` holds the
   node to fill so the allocNode result flows straight to the tail without a
   re-read. Stores the type at +0x48 and copies the key. */
// FUNCTION: EZ2AC_UNPACKED 0x004144f0
KProfileNode *KProfile::create(char *key, int type, int size)
{
    KProfileNode *node = root;
    KProfileNode *target;
    if (node == 0)
    {
        target = allocNode(size);
        root = target;
    }
    else
    {
        for (;;)
        {
            int cmp = _stricmp(node->key, key);
            if (cmp < 0)
            {
                if (node->left != 0) { node = node->left; continue; }
                target = allocNode(size);
                node->left = target;
                break;
            }
            else if (cmp > 0)
            {
                if (node->right != 0) { node = node->right; continue; }
                target = allocNode(size);
                node->right = target;
                break;
            }
            target = node;
            break;
        }
    }
    target->type = type;
    strcpy(target->key, key);
    return target;
}

/* Case-insensitive binary-tree lookup: walk from root(+0x14) comparing
   _stricmp(node->key, name) — <0 left(+0), >0 right(+4), ==0 hit. The 3-way
   compare reuses one `test eax,eax`'s flags across jge+jle. */
// FUNCTION: EZ2AC_UNPACKED 0x00414460
KProfileNode *KProfile::find(char *name)
{
    KProfileNode *node = root;
    while (node != 0)
    {
        int cmp = _stricmp(node->key, name);
        if (cmp < 0)
            node = node->left;
        else if (cmp > 0)
            node = node->right;
        else
            return node;
    }
    return 0;
}

/* Set a string value: find-or-create the key's node (type 3, a 0xc00-byte
   value slot) and copy the string into the value field at +0x50. */
// FUNCTION: EZ2AC_UNPACKED 0x00413ef0
int KProfile::setStr(char *key, char *value)
{
    KProfileNode *n = find(key);
    if (n == 0)
        n = create(key, 3, 0xc00);
    strcpy((char *)&n->value, value);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413f40
int KProfile::getInt(char *name, int *out)
{
    KProfileNode *n = find(name);
    if (n == 0)
        return 0;
    *out = n->value;
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413f70
int KProfile::getStr(char *name, char *out)
{
    KProfileNode *n = find(name);
    if (n == 0)
        return 0;
    strcpy(out, (char *)&n->value);
    return 1;
}

// FUNCTION: EZ2AC_UNPACKED 0x00413fb0
int KProfile::getValue(char *name, char **out)
{
    KProfileNode *n = find(name);
    if (n == 0)
        return 0;
    *out = (char *)&n->value;
    return 1;
}

/* Write the profile back out: open a KFile for mode 2, walk the tree through
   it, close and release. The file lives in the same +0x18 slot the texture
   cache uses for its owner - it is scratch for the duration of the write, which
   is why writeTree reaches it through the object rather than as an argument.

   The two teardown paths are deliberately asymmetric, and that is what the
   original does: on the open failure the `m_file = 0` sits INSIDE the null
   guard (cl folds `delete`'s own test into it), while on success the delete
   carries its own test and the store follows unconditionally. */
// FUNCTION: EZ2AC_UNPACKED 0x004142b0
int KProfile::writeFile(char *name)
{
    KFile *f = new KFile;

    m_owner = (KGraphics *)f;

    if (f->open(name, 2) == 0)
    {
        if (m_owner != 0)
        {
            delete (KFile *)m_owner;
            m_owner = 0;
        }
        return 0;
    }

    writeTree(root);
    ((KFile *)m_owner)->close();
    delete (KFile *)m_owner;
    m_owner = 0;
    return 1;
}
