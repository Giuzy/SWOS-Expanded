#ifndef EXPANDED_CLASSIC_CUP_VIEW_ROUTE_H
#define EXPANDED_CLASSIC_CUP_VIEW_ROUTE_H
/* Pure VIEW routing: historical European type-1 cups are observable even
 * when the manager has no cup slot. The native competition template, not
 * the career working copy, is passed into the resident read-only viewer. */
static inline int Expanded_ClassicCupResidentViewerRoute(
    unsigned gameType,int historical,const void *selected,
    const void *original,const void *careerCopy,
    const void *activeSlot,int activeSlotIsThisCup)
{
    return gameType==4u && historical && selected && original &&
           (selected==original ||
            (careerCopy && selected==careerCopy) ||
            (activeSlotIsThisCup && activeSlot && selected==activeSlot));
}
/* Do not mistake the stored *previous* edition for the current OG season. */
static inline int Expanded_ClassicCupCurrentEdition(
    unsigned candidateYear,unsigned worldYear,int resident)
{
    return resident && candidateYear==worldYear;
}
#endif
