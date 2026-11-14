#!/usr/bin/env bash
# Runs real code shared with the OG/LWE native import. Headless, not Windows OG.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -Iinclude \
  tests/test_native_history_occurrence.c -o "$DIR/native_history"
"$DIR/native_history"
