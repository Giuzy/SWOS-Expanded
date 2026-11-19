#ifndef EXPANDED_MIDSEASON_JOIN_H
#define EXPANDED_MIDSEASON_JOIN_H

#include "swos_types.h"
#include "expanded_world_time.h" /* calendar inspector uses real civil date */

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
/* Calendar state is read-only except when PositionClubCalendar is invoked
 * after a real manager bind. A completed season must never present a
 * fictitious past event as the next match. */
typedef enum {
    EXP_OG_CLUB_CALENDAR_ND=0,
    EXP_OG_CLUB_CALENDAR_FUTURE=1,
    EXP_OG_CLUB_CALENDAR_COMPLETE=2,
    EXP_OG_CLUB_CALENDAR_BEHIND=3,
    EXP_OG_CLUB_CALENDAR_INVALID=4
} ExpandedOGClubCalendarState;
typedef struct {
    ExpandedOGClubCalendarState status;
    uint16_t current_index,expected_index,total;
    uint8_t cutoff,current_phase,next_phase;
} ExpandedOGClubCalendarAudit;
/* Pure state classifier; a no-future cache is COMPLETE only with a
 * verified OG resident season. Other missing dates are INVALID. */
static inline ExpandedOGClubCalendarState ExpandedOGClubCalendar_Classify(
    unsigned planned,unsigned total,unsigned cursor,int valid,int og_complete)
{
    if(!valid || !total || planned>total || cursor>total)
        return EXP_OG_CLUB_CALENDAR_INVALID;
    if(planned==total)
        return og_complete?EXP_OG_CLUB_CALENDAR_COMPLETE:
                           EXP_OG_CLUB_CALENDAR_INVALID;
    if(cursor==total)return EXP_OG_CLUB_CALENDAR_INVALID;
    return cursor<planned?EXP_OG_CLUB_CALENDAR_BEHIND:
                          EXP_OG_CLUB_CALENDAR_FUTURE;
}
#ifdef SWOS_SDL_BUILD
/* Does NOT move cursors, consume RNG, update scores or load a CAR. */
int Expanded_OGWorld_ClubCalendarAudit(uint16_t year,ExpandedWorldDate now,
                                        ExpandedOGClubCalendarAudit *out);
#endif

/* A native league whose first fixture is before August can already
 * have results during July. Pure helper: returns 1 only if correction
 * is applicable; it never certifies an OG/LWE handoff. */
int Expanded_MidseasonJoin_SouthernCutoff(uint16_t first_year,
                                         uint32_t elapsed_weeks,
                                         uint8_t earliest_league_key,
                                         uint16_t *out_season,
                                         uint8_t *out_key);
/* Runtime verdict: 0=NOT_TESTED, 1=strict world/OG table imported,
 * 2=unsupported or failed (do not enter a fabricated empty table). */
int Expanded_MidseasonJoin_SouthernStatus(void);
/* Instrument actual clock transitions in Windows career; no game-state writes.
 * 0=NOT_TESTED/RECOVERING, 1=OBSERVED_NEAR, 2=FAIL.
 * A near reading is never proof of complete handoff correctness. */
void Expanded_ClockTrace_Reset(void); /* explicit NEW career only */
int Expanded_MidseasonJoin_HandoffReached(uint8_t target,uint8_t actual);
void Expanded_ClockTrace_Sample(const char *event);
int Expanded_ClockTrace_Status(void);
uint32_t Expanded_ClockTrace_Samples(void);
uint32_t Expanded_ClockTrace_Changes(void);
int32_t Expanded_ClockTrace_DeltaDays(void);
const char *Expanded_ClockTrace_FirstProblem(void);
const char *Expanded_ClockTrace_LastEvent(void);
#endif
