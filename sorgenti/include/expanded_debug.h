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
#endif
#endif
