#!/usr/bin/env bash
# Real DATA_JSON club membership, original fixture-round C engine, 70 seasons.
# All catalog failures are reported; no saves modified, no Python required.
set -euo pipefail
cd "$(dirname "$0")/../.."
shopt -s nullglob
catalogs=(data_json/teams/*.json)
if (( ${#catalogs[@]}==0 )); then
  echo 'REAL_JSON_70_SEASONS=FAIL (no installed JSON team files)'
  exit 1
fi
CC="${CC:-gcc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
cd sorgenti
COMMON=(-std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -Iinclude -DFIXTURE_DATA_C=1)
"$CC" "${COMMON[@]}" -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc -include stdlib.h -c src/mjson.c -o "$DIR/mjson.o"
"$CC" "${COMMON[@]}" -Wno-type-limits -c src/miniz_impl.c -o "$DIR/miniz.o"
"$CC" "${COMMON[@]}" src/expanded_world_time.c src/expanded_world_ledger.c \
  src/fixture_data.c src/fixture_rounds.c src/expanded_world_live.c \
  tests/native_fixture_table_ref.c tests/test_real_json_world_70.c \
  "$DIR/mjson.o" "$DIR/miniz.o" -o "$DIR/real_json_world_70"
cd ..
echo "[real-json] checking ${#catalogs[@]} actual JSON files"
"$DIR/real_json_world_70" "${catalogs[@]}"
