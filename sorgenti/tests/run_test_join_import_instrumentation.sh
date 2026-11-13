#!/usr/bin/env bash
# These are static link/wiring checks, not native match or cup runtime PASS.
set -euo pipefail
cd "$(dirname "$0")/../.."
grep -q 'void DIY_WorldAuditStandingsAtMenu(void)' sorgenti/src/diy_competition.c
grep -q 'DIY_WorldAuditStandingsAtMenu();' sorgenti/src/diy_competition.c
grep -q 'ExpandedWorldLive_Table(country,division,world,32u,&found)' sorgenti/src/diy_competition.c
grep -q 'OG/LWE TABLE COMPARISON' sorgenti/src/platform_sdl.c
grep -q 's_world_debug_import_page' sorgenti/src/platform_sdl.c
grep -q 'ExpandedWorldLive_ImportSamples(sample,20u)' sorgenti/src/platform_sdl.c
grep -q 'track_rejection(t,file,LIVE_IMPORT_OUTSIDE)' sorgenti/src/expanded_world_live_adapter.c
grep -q 'track_rejection(t,file,LIVE_IMPORT_UNSUPPORTED_DIVISION)' sorgenti/src/expanded_world_live_adapter.c
grep -q 'OG CUP REGISTERED != OG CUP SIMULATED' sorgenti/src/platform_sdl.c
grep -q 'new witnessed OG matches' sorgenti/src/career_init.c
echo 'JOIN_STANDINGS_AUDIT_STATIC=PASS'
echo 'F10_IMPORT_DIAGNOSTICS_STATIC=PASS'
echo 'OG_CUP_AUDIT_STATIC=PASS'
echo 'JOIN_STANDINGS_AUDIT_WINDOWS_RUNTIME=NOT_TESTED'
echo 'F10_IMPORT_DIAGNOSTICS_WINDOWS_RUNTIME=NOT_TESTED'
echo 'OG_CUP_UNEMPLOYED_RUNTIME=NOT_TESTED'
