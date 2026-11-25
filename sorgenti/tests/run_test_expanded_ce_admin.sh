#!/usr/bin/env bash
# v00.13.301: administrative results of TEST clubs (pure module + mjson).
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Iinclude -Wno-unused-but-set-variable -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_ce_admin.c tests/test_expanded_ce_admin.c "$DIR/mjson.o" -o "$DIR/ce_admin"
"$DIR/ce_admin"
