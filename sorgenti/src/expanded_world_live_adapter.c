/* Initialize the ONE world on NEW CAREER -> START for both employed and
 * unemployed starts, BEFORE either team picker or unemployment modal.
 * Loading team files inside a modal callback would overwrite menu/team state.
 * Same five validated data files already used by club offer generation.
 * Never call this again on an accepted contract or team change.
 */
#include "expanded_world_live.h"
#include "world.h"
#include "game_types.h"
#include "teams.h"
#include "comp_identity.h"
#ifdef SWOS_SDL_BUILD
#include <stdio.h>
#include <string.h>
extern uint16_t _teamsLoaded, _skipLeagueNumbers;
extern uint8_t _teamFileBuffer[];
extern void *_competitionsTable[];
static ExpandedLiveImportAudit s_import_audit;
int ExpandedWorldLive_ImportAudit(ExpandedLiveImportAudit *out) {
    if (!out) return 0;
    *out=s_import_audit;
    return 1;
}

static unsigned import_file(uint8_t file, uint8_t country,
                            unsigned divisions, unsigned points) {
    unsigned imported=0;
    _teamsLoaded=0;
    if (LoadTeamFile(file)!=0) return 0;
    unsigned count=*(const uint16_t*)_teamFileBuffer;
    if (count>TEAM_FILE_MAX_TEAMS) return 0;
    const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
    for(unsigned i=0;i<count;++i) {
        const TeamFileEx *t=&teams[i];
        if (t->team.countryNumber != file || t->team.league>=divisions ||
            t->team.league>=16) continue;
        unsigned total=0,valid=0;
        for(unsigned j=0;j<16;++j) {
            unsigned price=(uint8_t)t->players[j].price;
            if (price<=49u) { total+=price; valid++; }
        }
        unsigned rating=100u+(valid ? (total*8u)/valid : 0u);
        uint16_t id=(uint16_t)(((uint16_t)file<<8)|t->team.teamOrdinal);
        char name[18];
        memcpy(name,t->team.teamName,17); name[17]=0;
        if (ExpandedWorldLive_AddClub(country,t->team.league,id,
                                      (uint16_t)rating,name,(uint8_t)points))
            imported++;
    }
    return imported;
}
/* The world owns all installed domestic leagues, not the five countries
 * from the original prototype. Both OG and 26/27 use their existing
 * career-country list; only valid native league records are imported.
 * The manager's nationality is NOT an input to this function. */
extern uint8_t _careerCompetitionIdRemapTable[];
void ExpandedWorldLive_LoadOfferCountries(void) {
    memset(&s_import_audit,0,sizeof s_import_audit);
    const uint8_t *countries=World_SeasonCountries();
    if (!countries) countries=_careerCompetitionIdRemapTable;
    if (!ExpandedWorldLive_Begin(World_FirstYear())) {
        fprintf(stderr,"[living-world] failed to reserve memory for pilot\n");
        return;
    }
    uint16_t old_skip=_skipLeagueNumbers;
    _skipLeagueNumbers=1; /* raw divisions, not prior-season promotion offsets */
    unsigned imported_countries=0;
    for(unsigned k=0;k<256u && countries[k]!=0xFFu;++k) {
        uint8_t country=countries[k];
        ++s_import_audit.candidate_countries;
        const void *const *table=(const void *const *)_competitionsTable[country];
        if (!table || !table[0] || (uintptr_t)table[0]==(uintptr_t)-1 ||
            (uintptr_t)table[0]==(uintptr_t)-2) {
            ++s_import_audit.skipped_country_records;
            continue;
        }
        const Competition *base=(const Competition *)table[0];
        if (base->type!=0 || comp_is_type0_cup(base->compId)) {
            ++s_import_audit.skipped_country_records;
            continue;
        }
        const CompetitionLeague *league=(const CompetitionLeague *)base;
        int divisions=league->header.divisionsCount;
        if (divisions<1 || divisions>16) {
            ++s_import_audit.skipped_country_records;
            continue;
        }
        ++s_import_audit.supported_countries;
        int points=league->header.pointsForWin;
        if (points!=2 && points!=3) points=3;
        ++s_import_audit.team_files_attempted;
        unsigned clubs=import_file(country,country,(unsigned)divisions,(unsigned)points);
        if (clubs) {
            ++imported_countries;
            ++s_import_audit.team_files_with_clubs;
            s_import_audit.imported_clubs+=clubs;
        } else ++s_import_audit.team_files_without_clubs;
#if SWOS_WORLDS
        uint8_t second=World_SecondFile(country);
        if (second && second!=country) {
            ++s_import_audit.team_files_attempted;
            unsigned extra=import_file(second,country,(unsigned)divisions,(unsigned)points);
            if (extra) {
                ++s_import_audit.team_files_with_clubs;
                s_import_audit.imported_clubs+=extra;
            } else ++s_import_audit.team_files_without_clubs;
        }
#endif
        /* Import the official competition dates into each world division.
         * This is the same header native InitializeNewSeason reads; no
         * second independent Aug-to-May calendar is invented. */
        /* OpenSWOS's playEachTeam/rounds field decides whether the
         * source league has one, two or more meeting cycles. Do not
         * unconditionally assume every nation plays home and away. */
        int original_legs=(int)league->header.rounds;
        if (original_legs<0) original_legs=1;
        if (original_legs>=1 && original_legs<=4) {
            for (unsigned d=0;d<(unsigned)divisions;++d)
                (void)ExpandedWorldLive_SetLeagueSchedule(
                    country,(uint8_t)d,
                    (uint8_t)base->beginMonth,(uint8_t)base->endMonth,
                    (uint8_t)original_legs);
        } else {
            ++s_import_audit.unsupported_rounds;
            fprintf(stderr,"[living-world] unsupported native rounds=%d "
                           "for country %u\n",original_legs,(unsigned)country);
        }
    }
    _skipLeagueNumbers=old_skip;
    _teamsLoaded=0;
    if (!ExpandedWorldLive_Seal()) {
        fprintf(stderr,"[living-world] no schedulable league, pilot disabled\n");
        ExpandedWorldLive_End();
    } else {
        s_import_audit.scheduled_leagues=ExpandedWorldLive_NativeScheduleLeagues();
        fprintf(stderr,"[living-world] REAL TEAM-file coverage: countries=%lu/%lu "
            "skipped=%lu rounds-unsupported=%lu files-with-clubs=%lu/%lu "
            "imported-clubs=%lu scheduled-leagues=%lu\n",
            (unsigned long)s_import_audit.supported_countries,
            (unsigned long)s_import_audit.candidate_countries,
            (unsigned long)s_import_audit.skipped_country_records,
            (unsigned long)s_import_audit.unsupported_rounds,
            (unsigned long)s_import_audit.team_files_with_clubs,
            (unsigned long)s_import_audit.team_files_attempted,
            (unsigned long)s_import_audit.imported_clubs,
            (unsigned long)s_import_audit.scheduled_leagues);
        fprintf(stderr,
            "[living-world] pilot ready: %u clubs, %u groups; start=%u, "
            "native SWOS league schedule groups=%lu; imported countries=%u\n",
            (unsigned)ExpandedWorldLive_Clubs(),
            (unsigned)ExpandedWorldLive_Leagues(),(unsigned)World_FirstYear(),
            (unsigned long)ExpandedWorldLive_NativeScheduleLeagues(),
            imported_countries);
    }
}
#else
int ExpandedWorldLive_ImportAudit(ExpandedLiveImportAudit *out) {
    if (!out) return 0;
    *out=(ExpandedLiveImportAudit){0};
    return 1;
}
void ExpandedWorldLive_LoadOfferCountries(void) {}
#endif
