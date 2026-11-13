#!/usr/bin/env bash
# Static wiring only: native play-through and cup persistence are NOT proven.
set -euo pipefail
cd "$(dirname "$0")/../.."
grep -q 'void Career_BeginNewCareer(void)' sorgenti/src/career_init.c
grep -q 'void InitCareerWorld(void)' sorgenti/src/career_init.c
grep -q 'void InitCareerManager(void)' sorgenti/src/career_init.c
grep -q 'void InitCareerClub(uint16_t teamNo)' sorgenti/src/career_init.c
grep -q 'void Career_AttachManagerToClub(uint16_t teamNo)' sorgenti/src/career_init.c
grep -q 'Career_AttachManagerToClub(newTeam);' sorgenti/src/expanded_career.c
grep -q 'Career_AdvanceUnemployedOGCups((uint32_t)s_waitWeeks);' sorgenti/src/expanded_career.c
grep -q 'OG_DatabaseLoadedTeams' sorgenti/src/platform_sdl.c
if grep -q 'InitCareer(newTeam);' sorgenti/src/expanded_career.c; then
  echo 'CAREER_LIFECYCLE_STATIC=FAIL'
  exit 1
fi
echo 'CAREER_LIFECYCLE_STATIC=PASS'
echo 'F12_DATABASE_COUNTERS_STATIC=PASS'
echo 'OG_UNEMPLOYED_CUP_HOOK_STATIC=PASS'
echo 'OG_UNEMPLOYED_CUP_RUNTIME=NOT_TESTED'
