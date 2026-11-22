#!/usr/bin/env sh
# RG-014/027 pure OG viewer edition choice (no game/SDL/CAR).
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-og-view-edition.XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
    "$ROOT/tests/test_og_view_edition.c" -o "$BIN"
"$BIN"
