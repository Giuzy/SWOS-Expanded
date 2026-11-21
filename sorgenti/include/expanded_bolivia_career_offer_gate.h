/* RG-014: new job offers are optional when the manager already has
 * a real OG club. They remain visible in JOB OFFERS but must not stall
 * either the manual or automatic civil advancement of that club.
 * Accepting an offer is a separate mandatory transition. Pure helper:
 * no result writer, no CAR mutation, no clock. */
#ifndef EXPANDED_BOLIVIA_CAREER_OFFER_GATE_H
#define EXPANDED_BOLIVIA_CAREER_OFFER_GATE_H

#include <stdint.h>

static inline int ExpandedBoliviaCareer_OfferStops(
    unsigned newOffers, uint16_t selectedClub, int nativeClubBound)
{
    return newOffers != 0u &&
           (selectedClub == UINT16_MAX || !nativeClubBound);
}

#endif /* EXPANDED_BOLIVIA_CAREER_OFFER_GATE_H */
