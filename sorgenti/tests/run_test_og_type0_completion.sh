#!/usr/bin/env sh
# RG-027 strict original OG TYPE0 completion evidence (no game/SDL/CAR).
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-og-type0-completion.XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
    "$ROOT/tests/test_og_type0_completion.c" -o "$BIN"
"$BIN"
