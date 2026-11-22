#!/usr/bin/env bash
# Real universal CAR codec + real C11 event/identity/clock, memory-only.
# Original 50Hz OG score is mocked in this isolated test; no EXE/CAR disk I/O.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_world_time.c src/mjson.c src/expanded_universal_car.c \
  tests/test_expanded_universal_car.c -lm -o "$DIR/universal_car"
"$DIR/universal_car"
