/* RG-001/014/027: the Bolivia event programme belongs to its COUNTRY
 * edition, not to the JULY administrative career year.
 * Pure-date C11 contract; not a gameplay, Windows or save test. */
#include <assert.h>
#include <stdio.h>
#include "expanded_country_season_year.h"
#include "expanded_bolivia_event_registry.h"

static int same(ExpandedWorldDate date,unsigned year,unsigned month,unsigned day)
{ return date.year==year && date.month==month && date.day==day; }

int main(void)
{
    const ExpandedCountrySeasonCycle cycle={1u,15u,1u,14u};
    ExpandedWorldDate start={0},expiry={0},next={0};
    const ExpandedWorldDate jan10={2027u,1u,10u};
    const ExpandedWorldDate jan15={2027u,1u,15u};
    const ExpandedWorldDate jan17={2027u,1u,17u};
    const ExpandedWorldDate june30={2027u,6u,30u};
    /* Historical first entry is Apr 3, 2026; next edition opens Jan15. */
    assert(ExpandedCountrySeason_AnnualDates(2027u,cycle,&start,&expiry));
    assert(same(start,2027u,1u,15u));
    assert(same(expiry,2028u,1u,14u));
    /* The 10-Jan forward has no older 2026 match date before 17 Jan:
     * the separate COUNTRY_START splitter must insert 15 Jan. */
    assert(ExpandedBolivia_NextBoundary(2026u,jan10,jan17,&next));
    assert(same(next,2027u,1u,17u));
    /* After the Jan-15 native tick, use country edition 2027.  The next
     * real 2027 League matchday must be Apr 1, not June 30 (old-year bug). */
    assert(ExpandedBolivia_NextBoundary(2026u,jan15,june30,&next));
    assert(same(next,2027u,6u,30u)); /* negative reproduces prior bug */
    assert(ExpandedBolivia_NextBoundary(2027u,jan15,june30,&next));
    assert(same(next,2027u,4u,1u));
    assert(ExpandedBolivia_NextBoundary(2027u,
          (ExpandedWorldDate){2027u,4u,1u},june30,&next));
    assert(same(next,2027u,4u,8u));
    /* No extra event when moving from the last known December matchday
     * to the real Jan-14 expiry; native fixtures remain the sole writer. */
    assert(ExpandedBolivia_NextBoundary(2026u,
          (ExpandedWorldDate){2026u,12u,12u},
          (ExpandedWorldDate){2027u,1u,14u},&next));
    assert(same(next,2027u,1u,14u));
    puts("PASS Bolivia country edition native boundaries Jan10/Jan15/Jun30 (ISO)");
    return 0;
}
