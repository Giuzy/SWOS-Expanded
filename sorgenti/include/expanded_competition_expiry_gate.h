/* RG-001/RG-014: per-competition lifecycle gate, not a simulator.
 * A civil expiry is necessary but never sufficient: only completed,
 * witnessed ORIGINAL OG fixtures and final/KO stages allow retirement.
 * This helper does not own an edition, world clock, RNG, or saved CAR. */
#ifndef EXPANDED_COMPETITION_EXPIRY_GATE_H
#define EXPANDED_COMPETITION_EXPIRY_GATE_H
#include <stdint.h>
typedef enum ExpandedCompetitionExpiryDecision {
    EXP_COMP_EXPIRY_INVALID=-1,
    EXP_COMP_EXPIRY_WAIT_DATE=0,
    EXP_COMP_EXPIRY_WAIT_OG=1,
    EXP_COMP_EXPIRY_CAN_RETIRE=2
} ExpandedCompetitionExpiryDecision;
static inline ExpandedCompetitionExpiryDecision
ExpandedCompetitionExpiry_Check(int32_t now,int32_t expiry,
    uint32_t total_expected,uint32_t total_played,
    uint32_t pending_fixtures,int og_completed)
{
    if(now<0 || expiry<0 || !total_expected ||
       total_played>total_expected || og_completed<0 || og_completed>1)
        return EXP_COMP_EXPIRY_INVALID;
    if(now<expiry)return EXP_COMP_EXPIRY_WAIT_DATE;
    if(total_played!=total_expected || pending_fixtures || !og_completed)
        return EXP_COMP_EXPIRY_WAIT_OG;
    return EXP_COMP_EXPIRY_CAN_RETIRE;
}
#endif
