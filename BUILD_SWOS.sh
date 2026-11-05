#!/usr/bin/env bash
set -Eeuo pipefail
if ! command -v cygpath >/dev/null 2>&1; then
  echo 'ERRORE: questo script richiede MSYS2 su Windows.'
  exit 1
fi
: "${SWOS_SRC_WIN:?Percorso sorgenti mancante}"
: "${SWOS_GAME_WIN:?Percorso gioco mancante}"
src="$(cygpath -u "$SWOS_SRC_WIN")"
game="$(cygpath -u "$SWOS_GAME_WIN")"
if [[ ! -f "$src/Makefile" || ! -f "$src/src/menu_data.c" ]]; then
  echo 'ERRORE: sorgenti incompleti.'
  exit 1
fi
if [[ ! -f "$game/SDL2.dll" || ! -f "$game/data/TEAM.008" ]]; then
  echo 'ERRORE: file di gioco mancanti (SDL2.dll o dati squadre).'
  exit 1
fi
# The new art is required by the pixel-art menu, not an optional download.
for art in expanded_title_bg.raw expanded_career_bg.raw expanded_calendar_bg.raw expanded_unemployed_bg.raw expanded_offer_bg.raw expanded_welcome_bg.raw expanded_sacked_popup.raw expanded_record_bg.raw expanded_squad_bg.raw club_business_bg.raw club_stadium_1.raw club_stadium_2.raw club_stadium_3.raw club_stadium_4.raw club_stadium_5.raw; do
  if [[ ! -s "$game/data/expanded/$art" ]]; then
    echo "ERRORE: manca data/expanded/$art"
    exit 1
  fi
done
if [[ ! -s "$game/data/expanded/expanded_loading_bg.bmp" ]]; then
  echo "ERRORE: manca data/expanded/expanded_loading_bg.bmp"
  exit 1
fi
exec > >(tee "$game/COMPILAZIONE_0140.log") 2>&1

echo '==== SWOS Expanded 0.10.40 - compilazione Windows x64 ===='
echo "Sorgenti: $src"
echo "Gioco:    $game"
echo 'Verifica delle dipendenze MSYS2...'
pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkgconf make

if ! command -v gcc >/dev/null 2>&1 || [[ "$(gcc -dumpmachine)" != *mingw* ]]; then
  echo 'ERRORE: compilatore MinGW64 non disponibile. Avvia MSYS2 MINGW64 e aggiorna con pacman -Syu.'
  exit 1
fi
cd "$src"
echo 'Controllo sintattico delle schermate e dei menu modificati...'
gcc -std=gnu11 -fsyntax-only -DSWOS_SDL_BUILD=1 -D_FILE_OFFSET_BITS=64 -include include/swos_platform.h \
  -Isrc -Iinclude $(pkg-config --cflags sdl2) \
  src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
  src/expanded_economy.c src/platform_sdl.c src/game_loop.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
echo 'Compilo il gioco 0.10.40...'
make -j4 NETPLAY=0
if [[ ! -s bin/SWOS.exe ]]; then
  echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
  exit 1
fi
cp -f bin/SWOS.exe "$game/SWOS_0140_new.exe"
mv -f "$game/SWOS_0140_new.exe" "$game/SWOS.exe"
echo 'Compilazione completata: SWOS.exe aggiornato.'
