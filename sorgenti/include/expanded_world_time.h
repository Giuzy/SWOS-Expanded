#ifndef EXPANDED_WORLD_TIME_H
#define EXPANDED_WORLD_TIME_H
/* Phase 1: Gregorian calendar independent of SWOS match/finance ticks.
 * Start dates (year, month and day) are provided by the caller, not fixed to 1996.
 * Persistent world event processing will be integrated separately.
 */
#include "swos_types.h"
typedef struct ExpandedWorldDate {
    uint16_t year;   /* 1900-2200 */
    uint8_t month;   /* 1-12 */
    uint8_t day;
} ExpandedWorldDate;
typedef struct ExpandedWorldClock {
    int32_t day_index; /* 0 = 1900-01-01 */
} ExpandedWorldClock;
int ExpandedWorld_DateToIndex(ExpandedWorldDate date, int32_t *out_index);
int ExpandedWorld_IndexToDate(int32_t day_index, ExpandedWorldDate *out_date);
int ExpandedWorld_Init(ExpandedWorldClock *clock, ExpandedWorldDate start);
int ExpandedWorld_AdvanceTo(ExpandedWorldClock *clock, int32_t target_day_index);
int ExpandedWorld_AddDays(ExpandedWorldClock *clock, int32_t days);
int ExpandedWorld_Weekday(int32_t day_index); /* Monday 0, Sunday 6 */
int ExpandedWorld_SeasonWeek(ExpandedWorldDate start, ExpandedWorldDate now,
                             uint32_t *out_week); /* 1-based, not ISO */
int ExpandedWorld_DateAfterWeeks(ExpandedWorldDate start, uint32_t elapsed_weeks,
                                  ExpandedWorldDate *out_date);
/* Native OpenSWOS sortKey: calendar month / 8 (+0x60 at year rollover).
 * Example: 56 -> 1 Aug 1996, 63 -> 28 Aug 1996, 96 -> 1 Jan 1997. */
int ExpandedWorld_NativePhaseToDate(uint16_t season_year, uint8_t native_key,
                                    ExpandedWorldDate *out_date);
#endif
