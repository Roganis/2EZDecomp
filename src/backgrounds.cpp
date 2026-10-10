/* GFBackGround and CatchBackGround - the two siblings of BackGround
   (src/panels.cpp). See include/backgrounds.h for the family table.

   All three destructors are the same four statements: tear the flag block down,
   free the mesh's buffers, and let the two members with destructors go. */

#include <stdio.h>
#include <string.h>
#include "backgrounds.h"
#include "directors.h"   /* TotalResultDirector - its trim buffer lives here too */
#include "ez2ac.h"   /* Obj40B970 - the flag block's getter @0x40b970 */

/* The graphics singleton under the name its already-matched COM wrappers use
   (src/com_wrappers.cpp); only the slot shape matters for the call. */
struct Obj40BB00
{
    void doOp7(int v);   /* 0x40a470 */
};

extern char g_textSlots[32][128];  /* 0x1b2ec18 (src/modename.cpp) */
extern int  g_elemCount;           /* 0x1b2e718 (src/loops.cpp) */
extern int  g_1b2eb6c;             /* src/ez2acapp.cpp */
extern int  g_1b2ea14;             /* src/panels.cpp */
extern char g_songInfoName[64];    /* 0x1b2eb80 (src/panels.cpp) */

/* Storage in src/classinfo.cpp. */
extern KRuntimeClass classGFBackGround;     /* 0x4ae650 */
extern EzObjectInfo  g_info_4ae660;         /* 0x4ae660 */
extern KRuntimeClass classCatchBackGround;  /* 0x4aeb3c */
extern EzObjectInfo  g_info_4aeb4c;         /* 0x4aeb4c */

// FUNCTION: EZ2AC_UNPACKED 0x0045c450
KRuntimeClass *GFBackGround::getRuntimeClass()
{
    return &classGFBackGround;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c460
DrawGroup *GFBackGround::getObjectInfo()
{
    return &g_info_4ae660;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045c550
GFBackGround::~GFBackGround()
{
    ((KGraphics *)g_sub1b2f054)->destroyFlag(&m_flag);
    ((KGraphics *)g_sub1b2f054)->freeBuffers(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0045cd20
// GFBackGround::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int GFBackGround::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x00499df0
struct GFBackGround;

/* GFBackGround's own trim buffer - the three classes each own one, which is
   why the original's ICF left their identical videoName() bodies unfolded. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd280
static char g_gfVideoName[260];

/* The ranking screen's replay flag (written by RankingDirector's ctor,
   src/rankingctor.cpp) sits 4 bytes below the trim buffer, and videoName's
   biased `buf[len - 4]` operand lands exactly on it. reccmp patch 13 shadows
   this address for displacement resolution, so that operand reads as
   g_gfVideoName-4 on both sides while the ctor's own absolute stores still
   resolve by name. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd27c
char g_37dd27c[4];

// FUNCTION: EZ2AC_UNPACKED 0x0045c470
char *GFBackGround::videoName()
{
    strcpy(g_gfVideoName, g_textSlots[g_elemCount]);

    if (g_gfVideoName[strlen(g_gfVideoName) - 3] == '-')
        g_gfVideoName[strlen(g_gfVideoName) - 3] = 0;
    else if (g_gfVideoName[strlen(g_gfVideoName) - 4] == '-')
        g_gfVideoName[strlen(g_gfVideoName) - 4] = 0;
    else if (g_gfVideoName[strlen(g_gfVideoName) - 5] == '-')
        g_gfVideoName[strlen(g_gfVideoName) - 5] = 0;

    return g_gfVideoName;
}

// FUNCTION: EZ2AC_UNPACKED 0x0045cbb0
GFBackGround::GFBackGround()
{
    char cwd[260];

    f102118 = 0;
    f24 = 0;
    f698 = 0;
    f20 = 0;
    g_1b2ea14 = 0;
    f10 = 0;
    f14 = 0;
    memset(m_102078, 0, sizeof(m_102078));
    fa8 = 0;
    ((KGraphics *)g_sub1b2f054)->createFlag(&m_flag, 0.3f, 0.00012f);
    m_mesh = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    GetCurrentDirectoryA(260, cwd);

    if (g_1b2eb6c == 0)
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, videoName(), videoName());
    else
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, g_songInfoName, g_songInfoName);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ab80
KRuntimeClass *CatchBackGround::getRuntimeClass()
{
    return &classCatchBackGround;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ab90
DrawGroup *CatchBackGround::getObjectInfo()
{
    return &g_info_4aeb4c;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046ac80
CatchBackGround::~CatchBackGround()
{
    ((KGraphics *)g_sub1b2f054)->destroyFlag(&m_flag);
    ((KGraphics *)g_sub1b2f054)->freeBuffers(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b990
// CatchBackGround::`scalar deleting destructor'

// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int CatchBackGround::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049d708
struct CatchBackGround;

// GLOBAL: EZ2AC_UNPACKED 0x037df330
static char g_catchVideoName[260];

// FUNCTION: EZ2AC_UNPACKED 0x0046aba0
char *CatchBackGround::videoName()
{
    strcpy(g_catchVideoName, g_textSlots[g_elemCount]);

    if (g_catchVideoName[strlen(g_catchVideoName) - 3] == '-')
        g_catchVideoName[strlen(g_catchVideoName) - 3] = 0;
    else if (g_catchVideoName[strlen(g_catchVideoName) - 4] == '-')
        g_catchVideoName[strlen(g_catchVideoName) - 4] = 0;
    else if (g_catchVideoName[strlen(g_catchVideoName) - 5] == '-')
        g_catchVideoName[strlen(g_catchVideoName) - 5] = 0;

    return g_catchVideoName;
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b820
CatchBackGround::CatchBackGround()
{
    char cwd[260];

    f10210c = 0;
    f1c = 0;
    f690 = 0;
    f18 = 0;
    g_1b2ea14 = 0;
    fac = 0;
    fb0 = 0;
    memset(m_items, 0, sizeof(m_items));
    fa0 = 0;
    ((KGraphics *)g_sub1b2f054)->createFlag(&m_flag, 0.3f, 0.00012f);
    m_mesh = 0;
    ((Obj40BB00 *)g_sub1b2f054)->doOp7(0);
    GetCurrentDirectoryA(260, cwd);

    if (g_1b2eb6c == 0)
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, videoName(), videoName());
    else
        sprintf(m_videoPath, "%s\\Video\\%s\\%s.spv", cwd, g_songInfoName, g_songInfoName);
}

/* BackGround::drawItem @0x41afc0 and CatchBackGround::drawItem @0x46afe0 - one
   kind-5 entry of the draw list, drawn through the background's own mesh.

   The item carries the whole render state: a mode, the blend pair and one
   texture for stage 0, with stage 1 unbound and blend recipe 0 selected. The
   mesh is then stepped and drawn, and the draw is a tail call - which is why
   the two graphics calls have to live in src/stubs.cpp. */
// FUNCTION: EZ2AC_UNPACKED 0x0041afc0
void BackGround::drawItem(BgItem *it)
{
    ((KGraphics *)g_sub1b2f054)->setMaterial(it->f20);
    ((KGraphics *)g_sub1b2f054)->setBlend(it->blendSrc, it->blendDst);
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, it->tex);
    ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
    ((KGraphics *)g_sub1b2f054)->m408130(0);
    ((KGraphics *)g_sub1b2f054)->m40a300(m_mesh);
    ((KGraphics *)g_sub1b2f054)->m409480(m_mesh);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046afe0
void CatchBackGround::drawItem(BgItem *it)
{
    ((KGraphics *)g_sub1b2f054)->setMaterial(it->f20);
    ((KGraphics *)g_sub1b2f054)->setBlend(it->blendSrc, it->blendDst);
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, it->tex);
    ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
    ((KGraphics *)g_sub1b2f054)->m408130(0);
    ((KGraphics *)g_sub1b2f054)->m40a300(m_mesh);
    ((KGraphics *)g_sub1b2f054)->m409480(m_mesh);
}

/* The two draw-list walkers: ten slots, each holding a pointer to an item whose
   own +4 says what to do with it. Kinds 2 and 3 refresh the matching chart,
   kind 4 and kind 5 are the class's two draws, and anything else is skipped.

   `i` is DEAD in both - the slot pointer and the chart pointer are its only
   uses and both became induction variables, which is what leaves the original
   with a separate down-counter.

   The flag block is stepped first, and the two classes differ there: BackGround
   steps it unconditionally and then bails on its own flag, CatchBackGround
   steps it only when the block itself asks. That getter @0x40b970 is the
   binary's first proven match, filed under the placeholder name Obj40B970 -
   this call site says the class is Obj40B2E0. Merging the two models is a
   later cleanup; the cast is what keeps the existing match intact. */
// FUNCTION: EZ2AC_UNPACKED 0x0041b040
void BackGround::drawList()
{
    int i;

    m_1c0.m40b710();

    if (f698 != 0)
    {
        return;
    }

    for (i = 0; i < 10; i++)
    {
        BgItem *it = m_items[i].item;

        if (it != 0)
        {
            if (it->kind == 2 || it->kind == 3)
            {
                m_charts[i].m403940();
            }
            else if (it->kind == 4)
            {
                drawItem4(it);
            }
            else if (it->kind == 5)
            {
                drawItem(it);
            }
        }
    }
}

// FUNCTION: EZ2AC_UNPACKED 0x0046b060
void CatchBackGround::drawList()
{
    int i;

    if (((Obj40B970 *)&m_1b8)->getField438() != 0)
    {
        m_1b8.m40b710();
    }

    for (i = 0; i < 10; i++)
    {
        BgItem *it = m_items[i].item;

        if (it != 0)
        {
            if (it->kind == 2 || it->kind == 3)
            {
                m_charts[i].m403940();
            }
            else if (it->kind == 4)
            {
                drawItem4(it);
            }
            else if (it->kind == 5)
            {
                drawItem(it);
            }
        }
    }
}

extern EzObjectInfo g_info_4ae660, g_info_4aeb4c;  /* 0x4ae660, 0x4aeb4c */

/* GFBackGround's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0045cf70
GFBackGround *createGFBackGround()
{
    g_objectRecord = &g_info_4ae660;
    return new GFBackGround;
}

/* CatchBackGround's new-plus-ctor factory: publish the record, take a slot out of its pool
   and construct. */
// FUNCTION: EZ2AC_UNPACKED 0x0046be20
CatchBackGround *createCatchBackGround()
{
    g_objectRecord = &g_info_4aeb4c;
    return new CatchBackGround;
}

/* THE FOURTH COPY OF videoName, and the one that is not a BackGround at all:
   TotalResultDirector::m4544f0 @0x4544f0 trims the same way over its own
   buffer, but takes the slot INDEX as an argument (`ret 4`) where the three
   background copies read g_elemCount. Its two callers in
   src/totalresultctor.cpp pass the stage. Found by scratchpad/twins.py at
   67% against BackGround::videoName - the parameter and the buffer address
   are the whole difference. It lives here rather than beside its callers so
   cl cannot see the body and inline it. */
// GLOBAL: EZ2AC_UNPACKED 0x037dd178
static char g_totalResultName[260];

// FUNCTION: EZ2AC_UNPACKED 0x004544f0
char *TotalResultDirector::m4544f0(int stage)
{
    strcpy(g_totalResultName, g_textSlots[stage]);

    if (g_totalResultName[strlen(g_totalResultName) - 3] == '-')
        g_totalResultName[strlen(g_totalResultName) - 3] = 0;
    else if (g_totalResultName[strlen(g_totalResultName) - 4] == '-')
        g_totalResultName[strlen(g_totalResultName) - 4] = 0;
    else if (g_totalResultName[strlen(g_totalResultName) - 5] == '-')
        g_totalResultName[strlen(g_totalResultName) - 5] = 0;

    return g_totalResultName;
}

/* THE AUDIENCE RATE'S FULL READOUT @0x00419a70, one of the three variants
   update @0x41a910 tail-jumps to. It lives here, and not in src/panels.cpp,
   because that file defines the update: a callee cl can see is inlined and
   the tail jumps vanish.

   Five columns per row - 269, 282, the point at 291, 300, 313 - with only
   the thousands digit gated on `>= 1000`, which is what makes this a rate
   read as hundreds-point-tens-units. The live value at f24 draws first at
   y+399; then the other side's f1c draws at y+416, in the `next` digits with
   no tint while it is BELOW the target and in the `cw` digits tinted f14
   once it reaches it. The magic multipliers in the original are just /1000,
   /100 and /10 - cl regenerates them from the divisions. */
// FUNCTION: EZ2AC_UNPACKED 0x00419a70
void AudienceRatePanel::drawFull()
{
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    if (f24 >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 1000 % 10], 269.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 100 % 10], 282.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(nextPoint, 291.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 10 % 10], 300.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 % 10], 313.0f, f28 + 399.0f, 0, 0, -1);

    if (f1c < f24)
    {
        if (f1c >= 1000)
            ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 269.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 282.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 291.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 300.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 313.0f, f28 + 416.0f, 0, 0, -1);
    }
    else
    {
        if (f1c >= 1000)
            ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 269.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 282.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 291.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 300.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 313.0f, f28 + 416.0f, 0, 0, f14);
    }
}
/* THE SAME READOUT 42 PIXELS RIGHT @0x00419e70 - drawSolo, the second of the three variants
   update @0x41a910 tail-jumps to. It lives here, and not in src/panels.cpp,
   because that file defines the update: a callee cl can see is inlined and
   the tail jumps vanish.

   Byte for byte drawFull with the five columns moved to 311, 324, the point
   at 333, 342, 355 - the same values, the same digit sets, the same tint
   rule. Only
   the thousands digit gated on `>= 1000`, which is what makes this a rate
   read as hundreds-point-tens-units. The live value at f24 draws first at
   y+399; then the other side's f1c draws at y+416, in the `next` digits with
   no tint while it is BELOW the target and in the `cw` digits tinted f14
   once it reaches it. The magic multipliers in the original are just /1000,
   /100 and /10 - cl regenerates them from the divisions. */
// FUNCTION: EZ2AC_UNPACKED 0x00419e70
void AudienceRatePanel::drawSolo()
{
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    if (f24 >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 1000 % 10], 311.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 100 % 10], 324.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(nextPoint, 333.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 10 % 10], 342.0f, f28 + 399.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 % 10], 355.0f, f28 + 399.0f, 0, 0, -1);

    if (f1c < f24)
    {
        if (f1c >= 1000)
            ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 311.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 324.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 333.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 342.0f, f28 + 416.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 355.0f, f28 + 416.0f, 0, 0, -1);
    }
    else
    {
        if (f1c >= 1000)
            ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 311.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 324.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 333.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 342.0f, f28 + 416.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 355.0f, f28 + 416.0f, 0, 0, f14);
    }
}

/* THE IDLE READOUT @0x0041a270 - the third variant, and NOT another shifted
   copy: it is 1680 bytes, not the 964 scan.py reports (the next annotated
   address falls inside it), and it draws THREE numbers where the other two
   draw two. The live value at f24 sits at y+366 in the `next` digits; both
   f1c and f20 follow at y+402 in `cw`, each untinted while below the target
   and tinted f14 once at or above it.

   Each two-arm number's FIFTH digit is written out in both arms here, and cl
   folds them back: the original pushes -1 at the end of the low arm and jumps
   into the high arm's last call, which is the push-merged-across-a-branch the
   cookbook says not to chase in the source.

   Its geometry is its own - the three rows do not share an x set:
   277/294/305/316/333, then 265/276/283/290/301, then 323/334/341/348/359. */
// FUNCTION: EZ2AC_UNPACKED 0x0041a270
void AudienceRatePanel::drawIdle()
{
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);

    if (f24 >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 1000 % 10], 277.0f, f28 + 366.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 100 % 10], 294.0f, f28 + 366.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(nextPoint, 305.0f, f28 + 366.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 / 10 % 10], 316.0f, f28 + 366.0f, 0, 0, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(next[f24 % 10], 333.0f, f28 + 366.0f, 0, 0, -1);

    if (f1c < f24)
    {
        if (f1c >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 265.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 276.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 283.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 290.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 301.0f, f28 + 402.0f, 0, 0, -1);
    }
    else
    {
        if (f1c >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 1000 % 10], 265.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 100 % 10], 276.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 283.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c / 10 % 10], 290.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f1c % 10], 301.0f, f28 + 402.0f, 0, 0, f14);
    }

    if (f20 < f24)
    {
        if (f20 >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 1000 % 10], 323.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 100 % 10], 334.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 341.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 10 % 10], 348.0f, f28 + 402.0f, 0, 0, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 % 10], 359.0f, f28 + 402.0f, 0, 0, -1);
    }
    else
    {
        if (f20 >= 1000)
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 1000 % 10], 323.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 100 % 10], 334.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cwPoint, 341.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 / 10 % 10], 348.0f, f28 + 402.0f, 0, 0, f14);
        ((KGraphics *)g_sub1b2f054)->drawTex(cw[f20 % 10], 359.0f, f28 + 402.0f, 0, 0, f14);
    }
}

/* GFBackGround::m41b370 @0x41b370 - fire the background script records whose
   time has come.

   The clock is the owner's, in quarter-ticks: `owner->clock->tick() >> 2`.
   The comparison is written as a measure/beat pair on BOTH sides, which is
   why the original divides by 0xc0 and multiplies straight back - numerically
   a no-op, but the source says `t / 0xc0` and `t % 0xc0` and then rebuilds the
   position, so cl emits the div and the mul.

   Each record carries three payload pointers; a payload whose first word is 5
   is latched and nothing more, anything else non-zero is latched and then
   dispatched through m41b2f0. Both arms call m45c840 first - written out twice
   rather than hoisted, which is what the original's two separate call sites
   say.

   Its only caller is GFBackGround's own @0x45cb04, which is why it is
   declared here despite sitting in BackGround's address range; the three
   background classes share this layout exactly. */
// FUNCTION: EZ2AC_UNPACKED 0x0041b370
void GFBackGround::m41b370(int a)
{
    unsigned int t = owner->clock->tick() >> 2;
    unsigned int meas = t / 0xc0;
    unsigned int beat = t % 0xc0;

    if (m_scriptIndex < m_scriptCount)
    {
        BgScriptRec *r = m_scriptCur;

        if (r->measure * 0xc0 + r->beat - 0xc < (int)(meas * 0xc0 + beat))
        {
            int i;

            m_scriptScratch = r;
            for (i = 0; i < 3; i++)
            {
                void *p = m_scriptScratch->payload[i];

                if (*(int *)p != 0)
                {
                    if (*(int *)p == 5)
                    {
                        m45c840(i);
                        m_scriptLatch[i] = m_scriptScratch->payload[i];
                    }
                    else
                    {
                        m45c840(i);
                        m_scriptLatch[i] = m_scriptScratch->payload[i];
                        m41b2f0(i);
                    }
                }
            }

            m_scriptIndex += 1;
            m_scriptCur = &m_script[m_scriptIndex];
        }
    }
}

/* CatchBackGround::m46b3e0 @0x46b3e0 - GFBackGround::m41b370's twin, and an
   exact one: 68 of 68 instructions match after masking the operands. The only
   differences are member offsets - the owner at +0x10 rather than +0x18, its
   clock at +0x168 rather than +0x14c, and the whole script block 0xc lower -
   plus this class's own pair of dispatch helpers. */
// FUNCTION: EZ2AC_UNPACKED 0x0046b3e0
void CatchBackGround::m46b3e0(int a)
{
    unsigned int t = owner->clock->tick() >> 2;
    unsigned int meas = t / 0xc0;
    unsigned int beat = t % 0xc0;

    if (m_scriptIndex < m_scriptCount)
    {
        GFBackGround::BgScriptRec *r = m_scriptCur;

        if (r->measure * 0xc0 + r->beat - 0xc < (int)(meas * 0xc0 + beat))
        {
            int i;

            m_scriptScratch = r;
            for (i = 0; i < 3; i++)
            {
                void *p = m_scriptScratch->payload[i];

                if (*(int *)p != 0)
                {
                    if (*(int *)p == 5)
                    {
                        m46b310(i);
                        m_scriptLatch[i] = m_scriptScratch->payload[i];
                    }
                    else
                    {
                        m46b310(i);
                        m_scriptLatch[i] = m_scriptScratch->payload[i];
                        m46b360(i);
                    }
                }
            }

            m_scriptIndex += 1;
            m_scriptCur = &m_script[m_scriptIndex];
        }
    }
}
