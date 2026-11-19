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

#endif
