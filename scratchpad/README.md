Scans used to pick targets. All of them parse `text.asm`, which is

    objdump -D -b binary -m i386 -Mintel --no-show-raw-insn --adjust-vma=0x400000 \
      --start-address=0x401000 --stop-address=0x478000 original/EZ2AC_Unpacked.exe \
      > scratchpad/text.asm

(gitignored along with the rest of the disassembly - regenerate it, do not
commit it). Function starts = call-rel32 targets + every dword in
0x478000..0x4b1000 that points into .text (vtable slots and jump tables) + the
addresses already annotated in src/.

  dis.py LO HI   print one address range out of text.asm
  scan.py        unmatched functions ranked by BRANCHES PER INSTRUCTION, ascending
  scan2.py       unmatched functions grouped by a callee: EzObject::EzObject
                 (ctors), __ehvec_dtor_iterator (dtors with array members),
                 __ehvec_ctor_iterator
  scan3.py       every class DESTRUCTOR: stores a 0x4xxxxx constant into [reg]
                 near the top and ends with a call to the folded empty ret
                 @0x402880 - prints the vtable address with each one
  census.py      matched .text bytes / real game-code bytes (the progress number
                 worth reporting)
