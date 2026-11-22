/* RG-009/014/027/031 — One-shot F12 career offer +7 calendar days.
 * Isolated C11 check only, NOT Windows build, UI, OG or CAR gameplay. */
#include "../include/expanded_debug_offer_week.h"
#include <assert.h>
#include <limits.h>
int main(void)
{
    ExpandedDebugOfferWeek s={0};
    assert(ExpandedDebugOffer_Take(&s,1u,500u)==0);
    assert(ExpandedDebugOffer_Toggle(&s,1u,100u)==1);
    assert(s.armed && s.due==107 && s.world==1u);
    assert(ExpandedDebugOffer_Take(&s,1u,106u)==0);
    assert(ExpandedDebugOffer_Take(&s,1u,107u)==1);
    assert(ExpandedDebugOffer_Take(&s,1u,107u)==0); /* once */
    assert(ExpandedDebugOffer_Toggle(&s,0u,200u)==1);
    assert(ExpandedDebugOffer_Toggle(&s,0u,200u)==0); /* OFF */
    assert(ExpandedDebugOffer_Take(&s,0u,207u)==0);
    assert(ExpandedDebugOffer_Toggle(&s,0u,201u)==1);
    assert(ExpandedDebugOffer_Take(&s,0u,220u)==1); /* first later valid tick */
    assert(ExpandedDebugOffer_Toggle(&s,1u,300u)==1);
    assert(ExpandedDebugOffer_Toggle(&s,2u,300u)==-1);
    assert(s.armed && s.world==1u && s.due==307); /* invalid unchanged */
    assert(ExpandedDebugOffer_Take(&s,0u,307u)==0); /* other world cancels */
    assert(s.armed==0);
    assert(ExpandedDebugOffer_Toggle(&s,0u,400u)==1);
    assert(ExpandedDebugOffer_Take(&s,0u,-1)==0); /* bad clock retain arm */
    assert(s.armed);
    ExpandedDebugOffer_Reset(&s);
    assert(!s.armed);
    assert(ExpandedDebugOffer_Toggle(&s,0u,INT32_MAX-6)==-1);
    assert(!s.armed);
    assert(ExpandedDebugOffer_Toggle((ExpandedDebugOfferWeek*)0,0u,100u)==-1);
    /* Generic ordinal-day boundary: +7 is exact arithmetic.
     * Actual Dec/Jan date-to-index mapping requires the world-time tests. */
    assert(ExpandedDebugOffer_Toggle(&s,1u,46384u)==1);
    assert(s.due==46391);
    assert(ExpandedDebugOffer_Take(&s,1u,46390u)==0);
    assert(ExpandedDebugOffer_Take(&s,1u,46391u)==1);
    return 0;
}
