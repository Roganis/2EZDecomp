#!/usr/bin/env python3
"""zdiff.py SRC.cpp SYMBOL LO HI [--raw] -- structural diff of a zoo-compiled
function against the original.

Call/data operands are masked (they are relocations in the .obj and real
addresses in the image), but a BRANCH is rendered as its displacement from the
next instruction, which is comparable across the two images and is what reccmp
actually scores. Masking branch targets outright - the obvious thing - inflates
the score badly on a branch-heavy function: it reports the block CONTENTS as
matching while the LAYOUT is still wrong. Pass --raw to mask them anyway.

  SRC.cpp   a TU under scratchpad/ (or src/) that zoo.sh can build
  SYMBOL    a prefix of the decorated name, e.g. '?update2@GFBackGround'
  LO HI     the original's address range, e.g. 0x45ca90 0x45cb1e
"""
import re, subprocess, difflib, sys

RAW = '--raw' in sys.argv
BR = (r'\b(call|jmp|je|jne|jg|jge|jl|jle|ja|jae|jb|jbe|js|jns|jo|jno|loop)\s+'
      r'(0x)?([0-9a-f]+)\b.*')

def mask(t, addr, nxt):
    m = re.match(BR, t)
    if m:
        op = m.group(1)
        if op == 'call' or RAW or nxt is None:
            return op + ' T'
        # displacement from the END of this instruction, as the encoding has it
        return '%s %+d' % (op, int(m.group(3), 16) - nxt)
    t = re.sub(r'<[^>]*>', '', t)
    t = re.sub(r'ds:0x[0-9a-f]+', 'ds:A', t)
    t = re.sub(r'\b0x[0-9a-f]{6,8}\b', 'A', t)
    return t.strip()

def finish(pairs, relocs=()):
    """Render each instruction, masking any operand the linker will fix up.

    A relocated operand reads as 0x0 in the .obj and as a real address in the
    image, so without this every `push <string>` and every absolute data
    reference counts as a difference - which is why the raw score used to sit
    several points under reccmp's on any function that touches a global.
    """
    out = []
    for i, (a, t) in enumerate(pairs):
        nxt = pairs[i+1][0] if i+1 < len(pairs) else None
        t = mask(t, a, nxt)
        if relocs and any(a <= r < (nxt if nxt is not None else a+16) for r in relocs):
            # Only the ZERO-addend case is masked, on purpose. A COFF DIR32
            # keeps its addend in the field, so `g_titleCodes + 0xf0` reads as
            # `cmp eax,0xf0` here and as `cmp eax,0x4ae108` in the image, and
            # masking the last literal would call those equal. That was tried
            # and it OVER-reports: TitleDirector::m44f300 read 100.0% where
            # reccmp said 98.67%, hiding a real defect in that very operand.
            # Under-reporting is the safe direction - a relocated operand with
            # a non-zero addend shows up as a difference and gets looked at.
            t = re.sub(r'\b0x0\b', 'A', t)
        out.append(t)
    return out

def coff_relocs(obj, sec):
    """Section-relative offsets of the relocations in COFF section `sec` (1-based).

    objdump -h does not print PointerToRelocations and objdump -r cannot say
    WHICH `.text` it means when every function is its own COMDAT of that name,
    so read the section header out of the file: a COFF object has no optional
    header, so the 40-byte section headers start at offset 20.
    """
    b = open(obj, 'rb').read()
    nsec = int.from_bytes(b[2:4], 'little')
    if not 1 <= sec <= nsec:
        return []
    h = 20 + (sec - 1) * 40
    ptr = int.from_bytes(b[h+24:h+28], 'little')
    n   = int.from_bytes(b[h+32:h+34], 'little')
    return [int.from_bytes(b[ptr+i*10:ptr+i*10+4], 'little') for i in range(n)]


def zoo(path, sym):
    """Disassemble ONE function out of the zoo .obj.

    Not with `objdump -d`: cl's /Gy puts every function in its own COMDAT
    section, each starting at offset 0, and objdump applies a local label
    ($L62867 at 0x3b in one section) to EVERY section at that offset - which
    splits an instruction in the middle of an unrelated function and emits
    `.byte` soup from there on. Instead ask the symbol table which section the
    function owns (a COMDAT holds exactly one function, so the section's size
    IS the function's size) and disassemble those bytes as raw binary.
    """
    p = subprocess.run(['./scratchpad/zoo.sh', path], capture_output=True, text=True)
    if p.returncode:
        sys.exit("cl failed:\n" + p.stdout[:2000])
    obj = 'scratchpad/obj/%s.obj' % path.split('/')[-1][:-4]

    sec = val = None
    t = subprocess.run(['objdump','-t',obj], capture_output=True, text=True).stdout
    for l in t.splitlines():
        m = re.match(r'^\[\s*\d+\]\(sec\s+(\d+)\).*?\s(0x[0-9a-f]+)\s+(\S+)\s*$', l)
        if m and m.group(3).startswith(sym):
            sec, val = int(m.group(1)), int(m.group(2), 16)
            break
    if sec is None:
        sys.exit("symbol %r not found in %s" % (sym, obj))

    h = subprocess.run(['objdump','-h',obj], capture_output=True, text=True).stdout
    off = size = None
    for l in h.splitlines():
        m = re.match(r'^\s*(\d+)\s+\S+\s+([0-9a-f]{8})\s+[0-9a-f]{8}\s+'
                     r'[0-9a-f]{8}\s+([0-9a-f]{8})', l)
        if m and int(m.group(1)) == sec - 1:      # -h Idx is 0-based, sec is 1-based
            size, off = int(m.group(2), 16), int(m.group(3), 16)
            break
    if off is None:
        sys.exit("section %d not found in %s" % (sec, obj))

    relocs = coff_relocs(obj, sec)

    d = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
        '--no-show-raw-insn','--start-address=%#x' % (off + val),
        '--stop-address=%#x' % (off + size), obj], capture_output=True, text=True)
    pairs = []
    for l in d.stdout.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\t(.*)$', l)
        if m: pairs.append((int(m.group(1), 16), m.group(2).strip()))
    # the raw-binary listing is file-offset based, so bias the section-relative
    # relocation offsets the same way
    return finish(pairs, set(off + r for r in relocs))

def orig(lo, hi):
    p = subprocess.run(['objdump','-D','-b','binary','-m','i386','-Mintel',
        '--no-show-raw-insn','--adjust-vma=0x400000','--start-address=%#x'%lo,
        '--stop-address=%#x'%hi,'original/EZ2AC_Unpacked.exe'],
        capture_output=True, text=True)
    pairs = []
    for l in p.stdout.splitlines():
        m = re.match(r'^\s*([0-9a-f]+):\t(.*)$', l)
        if m: pairs.append((int(m.group(1), 16), m.group(2).strip()))
    return finish(pairs)

a = orig(int(sys.argv[3], 16), int(sys.argv[4], 16))
b = zoo(sys.argv[1], sys.argv[2])
same = sum(t[2]-t[1] for t in difflib.SequenceMatcher(None, a, b).get_opcodes()
           if t[0] == 'equal')
print("original %d insns, zoo %d insns, %d in common (%.1f%%)"
      % (len(a), len(b), same, 100.0*same/max(len(a), 1)))
for l in difflib.unified_diff(a, b, 'original', 'zoo', lineterm='', n=2):
    print(l)
