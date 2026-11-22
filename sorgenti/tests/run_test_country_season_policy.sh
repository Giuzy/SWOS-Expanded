#!/bin/sh
# RG-001/012/014/027: pure country season policy, never a Windows engine test.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=$(mktemp "${TMPDIR:-/tmp}/swos-country-season-XXXXXX")
trap 'rm -f "$OUT"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/include" \
  "$ROOT/tests/test_country_season_policy.c" -o "$OUT"
"$OUT"
