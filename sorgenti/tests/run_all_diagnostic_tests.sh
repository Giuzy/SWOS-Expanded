#!/usr/bin/env bash
# Current OG contracts only. Obsolete isolated LWE/world replay tests remain
# in Git for provenance but are deliberately NOT EXECUTED by this runner.
# Not an engine, not a replacement for real Windows gameplay.
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
current_total=0; current_failed=0; excluded=0; unknown=0
run_one() {
    local script="$1" name rc logfile
    name="${script##*/}"; name="${name%.sh}"
    logfile="$out/${name}.log"
    local scope="OG_CURRENT"
    case "$name" in
        run_test_expanded_event_scheduler|run_test_expanded_competition_history|\
        run_test_expanded_og_cache_match|run_test_expanded_og_league_bridge|\
        run_test_expanded_qualification_transition|\
        run_test_expanded_universal_fixture_registry|\
        run_test_expanded_universal_world_catalog|\
        run_test_expanded_universal_car)
            scope="COMPETITION_ENGINE_ISOLATED" ;;
        run_test_expanded_f12_compare)
            scope="F12_READ_ONLY_ISOLATED" ;;
    esac
    printf 'TEST=%s scope=%s action=RUN log=%s\n' "$name" "$scope" "$logfile" >> "$out/ELENCO_TEST.txt"
    if /usr/bin/bash "$script" > "$logfile" 2>&1; then
        rc=0
        printf 'TEST=%s scope=%s status=PASS_ISOLATED_ONLY log=%s\n' "$name" "$scope" "$logfile" >> "$out/RIEPILOGO_TEST.txt"
        printf '[tests-og] PASS %s\n' "$name"
    else
        rc=$?
        printf 'TEST=%s scope=%s status=FAIL exit=%d log=%s\n' "$name" "$scope" "$rc" "$logfile" >> "$out/RIEPILOGO_TEST.txt"
        printf '[tests-og] FAIL %s exit=%d\n' "$name" "$rc"
        current_failed=$((current_failed+1))
    fi
    current_total=$((current_total+1))
}
exclude_one() {
    local name="${1##*/}" reason="$2"
    excluded=$((excluded+1))
    printf 'TEST=%s scope=HISTORICAL action=EXCLUDED reason=%s\n' "$name" "$reason" >> "$out/ELENCO_TEST.txt"
    printf 'TEST=%s scope=HISTORICAL status=NOT_RUN_OBSOLETE reason=%s\n' "$name" "$reason" >> "$out/RIEPILOGO_TEST.txt"
}
# Review a newly added runner explicitly before it becomes an authority:
# unclassified scripts do not silently run or silently pass.
shopt -s nullglob
for script in sorgenti/tests/run_test*.sh sorgenti/tests/run_audit_competition_formats.sh; do
    case "${script##*/}" in
        run_test_database_inventory.sh|run_test_expanded_debug_only.sh|\
        run_test_expanded_midseason_join.sh|run_test_expanded_world_time.sh|\
        run_test_native_history_occurrence.sh|run_test_og_runtime_parser.sh|\
        run_test_og_cup_native_byes.sh|run_test_expanded_phase12.sh|\
        run_audit_competition_formats.sh|run_test_final_og_contract.sh|\
        run_test_mod2627_coverage.sh|run_test_og_uefa_wiring.sh|\
        run_test_og_offer_rollback.sh|run_test_expanded_event_scheduler.sh|\
        run_test_expanded_competition_history.sh|run_test_expanded_og_cache_match.sh|\
        run_test_expanded_og_league_bridge.sh|\
        run_test_expanded_qualification_transition.sh|\
        run_test_expanded_universal_fixture_registry.sh|\
        run_test_expanded_universal_world_catalog.sh|\
        run_test_expanded_universal_car.sh|\
        run_test_expanded_f12_compare.sh|run_test_save_car_zstd.sh|\
        run_test_expanded_og_fixture_hash.sh)
            run_one "$script" ;;
        run_test_engine_suite.sh|run_test_legacy_suite.sh)
            exclude_one "$script" "DUPLICATE_AGGREGATE" ;;
        run_test_expanded_world_live.sh|run_test_living_world_70.sh|\
        run_test_living_world_mobility.sh|run_test_real_json_world_70.sh|\
        run_test_calendar_integrity_30.sh)
            exclude_one "$script" "OBSOLETE_LWE_SIMULATION" ;;
        run_test_career_init_lifecycle.sh|run_test_join_import_instrumentation.sh)
            exclude_one "$script" "OBSOLETE_OG_LWE_WIRING_ASSUMPTION" ;;
        run_test11_offer.sh)
            exclude_one "$script" "OLD_ISOLATED_OFFER_HARNESS_NOT_NATIVE_RESIDENT" ;;
        *)
            unknown=$((unknown+1))
            printf 'TEST=%s status=NOT_RUN_UNCLASSIFIED reason=REVIEW_REQUIRED\n' "$script" >> "$out/RIEPILOGO_TEST.txt"
            printf 'TEST=%s scope=UNKNOWN action=NOT_RUN\n' "$script" >> "$out/ELENCO_TEST.txt" ;;
    esac
done
printf 'TEST=TEST_LIVING_WORLD.bat status=ALIAS_TEST_OG_WORLD.bat\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'TEST=audit_windows_runtime_log.sh status=ALIAS_ANALIZZA_SWOS_LOG.bat\n' >> "$out/RIEPILOGO_TEST.txt"
printf 'OG_CURRENT_TESTS=%s tested=%d failed=%d\n' \
    "$([[ $current_failed -eq 0 && $current_total -gt 0 ]] && echo PASS || echo FAIL)" \
    "$current_total" "$current_failed" | tee -a "$out/RIEPILOGO_TEST.txt"
printf 'OBSOLETE_EXCLUDED=%d new-unclassified=%d\n' "$excluded" "$unknown" | tee -a "$out/RIEPILOGO_TEST.txt"
echo 'OG_GAMEPLAY=NOT_TESTED_BY_ISOLATED_TESTS' | tee -a "$out/RIEPILOGO_TEST.txt"
if ((current_failed || unknown || current_total==0)); then exit 1; fi
exit 0
