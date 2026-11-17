#ifndef EXPANDED_CHAMPIONS_VIEW_ROUTE_H
#define EXPANDED_CHAMPIONS_VIEW_ROUTE_H
/* Pure routing rule shared with the historical OG career viewer.
 * A club contract is NOT a condition for consulting a world tournament:
 * neither slot-3 participation nor a second simulation belongs here. */
static inline int Expanded_ChampionsResidentViewerRoute(
    unsigned gameType,int historicalWorld,const void *selected,
    const void *original,const void *careerCopy)
{
    return gameType==4u && historicalWorld && selected &&
           (selected==original || selected==careerCopy);
}
#endif
