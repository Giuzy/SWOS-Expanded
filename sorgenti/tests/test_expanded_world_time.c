#include <assert.h>
#include <stdio.h>
#include "expanded_world_time.h"
static void roundtrip(unsigned y, unsigned m, unsigned d) {
    ExpandedWorldDate date = {(uint16_t)y, (uint8_t)m, (uint8_t)d}, back;
    int32_t n = -1;
    assert(ExpandedWorld_DateToIndex(date, &n));
    assert(ExpandedWorld_IndexToDate(n, &back));
    assert(back.year == y && back.month == m && back.day == d);
}
int main(void) {
    int32_t a, b;
    ExpandedWorldDate aug = {1996,8,1}, leap = {2000,2,29};
    ExpandedWorldDate future = {2026,8,1}, custom = {2034,2,15};
    ExpandedWorldDate back;
    ExpandedWorldClock c;
    uint32_t week;
    assert(ExpandedWorld_DateToIndex(aug, &a));
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){1996,8,8}, &b));
    assert(b - a == 7 && ExpandedWorld_Weekday(a) == 3);
    assert(!ExpandedWorld_DateToIndex((ExpandedWorldDate){1900,2,29}, &a));
    assert(!ExpandedWorld_DateToIndex((ExpandedWorldDate){2100,2,29}, &a));
    assert(!ExpandedWorld_DateToIndex((ExpandedWorldDate){2026,13,1}, &a));
    /* Approved European manager start date: July 1. */
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){1996,7,1}, &a));
    assert(ExpandedWorld_Weekday(a)==0); /* Monday */
    assert(ExpandedWorld_DateAfterWeeks((ExpandedWorldDate){1996,7,1},23,&back));
    assert(back.year==1996 && back.month==12 && back.day==9);
    roundtrip(1900,1,1); roundtrip(1996,8,1); roundtrip(1999,12,31);
    roundtrip(2000,2,29); roundtrip(2000,3,1); roundtrip(2099,12,31);
    roundtrip(2100,3,1); roundtrip(2200,12,31);
    assert(ExpandedWorld_DateToIndex(leap, &a));
    assert(ExpandedWorld_Weekday(a) == 1);
    assert(ExpandedWorld_DateAfterWeeks(future, 0, &back));
    assert(back.year == 2026 && back.month == 8 && back.day == 1);
    assert(ExpandedWorld_DateAfterWeeks(future, 52, &back));
    assert(back.year == 2027 && back.month == 7 && back.day == 31);
    assert(ExpandedWorld_SeasonWeek(future, future, &week) && week == 1);
    assert(ExpandedWorld_SeasonWeek(future, back, &week) && week == 53);
    assert(ExpandedWorld_DateAfterWeeks(custom, 2, &back));
    assert(back.year == 2034 && back.month == 3 && back.day == 1);
    assert(!ExpandedWorld_SeasonWeek(back, custom, &week));
    assert(!ExpandedWorld_DateAfterWeeks(future, UINT32_MAX, &back));
    assert(ExpandedWorld_Init(&c, aug) && ExpandedWorld_AddDays(&c, 7));
    assert(ExpandedWorld_IndexToDate(c.day_index, &back));
    assert(back.year == 1996 && back.month == 8 && back.day == 8);
    assert(!ExpandedWorld_AddDays(&c, -1));
    assert(!ExpandedWorld_AdvanceTo(&c, c.day_index - 1));
    assert(!ExpandedWorld_AddDays(&c, INT32_MAX));
    /* Regression: UI showed THU 28 AUG 1997 for a 1996/97
     * season with native sortKey 56 (mistakenly adding 56 weeks). */
    assert(ExpandedWorld_NativePhaseToDate(1996,56,&back));
    assert(back.year==1996 && back.month==8 && back.day==1);
    assert(ExpandedWorld_NativePhaseToDate(1996,63,&back));
    assert(back.year==1996 && back.month==8 && back.day==28);
    assert(ExpandedWorld_NativePhaseToDate(1996,64,&back));
    assert(back.year==1996 && back.month==9 && back.day==1);
    assert(ExpandedWorld_NativePhaseToDate(1996,88,&back));
    assert(back.year==1996 && back.month==12 && back.day==1);
    assert(ExpandedWorld_NativePhaseToDate(1996,96,&back));
    assert(back.year==1997 && back.month==1 && back.day==1);
    assert(ExpandedWorld_NativePhaseToDate(1996,112,&back));
    assert(back.year==1997 && back.month==3 && back.day==1);
    assert(ExpandedWorld_NativePhaseToDate(2026,56,&back));
    assert(back.year==2026 && back.month==8 && back.day==1);
    assert(!ExpandedWorld_NativePhaseToDate(1996,56,NULL));
    puts("OK: calendar dates, season weeks, leap years, monotonic advances");
    return 0;
}
