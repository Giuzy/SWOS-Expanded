#!/usr/bin/env bash
# Real universal CAR codec + real C11 event/identity/clock, memory-only.
# Original 50Hz OG score is mocked in this isolated test; no EXE/CAR disk I/O.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
# platform_sdl.h (monotonic timings used by the CAR code) includes SDL.h.
# The test only COMPILES against the SDL2 headers (never links/runs SDL).
SDL_INC=""
for d in "$(sdl2-config --prefix 2>/dev/null || true)/include/SDL2" \
         /mingw64/include/SDL2 /usr/include/SDL2 /c/msys64/mingw64/include/SDL2; do
  if [ -f "$d/SDL.h" ]; then SDL_INC="-I$d"; break; fi
done
# Original mjson.c contains two pre-existing unused-but-set local variables
# (plen2/mystrtod). Exempt ONLY this existing translation unit, not the
# new Competition Engine codec or its negative tests, from that warning.
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-but-set-variable \
  -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 \
  -Iinclude $SDL_INC \
  src/expanded_world_time.c src/expanded_car_compact.c src/expanded_edition_archive.c src/expanded_universal_car.c \
  tests/test_expanded_universal_car.c "$DIR/mjson.o" -lm -o "$DIR/universal_car"
"$DIR/universal_car"
