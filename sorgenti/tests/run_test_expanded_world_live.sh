#!/usr/bin/env bash
# Pure C test: synthetic CLUB records to exercise live calendar generator.
# NOT a full Windows build or proof of native career state preservation.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
# Keep strict warnings for our code, but not vendor-only warnings introduced
# by newer GCC versions in the bundled mjson and miniz translation units.
COMMON=(-std=c11 -O1 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" \
  src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c \
  src/expanded_world_live.c tests/native_fixture_table_ref.c \
  tests/test_expanded_world_live.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/world_live"
"$DIR/world_live"
