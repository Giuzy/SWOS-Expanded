#!/usr/bin/env bash
# World2627 catalog now receives rule values from a mocked JSON reader.
# Pairing kernel and catalog C code are real; no OG seed/table input.
# v00.13.309: the catalog reads start-history JSON through mjson.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Iinclude -Wno-unused-but-set-variable -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_world_time.c src/expanded_international_windows.c src/expanded_universal_world_catalog.c \
  src/expanded_universal_date_planner.c \
  src/expanded_ce_calendar_validator.c tests/test_expanded_universal_world_catalog.c "$DIR/mjson.o" \
  -o "$DIR/universal_world_catalog"
"$DIR/universal_world_catalog"
echo "WORLD_2627_JSON_FIRST_PAIRINGS=PASS_ISOLATED_ONLY_IF_ASSERTIONS_RAN"
echo "WORLD_2627_FULL_JSON_PARSER_AND_CAR_LIVE=NOT_TESTED"
echo "WINDOWS_ORIGINAL_OG_50HZ_AND_CAR=NOT_TESTED"
