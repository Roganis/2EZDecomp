#ifndef SCOREKEEPER_H
#define SCOREKEEPER_H

/* The round's score bookkeeping. Layout from wip/scorekeeper.cpp, which decoded
   commit @0x472d10; the record stride is confirmed independently by rank()
   @0x472b40 indexing it as `player * 15 * 4` = 0x3c. */

struct ScoreRecord
{
    int   f0;    /* 0x00 */
    int   f4;
    int   f8;    /* 0x08 */
    int   fc;
    int   f10;
    int   f14;
    int   f18;
    int   f1c;
    int   f20;
    int   f24;
    int   f28;
    float f2c;   /* 0x2c - the only float in the record */
    int   f30;   /* 0x30 - the rank commit @0x472b40 hands back */
    int   f34;
    int   f38;
};

/* The object that keeps the round's scores. Only what commit @0x472d10 touches
   is modeled: a block of nine counters at +0x1c8 that the round accumulates
   into, the five per-player records, and three more counters at +0x54c. */
struct ScoreKeeper
{
    char  _pad0[0x1c8];
    int   m_1c8;   /* 0x1c8 */
    int   m_1cc;
    int   m_1d0;
    int   m_1d4;
    int   m_1d8;
    int   m_1dc;
    int   m_1e0;
    int   m_1e4;
    int   m_1e8;
    float m_1ec;   /* 0x1ec */
    char  _pad1f0[0x1f4 - 0x1f0];
    ScoreRecord rec[5];            /* 0x1f4 */
    char  _pad320[0x35c - 0x320];
    int   m_35c;                   /* 0x35c */
    char  _pad360[0x54c - 0x360];
    int   m_54c;                   /* 0x54c */
    int   m_550;
    char  _pad554[0x558 - 0x554];
    int   m_558;                   /* 0x558 */

    int  rank();     /* 0x472b40 (src/scorerank.cpp) */
    void commit();   /* 0x472d10 (src/scorerank.cpp) */
};

#endif /* SCOREKEEPER_H */
