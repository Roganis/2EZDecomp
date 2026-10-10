/* CatchPanel::LoadPVI @0x472650 - the Catch note field's .gds skin reader, and
   the fourth member of the loadGds family (EZ2DJ's @0x421190, GF's @0x45faf0,
   Catch's director @0x46d8d0). Its shape is the family's: construct a KParser
   over a 16KB buffer, read the file, then loop on readGdsSection dispatching a
   section name through an _stricmp chain.

   THE FUNCTION NAMES ITSELF. The fall-through arm logs

       "CatchPanel::LoadPVI: '%s' Unknown Section"

   which is where the class and the method name both come from - no guessing.

   AND IT IDENTIFIES Obj46F660. The `TrackN` arm calls loadItem with `this`, so
   the address-named placeholder in include/gdsitem.h - documented there as
   "only reached through 0x472650" - was CatchPanel all along. See the note left
   in that header for the three checks that confirm it.

   Seventeen sections, sixteen of them a bare `strcmp` + one loader call. Only
   two arms are not that shape: `TrackN` splits the digits off the section name
   and indexes the item table with them, and `General` reads a plain int through
   the shared readGdsInt rather than a method of its own. */

#include "gamepanels.h"
#include "kparser.h"
#include "players.h"   /* SlotSampleSink, SlotState, SlotSound */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern int readGdsInt(int *out, KParser *p);            /* 0x456270 */
extern int readGdsSection(char *out, KParser *p);       /* 0x4561b0 */
extern void Log(const char *fmt, ...);                  /* 0x402880 */

extern EzSubsystem *g_sub1b2f054;   /* the graphics singleton */

/* The CV2 flag (CV2Mix assets and rules - CV2\\... paths everywhere it is
   tested); the same flag the score ladder and the backgrounds branch on.
   Storage in src/ez2acapp.cpp. */
extern int g_1b2eb6c;

/* The running song's short name - the two hard-coded titles below get a
   different catcher bar than everything else. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2f004
char g_songShortName[0x40];

/* Which catcher skin the normal path uses; the bar is numbered from it.
   Storage is in src/modeselectctor.cpp, which resets it. */
extern int g_1b2efa0;

// FUNCTION: EZ2AC_UNPACKED 0x00472650
int CatchPanel::LoadPVI(char *name)
{
    Obj402860Head *head;
    int            n;
    char           sec[0x100];
    char           path[0x100];
    char           digits[0x80];
    KParser        parser;
    char           buf[0x4000];

    parser.setBuffer(buf, 0x4000);

    /* Both failure returns are plain early returns; cl cross-jumps their two
       tails into the single out-of-line block at 0x472a63, which is why the
       `je` here and the unknown-section log both land on it. */
    if (parser.readFile(name) == 0)
    {
        return 0;
    }

    f1b54 = 0;

    while (readGdsSection(sec, &parser) != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x004906b8
        if (_strnicmp("Track", sec, 5) == 0)
        {
            strcpy(digits, sec + 5);
            /* The index must go through a NAMED LOCAL: written inline as
               `atoi(digits) - 1` cl emits `sub eax,1`, but held in a local
               first it emits the original's `add eax,-1`. */
            n = atoi(digits);
            loadItem(&parser, n - 1);
        }
        // STRING: EZ2AC_UNPACKED 0x004906a8
        else if (_stricmp("GroupTexture", sec) == 0)
        {
            ((GFPanel *)this)->m460ac0(&parser);
        }
        else if (_stricmp("General", sec) == 0)
        {
            readGdsInt(&f2f8, &parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0049069c
        else if (_stricmp("Judgment", sec) == 0)
        {
            m4700f0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490678
        else if (_stricmp("CoolBomb", sec) == 0)
        {
            m470210(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490668
        else if (_stricmp("LongNoteBomb", sec) == 0)
        {
            m470650(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0049065c
        else if (_stricmp("CoolCombo", sec) == 0)
        {
            m471750(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490650
        else if (_stricmp("KeyPanel", sec) == 0)
        {
            m470940(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490644
        else if (_stricmp("TargetBar", sec) == 0)
        {
            m4709a0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490634
        else if (_stricmp("MaxCoolCombo", sec) == 0)
        {
            m470bc0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0049062c
        else if (_stricmp("Score", sec) == 0)
        {
            m470cc0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490620
        else if (_stricmp("ComboGauge", sec) == 0)
        {
            m470dc0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x00490614
        else if (_stricmp("GrooveGauge", sec) == 0)
        {
            m470e50(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x0048e6bc
        else if (_stricmp("Effector", sec) == 0)
        {
            m471060(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905d8
        else if (_stricmp("GrooveLight", sec) == 0)
        {
            m470ee0(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905bc
        else if (_stricmp("ComboEffect", sec) == 0)
        {
            m471200(&parser);
        }
        // STRING: EZ2AC_UNPACKED 0x004905a0
        else if (_stricmp("GoodBomb", sec) == 0)
        {
            m4704f0(&parser);
        }
        else
        {
            /* An unrecognized section ABORTS the load - after the log the
               original falls straight through into the return-0 epilogue at
               0x472a63 with no jump, which is only reachable this way. */
            // STRING: EZ2AC_UNPACKED 0x0049e1c8
            Log("CatchPanel::LoadPVI: '%s' Unknown Section", sec);
            return 0;
        }
    }

    /* The catcher bar is not part of the .gds at all - it is chosen after the
       file is read, from the cabinet flag and the song. Two songs get bar 3. */
    if (g_1b2eb6c != 0)
    {
        // STRING: EZ2AC_UNPACKED 0x0049e218
        if (_stricmp(g_songShortName, "8ashtray-ca1") != 0 &&
            // STRING: EZ2AC_UNPACKED 0x0049e208
            _stricmp(g_songShortName, "therhythm-ca1") != 0)
        {
            // STRING: EZ2AC_UNPACKED 0x0049e1f4
            sprintf(path, "Catcher1\\Bar1.str");
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x0049e1b4
            sprintf(path, "Catcher3\\Bar3.str");
        }
    }
    else
    {
        // STRING: EZ2AC_UNPACKED 0x0049e1a0
        sprintf(path, "Catcher%d\\Bar%d.str", g_1b2efa0 + 1, g_1b2efa0 + 1);
    }

    ((KGraphics *)g_sub1b2f054)->loadStr(path, &head);
    m_28.attach(head, 1);

    return 1;
}

/* The period's FastFloatToInt, as in src/gamepanels.cpp. */
__forceinline void FloatToInt(int *i, float f)
{
    __asm fld f
    __asm mov edx, i
    __asm fistp dword ptr [edx]
}

/* The state record the catch gauge reports into, hanging off the slot. */
struct CatchLifeView
{
    char _pad0[0x768];
    int  state;                    /* 0x768 - 2 alive, 3 topped out */
};

struct CatchStageView
{
    char _pad0[0x360];
    struct CatchLifeView *life;    /* 0x360 */
};

extern int g_eventMode;   /* 0x01696404 (src/settings.cpp) */
extern int g_d36cb0;      /* 0x00d36cb0 (src/maingame.cpp) */
extern int g_1b2eb6c;     /* the mode's ladder flag (src/ez2acapp.cpp) */
extern int g_1b2ebac;     /* (src/gamepanels.cpp) */
extern int g_1b5f19c;     /* (src/playerslot.cpp) */
extern int g_1b2e704;     /* (src/playerslot.cpp) */
extern int g_1b2ea34;     /* (src/moderunner.cpp) */
extern int g_gameLevel;   /* 0x04a8d74 (src/settings.cpp) */
extern float g_catchGaugeRates[16][6];  /* 0x49e3c8 (src/globals.cpp) */
extern int countListB();  /* 0x4183e0 (src/nodelists.cpp) */
extern void Log(const char *fmt, ...);  /* 0x402880 (src/applog.cpp) */

/* The commit's view of the rate carrier at slot+0x360. */
struct CatchRateView
{
    char  _pad0[0x314];
    float rates[6];            /* 0x314 - per-judge gauge deltas */
};
extern int g_elemCount;   /* 0x01b2e718 (src/loops.cpp) */
extern int g_1b2ea28;     /* (src/moderunner.cpp) */
extern int g_1b2eba4;     /* 0x01b2eba4 (src/maingame.cpp) */
extern int g_1b2efb4;     /* 0x01b2efb4 (src/moderunner.cpp) */

/* Walk one item's note run and draw every hit before the window closes -
   holds through m46fbb0, taps through m46fe30 - carrying the item's own
   baseline. The catch panel has no reskin gates: the voice is the column's
   skin throughout. The window math is the shared double-x87 truncation
   with the parameter reused as the fild home and the limit.

   56.68%, all of it one scheduling divergence: the original SINKS the
   x87 tail (fmul/fdivp/fldcw/fistp) and the limit arithmetic below the
   begin call's six pushes, with the cw temp in snd's dead slot and the
   fistp in n's own - ours completes the chain before the pushes with the
   temps in the dead param homes, and every following operand shifts.
   The m42b2f0/m463b40 pool-width family's root, amplified; declaration
   positions and the voice's placement do not move it. Semantically
   complete; re-score after seed rolls. */
/* 57.75%. NOT the gauges' aggregate defect - same frame size (0x34), same
   instruction count within one - but a single scratch-slot choice that
   cascades. Both sides reuse the FIRST argument's home for the fnstcw
   scratch; for the SECOND scratch (the modified control word, and the
   fistp destination) the original takes [F-0x30] - `snd`'s own slot, whose
   live range has not started yet - while cl takes [F+0xc], the home of the
   third argument `col`, because our `col` dies at the `voice` load and the
   original's is still live there. The original loads `voice` immediately
   before the begin() pushes; cl hoists it above the divide.

   Three forms measured, none of which moves it: `voice = e->f68[col]`
   before the divide rather than after (57.75%), the same as a declaration
   with initializer at the assignment point (57.75%), and no `voice` local
   at all with `e->f68[col]` inline at both call sites - which should keep
   `col` live into the loop and does not (56.84%). Read the frames with
   scratchpad/frame.py on both binaries before trying a fourth. */
/* 57.75%, and NOT for want of a source shape - thirteen were swept on
   2026-08-25 and every one compiled to the identical listing. The
   reconstruction is 94 instructions against the original's 93 and the
   multiset is otherwise the same; what differs is the SCHEDULE.

   The original interleaves the begin() argument pushes INTO the inline
   ftol: it saves the control word, computes `voice`, pushes all six
   arguments, and only then runs `fmul`/`fdivp`/`fldcw`/`fistp` - and it can,
   because it packed the ftol temporary onto `snd`'s own slot ([esp+0x10] is
   both the control-word scratch and the address pushed as &snd), which is
   safe since the callee cannot write through the pointer until the call.
   cl finishes the whole ftol first, keeps the temp on a slot of its own, and
   computes `voice` early instead. The one extra instruction is `mov ecx,edi`
   emitted in both arms of the width test where the original hoists it above
   the branch - and interleaves it between the two shared pushes.

   Swept with no effect: five statement orders (voice at its declaration as
   the Panel twin has it, `e->ffc = 0` either side of the `item` update, the
   conversion inlined with no named `n`, st/snd declaration order), four
   scopings of the two out-parameter aggregates (function scope, the
   `if` block, the innermost block, and both orders there - the documented
   lever for exactly this slot overlap), and four shapes of the two-call
   branch (ternary, inverted arms, a named `SlotState *`). The frame is the
   same size on both sides (`sub esp,0x34`) and the first thirteen
   instructions are identical, so it is not a size error either.

   Note the Panel twin m42b2f0 @0x42b2f0 has the SAME source shape and scores
   95.73%, with its original computing `voice` EARLY - the opposite schedule
   from the same text. The catch copy has no reskin block, so it carries one
   less live value through the loop, and that is what moves the scheduler.
   Do not re-sweep this from source. */
// FUNCTION: EZ2AC_UNPACKED 0x0046ff40
void CatchPanel::m46ff40(int item, int start, int col)
{
    SlotSound snd;
    SlotState st;
    GdsItem *e = (GdsItem *)((char *)this + item * 0x104 + 0x2fc);

    if (e->f00 != 0)
    {
        float t = f1950 - e->f08 + f1958;
        KTexture *voice;
        int n;

        item = f254;
        {
            double q = 48.0f / item;

            n = (int)(float)((t + e->f08 - e->f08) / (q * e->fe8));
        }
        voice = e->f68[col];
        item = n + item * 4 + f250;
        e->ffc = 0;
        {
            int r = ((SlotSampleSink *)slot->f364)->begin(e->fec, start,
                                                          0, 1, &st, &snd);

            while (r != 0)
            {
                int x;

                if ((unsigned int)st.start >= (unsigned int)item)
                    break;
                if (st.width > 6)
                    x = m46fbb0(e, &st, voice, t);
                else
                    x = m46fe30(e, &st, voice, t);
                if (x == 2)
                    break;
                r = ((SlotSampleSink *)slot->f364)->next(1, &st, &snd);
            }
        }
    }
}

/* The catch gauge: the plate at its own raw x/y, the clip chart pumped,
   the level chased a fifth per frame with an eight-point beat jolt, the
   ready/danger state pushed into the slot's record unless event mode is
   on (topping out pins the bar full), the bar drawn as a sprite span,
   and the over-100 flash blitting the named texture twice in random
   colors at the bar's moving edge.

   Fully verified by an exact stack-depth tracker (the earlier draft's
   doubts are resolved): the span is (sprite, (float)nx2, (float)delta2,
   0, fnd, 0, (h-fnd)*fr, 1.0f, fr*fnd, -1), the flash's y recomputes
   nq - nd2 fresh, and delta1/y0/x2 are computed-but-unused locals the
   inline-asm presence keeps alive (cl pins the frame around __asm). The
   score residue is the gauges' frame-slot seed family. */
// FUNCTION: EZ2AC_UNPACKED 0x00471c20
void CatchPanel::m471c20()
{
    int nd2;
    int nd;
    int nx2;
    float disp;
    float h;
    float w;
    float h2;
    /* The address-exposed block - see m472020 below for why. */
    struct { int nx, delta2, x2, nq, y0, delta1, nw2, nhfull; } v;
    unsigned int q;
    float fnd;
    float fr;

    if (s1b04.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->drawTex(s1b04.f0c, s1b04.f04,
                                             s1b04.f08, 0, 0, -1);
        if (f1bb0 != 0)
        {
            f1c4c->m403940();
            if (f1c4c->isDone() != 0)
                f1c4c->attach(0, 0);
        }
        w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1b04.f18);
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1b04.f18);
        h2 = h + s1b04.f14;
        q = f250 / f254;
        f1b2c = (f1b24 - f1b2c) * 0.2f + f1b2c;
        disp = (f1b2c * h) * 0.01f;
        if (f1b30 != (int)q)
            f1b2c = f1b2c + 8.0f;
        if (g_eventMode == 0)
        {
            if (f1b24 >= 100.0f)
            {
                ((CatchStageView *)slot)->life->state = 3;
                disp = h;
            }
            else
                ((CatchStageView *)slot)->life->state = 2;
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&v.nhfull, h);
        v.delta1 = v.nhfull - nd;
        v.y0 = 0;
        FloatToInt(&v.nw2, w);
        FloatToInt(&v.nq, h2);
        v.delta2 = v.nq - nd;
        FloatToInt(&v.nx, s1b04.f10);
        v.x2 = v.nx + v.nw2;
        FloatToInt(&nx2, s1b04.f10);
        FloatToInt(&nd2, disp);
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        fnd = (float)nd2;
        fr = 1.0f / h;
        ((KGraphics *)g_sub1b2f054)->m404660(
            (KTexture *)s1b04.f18, (float)nx2, (float)v.delta2, 0,
            fnd, 0, (h - fnd) * fr, 1.0f, fr * fnd, -1);
        if (f1b24 >= 100.0f)
        {
            unsigned int c;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->drawTex(s1b04.f1c, s1b04.f10,
                                                 (float)(v.nq - nd2), 0, 0, c);
            c = (((rand() % 256 << 8 | rand() % 256) << 8 |
                  rand() % 256) << 8) | rand() % 256;
            ((KGraphics *)g_sub1b2f054)->drawTex(s1b04.f1c, s1b04.f10,
                                                 (float)(v.nq - nd2), 0, 0, c);
        }
    }
}

/* The catch player-life gauge, over s1b34: the same shape as the stage
   gauge above but blended first, chasing the slot's own life (f1ec) into
   f1b28 with a three-point jolt and owning the beat latch; a death sets
   the retire flag and raises the game-over globals (solo with no
   elements -> g_1b2efb4, otherwise g_1b2eba4), the span only draws while
   alive, and there is no flash.

   56.73%: unlike the stage gauge, this compile register-allocates the
   computed-but-unused extents (fewer live values here), so the original's
   0x40 frame comes out 0x30 and every slot shifts; the volatiles on the
   dead extents (semantically no-op) recover the hoisted 1-in-edi but not
   the frame. The gauges' seed family. */
/* 56.73%, and the defect is STRUCTURAL, not a codegen coin: the original's
   frame is `sub esp,0x40` and ours is `sub esp,0x30`, so this reconstruction
   is FOUR WORDS OF LOCALS SHORT and every slot below them shifts. The
   original's extra words live at the top of the frame and one of them is
   read back much later (`cvtsi2ss xmm0,[esp+0x3c]` at 0x4722cf against a
   store at 0x472187), so they are real variables with real consumers, not
   spill scratch. Where the original writes a difference and a zero into two
   distinct slots around 0x472187, ours writes both into the slot it also
   uses as the FloatToInt pointer home.

   Next step is scratchpad/frame.py on both sides - the raw `[esp+X]`
   offsets cannot be compared directly because the push depth differs at
   every call site, which is what that script normalizes away. Do that
   before guessing at which locals are missing. */
// FUNCTION: EZ2AC_UNPACKED 0x00472020
void CatchPanel::m472020()
{
    int nd2;
    int nd;
    int nx2;
    float disp;
    float h;
    float w;
    float h2;
    /* ONE ADDRESS-EXPOSED BLOCK, not eight locals. Three of these - v.delta1,
       v.y0 and v.x2 - are computed and never read, and the original still stores
       all three into slots of their own: only an aggregate whose address is
       taken keeps a dead store AND reserves its slot. As plain locals cl
       deletes them; as `volatile` locals it keeps the stores but packs all
       three into one slot, because their live ranges are empty either way.
       The field order is the original's slot order, lowest address first. */
    struct { int nx, delta2, x2, nq, y0, delta1, nw2, nhfull; } v;
    unsigned int q;
    float fnd;
    float fr;

    if (s1b34.f00 != 0)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
        ((KGraphics *)g_sub1b2f054)->drawTex(s1b34.f0c, s1b34.f04,
                                             s1b34.f08, 0, 0, -1);
        if (f1bb0 != 0)
        {
            f1c48->m403940();
            if (f1c48->isDone() != 0)
                f1c48->attach(0, 0);
        }
        w = ((KGraphics *)g_sub1b2f054)->widthOf((Sprite *)s1b34.f18);
        h = ((KGraphics *)g_sub1b2f054)->heightOf((Sprite *)s1b34.f18);
        h2 = h + s1b34.f14;
        f1b28 = (slot->f1ec - f1b28) * 0.2f + f1b28;
        q = f250 / f254;
        disp = (f1b28 * 0.01f) * h;
        if (f1b30 != (int)q)
        {
            f1b30 = q;
            f1b28 = f1b28 + 3.0f;
        }
        if (disp > h)
            disp = h;
        FloatToInt(&nd, disp);
        FloatToInt(&v.nhfull, h);
        v.delta1 = v.nhfull - nd;
        v.y0 = 0;
        FloatToInt(&v.nw2, w);
        FloatToInt(&v.nq, h2);
        v.delta2 = v.nq - nd;
        FloatToInt(&v.nx, s1b34.f10);
        v.x2 = v.nx + v.nw2;
        FloatToInt(&nx2, s1b34.f10);
        FloatToInt(&nd2, disp);
        if (0.0f >= slot->f1ec)
        {
            g_1b2ea28 = 1;
            if (g_1b2eb6c == 0)
            {
                if (g_elemCount == 0)
                    g_1b2efb4 = 1;
                else
                    g_1b2eba4 = 1;
            }
            else
                g_1b2eba4 = 1;
        }
        if (slot->f1ec > 0.0f)
        {
            float frac;

            ((KGraphics *)g_sub1b2f054)->setBlend(2, 1);
            fnd = (float)nd2;
            fr = 1.0f / h;
            frac = fr * fnd;
            if (frac > 0.0f)
                ((KGraphics *)g_sub1b2f054)->m404660(
                    (KTexture *)s1b34.f18, (float)nx2, (float)v.delta2, 0,
                    fnd, 0, (h - fnd) * fr, 1.0f, frac, -1);
        }
    }
}

#include <math.h>

/* The catch judge commit: five kinds of bookkeeping keyed on the judge
   (5 = a catch, 1 = a drop), three mode shapes (the second ladder, the
   crash-counting alternative, and the plain game), the log10 combo bonus
   into the score, the per-judge gauge delta scaled by the game level's
   ladder, the 0..100 clamp with the game-over decision at zero, and the
   per-judge counters. */
// FUNCTION: EZ2AC_UNPACKED 0x00472fc0
void PanelSlot::m472fc0(int judge, int n, int flag)
{
    if (n > 0)
    {
        if (g_1b2eb6c == 1)
        {
            if (g_1b2ebac == 0)
            {
                if (judge == 5)
                {
                    if (flag != 0)
                    {
                        if (f1c0 == 1)
                            f1dc += n;
                        else
                        {
                            f1dc = n - 1;
                            f1c0 = 1;
                        }
                        if (f1dc > f1e4)
                            f1e4 = f1dc;
                        g_1b5f19c = 0;
                        goto tail;
                    }
                }
                else if (judge == 1)
                    goto miss;
                goto score;
            }
            if (judge == 5)
            {
                f54c += 1;
                f1dc = 0;
                f1c0 = 0;
                g_1b2e704 += 1;
                {
// STRING: EZ2AC_UNPACKED 0x0049e4a0
                    Log("CRASH !! : %d", f54c);
                }
                goto tail;
            }
            if (judge == 1)
            {
                f1dc += n;
                f550 += 1;
            }
            goto tail;
        }
        if (judge == 5)
        {
            if (flag != 0)
            {
                f1dc += n;
                if (f1dc > f1e4)
                    f1e4 = f1dc;
                g_1b5f19c = 0;
            }
            goto tail;
        }
        if (judge == 1)
        {
miss:
            if (flag != 0)
            {
                f1c0 = 0;
                f1dc = 0;
            }
            g_1b2e704 += 1;
        }
tail:
        if (g_1b2ebac != 0)
        {
            f1e8 += f1dc / 50 + 1;
            goto rates;
        }
score:
        /* The combo is read into the dead `flag`'s home, as the original. */
        flag = f1dc;
        if (flag > 0)
            f1e8 += (int)((log10((float)flag) * 0.2f) * 250.0f + 250.0f);
rates:
        if (countListB() == 1 && g_1b2ea28 == 0)
            f1ec += ((CatchRateView *)f360)->rates[judge] *
                    g_catchGaugeRates[g_gameLevel][judge];
        if (0.0f >= f1ec)
        {
            if (g_elemCount >= 1)
            {
                g_1b2ea34 = 0;
                if (g_1b2efb4 == 1)
                    goto zero;
            }
            g_1b2ea34 = 1;
zero:
            f1ec = 0.0f;
        }
        else if (f1ec > 100.0f)
            f1ec = 100.0f;
        f1c4[judge] += 1;
        f1e0 = f1e4;
    }
}

/* And the catch panel's copy of it, over its own three heads. */
// FUNCTION: EZ2AC_UNPACKED 0x004712d0
void CatchPanel::m4712d0(int, int mode)
{
    if (mode == 2 && f1bb0 != 0)
    {
        f1c44->attach(f1bb4, 0);
        f1c4c->attach(f1bbc, 0);
        f1c48->attach(f1bb8, 0);
    }
}

/* Draw every note run for the current position. Two values come off the
   clock pair at +0x250 and +0x254: a column index from the fraction of the
   current span - six columns, one less, floored at zero - and a start tick
   six hundred spans back, or zero if the song has not run that far yet. The
   clamp is branchless in the original, so it is written as a select rather
   than an overwrite. */
// FUNCTION: EZ2AC_UNPACKED 0x00470070
void CatchPanel::m470070()
{
    unsigned int now = f250;
    unsigned int span = f254;
    unsigned int start;
    int c;
    int i;

    c = (int)(now % span * 6 / span) - 1;
    c = c < 0 ? 0 : c;

    if (now > span * 600)
        start = now - span * 600;
    else
        start = 0;

    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);

    for (i = 0; i < f2f8; i++)
        m46ff40(i, start, c);
}
