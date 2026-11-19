#!/bin/sh
# Uses the same vendored C sources as the SWOS Windows Makefile.
# This is NOT a gameplay/save-state/Windows verification.
set -eu
cd "$(dirname "$0")/.."
CC="${CC:-gcc}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT HUP INT TERM
FILES="$(find third_party/zstd/lib/common third_party/zstd/lib/compress \
 third_party/zstd/lib/decompress -maxdepth 1 -name '*.c' \
 | sort)"
# shellcheck disable=SC2086
"$CC" -std=gnu11 -O2 -DSWOS_SDL_BUILD=1 -DSWOS_CAR_ZSTD=1 \
 -DZSTD_DISABLE_ASM=1 -DXXH_NAMESPACE=ZSTD_ \
 -Iinclude -Ithird_party/zstd/lib \
 src/save_car_zstd.c tests/test_save_car_zstd.c $FILES -o "$TMP/test_car_zstd"
"$TMP/test_car_zstd"
printf '%s\n' 'OG_WORLD_CAR_SAVE_CONTINUE_WINDOWS=NOT_TESTED_BY_ISOLATED_TEST'
