#ifndef OBJ414750_H
#define OBJ414750_H

#include <windows.h>
#include "ksongplayer.h"   /* KLock, KSongPlayerBase */

/* The song-player pump: owns up to 64 KSongPlayerBase-derived drivers and
   ticks them, either from its own worker thread or (m_useThread == 0) from
   whoever calls it. No vtable - neither the ctor @0x414750 nor the dtor
   @0x414d30 stores one, and the dtor is invoked as a plain member destructor
   from its owners' compound dtors (e.g. `lea ecx,[esi+0x24]; call 0x414d30`
   at 0x422ef3). No class-name string exists anywhere in the cluster, so this
   keeps the repo's address-derived naming for unidentified classes.

   Layout is proven by the ctor: `rep stosd` of 0x40 dwords over offset 0 (the
   slot array), then count/hThread = 0, priority = 1, sleep = 1, with the four
   KLocks constructed at 0x110..0x11c ahead of them. */
class Obj414750
{
public:
    Obj414750();                                /* 0x414750 */
    ~Obj414750();                               /* 0x414d30 */

    int   stop();                               /* 0x414850 */
    void  stopAll();                            /* 0x4148b0 - a bare jmp to stop, so another TU */
    void  rewindAll();                          /* 0x4148c0 */
    int   restart();                            /* 0x414910 */
    void  suspend();                            /* 0x414930 */
    int   addPlayer(KSongPlayerBase *player);   /* 0x414980 */
    int   removePlayer(KSongPlayerBase *player);/* 0x4149e0 */
    int   removeAll();                          /* 0x414a50 */
    void  pump();                               /* 0x414b70 */
    int   start(DWORD sleep, int useThread);    /* 0x414dc0 */
    DWORD run();                                /* 0x414ab0 (worker body) */

    KSongPlayerBase *m_players[64];  /* 0x000 */
    int    m_count;                  /* 0x100 */
    DWORD  m_sleep;                  /* 0x104 - Sleep() interval per pass */
    int    m_priority;               /* 0x108 - SetThreadPriority value */
    HANDLE m_hThread;                /* 0x10c */
    KLock  m_lock110;                /* 0x110 - KLock(0, 1) */
    KLock  m_lock114;                /* 0x114 - KLock(1, 0) */
    KLock  m_lock118;                /* 0x118 - KLock(1, 1) */
    KLock  m_run;                    /* 0x11c - KLock(1, 0), the run gate */
    int    m_useThread;              /* 0x120 */
};

#endif /* OBJ414750_H */
