#!/bin/sh
# RG-014 calendar isolated C11 test (never a Windows/gameplay PASS).
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUT="${TMPDIR:-/tmp}/swos_rg014_manager_calendar_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
  "$SCRIPT_DIR/test_bolivia_manager_calendar_gate.c" -o "$OUT"
"$OUT"
