#!/usr/bin/env bash
# STATIC parser regression fixtures, NOT gameplay / not a second match engine.
# Uses the existing GCC toolchain only. Full OG world test uses SWOS.log.
set -euo pipefail
cd "$(dirname "$0")/../.."
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
gcc -std=c11 -O2 -Wall -Wextra -Werror \
    sorgenti/tests/audit_windows_runtime_log.c -o "$work/audit"
version="$(sed -n 's/^#define SWOS_EXPANDED_VERSION "\([^"]*\)".*/\1/p' sorgenti/include/build_info.h)"
[[ -n "$version" ]]
{
    echo "[build] SWOS Expanded $version"
    echo "[og-career] RESET new-career-start"
    echo "[og-world-audit] stage=TICK year=2026 phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
    echo "[og-world] START READY: date=2026-07-01 native-phase=48 resident-leagues=134"
    echo "[og-world-audit] stage=START year=2026 phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
    echo "[og-world-audit] stage=TICK year=2027 phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
    echo "[og-career] RESET new-career-start"
    echo "[og-world-audit] stage=TICK year=2026 phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
    echo "[og-world] START READY: date=2026-07-01 native-phase=48 resident-leagues=134"
    echo "[og-world-audit] stage=START year=2026 phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
    echo "[og-feature-audit] stage=STRESS_BEFORE year=2026 resident-leagues=134 native-matches=5001 cup-definitions=101 cup-resident=0 scorer-players=2144 scorer-positive=420 scorer-league-goals=910 standings-goals=940 scorer-leagues-with-goals=130"
    echo "[og-external-competition] stage=START category=UEFA_CLUB name=CHAMPIONS_LEAGUE compId=0 defined=1 activity=0 fixture-proof=NOT_VERIFIED"
    echo "[og-external-competition] stage=START category=UEFA_CLUB name=EUROPA_LEAGUE compId=1 defined=1 activity=0 fixture-proof=NOT_VERIFIED"
    echo "[og-external-competition] stage=START category=UEFA_CLUB name=CONFERENCE_LEAGUE compId=2 defined=1 activity=0 fixture-proof=NOT_VERIFIED"
    echo "[og-external-competition] stage=START category=NATIONAL name=WORLD_CUP compId=4 defined=1 activity=0 fixture-proof=NOT_VERIFIED"
    echo "[autosave-title] status=COMPATIBLE_ON_DISK world=2627 file=AUTO2627.CAR scope=SCHEMA_KIND_WORLD_ONLY"
    echo "[og-manager-world-fixture] world=2627 edition=2027 country=8 div=3 home=0857 away=0847 phase=76 serial=163 score=2-1 source=LOCAL_MANAGER_OG historic-pair-other-phase=0 resident-same-phase=0 status=LOCAL_OG_WITNESSED_PENDING_RESIDENT_BRIDGE"
    echo "[og-manager-bridge] COMMITTED world=2627 country=8 division=3 year=2027 phase=76 score=2-1 native-matches=163 remapped=24 journal=OBSERVED source=LOCAL_OG_NO_SECOND_SIM"
    echo "[og-manager-fixture] world=2627 year=2027 div=3 home=0857 away=0847 score=2-1 cache=NOT_FOUND calendar-phase=PLAYED calendar-home=0847 calendar-away=0844 status=CALENDAR_PAIR_MISMATCH no-world-mutation=1"
    echo "[og-stress] ARMED years=70 press FORWARD TIME once"
    echo "[og-stress] START years=70 from=2026-07-01 to=2096-07-01 mode=ANNUAL_COMMIT"
    for n in $(seq 1 70); do
        year=$((2025+n))
        next=$((year+1))
        echo "[og-season-transition] CLOSE year=$year resident-leagues=134 played-leagues=130 native-matches=13000 invalid=0"
        echo "[og-season-transition] OPEN year=$next resident-leagues=134 invalid=0"
        echo "[og-world-audit] stage=TICK year=$next phase=48 resident-leagues=134 played-leagues=39 completed-leagues=4 native-matches=5001 invalid=0 countries=91 club-bound=0 cup-definitions=101 cup-countries=87 cup-resident=0 yearend-callbacks=0"
        echo "[og-stress] STEP number=$n of=70 date=$next-07-01 remaining=$((70-n))"
    done
    echo "[og-feature-audit] stage=STRESS_AFTER year=2096 resident-leagues=134 native-matches=5001 cup-definitions=101 cup-resident=0 scorer-players=2144 scorer-positive=430 scorer-league-goals=940 standings-goals=960 scorer-leagues-with-goals=130"
    echo "[og-stress] FINISHED years=70 date=2096-07-01 actual-date=2096-07-01 exact=1"
} > "$work/positive.log"
# Synthetic parser-only regression: a verified OG bye must count as
# RESIDENT, not as an invented extra team. Missing countries remain
# INCOMPLETE because the official world definition count is 101/87.
cat >> "$work/positive.log" <<'EOF'
[og-qual-next] ROUTE world=2627 year=2027 competition=CHAMPIONS status=FAIL_NEW_EDITION_MISSING expected=36 actual=0 missing=36
[og-qual-next] SUMMARY world=2627 year=2027 status=FAIL checked=1 passed=0 failed=1 not-tested=3
[og-qual-next] ROUTE world=9697 year=1997 competition=CHAMPIONS status=PASS_EXACT_IDS expected=16 actual=16 matched=16 missing=0
[og-qual-next] SUMMARY world=9697 year=1997 status=PARTIAL checked=1 passed=1 failed=0 not-tested=3
[og-cup-entry] world=2627 year=2026 country=0 slot=2 compId=109 source=INITIAL_NATIVE_SEED expected=10 actual=10 matched=10 missing=0 extra=0 dup-source=0 dup-resident=0 status=PASS_EXACT_NATIVE_CLUB_IDS manual-override=NONE provenance=NATIVE_FIRST_YEAR_LIST
[og-cup-entry] world=2627 year=2027 country=0 slot=2 compId=109 source=NATIVE_NEXT_YEAR_ACCESS expected=0 actual=10 matched=0 missing=0 extra=10 dup-source=0 dup-resident=0 status=FAIL_OG_ACCESS_NOT_DERIVED manual-override=NONE provenance=ACCESS_UNAVAILABLE
[og-manual-season] world=2627 year=2027 native=1 clock=1 bound=1 status=SYNCED native-ms=30.000 clock-ms=1.000 bind-ms=0.500 save=PERMITTED
[og-forward-perf] world=2627 year=2027 phase=48 leagues-ms=10.000 cups-ms=9.000 international-ms=3.000 overhead-ms=8.000 total-ms=30.000 league-count=134 cup-count=101 faults=0
[og-forward-perf] world=9697 year=1997 phase=48 leagues-ms=4.000 cups-ms=3.000 international-ms=1.000 overhead-ms=2.000 total-ms=10.000 league-count=88 cup-count=51 faults=0
[og-unemployed-qual] world=2627 closed=2026 next=2027 source=OG_RESIDENT_FINAL_STANDINGS_AND_WINNERS countries-read=91 absent-country-finals=0 cup-winners=87 clubs=36,36,36 copa=0 status=NATIVE_FIELD_READY no-replay=1 no-manager-TMD=1
[og-unemployed-qualification] world=2627 closed=2026 opened=2027 status=OG_FIELD_CAPTURED no-replay=1
[og-unemployed-qual] world=9697 closed=1996 next=1997 source=OG_RESIDENT_FINAL_STANDINGS_AND_WINNERS countries-read=86 absent-country-finals=2 cup-winners=51 clubs=16,32,32 copa=20 status=NATIVE_FIELD_READY no-replay=1 no-manager-TMD=1
[og-unemployed-qualification] world=9697 closed=1996 opened=1997 status=OG_FIELD_CAPTURED no-replay=1
[og-qual-car] SAVE world=2627 year=2027 counts=36,36,36,0 status=CHECKSUMMED
[og-qual-car] PRECHECK world=2627 year=2027 source-checksum=VALID resident-fields-compared=3 status=VERIFIED_SAME_YEAR_IDS
[og-qual-car] RESTORE world=2627 year=2027 valid=1 counts=36,36,36,0 status=CHECKSUM_VERIFIED
[og-cup-bye] NATIVE DRAW VERIFIED country=0 compId=109 real=10 bracket=16 round1-ties=2 byes=6 (OG ONLY)
[og-cup-coverage] country=0 slot=2 compId=109 year=2096 expected=16 selected=10 state=RESIDENT reason=NATIVE_BYES_VERIFIED
[og-cup-rule] country=0 slot=2 compId=109 year=2096 bracket=16 selected=10 real-entrants=10 first-round-ties=2 byes=6 padded-slots=6 native-bracket=16 native-round1-ties=2 native-round1-pairs=8 classification=OG_BYE_FORMAT_CANDIDATE round-structure=OG_ROUND1_MATCHES_RULE native-validation=NATIVE_DRAW_IDENTITIES_VERIFIED fixtures=NOT_VERIFIED_BY_RULE
[og-cup-coverage] country=1 slot=2 compId=110 year=2096 expected=16 selected=12 state=UNSUPPORTED reason=SELECTOR_UNDERFILLED
[og-cup-rule] country=1 slot=2 compId=110 year=2096 bracket=16 selected=12 real-entrants=12 first-round-ties=4 byes=4 padded-slots=4 native-bracket=16 native-round1-ties=4 native-round1-pairs=8 classification=OG_BYE_FORMAT_CANDIDATE round-structure=OG_ROUND1_MATCHES_RULE native-validation=UNVERIFIED fixtures=NOT_VERIFIED_BY_RULE
EOF
set +e
"$work/audit" "$work/positive.log" "$version" > "$work/positive.out"
rc=$?
set -e
[[ "$rc" -eq 2 ]] || { cat "$work/positive.out"; echo "Expected incomplete full world (2), got $rc"; exit 1; }
grep -q '^OG_70_NATIVE_SEASONS=PASS_TRANSITIONS_ONLY ' "$work/positive.out"
grep -q '^OG_TITLE_AUTOSAVE=ON_DISK_PREFLIGHT_OBSERVED verified=1 failures=0 ' "$work/positive.out"
grep -q '^OG_EXTERNAL_COMPETITION name=CHAMPIONS_LEAGUE result=DEFINITION_ONLY_NO_RESIDENT_FIXTURE_EVIDENCE defined=1 native-activity=0 ' "$work/positive.out"
grep -q '^OG_EXTERNAL_COMPETITION name=WORLD_CUP result=DEFINITION_ONLY_NO_RESIDENT_FIXTURE_EVIDENCE defined=1 native-activity=0 ' "$work/positive.out"
grep -q '^OG_EXTERNAL_COMPETITION name=COPA_LIBERTADORES result=NOT_TESTED ' "$work/positive.out"
grep -q '^OG_CUPS_BEFORE_AFTER=NOT_IMPLEMENTED ' "$work/positive.out"
grep -q '^OG_SCORERS_BEFORE_AFTER=NATIVE_COUNTERS_OBSERVED ' "$work/positive.out"
grep -q '^OG_CUP_COVERAGE=PARTIAL_WITH_EXPLICIT_REASONS .*native-byes-verified=1 divergent=1' "$work/positive.out"
grep -q '^OG_CUP_COUNTRY year=2096 country=0 definitions=1 resident=1 native-byes=1 unavailable=0 state=ALL_RESIDENT' "$work/positive.out"
grep -q '^OG_CUP_COUNTRY year=2096 country=1 definitions=1 resident=0 native-byes=0 unavailable=1 state=INCOMPLETE' "$work/positive.out"
grep -q '^OG_CUP_COUNTRIES=PARTIAL expected=87 audited=2 incomplete=1' "$work/positive.out"
grep -q '^OG_MANAGER_LOCAL_OG_BRIDGE=SNAPSHOTS_COMMITTED witnesses=1 resident-commits=1 blocked=0 earlier-cycle-pairs=0 same-phase-collisions=0 ' "$work/positive.out"
grep -q '^OG_MANAGER_CALENDAR_PAIR_IDENTITY=FAIL_REAL_PAIR_MISMATCH audits=1 native-calendar-mismatches=1 ' "$work/positive.out"
grep -q '^OG_NEXT_SEASON_QUALIFICATION=FAIL_REAL_HANDOFF rollovers=2 checked=2 passed=1 failed=1 missing-editions=1 wrong-clubs=0 not-tested=6 world9697=1 world2627=1 ' "$work/positive.out"
grep -q '^OG_FIRST_EDITION_CUP_SEED=PARTIAL_OG_ONLY exact=1 wrong=0 unsupported=0 ' "$work/positive.out"
grep -q '^OG_NEXT_EDITION_DOMESTIC_CUP_IDS=FAIL exact=0 wrong=1 no-access-rule=0 ' "$work/positive.out"
grep -q '^OG_MANUAL_SEASON_SYNC=OBSERVED synced=1 blocked=0 ' "$work/positive.out"
grep -q '^OG_UNEMPLOYED_QUALIFICATION=BOTH_WORLDS_CAPTURED world9697-fields=1 world2627-fields=1 incomplete=0 ' "$work/positive.out"
grep -q '^OG_UNEMPLOYED_QUALIFICATION_CAR=SAVED_AND_RESTORED_CHECKSUM_VERIFIED checksummed-saves=1 checksum-verified-restores=1 legacy-no-source=0 ' "$work/positive.out"
grep -q '^OG_FORWARD_PERFORMANCE=MEASURED_NATIVE_COMPONENTS samples=2 modern-samples=1 modern-total-ms=30.000 ' "$work/positive.out"
grep -q '^OG_CUP_NATIVE_BYE_EVENTS=OBSERVED verified=1 rejected=0' "$work/positive.out"
! grep -q '^FIRST_RUNTIME_FAILURE:' "$work/positive.out"
# A complete 70-year log without a FINISHED marker must never pass.
sed '/^\[og-stress\] FINISHED years=70/d' "$work/positive.log" > "$work/negative.log"
echo "[og-world-step] BEGIN year=2096 phase=48 country=28 division=0" >> "$work/negative.log"
echo "[og-manual-season] world=2627 year=2028 native=0 clock=0 bound=0 status=BLOCKED save=PREVIOUS_CAR_PRESERVED" >> "$work/negative.log"
echo "[og-unemployed-qualification] world=2627 closed=2027 opened=2028 status=FIELD_INCOMPLETE no-replay=1" >> "$work/negative.log"
set +e
"$work/audit" "$work/negative.log" "$version" > "$work/negative.out"
rc=$?
set -e
[[ "$rc" -eq 1 ]] || { cat "$work/negative.out"; echo "Expected interrupted stress FAIL (1), got $rc"; exit 1; }
grep -q '^OG_70_NATIVE_SEASONS=FAIL ' "$work/negative.out"
grep -q '^OG_MANUAL_SEASON_SYNC=FAIL synced=1 blocked=1 ' "$work/negative.out"
grep -q '^OG_UNEMPLOYED_QUALIFICATION=INCOMPLETE world9697-fields=1 world2627-fields=1 incomplete=1 ' "$work/negative.out"
grep -q '^OG_LAST_WORLD_STEP=NO_END_RECORDED year=2096 phase=48 country=28 division=0' "$work/negative.out"
# CONTINUE-only CE process: no native START is expected. Never manufacture a PASS.
{
    echo "[build] SWOS Expanded $version"
    echo "[competition-coordinator] LOAD_READY world=2627 owner=UNIVERSAL_JSON civil=EXPANDED no-replay=1"
    echo "[ce-engine] DEFINITIONS world=2627 leagues=134 json=5"
    echo "[ce-engine] RESTORED editions=151 source=UL01 changed=0"
    echo "[universal-manager-menu] FORWARD_OK target=2026-10-18"
} > "$work/ce-load.log"
set +e
"$work/audit" "$work/ce-load.log" "$version" > "$work/ce-load.out"
rc=$?
set -e
[[ "$rc" -eq 2 ]]
grep -q '^CE_RUNTIME starts=0 loads=1 definitions_logs=1 .*restored=1 manager_forward_ok=1 ' "$work/ce-load.out"
grep -q '^WINDOWS_OG_INTEGRATION=NOT_APPLICABLE reason=CE_WORLD' "$work/ce-load.out"
grep -q '^OVERALL=INCOMPLETE ' "$work/ce-load.out"
echo '[universal-live] CPU_FAULT event=1' >> "$work/ce-load.log"
set +e
"$work/audit" "$work/ce-load.log" "$version" > "$work/ce-fault.out"
rc=$?
set -e
[[ "$rc" -eq 1 ]]
grep -q '^CE_VERDICT=FAIL' "$work/ce-fault.out"
grep -q '^OVERALL=FAIL' "$work/ce-fault.out"
echo "CE_CONTINUE_ONLY_RUNTIME_PARSER=PASS"
echo "OG_RUNTIME_PARSER_STATIC_FIXTURES=PASS"
echo "OG_REAL_WINDOWS_GAMEPLAY=NOT_TESTED_BY_THESE_FIXTURES"
