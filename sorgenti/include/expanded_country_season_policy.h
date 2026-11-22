/* RG-001/012/014/027: country owns manager-season policy, not July.
 * A national competition inherits its country's period unless the
 * individual edition explicitly overrides BOTH start and expiry.
 * Pure selection and gate only: no OG tick, fixtures, RNG, CAR or clock.
 * All dates must be indices of the SAME ExpandedWorldDate civil clock. */
#ifndef EXPANDED_COUNTRY_SEASON_POLICY_H
#define EXPANDED_COUNTRY_SEASON_POLICY_H
#include <stddef.h>
#include <stdint.h>
#include "expanded_competition_expiry_gate.h"

typedef struct ExpandedCountrySeasonWindow {
    uint16_t country;
    int32_t start;
    int32_t expiry;
} ExpandedCountrySeasonWindow;

/* -1/-1 means inherit; only a COMPLETE per-edition pair can override.
 * Exact country identity, not competition ID or country of previous club.
 * Reject duplicates/unknown rows: do not silently use the Europe cycle. */
static inline int ExpandedCountrySeason_Resolve(
    const ExpandedCountrySeasonWindow *countries,size_t count,
    uint16_t current_club_country,int32_t override_start,
    int32_t override_expiry,ExpandedCountrySeasonWindow *out)
{
    const ExpandedCountrySeasonWindow *found=NULL;
    if(!countries || !out || count==0u || count>256u ||
       current_club_country>=256u)return 0;
    for(size_t i=0u;i<count;++i) {
        if(countries[i].country==current_club_country) {
            if(found)return 0;
            found=&countries[i];
        }
    }
    if(!found || found->start<0 || found->expiry<found->start)return 0;
    /* Reject ALL malformed overrides before touching the caller's
     * output: a failure must never replace the manager's prior binding. */
    if(override_start==-1 && override_expiry==-1) {
        *out=*found;
        return 1;
    }
    if(override_start<0 || override_expiry<override_start)return 0;
    *out=*found;
    out->start=override_start;
    out->expiry=override_expiry;
    return 1;
}

/* Administrative 1 July is NOT a signal here. Only a native OG
 * league/cup/KO final witness can permit this competition to retire. */
static inline ExpandedCompetitionExpiryDecision
ExpandedCountrySeason_CanRetire(
    int32_t now,const ExpandedCountrySeasonWindow *edition,
    uint32_t expected,uint32_t played,uint32_t pending,int og_complete)
{
    if(!edition || edition->start<0 ||
       edition->expiry<edition->start || now<edition->start)
        return EXP_COMP_EXPIRY_INVALID;
    return ExpandedCompetitionExpiry_Check(now,edition->expiry,
        expected,played,pending,og_complete);
}

/* Old edition and valid journal must be retired BEFORE a successor can
 * be opened. No second active native fixture set for the same key. */
static inline int ExpandedCountrySeason_CanOpenNext(
    int32_t now,int32_t successor_start,int previous_edition_exists,
    ExpandedCompetitionExpiryDecision previous)
{
    if(now<0 || successor_start<0 || previous_edition_exists<0 ||
       previous_edition_exists>1 || now<successor_start)return 0;
    if(!previous_edition_exists)return 1;
    return previous==EXP_COMP_EXPIRY_CAN_RETIRE;
}
#endif
