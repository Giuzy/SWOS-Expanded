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
static int s_native_mobility_prepared=0;
int ExpandedWorldLive_NativeMobilityPrepared(void) {
    return s_native_mobility_prepared;
}
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
            for (unsigned d=0;d<(unsigned)divisions;++d) {
                (void)ExpandedWorldLive_SetLeagueSchedule(
                    country,(uint8_t)d,
                    (uint8_t)base->beginMonth,(uint8_t)base->endMonth,
                    (uint8_t)original_legs);
                /* Read the ORIGINAL division's automatic promotions, relegations
                 * and playoff flags. Unsupported or grouped systems are logged;
                 * they are never replaced with fabricated rank cutoffs. */
                const Division *rule=&league->divisions[d];
                if(rule->promoted>=0 && rule->relegated>=0 &&
                   rule->promotedPo>=0 && rule->relegatedPo>=0) {
                    (void)ExpandedWorldLive_SetLeagueMovement(
                        country,(uint8_t)d,(uint8_t)rule->promoted,
                        (uint8_t)rule->relegated,(uint8_t)rule->promotedPo,
                        (uint8_t)rule->relegatedPo);
                } else fprintf(stderr,"[living-world] unsupported signed "
                       "promotion rules country=%u division=%u\n",
                       (unsigned)country,d);
            }
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

/* Recover the CURRENT world league roster into the original OG career
 * division-offset tables BEFORE InitializeNewSeason builds the brackets.
 * Every member keeps its original immutable TEAM id/globalTeamNumber.
 * Two-pass validation prevents half-applied promotion state. Native OG
 * competition creation then resolves members with its existing loader,
 * including domestic cup entries and unmodified match callbacks.
 *
 * Scope: ordinary already-imported domestic clubs. National-only pools,
 * country-specific special playoffs and cup brackets outside the user's
 * country are not manufactured here. */
extern uint8_t _diyLeagueSeasonStateTable[];
extern uint8_t _diyLeagueScratchBuffer[];
int ExpandedWorldLive_StageNativeMobility(void) {
    s_native_mobility_prepared=0;
    if (!ExpandedWorldLive_Clubs()) return 0;
    uint8_t source_files[256]={0}, seen[GLOBAL_TEAM_NUMBERS]={0};
    int8_t delta[GLOBAL_TEAM_NUMBERS]={0};
    uint8_t season_division[65536];
    memset(season_division,255,sizeof season_division);
    const uint32_t world_leagues=ExpandedWorldLive_Leagues();
    for(uint32_t l=0;l<world_leagues;++l) {
        uint8_t country=0,division=0,clubs=0,scheduled=0;
        ExpandedLiveRow rows[32];
        uint32_t found=0;
        if(!ExpandedWorldLive_DivisionStatus(l,&country,&division,
                                              &clubs,&scheduled)) return 0;
        if(!scheduled) continue;
        if(!ExpandedWorldLive_Table(country,division,rows,32u,&found) ||
           found!=clubs) {
            fprintf(stderr,"[world-native-roster] no complete season roster "
                           "country=%u div=%u year=%u\n",
                    (unsigned)country,(unsigned)division,
                    (unsigned)ExpandedWorldLive_SeasonYear());
            return 0;
        }
        for(uint32_t j=0;j<found;++j) {
            if(season_division[rows[j].club]!=255u) return 0;
            season_division[rows[j].club]=division;
        }
    }
    uint32_t files=ExpandedWorldLive_ClubFiles(source_files,256u);
    if (!files) return 0;
    const uint16_t prev_skip=_skipLeagueNumbers;
    const uint16_t prev_loaded=_teamsLoaded;
    uint32_t mapped=0,changed=0;
    int valid=1;
    for(uint32_t i=0;i<files && valid;++i) {
        const uint8_t file=source_files[i];
        _skipLeagueNumbers=1;
        _teamsLoaded=0;
        if(LoadTeamFile(file)!=0) {valid=0;break;}
        uint16_t count=*(const uint16_t *)_teamFileBuffer;
        if(count>TEAM_FILE_MAX_TEAMS) {valid=0;break;}
        const TeamFileEx *teams=(const TeamFileEx *)(_teamFileBuffer+2);
        for(uint16_t j=0;j<count;++j) {
            const TeamFileEx *t=&teams[j];
            const uint16_t id=(uint16_t)(((uint16_t)t->team.countryNumber<<8)|
                                          t->team.teamOrdinal);
            const uint8_t division=season_division[id];
            if(division==255u) continue; /* Not a scheduled domestic club. */
            const unsigned global=(unsigned)t->team.globalTeamNumber;
            const int diff=(int)division-(int)(int8_t)t->team.league;
            if(global>=GLOBAL_TEAM_NUMBERS || division>=16u ||
               diff < -127 || diff > 127 ||
               (seen[global] && delta[global]!=diff)) {
                fprintf(stderr,"[world-native-roster] invalid mapping "
                               "team=%04x global=%u initial=%d current=%u\n",
                        (unsigned)id,global,(int)(int8_t)t->team.league,
                        (unsigned)division);
                valid=0;break;
            }
            seen[global]=1u;
            delta[global]=(int8_t)diff;
            ++mapped;
            if(diff) ++changed;
        }
    }
    _skipLeagueNumbers=prev_skip;
    /* LoadTeamFile replaced the shared team buffer; always force the next
     * native consumer to reload its own file (like the world importer). */
    _teamsLoaded=0;
    (void)prev_loaded;
    if(!valid) {
        fprintf(stderr,"[world-native-roster] rejected; original OG state "
                       "preserved, NOT a successful multi-year join\n");
        return 0;
    }
    for(unsigned g=0;g<GLOBAL_TEAM_NUMBERS;++g) {
        if(!seen[g]) continue;
        _diyLeagueSeasonStateTable[g]=(uint8_t)delta[g];
        _diyLeagueScratchBuffer[g]=(uint8_t)delta[g];
    }
    fprintf(stderr,"[world-native-roster] world year=%u native mapped=%lu "
                   "changed division=%lu source files=%lu\n",
            (unsigned)ExpandedWorldLive_SeasonYear(),
            (unsigned long)mapped,(unsigned long)changed,
            (unsigned long)files);
    s_native_mobility_prepared=mapped!=0u;
    return s_native_mobility_prepared;
}

#else
int ExpandedWorldLive_ImportAudit(ExpandedLiveImportAudit *out) {
    if (!out) return 0;
    *out=(ExpandedLiveImportAudit){0};
    return 1;
}
void ExpandedWorldLive_LoadOfferCountries(void) {}
int ExpandedWorldLive_StageNativeMobility(void) {return 0;}
int ExpandedWorldLive_NativeMobilityPrepared(void) {return 0;}
#endif
