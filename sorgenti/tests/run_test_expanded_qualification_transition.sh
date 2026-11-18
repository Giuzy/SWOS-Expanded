#!/usr/bin/env bash
# Static identity-matcher only. Runtime rollover must be checked in SWOS.log.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -Iinclude \
  tests/test_expanded_qualification_transition.c -o "$work/qual_audit"
"$work/qual_audit"
