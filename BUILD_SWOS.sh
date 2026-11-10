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
if [[ ! -s "$game/data/expanded/expanded_loading_bg.bmp" ]]; then
  echo "ERRORE: manca data/expanded/expanded_loading_bg.bmp"
  exit 1
fi
# Gli asset approvati sono PNG e atlanti separati dai testi dinamici.
for art in assets/title/buttons/title_primary_blank.png assets/title/buttons/title_secondary_blank.png \
  assets/ui/legends/a_select.png assets/ui/legends/a_select_b_back.png \
  assets/ui/legends/a_select_b_back_x_info.png \
  assets/fonts/new_career/ui_form.png assets/fonts/new_career/ui_form.json \
  assets/fonts/new_career/ui_title.png assets/fonts/new_career/ui_title.json; do
  if [[ ! -s "$game/$art" ]]; then
    echo "ERRORE: manca l'asset approvato: $art" >&2
    exit 1
  fi
done
[[ -s "$src/tools/patch_title_ui_01065.sh" && -s "$src/include/expanded_title_ui_01065.h" ]] || {
  echo 'ERRORE: manca la patch di integrazione Title 0.10.65.' >&2
  exit 1
}
exec > >(tee "$game/COMPILAZIONE_0140.log") 2>&1

echo '==== SWOS Expanded - compilazione Windows x64 ===='
echo "Sorgenti: $src"
echo "Gioco:    $game"
echo 'Verifica delle dipendenze MSYS2 gia installate...'
# Read-only checks: never install or update packages from this build script.
check_dependency() {
  local tool="$1" package="$2"
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "ERRORE: manca $tool nel PATH MINGW64. Pacchetto necessario: $package." >&2
    return 1
  fi
  if ! pacman -Q "$package" >/dev/null 2>&1; then
    echo "ERRORE: pacchetto necessario non installato: $package (per $tool)." >&2
    return 1
  fi
  if ! "$tool" --version >/dev/null 2>&1; then
    echo "ERRORE: $tool non funziona. Verificare il pacchetto $package e le sue dipendenze." >&2
    return 1
  fi
}
if ! command -v pacman >/dev/null 2>&1; then
  echo 'ERRORE: pacman non disponibile per interrogare i pacchetti. Usare la shell MSYS2 MINGW64.' >&2
  exit 1
fi
check_dependency gcc mingw-w64-x86_64-gcc
check_dependency pkg-config mingw-w64-x86_64-pkgconf
check_dependency make make
if [[ "$(gcc -dumpmachine)" != x86_64-w64-mingw32 ]]; then
  echo 'ERRORE: GCC non e MinGW64 x64. Usare MSYS2 MINGW64 e il pacchetto mingw-w64-x86_64-gcc.' >&2
  exit 1
fi
if ! pacman -Q mingw-w64-x86_64-SDL2 >/dev/null 2>&1 ||
   ! pkg-config --exists sdl2 ||
   ! pkg-config --cflags --libs sdl2 >/dev/null 2>&1; then
  echo 'ERRORE: SDL2 di sviluppo non disponibile. Pacchetto necessario: mingw-w64-x86_64-SDL2; verificare il percorso pkg-config MINGW64.' >&2
  exit 1
fi

# The approved Windows icon is a required repository asset, embedded by SWOS.rc.
if [[ ! -s "$src/../development_assets/branding/windows/SWOS_EX.ico" ]]; then
  echo 'ERRORE: manca development_assets/branding/windows/SWOS_EX.ico.'
  exit 1
fi
cd "$src"
# Silent debug remains active. Apply only the Title-B correction to local SDL sources.
# This script is idempotent; no commit/push, and no gameplay edits.
bash "$src/tools/patch_title_b_01063.sh" "$src"
# Approved blank Title buttons + runtime career font + A command legend.
bash "$src/tools/patch_title_ui_01065.sh" "$src"
echo 'Controllo sintattico delle schermate e dei menu modificati...'
gcc -std=gnu11 -fsyntax-only -DSWOS_SDL_BUILD=1 -D_FILE_OFFSET_BITS=64 -include include/debug_no_audio_01060.h -include include/swos_platform.h \
  -Isrc -Iinclude $(pkg-config --cflags sdl2) \
  src/menu_data.c src/menu_system.c src/menus.c src/input_sdl.c src/expanded_career.c \
  src/expanded_economy.c src/platform_sdl.c src/game_loop.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c src/expanded_new_career.c src/expanded_title_overlay.c
echo 'Compilo il gioco... AUDIO DISABILITATO PER DEBUG (fino a nuovo ordine)'
# Silent debugging: force-include debug_no_audio_01060.h before the
# Makefile's swos_platform.h. The header overrides its audio=0 default.
# The Makefile build-flags stamp sees CC change and rebuilds all modules.
# Audio stays disabled until the user explicitly authorizes re-enabling it.
# Gli ZIP preservano i timestamp: dopo l'estrazione i sorgenti possono
# risultare piu' vecchi degli .o, per cui make potrebbe saltare le modifiche.
# Ricompiliamo sempre il numero build e i due renderer HD interessati.
echo 'Versione letta dai sorgenti:'
grep '^#define SWOS_EXPANDED_VERSION ' include/build_info.h || true
printf '%s\n' 'Ricompilo versione, NEW CAREER e Title per evitare oggetti obsoleti.'
touch src/build_info.c src/expanded_new_career.c src/expanded_title_overlay.c
make -j4 NETPLAY=0 CC='gcc -include include/debug_no_audio_01060.h'
if [[ ! -s bin/SWOS.exe ]]; then
  echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
  exit 1
fi
cp -f bin/SWOS.exe "$game/SWOS_Expanded_new.exe"
mv -f "$game/SWOS_Expanded_new.exe" "$game/SWOS_Expanded.exe"
echo 'Compilazione completata: SWOS_Expanded.exe aggiornato.'
