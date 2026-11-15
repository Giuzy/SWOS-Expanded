#!/usr/bin/env bash
# Run every standalone source test, including old LWE regression experiments.
# One OG runtime engine remains authoritative. This is NOT gameplay.
set -uo pipefail
cd "$(dirname "$0")/../.."
if [[ -n "${SWOS_DIAGNOSTIC_DIR:-}" ]]; then
    out="$(cygpath -u "$SWOS_DIAGNOSTIC_DIR")/TEST_ESEGUITI"
else
    out="RISULTATI_DIAGNOSTICI/TEST_ESEGUITI_STANDALONE"
fi
mkdir -p "$out"
: > "$out/ELENCO_TEST.txt"
: > "$out/RIEPILOGO_TEST.txt"
current_total=0; current_failed=0
legacy_total=0; legacy_failed=0
run_one() {
    local script="$1" scope="$2" name rc logfile
    name="${script##*/}"
    name="${name%.sh}"
    logfile="$out/${name}.log"
    printf 'TEST=%s scope=%s result=PENDING\n' "$name" "$scope" >> "$out/ELENCO_TEST.txt"
    printf '\n[diagnostic-tests] START %s [%s]\n' "$name" "$scope"
    if /usr/bin/bash "$script" > "$logfile" 2>&1; then
        rc=0
        printf 'TEST=%s scope=%s status=PASS_ISOLATED_ONLY exit=%d log=%s\n' \
            "$name" "$scope" "$rc" "$logfile" >> "$out/RIEPILOGO_TEST.txt"
        printf '[diagnostic-tests] PASS %s\n' "$name"
    else
        rc=$?
        printf 'TEST=%s scope=%s status=FAIL_OR_OBSOLETE exit=%d log=%s\n' \
            "$name" "$scope" "$rc" "$logfile" >> "$out/RIEPILOGO_TEST.txt"
        printf '[diagnostic-tests] FAIL %s exit=%d\n' "$name" "$rc"
    fi
    if [[ "$scope" == "OG_SOURCE_CHECK" ]]; then
        current_total=$((current_total+1))
        if ((rc)); then current_failed=$((current_failed+1)); fi
    else
        legacy_total=$((legacy_total+1))
        if ((rc)); then legacy_failed=$((legacy_failed+1)); fi
    fi
}
# Dynamically find new/old tests without running aggregate wrappers twice.
shopt -s nullglob
for script in sorgenti/tests/run_test*.sh sorgenti/tests/run_audit_competition_formats.sh; do
    case "${script##*/}" in
       run_test_engine_suite.sh|run_test_legacy_suite.sh) continue ;;
       run_test_expanded_debug_only.sh|run_test_og_runtime_parser.sh|\
       run_test_expanded_world_time.sh|run_test_native_history_occurrence.sh|\
       run_test_expanded_midseason_join.sh|run_test_database_inventory.sh|\
       run_audit_competition_formats.sh)
           run_one "$script" "OG_SOURCE_CHECK" ;;
       *)
           run_one "$script" "LEGACY_ISOLATED" ;;
    esac
done
# Compatibility wrappers for the SAME audit/tests, not duplicate runs.
printf 'TEST=audit_windows_runtime_log.sh status=ALIAS_ANALIZZA_SWOS_LOG.bat\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'TEST=TEST_LIVING_WORLD.bat status=ALIAS_TEST_OG_WORLD.bat\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'TEST=run_test_engine_suite.sh status=WRAPPER_SCRIPTS_ENUMERATED\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'TEST=run_test_legacy_suite.sh status=WRAPPER_SCRIPTS_ENUMERATED\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'TEST=F12+8_70_NATIVE_SEASONS status=REQUIRES_REAL_WINDOWS_GAMEPLAY\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'OG_SOURCE_CHECKS=%s tested=%d failed=%d\n' \
    "$([[ $current_failed -eq 0 ]] && echo PASS || echo FAIL)" \
    "$current_total" "$current_failed" | tee -a "$out/RIEPILOGO_TEST.txt"
printf 'HISTORICAL_ISOLATED_TESTS=%s tested=%d failed=%d\n' \
    "$([[ $legacy_failed -eq 0 ]] && echo PASS_ISOLATED_ONLY || echo FAIL_OR_OBSOLETE)" \
    "$legacy_total" "$legacy_failed" | tee -a "$out/RIEPILOGO_TEST.txt"
echo "OG_GAMEPLAY=NOT_TESTED_BY_ISOLATED_TESTS" | tee -a "$out/RIEPILOGO_TEST.txt"
if ((current_failed)); then exit 1; fi
# Historical failures remain visible without claiming OG gameplay failure.
exit 0
