#!/usr/bin/env bash
# v00.13.342: multi-year cycle of the Italian pyramid with the REAL JSON definitions through the
# generic Competition Engine + real registry/RunDue + real CAR codec. OG match, disk and first editions
# are mocked/synthetic: NOT a Windows build or gameplay test.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
SDL_INC=""
for d in "$(sdl2-config --prefix 2>/dev/null || true)/include/SDL2" \
         /mingw64/include/SDL2 /usr/include/SDL2 /c/msys64/mingw64/include/SDL2; do
  if [ -f "$d/SDL.h" ]; then SDL_INC="-I$d"; break; fi
done
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-but-set-variable \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 \
  -DSDL_MAIN_HANDLED -Iinclude $SDL_INC \
  src/expanded_world_time.c src/expanded_international_windows.c src/expanded_ce_admin.c src/expanded_car_compact.c \
  src/expanded_edition_archive.c src/expanded_universal_car.c \
  src/expanded_ce_rules.c src/expanded_ce_table.c src/expanded_ce_engine.c src/expanded_fifa_ranking.c src/expanded_ce_calendar_validator.c \
  src/expanded_ce_definitions.c src/expanded_universal_json_leagues.c \
  src/expanded_universal_date_planner.c \
  tests/test_expanded_ce_italy_cycle.c "$DIR/mjson.o" -lm -o "$DIR/it_cycle"
"$DIR/it_cycle"
