/* SongSelectDirector - the song-select screen's director (class named by its
   own KRuntimeClass @0x4aaed8; see include/songselect.h for the layout its
   destructor writes down).

   Only the teardown path is reconstructed so far: the destructor @0x431090, the
   scalar deleting destructor @0x438d10 that the vtable's slot 1 holds, and the
   two descriptor leaves. */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include "ezscreen.h"   /* SongSelectDirectorObj, g_objectRecord */
#include "songselect.h"
#include "ez2acapp.h"
#include "bigbase.h"

extern void Log(const char *fmt, ...);   /* 0x402880 (src/applog.cpp) */

/* The class's own two descriptors (storage in src/classinfo.cpp). */
extern KRuntimeClass classSongSelectDirector;  /* 0x4aaed8 */
extern int g_4abd14;                           /* src/gamestate.cpp - the difficulty, 1-based */
extern int  g_1b2eb70;                         /* src/resultctor.cpp */
extern int  g_1b2e89c;                         /* src/gamestate.cpp - the -hd page */
extern int  g_1b2ebf4;                         /* src/gamestate.cpp */
extern int  g_modeIndex;                       /* 0x1b2e548 (src/modename.cpp) */
extern char g_modeName[256];                   /* 0x1b2e680 (src/modename.cpp) */
extern EzObjectInfo  g_info_4aaee8;            /* 0x4aaee8 */

/* The rolling text slots and the cursor into them (src/modename.cpp,
   src/loops.cpp). */
extern char g_textSlots[32][128];
extern int  g_elemCount;

/* The buffer the "Channel Name" line is logged straight out of. Nothing else
   reconstructed so far touches it, so this is placeholder storage pinned at the
   original's address - same convention as src/globals.cpp. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2e728
char g_channelName[0x80];

// FUNCTION: EZ2AC_UNPACKED 0x00430b30
KRuntimeClass *SongSelectDirector::getRuntimeClass()
{
    return &classSongSelectDirector;
}

// FUNCTION: EZ2AC_UNPACKED 0x00430b40
DrawGroup *SongSelectDirector::getObjectInfo()
{
    return &g_info_4aaee8;
}

/* Put the view back the way the rest of the game expects it, drop the cabinet's
   outputs, log what the player left with, and let the forty text records go.

   The camera reset is eye {0,0,-1}, target {0,0,0}, up {0,1,0}, then a 1.0 ..
   10000.0 frustum at a 45-degree field of view. The graphics singleton is read
   at BOTH call sites - hoisting it into a local puts it in a register and both
   `mov ecx,ds:0x1b2f054` reloads vanish. */
// FUNCTION: EZ2AC_UNPACKED 0x00431090
SongSelectDirector::~SongSelectDirector()
{
    GVec3 eye;
    GVec3 at;
    GVec3 up;

    eye.x = 0.0f;
    eye.y = 0.0f;
    eye.z = -1.0f;
    at.x = 0.0f;
    at.y = 0.0f;
    at.z = 0.0f;
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;
    ((KGraphics *)g_sub1b2f054)->setCamera(&eye, &at, &up);
    ((KGraphics *)g_sub1b2f054)->setProjection(1.0f, 10000.0f, 0.785398185f);
    g_bigInstance->setFlags(0);
    // STRING: EZ2AC_UNPACKED 0x00491928
    Log("Selected song : %s", g_textSlots[g_elemCount]);
    // STRING: EZ2AC_UNPACKED 0x00491914
    Log("Channel Name : %s", g_channelName);
    // STRING: EZ2AC_UNPACKED 0x004918f8
    Log("SongSelectDirector::Delete");
}

// FUNCTION: EZ2AC_UNPACKED 0x00438d10
// SongSelectDirector::`scalar deleting destructor'

/* Slot 5 is the shared `xor eax,eax; ret 0xc`, ICF-folded across every class
   that has one; src/constants.cpp owns the address. */
// FUNCTION: EZ2AC_UNPACKED 0x00401770 folded
int SongSelectDirector::v5(int, int, int)
{
    return 0;
}

// VTABLE: EZ2AC_UNPACKED 0x0049193c
struct SongSelectDirector;

/* The two-digit readout in the top-right corner: the ones digit at x=566 and
   the tens digit at x=526, each 64 square, drawn twice over - once out of the
   glow run under an additive blend, then out of the solid run under the normal
   one. Three of the director's charts are refreshed first. */
// FUNCTION: EZ2AC_UNPACKED 0x00439dc0
void SongSelectDirector::m439dc0()
{
    m4391b0();
    m_bccc8.m403940();
    m_bf278.m403940();
    m_bf05c.m403940();

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_bd344[f_bfd48 % 10],
                                         566.0f, 0.0f, 64.0f, 64.0f, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_bd344[f_bfd48 / 10 % 10],
                                         526.0f, 0.0f, 64.0f, 64.0f, -1);

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 % 10],
                                         566.0f, 0.0f, 64.0f, 64.0f, -1);
    ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 / 10 % 10],
                                         526.0f, 0.0f, 64.0f, 64.0f, -1);
}

extern EzObjectInfo g_info_4aaee8;   /* 0x4aaee8 (src/classinfo.cpp) */

/* SongSelectDirector's new-plus-ctor factory - 0x20eb18 bytes, the biggest
   object the pool serves, and the `push` is what audits that size. */
// FUNCTION: EZ2AC_UNPACKED 0x004463e0
SongSelectDirectorObj *createSongSelectDirector()
{
    g_objectRecord = &g_info_4aaee8;
    return (SongSelectDirectorObj *)new SongSelectDirector;
}

/* @0x00430f50 - the song-select copy of the videoName trim, and the one that
   copies from a MEMBER rather than a global: entry `i` of the 0x374-stride
   array at this+0x5e2c4 leads with its name, and the trailing "-xx" (three,
   four or five back) is cut off in a shared buffer.

   The same trim as TotalResultDirector::m4544f0 and the three BackGround
   copies; the buffer sits with the function, as theirs do. */
// GLOBAL: EZ2AC_UNPACKED 0x037dcf40
static char g_songSelectName[260];

// FUNCTION: EZ2AC_UNPACKED 0x00430f50
char *SongSelectDirector::m430f50(int i)
{
    strcpy(g_songSelectName, entries[i].name);

    if (g_songSelectName[strlen(g_songSelectName) - 3] == '-')
        g_songSelectName[strlen(g_songSelectName) - 3] = 0;
    else if (g_songSelectName[strlen(g_songSelectName) - 4] == '-')
        g_songSelectName[strlen(g_songSelectName) - 4] = 0;
    else if (g_songSelectName[strlen(g_songSelectName) - 5] == '-')
        g_songSelectName[strlen(g_songSelectName) - 5] = 0;

    return g_songSelectName;
}

/* SongSelectDirector::m4332c0 @0x4332c0 - republish the selected entry's key
   when the selection moves. Shared by EIGHT of the mode-input handlers
   (@0x43aa2b, 0x43b5c4, 0x43c0c3, 0x43cc0d, 0x440807, 0x441236, 0x441c9e,
   0x44307a), and include/modeinput.h names it one of the five helpers that
   are the enabling work for that whole family.

   Both index computations are the 0x374 stride spelled out as shifts -
   `((8i - i) * 16 - i) * 2 - i` is 221 ints - so cl regenerates them from
   plain indexing. The second read is off a DIFFERENT table of the same stride
   at +0x49c; see the note on `keys` in the header for what is and is not
   established about it.

   Not in src/accessors.cpp, which is where both callees live: the original
   CALLS setCell and TAIL-CALLS reset, and cl would inline either one it could
   see. The global is in src/globals.cpp rather than here for the other TU
   rule - this file's g_songSelectName is read with a `strlen - 3` bias, and a
   new global landing on that biased address would break m430f50. */
extern int g_selectedKey;   /* src/globals.cpp - 0x1b2efd4 */

// FUNCTION: EZ2AC_UNPACKED 0x004332c0
void SongSelectDirector::m4332c0()
{
    f_bfddc = entries[f_bfdd8].f368;
    f_bfde0 = entries[f84].f368;

    if (f34 != 0)
        g_selectedKey = f_bfde0;

    if (f_bc57c >= 2 && f_bc57c <= 0x12 && f_bfddc != f_bfde0)
    {
        f_bfdd4 = f_bfde0;
        g_selectedKey = f_bfde0;
        m_beaf4.setCell(2, 0, f_bffc0[rows[f84].f264]);
        m_beaf4.reset();
    }
}

/* Drive the four option lamps 0xa..0xd from four flags. Every argument goes
   through the `xor/cmp/setne` shape of `!= 0`, and `this` is never read - but
   every caller loads it (the twelve mode-input handlers with `mov ecx,esi`), so
   it is a member. */
// FUNCTION: EZ2AC_UNPACKED 0x00434b20
void SongSelectDirector::setOptionLamps(int a, int b, int c, int d)
{
    g_bigInstance->setOutput(0xa, a != 0);
    g_bigInstance->setOutput(0xb, b != 0);
    g_bigInstance->setOutput(0xc, c != 0);
    g_bigInstance->setOutput(0xd, d != 0);
}

/* Chase `*v` toward `target` by the fraction `k` per call, snapping onto it
   once within half a unit. */
static void ease(float *v, float target, float k)
{
    *v = (target - *v) * k + *v;
    if (target - *v < 0.5f && target - *v > -0.5f)
        *v = target;
}

/* The banner behind the four level digits slides in (512 -> 0) once the
   selection is taken (phase 5) and out again otherwise; the difficulty
   cursor eases toward the current difficulty's column. Then the two banner
   textures, the four levels of the selected entry with the current one
   highlighted, and the level chart's position. */
// FUNCTION: EZ2AC_UNPACKED 0x004343b0
void SongSelectDirector::drawLevels()
{
    float target = 512.0f;
    float y;
    Obj402860 *ch;

    if (f_bfdb4 != 5)
        target = 0.0f;
    ease(&f_c0470, target, 0.2f);
    ease(&f_c0474, (float)(g_4abd14 - 1) * 80.0f, 0.33333334f);

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(f_c0468, -5.0f, 157.0f - (512.0f - f_c0470), 645.0f, 0.0f, -1);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    ((KGraphics *)g_sub1b2f054)->drawTex(f_c0464, 64.0f, 168.0f - (512.0f - f_c0470), 0.0f, 0.0f, -1);

    switch (g_4abd14 - 1)
    {
    case 0:
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[0].level], 169.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xff46e1ff, 0xf53cd7f5);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[1].level], 249.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[2].level], 329.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[3].level], 409.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        break;
    case 1:
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[0].level], 169.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[1].level], 249.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xff46e1ff, 0xf53cd7f5);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[2].level], 329.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[3].level], 409.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        break;
    case 2:
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[0].level], 169.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[1].level], 249.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[2].level], 329.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xff46e1ff, 0xf53cd7f5);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[3].level], 409.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        break;
    case 3:
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[0].level], 169.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[1].level], 249.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[2].level], 329.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff, 0x64646464);
        ((KGraphics *)g_sub1b2f054)->m404950(f_c040c[(int)rows[f84].diff[3].level], 409.0f, 207.0f - (512.0f - f_c0470), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xff46e1ff, 0xf53cd7f5);
        break;
    }

    ch = &m_20cd2c;
    ch->m403940();
    y = 257.0f - (512.0f - f_c0470);
    ch->f214 = f_c0474 + 193.0f;
    ch->f218 = y;
}

/* The two-digit counter at the top right, from f_bfd58 once the selection is
   taken and f_bfda4 is set, from f_bfd48 otherwise.

   NESTED, with the f_bfd48 body written TWICE. The original hoists
   setBlend's set-up (`mov ecx,ds:g_sub1b2f054` / `push 2` / `push 2`) into
   the slot between the first `cmp` and its `jne`, keeping only the `call`
   in each arm. That is cl's head-merge applied twice: the inner if/else
   hoists the common head into its own test block, the outer if/else then
   finds that both of ITS successors open with it and hoists it again past
   its own compare - and cross-jumping folds the two f_bfd48 copies back
   into one. The flat `if (f_bfdb4 != 5 || f_bfda4 == 0) ... else ...` has a
   block reachable from two tests, so nothing can be hoisted (91.06%).
   Found from scratchpad/perm's check.py --diff, 2026-10-09. */
// FUNCTION: EZ2AC_UNPACKED 0x00434a30
void SongSelectDirector::drawCounter()
{
    if (f_bfdb4 == 5)
    {
        if (f_bfda4 != 0)
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd58 % 10], 590.0f, 20.0f, 32.0f, 32.0f, -1);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd58 / 10 % 10], 568.0f, 20.0f, 32.0f, 32.0f, -1);
        }
        else
        {
            ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 % 10], 590.0f, 20.0f, 32.0f, 32.0f, -1);
            ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 / 10 % 10], 568.0f, 20.0f, 32.0f, 32.0f, -1);
        }
    }
    else
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 % 10], 590.0f, 20.0f, 32.0f, 32.0f, -1);
        ((KGraphics *)g_sub1b2f054)->drawTex(m_bd31c[f_bfd48 / 10 % 10], 568.0f, 20.0f, 32.0f, 32.0f, -1);
    }
}

/* Read the selected song's rank file for this mode - "sound\\e_rank_..." for
   the EZ2CATCH pass with the flag at 0x1b2eb70 set, "sound\\rank_..." otherwise -
   creating a default one of five "EZ2AC_FN" / 0 entries when there is none.
   The path is built the same way twice, as the original does. */
// FUNCTION: EZ2AC_UNPACKED 0x00434b80
void SongSelectDirector::loadRankFile()
{
    char path[260];
    FILE *f;
    int i;

    if (g_1b2eb70 != 0)
    {
        if (g_modeIndex == 10)
        {
            sprintf(path, "sound\\e_rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
        }
        else
        {
            // STRING: EZ2AC_UNPACKED 0x00491c68
            sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
        }
    }
    else
    {
        sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
    }
    if (_access(path, 0) == -1)
    {
        f = fopen(path, "wb");
        for (i = 0; i < 5; i++)
        {
            int j;
            for (j = 0; j < 8; j++)
                m_rankNames[i][j] = "EZ2AC_FN"[j];
            m_rankScores[i] = 0;
        }
        fwrite(m_rankNames, 8, 5, f);
        fwrite(m_rankScores, 4, 5, f);
        fclose(f);
    }
    if (g_1b2eb70 != 0)
    {
        if (g_modeIndex == 10)
        {
            sprintf(path, "sound\\e_rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
        }
        else
        {
            sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
        }
    }
    else
    {
        sprintf(path, "sound\\rank_%s_%s.bin", g_modeName, g_textSlots[g_elemCount]);
    }
    f = fopen(path, "rb");
    fread(m_rankNames, 8, 5, f);
    fread(m_rankScores, 4, 5, f);
    fclose(f);
}

/* Eight NUL bytes in .rdata: the third of the three "empty" names the
   ranking reader replaces, beside "        " and "AAAAAAAA". Placeholder
   storage pinned at the original's address. */
// GLOBAL: EZ2AC_UNPACKED 0x00491960
static const char kEmptyRankName[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/* Read `song`'s ranking file for this mode - the -hd one on the hard page -
   into m_rankingNames / m_rankingScores. Every entry starts as EZ2AC_FN / 0;
   the eight names are read as 50-byte records (which overrun into their
   neighbours, as the original does), a name of spaces, NULs or As is reset,
   and the eight scores follow at +0x318. */
// FUNCTION: EZ2AC_UNPACKED 0x00431420
void SongSelectDirector::loadRankingFile(char *song)
{
    char path[260];
    FILE *f;
    int i;

    if (g_1b2e89c == 1)
    {
        // STRING: EZ2AC_UNPACKED 0x004919a8
        sprintf(path, "system\\ranking\\ranking_%s_%s-hd.bin", g_modeName, song);
    }
    else
    {
        sprintf(path, "system\\ranking\\ranking_%s_%s.bin", g_modeName, song);
    }
    Log("%s", path);
    f = fopen(path, "rb");
    for (i = 0; i < 8; i++)
    {
        // STRING: EZ2AC_UNPACKED 0x00491978
        memcpy(m_rankingNames[i], "EZ2AC_FN", 8);
    }
    for (i = 0; i < 8; i++)
        m_rankingScores[i] = 0;
    if (f != 0)
    {
        for (i = 0; i < 8; i++)
        {
            fread(m_rankingNames[i], 0x32, 1, f);
            // STRING: EZ2AC_UNPACKED 0x0049196c
            if (memcmp(m_rankingNames[i], "        ", 8) == 0 ||
                memcmp(m_rankingNames[i], kEmptyRankName, 8) == 0 ||
                // STRING: EZ2AC_UNPACKED 0x00491954
                memcmp(m_rankingNames[i], "AAAAAAAA", 8) == 0)
            {
                memcpy(m_rankingNames[i], "EZ2AC_FN", 8);
            }
        }
        fseek(f, 0x318, SEEK_SET);
        for (i = 0; i < 8; i++)
            fread(&m_rankingScores[i], 4, 1, f);
        fclose(f);
    }
}

#include <stdlib.h>      /* atof, atoi */
#include "kprofile.h"

/* Load a course's channel list from system\radiomix\Stage_<n>.ini.

   87.39%, STRUCTURALLY COMPLETE, parked on the big-function register wall.
   Every call, string, field, loop and branch is present and in the original's
   order (the frame map was recovered with scratchpad/esptrace.py: F0x18 the
   KProfile, F0x34 the path/key buffer, F0x1bc the 0x2000 tokenise scratch, and
   the five per-channel walkers rowKey/nameDst/fdst/lvl/i). The residual is
   register allocation the source cannot steer:
     - the ZERO register and the `stage` argument are swapped esi<->edi from
       the original (an assignment coin - the zero is used in five places and
       cl ranks it into the other callee-saved register);
     - [RETRACTED 2026-08-25] this bullet used to read "every inline
       `scratch[strlen(scratch)]=0` writes `[esp+eax+0x1c8]` where the original
       writes `[esp+eax+0x1c9]` - the known inline-strlen register-pair pick",
       and that was a MISREADING. The one-byte displacement was not a register
       artifact at all: the original writes at `strlen + 1`, a second NUL after
       the terminator, which is the sentinel the tokenizer below needs. It was
       a real bug in the reconstruction and is fixed. When a displacement is
       off by exactly the size of one element, check the INDEX before blaming
       the register allocator;
     - `f7c++` in the split loop is an in-memory `add` where the original keeps
       the count in a register for the stride (routing it through a local
       regressed the whole function 85->66, so it is left).
   The strcpy calls (not hand-written loops) and the indexed float writes (not
   pointer walks) are what took it from 70% to here. What remains is the
   esi/edi coin; the semantics are settled, and the note above lists what has
   since been measured and rejected.

   The `channel_normal` key is a ';'-separated list of channel names, split
   into rows[]. Each channel then has "<name>_grow" (default 8;8;9;10),
   "<name>_goal" (default 10;17;26;35) and "<name>_level" keys read, the grow
   and goal parsed as four floats each into the per-channel float arrays at
   0xda484(+0x640) and the level atoi'd into the int array at 0xdb104; the
   channel's own name field is split into five sub-names at 0xc0e44. A second
   pass loads each channel's legacy disc/info textures, and the tail primes the
   preview sound for the current cursor. The interleaved per-channel arrays are
   addressed by `this + base + i*stride` the way the original does. */
/* 85.23% -> 88.62%. Two things in it were WRONG, not merely mis-shaped:

   - the guard's terminator is `scratch[strlen(scratch) + 1] = 0`, not
     `[strlen(scratch)]`. The tokenizer below walks `tok += strlen(tok) + 1`
     and stops on `tok[0] == 0`, so the sentinel it needs is a SECOND NUL
     after the string's own - writing at [strlen] only rewrites the
     terminator already there and leaves the walk running off the end of a
     string with no ';' in it. Five sites; the original's
     `mov [esp+eax+0x1c9],cl` against our +0x1c8 is what says so.
   - each `p = scratch` and the tokenizer's `tok = scratch` belong OUTSIDE
     their `if (scratch[0] != 0)` guard. The original computes the pointer
     between the guard's `cmp` and its `je` - `cmp byte [buf],cl` /
     `lea eax,[buf]` / `je` - which is the documented tell that the source
     assigns it before the test. Written as `if (buf[0]) for (p = buf; ...)`
     the `lea` lands after the branch and every displacement in the loop
     moves with it.

   WHAT IS LEFT, AND WHAT NOT TO RE-TRY. The zero register is swapped
   (original esi, ours edi) with `stage` taking the other, and the three
   `f7c/f84/f8c = 0` stores sit after the sprintf where the original has
   them before the `je` - and cl NORMALISES that: moving the three
   assignments above the `if (m_sndPreview)` in the source changes nothing
   at all, so it is not statement order. Also measured and rejected:
   storing the incremented row count back before the strcpy rather than
   after (59.5%), and rewriting the two four-element atof loops as an
   explicit down-counter over a walking pointer (no change, 88.62%) -
   the down-counter rule does not bite here because the index is dead
   either way. */
// FUNCTION: EZ2AC_UNPACKED 0x00433380
void SongSelectDirector::loadStageCourse(int stage)
{
    char scratch[0x2000];
    char path[0x108];
    char key[0x80];
    char *base = (char *)this;
    char *lvl;
    char *rowKey;
    char *nameDst;
    char *fdst;

    memset(scratch, 0, sizeof(scratch));
    if (m_sndPreview)
        m_sndPreview->q5();
    int i;
    f7c = 0;
    f84 = 0;
    f8c = 0;

    sprintf(path, "system\\radiomix\\Stage_%d.ini", stage);
    KProfile parser(0xc0000, 0);
    parser.Load(path);

    sprintf(key, "channel_normal", stage);
    parser.getStr(key, scratch);
    scratch[strlen(scratch) + 1] = 0;
    {
        char *p;

        p = scratch;
        if (*p != 0)
        {
            do
            {
                if (*p == ';')
                    *p = 0;
                p++;
            }
            while (*p);
        }
    }
    {
        char *tok;

        tok = scratch;
        if (*tok != 0)
        {
            do
            {
                strcpy(rows[f7c].name, tok);
                f7c++;
                tok += strlen(tok) + 1;
            } while (tok[0] != 0);
        }
    }

    if (f7c > 0)
    {
        lvl = base + 0xdb104;
        rowKey = base + 0x238;
        fdst = base + 0xda484;
        nameDst = base + 0xc0e44;
        i = 0;
        do
        {
            char *p;
            char *s;
            char *d;
            int k;

            parser.getStr(rowKey, scratch);
            scratch[strlen(scratch) + 1] = 0;
            p = scratch;
            if (*p != 0)
            {
                do
                {
                    if (*p == ';')
                        *p = 0;
                    p++;
                }
                while (*p);
            }
            s = scratch;
            g_1b2ebf4 = f7c;

            d = nameDst;
            for (k = 0; k < 5; k++)
            {
                strcpy(d, s);
                s += strlen(s) + 1;
                d += 0x104;
            }

            sprintf(path, "%s_grow", rowKey);
            memcpy(scratch, "8;8;9;10", 9);
            parser.getStr(path, scratch);
            scratch[strlen(scratch) + 1] = 0;
            p = scratch;
            if (*p != 0)
            {
                do
                {
                    if (*p == ';')
                        *p = 0;
                    p++;
                }
                while (*p);
            }
            s = scratch;
            for (k = 0; k < 4; k++)
            {
                *(float *)(fdst + 0x640 + k * 4) = (float)atof(s);
                s += strlen(s) + 1;
            }

            sprintf(path, "%s_goal", rowKey);
            memcpy(scratch, "10;17;26;35", 12);
            parser.getStr(path, scratch);
            scratch[strlen(scratch) + 1] = 0;
            p = scratch;
            if (*p != 0)
            {
                do
                {
                    if (*p == ';')
                        *p = 0;
                    p++;
                }
                while (*p);
            }
            s = scratch;
            for (k = 0; k < 4; k++)
            {
                *(float *)(fdst + k * 4) = (float)atof(s);
                s += strlen(s) + 1;
            }

            sprintf(path, "%s_level", rowKey);
            parser.getStr(path, scratch);
            scratch[strlen(scratch) + 1] = 0;
            *(int *)lvl = atoi(scratch);
            // STRING: EZ2AC_UNPACKED 0x00491bc0
            Log("Course Name = %s", path);
            // STRING: EZ2AC_UNPACKED 0x00491bac
            Log("Course Level = %02d", *(int *)lvl);

            /* the original's order for the per-row steps */
            nameDst += 0x410;
            rowKey += 0x374;
            i++;
            lvl += 4;
            fdst += 0x10;
        } while (i < f7c);
    }

    for (i = 0; i < f7c; i++)
    {
        char *row = base + 0x238 + i * 0x374;
        // STRING: EZ2AC_UNPACKED 0x00491b88
        sprintf(path, "system\\channel_disc\\legacy\\%s.bmp", row);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)(row - 0x10c), 0);
        // STRING: EZ2AC_UNPACKED 0x00491b64
        sprintf(path, "system\\channel_info\\legacy\\%s.bmp", row);
        ((KGraphics *)g_sub1b2f054)->loadTexture(path, (KTexture **)(base + 0x20d9d4 + i * 4), 0);
    }

    // STRING: EZ2AC_UNPACKED 0x00491b44
    sprintf(path, "system\\preview_legacy\\%s.wav", rows[f84].name);
    if (g_sub1b2eae4->loadSample((EzSample *)m_sndPreview, path) == 0)
        m_sndPreview = (EzSoundObject *)g_sub1b2eae4->createSample(
            "System\\Preview\\0Blank.wav", 1, 0);
    f84 = 0;
    f34 = 1;
    m_sndMove->q5();
    m_sndMove->q4(0);
    f_20e894 = 0;
}

/* The table's x origin, 213.0f @0x491b0c, is a `static float` rather than a
   literal: as a rematerializable pool constant cl evaluates the digit block
   (the constant's LAST use) with `wx - 55` in xmm0 and the origin in xmm1, the
   reverse of the original and of every earlier block; a load from its own
   symbol keeps the origin first everywhere. The class's own draws and
   SongSelectDirector::update2's two clamps (@0x4474ae/@0x448376) read it. */
// GLOBAL: EZ2AC_UNPACKED 0x00491b0c
float k_tableOriginX = 213.0f;

/* Draw the ranking table at (x, y): the two header textures, then per row the
   eight-character name (right to left, 8 px per glyph, 0 and ' ' skipped) and
   the seven-digit score (MSD first). The colour is the brightness byte
   replicated into all four channels; the x origin is f_20c69c - 213. Rows are
   indexed (`m_rankingNames[i][j]`, `m_rankingScores[i]`) and cl reduces both to
   walking pointers - explicit pointers get the two frame slots swapped. */
// FUNCTION: EZ2AC_UNPACKED 0x00432bb0
void SongSelectDirector::drawRankTable(float x, float y)
{
    float cx = x + 24.0f;
    float wx = cx;
    float ry = y;
    float hy;
    int c;
    int b;
    int i;
    int j;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    x -= 72.0f;
    hy = y - 21.0f;
    b = f_20c698;
    c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bfe9c[12], f_20c69c - k_tableOriginX + x - 41.0f, hy, 0.0f, 0.0f, c);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    b = f_20c698;
    c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bfe9c[13], f_20c69c - k_tableOriginX + x - 41.0f, hy, 0.0f, 0.0f, c);

    for (i = 0; i < 8; i++)
    {
        for (j = 7; j >= 0; j--)
        {
            if (m_rankingNames[i][j] != 0 && m_rankingNames[i][j] != ' ')
            {
                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                b = f_20c698;
                c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
                ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_20be8c[f_20c240[m_rankingNames[i][j]]],
                                                     f_20c69c - k_tableOriginX + wx - 41.0f, ry + 17.0f, 16.0f, 16.0f, c);
            }
            wx -= 8.0f;
        }
        wx = cx;
        ry += 37.0f;
        {
            int div = 1000000;
            int k;
            for (k = 7; k > 0; k--)
            {
                int d = m_rankingScores[i] / div;
                while (d % 10 != d)
                    d = d % 10;
                ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
                b = f_20c698;
                c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
                ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_20bef4[d],
                                                     f_20c69c - k_tableOriginX + (wx - 55.0f) - 41.0f, y + 31.0f, 16.0f, 16.0f, c);
                wx += 8.0f;
                div = div / 10;
            }
        }
        wx = cx;
        y += 37.0f;
    }
}

/* The course-mode ranking table: course `course`'s stage `stage` at (x, y) -
   the two header textures always, then when `body` is set five names (right
   to left, glyphs the char->glyph map has; -1 skips) and five seven-digit
   scores, all in plain white. The score rows write the row's y INTO THE `x`
   PARAMETER (`x = ry + 14.0f`) - that is what puts it in x's frame slot and the
   row counter in y's; a fresh local gets the two slots the other way round.
   `div` is initialised before that store, which is where cl schedules it. */
// FUNCTION: EZ2AC_UNPACKED 0x00433d50
void SongSelectDirector::drawCourseRankTable(float x, float y, int course, int stage, char body)
{
    float cx = x + 24.0f;
    float wx = cx;
    float ry0 = y - 10.0f;
    float ry = ry0;
    float hy;
    int c;
    int b;
    int i;
    int j;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    x -= 72.0f;
    hy = y - 48.0f;
    b = f_20c698;
    c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bfe9c[12], k_tableOriginX - f_20c69c + x, hy, 0.0f, 0.0f, c);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    b = f_20c698;
    c = b; c = c << 8 | b; c = c << 8 | b; c = c << 8 | b;
    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_bfe9c[13], k_tableOriginX - f_20c69c + x, hy, 0.0f, 0.0f, c);

    if (body)
    {
        ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
        for (i = 0; i < 5; i++)
        {
            for (j = 7; j >= 0; j--)
            {
                int g = f_20c240[rows[course].rankNames[stage][i][j]];
                if (g != -1)
                    ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_20be8c[g], k_tableOriginX - f_20c69c + wx, ry, 16.0f, 16.0f, -1);
                wx -= 8.0f;
            }
            wx = cx;
            ry += 37.0f;
        }
        ry = ry0;
        for (i = 0; i < 5; i++)
        {
            int div = 1000000;
            int k;
            x = ry + 14.0f;
            for (k = 7; k > 0; k--)
            {
                int d = rows[course].rankScores[stage][i] / div;
                while (d % 10 != d)
                    d = d % 10;
                ((KGraphics *)g_sub1b2f054)->drawTex((KTexture *)f_20bef4[d], k_tableOriginX - f_20c69c + (wx - 55.0f), x, 16.0f, 16.0f, -1);
                wx += 8.0f;
                div = div / 10;
            }
            wx = cx;
            ry += 37.0f;
        }
    }
}

/* The song wheel. Entry i sits at i*100 - f_c4, wrapped into +-50*f7c, and
   only the ones within +-150 of centre are drawn (drawEntry @0x432ac0 - the
   Obj432AC0 view of this class in src/kgraphicsquad.cpp turns the position
   into a sine). Then the three cell tables are refreshed - the third seeks to
   row 15 once it is done - and the ranking table drawn at (120, 89). Note the
   wrap-up is `100n + pos`, constant first, which is what the movaps records. */
struct Obj432AC0 { void drawEntry(int i, float angle); };

// FUNCTION: EZ2AC_UNPACKED 0x00433180 folded
void SongSelectDirector::drawWheel()
{
#include "drawwheel433180.inl"
}

/* Modes 8 and 9 draw the same wheel through their own copies, folded onto
   0x433180 with drawWheel (mode 6) - see m439f00's note in
   src/songselectpage.cpp for why the arms prove it. */
// FUNCTION: EZ2AC_UNPACKED 0x00433180 folded
void SongSelectDirector::wheelMode8()
{
#include "drawwheel433180.inl"
}

// FUNCTION: EZ2AC_UNPACKED 0x00433180 folded
void SongSelectDirector::wheelMode9()
{
#include "drawwheel433180.inl"
}

/* The list panel: a 645x485 backdrop, fifteen row textures 20 px apart, and
   the cursor bar at the eased row y - it snaps when within half a pixel
   (double literals, hence the x87 compares) and closes a third of the gap
   otherwise. */
// FUNCTION: EZ2AC_UNPACKED 0x00432f00
void SongSelectDirector::drawListPanel()
{
    int i;
    float t;
    float d;

    ((KGraphics *)g_sub1b2f054)->setBlend(9, 6);
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bc5c8, -5.0f, -5.0f, 645.0f, 485.0f, -1);
    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    for (i = 0; i < 15; i++)
        ((KGraphics *)g_sub1b2f054)->drawTex(f_bc58c[i], 17.0f, (float)i * 20.0f + 17.0f, 0.0f, 0.0f, -1);
    t = (float)f_d4 * 20.0f;
    d = t - f_e8;
    if (d < 0.5 && d > -0.5)
        f_e8 = t;
    else
        f_e8 = d * 0.33333334f + f_e8;
    ((KGraphics *)g_sub1b2f054)->drawTex(f_bc5cc, 11.0f, f_e8 + 27.0f, 0.0f, 0.0f, -1);
}

/* drawWheel with the list panel on top when f_dc is set - the same body,
   written out again (the original keeps two copies), plus the tail call. */
// FUNCTION: EZ2AC_UNPACKED 0x00433030
void SongSelectDirector::drawWheelWithList()
{
    int i;
    float pos;
    Obj402860 *t;

    ((KGraphics *)g_sub1b2f054)->setBlend(5, 6);
    for (i = 0; i < f7c; i++)
    {
        pos = (float)(i * 100) - f_c4;
        if (pos > (float)(f7c * 50))
            pos -= (float)(f7c * 100);
        if ((float)(-f7c * 50) > pos)
            pos = (float)(f7c * 100) + pos;
        if (pos > -150.0f && 150.0f > pos)
            ((Obj432AC0 *)this)->drawEntry(i, pos);
    }
    m_bcaac.m403940();
    m_be6bc.m403940();
    t = &m_20d7b8;
    t->m403940();
    if (t->isDone())
        t->seek(15);
    if (f_20c0e0 == 1)
        m_20d380.m403940();
    else if (f_20c0e0 == 0)
        m_20d164.m403940();
    drawRankTable(120.0f, 89.0f);
    if (f_dc != 0)
        drawListPanel();
}

/* The genre strip: the 47 labels sit 100 units apart on a wheel scrolled by
   f_bc584 (wrapped into +-2350), drawn at x = pos*0.45 + 10 when within
   (-1500, 2500), the current one (f_bc574 <- f_bc57c) in the highlight tint.
   Then the mode label for g_elemCount at (138, 35). Two calls, not a ternary
   colour - cl cross-jumps them back into the original's one call site. */
// FUNCTION: EZ2AC_UNPACKED 0x00432200
void SongSelectDirector::drawGenreStrip()
{
    int i;
    float pos;

    ((KGraphics *)g_sub1b2f054)->setBlend(2, 2);
    f_bc574 = f_bc57c;
    for (i = 0; i < 47; i++)
    {
        pos = (float)i * 100.0f - f_bc584;
        if (pos > 2350.0f)
            pos -= 4700.0f;
        if (-2350.0f > pos)
            pos += 4700.0f;
        if (2500.0f > pos && pos > -1500.0f)
        {
            if (i == f_bc574)
                ((KGraphics *)g_sub1b2f054)->drawTex(f_bc4b8[i], pos * 0.45f + 10.0f, 58.0f, 0.0f, 0.0f, 0xff46e1ff);
            else
                ((KGraphics *)g_sub1b2f054)->drawTex(f_bc4b8[i], pos * 0.45f + 10.0f, 58.0f, 0.0f, 0.0f, 0xff1e4678);
        }
    }
    ((KGraphics *)g_sub1b2f054)->drawTex(f_c0084[g_elemCount], 138.0f, 35.0f, 0.0f, 0.0f, -1);
}

