#ifndef EXPANDED_WORLD_LIVE_H
#define EXPANDED_WORLD_LIVE_H
#include "swos_types.h"
#include "expanded_world_time.h"
/* World competition ledger: real club IDs and original native SWOS
 * league fixture tables/month phases. Native career replays the same
 * results into the original standings/bookkeeping routines.
 * Full world saves and post-hire manager switches remain work in progress. */
typedef struct ExpandedLivePreseason {
    uint16_t friendlies,goals_for,goals_against;
    uint8_t morale,fatigue,injury_events;
    /* Provisional balance units in the pilot, NOT native finances. */
    int32_t net_income;
} ExpandedLivePreseason;

typedef struct ExpandedLiveRow {
    uint16_t club;
    char name[18];
    uint16_t played, wins, drawn, lost, scored, conceded;
    int32_t points;
} ExpandedLiveRow;
/* One chronological world clock and one source of league events.
 * No new saved fields: the snapshot already persists s_ledger.now.
 * next_* is the first unplayed native-table fixture, not an OG cup event. */
typedef struct ExpandedLiveCalendarState {
    ExpandedWorldDate today, next_date;
    uint16_t next_home, next_away;
    uint32_t pending, overdue, next_matches;
    uint8_t has_next;
} ExpandedLiveCalendarState;
/* Daily API also handles mid-week dates; the weekly entry point is a wrapper.
 * Neither entry point changes original 50 Hz match ticks or OG cup handlers. */
int ExpandedWorldLive_AdvanceToDate(ExpandedWorldDate date);
/* Read-only audit/status. A non-zero overdue count indicates missing processing. */
int ExpandedWorldLive_CalendarState(ExpandedLiveCalendarState *out);
/* Return source-file IDs from leagues actually scheduled by this world.
 * Offer discovery is global and independent of the human's nationality. */
uint32_t ExpandedWorldLive_OfferFiles(uint8_t *files,uint32_t capacity);
/* OG knockout engine reports completed real cup matches and champions.
 * The Living World records, archives and persists them chronologically;
 * it does not simulate a second bracket or replace the native callbacks. */
int ExpandedWorldLive_RecordNativeCupMatch(uint16_t season,const char *name,
             uint8_t slot,uint8_t phase,uint8_t round,uint8_t match,
             uint8_t leg,uint16_t home,uint16_t away,
             uint8_t goals_home,uint8_t goals_away,uint8_t flags);
int ExpandedWorldLive_RecordNativeCupChampion(uint16_t season,const char *name,
             uint8_t slot,uint8_t phase,uint16_t winner);
uint32_t ExpandedWorldLive_CupResidentMatches(void);
uint32_t ExpandedWorldLive_CupResidentTitles(void);
uint32_t ExpandedWorldLive_CupArchivedMatches(void);
uint32_t ExpandedWorldLive_CupArchivedTitles(void);
/* Permanent original OG cup champions, across two-year fixture rollovers.
 * Return false for years absent from old snapshots; never invent winners. */
uint32_t ExpandedWorldLive_CupHistoricalTitles(void);
int ExpandedWorldLive_CupHistoryChampion(uint16_t season,const char *name,
                                          uint8_t slot,uint16_t *winner);
int ExpandedWorldLive_Begin(uint16_t start_year);
int ExpandedWorldLive_AddClub(uint8_t country, uint8_t division,
                              uint16_t club, uint16_t rating,
                              const char *name, uint8_t points_per_win);
/* The world uses original OpenSWOS league fixture tables and phase
 * calendar. Called before Seal(), from each country's competition record. */
/* Automatic native league places only; playoff places remain resolved by OG.
 * Counts come directly from CompetitionLeague.divisions. The world never
 * invents promotion-playoff winners or a separate competition bracket. */
int ExpandedWorldLive_SetLeagueMovement(uint8_t country,uint8_t division,
                                        uint8_t auto_up,uint8_t auto_down,
                                        uint8_t playoff_up,uint8_t playoff_down);
/* Count actual club division changes between generated world seasons. */
uint32_t ExpandedWorldLive_Promotions(void);
uint32_t ExpandedWorldLive_Relegations(void);
uint32_t ExpandedWorldLive_MobilityUnsupported(void);
uint8_t ExpandedWorldLive_ClubDivision(uint16_t club);
int ExpandedWorldLive_SetLeagueSchedule(uint8_t country,uint8_t division,
                                        uint8_t begin_phase,uint8_t end_phase,
                                        uint8_t legs);
/* During native league creation, use the same club-to-fixture-index order
 * as the canonical world ledger, but only for an exactly matching roster.
 * Other competitions use the original OpenSWOS shuffle unchanged. */
int ExpandedWorldLive_NativeOrder(const uint16_t *native_ids,uint32_t count,
                                  uint8_t *out_order);
uint32_t ExpandedWorldLive_NativeOrderApplied(void);
uint32_t ExpandedWorldLive_NativeScheduleLeagues(void);
int ExpandedWorldLive_Seal(void);
int ExpandedWorldLive_AdvanceWeeks(uint32_t weeks);
/* Anchor a stable native OpenSWOS career season to the same world clock.
 * Called at OG season transition, not during a live 50 Hz match. */
int ExpandedWorldLive_AdvanceToSeason(uint16_t season_year);
int ExpandedWorldLive_Table(uint8_t country, uint8_t division,
                            ExpandedLiveRow *rows, uint32_t capacity,
                            uint32_t *total);
/* A PREHIRE pilot league result can be replayed once for the exact
 * same home/away clubs within the same season, even though the two
 * schedules assign that pairing to different calendar dates.
 * Only results already played before signing are eligible. Cups excluded.
 * Does not alter either fixture calendar or guarantee matching tables. */
/* Read the same world result without replay side effects. Used by the
 * direct OG league-state import at manager hire. */
int ExpandedWorldLive_PeekNativeLeagueResult(uint16_t home,uint16_t away,
                                            uint8_t *goals_home,
                                            uint8_t *goals_away);
int ExpandedWorldLive_TakeNativeLeagueResult(uint16_t home,uint16_t away,
                                             uint8_t native_phase,
                                             uint8_t *goals_home,
                                             uint8_t *goals_away);
/* After the manager joins, publish an actual native league score into
 * the same world result ledger, preserving OpenSWOS match physics. Only
 * an identical fixture in the original month phase can be updated. */
int ExpandedWorldLive_RecordNativeLeagueResult(uint16_t home,uint16_t away,
                                               uint8_t phase,
                                               uint8_t goals_home,
                                               uint8_t goals_away);
uint32_t ExpandedWorldLive_NativeMatches(void);
uint32_t ExpandedWorldLive_NativeMisses(void);
uint32_t ExpandedWorldLive_Results(void);
/* Resident window remains bounded to two seasons. Completed native league
 * titles and their match-count/digest are stored as compact, immutable
 * summaries in the SAME career save (world snapshot v3). */
uint32_t ExpandedWorldLive_ArchivedResults(void);
uint32_t ExpandedWorldLive_ResidentResults(void);
uint32_t ExpandedWorldLive_ArchivedLeagues(void);
uint16_t ExpandedWorldLive_WindowFirstYear(void);
int ExpandedWorldLive_HistoryChampion(uint16_t year,uint8_t country,
                                      uint8_t division,uint16_t *champion);
/* Diagnostic totals: player slots refer to 16 records per imported club;
 * MemoryBytes reports pilot RAM, not installed database file size. */
uint32_t ExpandedWorldLive_Fixtures(void);
uint32_t ExpandedWorldLive_PlayerSlots(void);
uint32_t ExpandedWorldLive_MemoryBytes(void);
uint32_t ExpandedWorldLive_Clubs(void);
uint32_t ExpandedWorldLive_Leagues(void);
uint16_t ExpandedWorldLive_SeasonYear(void);
uint32_t ExpandedWorldLive_Weeks(void);
uint32_t ExpandedWorldLive_PreseasonGamesTotal(void);
int ExpandedWorldLive_Preseason(uint16_t club,ExpandedLivePreseason *out);
void ExpandedWorldLive_End(void);
/* Actual source-data coverage from the Windows career-country loader.
 * These counts describe the real binary TEAM.* files; standalone C
 * stress tests deliberately use synthetic teams and cannot fill them. */
typedef struct ExpandedLiveImportAudit {
    uint32_t candidate_countries;
    uint32_t supported_countries;
    uint32_t skipped_country_records;
    uint32_t unsupported_rounds;
    uint32_t team_files_attempted;
    uint32_t team_files_with_clubs;
    uint32_t team_files_without_clubs;
    uint32_t imported_clubs;
    uint32_t scheduled_leagues;
} ExpandedLiveImportAudit;
int ExpandedWorldLive_ImportAudit(ExpandedLiveImportAudit *out);
void ExpandedWorldLive_LoadOfferCountries(void);
#ifdef SWOS_SDL_BUILD
/* Structured, versioned .CAR snapshot on SDL hosts only. */
int ExpandedWorldLive_WriteJson(int (*emit)(const char *,int,void *),void *ctx);
int ExpandedWorldLive_ReadJson(const char *json,int len);
#endif
#endif
