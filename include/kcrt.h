#ifndef KCRT_H
#define KCRT_H

#include <stdlib.h>
#include <new>   /* operator delete is throw() - without it cl assumes a
                     delete can throw and gives ~KSNDMixer an EH frame */

/* The CRT allocator, as the original links it: game code never calls malloc
   or free directly. Every new-expression calls the libcpmt `operator new`
   @0x478528 (newop.obj - match.sh links libcpmt.lib AHEAD of libcmt.lib,
   whose own new.obj is the non-looping _nh_malloc form) (the _callnewh retry loop around malloc @0x47a5e8), every delete
   the libcmt `operator delete` @0x4788ca (a `jmp free`, free @0x47afc4), and
   the array forms are the one-line forwarders `new[]` @0x47854c
   (`jmp 0x478528`) and `delete[]` @0x47927d (`jmp 0x4788ca`). All six are
   LIBRARY-annotated in src/library.cpp, so this header no longer defines
   anything; it stays as the one place that says so.

   Until 2026-10-01 this header defined scalar new/delete INLINE as
   malloc/free, and library.cpp named the operator addresses `malloc` and
   `free`. The call bytes are identical either way (a call is a call), which
   is how that stood; but it named 0x478528/0x4788ca wrongly and left the real
   malloc/free unannotated. */

#endif /* KCRT_H */
