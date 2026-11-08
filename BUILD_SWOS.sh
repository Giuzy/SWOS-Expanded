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
# Approved native BMP layers are read by the game at runtime, not converted.
for art in assets/business/business_ui_approved_2026-11-07.bmp assets/ui/controller_abc_approved_2026-11-07.bmp; do
  if [[ ! -s "$game/$art" ]]; then
    echo "ERRORE: asset BMP approvato mancante: $art"
    exit 1
  fi
done
# Full release Title must have all approved PNGs; do not silently use
# missing-sprite placeholders or the previously incorrect letter-D logo.
for art in \
  assets/title/title_background_approved_2026-11-08.png \
  assets/title/swos_expanded_logo_approved_2026-11-08.png \
  assets/title/buttons/career.png \
  assets/title/buttons/hall_of_fame.png \
  assets/title/buttons/classic_modes.png \
  assets/title/buttons/options.png \
  assets/title/buttons/exit_game.png \
  assets/title/buttons/continue_career.png \
  assets/title/buttons/fullscreen_expand.png \
  assets/title/buttons/windowed_collapse.png; do
  if [[ ! -s "$game/$art" ]]; then
    echo "ERRORE RELEASE: asset Title mancante: $art"
    exit 1
  fi
done
logo_hash="$(sha256sum "$game/assets/title/swos_expanded_logo_approved_2026-11-08.png" | cut -d' ' -f1)"
if [[ "$logo_hash" != "8fd34795962704cee3e3242a264d82eeffa4221ab31b1c583bd6df174a9352c6" ]]; then
  echo 'ERRORE RELEASE: il logo non e il PNG corretto approvato.'
  echo 'Sincronizza assets/title/ via GitKraken prima di compilare.'
  exit 1
fi
if [[ ! -s "$game/data/expanded/expanded_loading_bg.bmp" ]]; then
  echo "ERRORE: manca data/expanded/expanded_loading_bg.bmp"
  exit 1
fi
exec > >(tee "$game/COMPILAZIONE_0200.log") 2>&1

echo '==== SWOS Expanded 0.20.00 - compilazione Windows x64 ===='
echo "Sorgenti: $src"
echo "Gioco:    $game"
echo 'Verifica delle dipendenze MSYS2...'
pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkgconf make

if ! command -v gcc >/dev/null 2>&1 || [[ "$(gcc -dumpmachine)" != *mingw* ]]; then
  echo 'ERRORE: compilatore MinGW64 non disponibile. Avvia MSYS2 MINGW64 e aggiorna con pacman -Syu.'
  exit 1
fi
# The approved Windows icon is a required repository asset, embedded by SWOS.rc.
if [[ ! -s "$src/../development_assets/branding/windows/SWOS_EX.ico" ]]; then
  echo 'ERRORE: manca development_assets/branding/windows/SWOS_EX.ico.'
  exit 1
fi
cd "$src"
echo 'Controllo sintattico delle schermate e dei menu modificati...'
gcc -std=gnu11 -fsyntax-only -DSWOS_SDL_BUILD=1 -D_FILE_OFFSET_BITS=64 -include include/swos_platform.h \
  -Isrc -Iinclude $(pkg-config --cflags sdl2) \
  src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
  src/expanded_economy.c src/platform_sdl.c src/game_loop.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
echo 'Compilo il gioco 0.20.00...'
make -j4 NETPLAY=0
if [[ ! -s bin/SWOS.exe ]]; then
  echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
  exit 1
fi
cp -f bin/SWOS.exe "$game/SWOS_Expanded_new.exe"
mv -f "$game/SWOS_Expanded_new.exe" "$game/SWOS_Expanded.exe"
echo 'Compilazione completata: SWOS_Expanded.exe aggiornato.'
