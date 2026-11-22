/* RG-001/012/014/027 - annually recurring country edition boundaries.
 * The DATA contain month/day, never a permanent 2027 deadline.
 * Build a date for the requested OG edition year. No calendar tick,
 * competition reset, fixture simulation, RNG or CAR side effect.
 * A special historical/transitional edition overrides this template.
 * Validate ALL inputs before modifying output (fail closed). */
#ifndef EXPANDED_COUNTRY_SEASON_YEAR_H
#define EXPANDED_COUNTRY_SEASON_YEAR_H
#include "expanded_world_time.h"

typedef struct ExpandedCountrySeasonCycle {
    uint8_t start_month,start_day,expiry_month,expiry_day;
} ExpandedCountrySeasonCycle;

static inline int ExpandedCountrySeason_LeapYear(unsigned y) {
    return (y%4u==0u) && ((y%100u!=0u)||(y%400u==0u));
}
static inline int ExpandedCountrySeason_DaysInMonth(unsigned year,unsigned month)
{
    static const uint8_t kDays[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    if(year<1900u||year>2200u||month<1u||month>12u)return 0;
    return (int)kDays[month-1u]+
           (month==2u && ExpandedCountrySeason_LeapYear(year));
}
static inline int ExpandedCountrySeason_ValidDay(
    unsigned year,unsigned month,unsigned day)
{
    const int total=ExpandedCountrySeason_DaysInMonth(year,month);
    return total>0 && day>=1u && day<=(unsigned)total;
}
static inline int ExpandedCountrySeason_AnnualDates(
    uint16_t edition_year,ExpandedCountrySeasonCycle cycle,
    ExpandedWorldDate *out_start,ExpandedWorldDate *out_expiry)
{
    if(!out_start||!out_expiry||out_start==out_expiry ||
       edition_year<1900u||edition_year>2199u ||
       !ExpandedCountrySeason_ValidDay(edition_year,
           cycle.start_month,cycle.start_day))return 0;
    /* An expiry earlier in the calendar than the start is in the
     * FOLLOWING civil year. Recompute whenever an edition increments. */
    const unsigned start_md=(unsigned)cycle.start_month*32u+cycle.start_day;
    const unsigned expiry_md=(unsigned)cycle.expiry_month*32u+cycle.expiry_day;
    if(expiry_md==start_md)return 0;
    const uint16_t expiry_year=(uint16_t)(
        edition_year+(expiry_md<start_md?1u:0u));
    if(!ExpandedCountrySeason_ValidDay(expiry_year,
            cycle.expiry_month,cycle.expiry_day))return 0;
    const ExpandedWorldDate start={
        edition_year,cycle.start_month,cycle.start_day};
    const ExpandedWorldDate expiry={
        expiry_year,cycle.expiry_month,cycle.expiry_day};
    *out_start=start;
    *out_expiry=expiry;
    return 1;
}
/* Resolve a non-leap-anniversaryed special edition from EXPLICIT
 * verified game/competition metadata, never by mutating old year.
 * In particular MLS 2027 sprint and historic Bolivia 2026 cannot
 * inherit generic annual templates as if formats had not changed. */
static inline int ExpandedCountrySeason_SpecialDates(
    ExpandedWorldDate start,ExpandedWorldDate expiry,
    ExpandedWorldDate *out_start,ExpandedWorldDate *out_expiry)
{
    if(!out_start||!out_expiry||out_start==out_expiry ||
       !ExpandedCountrySeason_ValidDay(start.year,start.month,start.day) ||
       !ExpandedCountrySeason_ValidDay(expiry.year,expiry.month,expiry.day) ||
       expiry.year<start.year ||
       (expiry.year==start.year &&
        (expiry.month<start.month ||
         (expiry.month==start.month && expiry.day<=start.day))))
        return 0;
    *out_start=start;
    *out_expiry=expiry;
    return 1;
}
#endif /* EXPANDED_COUNTRY_SEASON_YEAR_H */
