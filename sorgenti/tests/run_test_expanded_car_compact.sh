#!/usr/bin/env bash
# v00.13.290: pure UE02 codec test (no game code, no SDL).
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -O1 -Iinclude \
  src/expanded_car_compact.c tests/test_expanded_car_compact.c -o "$DIR/compact"
"$DIR/compact"
