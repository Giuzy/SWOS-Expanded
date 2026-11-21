/* RG-014: user-reported 19/09/2026 offer must not stop an employed
 * manager's manual OR automatic calendar. Pure C11, no Windows runtime. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "expanded_bolivia_career_offer_gate.h"

int main(void)
{
    /* Actual report: manager at Nacional Potosi, 1 NEW offer. */
    assert(!ExpandedBoliviaCareer_OfferStops(1u,0x2d06u,1));
    assert(!ExpandedBoliviaCareer_OfferStops(8u,0x2d06u,1));
    assert(!ExpandedBoliviaCareer_OfferStops(1u,0x2d0au,1));
    /* Unemployed or no matching OG bound club is NOT ignored. */
    assert(ExpandedBoliviaCareer_OfferStops(1u,UINT16_MAX,0));
    assert(ExpandedBoliviaCareer_OfferStops(3u,UINT16_MAX,0));
    assert(ExpandedBoliviaCareer_OfferStops(1u,0x2d06u,0));
    assert(ExpandedBoliviaCareer_OfferStops(1u,UINT16_MAX,1));
    assert(!ExpandedBoliviaCareer_OfferStops(0u,UINT16_MAX,0));
    assert(!ExpandedBoliviaCareer_OfferStops(0u,0x2d06u,1));
    puts("RG-014 CAREER_OFFER_GATE_ISO PASS: 9 employed/unemployed negatives");
    puts("WINDOWS_CHANGE_CLUB_CAR: NOT_TESTED");
    return 0;
}
