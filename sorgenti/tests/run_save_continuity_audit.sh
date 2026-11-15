#!/usr/bin/env bash
# Read-only cross-session hints. Cannot prove complete OG save without native
# league/cup/player/fixture IDs and RNG in the serialized file.
set -uo pipefail
if [[ -z "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=NO_DIAGNOSTIC_SESSION"
 exit 2
fi
dir="$(cygpath -u "$SWOS_DIAGNOSTIC_DIR")"
current="$dir/SWOS.log"
old="$dir/LOG_PRECEDENTI/SWOS_PRE_SESSIONE.log"
if [[ ! -f "$current" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=CURRENT_LOG_MISSING"
 exit 2
fi
newRestore=$(grep -F '[og-feature-audit] stage=RESTORE_AFTER ' "$current" | tail -n 1 || true)
newAuto=$(grep -F '[autosave] restored career from ' "$current" | tail -n 1 || true)
if [[ -z "$newRestore" || -z "$newAuto" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=NO_LOAD_PERFORMED_IN_GAME"
 exit 2
fi
echo "OG_RESTORE_SEEN=$newRestore"
echo "OG_RESTORE_CAREER=$newAuto"
if [[ ! -f "$old" ]]; then
 echo "OG_CROSS_SESSION_SAVE=RESTORE_OBSERVED_PREVIOUS_SAVE_NOT_AVAILABLE"
 exit 2
fi
oldSave=$(grep -F '[og-feature-audit] stage=SAVE_AFTER ' "$old" | tail -n 1 || true)
oldAuto=$(grep -F '[autosave] saved ' "$old" | tail -n 1 || true)
if [[ -z "$oldSave" || -z "$oldAuto" ]]; then
 echo "OG_CROSS_SESSION_SAVE=RESTORE_OBSERVED_PREVIOUS_SAVE_SNAPSHOT_MISSING"
 exit 2
fi
echo "OG_PREVIOUS_SAVE_SEEN=$oldSave"
echo "OG_PREVIOUS_SAVE_CAREER=$oldAuto"
# Cross-session log snapshots are not necessarily the same save, and
# snapshots in RAM do not prove serialization. Never return COMPLETE/PASS.
echo "OG_CROSS_SESSION_SAVE=SNAPSHOTS_AVAILABLE_IDENTITY_AND_FIXTURE_RNG_NOT_VERIFIED"
echo "SAVE_LOAD_RESULT=NOT_PROVEN_BY_LOG_COUNTERS"
exit 0
