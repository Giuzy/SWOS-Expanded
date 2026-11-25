/* The Competition Engine imports declared competition structure directly from
 * data_json/worlds/2627/competitions/leagues.json. No OG pairing/seed order.
 * v00.13.280: round DATES come from league_calendar.json (CE date planner),
 * never from OG native phases. begin_phase/end_phase stay only as the
 * declared JSON structure used by the NATIVE_RULE_DIFF diagnostic. */
#ifndef EXPANDED_UNIVERSAL_JSON_LEAGUES_H
#define EXPANDED_UNIVERSAL_JSON_LEAGUES_H
#include <stdint.h>
#include "expanded_universal_pairings.h"
#define EXP_JSON_LEAGUE_MAX_BREAKS 8u
typedef struct ExpandedUniversalJsonLeagueBreak {
    uint8_t from_month, from_day, to_month, to_day;
} ExpandedUniversalJsonLeagueBreak;
typedef struct ExpandedUniversalJsonLeague {
    uint8_t country, comp_id, division, begin_phase, end_phase, meetings;
    uint16_t declared_clubs;
    /* CE calendar window (league_calendar.json); has_calendar==0 means the
     * league has no date source and must fail closed. */
    uint8_t has_calendar;
    uint8_t first_month, first_day, last_month, last_day, last_year_offset;
    uint8_t match_weekdays, extra_weekdays, min_rest_days, break_count;
    ExpandedUniversalJsonLeagueBreak breaks[EXP_JSON_LEAGUE_MAX_BREAKS];
    /* Fase 2: OG native rules of the league record (leagues.json, exported
     * from the native CompetitionLeague): points for a win, substitutions
     * allowed (hi nibble of subsAndMaxSubs), bench (lo nibble), and the
     * division's promotion/relegation counts. has_rules==0 = not declared. */
    uint8_t has_rules, points_for_win, subs, bench;
    uint8_t divisions, promoted, relegated, promoted_po, relegated_po;
    ExpPairExtras extras;
} ExpandedUniversalJsonLeague;
/* i-th division row (Fase 2 definitions builder). */
int ExpandedUniversalJsonLeagues_At(uint32_t i, ExpandedUniversalJsonLeague *out);
int ExpandedUniversalJsonLeagues_Load(void);
void ExpandedUniversalJsonLeagues_Reset(void);
int ExpandedUniversalJsonLeagues_Find(uint8_t country, uint8_t comp_id,
    uint8_t division, ExpandedUniversalJsonLeague *out);
uint32_t ExpandedUniversalJsonLeagues_Count(void);
/* Pure parsers shared with tests: "MM-DD" and "SAT,SUN" (bit0=MON). */
int ExpandedUniversalJsonLeagues_ParseMonthDay(const char *text,
    uint8_t *month, uint8_t *day);
int ExpandedUniversalJsonLeagues_ParseWeekdays(const char *text,
    uint8_t *mask);
#endif
