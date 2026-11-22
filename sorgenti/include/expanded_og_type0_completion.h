/* SWOS Expanded: evidence-only completion of an ordinary OG TYPE-0 league.
 * No fixture generation, standings mutation, RNG or CAR writes. */
#ifndef EXPANDED_OG_TYPE0_COMPLETION_H
#define EXPANDED_OG_TYPE0_COMPLETION_H
#include <stdint.h>

#define EXP_OG_TYPE0_PROOF_CAPACITY 64u
typedef struct {
    int32_t team_index;         /* OG resident selectedTeams index, not -1 */
    uint16_t club_id;          /* Stable OG packed country + team ordinal */
    uint16_t games, wins, draws, losses, goals_for, goals_against;
} ExpandedOGType0ProofRow;

static inline int ExpandedOGType0_CompletedFromOG(
    unsigned teams, unsigned cycles, unsigned matches,
    const ExpandedOGType0ProofRow *rows)
{
    if(!rows || teams<2u || teams>EXP_OG_TYPE0_PROOF_CAPACITY ||
       cycles<1u || cycles>4u)return 0;
    const unsigned each=(teams-1u)*cycles;
    const unsigned expected=teams*each/2u;
    if(!expected || matches!=expected)return 0;
    uint8_t seen[EXP_OG_TYPE0_PROOF_CAPACITY]={0};
    uint64_t games=0u,wins=0u,draws=0u,losses=0u,gf=0u,ga=0u;
    for(unsigned i=0u;i<teams;++i) {
        const ExpandedOGType0ProofRow *r=&rows[i];
        if(r->team_index<0 || (unsigned)r->team_index>=teams ||
           seen[r->team_index] || r->club_id==UINT16_MAX ||
           r->games!=each ||
           (unsigned)r->wins+r->draws+r->losses!=r->games)return 0;
        for(unsigned j=0u;j<i;++j)
            if(rows[j].club_id==r->club_id)return 0;
        seen[r->team_index]=1u;
        games+=r->games;wins+=r->wins;draws+=r->draws;
        losses+=r->losses;gf+=r->goals_for;ga+=r->goals_against;
    }
    return games==(uint64_t)matches*2u && wins==losses &&
           (draws%2u)==0u && gf==ga;
}
#endif
