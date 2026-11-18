#!/usr/bin/env bash
# Isolated C scheduler and real Gregorian library. Does NOT simulate OG games.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  src/expanded_world_time.c tests/test_expanded_event_scheduler.c \
  -o "$DIR/event_scheduler"
"$DIR/event_scheduler"
echo "POINT3_GAMEPLAY=NOT_TESTED"
echo "POINT3_ATOMIC_CAR_INTEGRATION=NOT_IMPLEMENTED"
