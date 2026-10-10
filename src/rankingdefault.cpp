/* RankingDirector::m4577b0 @0x4577b0 - the standalone default-name
   rewrite: when the slot-0 entry's eight characters are all 'A' or all
   spaces, replace them with "EZ2AC_FN" - on the second table (keyed by
   f85dfc) when f85df8 is set, else on the visible one (keyed by
   f109b48). Written FLAT, as the original is: sixteen compares and
   eight stores per table, the key re-read at every store. m45aaf0
   calls it when typing reaches full length. */

#include "ezscreen.h"
#include "directors.h"

#define NB(k) (((char *)this + 0x85e04)[f85dfc * 50 + (k)])
#define NA(k) (((char *)this + 0x20cc)[f109b48 * 50 + (k)])

// FUNCTION: EZ2AC_UNPACKED 0x004577b0
void RankingDirector::m4577b0()
{
    if (f85df8 != 0)
    {
        if ((NB(0) == 'A' && NB(1) == 'A' && NB(2) == 'A' && NB(3) == 'A' &&
             NB(4) == 'A' && NB(5) == 'A' && NB(6) == 'A' && NB(7) == 'A') ||
            (NB(0) == ' ' && NB(1) == ' ' && NB(2) == ' ' && NB(3) == ' ' &&
             NB(4) == ' ' && NB(5) == ' ' && NB(6) == ' ' && NB(7) == ' '))
        {
        NB(0) = 'E';
        NB(1) = 'Z';
        NB(2) = '2';
        NB(3) = 'A';
        NB(4) = 'C';
        NB(5) = '_';
        NB(6) = 'F';
        NB(7) = 'N';
        }
    }
    else
    {
        if ((NA(0) == 'A' && NA(1) == 'A' && NA(2) == 'A' && NA(3) == 'A' &&
             NA(4) == 'A' && NA(5) == 'A' && NA(6) == 'A' && NA(7) == 'A') ||
            (NA(0) == ' ' && NA(1) == ' ' && NA(2) == ' ' && NA(3) == ' ' &&
             NA(4) == ' ' && NA(5) == ' ' && NA(6) == ' ' && NA(7) == ' '))
        {
        NA(0) = 'E';
        NA(1) = 'Z';
        NA(2) = '2';
        NA(3) = 'A';
        NA(4) = 'C';
        NA(5) = '_';
        NA(6) = 'F';
        NA(7) = 'N';
        }
    }
}
