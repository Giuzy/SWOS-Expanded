#!/bin/sh
# Isolated: no Windows toolchain change, no fixture simulation.
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$(mktemp "${TMPDIR:-/tmp}/swos-bolivia-action.XXXXXX")
trap 'rm -f "$BIN"' EXIT HUP INT TERM
"${CC:-gcc}" -std=c11 -Wall -Wextra -Werror -pedantic \
  -I"$ROOT/include" \
  "$ROOT/src/expanded_world_time.c" \
  "$ROOT/tests/test_bolivia_career_action.c" \
  -o "$BIN"
"$BIN"
