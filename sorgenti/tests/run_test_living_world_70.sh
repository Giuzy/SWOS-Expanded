#!/usr/bin/env bash
# 70 fully archived consecutive seasons, original SWOS fixture tables.
# Isolated C harness. Does NOT load the full Windows career manager state.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
echo "[build] isolated Living World Engine 70-season stress binary"
# Keep strict warnings-as-errors for our engine and tests. The bundled
# upstream mjson/miniz have known legacy warnings under GCC 16.2; compile
# those TWO vendor units separately with only their warning categories
# disabled. Never disable diagnostics for expanded_world_live.c.
COMMON=(-std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
# In this isolated harness mjson uses the C runtime realloc() instead of
# the game's swos_io_realloc() allocation shim, which is intentionally NOT
# linked into the standalone world test. This is restricted to mjson.o only.
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc -include stdlib.h -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" \
  src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c \
  src/expanded_world_live.c tests/native_fixture_table_ref.c \
  tests/test_living_world_70.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/living_world_70"
echo "[run] simulate 70 completed native-league seasons without rendering"
"$DIR/living_world_70"
