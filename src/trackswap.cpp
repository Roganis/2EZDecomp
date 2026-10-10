/* PlayerSlot::m430500 @0x430500 - point the in-game song-select overlay at
   one of this player's two keyed track entries.

   The two entries are found by key, 0xf and 0x17, through findTrack
   @0x42eee0. For player 1 that is all there is to it: the owner's +0x7c0
   just records the second one. For anybody else the two entries first
   exchange their sound HANDLES inside the owner's track list, the owner
   re-stamps the list so every sound slot points back at the entry that now
   owns it (claimTracks @0x422ca0), and +0x7c0 records the first entry
   instead. Either way the panel is told, with the same flag and the same
   two indices, through its own twin of this swap (Panel::m429950
   @0x429950).

   The panel hangs off the player's +0x3cc, which this tree models as a
   TrackSink - the same object under the name the judgement side gave it -
   so the call casts rather than merging two large layouts. */
#include "ez2acapp.h"
#include "players.h"
#include "gamepanels.h"

// FUNCTION: EZ2AC_UNPACKED 0x00430500
void PlayerSlot::m430500(int isFirst)
{
    int a = findTrack(0x0f);
    int b = findTrack(0x17);

    if (isFirst == 0)
    {
        TrackList *t = &f3d0->tracks[id];
        int       *ha = &t->items[a].handle;
        int       *hb = &t->items[b].handle;
        int        h = *ha;

        *ha = *hb;
        *hb = h;

        f3d0->claimTracks((ListNode *)this);
        f3d0->f7c0 = a;
    }
    else
    {
        f3d0->f7c0 = b;
    }

    ((Panel *)f3cc)->m429950(isFirst, a, b);
}
