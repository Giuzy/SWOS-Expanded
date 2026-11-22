/* OG-only country edition transition counter proof.
 * The active resident counter may SHRINK when a verified closed edition
 * leaves the active slot and its successor opens with zero played games.
 * The historical match journal/edition archive remains the anti-replay
 * authority; this pure function never edits fixtures, saves or OG data.
 * Callers must independently certify the old OG competition finished. */
#ifndef EXPANDED_OG_EDITION_TRANSITION_H
#define EXPANDED_OG_EDITION_TRANSITION_H
#include <stdint.h>
#include <limits.h>
static inline int ExpandedOGEditionTransition_CertifiedCounter(
    uint16_t oldEdition,uint16_t newEdition,
    uint32_t expectedRetiredMatches,uint32_t actualRetiredMatches,
    uint32_t newEditionMatches,int oldOGCertified,
    uint32_t worldBefore,uint32_t worldAfter,
    uint32_t *worldAfterComparable)
{
    if(!worldAfterComparable || !oldOGCertified || !oldEdition ||
       oldEdition==UINT16_MAX ||
       newEdition!=(uint16_t)(oldEdition+1u) ||
       !expectedRetiredMatches ||
       actualRetiredMatches!=expectedRetiredMatches ||
       newEditionMatches!=0u ||
       worldBefore<actualRetiredMatches ||
       worldAfter>UINT32_MAX-actualRetiredMatches)
        return 0;
    const uint32_t comparable=worldAfter+actualRetiredMatches;
    if(comparable<worldBefore)return 0;
    *worldAfterComparable=comparable;
    return 1;
}
#endif
