#!/usr/bin/env bash
# Source-only audit of current OG wiring. No fixture, save or Windows gameplay test.
set -euo pipefail
cd "$(dirname "$0")/../.."
src=sorgenti/src/career_status.c
start=sorgenti/src/career_init.c
grep -Fq 'int Expanded_OGWorld_AdvanceUefa(uint16_t year,uint8_t phase)' "$src"
grep -Fq 'World_LeaguePhaseCups(&places,&nAssoc)' "$src"
grep -Fq 'InitCompetitionTeamSelectionFromAltTableHeader((uint8_t *)rec)' "$src"
grep -Fq 'Career_AutoAdvanceCompetitionSlots();' "$src"
grep -Fq 'ctx->lastPhase=phase;ctx->used=1;' "$src"
grep -Fq 'int exp_og_view_uefa' "$src"
grep -Fq 'Expanded_OGWorld_AdvanceUefa(year,phase)' "$start"
grep -Fq '[og-international-s1]' "$start"
grep -Fq 'ct_world_prepare(void)' sorgenti/src/world_conti.c
grep -Fq 'ct_world_advance_month(u8 month)' sorgenti/src/world_conti.c
echo 'OG_UEFA_TYPE2_SOURCE_WIRING=PASS_SOURCE_ONLY three_cups=3'
echo 'OG_UEFA_FIXTURE_DATES=NOT_VERIFIED_BY_SOURCE'
echo 'OG_COPA_LIBERTADORES_RESIDENT=NOT_CONNECTED'
echo 'OG_CONTINENTAL_SAVE_LOAD=NOT_VERIFIED'
