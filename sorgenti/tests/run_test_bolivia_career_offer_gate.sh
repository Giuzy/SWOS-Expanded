#!/bin/sh
# RG-014 employed new-offer gate, pure C11; not a live OG/CAR test.
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUT="${TMPDIR:-/tmp}/swos_rg014_career_offer_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/include" "$SCRIPT_DIR/test_bolivia_career_offer_gate.c" -o "$OUT"
"$OUT"
