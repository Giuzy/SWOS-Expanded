#!/usr/bin/env bash
# v00.13.303: universal round-robin home/away distribution.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -Iinclude tests/test_expanded_round_robin_home_away.c -o "$DIR/rr"
"$DIR/rr"
