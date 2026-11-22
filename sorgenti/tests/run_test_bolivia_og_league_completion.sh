#!/usr/bin/env bash
# Isolated contract, NOT a 2027 career, SAVE/CAR or Windows test.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
BIN="$(mktemp)"
trap 'rm -f "$BIN"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_bolivia_og_league_completion.c -o "$BIN"
"$BIN"
