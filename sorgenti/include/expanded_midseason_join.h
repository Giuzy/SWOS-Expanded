#ifndef EXPANDED_MIDSEASON_JOIN_H
#define EXPANDED_MIDSEASON_JOIN_H

#include "swos_types.h"

/* The native career is initialized normally; only the CURRENT world's
 * season is aligned and the OG league/cup fixtures before the offer are
 * imported or replayed through authentic callbacks. Archived years are
 * never replayed. World-wide offscreen OG cup state remains pending.
 */
/* Align fresh OG season counters to the live world's real season before
 * native fixture import. Does not replay archived previous years. */
int Expanded_MidseasonJoin_AlignSeason(uint32_t unemployed_weeks);
void Expanded_MidseasonJoin_Begin(uint32_t unemployed_weeks);
int Expanded_MidseasonJoin_Active(void);
/* Fixture phase (month eighths) of the next unplayed native contest.
 * Separate from the finance accumulator, which tracks last charged phase. */
uint8_t Expanded_Career_NativeCalendarKey(void);
uint8_t Expanded_MidseasonJoin_TargetPhase(void);
/* 0 = no attempt; 1 = reached hire date; 2 = abort (see log). */
int Expanded_MidseasonJoin_LastResult(void);
int Expanded_MidseasonJoin_LastKey(void);
/* OG cup/other events before contract: why legacy catch-up is still required. */
uint32_t Expanded_MidseasonJoin_HistoricalCupEvents(void);
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
/* All elapsed weeks since the FIRST July, returning the current season year
 * and the OG native phase of the hiring day. No archived-season replay. */
int Expanded_MidseasonJoin_AbsoluteCutoff(uint16_t first_year,
                                          uint32_t elapsed_weeks,
                                          uint16_t *out_season,
                                          uint8_t *out_key);
#endif
