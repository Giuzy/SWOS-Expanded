#!/usr/bin/env bash
# 30 exact civil anniversaries, northern/southern native schedules,
# Jan/Jul transitions and .CAR snapshots. NOT OG Windows proof.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
COMMON=(-std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc \
  -include stdlib.h -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits \
  -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" \
  src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c \
  src/expanded_world_live.c tests/native_fixture_table_ref.c \
  tests/test_calendar_integrity_30.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/calendar_30"
"$DIR/calendar_30"
