#!/bin/sh
# RG-014 Bolivia original OG calendar VISUAL viewport only; not a Windows GP test.
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUT="${TMPDIR:-/tmp}/swos_rg014_calendar_view_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/include" "$SCRIPT_DIR/test_bolivia_calendar_viewport.c" -o "$OUT"
"$OUT"
