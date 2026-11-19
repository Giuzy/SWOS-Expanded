/*
 * Amiga SWOS RE Project
 * SWOS Amiga C/68000 Reversed by Ben Baker (https://github.com/benbaker76)
 * SWOS DOS Reversed by Zlatko Karakaš (https://github.com/zlatkok)
 */
/* career_status.c — Player status decay, availability, team init, career finances,
 * chairman warnings, board confidence, reputation bonuses.
 *
 * Ported from SWOS2.s: CyclePlayerStatusFlagsForTeam, MaybeDecayPlayerStatusBucket,
 * MaybeDecayPlayerStatusBucketForTeam, InitInGameTeams_PlayerControlled,
 * InitInGameTeams_WithMatchParam, BuildCareerTeamUnavailablePlayerList,
 * ClearCareerMatchPlayerStatusBucket, ShouldApplyRandomPlayerStatusAfterSim,
 * IsPlayerInjured, GetNumberOfSubstitutes, GetNumberOfSubstitutesInSelectionList,
 * GetNumberOfAvailablePlayers, GetNumberOfAvailableSubstitutes,
 * GetNumberOfAvailableStartingPlayers, GetAgeAdjustedPlayerFormFactor,
 * ClampNewBalanceAndUpdateClubStatus, ClearOverdraftStateIfRecovered,
 * SetOverdraftWarningCooldown, SetPerformanceWarningCooldown,
 * IsMajorInternationalTournament, IsMajorTournamentAllowingRandomStatus,
 * IsMajorTournamentAllowingSquadChanges, IsTournamentAllowingSquadChangesIfNotCareer,
 * ShouldSetupPlayers, CalcLeagueMatchGateReceiptsFactor, CalcCupMatchGateReceiptsFactor,
 * UpdateCareerFinancesAfterMatch, MaybeSackManagerAfterSeasonReview,
 * UpdateCareerChairmanWarningsAfterMatch, UpdateCareerAfterMatch,
 * ApplyWeeklyFinancialDeductions, InitializeCareerSeasonFinancials,
 * GetBoardConfidenceScore, UpdateBoardConfidenceScore, PushBoardConfidenceHistory,
 * CalcBoardConfidenceDeltaPercent, UpdateBoardConfidenceModifiers,
 * UpdateCareerReputationBonuses, AddReputationFromBonusValueDiv4,
 * AddReputationFromBonusValueDiv8, InitializeStartingBalanceForNewSeason,
 * GetExpectedLeagueRankIndex.
 */

#include "career_status.h"
#include "teams.h"
#include "league_phase.h"
#include "uefa_coef.h"
#include "uefa_access.h" /* original association inputs and qualifiers */
#include "conti_season.h"
#include "world_conti.h"   /* reset staged CT world cursor at new career */
#include "test_rules.h"
#include "comp_identity.h"
#include "diy_competition.h"
#include "endian_helpers.h"
#include "career_teams.h"
/* Do not include competition_data.h here: legacy OG globals in this file
 * are declared as byte arrays. The three European records actually used
 * by our native resident path have individually typed declarations. */
#include "game_types.h"
/* competition_data.h conflicts with legacy byte-array externs in this file.
 * The historic Copa resident path calls Comp_ById before LogNoData; make its
 * pointer-returning prototype visible here instead of only inside LogNoData. */
extern Competition *Comp_ById(uint8_t);
extern CompetitionTournament _euroCup;
extern CompetitionCup _cupWinnersCup;
extern CompetitionCup _uefaCup;
#include "menu_system.h"
#include "auto_skip_debug.h"
#include "string_utils.h"
#include "job_offers.h"
#include <string.h>
#include <stddef.h>
#ifdef SWOS_SDL_BUILD
#include "world.h"
#include "league_split.h" /* OG native stage reset: read-only scorer audit */
#include "squad_extras.h"
#include "world_cups.h"
#include "world_cupd.h"
#include "expanded_og_cup_validation.h"
#include "expanded_og_copa_phase.h"
#include "expanded_og_season_retention.h"
#include "expanded_og_cache_match.h"
#include "expanded_og_fixture_history.h"
#include "expanded_og_league_bridge.h"
#include "expanded_qualification_transition.h"
#include "expanded_debug.h"
#include "expanded_world_time.h"
#include "expanded_world_live.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#endif

/* ---- Assembly externs (functions still in asm) ---- */
extern uint16_t Rand(void);
extern uint16_t Rand2(void);
extern TeamFileEx *VerifyTeamLoaded(uint16_t teamNum);
extern uint16_t GetAveragePlayerPrice(TeamFileEx *team);
#include "career_params.h"
extern uint16_t CalcGateReceiptsBaseFromTeamValue(uint16_t avgPrice);
extern uint16_t GetTeamValueBase(uint16_t avgPrice);

/* Replaces _CGetTeamValueForGateReceipts asm bridge */
static uint16_t getTeamValueForGateReceipts(TeamFileEx *team) {
    uint16_t teamNum = BE_READ_U16(team);  /* original: move.w (a0),d0 — reads first 2 bytes */
    VerifyTeamLoaded(teamNum);
    uint16_t avgPrice = GetAveragePlayerPrice(team);
    return CalcGateReceiptsBaseFromTeamValue(avgPrice);
}
/* FindTeamInDIY and GetMoreExpensiveTeamsCount declared in diy_competition.h. */
extern void ShowChairmanMessage(void);
extern void AddCareerTeamPlayer(void);
extern void CheckPlayersLeavingComing(void);
extern uint8_t *TeamSelectionKeyCache_GetCurrentListPtr(void);
extern TeamSelectionKeyCacheMatchSlot *TeamSelectionKeyCache_GetCurrentMatchSlot(void);

/* C-callable asm bridges for GetPlayer/GetPlayerAtIndex */
extern uint8_t *GetPlayer(uint16_t teamNumber, uint16_t playerIndex);
extern uint8_t *GetPlayerAtIndex(uint16_t teamNumber, uint16_t ordinal);
extern uint16_t _cGetPlayerAtIndexFileIdx;

/* Already-ported C functions */
extern uint16_t GetDisplayedPlayerValue(uint8_t *ptr);

/* ---- Data externs ---- */
extern TeamFileEx _careerTeam;
extern uint8_t    _currentMatchPlayers[32];
extern uint16_t   _careerNumPlayers;
extern TeamFileEx _currentTeam;
extern uint8_t    _playerPositions[32];
extern uint16_t   _numPlayers;
extern uint16_t   _trainingGame;
extern uint16_t   _gameType;
extern volatile uint16_t   _isNationalSelector;
extern uint8_t    _careerUnavailablePlayerList[16];
extern uint8_t    _careerUnavailablePlayerLookupTable[];
extern uint8_t    _playerFormModifiers[32];
extern uint16_t   _selTeamNumber;
extern uint16_t   _panicReasonCode;
extern void       PanicHaltLoop(void);

/* Financial globals */
extern int32_t    _newBalance;
extern int32_t    _gateReceipts;
extern int16_t    _gateReceiptsFactor;
extern int32_t    _seasonGateReceiptsAccum;
extern int32_t    _seasonGateReceiptsAccum2;
extern uint16_t   _seasonMatchReceiptsCount;
extern uint16_t   _seasonWeeksDeducted;
extern int32_t    _seasonRunningCostsAccum;
extern int32_t    _seasonRunningCostsAccum2;
extern int32_t    _seasonWagesAccum;
extern int32_t    _playerWagesBill;
extern int32_t    _generalRunningCosts;
extern int32_t    _weeklyPlayerWages;
extern int32_t    _weeklyRunningCosts;
extern int32_t    _totalTeamValue;
extern uint16_t   _teamValueDivisor;
extern uint16_t   _wageScalingDivisor;
extern int32_t    _weeklyWageByPlayerIndex[];
extern uint16_t   _avgTeamValueBases[];
extern int32_t    _old_balance;
extern int32_t    _startingBalance;
extern int32_t    _startingBalanceCopy;
extern int32_t    _competitionBonuses;

/* Warning/sacking state */
extern uint16_t   _overdraftStreak;
extern uint16_t   _overdraftWarningState;
extern uint16_t   _performanceWarningState;
extern uint16_t   _overdraftWarningCooldown;
extern uint16_t   _performanceWarningCooldown;
extern uint16_t   _clubBusinessActive;
extern volatile uint16_t   _youreSacked;
extern volatile uint16_t   _currentMessage;
extern uint16_t   _stayingInOldClub;
extern uint16_t   _careerTeamLostMatch;
extern uint16_t   _competitionSlotUpdatedFlag;
extern uint16_t   _removedOfferPlayersFromSelectedTeamCount;

/* Board confidence */
extern int16_t    _boardConfidenceScore;
extern uint16_t   _boardConfidenceHistory[4];     /* (newest first; read signed) */

/* Trophy/win flags */
extern uint16_t   _wonWorldCup;
extern uint16_t   _wonContinentalCup;
extern uint16_t   _wonInvitationTournament;
extern uint16_t   _playWinCupAnimation;
extern uint16_t   _playPromotedAnimation;
extern uint16_t   _majorTrophyBonusActive;
extern uint16_t   _cupBonus1Active;
extern uint16_t   _cupBonus2Active;
extern uint16_t   _minorTrophyBonusActive;
extern uint16_t   _leagueTitleBonusActive;

/* Competition pointers and contexts */
extern uint8_t *_currentCompetitionPtr;
extern uint8_t *_secondaryCompetitionPtr;
extern uint8_t *_tertiaryCompetitionPtr;
extern uint8_t *_quaternaryCompetitionPtr;
extern CompetitionSlotBuffer _competitionContextBuf;          /* slot-1 untangle (S4): typed buffer */
extern CompetitionSlotBuffer _tertiaryCompetitionContextBuf;   /* slot-2 untangle (S3): typed buffer */
extern CompetitionSlotBuffer _euroCupsCompetitionContextBuf;   /* slot-3 untangle (S5): typed buffer */
extern CompetitionSlotBuffer _primaryCompetitionBuf;           /* slot-0 untangle (S6): typed buffer */
extern volatile uint16_t   _seasonUpdateInProgress;
extern uint16_t   _teamsLoaded;
extern uint16_t   _numSelectedTeams;

/* Competition copy symbols (addresses used for comparisons) */
extern uint8_t    _worldCup2Copy[];
extern uint8_t    _europeanChampionships2Copy[];
extern uint8_t    _copaAmericaCopy[];
extern uint8_t    _africanNationsCupCopy[];
extern uint8_t    _northAmericanCupCopy[];
extern uint8_t    _asianCupCopy[];
extern uint8_t    _oceaniaCupCopy[];
extern uint8_t    _invitationTournamentCopy[];
extern uint8_t    _tourCopy[];
extern uint8_t    _friendlyCopy[];
extern uint8_t    _worldCupQualifyingGroup1Copy[];
extern uint8_t    _worldCupQualifyingGroup8Copy[];
extern uint8_t    _ecQualifyingGroup1Copy[];
extern uint8_t    _ecQualifyingGroup8Copy[];
extern uint8_t    _northAmericaWorldCupQualificationCopy[];
extern uint8_t    _southAmericaWorldCupQualificationCopy[];
extern uint8_t    _africaWorldCupQualificationCopy[];
extern uint8_t    _asiaWorldCupQualificationCopy[];
extern uint8_t    _oceaniaWorldCupQualificationCopy[];

/* Season/career state */
extern uint16_t   _currentDivisionIndex;
extern uint16_t   _seasonReview;
extern uint16_t   _seasonFinished;
/* secondaryCompFinished migrated to competitionContextBuf.tourn.header.compFinished (slot-1 untangle, S4) */
/* tertiaryCompFinished migrated to tertiaryCompetitionContextBuf.tourn.header.compFinished (slot-2 untangle, S3) */
/* leaguePromotionCount migrated to primaryCompetitionBuf.tourn.header.promotionCount (slot-0 untangle, S6) */
/* leagueRelegationCount migrated to primaryCompetitionBuf.tourn.header.relegationCount (slot-0 untangle, S6) */
/* leagueWarningsEnabled migrated to primaryCompetitionBuf.tourn.leagueWarningsEnabled (slot-0 untangle, S6) */
extern uint16_t   _expectedLeagueRankIndex;
extern uint16_t   _diyFoundTeamIndex;
extern uint16_t   _diyCompHasGroupStage;
/* numDiyTeams migrated to primaryCompetitionBuf.tourn.header.numTeams (slot-0 untangle, S6) */
extern uint16_t   _isCareer;
extern int8_t     _careerJobOfferPhaseBias;
extern uint8_t    _viewRandomPhase;
extern uint8_t    _someTeamFlags;
extern uint16_t   _worldCupQualifyingTeam;
extern uint16_t   _worldCupHostTeam;
extern uint16_t   _euroChampionshipQualTeam;
extern uint16_t   _ballSpeed;
extern volatile uint16_t   _currentGameTick;
extern uint8_t    _lastFinancialWeekIndex;
extern DIYTournamentState _diyFileBufferCopy;

/* Winner teams */
/* primaryCompWinnerTeam migrated to primaryCompetitionBuf.tourn.header.winnerTeam (slot-0 untangle, S6) */
/* secondaryCompWinnerTeam migrated to competitionContextBuf.tourn.header.winnerTeam (slot-1 untangle, S4) */
/* tertiaryCompWinnerTeam migrated to tertiaryCompetitionContextBuf.tourn.header.winnerTeam (slot-2 untangle, S3) */
/* quaternaryCompWinnerTeam migrated to euroCupsCompetitionContextBuf.tourn.header.winnerTeam (slot-3 untangle, S5) */
extern uint16_t   _euroCupSlotIndex;

/* Qualifying flags */
extern uint16_t   _qualifyingStage1PendingFlag0;
extern uint16_t   _qualifyingStage1PendingFlag1;
extern uint16_t   _qualifyingStage2PendingFlag0;
extern uint16_t   _qualifyingStage2PendingFlag1;

/* Reputation */
extern int16_t    _nationalSelectorReputationPoints;
extern int16_t    _careerReputationPoints;
extern uint16_t   _winCupBonusCount;
extern uint16_t   _minorTrophyBonusCount;
extern uint16_t   _majorTrophyBonusCount;
/* majorTrophyBonusValue migrated to primaryCompetitionBuf.tourn.header.trophyBonusValue (slot-0 untangle, S6) */
/* minorTrophyBonusValue migrated to competitionContextBuf.tourn.header.trophyBonusValue (slot-1 untangle, S4) */
/* leagueTitleBonusValue migrated to tertiaryCompetitionContextBuf.tourn.header.trophyBonusValue (slot-2 untangle, S3) */
/* majorTrophyOrCupBonusValue migrated to euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue (slot-3 untangle, S5) */

/* DIY competition buffer — kept as uint8_t[] to avoid size warnings */
extern DIYCompetitionEx _diyCompetitionStart;

/* DIY competition struct cast for diyFileBufferCopy / diyCompetitionStart access */

#define GAME_TYPE_CAREER 4
#define GAME_TYPE_SEASON 3

/* cardsInjuries bit masks */
#define CI_CARD_BITS         0x03   /* bits 0-1: card severity */
#define CI_INJURY_BITS       0x1C   /* bits 2-4: injury severity */
#define CI_STATUS_BUCKET     0xE0   /* bits 5-7: status bucket */

/* PF_POSITION_MASK / PF_POSITION_SHIFT now defined in game_types.h */

/* ================================================================
 * Helper: get the player index list pointer for a given team.
 * Returns the appropriate array (currentMatchPlayers, _playerPositions,
 * or team->team.playerNumbers) and sets *outCount to the player count.
 * ================================================================ */
static uint8_t *getPlayerList(TeamFileEx *team, uint16_t *outCount) {
    if (team == &_careerTeam) {
        *outCount = _careerNumPlayers;
        return _currentMatchPlayers;
    }
    if (team == &_currentTeam) {
        *outCount = _numPlayers;
        return _playerPositions;
    }
    *outCount = 16;
    return team->team.playerNumbers;
}

/* ================================================================
 * MaybeDecayPlayerStatusBucket (static)
 * Probabilistically decays the status bucket field (bits 5-7) of a
 * player's cardsInjuries byte.
 *
 * Input: d1 = current bucket bits (already masked to 0xE0), a1 = player record
 * The pointer a1 is team_base + playerIndex * 38 (shifted).
 * ================================================================ */
static void maybeDecayPlayerStatusBucket(uint16_t bucketBits, PlayerFile *playerFile) {
    uint16_t d1 = bucketBits;

    if (d1 == 0x20) {
        /* Bucket level 1: 50% chance to decay to 0 */
        uint16_t r = Rand2();
        if (r & 1)
            return;  /* no decay */
        d1 = 0;
    } else if (d1 == 0xC0) {
        /* Bucket level 6: 1/8 chance to decay to random level 1-5 */
        uint16_t r = Rand2();
        if ((r & 7) != 0)
            return;  /* no decay */
        r = Rand2();
        uint16_t newLevel = (uint16_t)((r * 5) >> 8);
        d1 = (newLevel << 5) + 0x20;
    } else if (d1 == 0xE0) {
        /* Bucket level 7: never decays */
        return;
    } else {
        /* Other levels: decrement by one step */
        d1 -= 0x20;
        if (d1 == 0x20) {
            /* Reached level 1 after decrement: 50% chance to clear to 0 */
            uint16_t r = Rand2();
            if (r & 1) {
                /* keep at 1, write it */
            } else {
                d1 = 0;
            }
        }
    }

    /* Write back the new bucket value */
    uint8_t ci = playerFile->cardsInjuries;
    ci = (ci & 0x1F) | (uint8_t)d1;
    playerFile->cardsInjuries = ci;
}

/* ================================================================
 * MaybeDecayPlayerStatusBucketForTeam (static)
 * Iterates all players in a team and calls MaybeDecayPlayerStatusBucket
 * for those with nonzero status bucket bits.
 * ================================================================ */
static void maybeDecayPlayerStatusBucketForTeam(TeamFileEx *team) {
    /* Skip computer-controlled teams */
    if (team->team.teamControls == 1)
        return;

    if (team == &_careerTeam) {
        /* Career team: use currentMatchPlayers for indirection */
        int16_t count = (int16_t)_careerNumPlayers - 1;
        for (int16_t i = count; i >= 0; i--) {
            int16_t idx = (int8_t)_currentMatchPlayers[i];
            PlayerFile *pf = &team->players[idx];
            uint16_t bucket = (uint16_t)pf->cardsInjuries & CI_STATUS_BUCKET;
            if (bucket != 0) {
                maybeDecayPlayerStatusBucket(bucket, pf);
            }
        }
    } else {
        /* Non-career: iterate 16 players sequentially */
        PlayerFile *pf = team->players;
        for (int16_t i = 15; i >= 0; i--) {
            uint16_t bucket = (uint16_t)pf->cardsInjuries & CI_STATUS_BUCKET;
            if (bucket != 0) {
                maybeDecayPlayerStatusBucket(bucket, pf);
            }
            pf++;
        }
    }
}

/* ================================================================
 * CyclePlayerStatusFlagsForTeam (static)
 * Decays injury severity (mode==0) or card severity (mode!=0) for
 * all players in a team.
 * ================================================================ */
static void cyclePlayerStatusFlagsForTeam(TeamFileEx *team, uint16_t mode) {
    /* Skip if training game or computer-controlled team */
    if (_trainingGame != 0)
        return;
    if (team->team.teamControls == 1)
        return;

    uint8_t *playerList;
    int16_t count;

    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
        count = (int16_t)_careerNumPlayers - 1;
    } else {
        playerList = team->team.playerNumbers;
        count = 15;
    }

    for (int16_t i = count; i >= 0; i--) {
        int16_t idx = (int8_t)playerList[i];
        PlayerFile *pf = &team->players[idx];
        uint8_t ci = pf->cardsInjuries;

        if (mode == 0) {
            /* Decay injury bits (2-4) */
            uint16_t injury = (uint16_t)ci & CI_INJURY_BITS;
            if (injury == 0)
                continue;

            uint16_t newInjury;
            if (injury == 0x10)
                newInjury = 0x0C;       /* severity 4 → 3 */
            else if (injury == 0x0C)
                newInjury = 0x08;       /* severity 3 → 2 */
            else if (injury == 0x08)
                newInjury = 0x00;       /* severity 2 → 0 (healed) */
            else
                continue;               /* other values: skip */

            ci = (ci & 0xE3) | (uint8_t)newInjury;
            pf->cardsInjuries = ci;
        } else {
            /* Decay card bits (0-1) */
            uint16_t card = (uint16_t)ci & CI_CARD_BITS;
            if (card == 0)
                continue;

            uint16_t newCard;
            if (card == 2)
                newCard = 1;            /* card level 2 → 1 */
            else
                newCard = 0;            /* card level 1 or 3 → 0 */

            ci = (ci & 0xFC) | (uint8_t)newCard;
            pf->cardsInjuries = ci;
        }
    }
}

/* ================================================================
 * InitInGameTeams_PlayerControlled
 * Decays status buckets and cycles injury/card flags for both teams.
 * mode parameter is passed to CyclePlayerStatusFlagsForTeam.
 * ================================================================ */
__attribute__((used, externally_visible))
void InitInGameTeams_PlayerControlled(TeamFileEx *team1, TeamFileEx *team2, uint16_t mode) {
    maybeDecayPlayerStatusBucketForTeam(team1);
    maybeDecayPlayerStatusBucketForTeam(team2);
    cyclePlayerStatusFlagsForTeam(team1, mode);
    cyclePlayerStatusFlagsForTeam(team2, mode);
    if (mode == 0 && !_trainingGame) {
        extern uint8_t _expandedFacilityLevels[8];
        /* Medical centre tier 3+: one extra injury-decay step for the
         * manager's team; preserve all non-injury bits and suspensions. */
        if (_expandedFacilityLevels[6] >= 1) {
            TeamFileEx *teams[2] = {team1,team2};
            for (int t=0;t<2;t++) if (teams[t] == &_careerTeam &&
                    teams[t]->team.teamControls != 1) {
                for (unsigned i=0;i<_careerNumPlayers && i<32;i++) {
                    int idx = (int8_t)_currentMatchPlayers[i];
                    if (idx<0 || idx>=32) continue;
                    PlayerFile *pf = &teams[t]->players[idx];
                    uint8_t ci=pf->cardsInjuries;
                    unsigned inj=ci & 0x1C;
                    /* +10/20/30/40/50% recovery per tier; deterministic
                     * PRNG, never affect suspensions or other teams. */
                    if((unsigned)Rand()>=_expandedFacilityLevels[6]*256u/10u) continue;
                    if (inj==0x10) pf->cardsInjuries=(ci&0xE3)|0x0C;
                    else if (inj==0x0C) pf->cardsInjuries=(ci&0xE3)|0x08;
                    else if (inj==0x08) pf->cardsInjuries=ci&0xE3;
                }
            }
        }
    }
}

/* ================================================================
 * InitInGameTeams_WithMatchParam / InitInGameTeams_WithMatchParam_Alt
 * Same as PlayerControlled but loads mode from diyCompetitionStart->substitutesCount.
 * The Alt variant is an identical copy in the original game.
 * ================================================================ */
__attribute__((used, externally_visible))
void InitInGameTeams_WithMatchParam(TeamFileEx *team1, TeamFileEx *team2) {
    uint16_t mode = _diyCompetitionStart.base.substitutesCount;
    maybeDecayPlayerStatusBucketForTeam(team1);
    maybeDecayPlayerStatusBucketForTeam(team2);
    cyclePlayerStatusFlagsForTeam(team1, mode);
    cyclePlayerStatusFlagsForTeam(team2, mode);
}

/* ================================================================
 * ShouldApplyRandomPlayerStatusAfterSim
 * Returns 0 if in career mode with national selector and not a major
 * tournament. Returns 1 otherwise.
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t ShouldApplyRandomPlayerStatusAfterSim(void) {
    if (_gameType == GAME_TYPE_CAREER) {
        if (_isNationalSelector != 0) {
            uint16_t isMajor = IsMajorTournamentAllowingRandomStatus();
            if (isMajor == 0) {
                return 0;
            }
        }
    }
    return 1;
}

/* ================================================================
 * BuildCareerTeamUnavailablePlayerList
 * Builds a list (max 4) of career team players who are injured.
 * ================================================================ */
__attribute__((used, externally_visible))
void BuildCareerTeamUnavailablePlayerList(void) {
    uint16_t *list = (uint16_t *)_careerUnavailablePlayerList;
    /* Init 4 entries to -1 (each is 4 bytes = 2 words) */
    list[0] = 0xFFFF;
    list[2] = 0xFFFF;
    list[4] = 0xFFFF;
    list[6] = 0xFFFF;

    if (ShouldApplyRandomPlayerStatusAfterSim() != 0)
        return;

    uint16_t slotsLeft = 4;
    TeamFileEx *teamEx = &_careerTeam;
    int16_t count = (int16_t)_careerNumPlayers - 1;
    uint8_t *outPtr = _careerUnavailablePlayerList;

    for (int16_t i = count; i >= 0; i--) {
        int16_t idx = (int8_t)_currentMatchPlayers[i];
        PlayerFile *pf = &teamEx->players[idx];
        uint16_t injury = (uint16_t)pf->cardsInjuries & CI_INJURY_BITS;
        if (injury == 0)
            continue;

        /* Copy 4-byte entry from lookup table */
        uint16_t tableOff = (uint16_t)i * 4;
        uint8_t *src = _careerUnavailablePlayerLookupTable + tableOff;
        outPtr[0] = src[0];
        outPtr[1] = src[1];
        outPtr[2] = src[2];
        outPtr[3] = src[3];
        outPtr += 4;

        slotsLeft--;
        if (slotsLeft == 0)
            return;
    }
}

/* ================================================================
 * ClearCareerMatchPlayerStatusBucket
 * Clears status bucket bits (5-7) for all career team players.
 * ================================================================ */
__attribute__((used, externally_visible))
void ClearCareerMatchPlayerStatusBucket(void) {
    TeamFileEx *teamEx = &_careerTeam;
    int16_t count = (int16_t)_careerNumPlayers - 1;

    for (int16_t i = count; i >= 0; i--) {
        int16_t idx = (int8_t)_currentMatchPlayers[i];
        PlayerFile *pf = &teamEx->players[idx];
        uint8_t ci = pf->cardsInjuries;
        ci &= 0x1F;  /* clear bits 5-7 */
        pf->cardsInjuries = ci;
    }
}

/* ================================================================
 * IsPlayerInjured
 * playerBase = team + playerIndex * 38 (shifted pointer).
 * Checks injury (bits 2-4), cards (bits 0-1), and status bucket
 * (bits 5-7) based on mode:
 *   mode -1: check both injuries and cards
 *   mode  0: check injuries only
 *   mode  1: check cards only
 * Returns 1 if injured, 0 if not.
 * ================================================================ */
/* Core injury/suspension status from the persisted cardsInjuries data —
 * the actual asm logic. IsPlayerInjured()/IsPlayerInjuredForDisplay() below
 * are thin wrappers (only difference: the beta no-injuries override). */
static uint16_t player_injury_status(PlayerFile *playerFile, int16_t mode) {
    PlayerFile *pf = &playerFile[2];
    uint8_t ci = pf->cardsInjuries;

    if (_trainingGame != 0)
        goto check_bucket;

    if (mode != 1) {
        /* Check injury bits (2-4) */
        uint16_t injury = (uint16_t)ci & CI_INJURY_BITS;
        if (injury != 0 && injury != 4)
            return 1;  /* injured (severity > 1) */

        if (mode >= 0)
            goto check_bucket;
    }

    /* Check card bits (0-1) */
    {
        uint16_t card = (uint16_t)ci & CI_CARD_BITS;
        if (card != 0)
            return 1;  /* has card suspension */
    }

check_bucket:
    /* Check status bucket bits (5-7) */
    {
        uint16_t bucket = (uint16_t)ci & CI_STATUS_BUCKET;
        if (bucket == 0 || bucket == 0x20)
            return 0;  /* no bucket or minor (0x20) → not injured */
        return 1;      /* serious bucket → injured */
    }
}

__attribute__((used, externally_visible))
uint16_t IsPlayerInjured(PlayerFile *playerFile, int16_t mode) {
    if (TestRules_NoInjuriesCards()) {
    /* The testers' rule (test_rules.h): nobody is ever unavailable for SELECTION/GAMEPLAY,
     * regardless of what's persisted in cardsInjuries. Catches any injury/
     * card/bucket source the per-write-site gates may have missed (loaded
     * saves with pre-existing injury bits, edit-tactics writes, etc.).
     * NOTE: this is a GAMEPLAY-availability override only — it must NOT
     * suppress the squad-screen DISPLAY (the red suspended/injured name), or
     * the name stops matching the drawn "3"/injury indicator. DISPLAY callers
     * use IsPlayerInjuredForDisplay() which keeps the honest status. */
        return 0;
    }
    return player_injury_status(playerFile, mode);
}

/* DISPLAY-only status query: reports the REAL persisted injury/suspension even
 * under the testers' rule (test_rules.h), so the edit-tactics squad screen colours a
 * suspended/injured player's name red to match its drawn indicator. On non-beta
 * builds this is identical to IsPlayerInjured (both just call the core). */
__attribute__((used, externally_visible))
uint16_t IsPlayerInjuredForDisplay(PlayerFile *playerFile, int16_t mode) {
    return player_injury_status(playerFile, mode);
}

/* ================================================================
 * GetNumberOfSubstitutes
 * Counts players with position == 0 (goalkeeper) in the team.
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t GetNumberOfSubstitutes(TeamFileEx *team, uint16_t numPl) {
    int16_t iter = (int16_t)numPl - 1;
    uint8_t *playerList;
    uint16_t dummy;

    /* Three-way dispatch for player list source */
    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
    } else if (team == &_currentTeam) {
        playerList = _playerPositions;
    } else {
        playerList = team->team.playerNumbers;
    }

    uint16_t count = 0;
    for (; iter >= 0; iter--) {
        int16_t idx = (int8_t)playerList[iter];
        PlayerFile *pf = &team->players[idx];
        uint16_t position = ((uint16_t)pf->positionAndFace & PF_POSITION_MASK) >> PF_POSITION_SHIFT;
        if (position == 0)
            count++;
    }
    return count;
}

/* ================================================================
 * GetNumberOfSubstitutesInSelectionList
 * Counts goalkeepers in selection list (indices 11 to extraCount+10).
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t GetNumberOfSubstitutesInSelectionList(TeamFileEx *team, uint16_t extraCount) {
    int16_t iter = (int16_t)extraCount + 10;
    uint8_t *playerList;

    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
    } else if (team == &_currentTeam) {
        playerList = _playerPositions;
    } else {
        playerList = team->team.playerNumbers;
    }

    uint16_t count = 0;
    for (; iter >= 11; iter--) {
        int16_t idx = (int8_t)playerList[iter];
        PlayerFile *pf = &team->players[idx];
        uint16_t position = ((uint16_t)pf->positionAndFace & PF_POSITION_MASK) >> PF_POSITION_SHIFT;
        if (position == 0)
            count++;
    }
    return count;
}

/* ================================================================
 * Common helper for availability checking.
 * Checks if a player is disabled based on injury/card/bucket bits.
 * Returns 1 if disabled, 0 if available.
 *
 * Shared core of GetNumberOfAvailablePlayers / GetNumberOfAvailableSubstitutes
 * / GetNumberOfAvailableStartingPlayers — i.e. SQUAD-LEGALITY, which is what
 * ShowOrHidePlayMatchButton gates the PLAY MATCH / VIEW RESULT button on
 * (SWOS2.s loc_14872A: GetNumberOfAvailablePlayers(d1=11); tst.w d6; bne →
 * loc_14876E hides the button). An injured starter therefore blocks the match
 * until he is substituted — correct, original SWOS behaviour.
 *
 * That makes this a GAMEPLAY-availability query, so it must honour the same
 * testers' rule (test_rules.h) override as IsPlayerInjured(). Without it the
 * beta flag only half-applied: IsPlayerInjured said "nobody is unavailable"
 * while these counters still read cardsInjuries directly, so a tester with a
 * pre-existing injury in a loaded save got NO start button at all and was
 * forced to substitute — exactly what the flag exists to prevent.
 * The squad screen still shows the red name / "3" indicator, since those are
 * DISPLAY queries and route through IsPlayerInjuredForDisplay().
 * ================================================================ */
static uint16_t isPlayerDisabled(PlayerFile *pf, int16_t excludeCardsFlag) {
    if (TestRules_NoInjuriesCards())
        return 0;
    uint8_t ci = pf->cardsInjuries;

    /* Check status bucket bits (5-7) */
    uint16_t bucket = (uint16_t)ci & CI_STATUS_BUCKET;
    if (bucket != 0 && bucket != 0x20)
        return 1;  /* serious bucket → disabled */

    if (_trainingGame != 0)
        return 0;  /* training games ignore injuries/cards */

    if (excludeCardsFlag != 1) {
        /* Check injury severity bits (2-4) */
        uint16_t injury = (uint16_t)ci & CI_INJURY_BITS;
        if (injury != 0 && injury != 4)
            return 1;  /* injured (severity > 1) */

        if (excludeCardsFlag >= 0)
            return 0;  /* mode 0: skip card check */
    }

    /* Check card bits (0-1) */
    uint16_t card = (uint16_t)ci & CI_CARD_BITS;
    if (card != 0)
        return 1;  /* has card → disabled */

    return 0;
}

/* Expose the exact OG selection predicate to the Expanded pre-match UI.
 * Injuries, suspensions, status bucket and tester rules remain unchanged. */
uint16_t Career_IsSelectionUnavailable(const PlayerFile *player,int16_t flags)
{
    return player ? isPlayerDisabled((PlayerFile *)player,flags) : 1u;
}

/* ================================================================
 * GetNumberOfAvailablePlayers
 * Returns (disabled << 16) | (available & 0xFFFF).
 * ================================================================ */
__attribute__((used, externally_visible))
uint32_t GetNumberOfAvailablePlayers(TeamFileEx *team, int16_t excludeCardsFlag, uint16_t numPl) {
    uint8_t *playerList;
    uint16_t totalCount;

    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
        totalCount = _careerNumPlayers;
    } else if (team == &_currentTeam) {
        playerList = _playerPositions;
        totalCount = _numPlayers;
    } else {
        playerList = team->team.playerNumbers;
        totalCount = 16;
    }

    if (numPl != 0)
        totalCount = numPl;

    uint16_t disabled = 0;
    uint16_t available = totalCount;
    int16_t iter = (int16_t)totalCount - 1;

    for (; iter >= 0; iter--) {
        int16_t idx = (int8_t)playerList[iter];
        PlayerFile *pf = &team->players[idx];
        if (isPlayerDisabled(pf, excludeCardsFlag)) {
            disabled++;
            available--;
        }
    }

    return ((uint32_t)disabled << 16) | (uint32_t)available;
}

/* ================================================================
 * GetNumberOfAvailableSubstitutes
 * Counts available goalkeeper-position players.
 * Returns (disabled << 16) | (available & 0xFFFF).
 * ================================================================ */
__attribute__((used, externally_visible))
uint32_t GetNumberOfAvailableSubstitutes(TeamFileEx *team, int16_t flag) {
    uint8_t *playerList;
    uint16_t totalCount;

    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
        totalCount = _careerNumPlayers;
    } else if (team == &_currentTeam) {
        playerList = _playerPositions;
        totalCount = _numPlayers;
    } else {
        playerList = team->team.playerNumbers;
        totalCount = 16;
    }

    uint16_t disabled = 0;
    uint16_t goalkeepers = 0;
    int16_t iter = (int16_t)totalCount - 1;

    for (; iter >= 0; iter--) {
        int16_t idx = (int8_t)playerList[iter];
        PlayerFile *pf = &team->players[idx];

        /* Check if goalkeeper (position == 0) */
        uint16_t position = ((uint16_t)pf->positionAndFace & PF_POSITION_MASK) >> PF_POSITION_SHIFT;
        if (position != 0)
            continue;

        goalkeepers++;
        if (isPlayerDisabled(pf, flag)) {
            disabled++;
            goalkeepers--;
        }
    }

    return ((uint32_t)disabled << 16) | (uint32_t)goalkeepers;
}

/* ================================================================
 * GetNumberOfAvailableStartingPlayers
 * Counts available non-goalkeeper players. Uses career path only
 * (does not check currentTeam).
 * Returns (disabled << 16) | (available & 0xFFFF).
 * ================================================================ */
__attribute__((used, externally_visible))
uint32_t GetNumberOfAvailableStartingPlayers(TeamFileEx *team, int16_t flag) {
    uint8_t *playerList;
    uint16_t totalCount;

    if (team == &_careerTeam) {
        playerList = _currentMatchPlayers;
        totalCount = _careerNumPlayers;
    } else {
        playerList = team->team.playerNumbers;
        totalCount = 16;
    }

    uint16_t disabled = 0;
    uint16_t starters = 0;
    int16_t iter = (int16_t)totalCount - 1;

    for (; iter >= 0; iter--) {
        int16_t idx = (int8_t)playerList[iter];
        PlayerFile *pf = &team->players[idx];

        /* Check if NOT goalkeeper (position != 0) */
        uint16_t position = ((uint16_t)pf->positionAndFace & PF_POSITION_MASK) >> PF_POSITION_SHIFT;
        if (position == 0)
            continue;

        starters++;
        if (isPlayerDisabled(pf, flag)) {
            disabled++;
            starters--;
        }
    }

    return ((uint32_t)disabled << 16) | (uint32_t)starters;
}

/* ================================================================
 * GetAgeAdjustedPlayerFormFactor
 * playerBase = team + playerIndex * 38 (shifted pointer).
 * Reads age (price field at +0x6C), form (someFlag at +0x71),
 * position (positionAndFace at +0x66).
 * Returns form / ageDivisor.
 * ================================================================ */

static const uint8_t kPlayerAgeToFormDivisorTable[50] = {
    0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
    0x04,0x04,0x05,0x05,0x05,0x05,0x05,0x05,0x06,0x06,
    0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x08,
    0x08,0x08,0x08,0x08,0x08,0x09,0x09,0x09,0x0A,0x0A,
    0x0C,0x0C,0x0C,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F
};

__attribute__((used, externally_visible))
int16_t GetAgeAdjustedPlayerFormFactor(uint8_t *playerBase) {
    PlayerFile *pf = (PlayerFile *)(playerBase + 76);
    int8_t age = pf->price;
    int16_t ageDivisor = (int16_t)(uint8_t)kPlayerAgeToFormDivisorTable[(uint8_t)age];

    int16_t formValue;
    uint8_t flagByte = (uint8_t)pf->form;

    if (flagByte == PLAYER_FORM_TRIALIST || flagByte == PLAYER_FORM_RESERVE) {
        /* Special career form: compute player index within careerTeam */
        uint32_t offset = (uintptr_t)((uintptr_t)playerBase - (uintptr_t)&_careerTeam);
        uint16_t playerIdx = offset / 38;
        formValue = (int16_t)(int8_t)_playerFormModifiers[playerIdx];
        formValue <<= 1;  /* * 2 */
    } else {
        formValue = (int16_t)(int8_t)flagByte;
        /* If bit 0 of positionAndFace is set, double the form */
        if (pf->positionAndFace & PF_INFORM) {
            formValue <<= 1;
        }
    }

    /* divs.w d1,d0: signed 32÷16 division */
    int32_t num = (int32_t)formValue;
    int16_t result = (int16_t)(num / ageDivisor);
    return result;
}

/* ================================================================
 * Chairman message constants
 * ================================================================ */
#define CHAIRMAN_BANK_BALANCE_OVERDRAFT      5
#define CHAIRMAN_OVERDRAFT_LAST_WARNING      6
#define CHAIRMAN_CLEAR_OVERDRAFT_IMMEDIATELY 7
#define CHAIRMAN_YOURE_SACKED                8
#define CHAIRMAN_IMPROVE_LEAGUE_POSITION     9
#define CHAIRMAN_IMPROVE_LEAGUE_POSITION_SOON 10
#define CHAIRMAN_THREE_MORE_MATCHES          11
#define CHAIRMAN_YOURE_SACKED_2              12
#define CHAIRMAN_WE_DIDNT_RENEW_YOUR_CONTRACT 38

/* ================================================================
 * Data tables (moved from asm to static const in C)
 * ================================================================ */
static const uint16_t leagueMatchReceiptsMultiplierTable[6] = {
    400, 300, 200, 150, 125, 100
};

static const uint16_t cupMatchReceiptsMultiplierTable[6] = {
    800, 600, 400, 300, 250, 200
};

static const int16_t seasonReviewConfidenceDeltaTable[5] = {
    50, 25, 0, -25, -50
};

static const int16_t nationalSelectorSeasonReviewReputationDeltaTable[5] = {
    5, 2, 0, -2, -5
};

/* ================================================================
 * Group A: Simple utility functions
 * ================================================================ */

/* ClampNewBalanceAndUpdateClubStatus
 * Clamp balance to 100M, set clubBusinessActive via smi, manage overdraftStreak.
 */
__attribute__((used, externally_visible))
void ClampNewBalanceAndUpdateClubStatus(void) {
    if (_newBalance > 100000000)
        _newBalance = 100000000;
    /* smi: set byte to -1 if negative, 0 if positive */
    _clubBusinessActive = (_newBalance < 0) ? -1 : 0;
    if (_newBalance < 0)
        _overdraftStreak++;
    else
        _overdraftStreak = 0;
}

/* ClearOverdraftStateIfRecovered */
__attribute__((used, externally_visible))
void ClearOverdraftStateIfRecovered(void) {
    if (_overdraftWarningState != 0) {
        if (_newBalance >= 0)
            _overdraftWarningState = 0;
    }
}

/* SetOverdraftWarningCooldown — random 4-8 */
static void setOverdraftWarningCooldown(void) {
    uint16_t r = Rand2();
    uint16_t val = (uint16_t)((r * 5) >> 8) + 4;
    _overdraftWarningCooldown = val;
}

/* SetPerformanceWarningCooldown — random 4-8 */
static void setPerformanceWarningCooldown(void) {
    uint16_t r = Rand2();
    uint16_t val = (uint16_t)((r * 5) >> 8) + 4;
    _performanceWarningCooldown = val;
}

/* ================================================================
 * Group B: Tournament check functions
 * ================================================================ */

/* IsMajorInternationalTournament
 * Returns 1 if current/secondary competition is a major international tournament.
 * Sets *outCtx to the appropriate competition context pointer.
 */
__attribute__((used, externally_visible))
uint16_t IsMajorInternationalTournament(uint8_t **outCtx) {
    /* Slot pointers may be NULL (empty slot); 0xFF matches no COMPID, so a NULL
     * slot falls through exactly as the original pointer-compare did. */
    uint8_t curId = _currentCompetitionPtr ? comp_id((void *)_currentCompetitionPtr) : 0xFFu;

    /* Check primary competition: World Cup + 6 continental cups + invitation + tour */
    if (curId == COMPID_worldCup2 ||
        comp_is_continental_cup(curId) ||
        curId == COMPID_invitationTournament ||
        curId == COMPID_tour) {
        *outCtx = (uint8_t *)&_primaryCompetitionBuf;
        return 1;
    }

    /* Check secondary competition: same set minus World Cup */
    uint8_t secId = _secondaryCompetitionPtr ? comp_id((void *)_secondaryCompetitionPtr) : 0xFFu;
    if (comp_is_continental_cup(secId) ||
        secId == COMPID_invitationTournament ||
        secId == COMPID_tour) {
        *outCtx = (uint8_t *)&_competitionContextBuf;
        return 1;
    }

    return 0;
}

/* IsMajorTournamentAllowingRandomStatus
 * Wraps IsMajorInternationalTournament, checks offset 0x44.
 * Returns nonzero if major tournament allows random status.
 */
__attribute__((used, externally_visible))
uint16_t IsMajorTournamentAllowingRandomStatus(void) {
    uint8_t *ctx;
    uint16_t isMajor = IsMajorInternationalTournament(&ctx);
    if (isMajor == 0)
        return 0;
    /* +0x44 = matchStarted in DIYCompetition base */
    return (uint16_t)((DIYCompetition *)ctx)->matchStarted;
}

/* IsMajorTournamentAllowingSquadChanges
 * Wraps IsMajorInternationalTournament, checks offset 0x46.
 */
__attribute__((used, externally_visible))
uint16_t IsMajorTournamentAllowingSquadChanges(void) {
    uint8_t *ctx;
    uint16_t isMajor = IsMajorInternationalTournament(&ctx);
    if (isMajor == 0)
        return 0;
    /* +0x46 = careerTeamInvolved in DIYCompetition base */
    return (uint16_t)((DIYCompetition *)ctx)->careerTeamInvolved;
}

/* IsTournamentAllowingSquadChangesIfNotCareer */
__attribute__((used, externally_visible))
uint16_t IsTournamentAllowingSquadChangesIfNotCareer(void) {
    if (_isCareer != 0)
        return 0;
    return IsMajorTournamentAllowingSquadChanges();
}

/* ShouldSetupPlayers */
__attribute__((used, externally_visible))
uint16_t ShouldSetupPlayers(void) {
    if (_gameType == GAME_TYPE_CAREER) {
        if (_isNationalSelector != 0) {
            uint16_t allows = IsMajorTournamentAllowingSquadChanges();
            if (allows == 0)
                return 0;
        }
    }
    return 1;
}

/* ================================================================
 * Group C: Gate receipt calculations
 * ================================================================ */

/* CalcLeagueMatchGateReceiptsFactor (static)
 * Computes gate receipts factor for league matches from team values.
 * Returns factor in d7. Panics on 16-bit overflow.
 */
static uint16_t calcLeagueMatchGateReceiptsFactor(TeamFileEx *team1, TeamFileEx *team2) {
    uint32_t d0;

    /* team1 value */
    if (team1 == &_careerTeam) {
        d0 = (uint32_t)_totalTeamValue;
    } else {
        d0 = getTeamValueForGateReceipts(team1);
    }
    d0 = (uint16_t)(d0 / _careerParams.gate_value_divisor);
    d0 = (uint32_t)(uint16_t)d0 * _careerParams.gate_league_mult_home;
    uint32_t d7 = d0;

    /* team2 value */
    if (team2 == &_careerTeam) {
        d0 = (uint32_t)_totalTeamValue;
    } else {
        d0 = getTeamValueForGateReceipts(team2);
    }
    d0 = (uint16_t)(d0 / _careerParams.gate_value_divisor);
    d0 = (uint32_t)(uint16_t)d0 * _careerParams.gate_league_mult_away;
    d7 += d0;

    d7 >>= 4;   /* lsr.l #4 */

    /* divu.w teamValueDivisor — check for 16-bit overflow */
    if (_teamValueDivisor == 0 || d7 > 0xFFFFFFFFu) {
        /* bvs → panic */
        _panicReasonCode = 0x9232;
        PanicHaltLoop();
        return 0; /* unreachable */
    }
    uint32_t quotient = d7 / (uint32_t)_teamValueDivisor;
    uint16_t remainder = (uint16_t)(d7 % (uint32_t)_teamValueDivisor);
    /* bvs checks if quotient > 0xFFFF (overflow in 16-bit result) */
    if (quotient > 0xFFFF) {
        _panicReasonCode = 0x9232;
        PanicHaltLoop();
        return 0;
    }
    return (uint16_t)quotient;
}

/* CalcCupMatchGateReceiptsFactor (static)
 * Same as league but with different multipliers.
 */
static uint16_t calcCupMatchGateReceiptsFactor(TeamFileEx *team1, TeamFileEx *team2) {
    uint32_t d0;

    if (team1 == &_careerTeam) {
        d0 = (uint32_t)_totalTeamValue;
    } else {
        d0 = getTeamValueForGateReceipts(team1);
    }
    d0 = (uint16_t)(d0 / _careerParams.gate_value_divisor);
    d0 = (uint32_t)(uint16_t)d0 * _careerParams.gate_cup_mult;
    uint32_t d7 = d0;

    if (team2 == &_careerTeam) {
        d0 = (uint32_t)_totalTeamValue;
    } else {
        d0 = getTeamValueForGateReceipts(team2);
    }
    d0 = (uint16_t)(d0 / _careerParams.gate_value_divisor);
    d0 = (uint32_t)(uint16_t)d0 * _careerParams.gate_cup_mult;
    d7 += d0;

    d7 >>= 4;

    if (_teamValueDivisor == 0 || d7 > 0xFFFFFFFFu) {
        _panicReasonCode = 0x9232;
        PanicHaltLoop();
        return 0;
    }
    uint32_t quotient = d7 / (uint32_t)_teamValueDivisor;
    if (quotient > 0xFFFF) {
        _panicReasonCode = 0x9232;
        PanicHaltLoop();
        return 0;
    }
    return (uint16_t)quotient;
}

/* UpdateCareerFinancesAfterMatch
 * Complex dispatcher that calculates gate receipts based on competition type.
 */
__attribute__((used, externally_visible))
void UpdateCareerFinancesAfterMatch(DIYCompetitionEx *comp, TeamFileEx *team, TeamFileEx *opponent) {
    if (_isNationalSelector != 0)
        return;

    /* The 0x2C word is OVERLOADED: it's named compSlot for its team-selection
     * cache-key role, but in THIS match-finance path it carries the competition
     * TYPE discriminator (0=league, 1/2/4=cup, 3=DIY) — hence reading compSlot
     * into a var called compType. (A future rename pass may give 0x2C a name
     * that captures both roles, or reconcile it with competitionType@0x00.)
     *
     * Read the whole (compSlot, slotId) word: *(uint16_t *) round-trips the
     * value written via *(uint16_t *)&compSlot = slot on both endians.
     * PACK_BE_U16(compSlot, slotId) is wrong on SDL LE because the byte layout
     * is reversed (slot lands in the compSlot byte, not slotId) — that produced
     * compType = slot << 8 and dispatched every match to the panic branch. */
    int16_t compType = (int16_t)*(uint16_t *)&comp->base.compSlot;
    uint32_t d7;

    if (compType == 0) {
        goto league_match;
    } else if (compType == 4) {
        goto cup_match_type4;
    } else if (compType == 2 && comp->base.hasGroups) {
        goto diy_comp_match;            /* (a world's state championship in slot 2: states.c) */
    } else if (compType == 1 || compType == 2) {
        goto cup_match_type1or2;
    } else if (compType == 3) {
        goto diy_comp_match;
    } else {
        /* Report the unexpected compType value */
        _panicReasonCode = 0x9200 | (compType & 0xFF);
        PanicHaltLoop();
        return;
    }

league_match:
    /* a1 = the home side, a2 = the opponent (SWOS2.s: CalcLeagueMatchGateReceiptsFactor
     * weighs both clubs; the C used to pass the home team twice). */
    d7 = calcLeagueMatchGateReceiptsFactor(team, opponent);
    {
        int16_t isDiy = comp->base.hasGroups;  /* 0x48 */
        if (isDiy != 0) {
            /* DIY league: check group state via file buffer stride */
            uint16_t groupIdxField = (uint16_t)_diyFileBufferCopy.header.roundComplete;
            DIYTournamentSlot *slot = (DIYTournamentSlot *)((uint8_t *)comp + groupIdxField * 0x76 + 0x15A);
            if (slot->isLeagueFormat != 0)
                goto compute_receipts;
            if (slot->formatFlag16E == 2)
                goto compute_receipts;
            goto increment_count;
        }

        /* Non-DIY league */
        if (comp->base.competitionType != 0) {
            _panicReasonCode = 0x9235;
            PanicHaltLoop();
            return;
        }

        /* Compute gate receipt factor based on division and rank */
        int16_t d3, d4;
        if (_currentDivisionIndex == 0) {
            d3 = 0x52; d4 = 0x7D;
        } else if (_currentDivisionIndex == 1) {
            d3 = 0x55; d4 = 0x81;
        } else if (_currentDivisionIndex == 2) {
            d3 = 0x59; d4 = 0x78;
        } else {
            d3 = 0x58; d4 = 0x71;
        }

        /* Find team rank in league table — search sorted team order for selTeamNumber.
         * ASM: a0 = leagueTeamIds, loop reads teamEntryOffsets,
         * divides by 18 to get team index, looks up in leagueTeamIds.
         * leagueTable is BE-packed (295c9f3); compare via BE_READ_U16. */
        uint16_t *teamEntryOffsets = comp->matchData.league.teamEntryOffsets;
        uint16_t *leagueTable = comp->matchData.league.leagueTeamIds;
        int16_t d1 = 0;
        do {
            int16_t tableEntry = (int16_t)teamEntryOffsets[d1];
            uint16_t teamIdx = (uint16_t)((uint32_t)(int32_t)tableEntry / 18);
            d1++;
        } while (BE_READ_U16(&leagueTable[(uint16_t)((uint32_t)(int32_t)(int16_t)teamEntryOffsets[d1 - 1] / 18)])
                 != _selTeamNumber);
        d1--;  /* convert to 0-based rank */

        int16_t numTeamsInDiv = comp->base.numTeams;  /* 0x32 */
        int16_t rankDelta = numTeamsInDiv - d1 - 1;
        int16_t numTeamsMinus1 = numTeamsInDiv - 1;
        uint16_t pct = (uint16_t)((uint32_t)((uint16_t)rankDelta * 100) / (uint16_t)numTeamsMinus1);
        int16_t range = d4 - d3;
        uint16_t factorFromRange = (uint16_t)((uint32_t)(pct * (uint16_t)range) / 100);
        int16_t receiptIndex = d3 + (int16_t)factorFromRange;
        _ballSpeed = (uint16_t)receiptIndex;

        /* Multiply by crowd factor from avgSkill (0x30), divide by avg player price.
         *
         * The divisor is GetAveragePlayerPrice(careerTeam) itself (SWOS2.s: `lea careerTeam,a0;
         * jsr GetAveragePlayerPrice; divu.w d0,d1`), the squad's average price INDEX -- not
         * the team value CalcGateReceiptsBaseFromTeamValue makes of it, which only the two
         * factor helpers use. Dividing by that value (thousands, not tens) left nothing:
         * league home games paid no gate at all, while cup games did (reports: "no gate
         * receipts for league games, only for cups"; fired for losing money). */
        int8_t crowdFactor = (int8_t)comp->base.avgSkill;  /* 0x30 */
        uint32_t product = (uint32_t)(uint16_t)receiptIndex * (uint16_t)(int16_t)crowdFactor;
        uint16_t avgPrice = GetAveragePlayerPrice(&_careerTeam);
        if (avgPrice == 0) avgPrice = 1; /* safety */
        uint16_t priceRatio = (uint16_t)(product / (uint32_t)avgPrice);
        /* muls.w d1,d7 is a 32-bit product, divs.w #100 then divides it: the product must not
         * be cut to 16 bits first, as it was. */
        int32_t gateProduct = (int32_t)(int16_t)d7 * (int32_t)(int16_t)priceRatio;
        int16_t gateFactorResult = (int16_t)(gateProduct / 100);
        _gateReceiptsFactor = gateFactorResult;
        d7 = (uint32_t)(uint16_t)gateFactorResult;
        goto compute_receipts;
    }

cup_match_type4:
    /* compType 4 falls through to cupMatchType1or2 */
cup_match_type1or2:
    d7 = calcCupMatchGateReceiptsFactor(team, opponent);
    {
        if (compType != 4) {
            /* compType 1 or 2: cup match */
            if (comp->base.competitionType != 1) {
                _panicReasonCode = 0x9236;
                PanicHaltLoop();
                return;
            }
        }
        int16_t totalRounds = comp->base.numRounds;          /* 0x5A */
        int16_t currentRound = comp->base.currentRoundIndex;  /* 0x58 */
        int16_t roundIndex = totalRounds - 1 - currentRound;
        d7 = (uint16_t)((uint32_t)(d7 * leagueMatchReceiptsMultiplierTable[roundIndex]) / 100);

        /* Check match result via roundLegs[currentRound] (0x66 + matchSlot) */
        if (comp->matchData.cup.roundLegs[currentRound] == 2)
            goto compute_receipts;
        goto increment_count;
    }

diy_comp_match:
    d7 = calcCupMatchGateReceiptsFactor(team, opponent);
    {
        int16_t isDiy = comp->base.hasGroups;  /* 0x48 */
        if (isDiy == 0) {
            /* Non-DIY comp type 3 */
            if (comp->base.competitionType == 0)
                goto compute_receipts;
            if (comp->base.competitionType == 1) {
                /* Cup match path */
                int16_t totalRounds = comp->base.numRounds;          /* 0x5A */
                int16_t currentRound = comp->base.currentRoundIndex;  /* 0x58 */
                int16_t roundIndex = totalRounds - 1 - currentRound;
                d7 = (uint16_t)((uint32_t)(d7 * cupMatchReceiptsMultiplierTable[roundIndex]) / 100);
                if (comp->matchData.cup.roundLegs[currentRound] == 2)
                    goto compute_receipts;
                goto increment_count;
            }
            _panicReasonCode = 0x9239;
            PanicHaltLoop();
            return;
        } else {
            /* DIY competition with group stages
             * NOTE: IDA bug at original L28446-28454. The original used
             * lea ($2DAD0).l,a0 which should be diyFileBufferCopy.
             * Also the second currentRound read used a0=cupMatchReceiptsMultiplierTable
             * but should have been diyFileBufferCopy->roundComplete (offset $54).
             */
            uint16_t totalRoundsField = (uint16_t)_diyFileBufferCopy.header.totalMatchesInRound;
            uint16_t curRoundField = (uint16_t)_diyFileBufferCopy.header.roundComplete;
            int16_t roundIndex = (int16_t)(totalRoundsField - 1 - curRoundField);
            d7 = (uint16_t)((uint32_t)(d7 * cupMatchReceiptsMultiplierTable[roundIndex]) / 100);

            /* Use curRoundField as group index to check match state via file buffer stride */
            DIYTournamentSlot *slot = (DIYTournamentSlot *)((uint8_t *)comp + curRoundField * 0x76 + 0x15A);
            if (slot->isLeagueFormat != 0)
                goto compute_receipts;
            if (slot->formatFlag16E == 2)
                goto compute_receipts;
            goto increment_count;
        }
    }

compute_receipts:
    if (team == &_careerTeam) {
        d7 <<= 1;   /* double for career team home matches */
        _seasonMatchReceiptsCount++;
    } else {
        d7 = 0;
        goto final_update;
    }

increment_count:
    _seasonMatchReceiptsCount++;

final_update:
    d7 &= 0xFFFF;
    d7 <<= 4;
    _gateReceipts += (int32_t)d7;
    _newBalance += (int32_t)d7;
    _seasonGateReceiptsAccum2 += (int32_t)d7;
    _seasonGateReceiptsAccum += (int32_t)d7;
    ClampNewBalanceAndUpdateClubStatus();
    ClearOverdraftStateIfRecovered();
}

/* ================================================================
 * Group D: Chairman warning system
 * ================================================================ */

/* MaybeSackManagerAfterSeasonReview */
__attribute__((used, externally_visible))
void MaybeSackManagerAfterSeasonReview(void) {
    if (_stayingInOldClub == 0)
        return;
    if (_wonWorldCup != 0)
        return;
    if (_wonContinentalCup != 0)
        return;
    if (_qualifyingStage1PendingFlag0 != 0)
        return;
    if (_qualifyingStage1PendingFlag1 != 0)
        return;
    if (_seasonReview < 2)
        return;

    uint16_t randVal = Rand();
    uint16_t threshold;
    if (_seasonReview == 2)
        threshold = 0x20;
    else if (_seasonReview == 3)
        threshold = 0x55;
    else
        threshold = 0xAA;

    if (randVal > threshold)
        return;

    _currentMessage = CHAIRMAN_WE_DIDNT_RENEW_YOUR_CONTRACT;
    ShowChairmanMessage();
    _youreSacked = 1;
}

/* Helper: check if secondary/tertiary comps are finished (blocks escalation) */
static uint16_t areActiveCompsFinished(void) {
    if (_secondaryCompetitionPtr != 0) {
        if (_competitionContextBuf.tourn.header.compFinished != 0)
            return 1;
    }
    if (_tertiaryCompetitionPtr != 0) {
        if (_tertiaryCompetitionContextBuf.tourn.header.compFinished != 0)
            return 1;
    }
    return 0;
}

/* UpdateCareerChairmanWarningsAfterMatch */
__attribute__((used, externally_visible))
void UpdateCareerChairmanWarningsAfterMatch(void) {
    if (_isNationalSelector != 0)
        return;

    /* Decrement cooldowns */
    if (_overdraftWarningCooldown != 0)
        _overdraftWarningCooldown--;
    if (_performanceWarningCooldown != 0)
        _performanceWarningCooldown--;

    /* Get team rank in DIY league */
    uint16_t d7 = FindTeamInDIY(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);

    /* Compute performance pressure factor */
    int16_t viewPhase = (int16_t)(int8_t)_viewRandomPhase;
    int16_t bias = (int16_t)_careerJobOfferPhaseBias;
    int16_t delta = (int16_t)(viewPhase - bias);
    int16_t pressureFactor = (int16_t)((int32_t)(delta << 2) / (int32_t)_wageScalingDivisor);

    if (pressureFactor == 0)
        goto skip_performance_warning;

    /* Check if league warnings are applicable */
    if (_primaryCompetitionBuf.tourn.header.competitionType != 2) {
        if (_primaryCompetitionBuf.tourn.leagueWarningsEnabled == 0)
            goto skip_performance_warning;
        /* Check if team is in relegation zone */
        uint16_t relegationBoundary = _primaryCompetitionBuf.tourn.header.promotionCount + _primaryCompetitionBuf.tourn.header.relegationCount;
        if (relegationBoundary == 0)
            relegationBoundary = 1;
        uint16_t boundary = _primaryCompetitionBuf.tourn.header.numTeams - relegationBoundary;
        if (d7 < boundary)
            goto check_expected_rank;
    }

    goto performance_warning_active;

check_expected_rank:
    {
        int16_t expRank = (int16_t)_expectedLeagueRankIndex;
        int16_t rankDiff = expRank - (int16_t)d7;
        int32_t pct = ((int32_t)rankDiff * 100) / (int32_t)(int16_t)_primaryCompetitionBuf.tourn.header.numTeams;
        if (pct > -25)
            goto clear_performance_warning;
    }

performance_warning_active:
    {
        uint16_t cooldown = _performanceWarningCooldown;
        if (_performanceWarningState == 0) {
            /* State 0: first warning */
            if (cooldown != 0)
                goto done_performance;
            _performanceWarningState = 1;
            setPerformanceWarningCooldown();
            _currentMessage = CHAIRMAN_IMPROVE_LEAGUE_POSITION;
            ShowChairmanMessage();
            return;
        }
        if (_performanceWarningState == 1) {
            /* State 1: second warning */
            if (cooldown != 0)
                goto done_performance;
            _performanceWarningState = 2;
            goto issue_improve_league_soon;
        }
        if (_performanceWarningState == 2) {
            /* State 2: third warning or escalate */
            if (cooldown != 0)
                goto done_performance;
            if (_overdraftWarningState >= 3)
                goto random_escalate;
            if (d7 == 0)
                goto random_escalate;
            if (_careerTeamLostMatch != 0)
                goto random_escalate;
            /* Check if secondary/tertiary comps block escalation */
            if (_secondaryCompetitionPtr != 0 && _competitionContextBuf.tourn.header.compFinished != 0)
                goto random_escalate;
            if (_tertiaryCompetitionPtr != 0 && _tertiaryCompetitionContextBuf.tourn.header.compFinished != 0)
                goto random_escalate;
            /* Issue 3-more-matches warning */
            _performanceWarningState = 3;
            setPerformanceWarningCooldown();
            _currentMessage = CHAIRMAN_THREE_MORE_MATCHES;
            ShowChairmanMessage();
            return;
        }
        /* State >= 3: countdown to sacking */
        if (d7 == 0) {
            _performanceWarningState = 3;
            goto done_performance;
        }
        _performanceWarningState++;
        if (_performanceWarningState < 6)
            goto done_performance;
        if (_careerTeamLostMatch != 0)
            goto done_performance;
        if (_competitionSlotUpdatedFlag != 0)
            goto done_performance;
        /* Sacked for poor performance */
        _currentMessage = CHAIRMAN_YOURE_SACKED_2;
        ShowChairmanMessage();
        _youreSacked = 1;
        return;
    }

random_escalate:
    /* 50% chance to escalate or re-issue warning */
    if ((_currentGameTick & 0x04) == 0) {
issue_improve_league_soon:
        setPerformanceWarningCooldown();
        _currentMessage = CHAIRMAN_IMPROVE_LEAGUE_POSITION_SOON;
        ShowChairmanMessage();
        return;
    }
    goto done_performance;

clear_performance_warning:
    _performanceWarningState = 0;

skip_performance_warning:
    setPerformanceWarningCooldown();

done_performance:
    /* Overdraft warning section */
    ClearOverdraftStateIfRecovered();

    if (_newBalance >= 0) {
        setOverdraftWarningCooldown();
        return;
    }

    /* Check if league warnings are applicable */
    if (_primaryCompetitionBuf.tourn.header.competitionType != 2) {
        if (_primaryCompetitionBuf.tourn.leagueWarningsEnabled == 0)
            return;
    }

    {
        uint16_t cooldown = _overdraftWarningCooldown;
        if (_overdraftWarningState == 0) {
            if (cooldown != 0)
                return;
            _overdraftWarningState = 1;
            setOverdraftWarningCooldown();
            _currentMessage = CHAIRMAN_BANK_BALANCE_OVERDRAFT;
            ShowChairmanMessage();
            return;
        }
        if (_overdraftWarningState == 1) {
            if (cooldown != 0)
                return;
            goto issue_overdraft_last_warning;
        }
        if (_overdraftWarningState == 2) {
            if (cooldown != 0)
                return;
            if (_performanceWarningState >= 3)
                goto issue_overdraft_last_warning;
            if (d7 == 0)
                goto issue_overdraft_last_warning;
            if (_careerTeamLostMatch != 0)
                goto issue_overdraft_last_warning;
            if (_secondaryCompetitionPtr != 0 && _competitionContextBuf.tourn.header.compFinished != 0)
                goto issue_overdraft_last_warning;
            if (_tertiaryCompetitionPtr != 0 && _tertiaryCompetitionContextBuf.tourn.header.compFinished != 0)
                goto issue_overdraft_last_warning;
            /* Clear overdraft immediately */
            _overdraftWarningState = 3;
            setOverdraftWarningCooldown();
            _currentMessage = CHAIRMAN_CLEAR_OVERDRAFT_IMMEDIATELY;
            ShowChairmanMessage();
            return;
        }
        /* State >= 3: countdown to sacking */
        if (d7 == 0) {
            _overdraftWarningState = 3;
            return;
        }
        _overdraftWarningState++;
        if (_overdraftWarningState < 6)
            return;
        if (_careerTeamLostMatch != 0)
            return;
        if (_competitionSlotUpdatedFlag != 0)
            return;
        _currentMessage = CHAIRMAN_YOURE_SACKED;
        ShowChairmanMessage();
        _youreSacked = 1;
        return;
    }

issue_overdraft_last_warning:
    _overdraftWarningState = 2;
    setOverdraftWarningCooldown();
    _currentMessage = CHAIRMAN_OVERDRAFT_LAST_WARNING;
    ShowChairmanMessage();
}

/* UpdateCareerAfterMatch — orchestrator */
__attribute__((used, externally_visible))
void UpdateCareerAfterMatch(void) {
    ClampNewBalanceAndUpdateClubStatus();
    ClearOverdraftStateIfRecovered();
    /* Do not sack a manager for results incurred before their arrival. */
    extern int Expanded_MidseasonJoin_Active(void);
    if (!Expanded_MidseasonJoin_Active())
        UpdateCareerChairmanWarningsAfterMatch();
    if (_removedOfferPlayersFromSelectedTeamCount != 0) {
        _removedOfferPlayersFromSelectedTeamCount--;
        AddCareerTeamPlayer();
    }
    CheckPlayersLeavingComing();
}

/* ================================================================
 * Group E: Weekly/season finances
 * ================================================================ */

/* ApplyWeeklyFinancialDeductions */
__attribute__((used, externally_visible))
void ApplyWeeklyFinancialDeductions(void) {
    uint8_t *listPtr = TeamSelectionKeyCache_GetCurrentListPtr();
    uint8_t currentWeek = listPtr[1];
    uint8_t lastWeek = _lastFinancialWeekIndex;
    int16_t diff = (int16_t)(uint16_t)currentWeek - (int16_t)(uint16_t)lastWeek;
    if (diff < 0)
        goto update_and_clamp;
    if (diff == 0)
        goto save_week_and_clamp;

    /* Deduct for (diff) weeks, counting from 0..diff-1 */
    int16_t weeks = diff - 1;
    for (int16_t i = weeks; i >= 0; i--) {
        int32_t wages = _weeklyPlayerWages;
        _playerWagesBill += wages;
        _newBalance -= wages;
        _seasonWagesAccum += wages;
        extern unsigned Expanded_AdministrationWeeklyCost(void);
        extern unsigned Expanded_ManagerWeeklyCost(void);
        int32_t running=(int32_t)(Expanded_AdministrationWeeklyCost()+
                                  Expanded_ManagerWeeklyCost());
        _generalRunningCosts += running;
        _newBalance -= running;
        _seasonRunningCostsAccum += running;
        _seasonRunningCostsAccum2 += running;
        _seasonWeeksDeducted++;
        { extern void Expanded_BusinessWeeklyStep(void);
          Expanded_BusinessWeeklyStep(); }
    }

save_week_and_clamp:
    listPtr = TeamSelectionKeyCache_GetCurrentListPtr();
    _lastFinancialWeekIndex = listPtr[1];

update_and_clamp:
    ClampNewBalanceAndUpdateClubStatus();
    ClearOverdraftStateIfRecovered();
}

/* GetExpectedLeagueRankIndex (static helper)
 * Computes expected league rank from competition context.
 * a6 = competition context buffer, uses d2 from GetMoreExpensiveTeamsCount.
 */
static uint16_t getExpectedLeagueRankIndex(uint8_t *ctx, uint16_t moreExpensiveCount) {
    DIYCompetitionEx *ctxEx = (DIYCompetitionEx *)ctx;
    DIYCompetition aligned_base;
    memcpy(&aligned_base, (const uint8_t *)ctxEx + offsetof(DIYCompetitionEx,base),
           sizeof aligned_base);
    const DIYCompetition *ctxBase = &aligned_base;
    uint16_t d3 = (uint16_t)ctxBase->numTeams;
    uint16_t compType = (uint16_t)ctxBase->competitionType;

    if (compType == 0) {
        /* League: (numTeams-1) * playEachTeam */
        uint16_t d7 = d3 - 1;
        d7 = (uint16_t)(d7 * (uint16_t)ctxBase->playEachTeam);
        return d7;
    }

    if (compType == 1) {
        /* Cup: walk through rounds halving d3, counting d7. roundPtr starts
         * at cup roundLegs[0] and advances by stride 2 (so roundPtr[i] iterates
         * through roundLegs[i]). */
        uint16_t d7 = 0;
        uint16_t *roundPtr = ctxEx->matchData.cup.roundLegs;
        for (;;) {
            d7++;
            if (*roundPtr++ == 2)
                d7++;
            d3 >>= 1;
            if (moreExpensiveCount >= d3)
                return d7;
            if (d3 == 1)
                return d7;
        }
    }

    /* DIY (compType >= 2): walk through groups */
    {
        uint16_t numGroups = (uint16_t)ctxBase->totalMatchesInRound;
        uint8_t *groupPtr = ctx;
        uint16_t d7 = 0;

        for (uint16_t i = numGroups; i > 0; i--) {
            DIYTournamentSlot *slot = (DIYTournamentSlot *)(groupPtr + 0x15A);
            if (slot->isLeagueFormat != 0) {
                uint16_t numRounds = slot->teamsPerGroup;
                uint16_t multiplier = slot->playEachTeam;
                uint16_t val = (numRounds - 1) * multiplier;
                d7 += val;
            } else {
                d7++;
                if (slot->formatFlag16E == 2)
                    d7++;
            }
            if (moreExpensiveCount >= slot->divisionFactor)
                return d7;
            groupPtr += 0x76;
        }
        return d7;
    }
}

/* InitializeCareerSeasonFinancials */
__attribute__((used, externally_visible))
void InitializeCareerSeasonFinancials(void) {
    /* Get more expensive teams count and set expected rank */
    uint16_t moreExpensiveCount = GetMoreExpensiveTeamsCount(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);
    _expectedLeagueRankIndex = moreExpensiveCount;

    /* Calculate team value divisor from all active competitions */
    uint16_t d5 = getExpectedLeagueRankIndex((uint8_t *)&_primaryCompetitionBuf, moreExpensiveCount);

    if (_secondaryCompetitionPtr != 0) {
        uint16_t mec2 = GetMoreExpensiveTeamsCount(_selTeamNumber, (DIYCompetition *)&_competitionContextBuf);
        d5 += getExpectedLeagueRankIndex((uint8_t *)&_competitionContextBuf, mec2);
    }
    if (_tertiaryCompetitionPtr != 0) {
        uint16_t mec3 = GetMoreExpensiveTeamsCount(_selTeamNumber, (DIYCompetition *)&_tertiaryCompetitionContextBuf);
        d5 += getExpectedLeagueRankIndex((uint8_t *)&_tertiaryCompetitionContextBuf, mec3);
    }
    if (_quaternaryCompetitionPtr != 0) {
        uint16_t mec4 = GetMoreExpensiveTeamsCount(_selTeamNumber, (DIYCompetition *)&_euroCupsCompetitionContextBuf);
        d5 += getExpectedLeagueRankIndex((uint8_t *)&_euroCupsCompetitionContextBuf, mec4);
    }

    _teamValueDivisor = d5;

    /* Compute total team value: weeklyPlayerWages * wageScalingDivisor */
    int32_t d6 = 0;
    int32_t wages = _weeklyPlayerWages;
    uint16_t scalingCount = _wageScalingDivisor;
    for (uint16_t i = 0; i < scalingCount; i++)
        d6 += wages;
    _totalTeamValue = d6;

    /* Compute weekly running costs from total team value */
    uint32_t d0 = (uint32_t)d6;
    d0 = (uint16_t)(d0 / 1000);           /* divu #$3E8 */
    d0 = (uint32_t)(uint16_t)d0 * 207;    /* mulu #$CF */
    d0 = (uint16_t)(d0 / 6144);           /* divu #$1800 */
    d0 = (uint32_t)(uint16_t)d0 * 96;     /* mulu #$60 */
    d0 = (uint16_t)(d0 / _wageScalingDivisor); /* divu wageScalingDivisor */
    d0 = (uint32_t)(uint16_t)d0 * 1000;   /* mulu #$3E8 */
    /* Expanded: replace native OTHER overhead with 45K/year administration
     * and the real contracted manager salary. Tier scale is TBD. */
    { extern unsigned Expanded_AdministrationWeeklyCost(void);
      extern unsigned Expanded_ManagerWeeklyCost(void);
      _weeklyRunningCosts=(int32_t)(Expanded_AdministrationWeeklyCost()+
                                     Expanded_ManagerWeeklyCost()); }

    /* Clear accumulators */
    _seasonRunningCostsAccum = 0;
    _seasonWagesAccum = 0;
    _seasonGateReceiptsAccum = 0;
    _seasonMatchReceiptsCount = 0;
    _seasonWeeksDeducted = 0;
    _seasonGateReceiptsAccum2 = 0;
    _seasonRunningCostsAccum2 = 0;

    /* Handle staying/leaving club balance adjustments */
    if (_stayingInOldClub == 0) {
        int32_t bal = _startingBalanceCopy + _competitionBonuses;
        _old_balance += bal;
        _startingBalanceCopy = 0;
        _competitionBonuses = 0;
    }

    /* Reset warning states */
    _overdraftStreak = 0;
    _overdraftWarningState = 0;
    _performanceWarningState = 0;
    _overdraftWarningCooldown = 0;
    _performanceWarningCooldown = 0;
}

/* ================================================================
 * Group F: Board confidence system
 * ================================================================ */

/* CalcBoardConfidenceDeltaPercent (static) */
static int16_t calcBoardConfidenceDeltaPercent(void) {
    if (_isNationalSelector != 0) {
        if (_seasonFinished == 0)
            return (int16_t)_boardConfidenceHistory[3];
        return seasonReviewConfidenceDeltaTable[_seasonReview];
    }

    /* Club manager: compute from league rank delta */
    uint16_t teamRank = FindTeamInDIY(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);
    int16_t d0 = (int16_t)_expectedLeagueRankIndex - (int16_t)teamRank;
    int16_t numTeams = ((DIYCompetitionEx *)&_primaryCompetitionBuf)->base.numTeams;
    d0 = (int16_t)(((int32_t)d0 * 100) / (int32_t)numTeams);
    return d0;
}

/* GetBoardConfidenceScore */
__attribute__((used, externally_visible))
int16_t GetBoardConfidenceScore(void) {
    int16_t d0 = calcBoardConfidenceDeltaPercent();
    d0 += (int16_t)_boardConfidenceHistory[0];
    d0 += (int16_t)_boardConfidenceHistory[1];
    d0 += (int16_t)_boardConfidenceHistory[2];
    d0 += _boardConfidenceScore;
    return d0;
}

/* UpdateBoardConfidenceScore */
__attribute__((used, externally_visible))
int16_t UpdateBoardConfidenceScore(void) {
    int16_t d0 = _boardConfidenceScore;

    if (_majorTrophyBonusActive != 0)
        d0 += 100;
    if (_cupBonus2Active != 0)
        d0 += 50;
    if (_cupBonus1Active != 0)
        d0 += 50;
    if (_playWinCupAnimation != 0 && _currentDivisionIndex == 0)
        d0 += 50;
    if (_minorTrophyBonusActive != 0)
        d0 += 25;
    if (_wonWorldCup != 0)
        d0 += 150;
    if (_wonContinentalCup != 0)
        d0 += 100;

    if (d0 > 200)
        d0 = 200;

    _boardConfidenceScore = d0;
    return (uint16_t)d0;
}

/* PushBoardConfidenceHistory */
__attribute__((used, externally_visible))
int16_t PushBoardConfidenceHistory(void) {
    _boardConfidenceHistory[3] = _boardConfidenceHistory[2];
    _boardConfidenceHistory[2] = _boardConfidenceHistory[1];
    _boardConfidenceHistory[1] = _boardConfidenceHistory[0];
    int16_t d0 = calcBoardConfidenceDeltaPercent();
    _boardConfidenceHistory[0] = (uint16_t)d0;
    return d0;
}

/* UpdateBoardConfidenceModifiers */
__attribute__((used, externally_visible))
void UpdateBoardConfidenceModifiers(void) {
    _qualifyingStage1PendingFlag0 = 0;
    _qualifyingStage1PendingFlag1 = 0;
    _qualifyingStage2PendingFlag0 = 0;
    _qualifyingStage2PendingFlag1 = 0;

    if (_isNationalSelector == 0)
        goto check_winners;

    /* National selector: check World Cup qualifying */
    if ((_someTeamFlags & 0x40) == 0)
        goto check_ec_qualifying;

    {
        uint16_t team = _selTeamNumber;
        if (team == _worldCupQualifyingTeam || team == _worldCupHostTeam)
            goto set_qual1_flag0;

        uint8_t curId = _currentCompetitionPtr ? comp_id((void *)_currentCompetitionPtr) : 0xFFu;
        if (!comp_is_wc_qualifier(curId))
            goto check_ec_qualifying;

        /* In WC qualifying: find team rank and determine qualifying slots */
        uint16_t teamRank = FindTeamInDIY(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);
        uint16_t qualSlots;

        if (curId == COMPID_northAmericaWorldCupQualification)
            qualSlots = 3;
        else if (curId == COMPID_southAmericaWorldCupQualification)
            qualSlots = 4;
        else if (curId == COMPID_africaWorldCupQualification)
            qualSlots = 5;
        else if (curId == COMPID_asiaWorldCupQualification)
            qualSlots = 2;
        else if (curId == COMPID_oceaniaWorldCupQualification)
            qualSlots = 1;
        else if (curId == COMPID_worldCupQualifyingGroup8)
            qualSlots = 1;
        else
            qualSlots = 2;

        if (_diyFoundTeamIndex != 0) {
            goto set_qual2_flag0;
        }
        if (_diyCompHasGroupStage == 0)
            goto check_ec_qualifying;
        if (teamRank >= qualSlots)
            goto set_qual2_flag0;
    }

set_qual1_flag0:
    _qualifyingStage1PendingFlag0 = 1;
    goto check_ec_qualifying;

set_qual2_flag0:
    _qualifyingStage2PendingFlag0 = 1;

check_ec_qualifying:
    if ((_someTeamFlags & 0x01) == 0)
        goto check_winners;

    {
        uint16_t team = _selTeamNumber;
        if (team == _euroChampionshipQualTeam)
            goto set_qual1_flag1;

        uint8_t curId = _currentCompetitionPtr ? comp_id((void *)_currentCompetitionPtr) : 0xFFu;
        if (!comp_is_ec_qualifier(curId))
            goto check_winners;

        uint16_t teamRank = FindTeamInDIY(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);
        uint16_t qualSlots;
        if (curId == COMPID_ecQualifyingGroup8)
            qualSlots = 1;
        else
            qualSlots = 2;

        if (_diyFoundTeamIndex != 0)
            goto set_qual2_flag1;
        if (_diyCompHasGroupStage == 0)
            goto check_winners;
        if (teamRank >= qualSlots)
            goto set_qual2_flag1;
    }

    _qualifyingStage1PendingFlag1 = 1;
    goto check_winners;

set_qual1_flag1:
    _qualifyingStage1PendingFlag1 = 1;
    goto check_winners;

set_qual2_flag1:
    _qualifyingStage2PendingFlag1 = 1;

check_winners:
    /* Clear trophy flags */
    _wonWorldCup = 0;
    _wonContinentalCup = 0;
    _wonInvitationTournament = 0;
    _playWinCupAnimation = 0;

    /* Check if we won the primary competition */
    if (_selTeamNumber != _primaryCompetitionBuf.tourn.header.winnerTeam)
        goto check_secondary_winner;
    {
        uint8_t curId = _currentCompetitionPtr ? comp_id((void *)_currentCompetitionPtr) : 0xFFu;
        if (curId == COMPID_friendly)
            goto check_secondary_winner;
        if (curId == COMPID_tour)
            goto check_secondary_winner;
        if (comp_is_ec_qualifier(curId))      /* Skip if EC qualifying */
            goto check_secondary_winner;
        if (comp_is_wc_qualifier(curId))      /* Skip if WC qualifying */
            goto check_secondary_winner;

        /* Determine which trophy was won */
        if (curId == COMPID_invitationTournament) {
            _wonInvitationTournament = 1;
        } else if (curId == COMPID_worldCup2) {
            _wonWorldCup = 1;
        } else if (comp_is_continental_cup(curId)) {
            _wonContinentalCup = 1;
        } else {
            _playWinCupAnimation = 1;
        }
    }

check_secondary_winner:
    _minorTrophyBonusActive = 0;

    if (_secondaryCompetitionPtr == 0)
        goto check_tertiary_winner;
    if (_selTeamNumber != _competitionContextBuf.tourn.header.winnerTeam)
        goto check_tertiary_winner;
    {
        /* _secondaryCompetitionPtr is non-NULL here (guarded above). */
        uint8_t secId = comp_id((void *)_secondaryCompetitionPtr);
        if (secId == COMPID_friendly)
            goto check_tertiary_winner;
        /* NOTE: original tests the PRIMARY ptr against tour here, not the
         * secondary — preserved verbatim (faithful to SWOS2.s). */
        if (_currentCompetitionPtr && comp_id((void *)_currentCompetitionPtr) == COMPID_tour)
            goto check_tertiary_winner;

        if (secId == COMPID_invitationTournament) {
            _wonInvitationTournament = 1;
        } else if (comp_is_continental_cup(secId)) {
            _wonContinentalCup = 1;
        } else {
            _minorTrophyBonusActive = 1;
        }
    }

check_tertiary_winner:
    _leagueTitleBonusActive = 0;

    if (_tertiaryCompetitionPtr != 0 &&
        _selTeamNumber == _tertiaryCompetitionContextBuf.tourn.header.winnerTeam &&
        comp_id((void *)_tertiaryCompetitionPtr) != COMPID_friendly) {  /* non-NULL: guarded by && above */
        _leagueTitleBonusActive = 1;
    }

    /* Check quaternary (Euro cups) */
    _majorTrophyBonusActive = 0;
    _cupBonus1Active = 0;
    _cupBonus2Active = 0;

    if (_quaternaryCompetitionPtr == 0)
        return;
    if (_selTeamNumber != _euroCupsCompetitionContextBuf.tourn.header.winnerTeam)
        return;

    if ((_euroCupSlotIndex == 7 || _euroCupSlotIndex == 8) && Conti_TrophyTier() < 0)
        return;                         /* (another confederation's tie: a trophy only as its cup's final) */
    if (_euroCupSlotIndex == 9)
        return;                         /* (a world's group contest: a title, not a cup) */
    const uint16_t euroIdx = _euroCupSlotIndex == 7 || _euroCupSlotIndex == 8 ? (uint16_t)Conti_TrophyTier()
                           : _euroCupSlotIndex >= 4 ? _euroCupSlotIndex - 4 : _euroCupSlotIndex;
    if (euroIdx == 0 || euroIdx == 3) {  /* (4..6: a world's qualifying tie, its cup's tier) */
        /* European Cup (0) or Copa Libertadores (3): the premier continental club
         * cup -> major trophy bonus. */
        _majorTrophyBonusActive = 1;
    } else if (euroIdx == 2) {
        _cupBonus2Active = 1;
    } else {
        _cupBonus1Active = 1;
    }
}

/* ================================================================
 * Group G: Reputation + starting balance
 * ================================================================ */

/* AddReputationFromBonusValueDiv4 (static) */
static void addReputationFromBonusValueDiv4(int8_t bonusValue) {
    int16_t val = (int16_t)bonusValue;
    val >>= 2;  /* lsr.w #2 - but ext.w d0 first, so this is unsigned shift */
    _careerReputationPoints += val;
}

/* AddReputationFromBonusValueDiv8 (static) */
static void addReputationFromBonusValueDiv8(int8_t bonusValue) {
    int16_t val = (int16_t)bonusValue;
    val >>= 3;
    _careerReputationPoints += val;
}

/* UpdateCareerReputationBonuses */
__attribute__((used, externally_visible))
void UpdateCareerReputationBonuses(void) {
    /* National selector reputation delta */
    if (_isNationalSelector != 0) {
        int16_t delta = nationalSelectorSeasonReviewReputationDeltaTable[_seasonReview];
        _nationalSelectorReputationPoints += delta;
        if (_nationalSelectorReputationPoints < 0)
            _nationalSelectorReputationPoints = 0;
        if (_nationalSelectorReputationPoints > 49)
            _nationalSelectorReputationPoints = 49;
    }

    /* Club reputation from league rank */
    DIYCompetitionEx *cfbEx = (DIYCompetitionEx *)&_primaryCompetitionBuf;
    if (cfbEx->base.competitionType != 2) {
        /* Not a pure cup — compute reputation from rank delta */
        uint16_t avgPrice = getTeamValueForGateReceipts(&_careerTeam);
        int16_t d7 = (int16_t)avgPrice;

        /* Find team rank in league table.
         * leagueTable is BE-packed (295c9f3 convention) — compare via
         * BE_READ_U16 against selTeamNumber, which the rest of the code
         * keeps in (country<<8)|ordinal form. Without this the loop
         * compares the host-native byte-swap of each entry and never
         * matches → hang on FINISH SEASON. */
        uint16_t *leagueTable = cfbEx->matchData.league.leagueTeamIds;
        int16_t *teamEntryOffsets = (int16_t *)cfbEx->matchData.league.teamEntryOffsets;
        int16_t d1 = -1;
        do {
            d1++;
            int16_t entry = teamEntryOffsets[d1];
            int32_t extended = (int32_t)entry;
            uint16_t divided = (uint16_t)((uint32_t)extended / 18);
            (void)divided;
        } while (BE_READ_U16(&leagueTable[(uint16_t)((uint32_t)(int32_t)teamEntryOffsets[d1] / 18)])
                 != _selTeamNumber);
        /* d1 is now the rank (0-based index into teamEntryOffsets) */

        int16_t rankDelta = (int16_t)_expectedLeagueRankIndex - d1;
        int16_t numTeams = cfbEx->base.numTeams;
        int16_t pct = (int16_t)(((int32_t)rankDelta * 100) / (int32_t)numTeams);
        int16_t repDelta = (int16_t)(((int32_t)pct * (int32_t)d7) / 200);
        _careerReputationPoints += repDelta;
    }

    /* Trophy reputation bonuses */
    if (_wonWorldCup != 0)
        addReputationFromBonusValueDiv4((int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue);

    if (_wonContinentalCup != 0) {
        /* Major continental (primary slot) vs minor (secondary context) */
        uint8_t curId = _currentCompetitionPtr ? comp_id((void *)_currentCompetitionPtr) : 0xFFu;
        int8_t bonusVal;
        if (comp_is_continental_cup(curId)) {
            bonusVal = (int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue;
        } else {
            bonusVal = (int8_t)_competitionContextBuf.tourn.header.trophyBonusValue;
        }
        addReputationFromBonusValueDiv4(bonusVal);
    }

    if (_playWinCupAnimation != 0) {
        _winCupBonusCount++;
        addReputationFromBonusValueDiv4((int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue);
    }

    if (_minorTrophyBonusActive != 0) {
        _minorTrophyBonusCount++;
        addReputationFromBonusValueDiv4((int8_t)_competitionContextBuf.tourn.header.trophyBonusValue);
    }

    if (_leagueTitleBonusActive != 0)
        addReputationFromBonusValueDiv8((int8_t)_tertiaryCompetitionContextBuf.tourn.header.trophyBonusValue);

    if (_majorTrophyBonusActive != 0 || _cupBonus1Active != 0 || _cupBonus2Active != 0) {
        _majorTrophyBonusCount++;
        addReputationFromBonusValueDiv4((int8_t)_euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue);
    }
}

/* InitializeStartingBalanceForNewSeason */
__attribute__((used, externally_visible))
void InitializeStartingBalanceForNewSeason(void) {
    /* Get team value base from avg player price */
    uint16_t avgPrice = getTeamValueForGateReceipts(&_careerTeam);
    uint16_t teamBase = GetTeamValueBase(avgPrice);
    uint16_t d2 = teamBase;

    /* Compute adjusted starting balance from expected rank */
    uint16_t teamRank = FindTeamInDIY(_selTeamNumber, (DIYCompetition *)&_primaryCompetitionBuf);
    int16_t rankDelta = (int16_t)_expectedLeagueRankIndex - (int16_t)teamRank;
    int16_t pct = (int16_t)(((int32_t)rankDelta * 100) / (int32_t)(int16_t)_primaryCompetitionBuf.tourn.header.numTeams);
    pct >>= 1;   /* asr.w #1 */
    pct += 100;   /* addi.w #$64 */
    uint16_t adjustedBase = (uint16_t)(((uint32_t)(uint16_t)pct * (uint32_t)d2) / 100);

    /* Add random factor (90-110%) */
    uint16_t r = Rand2();
    uint16_t randPct = (uint16_t)((r * 20) >> 8) + 90;  /* 90 to ~110 */
    uint16_t finalBase = (uint16_t)(((uint32_t)randPct * (uint32_t)adjustedBase) / 100);
    _startingBalance = (int32_t)((uint32_t)finalBase * 1000);

    /* Cup win bonuses by division */
    if (_playWinCupAnimation != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue;
        if (_currentDivisionIndex == 0) {
            /* Division 0: add twice (once extra) */
            _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * 0xEDC8);
            _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * 0xEDC8);
        } else {
            uint32_t multiplier;
            if (_currentDivisionIndex == 1)
                multiplier = 0xB720;
            else if (_currentDivisionIndex == 2)
                multiplier = 0x7FDD;
            else
                multiplier = 0x61A8;
            _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * multiplier);
        }
    }

    /* Minor trophy bonus */
    if (_minorTrophyBonusActive != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_competitionContextBuf.tourn.header.trophyBonusValue;
        _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * 0xCED0);
    }

    /* League title bonus */
    if (_leagueTitleBonusActive != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_tertiaryCompetitionContextBuf.tourn.header.trophyBonusValue;
        _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * 0x44F1);
    }

    /* Major trophy / Euro cup bonuses */
    if (_majorTrophyBonusActive != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue;
        uint32_t amount = (uint32_t)(uint16_t)bonusVal * 0xF426;
        _startingBalance += (int32_t)amount;
        _startingBalance += (int32_t)amount;  /* added twice in asm */
    }

    if (_cupBonus1Active != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue;
        uint32_t amount = (uint32_t)(uint16_t)bonusVal * 0x7A14;
        _startingBalance += (int32_t)amount;
        _startingBalance += (int32_t)amount;
    }

    if (_cupBonus2Active != 0) {
        int16_t bonusVal = (int16_t)(int8_t)_euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue;
        uint32_t amount = (uint32_t)(uint16_t)bonusVal * 0xB190;
        _startingBalance += (int32_t)amount;
        _startingBalance += (int32_t)amount;
    }

    /* Promoted bonus (only if no cup win) */
    if (_playWinCupAnimation != 0)
        return;
    if (_playPromotedAnimation == 0)
        return;

    {
        int16_t bonusVal = (int16_t)(int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue;
        if (_currentDivisionIndex == 0) {
            /* Cannot be promoted to division 0 — panic */
            _panicReasonCode = 0x1782;
            PanicHaltLoop();
            return;
        }
        uint32_t multiplier;
        if (_currentDivisionIndex == 1)
            multiplier = 0x6DE0;
        else if (_currentDivisionIndex == 2)
            multiplier = 0x4709;
        else
            multiplier = 0x30D4;
        _startingBalance += (int32_t)((uint32_t)(uint16_t)bonusVal * multiplier);
    }
}

/* ================================================================
 * recalcTotalWeeklyWages  (SWOS2.s line 86716, loc_14B3AE)
 *
 * Iterates all career team players and sums their cached
 * weekly wages into weeklyPlayerWages.
 * Shared by Career_UpdatePlayerWeeklyWageCache (fall-through)
 * and Career_RemovePlayerWeeklyWageAndRecalcWeeklyWages.
 * ================================================================ */
static void recalcTotalWeeklyWages(void)
{
    int32_t total = 0;
    int16_t i;
    for (i = (int16_t)(_careerNumPlayers - 1); i >= 0; i--) {
        GetPlayerAtIndex(_selTeamNumber, (uint16_t)i);
        uint16_t fileIdx = _cGetPlayerAtIndexFileIdx;
        total += _weeklyWageByPlayerIndex[fileIdx];
    }
    _weeklyPlayerWages = total;
}

/* ================================================================
 * GetAvgTeamValueBaseForCurrentPlayer  (SWOS2.s line 71480)
 *
 * Gets the average team value base for a player.
 * Calls GetPlayer to get player pointer, then GetDisplayedPlayerValue
 * to get the displayed value (1-80), and indexes into avgTeamValueBases.
 *
 * In asm, d0=teamNumber and d1=playerIndex are set by caller.
 * We take playerIndex as a parameter and read selTeamNumber internally,
 * matching the actual calling convention where d0 is always selTeamNumber.
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t GetAvgTeamValueBaseForCurrentPlayer(uint16_t teamNumber, uint16_t playerIndex)
{
    uint8_t *player = GetPlayer(teamNumber, playerIndex);
    uint16_t displayedValue = GetDisplayedPlayerValue(player);
    displayedValue &= 0xFF;
    return _avgTeamValueBases[displayedValue];
}

/* ================================================================
 * Career_UpdatePlayerWeeklyWageCache  (SWOS2.s line 86678)
 *
 * Calculates and stores the weekly wage for a single player.
 * Steps:
 *   1. Get average team value base for the player
 *   2. Check player status flags for wage reduction
 *      (0x9B = trialist / "TRIAL" → 33%, 0x9A = reserve / "RES" → 66%)
 *   3. Apply scaling: value * 1000 / 768 * 96 / wageScalingDivisor
 *   4. Store in weeklyWageByPlayerIndex[playerIndex]
 *   5. Recalculate total weekly wages
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_UpdatePlayerWeeklyWageCache(uint16_t playerIndex)
{
    uint16_t avgBase = GetAvgTeamValueBaseForCurrentPlayer(_selTeamNumber, playerIndex);

    /* Get player pointer to check status flags */
    uint8_t *player = GetPlayer(_selTeamNumber, playerIndex);
    PlayerFile *pf = (PlayerFile *)(player + 76);
    uint8_t someFlag = (uint8_t)pf->form;

    if (someFlag == PLAYER_FORM_TRIALIST) {
        /* Trialist: reduce wage (default 33%) */
        avgBase = (uint16_t)((uint32_t)avgBase * _careerParams.wage_trialist_pct / 100);
    } else if (someFlag == PLAYER_FORM_RESERVE
#ifdef SWOS_SDL_BUILD
               || SquadExtras_IsOwnExtra(pf)
#endif
              ) {
        /* OG reserve, or a real 2026/27 player 17..30 on his original club. */
        avgBase = (uint16_t)((uint32_t)avgBase * _careerParams.wage_reserve_pct / 100);
    }

    /* Scale: value * 1000 / 768 * scale / wageScalingDivisor */
    uint32_t wage = (uint32_t)avgBase * 1000;
    wage = wage / 768;
    wage = wage * _careerParams.wage_scale;
    wage = wage / (uint32_t)_wageScalingDivisor;
    wage &= 0xFFFF;  /* keep 16-bit quotient (68k divu behavior) */

    _weeklyWageByPlayerIndex[playerIndex] = (int32_t)wage;

    recalcTotalWeeklyWages();
}

/* ================================================================
 * Career_RemovePlayerWeeklyWageAndRecalcWeeklyWages  (SWOS2.s line 86711)
 *
 * Clears the weekly wage entry for a player and recalculates
 * the total weekly wages.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_RemovePlayerWeeklyWageAndRecalcWeeklyWages(uint16_t playerIndex)
{
    _weeklyWageByPlayerIndex[playerIndex] = 0;
    recalcTotalWeeklyWages();
}

/* ================================================================
 * ShowXxxMessage functions  (SWOS2.s lines 38068-38130)
 *
 * Each selects a message ID based on isNationalSelector
 * (national vs club chairman messages), sets currentMessage,
 * then calls ShowChairmanMessage.
 * ================================================================ */

/* ShowLeavingMessage — "Thanks for all your help" */
__attribute__((used, externally_visible))
void ShowLeavingMessage(void)
{
    _currentMessage = _isNationalSelector ? 32 : 18;
    ShowChairmanMessage();
}

/* ShowGreetingMessage — "Welcome, we look forward..." */
__attribute__((used, externally_visible))
void ShowGreetingMessage(void)
{
    _currentMessage = _isNationalSelector ? 33 : 19;
    if (!_isNationalSelector) {
        extern void Expanded_EnsureClubSentiment(void);
        Expanded_EnsureClubSentiment();
        /* One-shot test build: the very first welcome shows 19+20=39.
         * The sacking popup opens before the next match is played. */
        extern void Expanded_TriggerFirstWelcomeSackingDemo(void);
        Expanded_TriggerFirstWelcomeSackingDemo();
    }
    ShowChairmanMessage();
}

/* ShowLastSeasonMessage — "Enjoy your last season" */
__attribute__((used, externally_visible))
void ShowLastSeasonMessage(void)
{
    _currentMessage = _isNationalSelector ? 34 : 20;
    ShowChairmanMessage();
}

/* ShowWishYouSuccessMessage — "Wish you success" */
__attribute__((used, externally_visible))
void ShowWishYouSuccessMessage(void)
{
    _currentMessage = _isNationalSelector ? 35 : 21;
    ShowChairmanMessage();
}

/* ShowEnjoyLastSeasonMessage — "Enjoy last season before..." */
__attribute__((used, externally_visible))
void ShowEnjoyLastSeasonMessage(void)
{
    _currentMessage = _isNationalSelector ? 36 : 22;
    ShowChairmanMessage();
}

/* ShowRetirementMessage — "Enjoy your retirement" */
__attribute__((used, externally_visible))
void ShowRetirementMessage(void)
{
    _currentMessage = _isNationalSelector ? 37 : 23;
    ShowChairmanMessage();
}

/* ================================================================
 * CalculateStartingBalance  (SWOS2.s line 86092)
 *
 * Calculates career starting balance for a given team.
 * Gets team value base * 3, applies random percentage (80-119%),
 * divides by 100 and multiplies by 1000 to get final balance.
 *
 * Asm: CalculateStartingBalance (d0=teamNumber → d1=balance)
 * ================================================================ */
extern TeamFileEx *GetTeamPointer(uint16_t teamId);
extern uint16_t GetAveragePlayerPrice(TeamFileEx *team);

__attribute__((used, externally_visible))
uint32_t CalculateStartingBalance(uint16_t teamNumber)
{
    TeamFileEx *teamPtr = GetTeamPointer(teamNumber);
    uint16_t avgPrice = GetAveragePlayerPrice(teamPtr);
    uint16_t baseValue = GetTeamValueBase(avgPrice);
    uint16_t base3 = (uint16_t)(baseValue * _careerParams.start_balance_value_mult);
    uint16_t r = Rand2();
    uint16_t randFactor = (uint16_t)((uint16_t)(r * _careerParams.start_balance_pct_range) >> 8);
    uint16_t pct = (uint16_t)(randFactor + _careerParams.start_balance_pct_min);  /* default 80-119 */
    uint32_t product = (uint32_t)pct * (uint32_t)base3;
    uint16_t divided = (uint16_t)(product / 100);                /* divu.w quotient */
    return (uint32_t)divided * _careerParams.start_balance_scale;
}

/* ================================================================
 * Chunk 95 — Chairman message helpers, post-match stat updates,
 *            CheckPlayersLeavingComing, AddTrialPlayer
 * ================================================================ */

/* Message ID constants for chairman messages */
#define MSG_TRIAL_PLAYER_LEFT_CLUB        24
#define MSG_RESERVE_PLAYER_LEFT_CLUB      25
#define MSG_PLAYER_JOINED_CLUB_ON_TRIAL   26

extern uint8_t *_plNameForMessage;
extern uint8_t *GetPlayer(uint16_t teamNumber, uint16_t playerFileIndex);
extern uint8_t *GetPlayerAtIndex(uint16_t teamNumber, uint16_t ordinal);
extern void Career_UpdatePlayerWeeklyWageCache(uint16_t playerIndex);
extern void RemovePlayerFromTeam(uint16_t playerOrd);

/* asm bridges for functions not yet ported */
extern void Career_ConvertPlayerToTrialistFromName(uint16_t teamNum, uint16_t playerIdx);
extern void TrainingMatch_SaveSelectedPlayerId(uint16_t value);

/* competitionFileBuffer migrated to primaryCompetitionBuf (slot-0 untangle, S6); buffer extern declared above */


/* ================================================================
 * PlayerJoinedClubOnTrial — show chairman message for player joining on trial.
 * Asm: PlayerJoinedClubOnTrial (a1=name ptr)
 * ================================================================ */
__attribute__((used, externally_visible))
void PlayerJoinedClubOnTrial(uint8_t *name)
{
    _plNameForMessage = name;
    _currentMessage = MSG_PLAYER_JOINED_CLUB_ON_TRIAL;
    ShowChairmanMessage();
}

/* ================================================================
 * TrialPlayerLeftClubMessage — show chairman message for trial player leaving.
 * Asm: TrialPlayerLeftClubMessage (a1=name ptr)
 * ================================================================ */
__attribute__((used, externally_visible))
void TrialPlayerLeftClubMessage(uint8_t *name)
{
    _plNameForMessage = name;
    _currentMessage = MSG_TRIAL_PLAYER_LEFT_CLUB;
    ShowChairmanMessage();
}

/* ================================================================
 * ReservePlayerLeftClubMessage — show chairman message for reserve player leaving.
 * Asm: ReservePlayerLeftClubMessage (a1=name ptr)
 * ================================================================ */
__attribute__((used, externally_visible))
void ReservePlayerLeftClubMessage(uint8_t *name)
{
    _plNameForMessage = name;
    _currentMessage = MSG_RESERVE_PLAYER_LEFT_CLUB;
    ShowChairmanMessage();
}

/* ================================================================
 * Career_UpdateSinglePlayerPostMatchStats — update one player's post-match form.
 * Asm: Career_UpdateSinglePlayerPostMatchStats (a6=player raw ptr within careerTeam)
 *
 * The player pointer is within careerTeam; index = (ptr - careerTeam) / 38.
 * Handles form transitions: $9B→$9A, $9A→restored form (20% chance),
 * in-form players get +2 to form clamped at 100.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_UpdateSinglePlayerPostMatchStats(uint8_t *playerSlotPtr)
{
    if (_isNationalSelector)
        return;

    /* Compute player index from raw offset */
    uint32_t offset = (uint32_t)(playerSlotPtr - (uint8_t *)&_careerTeam);
    uint16_t playerIdx = (uint16_t)(offset / 38);

    PlayerFile *pf = (PlayerFile *)(playerSlotPtr + 76);
    uint8_t positionAndFace = pf->positionAndFace;
    uint8_t form = (uint8_t)pf->form;

    if (!(positionAndFace & PF_INFORM)) {
        /* Player is NOT in form (bit 0 clear) */
        if (form == PLAYER_FORM_TRIALIST) {
            /* Trialist hired → demote to "on trial" ($9A) */
            pf->form = (int8_t)PLAYER_FORM_RESERVE;
            Career_UpdatePlayerWeeklyWageCache(playerIdx);
            return;
        }

        if (form != PLAYER_FORM_RESERVE)
            return;  /* Not trial — nothing to do */

        /* Form = $9A (on trial) — ~20% chance to promote */
        if (Rand() >= 0x33)
            return;

        /* Restore form from saved modifier table and set in-form bit */
        pf->form = (int8_t)_playerFormModifiers[playerIdx];
        pf->positionAndFace = positionAndFace | PF_INFORM;  /* bset #0 */
        Career_UpdatePlayerWeeklyWageCache(playerIdx);
        return;
    }

    /* Player is in form (bit 0 set) */
    form = (uint8_t)(form + 2);
    pf->form = (int8_t)form;

    if ((int8_t)form < 0) {
        /* Overflowed past 127 → clear in-form bit */
        pf->positionAndFace = positionAndFace & (uint8_t)~PF_INFORM;
    }

    /* Clamp form at 100 ($64) */
    if ((int8_t)pf->form > PLAYER_FORM_MAX) {
        pf->form = PLAYER_FORM_MAX;
    }
}

/* ================================================================
 * Career_ApplyPostMatchPlayerStatModsIfCareerTeam — apply post-match
 * stat mods to career team players.
 * Asm: Career_ApplyPostMatchPlayerStatModsIfCareerTeam (a0=team, a1=mods)
 *
 * Only processes if NOT national selector and team IS careerTeam.
 * Iterates 11 players (d7=10..0), uses currentMatchPlayers[d7] as
 * player file index, checks remap[fileIndex] — if negative, 20% chance
 * to use remap value as replacement modifier.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_ApplyPostMatchPlayerStatModsIfCareerTeam(TeamFileEx *team, uint8_t *mods)
{
    if (_isNationalSelector)
        return;
    if (team != &_careerTeam)
        return;

    uint8_t *teamRaw = (uint8_t *)team;
    int16_t d7;
    for (d7 = 0x0A; d7 >= 0; d7--) {
        int8_t d6 = (int8_t)_currentMatchPlayers[d7];
        uint8_t modByte = mods[(uint8_t)d6];

        if (!((int8_t)modByte < 0)) {
            /* modByte is non-negative — check 12.5% chance */
            if (Rand() >= 0x20)
                goto do_update;
            d6 = (int8_t)modByte;
        }

    do_update:
        {
            uint16_t d2 = (uint16_t)(uint8_t)d6;
            d2 = (uint16_t)(d2 * 38);  /* mulu.w #$26 */
            uint8_t *playerSlotPtr = teamRaw + d2;
            Career_UpdateSinglePlayerPostMatchStats(playerSlotPtr);
        }
    }
}

/* ================================================================
 * AddTrialPlayer — generate a random trial player and add to roster.
 * Asm: AddTrialPlayer (no params, result: a3=new player ptr)
 *
 * Picks a random position from trialPlayerPositionTable, then calls
 * TrainingMatch_AddTrialPlayerAtPosition.
 * Returns pointer to the new player (for accessing name at offset 79).
 * ================================================================ */
static const uint8_t trialPlayerPositionTable[] = {3,3,6,6,7,7,0,1,2,4,5,0};

__attribute__((used, externally_visible))
uint8_t *AddTrialPlayer(void)
{
    uint16_t r = Rand();
    uint16_t posIdx = (uint16_t)((uint16_t)(r * 11) >> 8);
    uint16_t position = (uint16_t)trialPlayerPositionTable[posIdx];
    return TrainingMatch_AddTrialPlayerAtPosition(position);
}

/* ================================================================
 * TrainingMatch_AddTrialPlayerAtPosition — add trial player at given position.
 * Asm: TrainingMatch_AddTrialPlayerAtPosition (d0=position)
 *
 * Finds unused player slot in team, sets up as trialist, inserts into
 * currentMatchPlayers array at correct position, assigns random form modifier.
 * Returns pointer to the new player record.
 * ================================================================ */
__attribute__((used, externally_visible))
uint8_t *TrainingMatch_AddTrialPlayerAtPosition(uint16_t position)
{
    /* Find an unused player index (not in currentMatchPlayers[0..careerNumPlayers-1]) */
    uint16_t freeSlot = 0;
    while (1) {
        uint16_t np = _careerNumPlayers;
        uint16_t found = 0;
        uint16_t i;
        for (i = 0; i < np; i++) {
            if (_currentMatchPlayers[i] == (uint8_t)freeSlot) {
                freeSlot++;
                found = 1;
                break;
            }
        }
        if (!found)
            break;
    }

    /* Get the player record at that slot */
    uint8_t *player = GetPlayer(_selTeamNumber, freeSlot);
    PlayerFile *pf = (PlayerFile *)(player + 76);
    TrainingMatch_SaveSelectedPlayerId(freeSlot);

    /* Set shirtNumber as a flag (used as a marker here) */
    pf->shirtNumber = 0x10;
    if (_selTeamNumber == BE_READ_U16(&_careerTeam))
        SquadNumbers_Free(freeSlot);    /* (real squad numbers: a free one, teams.c) */

    /* Store position into positionAndFace (position << 5) */
    pf->positionAndFace = (uint8_t)(position << PF_POSITION_SHIFT);

    /* Convert player to trialist */
    Career_ConvertPlayerToTrialistFromName(_selTeamNumber, freeSlot);

    /* Determine insertion point in currentMatchPlayers */
    uint16_t insertOffset;
    {
        DIYCompetitionEx *compBufEx = (DIYCompetitionEx *)&_primaryCompetitionBuf;
        if (compBufEx->base.competitionType == 2) {
            insertOffset = (uint16_t)compBufEx->base.numTeamsInStage;
        } else {
            insertOffset = compBufEx->matchData.cup.roundLegs[0];
        }
        insertOffset += 0x0B;
    }

    /* Shift entries up to make room at insertOffset.
     * ASM: a0 = arr + careerNumPlayers, copies a0[-1]→a0[0], a0--, repeat.
     * This expands the array by one, shifting elements rightward. */
    {
        uint8_t *arr = _currentMatchPlayers;
        uint16_t endIdx = _careerNumPlayers;
        uint8_t *endPtr = &arr[endIdx];
        uint16_t count = endIdx - insertOffset;
        if (count > 0) {
            uint16_t i;
            for (i = 0; i < count; i++) {
                endPtr[-(int16_t)i] = endPtr[-1 - (int16_t)i];
            }
        }
        arr[insertOffset] = (uint8_t)freeSlot;
    }

    /* Assign random negative form modifier */
    {
        uint16_t r = Rand();
        uint16_t formVal = (uint16_t)((uint16_t)(r * 50) >> 8);  /* 0-49 */
        formVal += 40;   /* 40-89 */
        formVal = (uint16_t)(-(int16_t)formVal);  /* -40 to -89 */
        _playerFormModifiers[freeSlot] = (uint8_t)formVal;
    }

    _careerNumPlayers += 1;
    Career_UpdatePlayerWeeklyWageCache(freeSlot);

    return player;
}

/* ================================================================
 * CheckPlayersLeavingComing — check for random player arrivals/departures.
 * Asm: CheckPlayersLeavingComing (no params)
 *
 * If not national selector and not sacked:
 * - If roster < 30 and 25/256 chance → add trial player, show message
 * - Otherwise scan roster backwards for trialists ($9B form) and
 *   reserves ($9A form) with small random chance to leave
 * ================================================================ */
__attribute__((used, externally_visible))
void CheckPlayersLeavingComing(void)
{
    if (_isNationalSelector)
        return;
    if (_youreSacked)
        return;

    /* Try to add a trial player if roster not full */
    if (_careerNumPlayers < 30) {
        extern uint8_t _expandedFacilityLevels[8];
        /* A larger scouting department finds more of the game's existing
         * procedurally generated trialists. Do not create new team IDs or
         * exceed the original 30-player squad safety limit. */
        uint16_t scoutChance = (uint16_t)(25 + 6 * (_expandedFacilityLevels[5] - 1));
        if (scoutChance > 49) scoutChance = 49;
        if (Rand() < scoutChance) {
            uint8_t *newPlayer = AddTrialPlayer();
            PlayerFile *npf = (PlayerFile *)(newPlayer + 76);
            /* Better scouts have a small chance to spot a higher-value
             * youngster, capped at the game's valid price index 0x31. */
            if (_expandedFacilityLevels[7] >= 2 && npf->price < 49 && Rand() < 128) {
                /* Youth academy determines incoming youngsters' quality;
                 * scouting determines how often they are found. */
                unsigned better = (unsigned)(uint8_t)npf->price +
                                  (unsigned)(_expandedFacilityLevels[7]-1u)*2u;
                npf->price = (int8_t)(better > 49 ? 49 : better);
            }
            PlayerJoinedClubOnTrial((uint8_t *)npf->playerName);
            return;
        }
    }

    /* Scan roster backwards for players that might leave.
     *
     * Asm passes the *file index* (= currentMatchPlayers[d2]) to
     * RemovePlayerFromTeam, not the ordinal d2 — GetPlayerAtIndex
     * returns d1=fileIndex, and the asm uses that d1 directly across
     * the form check and into RemovePlayerFromTeam.
     *
     * Passing d2 (the ordinal) here would try to remove a career-roster
     * slot that may not exist (a "hole" left by previous removals),
     * causing GetPlayerSlotIndexByPlayerNumber to panic searching for
     * the missing value. */
    {
        int16_t d2;
        for (d2 = (int16_t)(_careerNumPlayers - 1); d2 >= 0; d2--) {
            uint8_t *player = GetPlayerAtIndex(_selTeamNumber, (uint16_t)d2);
            uint16_t fileIndex = (uint16_t)_currentMatchPlayers[d2];
            PlayerFile *pf = (PlayerFile *)(player + 76);
            uint8_t form = (uint8_t)pf->form;

            if (form == PLAYER_FORM_TRIALIST) {
                /* Trial player ($9B) — 5/256 chance to leave */
                if (Rand() >= 5)
                    continue;
                TrialPlayerLeftClubMessage((uint8_t *)pf->playerName);
                goto remove;
            }

            if (form == PLAYER_FORM_RESERVE) {
                /* Reserve/loan player ($9A) — 2/256 chance to leave */
                if (Rand() >= 2)
                    continue;
                ReservePlayerLeftClubMessage((uint8_t *)pf->playerName);
                goto remove;
            }

            continue;

        remove:
            RemovePlayerFromTeam(fileIndex);
            return;  /* Only one removal per call */
        }
    }
}

/* ================================================================
 * Chunk 101: Club Business Init & Career Season Finance Init
 * ================================================================ */

/* Additional externs needed for Chunk 101 */
extern MenuEntry _menuEntries[];
extern int32_t _playerSales;
extern int32_t _playerPurchases;
extern uint16_t _contractNegotiationActive;
extern uint16_t _sensisoftPlcSponsorshipFactor;
extern int32_t _sensisoftPlcSponsorship;
extern uint16_t _additionalInvestments;
extern uint16_t _financeTotalsNeedResetFlag;
extern uint8_t _aTotalLoss[];
extern uint8_t _aBalanceAtStart[];
extern uint8_t _aBalanceAtEndOf[];
extern uint8_t _aSponsorshipFor[];
extern uint8_t _aSeasonFromSens[];
extern uint8_t _aAdditionalInve[];

/* Already-ported C functions used by Chunk 101 */
extern uint16_t GetTeamValueBase(uint16_t avgPrice);

/* Helper: menu entry from byte offset */

/* Helper: access bg as the high 16 bits (big-endian BACKANDFRAMECOLOR) */

/* Compute total profit/loss from all financial components */
static inline int32_t computeTotalProfitLoss(void) {
    int32_t total = _startingBalanceCopy;
    total += _competitionBonuses;
    total += _gateReceipts;
    total += _playerSales;
    total -= _playerPurchases;
    total -= _playerWagesBill;
    total -= _generalRunningCosts;
    return total;
}

/* Common: format new balance entry with color/flag adjustments if negative */
static void formatNewBalanceEntry(MenuEntry *e, int32_t balance) {
    if (balance < 0) {
        /* Set blinking flag (bit 13 of stringFlags) */
        e->stringFlags |= (1 << 13);
        /* Change color to RED_12 */
        uint16_t bg = e->bgColor;
        bg = (bg & 0xFFF0) | 12;
        e->bgColor = bg;
    }
    Int2PoundsSigned(balance, (uint8_t *)e->fg);
}

/* ================================================================
 * ClubBusinessInit  (SWOS2.s line 83165)
 *
 * Initializes the Club Business menu display. Three paths:
 * 1) contractNegotiationActive → full financial summary
 * 2) !stayingInOldClub → abbreviated (no gate receipts/costs)
 * 3) stayingInOldClub → extended with sponsorship info
 * ================================================================ */
__attribute__((used, externally_visible))
void ClubBusinessInit(void)
{
    if (_contractNegotiationActive) {
        /* ---- Path 1: Full financial summary ---- */
        /* Old balance */
        Int2PoundsSigned(_old_balance, (uint8_t *)_menuEntries[12].fg);
        /* Gate receipts */
        WriteAccountBalance(_gateReceipts, (uint8_t *)_menuEntries[14].fg);
        /* General running costs (negated) */
        WriteAccountBalance(-_generalRunningCosts, (uint8_t *)_menuEntries[15].fg);
        /* Player sales */
        WriteAccountBalance(_playerSales, (uint8_t *)_menuEntries[17].fg);
        /* Player purchases (negated) */
        WriteAccountBalance(-_playerPurchases, (uint8_t *)_menuEntries[18].fg);
        /* Player wages bill (negated) */
        WriteAccountBalance(-_playerWagesBill, (uint8_t *)_menuEntries[19].fg);

        /* Compute total profit/loss */
        int32_t total = computeTotalProfitLoss();
        if (total < 0) {
            _menuEntries[10].fg = (uintptr_t)_aTotalLoss;
        }

        /* Write total */
        WriteAccountBalance(total, (uint8_t *)_menuEntries[20].fg);

        /* New balance with red color if negative */
        formatNewBalanceEntry(&_menuEntries[21], _newBalance);
        return;
    }

    if (!_stayingInOldClub) {
        /* ---- Path 2: Abbreviated summary (no gate receipts etc.) ---- */
        MenuEntry *e70 = &_menuEntries[2];
        e70->y += 30;
        e70->fg = (uintptr_t)_aBalanceAtStart;

        MenuEntry *e2A0 = &_menuEntries[12];
        e2A0->y += 30;
        Int2PoundsSigned(_old_balance, (uint8_t *)e2A0->fg);

        /* Hide entries: E0, 118, 310, 348 */
        _menuEntries[4].isInvisible = 1;
        _menuEntries[5].isInvisible = 1;
        _menuEntries[14].isInvisible = 1;
        _menuEntries[15].isInvisible = 1;

        /* Player sales & purchases */
        WriteAccountBalance(_playerSales, (uint8_t *)_menuEntries[17].fg);
        WriteAccountBalance(-_playerPurchases, (uint8_t *)_menuEntries[18].fg);

        /* Hide: 1F8, 428 */
        _menuEntries[9].isInvisible = 1;
        _menuEntries[19].isInvisible = 1;

        /* Adjust Y positions */
        _menuEntries[10].y -= 15;

        /* Compute total */
        int32_t total = computeTotalProfitLoss();
        if (total < 0) {
            _menuEntries[10].fg = (uintptr_t)_aTotalLoss;
        }

        /* Write total */
        _menuEntries[20].y -= 15;
        WriteAccountBalance(total, (uint8_t *)_menuEntries[20].fg);

        _menuEntries[11].y -= 15;

        /* New balance with color adjustments */
        MenuEntry *e498 = &_menuEntries[21];
        e498->y -= 15;
        formatNewBalanceEntry(e498, _newBalance);
        return;
    }

    /* ---- Path 3: Extended with sponsorship ---- */
    if (_sensisoftPlcSponsorshipFactor) {
        /* Shift entries 0x70..0x70+9*56 up by 13 each */
        MenuEntry *e = &_menuEntries[2];
        for (int16_t i = 9; i >= 0; i--) {
            e->y -= 13;
            e++;
        }
        e = &_menuEntries[12];
        for (int16_t i = 9; i >= 0; i--) {
            e->y -= 13;
            e++;
        }
    }

    /* Set "BALANCE AT END OF LAST SEASON:" */
    MenuEntry *e70 = &_menuEntries[2];
    e70->fg = (uintptr_t)_aBalanceAtEndOf;
    e70->y += 5;

    MenuEntry *e2A0 = &_menuEntries[12];
    e2A0->y += 5;
    Int2PoundsSigned(_old_balance, (uint8_t *)e2A0->fg);

    /* Show entries: A8, 2D8 */
    _menuEntries[3].isInvisible = 0;
    _menuEntries[13].isInvisible = 0;

    /* Competition bonuses */
    WriteAccountBalance(_competitionBonuses, (uint8_t *)_menuEntries[13].fg);

    /* Shift entries E0..E0+7*56 down by 15 */
    {
        MenuEntry *e = &_menuEntries[4];
        for (int16_t i = 7; i >= 0; i--) {
            e->y += 15;
            e++;
        }
    }
    {
        MenuEntry *e = &_menuEntries[14];
        for (int16_t i = 7; i >= 0; i--) {
            e->y += 15;
            e++;
        }
    }

    /* Set sponsorship text */
    MenuEntry *eE0 = &_menuEntries[4];
    eE0->fg = (uintptr_t)_aSponsorshipFor;
    eE0->y += 5;

    _menuEntries[5].fg = (uintptr_t)_aSeasonFromSens;

    /* Starting balance copy */
    _menuEntries[14].y += 10;
    _menuEntries[15].isInvisible = 1;
    WriteAccountBalance(_startingBalanceCopy, (uint8_t *)_menuEntries[14].fg);

    if (_sensisoftPlcSponsorshipFactor) {
        /* Show sponsorship entries */
        _menuEntries[6].isInvisible = 0;
        _menuEntries[16].isInvisible = 0;
        WriteAccountBalance(_sensisoftPlcSponsorship, (uint8_t *)_menuEntries[16].fg);

        if (_additionalInvestments) {
            _menuEntries[6].fg = (uintptr_t)_aAdditionalInve;
        }

        /* Shift entries 188..188+4*56 down by 15 */
        {
            MenuEntry *e = &_menuEntries[7];
            for (int16_t i = 4; i >= 0; i--) {
                e->y += 15;
                e++;
            }
        }
        {
            MenuEntry *e = &_menuEntries[17];
            for (int16_t i = 4; i >= 0; i--) {
                e->y += 15;
                e++;
            }
        }
    }

    /* Player sales & purchases */
    WriteAccountBalance(_playerSales, (uint8_t *)_menuEntries[17].fg);
    WriteAccountBalance(-_playerPurchases, (uint8_t *)_menuEntries[18].fg);

    /* Hide: 1F8, 428 */
    _menuEntries[9].isInvisible = 1;
    _menuEntries[19].isInvisible = 1;

    /* Adjust and compute total */
    _menuEntries[10].y -= 15;

    int32_t total = computeTotalProfitLoss();
    if (total < 0) {
        _menuEntries[10].fg = (uintptr_t)_aTotalLoss;
    }

    _menuEntries[20].y -= 15;
    WriteAccountBalance(total, (uint8_t *)_menuEntries[20].fg);

    _menuEntries[11].y -= 15;

    MenuEntry *e498 = &_menuEntries[21];
    e498->y -= 15;
    formatNewBalanceEntry(e498, _newBalance);
}

/* ================================================================
 * Career_InitializeNewSeasonFinances  (SWOS2.s line 83433)
 *
 * Initializes financial state for a new season: recalculates wages,
 * resets accumulators, handles sponsorship and competition bonuses.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_InitializeNewSeasonFinances(void)
{
    /* Recalculate all player wages */
    for (int16_t i = (int16_t)(_careerNumPlayers - 1); i >= 0; i--) {
        GetPlayerAtIndex(_selTeamNumber, (uint16_t)i);
        Career_UpdatePlayerWeeklyWageCache((uint16_t)i);
    }

    /* Transfer newBalance → old_balance and clear accumulators */
    _old_balance = _newBalance;
    _playerPurchases = 0;
    _playerSales = 0;
    _playerWagesBill = 0;
    _generalRunningCosts = 0;
    _gateReceipts = 0;
    _sensisoftPlcSponsorship = 0;

    if (_stayingInOldClub) {
        _additionalInvestments = 0;
        uint16_t d7 = _sensisoftPlcSponsorshipFactor;

        if (d7 == 0) {
            /* 1/8 chance of getting sponsorship */
            if (Rand2() >= 0x20) {
                goto skipSponsorship;
            }
            d7 = 0x10;
            _additionalInvestments = 1;
        }

        /* Random sponsorship level: 1..32, capped at d7 */
        {
            uint16_t rnd = Rand2();
            rnd = (rnd & 0x1F) + 1;
            _sensisoftPlcSponsorshipFactor = 0;
            if (rnd > d7) {
                goto skipSponsorship;
            }
            _sensisoftPlcSponsorshipFactor = rnd;
            d7 = rnd;
        }

        /* Calculate sponsorship amount based on team value */
        {
            VerifyTeamLoaded(_selTeamNumber);
            uint16_t avgPrice = GetAveragePlayerPrice(&_careerTeam);
            uint32_t d0 = (uint32_t)GetTeamValueBase(avgPrice);
            d0 = d0 * d7;

            if (d0 > 10000) {
                d0 = 10000;
                uint16_t rnd = Rand2() & 0xF;
                rnd = rnd * 100;
                d0 = 10000 - rnd;
            }

            d0 = d0 * 1000;
            _sensisoftPlcSponsorship = (int32_t)d0;
            _newBalance += (int32_t)d0;
        }
    }

skipSponsorship:
    /* Competition bonuses */
    _competitionBonuses = 0;

    if (_stayingInOldClub && _quaternaryCompetitionPtr != 0) {
        int16_t bonus = (int8_t)_euroCupsCompetitionContextBuf.tourn.header.trophyBonusValue;
        uint32_t d0;

        const uint16_t euroIdx = _euroCupSlotIndex == 7 ? 1     /* (another confederation's cups) */
                               : _euroCupSlotIndex == 9 ? 2     /* (a world's group contest) */
                               : _euroCupSlotIndex == 8 ? (uint16_t)(Conti_TrophyTier() < 0 ? 1 : Conti_TrophyTier())
                               : _euroCupSlotIndex >= 4 ? _euroCupSlotIndex - 4 : _euroCupSlotIndex;
        if (euroIdx == 1) {             /* (4..6: a world's qualifying tie, its cup's tier) */
            d0 = (uint32_t)((uint16_t)bonus) * 62644;
        } else if (euroIdx == 2) {
            d0 = (uint32_t)((uint16_t)bonus) * 41668;
            d0 <<= 1;
        } else {
            /* European Cup (0) or Copa Libertadores (3): euroCup-tier bonus. */
            d0 = (uint32_t)((uint16_t)(bonus << 1)) * 50011;
        }

        d0 = (d0 / 1000) * 1000;
        _competitionBonuses += (int32_t)d0;
    }

    if (_stayingInOldClub) {
        int16_t bonus = (int8_t)_primaryCompetitionBuf.tourn.header.trophyBonusValue;
        uint32_t d0;

        if (_currentDivisionIndex == 0) {
            d0 = (uint32_t)((uint16_t)(bonus << 1)) * 0x84D0;
        } else if (_currentDivisionIndex == 1) {
            d0 = (uint32_t)((uint16_t)bonus) * 0x7E61;
        } else if (_currentDivisionIndex == 2) {
            d0 = (uint32_t)((uint16_t)bonus) * 16667;
        } else {
            d0 = (uint32_t)((uint16_t)bonus) * 11112;
        }

        d0 = (d0 / 1000) * 1000;
        _competitionBonuses += (int32_t)d0;
    }

    /* Apply competition bonuses and starting balance */
    _newBalance += _competitionBonuses;
    _startingBalanceCopy = _startingBalance;
    _newBalance += _startingBalanceCopy;

    /* Expanded sponsorship + TV rights are paid into the same balance
     * consumed by the original transfer market. Original match receipts
     * and competition bonuses above are NOT counted twice. */
    extern void Expanded_EconomyNewSeason(void);
    Expanded_EconomyNewSeason();
    _financeTotalsNeedResetFlag = 1;
    _clubBusinessActive = 0;
}

/* ================================================================
 * Career_HandleRetirement  (SWOS2.s line 36190)
 *
 * Sets retired flag, then runs end-of-season career updates:
 * board confidence modifiers/score, reputation, job offers, history.
 * ================================================================ */
extern uint16_t _retired;
extern void Career_FillCurrentSeasonInfo(void);
extern void Career_AutoAdvanceCompetitionAndPersistSlotState(uint8_t *compPtr);
extern uint16_t _numJobOffers;
extern uint16_t _numNewJobOffers;

__attribute__((used, externally_visible))
void Career_HandleRetirement(void)
{
    _retired = 1;
    UpdateBoardConfidenceModifiers();
    UpdateCareerReputationBonuses();
    UpdateBoardConfidenceScore();
    Career_MaybeGenerateJobOffer();
    PushBoardConfidenceHistory();
}

/* ================================================================
 * Career_PostMatchUpdate  (SWOS2.s line 35371)
 *
 * Called after each career match. Fills season info, removes expired
 * job offers, generates 1-2 new job offers (up to 100 attempts),
 * then auto-advances all competition slots. Falls through to
 * Career_HandleRetirement at the end.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_PostMatchUpdate(void)
{
    /* Fill current season info (preserves all regs in asm) */
    Career_FillCurrentSeasonInfo();

    /* Remove expired job offers */
    JobOffers_RemoveExpiredOffers();

    /* Decide how many job offers to target: 1 or 2 */
    uint16_t target = (Rand2() & 1) + 1;

    /* Try up to 100 times to generate enough job offers */
    for (int16_t i = 99; i >= 0; i--) {
        uint16_t total = _numJobOffers + _numNewJobOffers;
        if (total >= target)
            break;
        Career_MaybeGenerateJobOffer();
    }

#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate ENTER isNationalSelector=%u "
                      "currentCompPtr=%p secondaryCompPtr=%p tertiaryCompPtr=%p "
                      "quaternaryCompPtr=%p youreSacked=%u",
                      (unsigned)_isNationalSelector,
                      (void *)(uintptr_t)_currentCompetitionPtr,
                      (void *)(uintptr_t)_secondaryCompetitionPtr,
                      (void *)(uintptr_t)_tertiaryCompetitionPtr,
                      (void *)(uintptr_t)_quaternaryCompetitionPtr,
                      (unsigned)_youreSacked);
#endif

    /* If national selector, skip competition auto-advance */
    if (_isNationalSelector != 0) {
#ifdef SWOS_DEBUG_AUTOSKIP
        SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate EXIT (isNationalSelector — no auto-advance)");
#endif
        Career_HandleRetirement();
        return;
    }

    /* Save and temporarily override selTeamNumber and teamControls */
    uint16_t savedSelTeam = _selTeamNumber;
    uint8_t savedControls = _careerTeam.team.teamControls;

    _selTeamNumber = 0xFFFF;
    _careerTeam.team.teamControls = 1;

    /* Auto-advance all competition slots */
#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate -> AutoAdvance(currentCompetitionPtr=%p)",
                      (void *)(uintptr_t)_currentCompetitionPtr);
#endif
    Career_AutoAdvanceCompetitionAndPersistSlotState((uint8_t *)_currentCompetitionPtr);

    if (_secondaryCompetitionPtr != 0) {
#ifdef SWOS_DEBUG_AUTOSKIP
        SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate -> AutoAdvance(secondaryCompetitionPtr=%p)",
                          (void *)(uintptr_t)_secondaryCompetitionPtr);
#endif
        Career_AutoAdvanceCompetitionAndPersistSlotState((uint8_t *)_secondaryCompetitionPtr);
    }

    if (_tertiaryCompetitionPtr != 0) {
#ifdef SWOS_DEBUG_AUTOSKIP
        SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate -> AutoAdvance(tertiaryCompetitionPtr=%p)",
                          (void *)(uintptr_t)_tertiaryCompetitionPtr);
#endif
        Career_AutoAdvanceCompetitionAndPersistSlotState((uint8_t *)_tertiaryCompetitionPtr);
    }

    if (_quaternaryCompetitionPtr != 0) {
#ifdef SWOS_DEBUG_AUTOSKIP
        SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate -> AutoAdvance(quaternaryCompetitionPtr=%p)",
                          (void *)(uintptr_t)_quaternaryCompetitionPtr);
#endif
        Career_AutoAdvanceCompetitionAndPersistSlotState((uint8_t *)_quaternaryCompetitionPtr);
    }

    /* Restore saved values */
    _careerTeam.team.teamControls = savedControls;
    _selTeamNumber = savedSelTeam;

#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug4-cup] PostMatchUpdate EXIT (after all 4 AutoAdvance calls)");
#endif

    /* Fall through to Career_HandleRetirement (bra.w *+4) */
    Career_HandleRetirement();
}

/* --- Chunk 179: Career Auto-Advance Chain --- */

extern uint16_t _careerAutoAdvanceInProgress;
/* Career_AutoAdvanceCompetitionSlots — forward declaration (ported below) */

/* Career_AutoAdvanceCompetitionAndPersistSlotState (SWOS2.s line 35559)
 * Takes a competition pointer, identifies the corresponding context buffer,
 * loads slot state, dispatches to the appropriate auto-advance function
 * based on competition type, then stores the slot state back. */
__attribute__((used, externally_visible))
void Career_AutoAdvanceCompetitionAndPersistSlotState(uint8_t *compPtr)
{
    _careerAutoAdvanceInProgress = 1;

    /* Find the matching competition context buffer */
    uint8_t *ctx = (uint8_t *)&_primaryCompetitionBuf;
    if (compPtr != (uint8_t *)_currentCompetitionPtr) {
        ctx = (uint8_t *)&_competitionContextBuf;
        if (compPtr != (uint8_t *)_secondaryCompetitionPtr) {
            ctx = (uint8_t *)&_tertiaryCompetitionContextBuf;
            if (compPtr != (uint8_t *)_tertiaryCompetitionPtr) {
                ctx = (uint8_t *)&_euroCupsCompetitionContextBuf;
            }
        }
    }

    Career_LoadCompetitionSlotStateFromBuffer(ctx);

    /* Dispatch by competition type. The asm reads `1(a0)` which on m68k
     * BE is the LOW byte of the uint16 type word — i.e. the actual type
     * value (0=league, 1=cup, 2=hybrid). On x86 LE the same byte index
     * picks the HIGH byte instead, which is always zero for legal types,
     * so the previous `ctx[1]` always evaluated to 0 and every comp was
     * mis-dispatched to AutoAdvanceLeagueFixtures. Read as a host-native
     * uint16 and take the low byte — works on both endians. */
    uint8_t compType = (uint8_t)*(uint16_t *)ctx;

    /* Sanity-check the loaded competition state. If numTeams or
     * totalMatchesInRound is out of range, the context buffer was
     * never properly populated for this slot — most likely the
     * competition pointers in this CAR/save reference a competition
     * the user isn't really in (FIRED.CAR repro had spanishLeague /
     * spanishCup pointing here for a manager who never owned a
     * Spanish team). Running the auto-advance with garbage causes
     * either a SIGSEGV inside getCurrentFixtureTeamPtrs (huge
     * totalMatchesInRound makes d5 = subRoundCounter * totalMatches
     * overflow into invalid memory) or an infinite loop in
     * Career_AutoAdvanceLeagueFixtures' main_loop (state never
     * progresses past the loop's break conditions).
     *
     * Skip the dispatch entirely on garbage state — the
     * persist-back at the bottom still runs so we don't corrupt
     * the context further, but the actual sim is bypassed. One-shot
     * log so we know which competition was bad. */
    {
        /* Read packed state directly; no potentially 8-byte-aligned base pointer. */
        int16_t numTeams = _diyCompetitionStart.base.numTeams;
        int16_t totalInRound = _diyCompetitionStart.base.totalMatchesInRound;
        int bail = 0;
        const char *bailReason = 0;
        uint32_t bailTeamSelPtr = 0;

        if (numTeams <= 0 || numTeams > LEAGUE_PHASE_CLUBS) {   /* (a world's league phase: 36) */
            bail = 1;
            bailReason = "numTeams out of range";
        } else if (totalInRound <= 0 || totalInRound > 32) {
            bail = 1;
            bailReason = "totalMatchesInRound out of range";
        } else if (compType == 0) {
            /* Standings-row validation only applies to LEAGUE comps
             * (type=0). For cup (type=1) and hybrid (type=2) comps,
             * `diyCompetitionStart+0x2B4` overlays diyCompetitionStart.cupTeamIds /
             * diyTeamGridPairings / diyTeamShuffleOrder / audioInstrumentBank
             * — see project memory `diycomp_league_table_overlay`. Reading
             * the first 4 bytes of that region as an int32 "teamSelOffset"
             * for a cup yields garbage like 0x0D0C0B0A (sequential team
             * grid indices), which would fail any sane offset range and
             * mis-flag the cup as corrupt. The Coppa Italia and European
             * Cup post-sack auto-advance regressions trace to exactly
             * this false positive: Playaveli's repro showed bail #1
             * compType=0 numTeams=32 bailTeamSelPtr=0x1A151E0B (Coppa)
             * and bail #2 compType=0 numTeams=8 bailTeamSelPtr=0x0D0C0B0A
             * (CL hybrid). Both reports had compType=0 wrongly because
             * of the byte-order bug fixed above; with the real type now
             * dispatched correctly, this guard naturally limits itself
             * to leagues where the standings rows actually exist.
             *
             * Each row is 18 bytes; teamSelOffset at offset 0 is now a team
             * IDENTITY (step-4 phase 3): -1 == careerTeam, >= 0 == selTeamsPtr
             * index (0..numTeams-1). The legacy byte-offset range below
             * [maxNegative, maxPositive] still bounds it correctly — every valid
             * identity (-1 and small positives) sits well inside it — and still
             * catches the MB-class garbage that motivated the validator (a cup
             * comp's standings overlay read as sequential grid indices). Left as
             * the wide legacy range to keep this a pure corruption guard; it
             * could be tightened to [-1, numTeams) now that the field is an id. */
            const int32_t maxPositive = 64 * 684;
            const int32_t maxNegative = -1 * 1024 * 1024;
            uint8_t *rowsBase = (uint8_t *)&_diyCompetitionStart + DIY_STANDINGS_OFFSET;
            for (int16_t i = 0; i < numTeams; i++) {
                int32_t teamSelOff = *(int32_t *)(rowsBase + i * 18);
                if (teamSelOff > maxPositive || teamSelOff < maxNegative) {
                    bail = 1;
                    bailReason = "standings row teamSelOffset out of range";
                    bailTeamSelPtr = (uint32_t)teamSelOff;
                    break;
                }
            }
        }

        if (bail) {
#ifdef SWOS_DEBUG_AUTOSKIP
            /* Log up to 20 bails (not just first) so we can see whether the
             * player's secondary/tertiary cups are bailing silently. */
            static int s_bad_state_count = 0;
            if (s_bad_state_count++ < 20) {
                SWOS_AUTOSKIP_LOG("[bug1] AutoAdvanceCompetition: bail #%d — "
                                  "compPtr=%p ctx=%p compType=%u numTeams=%d "
                                  "totalMatchesInRound=%d reason='%s' "
                                  "bailTeamSelPtr=0x%08X",
                                  s_bad_state_count,
                                  (void *)compPtr, (void *)ctx,
                                  (unsigned)compType,
                                  (int)numTeams, (int)totalInRound,
                                  bailReason ? bailReason : "?",
                                  (unsigned)bailTeamSelPtr);
            }
#endif
            Career_StoreCompetitionSlotStateToBuffer(ctx);
            _careerAutoAdvanceInProgress = 0;
            return;
        }
    }

    if (compType == 1) {
        Career_AutoAdvanceCupMatches();
    } else if (compType == 2) {
        Career_AutoAdvanceCompetitionSlots();
    } else {
        Career_AutoAdvanceLeagueFixtures();
    }

    Career_StoreCompetitionSlotStateToBuffer(ctx);
    _careerAutoAdvanceInProgress = 0;
}

extern uint16_t _careerLoadedFlag;
extern uint16_t _internationalAutoAdvanceFlag;
extern uint16_t _teamSelectionKeyCacheEnd;
/* viewRandomPhase already declared above as uint8_t */
extern uint8_t  _seasonPlaying;
extern uint8_t  _englishCup[];
extern uint8_t  _englishCup_1[];
extern uint8_t  _englishLeagueCup[];
extern uint8_t  _englishLeagueCup_1[];

extern void SaveRand2State(void);
extern void RestoreRand2State(void);
extern void Randomize2(uint16_t _seed);
extern void Career_InitCompetitionTeamSelectionFromTableHeaderSpec(uint8_t *record);
extern void Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec(uint8_t *record);
extern void Career_ApplyPendingForeignMarketPlayersToSelectedTeams(void);
extern void InitDIYCupFromRecord(uint8_t *record);
extern void InitDIYLeagueFromRecord(uint8_t *record);
extern void InitDIYCompetitionFromTableEntry(uint16_t index, uint8_t *record);
extern void InitTeams(void);
extern void Career_InitCompetitionTeamSelectionFromTableEntrySpec(uint8_t *record, uint16_t index);

/* Career_AutoAdvanceCupCompetitionFromRecord (SWOS2.s line 35246)
 * Auto-advances a cup competition: initializes team selection from record,
 * loads competition, runs cup match advancement, then restores RNG state.
 * a0 = competition record pointer. */
__attribute__((used, externally_visible))
void Career_AutoAdvanceCupCompetitionFromRecord(uint8_t *record)
{
    /* Special-case English cup aliases (118/119 → 120/121). */
    uint8_t rid = record ? comp_id(record) : 0xFFu;
    if (rid == COMPID_englishCup)
        record = _englishCup_1;
    else if (rid == COMPID_englishLeagueCup)
        record = _englishLeagueCup_1;

    _careerLoadedFlag = 1;

    if (_seasonUpdateInProgress == 0 && _internationalAutoAdvanceFlag == 0)
        _teamSelectionKeyCacheEnd = 1;

    _teamsLoaded = 0;
    SaveRand2State();

    /* Save viewRandomPhase + seasonPlaying (contiguous bytes, saved as word in asm) */
    uint8_t savedViewRandom = _viewRandomPhase;
    uint8_t savedSeasonPlaying = _seasonPlaying;

    /* Seed RNG from record byte and seasonPlaying */
    uint16_t seed = ((uint16_t)record[0] << 8) | (uint16_t)_seasonPlaying;
    Randomize2(seed);

    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromTableHeaderSpec(record);
    Career_ApplyPendingForeignMarketPlayersToSelectedTeams();
    InitDIYCupFromRecord(record);
    InitTeams();
    Career_AutoAdvanceCupMatches();

    _viewRandomPhase = savedViewRandom;
    _seasonPlaying = savedSeasonPlaying;
    RestoreRand2State();
    _careerLoadedFlag = 0;
}

/* Career_AutoAdvanceLeagueCompetitionFromRecord (SWOS2.s line 35291)
 * Same as above but for league competitions. */
__attribute__((used, externally_visible))
void Career_AutoAdvanceLeagueCompetitionFromRecord(uint8_t *record)
{
    _careerLoadedFlag = 1;

    if (_seasonUpdateInProgress == 0 && _internationalAutoAdvanceFlag == 0)
        _teamSelectionKeyCacheEnd = 1;

    _teamsLoaded = 0;
    SaveRand2State();

    uint8_t savedViewRandom = _viewRandomPhase;
    uint8_t savedSeasonPlaying = _seasonPlaying;

    uint16_t seed = ((uint16_t)record[0] << 8) | (uint16_t)_seasonPlaying;
    Randomize2(seed);

    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec(record);
    Career_ApplyPendingForeignMarketPlayersToSelectedTeams();
    InitDIYLeagueFromRecord(record);
    InitTeams();
    Career_AutoAdvanceCompetitionSlots();

    _viewRandomPhase = savedViewRandom;
    _seasonPlaying = savedSeasonPlaying;
    RestoreRand2State();
    _careerLoadedFlag = 0;
}


/* Expanded world: resident ORIGINAL OG league contexts. Never bind an
 * unemployed world to the human manager's selection-key cache. */
#ifdef SWOS_SDL_BUILD
#define EXP_OG_MAX_LEAGUES 512u
#define EXP_OG_MAX_TEAMS 36u /* UEFA native league phases use 36, domestic <=32 */
/* Native cup bracketScratch.teamOffsetTable has exactly 64 identities.
 * Even though the selectedTeams scratch can hold 100, larger cups would
 * overrun the original OG bracket; report unsupported instead. */
#define EXP_OG_MAX_CUPS 256u
#define EXP_OG_MAX_CUP_TEAMS 64u
typedef struct {
    uint8_t used,country,division,lastPhase,lastMatch,seed,randIndex,randKey;
    uint16_t year,teamCount;
    DIYCompetitionEx comp;
    DIYTournamentState tournament;
    TeamFileEx teams[EXP_OG_MAX_TEAMS];
} ExpandedOGLeague;
static ExpandedOGLeague s_expOG[EXP_OG_MAX_LEAGUES];
static uint32_t s_expOGCount;
static ExpandedOGLeague *s_expOGCurrent;
static ExpandedOGLeague s_expOGUefa[3]; /* distinct from 134 country divisions */
static uint16_t s_expOGUefaCalendarYear; /* no previous-year FINAL as current */
static int s_expOGPhase=-1,s_expOGBound,s_expOGViewing,s_expOGFault;
static int exp_og_is_uefa(void) {
    return s_expOGCurrent && (s_expOGCurrent==&s_expOGUefa[0] ||
        s_expOGCurrent==&s_expOGUefa[1] || s_expOGCurrent==&s_expOGUefa[2]);
}
uint32_t Expanded_OGWorld_UefaResidentCount(void) {
    /* Slot 1 in 9697 stores the separate Copa Libertadores type-2 context,
     * not a second European championship. Exclude it from UEFA diagnostics. */
    const unsigned slots=World_Current()==WORLD_9697?1u:3u;
    uint32_t n=0u;for(unsigned k=0;k<slots;++k)if(s_expOGUefa[k].used)++n;return n;
}
typedef struct {
    uint8_t used,country,slot,compId,lastPhase,lastMatch,managed_excluded,unsupported_setup;
    uint8_t seed,randIndex,randKey,nativeByeVerified;
    uint16_t year,teamCount,observedSelected;
    uint16_t observedBracketTeams,observedRoundTies,observedRoundPairs;
    uint32_t matchesPlayed;
    DIYCompetitionEx comp;
    DIYTournamentState tournament;
    TeamFileEx teams[EXP_OG_MAX_CUP_TEAMS];
} ExpandedOGCup;
static ExpandedOGCup s_expOGCups[EXP_OG_MAX_CUPS];
static uint32_t s_expOGCupCount,s_expOGCupUnsupported,s_expOGCupFaults;
static ExpandedOGCup *s_expOGCupCurrent;
static int s_expOGCupPhase=-1,s_expOGCupTickFault;
/* Latch a failed local->resident bridge. A rejected fixture MUST NOT make
 * Career_AutoAdvanceLeagueFixtures retry the same fixture indefinitely. */
static int s_expOGBridgeBlocked;
static uint32_t s_expOGBridgeRejections;
static uint32_t s_expOGManagerPairChecked,s_expOGManagerPairRepaired;
static uint32_t s_expOGManagerPairMismatch;
int Expanded_OGWorld_BridgeBlocked(void){return s_expOGBridgeBlocked;}


/* Read-only season-end qualification -> next-edition audit. This witness is
 * never serialized and must not generate any fixture, score or draw. */
typedef struct ExpandedOGQualificationSource {
    int valid; unsigned world,last_world; uint16_t year,last_year;
    unsigned count[4],invalid[4];
    uint16_t ids[4][EXP_QUAL_MAX_ENTRANTS];
} ExpandedOGQualificationSource;
static ExpandedOGQualificationSource s_expQual;
/* One guarded call to the ORIGINAL career qualification functions after
 * the resident OG season is closed. Never a second fixture simulator. */
static int s_expOGQualUnemployedActive,s_expOGQualUnemployedError;
void Expanded_OGWorld_UnemployedQualificationError(const char *reason)
{
    ++s_expOGQualUnemployedError;
    if(s_expOGQualUnemployedError==1)
        fprintf(stderr,"[og-unemployed-qual] REJECT reason=%s "
                       "state=SOURCE_INVALID_NO_NEW_FIELD\n",
                reason?reason:"UNKNOWN");
}
int Expanded_OGWorld_UnemployedQualificationActive(void)
{return s_expOGQualUnemployedActive;}
static const char *exp_qual_name(unsigned c)
{
    if(c==0u)return "CHAMPIONS";
    if(c==1u)return World_Current()==WORLD_9697?"CUP_WINNERS_CUP":"EUROPA";
    if(c==2u)return World_Current()==WORLD_9697?"UEFA_CUP":"CONFERENCE";
    return "LIBERTADORES";
}
void Expanded_OGWorld_QualCaptureNative(uint16_t year,
    const uint8_t *a,unsigned na,const uint8_t *b,unsigned nb,
    const uint8_t *c,unsigned nc,const uint8_t *d,unsigned nd)
{
    const uint8_t *const p[4]={a,b,c,d};
    const unsigned n[4]={na,nb,nc,nd};
    memset(&s_expQual,0,sizeof s_expQual);
    s_expQual.valid=1;s_expQual.world=(unsigned)World_Current();
    s_expQual.year=year;s_expQual.last_world=(unsigned)-1;
    for(unsigned k=0;k<4u;++k) {
        if(n[k]>EXP_QUAL_MAX_ENTRANTS || (n[k] && !p[k]))
            s_expQual.invalid[k]=1u;
        else {
            s_expQual.count[k]=n[k];
            for(unsigned j=0;j<n[k];++j)
                s_expQual.ids[k][j]=(uint16_t)
                    (((uint16_t)p[k][2u*j]<<8)|p[k][2u*j+1u]);
        }
        fprintf(stderr,"[og-qual-next] SOURCE world=%s year=%u "
                       "competition=%s count=%u invalid=%u "
                       "origin=OG_NATIVE_SEASON_END\n",
                World_Tag(),(unsigned)year,exp_qual_name(k),
                n[k],s_expQual.invalid[k]);
        /* Keep real earned team IDs in the external SWOS.log, so a
         * SAVE/CONTINUE across processes can be audited offline too. */
        for(unsigned j=0;j<s_expQual.count[k];++j)
            fprintf(stderr,"[og-qual-next-source] world=%s year=%u "
                           "competition=%s club=%04x source=OG_NATIVE\n",
                    World_Tag(),(unsigned)year,exp_qual_name(k),
                    s_expQual.ids[k][j]);
    }
}
unsigned Expanded_OGWorld_QualReadNext(uint16_t year,unsigned cup,
                                      uint16_t *ids,unsigned capacity)
{
    if(!ids || cup>=4u)return 0u;
    /* A new process restored a VERIFIED resident 2027/2028 edition from
     * CAR but not this transient diagnostic qualification witness.
     * Resume that same real OG draw; never manufacture another edition. */
    if(!s_expQual.valid ||
       !ExpandedQual_SameEdition(s_expQual.world,s_expQual.year,
                                 (unsigned)World_Current(),year)) {
        const ExpandedOGLeague *existing=NULL;
        if(World_Current()==WORLD_9697 && (cup==1u || cup==2u)) {
            for(uint32_t z=0;z<s_expOGCupCount;++z) {
                const ExpandedOGCup *c=&s_expOGCups[z];
                if(c->used && c->country==254u && c->slot==cup &&
                   c->year==year && c->teamCount<=capacity) {
                    for(unsigned i=0;i<c->teamCount;++i)
                        ids[i]=PACK_BE_U16(c->teams[i].team.countryNumber,
                                          c->teams[i].team.teamOrdinal);
                    return c->teamCount;
                }
            }
        }
        if(cup<3u || (cup==3u && World_Current()==WORLD_9697))
            existing=&s_expOGUefa[cup==3u?1u:cup];
        if(existing && existing->used && existing->year==year &&
           existing->teamCount<=capacity) {
            for(unsigned i=0;i<existing->teamCount;++i)
                ids[i]=PACK_BE_U16(existing->teams[i].team.countryNumber,
                                  existing->teams[i].team.teamOrdinal);
            return existing->teamCount;
        }
        return 0u;
    }
    if(capacity<s_expQual.count[cup] || s_expQual.invalid[cup])
        return 0u;
    const unsigned n=s_expQual.count[cup];
    if(n<2u || n>EXP_QUAL_MAX_ENTRANTS)return 0u;
    for(unsigned i=0;i<n;++i) {
        const uint16_t id=s_expQual.ids[cup][i];
        if(id==0xffffu)return 0u;
        for(unsigned j=0;j<i;++j)
            if(s_expQual.ids[cup][j]==id)return 0u;
        ids[i]=id;
    }
    return n;
}
static unsigned exp_qual_resident(unsigned cup,uint16_t year,
    uint16_t *out,uint16_t *actualYear,int *available)
{
    *available=0;*actualYear=0u;
    if(World_Current()==WORLD_9697 && (cup==1u || cup==2u)) {
        for(uint32_t k=0;k<s_expOGCupCount;++k) {
            const ExpandedOGCup *const x=&s_expOGCups[k];
            if(!x->used || x->country!=254u || x->slot!=cup)continue;
            *actualYear=x->year;
            if(x->year!=year || x->managed_excluded ||
               x->unsupported_setup || x->teamCount>EXP_QUAL_MAX_ENTRANTS)
                return 0u;
            *available=1;
            for(unsigned j=0;j<x->teamCount;++j) {
                const TeamFile *const t=&x->teams[j].team;
                out[j]=PACK_BE_U16(t->countryNumber,t->teamOrdinal);
            }
            return x->teamCount;
        }
        return 0u;
    }
    if(cup==3u && World_Current()==WORLD_2627)
        return 0u; /* modern CONMEBOL belongs to CT_ED, not historic type 2 */
    const ExpandedOGLeague *const x=&s_expOGUefa[cup==3u?1u:cup];
    if(!x->used)return 0u;
    *actualYear=x->year;
    if(x->year!=year || x->teamCount>EXP_QUAL_MAX_ENTRANTS)return 0u;
    *available=1;
    for(unsigned j=0;j<x->teamCount;++j) {
        const TeamFile *const t=&x->teams[j].team;
        out[j]=PACK_BE_U16(t->countryNumber,t->teamOrdinal);
    }
    return x->teamCount;
}
void Expanded_OGWorld_QualAuditNextYear(uint16_t year)
{
    const unsigned world=(unsigned)World_Current();
    if(year<=World_FirstYear())return;
    if(s_expQual.last_world==world && s_expQual.last_year==year)return;
    s_expQual.last_world=world;s_expQual.last_year=year;
    if(!s_expQual.valid ||
       !ExpandedQual_SameEdition(s_expQual.world,s_expQual.year,world,year)){
        /* An unemployed world genuinely has NO career-manager callback
         * from which the current architecture gathers qualified teams.
         * This is a FAILED handoff requirement, not a harmless omission:
         * the OG world must eventually emit those fields independently
         * of the current manager. Never create fake champions as a fix. */
        const int unemployed=_selTeamNumber==0xffffu;
        fprintf(stderr,"[og-qual-next] SUMMARY world=%s year=%u "
                       "status=%s checked=%u passed=0 failed=%u "
                       "not-tested=%u reason=%s\n",
                World_Tag(),(unsigned)year,
                unemployed?"FAIL_UNEMPLOYED_QUALIFICATION_NOT_CONNECTED":
                           "NOT_TESTED_SOURCE_UNAVAILABLE",
                unemployed?3u:0u,unemployed?3u:0u,
                unemployed?1u:4u,
                unemployed?"OG_NATIVE_CAREER_SOURCE_ABSENT":
                           "RESTORE_OR_TRANSITION_SOURCE_ABSENT");
        return;
    }
    unsigned checked=0u,pass=0u,fail=0u,untested=0u;
    for(unsigned c=0;c<4u;++c) {
        uint16_t actual[EXP_QUAL_MAX_ENTRANTS]={0};
        uint16_t actualYear=0u;int available=0;
        const unsigned n=exp_qual_resident(c,year,actual,
                                          &actualYear,&available);
        const unsigned ne=s_expQual.count[c];
        if(!ne && !s_expQual.invalid[c]) {
            ++untested;
            fprintf(stderr,"[og-qual-next] ROUTE world=%s year=%u "
                           "competition=%s status=NOT_TESTED_SOURCE_EMPTY "
                           "expected=0 actual=%u\n",
                    World_Tag(),(unsigned)year,exp_qual_name(c),n);
            continue;
        }
        const ExpandedQualDiff diff=ExpandedQual_Compare(
            s_expQual.ids[c],ne,actual,n);
        const int ok=available && !s_expQual.invalid[c] &&
                     ExpandedQual_IsExact(&diff);
        const char *const verdict=s_expQual.invalid[c]?
            "FAIL_INVALID_SOURCE":!available?"FAIL_NEW_EDITION_MISSING":
            !ExpandedQual_IsExact(&diff)?"FAIL_WRONG_QUALIFIERS":
            "PASS_EXACT_IDS";
        ++checked;if(ok)++pass;else ++fail;
        fprintf(stderr,"[og-qual-next] ROUTE world=%s year=%u "
                       "competition=%s status=%s source-year=%u "
                       "resident-year=%u expected=%u actual=%u "
                       "matched=%u missing=%u extra=%u dup-source=%u "
                       "dup-resident=%u invalid-source=%u invalid-resident=%u "
                       "first-missing=%04x first-extra=%04x\n",
                World_Tag(),(unsigned)year,exp_qual_name(c),verdict,
                (unsigned)s_expQual.year,(unsigned)actualYear,ne,n,
                diff.matched,diff.missing,diff.extra,diff.duplicate_source,
                diff.duplicate_resident,diff.invalid_source+s_expQual.invalid[c],
                diff.invalid_resident,diff.first_missing,diff.first_extra);
        if(!ok)
            for(unsigned j=0;j<ne;++j) {
                const uint16_t club=s_expQual.ids[c][j];
                unsigned present=0u;
                for(unsigned z=0;z<n;++z)
                    if(actual[z]==club){present=1u;break;}
                if(!present)
                    fprintf(stderr,"[og-qual-next-team] world=%s year=%u "
                                   "competition=%s club=%04x "
                                   "status=MISSING_FROM_NEXT_OG_EDITION\n",
                            World_Tag(),(unsigned)year,
                            exp_qual_name(c),club);
            }
    }
    fprintf(stderr,"[og-qual-next] SUMMARY world=%s year=%u "
                   "status=%s checked=%u passed=%u failed=%u not-tested=%u "
                   "domestic-cups=NOT_TESTED_SOURCE_MAPPING "
                   "other-confederations=NOT_TESTED_ACCESS_MAPPING\n",
            World_Tag(),(unsigned)year,
            fail?"FAIL":untested?"PARTIAL":"PASS_EXACT_IDS",
            checked,pass,fail,untested);
}

/* Append-only EVIDENCE from OG, not a second result engine. This is
 * deliberately separate from the ABI-sensitive resident OG context.
 * Wire fields are endian-neutral: kind/country/slot/phase, edition year,
 * native sequence, oriented home/away, scores, leg, source, civil as-of.
 * The as-of day is a tick upper bound, NOT an invented fixture date. */
#define EXP_OG_FIXTURE_SIZE 20u
#define EXP_OG_FIXTURE_MAX 65536u
static uint8_t *s_expOGFixtures;
/* Open-addressed identity index: O(1) replay check even when 130+ native
 * leagues have interleaved 20,000+ results in one world season. */
#define EXP_OG_FIXTURE_BUCKETS 131072u
static uint32_t s_expOGFixtureBuckets[EXP_OG_FIXTURE_BUCKETS];
static size_t s_expOGFixtureCount,s_expOGFixtureCapacity;
static uint32_t s_expOGFixtureIssues,s_expOGFixturePartial;
static uint64_t s_expOGCapturedLeague,s_expOGCapturedCup,s_expOGCapturedUefa;
static ExpandedWorldDate s_expOGFixtureAsOf;
static uint32_t s_expOGTickNativeLeague,s_expOGTickNativeCup;
static uint64_t s_expOGTickCapturedLeague,s_expOGTickCapturedCup;
static uint16_t s_expOGTickYear;
static int s_expOGTickActive;
static void exp_fixture_put16(uint8_t *p,uint16_t value)
{p[0]=(uint8_t)(value>>8);p[1]=(uint8_t)value;}
static uint16_t exp_fixture_get16(const uint8_t *p)
{return (uint16_t)(((uint16_t)p[0]<<8)|p[1]);}
static void exp_fixture_put32(uint8_t *p,uint32_t value)
{p[0]=(uint8_t)(value>>24);p[1]=(uint8_t)(value>>16);
 p[2]=(uint8_t)(value>>8);p[3]=(uint8_t)value;}
static uint32_t exp_fixture_get32(const uint8_t *p)
{return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|
        ((uint32_t)p[2]<<8)|p[3];}
static uint64_t exp_fixture_hash(const uint8_t *bytes,size_t len)
{
    uint64_t h=UINT64_C(14695981039346656037);
    for(size_t i=0;i<len;i++){h^=bytes[i];h*=UINT64_C(1099511628211);}
    return h;
}
void Expanded_OGWorld_FixtureReset(void)
{
    free(s_expOGFixtures);s_expOGFixtures=NULL;
    memset(s_expOGFixtureBuckets,0,sizeof s_expOGFixtureBuckets);
    s_expOGFixtureCount=s_expOGFixtureCapacity=0u;
    s_expOGFixtureIssues=0u;s_expOGFixturePartial=0u;
    s_expOGCapturedLeague=s_expOGCapturedCup=s_expOGCapturedUefa=0u;
    memset(&s_expOGFixtureAsOf,0,sizeof s_expOGFixtureAsOf);
    s_expOGTickActive=0;
}
void Expanded_OGWorld_FixtureSetAsOf(ExpandedWorldDate date)
{s_expOGFixtureAsOf=date;}
static uint32_t exp_fixture_key_hash(const uint8_t *r)
{
    uint32_t h=2166136261u;
    for(unsigned i=0;i<12u;i++){h^=r[i];h*=16777619u;}
    h^=r[14];h*=16777619u;
    return h;
}
static int exp_fixture_key_same(const uint8_t *a,const uint8_t *b)
{return memcmp(a,b,12u)==0 && a[14]==b[14];}
/* Returns 1 if a duplicate stable witness key exists, 0 if a free bucket.
 * The same function is used for scratch validation on CAR restore. */
static int exp_fixture_locate(const uint8_t *wire,const uint32_t *buckets,
                              const uint8_t *record,uint32_t *position)
{
    uint32_t pos=exp_fixture_key_hash(record)&(EXP_OG_FIXTURE_BUCKETS-1u);
    for(uint32_t k=0;k<EXP_OG_FIXTURE_BUCKETS;k++) {
        const uint32_t entry=buckets[pos];
        if(!entry){*position=pos;return 0;}
        if(exp_fixture_key_same(wire+
               ((size_t)entry-1u)*EXP_OG_FIXTURE_SIZE,record)) {
            *position=pos;return 1;
        }
        pos=(pos+1u)&(EXP_OG_FIXTURE_BUCKETS-1u);
    }
    return -1;
}
static int exp_fixture_build_index(const uint8_t *wire,size_t count,
                                    uint32_t *buckets)
{
    memset(buckets,0,EXP_OG_FIXTURE_BUCKETS*sizeof(*buckets));
    for(size_t i=0;i<count;i++) {
        uint32_t pos=0u;
        if(exp_fixture_locate(wire,buckets,
            wire+i*EXP_OG_FIXTURE_SIZE,&pos)!=0)return 0;
        buckets[pos]=(uint32_t)i+1u;
    }
    return 1;
}
static int exp_fixture_append(const uint8_t *rec)
{
    /* Any exact replay from the same source is an error, not a second
     * score. The native progression counters provide the serial; other
     * callbacks are deliberately never invoked by this observer. */
    const size_t n=s_expOGFixtureCount;
    uint32_t pos=0u;
    const int match=exp_fixture_locate(s_expOGFixtures,
                                       s_expOGFixtureBuckets,rec,&pos);
    if(match<0){++s_expOGFixtureIssues;return 0;}
    if(match) {
        uint8_t *prior=s_expOGFixtures+
            ((size_t)s_expOGFixtureBuckets[pos]-1u)*EXP_OG_FIXTURE_SIZE;
        ++s_expOGFixtureIssues;
        fprintf(stderr,"[og-fixture-journal] %s world=%s "
                       "edition=%u country=%u slot=%u phase=%u "
                       "serial=%u home=%04x away=%04x "
                       "previous=%u-%u new=%u-%u source-old=%u new=%u "
                       "status=BLOCK_NEW_CAR_NO_REPLAY\n",
                prior[12]!=rec[12] || prior[13]!=rec[13]?
                    "SCORE_CONFLICT":"TWO_WRITERS",
                World_Tag(),(unsigned)exp_fixture_get16(rec+4),
                rec[1],rec[2],rec[3],
                (unsigned)exp_fixture_get16(rec+6),
                (unsigned)exp_fixture_get16(rec+8),
                (unsigned)exp_fixture_get16(rec+10),
                prior[12],prior[13],rec[12],rec[13],
                prior[15],rec[15]);
        return 0;
    }
    if(n>=EXP_OG_FIXTURE_MAX) {
        ++s_expOGFixtureIssues;
        fprintf(stderr,"[og-fixture-journal] CAPACITY_EXHAUSTED old-CAR-protected=1\n");
        return 0;
    }
    if(n==s_expOGFixtureCapacity) {
        size_t cap=s_expOGFixtureCapacity?s_expOGFixtureCapacity*2u:1024u;
        if(cap>EXP_OG_FIXTURE_MAX)cap=EXP_OG_FIXTURE_MAX;
        uint8_t *larger=(uint8_t *)realloc(s_expOGFixtures,
                                           cap*EXP_OG_FIXTURE_SIZE);
        if(!larger) {
            ++s_expOGFixtureIssues;
            fprintf(stderr,"[og-fixture-journal] ALLOCATION_FAILED no-save=1\n");
            return 0;
        }
        s_expOGFixtures=larger;s_expOGFixtureCapacity=cap;
    }
    memcpy(s_expOGFixtures+n*EXP_OG_FIXTURE_SIZE,rec,EXP_OG_FIXTURE_SIZE);
    s_expOGFixtureBuckets[pos]=(uint32_t)n+1u;
    ++s_expOGFixtureCount;
    return 1;
}
/* Only the last two JULY-JUNE observation seasons remain detailed. Old
 * edition conclusions/qualifiers continue to be held by native OG; this
 * does not claim to be the final Competition Engine archive format. */
void Expanded_OGWorld_FixturePrune(uint16_t openedYear)
{
    if(openedYear<=World_FirstYear()+1u)return;
    ExpandedWorldDate start={(uint16_t)(openedYear-1u),7u,1u};
    int32_t index=0;
    if(!ExpandedWorld_DateToIndex(start,&index))return;
    size_t kept=0u,discarded=0u;
    for(size_t i=0;i<s_expOGFixtureCount;i++) {
        uint8_t *row=s_expOGFixtures+i*EXP_OG_FIXTURE_SIZE;
        const uint32_t day=exp_fixture_get32(row+16);
        if(day<(uint32_t)index) {++discarded;continue;}
        if(kept!=i)memmove(s_expOGFixtures+kept*EXP_OG_FIXTURE_SIZE,
                           row,EXP_OG_FIXTURE_SIZE);
        ++kept;
    }
    s_expOGFixtureCount=kept;
    if(!exp_fixture_build_index(s_expOGFixtures,kept,
                                 s_expOGFixtureBuckets))
        ++s_expOGFixtureIssues;
    if(discarded)fprintf(stderr,"[og-fixture-journal] PRUNE opened=%u "
        "discarded=%lu retained=%lu cross-year=BY_OBSERVATION_DAY\n",
        (unsigned)openedYear,(unsigned long)discarded,(unsigned long)kept);
}
/* Diagnostic snapshots live only in RAM, never in portable career saves. */
static ExpandedF12Snapshot s_f12BeforeHire,s_f12AfterHire;
static ExpandedOGDashboard s_f12BeforePanel,s_f12AfterPanel;
static uint32_t s_f12DiffLogHash;
int Expanded_OGWorld_IsSimulating(void) {
    return s_expOGCurrent!=NULL || s_expOGCupCurrent!=NULL;
}
uint16_t Expanded_OGWorld_CurrentCupYear(void) {
    return s_expOGCupCurrent?s_expOGCupCurrent->year:0u;
}
void Expanded_OGWorld_CupMarkPlayed(void) {
    if(s_expOGCupCurrent)++s_expOGCupCurrent->matchesPlayed;
}
uint32_t Expanded_OGWorld_CupCount(void) {
    uint32_t count=0u;
    for(uint32_t k=0;k<s_expOGCupCount;++k)
        if(s_expOGCups[k].used)++count;
    return count;
}
uint32_t Expanded_OGWorld_CupMatches(void) {
    uint32_t count=0u;
    for(uint32_t k=0;k<s_expOGCupCount;++k)
        if(s_expOGCups[k].used)count+=s_expOGCups[k].matchesPlayed;
    return count;
}
uint32_t Expanded_OGWorld_CupUnsupported(void){return s_expOGCupUnsupported;}
uint32_t Expanded_OGWorld_CupFaults(void){return s_expOGCupFaults;}
/* -1: absent/unavailable, 0: not among this year's native entrants,
 *  1: club found in the *existing* resident OG cup draw. */
int Expanded_OGWorld_CupContainsTeam(uint8_t country,uint8_t slot,
                                     uint16_t year,uint16_t team)
{
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(c->country!=country || c->slot!=slot || c->year!=year)continue;
        if(!c->used || c->managed_excluded || c->unsupported_setup ||
           c->teamCount<2u || c->teamCount>EXP_OG_MAX_CUP_TEAMS)return -1;
        for(uint16_t j=0;j<c->teamCount;++j) {
            const TeamFile *t=&c->teams[j].team;
            if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==team)
                return 1;
        }
        return 0;
    }
    return -1;
}

/* The first historical Champions is already resident at START. Use the
 * exact native OG entrants (not a second copy's obsolete byte offsets)
 * to decide whether the employed manager has a slot-3 calendar entry.
 * -1: not resident, 0: not participating, 1: native participant. */
int Expanded_OGWorld_HistoricChampionsContainsTeam(uint16_t club)
{
    if(World_Current()!=WORLD_9697)return -1;
    const ExpandedOGLeague *ctx=&s_expOGUefa[0];
    if(!ctx->used || ctx->year!=s_expOGUefaCalendarYear ||
       ctx->teamCount!=16u || ctx->tournament.header.competitionType!=2)
        return -1;
    for(unsigned i=0;i<ctx->teamCount;++i) {
        const TeamFile *t=&ctx->teams[i].team;
        if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==club)
            return 1;
    }
    return 0;
}

/* First OpenSWOS 1996 Libertadores membership: 20 OG resident entrants.
 * The Mod's 1997 holder bye must NOT be spliced into the 1996 edition. */
int Expanded_OGWorld_HistoricCopaContainsTeam(uint16_t club)
{
    if(World_Current()!=WORLD_9697)return -1;
    const ExpandedOGLeague *ctx=&s_expOGUefa[1];
    if(!ctx->used || ctx->country!=82u ||
       ctx->year!=s_expOGUefaCalendarYear || ctx->teamCount!=20u ||
       ctx->tournament.header.competitionType!=2)return -1;
    for(unsigned i=0;i<ctx->teamCount;++i) {
        const TeamFile *t=&ctx->teams[i].team;
        if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==club)return 1;
    }
    return 0;
}

/* Only native, actually selected cup entrants are reported. Never synthesize
 * a missing participant to make the count match the definition. */
void Expanded_OGWorld_CupSetupAudit(uint8_t country,uint8_t slot,
                                    uint16_t year,unsigned *bracket,
                                    unsigned *ties,unsigned *pairs)
{
    if(bracket)*bracket=0u;
    if(ties)*ties=0u;
    if(pairs)*pairs=0u;
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(c->country!=country || c->slot!=slot || c->year!=year)continue;
        if(bracket)*bracket=c->observedBracketTeams;
        if(ties)*ties=c->observedRoundTies;
        if(pairs)*pairs=c->observedRoundPairs;
        break;
    }
}
unsigned Expanded_OGWorld_CupSelectedTeams(uint8_t country,uint8_t slot,
                                           uint16_t year)
{
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(c->country==country && c->slot==slot && c->year==year)
            return (unsigned)c->observedSelected;
    }
    return 0u;
}
const char *Expanded_OGWorld_CupCoverageReason(uint8_t country,uint8_t slot,
                                               uint16_t year,unsigned expected)
{
    if(expected<2u || expected>EXP_OG_MAX_CUP_TEAMS || (expected&1u))
        return "INVALID_NATIVE_BRACKET_SIZE";
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(c->country!=country || c->slot!=slot || c->year!=year)continue;
        if(c->managed_excluded)return "MANAGER_SLOT_NOT_BOUND";
        if(c->unsupported_setup) {
            if(c->observedSelected<expected)return "SELECTOR_UNDERFILLED";
            if(c->observedSelected>expected)return "SELECTOR_OVERFILLED";
            return "NATIVE_BRACKET_SETUP_INVALID";
        }
        if(!c->used)return "NATIVE_CONTEXT_UNAVAILABLE";
        /* OG intentionally pads bracket places when some clubs get a bye.
         * The real team selection remains smaller than base.numTeams.
         * This state was validated against the original draw and team IDs,
         * not accepted merely because its numbers look plausible. */
        if(c->nativeByeVerified && c->observedSelected<expected)
            return "NATIVE_BYES_VERIFIED";
        if(c->observedSelected!=expected)return "ENTRY_COUNT_MISMATCH";
        return "OK";
    }
    return "NATIVE_CONTEXT_NOT_CREATED";
}
void Expanded_OGWorld_InvalidFixture(int32_t a,int32_t b,uint16_t teamCount)
{
    if(!s_expOGCurrent)return;
    s_expOGFault=1;
    fprintf(stderr,"[og-world-fixture] INVALID country=%u division=%u "
                   "year=%u phase=%d teamA=%ld teamB=%ld loaded=%u "
                   "played=%u round=%u slot=%u\n",
            (unsigned)s_expOGCurrent->country,
            (unsigned)s_expOGCurrent->division,
            (unsigned)s_expOGCurrent->year,s_expOGPhase,
            (long)a,(long)b,(unsigned)teamCount,
            (unsigned)_diyCompetitionStart.matchesPlayedCount,
            (unsigned)_diyCompetitionStart.base.currentRoundIndex,
            (unsigned)_diyCompetitionStart.leagueCurrentSlot);
}
int Expanded_OGWorld_ViewActive(void) { return s_expOGViewing; }
extern uint8_t _seed2,_randXorIndex2,_randXorKey2;
extern uint8_t _careerLastMatchIndexByCompetitionSlot[];
extern void Career_AutoAdvanceInternationalCompetitionFixtures(uint8_t *,uint16_t);
extern void Career_AutoAdvanceLeagueFixtures(void);
extern void SaveSelectedTeams(void);
extern void RestoreSelectedTeams(void);
/* July-year rollover resets domestic OG states, but may preserve the
 * still-active original continental edition. Full reset remains obligatory
 * for a new career or switching world. No games are simulated here. */
static void expanded_og_world_reset_internal(int season_transition) {
    /* The witness journal belongs to ONE career, not to the global
     * process. Season rollover retains only its last two seasons. */
    if(!season_transition) {
        Expanded_OGWorld_FixtureReset();
        memset(&s_expQual,0,sizeof s_expQual);
    }
#if SWOS_WORLDS
    const int keep_continental =
        season_transition && World_Current()==WORLD_2627 &&
        ct_world_next_month()!=0xffu && CT_ED.mark==0x4543;
    if (!keep_continental)
        ct_world_reset();
    else
        fprintf(stderr,"[og-season-transition] CONTINENTAL_PRESERVED "
                       "edition=%u cursor=%u (no reset/no replay)\n",
                (unsigned)CT_ST.season,(unsigned)ct_world_next_month());
#else
    (void)season_transition;
#endif
    /* The July transition retires the season's NATIONAL league/cup
     * schedules. It must not erase the actual continental results while
     * next-year OG qualifications are still unconnected: those would be
     * irrecoverable at the next autosave. Keep only the final played
     * continental edition in the already-serialized OG slots, not a
     * falsely re-seeded next-year competition. New career/full reset
     * continues to clear everything. */
    const uint16_t oldContinentalYear=s_expOGUefaCalendarYear;
    unsigned retainedUefa=0u,retainedCups=0u;
    if(season_transition) {
        for(unsigned k=0;k<3u;++k)
            if(ExpandedOG_RetainContinentalType2(
                   season_transition,s_expOGUefa[k].used,
                   s_expOGUefa[k].year,oldContinentalYear))
                ++retainedUefa;
        for(uint32_t i=0;i<s_expOGCupCount;++i) {
            const ExpandedOGCup *c=&s_expOGCups[i];
            if(ExpandedOG_RetainContinentalType1(
                   season_transition,c->used,c->country,c->slot,
                   c->year,oldContinentalYear)) {
                if(retainedCups!=i)
                    memmove(&s_expOGCups[retainedCups],c,sizeof(*c));
                ++retainedCups;
            }
        }
    }
    memset(s_expOG,0,sizeof s_expOG);s_expOGCount=0;
    if(!retainedUefa) {
        memset(s_expOGUefa,0,sizeof s_expOGUefa);
        s_expOGUefaCalendarYear=0u;
        for(unsigned k=0;k<3u;++k)s_expOGUefa[k].lastMatch=0xffu;
    }
    s_expOGCurrent=NULL;s_expOGPhase=-1;s_expOGBound=0;s_expOGViewing=0;s_expOGFault=0;
    s_expOGBridgeBlocked=0;s_expOGBridgeRejections=0u;
    s_expOGManagerPairChecked=0u;s_expOGManagerPairRepaired=0u;
    s_expOGManagerPairMismatch=0u;
    memset(s_expOGCups+retainedCups,0,
           (EXP_OG_MAX_CUPS-retainedCups)*sizeof s_expOGCups[0]);
    s_expOGCupCount=retainedCups;s_expOGCupUnsupported=0;s_expOGCupFaults=0;
    s_expOGCupCurrent=NULL;s_expOGCupPhase=-1;s_expOGCupTickFault=0;
    if(season_transition && (retainedUefa || retainedCups))
        fprintf(stderr,"[og-season-retain] KEEP_LAST_CONTINENTAL_EDITION "
                "world=%s original-year=%u uefa-type2=%u classic-type1=%u "
                "results=READ_ONLY_IN_CAR new-edition=QUALIFICATION_PENDING\n",
                World_Tag(),(unsigned)oldContinentalYear,
                retainedUefa,retainedCups);
    /* A different career/world or season must not inherit the old hire
     * comparison. Old log snapshots remain in the diagnostic ZIP. */
    memset(&s_f12BeforeHire,0,sizeof s_f12BeforeHire);
    memset(&s_f12AfterHire,0,sizeof s_f12AfterHire);
    memset(&s_f12BeforePanel,0,sizeof s_f12BeforePanel);
    memset(&s_f12AfterPanel,0,sizeof s_f12AfterPanel);
    s_f12DiffLogHash=0u;
}
void Expanded_OGWorld_Reset(void) { expanded_og_world_reset_internal(0); }
void Expanded_OGWorld_ResetSeason(void) { expanded_og_world_reset_internal(1); }
uint32_t Expanded_OGWorld_LeagueCount(void){
    uint32_t resident=0u;
    for(uint32_t i=0;i<s_expOGCount;++i)
        if(s_expOG[i].used)++resident;
    return resident;
}
int Expanded_OGWorld_ClubBound(void){return s_expOGBound;}

/* After hire, club-local _selectedTeams is merely scratch for native menus.
 * GoalStats continue to belong to the existing resident OG league.
 * The viewer already installs a read-only image of the correct league. */
const TeamFileEx *Expanded_OGWorld_ScorerTeams(uint16_t *count)
{
    if(count)*count=0u;
    if(_gameType!=4)return NULL;
    if(s_expOGViewing && _numSelectedTeams>0u &&
       _numSelectedTeams<=EXP_OG_MAX_TEAMS) {
        if(count)*count=_numSelectedTeams;
        return _selectedTeams;
    }
    if(!s_expOGBound || _selTeamNumber==0xffffu)return NULL;
    const ExpandedOGLeague *best=NULL;
    for(uint32_t i=0;i<s_expOGCount;++i) {
        const ExpandedOGLeague *c=&s_expOG[i];
        if(!c->used || c->division!=(uint8_t)_currentDivisionIndex ||
           c->teamCount<2u || c->teamCount>EXP_OG_MAX_TEAMS)continue;
        for(uint16_t j=0;j<c->teamCount;++j) {
            const TeamFile *t=&c->teams[j].team;
            if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==_selTeamNumber) {
                if(!best || c->year>best->year)best=c;
                break;
            }
        }
    }
    if(!best)return NULL;
    if(count)*count=best->teamCount;
    return best->teams;
}
const TeamFileEx *Expanded_OGWorld_ScorerTeam(uint16_t teamId)
{
    uint16_t n=0u;
    const TeamFileEx *teams=Expanded_OGWorld_ScorerTeams(&n);
    for(uint16_t i=0;teams && i<n;++i) {
        const TeamFile *t=&teams[i].team;
        if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==teamId)
            return &teams[i];
    }
    return NULL;
}

/* Inspect only the real resident original-OG league contexts. Never
 * call simulation, consume an RNG value, or mutate the selected teams. */
void Expanded_OGWorld_Audit(ExpandedOGWorldAudit *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    out->club_bound=(uint32_t)(s_expOGBound!=0);
    uint8_t seen[256]={0};
    for(uint32_t i=0;i<s_expOGCount;++i) {
        const ExpandedOGLeague *c=&s_expOG[i];
        /* A known format mismatch is an explicit UNSUPPORTED division,
         * not a corrupted resident tournament and not a played fixture. */
        if(!c->used && c->lastMatch==0xfeu)continue;
        if(!c->used || c->teamCount<2u || c->teamCount>EXP_OG_MAX_TEAMS ||
           c->comp.base.numTeams<2 || c->comp.base.numTeams>EXP_OG_MAX_TEAMS ||
           c->comp.base.competitionType!=0) {
            ++out->invalid_contexts;
            continue;
        }
        ++out->resident_leagues;
        out->native_matches+=(uint32_t)c->comp.matchesPlayedCount;
        if(c->comp.matchesPlayedCount)++out->leagues_with_matches;
        if(c->comp.minSubstitutes==0 && c->comp.matchesPlayedCount)
            ++out->completed_leagues;
        if(!seen[c->country]) { seen[c->country]=1u; ++out->countries; }
        if(!out->min_year || c->year<out->min_year)out->min_year=c->year;
        if(c->year>out->max_year)out->max_year=c->year;
    }
}
/* World tick read-back: compares OG's actual resident match counters to
 * the number of native match callbacks observed this tick. Works for
 * employed and UNEMPLOYED. Never calls match simulation or RNG. */
extern uint16_t Expanded_OGWorld_SaveYear(void);
void Expanded_OGWorld_FixtureTickBegin(void)
{
    ExpandedOGWorldAudit a;
    Expanded_OGWorld_Audit(&a);
    s_expOGTickNativeLeague=a.native_matches;
    s_expOGTickNativeCup=Expanded_OGWorld_CupMatches();
    s_expOGTickCapturedLeague=s_expOGCapturedLeague;
    s_expOGTickCapturedCup=s_expOGCapturedCup;
    s_expOGTickYear=Expanded_OGWorld_SaveYear();
    s_expOGTickActive=1;
}
void Expanded_OGWorld_FixtureTickAbort(const char *reason)
{
    ++s_expOGFixtureIssues;
    fprintf(stderr,"[og-fixture-tick] ABORT reason=%s "
                   "prior-CAR-preserved=1\n",
                   reason?reason:"FAILED_NATIVE_TICK");
}
void Expanded_OGWorld_FixtureTickEnd(void)
{
    if(!s_expOGTickActive)return;
    s_expOGTickActive=0;
    ExpandedOGWorldAudit a;
    Expanded_OGWorld_Audit(&a);
    const uint32_t cups=Expanded_OGWorld_CupMatches();
    const uint64_t observedLeague=s_expOGCapturedLeague-s_expOGTickCapturedLeague;
    const uint64_t observedCup=s_expOGCapturedCup-s_expOGTickCapturedCup;
    const int crossed=Expanded_OGWorld_SaveYear()!=s_expOGTickYear;
    const int monotonic=!crossed &&
        a.native_matches>=s_expOGTickNativeLeague &&
        cups>=s_expOGTickNativeCup;
    const uint64_t dl=monotonic?
        (uint64_t)(a.native_matches-s_expOGTickNativeLeague):0u;
    const uint64_t dc=monotonic?
        (uint64_t)(cups-s_expOGTickNativeCup):0u;
    const int identical=monotonic && dl==observedLeague && dc==observedCup;
    if(!crossed && !identical) {
        ++s_expOGFixtureIssues;
        fprintf(stderr,"[og-fixture-tick] WRITE_BLOCKED "
                       "cause=CALLBACK_COUNTER_MISMATCH prior-CAR-preserved=1\n");
    }
    fprintf(stderr,"[og-fixture-tick] world=%s year=%u manager=%s "
                   "native-league-delta=%llu captured-league=%llu "
                   "native-cup-delta=%llu captured-cup=%llu "
                   "journal-count=%lu journal-issues=%u legacy-partial=%u "
                   "status=%s comparison=%s\n",
        World_Tag(),(unsigned)Expanded_OGWorld_SaveYear(),
        _selTeamNumber==0xffffu?"UNEMPLOYED":"EMPLOYED",
        (unsigned long long)dl,(unsigned long long)observedLeague,
        (unsigned long long)dc,(unsigned long long)observedCup,
        (unsigned long)s_expOGFixtureCount,(unsigned)s_expOGFixtureIssues,
        (unsigned)s_expOGFixturePartial,
        crossed?"CROSS_SEASON_SNAPSHOT":
        identical?"MATCH_COUNTERS_AND_CALLBACKS_EQUAL":"NOT_RECONCILED",
        "CPU_WORLD_ONLY_NO_MANAGER_MERGE");
}
/* An OG split can clear the league's GF table at the beginning of a new
 * stage without clearing season-to-date goals stored in TeamFileEx. Inspect
 * the same native LeagueSplitState and baked table used by LeagueSplit.
 * Do not infer any missing scores or alter the native state. */
static int exp_og_scorer_has_reset_stage(const ExpandedOGLeague *ctx)
{
    const LeagueSplitState *state=&ctx->comp.leagueSplit;
    int32_t count=0;
    const LeagueSplit *splits=World_LeagueSplits(&count);
    if(!splits || state->magic!=LEAGUE_SPLIT_MAGIC ||
       !state->entry || (int32_t)state->entry>count ||
       !state->frozen || state->compId!=ctx->comp.base.field_2E ||
       state->n!=ctx->comp.base.numTeams)return 0;
    const LeagueSplit *split=&splits[state->entry-1u];
    if(split->country!=ctx->country || split->division!=ctx->division ||
       !split->table || split->table[3]!=ctx->comp.base.numTeams)return 0;
    const uint8_t *tab=split->table;
    const unsigned stages=(unsigned)tab[7];
    if(stages==0u || stages>16u || state->frozen>stages)return 0;
    for(unsigned k=0;k<(unsigned)state->frozen;++k)
        if(tab[16u+28u*k+2u]&1u) /* LeagueSplit F_RESET */
            return 1;
    return 0;
}
/* Read-only native player/own-goal counters. In stage-reset leagues, the
 * season totals and the current standings cover DIFFERENT time windows. */
void Expanded_OGWorld_ScorerAudit(ExpandedOGScorerAudit *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    unsigned delta_examples=0u;
    for(uint32_t k=0;k<s_expOGCount;++k) {
        const ExpandedOGLeague *c=&s_expOG[k];
        if(!c->used || c->teamCount<2u ||
           c->teamCount>EXP_OG_MAX_TEAMS ||
           c->comp.base.competitionType!=0 ||
           c->comp.base.numTeams<2 ||
           c->comp.base.numTeams>EXP_OG_MAX_TEAMS)continue;
        ++out->resident_leagues;
        const unsigned teams=(unsigned)c->teamCount;
        uint32_t scorerGoals=0u,ownGoals=0u;
        for(unsigned j=0;j<teams;++j) {
            /* OG AssignSimulatedGoalsToScorers() increments this field
             * on a non-player (own-goal) event. teamInit() resets it at
             * the same season boundary as the player goal counters. */
            ownGoals+=(uint32_t)c->teams[j].team.writesZeroHere;
            for(unsigned i=0;i<16u;++i) {
                const unsigned goals=(unsigned)c->teams[j].players[i].goalStats[0];
                ++out->players;
                out->player_league_goals+=goals;
                scorerGoals+=goals;
                if(goals)++out->players_with_league_goals;
            }
        }
        unsigned sum=0u;
        const unsigned n=(unsigned)c->comp.base.numTeams;
        for(unsigned j=0;j<n;++j)
            sum+=(unsigned)c->comp.leagueStandings[j].goalsFor;
        if(sum)++out->leagues_with_goals;
        out->standings_goals+=sum;
        out->own_goals+=ownGoals;
        if(scorerGoals!=sum) {
            ++out->leagues_with_delta;
            if(scorerGoals>sum)++out->player_goals_greater;
            else ++out->standings_goals_greater;
            if(scorerGoals+ownGoals==sum)
                ++out->leagues_reconciled_by_own_goals;
        }
        const int32_t residual=(int32_t)(scorerGoals+ownGoals)-
                               (int32_t)sum;
        const int resetStage=exp_og_scorer_has_reset_stage(c);
        if(resetStage) {
            ++out->leagues_with_reset_standings;
            if(residual)++out->reset_scope_differences;
        } else if(residual) {
            ++out->leagues_with_residual_delta;
        }
#ifdef SWOS_SDL_BUILD
        /* Report all nonzero differences by category, with at most six
         * detailed examples per checkpoint. Stage-reset differences are
         * expected scope mismatches, NOT fabricated missing goals. */
        if(residual && delta_examples++<6u)
            fprintf(stderr,"[og-scorer-delta] country=%u division=%u "
                           "year=%u played=%u player-goals=%lu "
                           "own-goals=%lu player-plus-own=%lu "
                           "standings-gf=%u residual=%ld "
                           "status=%s\n",
                    (unsigned)c->country,(unsigned)c->division,
                    (unsigned)c->year,(unsigned)c->comp.matchesPlayedCount,
                    (unsigned long)scorerGoals,(unsigned long)ownGoals,
                    (unsigned long)(scorerGoals+ownGoals),sum,
                    (long)residual,
                    resetStage?"STAGE_RESET_DIFFERENT_SCOPE":
                               "UNEXPLAINED_SAME_SCOPE");
#endif
    }
}
/* F12 dashboard: all counters below are READ-ONLY. Coverage is per-country
 * and complete only when every defined OG division/cup slot for that country
 * has a current-year resident native context. No fixture generation, reset,
 * readjusted date, data imports or RNG consumption is allowed here. */
static int exp_debug_region(uint8_t country)
{
    extern QualifyingGroup _europeTable,_africaTable,_southAmericaTable;
    extern QualifyingGroup _northAmericaTable,_asiaTable,_oceaniaTable;
    const QualifyingGroup *const original[EXP_OG_DEBUG_REGIONS]={
        &_europeTable,&_africaTable,&_southAmericaTable,
        &_northAmericaTable,&_asiaTable,&_oceaniaTable
    };
    for(unsigned region=0;region<EXP_OG_DEBUG_REGIONS;++region) {
        const uint8_t *countries=World_ContinentCountries((uint8_t)(80u+region));
        unsigned cap=128u; /* world list is 255-terminated */
        if(!countries) {
            countries=(const uint8_t *)original[region]->countries;
            cap=(unsigned)sizeof(original[region]->countries) /
                (unsigned)sizeof(original[region]->countries[0]);
        }
        for(unsigned i=0;i<cap && countries[i]!=255u;++i)
            if(countries[i]==country)return (int)region;
    }
    return -1; /* A country outside the active world's region table is NOT guessed. */
}
static int exp_debug_live_league(const ExpandedOGLeague *c,uint16_t year)
{
    return c->used && c->year==year && c->teamCount>=2u &&
        c->teamCount<=EXP_OG_MAX_TEAMS &&
        c->comp.base.competitionType==0 &&
        c->comp.base.numTeams>=2;
}
static int exp_debug_live_cup(const ExpandedOGCup *c,uint16_t year)
{
    return c->used && c->year==year && !c->managed_excluded &&
        !c->unsupported_setup && c->teamCount>=2u &&
        c->teamCount<=EXP_OG_MAX_CUP_TEAMS;
}
static unsigned exp_debug_count_competitions(const void *packed_array,unsigned cap)
{
    unsigned count=0;
    /* QualifyingGroup is packed(1). Dereference pointers only after copying
     * their representation into naturally aligned local storage. */
    for(unsigned i=0;i<cap;++i) {
        Competition *item=NULL;
        memcpy(&item,(const uint8_t *)packed_array + i*sizeof item,sizeof item);
        const uintptr_t ptr=(uintptr_t)item;
        if(!ptr || ptr==(uintptr_t)-1)break;
        if(ptr<(uintptr_t)0x10000u)continue; /* native table special markers */
        ++count;
    }
    return count;
}
/* Original OG league standings read-only consistency checks. A multistage
 * competition can reset its table while matchesPlayedCount stays cumulative. */
static int exp_f12_standings_coherent(const ExpandedOGLeague *ctx)
{
    const unsigned n=(unsigned)ctx->comp.base.numTeams;
    uint64_t gp=0,w=0,d=0,l=0,gf=0,ga=0;
    if(n<2u || n>64u || n>ctx->teamCount)return 0;
    for(unsigned i=0;i<n;++i) {
        DIYTeamStats aligned_stat;
        memcpy(&aligned_stat,
               (const uint8_t *)&ctx->comp +
                   offsetof(DIYCompetitionEx,leagueStandings) +
                   i * sizeof aligned_stat,
               sizeof aligned_stat);
        const DIYTeamStats *s=&aligned_stat;
        if((uint32_t)s->gamesPlayed!=(uint32_t)s->wins+s->draws+s->losses)
            return 0;
        gp+=s->gamesPlayed;w+=s->wins;d+=s->draws;l+=s->losses;
        gf+=s->goalsFor;ga+=s->goalsAgainst;
    }
    return !(gp&1u) && w==l && !(d&1u) && gf==ga &&
           gp<=(uint64_t)ctx->comp.matchesPlayedCount*2u;
}
void Expanded_OGWorld_Dashboard(ExpandedOGDashboard *out)
{
    extern int Expanded_OGWorld_SaveStarted(void);
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    extern uint8_t _careerCompetitionIdRemapTable[];
    extern void *_competitionsTable[];
    extern QualifyingGroup _worldTable,_worldCupQualifyingGroups,_ecQualifyingGroups;
    extern QualifyingGroup _africaTable,_southAmericaTable,_northAmericaTable;
    extern QualifyingGroup _asiaTable,_oceaniaTable;
    extern Competition *_continentalCups[];
    if(!out)return;
    memset(out,0,sizeof(*out));
    if(_gameType!=4 || !Expanded_OGWorld_SaveStarted())return;
    out->ready=1u;
    out->worldYear=Expanded_OGWorld_SaveYear();
    const uint16_t year=out->worldYear;
    const uint8_t *countries=World_SeasonCountries();
    if(!countries)countries=_careerCompetitionIdRemapTable;
    /* The same civil-to-native phase conversion used by FORWARD TIME:
     * at START on July 1, native phase is 48, even with zero matches. */
    ExpandedLiveCalendarState live;
    if(ExpandedWorldLive_CalendarState(&live)) {
        const uint16_t civilYear=(uint16_t)(live.today.year-
                                              (live.today.month<7u?1u:0u));
        uint8_t phase=0u;
        if(civilYear==year &&
           ExpandedWorld_DateToNativePhaseNext(year,live.today,&phase)) {
            out->nativePhase=phase;
            out->phaseReady=1u;
        }
    }

    int32_t worldCupDefs=0;
    const WorldCup *worldCups=World_Cups(&worldCupDefs);
    for(unsigned i=0;countries && i<256u && countries[i]!=255u;++i) {
        const uint8_t country=countries[i];
        const int region=exp_debug_region(country);
        if(region<0)continue;
        const void *const *table=(const void *const *)_competitionsTable[country];
        if(!table)continue;

        unsigned divisions=0,liveDivisions=0,advancedDivisions=0;
        unsigned inPhaseDivisions=0,brokenDivisions=0;
        const uintptr_t first=(uintptr_t)table[0];
        if(first>0x10000u && first!=(uintptr_t)-1 &&
           first!=(uintptr_t)-2) {
            const Competition *rec=(const Competition *)first;
            if(rec->type==0 && !comp_is_type0_cup(rec->compId)) {
                const CompetitionLeague *league=(const CompetitionLeague *)rec;
                const unsigned count=league->header.divisionsCount;
                if(count>=1u && count<=16u) {
                    for(unsigned d=0;d<count;++d) {
                        if(league->divisions[d].teams<2u)continue;
                        ++divisions;
                        ++out->divisionsExpected[region];
                        for(uint32_t j=0;j<s_expOGCount;++j) {
                            const ExpandedOGLeague *ctx=&s_expOG[j];
                            if(ctx->country!=country || ctx->division!=d ||
                               !exp_debug_live_league(ctx,year))continue;
                            ++liveDivisions;
                            ++out->divisionsResident[region];
                            out->leagueMatches[region]+=
                                (uint32_t)ctx->comp.matchesPlayedCount;
                            if(ctx->comp.matchesPlayedCount>0u)++advancedDivisions;
                            ++out->fixturesUnavailable; /* no dated fixture ledger */
                            if(!exp_f12_standings_coherent(ctx)) {
                                ++brokenDivisions;
                                ++out->resultsIssues;
                            } else {
                                ++out->resultsChecked;
                                if(out->phaseReady &&
                                   ctx->lastPhase==out->nativePhase) {
                                    ++inPhaseDivisions;
                                    ++out->divisionsInPhase[region];
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        /* State competitions are a separate denominator; national Brazilian
         * leagues must not be judged against thirteen unconnected states. */
        const WorldStates *stateWorld=World_States(country);
        if(stateWorld && stateWorld->contest && stateWorld->states>0u)
            out->statesExpected+=(uint32_t)stateWorld->states;
        if(divisions) {
            ++out->leagueExpected[region];
            if(liveDivisions==divisions)++out->leagueResident[region];
            if(advancedDivisions==divisions)++out->leagueAdvanced[region];
            if(inPhaseDivisions==divisions)++out->leagueInPhase[region];
            if(brokenDivisions)++out->leagueIssues[region];
        }

        /* Two OG national-cup slots, plus the active Mod's explicit cup
         * definitions (which may not yet have been connected to the table).
         * Slot 1 in WorldCup == table[2]; slot 2 == table[3]. */
        unsigned expectedMask=0u;
        for(unsigned slot=2;slot<=3u;++slot) {
            const uintptr_t ptr=(uintptr_t)table[slot];
            if(!ptr || ptr==(uintptr_t)-1)break;
            if(ptr==(uintptr_t)-2 || ptr<0x10000u)continue;
            if(((const Competition *)ptr)->type==1)
                expectedMask|=1u<<(slot-2u);
        }
        for(int32_t j=0;worldCups && j<worldCupDefs;++j)
            if(worldCups[j].country==country &&
               worldCups[j].slot>=1u && worldCups[j].slot<=2u)
                expectedMask|=1u<<(worldCups[j].slot-1u);
        if(expectedMask) {
            unsigned residentMask=0u,advancedMask=0u,inPhaseMask=0u;
            ++out->cupExpected[region];
            for(uint32_t j=0;j<s_expOGCupCount;++j) {
                const ExpandedOGCup *ctx=&s_expOGCups[j];
                if(ctx->country!=country || ctx->slot<2u ||
                   ctx->slot>3u || !exp_debug_live_cup(ctx,year))
                    continue;
                const unsigned bit=1u<<(ctx->slot-2u);
                if(!(expectedMask&bit) || (residentMask&bit))continue;
                residentMask|=bit;
                out->cupMatches[region]+=ctx->matchesPlayed;
                ++out->fixturesUnavailable; /* totals, not dated fixture results */
                if(ctx->matchesPlayed)advancedMask|=bit;
                if(out->phaseReady && ctx->lastPhase==out->nativePhase &&
                   ctx->comp.base.competitionType==1)
                    inPhaseMask|=bit;
            }
            if((inPhaseMask&expectedMask)==expectedMask)
                ++out->cupInPhase[region];
            if((residentMask&expectedMask)==expectedMask)
                ++out->cupResident[region];
            if((advancedMask&expectedMask)==expectedMask)
                ++out->cupAdvanced[region];
        }
    }

    /* European club competitions live in the three dedicated OG contexts.
     * In the historical era the second context is the distinct 1996 Copa
     * Libertadores, and two more European cups are in native cup contexts
     * with country 254. Modern extra-European CT definitions are NOT counted
     * resident until they have individual native contexts and match state. */
    const int historical=World_Current()==WORLD_9697;
    int32_t nConti=0;
    (void)World_ContiCups(&nConti);
    out->internationalExpected=historical?4u:
        (uint32_t)(3+(nConti>0?nConti:0));
    out->uefaExpected=3u; /* 96: Champions, CWC, UEFA Cup; 26: CL, EL, ECL */
    out->otherExpected=historical?1u:(uint32_t)(nConti>0?nConti:0);
    for(unsigned k=0;k<(historical?2u:3u);++k) {
        const ExpandedOGLeague *ctx=&s_expOGUefa[k];
        if(!ctx->used || ctx->year!=year ||
           ctx->teamCount<2u || ctx->teamCount>EXP_OG_MAX_TEAMS ||
           ctx->tournament.header.competitionType!=2)continue;
        ++out->internationalResident;
        out->internationalMatches+=(uint32_t)ctx->comp.matchesPlayedCount;
        ++out->fixturesUnavailable;
        if(ctx->comp.matchesPlayedCount>0u)++out->internationalAdvanced;
        if(out->phaseReady && ctx->lastPhase==out->nativePhase)
            ++out->internationalInPhase;
        /* The 1996 slot 1 is the original Copa Libertadores, NOT UEFA. */
        if(historical && k==1u) {
            ++out->otherResident;
            out->otherMatches+=(uint32_t)ctx->comp.matchesPlayedCount;
            if(out->phaseReady && ctx->lastPhase==out->nativePhase)
                ++out->otherInPhase;
        } else {
            ++out->uefaResident[k];
            out->uefaMatches[k]+=(uint32_t)ctx->comp.matchesPlayedCount;
            if(out->phaseReady && ctx->lastPhase==out->nativePhase)
                ++out->uefaInPhase[k];
        }
    }
    if(historical) {
        for(uint32_t j=0;j<s_expOGCupCount;++j) {
            const ExpandedOGCup *ctx=&s_expOGCups[j];
            if(ctx->country!=254u || ctx->slot<1u || ctx->slot>2u ||
               !exp_debug_live_cup(ctx,year))continue;
            ++out->internationalResident;
            out->internationalMatches+=ctx->matchesPlayed;
            ++out->fixturesUnavailable;
            if(ctx->matchesPlayed)++out->internationalAdvanced;
            if(out->phaseReady && ctx->lastPhase==out->nativePhase)
                ++out->internationalInPhase;
            ++out->uefaResident[ctx->slot];
            out->uefaMatches[ctx->slot]+=ctx->matchesPlayed;
            if(out->phaseReady && ctx->lastPhase==out->nativePhase)
                ++out->uefaInPhase[ctx->slot];
        }
    }

    /* National-team finals and qualifying competitions are definitions
     * only in the CURRENT worldwide resident architecture. Count genuine
     * source definitions; never call them resident because a PRESET viewer
     * exists or the manager has an unrelated local slot. */
    out->nationalExpected+=exp_debug_count_competitions(
        ((const uint8_t *)&_worldTable + offsetof(QualifyingGroup,compCup)),9u);
    out->nationalExpected+=exp_debug_count_competitions(
        _continentalCups,7u);
    out->nationalExpected+=exp_debug_count_competitions(
        ((const uint8_t *)&_worldCupQualifyingGroups + offsetof(QualifyingGroup,compCup)),9u);
    out->nationalExpected+=exp_debug_count_competitions(
        ((const uint8_t *)&_ecQualifyingGroups + offsetof(QualifyingGroup,compCup)),9u);
    QualifyingGroup *const regionalQualifiers[5]={
        &_africaTable,&_southAmericaTable,&_northAmericaTable,
        &_asiaTable,&_oceaniaTable
    };
    for(unsigned k=0;k<5u;++k) {
        const uintptr_t ptr=(uintptr_t)regionalQualifiers[k]->compCup[0];
        if(ptr>0x10000u && ptr!=(uintptr_t)-1)++out->nationalExpected;
    }
}

/* Observe exactly the same resident keys before and after employment.
 * This is an API-level viewer-readiness check; the actual picker focus/B
 * navigation still needs the user's Windows interaction test. */
static int exp_f12_view_league(const ExpandedOGLeague *c)
{
    extern void *_competitionsTable[];
    const void *const *table=(const void *const *)_competitionsTable[c->country];
    if(!table || (uintptr_t)table[0]<0x10000u ||
       (uintptr_t)table[0]==(uintptr_t)-1 ||
       (uintptr_t)table[0]==(uintptr_t)-2)return 0;
    const Competition *native=(const Competition *)table[0];
    const unsigned n=(unsigned)c->comp.base.numTeams;
    if(native->type!=0 || c->division>=16u ||
       n<2u || n>EXP_OG_MAX_TEAMS || n>c->teamCount)return 0;
    for(unsigned i=0;i<n;++i) {
        const unsigned off=(unsigned)c->comp.matchData.league.teamEntryOffsets[i];
        const int32_t team=c->comp.leagueStandings[i].teamSelOffset;
        if(off%18u || off/18u>=n || team<0 ||
           (uint32_t)team>=c->teamCount)return 0;
    }
    return 1;
}
static int exp_f12_view_cup(const ExpandedOGCup *c)
{
    extern void *_competitionsTable[];
    const Competition *native=NULL;
    if(c->country==254u && World_Current()==WORLD_9697 &&
       (c->slot==1u || c->slot==2u)) {
        native=c->slot==1u?(const Competition *)&_cupWinnersCup
                          :(const Competition *)&_uefaCup;
    } else if(c->slot<=3u) {
        const void *const *table=(const void *const *)_competitionsTable[c->country];
        if(table && (uintptr_t)table[c->slot]>=0x10000u &&
           (uintptr_t)table[c->slot]!=(uintptr_t)-1 &&
           (uintptr_t)table[c->slot]!=(uintptr_t)-2)
            native=(const Competition *)table[c->slot];
    }
    const unsigned n=(unsigned)c->comp.base.numTeams;
    return native && native->type==1 && native->compId==c->compId &&
           n>=2u && n<=EXP_OG_MAX_CUP_TEAMS &&
           c->teamCount>=2u && c->teamCount<=EXP_OG_MAX_CUP_TEAMS;
}
static void exp_f12_append(ExpandedF12Snapshot *s,uint8_t kind,
                           uint8_t country,uint8_t slot,uint16_t year,
                           uint8_t phase,uint32_t matches,uint8_t viewable)
{
    if(s->count>=EXP_F12_MAX_CONTEXTS) {
        s->valid=0u; /* never silently truncate and claim no regression */
        return;
    }
    s->rows[s->count++]=(ExpandedF12Context){
        kind,country,slot,viewable,phase,year,matches
    };
}
static void exp_f12_capture(ExpandedF12Snapshot *s,
                            ExpandedOGDashboard *panel)
{
    memset(s,0,sizeof(*s));
    Expanded_OGWorld_Dashboard(panel); /* uses only native read-only data */
    if(!panel->ready)return;
    s->valid=1u;s->world=(uint8_t)World_Current();s->year=panel->worldYear;
    for(uint32_t i=0;i<s_expOGCount;++i) {
        const ExpandedOGLeague *c=&s_expOG[i];
        if(!c->used || c->year!=s->year)continue;
        exp_f12_append(s,0u,c->country,c->division,c->year,c->lastPhase,
                       (uint32_t)c->comp.matchesPlayedCount,
                       (uint8_t)exp_f12_view_league(c));
    }
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(!c->used || c->year!=s->year)continue;
        exp_f12_append(s,1u,c->country,c->slot,c->year,c->lastPhase,
                       c->matchesPlayed,(uint8_t)exp_f12_view_cup(c));
    }
    for(unsigned i=0;i<(World_Current()==WORLD_9697?2u:3u);++i) {
        const ExpandedOGLeague *c=&s_expOGUefa[i];
        if(!c->used || c->year!=s->year)continue;
        const unsigned entrants=World_Current()==WORLD_2627?36u:
                                (i==1u?20u:16u);
        const uint8_t viewable=(uint8_t)(c->teamCount==entrants &&
             c->tournament.header.competitionType==2);
        exp_f12_append(s,2u,c->country,(uint8_t)i,c->year,c->lastPhase,
                       (uint32_t)c->comp.matchesPlayedCount,viewable);
    }
    if(!s->valid)
        fprintf(stderr,"[f12-hire] OVERFLOW inventory count=%u cap=%u "
                       "NOT_COMPARABLE\n",(unsigned)s->count,
                       (unsigned)EXP_F12_MAX_CONTEXTS);
}
static void exp_f12_log_differences(const ExpandedF12Snapshot *before,
                                    const ExpandedF12Snapshot *after,
                                    const ExpandedF12Diff *d)
{
    if(!d->comparable)return;
    for(uint16_t i=0;i<before->count;++i) {
        const ExpandedF12Context *b=&before->rows[i];
        const ExpandedF12Context *a=ExpandedF12_Find(after,b);
        if(!a || (b->viewable != a->viewable) ||
           (a && (a->matches<b->matches || a->phase<b->phase))) {
            fprintf(stderr,"[f12-hire-detail] kind=%u country=%u slot=%u "
                "year=%u PRE_VIEW=%u POST_VIEW=%u PRE_GAMES=%lu "
                "POST_GAMES=%lu PRE_PHASE=%u POST_PHASE=%u status=%s\n",
                (unsigned)b->kind,(unsigned)b->country,(unsigned)b->slot,
                (unsigned)b->year,(unsigned)b->viewable,
                a?(unsigned)a->viewable:0u,(unsigned long)b->matches,
                a?(unsigned long)a->matches:0ul,(unsigned)b->phase,
                a?(unsigned)a->phase:0u,!a?"MISSING":
                b->viewable&&!a->viewable?"VIEW_LOST":
                !b->viewable&&a->viewable?"VIEW_GAINED":"STATE_REGRESSED");
        }
    }
    for(uint16_t i=0;i<after->count;++i) {
        const ExpandedF12Context *a=&after->rows[i];
        if(!ExpandedF12_Find(before,a))
            fprintf(stderr,"[f12-hire-detail] kind=%u country=%u slot=%u "
                "year=%u status=NEW_POST_HIRE games=%lu phase=%u view=%u\n",
                (unsigned)a->kind,(unsigned)a->country,(unsigned)a->slot,
                (unsigned)a->year,(unsigned long)a->matches,
                (unsigned)a->phase,(unsigned)a->viewable);
    }
}
void Expanded_OGWorld_DebugHireCheckpoint(uint8_t after)
{
    if(!after) {
        exp_f12_capture(&s_f12BeforeHire,&s_f12BeforePanel);
        memset(&s_f12AfterHire,0,sizeof s_f12AfterHire);
        memset(&s_f12AfterPanel,0,sizeof s_f12AfterPanel);
        s_f12DiffLogHash=0u;
        fprintf(stderr,"[f12-hire] PRE world=%s year=%u ready=%u "
                       "contexts=%u\n",World_Tag(),
                (unsigned)s_f12BeforeHire.year,
                (unsigned)s_f12BeforeHire.valid,
                (unsigned)s_f12BeforeHire.count);
        return;
    }
    exp_f12_capture(&s_f12AfterHire,&s_f12AfterPanel);
    ExpandedF12Diff diff;
    ExpandedF12_Compare(&s_f12BeforeHire,&s_f12AfterHire,&diff);
    fprintf(stderr,"[f12-hire] POST world=%s year=%u ready=%u contexts=%u "
                   "comparable=%u missing=%lu new=%lu viewLost=%lu "
                   "viewGained=%lu gamesBack=%lu phaseBack=%lu dup=%lu\n",
            World_Tag(),(unsigned)s_f12AfterHire.year,
            (unsigned)s_f12AfterHire.valid,(unsigned)s_f12AfterHire.count,
            (unsigned)diff.comparable,(unsigned long)diff.missing,
            (unsigned long)diff.appeared,(unsigned long)diff.viewLost,
            (unsigned long)diff.viewGained,(unsigned long)diff.matchesBack,
            (unsigned long)diff.phaseBack,(unsigned long)diff.duplicates);
    exp_f12_log_differences(&s_f12BeforeHire,&s_f12AfterHire,&diff);
}
/* SaveStarted is needed below (Dashboard's earlier prototype was local).
 * SaveYear is already declared before FixtureTickBegin. */
extern int Expanded_OGWorld_SaveStarted(void);
/* F12 never runs the next-season qualification callback: inspect only
 * source IDs previously recorded from OG and the current native residents.
 * This count is scoped to the four known routes, NOT all world cups. */
void Expanded_OGWorld_IntegrityAudit(ExpandedOGIntegrityAudit *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    out->fixtureRecords=(uint32_t)s_expOGFixtureCount;
    out->fixtureIssues=s_expOGFixtureIssues;
    out->fixturePartial=s_expOGFixturePartial;
    out->bridgeRejections=s_expOGBridgeRejections;
    out->managerPairChecked=s_expOGManagerPairChecked;
    out->managerPairRepaired=s_expOGManagerPairRepaired;
    out->managerPairMismatched=s_expOGManagerPairMismatch;
    out->bridgeBlocked=(uint8_t)(s_expOGBridgeBlocked!=0);
    out->cupFaults=s_expOGCupFaults;
    out->cupUnsupported=s_expOGCupUnsupported;
    out->unemployedQualErrors=(uint32_t)s_expOGQualUnemployedError;
    out->qualYear=Expanded_OGWorld_SaveYear();
    if(!Expanded_OGWorld_SaveStarted() ||
       out->qualYear<=World_FirstYear())return;
    if(!s_expQual.valid ||
       !ExpandedQual_SameEdition(s_expQual.world,s_expQual.year,
                                 (unsigned)World_Current(),out->qualYear)) {
        out->qualUnknown=4u;
        return;
    }
    out->qualSourceReady=1u;
    for(unsigned c=0;c<4u;++c) {
        uint16_t resident[EXP_QUAL_MAX_ENTRANTS]={0};
        uint16_t residentYear=0u;
        int available=0;
        const unsigned count=exp_qual_resident(
            c,out->qualYear,resident,&residentYear,&available);
        const unsigned source=s_expQual.count[c];
        out->qualSourceCount[c]=(uint16_t)source;
        out->qualResidentCount[c]=(uint16_t)count;
        if(s_expQual.invalid[c]) {
            out->qualRouteStatus[c]=2u;
            ++out->qualSourceInvalid;
            ++out->qualFail;
            ++out->qualChecked;
        } else if(source<2u || source>EXP_QUAL_MAX_ENTRANTS) {
            /* First edition and unsupported entry paths have no source
             * proof. An empty list matching another empty list is NOT OK. */
            ++out->qualUnknown;
        } else {
            const ExpandedQualDiff d=ExpandedQual_Compare(
                s_expQual.ids[c],source,resident,count);
            ++out->qualChecked;
            if(available && residentYear==out->qualYear &&
               count>=2u && ExpandedQual_IsExact(&d)) {
                out->qualRouteStatus[c]=1u;
                ++out->qualPass;
            } else {
                out->qualRouteStatus[c]=2u;
                ++out->qualFail;
            }
        }
    }
}
void Expanded_OGWorld_DebugHireAudit(ExpandedOGHireAudit *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    out->before=s_f12BeforePanel;
    out->beforeReady=s_f12BeforeHire.valid;
    /* Exact snapshots taken immediately before/after the CONTRACT, even
     * if the calendar has subsequently advanced. Keep the existing live
     * comparison separately for regressions later in the season. */
    out->atHire=s_f12AfterPanel;
    out->atHireReady=s_f12AfterHire.valid;
    ExpandedF12_Compare(&s_f12BeforeHire,&s_f12AfterHire,&out->hireDiff);
    if(!s_f12AfterHire.valid)return;
    /* Re-evaluate after each FORWARD TIME too: a competition lost a week
     * after employment must not escape the hire regression detector. */
    ExpandedF12Snapshot live;
    exp_f12_capture(&live,&out->after);
    out->afterReady=live.valid;
    ExpandedF12_Compare(&s_f12BeforeHire,&live,&out->diff);
    if(out->diff.comparable &&
       (out->diff.missing || out->diff.appeared || out->diff.viewLost ||
        out->diff.viewGained || out->diff.matchesBack ||
        out->diff.phaseBack || out->diff.duplicates)) {
        uint32_t hash=2166136261u;
        for(uint16_t i=0;i<live.count;++i) {
            const ExpandedF12Context *c=&live.rows[i];
            hash=(hash^(unsigned)c->kind)*16777619u;
            hash=(hash^(unsigned)c->country)*16777619u;
            hash=(hash^(unsigned)c->slot)*16777619u;
            hash=(hash^(unsigned)c->viewable)*16777619u;
            hash=(hash^(unsigned)c->matches)*16777619u;
            hash=(hash^(unsigned)c->phase)*16777619u;
        }
        if(hash!=s_f12DiffLogHash) {
            s_f12DiffLogHash=hash;
            fprintf(stderr,"[f12-hire] LIVE regression: missing=%lu "
                           "new=%lu viewLost=%lu gamesBack=%lu "
                           "phaseBack=%lu duplicates=%lu\n",
                    (unsigned long)out->diff.missing,
                    (unsigned long)out->diff.appeared,
                    (unsigned long)out->diff.viewLost,
                    (unsigned long)out->diff.matchesBack,
                    (unsigned long)out->diff.phaseBack,
                    (unsigned long)out->diff.duplicates);
            exp_f12_log_differences(&s_f12BeforeHire,&live,&out->diff);
        }
    }
}

static ExpandedOGLeague *exp_find(uint8_t country,uint8_t division,uint16_t year) {
    for(uint32_t i=0;i<s_expOGCount;++i)
        if((s_expOG[i].used || s_expOG[i].lastMatch==0xfeu) &&
           s_expOG[i].country==country &&
           s_expOG[i].division==division && s_expOG[i].year==year)return &s_expOG[i];
    return NULL;
}
int Expanded_OGWorld_AdvanceLeague(uint8_t *record,uint8_t country,
                                   uint8_t division,uint16_t year,
                                   uint8_t phase) {
    if(!record||_gameType!=4 || s_expOGCurrent)return 0;
    const uint8_t compId=(uint8_t)((Competition *)record)->compId;
    ExpandedOGLeague *ctx=exp_find(country,division,year);
    if(ctx && !ctx->used && ctx->lastMatch==0xfeu)
        return -1; /* a diagnosed native record mismatch: never retry/panic */
    /* The local manager OG context owns every fixture of this league.
     * Verify the native club is actually resident: a stale bound flag
     * after a job change cannot freeze an unrelated competition. */
    int manager_resident=0;
    if(ctx && ctx->used && ctx->teamCount<=EXP_OG_MAX_TEAMS &&
       _selTeamNumber!=0xffffu)
        for(unsigned i=0;i<ctx->teamCount;i++) {
            const TeamFile *t=&ctx->teams[i].team;
            if(PACK_BE_U16(t->countryNumber,t->teamOrdinal)==
               _selTeamNumber){manager_resident=1;break;}
        }
    if(manager_resident && s_expOGBound &&
       !_youreSacked && _selTeamNumber!=0xffffu &&
       World_FileCountry((uint8_t)(_selTeamNumber>>8))==country &&
       (uint8_t)_currentDivisionIndex==division) {
        if(phase>ctx->lastPhase)
            fprintf(stderr,"[og-manager-bridge] DEFER_RESIDENT_CPU "
                "country=%u division=%u year=%u phase=%u "
                "matches=%u owner=LOCAL_OG\n",
                (unsigned)country,(unsigned)division,(unsigned)year,
                (unsigned)phase,(unsigned)ctx->comp.matchesPlayedCount);
        return 1;
    }
    if(ctx && phase<=ctx->lastPhase)return 1;
    if(!ctx) {
        if(s_expOGCount==EXP_OG_MAX_LEAGUES)return 0;
        ctx=&s_expOG[s_expOGCount++];
        memset(ctx,0,sizeof(*ctx));
        ctx->country=country;ctx->division=division;ctx->year=year;
        ctx->lastMatch=0xffu;
    }
    if(ctx->used && phase<=ctx->lastPhase)return 1; /* phase already applied */
#ifdef SWOS_SDL_BUILD
    /* Session 22 proved that even the original TEAM.012 file contains
     * 14 first-division flags against a native record requiring 12.
     * Before entering OG's selection code (which panics on mismatch), audit
     * the ORIGINAL selected-team metadata for every historical country.
     * Do not delete clubs, rewrite the competition size or invent results.
     * An unsupported league is isolated; other native leagues still START. */
    if(World_Current()==WORLD_9697 && !ctx->used) {
        extern int16_t LoadTeamFile(uint16_t);
        extern uint8_t _teamFileBuffer[];
        extern uint16_t _teamsLoaded;
        const CompetitionLeague *native=(const CompetitionLeague *)record;
        const unsigned expected=(unsigned)native->divisions[division].teams;
        _teamsLoaded=0; /* force source re-read and season league adjustments */
        if(LoadTeamFile(country)!=0)return 0;
        const unsigned n=(unsigned)*(const uint16_t *)_teamFileBuffer;
        const TeamFileEx *entries=(const TeamFileEx *)(_teamFileBuffer+2);
        unsigned actual=0;
        if(n>256u)return 0;
        for(unsigned i=0;i<n;++i)
            if(entries[i].team.league==division)++actual;
        if(actual!=expected) {
            fprintf(stderr,"[og-native-league-preflight] UNSUPPORTED "
                           "world=9697 country=%u division=%u year=%u "
                           "expected=%u actual=%u file-count=%u "
                           "reason=NATIVE_DATA_FORMAT_MISMATCH "
                           "no-matches-generated\n",
                    (unsigned)country,(unsigned)division,(unsigned)year,
                    expected,actual,n);
            ctx->lastMatch=0xfeu; /* distinct from 0xff uninitialized */
            ctx->lastPhase=phase;
            return -1;
        }
    }
#endif
    DIYCompetitionEx oldComp;
    DIYTournamentState oldTournament;
    const uint8_t last=_careerLastMatchIndexByCompetitionSlot[compId];
    const uint8_t oldSeason=_seasonPlaying,oldView=_viewRandomPhase;
    const uint16_t oldFlag=_internationalAutoAdvanceFlag;
    memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
    memcpy(&oldTournament,&_diyFileBufferCopy,sizeof oldTournament);
    SaveSelectedTeams();
    _internationalAutoAdvanceFlag=1;
    _seasonPlaying=(uint8_t)((96u+(uint32_t)year-World_FirstYear())%100u);
    s_expOGFault=0;
    s_expOGCurrent=ctx;s_expOGPhase=phase;
    Career_AutoAdvanceInternationalCompetitionFixtures(record,division);
    const int failed=s_expOGFault;
    s_expOGCurrent=NULL;s_expOGPhase=-1;
    _careerLastMatchIndexByCompetitionSlot[compId]=last;
    _seasonPlaying=oldSeason;_viewRandomPhase=oldView;
    _internationalAutoAdvanceFlag=oldFlag;
    RestoreSelectedTeams();
    memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
    memcpy(&_diyFileBufferCopy,&oldTournament,sizeof oldTournament);
    return !failed && ctx->used!=0;
}

/* The three UEFA league phases are native OG TYPE-2 contests, NOT an
 * independent international simulator. The real 2026/27 entrant words and
 * competition record come from World_LeaguePhaseCups. Save the full OG DIY
 * competition and team/RNG state, exactly as with domestic resident cups.
 * The human-club binding and cross-save persistence still require an audit. */
/* OpenSWOS career uses this complete wrapper (LoadSomeEuroCup + selector),
 * not the low-level selector alone. Keep the 96/97 templates untouched. */
extern uint16_t InitCompetitionTeamSelectionFromAltTableHeader(uint8_t *);
extern uint16_t _careerFileBuffer, _numNeededTeams;
extern void InitDIYLeagueFromRecord(uint8_t *);
extern void InitTeams(void);
int Expanded_OGWorld_AdvanceUefa(uint16_t year,uint8_t phase) {
    const int modern=World_Current()==WORLD_2627;
    const int classic=World_Current()==WORLD_9697;
    if(!modern && !classic)return 1;
    if(_gameType!=4 || s_expOGCurrent || s_expOGCupCurrent)return 0;
    /* OG career qualification already produced the exact list after the
     * previous season. Reuse it, not a second access/score engine. Each
     * current-year field must be complete, unique and world/year-specific.
     * Unemployed seasons without that native witness still fail closed. */
    const uint16_t first=World_FirstYear();
    const int later=year>first;
    uint16_t qualified[4][EXP_QUAL_MAX_ENTRANTS]={{0}};
    unsigned historicCopaQualified=0u;
    const unsigned expected[3]={modern?36u:16u,modern?36u:32u,
                                modern?36u:32u};
    int ready=1;
    if(later) {
        for(unsigned k=0;k<3u;++k)
            if(Expanded_OGWorld_QualReadNext(year,k,qualified[k],
                        EXP_QUAL_MAX_ENTRANTS)!=expected[k])
                ready=0;
        if(!ready) {
            if(phase==0u)
                fprintf(stderr,"[og-uefa-resident] NEW_EDITION_PENDING "
                        "world=%s requested-year=%u last-real-edition=%u "
                        "qualification=NO_COMPLETE_NATIVE_SOURCE "
                        "mode=%s old-edition=READ_ONLY_PRESERVED\n",
                        World_Tag(),(unsigned)year,
                        (unsigned)s_expOGUefaCalendarYear,
                        _selTeamNumber==0xffffu?"UNEMPLOYED":"EMPLOYED");
            return 1;
        }
        /* Libertadores 1996/97 is a separate historical 20-team type-2
         * record. An absent Copa witness must NOT change its original field. */
        historicCopaQualified=Expanded_OGWorld_QualReadNext(
            year,3u,qualified[3],EXP_QUAL_MAX_ENTRANTS);
    }
    if(phase==0u) {
        s_expOGUefaCalendarYear=year;
        if(later) {
            const int actualSource=s_expQual.valid &&
                ExpandedQual_SameEdition(s_expQual.world,s_expQual.year,
                                         (unsigned)World_Current(),year);
            fprintf(stderr,"[og-uefa-resident] %s "
                           "world=%s year=%u fields=3 "
                           "qualifiers=%s no-independent-resimulation=1\n",
                    actualSource?"NEW_EDITION_READY":"RESUME_EXISTING_EDITION",
                    World_Tag(),(unsigned)year,
                    actualSource?"OG_CAREER_NATIVE":"CAR_RESIDENT_OG");
        }
    }
    const uint8_t (*places)[4]=NULL;int32_t nAssoc=0;
    CompetitionLeaguePhase *cups=modern?World_LeaguePhaseCups(&places,&nAssoc):NULL;
    if(modern && (!cups || !places || nAssoc<=0))return 0;
    /* Original OpenSWOS 96 first edition, NOT the Mod's 1997 bye edition:
     * twenty seeds, five groups of four, then the native 16-team round.
     * Slot 1 is the historic South-American resident type-2 tournament,
     * stored separately from the historic UEFA Champions at slot 0.
     * Both use the same OG tournament engine and existing CAR snapshot. */
    Competition *historicCopa=classic?Comp_ById(COMPID_copaLibertadores):NULL;
    if(classic && (!historicCopa || historicCopa->type!=2)) {
        fprintf(stderr,"[og-libertadores-96] NO_RECORD at=START advance=SKIPPED\n");
    }
    /* Temporary record overlays: template rules and fixture algorithms
     * stay OG; only the original OG-qualified entrant IDs differ. */
    static CompetitionLeaguePhase modernNext[3];
    static CompetitionTournament historicChampNext,historicCopaNext;
    for(unsigned k=0;k<(modern?3u:(historicCopa?2u:1u));++k) {
        const int isHistoricCopa=classic && k==1u;
        if(later && isHistoricCopa &&
           historicCopaQualified!=20u) {
            if(phase==0u)
                fprintf(stderr,"[og-libertadores-96] NEW_EDITION_PENDING "
                               "year=%u reason=NO_20_CLUB_OG_QUALIFIER_FIELD\n",
                        (unsigned)year);
            continue;
        }
        /* Classic Copa's OG record plays at phases 16..40 (MAR-JUN).
         * The Expanded 96/97 world starts in JUL 1996 at phase 48.
         * Forward the same ORIGINAL native tournament in MAR-JUN 1997,
         * not retroactively during July 1996. Keep lastPhase as the
         * actual world key for idempotent weekly FORWARD TIME. */
        const uint8_t fixturePhase=isHistoricCopa?
            Expanded_OGCopa1996_FixturePhase(phase):phase;
        ExpandedOGLeague *ctx=&s_expOGUefa[k];
        if(ctx->used && ctx->year!=year) {
            /* The previous edition was already journaled; never treat its
             * final as a current-year fixture or use its old RNG. */
            fprintf(stderr,"[og-uefa-resident] EDITION_REPLACED "
                           "cup=%u old=%u new=%u old-matches=%u "
                           "old-results=JOURNAL_ONLY\n",
                    k,(unsigned)ctx->year,(unsigned)year,
                    (unsigned)ctx->comp.matchesPlayedCount);
            memset(ctx,0,sizeof(*ctx));
            ctx->lastMatch=0xffu;
        }
        const uint32_t beforeMatches=ctx->used?
            (uint32_t)ctx->comp.matchesPlayedCount:0u;
        const unsigned beforeStage=ctx->used?
            (unsigned)ctx->tournament.header.roundComplete:0u;
        if(!ctx->used)ctx->lastMatch=0xffu;
        Competition *rec=isHistoricCopa?historicCopa:
                         modern?(Competition *)&cups[k]:(Competition *)&_euroCup;
        if(later) {
            if(modern) {
                memcpy(&modernNext[k],&cups[k],sizeof modernNext[k]);
                for(unsigned i=0;i<36u;++i) {
                    modernNext[k].teamSeedList[i].countryNumber=
                        (uint8_t)(qualified[k][i]>>8);
                    modernNext[k].teamSeedList[i].teamOrdinal=
                        (uint8_t)qualified[k][i];
                }
                rec=(Competition *)&modernNext[k];
            } else if(isHistoricCopa) {
                memcpy(&historicCopaNext,historicCopa,
                       sizeof historicCopaNext);
                for(unsigned i=0;i<20u;++i) {
                    historicCopaNext.teamSeedList[i].countryNumber=
                        (uint8_t)(qualified[3][i]>>8);
                    historicCopaNext.teamSeedList[i].teamOrdinal=
                        (uint8_t)qualified[3][i];
                }
                rec=(Competition *)&historicCopaNext;
            } else {
                memcpy(&historicChampNext,&_euroCup,
                       sizeof historicChampNext);
                for(unsigned i=0;i<16u;++i) {
                    historicChampNext.teamSeedList[i].countryNumber=
                        (uint8_t)(qualified[0][i]>>8);
                    historicChampNext.teamSeedList[i].teamOrdinal=
                        (uint8_t)qualified[0][i];
                }
                rec=(Competition *)&historicChampNext;
            }
        }
        const TeamId *seed=isHistoricCopa?comp_team_seed_list(rec):
                           modern?((CompetitionLeaguePhase *)rec)->teamSeedList:
                                  ((CompetitionTournament *)rec)->teamSeedList;
        const unsigned entrants=isHistoricCopa?20u:(modern?36u:16u);
        if(!seed || rec->type!=2 ||
           (uint8_t)rec->compId!=(isHistoricCopa?COMPID_copaLibertadores:k))
            return 0;
        if(isHistoricCopa) {
            const CompetitionTournament *t=(const CompetitionTournament *)rec;
            if(t->header.rounds[0].teamCount!=20 ||
               t->header.rounds[0].groupCount!=5 ||
               t->header.rounds[0].teamGroupCount!=4 ||
               t->header.rounds[1].teamCount!=16) {
                fprintf(stderr,"[og-libertadores-96] REJECT non-original-format\n");
                return 0;
            }
        }
        if(ctx->used && ctx->year==year && phase<=ctx->lastPhase)continue;
        unsigned unique=0;
        for(unsigned i=0;i<entrants;++i) {
            const uint16_t id=TEAMID_PACK(seed[i]);
            if(id==0xffffu || !id)return 0;
            for(unsigned j=0;j<i;++j)
                if(id==TEAMID_PACK(seed[j]))return 0;
            ++unique;
        }
        /* Manager's original quaternary slot is not yet merged: do not
         * schedule a second match for the same club. */
        int manager=0;
        for(unsigned i=0;i<entrants && _selTeamNumber!=0xffffu;++i)
            if(TEAMID_PACK(seed[i])==_selTeamNumber)
                manager=1;
        if(manager && fixturePhase>0) {
            /* The manager's OG slot is not reconciled into this resident
             * draw yet. DO NOT double-simulate their cup encounters. */
            if(phase==112u || phase%8u==0u)
                fprintf(stderr,
                    "[og-uefa-resident] PENDING_MANAGER_BIND "
                    "year=%u cup=%u world-phase=%u fixture-phase=%u "
                    "OG-matches=%lu (NO DOUBLE MATCHES)\n",
                    (unsigned)year,k,(unsigned)phase,
                    (unsigned)fixturePhase,(unsigned long)beforeMatches);
            if(ctx->used)ctx->lastPhase=phase;
            continue;
        }
        DIYCompetitionEx oldComp;
        DIYTournamentState oldTourn;
        const uint8_t oldSeason=_seasonPlaying,oldView=_viewRandomPhase;
        const uint16_t oldFlag=_internationalAutoAdvanceFlag;
        const uint16_t oldLoaded=_careerLoadedFlag,oldTeamsLoaded=_teamsLoaded;
        const uint16_t oldCareerFile=_careerFileBuffer,oldNeeded=_numNeededTeams;
        const uint8_t cid=(uint8_t)rec->compId;
        const uint8_t oldLast=_careerLastMatchIndexByCompetitionSlot[cid];
        memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
        memcpy(&oldTourn,&_diyFileBufferCopy,sizeof oldTourn);
        SaveSelectedTeams();SaveRand2State();
        _internationalAutoAdvanceFlag=1;_careerLoadedFlag=1;_teamsLoaded=0;
        _seasonPlaying=(uint8_t)((96u+(unsigned)year-World_FirstYear())%100u);
        s_expOGCurrent=ctx;s_expOGPhase=(int)fixturePhase;s_expOGFault=0;
        if(ctx->used) {
            if(ctx->teamCount!=entrants)s_expOGFault=1;
            if(!s_expOGFault) {
                memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
                memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
                _numSelectedTeams=ctx->teamCount;
                memcpy(_selectedTeams,ctx->teams,entrants*sizeof(TeamFileEx));
                _seed2=ctx->seed;_randXorIndex2=ctx->randIndex;_randXorKey2=ctx->randKey;
            }
        } else {
            Randomize2((uint16_t)(((uint16_t)((uint8_t *)rec)[0]<<8)|_seasonPlaying));
            _numSelectedTeams=0;
            InitCompetitionTeamSelectionFromAltTableHeader((uint8_t *)rec);
            InitDIYLeagueFromRecord((uint8_t *)rec);
            InitTeams();
            if(_numSelectedTeams!=entrants) {
                fprintf(stderr,"[og-uefa-resident] INIT_COUNT_MISMATCH year=%u cup=%u wanted=%u selected=%u native-type=%u\n",
                        (unsigned)year,k,entrants,(unsigned)_numSelectedTeams,
                        (unsigned)_diyFileBufferCopy.header.competitionType);
                s_expOGFault=1;
            }
        }
        _careerLastMatchIndexByCompetitionSlot[cid]=ctx->lastMatch;
        if(!s_expOGFault && fixturePhase>0) {
            if(!isHistoricCopa || phase%8u==0u || phase==113u)
                fprintf(stderr,
                    "[og-uefa-resident] BEFORE_ADVANCE world=%u year=%u "
                    "cup=%u world-phase=%u fixture-phase=%u slot=%u "
                    "viewPhase=%u played=%u remaining=%u\n",
                    (unsigned)World_Current(),(unsigned)year,k,
                    (unsigned)phase,(unsigned)fixturePhase,
                    (unsigned)_diyFileBufferCopy.header.roundComplete,
                    (unsigned)_diyCompetitionStart.base.viewRandomPhase,
                    (unsigned)_diyCompetitionStart.matchesPlayedCount,
                    (unsigned)_diyCompetitionStart.minSubstitutes);
            Career_AutoAdvanceCompetitionSlots();
        }
        if(!s_expOGFault && _numSelectedTeams==entrants) {
            ctx->lastMatch=_careerLastMatchIndexByCompetitionSlot[cid];
            memcpy(&ctx->comp,&_diyCompetitionStart,sizeof ctx->comp);
            memcpy(&ctx->tournament,&_diyFileBufferCopy,sizeof ctx->tournament);
            memcpy(ctx->teams,_selectedTeams,entrants*sizeof(TeamFileEx));
            ctx->teamCount=(uint16_t)entrants;
            ctx->country=(uint8_t)(isHistoricCopa?82u:80u);
            ctx->division=(uint8_t)k;
            ctx->year=year;ctx->lastPhase=phase;ctx->used=1;
            ctx->seed=_seed2;ctx->randIndex=_randXorIndex2;ctx->randKey=_randXorKey2;
        } else s_expOGFault=1;
        const int failed=s_expOGFault;
        s_expOGCurrent=NULL;s_expOGPhase=-1;
        _careerLastMatchIndexByCompetitionSlot[cid]=oldLast;
        _seasonPlaying=oldSeason;_viewRandomPhase=oldView;
        _internationalAutoAdvanceFlag=oldFlag;_careerLoadedFlag=oldLoaded;
        _teamsLoaded=oldTeamsLoaded;
        _careerFileBuffer=oldCareerFile;_numNeededTeams=oldNeeded;
        RestoreRand2State();RestoreSelectedTeams();
        memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
        memcpy(&_diyFileBufferCopy,&oldTourn,sizeof oldTourn);
        if(failed) {
            fprintf(stderr,"[og-uefa-resident] FAILED year=%u cup=%u phase=%u\n",
                    (unsigned)year,k,(unsigned)phase);
            return 0;
        }
        if(isHistoricCopa) {
            const uint32_t afterMatches=(uint32_t)ctx->comp.matchesPlayedCount;
            const unsigned afterStage=(unsigned)ctx->tournament.header.roundComplete;
            if(phase==0u || phase==48u || phase==96u ||
               phase==112u || phase==136u || phase==143u ||
               afterMatches!=beforeMatches || afterStage!=beforeStage ||
               (phase>=112u && phase%8u==0u)) {
                fprintf(stderr,
                    "[og-libertadores-96] year=%u world-phase=%u "
                    "fixture-phase=%u period=MAR-JUN-1997 "
                    "resident=1 entrants=%u OG-matches=%lu->%lu "
                    "stage=%u->%u holder-bye=NO_NATIVE_RULE\n",
                    (unsigned)year,(unsigned)phase,
                    (unsigned)fixturePhase,unique,
                    (unsigned long)beforeMatches,(unsigned long)afterMatches,
                    beforeStage,afterStage);
                if(fixturePhase==0u && beforeMatches>0u)
                    fprintf(stderr,
                        "[og-libertadores-96] LEGACY_EARLY_MATCHES "
                        "year=%u already-played=%lu "
                        "status=NOT_REWOUND (save preserved)\n",
                        (unsigned)year,(unsigned long)beforeMatches);
            }
        } else {
            fprintf(stderr,
                    "[og-uefa-resident] world=%u year=%u cup=%u phase=%u "
                    "residents=1 entrants=%u OG-matches=%u\n",
                    (unsigned)World_Current(),(unsigned)year,k,
                    (unsigned)phase,unique,
                    (unsigned)ctx->comp.matchesPlayedCount);
        }
    }
    return 1;
}

/* Resident OG national cup state, owned by the same START / FORWARD TIME
 * world as OG leagues. Country + table slot + year, NEVER compId alone:
 * compIds can be repeated in country/league/cup contexts. */
static ExpandedOGCup *exp_cup_find(uint8_t country,uint8_t slot,uint16_t year)
{
    for(uint32_t i=0;i<s_expOGCupCount;++i)
        if(s_expOGCups[i].country==country &&
           s_expOGCups[i].slot==slot &&
           s_expOGCups[i].year==year)
            return &s_expOGCups[i];
    return NULL;
}

/* The existing end-of-season OG qualification callbacks expect ten real
 * TeamFileEx records: nine ranked league clubs plus the cup winner. Read
 * those records from the CLOSED resident competition, not from manager
 * menus or a reconstructed LWE table. Fail closed on ambiguous identities.
 * No match, draw, RNG or resident data is touched by this reader. */
static int exp_og_qualification_country(uint8_t country,uint16_t closedYear,
                                      TeamFileEx out[10],unsigned *ranked,
                                      int *cupVerified)
{
    memset(out,0,10u*sizeof(*out));
    *ranked=0u;*cupVerified=0;
    const ExpandedOGLeague *ctx=exp_find(country,0u,closedYear);
    if(ctx && ctx->used && ctx->lastPhase>=56u &&
       ctx->teamCount>=2u && ctx->teamCount<=EXP_OG_MAX_TEAMS &&
       ctx->comp.base.competitionType==0 &&
       ctx->comp.matchesPlayedCount>0u &&
       ctx->comp.minSubstitutes==0u) {
        const unsigned n=(unsigned)ctx->comp.base.numTeams;
        if(n<2u || n>ctx->teamCount)return 0;
        for(unsigned i=0;i<n && i<9u;++i) {
            const unsigned offset=ctx->comp.matchData.league.teamEntryOffsets[i];
            if(offset%sizeof(DIYTeamStats)!=0u ||
               offset/sizeof(DIYTeamStats)>=n)return 0;
            DIYTeamStats row;
            memcpy(&row,(const uint8_t *)&ctx->comp+
                offsetof(DIYCompetitionEx,leagueStandings)+offset,sizeof row);
            if(row.teamSelOffset<0 ||
               (unsigned)row.teamSelOffset>=ctx->teamCount)return 0;
            const TeamFileEx *t=&ctx->teams[row.teamSelOffset];
            if(World_FileCountry(t->team.countryNumber)!=country)return 0;
            const uint16_t id=PACK_BE_U16(t->team.countryNumber,
                                          t->team.teamOrdinal);
            if(id==0xffffu)return 0;
            for(unsigned j=0;j<i;++j)
                if(PACK_BE_U16(out[j].team.countryNumber,
                               out[j].team.teamOrdinal)==id)return 0;
            memcpy(&out[i],t,sizeof(*t));
            ++*ranked;
        }
    }
    /* The original OG cup result is the winnerTeam identity, NOT the
     * seed list's first club or the last remaining bracket index. */
    for(uint8_t slot=2u;slot<=3u && !*cupVerified;++slot) {
        const ExpandedOGCup *cup=exp_cup_find(country,slot,closedYear);
        if(!cup || !cup->used || cup->managed_excluded ||
           cup->unsupported_setup || cup->lastPhase<56u ||
           !cup->comp.base.competitionFinished)continue;
        const uint16_t id=cup->tournament.header.winnerTeam;
        if(!id || id==0xffffu)continue;
        for(unsigned i=0;i<cup->teamCount;++i) {
            const TeamFileEx *t=&cup->teams[i];
            if(PACK_BE_U16(t->team.countryNumber,
                           t->team.teamOrdinal)==id) {
                memcpy(&out[9],t,sizeof(*t));
                *cupVerified=1;break;
            }
        }
    }
    return *ranked || *cupVerified;
}

static const TeamFileEx *exp_og_qualification_holder(unsigned cup,
                                                    uint16_t closedYear)
{
    const ExpandedOGLeague *l=NULL;
    const ExpandedOGCup *c=NULL;
    if(World_Current()==WORLD_9697 && cup!=0u)
        c=exp_cup_find(254u,(uint8_t)cup,closedYear);
    else l=&s_expOGUefa[cup];
    if(l && (!l->used || l->year!=closedYear ||
             !l->comp.base.competitionFinished))l=NULL;
    if(c && (!c->used || c->year!=closedYear ||
             !c->comp.base.competitionFinished))c=NULL;
    /* OG tournament winner is in the persisted tournament header.
     * The DIY competition's 0x3A doubles as resultCode. */
    const uint16_t id=l?l->tournament.header.winnerTeam:
                      c?c->tournament.header.winnerTeam:0xffffu;
    if(!id || id==0xffffu)return NULL;
    const unsigned n=l?l->teamCount:c?c->teamCount:0u;
    for(unsigned i=0;i<n;++i) {
        const TeamFileEx *t=l?&l->teams[i]:&c->teams[i];
        if(PACK_BE_U16(t->team.countryNumber,t->team.teamOrdinal)==id)
            return t;
    }
    return NULL;
}

/* The unemployed manager has no OG season-end callback, but the ORIGINAL
 * native league/cup contexts are closed by the same world. Feed their exact
 * club records to OG's existing career qualification functions without
 * simulating a fixture or touching the manager's TMD pool.
 * The resulting ID lists are stored as part of the resident .CAR. */
int Expanded_OGWorld_QualifyUnemployed(uint16_t closedYear,uint16_t nextYear)
{
    if(_gameType!=4 || (World_Current()!=WORLD_2627 &&
                        World_Current()!=WORLD_9697) ||
       (_selTeamNumber!=0xffffu && !_youreSacked) ||
       s_expOGQualUnemployedActive)return 0;
    if(s_expQual.valid &&
       ExpandedQual_SameEdition(s_expQual.world,s_expQual.year,
                       (unsigned)World_Current(),nextYear))
        return 1; /* existing witnessed OG field, never draw twice */
    extern void Career_PrepareEuropeanCupQualificationState(void);
    extern void Career_ProcessCompetitionForEuropeanCupQualification(void);
    extern void Career_DrawEuropeanCupTeamsAndCopyGrids(void);
    extern void Career_PrepareCopaLibertadoresQualificationState(void);
    extern void Career_ProcessCompetitionForCopaLibertadoresQualification(void);
    extern uint16_t Career_CopaLibertadoresQualifiedTeamCount(void);
    extern Competition *Career_CopaWorkingCopy(void);
    extern uint8_t _primaryQualifiedTeamGrids[];
    extern uint16_t _cachedPrimaryQualifiedTeamIds[];
    extern uint16_t _cachedSecondaryQualifiedTeamIds[];
    extern uint16_t _cachedTertiaryQualifiedTeamId;
    extern uint8_t _currentCompetitionIndex;
    /* _isNationalSelector is declared volatile at file scope; do not
     * redeclare it here with incompatible qualifiers. */
    extern uint16_t _teamNoOfferAccepted;
    extern uint16_t _euroCupQualifiedTeamCount,_cupWinnersCupQualifiedTeamCount;
    extern uint16_t _uefaCupQualifiedTeamCount;
    extern uint8_t *_euroCupTeamListPtr,*_euroCupTeamListWritePtr;
    extern uint8_t *_cupWinnersCupTeamListPtr,*_cupWinnersCupTeamListWritePtr;
    extern uint8_t *_uefaCupTeamListPtr,*_uefaCupTeamListWritePtr;
    extern uint16_t _euroCupTitleHolderTeamId,_cupWinnersCupTitleHolderTeamId;
    extern uint16_t _uefaCupTitleHolderTeamId;
    extern uint8_t _euroCupTitleHolderGrid[],_cupWinnersCupTitleHolderGrid[];
    extern uint8_t _uefaCupTitleHolderGrid[];
    extern uint8_t _careerCompetitionIdRemapTable[];
    extern void *_countriesTable[];
    /* A separate scratch list replaces each *Copy's original write cursor.
     * The original qualification callbacks still choose all entrants. */
    static uint8_t field[3][EXP_QUAL_MAX_ENTRANTS*2u];
    TeamFileEx savedGrids[10],savedHolders[3];
    uint16_t savedPrimary[9],savedSecondary[3];
    const uint16_t savedTertiary=_cachedTertiaryQualifiedTeamId;
    const uint16_t savedSel=_selTeamNumber,savedNat=_isNationalSelector;
    const uint16_t savedOffer=_teamNoOfferAccepted;
    const uint8_t savedComp=_currentCompetitionIndex;
    const uint16_t savedCounts[3]={
        _euroCupQualifiedTeamCount,_cupWinnersCupQualifiedTeamCount,
        _uefaCupQualifiedTeamCount};
    uint8_t *const savedStart[3]={
        _euroCupTeamListPtr,_cupWinnersCupTeamListPtr,_uefaCupTeamListPtr};
    uint8_t *const savedEnd[3]={
        _euroCupTeamListWritePtr,_cupWinnersCupTeamListWritePtr,
        _uefaCupTeamListWritePtr};
    uint16_t *const titleIds[3]={
        &_euroCupTitleHolderTeamId,&_cupWinnersCupTitleHolderTeamId,
        &_uefaCupTitleHolderTeamId};
    uint8_t *const titleGrids[3]={
        _euroCupTitleHolderGrid,_cupWinnersCupTitleHolderGrid,
        _uefaCupTitleHolderGrid};
    uint16_t savedTitle[3];
    /* Capture the original first-year Copa list before the native
     * selector updates its career working copy. New editions consume
     * the verified s_expQual IDs; the old template must not change. */
    Competition *const copaWorking=Career_CopaWorkingCopy();
    TeamId originalCopa[20];
    TeamId *const copaWorkingSeed=copaWorking?
        comp_team_seed_list(copaWorking):NULL;
    if(copaWorkingSeed)
        memcpy(originalCopa,copaWorkingSeed,sizeof originalCopa);
    memcpy(savedGrids,_primaryQualifiedTeamGrids,sizeof savedGrids);
    memcpy(savedPrimary,_cachedPrimaryQualifiedTeamIds,sizeof savedPrimary);
    memcpy(savedSecondary,_cachedSecondaryQualifiedTeamIds,sizeof savedSecondary);
    for(unsigned k=0;k<3u;++k) {
        savedTitle[k]=*titleIds[k];
        memcpy(&savedHolders[k],titleGrids[k],sizeof(TeamFileEx));
    }
    memset(field,0,sizeof field);
    _selTeamNumber=0xffffu;_teamNoOfferAccepted=0xffffu;
    s_expOGQualUnemployedActive=1;s_expOGQualUnemployedError=0;
    /* The original Prepare treats a club's country as a required lookup.
     * In unemployment it doesn't exist: no selected country is excluded. */
    _isNationalSelector=1u;
    Career_PrepareEuropeanCupQualificationState();
    _isNationalSelector=0u;
    Career_PrepareCopaLibertadoresQualificationState();
    _euroCupTeamListPtr=_euroCupTeamListWritePtr=field[0];
    _cupWinnersCupTeamListPtr=_cupWinnersCupTeamListWritePtr=field[1];
    _uefaCupTeamListPtr=_uefaCupTeamListWritePtr=field[2];
    _euroCupQualifiedTeamCount=_cupWinnersCupQualifiedTeamCount=0u;
    _uefaCupQualifiedTeamCount=0u;
    unsigned sources=0u,missing=0u,cupWinners=0u;
    for(unsigned k=0;k<3u;++k) {
        const TeamFileEx *holder=exp_og_qualification_holder(k,closedYear);
        *titleIds[k]=holder?PACK_BE_U16(
            holder->team.countryNumber,holder->team.teamOrdinal):0xffffu;
        if(holder)memcpy(titleGrids[k],holder,sizeof(*holder));
    }
    const uint8_t *countries=World_SeasonCountries();
    if(!countries)countries=_careerCompetitionIdRemapTable;
    for(unsigned i=0;i<256u && countries && countries[i]!=0xffu;++i) {
        const uint8_t country=countries[i];
        const uint8_t *tag=(const uint8_t *)_countriesTable[country];
        if(!tag || (uintptr_t)tag==(uintptr_t)-1 ||
           (uintptr_t)tag==(uintptr_t)-2)continue;
        /* Eligibility and continental association are applied by the
         * actual native OG callbacks, not a new set of football rules. */
        TeamFileEx ranked[10];
        unsigned n=0u;int cup=0;
        if(!exp_og_qualification_country(country,closedYear,
                                         ranked,&n,&cup)) {
            ++missing;
            fprintf(stderr,"[og-unemployed-qual-country] "
                "world=%s year=%u country=%u "
                "status=NO_VERIFIED_OG_FINAL_TABLE_OR_CUP\n",
                World_Tag(),(unsigned)closedYear,(unsigned)country);
            continue;
        }
        ++sources;if(cup)++cupWinners;
        memcpy(_primaryQualifiedTeamGrids,ranked,sizeof ranked);
        for(unsigned j=0;j<9u;++j)
            _cachedPrimaryQualifiedTeamIds[j]=
                PACK_BE_U16(ranked[j].team.countryNumber,
                            ranked[j].team.teamOrdinal);
        _cachedSecondaryQualifiedTeamIds[0]=cup?
            PACK_BE_U16(ranked[9].team.countryNumber,
                        ranked[9].team.teamOrdinal):0xffffu;
        _cachedTertiaryQualifiedTeamId=0xffffu;
        _currentCompetitionIndex=country;
        /* Modern UEFAQ reads the same OG table but must NOT enter
         * Conti_Country -> ct_finish: the CT engine is a different,
         * still-unbound resident competition and may run old rounds. */
        if(World_Current()==WORLD_2627)UefaQual_Country();
        else Career_ProcessCompetitionForEuropeanCupQualification();
        if(World_Current()==WORLD_9697)
            Career_ProcessCompetitionForCopaLibertadoresQualification();
    }
    /* Modern UEFAQ may fill a missing association with a best-unused
     * club. That is a valid OG bracket fallback, but NOT proof of actual
     * eligibility from all 55 resident league/cup tables. Require each
     * association's real OG input, including cup-only associations. */
    if(World_Current()==WORLD_2627) {
        const WorldUefaSeeds *const seeds=World_UefaSeeds();
        unsigned present=0u;
        for(unsigned rk=1u;rk<NA;++rk)
            if(UQ_IN.present[rk])++present;
        if(!seeds || present!=(unsigned)seeds->nCountries) {
            Expanded_OGWorld_UnemployedQualificationError(
                "UEFA_ASSOCIATION_FINAL_INPUT_MISSING");
            fprintf(stderr,"[og-unemployed-qual] UEFA_ACCESS "
                           "present=%u required=%u status=INCOMPLETE\n",
                    present,seeds?(unsigned)seeds->nCountries:0u);
        }
    }
    /* OG draws only QUALIFICATION brackets via its existing algorithm.
     * No played league/continental fixture is executed here. */
    if(World_Current()==WORLD_2627)UefaQual_Fill();
    Career_DrawEuropeanCupTeamsAndCopyGrids();
    const unsigned n[3]={_euroCupQualifiedTeamCount,
        _cupWinnersCupQualifiedTeamCount,_uefaCupQualifiedTeamCount};
    const unsigned expected[3]={
        World_Current()==WORLD_2627?36u:16u,
        World_Current()==WORLD_2627?36u:32u,
        World_Current()==WORLD_2627?36u:32u};
    const unsigned copaCount=Career_CopaLibertadoresQualifiedTeamCount();
    const Competition *const copa=Career_CopaWorkingCopy();
    const uint8_t *const copaIds=copa?
        (const uint8_t *)comp_team_seed_list((Competition *)copa):NULL;
    int valid=n[0]==expected[0] && n[1]==expected[1] &&
        n[2]==expected[2] && (!copaCount || copaCount==20u) &&
        !s_expOGQualUnemployedError;
    /* The same club cannot play two of the three UEFA club editions.
     * This also rejects zero/invalid club IDs before they can enter CAR. */
    for(unsigned k=0;k<3u;++k)
        for(unsigned i=0;i<n[k] && i<EXP_QUAL_MAX_ENTRANTS;++i) {
            const uint16_t id=(uint16_t)
                (((uint16_t)field[k][2u*i]<<8)|field[k][2u*i+1u]);
            if(id==0xffffu)valid=0;
            for(unsigned j=0;j<i;++j)
                if(field[k][2u*j]==field[k][2u*i] &&
                   field[k][2u*j+1u]==field[k][2u*i+1u])valid=0;
            for(unsigned q=0;q<k;++q)
                for(unsigned j=0;j<n[q] && j<EXP_QUAL_MAX_ENTRANTS;++j)
                    if(field[q][2u*j]==field[k][2u*i] &&
                       field[q][2u*j+1u]==field[k][2u*i+1u])valid=0;
        }
    fprintf(stderr,"[og-unemployed-qual] world=%s closed=%u next=%u "
        "source=OG_RESIDENT_FINAL_STANDINGS_AND_WINNERS "
        "countries-read=%u absent-country-finals=%u cup-winners=%u "
        "clubs=%u,%u,%u copa=%u status=%s "
        "no-replay=1 no-manager-TMD=1\n",
        World_Tag(),(unsigned)closedYear,(unsigned)nextYear,
        sources,missing,cupWinners,n[0],n[1],n[2],
        copaCount,valid?"NATIVE_FIELD_READY":"INCOMPLETE_FIELD");
    /* Libertadores is validated separately: one invalid or duplicate
     * club must not make the next CAR fail its checksum/ID guard. */
    if(copaCount && copaIds)for(unsigned j=0;j<copaCount && j<20u;++j) {
        const uint16_t id=(uint16_t)
            (((uint16_t)copaIds[2u*j]<<8)|copaIds[2u*j+1u]);
        if(id==0xffffu)valid=0;
        for(unsigned z=0;z<j;++z)
            if(copaIds[2u*z]==copaIds[2u*j] &&
               copaIds[2u*z+1u]==copaIds[2u*j+1u])valid=0;
    }
    if(copaCount && !copaIds)valid=0;
    if(valid)
        Expanded_OGWorld_QualCaptureNative(nextYear,
             field[0],n[0],field[1],n[1],field[2],n[2],
             copaIds,copaCount);
    /* Restore local manager/UI state. Original OG qualification state
     * (UEFA coefficients and bracket) and the resident league/cup records
     * stay owned by the ONE world; its next entrants live in qual_next CAR. */
    for(unsigned k=0;k<3u;++k) {
        *titleIds[k]=savedTitle[k];
        memcpy(titleGrids[k],&savedHolders[k],sizeof(TeamFileEx));
    }
    memcpy(_primaryQualifiedTeamGrids,savedGrids,sizeof savedGrids);
    memcpy(_cachedPrimaryQualifiedTeamIds,savedPrimary,sizeof savedPrimary);
    memcpy(_cachedSecondaryQualifiedTeamIds,savedSecondary,sizeof savedSecondary);
    _cachedTertiaryQualifiedTeamId=savedTertiary;
    _euroCupTeamListPtr=savedStart[0];
    _cupWinnersCupTeamListPtr=savedStart[1];
    _uefaCupTeamListPtr=savedStart[2];
    _euroCupTeamListWritePtr=savedEnd[0];
    _cupWinnersCupTeamListWritePtr=savedEnd[1];
    _uefaCupTeamListWritePtr=savedEnd[2];
    _euroCupQualifiedTeamCount=savedCounts[0];
    _cupWinnersCupQualifiedTeamCount=savedCounts[1];
    _uefaCupQualifiedTeamCount=savedCounts[2];
    if(copaWorkingSeed)
        memcpy(copaWorkingSeed,originalCopa,sizeof originalCopa);
    _selTeamNumber=savedSel;_isNationalSelector=savedNat;
    _teamNoOfferAccepted=savedOffer;_currentCompetitionIndex=savedComp;
    s_expOGQualUnemployedActive=0;
    return valid;
}
/* Read actual selected IDs, not the first-year template, when comparing
 * national cup access rules. No mutable pointers escape the resident. */
unsigned Expanded_OGWorld_ReadCupEntries(uint8_t country,uint8_t slot,
    uint16_t year,TeamId *out,unsigned capacity)
{
    const ExpandedOGCup *const c=exp_cup_find(country,slot,year);
    if(!out || !c || !c->used || c->unsupported_setup ||
       c->managed_excluded || !c->teamCount ||
       c->teamCount>EXP_OG_MAX_CUP_TEAMS || c->teamCount>capacity)
        return 0u;
    for(unsigned i=0;i<c->teamCount;++i) {
        out[i].countryNumber=c->teams[i].team.countryNumber;
        out[i].teamOrdinal=c->teams[i].team.teamOrdinal;
    }
    return c->teamCount;
}
/* Permanent, READ-ONLY gap detector: a definition is not a resident OG
 * competition. No draws, RNG, teams or fixture arrays are changed here.
 * Country+division / country+slot identify native records; compId alone
 * is not unique. Called at START, season boundary and SAVE/RESTORE. */
void Expanded_OGWorld_LogNoData(const char *stage,uint16_t year)
{
    extern uint8_t _careerCompetitionIdRemapTable[];
    extern void *_competitionsTable[];
    const uint8_t *countries=World_SeasonCountries();
    if(!countries)countries=_careerCompetitionIdRemapTable;
    unsigned missing=0u,review=0u,covered=0u;
    for(unsigned i=0;countries && i<256u && countries[i]!=0xffu;++i) {
        const uint8_t country=countries[i];
        const void *const *tab=(const void *const *)_competitionsTable[country];
        if(!tab)continue;
        const uintptr_t raw=(uintptr_t)tab[0];
        if(raw && raw!=(uintptr_t)-1 && raw!=(uintptr_t)-2) {
            const Competition *rec=(const Competition *)raw;
            if(rec->type==0 && !comp_is_type0_cup(rec->compId)) {
                const CompetitionLeague *league=(const CompetitionLeague *)rec;
                const unsigned divisions=(unsigned)league->header.divisionsCount;
                for(unsigned d=0;d<divisions && d<16u;++d) {
                    const ExpandedOGLeague *ctx=exp_find(country,(uint8_t)d,year);
                    if(!ctx || !ctx->used) {
                        ++missing;
                        fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                            "category=LEAGUE country=%u division=%u compId=%u "
                            "status=NO_RESIDENT reason=%s\n",
                            stage,World_Tag(),(unsigned)year,(unsigned)country,d,
                            (unsigned)rec->compId,
                            ctx?"NATIVE_UNSUPPORTED":"MISSING_CONTEXT");
                    } else ++covered;
                }
                if(divisions>16u) {
                    ++missing;
                    fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                        "category=LEAGUE country=%u status=UNSUPPORTED_DIVISION_COUNT count=%u\n",
                        stage,World_Tag(),(unsigned)year,(unsigned)country,divisions);
                }
            }
        }
        for(unsigned slot=2u;slot<=3u;++slot) {
            const uintptr_t ptr=(uintptr_t)tab[slot];
            if(!ptr || ptr==(uintptr_t)-1)break;
            if(ptr==(uintptr_t)-2)continue;
            const Competition *cup=(const Competition *)ptr;
            if(cup->type!=1)continue;
            const ExpandedOGCup *ctx=exp_cup_find(country,(uint8_t)slot,year);
            if(!ctx || !ctx->used) {
                ++missing;
                fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                    "category=DOMESTIC_CUP country=%u slot=%u compId=%u "
                    "status=NO_RESIDENT reason=%s\n",
                    stage,World_Tag(),(unsigned)year,(unsigned)country,slot,
                    (unsigned)cup->compId,ctx?"NATIVE_UNSUPPORTED":"MISSING_CONTEXT");
            } else ++covered;
        }
    }
    if(year==World_FirstYear()) {
        const unsigned ueExpected=World_Current()==WORLD_2627?3u:1u;
        for(unsigned k=0;k<ueExpected;++k) {
            if(!s_expOGUefa[k].used || s_expOGUefa[k].year!=year) {
                ++missing;
                fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                    "category=UEFA_CLUB cup=%u status=NO_RESIDENT "
                    "reason=FIRST_EDITION_MISSING\n",stage,World_Tag(),(unsigned)year,k);
            } else ++covered;
        }
        if(World_Current()==WORLD_9697) {
            for(unsigned slot=1u;slot<=2u;++slot) {
                const ExpandedOGCup *ctx=exp_cup_find(254u,(uint8_t)slot,year);
                if(!ctx || !ctx->used) {
                    ++missing;
                    fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                        "category=UEFA_CLASSIC_CUP country=254 slot=%u "
                        "status=NO_RESIDENT\n",stage,World_Tag(),(unsigned)year,slot);
                } else ++covered;
            }
        }
    } else {
        ++review;
        fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
            "category=UEFA_CLUB status=QUALIFICATION_NOT_CONNECTED "
            "severity=REVIEW\n",stage,World_Tag(),(unsigned)year);
    }
    if(World_Current()==WORLD_9697) {
        const Competition *copa=Comp_ById(COMPID_copaLibertadores);
        const ExpandedOGLeague *resident=&s_expOGUefa[1];
        const int ready=copa && resident->used && resident->country==82u &&
                        resident->year==year && resident->teamCount==20u &&
                        resident->tournament.header.competitionType==2;
        if(ready)++covered;
        else ++missing;
        fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
            "category=CONMEBOL_CLUB name=COPA_LIBERTADORES compId=%u "
            "defined=%u status=%s reason=%s native-matches=%u\n",
            stage,World_Tag(),(unsigned)year,(unsigned)COMPID_copaLibertadores,
            (unsigned)(copa!=NULL),ready?"RESIDENT_OG":"NO_RESIDENT",
            ready?"1996_OPEN_SWOS_NATIVE":"NOT_INITIALIZED_OR_DIFFERENT_YEAR",
            ready?(unsigned)resident->comp.matchesPlayedCount:0u);
    } else if(World_Current()==WORLD_2627) {
        int32_t n=0;
        const WorldContiCup *cups=World_ContiCups(&n);
        for(int32_t k=0;cups && k<n;++k) {
            if(!ct_world_first_ready()) {
                ++missing;
                fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                    "category=CONTINENTAL_CLUB index=%ld name=%s "
                    "status=NO_RESIDENT reason=FIRST_EDITION_NOT_CONNECTED\n",
                    stage,World_Tag(),(unsigned)year,(long)k,
                    cups[k].name?cups[k].name:"UNNAMED");
            } else ++review; /* field ready != proof of per-fixture OG play */
        }
        const WorldStates *states=World_States(CN_BRAZIL);
        for(unsigned k=0;states && states->contest && k<states->states;++k) {
            ++missing;
            fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                "category=BRAZIL_STATE index=%u name=%s status=NO_WORLD_RESIDENT "
                "reason=SEASON_END_SIM_NOT_A_DATED_OG_CONTEXT\n",
                stage,World_Tag(),(unsigned)year,k,
                states->contest[k].name[0]?states->contest[k].name:"UNNAMED");
        }
        if(World_CupD()) {
            ++missing;
            fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
                "category=ITALY_CUPD name=COPPA_ITALIA_SERIE_D "
                "status=NO_WORLD_RESIDENT reason=CLUB_SLOT_IS_NOT_ALL_WORLD_FIXTURES\n",
                stage,World_Tag(),(unsigned)year);
        }
    }
    /* Original national-team definitions exist, but their real annual
     * schedule/qualification must not be inferred from the club calendar.
     * Mark REVIEW, not missing in an edition where no such cup is due. */
    ++review;
    fprintf(stderr,"[og-no-data] stage=%s world=%s year=%u "
        "category=NATIONAL_TEAMS status=CALENDAR_AND_QUALIFICATION_UNVERIFIED "
        "severity=REVIEW\n",stage,World_Tag(),(unsigned)year);
    fprintf(stderr,"[og-no-data-summary] stage=%s world=%s year=%u "
        "resident-checked=%u no-resident=%u review=%u "
        "scope=DEFINITIONS_VS_OG_RESIDENCY_ONLY\n",
        stage,World_Tag(),(unsigned)year,covered,missing,review);
}


/* Resident OG contexts in a CAR: pointer-neutral, checksummed, versioned.
 * This saves the ORIGINAL competition/teams/RNG state without replaying a
 * fixture. The ABI sizes are recorded: refuse incompatible binaries. */
#ifdef SWOS_SDL_BUILD
typedef int (*OGEmit)(const char *,int,void *);
static uint64_t og_saved_hash(uint64_t h,const uint8_t *p,size_t n) {
    for(size_t i=0;i<n;i++){h^=p[i];h*=UINT64_C(1099511628211);}return h;
}
static int og_saved_nibble(char c) {
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return c-'a'+10;
    return -1;
}
static int og_saved_hex(OGEmit emit,void *ud,const uint8_t *p,size_t n,
                         size_t ptr1,size_t ptr2,uint64_t *hash) {
    static const char hex[]="0123456789abcdef";
    char buf[1024];size_t k=0;
    for(size_t i=0;i<n;i++){
        uint8_t b=p[i];
        if((i>=ptr1 && i-ptr1<sizeof(void *)) ||
           (i>=ptr2 && i-ptr2<sizeof(void *)))b=0;
        *hash^=b;*hash*=UINT64_C(1099511628211);
        buf[k++]=hex[b>>4];buf[k++]=hex[b&15];
        if(k==sizeof(buf)){if(emit(buf,(int)k,ud)!=(int)k)return 0;k=0;}
    }
    return !k || emit(buf,(int)k,ud)==(int)k;
}
static int og_saved_emit_context(OGEmit emit,void *ud,const void *ptr,
                                  int cup,unsigned idx) {
    const ExpandedOGLeague *l=(const ExpandedOGLeague *)ptr;
    const ExpandedOGCup *c=(const ExpandedOGCup *)ptr;
    const unsigned n=cup?c->teamCount:l->teamCount;
    const unsigned country=cup?c->country:l->country;
    const unsigned slot=cup?c->slot:l->division;
    const unsigned year=cup?c->year:l->year;
    const unsigned cid=cup?c->compId:(unsigned)l->comp.base.field_2E;
    const size_t core=cup?offsetof(ExpandedOGCup,teams):offsetof(ExpandedOGLeague,teams);
    const uint8_t *team=(const uint8_t *)(cup?(const void *)c->teams:(const void *)l->teams);
    const size_t nteam=(size_t)n*sizeof(TeamFileEx);
    const size_t p1=(cup?offsetof(ExpandedOGCup,comp):offsetof(ExpandedOGLeague,comp))+
                     offsetof(DIYCompetitionEx,base.namePtr);
    const size_t p2=(cup?offsetof(ExpandedOGCup,tournament):offsetof(ExpandedOGLeague,tournament))+
                     offsetof(DIYTournamentState,header.namePtr);
    if(n>(cup?EXP_OG_MAX_CUP_TEAMS:EXP_OG_MAX_TEAMS))return 0;
    uint64_t hash=UINT64_C(14695981039346656037);
    /* Seven numeric properties plus core_hex key: exactly 15 varargs.
     * The previous 17-specifier / 15-argument mismatch caused a CRT
     * invalid read (0xEC0) during the first START CAREER autosave. */
    mjson_printf(emit,ud,"{%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:",
                 "index",idx,"country",country,"slot",slot,"year",year,
                 "comp_id",cid,"team_count",n,"core_size",(unsigned)core,
                 "core_hex");
    if(emit("\"",1,ud)!=1 || !og_saved_hex(emit,ud,(const uint8_t *)ptr,core,p1,p2,&hash) ||
       emit("\"",1,ud)!=1)return 0;
    mjson_printf(emit,ud,",%Q:","teams_hex");
    if(emit("\"",1,ud)!=1 ||
       !og_saved_hex(emit,ud,team,nteam,(size_t)-1,(size_t)-1,&hash) ||
       emit("\"",1,ud)!=1)return 0;
    char checksum[17];static const char digits[]="0123456789abcdef";
    for(int i=15;i>=0;i--){checksum[i]=digits[hash&15];hash>>=4;}
    checksum[16]=0;
    mjson_printf(emit,ud,",%Q:%Q}","checksum",checksum);
    return 1;
}
/* Portably checkpoint the source qualification IDs in the SAME atomic CAR
 * as its native OG contexts. This is a WITNESS, not a separate calendar or
 * simulation. Missing data in older CAR files must remain unverified.
 * Hashes are over an endian-independent 524-byte wire, not native structs. */
#define EXP_OG_QUAL_WIRE_SIZE (12u+4u*EXP_QUAL_MAX_ENTRANTS*2u)
static int exp_qual_emit_json(OGEmit emit,void *ud)
{
    if(!s_expQual.valid)return 1;
    uint8_t wire[EXP_OG_QUAL_WIRE_SIZE]={0};
    wire[0]=1u;wire[1]=(uint8_t)s_expQual.world;
    wire[2]=(uint8_t)(s_expQual.year>>8);
    wire[3]=(uint8_t)s_expQual.year;
    for(unsigned k=0;k<4u;++k) {
        if(s_expQual.count[k]>EXP_QUAL_MAX_ENTRANTS ||
           s_expQual.invalid[k]>1u)return 0;
        wire[4u+k]=(uint8_t)s_expQual.count[k];
        wire[8u+k]=(uint8_t)s_expQual.invalid[k];
        for(unsigned j=0;j<s_expQual.count[k];++j) {
            const uint16_t id=s_expQual.ids[k][j];
            if(id==0xffffu)return 0;
            for(unsigned z=0;z<j;++z)
                if(s_expQual.ids[k][z]==id)return 0;
            const unsigned pos=12u+2u*(k*EXP_QUAL_MAX_ENTRANTS+j);
            wire[pos]=(uint8_t)(id>>8);
            wire[pos+1u]=(uint8_t)id;
        }
    }
    uint64_t hash=UINT64_C(14695981039346656037);
    mjson_printf(emit,ud,",%Q:{%Q:%u,%Q:",
                 "qual_next","format",1u,"hex");
    if(emit("\"",1,ud)!=1 ||
       !og_saved_hex(emit,ud,wire,sizeof wire,(size_t)-1,
                     (size_t)-1,&hash) ||
       emit("\"",1,ud)!=1)return 0;
    static const char digits[]="0123456789abcdef";
    char checksum[17];
    for(int i=15;i>=0;--i){checksum[i]=digits[hash&15u];hash>>=4;}
    checksum[16]=0;
    mjson_printf(emit,ud,",%Q:%Q}","checksum",checksum);
    fprintf(stderr,"[og-qual-car] SAVE world=%s year=%u "
        "counts=%u,%u,%u,%u status=CHECKSUMMED\n",
        World_Tag(),(unsigned)s_expQual.year,
        s_expQual.count[0],s_expQual.count[1],
        s_expQual.count[2],s_expQual.count[3]);
    return 1;
}
static int exp_fixture_emit_json(OGEmit emit,void *ud);
static int exp_fixture_read_json(const char *root,int len,
                                  uint8_t **data,size_t *count,
                                  uint32_t *partial);
int Expanded_OGWorld_WriteSaveJson(OGEmit emit,void *ud) {
    if(!emit || !Expanded_OGWorld_SaveStarted() || !s_expOGCount ||
       s_expOGCount>EXP_OG_MAX_LEAGUES || s_expOGCupCount>EXP_OG_MAX_CUPS)return 0;
    mjson_printf(emit,ud,",\n  %Q:{%Q:%u,%Q:%Q,%Q:%u,%Q:%u,%Q:%u,%Q:%u,"
                         "%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%u",
        "og_resident","format",1u,"world",World_Tag(),
        "year",(unsigned)Expanded_OGWorld_SaveYear(),
        "league_slots",(unsigned)s_expOGCount,
        "cup_slots",(unsigned)s_expOGCupCount,
        "uefa_year",(unsigned)s_expOGUefaCalendarYear,
        "league_size",(unsigned)sizeof(ExpandedOGLeague),
        "cup_size",(unsigned)sizeof(ExpandedOGCup),
        "team_size",(unsigned)sizeof(TeamFileEx),
        "club_bound",(unsigned)(s_expOGBound!=0),
        "cup_unsupported",(unsigned)s_expOGCupUnsupported,
        "cup_faults",(unsigned)s_expOGCupFaults);
    mjson_printf(emit,ud,",%Q:[","leagues");
    for(uint32_t i=0;i<s_expOGCount;i++){
        if(i)mjson_printf(emit,ud,",");
        if(!og_saved_emit_context(emit,ud,&s_expOG[i],0,i))return 0;
    }
    mjson_printf(emit,ud,"],%Q:[","uefa");
    for(unsigned i=0;i<3u;i++){
        if(i)mjson_printf(emit,ud,",");
        if(!og_saved_emit_context(emit,ud,&s_expOGUefa[i],0,i))return 0;
    }
    mjson_printf(emit,ud,"],%Q:[","cups");
    for(uint32_t i=0;i<s_expOGCupCount;i++){
        if(i)mjson_printf(emit,ud,",");
        if(!og_saved_emit_context(emit,ud,&s_expOGCups[i],1,i))return 0;
    }
    mjson_printf(emit,ud,"]");
    if(!exp_qual_emit_json(emit,ud))return 0;
    if(!exp_fixture_emit_json(emit,ud))return 0;
    mjson_printf(emit,ud,"}");
    return 1;
}
static int og_saved_next(const char *p,int n,int *pos,const char **out,int *len) {
    int i=*pos;
    while(i<n && (p[i]=='[' || p[i]==',' || p[i]==' ' ||
                  p[i]=='\r' || p[i]=='\n' || p[i]=='\t'))i++;
    if(i>=n || p[i]==']')return 0;
    if(p[i]!='{')return -1;
    int start=i,depth=0,quoted=0,esc=0;
    for(;i<n;i++){
        char ch=p[i];
        if(quoted) {
            if(esc)esc=0;
            else if(ch=='\\')esc=1;
            else if(ch=='"')quoted=0;
        } else if(ch=='"')quoted=1;
        else if(ch=='{')depth++;
        else if(ch=='}' && --depth==0){
            *out=p+start;*len=i+1-start;*pos=i+1;return 1;
        }
    }
    return -1;
}
static int og_saved_num(const char *s,int n,const char *key,long *v) {
    return mjson_get_long(s,n,key,v)>0;
}
static int og_saved_read_hex(const char *s,int len,const char *key,uint8_t *dst,
                             size_t bytes,uint64_t *hash) {
    const char *v=NULL;int n=0;
    if(mjson_find(s,len,key,&v,&n)!=MJSON_TOK_STRING ||
       n<2 || v[0]!='"' || v[n-1]!='"' || (size_t)(n-2)!=bytes*2)return 0;
    for(size_t i=0;i<bytes;i++){
        int hi=og_saved_nibble(v[1+2*i]),lo=og_saved_nibble(v[2+2*i]);
        if(hi<0||lo<0)return 0;
        dst[i]=(uint8_t)((hi<<4)|lo);
    }
    *hash=og_saved_hash(*hash,dst,bytes);
    return 1;
}
/* A separate, checksummed fixture witness inside the same atomic CAR.
 * Byte wire is portable across endian/ABI, unlike packed native pointers.
 * This is NOT permission to replay or synthesize any result on CONTINUE. */
static int exp_fixture_emit_json(OGEmit emit,void *ud)
{
    if(s_expOGFixtureIssues || s_expOGFixtureCount>EXP_OG_FIXTURE_MAX)
        return 0; /* bad tick must preserve the prior CAR */
    const size_t n=s_expOGFixtureCount*EXP_OG_FIXTURE_SIZE;
    if(n>(size_t)EXP_OG_FIXTURE_MAX*EXP_OG_FIXTURE_SIZE)return 0;
    char *hex=(char *)malloc(n*2u+1u);
    if(!hex)return 0;
    static const char digits[]="0123456789abcdef";
    for(size_t i=0;i<n;i++) {
        const uint8_t b=s_expOGFixtures[i];
        hex[2u*i]=digits[b>>4];hex[2u*i+1u]=digits[b&15u];
    }
    hex[n*2u]=0;
    const uint64_t hash=exp_fixture_hash(s_expOGFixtures,n);
    char checksum[17];
    uint64_t value=hash;
    for(int i=15;i>=0;i--){checksum[i]=digits[value&15u];value>>=4;}
    checksum[16]=0;
    mjson_printf(emit,ud,",%Q:{%Q:%u,%Q:%u,%Q:%u,%Q:%u,%Q:%Q,%Q:%Q}",
        "fixture_journal","format",1u,
        "record_size",EXP_OG_FIXTURE_SIZE,
        "count",(unsigned)s_expOGFixtureCount,
        "partial",(unsigned)s_expOGFixturePartial,
        "hash",checksum,"hex",hex);
    free(hex);
    fprintf(stderr,"[og-fixture-journal] SERIALIZED world=%s count=%lu "
                   "hash=%016llx partial=%u status=CHECKSUMMED\n",
        World_Tag(),(unsigned long)s_expOGFixtureCount,
        (unsigned long long)hash,(unsigned)s_expOGFixturePartial);
    return 1;
}
static int exp_fixture_read_json(const char *root,int len,
                                  uint8_t **data,size_t *count,
                                  uint32_t *partial)
{
    *data=NULL;*count=0u;*partial=1u;
    const char *obj=NULL;int size=0;
    const int type=mjson_find(root,len,"$.fixture_journal",&obj,&size);
    if(type==MJSON_TOK_INVALID) {
        /* An older .CAR still restores its actual native OG state. The
         * missing per-fixture witnesses are explicitly UNKNOWN. */
        return 1;
    }
    if(type!=MJSON_TOK_OBJECT)return 0;
    long fmt=0,recordSize=0,entries=0,isPartial=0;
    if(!og_saved_num(obj,size,"$.format",&fmt) || fmt!=1 ||
       !og_saved_num(obj,size,"$.record_size",&recordSize) ||
       recordSize!=EXP_OG_FIXTURE_SIZE ||
       !og_saved_num(obj,size,"$.count",&entries) ||
       entries<0 || entries>EXP_OG_FIXTURE_MAX ||
       !og_saved_num(obj,size,"$.partial",&isPartial) ||
       (isPartial!=0 && isPartial!=1))return 0;
    const size_t n=(size_t)entries*EXP_OG_FIXTURE_SIZE;
    uint8_t *wire=(uint8_t *)malloc(n?n:1u);
    if(!wire)return 0;
    uint64_t hash=UINT64_C(14695981039346656037);
    int valid=og_saved_read_hex(obj,size,"$.hex",wire,n,&hash);
    char checksum[32]={0};
    if(valid)valid=mjson_get_string(obj,size,"$.hash",checksum,
                                    sizeof checksum)==16;
    if(valid) {
        uint64_t claimed=0;
        for(unsigned i=0;i<16u;i++){
            int v=og_saved_nibble(checksum[i]);
            if(v<0){valid=0;break;}
            claimed=(claimed<<4)|(unsigned)v;
        }
        if(claimed!=hash)valid=0;
    }
    /* Validate all IDs/wire observations without using OG simulation.
     * The observed civil date is an UPPER bound, not fixture kick-off. */
    for(size_t i=0;valid && i<(size_t)entries;i++){
        const uint8_t *p=wire+i*EXP_OG_FIXTURE_SIZE;
        ExpandedWorldDate observed;
        const uint32_t day=exp_fixture_get32(p+16);
        if(p[0]>3u || !p[15] || (p[15]&~3u)!=0u ||
           exp_fixture_get16(p+4)<World_FirstYear() ||
           exp_fixture_get16(p+8)==exp_fixture_get16(p+10) ||
           day>INT32_MAX ||
           !ExpandedWorld_IndexToDate((int32_t)day,&observed))
            valid=0;
    }
    if(valid) {
        uint32_t *scratch=(uint32_t *)calloc(
            EXP_OG_FIXTURE_BUCKETS,sizeof(uint32_t));
        if(!scratch)valid=0;
        else {
            valid=exp_fixture_build_index(wire,(size_t)entries,scratch);
            free(scratch);
        }
    }
    if(!valid){free(wire);return 0;}
    *data=wire;*count=(size_t)entries;*partial=(uint32_t)isPartial;
    return 1;
}
static int og_saved_read_context(const char *obj,int len,void *dest,int cup,unsigned idx) {
    ExpandedOGLeague *l=(ExpandedOGLeague *)dest;
    ExpandedOGCup *c=(ExpandedOGCup *)dest;
    const size_t size=cup?offsetof(ExpandedOGCup,teams):offsetof(ExpandedOGLeague,teams);
    const unsigned max=cup?EXP_OG_MAX_CUP_TEAMS:EXP_OG_MAX_TEAMS;
    long index=0,country=0,slot=0,year=0,cid=0,teams=0,sz=0;
    if(!og_saved_num(obj,len,"$.index",&index) || index!=(long)idx ||
       !og_saved_num(obj,len,"$.country",&country) ||
       !og_saved_num(obj,len,"$.slot",&slot) ||
       !og_saved_num(obj,len,"$.year",&year) ||
       !og_saved_num(obj,len,"$.comp_id",&cid) ||
       !og_saved_num(obj,len,"$.team_count",&teams) ||
       !og_saved_num(obj,len,"$.core_size",&sz) || sz!=(long)size ||
       teams<0 || teams>(long)max || year<0 || year>2198)
        return 0;
    uint64_t hash=UINT64_C(14695981039346656037);
    uint8_t *team=(uint8_t *)(cup?(void *)c->teams:(void *)l->teams);
    if(!og_saved_read_hex(obj,len,"$.core_hex",(uint8_t *)dest,size,&hash) ||
       !og_saved_read_hex(obj,len,"$.teams_hex",team,
                          (size_t)teams*sizeof(TeamFileEx),&hash))return 0;
    const char *v=NULL;int n=0;
    if(mjson_find(obj,len,"$.checksum",&v,&n)!=MJSON_TOK_STRING ||
       n!=18 || v[0]!='"' || v[17]!='"')return 0;
    uint64_t expected=0;
    for(int i=1;i<17;i++){int d=og_saved_nibble(v[i]);if(d<0)return 0;expected=(expected<<4)|(unsigned)d;}
    if(expected!=hash ||
       (cup?c->country:l->country)!=(unsigned)country ||
       (cup?c->slot:l->division)!=(unsigned)slot ||
       (cup?c->year:l->year)!=(unsigned)year ||
       (cup?c->compId:(unsigned)l->comp.base.field_2E)!=(unsigned)cid ||
       (cup?c->teamCount:l->teamCount)!=(unsigned)teams ||
       (cup?c->used:l->used)>1 ||
       ((cup?c->used:l->used)!=0 && year<World_FirstYear()))return 0;
    DIYCompetitionEx *comp=cup?&c->comp:&l->comp;
    DIYTournamentState *tourn=cup?&c->tournament:&l->tournament;
    if(comp->base.namePtr || tourn->header.namePtr)return 0;
    comp->base.namePtr=(uint8_t *)comp->base.competitionName;
    tourn->header.namePtr=(uint8_t *)tourn->header.competitionName;
    return 1;
}

static int exp_qual_read_json(const char *root,int len,
                               ExpandedOGQualificationSource *out)
{
    memset(out,0,sizeof *out);
    const char *obj=NULL;int n=0;
    const int token=mjson_find(root,len,"$.qual_next",&obj,&n);
    if(token==MJSON_TOK_INVALID)return 1; /* old CAR: never forge */
    if(token!=MJSON_TOK_OBJECT)return 0;
    long format=0;
    if(!og_saved_num(obj,n,"$.format",&format)||format!=1)return 0;
    uint8_t wire[EXP_OG_QUAL_WIRE_SIZE];
    uint64_t hash=UINT64_C(14695981039346656037);
    if(!og_saved_read_hex(obj,n,"$.hex",wire,sizeof wire,&hash))
        return 0;
    char checksum[32]={0};
    if(mjson_get_string(obj,n,"$.checksum",checksum,
                         sizeof checksum)!=16)return 0;
    uint64_t claimed=0;
    for(unsigned i=0;i<16u;++i) {
        const int digit=og_saved_nibble(checksum[i]);
        if(digit<0)return 0;
        claimed=(claimed<<4)|(unsigned)digit;
    }
    if(claimed!=hash || wire[0]!=1u ||
       wire[1]!=(unsigned)World_Current())return 0;
    out->world=wire[1];
    out->year=(uint16_t)(((unsigned)wire[2]<<8)|wire[3]);
    if(out->year<World_FirstYear()||out->year>2198u)return 0;
    out->valid=1;out->last_world=(unsigned)-1;
    for(unsigned k=0;k<4u;++k) {
        const unsigned count=wire[4u+k],invalid=wire[8u+k];
        if(count>EXP_QUAL_MAX_ENTRANTS || invalid>1u)return 0;
        out->count[k]=count;out->invalid[k]=invalid;
        for(unsigned j=0;j<EXP_QUAL_MAX_ENTRANTS;++j) {
            const unsigned pos=12u+2u*(k*EXP_QUAL_MAX_ENTRANTS+j);
            const uint16_t id=(uint16_t)
                (((uint16_t)wire[pos]<<8)|wire[pos+1u]);
            if(j>=count) {if(id)return 0;continue;}
            if(id==0xffffu)return 0;
            for(unsigned z=0;z<j;++z)
                if(out->ids[k][z]==id)return 0;
            out->ids[k][j]=id;
        }
    }
    return 1;
}
extern void Expanded_OGWorld_AdoptRestoredYear(uint16_t year);
int Expanded_OGWorld_ReadSaveJson(const char *json,int len,int apply) {
    const char *root=NULL;int nr=0;
    if(mjson_find(json,len,"$.og_resident",&root,&nr)!=MJSON_TOK_OBJECT)return 0;
    long ver=0,year=0,lc=0,cc=0,uy=0,sl=0,sc=0,st=0,bound=0,unsupported=0,faults=0;
    char world[16]={0};
    if(!og_saved_num(root,nr,"$.format",&ver) || ver!=1 ||
       mjson_get_string(root,nr,"$.world",world,sizeof world)<=0 ||
       strcmp(world,World_Tag()) ||
       !og_saved_num(root,nr,"$.year",&year) ||
       !og_saved_num(root,nr,"$.league_slots",&lc) ||
       !og_saved_num(root,nr,"$.cup_slots",&cc) ||
       !og_saved_num(root,nr,"$.uefa_year",&uy) ||
       !og_saved_num(root,nr,"$.league_size",&sl) ||
       !og_saved_num(root,nr,"$.cup_size",&sc) ||
       !og_saved_num(root,nr,"$.team_size",&st) ||
       !og_saved_num(root,nr,"$.club_bound",&bound) ||
       !og_saved_num(root,nr,"$.cup_unsupported",&unsupported) ||
       !og_saved_num(root,nr,"$.cup_faults",&faults) ||
       sl!=(long)sizeof(ExpandedOGLeague) ||
       sc!=(long)sizeof(ExpandedOGCup) ||
       st!=(long)sizeof(TeamFileEx) ||
       lc<1 || lc>EXP_OG_MAX_LEAGUES || cc<0 || cc>EXP_OG_MAX_CUPS ||
       year<World_FirstYear() || year>2198 || uy<0 || uy>2198 ||
       bound<0 || bound>1 || unsupported<0 || faults<0)return 0;
    ExpandedOGLeague *l=(ExpandedOGLeague *)calloc((size_t)lc,sizeof(*l));
    ExpandedOGLeague *u=(ExpandedOGLeague *)calloc(3u,sizeof(*u));
    ExpandedOGCup *c=(ExpandedOGCup *)calloc((size_t)(cc?cc:1),sizeof(*c));
    if(!l||!u||!c){free(l);free(u);free(c);return 0;}
    int valid=1;unsigned live=0;
    const char *const paths[]={"$.leagues","$.uefa","$.cups"};
    for(int k=0;k<3 && valid;k++){
        const char *arr=NULL;int al=0,pos=0;
        const long count=k==0?lc:k==1?3:cc;
        if(mjson_find(root,nr,paths[k],&arr,&al)!=MJSON_TOK_ARRAY){valid=0;break;}
        for(long i=0;i<count;i++){
            const char *obj=NULL;int ol=0;
            if(og_saved_next(arr,al,&pos,&obj,&ol)!=1 ||
               !og_saved_read_context(obj,ol,k==0?(void *)&l[i]:
                   k==1?(void *)&u[i]:(void *)&c[i],k==2,(unsigned)i)) {
                valid=0;break;
            }
        }
        if(valid) { const char *extra=NULL; int extraLen=0;
            if(og_saved_next(arr,al,&pos,&extra,&extraLen)!=0)valid=0; }
    }
    if(valid)for(long i=0;i<lc;i++){
        if(l[i].used){
            if(l[i].teamCount<2 || l[i].year>(unsigned)year)valid=0;
            live++;
        }
        for(long j=0;j<i;j++)
            if(l[i].used && l[j].used &&
               l[i].country==l[j].country && l[i].division==l[j].division &&
               l[i].year==l[j].year)valid=0;
    }
    /* Unique OG competition keys: never import two resident brackets for
     * the same country, table slot and season. Different countries may
     * legitimately reuse a native compId. */
    if(valid)for(long i=0;i<cc;i++){
        if(c[i].used && c[i].teamCount<2)valid=0;
        for(long j=0;j<i;j++)
            if(c[i].used && c[j].used &&
               c[i].country==c[j].country && c[i].slot==c[j].slot &&
               c[i].year==c[j].year)valid=0;
    }
    if(valid)for(unsigned i=0;i<3u;i++)
        if(u[i].used && (u[i].division!=i || u[i].teamCount<2))
            valid=0;
    if(!live)valid=0;
    ExpandedOGQualificationSource savedQual;
    if(valid && !exp_qual_read_json(root,nr,&savedQual))
        valid=0;
    if(valid && savedQual.valid &&
       savedQual.year!=(uint16_t)year &&
       savedQual.year!=(uint16_t)((unsigned)year+1u))
        valid=0;
    /* Compare verified CAR qualification IDs against any already
     * installed SAME-YEAR native OG edition BEFORE applying the save.
     * No new fixture or draw: a mismatch rejects the CAR atomically. */
    unsigned comparedFields=0u;
    if(valid && savedQual.valid) {
        for(unsigned k=0;k<4u;++k) {
            if(!savedQual.count[k] || savedQual.invalid[k])continue;
            const TeamFileEx *teams=NULL;
            unsigned count=0u;
            if(World_Current()==WORLD_9697 && (k==1u || k==2u)) {
                for(long j=0;j<cc;++j)
                    if(c[j].used && c[j].country==254u &&
                       c[j].slot==k && c[j].year==savedQual.year) {
                        teams=c[j].teams;count=c[j].teamCount;break;
                    }
            } else if(k!=3u || World_Current()==WORLD_9697) {
                const ExpandedOGLeague *const slot=
                    &u[k==3u?1u:k];
                if(slot->used && slot->year==savedQual.year) {
                    teams=slot->teams;count=slot->teamCount;
                }
            }
            if(!teams)continue; /* a future edition has not opened */
            uint16_t resident[EXP_QUAL_MAX_ENTRANTS]={0};
            if(!count || count>EXP_QUAL_MAX_ENTRANTS) {
                valid=0;break;
            }
            for(unsigned j=0;j<count;++j)
                resident[j]=PACK_BE_U16(
                    teams[j].team.countryNumber,teams[j].team.teamOrdinal);
            const ExpandedQualDiff diff=ExpandedQual_Compare(
                savedQual.ids[k],savedQual.count[k],resident,count);
            if(!ExpandedQual_IsExact(&diff)) {
                fprintf(stderr,"[og-qual-car] REJECT world=%s "
                    "year=%u cup=%u status=SOURCE_RESIDENT_ID_MISMATCH "
                    "missing=%u extra=%u duplicate=%u\n",
                    World_Tag(),(unsigned)savedQual.year,k,
                    diff.missing,diff.extra,diff.duplicate_resident);
                valid=0;break;
            }
            ++comparedFields;
        }
    }
    if(valid && savedQual.valid)
        fprintf(stderr,"[og-qual-car] PRECHECK world=%s year=%u "
            "source-checksum=VALID resident-fields-compared=%u "
            "status=%s\n",World_Tag(),(unsigned)savedQual.year,
            comparedFields,comparedFields?
                "VERIFIED_SAME_YEAR_IDS":"SOURCE_ONLY_EDITION_NOT_YET_OPEN");
    uint8_t *fixtureData=NULL;
    size_t fixtureCount=0u;
    uint32_t fixturePartial=1u;
    if(valid && !exp_fixture_read_json(root,nr,&fixtureData,
                                        &fixtureCount,&fixturePartial))
        valid=0;
    if(valid && apply){
        memset(s_expOG,0,sizeof s_expOG);
        memset(s_expOGUefa,0,sizeof s_expOGUefa);
        memset(s_expOGCups,0,sizeof s_expOGCups);
        memcpy(s_expOG,l,(size_t)lc*sizeof(*l));
        memcpy(s_expOGUefa,u,3u*sizeof(*u));
        if(cc)memcpy(s_expOGCups,c,(size_t)cc*sizeof(*c));
        for(long i=0;i<lc;i++){
            s_expOG[i].comp.base.namePtr=(uint8_t *)s_expOG[i].comp.base.competitionName;
            s_expOG[i].tournament.header.namePtr=(uint8_t *)s_expOG[i].tournament.header.competitionName;
        }
        for(unsigned i=0;i<3;i++){
            s_expOGUefa[i].comp.base.namePtr=(uint8_t *)s_expOGUefa[i].comp.base.competitionName;
            s_expOGUefa[i].tournament.header.namePtr=(uint8_t *)s_expOGUefa[i].tournament.header.competitionName;
        }
        for(long i=0;i<cc;i++){
            s_expOGCups[i].comp.base.namePtr=(uint8_t *)s_expOGCups[i].comp.base.competitionName;
            s_expOGCups[i].tournament.header.namePtr=(uint8_t *)s_expOGCups[i].tournament.header.competitionName;
        }
        s_expOGCount=(uint32_t)lc;s_expOGCupCount=(uint32_t)cc;
        s_expOGUefaCalendarYear=(uint16_t)uy;
        s_expOGCurrent=NULL;s_expOGCupCurrent=NULL;
        s_expOGPhase=-1;s_expOGCupPhase=-1;s_expOGViewing=0;
        s_expOGCupTickFault=0;s_expOGFault=0;s_expOGBound=(int)bound;
        s_expOGCupUnsupported=(uint32_t)unsupported;
        s_expOGCupFaults=(uint32_t)faults;
        s_expQual=savedQual;
        fprintf(stderr,"[og-qual-car] RESTORE world=%s year=%u "
            "valid=%d counts=%u,%u,%u,%u status=%s\n",
            World_Tag(),(unsigned)s_expQual.year,s_expQual.valid,
            s_expQual.count[0],s_expQual.count[1],
            s_expQual.count[2],s_expQual.count[3],
            s_expQual.valid?"CHECKSUM_VERIFIED":"LEGACY_SOURCE_UNAVAILABLE");
        Expanded_OGWorld_AdoptRestoredYear((uint16_t)year);
        free(s_expOGFixtures);
        s_expOGFixtures=fixtureData;
        s_expOGFixtureCount=fixtureCount;
        s_expOGFixtureCapacity=fixtureCount;
        s_expOGFixturePartial=fixturePartial;
        s_expOGFixtureIssues=0u;
        if(!exp_fixture_build_index(s_expOGFixtures,s_expOGFixtureCount,
                                    s_expOGFixtureBuckets))
            ++s_expOGFixtureIssues;
        s_expOGCapturedLeague=s_expOGCapturedCup=s_expOGCapturedUefa=0u;
        fixtureData=NULL;
        fprintf(stderr,"[og-fixture-journal] RESTORE world=%s year=%ld "
            "count=%lu partial=%u hash=%016llx status=%s\n",
            World_Tag(),year,(unsigned long)fixtureCount,
            (unsigned)fixturePartial,
            (unsigned long long)exp_fixture_hash(
                s_expOGFixtures,s_expOGFixtureCount*EXP_OG_FIXTURE_SIZE),
            fixturePartial?"LEGACY_INCOMPLETE":"CHECKSUM_VERIFIED");
        fprintf(stderr,"[og-world-save] RESTORED world=%s year=%ld leagues=%ld "
                       "uefa=3 cups=%ld checksum=VALID\n",World_Tag(),year,lc,cc);
    }
    free(l);free(u);free(c);free(fixtureData);
    return valid;
}
#endif /* SWOS_SDL_BUILD */

/* Cross-process save/restore probe. This is deliberately a FIELD fingerprint,
 * not a proof that the JSON contains the complete world. Hash only pointer-free
 * native bytes and identity/roster/RNG data; never simulate or mutate. A
 * missing fingerprint or a changed value after CONTINUE is evidence of a gap. */
static uint64_t exp_og_fp_bytes(uint64_t h,const void *ptr,size_t size)
{
    const uint8_t *p=(const uint8_t *)ptr;
    for(size_t i=0;i<size;++i){h^=p[i];h*=UINT64_C(1099511628211);}
    return h;
}
static uint64_t exp_og_fp_core(uint64_t h,const DIYCompetitionEx *comp,
                               const DIYTournamentState *tourn,
                               const TeamFileEx *teams,uint16_t n)
{
    /* Skip namePtr in each DIYCompetition base (ASLR addresses differ
     * between Windows processes). Every byte following the base in
     * DIYCompetitionEx and every tournament-team/slot record is pointer-free. */
    h=exp_og_fp_bytes(h,comp->base.competitionName,sizeof comp->base.competitionName);
    h=exp_og_fp_bytes(h,&comp->base.competitionType,sizeof comp->base.competitionType);
    h=exp_og_fp_bytes(h,&comp->base.numTeams,sizeof comp->base.numTeams);
    h=exp_og_fp_bytes(h,&comp->base.viewRandomPhase,sizeof comp->base.viewRandomPhase);
    h=exp_og_fp_bytes(h,&comp->base.winnerTeam,sizeof comp->base.winnerTeam);
    h=exp_og_fp_bytes(h,&comp->minSubstitutes,
                     sizeof(*comp)-offsetof(DIYCompetitionEx,minSubstitutes));
    h=exp_og_fp_bytes(h,&tourn->header.competitionType,
                     sizeof(tourn->header.competitionType));
    h=exp_og_fp_bytes(h,&tourn->teamIds,
                     sizeof(*tourn)-offsetof(DIYTournamentState,teamIds));
    h=exp_og_fp_bytes(h,teams,(size_t)n*sizeof(TeamFileEx));
    return h;
}
void Expanded_OGWorld_LogSaveFingerprints(const char *stage)
{
    if(!stage || (strcmp(stage,"SAVE_AFTER") && strcmp(stage,"RESTORE_AFTER")))
        return;
    unsigned entries=0u;
    for(uint32_t i=0;i<s_expOGCount;++i) {
        const ExpandedOGLeague *c=&s_expOG[i];
        if(!c->used || !c->teamCount || c->teamCount>EXP_OG_MAX_TEAMS)continue;
        uint64_t h=UINT64_C(14695981039346656037);
        h=exp_og_fp_bytes(h,&c->year,sizeof c->year);
        h=exp_og_fp_bytes(h,&c->country,sizeof c->country);
        h=exp_og_fp_bytes(h,&c->division,sizeof c->division);
        h=exp_og_fp_bytes(h,&c->lastPhase,sizeof c->lastPhase);
        h=exp_og_fp_bytes(h,&c->lastMatch,sizeof c->lastMatch);
        h=exp_og_fp_bytes(h,&c->seed,3u);
        h=exp_og_fp_core(h,&c->comp,&c->tournament,c->teams,c->teamCount);
        fprintf(stderr,"[og-state-fp] stage=%s world=%s kind=LEAGUE "
             "country=%u slot=%u year=%u teams=%u hash=%016llx\n",
             stage,World_Tag(),(unsigned)c->country,(unsigned)c->division,
             (unsigned)c->year,(unsigned)c->teamCount,(unsigned long long)h);
        ++entries;
    }
    for(unsigned k=0;k<3u;++k) {
        const ExpandedOGLeague *c=&s_expOGUefa[k];
        if(!c->used || !c->teamCount || c->teamCount>EXP_OG_MAX_TEAMS)continue;
        uint64_t h=UINT64_C(14695981039346656037);
        h=exp_og_fp_bytes(h,&c->year,sizeof c->year);
        h=exp_og_fp_bytes(h,&c->lastPhase,sizeof c->lastPhase);
        h=exp_og_fp_bytes(h,&c->lastMatch,sizeof c->lastMatch);
        h=exp_og_fp_bytes(h,&c->seed,3u);
        h=exp_og_fp_core(h,&c->comp,&c->tournament,c->teams,c->teamCount);
        fprintf(stderr,"[og-state-fp] stage=%s world=%s kind=UEFA "
             "country=254 slot=%u year=%u teams=%u hash=%016llx\n",
             stage,World_Tag(),k,(unsigned)c->year,
             (unsigned)c->teamCount,(unsigned long long)h);
        ++entries;
    }
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        const ExpandedOGCup *c=&s_expOGCups[i];
        if(!c->used || !c->teamCount || c->teamCount>EXP_OG_MAX_CUP_TEAMS)continue;
        uint64_t h=UINT64_C(14695981039346656037);
        h=exp_og_fp_bytes(h,&c->year,sizeof c->year);
        h=exp_og_fp_bytes(h,&c->country,sizeof c->country);
        h=exp_og_fp_bytes(h,&c->slot,sizeof c->slot);
        h=exp_og_fp_bytes(h,&c->compId,sizeof c->compId);
        h=exp_og_fp_bytes(h,&c->lastPhase,sizeof c->lastPhase);
        h=exp_og_fp_bytes(h,&c->lastMatch,sizeof c->lastMatch);
        h=exp_og_fp_bytes(h,&c->seed,3u);
        h=exp_og_fp_bytes(h,&c->matchesPlayed,sizeof c->matchesPlayed);
        h=exp_og_fp_core(h,&c->comp,&c->tournament,c->teams,c->teamCount);
        fprintf(stderr,"[og-state-fp] stage=%s world=%s kind=CUP "
             "country=%u slot=%u year=%u teams=%u hash=%016llx\n",
             stage,World_Tag(),(unsigned)c->country,(unsigned)c->slot,
             (unsigned)c->year,(unsigned)c->teamCount,(unsigned long long)h);
        ++entries;
    }
    fprintf(stderr,"[og-state-fp-summary] stage=%s world=%s entries=%u "
           "scope=POINTER_FREE_NATIVE_FIELDS_NOT_FULL_SAVE_PROOF\n",
           stage,World_Tag(),entries);
}

extern void Career_AutoAdvanceCupMatches(void);
/* Native bye detection for a real country cup, NOT a general exemption
 * from bracket validation. WorldCups_Entrants/Padded/CupMade have already
 * built the draw. Inspect only their output and original metadata.
 * The team cache contains selected-team INDICES (not raw pointers).
 * Dummy padded places must never be simulated as 0-v-0 fixtures. */
static int exp_og_cup_valid_native_byes(const uint8_t *record,
                                       uint8_t country,
                                       unsigned selected,unsigned bracket)
{
    int32_t count=0;
    const WorldCup *rules=World_Cups(&count);
    if(!rules)return 0;
    for(int32_t k=0;k<count;++k) {
        const WorldCup *w=&rules[k];
        if(w->country!=country || w->record!=(const void *)record)
            continue;
        const unsigned pairs=(unsigned)_diyCompetitionStart.base.matchPairsPerRound;
        const unsigned ties=(unsigned)_diyCompetitionStart.base.totalMatchesInRound;
        if(!ExpandedOGCup_NativeByeLayout(bracket,selected,(unsigned)w->n,
                                            (unsigned)w->ties,bracket,
                                            ties,pairs))return 0;
        const uint8_t *grid=_diyCompetitionStart.teamGridPairings;
        for(unsigned i=0;i<pairs;++i) {
            const unsigned a=(unsigned)grid[2u*i];
            const unsigned b=(unsigned)grid[2u*i+1u];
            if(a>=selected || b>=selected || a>=64u || b>=64u)
                return 0;
            /* Access the packed fields DIRECTLY: no potentially
             * unaligned int32_t* escapes the OG structure. */
            const int32_t idA=_diyCompetitionStart.bracketScratch.teamOffsetTable[a];
            const int32_t idB=_diyCompetitionStart.bracketScratch.teamOffsetTable[b];
            if(idA<0 || idB<0 ||
               (unsigned)idA>=selected || (unsigned)idB>=selected)
                return 0;
            if(i<(unsigned)w->ties) {
                if(a==b || idA==idB)return 0;
            } else {
                if(a!=b || idA!=idB)return 0;
            }
        }
        fprintf(stderr,"[og-cup-bye] NATIVE DRAW VERIFIED "
                       "country=%u compId=%u real=%u bracket=%u "
                       "round1-ties=%u byes=%u (OG ONLY)\n",
                (unsigned)country,(unsigned)((const Competition *)record)->compId,
                selected,bracket,(unsigned)w->ties,selected-2u*(unsigned)w->ties);
        return 1;
    }
    return 0;
}
int Expanded_OGWorld_AdvanceCup(uint8_t *record,uint8_t country,
                                uint8_t slot,uint16_t year,uint8_t phase)
{
    if(!record||_gameType!=4||s_expOGCupCurrent||s_expOGCurrent||
       ((Competition *)record)->type!=1) return 0;
    const CompCupHeader *cup=(const CompCupHeader *)record;
    const unsigned entrants=(unsigned)cup->teams;
    if(entrants<2u || entrants>EXP_OG_MAX_CUP_TEAMS ||
       (entrants&1u)) {
        if(phase==0u) {
            ++s_expOGCupUnsupported;
            fprintf(stderr,"[og-cup-resident] UNSUPPORTED country=%u "
                           "slot=%u year=%u teams=%u\n",
                    (unsigned)country,(unsigned)slot,(unsigned)year,entrants);
        }
        return -1;
    }
    ExpandedOGCup *ctx=exp_cup_find(country,slot,year);
    if(ctx && (ctx->managed_excluded || ctx->unsupported_setup))return -1;
    if(ctx && ctx->used &&
       (ctx->teamCount<2u || ctx->teamCount>EXP_OG_MAX_CUP_TEAMS))
        return 0;
    if(ctx && ctx->used && phase<=ctx->lastPhase)return 1;
    if(!ctx) {
        if(s_expOGCupCount==EXP_OG_MAX_CUPS) {
            ++s_expOGCupUnsupported;
            fprintf(stderr,"[og-cup-resident] CAPACITY year=%u\n",
                    (unsigned)year);
            return -1;
        }
        ctx=&s_expOGCups[s_expOGCupCount++];
        memset(ctx,0,sizeof(*ctx));
        ctx->country=country;ctx->slot=slot;ctx->year=year;
        ctx->compId=(uint8_t)((Competition *)record)->compId;
        ctx->lastMatch=0xffu;
    }
    DIYCompetitionEx oldComp;
    DIYTournamentState oldTournament;
    const uint8_t oldSeason=_seasonPlaying,oldView=_viewRandomPhase;
    const uint16_t oldFlag=_internationalAutoAdvanceFlag;
    const uint16_t oldCareerLoaded=_careerLoadedFlag;
    const uint16_t oldTeamsLoaded=_teamsLoaded;
    const uint8_t globalLast=_careerLastMatchIndexByCompetitionSlot[ctx->compId];
    memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
    memcpy(&oldTournament,&_diyFileBufferCopy,sizeof oldTournament);
    SaveSelectedTeams();
    SaveRand2State();
    _internationalAutoAdvanceFlag=1;
    _careerLoadedFlag=1;
    _teamsLoaded=0;
    _seasonPlaying=(uint8_t)((96u+(uint32_t)year-World_FirstYear())%100u);
    /* The OG tournament is initialized ONLY ONCE per country/slot/year.
     * Later dated ticks restore the SAME bracket, club rosters and RNG.
     * Recalling InitDIYCupFromRecord every phase would re-run draw hooks.
     * Never call the complete-year legacy auto-advance callback here. */
    if(ctx->used) {
        memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
        memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
        _numSelectedTeams=ctx->teamCount;
        memcpy(_selectedTeams,ctx->teams,
               (size_t)ctx->teamCount*sizeof(TeamFileEx));
        _seed2=ctx->seed;
        _randXorIndex2=ctx->randIndex;
        _randXorKey2=ctx->randKey;
    } else {
        Randomize2((uint16_t)(((uint16_t)record[0]<<8)|_seasonPlaying));
        _numSelectedTeams=0;
        Career_InitCompetitionTeamSelectionFromTableHeaderSpec(record);
        InitDIYCupFromRecord(record);
        InitTeams();
    }
    const unsigned selected=(unsigned)_numSelectedTeams;
    const unsigned bracket=(unsigned)_diyCompetitionStart.base.numTeams;
    /* Standard cups: all selected teams occupy actual bracket places.
     * OG world cups: fewer selected clubs is CORRECT only when the
     * ORIGINAL WorldCups rule, padded draw, and all real/bye pairing
     * identities match exactly. Recheck only at initial setup because
     * later rounds necessarily have different fixture counts. */
    const int nativeBye=ctx->used ? (ctx->nativeByeVerified!=0) :
        (selected<bracket && selected>=2u &&
         exp_og_cup_valid_native_byes(record,country,selected,bracket));
    const int valid=selected>=2u && selected<=EXP_OG_MAX_CUP_TEAMS &&
        bracket>=2u && bracket<=EXP_OG_MAX_CUP_TEAMS &&
        (bracket==selected || nativeBye);
    int failed=0;
    ctx->observedSelected=(uint16_t)selected;
    /* Capture what the OG bracket builder ACTUALLY produced, even when
     * resident verification must reject the resulting scratch. */
    ctx->observedBracketTeams=(uint16_t)_diyCompetitionStart.base.numTeams;
    ctx->observedRoundTies=(uint16_t)_diyCompetitionStart.base.totalMatchesInRound;
    ctx->observedRoundPairs=(uint16_t)_diyCompetitionStart.base.matchPairsPerRound;
    const uint32_t oldPlayed=ctx->matchesPlayed;
    if(!valid)ctx->unsupported_setup=1u;
    if(nativeBye)ctx->nativeByeVerified=1u;
    if(!valid && selected<bracket)
        fprintf(stderr,"[og-cup-bye] REJECT country=%u slot=%u "
                       "year=%u real=%u bracket=%u "
                       "(no OG rule or invalid native pairing)\n",
                (unsigned)country,(unsigned)slot,
                (unsigned)year,selected,bracket);
    int managed=0;
    /* Existing employed OG career slots are still authoritative for their
     * own cup: without a proven identity-remapping bind, NEVER play those
     * same fixtures in the resident cup too. Mark this cup unsupported for
     * the remainder of its season rather than replay manager results when
     * fired. This is an explicit temporary integration limit, NOT a pass. */
    if(valid && _selTeamNumber!=0xffffu) {
        for(unsigned i=0;i<(unsigned)_numSelectedTeams;++i) {
            const TeamFile *t=&_selectedTeams[i].team;
            const uint16_t id=PACK_BE_U16(t->countryNumber,t->teamOrdinal);
            if(id==_selTeamNumber) {managed=1;break;}
        }
        if(managed) {
            ctx->managed_excluded=1u;
            ctx->used=0u;
        }
    }
    if(valid && !managed) {
        _careerLastMatchIndexByCompetitionSlot[ctx->compId]=ctx->lastMatch;
        s_expOGCupTickFault=0;
        s_expOGCupCurrent=ctx;s_expOGCupPhase=(int)phase;
        if(phase>0u)Career_AutoAdvanceCupMatches();
        failed=s_expOGCupTickFault;
        s_expOGCupCurrent=NULL;s_expOGCupPhase=-1;
        /* No discarded/partial result can enter the resident context. */
        if(!failed) {
            ctx->lastMatch=_careerLastMatchIndexByCompetitionSlot[ctx->compId];
            memcpy(&ctx->comp,&_diyCompetitionStart,sizeof ctx->comp);
            memcpy(&ctx->tournament,&_diyFileBufferCopy,sizeof ctx->tournament);
            ctx->teamCount=_numSelectedTeams;
            if(ctx->teamCount>EXP_OG_MAX_CUP_TEAMS)failed=1;
            if(!failed) {
                memcpy(ctx->teams,_selectedTeams,
                       (size_t)ctx->teamCount*sizeof(TeamFileEx));
                ctx->seed=_seed2;
                ctx->randIndex=_randXorIndex2;
                ctx->randKey=_randXorKey2;
                ctx->lastPhase=phase;
                ctx->used=1;
            }
        }
    }
    _careerLastMatchIndexByCompetitionSlot[ctx->compId]=globalLast;
    _seasonPlaying=oldSeason;_viewRandomPhase=oldView;
    _internationalAutoAdvanceFlag=oldFlag;
    _careerLoadedFlag=oldCareerLoaded;
    _teamsLoaded=oldTeamsLoaded;
    RestoreRand2State();
    RestoreSelectedTeams();
    memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
    memcpy(&_diyFileBufferCopy,&oldTournament,sizeof oldTournament);
    if(!valid) {
        ++s_expOGCupUnsupported;
        fprintf(stderr,"[og-cup-resident] UNSUPPORTED SETUP "
                       "country=%u slot=%u year=%u teams=%u selected=%u\n",
                (unsigned)country,(unsigned)slot,(unsigned)year,entrants,
                selected);
        return -1;
    }
    if(managed) {
        fprintf(stderr,"[og-cup-resident] MANAGER_SLOT_UNBOUND "
                       "country=%u slot=%u year=%u (NO DOUBLE MATCHES)\n",
                (unsigned)country,(unsigned)slot,(unsigned)year);
        ++s_expOGCupUnsupported;
        return -1;
    }
    if(failed) {
        ctx->matchesPlayed=oldPlayed;
        ++s_expOGCupFaults;
        fprintf(stderr,"[og-cup-resident] FAILED country=%u slot=%u "
                       "year=%u phase=%u selected=%u\n",
                (unsigned)country,(unsigned)slot,(unsigned)year,
                (unsigned)phase,(unsigned)_numSelectedTeams);
        return 0;
    }
    if(phase==0u || ctx->matchesPlayed!=oldPlayed || phase==56u)
        fprintf(stderr,"[og-cup-resident] TICK country=%u slot=%u "
                       "year=%u phase=%u matches=%lu finished=%u\n",
                (unsigned)country,(unsigned)slot,(unsigned)year,
                (unsigned)phase,(unsigned long)ctx->matchesPlayed,
                (unsigned)ctx->comp.base.competitionFinished);
    return 1;
}

/* Unemployed CLUB OFFER must read the SAME resident OpenSWOS league
 * advanced at START/FORWARD TIME. The old DIY_OGStandings_Read cache
 * is populated by OG menu callbacks and can be absent even when the
 * authoritative s_expOG league has already completed many fixtures.
 * This function neither initializes a competition nor advances its
 * schedule, alters an RNG, or opens an OG menu. */
static int exp_og_offer_row_compare(const ExpandedLiveRow *a,
                                    const ExpandedLiveRow *b)
{
    if(a->points!=b->points)return a->points>b->points?-1:1;
    const int gdA=(int)a->scored-(int)a->conceded;
    const int gdB=(int)b->scored-(int)b->conceded;
    if(gdA!=gdB)return gdA>gdB?-1:1;
    if(a->scored!=b->scored)return a->scored>b->scored?-1:1;
    const int names=strcmp(a->name,b->name);
    if(names)return names<0?-1:1;
    return a->club<b->club?-1:(a->club>b->club?1:0);
}
int Expanded_OGWorld_ReadLeagueStandings(uint8_t country,uint8_t division,
                                         uint16_t year,ExpandedLiveRow *rows,
                                         uint32_t capacity,uint32_t *total)
{
    if(total)*total=0;
    if(!rows||!total||_gameType!=4)return 0;
    const ExpandedOGLeague *ctx=exp_find(country,division,year);
    if(!ctx || !ctx->used)return 0;
    const DIYCompetitionEx *comp=&ctx->comp;
    const unsigned n=(unsigned)comp->base.numTeams;
    if(comp->base.competitionType!=0 || n<2u ||
       n>EXP_OG_MAX_TEAMS || n>ctx->teamCount || n>capacity)return 0;
    for(unsigned i=0;i<n;i++) {
        const unsigned off=comp->matchData.league.teamEntryOffsets[i];
        if(off%sizeof(DIYTeamStats)!=0u ||
           off/sizeof(DIYTeamStats)>=n)return 0;
        DIYTeamStats aligned_stat;
        memcpy(&aligned_stat,
               (const uint8_t *)comp +
                   offsetof(DIYCompetitionEx,leagueStandings) + off,
               sizeof aligned_stat);
        const DIYTeamStats *stat=&aligned_stat;
        const int32_t id=stat->teamSelOffset;
        if(id<0 || (uint32_t)id>=ctx->teamCount)return 0;
        const TeamFile *team=&ctx->teams[id].team;
        if(World_FileCountry(team->countryNumber)!=country ||
           team->league!=division)return 0;
        ExpandedLiveRow *row=&rows[i];
        memset(row,0,sizeof *row);
        row->club=PACK_BE_U16(team->countryNumber,team->teamOrdinal);
        memcpy(row->name,team->teamName,17u);
        row->name[17]=0;
        row->played=stat->gamesPlayed;
        row->wins=stat->wins;
        row->drawn=stat->draws;
        row->lost=stat->losses;
        row->scored=stat->goalsFor;
        row->conceded=stat->goalsAgainst;
        row->points=(int32_t)stat->points;
        if((uint32_t)row->wins+row->drawn+row->lost!=row->played)return 0;
        for(unsigned j=0;j<i;j++)
            if(rows[j].club==row->club)return 0; /* duplicate ID */
    }
    /* Presentation-only ordering: match OG standings tie-break order
     * (PTS, GD, GF, name). Sort local rows, NEVER modify resident OG. */
    for(unsigned i=1;i<n;i++) {
        ExpandedLiveRow key=rows[i];
        unsigned j=i;
        while(j && exp_og_offer_row_compare(&key,&rows[j-1])<0) {
            rows[j]=rows[j-1];--j;
        }
        rows[j]=key;
    }
    *total=n;
    fprintf(stderr,"[og-offer-resident] READ country=%u div=%u year=%u "
                   "teams=%u phase=%u played-matches=%u (NO SIMULATION)\n",
            (unsigned)country,(unsigned)division,(unsigned)year,n,
            (unsigned)ctx->lastPhase,(unsigned)comp->matchesPlayedCount);
    return 1;
}
/* DEBUG WORLD / career VIEW WORLD: read the resident OG league instead of
 * initializing or auto-simulating an external competition on menu entry.
 * The native OG viewer receives a disposable copy, never s_expOG itself.
 * Unavailable cups/leagues are reported, not invented or fast-forwarded. */
extern void *_competitionsTable[];
extern void ShowErrorMenu(uint8_t *text);
extern void ShowDiyLeaguesTournamentMenu(uint16_t type,uint16_t viewOnly,uint16_t focusOverride);
extern void ShowDIYCupMenuWithParams(uint16_t,uint16_t,uint16_t);
static int exp_og_view_cup(const uint8_t *record)
{
    static uint8_t missing[]="OG WORLD: CUP NOT RESIDENT";
    const Competition *wanted=(const Competition *)record;
    ExpandedOGCup *exact=NULL,*fallback=NULL;
    unsigned matches=0u;
    for(uint32_t i=0;i<s_expOGCupCount;++i) {
        ExpandedOGCup *ctx=&s_expOGCups[i];
        if(!ctx->used || ctx->year==0u)continue;
        const Competition *native=NULL;
        if(World_Current()==WORLD_9697 && ctx->country==254u &&
           (ctx->slot==1u || ctx->slot==2u)) {
            /* European Cup Winners' Cup and UEFA Cup are separate from the
             * country's domestic table in the classic OG Europe picker. */
            native=ctx->slot==1u?(const Competition *)&_cupWinnersCup
                                :(const Competition *)&_uefaCup;
        } else {
            const void *const *tab=(const void *const *)_competitionsTable[ctx->country];
            if(!tab)continue;
            native=(const Competition *)tab[ctx->slot];
        }
        if(native==wanted) {exact=ctx;break;}
        if(native && native->compId==wanted->compId) {
            ++matches;fallback=ctx;
        }
    }
    ExpandedOGCup *ctx=exact?exact:(matches==1u?fallback:NULL);
    if(!ctx || ctx->teamCount<2u ||
       ctx->teamCount>EXP_OG_MAX_CUP_TEAMS ||
       ctx->comp.base.competitionType!=1 ||
       ctx->comp.base.numTeams<2 ||
       ctx->comp.base.numTeams>EXP_OG_MAX_CUP_TEAMS) {
        fprintf(stderr,"[og-cup-view] NOT RESIDENT compId=%u matches=%u\n",
                (unsigned)wanted->compId,matches);
        ShowErrorMenu(missing);
        return 0;
    }
    DIYCompetitionEx oldComp;
    DIYTournamentState oldTournament;
    const uint8_t oldPhase=_viewRandomPhase,oldSeason=_seasonPlaying;
    const uint8_t oldLast=_careerLastMatchIndexByCompetitionSlot[ctx->compId];
    memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
    memcpy(&oldTournament,&_diyFileBufferCopy,sizeof oldTournament);
    SaveSelectedTeams();
    SaveRand2State();
    memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
    memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
    _numSelectedTeams=ctx->teamCount;
    memcpy(_selectedTeams,ctx->teams,
           (size_t)ctx->teamCount*sizeof(TeamFileEx));
    _diyCompetitionStart.base.namePtr=
        (uint8_t *)_diyCompetitionStart.base.competitionName;
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=1;
    const uint32_t before=ctx->matchesPlayed;
    if(ctx->country==254u) {
        extern uint16_t Expanded_OGWorld_SaveYear(void);
        if(ctx->year!=Expanded_OGWorld_SaveYear())
            fprintf(stderr,"[og-cup-view] LAST_COMPLETED_EDITION "
                "slot=%u edition-year=%u active-world-year=%u "
                "new-qualification=NOT_CONNECTED (NO NEW RESULTS)\n",
                (unsigned)ctx->slot,(unsigned)ctx->year,
                (unsigned)Expanded_OGWorld_SaveYear());
    }
    fprintf(stderr,"[og-cup-view] READ ONLY country=%u slot=%u year=%u "
                   "phase=%u matches=%lu\n",
            (unsigned)ctx->country,(unsigned)ctx->slot,
            (unsigned)ctx->year,(unsigned)ctx->lastPhase,
            (unsigned long)before);
    ShowDIYCupMenuWithParams(1,1,0);
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=0;
    RestoreRand2State();
    RestoreSelectedTeams();
    memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
    memcpy(&_diyFileBufferCopy,&oldTournament,sizeof oldTournament);
    _careerLastMatchIndexByCompetitionSlot[ctx->compId]=oldLast;
    _viewRandomPhase=oldPhase;_seasonPlaying=oldSeason;
    fprintf(stderr,"[og-cup-view] BACK unchanged=%u\n",
            (unsigned)(before==ctx->matchesPlayed));
    return before==ctx->matchesPlayed;
}
/* Type-2 resident UEFA viewer. Like the native domestic cups viewer,
 * this is a temporary read-only scratch image, never a re-initialization. */
static int exp_og_view_uefa(const Competition *wanted)
{
    static uint8_t missing[]="OG WORLD: UEFA CUP NOT RESIDENT";
    static uint8_t unsupported[]="OG WORLD: CONTINENTAL NOT RESIDENT";
    const int modern=World_Current()==WORLD_2627;
    const int classic=World_Current()==WORLD_9697;
    const Competition *historicCopa=classic?Comp_ById(COMPID_copaLibertadores):NULL;
    const int copa=classic && wanted && wanted==historicCopa &&
                   wanted->type==2;
    if((!modern && !classic) || !wanted ||
       (!copa && wanted->compId>=3) ||
       (classic && !copa && wanted!=(const Competition *)&_euroCup)) {
        fprintf(stderr,"[og-conti-view] NO RESIDENT TYPE2 cup=%u world=%u (no simulated fallback)\n",
                wanted?(unsigned)wanted->compId:255u,(unsigned)World_Current());
        ShowErrorMenu(unsupported);
        return 0;
    }
    ExpandedOGLeague *ctx=&s_expOGUefa[copa?1u:(unsigned)wanted->compId];
    const unsigned entrants=copa?20u:(modern?36u:16u);
    if(copa && ctx->country!=82u) {
        fprintf(stderr,"[og-libertadores-96] VIEW_REJECT missing-resident\n");
        ShowErrorMenu(missing);
        return 0;
    }
    if(!ctx->used || ctx->year!=s_expOGUefaCalendarYear ||
       ctx->teamCount!=entrants ||
       ctx->tournament.header.competitionType!=2) {
        fprintf(stderr,"[og-uefa-view] NOT RESIDENT cup=%d year=%u\n",
                (int)wanted->compId,(unsigned)ctx->year);
        ShowErrorMenu(missing);
        return 0;
    }
    DIYCompetitionEx oldComp;
    DIYTournamentState oldTournament;
    const uint8_t oldPhase=_viewRandomPhase,oldSeason=_seasonPlaying;
    const uint8_t cid=(uint8_t)wanted->compId;
    const uint8_t oldLast=_careerLastMatchIndexByCompetitionSlot[cid];
    memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
    memcpy(&oldTournament,&_diyFileBufferCopy,sizeof oldTournament);
    SaveSelectedTeams();SaveRand2State();
    memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
    memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
    _numSelectedTeams=ctx->teamCount;
    memcpy(_selectedTeams,ctx->teams,entrants*sizeof(TeamFileEx));
    _diyCompetitionStart.base.namePtr=
        (uint8_t *)_diyCompetitionStart.base.competitionName;
    _diyFileBufferCopy.header.namePtr=
        (uint8_t *)_diyFileBufferCopy.header.competitionName;
    const unsigned before=(unsigned)ctx->comp.matchesPlayedCount;
    extern uint16_t Expanded_OGWorld_SaveYear(void);
    if(ctx->year!=Expanded_OGWorld_SaveYear())
        fprintf(stderr,"[og-uefa-view] LAST_COMPLETED_EDITION "
                       "cup=%u edition-year=%u active-world-year=%u "
                       "new-qualification=NOT_CONNECTED (NO NEW RESULTS)\n",
                (unsigned)cid,(unsigned)ctx->year,
                (unsigned)Expanded_OGWorld_SaveYear());
    fprintf(stderr,"[og-uefa-view] READ ONLY cup=%u year=%u phase=%u native-matches=%u\n",
            (unsigned)cid,(unsigned)ctx->year,(unsigned)ctx->lastPhase,before);
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=1;
    DIY_EditCompetitionSlot(1,1,0); /* original type-2 OG viewer */
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=0;
    RestoreRand2State();RestoreSelectedTeams();
    memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
    memcpy(&_diyFileBufferCopy,&oldTournament,sizeof oldTournament);
    _careerLastMatchIndexByCompetitionSlot[cid]=oldLast;
    _viewRandomPhase=oldPhase;_seasonPlaying=oldSeason;
    fprintf(stderr,"[og-uefa-view] BACK unchanged=%u\n",
            (unsigned)(before==(unsigned)ctx->comp.matchesPlayedCount));
    return before==(unsigned)ctx->comp.matchesPlayedCount;
}
int Expanded_OGWorld_ViewLeague(const uint8_t *record,uint16_t division)
{
    static uint8_t unavailable[]="OG WORLD: COMPETITION NOT RESIDENT";
    if(!record || _gameType!=4)return 0;
    const Competition *wanted=(const Competition *)record;
    if(wanted->type==1)return exp_og_view_cup(record);
    if(wanted->type==2 &&
       (World_Current()==WORLD_2627 ||
        (World_Current()==WORLD_9697 &&
         (wanted==(const Competition *)&_euroCup ||
          wanted==Comp_ById(COMPID_copaLibertadores)))))
        return exp_og_view_uefa(wanted);
    ExpandedOGLeague *exact=NULL,*byId=NULL;
    unsigned idMatches=0;
    for(uint32_t i=0;i<s_expOGCount;++i) {
        ExpandedOGLeague *ctx=&s_expOG[i];
        if(!ctx->used || !ctx->teamCount || ctx->division!=division)continue;
        const void *const *table=(const void *const *)_competitionsTable[ctx->country];
        if(!table || !table[0])continue;
        const Competition *native=(const Competition *)table[0];
        if(native==wanted){exact=ctx;break;}
        /* Career competition copies sometimes have a different address.
         * Resolve by OG competition ID ONLY when unambiguous. */
        if(native->compId==wanted->compId){byId=ctx;++idMatches;}
    }
    ExpandedOGLeague *ctx=exact ? exact : (idMatches==1u ? byId : NULL);
    /* The OG viewer resolves every standings team through _selectedTeams.
     * Reject malformed/incomplete scratch images BEFORE entering the OG
     * view menu, so a corrupt context cannot dereference a bad team ID. */
    int valid=(ctx!=NULL && wanted->type==0);
    unsigned n=valid?(unsigned)ctx->comp.base.numTeams:0u;
    if(valid && (ctx->teamCount<2u || ctx->teamCount>EXP_OG_MAX_TEAMS ||
                 n<2u || n>EXP_OG_MAX_TEAMS || n>ctx->teamCount))
        valid=0;
    if(valid) for(unsigned i=0;i<n;i++) {
        const unsigned off=(unsigned)ctx->comp.matchData.league.teamEntryOffsets[i];
        if(off%18u!=0u || off/18u>=n) { valid=0;break; }
        const int32_t team=(int32_t)ctx->comp.leagueStandings[i].teamSelOffset;
        if(team<0 || (uint32_t)team>=ctx->teamCount) {valid=0;break;}
    }
    if(!valid) {
        fprintf(stderr,"[og-world-view] MISSING/INVALID id=%u div=%u "
                       "matches=%u teams=%u n=%u\n",
                (unsigned)wanted->compId,(unsigned)division,idMatches,
                ctx?(unsigned)ctx->teamCount:0u,n);
        ShowErrorMenu(unavailable);
        return 0;
    }
    DIYCompetitionEx oldComp;
    DIYTournamentState oldTournament;
    const uint8_t oldPhase=_viewRandomPhase;
    const uint8_t oldSeason=_seasonPlaying;
    const uint8_t compId=(uint8_t)ctx->comp.base.field_2E;
    const uint8_t oldLast=_careerLastMatchIndexByCompetitionSlot[compId];
    memcpy(&oldComp,&_diyCompetitionStart,sizeof oldComp);
    memcpy(&oldTournament,&_diyFileBufferCopy,sizeof oldTournament);
    SaveSelectedTeams();
    SaveRand2State();
    memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
    memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
    _numSelectedTeams=ctx->teamCount;
    memcpy(_selectedTeams,ctx->teams,(size_t)ctx->teamCount*sizeof(TeamFileEx));
    /* This pointer must belong to the LIVE scratch copy, not the
     * original scratch buffer used while the world was initialized. */
    _diyCompetitionStart.base.namePtr=
        (uint8_t *)_diyCompetitionStart.base.competitionName;
    fprintf(stderr,"[og-world-view] READ ONLY country=%u division=%u "
                   "year=%u phase=%u native-matches=%u\n",
            (unsigned)ctx->country,(unsigned)ctx->division,(unsigned)ctx->year,
            (unsigned)ctx->lastPhase,(unsigned)ctx->comp.matchesPlayedCount);
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=1;
    ShowDiyLeaguesTournamentMenu(1,1,0);
    { extern void InputSDL_ClearWorldViewExit(void);
      InputSDL_ClearWorldViewExit(); }
    s_expOGViewing=0;
    RestoreRand2State();
    RestoreSelectedTeams();
    memcpy(&_diyCompetitionStart,&oldComp,sizeof oldComp);
    memcpy(&_diyFileBufferCopy,&oldTournament,sizeof oldTournament);
    _careerLastMatchIndexByCompetitionSlot[compId]=oldLast;
    _viewRandomPhase=oldPhase;
    _seasonPlaying=oldSeason;
    return 1;
}
/* Offer -> native OG viewer, directly resolved by the authoritative resident
 * country / division / year (the offer modal already knows all three). */
int Expanded_OGWorld_ViewLeagueForCountry(uint8_t country,uint8_t division,
                                          uint16_t year)
{
    const ExpandedOGLeague *ctx=exp_find(country,division,year);
    if(!ctx||!ctx->used) {
        static uint8_t unavailable[]="OG WORLD: COMPETITION NOT RESIDENT";
        fprintf(stderr,"[og-offer-view] MISSING country=%u division=%u year=%u\n",
                (unsigned)country,(unsigned)division,(unsigned)year);
        ShowErrorMenu(unavailable);
        return 0;
    }
    const void *const *table=(const void *const *)_competitionsTable[country];
    if(!table || !table[0] || (uintptr_t)table[0]==(uintptr_t)-1) {
        static uint8_t unavailable[]="OG WORLD: COMPETITION NOT AVAILABLE";
        ShowErrorMenu(unavailable);
        return 0;
    }
    const uint16_t before=ctx->comp.matchesPlayedCount;
    const int ok=Expanded_OGWorld_ViewLeague((const uint8_t *)table[0],division);
    const uint16_t after=ctx->comp.matchesPlayedCount;
    fprintf(stderr,"[og-offer-view] RESIDENT country=%u division=%u year=%u "
                   "before=%u after=%u unchanged=%u ok=%d\n",
            (unsigned)country,(unsigned)division,(unsigned)year,
            (unsigned)before,(unsigned)after,
            (unsigned)(before==after),ok);
    return ok && before==after;
}
/* Bind a manager to the ONE resident OG league. The debug month jump
 * must first validate the same ID mapping WITHOUT writing any career slot:
 * an invalid one must never cause OG to simulate a month that cannot be
 * rebound to the player's menu. */
static int expanded_og_club_image(uint16_t club,uint8_t division,
                                  uint16_t year,DIYCompetitionEx *image,
                                  ExpandedOGLeague **resident)
{
    ExpandedOGLeague *ctx=exp_find(World_FileCountry((uint8_t)(club>>8)),
                                   division,year);
    if(!image || !ctx || !ctx->used ||
       ctx->teamCount<2u || ctx->teamCount>EXP_OG_MAX_TEAMS ||
       _primaryCompetitionBuf.tourn.header.competitionType!=0)return 0;
    memcpy(image,&ctx->comp,sizeof(*image));
    const unsigned n=(unsigned)image->base.numTeams;
    if(n<2u || n>EXP_OG_MAX_TEAMS || n>ctx->teamCount)return 0;
    for(unsigned i=0;i<n;++i) {
        const int32_t id=image->leagueStandings[i].teamSelOffset;
        if(id<0 || (uint32_t)id>=ctx->teamCount)return 0;
        const TeamFile *original=&ctx->teams[id].team;
        const uint16_t word=PACK_BE_U16(original->countryNumber,
                                        original->teamOrdinal);
        int32_t mapped=-2;
        if(word==club)mapped=-1;
        else for(uint16_t j=0;j<_numSelectedTeams;++j) {
            const TeamFile *local=&_selectedTeams[j].team;
            if(local->countryNumber==original->countryNumber &&
               local->teamOrdinal==original->teamOrdinal) {
                mapped=(int32_t)j;break;
            }
        }
        if(mapped==-2)return 0;
        image->leagueStandings[i].teamSelOffset=mapped;
    }
    if(resident)*resident=ctx;
    return 1;
}

/* The manager's league runs in the original OG local context. The resident
 * copy receives ONLY fully validated results, with all signed local
 * team-selection indices mapped back to stable original OG team indices. */
typedef struct {
    uint16_t native_ids[EXP_OG_BRIDGE_MAX_ROWS];
    uint16_t selected_ids[SELECTED_TEAMS_MAX];
    uint16_t native_for_row[EXP_OG_BRIDGE_MAX_ROWS];
    uint16_t native_row[EXP_OG_BRIDGE_MAX_ROWS];
    int32_t local_offsets[EXP_OG_BRIDGE_MAX_ROWS];
    int32_t native_offsets[EXP_OG_BRIDGE_MAX_ROWS];
    ExpandedOGLeagueTotals before[EXP_OG_BRIDGE_MAX_ROWS];
    ExpandedOGLeagueTotals after[EXP_OG_BRIDGE_MAX_ROWS];
} ExpandedOGBridgeScratch;
static uint16_t exp_bridge_club(const TeamFileEx *t)
{
    return PACK_BE_U16(t->team.countryNumber,t->team.teamOrdinal);
}
static ExpandedOGLeagueTotals exp_bridge_totals(const DIYTeamStats *r)
{
    ExpandedOGLeagueTotals v;
    v.played=r->gamesPlayed;v.wins=r->wins;v.draws=r->draws;
    v.losses=r->losses;v.for_goals=r->goalsFor;
    v.against_goals=r->goalsAgainst;v.points=r->points;
    return v;
}
static int exp_bridge_totals_equal(const ExpandedOGLeagueTotals *a,
                                    const ExpandedOGLeagueTotals *b)
{
    return a->played==b->played && a->wins==b->wins &&
        a->draws==b->draws && a->losses==b->losses &&
        a->for_goals==b->for_goals &&
        a->against_goals==b->against_goals && a->points==b->points;
}
static int exp_bridge_local_active(const DIYCompetitionEx *local)
{
    if(!local || !s_expOGBound || _gameType!=4 || _youreSacked ||
       _selTeamNumber==0xffffu || s_expOGCurrent || s_expOGCupCurrent ||
       s_expOGViewing || local->base.competitionType!=0 ||
       local->base.numTeams<2 || local->base.numTeams>EXP_OG_MAX_TEAMS)
        return 0;
    /* Prevent an unrelated type-0 cup/group from masquerading as the
     * national league. Manager identity is exactly one -1 local row. */
    if(local->base.field_2E !=
       _primaryCompetitionBuf.tourn.header.field_2E)return 0;
    unsigned managers=0u;
    for(unsigned i=0;i<(unsigned)local->base.numTeams;++i)
        if(local->leagueStandings[i].teamSelOffset==-1)++managers;
    return managers==1u;
}
/* Pure lookup of split metadata. LeagueSplit_Count is MUTATING and must
 * never be used as a preflight predicate (it consumes the current day). */
static int exp_bridge_has_split(void)
{
    int32_t count=0;
    const LeagueSplit *defs=World_LeagueSplits(&count);
    if(count<0 || count>1024 || (count && !defs))return 1;
    const uint8_t country=World_FileCountry((uint8_t)(_selTeamNumber>>8));
    for(int32_t i=0;i<count;i++)
        if(defs[i].country==country &&
           defs[i].division==(uint8_t)_currentDivisionIndex)return 1;
    return 0;
}
static ExpandedOGLeague *exp_bridge_resident(const DIYCompetitionEx *local)
{
    ExpandedLiveCalendarState cal;
    if(!exp_bridge_local_active(local) ||
       !ExpandedWorldLive_CalendarState(&cal) ||
       cal.today.year<(cal.today.month<7u?1u:0u))return NULL;
    const uint16_t year=(uint16_t)(
        cal.today.year-(cal.today.month<7u?1u:0u));
    ExpandedOGLeague *ctx=exp_find(
        World_FileCountry((uint8_t)(_selTeamNumber>>8)),
        (uint8_t)_currentDivisionIndex,year);
    if(!ctx || !ctx->used ||
       ctx->comp.base.field_2E!=local->base.field_2E ||
       ctx->comp.base.competitionType!=0 ||
       /* Group/split stage indices require a distinct verified bridge.
        * Never copy a tournament seed/order using league-only offsets. */
       ctx->comp.base.hasGroups!=0 || local->base.hasGroups!=0 ||
       exp_bridge_has_split() ||
       ctx->comp.base.numTeams!=local->base.numTeams ||
       ctx->teamCount!=(uint16_t)local->base.numTeams ||
       ctx->comp.pointsForAWin!=local->pointsForAWin)return NULL;
    return ctx;
}
static int exp_bridge_rows(const ExpandedOGLeague *ctx,
                            const DIYCompetitionEx *local,
                            ExpandedOGBridgeScratch *b)
{
    if(!ctx || !local || !b || _numSelectedTeams>SELECTED_TEAMS_MAX)
        return 0;
    const unsigned n=(unsigned)local->base.numTeams;
    if(n<2u || n>EXP_OG_BRIDGE_MAX_ROWS || n!=ctx->teamCount)return 0;
    for(unsigned i=0;i<n;++i) {
        b->native_ids[i]=exp_bridge_club(&ctx->teams[i]);
        b->native_offsets[i]=ctx->comp.leagueStandings[i].teamSelOffset;
        b->local_offsets[i]=local->leagueStandings[i].teamSelOffset;
    }
    for(unsigned i=0;i<(unsigned)_numSelectedTeams;++i)
        b->selected_ids[i]=exp_bridge_club(&_selectedTeams[i]);
    if(!ExpandedOGLeagueBridge_Map(b->native_ids,n,b->native_offsets,
             b->selected_ids,(unsigned)_numSelectedTeams,
             _selTeamNumber,b->local_offsets,n,
             b->native_for_row,b->native_row))return 0;
    for(unsigned i=0;i<n;++i) {
        const unsigned original=b->native_for_row[i];
        /* Both DIYCompetitionEx and its relocated standings have OG
         * packed ancestry. Copy into aligned locals before taking a
         * DIYTeamStats pointer (no -Waddress-of-packed-member). */
        DIYTeamStats world_row,local_row;
        memcpy(&world_row,(const uint8_t *)&ctx->comp+
               offsetof(DIYCompetitionEx,leagueStandings)+
               (size_t)b->native_row[i]*sizeof world_row,sizeof world_row);
        memcpy(&local_row,(const uint8_t *)local+
               offsetof(DIYCompetitionEx,leagueStandings)+
               (size_t)i*sizeof local_row,sizeof local_row);
        b->before[original]=exp_bridge_totals(&world_row);
        b->after[i]=exp_bridge_totals(&local_row);
    }
    return 1;
}
static int exp_bridge_equal(const ExpandedOGBridgeScratch *b,unsigned n)
{
    for(unsigned i=0;i<n;++i)
        if(!exp_bridge_totals_equal(&b->before[b->native_for_row[i]],
                                   &b->after[i]))return 0;
    return 1;
}
static void exp_bridge_fault(const char *reason,
                              const DIYCompetitionEx *local,
                              const ExpandedOGLeague *ctx)
{
    /* Fail closed: never corrupt an existing CAR or silently replay. */
    s_expOGBridgeBlocked=1;
    ++s_expOGBridgeRejections;
    if(s_expOGBridgeRejections==1u) {
        ++s_expOGFixtureIssues;
        fprintf(stderr,"[og-manager-bridge] WRITE_BLOCKED world=%s club=%04x "
            "reason=%s local-matches=%u resident-matches=%u "
            "no-replay=1 prior-CAR-preserved=1 loop-abort=1\n",
            World_Tag(),(unsigned)_selTeamNumber,reason,
            local?(unsigned)local->matchesPlayedCount:0u,
            ctx?(unsigned)ctx->comp.matchesPlayedCount:0u);
    }
}
/* Count the REAL OG league fixtures, not a menu phase or cached date.
 * A 24-team two-leg league finishes after exactly 552 games. Manager hire
 * must never cause the previous season's exhausted native fixture iterator
 * to be invoked for game 553. Fail closed if the resident is unavailable. */
int Expanded_OGWorld_ManagerLeagueFinished(const DIYCompetitionEx *local)
{
    if(!s_expOGBound || !exp_bridge_local_active(local))return 0;
    ExpandedOGLeague *ctx=exp_bridge_resident(local);
    return ctx &&
        local->base.playEachTeam==ctx->comp.base.playEachTeam &&
        ExpandedOGLeagueBridge_SeasonComplete(
            (unsigned)local->base.numTeams,
            (unsigned)local->base.playEachTeam,
            (unsigned)local->matchesPlayedCount,
            (unsigned)ctx->comp.matchesPlayedCount);
}
int Expanded_OGWorld_PreflightLocalLeague(const DIYCompetitionEx *local)
{
    if(!exp_bridge_local_active(local))return 1;
    ExpandedOGLeague *ctx=exp_bridge_resident(local);
    if(!ctx){
        exp_bridge_fault("MISSING_OR_INCOMPATIBLE_RESIDENT",local,NULL);
        return 0;
    }
    ExpandedOGBridgeScratch b={0};
    if(ctx->comp.matchesPlayedCount!=local->matchesPlayedCount ||
       !exp_bridge_rows(ctx,local,&b) ||
       !exp_bridge_equal(&b,(unsigned)local->base.numTeams)){
        exp_bridge_fault("STALE_OR_DIVERGENT_LOCAL_BASE",local,ctx);
        return 0;
    }
    return 1;
}
int Expanded_OGWorld_CommitLocalLeagueFixture(
    const TeamFileEx *home,const TeamFileEx *away,
    const DIYCompetitionEx *local,uint16_t gh,uint16_t ga,uint8_t phase)
{
    if(!exp_bridge_local_active(local))return 1;
    ExpandedOGLeague *ctx=exp_bridge_resident(local);
    if(!ctx || !home || !away){
        exp_bridge_fault("MISSING_RESIDENT_AFTER_RESULT",local,ctx);
        return 0;
    }
    ExpandedOGBridgeScratch b={0};
    const unsigned n=(unsigned)local->base.numTeams;
    if(ctx->comp.matchesPlayedCount==UINT16_MAX ||
       local->matchesPlayedCount!=ctx->comp.matchesPlayedCount+1u ||
       !exp_bridge_rows(ctx,local,&b)){
        exp_bridge_fault("SERIAL_OR_TEAM_MAP_DIVERGED",local,ctx);
        return 0;
    }
    const uint16_t home_id=exp_bridge_club(home);
    const uint16_t away_id=exp_bridge_club(away);
    unsigned hi=n,ai=n;
    for(unsigned i=0;i<n;++i){
        if(b.native_ids[i]==home_id)hi=i;
        if(b.native_ids[i]==away_id)ai=i;
    }
    if(hi>=n || ai>=n ||
       !ExpandedOGLeagueBridge_CheckStep(b.before,b.after,
             b.native_for_row,n,hi,ai,gh,ga,
             (unsigned)ctx->comp.pointsForAWin)){
        exp_bridge_fault("SCORE_OR_TABLE_DELTA_INVALID",local,ctx);
        return 0;
    }
    DIYCompetitionEx image;
    TeamFileEx teams[EXP_OG_MAX_TEAMS];
    memcpy(&image,local,sizeof image);
    memcpy(teams,ctx->teams,(size_t)n*sizeof(TeamFileEx));
    for(unsigned i=0;i<n;++i){
        const unsigned original=b.native_for_row[i];
        const int32_t local_index=b.local_offsets[i];
        const TeamFileEx *source=local_index==-1?&_careerTeam:
                                       &_selectedTeams[local_index];
        if(exp_bridge_club(source)!=b.native_ids[original]){
            exp_bridge_fault("TEAM_FILE_ID_CHANGED",local,ctx);
            return 0;
        }
        image.leagueStandings[i].teamSelOffset=(int32_t)original;
        teams[original]=*source;
        /* Human-only team control cannot enter the CPU resident roster. */
        teams[original].team.teamControls=
            ctx->teams[original].team.teamControls;
    }
    /* Commit as one state transfer, never by invoking OG match routines.
     * The existing CAR resident serializer includes competition, native
     * tournament state, teams and RNG; the witness is in that SAME CAR. */
    memcpy(&ctx->comp,&image,sizeof image);
    ctx->comp.base.namePtr=(uint8_t *)ctx->comp.base.competitionName;
    /* Standard OG league games do not modify diyFileBufferCopy
     * (DIY_SyncProgressToDiyFileBufferCopy returns on hasGroups==0).
     * Preserve the already-native tournament indices/seed orders instead
     * of importing manager-local team-index order without a second map. */
    memcpy(ctx->teams,teams,(size_t)n*sizeof(TeamFileEx));
    ctx->seed=_seed2;ctx->randIndex=_randXorIndex2;ctx->randKey=_randXorKey2;
    ctx->lastMatch=_careerLastMatchIndexByCompetitionSlot[local->base.field_2E];
    if(phase>ctx->lastPhase)ctx->lastPhase=phase;
    if(s_expOGTickActive)++s_expOGCapturedLeague;
    fprintf(stderr,"[og-manager-bridge] COMMITTED world=%s country=%u "
        "division=%u year=%u phase=%u score=%u-%u "
        "native-matches=%u remapped=%u journal=OBSERVED "
        "source=LOCAL_OG_NO_SECOND_SIM\n",
        World_Tag(),(unsigned)ctx->country,(unsigned)ctx->division,
        (unsigned)ctx->year,(unsigned)phase,(unsigned)gh,(unsigned)ga,
        (unsigned)ctx->comp.matchesPlayedCount,n);
    return 1;
}

int Expanded_OGWorld_DebugCanAdvanceClub(uint16_t club,uint8_t division,
                                         uint16_t year)
{
    /* Never rewrite matches the manager has played locally but which
     * have not entered the resident OG context. These require a proper
     * match-by-match synchronization, NOT an automatic VIEW RESULT. */
    DIYCompetitionEx image;
    ExpandedOGLeague *resident=NULL;
    if(!s_expOGBound ||
       !expanded_og_club_image(club,division,year,&image,&resident))return 0;
    /* Type-0 club slots are copied from byte ZERO of the buffer
     * (Career_StoreCompetitionSlotStateToBuffer), NOT stageSnapshot:
     * the latter is the type-2 tournament-only trailing storage. */
    const DIYCompetitionEx *local=
        (const DIYCompetitionEx *)(const void *)&_primaryCompetitionBuf;
    if(local->matchesPlayedCount!=resident->comp.matchesPlayedCount ||
       memcmp(local->leagueStandings,image.leagueStandings,
              sizeof image.leagueStandings)!=0) {
        fprintf(stderr,"[debug-month-career] BLOCKED unsynced OG club league "
                "local-matches=%u resident-matches=%u "
                "standings=DIFFERENT no-overwrite\n",
                (unsigned)local->matchesPlayedCount,
                (unsigned)resident->comp.matchesPlayedCount);
        return 0;
    }
    return 1;
}

/* Observe genuine OG result after the normal local->resident commit.
 * The keycache is not a universal journal. All checks are read-only EXCEPT
 * the narrow verified correction of 4 ID bytes in a played cache row.
 * OG results/standings/RNG remain immutable and failures stay logged. */
void Expanded_OGWorld_AuditManagerLeagueResult(const TeamFileEx *home,
    const TeamFileEx *away,const DIYCompetitionEx *local,
    uint16_t goals_home,uint16_t goals_away,uint8_t native_phase)
{
    if(!home || !away || !local || _gameType!=4 || !s_expOGBound ||
       s_expOGCurrent || s_expOGCupCurrent || s_expOGViewing ||
       _selTeamNumber==0xffffu || local->base.competitionType!=0)return;
    const uint16_t home_id=PACK_BE_U16(home->team.countryNumber,
                                        home->team.teamOrdinal);
    const uint16_t away_id=PACK_BE_U16(away->team.countryNumber,
                                        away->team.teamOrdinal);
    if(home_id!=_selTeamNumber && away_id!=_selTeamNumber)return;
    ExpandedLiveCalendarState cal;
    if(!ExpandedWorldLive_CalendarState(&cal) ||
       cal.today.year<(cal.today.month<7u?1u:0u))return;
    const uint16_t year=(uint16_t)(cal.today.year-
                                    (cal.today.month<7u?1u:0u));
    ExpandedOGLeague *ctx=exp_find(World_FileCountry((uint8_t)
                                   (_selTeamNumber>>8)),
                                   (uint8_t)_currentDivisionIndex,year);
    if(!ctx || !ctx->used) {
        fprintf(stderr,"[og-manager-fixture] status=NO_RESIDENT world=%s "
                       "club=%04x home=%04x away=%04x year=%u\n",
                World_Tag(),(unsigned)_selTeamNumber,
                (unsigned)home_id,(unsigned)away_id,(unsigned)year);
        return;
    }
    extern uint8_t _teamSelectionKeyCache[8192];
    extern uint16_t _teamSelectionKeyCacheOffset;
    extern uint16_t _teamSelectionKeyCacheCount;
    if((unsigned)_teamSelectionKeyCacheOffset>
       8192u-sizeof(TeamSelectionKeyCacheEntry)) {
        fprintf(stderr,"[og-manager-fixture] status=INVALID_CACHE_OFFSET "
                       "offset=%u no-world-mutation=1\n",
                (unsigned)_teamSelectionKeyCacheOffset);
        return;
    }
    const TeamSelectionKeyCacheEntry *active=
        (const TeamSelectionKeyCacheEntry *)(const void *)
        (_teamSelectionKeyCache+_teamSelectionKeyCacheOffset);
    const uint8_t slot=active->compSlot,phase=active->sortKey;
    ExpandedOGCacheMatch match;
    const ExpandedOGCacheStatus cache=ExpandedOGCache_Find(
        _teamSelectionKeyCache,8192u,_teamSelectionKeyCacheCount,
        slot,phase,home_id,away_id,&match);
    const uint16_t packed=(uint16_t)((goals_home<<8)|
                                      (goals_away&255u));
    const int cached_score_ok=
        cache==EXP_OG_CACHE_PLAYED && match.packed_goals==packed;
    /* Some native calendar rows are oriented opposite to the actual
     * home/away callback. Diagnose that separately; swapping identities
     * is NOT a verified fixture bind and must never produce a false PASS. */
    ExpandedOGCacheMatch reverse_match;
    const ExpandedOGCacheStatus reverse_cache=ExpandedOGCache_Find(
        _teamSelectionKeyCache,8192u,_teamSelectionKeyCacheCount,
        slot,phase,away_id,home_id,&reverse_match);
    const uint16_t mirrored=(uint16_t)((goals_away<<8)|
                                        (goals_home&255u));
    const int reverse_score_ok=
        reverse_cache==EXP_OG_CACHE_PLAYED &&
        reverse_match.packed_goals==mirrored;
    /* The keycache can contain an ACTUAL PLAYED score for a DIFFERENT
     * home/away pair at the same week. That is NOT a missing result
     * and it must not be called reconciled because the points tables
     * happen to agree. Diagnose the calendar identity separately. */
    ExpandedOGCacheMatch phase_match={0};
    const ExpandedOGCacheStatus phase_cache=ExpandedOGCache_FindPhaseSlot(
        _teamSelectionKeyCache,8192u,_teamSelectionKeyCacheCount,
        slot,phase,&phase_match);
    const int phase_has_pair=
        phase_cache==EXP_OG_CACHE_PLAYED ||
        phase_cache==EXP_OG_CACHE_SCHEDULED;
    const int phase_identity_matches=phase_has_pair &&
        phase_match.home==home_id && phase_match.away==away_id;
    const int phase_other_pair=phase_has_pair && !phase_identity_matches;
    const unsigned world_n=(unsigned)ctx->comp.base.numTeams;
    const unsigned local_n=(unsigned)local->base.numTeams;
    unsigned unmapped=0u,table_differences=0u;
    if(ctx->teamCount<2u || ctx->teamCount>EXP_OG_MAX_TEAMS ||
       world_n!=local_n || world_n<2u || world_n>EXP_OG_MAX_TEAMS) {
        unmapped=1u;
    } else {
        /* The row indices/offsets are scratch identities, NOT club IDs.
         * Compare club by club, ignoring rank and the local -1 manager
         * marker, so a reordering cannot look like a different world. */
        for(unsigned i=0;i<world_n;++i) {
            const int32_t wi=ctx->comp.leagueStandings[i].teamSelOffset;
            if(wi<0 || (unsigned)wi>=ctx->teamCount) {
                ++unmapped;continue;
            }
            const uint16_t club=PACK_BE_U16(
                ctx->teams[wi].team.countryNumber,
                ctx->teams[wi].team.teamOrdinal);
            unsigned found=0u;
            for(unsigned j=0;j<local_n;++j) {
                const int32_t li=local->leagueStandings[j].teamSelOffset;
                uint16_t local_club=0xffffu;
                if(li==-1)local_club=_selTeamNumber;
                else if(li>=0 && (unsigned)li<_numSelectedTeams)
                    local_club=PACK_BE_U16(
                        _selectedTeams[li].team.countryNumber,
                        _selectedTeams[li].team.teamOrdinal);
                if(local_club!=club)continue;
                ++found;
                if(ctx->comp.leagueStandings[i].wins!=
                        local->leagueStandings[j].wins ||
                   ctx->comp.leagueStandings[i].draws!=
                        local->leagueStandings[j].draws ||
                   ctx->comp.leagueStandings[i].losses!=
                        local->leagueStandings[j].losses ||
                   ctx->comp.leagueStandings[i].goalsFor!=
                        local->leagueStandings[j].goalsFor ||
                   ctx->comp.leagueStandings[i].goalsAgainst!=
                        local->leagueStandings[j].goalsAgainst ||
                   ctx->comp.leagueStandings[i].points!=
                        local->leagueStandings[j].points)
                    ++table_differences;
            }
            if(found!=1u)++unmapped;
        }
    }
    /* The OG result has been committed to its resident context. Only now
     * reconcile a *unique already-played* calendar entry that the OLD club
     * fixture keycache associated with a different opponent. Identity is
     * from the exact native callback, phase, score and the same manager.
     * Never rewrite score/flags or accept a phase-only/score-only match. */
    int pair_rebound=0;
    ++s_expOGManagerPairChecked;
    if(phase_other_pair) {
        ExpandedOGCacheMatch previous={0};
        const int can_rebind=!s_expOGBridgeBlocked &&
            !unmapped && !table_differences &&
            ctx->comp.matchesPlayedCount==local->matchesPlayedCount &&
            active->phaseFlag==0u && slot==local->base.field_2E &&
            native_phase==phase &&
            phase_cache==EXP_OG_CACHE_PLAYED &&
            phase_match.packed_goals==packed &&
            cache==EXP_OG_CACHE_NOT_FOUND;
        const int rebound=can_rebind?ExpandedOGCache_RebindPlayedIdentity(
            _teamSelectionKeyCache,8192u,_teamSelectionKeyCacheCount,
            slot,phase,_selTeamNumber,home_id,away_id,packed,
            &previous):-1;
        if(rebound==1) {
            ++s_expOGManagerPairRepaired;pair_rebound=1;
            fprintf(stderr,"[og-manager-calendar] REBOUND_EXACT_OG "
                    "world=%s year=%u native-phase=%u slot=%u "
                    "old=%04x-%04x new=%04x-%04x score=%u-%u "
                    "identity-only=1 scores-unchanged=1 "
                    "resident-OG-untouched=1\n",
                    World_Tag(),(unsigned)year,(unsigned)phase,(unsigned)slot,
                    (unsigned)previous.home,(unsigned)previous.away,
                    (unsigned)home_id,(unsigned)away_id,
                    (unsigned)goals_home,(unsigned)goals_away);
        } else {
            ++s_expOGManagerPairMismatch;
            fprintf(stderr,"[og-manager-calendar] BLOCK_PAIR_RECONCILIATION "
                    "world=%s year=%u slot=%u native-phase=%u "
                    "cache-phase=%u home=%04x away=%04x "
                    "score=%u-%u reason=NOT_PROVEN_EXACT "
                    "scores-unchanged=1\n",
                    World_Tag(),(unsigned)year,(unsigned)slot,
                    (unsigned)native_phase,(unsigned)phase,
                    (unsigned)home_id,(unsigned)away_id,
                    (unsigned)goals_home,(unsigned)goals_away);
        }
    }
    const char *cache_name=
        cache==EXP_OG_CACHE_PLAYED?"PLAYED":
        cache==EXP_OG_CACHE_SCHEDULED?"SCHEDULED":
        cache==EXP_OG_CACHE_AMBIGUOUS?"AMBIGUOUS":
        cache==EXP_OG_CACHE_CORRUPT?"CORRUPT":"NOT_FOUND";
    const unsigned resident_matches=(unsigned)ctx->comp.matchesPlayedCount;
    const unsigned local_matches=(unsigned)local->matchesPlayedCount;
    const char *result=
        unmapped?"CLUB_MAP_INVALID":
        table_differences?"TABLES_DIVERGED":
        resident_matches!=local_matches?"COUNTERS_DIVERGED":
        phase_other_pair?(pair_rebound?"CALENDAR_PAIR_REBOUND_EXACT_OG":
                                       "CALENDAR_PAIR_MISMATCH"):
        phase_cache==EXP_OG_CACHE_CORRUPT?"CALENDAR_CACHE_CORRUPT":
        phase_cache==EXP_OG_CACHE_AMBIGUOUS?"CALENDAR_PHASE_AMBIGUOUS":
        phase_cache==EXP_OG_CACHE_NOT_FOUND?"CALENDAR_PHASE_MISSING":
        phase_cache==EXP_OG_CACHE_SCHEDULED?"CALENDAR_RESULT_NOT_PLAYED":
        !cached_score_ok?"CALENDAR_SCORE_MISMATCH":"RESIDENT_AND_CACHE_MATCH";
    const char *phase_name=
        phase_cache==EXP_OG_CACHE_PLAYED?"PLAYED":
        phase_cache==EXP_OG_CACHE_SCHEDULED?"SCHEDULED":
        phase_cache==EXP_OG_CACHE_AMBIGUOUS?"AMBIGUOUS":
        phase_cache==EXP_OG_CACHE_CORRUPT?"CORRUPT":"NOT_FOUND";
    fprintf(stderr,
        "[og-manager-fixture] world=%s year=%u div=%u "
        "home=%04x away=%04x score=%u-%u cache=%s "
        "cache-score=%s reverse-cache=%s slot=%u phase=%u "
        "calendar-phase=%s calendar-home=%04x calendar-away=%04x "
        "calendar-score=%u-%u calendar-flags=%02x "
        "world-matches=%u local-matches=%u row-diff=%u unmapped=%u "
        "cache-used=%lu match-end=%lu car-raw-limit=8192 "
        "car-capacity-risk=%s status=%s cache-id-rebound=%u "
        "no-OG-score-mutation=1\n",
        World_Tag(),(unsigned)year,(unsigned)_currentDivisionIndex,
        (unsigned)home_id,(unsigned)away_id,
        (unsigned)goals_home,(unsigned)goals_away,cache_name,
        cached_score_ok?"MATCH":"UNVERIFIED",
        reverse_score_ok?"PLAYED_MIRRORED":
            reverse_cache==EXP_OG_CACHE_AMBIGUOUS?"AMBIGUOUS":
            reverse_cache==EXP_OG_CACHE_CORRUPT?"CORRUPT":
            reverse_cache==EXP_OG_CACHE_PLAYED?"PLAYED_SCORE_DIFF":
            reverse_cache==EXP_OG_CACHE_SCHEDULED?"SCHEDULED":"NOT_FOUND",
        (unsigned)slot,(unsigned)phase,phase_name,
        phase_has_pair?(unsigned)phase_match.home:0xffffu,
        phase_has_pair?(unsigned)phase_match.away:0xffffu,
        phase_has_pair?(unsigned)(phase_match.packed_goals>>8):0u,
        phase_has_pair?(unsigned)(phase_match.packed_goals&255u):0u,
        phase_has_pair?(unsigned)phase_match.flags:0u,
        resident_matches,local_matches,table_differences,unmapped,
        (unsigned long)match.used_bytes,(unsigned long)match.record_end,
        match.used_bytes>EXP_OG_CACHE_CAPACITY ||
        match.record_end>EXP_OG_CACHE_CAPACITY?"YES":"NO",
        result,(unsigned)pair_rebound);
}
/* Called ONLY AFTER OG has calculated and applied the result. This
 * records the canonical OG score as EVIDENCE, never rolls another score.
 * The native serial disambiguates repeated oriented pairs in 3/4-cycle
 * leagues; dates are upper bounds from the civil cursor, not fabricated. */
int Expanded_OGWorld_ObserveFixture(int cup,const TeamFileEx *home,
    const TeamFileEx *away,const DIYCompetitionEx *local,
    uint16_t goalsHome,uint16_t goalsAway,uint16_t serial,
    uint8_t phase,uint8_t leg)
{
    if(_gameType!=4 || !home || !away || !local ||
       goalsHome>255u || goalsAway>255u)return 0;
    const uint16_t h=PACK_BE_U16(home->team.countryNumber,
                                 home->team.teamOrdinal);
    const uint16_t a=PACK_BE_U16(away->team.countryNumber,
                                 away->team.teamOrdinal);
    if(h==a)return 0;
    uint8_t kind=0u,country=0u,slot=0u,origin=0u;
    uint16_t edition=0u;
    ExpandedWorldDate stamp=s_expOGFixtureAsOf;
    if(s_expOGCupCurrent) {
        kind=1u;origin=1u;country=s_expOGCupCurrent->country;
        slot=s_expOGCupCurrent->slot;edition=s_expOGCupCurrent->year;
    } else if(s_expOGCurrent) {
        kind=exp_og_is_uefa()?2u:0u;origin=1u;
        country=s_expOGCurrent->country;
        slot=s_expOGCurrent->division;
        edition=s_expOGCurrent->year;
    } else {
        if(_selTeamNumber==0xffffu)return 0;
        const int manager=(h==_selTeamNumber || a==_selTeamNumber);
        /* The same local OG league also plays CPU fixtures between the
         * manager's games. Include ALL of those results in this journal. */
        const int bound_local=!cup && exp_bridge_local_active(local);
        if(!manager && !bound_local)return 0;
        origin=manager?2u:3u; /* source 3: CPU game in local OG league */
        kind=cup?3u:0u;
        country=World_FileCountry((uint8_t)(_selTeamNumber>>8));
        slot=cup?(uint8_t)local->base.field_2E:
                 (uint8_t)_currentDivisionIndex;
        ExpandedLiveCalendarState cal;
        if(!ExpandedWorldLive_CalendarState(&cal))return 0;
        stamp=cal.today;
        edition=(uint16_t)(stamp.year-(stamp.month<7u?1u:0u));
    }
    int32_t day=0;
    if(!edition || !ExpandedWorld_DateToIndex(stamp,&day) || day<0)
        return 0;
    uint8_t record[EXP_OG_FIXTURE_SIZE]={0};
    record[0]=kind;record[1]=country;record[2]=slot;
    record[3]=phase;
    exp_fixture_put16(record+4,edition);
    exp_fixture_put16(record+6,serial);
    exp_fixture_put16(record+8,h);
    exp_fixture_put16(record+10,a);
    record[12]=(uint8_t)goalsHome;record[13]=(uint8_t)goalsAway;
    record[14]=leg;record[15]=origin;
    exp_fixture_put32(record+16,(uint32_t)day);
    if(!exp_fixture_append(record))return 0;
    if(origin==1u) {
        if(kind==0u)++s_expOGCapturedLeague;
        else if(kind==1u)++s_expOGCapturedCup;
        else ++s_expOGCapturedUefa;
    } else if(!cup) {
        /* Manager and local CPU share ONE OG league executor. The resident
         * snapshot is published by CommitLocalLeagueFixture AFTER this
         * witness, never through another CPU simulation. An older resident
         * match with the same oriented pair in a DIFFERENT native phase is
         * a legitimate second/third/fourth meeting, not divergence. */
        const ExpandedOGFixtureHistory history=
            ExpandedOGFixtureHistory_Check(s_expOGFixtures,
                                           s_expOGFixtureCount,record);
        if(history.resident_same_phase) {
            ++s_expOGFixtureIssues;
            fprintf(stderr,"[og-manager-world-fixture] "
                "RESIDENT_SAME_PHASE_COLLISION_BLOCK_NEW_CAR "
                "world=%s edition=%u country=%u div=%u "
                "home=%04x away=%04x phase=%u previous=%u "
                "score-conflicts=%u no-replay=1\n",
                World_Tag(),(unsigned)edition,(unsigned)country,
                (unsigned)slot,(unsigned)h,(unsigned)a,
                (unsigned)phase,(unsigned)history.resident_same_phase,
                (unsigned)history.resident_same_phase_score_conflict);
        }
        fprintf(stderr,"[og-manager-world-fixture] world=%s "
            "edition=%u country=%u div=%u home=%04x away=%04x "
            "phase=%u serial=%u score=%u-%u source=%s "
            "historic-pair-other-phase=%u resident-same-phase=%u "
            "status=%s\n",
            World_Tag(),(unsigned)edition,(unsigned)country,
            (unsigned)slot,(unsigned)h,(unsigned)a,(unsigned)phase,
            (unsigned)serial,(unsigned)record[12],(unsigned)record[13],
            origin==3u?"LOCAL_CPU_OG":"LOCAL_MANAGER_OG",
            (unsigned)history.historical_other_phase,
            (unsigned)history.resident_same_phase,
            history.resident_same_phase?
                "BLOCK_NEW_CAR_RESIDENT_PHASE_COLLISION":
            history.historical_other_phase?
                "LOCAL_OG_WITNESSED_PREVIOUS_CYCLE":
                "LOCAL_OG_WITNESSED_PENDING_RESIDENT_BRIDGE");
    }
    return 1;
}
int Expanded_OGWorld_BindClub(uint16_t club,uint8_t division,uint16_t year) {
    DIYCompetitionEx image;
    ExpandedOGLeague *ctx=NULL;
    if(!expanded_og_club_image(club,division,year,&image,&ctx)) {
        s_expOGBound=0;
        return 0;
    }
    image.base.namePtr=
        (uint8_t *)_primaryCompetitionBuf.tourn.header.competitionName;
    memcpy(&_diyCompetitionStart,&image,sizeof image);
    Career_StoreCompetitionSlotStateToBuffer((uint8_t *)&_primaryCompetitionBuf);
    _careerLastMatchIndexByCompetitionSlot[image.base.field_2E]=ctx->lastMatch;
    s_expOGBound=1;
    s_expOGBridgeBlocked=0;s_expOGBridgeRejections=0u;
    fprintf(stderr,"[og-bind] club=%04x division=%u year=%u matches=%u "
                   "NO HISTORICAL SIMULATION\n",
            (unsigned)club,(unsigned)division,(unsigned)year,
            (unsigned)ctx->comp.matchesPlayedCount);
    return 1;
}
#endif

/* Career_AutoAdvanceInternationalCompetitionFixtures (SWOS2.s line 35208)
 * Auto-advances an international competition: initializes team selection from
 * table entry spec, loads competition from table entry, runs league fixture
 * advancement, then restores RNG state.
 * a0 = competition record pointer, d0 = table index. */
__attribute__((used, externally_visible))
void Career_AutoAdvanceInternationalCompetitionFixtures(uint8_t *record, uint16_t index)
{
    _careerLoadedFlag = 1;
    if (_seasonUpdateInProgress == 0 && _internationalAutoAdvanceFlag == 0)
        _teamSelectionKeyCacheEnd = 1;
    _teamsLoaded = 0;
    SaveRand2State();
    uint8_t savedViewRandom = _viewRandomPhase;
    uint8_t savedSeasonPlaying = _seasonPlaying;

    uint16_t seed = ((uint16_t)record[0] << 8) | (uint16_t)_seasonPlaying;
    Randomize2(seed);
    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromTableEntrySpec(record, index);
    Career_ApplyPendingForeignMarketPlayersToSelectedTeams();
    InitDIYCompetitionFromTableEntry(index, record);
    InitTeams();
#ifdef SWOS_SDL_BUILD
    if(s_expOGCurrent) {
        ExpandedOGLeague *ctx=s_expOGCurrent;
        const uint8_t compId=(uint8_t)((Competition *)record)->compId;
        if(ctx->used) {
            memcpy(&_diyCompetitionStart,&ctx->comp,sizeof ctx->comp);
            memcpy(&_diyFileBufferCopy,&ctx->tournament,sizeof ctx->tournament);
            _numSelectedTeams=ctx->teamCount;
            memcpy(_selectedTeams,ctx->teams,
                   (size_t)ctx->teamCount*sizeof(TeamFileEx));
            _seed2=ctx->seed;_randXorIndex2=ctx->randIndex;
            _randXorKey2=ctx->randKey;
        }
        _careerLastMatchIndexByCompetitionSlot[compId]=ctx->lastMatch;
        if(s_expOGPhase>0)Career_AutoAdvanceLeagueFixtures();
        if(!s_expOGFault) { /* do not persist an invalid OG fixture */
        ctx->lastMatch=_careerLastMatchIndexByCompetitionSlot[compId];
        memcpy(&ctx->comp,&_diyCompetitionStart,sizeof ctx->comp);
        memcpy(&ctx->tournament,&_diyFileBufferCopy,sizeof ctx->tournament);
        ctx->teamCount=_numSelectedTeams;
        if(ctx->teamCount>EXP_OG_MAX_TEAMS)ctx->teamCount=0;
        if(ctx->teamCount)
            memcpy(ctx->teams,_selectedTeams,
                   (size_t)ctx->teamCount*sizeof(TeamFileEx));
        ctx->seed=_seed2;ctx->randIndex=_randXorIndex2;ctx->randKey=_randXorKey2;
        ctx->lastPhase=(uint8_t)s_expOGPhase;
        ctx->used=ctx->teamCount!=0;
        }
    } else
#endif
    Career_AutoAdvanceLeagueFixtures();

    _viewRandomPhase = savedViewRandom;
    _seasonPlaying = savedSeasonPlaying;
    RestoreRand2State();
    _careerLoadedFlag = 0;
}

/* ================================================================
 * UpdateCareerPlayersFormAfterMatch (SWOS2.s line 27475, Chunk 188)
 *
 * Adjusts career team players' form (someFlag field) after a match.
 * Considers: goals scored, injuries, position type, match performance,
 * comparison of current form vs expected form.
 *
 * ASM register interface:
 *   a0 = competition context (diyCompetitionStart or similar)
 *   a1 = team1 pointer, a2 = team2 pointer
 *   d1 = team1 goals, d2 = team2 goals
 * ================================================================ */
extern uint8_t _team1GoalCounterByPlayer[];   /* 32 bytes: goal count per player slot (team 1) */
extern uint8_t _team2GoalCounterByPlayer[];   /* 32 bytes: goal count per player slot (team 2) */
extern uint16_t _avgPlayerPriceHalfClamped;
extern uint8_t _playerFormDeltaRandomTable[]; /* 144 bytes: 18×8 lookup table */

__attribute__((used, externally_visible))
void UpdateCareerPlayersFormAfterMatch(DIYCompetitionEx *comp, TeamFileEx *team1, TeamFileEx *team2,
                                         uint16_t team1Goals, uint16_t team2Goals)
{
    if (_isNationalSelector)
        return;

    /* Select goal counter based on which team is the career team.
     * ASM: if a1 == careerTeam, use team1GoalCounterByPlayer; else swap d1/d2 and use team2. */
    uint8_t *_goalCounter = _team1GoalCounterByPlayer;
    uint16_t d1 = team1Goals;
    uint16_t d2 = team2Goals;
    if (team1 != &_careerTeam) {
        uint16_t tmp = d1;
        d1 = d2;
        d2 = tmp;
        _goalCounter = _team2GoalCounterByPlayer;
    }

    /* d6 = substitutes count from competition context */
    uint16_t d6 = (uint16_t)comp->base.substitutesCount;  /* 0x3C */

    /* d7 = max match player index.
     * Primary competition (comp == diyCompetitionStart): use pointsForAWin (0x62)
     * Other: use roundLegs[0] (0x66) */
    uint16_t d7;
    if (comp == &_diyCompetitionStart) {
        d7 = comp->pointsForAWin;  /* 0x62 */
    } else {
        d7 = comp->matchData.cup.roundLegs[0];   /* 0x66 */
    }
    d7 += 11;

    /* Get average player price for career team, halve and clamp to 17 */
    uint16_t avgPrice = GetAveragePlayerPrice(&_careerTeam);
    avgPrice >>= 1;
    if (avgPrice > 0x11)
        avgPrice = 0x11;
    _avgPlayerPriceHalfClamped = avgPrice;

    /* Loop through all career players */
    for (uint16_t d3 = 0; d3 < _careerNumPlayers; d3++) {
        int16_t slot = (int8_t)_currentMatchPlayers[d3];
        uint8_t *goalCounterForPlayer = _goalCounter + slot;

        PlayerFile *pl = &_careerTeam.players[slot];
        uint8_t plSomeFlag = (uint8_t)pl->form;

        /* Skip trialist/reserve leaving special states */
        if (plSomeFlag == PLAYER_FORM_TRIALIST || plSomeFlag == PLAYER_FORM_RESERVE)
            goto nextPlayer;

        int8_t d4;  /* form delta to apply */

        /* Check if player is injured */
        if (IsPlayerInjured(pl - 2, (int16_t)d6)) {
            /* IsPlayerInjured takes the TeamFile-relative player base; the
             * Amiga asm passes the slot-base (careerTeam + slot*38, two
             * PlayerFile records before the players[] start). Preserve
             * that calling convention. */
            /* Injured: 33% chance of -1 form */
            if (Rand2() > 0x33)
                goto clampForm;
            d4 = -1;
            goto applyGoalBonus;
        }

        if (d3 == 0) {
            /* Goalkeeper (index 0) */
            if (Rand2() > 0x33)
                goto clampForm;
            if (d2 == 0) {
                d4 = 4;  /* clean sheet: +4 */
            } else {
                /* Goals taken: delta = -(goals taken).
                 * ASM: muls.w #$FFFF,d4 (multiply by -1) */
                d4 = (int8_t)(-((int16_t)d2));
            }
            goto applyGoalBonus;
        } else if (d3 < 11) {
            /* Starting outfield player (index 1-10): form-based adjustment */

            /* Expected form: someFlag/10 + price */
            int8_t sf = (int8_t)plSomeFlag;
            int16_t sfExt = (int16_t)sf;
            int32_t sfLong = (int32_t)sfExt;
            int16_t div10 = (int16_t)(sfLong / 10);  /* divs.w #$A */

            uint8_t price = (uint8_t)pl->price;
            uint8_t expected = (uint8_t)div10 + price;  /* add.b */

            /* formDelta = field_1 - expected */
            uint8_t field1 = (uint8_t)pl->cachedMatchValue;
            int8_t formDelta = (int8_t)(field1 - expected);  /* sub.b */

            int16_t d4s = (int16_t)formDelta;

            if (d4s >= 0) {
                /* Performing above expectations: positive random adjustment */
                if (d4s > 0x11) d4s = 0x11;
                uint16_t r = Rand();
                d4s <<= 3;
                uint16_t rIdx = r & 7;
                d4 = (int8_t)_playerFormDeltaRandomTable[d4s + rIdx];
            } else {
                /* Performing below expectations: negative random adjustment */
                d4s = -d4s;
                if (d4s > 0x11) d4s = 0x11;
                uint16_t r = Rand();
                d4s <<= 3;
                uint16_t rIdx = r & 7;
                d4 = -(int8_t)_playerFormDeltaRandomTable[d4s + rIdx];
            }
            goto applyGoalBonus;
        } else if (d3 < d7) {
            /* Substitute who played (index 11..d7-1) */
            if (Rand2() > 0x33)
                goto clampForm;

            uint8_t posType = (((uint8_t)pl->positionAndFace) & PF_POSITION_MASK) >> 5;
            if (posType == 0) {
                /* GK sub: random ±1 */
                uint16_t r = Rand2();
                d4 = (int8_t)(r & 1);
                d4 = -d4;  /* 0→0 or 1→-1 */
            } else {
                /* Outfield sub: -1 */
                d4 = -1;
            }
            goto applyGoalBonus;
        } else {
            /* Unused substitute (index >= d7): random small form change */
            if (Rand2() > 0x55)
                goto clampForm;
            uint16_t r = Rand2();
            d4 = (int8_t)(r & 1);
            d4 = -d4;
            d4 -= 1;  /* 0→-1 or 1→-2 */
            goto applyGoalBonus;
        }

applyGoalBonus:
        /* Goal scorer bonus */
        {
            uint8_t goals = *goalCounterForPlayer;
            if (goals != 0) {
                d4 += 1;
                if ((Rand2() & 1) == 0)
                    d4 += 1;
                if (goals != 1) {
                    if ((Rand2() & 3) == 0)
                        d4 += 1;
                }
            }
        }

        /* Clamp delta to [-maxDelta..+maxDelta] where maxDelta = random 1 or 2 */
        {
            uint16_t r = Rand2();
            int16_t maxDelta = (int16_t)((r & 1) + 1);
            if ((int8_t)d4 > (int8_t)maxDelta)
                d4 = (int8_t)maxDelta;
            int8_t negMax = (int8_t)(-maxDelta);
            if ((int8_t)d4 < negMax)
                d4 = negMax;
        }

        /* Apply delta to someFlag.
         * ASM has dead code checking d0 against 0x9B/0x9A (impossible values).
         * Replicated as no-op. */
        pl->form = (int8_t)((int8_t)pl->form + d4);

clampForm:
        /* Clamp someFlag to [-100..100] signed (0x9C..0x64 unsigned) */
        if ((int8_t)pl->form < (int8_t)PLAYER_FORM_MIN)
            pl->form = (int8_t)PLAYER_FORM_MIN;
        if ((int8_t)pl->form > (int8_t)PLAYER_FORM_MAX)
            pl->form = (int8_t)PLAYER_FORM_MAX;

nextPlayer:
        ;
    }
}

/* ===================================================================
 * Chunk 189: Career_UpdateStatsAfterMatch  (SWOS2.s line 32584)
 *
 * Updates board confidence modifiers, win/loss/draw/goals stats,
 * and stores match result into the competition slot record.
 *
 * ASM register inputs:
 *   a1 = team1 ptr, a2 = team2 ptr
 *   d0 = competition type (0=league, non-zero=cup)
 *   d4 = packed goals (team1<<8 | team2)
 *   d5 = aggregate score (word)
 *   d6 = penalty score packed (team1<<8 | team2)
 *   d7 = flags (bit 3 = went to penalties; low byte stored to result)
 * =================================================================== */
extern uint8_t _teamSelectionKeyCache[];
extern uint16_t _teamSelectionKeyCacheOffset;
extern uint16_t _boardConfidenceMod0;
extern uint16_t _boardConfidenceMod1;
extern uint16_t _boardConfidenceMod2;
extern uint16_t _boardConfidenceMod3;
extern uint16_t _boardConfidenceMod4;
extern uint16_t _boardConfidenceMod5;
extern uint16_t _boardConfidenceMod6;
extern uint16_t _boardConfidenceMod7;
extern uint16_t _boardConfidenceMod8;
extern uint16_t _boardConfidenceMod9;
extern uint16_t _boardConfidenceValue0;
extern uint16_t _boardConfidenceValue1;
extern uint16_t _gamesPlayed;
extern uint16_t _numWins;
extern uint16_t _numDrawn;
extern uint16_t _numLost;
extern uint16_t _goalsScored;
extern uint16_t _goalsTaken;
extern uint16_t _careerSlotProcessingInProgress;
extern uint32_t _aEuropeanTour;  /* string label — used as address boundary */
extern uint32_t _aOceaniaTour;   /* string label — used as address boundary */

extern uint16_t _seasonNumberOfSquads;
extern uint16_t _seasonSquadTeamNumbers[];

/* Does this fixture belong to a team the PLAYER controls — i.e. should it
 * consume one of the round's keycache match slots?
 *
 * Career (gameType 4): the one career team, selTeamNumber.
 *
 * Season (gameType 3): selTeamNumber is NEVER assigned. SeasonMenu calls
 * InitCareerVariables(GAME_TYPE_SEASON), which sets it to 0xFFFF, and the only
 * writers (Career_InitializeSelectedTeamAndStartingBalance, via InitCareer /
 * Career_ContinueToNextSeasonInitTeamAndFunds) are career-only paths — so it
 * stays 0xFFFF for the whole season. The human-controlled teams live in the
 * seasonSquadTeamNumbers list instead, which InitNewSeason fills from the
 * non-computer selected teams using the same PACK_BE_U16 {country, ordinal}
 * key we compare against here.
 *
 * Getting this wrong is not cosmetic: a bare selTeamNumber test is false for
 * EVERY season match, so the caller's store block never runs — matchFlags /
 * packedGoals are never written (the season fixture list keeps rendering
 * "VALENCIA V SEVILLA" for matches already played) and remainingElements never
 * decrements (Career_PlayMatchMenuLoop then takes its no-advance path, so the
 * list cursor desyncs from the real league fixture index).
 */
static int KeyCacheFixtureIsPlayerControlled(uint16_t t1num, uint16_t t2num)
{
    if (t1num == _selTeamNumber || t2num == _selTeamNumber)
        return 1;

    if (_gameType == GAME_TYPE_SEASON) {
        for (uint16_t i = 0; i < _seasonNumberOfSquads; i++) {
            uint16_t id = _seasonSquadTeamNumbers[i];
            if (id == t1num || id == t2num)
                return 1;
        }
    }
    return 0;
}

__attribute__((used, externally_visible))
void Career_UpdateStatsAfterMatch(TeamFileEx *team1, TeamFileEx *team2, uint16_t compType,
                                    uint16_t packedGoals, uint16_t aggregate,
                                    uint16_t penalty, uint16_t flags)
{
    _teamSelectionKeyCacheEnd = 1;

    /* Pre-hire matches belong to the CLUB, not to the new manager.
     * Still run the native competition score/cache bookkeeping below. */
    extern int Expanded_MidseasonJoin_Active(void);
    if (_youreSacked || Expanded_MidseasonJoin_Active())
        goto skipStats;

    /* Determine which team is the career team and assign goals accordingly */
    uint8_t team1Goals = (uint8_t)(packedGoals >> 8);
    uint8_t team2Goals = (uint8_t)(packedGoals & 0xFF);
    uint8_t team1Pen = (uint8_t)(penalty >> 8);
    uint8_t team2Pen = (uint8_t)(penalty & 0xFF);

    int16_t result;  /* 1=win, -1=loss, 0=draw */

    if (team1 == &_careerTeam) {
        /* Career team is team1: career goals = high byte, opponent = low byte */
        _boardConfidenceMod0++;
        _gamesPlayed++;
        _boardConfidenceValue0 += team1Goals;
        _goalsScored += team1Goals;
        _boardConfidenceValue1 += team2Goals;
        _goalsTaken += team2Goals;

        if (team1Goals > team2Goals) {
            result = 1;  /* Win */
        } else if (team1Goals < team2Goals) {
            result = -1;  /* Loss */
        } else {
            /* Scores equal — check penalties */
            if (!(flags & 8)) {
                result = 0;  /* Draw (no penalties) */
            } else if (team1Pen > team2Pen) {
                result = 1;  /* Win on penalties */
            } else {
                result = -1;  /* Loss on penalties */
            }
        }
    } else if (team2 == &_careerTeam) {
        /* Career team is team2: career goals = low byte, opponent = high byte */
        _boardConfidenceMod0++;
        _gamesPlayed++;
        _boardConfidenceValue1 += team1Goals;
        _goalsTaken += team1Goals;
        _boardConfidenceValue0 += team2Goals;
        _goalsScored += team2Goals;

        if (team1Goals < team2Goals) {
            result = 1;  /* Win (opponent < career) */
        } else if (team1Goals > team2Goals) {
            result = -1;  /* Loss (opponent > career) */
        } else {
            /* Scores equal — check penalties */
            if (!(flags & 8)) {
                result = 0;  /* Draw */
            } else if (team1Pen > team2Pen) {
                result = -1;  /* Loss on penalties (opponent won) */
            } else {
                result = 1;  /* Win on penalties */
            }
        }
    } else {
        /* Neither team is career team — skip stats */
        goto skipStats;
    }

    /* Apply result to confidence modifiers and stat counters */
    if (_gameType == GAME_TYPE_CAREER) {
        extern void Expanded_SentimentMatchResult(int result,int away);
        Expanded_SentimentMatchResult(result,team2==&_careerTeam);
    }

    if (result > 0) {
        /* Win */
        _boardConfidenceMod1++;
        _numWins++;
        if (compType == 0) {
            uint8_t *tourPtr = _diyCompetitionStart.base.namePtr;
            if (tourPtr >= (uint8_t *)&_aEuropeanTour && tourPtr <= (uint8_t *)&_aOceaniaTour) {
                _boardConfidenceMod7++;
            } else if (_diyCompetitionStart.matchData.league.hasGroupsDiy) {
                _boardConfidenceMod4++;
            }
        }
    } else if (result < 0) {
        /* Loss */
        _boardConfidenceMod3++;
        _numLost++;
        if (compType == 0) {
            uint8_t *tourPtr = _diyCompetitionStart.base.namePtr;
            if (tourPtr >= (uint8_t *)&_aEuropeanTour && tourPtr <= (uint8_t *)&_aOceaniaTour) {
                _boardConfidenceMod9++;
            } else if (_diyCompetitionStart.matchData.league.hasGroupsDiy) {
                _boardConfidenceMod6++;
            }
        }
    } else {
        /* Draw */
        _boardConfidenceMod2++;
        _numDrawn++;
        if (compType == 0) {
            uint8_t *tourPtr = _diyCompetitionStart.base.namePtr;
            if (tourPtr >= (uint8_t *)&_aEuropeanTour && tourPtr <= (uint8_t *)&_aOceaniaTour) {
                _boardConfidenceMod8++;
            } else if (_diyCompetitionStart.matchData.league.hasGroupsDiy) {
                _boardConfidenceMod5++;
            }
        }
    }

skipStats: ;
    /* Store match result into competition slot record.
     * ASM: a1 is overwritten by GetCurrentSelectedElementPtr (returns in a1).
     * The base cache entry (a0) gets its counter decremented,
     * while the selected element (a1) receives match result data. */
    /* Only the genuine SELECTED-team fixture consumes a keycache slot. During cup
     * auto-advance the code sets selTeamNumber=0xFFFF and the player's other legs
     * are auto-played through a STALE selTeams copy of careerTeam that carries
     * teamControls=2 (the Amiga loads those cup slots as teamControls=COMPUTER, so
     * the whole block below is skipped there). On SDL the copy's ctl=2 let every
     * auto-played leg over-decrement the cursor entry past 0 -> stray match-score
     * writes smashed an entryStride -> DIY_RecordCupBracketProgress panic 0x6666.
     * Gate on the player's team: its number during interactive play, and no
     * match during auto-advance (selTeamNumber is 0xFFFF there). This
     * reproduces the Amiga's net keycache effect regardless of the copy's wrong
     * teamControls. (See panic_0x6666 analysis.)
     *
     * The player-team test is MODE-AWARE — season mode never assigns
     * selTeamNumber, so a bare comparison against it silently disabled this
     * whole block for every season match. See
     * KeyCacheFixtureIsPlayerControlled. */
    uint16_t t1num = PACK_BE_U16(team1->team.countryNumber, team1->team.teamOrdinal);
    uint16_t t2num = PACK_BE_U16(team2->team.countryNumber, team2->team.teamOrdinal);
    if ((team1->team.teamControls != 1 || team2->team.teamControls != 1 ||
         Expanded_MidseasonJoin_Active()) &&
        KeyCacheFixtureIsPlayerControlled(t1num, t2num)) {
        /* Asm calls GetCurrentSelectedElementPtr FIRST, THEN decrements
         * remainingElements. Order matters: the slot pointer depends on
         * (totalElements - remainingElements) * 12. */
        TeamSelectionKeyCacheMatchSlot *slot = TeamSelectionKeyCache_GetCurrentMatchSlot();

        /* Base cache entry — decrement remainingElements AFTER getting slot */
        TeamSelectionKeyCacheEntry *base_entry =
            (TeamSelectionKeyCacheEntry *)(_teamSelectionKeyCache + _teamSelectionKeyCacheOffset);
        base_entry->remainingElements -= 1;

        slot->matchFlags    = (uint8_t)flags;
        slot->packedGoals   = packedGoals;
        slot->aggregate     = aggregate;
        slot->penaltyScore  = penalty;
    }

    /* Set career slot processing variable */
    if (compType == 0) {
        _careerSlotProcessingInProgress = _diyCompetitionStart.leagueCurrentSlot;
    } else {
        _careerSlotProcessingInProgress = (uint16_t)_diyCompetitionStart.base.roundComplete;
    }
}

/* ========================================================================
 * Chunk 194 — Career_AutoAdvanceLeagueFixtures, Career_AutoAdvanceCupMatches
 * ======================================================================== */

extern uint8_t _careerLastMatchIndexByCompetitionSlot[];
extern uint16_t _highlightedLeagueTeamId;
extern uint16_t _teamSelectionKeyCacheIndex;
extern uint16_t _teamSelectionKeyCacheCount;

/* Forward declarations for Chunk 194 dependencies */
extern uint8_t *TeamSelectionKeyCache_GetCurrentListPtr(void);
extern void DIY_AdvanceFixtureIndex(uint16_t lo, uint16_t hi);
extern uint16_t DIY_PlayOrSimulateLeagueFixture(int16_t allowSquadChanges, int16_t selector);
extern uint16_t DIY_PlayCupMatchAndAdvance(int16_t allowSquadChanges, int16_t mode);
extern void DIY_UpdateCompetitionProgress(void);
extern void ShowDiyLeaguesTournamentMenu(uint16_t type, uint16_t viewOnly, uint16_t focusOverride);
extern void ShowDIYCupMenuWithParams(uint16_t arg0, uint16_t arg1, uint16_t arg2);

/* Career_AutoAdvanceLeagueFixtures (SWOS2.s line 31608)
 *
 * Auto-simulates all league fixtures for the current competition up to the
 * current game phase. Checks guard flags first, then loops simming matches
 * until the competition phase catches up with the current/next cache phase.
 * When done, optionally shows the league/tournament menu.
 */
#ifdef SWOS_DEBUG_AUTOSKIP
/* Host-native compSlot/slotId word, but the parent is only 2-byte
 * aligned on x64: memcpy avoids int16 pointer aliasing and misalignment. */
static uint16_t Career_ReadCompSlotWordUnaligned(const DIYCompetitionEx *comp)
{
    uint16_t word;
    memcpy(&word, (const uint8_t *)comp + offsetof(DIYCompetitionEx,base.compSlot),
           sizeof word);
    return word;
}
#endif
__attribute__((used, externally_visible))
void Career_AutoAdvanceLeagueFixtures(void)
{
    uint8_t currentPhase, nextPhase;
#ifdef SWOS_SDL_BUILD
    /* Never dereference a stale keycache cursor or advance any fixture
     * after the exact resident season is already complete. The menu must
     * transition via FINISH SEASON, not retry an impossible match. */
    if(_gameType==4 && Expanded_OGWorld_ManagerLeagueFinished(
                          &_diyCompetitionStart)) {
        extern uint16_t _diyAllMatchesPlayed;
        _diyAllMatchesPlayed=1;
        return;
    }
#endif

    /* World OG updates have their own phase, not a manager's keycache. */
#ifdef SWOS_SDL_BUILD
    if(s_expOGCurrent && s_expOGPhase>=0) {
        currentPhase=(uint8_t)s_expOGPhase;
        nextPhase=currentPhase;
    } else
#endif
    if (_careerAutoAdvanceInProgress != 0 || _seasonUpdateInProgress != 0 ||
        _internationalAutoAdvanceFlag != 0 || _seasonFinished != 0) {
        currentPhase = 0xFF;
        nextPhase = 0xFF;
    } else {
        /* Get phase bytes (sortKey) from TeamSelectionKeyCache */
        TeamSelectionKeyCacheEntry *cacheEntry =
            (TeamSelectionKeyCacheEntry *)TeamSelectionKeyCache_GetCurrentListPtr();
        currentPhase = cacheEntry->sortKey;
        uint16_t nextIdx = _teamSelectionKeyCacheIndex + 1;
        if (nextIdx == _teamSelectionKeyCacheCount) {
            nextPhase = 0xFF;
        } else {
            TeamSelectionKeyCacheEntry *next =
                (TeamSelectionKeyCacheEntry *)((uint8_t *)cacheEntry + cacheEntry->entryStride);
            nextPhase = next->sortKey;
        }
    }

    /* Access the packed parent directly; OG updates remain live. */
    uint8_t *lastMatchSlots = _careerLastMatchIndexByCompetitionSlot;
    uint8_t slotIdx = _diyCompetitionStart.base.field_2E;
    uint8_t *lastMatchPtr = lastMatchSlots + (uint16_t)slotIdx;
    uint8_t lastMatch = *lastMatchPtr;

#ifdef SWOS_DEBUG_AUTOSKIP
    {
        DIYCompetitionEx *bex = &_diyCompetitionStart;
        SWOS_AUTOSKIP_LOG("[bug3-league] AutoAdvanceLeague ENTER compId=0x%04X slotIdx=%u "
                          "currentPhase=%u nextPhase=%u viewRandomPhase=%u "
                          "lastMatch=%u leagueMatchProgress=%u sectionsLeft(minSubs)=%u "
                          "leagueCurrentSlot=%u totalMatchesPerSectionLive=%d "
                          "totalMatchesInRound=%d subRoundCounter=%d "
                          "seasonFinished=%u",
                          (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart), (unsigned)slotIdx,
                          (unsigned)currentPhase, (unsigned)nextPhase,
                          (unsigned)_diyCompetitionStart.base.viewRandomPhase,
                          (unsigned)lastMatch, (unsigned)bex->leagueMatchProgress,
                          (unsigned)bex->minSubstitutes,
                          (unsigned)bex->leagueCurrentSlot,
                          (int)bex->matchData.league.totalMatchesPerSectionLive,
                          (int)_diyCompetitionStart.base.totalMatchesInRound,
                          (int)_diyCompetitionStart.base.subRoundCounter,
                          (unsigned)_seasonFinished);
    }
#endif

#ifdef SWOS_SDL_BUILD
    /* This OG competition is finished. A later civil-time tick must not
     * enter the native fixture loop again and produce self-matches. */
    if(s_expOGCurrent && _diyCompetitionStart.minSubstitutes==0 &&
       _diyCompetitionStart.matchesPlayedCount>0u) {
        fprintf(stderr,"[og-world-fixture] FINISHED country=%u division=%u "
                       "year=%u phase=%u played=%u (NO EXTRA MATCH)\n",
                (unsigned)s_expOGCurrent->country,
                (unsigned)s_expOGCurrent->division,
                (unsigned)s_expOGCurrent->year,(unsigned)currentPhase,
                (unsigned)_diyCompetitionStart.matchesPlayedCount);
        return;
    }
    /* The manager may accept a club after its OG resident league finished.
     * On both worlds the 24-team 2-leg table has 552 results already.
     * The native fixture iterator must not see match 553 or a self-pair. */
    if(Expanded_OGWorld_ManagerLeagueFinished(&_diyCompetitionStart)) {
        fprintf(stderr,"[og-manager-bridge] FINISHED_NO_EXTRA_FIXTURE "
                       "world=%s club=%04x played=%u "
                       "resident=COMPLETE no-replay=1\n",
                World_Tag(),(unsigned)_selTeamNumber,
                (unsigned)_diyCompetitionStart.matchesPlayedCount);
        return;
    }
    uint32_t ogWorldIterations=0;
#endif
    /* Inline DIY_AdvanceLeagueFixtureIndex: advance fixture and set d6 */
    uint8_t viewRP = _diyCompetitionStart.base.viewRandomPhase;
    DIY_AdvanceFixtureIndex(0, (uint16_t)viewRP);
    /* d6 tracking not needed — sim path (selector=0) ignores allowSquadChanges */

    /* Main simulation loop */
main_loop:
#ifdef SWOS_SDL_BUILD
    if(s_expOGCurrent && ++ogWorldIterations>8192u) {
        s_expOGFault=1;
        fprintf(stderr,"[og-world-fixture] LOOP LIMIT phase=%u played=%u\n",
                (unsigned)currentPhase,
                (unsigned)_diyCompetitionStart.matchesPlayedCount);
        return;
    }
    if(s_expOGCurrent && _diyCompetitionStart.minSubstitutes==0 &&
       _diyCompetitionStart.matchesPlayedCount>0u)return;
    if(Expanded_OGWorld_ManagerLeagueFinished(&_diyCompetitionStart))return;
#endif
    if (currentPhase <= _diyCompetitionStart.base.viewRandomPhase)
        goto check_next_phase;

    /* Sim league fixtures until phase catches up */
    if(DIY_PlayOrSimulateLeagueFixture(0, 0)!=0) {
#ifdef SWOS_SDL_BUILD
        if(s_expOGBridgeBlocked && !s_expOGCurrent && !s_expOGCupCurrent) {
            /* The failed preflight did not consume an OG fixture. Leaving
             * leagueCurrentSlot unchanged is correct, but retrying it
             * would trap the calendar menu in a busy loop. */
            return;
        }
        if(s_expOGCurrent){s_expOGFault=1;return;}
#endif
    }
#ifdef SWOS_SDL_BUILD
    if(s_expOGCurrent && s_expOGFault)return;
#endif
    if (_diyCompetitionStart.leagueCurrentSlot == 0)
        goto main_loop; /* more matches pending in current fixture set */

    if (_diyCompetitionStart.minSubstitutes != 0) {
        /* Advance fixture index and continue */
        viewRP = _diyCompetitionStart.base.viewRandomPhase;
        DIY_AdvanceFixtureIndex((uint16_t)(viewRP + 1), (uint16_t)viewRP);
        goto main_loop;
    }

    /* Check groups type */
    if (_diyCompetitionStart.base.hasGroups == 0)
        goto show_menu_check;
    if (_diyCompetitionStart.base.hasGroups == 2)
        goto show_menu_check;
    return; /* groups type 1/3 — return silently */

check_next_phase:
    if (nextPhase > _diyCompetitionStart.base.viewRandomPhase)
        goto catch_up_phase;

show_menu_check:
    if (_internationalAutoAdvanceFlag != 0)
        return;
    if (_careerAutoAdvanceInProgress != 0)
        return;

#ifdef SWOS_DEBUG_AUTOSKIP
    {
        DIYCompetitionEx *bex = &_diyCompetitionStart;
        SWOS_AUTOSKIP_LOG("[bug3-league] AutoAdvanceLeague EXIT(show_menu) compId=0x%04X "
                          "viewRandomPhase=%u leagueMatchProgress=%u sectionsLeft=%u "
                          "leagueCurrentSlot=%u totalMatchesPerSectionLive=%d subRoundCounter=%d",
                          (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                          (unsigned)_diyCompetitionStart.base.viewRandomPhase,
                          (unsigned)bex->leagueMatchProgress,
                          (unsigned)bex->minSubstitutes,
                          (unsigned)bex->leagueCurrentSlot,
                          (int)bex->matchData.league.totalMatchesPerSectionLive,
                          (int)_diyCompetitionStart.base.subRoundCounter);
    }
#endif

    /* Show league menu. SWOS2.s loc_12D754/loc_12D764 calls (1,1,0)
     * unconditionally here — the highlightedLeagueTeamId check belongs
     * only on the catch_up_phase first-time path (loc_12D790).
     * arg1=1 is required so DIYLeagueTournamentInit's seasonUpdate-pre-
     * select-entry-42 isn't overridden by the menuState fallback in
     * UpdateMenuEntryVisibilityAndFocus, which would otherwise drop the
     * controlMask=0xFFFF auto-skip the season transition relies on. */
    ShowDiyLeaguesTournamentMenu(1, 1, 0);
    return;

catch_up_phase:
#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug3-league] AutoAdvanceLeague catch_up_phase compId=0x%04X "
                      "lastMatch=%u viewRandomPhase=%u (eq=%d)",
                      (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                      (unsigned)lastMatch, (unsigned)_diyCompetitionStart.base.viewRandomPhase,
                      (int)(lastMatch == _diyCompetitionStart.base.viewRandomPhase));
#endif
    /* Check if lastMatch matches current viewRandomPhase */
    if (lastMatch == _diyCompetitionStart.base.viewRandomPhase) {
        /* Catch-up loop: sim until leagueCurrentSlot != 0 */
        do {
#ifdef SWOS_SDL_BUILD
            if(Expanded_OGWorld_ManagerLeagueFinished(&_diyCompetitionStart))
                return;
            if(s_expOGCurrent && ++ogWorldIterations>8192u) {
                s_expOGFault=1;
                fprintf(stderr,"[og-world-fixture] CATCHUP LOOP LIMIT phase=%u\n",
                        (unsigned)currentPhase);
                return;
            }
#endif
            if(DIY_PlayOrSimulateLeagueFixture(0, 0)!=0) {
#ifdef SWOS_SDL_BUILD
                if(s_expOGBridgeBlocked && !s_expOGCurrent && !s_expOGCupCurrent) {
                    /* Fail closed also in catch-up; never retry a blocked
                     * fixture forever on the unchanged leagueCurrentSlot. */
                    fprintf(stderr,"[og-manager-bridge] ABORT_CATCHUP "
                                   "reason=LOCAL_RESIDENT_DESYNC no-replay=1\n");
                    return;
                }
                if(s_expOGCurrent){s_expOGFault=1;return;}
#endif
            }
#ifdef SWOS_SDL_BUILD
            if(s_expOGCurrent && s_expOGFault)return;
#endif
        } while (_diyCompetitionStart.leagueCurrentSlot == 0);
        goto show_menu_check;
    }

    /* Store current viewRandomPhase as last match index */
    *lastMatchPtr = _diyCompetitionStart.base.viewRandomPhase;
    if (_highlightedLeagueTeamId != 0xFFFF) {
        ShowDiyLeaguesTournamentMenu(1, 1, 0);
    } else {
        ShowDiyLeaguesTournamentMenu(1, 0, 0);
    }
}

/* Career_AutoAdvanceCupMatches (SWOS2.s line 31708)
 *
 * Auto-simulates all cup matches for the current competition up to the
 * current game phase. Similar to league version but uses cup-specific
 * functions and checks roundComplete (0x54) and finishedFlag (0x56).
 */
__attribute__((used, externally_visible))
void Career_AutoAdvanceCupMatches(void)
{
    uint8_t currentPhase, nextPhase;

    /* For a world-resident OG cup, the phase is the ONE authoritative
     * world phase, never the manager's (unrelated) key-cache cursor.
     * The other branch is unchanged for CLASSIC and employed CAREER. */
#ifdef SWOS_SDL_BUILD
    if(s_expOGCupCurrent || exp_og_is_uefa()) {
        currentPhase=s_expOGCupCurrent?(uint8_t)s_expOGCupPhase:(uint8_t)s_expOGPhase;
        nextPhase=0u; /* no fictitious "next" manager fixture */
        if(_diyCompetitionStart.base.competitionFinished)return;
        if(currentPhase<=_diyCompetitionStart.base.viewRandomPhase)return;
    } else
#endif
    /* Guard checks */
    if (_careerAutoAdvanceInProgress != 0 || _seasonUpdateInProgress != 0 ||
        _internationalAutoAdvanceFlag != 0 || _seasonFinished != 0) {
        currentPhase = 0xFF;
        nextPhase = 0xFF;
    } else {
        TeamSelectionKeyCacheEntry *cacheEntry =
            (TeamSelectionKeyCacheEntry *)TeamSelectionKeyCache_GetCurrentListPtr();
        currentPhase = cacheEntry->sortKey;
        uint16_t nextIdx = _teamSelectionKeyCacheIndex + 1;
        if (nextIdx == _teamSelectionKeyCacheCount) {
            nextPhase = 0xFF;
        } else {
            TeamSelectionKeyCacheEntry *next =
                (TeamSelectionKeyCacheEntry *)((uint8_t *)cacheEntry + cacheEntry->entryStride);
            nextPhase = next->sortKey;
        }
    }

    /* Access the packed parent directly; OG updates remain live. */
    uint8_t *lastMatchSlots = _careerLastMatchIndexByCompetitionSlot;
    uint8_t slotIdx = _diyCompetitionStart.base.field_2E;
    uint8_t *lastMatchPtr = lastMatchSlots + (uint16_t)slotIdx;
    uint8_t lastMatch = *lastMatchPtr;

#ifdef SWOS_DEBUG_AUTOSKIP
    int debug_cup_iters = 0;
    SWOS_AUTOSKIP_LOG("[bug4-cup] AutoAdvanceCupMatches ENTER compId=0x%04X "
                      "currentPhase=%u nextPhase=%u viewRandomPhase=%u lastMatch=%u "
                      "currentRoundIdx=%d/%d matchIdx=%d/%d roundComplete=%d "
                      "competitionFinished=%d hasGroups=%d",
                      (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                      (unsigned)currentPhase, (unsigned)nextPhase,
                      (unsigned)_diyCompetitionStart.base.viewRandomPhase, (unsigned)lastMatch,
                      (int)_diyCompetitionStart.base.currentRoundIndex, (int)_diyCompetitionStart.base.numRounds,
                      (int)_diyCompetitionStart.matchData.cup.currentMatchIndex,
                      (int)_diyCompetitionStart.base.totalMatchesInRound,
                      (int)_diyCompetitionStart.base.roundComplete,
                      (int)_diyCompetitionStart.base.competitionFinished,
                      (int)_diyCompetitionStart.base.hasGroups);
#endif

    /* Inline DIY_AdvanceCupFixtureIndex */
    uint8_t viewRP = _diyCompetitionStart.base.viewRandomPhase;
    DIY_AdvanceFixtureIndex(0, (uint16_t)viewRP);

    /* First check: if currentPhase <= viewRandomPhase, skip main loop */
    if (currentPhase <= _diyCompetitionStart.base.viewRandomPhase)
        goto check_next_phase_cup;

    /* Main cup sim loop. A resident cup must never spin forever when
     * advancing a malformed OG bracket; normal OG callback is unchanged. */
#ifdef SWOS_SDL_BUILD
    unsigned residentCupIterations=0u;
#endif
cup_main_loop:
#ifdef SWOS_SDL_BUILD
    if((s_expOGCupCurrent || exp_og_is_uefa()) &&
       ++residentCupIterations>2048u) {
        if(s_expOGCupCurrent)s_expOGCupTickFault=1;
        if(exp_og_is_uefa())s_expOGFault=1;
        fprintf(stderr,"[og-cup-resident] LOOP LIMIT phase=%u kind=%s\n",
                (unsigned)currentPhase,
                exp_og_is_uefa()?"UEFA_TYPE2":"DOMESTIC_CUP");
        return;
    }
#endif
#ifdef SWOS_DEBUG_AUTOSKIP
    if (++debug_cup_iters > 500) {
        SWOS_AUTOSKIP_LOG("[bug4-cup] AutoAdvanceCupMatches EMERGENCY BAIL "
                          "iter>%d compId=0x%04X viewRandomPhase=%u currentPhase=%u "
                          "roundComplete=%d competitionFinished=%d",
                          debug_cup_iters,
                          (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                          (unsigned)_diyCompetitionStart.base.viewRandomPhase, (unsigned)currentPhase,
                          (int)_diyCompetitionStart.base.roundComplete,
                          (int)_diyCompetitionStart.base.competitionFinished);
        return;
    }
#endif
    if(DIY_PlayCupMatchAndAdvance(0, 0)!=0) {
#ifdef SWOS_SDL_BUILD
        if(s_expOGCupCurrent || exp_og_is_uefa()) {
            if(s_expOGCupCurrent)s_expOGCupTickFault=1;
            if(exp_og_is_uefa())s_expOGFault=1;
            return;
        }
#endif
    } else {
#ifdef SWOS_SDL_BUILD
        if(s_expOGCupCurrent)Expanded_OGWorld_CupMarkPlayed();
#endif
    }

    if (_diyCompetitionStart.base.roundComplete == 0)
        goto cup_main_loop; /* roundComplete == 0, keep going */

    if (_diyCompetitionStart.base.competitionFinished != 0)
        goto cup_finished; /* finishedFlag != 0 */

    if (currentPhase <= _diyCompetitionStart.base.viewRandomPhase)
        goto check_next_phase_cup;

    /* Update competition progress and advance fixture */
    DIY_UpdateCompetitionProgress();
    viewRP = _diyCompetitionStart.base.viewRandomPhase;
    DIY_AdvanceFixtureIndex(0, (uint16_t)viewRP);
    goto cup_main_loop;

cup_finished:
#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug4-cup] AutoAdvanceCupMatches cup_finished compId=0x%04X "
                      "iters=%d hasGroups=%d viewRandomPhase=%u",
                      (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                      debug_cup_iters, (int)_diyCompetitionStart.base.hasGroups,
                      (unsigned)_diyCompetitionStart.base.viewRandomPhase);
#endif
    if (_diyCompetitionStart.base.hasGroups == 0)
        goto cup_show_menu_check;
    if (_diyCompetitionStart.base.hasGroups == 2)
        goto cup_show_menu_check;
    return; /* groups type 1/3 — return silently */

check_next_phase_cup:
    if (nextPhase > _diyCompetitionStart.base.viewRandomPhase)
        goto cup_catch_up;

cup_show_menu_check:
    if (_internationalAutoAdvanceFlag != 0)
        return;
    if (_careerAutoAdvanceInProgress != 0)
        return;

#ifdef SWOS_DEBUG_AUTOSKIP
    SWOS_AUTOSKIP_LOG("[bug4-cup] AutoAdvanceCupMatches EXIT (show_menu) compId=0x%04X "
                      "iters=%d currentRoundIdx=%d viewRandomPhase=%u "
                      "roundComplete=%d competitionFinished=%d",
                      (unsigned)Career_ReadCompSlotWordUnaligned(&_diyCompetitionStart),
                      debug_cup_iters, (int)_diyCompetitionStart.base.currentRoundIndex,
                      (unsigned)_diyCompetitionStart.base.viewRandomPhase,
                      (int)_diyCompetitionStart.base.roundComplete,
                      (int)_diyCompetitionStart.base.competitionFinished);
#endif

    /* Show cup menu. SWOS2.s loc_12D884/loc_12D894 calls (1,1,0)
     * unconditionally here — the highlightedLeagueTeamId check belongs
     * only on the catch_up_phase first-time path (loc_12D8D6).
     * arg1=1 is required so DIYCupInit's seasonUpdate-pre-select-entry-
     * 66 isn't overridden by the menuState fallback in
     * DesignDIYCup_UpdateMenuEntriesVisibilityAndSelection, which would
     * otherwise drop the controlMask=0xFFFF auto-skip the season
     * transition relies on. */
    ShowDIYCupMenuWithParams(1, 1, 0);
    return;

cup_catch_up:
    /* If roundComplete, update progress and advance fixture */
    if (_diyCompetitionStart.base.roundComplete != 0) {
        DIY_UpdateCompetitionProgress();
        viewRP = _diyCompetitionStart.base.viewRandomPhase;
        DIY_AdvanceFixtureIndex(0, (uint16_t)viewRP);
    }

    if (lastMatch == _diyCompetitionStart.base.viewRandomPhase) {
        /* Catch-up loop */
        do {
            DIY_PlayCupMatchAndAdvance(0, 0);
        } while (_diyCompetitionStart.base.roundComplete == 0);
        goto cup_show_menu_check;
    }

    /* Store current viewRandomPhase as last match index */
    *lastMatchPtr = _diyCompetitionStart.base.viewRandomPhase;
    if (_highlightedLeagueTeamId != 0xFFFF) {
        ShowDIYCupMenuWithParams(1, 1, 0);
    } else {
        ShowDIYCupMenuWithParams(1, 0, 0);
    }
}

extern void DIY_SortLeagueTableByStandings(void);
extern void DIY_AdvanceToNextCompetitionSlot(void);

/* Career_AutoAdvanceCompetitionSlots (SWOS2.s line 31635)
 *
 * Iterates through competition slots in diyFileBufferCopy, dispatching each
 * to league or cup auto-advance based on the slot type flag at offset 0x15E.
 * Sets hasGroups (0x48) to indicate slot position (1=normal, 2=last).
 * Loops via tail-call recursion until all slots are processed.
 */
__attribute__((used, externally_visible))
void Career_AutoAdvanceCompetitionSlots(void)
{
#ifdef SWOS_SDL_BUILD
    unsigned residentStages=0u;
#endif
restart:
#ifdef SWOS_SDL_BUILD
    if(exp_og_is_uefa() && ++residentStages>24u) {
        s_expOGFault=1;
        fprintf(stderr,"[og-uefa-resident] INVALID_NATIVE_STAGE_LOOP\n");
        return;
    }
#endif
    ;
    uint8_t *bufCopy = (uint8_t *)&_diyFileBufferCopy;
    DIYCompetition *bufCopyBase = (DIYCompetition *)bufCopy;
    uint16_t slotIndex = (uint16_t)bufCopyBase->roundComplete;
#ifdef SWOS_SDL_BUILD
    if(exp_og_is_uefa()) {
        const unsigned maxSlots=(unsigned)(sizeof _diyFileBufferCopy.slots /
                                           sizeof _diyFileBufferCopy.slots[0]);
        const unsigned stages=(unsigned)bufCopyBase->totalMatchesInRound;
        if(!stages || stages>maxSlots || slotIndex>=stages) {
            /* A malformed or finished snapshot must not dereference the
             * next slot of a 1996/97 or 2026/27 resident tournament. */
            s_expOGFault=1;
            fprintf(stderr,"[og-uefa-stage] INVALID_INDEX world=%u "
                           "phase=%d index=%u count=%u capacity=%u\n",
                    (unsigned)World_Current(),s_expOGPhase,
                    (unsigned)slotIndex,stages,maxSlots);
            return;
        }
    }
#endif
    DIYTournamentSlot *slotRec = (DIYTournamentSlot *)(bufCopy + (uint32_t)slotIndex * 0x76 + 0x15A);
    DIYCompetitionEx *compEx = &_diyCompetitionStart;

    /* Determine hasGroups value based on slot position */
    uint16_t hasGroups = 1;
    if (slotIndex + 1 == (uint16_t)bufCopyBase->totalMatchesInRound)
        hasGroups = 2; /* last slot */

    if (slotRec->isLeagueFormat != 0) {
        /* League path */
        compEx->base.hasGroups = (int16_t)hasGroups;
        Career_AutoAdvanceLeagueFixtures();
#ifdef SWOS_SDL_BUILD
        /* Never sort/advance a native stage after its local fixture failed.
         * In particular an unchanged leagueCurrentSlot would loop forever. */
        if(s_expOGBridgeBlocked && !s_expOGCurrent && !s_expOGCupCurrent) {
            fprintf(stderr,"[og-manager-bridge] ABORT_COMPETITION_STAGES "
                           "world=%s reason=LOCAL_RESIDENT_DESYNC\n",
                    World_Tag());
            return;
        }
#endif
        if (compEx->base.hasGroups < 0)
            return; /* hasGroups went negative → done */
#ifdef SWOS_SDL_BUILD
        if(exp_og_is_uefa() &&
           (compEx->minSubstitutes!=0u ||
            compEx->matchesPlayedCount==0u)) {
            /* The original career executor is allowed to walk every stage
             * at season-end. A resident world tick is NOT: if today's
             * native fixtures haven't completed this league stage, keep
             * it and its table resident. Previously it advanced straight
             * to the FINAL with zero OG matches. */
            if(s_expOGPhase==0 || s_expOGPhase%4==0)
                fprintf(stderr,"[og-uefa-stage] HOLD_LEAGUE phase=%d "
                               "round=%u played=%u sectionsLeft=%u viewPhase=%u\n",
                        s_expOGPhase,(unsigned)bufCopyBase->roundComplete,
                        (unsigned)compEx->matchesPlayedCount,
                        (unsigned)compEx->minSubstitutes,
                        (unsigned)compEx->base.viewRandomPhase);
            return;
        }
#endif
        DIY_SortLeagueTableByStandings();
        DIY_AdvanceToNextCompetitionSlot();
        if (bufCopyBase->currentRoundIndex == 0)
            goto restart; /* more slots remain */
    } else {
        /* Cup path */
        compEx->base.hasGroups = (int16_t)hasGroups;
        Career_AutoAdvanceCupMatches();
        if (compEx->base.hasGroups < 0)
            return; /* hasGroups went negative → done */
#ifdef SWOS_SDL_BUILD
        if(exp_og_is_uefa() && !compEx->base.competitionFinished) {
            /* Do not skip an unplayed European knockout round while the
             * world calendar is still before its native round dates. */
            if(s_expOGPhase==0 || s_expOGPhase%4==0)
                fprintf(stderr,"[og-uefa-stage] HOLD_KNOCKOUT phase=%d "
                               "round=%u played=%u viewPhase=%u\n",
                        s_expOGPhase,(unsigned)bufCopyBase->roundComplete,
                        (unsigned)compEx->matchesPlayedCount,
                        (unsigned)compEx->base.viewRandomPhase);
            return;
        }
#endif
        DIY_AdvanceToNextCompetitionSlot();
        if (bufCopyBase->currentRoundIndex == 0)
            goto restart; /* more slots remain */
    }
}


/* ================================================================
 * Chunk 196 — Career_SetCurrentCompetitionSlot
 * ================================================================ */

extern uint8_t *_careerCompetitionSlotPtrTable[];
extern volatile uint16_t _isNationalSelector;
extern void Career_ApplyForeignMarketPlayersToSelectedTeams(void);
extern void CopySelectedTeamNumbers(uint16_t count, uint8_t *dest);
extern void InitCompetitionFromRecord(uint16_t index, uint8_t *record);
extern void Career_LoadCompetitionSlotStateFromBuffer(uint8_t *src);
extern void Career_StoreCompetitionSlotStateToBuffer(uint8_t *dst);
extern void Career_InitCompetitionTeamSelection_Dispatch(uint8_t *spec, uint16_t idx);
extern void DIY_UpdateCompetitionProgressOrAdvanceSlot(uint8_t *blk);
extern void TeamSelectionKeyCache_BuildFromCompetitionBlock(uint8_t *blk);

/* ================================================================
 * Career_SetCurrentCompetitionSlot
 * (SWOS2.s line 31086)
 *
 * Sets up a competition slot for career mode:
 * 1) Stores comp pointer in currentCompetitionPtr[slot]
 * 2) Loads slot state from buffer, writes slot index to state
 * 3) Validates comp type byte against slot index (panics on mismatch)
 * 4) For slot 4 (DIY): fills diyCompetitionStart.cupTeamIds with career team id
 * 5) For other slots with hasSelectedTeams: copies team selections
 * 6) For other slots without: dispatches team selection + foreign market
 * 7) Initializes competition from record, stores state, builds key cache
 * 8) Unless slot 4: loads state again and updates competition progress
 *
 * Parameters:
 *   comp — pointer to competition data (a0)
 *   slot — competition slot index 0-4 (d0)
 *   divisionIndex — division index passed through to InitCompetitionFromRecord (d1, hidden param)
 *   hasSelectedTeams — flag indicating teams are pre-selected (d2)
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_SetCurrentCompetitionSlot(uint8_t *comp, uint16_t slot, uint16_t divisionIndex, uint16_t hasSelectedTeams)
{
    /* Store comp pointer in the named per-slot global. On Amiga
     * `currentCompetitionPtr` and its 4 siblings were 5 adjacent
     * uint32 BSS labels, so the original code walked them as
     * `((uint32_t *)&currentCompetitionPtr)[slot]`. We extracted them
     * into typed standalone pointers (Phase 6) — they are no longer
     * guaranteed to be adjacent, and the linker is free to put them
     * anywhere. Address each by name instead. */
    extern uint8_t *_secondaryCompetitionPtr;
    extern uint8_t *_tertiaryCompetitionPtr;
    extern uint8_t *_quaternaryCompetitionPtr;
    extern uint8_t *_diyCompetitionPtr;
    switch (slot) {
        case 0: _currentCompetitionPtr    = comp; break;
        case 1: _secondaryCompetitionPtr  = comp; break;
        case 2: _tertiaryCompetitionPtr   = comp; break;
        case 3: _quaternaryCompetitionPtr = comp; break;
        case 4: _diyCompetitionPtr        = comp; break;
        default:
            _panicReasonCode = 0x4459;  /* unknown slot index */
            PanicHaltLoop();
    }

    /* Get the slot state pointer from careerCompetitionSlotPtrTable.
     * +0x2C = compSlot/slotId word in DIYCompetition base. */
    uint8_t *slotState = _careerCompetitionSlotPtrTable[slot];
    *(uint16_t *)&((DIYCompetition *)slotState)->compSlot = slot;

    /* Load competition slot state from buffer */
    /* asm: exg a0,a1 → a0=slotState for the call, then exg back */
    Career_LoadCompetitionSlotStateFromBuffer(slotState);

    /* Validate comp type vs slot — panic if mismatch */
    const Competition *compHdr = (const Competition *)comp;
    int8_t compType = compHdr->type;
    if (slot == 0) {
        /* Slot 0: compType must NOT be 1 */
        if (compType == 1)
            goto panic_mismatch;
        goto validated;
    } else if (slot == 4) {
        /* Slot 4: compType must be 1 */
        if (compType != 1)
            goto panic_mismatch;
        goto validated;
    } else if (slot == 3) {
        goto validated;
    } else {
        /* Slots 1, 2: check isNationalSelector */
        if (_isNationalSelector) {
            if (compType == 0 || compType == 2)
                goto validated;
        }
        if (compType == 2 && slot == 2 && (uint8_t)compHdr->compId == COMPID_stateContest)
            goto validated;                     /* (a world's state championship: states.c) */
        /* Default: compType must be 1 */
        if (compType != 1)
            goto panic_mismatch;
        goto validated;
    }

panic_mismatch:
    _panicReasonCode = 0x4459;  /* 'DY' */
    PanicHaltLoop();

validated:
    /* Slot 4 special case: fill diyCompetitionStart.cupTeamIds with career team id */
    if (slot == 4) {
        uint16_t teamId = PACK_BE_U16(_careerTeam.team.countryNumber, _careerTeam.team.teamOrdinal);
        /* Slot 4 is validated as compType==1 (cup) above, so comp is a
         * CompetitionCup — byte 0xA = CompetitionCup.teams. */
        int16_t count = (int16_t)((const CompCupHeader *)comp)->teams;
        int16_t i = count - 1;
        while (i >= 0) {
            /* cupTeamIds is BE-packed {country, ordinal} (295c9f3 convention);
             * every reader uses BE_READ_U16 (e.g. DIY_BuildTeamSelectionPointerCache).
             * The Amiga `move.w d0,(a0)+` stored a big-endian word; a host-native
             * `cupTeamIds[i] = teamId` on an LE host stores the bytes swapped, so
             * the BE read came back byte-swapped (careerTeam 0x084d -> 0x4d08) and
             * VerifyTeamLoaded panicked while building the slot-4 (playoff) cup
             * bracket during season transition. Store BE to match every reader. */
            BE_WRITE_U16(&_diyCompetitionStart.cupTeamIds[i], teamId);
            i--;
        }
        goto init_competition;
    }

    /* Non-slot-4: handle team selection */
    if (hasSelectedTeams) {
        /* Copy team numbers from selection to appropriate team ID list */
        uint16_t *teamIdList;
        if (compType == 0) {
            teamIdList = _diyCompetitionStart.matchData.league.leagueTeamIds;
        } else if (compType == 1) {
            teamIdList = _diyCompetitionStart.cupTeamIds;
        } else {
            teamIdList = _diyFileBufferCopy.teamIds;
        }
        CopySelectedTeamNumbers(_numSelectedTeams, (uint8_t *)teamIdList);
    } else {
        /* Initialize team selection from competition spec
         * asm: jsr Career_InitCompetitionTeamSelection_Dispatch with d1=divisionIndex, a0=comp */
        Career_InitCompetitionTeamSelection_Dispatch(comp, divisionIndex);
        Career_ApplyForeignMarketPlayersToSelectedTeams();
    }

init_competition:
    /* Initialize competition from record — original asm does exg d0,d1 to pass
     * divisionIndex (originally in d1) as the first param to InitCompetitionFromRecord,
     * then exg back to restore d0=slot */
    InitCompetitionFromRecord(divisionIndex, comp);

    /* Store slot index to both diyFileBufferCopy and diyCompetitionStart.
     * +0x2C = compSlot/slotId word in DIYCompetition base. */
    *(uint16_t *)&_diyFileBufferCopy.header.compSlot = slot;
    *(uint16_t *)&_diyCompetitionStart.base.compSlot = slot;  /* writes compId+slotId as word */

    /* Write comp type to slot state header (sign-extended from int8). */
    int16_t typeWord = (int16_t)compType;
    *(uint16_t *)slotState = (uint16_t)typeWord;

    /* Store competition slot state to buffer */
    Career_StoreCompetitionSlotStateToBuffer(slotState);

    /* Build team selection key cache */
    TeamSelectionKeyCache_BuildFromCompetitionBlock(slotState);

    /* Slot 4 doesn't do the final load+progress step */
    if (slot == 4)
        return;

    /* Reload state and update competition progress */
    Career_LoadCompetitionSlotStateFromBuffer(slotState);
    if (compType == 1) {
        /* Stage-3 band-aid removal (d16ad5b, TEAMOFFSETTABLE_OVERLAY_UNTANGLE.md
         * §4). The teamOffsetTable rebuild before the season keycache fill is
         * REMOVED. It guarded the selTeams-index staleness — "the cup slot load
         * restores bracketScratch.teamOffsetTable (selTeams INDICES that go stale
         * when the careerFileBuffer overlay reorders selTeams), so the fill
         * resolves a slot to a duplicate team → two fixtures in one 1-game
         * keycache entry → walker desync → VerifyTeamLoaded garbage-id panic" —
         * but Part B retired that overlay, killing the staleness root (same as the
         * 8c660f4 removal). The §4 probe confirmed it a no-op 14/14 across many
         * cup/euro seasons, INCLUDING roundIterator (this site never save/restored
         * it, but the loaded slot's value already equals the Part-2 recompute, so
         * that was redundant too). Production no longer rebuilds; debug keeps a
         * NON-WRITING check (run the rebuild, diff teamOffsetTable + roundIterator,
         * restore both so debug == production; log if it ever WOULD change). */
#ifdef SWOS_DEBUG_AUTOSKIP
        extern void DIY_BuildTeamSelectionPointerCache(void);
        {
            int16_t ri_before = _diyCompetitionStart.base.roundIterator;
            int32_t tot_before[64];
            for (int i = 0; i < 64; i++)
                tot_before[i] = _diyCompetitionStart.bracketScratch.teamOffsetTable[i];
            DIY_BuildTeamSelectionPointerCache();
            int m = 0, first = -1;
            long new_first = 0;
            for (int i = 0; i < 64; i++)
                if (_diyCompetitionStart.bracketScratch.teamOffsetTable[i] != tot_before[i]) {
                    if (first < 0) {
                        first = i;
                        new_first = (long)_diyCompetitionStart.bracketScratch.teamOffsetTable[i];
                    }
                    m++;
                }
            int ri_new = _diyCompetitionStart.base.roundIterator;
            int ri_chg = (ri_new != ri_before);
            for (int i = 0; i < 64; i++)
                _diyCompetitionStart.bracketScratch.teamOffsetTable[i] = tot_before[i];
            _diyCompetitionStart.base.roundIterator = ri_before;
            if (m || ri_chg)
                SWOS_AUTOSKIP_LOG("[tot-rebuild-d16ad5b] REMOVED but rebuild WOULD CHANGE tot=%d/64 "
                                  "(idx %d %ld->%ld) roundIter(%d->%d) -> STALENESS BACK, revisit",
                                  m, first, (long)(first >= 0 ? tot_before[first] : 0), new_first,
                                  ri_before, ri_new);
            else
                SWOS_AUTOSKIP_LOG("[tot-rebuild-d16ad5b] band-aid REMOVED, no-rebuild state stays correct");
        }
#endif
    }
    DIY_UpdateCompetitionProgressOrAdvanceSlot(slotState);
}
