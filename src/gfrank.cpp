/* PlayerRecord::m465b30 @0x465b30 - the GF grade ladder, and the twin of
   PlayerSlot::m42e190 already in src/keysrank.cpp. Integrated at 71.79%.

   65 instructions against 65. The head, both ladders' compares, every float
   constant, the register and xmm assignment, the published percentage and the
   shared `>= 50 -> 1 / else 0` tail are byte-identical on the first build. The
   only defect is which arm owns the six `mov eax,K; ret` blocks that both
   ladders end their middle steps with - the original lays the CV2 arm first
   and gives it those blocks, cl does the reverse. That is the same cross-jump
   coin that parks the keys twin, and this is its second data point. */

#include "players.h"

extern int g_1b2eb6c;    /* src/ez2acapp.cpp - 1 = CV2Mix */
extern int g_elemCount;  /* 0x1b2e718 - the player index this run is for */

/* Written here and read nowhere in the binary. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e664
float g_1b2e664;                /* the score percentage this ladder last graded */

/* The marker was deliberately withheld while the tree was at Accuracy 100.00%;
   the port track retired that rule long ago. Original note follows. The same
   way wip/keysrank.cpp withholds its twin's. */
// FUNCTION: EZ2AC_UNPACKED 0x00465b30
int PlayerRecord::m465b30()
{
    const int *b = &block[g_elemCount * 16];
    int notes = b[0];
    int top = b[6] + b[5];
    float hitPct = (float)top / (float)notes * 100.0f;
    float scorePct = (float)b[9] / (float)(notes * 300) * 100.0f;

    g_1b2e664 = scorePct;

    if (g_1b2eb6c != 0)
    {
        if (hitPct >= 95.0f)  return 6;
        if (hitPct >= 90.0f)  return 5;
        if (hitPct >= 80.0f)  return 4;
        if (hitPct >= 70.0f)  return 3;
        if (hitPct >= 60.0f)  return 2;
        if (hitPct >= 50.0f)  return 1;
        return 0;
    }

    if (scorePct >= 100.0f) return 10;
    if (scorePct >= 98.0f)  return 9;
    if (scorePct >= 95.0f)  return 8;
    if (scorePct >= 93.0f)  return 7;
    if (scorePct >= 90.0f)  return 6;
    if (scorePct >= 85.0f)  return 5;
    if (scorePct >= 80.0f)  return 4;
    if (scorePct >= 70.0f)  return 3;
    if (scorePct >= 60.0f)  return 2;
    if (scorePct >= 50.0f)  return 1;
    return 0;
}
