#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -O2 \
  -Iinclude tests/test_expanded_og_fixture_hash.c -o "$TMP/hash"
"$TMP/hash"
printf '%s\n' 'OG_JOURNAL_PRUNE_AND_WINDOWS_CAR=NOT_TESTED_BY_ISOLATED_TEST'
