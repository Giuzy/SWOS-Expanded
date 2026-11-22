#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
CC="${CC:-gcc}"
OUT="${TMPDIR:-/tmp}/swos_competition_expiry_${USER:-test}_$$"
trap 'rm -f "$OUT"' EXIT
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -pedantic \
  -I "$ROOT/include" \
  "$ROOT/tests/test_competition_expiry_gate.c" -o "$OUT"
"$OUT"
