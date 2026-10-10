#!/usr/bin/env bash
# try.sh — compile one src TU via the zoo and print a function's float block,
# for fast register-allocation experiments (~3s vs a ~3min match.sh).
#   scratchpad/try.sh src/gamepanels.cpp '??0GFPanel@@QAE@XZ'
cd "$(dirname "${BASH_SOURCE[0]}")/.."
./scratchpad/zoo.sh "$1" 2>/dev/null | sed -n "/^00000000 <$2>:/,/^\$/p" \
  | grep -nE "xorps|movss|movaps" | head -20
