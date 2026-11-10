#ifndef EXPANDED_WORLD_LEDGER_H
#define EXPANDED_WORLD_LEDGER_H
/* Phase 1-2 core, NOT yet connected to live SWOS match fixtures.
 * The resolver will eventually use real OpenSWOS simulation. A ledger
 * neither loads clubs nor mutates the career's active fixture caches.
 */
#include "swos_types.h"
typedef struct ExpandedWorldFixture {
    int32_t day;         /* expanded_world_time day index */
    uint32_t id;         /* stable ID inside a competition and season */
    uint16_t competition;
    uint16_t home, away;
    uint8_t played;
    uint8_t goals_home, goals_away;
} ExpandedWorldFixture;
typedef struct ExpandedWorldLedger {
    ExpandedWorldFixture *fixtures;
    uint32_t count, capacity, played;
    int32_t now;
} ExpandedWorldLedger;
typedef int (*ExpandedWorldResolve)(const ExpandedWorldFixture *fixture,
                                    uint8_t *home_goals, uint8_t *away_goals,
                                    void *userdata);
int ExpandedWorldLedger_Init(ExpandedWorldLedger *ledger,
                             ExpandedWorldFixture *storage, uint32_t capacity,
                             int32_t start_day);
int ExpandedWorldLedger_Add(ExpandedWorldLedger *ledger, ExpandedWorldFixture fixture);
int ExpandedWorldLedger_Advance(ExpandedWorldLedger *ledger, int32_t target_day,
                                ExpandedWorldResolve resolve, void *userdata);
int ExpandedWorldLedger_Record(ExpandedWorldLedger *ledger, uint16_t competition,
                               uint32_t match_id, uint8_t home_goals,
                               uint8_t away_goals);
uint32_t ExpandedWorldLedger_NextPending(const ExpandedWorldLedger *ledger);
typedef struct ExpandedWorldStanding {
    uint16_t club;
    uint16_t played, wins, drawn, lost;
    uint16_t scored, conceded;
    int32_t points;
} ExpandedWorldStanding;
/* Clubs must be pre-listed. Points for a win are league-specific (2 or 3). */
int ExpandedWorldLedger_Standings(const ExpandedWorldLedger *ledger,
                                  uint16_t competition,
                                  ExpandedWorldStanding *table, uint32_t clubs,
                                  uint8_t points_for_win);
#endif
