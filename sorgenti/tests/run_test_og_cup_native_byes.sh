#!/usr/bin/env bash
# No new toolchain, isolated contract test. Called only by AVVIA_DIAGNOSTICA.
set -euo pipefail
cd "$(dirname "$0")/.."
dir="$(mktemp -d)"
trap 'rm -rf "$dir"' EXIT
"${CC:-gcc}" -std=c11 -Wall -Wextra -Werror -O2 \
  -Iinclude tests/test_og_cup_native_byes.c -o "$dir/og_cup_native_byes"
"$dir/og_cup_native_byes"
