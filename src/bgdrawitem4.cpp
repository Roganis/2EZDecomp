/* BackGround::drawItem4 @0x45c730 and CatchBackGround::drawItem4 @0x46aed0 -
   the kind-4 draw of the two backgrounds' draw lists, and the counterpart of
   the kind-5 drawItem pair in src/backgrounds.cpp.

   The two are BYTE-IDENTICAL (100.0% by mnemonic sequence, and the original's
   ICF did not fold them because each addresses its own class's flag block at
   +0x101bb8) - so they are written out twice here, exactly as the kind-5 pair
   is.

   What it does: wind the item's rotation on by its per-tick rate, and when it
   has a wave rate, bounce the phase between 1.5 and 3.5 - down while the flag
   at +0x30 is set, up otherwise, flipping the flag at each end. Then the flag
   mesh is updated, the item's blend and texture are bound, the matrix is
   reset and given the phase as a Z translation at (-1.9, -1.9), rotated by
   the wound angle, and the flag is drawn. */

#include "panels.h"
#include "backgrounds.h"
#include "ez2acapp.h"

/* The matrix ops under the name src/accessors.cpp defines them with. */
struct Obj40BBB0
{
    void loadIdentity();                        /* 0x40bbb0 */
    void translate(float x, float y, float z);  /* 0x40bbd0 */
    void rotateZ(int angle);                    /* 0x40bc70 */
};

/* And the sub-op under the name src/com_wrappers.cpp defines it with. */
struct Obj40BB00 { void doSub(); };             /* 0x40bb00 */

// FUNCTION: EZ2AC_UNPACKED 0x0045c730
void BackGround::drawItem4(BgItem *it)
{
    it->f28 += it->f08;

    if (it->f0c > 0.0f)
    {
        if (it->f30 != 0)
        {
            it->f2c = it->f2c - it->f0c;
            if (1.5f > it->f2c)
                it->f30 = 0;
        }
        else
        {
            it->f2c = it->f2c + it->f0c;
            if (it->f2c > 3.5f)
                it->f30 = 1;
        }
    }

    ((KGraphics *)g_sub1b2f054)->m406900(&m_flag);
    ((KGraphics *)g_sub1b2f054)->setMaterial(it->f20);
    ((KGraphics *)g_sub1b2f054)->setBlend(it->blendSrc, it->blendDst);
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, it->tex);
    ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
    ((KGraphics *)g_sub1b2f054)->m408130(0);
    ((Obj40BBB0 *)g_sub1b2f054)->loadIdentity();
    ((Obj40BBB0 *)g_sub1b2f054)->translate(-1.9f, -1.9f, it->f2c);
    ((Obj40BBB0 *)g_sub1b2f054)->rotateZ(it->f28);
    ((Obj40BB00 *)g_sub1b2f054)->doSub();
    ((KGraphics *)g_sub1b2f054)->drawMesh((MeshDraw *)&m_flag);
}

// FUNCTION: EZ2AC_UNPACKED 0x0046aed0
void CatchBackGround::drawItem4(BgItem *it)
{
    it->f28 += it->f08;

    if (it->f0c > 0.0f)
    {
        if (it->f30 != 0)
        {
            it->f2c = it->f2c - it->f0c;
            if (1.5f > it->f2c)
                it->f30 = 0;
        }
        else
        {
            it->f2c = it->f2c + it->f0c;
            if (it->f2c > 3.5f)
                it->f30 = 1;
        }
    }

    ((KGraphics *)g_sub1b2f054)->m406900(&m_flag);
    ((KGraphics *)g_sub1b2f054)->setMaterial(it->f20);
    ((KGraphics *)g_sub1b2f054)->setBlend(it->blendSrc, it->blendDst);
    ((KGraphics *)g_sub1b2f054)->m4080c0(0, it->tex);
    ((KGraphics *)g_sub1b2f054)->m4080c0(1, 0);
    ((KGraphics *)g_sub1b2f054)->m408130(0);
    ((Obj40BBB0 *)g_sub1b2f054)->loadIdentity();
    ((Obj40BBB0 *)g_sub1b2f054)->translate(-1.9f, -1.9f, it->f2c);
    ((Obj40BBB0 *)g_sub1b2f054)->rotateZ(it->f28);
    ((Obj40BB00 *)g_sub1b2f054)->doSub();
    ((KGraphics *)g_sub1b2f054)->drawMesh((MeshDraw *)&m_flag);
}
