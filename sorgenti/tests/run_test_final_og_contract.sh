#!/usr/bin/env bash
# Static contract verification ONLY; cannot pass entire OG gameplay or save.
set -euo pipefail
cd "$(dirname "$0")/../.."
grep -q 'void Expanded_OGWorld_LogCheckpoint(const char \*stage)' sorgenti/src/career_init.c
grep -q 'expanded_og_external_inventory(stage);' sorgenti/src/career_init.c
grep -q '\[og-cup-rule\]' sorgenti/src/career_init.c
grep -q 'void Expanded_OGWorld_CupSetupAudit' sorgenti/src/career_status.c
grep -q 'Expanded_OGWorld_LogCheckpoint("SAVE_AFTER")' sorgenti/src/menus.c
grep -q 'Expanded_OGWorld_LogCheckpoint("RESTORE_AFTER")' sorgenti/src/menus.c
grep -q 'AUTO9697.CAR' sorgenti/src/menus.c
grep -q 'AUTO2627.CAR' sorgenti/src/menus.c
grep -q 'WorldCups_CupMade' sorgenti/src/world_cups.c
grep -q 'ExpandedOGCup_NativeByeLayout' sorgenti/src/career_status.c
grep -q 'NATIVE_BYES_VERIFIED' sorgenti/src/career_status.c
grep -q 'NATIVE DRAW VERIFIED' sorgenti/src/career_status.c
grep -q '\[og-cup-country\]' sorgenti/src/career_init.c
grep -q 'OG_CUP_COUNTRIES=' sorgenti/tests/audit_windows_runtime_log.c
grep -q 'OG_CUP_START_COVERAGE=' sorgenti/tests/audit_windows_runtime_log.c
grep -q 'WorldCups_Padded' sorgenti/src/world_cups.c
grep -q 'OG_RESIDENT_SAVE_RELOAD=' sorgenti/tests/audit_windows_runtime_log.c
grep -q 'OG_CUP_FORMAT_RULES=' sorgenti/tests/audit_windows_runtime_log.c
grep -q 'OG_INTERNATIONAL_NATIONAL=' sorgenti/tests/audit_windows_runtime_log.c
echo 'OG_FINAL_WIRING_STATIC=PASS_ISOLATED_ONLY'
echo 'OG_CUP_BYES_NATIVE_RULE=REFERENCE_READ_ONLY'
echo 'OG_WORLD_SAVE_70_YEARS=NOT_PROVEN_BY_STATIC_CHECK'
echo 'OG_INTERNATIONAL_NATIONAL_FIXTURES=NOT_PROVEN_BY_STATIC_CHECK'
