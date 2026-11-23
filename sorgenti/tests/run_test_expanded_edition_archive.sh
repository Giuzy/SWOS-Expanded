#!/usr/bin/env bash
# v00.13.290: pure edition-archive test (headers + one .c, no SDL, no game).
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -O1 -Iinclude \
  src/expanded_edition_archive.c tests/test_expanded_edition_archive.c -o "$DIR/archive"
"$DIR/archive"
