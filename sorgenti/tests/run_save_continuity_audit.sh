#!/usr/bin/env bash
# Cross-process OG resident-world continuity probe. Read-only: never writes
# to a .CAR, never synthesizes a match and never certifies the full save.
# Requires two Windows AVVIA_DIAGNOSTICA sessions of the SAME world/save slot:
# first SAVE + exit, second CONTINUE without FORWARD TIME.
set -uo pipefail
if [[ -z "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=NO_DIAGNOSTIC_SESSION"
 exit 2
fi
if command -v cygpath >/dev/null 2>&1; then
 dir="$(cygpath -u "$SWOS_DIAGNOSTIC_DIR")"
else
 dir="$SWOS_DIAGNOSTIC_DIR"
fi
current="$dir/SWOS.log"
old="$dir/LOG_PRECEDENTI/SWOS_PRE_SESSIONE.log"
if [[ ! -f "$current" || ! -f "$old" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=PREVIOUS_OR_CURRENT_LOG_MISSING"
 exit 2
fi
last_log(){ grep -F "$2" "$1" | tail -n 1 || true; }
oldAuto="$(last_log "$old" '[autosave] saved ')"
newAuto="$(last_log "$current" '[autosave] restored career from ')"
oldFeat="$(last_log "$old" '[og-feature-audit] stage=SAVE_AFTER ')"
newFeat="$(last_log "$current" '[og-feature-audit] stage=RESTORE_AFTER ')"
if [[ -z "$oldAuto" || -z "$newAuto" || -z "$oldFeat" || -z "$newFeat" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=SAVE_OR_RESTORE_CHECKPOINT_ABSENT"
 exit 2
fi
world_of(){
 printf '%s\n' "$1" | grep -oE 'world=(9697|2627)' | tail -n 1 | cut -d= -f2 || true
}
file_of(){
 printf '%s\n' "$1" | grep -oE 'AUTO(9697|2627)\.CAR' | tail -n 1 || true
}
oldWorld="$(world_of "$oldAuto")"
newWorld="$(world_of "$newAuto")"
oldSlot="$(file_of "$oldAuto")"
newSlot="$(file_of "$newAuto")"
if [[ -z "$oldWorld" || "$oldWorld" != "$newWorld" ||
      -z "$oldSlot" || "$oldSlot" != "$newSlot" ]]; then
 echo "OG_CROSS_SESSION_SAVE=NOT_TESTED reason=DIFFERENT_WORLD_OR_SAVE_SLOT old=$oldWorld/$oldSlot new=$newWorld/$newSlot"
 exit 2
fi
echo "OG_SAVE_WORLD=$newWorld OG_SAVE_SLOT=$newSlot"
echo "OG_PREVIOUS_SAVE_SEEN=$oldFeat"
echo "OG_RESTORE_SEEN=$newFeat"
field(){
 printf '%s\n' "$1" | grep -oE "(^|[[:space:]])$2=[0-9]+" | tail -n 1 | cut -d= -f2 || true
}
oldYear="$(field "$oldFeat" year)"
newYear="$(field "$newFeat" year)"
oldLeagues="$(field "$oldFeat" resident-leagues)"
newLeagues="$(field "$newFeat" resident-leagues)"
oldCups="$(field "$oldFeat" cup-resident)"
newCups="$(field "$newFeat" cup-resident)"
if [[ -z "$oldYear" || "$oldYear" != "$newYear" ]]; then
 echo "OG_CROSS_SESSION_SAVE=REVIEW reason=YEAR_MISMATCH old=$oldYear new=$newYear"
 exit 2
fi
if [[ -n "$oldLeagues" && "$oldLeagues" -gt 0 &&
      ( -z "$newLeagues" || "$newLeagues" -eq 0 ) ]]; then
 echo "OG_CROSS_SESSION_SAVE=FAIL reason=ALL_NATIVE_LEAGUES_ABSENT_AFTER_RESTORE saved=$oldLeagues restored=${newLeagues:-missing}"
 exit 1
fi
if [[ -n "$oldCups" && "$oldCups" -gt 0 &&
      ( -z "$newCups" || "$newCups" -eq 0 ) ]]; then
 echo "OG_CROSS_SESSION_SAVE=FAIL reason=ALL_NATIVE_CUPS_ABSENT_AFTER_RESTORE saved=$oldCups restored=${newCups:-missing}"
 exit 1
fi
# Extract only the LAST complete fingerprint block for the requested world
# and stage. Prior saves/loads in the same run must not create duplicates.
last_block(){
 awk -v marker="$2" -v world="$3" '
   index($0,"[og-state-fp] stage=" marker " world=" world " ") { block=block $0 "\n"; }
   index($0,"[og-state-fp-summary] stage=" marker " world=" world " ") {
      latest=block;block="";
   }
   END { printf "%s",latest; }
 ' "$1" | sed -E 's/stage=(SAVE_AFTER|RESTORE_AFTER) //' | LC_ALL=C sort
}
oldFP="$(last_block "$old" SAVE_AFTER "$newWorld")"
newFP="$(last_block "$current" RESTORE_AFTER "$newWorld")"
if [[ -z "$oldFP" || -z "$newFP" ]]; then
 echo "OG_NATIVE_FINGERPRINTS=NOT_TESTED reason=SOURCE_BUILD_WITHOUT_FINGERPRINTS"
 echo "OG_CROSS_SESSION_SAVE=NOT_PROVEN_BY_COUNTERS"
 exit 2
fi
oldCount="$(printf '%s\n' "$oldFP" | wc -l | tr -d ' ')"
newCount="$(printf '%s\n' "$newFP" | wc -l | tr -d ' ')"
echo "OG_NATIVE_FINGERPRINT_COUNT saved=$oldCount restored=$newCount"
if [[ "$oldFP" != "$newFP" ]]; then
 echo "OG_NATIVE_FINGERPRINTS=FAIL reason=IDENTITY_FIXTURE_OR_ROSTER_FIELD_DIFFERENCE"
 diff -u <(printf '%s\n' "$oldFP") <(printf '%s\n' "$newFP") | head -n 60 || true
 echo "OG_CROSS_SESSION_SAVE=INCONSISTENT_OG_RESIDENT_STATE"
 exit 1
fi
echo "OG_NATIVE_FINGERPRINTS=MATCH_POINTER_FREE_FIELDS world=$newWorld count=$oldCount"
echo "OG_CROSS_SESSION_SAVE=INCOMPLETE_NOT_FULL_SERIALIZATION_PROOF"
echo "SAVE_LOAD_RESULT=NOT_PROVEN_UNTIL_ALL_OG_CONTEXTS_AND_CAR_CONTENT_AUDITED"
exit 2
