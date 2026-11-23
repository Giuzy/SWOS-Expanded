/* SWOS Expanded Competition Engine: civil-day date planner (v00.13.280).
 * Pure function: assigns one REAL civil day (any day of the Gregorian
 * calendar) to every round of a competition, from data only: a season
 * window, the preferred match weekdays, optional extra (midweek) weekdays,
 * a minimum rest gap and optional break ranges. It never reads OG native
 * phases, never ticks a clock, never simulates or saves anything.
 * Impossible requests fail closed (return 0) instead of inventing dates.
 */
#ifndef EXPANDED_UNIVERSAL_DATE_PLANNER_H
#define EXPANDED_UNIVERSAL_DATE_PLANNER_H
#include <stdint.h>

#define EXP_DATE_PLAN_MAX_BREAKS 8u

/* Weekday bits follow ExpandedWorld_Weekday(): bit0=Monday ... bit6=Sunday. */
typedef struct ExpandedDatePlanBreak {
    int32_t first_day, last_day;     /* inclusive civil day indexes */
} ExpandedDatePlanBreak;
typedef struct ExpandedDatePlanPolicy {
    int32_t first_day, last_day;     /* inclusive season window */
    uint8_t match_weekdays;          /* preferred days, must be non-zero */
    uint8_t extra_weekdays;          /* used ONLY when preferred are short */
    uint8_t min_rest_days;           /* minimum distance between two rounds */
    uint8_t break_count;
    ExpandedDatePlanBreak breaks[EXP_DATE_PLAN_MAX_BREAKS];
} ExpandedDatePlanPolicy;

/* Result reasons for diagnostics (0 = planned). */
enum {
    EXP_DATE_PLAN_OK = 0,
    EXP_DATE_PLAN_BAD_INPUT = 1,
    EXP_DATE_PLAN_NOT_ENOUGH_DAYS = 2,
    EXP_DATE_PLAN_REST_VIOLATION = 3
};

/* Fills out_days[0..rounds-1] with strictly increasing day indexes.
 * Returns 1 on success; on failure returns 0 and *out_reason (optional)
 * tells why. out_days is left unspecified on failure. */
int ExpandedDatePlanner_Plan(const ExpandedDatePlanPolicy *policy,
                             uint32_t rounds, int32_t *out_days,
                             int *out_reason);
#endif
