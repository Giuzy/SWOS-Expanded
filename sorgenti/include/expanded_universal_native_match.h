/* The SINGLE original OG scoring boundary for a universal 2627 fixture.
 * Automatic CPU and manager VIEW RESULT use OG CalculateViewResult QUICK;
 * manually selected PLAY/COACH uses the full original 50 Hz football game.
 * Never a calendar, table, bracket, qualification or save writer;
 * WORLD9697/CLASSIC/DIY/SEASON keep their original callbacks.
 *
 * Call Prepare before CLAIMED/CAR: cancellation of lineup is harmless.
 * Call Run only after the universal dispatcher has durably claimed exactly
 * this fixture and bound long-lived mutable TeamFileEx records.
 */
#ifndef EXPANDED_UNIVERSAL_NATIVE_MATCH_H
#define EXPANDED_UNIVERSAL_NATIVE_MATCH_H
#ifdef SWOS_SDL_BUILD
#include "game_types.h"
#include "expanded_universal_fixture_registry.h"

typedef struct ExpandedUniversalNativeMatch {
    TeamFileEx *home, *away; /* stable live roster records, not temp TEAM buffers */
    const char *competition_name;
    int16_t minimum_substitutes, maximum_substitutes;
    int16_t match_parameters, match_postprocess;
    uint16_t pitch_type, season, pitch_type_or_season;
    uint8_t interactive;    /* 1: Show original lineup BEFORE CAR claim */
    uint8_t lineup_ready;   /* set only after original SetupPlayers succeeds */
    uint8_t abandoned;      /* set when the 50 Hz match was aborted before its end (ESC) */
    /* Fase 2: competition rules of THIS fixture (CE engine), applied with
     * the original OG mechanisms only: ET/penalties/aggregate/away goals
     * through the OG knockout globals (50 Hz) or the OG quick resolvers
     * (ExtraTimeSecondPhaseScoring / PenaltyShootoutScorerPair). */
    uint8_t decides_tie;    /* a level result is not final (KO deciding match) */
    uint8_t extra_time, penalties, away_goals, second_leg;
    uint16_t first_leg_home, first_leg_away; /* goals of THIS home/away side */
    /* substitution windows (CE adapter, 0 = OG behaviour, no limit) */
    uint8_t sub_windows, halftime_free, et_extra_subs, et_extra_windows;
    uint8_t og_mode_flag;   /* OG quick modeFlag: 0xFF single-meeting table */
} ExpandedUniversalNativeMatch;
/* Fase 2 substitution-window adapter, active ONLY inside a CE 50 Hz match
 * whose rules set a window limit (bench_subs.c / game_state.c hooks). */
int ExpandedUniversal_SubWindowAllows(int team_number);
void ExpandedUniversal_SubWindowSessionBegin(int team_number);
void ExpandedUniversal_SubWindowCommitted(int team_number);
void ExpandedUniversal_SubWindowExtraTime(void);

/* Read-only legacy match UI safeguard; automatic 2627 games now use
 * OG quick result and never enter GameLoop. */
int ExpandedUniversal_NativeCpuSilent(void);
/* GameLoop calls this ONLY at its genuine end_of_match_flow label.
 * A forced SDL window close / aborted frame must not certify OG results,
 * even if the previous match's cached score equals current counters. */
void ExpandedUniversal_NativeMarkMatchCompleted(void);
/* Canonical identity from actual team bytes, including second-country files.
 * A fixture's stable_club is (original-country << 16) | (slot + 1).
 * This is independent of manager employment and old TSKC/phase.
 */
int ExpandedUniversal_NativeTeamKey(const TeamFileEx *team,
                                    ExpandedClubKey *club);
/* 1 = ready, 0 = cancel/invalid. Does not run the game or mutate CAR.
 * A caller must cancel immediately if preparation returns zero. */
int ExpandedUniversal_NativePrepare(
    ExpandedUniversalNativeMatch *context,
    const ExpandedUniversalFixture *fixture);
/* OG career RESULT: the manager's quick result first shows the original
 * team screen (VIEW RESULT button). UI only; set around the one call. */
void ExpandedUniversal_NativeSetResultPrompt(int on);
/* Callback for ExpandedUniversal_RunDue, AFTER durable save_claim.
 * Automatic: original OG CalculateViewResult; manual: original OG 50Hz.
 * Both return ONE same-key witness, or fail closed with CLAIMED retained.
 * No DIY_PlayOrSimulateLeagueFixture/legacy competition writer. */
int ExpandedUniversal_NativeRunOriginal(
    void *context,const ExpandedUniversalFixture *fixture,
    ExpandedUniversalMatchWitness *witness);
#endif
#endif
