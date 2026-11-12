#!/usr/bin/env bash
# One entry point for the existing C stress, calendar and debug regressions
# plus native disk-data inventory. No Python, no new compiler, no game save edits.
set -euo pipefail
cd "$(dirname "$0")/../.."
run_stage() {
  local label="$1" script="$2"
  printf '\n=======================================================\n'
  printf '  %s\n' "$label"
  printf '=======================================================\n'
  if bash "$script"; then
    printf '[PASS] %s\n' "$label"
  else
    printf '[FAIL] %s\n' "$label"
    return 1
  fi
}
run_stage "70 SEASONS / NATIVE FIXTURES / CUP JOURNAL" "sorgenti/tests/run_test_living_world_70.sh"
run_stage "FOUR-DIVISION 70-SEASON MOVEMENT / SAVE-LOAD" "sorgenti/tests/run_test_living_world_mobility.sh"
run_stage "MIDSEASON JOIN CALENDAR REGRESSIONS" "sorgenti/tests/run_test_expanded_midseason_join.sh"
run_stage "LIVING WORLD UNIT REGRESSIONS" "sorgenti/tests/run_test_expanded_world_live.sh"
run_stage "DEBUG RESULTS / WORLD LEDGER REGRESSIONS" "sorgenti/tests/run_test_expanded_phase12.sh"
run_stage "REAL INSTALLED DATABASE INVENTORY" "sorgenti/tests/run_test_database_inventory.sh"
echo
echo "ENGINE_UNIT_REGRESSIONS=PASS"
echo "ENGINE_SUITE=PASS"
echo "COVERAGE_WARNING=REAL_DB_NOT_70_SEASON_SIMULATED"
echo "MOBILITY_TEST_STATUS=NATIVE_AUTOMATIC_PLACES_TESTED"
echo "MOBILITY_PLAYOFF_STATUS=PENDING_NATIVE_RESOLUTION"
echo "OFFSCREEN_CUP_TEST_STATUS=NOT_IMPLEMENTED"
echo "[NOTE] Only automatic native promotion/relegation places are tested."
echo "       Playoff outcomes, nationwide real DB multi-season coverage, and"
echo "       offscreen worldwide cups are NOT claimed as complete."
