/* Accumulation loops over game arrays. */

/* Global element count (DAT_01b2e718). The GLOBAL marker maps this symbol to the
   original's address so reccmp equates the data reference. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e718
int g_elemCount;

/* Sum @0x00466660 (__thiscall): total += elem[i].a + elem[i].b for i in
   [0, g_elemCount), elements at this+0x80 with stride 0x40. */
struct Elem40 { int a; int b; char _pad[0x40 - 8]; };
struct Obj466660
{
    char   _pad[0x80];
    Elem40 elems[1];   /* 0x80 */
    int sumElems();
};

// FUNCTION: EZ2AC_UNPACKED 0x00466660
int Obj466660::sumElems()
{
    int n = g_elemCount;
    int total = 0;
    if (n > 0)
    {
        /* cl anchors the induction pointer at the .b field (this+0x84), reading
           .a via [-4] and .b via [0]; walk 0x40 bytes (16 ints) per element. */
        int *p = &elems[0].b;
        do
        {
            total += p[-1] + p[0];
            p += 16;
        } while (--n);
    }
    return total;
}
