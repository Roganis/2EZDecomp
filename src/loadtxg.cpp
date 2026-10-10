#include <string.h>
#include "ez2acapp.h"
#include "kfile.h"
#include "kprofile.h"

/* KGraphics::loadTXG @0x408f70 - the texture ATLAS loader, integrated from
   wip/loadtxg.cpp at 78.71%.

   STRUCTURALLY EXACT: every call, every branch, the aligned /GS frame, the
   0x248 frame size, the two 0x104 buffers, the EH states and the whole float
   block. The one difference is which value wins a callee-saved register in the
   loop, and it cascades into about thirty instructions:

     original   ebx = 0, and `i` is SPILLED to [esp+0x28] - so the tests read
                `cmp [count],ebx`, `push ebx`, `cmp eax,ebx`, and the loop tail
                is `mov eax,[i]; add eax,1; cmp eax,[count]; mov [i],eax; jl`
     ours       ebx = i, and the zero is spelled as immediates

   This is CLAUDE.md's cited example of the zero-register-versus-counter coin,
   and the later refinement of that rule does not rescue it: the check is to
   confirm the original is comparing against what you think it is, and here it
   is - `cmp [count],ebx` with ebx zero really is `count == 0`, not a
   misread constant. Both builds allocate the same number of callee-saved
   registers; only the assignment differs.

/* A .txg file describes ONE texture atlas and the named sprites cut out of it:
   the magic "TXG0", two unused header words, the texture's own file name, then
   a count and that many records of { left, top, right, bottom, name }. Each
   record becomes a KTextureNode in the graphics profile that shares the atlas
   surface and carries the rectangle in pixels (w/h) and normalized (u0..v1).
   A name already in the profile is skipped, not replaced. */
// FUNCTION: EZ2AC_UNPACKED 0x00408f70
int KGraphics::loadTXG(char *path)
{
    KFile f;
    int count;
    int left;
    int right;
    int top;
    int i;
    int magic;
    int u1, u2;
    char name[0x104];
    char texName[0x104];
    KTexture *tex;
    int bottom;

    if (f.open(path, 1) == 0)
    {
        return 0;
    }

    f.read(&magic, 4);
    if (magic != 0x30475854)
    {
        return 0;
    }

    f.read(&u1, 4);
    f.read(&u2, 4);
    f.read(texName, 0x104);
    {
        KTexture *atlas;

        loadTexture(texName, &atlas, 0);
        f.read(&count, 4);
        tex = atlas;
    }

    for (i = 0; i < count; i++)
    {
        f.read(&left, 4);
        f.read(&top, 4);
        f.read(&right, 4);
        f.read(&bottom, 4);
        f.read(name, 0x104);

        if (m_profile11c->find(name) == 0)
        {
            KTextureNode *n = (KTextureNode *)m_profile11c->create(name, 0, 0xa0);

            n->tex.f88 = 1;
            n->tex.bytes = 0;
            n->tex.w = (float)(right - left + 1);
            n->tex.h = (float)(bottom - top + 1);
            n->tex.u0 = (float)left / tex->w;
            n->tex.v0 = (float)top / tex->h;
            n->tex.u1 = (float)right / tex->w;
            n->tex.v1 = (float)bottom / tex->h;
            n->tex.surface = tex->surface;
        }
    }

    f.close();
    return 1;
}
