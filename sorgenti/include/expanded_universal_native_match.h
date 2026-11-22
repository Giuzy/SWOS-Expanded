/* The SINGLE original 50 Hz football game boundary for a future
 * universal Competition Engine career fixture. Never a calendar, table,
 * bracket, qualification or save writer. Not enabled by legacy menus.
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
} ExpandedUniversalNativeMatch;

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
/* Callback for ExpandedUniversal_RunDue, AFTER durable save_claim. Original
 * InitializeInGameTeamsAndStartGame is the sole score producer. On a
 * canceled or mismatching game return 0 and leave recovery required.
 * No invocation of DIY_PlayOrSimulateLeagueFixture or other legacy writer.
 */
int ExpandedUniversal_NativeRunOriginal(
    void *context,const ExpandedUniversalFixture *fixture,
    ExpandedUniversalMatchWitness *witness);
#endif
#endif
