/* RG-014/022/027: read-only CAR guard for a country OG edition that
 * begins before the 1 July administrative compatibility year increments.
 * Do not change the CAR schema, global year, native result writer or clock.
 * The resident's own verified country cycle is the only exemption. */
#ifndef EXPANDED_OG_CAR_COUNTRY_EDITION_H
#define EXPANDED_OG_CAR_COUNTRY_EDITION_H

#include <stdint.h>
#include "expanded_country_season_baked.h"

static inline int ExpandedOGCar_FutureCountryEditionAllowed(
    int modern_world,uint16_t global_year,uint16_t country,uint16_t edition_year,
    ExpandedWorldDate today)
{
    ExpandedCountrySeasonCycle cycle;
    ExpandedWorldDate start={0},expiry={0};
    /* No future years for old worlds, unregistered/unknown countries,
     * skipped years or unverified one-off transitions (MLS 2027). */
    if(modern_world!=1 || global_year<1900u || global_year>=2199u ||
       edition_year!=(uint16_t)(global_year+1u) ||
       today.year!=edition_year ||
       (country==73u && edition_year==2027u) ||
       !ExpandedCountrySeason_Baked2627(country,&cycle) ||
       !ExpandedCountrySeason_AnnualDates(edition_year,cycle,
                                           &start,&expiry) ||
       !ExpandedCountrySeason_ValidDay(today.year,today.month,today.day))
        return 0;
    /* Exact monotonic YYYYMMDD ordering; no extra clock or timer. */
    const uint32_t now=(uint32_t)today.year*10000u+
                         (uint32_t)today.month*100u+today.day;
    const uint32_t begin=(uint32_t)start.year*10000u+
                           (uint32_t)start.month*100u+start.day;
    const uint32_t end=(uint32_t)expiry.year*10000u+
                         (uint32_t)expiry.month*100u+expiry.day;
    return now>=begin && now<=end;
}
#endif /* EXPANDED_OG_CAR_COUNTRY_EDITION_H */
