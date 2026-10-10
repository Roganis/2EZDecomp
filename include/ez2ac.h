#ifndef EZ2AC_H
#define EZ2AC_H

/* Reconstructed EZ2AC types. Offsets are hard facts from the binary; class
   identities are provisional until proven by cross-reference. */

/* Class owning the accessor at 0x0040b970. Identity TBD (reads this+0x438). */
struct Obj40B970
{
    char _pad0[0x438];
    int  field_0x438;

    int getField438();   /* __thiscall member -> this in ecx */
};

/* Class owning the setter at 0x0040d0d0. Identity TBD (writes this+0x8, this+0x4). */
struct Obj40D0D0
{
    char _pad0[4];
    int  field_0x4;
    int  field_0x8;

    void setPair(int a, int b);   /* __thiscall */
};

#endif /* EZ2AC_H */
