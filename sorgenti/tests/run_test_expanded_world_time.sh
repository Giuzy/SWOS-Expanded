#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/.."
CC=cc
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude src/expanded_world_time.c tests/test_expanded_world_time.c -o "$DIR/world_time"
"$DIR/world_time"
"$CC" -std=c11 -Wall -Wextra -Werror -O2 -Iinclude src/expanded_world_time.c tests/test_expanded_world_time_all_days.c -o "$DIR/world_time_all_days"
"$DIR/world_time_all_days"
