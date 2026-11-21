#!/usr/bin/env bash
# ISOLATED Bolivia data/scheduler contract; NOT OG gameplay or CAR QA.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -DFIXTURE_DATA_C=1 \
  -Iinclude src/expanded_world_time.c src/fixture_data.c \
  tests/test_bolivia_event_registry.c -o "$DIR/bolivia_event_registry"
"$DIR/bolivia_event_registry"
