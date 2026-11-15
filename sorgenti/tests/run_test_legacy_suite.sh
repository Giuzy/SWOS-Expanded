#!/usr/bin/env bash
# HISTORICAL isolated checks only. The old LWE runner is NOT a game engine,
# its simulated tables are not OG career results and never count as a native
# gameplay or resident-world PASS. Scripts write to disposable mktemp dirs.
set -uo pipefail
cd "$(dirname "$0")/../.."
failed=0
run_legacy() {
  local label="$1" script="$2"
  printf '\n===== LEGACY NON-AUTHORITATIVE: %s =====\n' "$label"
  if /usr/bin/bash "$script"; then
    printf 'LEGACY_TEST=%s result=PASS_ISOLATED_ONLY\n' "$label"
  else
    printf 'LEGACY_TEST=%s result=FAIL_OR_OBSOLETE\n' "$label"
    failed=$((failed+1))
  fi
}
run_legacy "CAREER_INIT_LIFECYCLE" "sorgenti/tests/run_test_career_init_lifecycle.sh"
run_legacy "EXPANDED_PHASE12" "sorgenti/tests/run_test_expanded_phase12.sh"
run_legacy "JOIN_IMPORT_STATIC_OLD" "sorgenti/tests/run_test_join_import_instrumentation.sh"
run_legacy "LWE_OLD_UNIT" "sorgenti/tests/run_test_expanded_world_live.sh"
run_legacy "LWE_OLD_70" "sorgenti/tests/run_test_living_world_70.sh"
run_legacy "LWE_OLD_MOBILITY" "sorgenti/tests/run_test_living_world_mobility.sh"
run_legacy "LWE_OLD_CALENDAR30" "sorgenti/tests/run_test_calendar_integrity_30.sh"
run_legacy "LWE_OLD_REAL_JSON70" "sorgenti/tests/run_test_real_json_world_70.sh"
echo "LEGACY_TEST11_ASAN=NOT_RUN reason=FIXED_TMP_PATH_AND_OPTIONAL_SANITIZER_TOOLCHAIN"
echo "OG_GAMEPLAY_PROOF=NOT_TESTED_BY_LEGACY_SUITES"
if ((failed)); then
  printf 'LEGACY_SUITE=FAIL_OR_OBSOLETE failed=%u (separate from OG)\n' "$failed"
  exit 1
fi
echo "LEGACY_SUITE=PASS_ISOLATED_ONLY"
