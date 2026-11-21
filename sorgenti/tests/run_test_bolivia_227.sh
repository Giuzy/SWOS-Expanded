#!/usr/bin/env bash
# RG-001/RG-014 ISO stage: original MSYS2/GCC or Linux GCC only.
set -euo pipefail
cd "$(dirname "$0")/.."
artifact="$(mktemp "${TMPDIR:-/tmp}/swos-bolivia-227.XXXXXX")"
trap 'rm -f "$artifact"' EXIT
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -Iinclude \
  tests/test_bolivia_227.c -o "$artifact"
"$artifact"
