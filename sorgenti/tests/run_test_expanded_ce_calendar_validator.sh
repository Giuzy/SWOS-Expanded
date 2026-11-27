#!/usr/bin/env bash
# Independent CE calendar validator: kernel positives + broken calendars. Isolated C only.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude \
  src/expanded_ce_calendar_validator.c tests/test_expanded_ce_calendar_validator.c -o "$DIR/cev"
"$DIR/cev"
