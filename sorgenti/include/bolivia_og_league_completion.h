/* RG-001/014/027: verify the native OG Bolivia Division Profesional has
 * finished ALL 30 league dates before a January edition can retire.
 * This is read-only: no match generation, flag mutation, RNG or CAR writes.
 * The OG type-0 flag 'competitionFinished' is not a reliable indicator of
 * an already-completed native league table in the resident world. */
#ifndef BOLIVIA_OG_LEAGUE_COMPLETION_H
#define BOLIVIA_OG_LEAGUE_COMPLETION_H
#include <stdint.h>

enum { BOLIVIA_OG_LEAGUE_TEAMS = 16,
       BOLIVIA_OG_LEAGUE_GAMES_PER_TEAM = 30,
       BOLIVIA_OG_LEAGUE_TOTAL_MATCHES = 240 };
typedef struct BoliviaOGLeagueProofRow {
    int32_t teamIndex;
    uint16_t gamesPlayed, wins, draws, losses, goalsFor, goalsAgainst;
} BoliviaOGLeagueProofRow;

static inline int BoliviaOGLeague_CompletedFromOG(
    unsigned teamCount, unsigned matchesPlayed,
    const BoliviaOGLeagueProofRow rows[BOLIVIA_OG_LEAGUE_TEAMS])
{
    if(!rows || teamCount!=BOLIVIA_OG_LEAGUE_TEAMS ||
       matchesPlayed!=BOLIVIA_OG_LEAGUE_TOTAL_MATCHES)return 0;
    uint32_t seen=0u;
    uint64_t gp=0,w=0,d=0,l=0,gf=0,ga=0;
    for(unsigned i=0;i<BOLIVIA_OG_LEAGUE_TEAMS;++i) {
        const BoliviaOGLeagueProofRow *s=&rows[i];
        if(s->teamIndex<0 ||
           (unsigned)s->teamIndex>=BOLIVIA_OG_LEAGUE_TEAMS ||
           (seen & (1u << (unsigned)s->teamIndex)) ||
           s->gamesPlayed!=BOLIVIA_OG_LEAGUE_GAMES_PER_TEAM ||
           (unsigned)s->gamesPlayed!=(unsigned)s->wins+s->draws+s->losses)
            return 0;
        seen|=1u<<(unsigned)s->teamIndex;
        gp+=s->gamesPlayed;w+=s->wins;d+=s->draws;l+=s->losses;
        gf+=s->goalsFor;ga+=s->goalsAgainst;
    }
    return seen==0xffffu && gp==2u*BOLIVIA_OG_LEAGUE_TOTAL_MATCHES &&
           w==l && !(d&1u) && gf==ga;
}
#endif
