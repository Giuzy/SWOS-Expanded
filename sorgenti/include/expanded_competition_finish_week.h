/* Career annual closure: civil Sunday boundaries, no match generation.
 * Monday=0 .. Sunday=6 follows ExpandedWorld_Weekday from 1900-01-01.
 * Strictly forward; the last step ends exactly at 30 June even if that
 * date is not a Sunday. This helper never owns a world clock or result. */
#ifndef EXPANDED_COMPETITION_FINISH_WEEK_H
#define EXPANDED_COMPETITION_FINISH_WEEK_H
#include <stdint.h>
static inline int ExpandedFinish_DaysToNextSundayOrEnd(
    int weekday,int32_t remaining,int32_t *out_days)
{
    if(!out_days || weekday<0 || weekday>6 || remaining<=0)
        return 0;
    const int32_t sunday=weekday==6?7:(int32_t)(6-weekday);
    *out_days=remaining<sunday?remaining:sunday;
    return *out_days>0 && *out_days<=7;
}
#endif
