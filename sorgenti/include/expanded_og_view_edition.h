/* RG-014/027 — Read-only native OG league viewer edition priority.
 * A competition record pointer can represent DIFFERENT true resident
 * years. Never display an older or future OG league as current.
 * Pure selector: no clock/fixture/random/cache/CAR writes. */
#ifndef EXPANDED_OG_VIEW_EDITION_H
#define EXPANDED_OG_VIEW_EDITION_H
#include <stdint.h>

typedef struct ExpandedOGViewEditionChoice {
    uint32_t index;
    uint16_t year;
    uint8_t country,rank,has,conflict;
} ExpandedOGViewEditionChoice;

static inline void ExpandedOGViewEdition_Consider(
    ExpandedOGViewEditionChoice *choice,uint32_t index,
    uint8_t country,uint16_t residentYear,uint16_t currentYear,
    int samePointer,int sameId)
{
    if(!choice || !sameId || !residentYear || !currentYear ||
       residentYear>currentYear)return;
    const unsigned rank=(residentYear==currentYear?4u:0u)+
                        (samePointer?2u:1u);
    if(!choice->has || rank>choice->rank) {
        choice->index=index;
        choice->year=residentYear;
        choice->country=country;
        choice->rank=(uint8_t)rank;
        choice->conflict=0u;
        choice->has=1u;
        return;
    }
    if(rank<choice->rank)return;
    if(country!=choice->country || residentYear==choice->year) {
        choice->conflict=1u;
        return;
    }
    /* Only when the true current edition is absent, choose the most
     * recent actual saved year of the same OG country/competition. */
    if(residentYear>choice->year) {
        choice->index=index;
        choice->year=residentYear;
    }
}
static inline int ExpandedOGViewEdition_Selected(
    const ExpandedOGViewEditionChoice *choice,uint32_t *index)
{
    if(!choice || !index || !choice->has || choice->conflict)return 0;
    *index=choice->index;
    return 1;
}
#endif
