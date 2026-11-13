#!/usr/bin/env bash
# One entry point for the existing C stress, calendar and debug regressions
# plus native disk-data inventory. No Python, no new compiler, no game save edits.
set -uo pipefail
cd "$(dirname "$0")/../.."
failed_stages=0
run_stage() {
  local label="$1" script="$2"
  printf '\n=======================================================\n'
  printf '  %s\n' "$label"
  printf '=======================================================\n'
  if bash "$script"; then
    printf '[PASS] %s\n' "$label"
  else
    printf '[FAIL] %s\n' "$label"
    failed_stages=$((failed_stages+1))
    return 0
  fi
}
run_stage "70 SEASONS / NATIVE FIXTURES / CUP JOURNAL" "sorgenti/tests/run_test_living_world_70.sh"
run_stage "FOUR-DIVISION 70-SEASON MOVEMENT / SAVE-LOAD" "sorgenti/tests/run_test_living_world_mobility.sh"
run_stage "MIDSEASON JOIN CALENDAR REGRESSIONS" "sorgenti/tests/run_test_expanded_midseason_join.sh"
run_stage "LIVING WORLD UNIT REGRESSIONS" "sorgenti/tests/run_test_expanded_world_live.sh"
run_stage "DEBUG RESULTS / WORLD LEDGER REGRESSIONS" "sorgenti/tests/run_test_expanded_phase12.sh"
run_stage "REAL INSTALLED DATABASE INVENTORY" "sorgenti/tests/run_test_database_inventory.sh"
run_stage "FULL DATABASE AUDIT / DOMESTIC 70 YEARS" "sorgenti/tests/run_test_real_json_world_70.sh"
echo
if (( failed_stages )); then
  printf 'ENGINE_SUITE=FAIL STAGES=%u\n' "$failed_stages"
  exit 1
fi
echo "ENGINE_UNIT_REGRESSIONS=PASS"
echo "ENGINE_SUITE=PASS"
echo "ENGINE_SUITE_SCOPE=HEADLESS_RUNTIME_ONLY"
echo "OG_LWE_WINDOWS_JOIN=NOT_TESTED"
echo "OG_CUP_WINDOWS_OFFSCREEN=NOT_TESTED"
echo "F10_WINDOWS_IMPORT=NOT_TESTED"
echo "DOMESTIC_GLOBAL_STRESS=ALL_FILES_ONE_WORLD_70_SEASONS"
echo "COVERAGE_WARNING=REAL_JSON_DOMESTIC_ONLY_CUPS_NATIONAL_TEAMS_UNTESTED"
echo "MOBILITY_TEST_STATUS=NATIVE_AUTOMATIC_PLACES_TESTED"
echo "MOBILITY_PLAYOFF_STATUS=PENDING_NATIVE_RESOLUTION"
echo "OFFSCREEN_CUP_TEST_STATUS=NOT_PROVEN_FOR_ALL_COUNTRIES"
echo "F12_CUP_INSPECTOR_STATUS=OG_OBSERVED_EVENTS_ONLY"
echo "NONDOMESTIC_COMPETITIONS_STATUS=NOT_SIMULATED_BY_JSON_LEAGUE_TEST"
echo "[NOTE] Only automatic native promotion/relegation places are tested."
echo "       OG playoff and complete worldwide offscreen cups are NOT proven."
echo "       Non-domestic JSON pools are inventoried, NOT fake leagues."

echo "DATABASE_96_97_EXPECTED_TEAMS=1620 PLAYER_SLOTS=26820"
echo "OG_CUP_UNEMPLOYED_RUNTIME=NOT_TESTED"
echo "F10_RUNTIME_COVERAGE=NOT_VERIFIED"
