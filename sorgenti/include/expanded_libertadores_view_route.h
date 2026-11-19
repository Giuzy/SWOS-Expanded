#ifndef EXPANDED_LIBERTADORES_VIEW_ROUTE_H
#define EXPANDED_LIBERTADORES_VIEW_ROUTE_H
/* The original 1996 Copa's world viewer is read-only. A spectator manager
 * must not need the Copa slot3; active-slot identity is only an extra route
 * when the club really owns that slot. This helper never mutates OG state. */
static inline int Expanded_LibertadoresResidentViewerRoute(
    unsigned gameType,int historicWorld,const void *selected,
    const void *original,const void *careerCopy,
    const void *activeSlot,int activeSlotIsCopa)
{
    return gameType==4u && historicWorld && selected && original &&
           (selected==original ||
            (careerCopy && selected==careerCopy) ||
            (activeSlotIsCopa && activeSlot && selected==activeSlot));
}
#endif
