#!/usr/bin/env bash
# Pure C check using the real Gregorian code; does NOT write a CAR file.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  src/expanded_world_time.c tests/test_expanded_competition_history.c \
  -o "$DIR/competition_history"
"$DIR/competition_history"
echo "HISTORY_OG_SINGLE_WRITER=NOT_CONNECTED"
echo "HISTORY_WINDOWS_GAMEPLAY=NOT_TESTED"
