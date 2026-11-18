#ifndef EXPANDED_DEBUG_H
#define EXPANDED_DEBUG_H
/* Development test toggles. Not saved and do not alter match physics. */
#include "swos_types.h"
int ExpandedDebug_ResultMode(void); /* 0 normal, 1 force win, 2 force loss, 4 force draw */
void ExpandedDebug_ToggleWin(void);
void ExpandedDebug_ToggleLose(void);
void ExpandedDebug_ToggleDraw(void);
void ExpandedDebug_ToggleSack(void);
int ExpandedDebug_SackArmed(void);
int ExpandedDebug_ConsumeSack(void);
void ExpandedDebug_ForceScore(int career_home, uint16_t *home, uint16_t *away);

/* Read-only counters from the genuine resident OpenSWOS competition states.
 * These functions NEVER initialize, simulate or advance a competition. */
#ifdef SWOS_SDL_BUILD
typedef struct {
    uint32_t resident_leagues, leagues_with_matches, completed_leagues;
    uint32_t native_matches, countries, invalid_contexts, club_bound;
    uint16_t min_year, max_year;
} ExpandedOGWorldAudit;
typedef struct {
    uint32_t domestic_definitions, domestic_countries;
    uint32_t resident_cups, completed_year_callbacks;
} ExpandedOGCupAudit;
void Expanded_OGWorld_Audit(ExpandedOGWorldAudit *out);
void Expanded_OGWorld_CupAudit(ExpandedOGCupAudit *out);
/* F12 single-page world coverage: each regional ratio counts COUNTRIES whose
 * complete OG league/cup set is resident in the current season. Match progress
 * counts countries with OG results; totals are original OG match counters.
 * International/national ratios count COMPETITIONS, not countries. */
#define EXP_OG_DEBUG_REGIONS 6
typedef struct {
    uint32_t leagueExpected[EXP_OG_DEBUG_REGIONS];
    uint32_t leagueResident[EXP_OG_DEBUG_REGIONS];
    uint32_t leagueAdvanced[EXP_OG_DEBUG_REGIONS];
    uint32_t leagueMatches[EXP_OG_DEBUG_REGIONS];
    uint32_t cupExpected[EXP_OG_DEBUG_REGIONS];
    uint32_t cupResident[EXP_OG_DEBUG_REGIONS];
    uint32_t cupAdvanced[EXP_OG_DEBUG_REGIONS];
    uint32_t cupMatches[EXP_OG_DEBUG_REGIONS];
    uint32_t internationalExpected,internationalResident;
    uint32_t internationalAdvanced,internationalMatches;
    uint32_t nationalExpected,nationalResident;
    uint32_t nationalAdvanced,nationalMatches;
    /* F12 verifies native phase progress and aggregate standings, read-only.
     * This does NOT prove the results of individual fixtures. */
    uint32_t leagueInPhase[EXP_OG_DEBUG_REGIONS];
    uint32_t cupInPhase[EXP_OG_DEBUG_REGIONS];
    uint32_t leagueIssues[EXP_OG_DEBUG_REGIONS];
    uint32_t internationalInPhase;
    /* Genuine per-division coverage; Brazil's state tournaments are NOT
     * included in country-level national league denominators. */
    uint32_t divisionsExpected[EXP_OG_DEBUG_REGIONS];
    uint32_t divisionsResident[EXP_OG_DEBUG_REGIONS];
    uint32_t divisionsInPhase[EXP_OG_DEBUG_REGIONS];
    uint32_t statesExpected,statesResident,statesInPhase,statesMatches;
    /* Each UEFA title is checked separately. In 1996 slots 1 and 2 are
     * Cup Winners' Cup and UEFA Cup, not modern Europa/Conference. */
    uint32_t uefaExpected,uefaResident[3],uefaInPhase[3],uefaMatches[3];
    uint32_t otherExpected,otherResident,otherInPhase,otherMatches;
    /* Aggregate OG standings invariants only, never fixture-by-fixture
     * certification where a date-specific ledger is unavailable. */
    uint32_t resultsChecked,resultsIssues,fixturesUnavailable;
    uint16_t worldYear;
    uint8_t ready,phaseReady,nativePhase;
} ExpandedOGDashboard;
void Expanded_OGWorld_Dashboard(ExpandedOGDashboard *out);
#include "expanded_f12_compare.h"
typedef struct {
    ExpandedOGDashboard before, after;
    ExpandedF12Diff diff;
    uint8_t beforeReady,afterReady;
} ExpandedOGHireAudit;
/* 0=just before a contract binds a manager, 1=after the bind.
 * Snapshots are read-only and transient; they never enter .CAR files. */
void Expanded_OGWorld_DebugHireCheckpoint(uint8_t after);
void Expanded_OGWorld_DebugHireAudit(ExpandedOGHireAudit *out);
/* Per-player goal counters in original resident OG league team files.
 * Not a second top scorers table: READ-ONLY inspector. */
typedef struct {
    uint32_t resident_leagues, players, players_with_league_goals;
    uint32_t player_league_goals, standings_goals, leagues_with_goals;
    /* Audit ONLY: totals are not necessarily the same scoring domain. */
    uint32_t leagues_with_delta, player_goals_greater, standings_goals_greater;
} ExpandedOGScorerAudit;
void Expanded_OGWorld_ScorerAudit(ExpandedOGScorerAudit *out);
void Expanded_OGWorld_LogCheckpoint(const char *stage);
/* Native world-resident OG national cups: never a second results engine. */
int Expanded_OGWorld_IsSimulating(void);
uint16_t Expanded_OGWorld_CurrentCupYear(void);
void Expanded_OGWorld_CupMarkPlayed(void);
uint32_t Expanded_OGWorld_CupCount(void);
uint32_t Expanded_OGWorld_CupMatches(void);
uint32_t Expanded_OGWorld_CupUnsupported(void);
uint32_t Expanded_OGWorld_CupFaults(void);
int Expanded_OGWorld_AdvanceCup(uint8_t *record,uint8_t country,
                                uint8_t slot,uint16_t year,uint8_t phase);
#endif
#endif
