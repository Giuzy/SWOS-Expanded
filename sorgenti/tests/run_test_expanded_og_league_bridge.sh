#!/usr/bin/env bash
# Pure C only: no Windows game, no CAR and no second simulator.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_expanded_og_league_bridge.c -o "$DIR/og_league_bridge"
"$DIR/og_league_bridge"
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_expanded_og_resident_selection.c -o "$DIR/og_resident_selection"
"$DIR/og_resident_selection"
echo "OG_MANAGER_RESIDENT_WINDOWS=NOT_TESTED"
