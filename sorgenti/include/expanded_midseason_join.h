#ifndef EXPANDED_MIDSEASON_JOIN_H
#define EXPANDED_MIDSEASON_JOIN_H

#include "swos_types.h"

/* The official OpenSWOS career is initialized normally, then historical
 * fixtures are simulated through its OWN NEXT MATCH / VIEW RESULT callbacks.
 * This prototype is limited to an initial single-season employment. It does
 * not transplant the separately-generated pilot world schedule.
 */
void Expanded_MidseasonJoin_Begin(uint32_t unemployed_weeks);
int Expanded_MidseasonJoin_Active(void);
/* Fixture phase (month eighths) of the next unplayed native contest.
 * Separate from the finance accumulator, which tracks last charged phase. */
uint8_t Expanded_Career_NativeCalendarKey(void);
uint8_t Expanded_MidseasonJoin_TargetPhase(void);
/* 0 = no attempt; 1 = reached hire date; 2 = abort (see log). */
int Expanded_MidseasonJoin_LastResult(void);
int Expanded_MidseasonJoin_LastKey(void);
void Expanded_MidseasonJoin_Tick(void);
/* Per-join diagnostics: actions fired and historical PLAY actions
 * redirected to genuine native CPU match simulation. */
uint32_t Expanded_MidseasonJoin_Actions(void);
/* Number of completed native league screens exited during historical catch-up. */
uint32_t Expanded_MidseasonJoin_LeagueExits(void);
uint32_t Expanded_MidseasonJoin_ForcedSimulations(void);
void Expanded_MidseasonJoin_RecordForcedSimulation(void);

/* Pure calendar helper for unit testing, July 1 -> native August key.
 * 0 = unsupported elapsed period (>=52 weeks or invalid date).
 * On success, out_key is the first native week NOT to simulate.
 */
int Expanded_MidseasonJoin_Cutoff(uint16_t season_year,
                                 uint32_t weeks_since_july,
                                 uint8_t *out_key);
#endif
