#include <stdio.h>
#include "kparser.h"
#include "systemmenu.h"
#include "directors.h"

extern int g_advSound;    /* 0x16963f8 (src/settings.cpp) */
extern int g_1696400;     /* 0x1696400 */
extern void *g_1b2e844;   /* 0x1b2e844 */
extern EzObjectInfo g_info_4afb70;   /* 0x4afb70 */

/* TestGameOptionSink::onMenu @0x474ee0 - the test menu's game-option page,
   integrated from wip/testgameoption-onmenu.cpp at 78.15%.

   EVERY INSTRUCTION IS RIGHT: the frame, the EH states, the switch lowering,
   the call sequence, the field offsets and the whole tail from 0x47504b are
   byte-exact. What differs is one REGISTER ROTATION through the middle third -
   the original runs {eax, edx, ecx} where cl runs {edx, ecx, ...} - and it all
   cascades from a single choice at 0x474fa4, where the original loads
   [ecx+0x3940] into eax and cl loads it into edx.

/* Row 2 resets both settings to their defaults; row 3 commits them - fold the
   list index back into the raw coins-per-credit value, write both keys, save
   and re-read the .ini, publish them to the engine, and leave the page. */
// FUNCTION: EZ2AC_UNPACKED 0x00474ee0
void TestGameOptionSink::onMenu(int, int what, int)
{
    TestGameOptionSinkFields *f = (TestGameOptionSinkFields *)this;

    if (what == 2)
    {
        f->m_coinIndex = 2;
        f->m_advSound = 0;
        f->f0c = 1;
        f->m_menu->m_rows[0].enabled = 1;
        f->m_menu->m_rows[0].value = f->m_coinIndex;
        f->m_menu->m_rows[1].value = f->m_advSound;
    }
    else if (what == 3)
    {
        char name[260];
        DrawNode *node;

        SystemMenu *menu = f->m_menu;

        f->m_coinIndex = menu->m_rows[0].value;
        switch (f->m_coinIndex)
        {
        case 1:
            f->m_creditCoins = 1;
            break;
        case 2:
            f->m_creditCoins = 4;
            break;
        case 3:
            f->m_creditCoins = 5;
            break;
        default:
            f->m_creditCoins = 0;
            break;
        }
        f->m_advSound = menu->m_rows[1].value;

        sprintf(name, "%s.ini", g_app->className());
        g_iniProfile->setInt("CreditCoins", f->m_creditCoins);
        g_iniProfile->setInt("AdvSound", f->m_advSound);
        g_iniProfile->writeFile(name);

        {
            KParser parser;

            parser.m4133d0(name);

            g_bigInstance->need = f->m_creditCoins;
            g_advSound = f->m_advSound;
            g_1696400 = f->f14;

            delete f->m_menu;
            node = 0;
            ((Obj401780 *)g_1b2e844)->nextInstance(&g_info_4afb70, &node);
            ((TestModeSink *)((char *)node + 0x10))->onMenu(
                2, (int)((char *)this - 0x10), 0);
            delete (TestGameOption *)((char *)this - 0x10);
        }
    }
}
