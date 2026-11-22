/* The Competition Engine imports declared competition structure directly from
 * data_json/worlds/2627/competitions/leagues.json. No OG pairing/seed order. */
#ifndef EXPANDED_UNIVERSAL_JSON_LEAGUES_H
#define EXPANDED_UNIVERSAL_JSON_LEAGUES_H
#include <stdint.h>
typedef struct ExpandedUniversalJsonLeague {
    uint8_t country, comp_id, division, begin_phase, end_phase, meetings;
    uint16_t declared_clubs;
} ExpandedUniversalJsonLeague;
int ExpandedUniversalJsonLeagues_Load(void);
void ExpandedUniversalJsonLeagues_Reset(void);
int ExpandedUniversalJsonLeagues_Find(uint8_t country, uint8_t comp_id,
    uint8_t division, ExpandedUniversalJsonLeague *out);
uint32_t ExpandedUniversalJsonLeagues_Count(void);
#endif
