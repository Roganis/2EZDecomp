#!/usr/bin/env bash
# Build the reconstructed sources with the period VC7.1 toolchain under Wine,
# then run reccmp to report per-function match % against the original.
#   ./match.sh              -> build + overall match table
#   ./match.sh 0x0040b970   -> build + full asm diff for that original address
set -euo pipefail
shopt -s nullglob

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

export WINEPREFIX="$ROOT/tools/wineprefix"
export WINEDEBUG=-all
export DISPLAY=
export WAYLAND_DISPLAY=
CL="$ROOT/tools/msvc71/bin/cl.exe"
LINK="$ROOT/tools/msvc71/bin/link.exe"

# Unix path -> Wine Z: path
towin() { printf 'Z:%s' "$(printf '%s' "$1" | tr '/' '\\')"; }

# Compiler search paths (append PSDK / DX8 when present)
INC="$(towin "$ROOT/tools/msvc71/include")"
LIBP="$(towin "$ROOT/tools/msvc71/lib")"
[ -d "$ROOT/tools/platformsdk/Include" ] && INC="$INC;$(towin "$ROOT/tools/platformsdk/Include")"
[ -d "$ROOT/tools/platformsdk/Lib" ]     && LIBP="$LIBP;$(towin "$ROOT/tools/platformsdk/Lib")"
[ -d "$ROOT/tools/dx8sdk/include" ]      && INC="$INC;$(towin "$ROOT/tools/dx8sdk/include")"
[ -d "$ROOT/tools/dx8sdk/lib" ]          && LIBP="$LIBP;$(towin "$ROOT/tools/dx8sdk/lib")"
export INCLUDE="$INC"
export LIB="$LIBP"

# Flags chosen to match the original's release codegen; refined per-function.
CFLAGS="/nologo /c /O2 /MT /GS /Gy /GF /Gd /G7 /arch:SSE2 /Zi /Iinclude /Iinclude/dx"

# Exception model, per TU. The original mixes the two: /EHsc (extern "C" assumed
# nothrow) is what all but one TU needs — under /EHs the Win32 calls in
# ~KSongPlayerBase / ~KSNDMixer / ~Obj414750 grow an EH frame the original does
# not have. src/kengineapp.cpp is the exception: loadProfiles @0x401240 resets its
# EH state to -1 after each `new`, which cl only emits when the next allocation
# call counts as a throw point, i.e. exactly when extern "C" may throw (/EHs).
eh_model() {
    case "$1" in
    src/kengineapp.cpp) echo "/EHs" ;;
    *)                  echo "/EHsc" ;;
    esac
}

mkdir -p build/obj
OBJS=()
for f in src/*.cpp harness/*.cpp; do
    b="$(basename "$f" .cpp)"
    wine "$CL" $CFLAGS "$(eh_model "$f")" "/Fobuild\\obj\\$b.obj" "/Fdbuild\\obj\\vc.pdb" "$(towin "$ROOT/$f")"
    OBJS+=("build\\obj\\$b.obj")
done

# /base:0x11000000 is LOAD-BEARING, not cosmetic. This engine's classes are
# enormous (Obj40C180 is 0x420290 bytes, and one per-player element is 14 MB), so
# an ordinary `this`-relative member offset is a number in the same range as an
# address at the default 0x400000 base. Whenever one of OUR entities happened to
# start exactly on such an offset, reccmp named the operand on our side against
# the original's raw number and a byte-identical function read as a mismatch —
# and the exact-base case cannot be fixed inside reccmp, because from one side a
# member offset landing on an entity's base is indistinguishable from a real
# reference to it. Basing the recomp image high puts every entity we own out of
# reach of every member offset (the largest seen is 14.7 MB, an 18x margin),
# which retires the whole class. It also retired src/aaa_pad.cpp, the 4 MB
# unreferenced array that used to push our globals clear of the range.
#
# The EXACT value is chosen, not round. A large IMMEDIATE is the mirror of the
# same problem: `mov eax, 0x10624dd3` is the magic multiplier for a signed
# divide by 1000, and at the old 0x10000000 base that constant landed 6 MB into
# our own .data, so our side symbolized it against the original's raw number
# (updateSpeedDigits @0x41d1d0). Unlike a displacement this cannot be fixed in
# the parser - a `mov reg, imm` is as often a real interior data reference
# (copyLane @0x412790 loads &g_tracks[0].records[n]) as a constant, so an
# exact-base-only rule breaks one to fix the other. Instead the base is picked so
# that NO constant in the original's game code can land in our image at all: of
# the 783 distinct >=16 MB hex literals in 0x401000..0x478000, the nearest one
# above 0x11000000 is 80 MB up, ten times the current image. Re-run that scan
# before moving the base again.
wine "$LINK" /nologo "/out:build\\EZ2AC_Unpacked.exe" /debug \
    "/pdb:build\\EZ2AC_Unpacked.pdb" "/map:build\\EZ2AC_Unpacked.map" /subsystem:console /base:0x11000000 \
    /opt:noref /opt:noicf /incremental:no libcpmt.lib libcmt.lib winmm.lib user32.lib gdi32.lib advapi32.lib ole32.lib dinput.lib ddraw.lib dsound.lib "${OBJS[@]}"

echo "=================== reccmp ==================="
if [ "${1:-}" != "" ]; then
    exec "$ROOT/.venv/bin/reccmp-reccmp" --target EZ2AC_UNPACKED --verbose "$1"
else
    # --nolib: LIBRARY annotations are address anchors for call resolution,
    # not reconstruction targets; don't score the CRT against itself.
    exec "$ROOT/.venv/bin/reccmp-reccmp" --target EZ2AC_UNPACKED --nolib
fi
