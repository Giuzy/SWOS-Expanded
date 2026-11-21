#!/bin/sh
# RG-010/014 pixel-art job offer blink gate, pure C11.
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
OUT="${TMPDIR:-/tmp}/swos_rg014_offer_flash_$$"
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/include" "$SCRIPT_DIR/test_career_job_offer_flash.c" -o "$OUT"
"$OUT"
