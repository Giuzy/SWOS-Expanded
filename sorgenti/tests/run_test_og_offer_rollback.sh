#!/usr/bin/env bash
# Old fake international text gallery must never precede native OG picker.
set -euo pipefail
cd "$(dirname "$0")/../.."
src=sorgenti/src/expanded_career.c
! grep -Fq 's_previewMode==3' "$src"
! grep -Fq 's_previewMode=3;' "$src"
! grep -Fq 'DEFINITIONS ONLY - OG RESULTS NOT READY' "$src"
grep -Fq 'Expanded_OfferChooseCompetitionMenu(country)' "$src"
grep -Fq 'Expanded_OGWorld_ViewLeague(selection,division)' "$src"
grep -Fq 'return membership>=0;' "$src"
echo 'OG_OFFER_ROLLBACK=PASS_SOURCE_ONLY'
echo 'OG_OFFER_UI_WINDOWS=NOT_TESTED'
