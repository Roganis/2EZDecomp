#!/bin/bash
# zq2.sh FILE.cpp SYMBOL-SUBSTR ADDR [N] : zoo-compile FILE and capstone-diff
# one function against the original (scratchpad/zd2.py). Unlike zoo.sh's
# objdump text, this reads the COFF section directly, so an inline-asm label
# mid-instruction cannot derail decoding, and relocated operands are masked
# by the relocation table instead of by value.
cd "$(dirname "$0")/.."
bash scratchpad/zoo.sh "$1" > /dev/null 2>scratchpad/obj/zq2.err || { head scratchpad/obj/zq2.err; exit 1; }
b=$(basename "$1" .cpp)
.venv/bin/python scratchpad/zd2.py scratchpad/obj/$b.obj "$2" "$3" ${4:-0}
