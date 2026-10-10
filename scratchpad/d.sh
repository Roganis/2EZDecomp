#!/bin/bash
# Inspect one function's diff against the CURRENT build/ - no rebuild.
cd "$(dirname "${BASH_SOURCE[0]}")/.."
exec ./.venv/bin/reccmp-reccmp --target EZ2AC_UNPACKED --verbose "$1" 2>&1 \
  | sed 's/\x1b\[[0-9;]*m//g' | grep -v "^\[WARNING\]\|^\[ERROR\]\|^\[INFO\]"
