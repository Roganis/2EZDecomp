#!/usr/bin/env bash
# vtry.sh FILE.cpp — zoo-build a blit variant and report the structural score
set -euo pipefail
f="$1"; b="$(basename "$f" .cpp)"
./scratchpad/zoo.sh "$f" > "scratchpad/$b.asm" 2>&1 || { tail -3 "scratchpad/$b.asm"; exit 1; }
ZOO_ASM="scratchpad/$b.asm" python3 scratchpad/blitdiff.py "${2:-0}"
