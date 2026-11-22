#!/bin/sh
# Annual per-country end dates: C11 isolated unit regression only.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-annual-country-XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/include" "$ROOT/tests/test_country_season_year.c" -o "$BIN"
"$BIN"
