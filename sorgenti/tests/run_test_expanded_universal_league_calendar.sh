#!/usr/bin/env bash
# v00.13.280: REAL world 2627 leagues.json + league_calendar.json through the
# REAL loader and CE date planner. Every division must get one civil day per
# round from data only (no OG native phase). Not gameplay, CAR or Windows UI.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
# vendor unit on its own, with its one known warning exemption
"$CC" -std=c11 -Iinclude -Wno-unused-but-set-variable -c src/mjson.c \
  -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_world_time.c src/expanded_international_windows.c src/expanded_universal_date_planner.c \
  src/expanded_universal_json_leagues.c \
  tests/test_expanded_universal_league_calendar.c "$DIR/mjson.o" \
  -o "$DIR/universal_league_calendar"
"$DIR/universal_league_calendar"
echo "WINDOWS_ORIGINAL_OG_50HZ_AND_CAR=NOT_TESTED"
