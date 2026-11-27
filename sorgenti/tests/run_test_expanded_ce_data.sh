#!/usr/bin/env bash
# Fase 2: validator + capability matrix over the REAL 2627 competition data,
# with the runtime's own definitions builder. Static data check only.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Iinclude -Wno-unused-but-set-variable -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -DSWOS_SDL_BUILD=1 -DSWOS_WORLDS=1 -Iinclude \
  src/expanded_world_time.c src/expanded_universal_date_planner.c \
  src/expanded_universal_json_leagues.c src/expanded_ce_rules.c \
  src/expanded_ce_table.c src/expanded_ce_engine.c src/expanded_fifa_ranking.c src/expanded_ce_definitions.c src/expanded_ce_admin.c \
  tests/test_expanded_ce_data.c "$DIR/mjson.o" -o "$DIR/ce_data"
"$DIR/ce_data"
