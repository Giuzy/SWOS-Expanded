#include "expanded_world_time.h"
#define WORLD_FIRST_YEAR 1900u
#define WORLD_LAST_YEAR  2200u
static int leap(uint16_t y) {
    return y % 4u == 0u && (y % 100u != 0u || y % 400u == 0u);
}
static uint8_t month_days(uint16_t y, uint8_t month) {
    static const uint8_t lengths[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month < 1u || month > 12u) return 0;
    return (uint8_t)(lengths[month - 1u] + (month == 2u && leap(y)));
}
int ExpandedWorld_DateToIndex(ExpandedWorldDate date, int32_t *out_index) {
    int32_t total = 0;
    if (!out_index || date.year < WORLD_FIRST_YEAR || date.year > WORLD_LAST_YEAR ||
        date.day < 1u || date.day > month_days(date.year, date.month)) return 0;
    for (uint16_t y = WORLD_FIRST_YEAR; y < date.year; ++y)
        total += leap(y) ? 366 : 365;
    for (uint8_t m = 1u; m < date.month; ++m)
        total += month_days(date.year, m);
    *out_index = total + (int32_t)date.day - 1;
    return 1;
}
int ExpandedWorld_IndexToDate(int32_t day_index, ExpandedWorldDate *out_date) {
    int32_t remaining = day_index;
    uint16_t year = WORLD_FIRST_YEAR;
    uint8_t month = 1u;
    if (!out_date || remaining < 0) return 0;
    while (year <= WORLD_LAST_YEAR) {
        int days = leap(year) ? 366 : 365;
        if (remaining < days) break;
        remaining -= days;
        ++year;
    }
    if (year > WORLD_LAST_YEAR) return 0;
    for (;;) {
        int days = month_days(year, month);
        if (remaining < days) break;
        remaining -= days;
        ++month;
    }
    out_date->year = year;
    out_date->month = month;
    out_date->day = (uint8_t)(remaining + 1);
    return 1;
}
int ExpandedWorld_Init(ExpandedWorldClock *clock, ExpandedWorldDate start) {
    if (!clock) return 0;
    return ExpandedWorld_DateToIndex(start, &clock->day_index);
}
int ExpandedWorld_AdvanceTo(ExpandedWorldClock *clock, int32_t target_day_index) {
    ExpandedWorldDate check;
    if (!clock || target_day_index < clock->day_index ||
        !ExpandedWorld_IndexToDate(target_day_index, &check)) return 0;
    clock->day_index = target_day_index;
    return 1;
}
int ExpandedWorld_AddDays(ExpandedWorldClock *clock, int32_t days) {
    if (!clock || days < 0) return 0;
    int64_t target = (int64_t)clock->day_index + days;
    if (target > INT32_MAX) return 0;
    return ExpandedWorld_AdvanceTo(clock, (int32_t)target);
}
int ExpandedWorld_Weekday(int32_t day_index) {
    ExpandedWorldDate check;
    if (!ExpandedWorld_IndexToDate(day_index, &check)) return -1;
    return day_index % 7; /* 1900-01-01 was Monday */
}
int ExpandedWorld_SeasonWeek(ExpandedWorldDate start, ExpandedWorldDate now,
                             uint32_t *out_week) {
    int32_t a, b;
    if (!out_week || !ExpandedWorld_DateToIndex(start, &a) ||
        !ExpandedWorld_DateToIndex(now, &b) || b < a) return 0;
    *out_week = (uint32_t)((b - a) / 7 + 1);
    return 1;
}
int ExpandedWorld_DateAfterWeeks(ExpandedWorldDate start, uint32_t elapsed_weeks,
                                  ExpandedWorldDate *out_date) {
    int32_t start_index;
    uint64_t target;
    if (!out_date || !ExpandedWorld_DateToIndex(start, &start_index)) return 0;
    target = (uint64_t)(uint32_t)start_index + (uint64_t)elapsed_weeks * 7u;
    if (target > INT32_MAX) return 0;
    return ExpandedWorld_IndexToDate((int32_t)target, out_date);
}

/* OpenSWOS native competition keys count 1/8 of a calendar month,
 * NOT weeks since August. A byte key advances by +0x60 every January:
 * 56=1 Aug in the season's first year, 96=1 Jan next year.
 * The original formatter treated a key of 56 as 56 elapsed weeks and
 * displayed AUG of the FOLLOWING year (user report 11 Nov 2026). */
int ExpandedWorld_NativePhaseToDate(uint16_t season_year, uint8_t native_key,
                                    ExpandedWorldDate *out_date) {
    uint32_t year=(uint32_t)season_year+(uint32_t)native_key/96u;
    unsigned month=((unsigned)native_key%96u)/8u+1u;
    unsigned eighth=(unsigned)native_key%8u;
    ExpandedWorldDate start,next;
    int32_t start_day=0,next_day=0;
    if(!out_date || year>2200u) return 0;
    start=(ExpandedWorldDate){(uint16_t)year,(uint8_t)month,1};
    next=(ExpandedWorldDate){(uint16_t)(year+(month==12u)),
             (uint8_t)(month==12u?1u:month+1u),1};
    if(!ExpandedWorld_DateToIndex(start,&start_day) ||
       !ExpandedWorld_DateToIndex(next,&next_day) ||
       next_day<=start_day) return 0;
    start.day=(uint8_t)(1u+eighth*(unsigned)(next_day-start_day)/8u);
    *out_date=start;
    return 1;
}
