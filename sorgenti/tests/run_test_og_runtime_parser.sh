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
grep -q '^OG_CUP_NATIVE_BYE_EVENTS=OBSERVED verified=1 rejected=0' "$work/positive.out"
! grep -q '^FIRST_RUNTIME_FAILURE:' "$work/positive.out"
# A complete 70-year log without a FINISHED marker must never pass.
sed '/^\[og-stress\] FINISHED years=70/d' "$work/positive.log" > "$work/negative.log"
echo "[og-world-step] BEGIN year=2096 phase=48 country=28 division=0" >> "$work/negative.log"
set +e
"$work/audit" "$work/negative.log" "$version" > "$work/negative.out"
rc=$?
set -e
[[ "$rc" -eq 1 ]] || { cat "$work/negative.out"; echo "Expected interrupted stress FAIL (1), got $rc"; exit 1; }
grep -q '^OG_70_NATIVE_SEASONS=FAIL ' "$work/negative.out"
grep -q '^OG_LAST_WORLD_STEP=NO_END_RECORDED year=2096 phase=48 country=28 division=0' "$work/negative.out"
echo "OG_RUNTIME_PARSER_STATIC_FIXTURES=PASS"
echo "OG_REAL_WINDOWS_GAMEPLAY=NOT_TESTED_BY_THESE_FIXTURES"
