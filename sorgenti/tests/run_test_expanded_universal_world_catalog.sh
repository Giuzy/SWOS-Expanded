#!/usr/bin/env bash
# One all-country 2627 catalogue adapter test; no new toolchain.
# Original OG pairing indices are fixture INPUTS, not CPU score producers.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_world_time.c src/expanded_universal_world_catalog.c \
  tests/test_expanded_universal_world_catalog.c \
  -o "$DIR/universal_world_catalog"
"$DIR/universal_world_catalog"
echo "WORLD_2627_CATALOG_LIVE=NOT_IMPLEMENTED"
echo "WINDOWS_ORIGINAL_OG_50HZ_AND_CAR=NOT_TESTED"
