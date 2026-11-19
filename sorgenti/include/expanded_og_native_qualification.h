/* SWOS Expanded: read-only identities of completed OG cup editions.
 * Native OG writes the winning club into DIYCompetition.resultCode,
 * the same field as base.winnerTeam in the active competition struct.
 * The detached tournament-header field is NOT the authoritative winner.
 * We only accept a unique club ID actually in the persisted OG roster.
 * No competition initialization, simulation, RNG or save mutation.
 */
#ifndef EXPANDED_OG_NATIVE_QUALIFICATION_H
#define EXPANDED_OG_NATIVE_QUALIFICATION_H
#include <stdint.h>

static inline int ExpandedOGQual_WinnerIndex(uint16_t native_winner,
                                            const uint16_t *participants,
                                            unsigned count)
{
    int index=-1;
    if(!participants || count<2u || count>64u ||
       native_winner==0xffffu)return -1;
    for(unsigned i=0;i<count;++i) {
        if(participants[i]!=native_winner)continue;
        if(index>=0)return -1; /* duplicate club -> not unique */
        index=(int)i;
    }
    return index;
}

/* The edition identity is (country, native table slot, edition year),
 * NOT compId alone (which several national cups can share). */
static inline int ExpandedOGQual_SameCupEdition(
    unsigned country,unsigned slot,uint16_t year,
    unsigned wanted_country,unsigned wanted_slot,uint16_t wanted_year)
{
    return country==wanted_country && slot==wanted_slot &&
           year==wanted_year;
}

/* Exact 1996/97 OG Cup Winners' Cup priority: three cup-bracket
 * finalists/candidates from DIY_CacheSecondaryQualifiedTeamsGrid.
 * That native routine swaps the first final pair by the final result
 * bit (matchPairsPerRound==1), then copies grid positions [0..2].
 * The caller supplies BE-decoded original cupTeamIds and verifies all
 * returned IDs against the original resident participant list. */
static inline int ExpandedOGQual_ClassicCupPodium(
    const uint8_t *grid,uint8_t finalResult,unsigned finalPairs,
    const uint16_t *nativeTeamIds,const uint16_t *participants,
    unsigned count,uint16_t knownWinner,uint16_t podium[3])
{
    if(!grid || !nativeTeamIds || !participants || !podium ||
       count<3u || count>64u || finalPairs!=1u)return 0;
    const unsigned slots[3]={
        (unsigned)grid[(finalResult&1u)?1u:0u],
        (unsigned)grid[(finalResult&1u)?0u:1u],
        (unsigned)grid[2]};
    for(unsigned k=0;k<3u;++k) {
        if(slots[k]>=count)return 0;
        const uint16_t id=nativeTeamIds[slots[k]];
        if(ExpandedOGQual_WinnerIndex(id,participants,count)<0)return 0;
        for(unsigned j=0;j<k;++j)
            if(podium[j]==id)return 0;
        podium[k]=id;
    }
    return podium[0]==knownWinner;
}

#endif
