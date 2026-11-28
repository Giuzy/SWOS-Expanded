#!/usr/bin/env bash
# Generic table criteria (H2H re-application, away wins, discipline, access list). Isolated C.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-but-set-variable -Iinclude -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude \
  src/expanded_ce_rules.c src/expanded_ce_table.c src/expanded_ce_discipline.c \
  tests/test_expanded_ce_table_criteria.c "$DIR/mjson.o" -o "$DIR/tbc"
"$DIR/tbc"
