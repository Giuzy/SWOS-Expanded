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
echo "OG_MANAGER_RESIDENT_MERGE=NOT_IMPLEMENTED"
echo "OG_WINDOWS_GAMEPLAY=NOT_TESTED"
