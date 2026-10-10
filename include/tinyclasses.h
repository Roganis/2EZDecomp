#ifndef TINYCLASSES_H
#define TINYCLASSES_H

/* Three classes with the same minimal shape: a single-slot vtable holding
   only the scalar deleting destructor, an empty destructor body and no data
   members of their own that any of their three functions touch. cl inlines
   the trivial ~T into ??_G, which is why the deleting destructor restores
   the vptr itself instead of calling ??1.

   No name string exists for any of them, so they keep the repo's
   address-derived naming (after the constructor). */

/* vtable @0x4911c8 */
struct Obj42DE30
{
    Obj42DE30();               /* 0x42de30 */
    virtual ~Obj42DE30();      /* 0x42de40 (deleting ??_G 0x42f060) */
};

/* vtable @0x49a680 */
struct Obj4659D0
{
    Obj4659D0();               /* 0x4659d0 */
    virtual ~Obj4659D0();      /* 0x4659e0 (deleting ??_G 0x4666e0) */
};

/* vtable @0x49e428 */
struct Obj472B20
{
    Obj472B20();               /* 0x472b20 */
    virtual ~Obj472B20();      /* 0x472b30 (deleting ??_G 0x4733e0) */
};

#endif /* TINYCLASSES_H */
