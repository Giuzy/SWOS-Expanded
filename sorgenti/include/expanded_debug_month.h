#ifndef EXPANDED_DEBUG_MONTH_H
#define EXPANDED_DEBUG_MONTH_H

#include "expanded_world_time.h"

/* Calendar-only, no world mutation. Returns a date one calendar month later,
 * clamped to the LAST day before OG's 1 July season change. Calling at 30 June
 * does nothing: the user must trigger regular FORWARD TIME to cross the year.
 * Day 31 -> last valid day in shorter target month, including leap years. */
static inline int ExpandedDebug_NextMonthBeforeSeason(ExpandedWorldDate today,
                                                       ExpandedWorldDate *out)
{
    int32_t now=0,limit=0,target=0;
    if(!out || !ExpandedWorld_DateToIndex(today,&now))return 0;
    const uint16_t season=(uint16_t)(today.year-(today.month<7u));
    if(season>=2200u)return 0;
    const ExpandedWorldDate last={(uint16_t)(season+1u),6u,30u};
    if(!ExpandedWorld_DateToIndex(last,&limit) || now>=limit)return 0;
    ExpandedWorldDate next=today;
    if(next.month==12u) {next.month=1u;++next.year;}
    else ++next.month;
    /* Preserve day number where possible; clamp invalid 29/30/31. */
    while(next.day>1u && !ExpandedWorld_DateToIndex(next,&target))
        --next.day;
    if(!ExpandedWorld_DateToIndex(next,&target))return 0;
    if(target>limit) {next=last;target=limit;}
    if(target<=now)return 0;
    *out=next;
    return 1;
}
/* The last day of the CURRENT OG career season, not the next calendar
 * year's arbitrary June. It never crosses 1 July or mutates the clock. */
static inline int ExpandedDebug_June30BeforeSeason(ExpandedWorldDate today,
                                                   ExpandedWorldDate *out)
{
    int32_t now=0,limit=0;
    if(!out || !ExpandedWorld_DateToIndex(today,&now))return 0;
    const uint16_t season=(uint16_t)(today.year-(today.month<7u));
    if(season>=2200u)return 0;
    const ExpandedWorldDate last={(uint16_t)(season+1u),6u,30u};
    if(!ExpandedWorld_DateToIndex(last,&limit) || now>=limit)return 0;
    *out=last;
    return 1;
}

#endif
