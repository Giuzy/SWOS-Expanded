#!/usr/bin/env bash
# Isolated immutable fixture-format check. NOT OG gameplay or CAR restore.
set -euo pipefail
cd "$(dirname "$0")/../.."
cc="${CC:-gcc}"
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
"$cc" -std=c11 -Wall -Wextra -Werror -pedantic -Isorgenti/include \
    sorgenti/tests/test_bolivia_cup_sequence.c -o "$out"
"$out"
