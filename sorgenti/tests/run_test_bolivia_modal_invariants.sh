#!/bin/sh
# RG-014 original OG Copa modal pure C11 invariants, not gameplay proof.
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUT="${TMPDIR:-/tmp}/swos_rg014_modal_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/include" \
  "$SCRIPT_DIR/test_bolivia_modal_invariants.c" -o "$OUT"
"$OUT"
