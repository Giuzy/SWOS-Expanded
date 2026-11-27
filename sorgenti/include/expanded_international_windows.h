#ifndef EXPANDED_INTERNATIONAL_WINDOWS_H
#define EXPANDED_INTERNATIONAL_WINDOWS_H
/* v00.13.354 international windows (criteria 23/24/25), WORLD 2627 only.
 * ONE global table (data_json/worlds/2627/competitions/international_windows.json,
 * FIFA Men's International Match Calendar 2023-2030, April 2026 edition)
 * read by every path that dates league rounds: START catalog and CE
 * editions. Official dates only for 2026/27; from "generated_from" the
 * calendar generates every window itself (two-year World Cup / EURO cycle).
 * A competition pauses ONLY for the window types it declares
 * (league_calendar.json "international_breaks", CE phase window
 * "international_breaks"). */
#include <stdint.h>
#include "expanded_universal_date_planner.h"

enum {
    EXP_INTL_FIFA     = 1u << 0,   /* FIFA windows, all confederations */
    EXP_INTL_AFC      = 1u << 1,   /* confederation final tournaments */
    EXP_INTL_CAF      = 1u << 2,
    EXP_INTL_CONCACAF = 1u << 3,
    EXP_INTL_CONMEBOL = 1u << 4,
    EXP_INTL_OFC      = 1u << 5,
    EXP_INTL_UEFA     = 1u << 6,
    EXP_INTL_INHERIT  = 1u << 7    /* CE phase window: not declared, inherit (league) or none */
};
#define EXP_INTL_MAX 64u
typedef struct ExpIntlWindow {
    int32_t first_day, last_day;   /* inclusive civil day indexes */
    uint8_t type;                  /* one EXP_INTL_* bit */
} ExpIntlWindow;

/* "FIFA,UEFA" -> mask. 1 ok (empty string = 0), 0 unknown name. */
int ExpIntl_ParseMask(const char *text, uint8_t *mask);
/* Load / reset the global table (2627). Missing file = empty table (1). */
int ExpIntl_Load(void);
int ExpIntl_LoadJson(const char *json, int len);
void ExpIntl_Reset(void);
uint32_t ExpIntl_Count(void);
/* last civil day covered by the official table (0 = empty) */
int32_t ExpIntl_LastCoveredDay(void);
/* first civil day whose windows the calendar GENERATES (World Cup / EURO
 * two-year cycle, see expanded_international_windows.c); 0 = none */
int32_t ExpIntl_GeneratedFrom(void);
/* Windows of the types in `mask` overlapping [first,last], in order.
 * Returns the number written (<= cap). */
uint32_t ExpIntl_Overlapping(uint8_t mask, int32_t first, int32_t last,
                             ExpIntlWindow *out, uint32_t cap);
/* Plan the rounds of a window that stops for the international windows of
 * `mask` (pol already holds the competition's own breaks). The windows are
 * added as breaks; when the rounds no longer fit, the games move AFTER the
 * stops: the window end is extended by the stopped days, repeatedly (max 4
 * times, max 120 days). *extended = days added. pol keeps the final breaks
 * (the day profile must respect them). 1 planned, 0 impossible. */
int ExpIntl_PlanWithWindows(ExpandedDatePlanPolicy *pol, uint8_t mask,
                            uint32_t rounds, int32_t *days, int32_t *extended);
#endif
