#!/usr/bin/env bash
# v00.13.287: ordered CAR writer, real threads + real files, mock codec.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
SDL_INC="";SDL_LIB=""
for d in "$(sdl2-config --prefix 2>/dev/null || true)" /mingw64 /usr /c/msys64/mingw64; do
  if [ -f "$d/include/SDL2/SDL.h" ]; then SDL_INC="-I$d/include/SDL2"; SDL_LIB="-L$d/lib"; break; fi
done
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSDL_MAIN_HANDLED -Iinclude $SDL_INC \
  src/car_writer.c tests/test_car_writer.c $SDL_LIB -lSDL2 -o "$DIR/car_writer"
"$DIR/car_writer" "$DIR"
