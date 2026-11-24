#!/usr/bin/env bash
# Fase 1: REAL expanded_universal_live.c + career dispatch + RunDue + CAR codec.
# The OG match (QUICK/50 Hz), catalog, menus and disk are mocked; this is NOT
# a Windows build or gameplay test.
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
"$CC" -std=gnu11 -Wall -Wextra -Werror -Wno-address-of-packed-member \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude $SDL_INC \
  src/expanded_world_time.c src/expanded_car_compact.c \
  src/expanded_edition_archive.c src/expanded_universal_car.c \
  src/expanded_universal_career_dispatch.c src/expanded_universal_live.c \
  src/expanded_ce_rules.c src/expanded_ce_table.c src/expanded_ce_engine.c \
  src/expanded_ce_definitions.c \
  src/expanded_universal_date_planner.c \
  tests/test_expanded_universal_live.c "$DIR/mjson.o" -lm \
  -o "$DIR/universal_live"
"$DIR/universal_live"
