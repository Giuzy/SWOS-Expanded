/* Debug-only one-shot calendar-week offer trigger.
 * Pure state machine. NOT an OG result engine, an offer generator,
 * a second civil clock or a CAR field. All dates are indices produced
 * by ExpandedWorld_DateToIndex on the SINGLE live world clock.
 * RG-009/014/027/031, 22 November 2026.
 */
#ifndef EXPANDED_DEBUG_OFFER_WEEK_H
#define EXPANDED_DEBUG_OFFER_WEEK_H
#include <stdint.h>
#include <limits.h>
typedef struct ExpandedDebugOfferWeek {
    int32_t due;
    uint16_t world;
    uint8_t armed;
} ExpandedDebugOfferWeek;
static inline void ExpandedDebugOffer_Reset(ExpandedDebugOfferWeek *s)
{
    if(s) {
        s->armed=0;
        s->due=0;
        s->world=0;
    }
}
/* Returns: 1 armed, 0 disarmed, -1 invalid; invalid input is
 * fail-closed and does not change the existing state. */
static inline int ExpandedDebugOffer_Toggle(ExpandedDebugOfferWeek *s,
                                             uint16_t world,int32_t today)
{
    if(!s || world>1u || today<0 || today>INT32_MAX-7)
        return -1;
    if(s->armed && s->world==world) {
        ExpandedDebugOffer_Reset(s);
        return 0;
    }
    s->world=world;
    s->due=today+7; /* EXACT seven civil days, including year boundaries */
    s->armed=1;
    return 1;
}
/* Do not consume on a failed native/civil advance. The game invokes
 * Take only from a successfully committed calendar/UI transition.
 * Return 1 once, on the first eligible frame at/after the due day.
 * Crossing to another world cancels the request; no state leak. */
static inline int ExpandedDebugOffer_Take(ExpandedDebugOfferWeek *s,
                                           uint16_t world,int32_t today)
{
    if(!s || !s->armed || today<0)return 0;
    if(world!=s->world || world>1u) {
        ExpandedDebugOffer_Reset(s);
        return 0;
    }
    if(today<s->due)return 0;
    ExpandedDebugOffer_Reset(s);
    return 1;
}
#endif
