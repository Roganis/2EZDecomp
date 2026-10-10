# Contributing

Every function already has C. The work now is making the remaining ones
**byte-identical**, and keeping all of them **correct**. A function counts as
done only when reccmp reports 100% for it.

## The one rule that is never relaxed

Nothing from the game goes into this repository: no executable, no assets, no
key or data tables copied out of the binary, and no hexdumps. Note that
`objdump -D` output *is* a hexdump (it prints the instruction bytes), so keep
disassembly listings out of commits. Addresses, struct layouts and analysis
are fine. Stage files by name - never `git add -A` after disassembling.

## Matching a function

1. **Pick a target.** `./match.sh` prints every function's score. Small,
   nearly-matching functions are the cheapest. The progress map in
   `docs/progress/` shows where the unmatched code is.
2. **Read the diff.** `./match.sh 0xADDRESS` shows reccmp's diff for one
   function. For a faster loop while editing, `scratchpad/perm/check.py
   0xADDRESS --diff` recompiles just that file and prints an aligned
   instruction diff in seconds.
3. **Check the cookbook first.** `docs/cookbook.md` lists every codegen
   pattern of this compiler that has been worked out, each with the evidence
   and the measured effect - declaration order, loop shapes, branch layout,
   register choice, float rewrites, switch layout, and more. Most diffs match
   one of its entries.
4. **Try the permuter on a "coin flip".** When only registers or block order
   differ, `scratchpad/perm/perm.sh 0xADDRESS` searches source rewrites
   automatically against the real compiler. Treat its output as a
   *candidate*: shrink it with `scratchpad/perm/minimize.py`, rewrite it by
   hand, make sure it means the same thing, and confirm with reccmp. Read
   `scratchpad/perm/README.md` for what it can and cannot see.
5. **Confirm and commit one function at a time**, with the reccmp result in
   the commit message. Update the comments your change makes wrong.

## Correctness checks

A match score cannot tell a register difference from a wrong constant. Before
trusting a low-scoring function's behaviour:

- `scratchpad/semscan.py` / `callseq.py` compare what a function *does*
  (callees, globals, constants) with registers ignored;
- `scratchpad/perm/floatsweep.py` compares float constants;
- `scratchpad/fmt/run.sh` runs the decomp's own file-format code on your game
  data and diffs it against an independent decoder.

## Conventions

- C++98/03 only, as Visual C++ 7.1 accepts it. No STL unless the original
  demonstrably uses it.
- One `// FUNCTION: EZ2AC_UNPACKED 0xADDRESS` annotation directly above each
  definition (reccmp's format; the target name refers to the user's own
  executable).
- `CLAUDE.md` holds the full working rules, including the toolchain set-up and
  the calling-convention pitfalls.

## Updating the progress figures

After a full `./match.sh`:

```bash
.venv/bin/reccmp-reccmp --target EZ2AC_UNPACKED --nolib --json rc.json
.venv/bin/python scratchpad/progress.py rc.json docs/progress
```
