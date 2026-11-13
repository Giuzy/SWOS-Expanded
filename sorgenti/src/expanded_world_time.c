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
    if(!out_date || native_key==255u || year>2200u) return 0;
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

/* Inverse of the canonical first-day mapping, without changing OG's
 * eight-phase month or rounding the authoritative civil date. */
static int native_phase_from_day(uint16_t season_year, ExpandedWorldDate day,
                                 uint8_t *out_phase, int next) {
    int32_t month_start=0,following=0;
    ExpandedWorldDate first,next_month,canonical;
    if (!out_phase || day.year<season_year ||
        day.year>season_year+2u ||
        !ExpandedWorld_DateToIndex(day,&month_start)) return 0;
    first=(ExpandedWorldDate){day.year,day.month,1u};
    next_month=(ExpandedWorldDate){
        (uint16_t)(day.year+(day.month==12u)),
        (uint8_t)(day.month==12u?1u:day.month+1u),1u};
    if (!ExpandedWorld_DateToIndex(first,&month_start) ||
        !ExpandedWorld_DateToIndex(next_month,&following) ||
        following<=month_start) return 0;
    const unsigned length=(unsigned)(following-month_start);
    const unsigned segment=((unsigned)day.day*8u-1u)/length;
    unsigned key=(unsigned)(day.year-season_year)*96u+
                 ((unsigned)day.month-1u)*8u+segment;
    if (key>=255u) return 0; /* OG 0xff marks no valid event. */
    if (next) {
        if (!ExpandedWorld_NativePhaseToDate(season_year,(uint8_t)key,
                                              &canonical)) return 0;
        if (canonical.day<day.day) ++key;
        if (key>=255u) return 0;
    }
    *out_phase=(uint8_t)key;
    return 1;
}
int ExpandedWorld_DateToNativePhaseFloor(uint16_t season_year,
                                         ExpandedWorldDate date,
                                         uint8_t *out_phase) {
    return native_phase_from_day(season_year,date,out_phase,0);
}
int ExpandedWorld_DateToNativePhaseNext(uint16_t season_year,
                                        ExpandedWorldDate date,
                                        uint8_t *out_phase) {
    return native_phase_from_day(season_year,date,out_phase,1);
}
/* Calendar-anniversary jump, never a guessed 52 weeks/year.
 * Feb 29 in a non-leap destination clamps to Feb 28 by definition. */
int ExpandedWorld_AddCalendarYears(ExpandedWorldDate date,uint16_t years,
                                    ExpandedWorldDate *out_date) {
    int32_t source_index=0;
    if (!out_date || !ExpandedWorld_DateToIndex(date,&source_index) ||
        (uint32_t)date.year+(uint32_t)years>2200u) return 0;
    ExpandedWorldDate result=date;
    result.year=(uint16_t)(date.year+years);
    if (result.month==2u && result.day==29u &&
        month_days(result.year,2u)==28u) result.day=28u;
    int32_t index=0;
    if (!ExpandedWorld_DateToIndex(result,&index)) return 0;
    *out_date=result;
    return 1;
}
