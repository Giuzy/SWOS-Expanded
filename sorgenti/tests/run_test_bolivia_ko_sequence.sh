#!/usr/bin/env bash
# Static C11 test of the 2026 knockout fixture identities, NOT OG gameplay.
set -euo pipefail
cd "$(dirname "$0")/../.."
cc="${CC:-gcc}"
out="$(mktemp)"
trap 'rm -f "$out"' EXIT
"$cc" -std=c11 -Wall -Wextra -Werror -pedantic -Isorgenti/include \
    sorgenti/tests/test_bolivia_ko_sequence.c -o "$out"
"$out"
