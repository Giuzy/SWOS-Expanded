#ifndef EXPANDED_WORLD_LIVE_H
#define EXPANDED_WORLD_LIVE_H
#include "swos_types.h"
/* Standalone competition preview: actual club IDs and squads, provisional
 * round-robin dates. No changes to native match physics or save schema. */
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
uint32_t ExpandedWorldLive_Clubs(void);
uint32_t ExpandedWorldLive_Leagues(void);
uint16_t ExpandedWorldLive_SeasonYear(void);
uint32_t ExpandedWorldLive_Weeks(void);
void ExpandedWorldLive_End(void);
void ExpandedWorldLive_LoadOfferCountries(void);
#endif
