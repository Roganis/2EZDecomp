#include "kcrt.h"
#include "ksong.h"

/* ~KSong lives in its own TU on purpose: the original's two virtual calls
   are genuine dynamic dispatches — the first through the just-stored vtable
   (`call [0x488874+0x18]`, the slot-address form), the second through a
   reloaded vptr (`call [eax+0x24]`). Writing `v6(); v9();` devirtualizes
   both to direct calls in any TU; calling through a vtable-shape mirror of
   the class defeats the devirtualizer while the stored-vtable constant
   still folds into the first call's operand. */
struct IKSongMirror
{
    virtual void q0();
    virtual void q1();
    virtual void q2();
    virtual void q3();
    virtual void q4();
    virtual void q5();
    virtual int q6();
    virtual void q7();
    virtual void q8();
    virtual int q9();
};

// FUNCTION: EZ2AC_UNPACKED 0x00410840
KSong::~KSong()
{
    IKSongMirror *p = (IKSongMirror *)this;
    p->q6();
    p->q9();
}
