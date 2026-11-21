#!/usr/bin/env bash
# Isolated C test of the live, OG-first world tick carrier.
# This does NOT exercise the Copa manager OG menus or CAR reload.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  src/expanded_world_time.c tests/test_expanded_native_timeline.c \
  -o "$DIR/native_timeline"
"$DIR/native_timeline"
echo "NATIVE_TIMELINE_GAMEPLAY=NOT_TESTED"
echo "NATIVE_TIMELINE_CAR_ROUNDTRIP=NOT_TESTED"
