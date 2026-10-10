/* KProfile::writeTree @0x414200 - the recursive in-order dump behind
   writeFile: left subtree, then `"key" = ` and the value by type -
   1 int, 2 float (promoted for the vararg), 3 the string inline at
   the value slot - a newline, then the right subtree. The file is the
   KFile parked in m_owner, reached fresh at every printf. In its own
   TU so writeFile keeps the calls. */
#include "kengineapp.h"
#include "kprofile.h"
#include "kfile.h"

// FUNCTION: EZ2AC_UNPACKED 0x00414200
int KProfile::writeTree(KProfileNode *n)
{
    if (n == 0)
        return 0;
    if (n->left != 0)
        writeTree(n->left);
    // STRING: EZ2AC_UNPACKED 0x00488c08
    ((KFile *)m_owner)->printf("\"%s\" = ", n->key);
    {
        int *v = &n->value;

        if (n->type == 1)
            ((KFile *)m_owner)->printf("%d", *v);
        else if (n->type == 2)
            // STRING: EZ2AC_UNPACKED 0x00488c00
            ((KFile *)m_owner)->printf("%f", (double)*(float *)v);
        else if (n->type == 3)
            // STRING: EZ2AC_UNPACKED 0x00488bf8
            ((KFile *)m_owner)->printf("\"%s\"", (char *)v);
    }
    // STRING: EZ2AC_UNPACKED 0x00488bf4
    ((KFile *)m_owner)->printf("\r\n");
    if (n->right != 0)
        writeTree(n->right);
    return 1;
}
