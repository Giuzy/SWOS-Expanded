#!/usr/bin/env bash
# Native SWOS fixture pairings plus four connected divisions / 70 years.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
COMMON=(-std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc -include stdlib.h -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c src/expanded_world_live.c \
  tests/native_fixture_table_ref.c tests/test_living_world_mobility.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/world_mobility"
"$DIR/world_mobility"
