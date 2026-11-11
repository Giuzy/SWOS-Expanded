#!/usr/bin/env bash
# 70 fully archived consecutive seasons, original SWOS fixture tables.
# Isolated C harness. Does NOT load the full Windows career manager state.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
echo "[build] isolated Living World Engine 70-season stress binary"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude \
  -DFIXTURE_DATA_C=1 \
  src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c \
  src/mjson.c src/miniz_impl.c \
  src/expanded_world_live.c tests/native_fixture_table_ref.c \
  tests/test_living_world_70.c -o "$DIR/living_world_70"
echo "[run] simulate 70 completed native-league seasons without rendering"
"$DIR/living_world_70"
