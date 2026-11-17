#ifndef EXPANDED_OG_SEASON_RETENTION_H
#define EXPANDED_OG_SEASON_RETENTION_H
#include <stdint.h>
/* Keep ONLY actual native results from the last continental edition.
 * Neither invent next-year entrants nor relabel a completed season.
 * Save format 1 already serializes its native snapshots. */
static inline int ExpandedOG_RetainContinentalType2(
    int rollover,unsigned used,uint16_t recordYear,uint16_t editionYear)
{
    return rollover && used && editionYear && recordYear==editionYear;
}
static inline int ExpandedOG_RetainContinentalType1(
    int rollover,unsigned used,uint8_t country,uint8_t slot,
    uint16_t recordYear,uint16_t editionYear)
{
    return ExpandedOG_RetainContinentalType2(
               rollover,used,recordYear,editionYear) &&
           country==254u && (slot==1u || slot==2u);
}
#endif
