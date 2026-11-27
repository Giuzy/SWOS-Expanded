#!/usr/bin/env bash
# Fase 2: REAL Competition Engine (rules, tables, lifecycle, qualification,
# promotion/relegation, blocks, UL01) + REAL registry/RunDue + REAL CAR
# module. Only the OG match result and the disk are mocked: NOT a Windows
# build or gameplay test.
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
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 \
  -DSDL_MAIN_HANDLED -Iinclude $SDL_INC \
  src/expanded_world_time.c src/expanded_ce_admin.c src/expanded_car_compact.c \
  src/expanded_edition_archive.c src/expanded_universal_car.c \
  src/expanded_ce_rules.c src/expanded_ce_table.c src/expanded_ce_engine.c src/expanded_fifa_ranking.c \
  src/expanded_universal_date_planner.c src/expanded_ce_viewer.c -DEXP_CE_VIEWER_PURE=1 \
  tests/test_expanded_ce_real_competitions.c "$DIR/mjson.o" -lm -o "$DIR/ce_real"
"$DIR/ce_real"
