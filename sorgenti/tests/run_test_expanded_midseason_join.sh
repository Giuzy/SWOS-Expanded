#!/usr/bin/env bash
# Pure-C calendar test; no SDL or gameplay link. NOT a Windows runtime test.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -O2 -Iinclude \
  src/expanded_world_time.c src/expanded_midseason_join.c \
  tests/test_expanded_midseason_join.c -o "$DIR/midseason_join"
"$DIR/midseason_join"
