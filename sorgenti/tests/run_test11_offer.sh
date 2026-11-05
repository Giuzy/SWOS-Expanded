#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cc -std=c11 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer \
   -ffunction-sections -fdata-sections -Iinclude -Wl,--gc-sections \
   tests/test11_offer.c src/expanded_career.c -o /tmp/swos_test11_offer
ASAN_OPTIONS=detect_leaks=0 /tmp/swos_test11_offer
