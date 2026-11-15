#!/usr/bin/env bash
# Unit test for debug controls only. Does NOT import/simulate any world.
set -euo pipefail
cd "$(dirname "$0")/.."
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
gcc -std=c11 -Wall -Wextra -Werror -Iinclude \
    src/expanded_debug.c tests/test_expanded_debug.c -o "$dir/expanded_debug"
"$dir/expanded_debug"
echo "OG_DEBUG_CONTROLS=PASS_SOURCE_UNIT_ONLY"
