#!/usr/bin/env bash
# perm.sh 0xADDR [perm.py options] - run the source permuter on one function.
# Keeps a persistent wineserver up so each cl.exe launch skips Wine's startup.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
export WINEPREFIX="$ROOT/tools/wineprefix" WINEDEBUG=-all DISPLAY= WAYLAND_DISPLAY=
pgrep -u "$USER" -x wineserver >/dev/null || { wineserver -p 600 &>/dev/null & disown; }
exec "$ROOT/.venv/bin/python" -W ignore "$ROOT/scratchpad/perm/perm.py" "$@"
