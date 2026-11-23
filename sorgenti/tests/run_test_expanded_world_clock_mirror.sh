#!/usr/bin/env bash
# v00.13.283: LWE cursor is a passive mirror of the CE clock (real LWE code,
# fake CE provider). Same harness as run_test_expanded_world_live.sh, plus
# -Wno-unused-function for a pre-existing unused LWE helper (live_roll_window).
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
COMMON=(-std=c11 -O1 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc -include stdlib.h -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" -Wno-unused-function \
  src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c \
  src/expanded_world_live.c tests/native_fixture_table_ref.c \
  tests/test_expanded_world_clock_mirror.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/clock_mirror"
"$DIR/clock_mirror"
