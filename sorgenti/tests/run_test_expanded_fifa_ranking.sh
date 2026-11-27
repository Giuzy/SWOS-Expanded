#!/usr/bin/env bash
# FIFA ranking state, isolated (real seed JSON). NOT a Windows build or gameplay test.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Wno-unused-but-set-variable -Iinclude -c src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude \
  src/expanded_fifa_ranking.c tests/test_expanded_fifa_ranking.c "$DIR/mjson.o" -lm -o "$DIR/fifa"
"$DIR/fifa"
