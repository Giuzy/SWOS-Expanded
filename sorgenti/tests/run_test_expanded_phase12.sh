#!/usr/bin/env bash
# GCC-only tests. No SDL, no game data, no change to installed toolchain.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude src/expanded_debug.c tests/test_expanded_debug.c -o "$DIR/debug"
"$DIR/debug"
for case_name in world_ledger world_35weeks; do
  "$CC" -std=c11 -Wall -Wextra -Werror -O2 -Iinclude \
    src/expanded_world_time.c src/expanded_world_ledger.c \
    "tests/test_expanded_${case_name}.c" -o "$DIR/${case_name}"
  "$DIR/${case_name}"
done
