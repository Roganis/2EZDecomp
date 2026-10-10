/* Judge::grade @0x426500 - THE GRADING CORE, integrated from
   wip/judge-grade.cpp at 84.11%.

   Every judged press in the game funnels through here: judgePress @0x426700
   and the release path @0x4268c0 both call it with the lane's window block,
   the slot-22 press time and the note's tick. It is the windows-to-grade step
   the port reimplements as ez2_judge(), which is why having it buildable
   matters more than the last sixteen points - see docs/judge-timing.md.

   WHAT IT DOES. dt = pressTime - noteTime; the sign picks one of two six-entry
   window tables in the lane block (early at +0x20, late at +0x38) and latches
   the corresponding flag. The walk starts at the TIGHTEST window with grade 5
   and steps DOWN, so the first window |dt| fits is the best grade it earns;
   running off the bottom returns 0, a stray that consumes no note. A grade of
   4 (COOL) additionally tallies the global FAST or SLOW counter from the
   latched side and CONSUMES the flag, so each press counts once.

   AND IT SETTLES AN OPEN QUESTION. The shipped defaults @0x48ff98 are
   {-1, -1, 24, 12, 4, 2} in memory order, and since no |dt| is ever <= -1,
   GRADES 1 AND 0 ARE UNREACHABLE in the shipped configuration - which closes
   open item 4 of docs/judge-timing.md.

   EXHAUSTED, MECHANISM UNDERSTOOD. The original holds the lane pointer in ebp,
   which it must push and pop for the purpose; cl puts it in edi, which is
   already saved, and then reuses edi for the loop's zero. Everything else is
   identical - the ebx=1 idiom, the edx side arithmetic, both leas, the loop,
   all four globals, both epilogues. The knock-on is that the original's
   push/mov/pop is six bytes where ours is four, and the loop head is 16-byte
   aligned, so cl fills the shortfall with a `jmp` over padding the original
   does not need: two defects, one cause.

   Thirty-five source forms across two sessions, and the four best are recorded
   in the tail comment below so nobody re-runs them. Form 4 is what is kept,
   and it is the CLAUDE.md "declare the accumulator before the pointer" rule
   doing exactly what that rule says - it is what moves `side` from eax into
   edx and matches the original. */

#include "judge.h"

/* The FAST and SLOW tallies the result screen shows. Only a COOL feeds them,
   and only once per press: the side flag is consumed as it is counted. */
// GLOBAL: EZ2AC_UNPACKED 0x01b2eaa8
int g_fastCount;
// GLOBAL: EZ2AC_UNPACKED 0x01b5f1a0
int g_slowCount;
/* Which of the two the panel should be showing right now - both already
   defined in src/gamepanels.cpp, which is where the panel reads them. */
extern int g_showSlow;   /* 0x1b2ea1c */
extern int g_showFast;   /* 0x1b2ef3c */

// FUNCTION: EZ2AC_UNPACKED 0x00426500
int Judge::grade(JudgeLane *lane, int pressTime, int noteTime)
{
    int dt = pressTime - noteTime;
    const int *w;
    int side;
    int g;

    if (dt < 0)
    {
        side = 1;
        dt = -dt;
        f24e8 = 1;
    }
    else
    {
        side = 2;
        f24ec = 1;
    }

    g = 5;
    w = &lane->windows[side - 1][5];

    do
    {
        if (dt <= *w)
        {
            goto graded;
        }
        g--;
        w--;
    } while (g >= 0);

    return 0;

graded:
    if (g == 4)
    {
        if (f24e8 == 1)
        {
            g_fastCount++;
            f24e8 = 0;
            g_showSlow = 0;
            g_showFast = 1;
            return g;
        }

        if (f24ec == 1)
        {
            g_slowCount++;
            f24ec = 0;
            g_showFast = 0;
            g_showSlow = 1;
        }
    }

    return g;
}

/* SOURCE FORMS ALREADY TRIED - do not re-attempt these:
     1.  do-while + goto out of the loop                          78.50%
     2.  for (g = 5; g >= 0; g--, w--) with a break               80.70%
         (duplicates the exit test - cl re-tests g after the loop)
     3.  for (;;) with `return 0` inside the loop                 65.42%
         (lays the return-0 block after the slow arm, and gives the
          slow arm its own epilogue)
     4.  form 1 with `g = 5` moved ABOVE the pointer computation  84.11%  <- best
         This is the CLAUDE.md "declaring the accumulator before the pointer"
         rule and it is what moves `side` from eax into edx, matching the
         original. Kept below.
     5.  g initialised at its declaration, above the sign test    (worse: the
         whole ebx=1 idiom collapses into edi)
     6.  while (g != -1) instead of while (g >= 0)                no change
     7.  if (*w >= dt) instead of if (dt <= *w)                   no change
     8.  a `const int (*row)[6]` local for lane->windows          no change
     9.  non-const `int *w`                                       no change
     10. flat index: &lane->windows[0][side * 6 - 1]              no change
     11. byte arithmetic: (char *)lane + side * 24 + 0x1c         no change
   
   ---- 2026-08-10: THE MECHANISM, and why the 0/1 route is now RULED OUT ----
   
   The note this paragraph replaces said only that side-as-0/1 "DOES make cl
   push ebp, so the allocation is reachable". Twenty-four more forms later,
   here is WHY, and why it cannot be reached with the real layout.
   
   cl gives `lane` a seventh register only when eax and edi are ALREADY
   committed where the `lea` is. eax is `g` and edi is the hoisted constant 0,
   and the zero is committed there only if it is CONSUMED BEFORE the pointer -
   which happens exactly when one of the sign-test arms uses it. With
   side in {0, 1} both constants are consumed in the arms (`xor edx,edx` and
   `mov edx,edi`), both are born in the entry block, and `lane` is pushed out
   to ebp. With side in {1, 2} only the 1 is consumed early, the zero is born
   after the `lea`, and cl reuses its register for `lane`. That is the whole
   of it, and it is visible side by side in scratchpad/zoo/l_side01.cpp
   against any of the others.
   
   THE 0/1 MODEL IS NOT AVAILABLE, and that is now read rather than argued.
   The lane initialiser @0x426420 writes the two window rows to `[esi-0x30]`
   and `[esi-0x18]`, and the loop that walks the lanes sets `lea esi,[ebx+0x64]`
   with the lane array based at Judge+0x14 and a 0x5c stride - so esi is
   laneBase + 0x50 and the rows are at laneBase + 0x20 and + 0x38. The 0x1c
   bias in the original's `lea` therefore forces side in {1, 2}. The two
   constraints are in genuine tension and no source form can satisfy both.
   
   FORMS TRIED SINCE, all 53 instructions and none with `push ebp` (each is a
   two-second build through scratchpad/zoo.sh; the TUs themselves are not
   committed, per .gitignore - the list below IS the record):
     indexing inside the loop; a row pointer; `for` instead of `do`;
     `while (w >= base)`; the pointer before `g`; `g` declared at the top;
     `--g` in the condition; byte arithmetic from `lane` (not from
     `lane->windows[0]`, which is what the 0/1 form used); `(const int *)lane
     + side*6 + 7`; a flat `side*6 - 1` index; a named `off`; named `one` and
     `zero` locals consumed in the arms and in the loop bound - cl
     constant-propagates every one of them, which is what closes the door; a
     `static` address helper and a `__forceinline` one; `volatile` on the lane
     pointer; a live use of `lane` after the loop (deleted as dead).
   
   ---- WHAT THE 16% ACTUALLY IS ---------------------------------------------
   
   Fifty instructions, of which nine differ, and NONE of them semantically:
   
     1. `push ebp` / `mov ebp,[esp+0x14]` / `pop ebp` against our single
        `mov edi,[esp+0x10]` - the register above, and its two-byte size
        difference is what makes cl fill the 16-byte-aligned loop head with a
        `jmp` where the original needs only a `lea ecx,[ecx]` pad.
     2. `mov eax,5` and `xor edi,edi` in the other order - a knock-on of 1.
     3. `xor eax,eax` and `mov [g_showSlow],ebx` scheduled BEFORE the epilogue
        pops where the original sinks each between `pop esi` and `pop ebx`.
     4. two `jne` displacements, which follow from 3 (the shared tail is
        entered at a different instruction).
   
   That enumeration is the point: it bounds what an oracle arm built on this
   file can be wrong about. Every instruction of the original is present, in
   the same basic blocks, with the same operands bar one register.
   
   The class model is in include/judge.h - JudgeLane and the two flag fields
   were carved out of the tail pad, same size and alignment, and the full
   match.sh was clean afterwards. */
