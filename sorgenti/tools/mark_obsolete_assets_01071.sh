#!/usr/bin/env bash
# Non-destructive classification of superseded runtime art, based on current
# active C source references AFTER the 0.10.71 patches have been applied.
# Never delete/overwrite, never touch historic raw/BMP still in use.
set -Eeuo pipefail
src="${1:?sorgenti directory}" game="${2:?game directory}"
obsolete=(
 'data/expanded/expanded_loading_bg.png'
 'data/expanded/expanded_loading_bg.bmp'
 'data/expanded/expanded_loading_title.bmp'
 'data/expanded/expanded_loading_caption.bmp'
 'assets/title/buttons/career.png'
 'assets/title/buttons/hall_of_fame.png'
 'assets/title/buttons/classic_modes.png'
 'assets/title/buttons/options.png'
 'assets/title/buttons/exit_game.png'
 'assets/title/buttons/continue_career.png'
)
for asset in "${obsolete[@]}";do
  path="$game/$asset"
  [[ -f "$path" ]] || continue
  ext="${asset##*.}"
  without_ext="${asset%.*}"
  destination="$game/${without_ext}_OBSOLETO.${ext}"
  # Look for the exact runtime path string, not a broad basename.
  if grep -RFq "\"$asset\"" "$src/src" "$src/include" 2>/dev/null;then
      echo "[OBSOLETO] in uso, NON rinominato: $asset"
      continue
  fi
  if [[ -e "$destination" ]];then
      echo "[OBSOLETO] gia esistente, non sovrascrivo: $destination"
      continue
  fi
  mv -- "$path" "$destination"
  echo "[OBSOLETO] $asset -> ${without_ext}_OBSOLETO.${ext}"
done
# Update the project rule in the same authorized code build, not in a
# separate docs-only commit. Only append when the local checkout has AGENTS.
if [[ -f "$game/AGENTS.md" ]] &&
   ! grep -Fq 'OBSOLETO — marcatura degli asset sostituiti (2026-11-10)' "$game/AGENTS.md";then
cat >> "$game/AGENTS.md" <<'RULE'

### OBSOLETO — marcatura degli asset sostituiti (2026-11-10)
Quando un asset approvato o storico viene sostituito da un nuovo asset runtime,
marcare il vecchio con suffisso `_OBSOLETO` prima dell'estensione, senza
sovrascriverlo e **solo dopo** avere verificato che nessun sorgente attivo lo
richiami. Lo stesso controllo si applica agli asset sostituiti in precedenza.
L'etichetta non autorizza una cancellazione immediata: il BAT periodico di
pulizia dovra' ricontrollare riferimenti, eventuali backup e file effettivamente
necessari prima di proporre la rimozione. Non rinominare mai un file ancora
richiamato dal runtime senza aggiornare tutti i relativi riferimenti.
RULE
  echo '[OBSOLETO] regola aggiunta ad AGENTS.md nella copia locale (nessun commit).'
fi
