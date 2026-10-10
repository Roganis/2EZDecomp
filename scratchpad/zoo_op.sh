#!/usr/bin/env bash
# zoo.sh FILE.cpp [symbol-filter]  — compile ONE TU with the project flags and
# disassemble the result, for fast codegen experiments (~2s vs a full match.sh).
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
export WINEPREFIX="$ROOT/tools/wineprefix" WINEDEBUG=-all DISPLAY= WAYLAND_DISPLAY=
towin() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }
INC="$(towin "$ROOT/tools/msvc71/include");$(towin "$ROOT/tools/platformsdk/Include");$(towin "$ROOT/tools/dx8sdk/include")"
export INCLUDE="$INC"
f="$1"; b="$(basename "$f" .cpp)"
mkdir -p scratchpad/obj
wine "$ROOT/tools/msvc71/bin/cl.exe" /nologo /c /O2 /MT /GS /Gy /GF /Gd /G7 /arch:SSE2 /EHsc /Op \
    /Iinclude /Iinclude/dx "/Foscratchpad\\obj\\$b.obj" "$(towin "$ROOT/$f")" >/dev/null
objdump -d -Mintel --no-show-raw-insn "scratchpad/obj/$b.obj" | sed 's/^ *//'
