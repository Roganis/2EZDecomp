#include "kprofile.h"
void Log(const char *fmt, ...);

/* Bump-allocate a node (0x50 header + `size` value bytes) from the arena.
   Kept in its own TU so KProfile::create emits a real call rather than
   inlining it. Advances the bump pointer at +0x10 and bounds-checks it
   against arena(+0xc)+f8(+0x8); on overflow logs and returns 0. */
// FUNCTION: EZ2AC_UNPACKED 0x004144a0
KProfileNode *KProfile::allocNode(int size)
{
    KProfileNode *node = (KProfileNode *)pad10;
    node->f4c = size;
    node->left = 0;
    node->right = 0;
    char *end = (char *)arena + f8;
    pad10 = (char *)pad10 + size + 0x50;
    if ((char *)pad10 >= end)
    {
        // STRING: EZ2AC_UNPACKED 0x00488ca0
        Log("KResourceContainer: Exceed pool size-> Bounding Error: %s", getName());
        return 0;
    }
    f4++;
    return node;
}
