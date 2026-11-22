#!/usr/bin/env bash
# Independent C11 native counter contract (not a gameplay/Windows/Save test).
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
BIN="$(mktemp)"
trap 'rm -f "$BIN"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_og_country_edition_transition.c -o "$BIN"
"$BIN"
