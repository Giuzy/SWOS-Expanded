#!/usr/bin/env bash
# Static source wiring only. No gameplay, scores, saves or multi-world switching.
set -euo pipefail
cd "$(dirname "$0")/../.."
src=sorgenti/src/career_status.c
start=sorgenti/src/career_init.c
grep -Fq 'int Expanded_OGWorld_AdvanceUefa(uint16_t year,uint8_t phase)' "$src"
grep -Fq 'World_LeaguePhaseCups(&places,&nAssoc)' "$src"
grep -Fq 'Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec((uint8_t *)rec)' "$src"
grep -Fq 'Career_AutoAdvanceCompetitionSlots();' "$src"
grep -Fq 'ctx->lastPhase=phase;ctx->used=1;' "$src"
grep -Fq 'int exp_og_view_uefa' "$src"
grep -Fq 'Expanded_OGWorld_AdvanceUefa(year,phase)' "$start"
grep -Fq 'copa=NEEDS_FIRST_YEAR_QUALIFIED_ENTRANTS' "$start"
grep -Fq 'ct_world_prepare(void)' sorgenti/src/world_conti.c
grep -Fq 'ct_world_advance_month(u8 month)' sorgenti/src/world_conti.c
echo 'OG_UEFA_TYPE2_SOURCE_WIRING=PASS_SOURCE_ONLY three_cups=3'
echo 'OG_UEFA_FIXTURE_DATES=NOT_VERIFIED_BY_SOURCE'
echo 'OG_COPA_LIBERTADORES_RESIDENT=NOT_CONNECTED'
echo 'OG_CONTINENTAL_SAVE_LOAD=NOT_VERIFIED'
