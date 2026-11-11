#ifndef EXPANDED_WORLD_LIVE_H
#define EXPANDED_WORLD_LIVE_H
#include "swos_types.h"
/* Standalone competition preview: actual club IDs and squads, provisional
 * round-robin dates. No changes to native match physics or save schema. */
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
int ExpandedWorldLive_Begin(uint16_t start_year);
int ExpandedWorldLive_AddClub(uint8_t country, uint8_t division,
                              uint16_t club, uint16_t rating,
                              const char *name, uint8_t points_per_win);
int ExpandedWorldLive_Seal(void);
int ExpandedWorldLive_AdvanceWeeks(uint32_t weeks);
int ExpandedWorldLive_Table(uint8_t country, uint8_t division,
                            ExpandedLiveRow *rows, uint32_t capacity,
                            uint32_t *total);
uint32_t ExpandedWorldLive_Results(void);
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
void ExpandedWorldLive_LoadOfferCountries(void);
#endif
