/* pickSample42de50 @0x42de50 - the sample m42f080 stamps into a claimed
   hold note: a twelve-entry {sound, weight} ladder walked with one
   rand()%100 per rung - a rung answers when the roll is at or UNDER its
   weight (`cmp edx,weight; jle`), so each rung is a ~4-6% chance - and a
   flat rand()%12 pick when none answers. The table's numbers are
   instruction immediates in the original, kept here as the initializers
   they were; the twelfth entry is left to the aggregate's zero fill,
   which is why the original stores it with its own zero register right
   after the weight-5 group.

   The C had the test reversed (weight <= roll) until 2026-10-01, which
   made rung 0 (sample 0x13) answer ~95% of the time
   (docs/PORT-DELTAS.md). */
#include <stdlib.h>

// FUNCTION: EZ2AC_UNPACKED 0x0042de50
int pickSample42de50()
{
    struct P { int snd; int weight; };
    P t[12] = {
        { 0x13, 5 }, { 0x14, 5 }, { 0, 5 }, { 3, 5 },
        { 6, 3 },    { 4, 5 },    { 1, 5 }, { 5, 5 },
        { 2, 5 },    { 7, 3 },    { 0x12, 3 },
    };
    int i;

    for (i = 0; i < 12; i++)
    {
        if (rand() % 100 <= t[i].weight)
            return t[i].snd;
    }
    return t[rand() % 12].snd;
}
