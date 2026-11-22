#!/usr/bin/env bash
# RG-001/014/027 pure country-edition fixture-boundary regression.
# Does NOT execute Windows gameplay, manager, CAR or native OG matches.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -Wall -Wextra -Werror -pedantic -DFIXTURE_DATA_C=1 \
    -Iinclude src/expanded_world_time.c src/fixture_data.c \
    tests/test_bolivia_country_edition_boundary.c -o "$DIR/bolivia-country-year"
"$DIR/bolivia-country-year"
