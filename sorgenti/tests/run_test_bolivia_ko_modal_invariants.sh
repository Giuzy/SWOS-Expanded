#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-bolivia-ko.XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-gcc}" -std=c11 -Wall -Wextra -Werror -pedantic \
    -I"$ROOT/include" "$ROOT/tests/test_bolivia_ko_modal_invariants.c" \
    -o "$BIN"
"$BIN"
