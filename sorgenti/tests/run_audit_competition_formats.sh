#!/usr/bin/env bash
# Parse all installed OG competition formats with the game's mjson.
# DATA FORMAT VALIDATION ONLY — never proof of completed cups or leagues.
set -euo pipefail
cd "$(dirname "$0")/../.."
CC=gcc
DIR="$(mktemp -d)"
trap 'rm -rf "$DIR"' EXIT
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -I sorgenti/include \
    -Wno-unused-but-set-variable -DMJSON_REALLOC=realloc -include stdlib.h \
    -c sorgenti/src/mjson.c -o "$DIR/mjson.o"
"$CC" -std=c11 -O2 -Wall -Wextra -Werror -DSWOS_SDL_BUILD -I sorgenti/include \
    sorgenti/tests/audit_competition_formats.c "$DIR/mjson.o" \
    -o "$DIR/audit_competition_formats"
"$DIR/audit_competition_formats" data_json/competitions
