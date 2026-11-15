#!/usr/bin/env bash
# Historical offer contract regression only, not an OG in-game test.
# No fixed /tmp executable; keep every temporary binary isolated.
set -euo pipefail
cd "$(dirname "$0")/.."
CC="${CC:-cc}"
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
COMMON=(-std=c11 -g -O1 -fno-omit-frame-pointer
        -ffunction-sections -fdata-sections -Iinclude -Wl,--gc-sections)
if printf 'int main(void){return 0;}\n' |
   "$CC" -x c -std=c11 -fsanitize=address,undefined - -o "$DIR/sanitizer_probe" \
     > "$DIR/sanitizer_probe.log" 2>&1; then
   COMMON+=(-fsanitize=address,undefined)
   echo "OFFER_SANITIZERS=ASAN_UBSAN"
else
   echo "OFFER_SANITIZERS=NOT_AVAILABLE fallback=ordinary-compiler"
fi
"$CC" "${COMMON[@]}" tests/test11_offer.c src/expanded_career.c -o "$DIR/swos_test11_offer"
ASAN_OPTIONS=detect_leaks=0 "$DIR/swos_test11_offer"
