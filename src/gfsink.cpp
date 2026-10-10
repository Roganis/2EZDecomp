#include "players.h"
#include "sink-view.h"

/* PlayerRecord::m465f60 @0x465f60 - the scratch game's score commit, the
   keys PlayerSlot::m42e620 (src/sink.cpp) without the CV2 combo rules and
   the audience meter: the same two value tables picked by the CV2 flag, the
   combo built by KOOL/COOL/GOOD and broken by FAIL/MISS (a MISS bumping the
   global miss tally), the value added straight to the score; then under
   mode 0 the gauge moves by the director's per-grade rate while it is
   positive and the one or two players are clamped - two players each
   re-fetched through findPlayer at every touch, game over raised when
   neither is left standing; under mode 1 a KOOL or COOL feeds the second
   gauge by the per-player base plus ten over the note count, clamped to
   [0, 100]. The grade's counter ticks last. Its own TU so the helpers stay
   calls. */

extern int g_1b2eb6c;        /* src/ez2acapp.cpp */
extern int g_elemCount;      /* 0x1b2e718 */
extern int g_1b2ea28;        /* src/moderunner.cpp */
extern int g_1b2ea34;        /* src/moderunner.cpp */
extern int g_1b2e704;        /* src/playerslot.cpp - the global miss tally */
extern int g_player0Down;    /* 0x1b2ef90 (src/globals.cpp) */
extern int g_player1Down;    /* 0x1b2ef64 */
extern int g_1b2e7d8;        /* src/globals.cpp */
extern float g_1b2eaec[2];   /* 0x1b2eaec */
extern int countListC();                 /* 0x4184a0 */
extern PlayerRecord *findPlayer(int id); /* 0x418f30 */

static __forceinline void checkThisPlayer(GFSinkSlot *self)
{
    if (0.0f >= self->gauge)
    {
        self->gauge = 0.0f;
        g_1b2ea28 = 1;
        self->dir->m45d860(self);
        g_1b2ea34 = 1;
    }
    else if (self->gauge > 100.0f)
    {
        self->gauge = 100.0f;
    }
}

static __forceinline void checkOnePlayer(GFSinkSlot *self, int index, int *down)
{
    if (0.0f >= ((GFSinkSlot *)findPlayer(index))->gauge)
    {
        ((GFSinkSlot *)findPlayer(index))->gauge = 0.0f;
        if (countListC() != 2)
        {
            g_1b2ea28 = 1;
            self->dir->m45d860(self);
        }
        *down = 1;
        g_1b2ea34 = 1;
    }
    else if (((GFSinkSlot *)findPlayer(index))->gauge > 100.0f)
    {
        ((GFSinkSlot *)findPlayer(index))->gauge = 100.0f;
    }
}

static __forceinline void checkBothPlayers(GFSinkSlot *self)
{
    checkOnePlayer(self, 0, &g_player0Down);
    checkOnePlayer(self, 1, &g_player1Down);

    if (g_player0Down == 1 && g_player1Down == 1)
    {
        g_1b2ea28 = 1;
        g_1b2ea34 = (g_elemCount < 1);
        self->dir->m45d860(self);
        g_1b2e7d8 = 2;
    }
}

static __forceinline void addAudienceValue(GFSinkSlot *self, float d, float hi)
{
    float a = d + self->audience;

    self->audience = a;
    if (0.0f > a)
        self->audience = 0.0f;
    else if (a > hi)
        self->audience = hi;
}

// FUNCTION: EZ2AC_UNPACKED 0x00465f60
void PlayerRecord::m465f60(int grade, int count, int flag)
{
    GFSinkSlot *self = (GFSinkSlot *)this;
    int value[6];

    value[2] = 0;
    value[1] = 0;
    value[0] = 0;
    if (g_1b2eb6c != 0)
    {
        value[3] = 40;
        value[4] = 100;
        value[5] = 170;
    }
    else
    {
        value[3] = 41;
        value[4] = 150;
        value[5] = 300;
    }

    if (count <= 0)
        return;

    if (grade == 5 || grade == 4 || grade == 3)
    {
        if (flag != 0)
        {
            self->combo += count;
            if (self->combo > self->maxCombo)
                self->maxCombo = self->combo;
        }
    }
    else if (grade == 2 || grade == 1)
    {
        if (grade == 1)
            g_1b2e704++;
        if (flag != 0)
        {
            self->combo = 0;
            self->fc = 0;
        }
    }

    self->score += value[grade];

    if (self->dir->mode == 0)
    {
        if (self->gauge > 0.0f)
            self->gauge = self->dir->gaugeRate[grade] + self->gauge;

        if (countListC() >= 2)
            checkBothPlayers(self);
        else
            checkThisPlayer(self);
    }
    else if (self->dir->mode == 1 && (grade == 5 || grade == 4))
    {
        float d = (g_1b2eaec[g_elemCount] + 10.0f) / (float)self->noteCount;

        addAudienceValue(self, d, 100.0f);
    }

    self->counts[grade]++;
}
