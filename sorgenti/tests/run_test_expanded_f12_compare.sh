#!/usr/bin/env bash
# F12 PRE/POST vs PRE/LIVE comparison, pure C read-only, no SWOS gameplay.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_expanded_f12_compare.c -o "$work/test_f12_compare"
"$work/test_f12_compare"
echo "F12_WINDOWS_RENDER=NOT_TESTED"
echo "CAR_GAMEPLAY_ROUNDTRIP=NOT_TESTED"
