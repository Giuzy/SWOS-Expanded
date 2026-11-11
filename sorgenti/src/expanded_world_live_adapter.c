/* Import only at new-career start, before the unemployed modal.
 * Loading team files inside a modal callback would overwrite menu/team state.
 * Same five validated data files already used by club offer generation.
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
void ExpandedWorldLive_LoadOfferCountries(void) {
    static const uint8_t countries[]={8,13,14,20,21};
    if (!ExpandedWorldLive_Begin(World_FirstYear())) {
        fprintf(stderr,"[living-world] failed to reserve memory for pilot\n");
        return;
    }
    uint16_t old_skip=_skipLeagueNumbers;
    _skipLeagueNumbers=1; /* raw divisions, not prior-season promotion offsets */
    for(unsigned k=0;k<sizeof countries/sizeof countries[0];++k) {
        uint8_t country=countries[k];
        const void *const *table=(const void *const *)_competitionsTable[country];
        if (!table || !table[0] || (uintptr_t)table[0]==(uintptr_t)-1 ||
            (uintptr_t)table[0]==(uintptr_t)-2) continue;
        const Competition *base=(const Competition *)table[0];
        if (base->type!=0 || comp_is_type0_cup(base->compId)) continue;
        const CompetitionLeague *league=(const CompetitionLeague *)base;
        int divisions=league->header.divisionsCount;
        if (divisions<1 || divisions>16) continue;
        int points=league->header.pointsForWin;
        if (points!=2 && points!=3) points=3;
        (void)import_file(country,country,(unsigned)divisions,(unsigned)points);
#if SWOS_WORLDS
        uint8_t second=World_SecondFile(country);
        if (second && second!=country)
            (void)import_file(second,country,(unsigned)divisions,(unsigned)points);
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
        fprintf(stderr,
            "[living-world] pilot ready: %u clubs, %u groups; start=%u, "
            "native SWOS league schedule groups=%lu\n",
            (unsigned)ExpandedWorldLive_Clubs(),
            (unsigned)ExpandedWorldLive_Leagues(),(unsigned)World_FirstYear(),
            (unsigned long)ExpandedWorldLive_NativeScheduleLeagues());
    }
}
#else
void ExpandedWorldLive_LoadOfferCountries(void) {}
#endif
