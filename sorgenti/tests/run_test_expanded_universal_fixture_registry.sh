#!/usr/bin/env bash
# Pure C11 proof against the REAL project event, identity and Gregorian headers.
# It does NOT build SDL/Windows or call original OG 50Hz match/CAR writer.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  src/expanded_world_time.c \
  tests/test_expanded_universal_fixture_registry.c \
  -o "$DIR/universal_fixture_registry"
"$DIR/universal_fixture_registry"
echo "POINTS_1_2_3_LIVE_CUTOVER=NOT_IMPLEMENTED"
echo "WINDOWS_GAMEPLAY_AND_CAR=NOT_TESTED"
