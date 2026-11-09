#!/usr/bin/env bash
# SWOS Expanded 0.10.63 — idempotent Title-only B button fix.
# The full source files are provided by the user's local GitKraken checkout.
# Only two verified SDL input fragments are changed. Abort on unexpected source.
set -Eeuo pipefail
src="${1:?repository sorgenti path required}"
p="$src/src/platform_sdl.c"
i="$src/src/input_sdl.c"
[[ -f "$p" && -f "$i" ]] || { echo 'ERRORE: sorgenti SDL non presenti'; exit 1; }

# Validate BOTH original/installed source shapes before changing either file.
if ! grep -Fq 'if (Expanded_TitleMenu_Level() == 1) Expanded_TitleMenu_QueueBack();' "$p"; then
  if ! grep -Fq 'expanded_title_main_active() &&' "$p" ||
     ! grep -Fq 'Expanded_TitleMenu_QueueBack();' "$p"; then
    echo 'ERRORE: gestore B nel Title non riconosciuto (platform_sdl.c). Nessuna modifica.' >&2
    exit 1
  fi
fi
if ! grep -Fq 'input_on_title_first_level(void)' "$i"; then
  if ! grep -Fq '#include <string.h>' "$i" ||
     ! grep -Fq '(!Expanded_UI_IsBusinessScreen() &&' "$i" ||
     ! grep -Fq 'for (int b = 0; b < nbtns; ++b) {' "$i"; then
    echo 'ERRORE: gestore del joypad non riconosciuto (input_sdl.c). Nessuna modifica.' >&2
    exit 1
  fi
fi

# B is BACK in CLASSIC MODES (level 1), but ignored at top-level (level 0).
# Keyboard ESC and the visible EXIT GAME button retain their original behavior.
if ! grep -Fq 'if (Expanded_TitleMenu_Level() == 1) Expanded_TitleMenu_QueueBack();' "$p"; then
  sed -i '/expanded_title_main_active() &&/,/continue;/ {
    s/^[[:space:]]*Expanded_TitleMenu_QueueBack();$/                if (Expanded_TitleMenu_Level() == 1) Expanded_TitleMenu_QueueBack();/
  }' "$p"
fi
if ! grep -Fq 'if (Expanded_TitleMenu_Level() == 1) Expanded_TitleMenu_QueueBack();' "$p"; then
  echo 'ERRORE: impossibile aggiornare platform_sdl.c' >&2; exit 1
fi

# The SDL controller poll ALSO treats B as FIRE. Suppress that mapping only
# at first-level Title, otherwise B could select EXIT GAME instead of quitting.
# Generic unmapped gamepads' button 1 is treated the same way.
if ! grep -Fq 'input_on_title_first_level(void)' "$i"; then
  sed -i '/^#include <string.h>$/a\
/* NEW CAREER 0.10.63: B does not act as fire at the first Title level. */\
extern uintptr_t _currentMenuPtr;\
extern uint8_t _swosMainMenu[];\
extern int Expanded_TitleMenu_Level(void);\
static int input_on_title_first_level(void) {\
    return _currentMenuPtr == (uintptr_t)_swosMainMenu && Expanded_TitleMenu_Level() == 0;\
}' "$i"
fi
if ! grep -Fq '(!Expanded_UI_IsBusinessScreen() && !input_on_title_first_level() &&' "$i"; then
  sed -i 's/(!Expanded_UI_IsBusinessScreen() \&\&/(!Expanded_UI_IsBusinessScreen() \&\& !input_on_title_first_level() \&\&/' "$i"
fi
if ! grep -Fq 'if (b == 1 && input_on_title_first_level()) continue;' "$i"; then
  sed -i '/for (int b = 0; b < nbtns; ++b) {/a\            if (b == 1 \&\& input_on_title_first_level()) continue;' "$i"
fi
if ! grep -Fq 'if (skip && !input_on_title_first_level() && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))' "$i"; then
  sed -i 's/if (skip \&\& SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))/if (skip \&\& !input_on_title_first_level() \&\& SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))/' "$i"
fi
if ! grep -Fq 'if (skip && !input_on_title_first_level() && nbtns >= 2 && SDL_JoystickGetButton(raw, 1))' "$i"; then
  sed -i 's/if (skip \&\& nbtns >= 2 \&\& SDL_JoystickGetButton(raw, 1))/if (skip \&\& !input_on_title_first_level() \&\& nbtns >= 2 \&\& SDL_JoystickGetButton(raw, 1))/' "$i"
fi

for required in \
 'input_on_title_first_level(void)' \
 '(!Expanded_UI_IsBusinessScreen() && !input_on_title_first_level() &&' \
 'if (b == 1 && input_on_title_first_level()) continue;' \
 'if (skip && !input_on_title_first_level() && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))' \
 'if (skip && !input_on_title_first_level() && nbtns >= 2 && SDL_JoystickGetButton(raw, 1))'; do
  if ! grep -Fq "$required" "$i"; then
    echo "ERRORE: patch controller incompleta, manca $required" >&2; exit 1
  fi
done
# sed -i updates the two source mtimes only when the patch is applied.
# Repeated launches do not recompile the giant SDL modules unnecessarily.
echo '[title-ui] B spento SOLO al primo livello; CLASSIC MODES B=BACK; A invariato.'
