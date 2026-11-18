#!/usr/bin/env bash
# Pure C read-only keycache lookup, not a world/CAR/Windows gameplay test.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_expanded_og_cache_match.c -o "$DIR/og_cache_match"
"$DIR/og_cache_match"
echo "OG_MANAGER_RESIDENT_BRIDGE=NOT_TESTED_BY_ISOLATED_CACHE_TEST"
echo "OG_CALENDAR_KEYCACHE_PAIR_BINDING=NOT_YET_RECONCILED_WHEN_MISMATCH"
echo "OG_WINDOWS_GAMEPLAY=NOT_TESTED"
