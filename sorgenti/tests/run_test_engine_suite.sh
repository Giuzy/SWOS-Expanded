#!/usr/bin/env bash
# Actual OG-related unit/data checks, not a synthetic second game world.
# Runtime gameplay is assessed separately by ANALIZZA_SWOS_LOG.bat.
set -uo pipefail
cd "$(dirname "$0")/../.."
failed=0
run_stage() {
  local label="$1" script="$2"
  printf '\n=====================================================\n%s\n=====================================================\n' "$label"
  if /usr/bin/bash "$script"; then
    printf '[PASS] %s (headless source/data ONLY)\n' "$label"
  else
    printf '[FAIL] %s\n' "$label"
    failed=$((failed+1))
  fi
}
run_stage "DEBUG INPUT RESULT CONTROL UNITS (NO WORLD)" "sorgenti/tests/run_test_expanded_debug_only.sh"
run_stage "OG RUNTIME AUDITOR REGRESSION FIXTURES (NOT GAMEPLAY)" "sorgenti/tests/run_test_og_runtime_parser.sh"
run_stage "OG CIVIL DATE AND NATIVE PHASE MATHEMATICS" "sorgenti/tests/run_test_expanded_world_time.sh"
run_stage "COMPETITION ENGINE EVENT SCHEDULER (ISOLATED, NO CAR)" "sorgenti/tests/run_test_expanded_event_scheduler.sh"
run_stage "COMPETITION ENGINE TWO-SEASON HISTORY (ISOLATED, NO CAR)" "sorgenti/tests/run_test_expanded_competition_history.sh"
run_stage "OG MANAGER FIXTURE KEYCACHE MATCH (READ-ONLY, NO CAR)" "sorgenti/tests/run_test_expanded_og_cache_match.sh"
run_stage "OG MATCH OCCURRENCE AND 3/4 CYCLES" "sorgenti/tests/run_test_native_history_occurrence.sh"
run_stage "MIDSEASON NATIVE PHASE DATE MATH" "sorgenti/tests/run_test_expanded_midseason_join.sh"
run_stage "INSTALLED OG COMPETITION DATA FORMATS" "sorgenti/tests/run_audit_competition_formats.sh"
run_stage "INSTALLED NATIVE TEAM DATA INVENTORY" "sorgenti/tests/run_test_database_inventory.sh"
echo
if ((failed)); then
  printf 'OG_WORLD_STATIC=FAIL failed-stages=%u\n' "$failed"
  exit 1
fi
echo "OG_WORLD_STATIC=PASS"
echo "OG_REAL_GAMEPLAY=NOT_TESTED_BY_HEADLESS_TESTS"
echo "OG_CUP_RESIDENT_WORLD=NOT_TESTED"
echo "OG_CUP_DATED_FIXTURES=NOT_TESTED"
echo "OG_WORLD_SAVE_LOAD=NOT_TESTED"
echo "OG_70_NATIVE_SEASONS=NOT_TESTED_BY_HEADLESS_CHECKS"
echo "OG_RETIREMENTS=NOT_TESTED"
echo "OG_LEAGUE_STANDINGS_AFTER_HIRE=NOT_TESTED"
echo "WINDOWS_NEXT_STEP=AVVIA_DIAGNOSTICA.bat_AND_SWOS_RUNTIME_AUDIT.log"
echo "[NOTE] No autonomous world score generator is run here."
