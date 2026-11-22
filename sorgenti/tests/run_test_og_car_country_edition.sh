#!/usr/bin/env sh
# RG-022 isolated pure CAR country-edition validator (no game/CAR writer).
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-og-car-edition.XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -I"$ROOT/include" \
    "$ROOT/tests/test_og_car_country_edition.c" -o "$BIN"
"$BIN"
