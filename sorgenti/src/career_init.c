#include "expanded_new_career.h"
/*
 * Amiga SWOS RE Project
 * SWOS Amiga C/68000 Reversed by Ben Baker (https://github.com/benbaker76)
 * SWOS DOS Reversed by Zlatko Karakaš (https://github.com/zlatkok)
 */
/* career_init.c — Career mode transfer market initialization.
 *
 * Chunk 61: 3 functions ported from SWOS2.s.
 */

#include "game_types.h"
/* Preserve the legacy byte-array externs in this translation unit.
 * Including competition_data.h conflicts with dozens of OG declarations.
 * The three European records are declared below as byte arrays. */
#include "league_playoffs.h"
#include "uefa_coef.h"
#include "world_conti.h"
#include "states.h"
#include "comp_identity.h"
#include <string.h>   /* memcpy in Rebuild_CupRecordNameSourcePointers */
#include "endian_helpers.h"
#include "menu_system.h"
#include "input_text.h"
#include "save_format_json.h"
#ifdef SWOS_SDL_BUILD
#include "diy_competition.h" /* DIY_OGStandings_SetSeason prototype */
#include "expanded_world_time.h"
#include "fixture_rounds.h"
#include "expanded_world_live.h"
#include "world.h"
#include "expanded_debug.h"
#include <stdio.h>
#endif
#include "random.h"
#include "string_utils.h"
#include "panic.h"
#include "auto_skip_debug.h"
#ifdef SWOS_INJECT_WC
#include <stdio.h>   /* fprintf — SWOS_INJECT_WC debug log only */
#endif

/* ---- External globals (all in SWOS2.s BSS) ---- */
extern uint16_t _numPlayersOnTransferMarket;
extern uint16_t _numForeignMarketPlayers2;
extern uint16_t _transferMarketRandOffset0;
extern uint16_t _transferMarketRandOffset1;
extern uint16_t _transferMarketRandOffset2;
extern uint16_t _transferMarketPhase;
extern uint16_t _timeToNegotiate;
extern uint16_t _numOffers;
extern uint16_t _transferPoolSlotsUsed;
extern uint16_t _offerSwapSlotsUsed;
extern uint16_t _lastOfferPrimaryPlayerOrdinal;
extern uint16_t _offerNegotiationAttemptCount;
extern uint16_t _careerSeasonsCompleted;
#include "career_params.h"
#include "league_groups.h"
#include "team_pool.h"
#include "world_playoff.h"
#include "world_winners.h"
#include "world_cups.h"
#include "world_cupd.h"
extern uint16_t _currentSeasonShowing;
extern uint16_t _seasonTransferQuota;
extern uint16_t _pendingForeignPlayerCount;
extern uint16_t _offerSwapSlotLimit;
extern uint16_t _transferPoolSlotLimit;
extern uint16_t _homeMarketUpdateBacklog;
extern uint16_t _foreignMarketUpdateBacklog;
extern uint16_t _offerExpiryIterationsRemaining;
extern uint16_t _removedOfferPlayersFromSelectedTeamCount;
extern uint16_t _removedOfferPlayersFromOtherTeamsCount;

/* careerCupPickList: 5-entry array of COMPETITION POINTERS (national-coach
 * cup pick list). Pointer-sized entries — was extern'd uint32_t (truncating
 * the 64-bit pointers); now a standalone void *[5] (see swos_data_typed_stubs.c). */
extern void *_careerCupPickList[];
extern uint8_t _invitationTournamentCopy[];
extern uint8_t _tourCopy[];

/* Globals for manager setup menu (Chunk 110) */
extern MenuEntry _menuEntries[];
extern uint16_t _coachOrPlayer;
extern uint16_t _mrOrMs;
extern uint8_t _managerSexBuffer[];
extern uint8_t _aMr[];           /* "MR" */
extern uint8_t _aMs[];           /* "MS" */

/* Globals for Career_InitializeSelectedTeamAndStartingBalance (Chunk 170) */
extern int32_t  _startingBalance;
extern uint16_t _teamNoOfferAccepted;
extern uint16_t _stayingInOldClub;
extern uint16_t _careerTeamLeague;
extern uint16_t _selTeamNumber;
extern TeamFileEx _careerTeam;
extern uint8_t  _currentMatchPlayers[];
extern uint16_t _teamsLoaded;
extern int32_t  _newBalance;
extern uint16_t _careerNumPlayers;
extern uint16_t _sensisoftPlcSponsorshipFactor;
extern uint16_t _additionalInvestments;
extern uint16_t _isNationalSelector;

/* ================================================================
 * Career_InitTransferMarketState  (SWOS2.s line 72836)
 *
 * Clears transfer market counters and generates three random
 * offsets (0..15) used during transfer market initialization.
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_InitTransferMarketState(void)
{
    _numPlayersOnTransferMarket = 0;
    _numForeignMarketPlayers2 = 0;
    _transferMarketRandOffset1 = Rand2() & 0xF;
    _transferMarketRandOffset2 = Rand2() & 0xF;
    _transferMarketPhase = 0;
    _transferMarketRandOffset0 = Rand2() & 0xF;
}

/* ================================================================
 * Career_InitSeasonTransferMarketState  (SWOS2.s line 72853)
 *
 * Initializes per-season transfer market state: negotiation time,
 * offer counts, pool quotas, and backlog counters.
 *
 * Season transfer quota formula:
 *   numerator = 432 - numForeignMarketPlayers2 - (7 - seasons) * 16
 *   divisor   = 20 - currentSeasonShowing
 *   quota     = (uint32_t)(int32_t)(int16_t)numerator / divisor
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_InitSeasonTransferMarketState(void)
{
    _timeToNegotiate = 6;
    _numOffers = 0;
    _transferPoolSlotsUsed = 0;
    _offerSwapSlotsUsed = 0;
    _lastOfferPrimaryPlayerOrdinal = 0xFFFF;
    _offerNegotiationAttemptCount = 0;

    /* Compute season transfer quota */
    uint16_t seasonsAdj = (uint16_t)(7 - _careerSeasonsCompleted) * 16;  /* mulu #$10 */
    uint16_t divisor = _careerParams.num_seasons - _currentSeasonShowing;
    int16_t numerator = (int16_t)(0x1B0 - _numForeignMarketPlayers2 - seasonsAdj);
    /* ext.l + divu.w: sign-extend to 32-bit then unsigned divide by word */
    uint16_t quota = (uint16_t)((uint32_t)(int32_t)numerator / divisor);

    _seasonTransferQuota = quota;
    _pendingForeignPlayerCount = 0;

    uint16_t half = quota >> 1;   /* lsr.w #1 */
    _offerSwapSlotLimit = half;
    _transferPoolSlotLimit = quota - half;   /* neg.w of (half - quota) */

    _homeMarketUpdateBacklog = 0;
    _foreignMarketUpdateBacklog = 0;
    _offerExpiryIterationsRemaining = 0;
    _removedOfferPlayersFromSelectedTeamCount = 0;
    _removedOfferPlayersFromOtherTeamsCount = 0;
}

/* ================================================================
 * Career_RandomizeInvitationTournamentCup  (SWOS2.s line 35893)
 *
 * Scans the 5-entry careerCupPickList for entries pointing to
 * invitationTournamentCopy. For each such entry, 50% chance to
 * replace it with tourCopy (randomized via Rand).
 * ================================================================ */
__attribute__((used, externally_visible))
void Career_RandomizeInvitationTournamentCup(void)
{
    void **pickList = _careerCupPickList;
    int16_t i;

    for (i = 4; i >= 0; i--) {
        void *p = pickList[i];
        /* Entries are competition-copy pointers or the table sentinels 0 / -1 / -2;
         * only dereference real pointers for the id check (the original compared the
         * raw pointer, which was sentinel-safe). The replacement stores the actual
         * tour competition object, so it keeps referencing &_tourCopy. */
        if (p != 0 && p != (void *)(uintptr_t)-1 && p != (void *)(uintptr_t)-2 &&
            comp_id(p) == COMPID_invitationTournament) {
            if (Rand() & 1)
                pickList[i] = (void *)_tourCopy;
        }
    }
}

/* ================================================================
 * Manager setup menu callbacks (Chunk 110)
 * Asm: SetPlayerCoachFieldColor, CheckSex, ChangeSex,
 *      SetName, SetSurname, CoachOrPlayerCoach
 * ================================================================ */

/* SetPlayerCoachFieldColor: update background color palette index
 * for menu entry 9 based on coachOrPlayer toggle.
 * The low nibble of the bg color word holds the color palette index. */
__attribute__((used, externally_visible))
void SetPlayerCoachFieldColor(void)
{
    MenuEntry *e = &_menuEntries[9];
    e->bgColor = (e->bgColor & 0xFFF0) | 13; /* BRIGHT_BLUE_TO_BLUE_13 */
    if (_coachOrPlayer != 0)
        e->bgColor = (e->bgColor & 0xFFF0) | 11; /* BLUE_TO_PURPLE_11 */
}

/* CheckSex: copy "MR" or "MS" to managerSexBuffer based on mrOrMs toggle */
__attribute__((used, externally_visible))
void CheckSex(void)
{
    uint8_t *src = _aMr;
    if (_mrOrMs != 0)
        src = _aMs;
    StringCopy(src, _managerSexBuffer);
}

/* ChangeSex: toggle mrOrMs, update sex string, redraw current menu item.
 * entry = a5 from menu system (current MenuEntry). */
__attribute__((used, externally_visible))
void ChangeSex(MenuEntry *entry)
{
    _mrOrMs ^= 1;
    CheckSex();
    CallDrawMenuItem(entry);
}

/* SetName: start text input for first name field (max 9 chars).
 * entry = a5 from menu system; InputText reads entry->fg itself. */
__attribute__((used, externally_visible))
void SetName(MenuEntry *entry)
{
    extern void InputText_IgnoreCareerOpeningFire(void);
    InputText_IgnoreCareerOpeningFire();
    InputText(9, (uint8_t *)entry);
}

/* SetSurname: start text input for surname field (max 13 chars).
 * entry = a5 from menu system; InputText reads entry->fg itself. */
__attribute__((used, externally_visible))
void SetSurname(MenuEntry *entry)
{
    extern void InputText_IgnoreCareerOpeningFire(void);
    InputText_IgnoreCareerOpeningFire();
    InputText(13, (uint8_t *)entry);
}

/* CoachOrPlayerCoach: toggle coach/player-coach mode, update color, redraw.
 * entry = a5 from menu system (current MenuEntry). */
__attribute__((used, externally_visible))
void CoachOrPlayerCoach(MenuEntry *entry)
{
    _coachOrPlayer ^= 1;
    SetPlayerCoachFieldColor();
    CallDrawMenuItem(entry);
}

/* ================================================================
 * Chunk 148: ChooseTeamAndStartCareer, LoadLiverpool
 * ================================================================ */

extern uint8_t _contestMenu[];
extern void ActivateMenu(uint8_t *menu);
extern void SetExitMenuFlag(void);

/* Forward declarations for Chunk 197 functions (defined later in this file) */
void InitCareer(uint16_t teamNo);
void Career_BeginNewCareer(void);
#ifdef SWOS_SDL_BUILD
int Expanded_OGWorld_Start(void);
int Expanded_OGWorld_AdvanceToDate(ExpandedWorldDate today);
int Expanded_OGWorld_ClubBound(void);
#endif
void InitCareerWorld(void);
void InitCareerManager(void);
void InitCareerClub(uint16_t teamNo);
void Career_AttachManagerToClub(uint16_t teamNo);
void ContinueCareerNextSeason(void);
void InitializeNewSeason(void);
void Expanded_InitializeClubSeasonState(void);
void DIY_SelectCompetitionSlot4FromCurrentDivision(void);
uint16_t Career_SelectEuroCupSlotIfTeamParticipates(uint8_t *cupCopy, uint16_t euroCupIndex);

extern uint16_t _selectedTeamCountryNumber;
extern TeamFileEx *_selectedTeamPtr;
void SelectTeamToManage(void);

/* ChooseTeamAndStartCareer: let user pick a team, then start career.
 * Called as menu onSelect callback. */
__attribute__((used, externally_visible))
void ChooseTeamAndStartCareer(void)
{
    Career_BeginNewCareer();
    /* OG world templates and resets are now initialized once above. */
    _teamsLoaded = 0;
#ifdef SWOS_SDL_BUILD
    /* START on NEW CAREER is the birth of the world, independently of
     * whether the manager starts employed or unemployed. Never recreate
     * the ledger when an offer is accepted: contracts/club changes only
     * change the manager's place in the existing world.
     * Import is done here, before any modal menu or team selection, because
     * the adapter must briefly load TEAM files into the shared buffer. */
    {
        extern void ExpandedWorldLive_LoadOfferCountries(void);
        ExpandedWorldLive_LoadOfferCountries();
        if (!ExpandedWorldLive_Clubs()) {
            extern uint16_t _fireResetFlag;
            fprintf(stderr,"[living-world] FAILED to start new career world "
                           "at START; no league snapshot available\n");
            _fireResetFlag=1;
            return;
        }
        fprintf(stderr,"[living-world] START imported %lu clubs, "
                       "%lu league records, %lu LWE fixtures\n",
                (unsigned long)ExpandedWorldLive_Clubs(),
                (unsigned long)ExpandedWorldLive_Leagues(),
                (unsigned long)ExpandedWorldLive_Fixtures());
        /* THE new career's OG world is initialized HERE, before any
         * manager has chosen or accepted a team. */
        if(!Expanded_OGWorld_Start()) {
            extern uint16_t _fireResetFlag;
            fprintf(stderr,"[og-world] START FAILED: no native league "
                           "contexts available; refusing empty career\n");
            _fireResetFlag=1;return;
        }
    }
#endif
    /* Expanded 0.8: zero-fame managers start without a club. The waiting
     * screen is outside the career engine; no uninitialised team state can
     * leak into fixtures or transfer processing. */
    extern uint8_t _expandedManagerFame;
#ifdef SWOS_SDL_BUILD
    /* Setup choice is independent from fame; never overwrite the manager scalar. */
    if (Expanded_NewCareer_Unemployed()) {
#else
    if (_expandedManagerFame == 0) {
#endif
        extern int Expanded_WaitForFirstOffer(uint16_t *team);
        uint16_t offerTeam = 0;
        if (!Expanded_WaitForFirstOffer(&offerTeam)) {
#ifdef SWOS_SDL_BUILD
            /* Setup cancelled: discard unsaved pilot from RAM only.
             * Never touch an existing AUTOSAVE.CAR on disk. */
            ExpandedWorldLive_End();
#endif
            /* BACK cancels only the unemployed calendar. It must return
             * to the previous career setup screen, not unwind the whole
             * menu stack to the main menu. ShowMenu already restored it. */
            extern uint16_t _fireResetFlag;
            _fireResetFlag = 1;
            return;
        }
        fprintf(stderr,"[expanded11] InitCareer: starting team=%04x\n",(unsigned)offerTeam);
        fflush(stderr);
        InitCareer(offerTeam);
        /* InitCareer creates a fresh native 1996/97 season. An offer after
         * years of unemployment must use the CURRENT Living World season;
         * archived seasons stay archived. This occurs before OG cup replay. */
        { extern int Expanded_MidseasonJoin_AlignSeason(uint32_t);
          extern unsigned Expanded_DebugUnemployedWeeks(void);
          if (!Expanded_MidseasonJoin_AlignSeason(
                  Expanded_DebugUnemployedWeeks())) {
              fprintf(stderr,"[midseason] cannot align OG to living-world year; "
                             "not starting a career in the wrong season\n");
              extern uint16_t _fireResetFlag;
              _fireResetFlag=1;
              return;
          }
        }
        extern void Expanded_CommitNegotiatedSalary(void);
        Expanded_CommitNegotiatedSalary();
        /* The actual midseason hire is NOT a fresh first-week contest:
         * fast-forward the native competition states before human input. */
        extern void Expanded_MidseasonJoin_Begin(uint32_t weeks);
        extern unsigned Expanded_DebugUnemployedWeeks(void);
#ifdef SWOS_SDL_BUILD
        if(Expanded_OGWorld_ClubBound())
            fprintf(stderr,"[og-world] accepted club uses existing OG table; "
                           "legacy midseason replay SKIPPED\n");
        else
#endif
            Expanded_MidseasonJoin_Begin(Expanded_DebugUnemployedWeeks());
#ifdef SWOS_SDL_BUILD
        /* A failed OG handoff is not an employed career. In particular,
         * don't overwrite the one autosave with a freshly zeroed table.
         * A new attempt can be made from setup; the old save is intact. */
        { extern int Expanded_MidseasonJoin_LastResult(void);
          if (Expanded_MidseasonJoin_LastResult()==2) {
              fprintf(stderr,"[midseason] JOIN FAILED: original standings "
                             "not reconciled; autosave NOT overwritten\n");
              extern uint16_t _fireResetFlag;
              _fireResetFlag=1;
              return;
          }
        }
        /* DO NOT save/reload while the OG cup/league catch-up cursor is
         * active. VERIFY builds reload the just-saved .CAR and reconstruct
         * competition slot pointers; that invalidates the running native
         * callback chain. The successful catch-up finish() already saves
         * "native-join-complete". Without catch-up, save immediately. */
        { extern int Expanded_MidseasonJoin_Active(void);
          if (!Expanded_MidseasonJoin_Active()) {
              extern void Expanded_AutoCareer_Save(const char *);
              Expanded_AutoCareer_Save("first-contract");
          } else {
              fprintf(stderr,"[autosave] deferred first-contract until "
                             "native-join-complete (OG catch-up active)\n");
          }
        }
#endif
        fprintf(stderr,"[expanded11] InitCareer: completed team=%04x\n",(unsigned)offerTeam);
        fflush(stderr);
    } else {
        SelectTeamToManage();
        if (_selectedTeamPtr == 0) {
#ifdef SWOS_SDL_BUILD
            ExpandedWorldLive_End(); /* No signed contract yet. */
            extern uint16_t _fireResetFlag;
            _fireResetFlag=1; /* Cancel team selection returns to setup. */
#else
            SetExitMenuFlag();
#endif
            return;
        }
        InitCareer(_selectedTeamCountryNumber);
#ifdef SWOS_SDL_BUILD
        { extern void Expanded_AutoCareer_Save(const char *);
          Expanded_AutoCareer_Save("first-contract"); }
#endif
    }
    ActivateMenu(_contestMenu);
}

/* LoadLiverpool: debug/cheat — start career with Liverpool (team 0x811).
 * Called as menu onSelect callback. */
__attribute__((used, externally_visible))
void LoadLiverpool(void)
{
    _coachOrPlayer = 1;
    Career_BeginNewCareer();
    InitCareer(0x811);
#ifdef SWOS_SDL_BUILD
    { extern void Expanded_AutoCareer_Save(const char *);
      Expanded_AutoCareer_Save("debug-new-career"); }
#endif
    ActivateMenu(_contestMenu);
}

/* ================================================================
 * Chunk 149: ContinueCareer  (SWOS2.s line 55140)
 *
 * If season == 19, show retirement message + reset career variables.
 * Otherwise advance to next season and re-activate contest menu.
 * ================================================================ */

extern void ShowRetirementMessage(void);
extern void InitCareerVariables(uint16_t gameTypeParam);

__attribute__((used, externally_visible))
void ContinueCareer(void)
{
    extern void Expanded_CompleteSeason(void);
    extern int Expanded_ShouldRetire(void);
    extern void Expanded_RecordRetirement(void);
    Expanded_CompleteSeason();
    if (Expanded_ShouldRetire()) {
        Expanded_RecordRetirement();
        ShowRetirementMessage();
        InitCareerVariables(0);
#ifdef SWOS_SDL_BUILD
        { extern void Expanded_AutoCareer_Delete(void);
          Expanded_AutoCareer_Delete(); }
#endif
        SetExitMenuFlag();
        return;
    }
    if (_currentSeasonShowing == _careerParams.num_seasons - 1) {
        Expanded_RecordRetirement();
        ShowRetirementMessage();
        InitCareerVariables(0);
#ifdef SWOS_SDL_BUILD
        { extern void Expanded_AutoCareer_Delete(void);
          Expanded_AutoCareer_Delete(); }
#endif
        SetExitMenuFlag();
    } else {
        ContinueCareerNextSeason();
#ifdef SWOS_SDL_BUILD
        { extern void Expanded_AutoCareer_Save(const char *);
          Expanded_AutoCareer_Save("new-season"); }
#endif
        ActivateMenu(_contestMenu);
    }
}

/* ================================================================
 * Chunk 151: InitCompetitionTeamSelection functions
 *
 * These initialize team selection state for career competition setup.
 * They all follow a similar pattern:
 *   1. Clear careerFileBuffer
 *   2. Call LoadSomeEuroCup
 *   3. Clear numSelectedTeams
 *   4. Call Career_InitCompetitionTeamSelectionFrom*Spec
 *   5. Read numNeededTeams from the competition record
 *   6. Return 0
 * ================================================================ */
extern uint16_t _numSelectedTeams;
extern uint16_t _careerFileBuffer;
extern uint16_t _numNeededTeams;

extern void LoadSomeEuroCup(uint8_t *whichCup);
extern void Career_InitCompetitionTeamSelectionFromTableEntrySpec(uint8_t *base, uint16_t idx);
extern void Career_InitCompetitionTeamSelectionFromTableHeaderSpec(uint8_t *base);
extern void Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec(uint8_t *base);

__attribute__((used, externally_visible))
uint16_t InitCompetitionTeamSelectionFromTableEntry(uint16_t idx, uint8_t *base)
{
    _careerFileBuffer = 0;
    LoadSomeEuroCup(base);
    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromTableEntrySpec(base, idx);

    /* numNeededTeams = record byte at offset 0xD in idx'th 6-byte entry */
    uint16_t offset = idx * 6;
    _numNeededTeams = (int8_t)base[0xD + offset];
    return 0;
}

__attribute__((used, externally_visible))
uint16_t InitCompetitionTeamSelectionFromTableHeader(uint8_t *base)
{
    _careerFileBuffer = 0;
    LoadSomeEuroCup(base);
    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromTableHeaderSpec(base);

    _numNeededTeams = (int8_t)base[0xA];
    return 0;
}

__attribute__((used, externally_visible))
uint16_t InitCompetitionTeamSelectionFromAltTableHeader(uint8_t *base)
{
    _careerFileBuffer = 0;
    LoadSomeEuroCup(base);
    _numSelectedTeams = 0;
    Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec(base);

    _numNeededTeams = (int8_t)base[0xF];
    return 0;
}

/* ================================================================
 * Career_InitializeSelectedTeamAndStartingBalance (SWOS2.s line 34621)
 *
 * Sets up the career team data by copying the selected team's file
 * data to both careerTeam and selTeamsPtr buffers, sets the
 * coach/player control byte, copies player order to currentMatchPlayers,
 * and initializes financial globals.
 *
 * Asm: d0=teamNumber (uint16), d1=startingBalance (int32)
 * ================================================================ */
extern int16_t GetTeamLeague(uint16_t teamNumber);
extern TeamFileEx *GetTeamPointer(uint16_t teamId);

__attribute__((used, externally_visible))
void Career_InitializeSelectedTeamAndStartingBalance(uint16_t teamNumber, int32_t balance)
{
    _startingBalance = balance;
    _teamNoOfferAccepted = 0xFFFF;
    _stayingInOldClub = 0xFFFF;
    _numSelectedTeams = 0;

    /* Get team league for career tracking */
    _careerTeamLeague = (uint16_t)GetTeamLeague(teamNumber);

    /* Get pointer to team file data and copy to both buffers */
    TeamFileEx *teamPtr = GetTeamPointer(teamNumber);
    _selTeamNumber = teamNumber;

    /* Copy 342 words (684 bytes = one TeamFileEx) to both careerTeam and selTeamsPtr */
    uint16_t *src = (uint16_t *)teamPtr;
    uint16_t *dst1 = (uint16_t *)&_careerTeam;
    uint16_t *dst2 = (uint16_t *)&_selTeamsPtr;
    for (int16_t i = 341; i >= 0; i--) {
        *dst1++ = *src;
        *dst2++ = *src++;
    }

    _numSelectedTeams = 1;

    /* Set teamNameControl byte (offset 4) based on coach-or-player setting:
     * 2 = coach only, 3 = player-coach */
    uint8_t controlByte = (_coachOrPlayer != 0) ? 2 : 3;
    _careerTeam.team.teamControls = controlByte;
    _selectedTeams->team.teamControls = controlByte;

    /* Copy 16 bytes of playerNumbers (TeamFile.playerNumbers) to currentMatchPlayers */
    teamPtr = GetTeamPointer(_selTeamNumber);
    for (int16_t i = 15; i >= 0; i--) {
        _currentMatchPlayers[i] = teamPtr->team.playerNumbers[i];
    }

    _careerNumPlayers = 16;
    _teamsLoaded = 0;
    _newBalance = 0;
    _sensisoftPlcSponsorshipFactor = 0;
    _additionalInvestments = 0;
}

/* ================================================================
 * Career_ContinueToNextSeasonInitTeamAndFunds (SWOS2.s line 72514)
 *
 * Prepares for a new career season: clears transfer market, increments
 * season counter, optionally rebuilds foreign market pool, applies a
 * random ±jitter (80-104%) to the base funds, then initializes the
 * team with the adjusted balance.
 *
 * Asm: d0=teamNumber (uint16), d1=baseFunds (uint32)
 * ================================================================ */
extern void Career_BuildForeignMarketPoolFromSelectedTeam(void);
extern uint16_t Rand2(void);

__attribute__((used, externally_visible))
void Career_ContinueToNextSeasonInitTeamAndFunds(uint16_t teamNumber, uint32_t baseFunds)
{
    _numPlayersOnTransferMarket = 0;
    _careerSeasonsCompleted++;

    if (_isNationalSelector == 0) {
        Career_BuildForeignMarketPoolFromSelectedTeam();
    }

    /* Apply random jitter to funds: 80-104% of original
     * Formula: ((baseFunds / 1000) * (80 + rand25pct)) / 100 * 1000
     * where rand25pct = (Rand2() * 25) >> 8 = 0..24 */
    uint16_t fundsDiv = (uint16_t)(baseFunds / 1000);
    uint16_t rand = Rand2();
    uint16_t jitterPct = ((rand * 25) >> 8) + 80;
    uint32_t adjusted = (uint32_t)fundsDiv * (uint32_t)jitterPct;
    uint16_t adjustedDiv = (uint16_t)(adjusted / 100);
    uint32_t finalFunds = (uint32_t)adjustedDiv * 1000;

    Career_InitializeSelectedTeamAndStartingBalance(teamNumber, (int32_t)finalFunds);
}

/* ================================================================
 * Chunk 197: InitCareer, ContinueCareerNextSeason,
 *   Career_SelectEuroCupSlotIfTeamParticipates,
 *   DIY_SelectCompetitionSlot4FromCurrentDivision,
 *   InitializeNewSeason, InitNewSeason
 * ================================================================ */

/* --- External globals (Chunk 197) --- */
extern uint16_t _worldCupHostTeam;
extern uint8_t  _diyLeagueSeasonStateTable[];
extern uint8_t  _diyLeagueScratchBuffer[];
extern uint16_t _sameNationalCallFlag;
extern uint16_t _boardConfidenceHistory[4];
extern uint16_t _boardConfidenceScore;
extern uint16_t _gamesPlayed;
extern uint16_t _numWins;
extern uint16_t _numDrawn;
extern uint16_t _numLost;
extern uint16_t _goalsScored;
extern uint16_t _goalsTaken;
extern uint16_t _careerReputationPoints;
extern uint16_t _winCupBonusCount;
extern uint16_t _minorTrophyBonusCount;
extern uint16_t _majorTrophyBonusCount;
extern uint16_t _playPromotedAnimation;
extern uint16_t _playRelegatedAnimation;
extern uint16_t _previousBoardConfidence;
extern uint16_t _playWinCupAnimation;
extern uint16_t _minorTrophyBonusActive;
extern uint16_t _leagueTitleBonusActive;
extern uint16_t _majorTrophyBonusActive;
extern uint16_t _cupBonus1Active;
extern uint16_t _cupBonus2Active;
extern uint16_t _wonWorldCup;
extern uint16_t _wonContinentalCup;
extern uint16_t _qualifyingStage1PendingFlag0;
extern uint16_t _qualifyingStage1PendingFlag1;
extern uint16_t _qualifyingStage2PendingFlag0;
extern uint16_t _qualifyingStage2PendingFlag1;
extern uint16_t _wonInvitationTournament;

extern uint8_t  _euroCup[];
extern uint8_t  _cupWinnersCup[];
extern uint8_t  _uefaCup[];

/* careerContests table — typed CompetitionCareer[] entries
 * {src_ptr, dest_ptr, byte_count} terminated by `competition ==
 * (CompetitionCup *)0xFFFFFFFFu`. Was `extern uint8_t careerContests[]`
 * with byte-stride walks (10 B per entry); on 64-bit each entry is
 * 18 B so consumers iterate the typed array instead. */
extern CompetitionCareer _careerContests[];

/* primaryCompSlotStateA/B/C migrated to primaryCompetitionBuf.tourn.header.slotState* (slot-0 untangle, S6) */
/* secondaryCompSlotStateA/B/C migrated to competitionContextBuf.tourn.header.slotState* (slot-1 untangle, S4) */
/* tertiaryCompSlotStateA/B/C migrated to tertiaryCompetitionContextBuf.tourn.header.slotState* (slot-2 untangle, S3) */
/* quaternaryCompSlotStateA/B/C migrated to euroCupsCompetitionContextBuf.tourn.header.slotState* (slot-3 untangle, S5) */
extern uint8_t  _seasonPlaying;

extern uint16_t _poolplyrLoaded;

extern void DIY_LoadLeagueScratchFromTable(void);
extern uint32_t CalculateStartingBalance(uint16_t teamNumber);

/* --- InitializeNewSeason externs --- */
extern uint16_t _selTeamSelectedPlayerPosition;
extern uint8_t  _careerUnavailablePlayerList[];
extern uint16_t _retired;
extern uint16_t _youreSacked;
extern uint16_t _competitionSlotUpdatedFlag;
extern uint16_t _boardConfidenceMod0;
extern uint16_t _boardConfidenceMod1;
extern uint16_t _boardConfidenceMod2;
extern uint16_t _boardConfidenceMod3;
extern uint16_t _boardConfidenceValue0;
extern uint16_t _boardConfidenceValue1;
extern uint16_t _boardConfidenceMod4;
extern uint16_t _boardConfidenceMod5;
extern uint16_t _boardConfidenceMod6;
extern uint16_t _boardConfidenceMod7;
extern uint16_t _boardConfidenceMod8;
extern uint16_t _boardConfidenceMod9;
extern uint16_t _currentDivisionIndex;
extern uint16_t _contractNegotiationActive;
extern uint16_t _removedPlayerStatAccum;
extern uint16_t _teamSelectionKeyCacheEnd;
extern uint16_t _isCareer;
extern uint16_t _careerMatchSimulatedFlag;
extern uint16_t _numNewJobOffers;
extern uint16_t _numJobOffers;
extern uint16_t _careerTeamLostMatch;
extern volatile uint16_t _currentMessage;
extern uint8_t  _careerLastMatchIndexByCompetitionSlot[];
extern uint8_t *_currentCompetitionPtr;
extern uint8_t *_secondaryCompetitionPtr;
extern uint8_t *_tertiaryCompetitionPtr;
extern uint8_t *_quaternaryCompetitionPtr;
extern uint8_t *_diyCompetitionPtr;
extern CompetitionSlotBuffer _primaryCompetitionBuf;          /* slot-0 untangle (S6): typed buffer */
extern CompetitionSlotBuffer _competitionContextBuf;          /* slot-1 untangle (S4): typed buffer */
extern CompetitionSlotBuffer _tertiaryCompetitionContextBuf;   /* slot-2 untangle (S3): typed buffer */
extern CompetitionSlotBuffer _euroCupsCompetitionContextBuf;   /* slot-3 untangle (S5): typed buffer */
extern CompetitionSlotBuffer _diyCompetitionContextBuf;   /* slot-4 untangle (S2): typed buffer */
extern uint8_t *_careerCompetitionSlotPtrTable[];
extern uint16_t _teamSelectionKeyCacheCount;
extern uint16_t _teamSelectionKeyCacheIndex;
extern uint16_t _teamSelectionKeyCacheOffset;
extern uint16_t _careerSlotProcessingInProgress;
extern uint16_t _lastNationalityCall;
extern void    *_careerTeamFlagsByRegionTable[];  /* 6-entry pointer table; native width on 32 and 64-bit. Storage is `void *[6]` in swos_amiga_ptr_imports.c. */
extern uint8_t  _someTeamFlags;
extern uint8_t  _nationalTeamNationality;
extern uint8_t  _nationalTeamFileId;
extern uint16_t _worldCupQualifyingTeam;
extern uint16_t _euroChampionshipQualTeam;
extern uint16_t _seasonCycleNationalFlagsA[];
extern uint16_t _seasonCycleNationalFlagsB[];
extern uint8_t *_primaryCompStringOffset;
extern uint8_t *_secondaryCompStringOffset;
extern uint8_t *_tertiaryCompStringOffset;
extern uint8_t *_quaternaryCompStringOffset;
extern uint8_t  _euroCupCopy[];
extern uint8_t  _cupWinnersCupCopy[];
extern uint8_t  _uefaCupCopy[];
extern void *_competitionsTable[];
/* careerCupPickList: declared at top of file (line 39) */
extern uint16_t _euroCupSlotIndex;
extern uint32_t *_diyCompetitionTablePtr;
extern uint8_t  _careerJobOfferPhaseBias;
extern uint8_t  _viewRandomPhase;
extern uint8_t  _lastFinancialWeekIndex;
extern uint16_t _wageScalingDivisor;
extern uint16_t _leagueFirstRoundNumber;
extern uint16_t _seasonFinished;
extern uint16_t _careerFinished;
extern uint8_t  _managerNameBuffer[];
extern uint8_t  _managerSurnameBuffer[];
extern uint16_t _seasonNumberOfSquads;
extern uint16_t _seasonSquadTeamNumbers[];
extern uint16_t _careerLoadedFlag;
extern uint8_t  _teamSelectionKeyCache[];
extern uint32_t _teamFundsAccepted;
extern uint8_t  _nationalTeamFileCode;

extern uint8_t *GetCurrentSeasonPointer(void);
extern void DIY_SaveLeagueScratchToTable(void);
extern TeamFileEx *VerifyTeamLoaded(uint16_t teamNo);
extern void InitTeams(void);
extern void FillForeignMarketPlayers(void);
extern void FillCareerTeamPlayers(void);
extern void Career_InitializeNewSeasonFinances(void);
extern void InitializeCareerSeasonFinancials(void);
extern void ShowEnjoyLastSeasonMessage(void);
extern void ShowWishYouSuccessMessage(void);
extern void ShowLastSeasonMessage(void);
extern void ShowGreetingMessage(void);
extern void ShowLeavingMessage(void);
extern void TrainingMatch_AddTrialPlayersForNewSeason(void);
extern void Career_InitInternationalSquadAndLookupTablesForNewSeason(TeamFileEx *teamPtr);
extern void Career_SetCupPickList_ForSelectedRegion(uint16_t *teamList);
extern uint16_t Career_UpdateLastNationalityCall(void);
extern void InitializeNewSeason_BuildTeamBudgetRankTable(void);
extern void Career_SetCurrentCompetitionSlot(uint8_t *comp, uint16_t slot, uint16_t divisionIndex, uint16_t hasSelectedTeams);
extern void ShowChairmanOpinionAboutTheSeason(void);
extern void InitializeStartingBalanceForNewSeason(void);
extern void Career_AdvanceToNextSeason_UpdateCompetitions(void);
extern uint32_t GetNationalityAndTeamFile(uint16_t teamNo);

/* ================================================================
 * InitCareer  (SWOS2.s line 31722)
 *
 * Initializes a new career: clears season state, calls transfer market
 * init, calculates starting balance, copies competition data, sets
 * slot states, then calls InitializeNewSeason.
 * ================================================================ */
/* World, manager and club are different lifetimes. In particular, an
 * unemployment-to-employment handoff must NEVER re-copy the OG competition
 * templates or erase the career's historical counters. */
static int s_careerWorldInitialized = 0;
static int s_careerCompetitionsInitialized = 0;
static int s_careerManagerInitialized = 0;
static uint32_t s_unemployedOgCupSeasons = 0;

__attribute__((used, externally_visible))
void InitCareerWorld(void)
{
    /* An explicit new-career entry creates a new world, once. It is never
     * invoked by a later job offer, a dismissal or a save reload. */
    if(s_careerWorldInitialized) return;
    _worldCupHostTeam = 20994;

    /* Clear 2000 bytes of diyLeagueSeasonStateTable (volatile to prevent memset optimization) */
    {
        volatile uint8_t *p = _diyLeagueSeasonStateTable;
        for (int16_t i = GLOBAL_TEAM_NUMBERS - 1; i >= 0; i--)
            *p++ = 0;
    }
    TeamPool_Reset();
    s_careerWorldInitialized = 1;
}

/* Original SWOS's competition tables/counters are a separate lifetime
 * from the world data and the manager.  Rehiring must NOT reset them.
 * This does not install another fixture calendar or run a second engine. */
__attribute__((used, externally_visible))
void InitCareerCompetitions(void)
{
    if (s_careerCompetitionsInitialized) return;
    if (!s_careerWorldInitialized) InitCareerWorld();
    WorldCups_Reset();
    CupD_Reset();
    UefaCoef_Reset();
    LeaguePlayoffs_ResetStages();
    WorldPlayoff_ResetStageTitles();
    WorldPlayoff_Reset();
    WorldWinners_Reset();
    Conti_Reset();
    States_Reset();

    DIY_LoadLeagueScratchFromTable();

    LoadSomeEuroCup(_euroCup);
    LoadSomeEuroCup(_cupWinnersCup);
    LoadSomeEuroCup(_uefaCup);

    /* Copy careerContests table entries: {src, dest, count} until
     * src == 0xFFFFFFFF. Iterate as the typed CompetitionCareer[]
     * — on 64-bit each entry is 18 bytes (two 8-byte pointers + uint16);
     * the old byte-stride walk assumed 10-byte entries and would slip
     * 8 bytes between each iteration. Volatile cursors prevent LTO
     * from eliminating the ROM→BSS copies (same pattern as the tactic
     * copy bug). */
    extern CompetitionCareer _careerContests[];
    for (CompetitionCareer *e = _careerContests;
         (uintptr_t)e->competition != 0xFFFFFFFFu; e++) {
        volatile uint8_t *src = (volatile uint8_t *)e->competition;
        volatile uint8_t *dst = (volatile uint8_t *)e->competitionCopy;
        uint16_t count = e->templateSize;
        for (int16_t i = count - 1; i >= 0; i--)
            *dst++ = *src++;
    }

    s_careerCompetitionsInitialized = 1;
}


#ifdef SWOS_SDL_BUILD
/* Coordinator of native OG LEAGUES: separate from the human manager.
 * League contexts are built at START, and Advanced to the real date.
 * This is not the old LWE scheduler and does not synthesize scores. */
extern void Expanded_OGWorld_Reset(void);
extern void Expanded_OGWorld_ResetSeason(void);
extern uint32_t Expanded_OGWorld_LeagueCount(void);
extern int Expanded_OGWorld_AdvanceLeague(uint8_t *,uint8_t,uint8_t,uint16_t,uint8_t);
extern int Expanded_OGWorld_BindClub(uint16_t,uint8_t,uint16_t);
extern int Expanded_OGWorld_ClubBound(void);
static int s_expOGWorldStarted;
static uint16_t s_expOGWorldYear;

/* These are native cup DEFINITIONS, not resident cups. Current s_expOG
 * instances cover leagues only. Old end-of-season callbacks are counted
 * separately, never as live knockout competitions. */
void Expanded_OGWorld_CupAudit(ExpandedOGCupAudit *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    out->completed_year_callbacks=s_unemployedOgCupSeasons;
    const uint8_t *countries=World_SeasonCountries();
    if(!countries) {
        extern uint8_t _careerCompetitionIdRemapTable[];
        countries=_careerCompetitionIdRemapTable;
    }
    if(!countries)return;
    extern uint32_t Expanded_OGWorld_CupCount(void);
    out->resident_cups=Expanded_OGWorld_CupCount();
    for(unsigned i=0;i<256u && countries[i]!=0xffu;++i) {
        const void *const *tab=(const void *const *)_competitionsTable[countries[i]];
        unsigned found=0;
        if(!tab)continue;
        for(unsigned j=2u;j<=3u;++j) {
            const uintptr_t raw=(uintptr_t)tab[j];
            /* -1 terminates a country table; some countries have only
             * three pointer slots. Do not read tab[3] past that array. */
            if(!raw || raw==(uintptr_t)-1)break;
            if(raw==(uintptr_t)-2)continue;
            if(((const Competition *)raw)->type!=1)continue;
            ++out->domestic_definitions;
            ++found;
        }
        if(found)++out->domestic_countries;
    }
}
static void expanded_og_world_log_audit(const char *stage,uint16_t year,
                                       uint8_t phase)
{
    ExpandedOGWorldAudit leagues;
    ExpandedOGCupAudit cups;
    Expanded_OGWorld_Audit(&leagues);
    Expanded_OGWorld_CupAudit(&cups);
    fprintf(stderr,"[og-world-audit] stage=%s year=%u phase=%u "
                   "resident-leagues=%lu played-leagues=%lu "
                   "completed-leagues=%lu native-matches=%lu invalid=%lu "
                   "countries=%lu club-bound=%lu cup-definitions=%lu "
                   "cup-countries=%lu cup-resident=%lu yearend-callbacks=%lu\n",
            stage,(unsigned)year,(unsigned)phase,
            (unsigned long)leagues.resident_leagues,
            (unsigned long)leagues.leagues_with_matches,
            (unsigned long)leagues.completed_leagues,
            (unsigned long)leagues.native_matches,
            (unsigned long)leagues.invalid_contexts,
            (unsigned long)leagues.countries,
            (unsigned long)leagues.club_bound,
            (unsigned long)cups.domestic_definitions,
            (unsigned long)cups.domestic_countries,
            (unsigned long)cups.resident_cups,
            (unsigned long)cups.completed_year_callbacks);
}
/* Checkpoint at START, before/after stress, and across every original
 * OG season transition. A cup record is NOT a resident cup fixture; its
 * absence remains explicit. Player counters are read from OG team files. */
/* Read-only inventory of OG's EXISTING international/national definitions
 * and subsystem state. This intentionally does NOT instantiate tournaments,
 * advance rounds or equate static definitions with resident competition
 * contexts. It establishes which pieces exist before we test full fixtures. */
static void expanded_og_external_inventory(const char *stage)
{
    extern Competition *_nationalTeamCompetitionsAndQualifications[39];
    unsigned references=0,unique=0,qualifiers=0,finals=0;
    uint8_t seen[256]={0};
    for(unsigned i=0;i<39u;++i) {
        const uintptr_t p=(uintptr_t)_nationalTeamCompetitionsAndQualifications[i];
        if(!p || p==(uintptr_t)-1)break;
        const Competition *c=(const Competition *)p;
        const uint8_t id=(uint8_t)c->compId;
        ++references;
        if(!seen[id]) {
            ++unique;
            seen[id]=1u;
            if(comp_is_wc_qualifier(id)||comp_is_ec_qualifier(id))++qualifiers;
            if(id==COMPID_worldCup || id==COMPID_worldCup2 ||
               id==COMPID_europeanChampionships ||
               comp_is_continental_cup(id))++finals;
        }
        if(stage && !strcmp(stage,"START"))
            fprintf(stderr,"[og-external-definition] kind=NATIONAL "
                           "index=%u compId=%u type=%d "
                           "state=DEFINITION_ONLY\n",i,(unsigned)id,(int)c->type);
    }
    int32_t worldCupDefinitions=0;
    (void)World_Cups(&worldCupDefinitions);
    fprintf(stderr,"[og-external-audit] stage=%s world=%s year=%u "
                   "national-references=%u national-unique=%u "
                   "qualifier-definitions=%u final-definitions=%u "
                   "club-continental-templates=3 world-cup-definitions=%ld "
                   "uefa-init=%u uefa-done=%u uefa-clubs=%u "
                   "conti-mark=%u conti-season=%u "
                   "world-cup-season-counter=%u "
                   "national-resident=NOT_PROVEN "
                   "international-resident=NOT_PROVEN\n",
            stage?stage:"UNNAMED",World_Tag(),(unsigned)s_expOGWorldYear,
            references,unique,qualifiers,finals,(long)worldCupDefinitions,
            (unsigned)g_uefaCoef.init,(unsigned)g_uefaCoef.done,
            (unsigned)g_uefaCoef.nClubs,
            (unsigned)CT_ST.mark,(unsigned)CT_ST.season,
            (unsigned)g_worldCups.seasons);

    /* Named inventory of every OG external-cup family, kept separate from
     * domestic resident type-1 cups. A definition or a nonzero native
     * subsystem counter is NOT evidence of dated fixtures/qualification.
     * Never initialize/simulate a tournament from an audit. */
    {
        const int modern=World_Current()==WORLD_2627;
        const char *const europe[3]={
            modern?"CHAMPIONS_LEAGUE":"EUROPEAN_CHAMPIONS_CUP",
            modern?"EUROPA_LEAGUE":"CUP_WINNERS_CUP",
            modern?"CONFERENCE_LEAGUE":"UEFA_CUP"
        };
        const uint8_t *const records[3]={_euroCup,_cupWinnersCup,_uefaCup};
        for(unsigned k=0;k<3u;++k)
            fprintf(stderr,"[og-external-competition] stage=%s "
                           "category=UEFA_CLUB name=%s compId=%u defined=%u "
                           "activity=%u fixture-proof=NOT_VERIFIED\n",
                    stage?stage:"UNNAMED",europe[k],k,
                    (unsigned)(comp_id(records[k])==(uint8_t)k),
                    (unsigned)g_uefaCoef.phaseEnded[k]);
        {
            extern Competition *Comp_ById(uint8_t);
            const int copa=Comp_ById(COMPID_copaLibertadores)!=NULL;
            fprintf(stderr,"[og-external-competition] stage=%s "
                           "category=CONMEBOL_CLUB name=COPA_LIBERTADORES "
                           "compId=%u defined=%u activity=%u "
                           "fixture-proof=NOT_VERIFIED\n",
                    stage?stage:"UNNAMED",COMPID_copaLibertadores,
                    (unsigned)copa,(unsigned)CT_ST.season);
        }
        fprintf(stderr,"[og-external-competition] stage=%s "
                       "category=CONTINENTAL_CLUB name=OTHER_CONTINENTAL_CLUB_CUPS "
                       "compId=243 defined=%u activity=%u fixture-proof=NOT_VERIFIED\n",
                stage?stage:"UNNAMED",CT_SPEC!=NULL,
                (unsigned)CT_ST.season);
        /* Inventory *each* original 2026/27 extra-European competition,
         * not an opaque OTHER bucket. Source of truth remains World_ContiCups
         * and World_ContiSpec (the Mod's own CT_SPEC), never a new fixture
         * generator. A record may be defined without being OG-resident. */
        {
            int32_t nConti=0;
            const WorldContiCup *conti=World_ContiCups(&nConti);
            for(int32_t c=0;conti && c<nConti;++c) {
                fprintf(stderr,"[og-external-cup-inventory] stage=%s "
                               "category=CONTINENTAL_CLUB index=%ld name=%s "
                               "defined=%u spec=%u resident-fixtures=NOT_PROVEN "
                               "advance=NOT_CONNECTED\n",
                        stage?stage:"UNNAMED",(long)c,
                        conti[c].name?conti[c].name:"(UNNAMED)",
                        (unsigned)(conti[c].name!=NULL),
                        (unsigned)(World_ContiSpec()!=NULL));
            }
        }
        static const struct { const char *name; uint8_t id; } national[]={
            {"WORLD_CUP",COMPID_worldCup},
            {"EUROPEAN_CHAMPIONSHIPS",COMPID_europeanChampionships},
            {"COPA_AMERICA",COMPID_copaAmerica},
            {"AFRICAN_NATIONS_CUP",COMPID_africanNationsCup},
            {"NORTH_AMERICAN_CUP",COMPID_northAmericanCup},
            {"ASIAN_CUP",COMPID_asianCup},
            {"OCEANIA_CUP",COMPID_oceaniaCup}
        };
        for(unsigned k=0;k<sizeof(national)/sizeof(national[0]);++k)
            fprintf(stderr,"[og-external-competition] stage=%s "
                           "category=NATIONAL name=%s compId=%u defined=%u "
                           "activity=0 fixture-proof=NOT_VERIFIED\n",
                    stage?stage:"UNNAMED",national[k].name,
                    (unsigned)national[k].id,(unsigned)seen[national[k].id]);
        fprintf(stderr,"[og-external-competition] stage=%s "
                       "category=NATIONAL name=WORLD_CUP_QUALIFIERS compId=5 "
                       "defined=%u activity=0 fixture-proof=NOT_VERIFIED\n",
                stage?stage:"UNNAMED",(unsigned)(qualifiers>0));
        fprintf(stderr,"[og-external-competition] stage=%s "
                       "category=NATIONAL name=EURO_QUALIFIERS compId=23 "
                       "defined=%u activity=0 fixture-proof=NOT_VERIFIED\n",
                stage?stage:"UNNAMED",(unsigned)(seen[COMPID_ecQualifyingGroup1]!=0));
    }
}
void Expanded_OGWorld_LogCheckpoint(const char *stage)
{
    ExpandedOGWorldAudit leagues;
    ExpandedOGCupAudit cups;
    ExpandedOGScorerAudit scorers;
    Expanded_OGWorld_Audit(&leagues);
    Expanded_OGWorld_CupAudit(&cups);
    Expanded_OGWorld_ScorerAudit(&scorers);
    fprintf(stderr,"[og-feature-audit] stage=%s year=%u "
                   "resident-leagues=%lu native-matches=%lu "
                   "cup-definitions=%lu cup-resident=%lu "
                   "cup-matches=%lu cup-unsupported=%lu cup-faults=%lu "
                   "scorer-players=%lu scorer-positive=%lu "
                   "scorer-league-goals=%lu standings-goals=%lu "
                   "scorer-leagues-with-goals=%lu\n",
            stage?stage:"UNNAMED",(unsigned)s_expOGWorldYear,
            (unsigned long)leagues.resident_leagues,
            (unsigned long)leagues.native_matches,
            (unsigned long)cups.domestic_definitions,
            (unsigned long)cups.resident_cups,
            (unsigned long)Expanded_OGWorld_CupMatches(),
            (unsigned long)Expanded_OGWorld_CupUnsupported(),
            (unsigned long)Expanded_OGWorld_CupFaults(),
            (unsigned long)scorers.players,
            (unsigned long)scorers.players_with_league_goals,
            (unsigned long)scorers.player_league_goals,
            (unsigned long)scorers.standings_goals,
            (unsigned long)scorers.leagues_with_goals);
    expanded_og_external_inventory(stage);
    /* Each actual definition is compared with its native resident state.
     * This reports unavailable future/extra cups without claiming a PASS. */
    extern void Expanded_OGWorld_LogNoData(const char *,uint16_t);
    Expanded_OGWorld_LogNoData(stage?stage:"UNNAMED",s_expOGWorldYear);
    /* Cross-process fingerprint: pointer-free native OG bytes, rosters,
     * standings, fixture scratch and RNG. Only SAVE/RESTORE emit records. */
    extern void Expanded_OGWorld_LogSaveFingerprints(const char *);
    Expanded_OGWorld_LogSaveFingerprints(stage?stage:"UNNAMED");
}
extern int Expanded_OGWorld_AdvanceCup(uint8_t *,uint8_t,uint8_t,uint16_t,uint8_t);
extern uint32_t Expanded_OGWorld_CupCount(void);
extern uint32_t Expanded_OGWorld_CupMatches(void);
extern uint32_t Expanded_OGWorld_CupUnsupported(void);
extern uint32_t Expanded_OGWorld_CupFaults(void);
extern int Expanded_OGWorld_AdvanceUefa(uint16_t,uint8_t);
extern uint32_t Expanded_OGWorld_UefaResidentCount(void);
static int expanded_og_world_run_year(uint16_t year,uint8_t phase) {
    const uint8_t *countries=World_SeasonCountries();
    if(!countries) {
        extern uint8_t _careerCompetitionIdRemapTable[];
        countries=_careerCompetitionIdRemapTable;
    }
    if(!countries)return 0;
    unsigned ok=0,skipped=0,failed=0;
    unsigned cups=0,cupsSkipped=0,cupsFailed=0;
    /* The reported 96/97 START crash predates this build and has no
     * current Windows call stack. Trace the exact original country/slot
     * reached before the crash, without altering the OG fixture engine. */
    if(phase==0u && World_Current()==WORLD_9697)
        fprintf(stderr,"[og-9697-start] ENTER year=%u mode=%s\n",
                (unsigned)year,_selTeamNumber==0xffffu?"UNEMPLOYED":"EMPLOYED");
    for(unsigned i=0;i<256u && countries[i]!=0xffu;++i) {
        const uint8_t country=countries[i];
        unsigned countryCupDefs=0u,countryCupResidents=0u;
        unsigned countryCupUnsupported=0u,countryCupFailed=0u;
        const void *const *tab=(const void *const *)_competitionsTable[country];
        if(!tab)continue;
        uintptr_t raw=(uintptr_t)tab[0];
        if(raw && raw!=(uintptr_t)-1 && raw!=(uintptr_t)-2) {
        const Competition *rec=(const Competition *)raw;
        if(rec->type==0 && !comp_is_type0_cup(rec->compId)) {
        const CompetitionLeague *league=(const CompetitionLeague *)rec;
        unsigned divisions=(unsigned)league->header.divisionsCount;
        if(divisions>0u && divisions<=16u) {
        for(unsigned d=0;d<divisions;++d) {
            if(!FixtureRounds_Real(league->divisions[d].teams)) {
                ++skipped;continue;
            }
            extern unsigned Expanded_OGStress_Remaining(void);
            if(Expanded_OGStress_Remaining() ||
               (phase==0u && World_Current()==WORLD_9697))
                fprintf(stderr,"[og-world-step] BEGIN year=%u phase=%u "
                               "country=%u division=%u\n",
                        (unsigned)year,(unsigned)phase,
                        (unsigned)country,d);
            const int leagueStatus=Expanded_OGWorld_AdvanceLeague(
                (uint8_t *)rec,country,(uint8_t)d,year,phase);
            if(leagueStatus>0) {
                ++ok;
                if(Expanded_OGStress_Remaining())
                    fprintf(stderr,"[og-world-step] END year=%u phase=%u "
                                   "country=%u division=%u\n",
                            (unsigned)year,(unsigned)phase,
                            (unsigned)country,d);
            } else if(leagueStatus<0) {
                ++skipped; /* explicitly unsupported format, never claim played */
                if(phase==0u)
                    fprintf(stderr,"[og-world] LEAGUE_UNSUPPORTED "
                                   "country=%u division=%u year=%u "
                                   "native-context=NOT_RESIDENT\n",
                            (unsigned)country,d,(unsigned)year);
            } else {
                ++failed;
                fprintf(stderr,"[og-world] LEAGUE ERROR country=%u "
                               "division=%u year=%u phase=%u\n",
                        (unsigned)country,d,(unsigned)year,(unsigned)phase);
            }
        }
        } /* supported division count */
        } /* genuine league record */
        } /* valid country table first entry */
        /* Domestic cup slots used by the employed OG career; no complete
         * season callback, no retroactive results, no LWE fixture engine. */
        for(unsigned j=2u;j<=3u;++j) {
            const uintptr_t rawCup=(uintptr_t)tab[j];
            if(!rawCup || rawCup==(uintptr_t)-1)break;
            if(rawCup==(uintptr_t)-2)continue;
            const Competition *cup=(const Competition *)rawCup;
            if(cup->type!=1)continue;
            ++countryCupDefs;
            if(phase==0u && World_Current()==WORLD_9697)
                fprintf(stderr,"[og-9697-start] CUP_BEGIN country=%u slot=%u id=%u\n",
                        (unsigned)country,j,(unsigned)cup->compId);
            const int status=Expanded_OGWorld_AdvanceCup(
                (uint8_t *)cup,country,(uint8_t)j,year,phase);
            if(status>0){++cups;++countryCupResidents;}
            else if(status<0){++cupsSkipped;++countryCupUnsupported;}
            else {
                ++countryCupFailed;
                ++cupsFailed;
                fprintf(stderr,"[og-cup-resident] ERROR country=%u "
                               "slot=%u compId=%u year=%u phase=%u\n",
                        (unsigned)country,j,(unsigned)cup->compId,
                        (unsigned)year,(unsigned)phase);
            }
            /* Audit each ORIGINAL domestic cup definition by country/slot,
             * not by non-unique compId. This only reads existing OG setup:
             * an undersized native team selection is NOT filled by a
             * separate simulator or silently counted as supported. */
            if(phase==0u) {
                extern unsigned Expanded_OGWorld_CupSelectedTeams(
                    uint8_t,uint8_t,uint16_t);
                extern const char *Expanded_OGWorld_CupCoverageReason(
                    uint8_t,uint8_t,uint16_t,unsigned);
                const unsigned expected=(unsigned)((const CompCupHeader *)cup)->teams;
                const unsigned selected=Expanded_OGWorld_CupSelectedTeams(
                    country,(uint8_t)j,year);
                const char *reason=Expanded_OGWorld_CupCoverageReason(
                    country,(uint8_t)j,year,expected);
                fprintf(stderr,"[og-cup-coverage] country=%u slot=%u "
                               "compId=%u year=%u expected=%u selected=%u "
                               "state=%s reason=%s\n",
                        (unsigned)country,j,(unsigned)cup->compId,
                        (unsigned)year,expected,selected,
                        status>0?"RESIDENT":status<0?"UNSUPPORTED":"FAILED",
                        reason);
                /* Important: a real-world cup's entrant list can contain
                 * fewer teams than the OG bracket because byes and padded
                 * grid slots are native SWOS mechanics. Read metadata only;
                 * WorldCups_Entrants() mutates the seed list, so NEVER call
                 * it from an audit. A valid bye format is NOT a fixture PASS.
                 * Other underfilled selectors remain unexplained. */
                int32_t num=0;
                const WorldCup *rules=World_Cups(&num);
                const WorldCup *matched=NULL;
                for(int32_t k=0;rules && k<num;++k) {
                    if(rules[k].country==country && rules[k].record==(const void *)cup) {
                        matched=&rules[k];
                        break;
                    }
                }
                const unsigned real=matched?(unsigned)matched->n:0u;
                const unsigned ties=matched?(unsigned)matched->ties:0u;
                extern void Expanded_OGWorld_CupSetupAudit(
                    uint8_t,uint8_t,uint16_t,unsigned *,unsigned *,unsigned *);
                unsigned nativeBracket=0,nativeTies=0,nativePairs=0;
                Expanded_OGWorld_CupSetupAudit(country,(uint8_t)j,year,
                    &nativeBracket,&nativeTies,&nativePairs);
                const int byeCandidate=matched && real==selected &&
                    real>=2u && real<=expected && ties*2u<=real &&
                    expected>real;
                const int roundMatchesRule=byeCandidate &&
                    nativeBracket==expected && nativeTies==ties &&
                    nativePairs>=nativeTies;
                fprintf(stderr,"[og-cup-rule] country=%u slot=%u "
                               "compId=%u year=%u bracket=%u selected=%u "
                               "real-entrants=%u first-round-ties=%u "
                               "byes=%u padded-slots=%u "
                               "native-bracket=%u native-round1-ties=%u "
                               "native-round1-pairs=%u "
                               "classification=%s "
                               "round-structure=%s "
                               "native-validation=%s "
                               "fixtures=NOT_VERIFIED_BY_RULE\n",
                        (unsigned)country,j,(unsigned)cup->compId,
                        (unsigned)year,expected,selected,real,ties,
                        matched && ties*2u<=real?real-ties*2u:0u,
                        matched && real<=expected?expected-real:0u,
                        nativeBracket,nativeTies,nativePairs,
                        byeCandidate?"OG_BYE_FORMAT_CANDIDATE":
                        matched?"OG_WORLD_CUP_FORMAT_UNCONFIRMED":
                        selected<expected?"UNEXPLAINED_UNDERFILL":
                        selected>expected?"UNEXPLAINED_OVERFILL":
                        "NO_SPECIAL_RULE",
                        roundMatchesRule?"OG_ROUND1_MATCHES_RULE":
                        "NOT_MATCHED",
                        status>0 && !strcmp(reason,"NATIVE_BYES_VERIFIED")?
                            "NATIVE_DRAW_IDENTITIES_VERIFIED":
                        status>0?"STANDARD_NATIVE_BRACKET":
                        "UNVERIFIED");
            }
        }
        /* Full per-country coverage at native setup: countries with a
         * single cup and countries with a second league cup both count.
         * A missing context is visible rather than guessed from metadata. */
        if(phase==0u && countryCupDefs)
            fprintf(stderr,"[og-cup-country] year=%u country=%u "
                           "definitions=%u resident=%u unsupported=%u "
                           "failed=%u initial=1\n",
                    (unsigned)year,(unsigned)country,
                    countryCupDefs,countryCupResidents,
                    countryCupUnsupported,countryCupFailed);
    }
    /* Two real OG UEFA eras, never the same records with new names:
     * 96/97 = historic Champions type 2, CWC/UEFA Cup type 1.
     * 26/27 = three native 36-team league-phase type 2 tournaments.
     * Later first-year successors need the original UEFA qualification
     * handoff; do NOT silently replay the first year's seed list. */
    int uefaOk=1;
#if SWOS_WORLDS
    /* The Mod's 2026/27 starting field is not the 96/97 Copa.
     * Never replay pre-career qualifiers or import an external EXE. */
    if(World_Current()==WORLD_2627 && year==World_FirstYear() && phase==0u) {
        const int ready=ct_world_prepare_first();
        fprintf(stderr,"[og-international-s1] year=%u source=NO_EXTERNAL_EXE "
                       "first-entries=%s dated-fixtures=%s\n",
                (unsigned)year,
                ready?"NATIVE_VERIFIED":"NEEDS_INDEPENDENT_FIELD",
                ready?"NATIVE_CONTEXT":"NOT_BOUND");
    }
#endif
    if(year==World_FirstYear() &&
       (World_Current()==WORLD_2627 || World_Current()==WORLD_9697)) {
        const int modern=World_Current()==WORLD_2627;
        uefaOk=Expanded_OGWorld_AdvanceUefa(year,phase);
        if(!modern) {
            const int cw=Expanded_OGWorld_AdvanceCup(
                (uint8_t *)&_cupWinnersCup,254u,1u,year,phase);
            const int uc=Expanded_OGWorld_AdvanceCup(
                (uint8_t *)&_uefaCup,254u,2u,year,phase);
            if(cw!=1 || uc!=1)
                fprintf(stderr,"[og-classic-europe] CUPS_UNAVAILABLE "
                               "year=%u phase=%u cwc=%d uefa=%d\n",
                        (unsigned)year,(unsigned)phase,cw,uc);
        }
        fprintf(stderr,"[og-international-resident] world=%u "
                       "year=%u phase=%u uefa-resident=%lu expected=%u "
                       "status=%s continental=VIEWER_NOT_BOUND\n",
                (unsigned)World_Current(),(unsigned)year,(unsigned)phase,
                (unsigned long)Expanded_OGWorld_UefaResidentCount(),
                modern?3u:1u,uefaOk?"NATIVE_ATTEMPT":"NATIVE_ERROR");
    } else if(phase==0u)
        fprintf(stderr,"[og-international-resident] world=%u "
                       "year=%u phase=0 status=PENDING_ORIGINAL_QUALIFICATION_HANDOFF\n",
                (unsigned)World_Current(),(unsigned)year);
    if(phase==0u)
        fprintf(stderr,"[og-cup-start] year=%u definitions=%u "
                       "resident=%u unsupported=%u failed=%u "
                       "visibility=%s scope=DOMESTIC_OG_TYPE1\n",
                (unsigned)year,cups+cupsSkipped+cupsFailed,
                cups,cupsSkipped,cupsFailed,
                cupsSkipped||cupsFailed?"PARTIAL":"ALL_RESIDENT");
    /* Same native season/phase as every league, once for each country's
     * OG type-1 cup record. Cup IDs are NOT unique worldwide, so the
     * resident native states are indexed by country + table slot + year. */
    fprintf(stderr,"[og-cup-resident] YEAR year=%u phase=%u "
                   "advanced=%u unsupported=%u failed=%u resident=%lu "
                   "played=%lu faults=%lu\n",
            (unsigned)year,(unsigned)phase,cups,cupsSkipped,cupsFailed,
            (unsigned long)Expanded_OGWorld_CupCount(),
            (unsigned long)Expanded_OGWorld_CupMatches(),
            (unsigned long)Expanded_OGWorld_CupFaults());
    fprintf(stderr,"[og-world] year=%u phase=%u native-leagues=%u "
                   "skipped=%u persistent=%lu (manager independent)\n",
            (unsigned)year,(unsigned)phase,ok,skipped,
            (unsigned long)Expanded_OGWorld_LeagueCount());
    if(failed||cupsFailed)
        fprintf(stderr,"[og-world] WORLD TICK FAILED: %u leagues, "
                       "%u cups; clock must not advance\n",failed,cupsFailed);
    /* A non-resident international must be reported, but must not erase a
     * correctly initialized domestic OG world or abort a player's career.
     * The Windows diagnostics will explicitly mark the UEFA integration
     * INCOMPLETE until all three have real fixture evidence. */
    if(!uefaOk)
        fprintf(stderr,"[og-international-resident] WARNING year=%u phase=%u "
                       "uefa=INCOMPLETE domestic-world-preserved\n",
                (unsigned)year,(unsigned)phase);
    return ok!=0 && failed==0 && cupsFailed==0;
}
int Expanded_OGWorld_Start(void) {
    /* This explicit boundary prevents consecutive careers in one SWOS.log
     * from being misclassified as a native calendar rewind. */
    fprintf(stderr,"[og-career] RESET new-career-start\n");
    Expanded_OGWorld_Reset();
    s_expOGWorldStarted=0;
    s_expOGWorldYear=World_FirstYear();
    DIY_OGStandings_SetSeason(s_expOGWorldYear);
    /* START establishes the resident OG world before the manager can be
     * employed or unemployed. Phase zero only initializes the native
     * contexts; it is NOT the real 1 July career start. Use the already
     * established civil world date, without creating a second clock.
     * The same dated OG routine will skip completed native phases on
     * the first FORWARD TIME (via each context's lastPhase). */
    s_expOGWorldStarted=expanded_og_world_run_year(s_expOGWorldYear,0);
    if(!s_expOGWorldStarted)return 0;
    ExpandedLiveCalendarState initial;
    if(!ExpandedWorldLive_CalendarState(&initial) ||
       initial.today.year!=s_expOGWorldYear ||
       initial.today.month!=7u || initial.today.day!=1u) {
        fprintf(stderr,"[og-world] START FAILED: missing/non-July-1 "
                       "calendar for world year=%u; no fallback date\n",
                (unsigned)s_expOGWorldYear);
        s_expOGWorldStarted=0;
        Expanded_OGWorld_Reset();
        return 0;
    }
    if(!Expanded_OGWorld_AdvanceToDate(initial.today)) {
        fprintf(stderr,"[og-world] START FAILED: native competitions could "
                       "not catch up to %04u-%02u-%02u\n",
                (unsigned)initial.today.year,(unsigned)initial.today.month,
                (unsigned)initial.today.day);
        s_expOGWorldStarted=0;
        Expanded_OGWorld_Reset();
        return 0;
    }
    uint8_t phase=0;
    (void)ExpandedWorld_DateToNativePhaseNext(s_expOGWorldYear,
                                              initial.today,&phase);
    fprintf(stderr,"[og-world] START READY: date=%04u-%02u-%02u "
                   "native-phase=%u resident-leagues=%lu "
                   "(all native league results through START date)\n",
            (unsigned)initial.today.year,(unsigned)initial.today.month,
            (unsigned)initial.today.day,(unsigned)phase,
            (unsigned long)Expanded_OGWorld_LeagueCount());
    expanded_og_world_log_audit("START",s_expOGWorldYear,phase);
    Expanded_OGWorld_LogCheckpoint("START");
    return 1;
}
int Expanded_OGWorld_AdvanceToDate(ExpandedWorldDate today) {
    if(!s_expOGWorldStarted)return 0;
    const uint16_t year=(uint16_t)(today.year-(today.month<7u?1u:0u));
    if(year<s_expOGWorldYear || year>2198u)return 0;
    while(s_expOGWorldYear<year) {
        const uint16_t closedYear=s_expOGWorldYear;
        if(!expanded_og_world_run_year(closedYear,56u))return 0;
        ExpandedOGWorldAudit closing;
        Expanded_OGWorld_Audit(&closing);
        /* Audit the SAME original OG competition contexts before resetting.
         * These are real played matches; no synthetic season score tables. */
        fprintf(stderr,"[og-season-transition] CLOSE year=%u "
                       "resident-leagues=%lu played-leagues=%lu "
                       "native-matches=%lu invalid=%lu\n",
                (unsigned)closedYear,
                (unsigned long)closing.resident_leagues,
                (unsigned long)closing.leagues_with_matches,
                (unsigned long)closing.native_matches,
                (unsigned long)closing.invalid_contexts);
        Expanded_OGWorld_LogCheckpoint("SEASON_CLOSE");
        if(!closing.resident_leagues || !closing.leagues_with_matches ||
           !closing.native_matches || closing.invalid_contexts) {
            fprintf(stderr,"[og-season-transition] FAIL CLOSE year=%u "
                           "empty/invalid OG state\n",(unsigned)closedYear);
            return 0;
        }
        ++s_expOGWorldYear;
        /* Keep a prepared OG continental edition across July while the
         * domestic league states enter their new career season. */
        Expanded_OGWorld_ResetSeason();
        DIY_OGStandings_SetSeason(s_expOGWorldYear);
        if(!expanded_og_world_run_year(s_expOGWorldYear,0))return 0;
        ExpandedOGWorldAudit opening;
        Expanded_OGWorld_Audit(&opening);
        fprintf(stderr,"[og-season-transition] OPEN year=%u "
                       "resident-leagues=%lu invalid=%lu\n",
                (unsigned)s_expOGWorldYear,
                (unsigned long)opening.resident_leagues,
                (unsigned long)opening.invalid_contexts);
        Expanded_OGWorld_LogCheckpoint("SEASON_OPEN");
        if(!opening.resident_leagues || opening.invalid_contexts) {
            fprintf(stderr,"[og-season-transition] FAIL OPEN year=%u "
                           "missing/invalid OG league states\n",
                    (unsigned)s_expOGWorldYear);
            return 0;
        }
    }
    uint8_t cutoff=0;
    if(!ExpandedWorld_DateToNativePhaseNext(year,today,&cutoff))return 0;
    const int ok=expanded_og_world_run_year(year,cutoff);
    if(ok)expanded_og_world_log_audit("TICK",year,cutoff);
    return ok;
}
#endif

/* Explicit NEW career. Do not reset the manager or world on a job offer. */
__attribute__((used, externally_visible))
void Career_BeginNewCareer(void)
{
#ifdef SWOS_SDL_BUILD
    extern void Expanded_ClockTrace_Reset(void);
    Expanded_ClockTrace_Reset();
#endif
    s_careerWorldInitialized = 0;
    s_careerCompetitionsInitialized = 0;
    s_careerManagerInitialized = 0;
    s_unemployedOgCupSeasons = 0;
#ifdef SWOS_SDL_BUILD
    s_expOGWorldStarted=0;
    Expanded_OGWorld_Reset();
    DIY_OGStandings_SetSeason(World_FirstYear());
#endif
    InitCareerWorld();
    InitCareerCompetitions();
    InitCareerManager(); /* native manager setup; never on a later offer */
}
#ifdef SWOS_SDL_BUILD
/* Native OG domestic cups during unemployed completed seasons: use OG
 * match callbacks instead of inventing another scoring engine.
 * Not a replacement for dated knockout rounds, UEFA/national competitions
 * or native promotion/cup qualification handoffs. */
extern void Career_AutoAdvanceCupCompetitionFromRecord(uint8_t *record);
extern void Career_AutoAdvanceInternationalCompetitionFixtures(uint8_t *record,uint16_t division);
extern void SaveSelectedTeams(void);
extern void RestoreSelectedTeams(void);
extern uint16_t _internationalAutoAdvanceFlag;
extern uint8_t _seasonPlaying;
void Career_UnemployedCupAudit(uint32_t *advanced,uint32_t *native,
                                int *initialized) {
    if(advanced)*advanced=s_unemployedOgCupSeasons;
    if(native)*native=(uint32_t)g_worldCups.seasons;
    if(initialized)*initialized=s_careerWorldInitialized;
}
void Career_AdvanceUnemployedOGCups(uint32_t elapsedWeeks)
{
    const uint32_t finished=elapsedWeeks/52u;
    /* Native cups have already been initialized and dated by the SAME OG
     * world as the leagues at START and FORWARD TIME. The legacy backup
     * replays complete seasons and is NEVER valid on top of resident OG,
     * particularly after F12+8 (2096 would replay from 2026). */
    if(s_expOGWorldStarted) {
        static uint32_t reported=~(uint32_t)0;
        if(finished!=reported) {
            reported=finished;
            fprintf(stderr,"[og-cups-skip] reason=RESIDENT_OG_WORLD "
                           "weeks=%lu seasons=%lu resident-cups=%lu "
                           "(no legacy replay)\n",
                    (unsigned long)elapsedWeeks,(unsigned long)finished,
                    (unsigned long)Expanded_OGWorld_CupCount());
        }
        return;
    }
    static uint32_t lastSkipReported=0;
    /* The saved native world-cup season counter is authoritative after
     * save/reload. Never replay a cup season already finalized by OG. */
    if(s_unemployedOgCupSeasons<(uint32_t)g_worldCups.seasons)
        s_unemployedOgCupSeasons=g_worldCups.seasons;
    if(!s_careerWorldInitialized || !s_careerCompetitionsInitialized ||
       finished<=s_unemployedOgCupSeasons) {
        if(finished && finished!=lastSkipReported) {
            fprintf(stderr,"[og-cups-skip] week=%lu finished=%lu advanced=%lu "
                           "native=%u career-world-init=%d reason=%s\n",
                    (unsigned long)elapsedWeeks,(unsigned long)finished,
                    (unsigned long)s_unemployedOgCupSeasons,
                    (unsigned)g_worldCups.seasons,s_careerWorldInitialized,
                    !s_careerWorldInitialized?"WORLD_NOT_INITIALIZED":
                    !s_careerCompetitionsInitialized?"OG_COMPETITIONS_NOT_INITIALIZED":
                    "NATIVE_SEASON_ALREADY_ADVANCED");
            lastSkipReported=finished;
        }
        return;
    }
    if(finished>90u) {
        fprintf(stderr,"[og-cups] unsupported season=%lu\n",
                (unsigned long)finished);
        return;
    }
    /* In OG 1996/97 the native country list is the original career
     * table. World_SeasonCountries() is NULL there by design; do not
     * silently skip all OG cups for the historical database. */
    const uint8_t *countries=World_SeasonCountries();
    if(!countries) {
        extern uint8_t _careerCompetitionIdRemapTable[];
        countries=_careerCompetitionIdRemapTable;
    }
    if(!countries) {
        fprintf(stderr,"[og-cups] FAIL: no native career countries for world=%s\n",World_Tag());
        return;
    }
    const uint16_t oldFlag=_internationalAutoAdvanceFlag;
    const uint8_t oldSeason=_seasonPlaying;
    SaveSelectedTeams();
    _internationalAutoAdvanceFlag=1;
    for(;s_unemployedOgCupSeasons<finished;++s_unemployedOgCupSeasons) {
        const uint32_t beforeMatches=ExpandedWorldLive_CupResidentMatches();
        const uint32_t beforeTitles=ExpandedWorldLive_CupResidentTitles();
        const uint32_t beforeArchiveMatches=ExpandedWorldLive_CupArchivedMatches();
        const uint32_t beforeArchiveTitles=ExpandedWorldLive_CupArchivedTitles();
        const uint32_t beforeHistoryTitles=ExpandedWorldLive_CupHistoricalTitles();
        unsigned cupRuns=0, leagueRuns=0;
        DIY_OGStandings_SetSeason((uint16_t)(World_FirstYear()+s_unemployedOgCupSeasons));
        _seasonPlaying=(uint8_t)((96u+s_unemployedOgCupSeasons)%100u);
        for(unsigned i=0;i<256u && countries[i]!=0xFFu;++i) {
            const void *const *table=(const void *const *)
                _competitionsTable[countries[i]];
            if(!table)continue;
            /* Process each finished OFFSCREEN league with the original
             * native SWOS competition procedure. The LWE never supplies
             * results or standings for these clubs. */
            {
                const uintptr_t raw=(uintptr_t)table[0];
                if(raw && raw!=(uintptr_t)-1 && raw!=(uintptr_t)-2) {
                    const Competition *comp=(const Competition *)raw;
                    /* Resident OG leagues have already been advanced per date.
                     * Do not execute the OLD completed-year offscreen callback
                     * on top of them a second time. Cup callback below remains. */
                    if(!s_expOGWorldStarted && comp->type==0 &&
                       !comp_is_type0_cup(comp->compId)) {
                        const CompetitionLeague *league=(const CompetitionLeague *)comp;
                        const unsigned divisions=(unsigned)league->header.divisionsCount;
                        if(divisions>=1u && divisions<=16u) {
                            for(unsigned d=0;d<divisions;++d) {
                                Career_AutoAdvanceInternationalCompetitionFixtures(
                                    (uint8_t *)comp,(uint16_t)d);
                                ++leagueRuns;
                            }
                        } else {
                            fprintf(stderr,"[og-unemployed] unsupported divisions country=%u count=%u\\n",
                                    (unsigned)countries[i],divisions);
                        }
                    }
                }
            }
            for(unsigned j=2;j<=3;++j) {
                const uintptr_t raw=(uintptr_t)table[j];
                if(!raw || raw==(uintptr_t)-1)break;
                if(raw==(uintptr_t)-2)continue;
                const Competition *cup=(const Competition *)table[j];
                if(cup->type!=1)continue;
                Career_AutoAdvanceCupCompetitionFromRecord((uint8_t *)cup);
                ++cupRuns;
            }
        }
        if(cupRuns)WorldCups_SeasonEnded();
        fprintf(stderr,"[og-unemployed] season=%lu native-league-divisions=%u native-cups=%u; "
                       "LWE result generator DISABLED\\n",
                (unsigned long)(s_unemployedOgCupSeasons+1u),leagueRuns,cupRuns);
        const uint32_t afterMatches=ExpandedWorldLive_CupResidentMatches();
        const uint32_t afterTitles=ExpandedWorldLive_CupResidentTitles();
        fprintf(stderr,"[og-cups] season=%lu native callbacks=%u "
                "new witnessed OG matches=%lu titles=%lu "
                "(international and national tournaments NOT PROVEN)\n",
                (unsigned long)(s_unemployedOgCupSeasons+1u),cupRuns,
                (unsigned long)(afterMatches>=beforeMatches?
                    afterMatches-beforeMatches:0u),
                (unsigned long)(afterTitles>=beforeTitles?
                    afterTitles-beforeTitles:0u));
        fprintf(stderr,"[og-cups-audit] season=%lu weeks=%lu callbacks=%u "
                       "resident-before=%lu/%lu after=%lu/%lu "
                       "archived-before=%lu/%lu after=%lu/%lu history-before=%lu after=%lu\n",
                (unsigned long)(s_unemployedOgCupSeasons+1u),
                (unsigned long)elapsedWeeks,cupRuns,
                (unsigned long)beforeMatches,(unsigned long)beforeTitles,
                (unsigned long)afterMatches,(unsigned long)afterTitles,
                (unsigned long)beforeArchiveMatches,(unsigned long)beforeArchiveTitles,
                (unsigned long)ExpandedWorldLive_CupArchivedMatches(),
                (unsigned long)ExpandedWorldLive_CupArchivedTitles(),
                (unsigned long)beforeHistoryTitles,
                (unsigned long)ExpandedWorldLive_CupHistoricalTitles());
        if(!cupRuns || (afterMatches==beforeMatches && afterTitles==beforeTitles &&
           beforeArchiveMatches==ExpandedWorldLive_CupArchivedMatches() &&
           beforeArchiveTitles==ExpandedWorldLive_CupArchivedTitles() &&
           beforeHistoryTitles==ExpandedWorldLive_CupHistoricalTitles()))
            fprintf(stderr,"[og-cups] UNVERIFIED season=%lu callbacks produced no "
                           "new recorded results across resident/archive/history\n",
                    (unsigned long)(s_unemployedOgCupSeasons+1u));
    }
    _seasonPlaying=oldSeason;
    _internationalAutoAdvanceFlag=oldFlag;
    RestoreSelectedTeams();
}
#endif

__attribute__((used, externally_visible))
void InitCareerManager(void)
{
    if (s_careerManagerInitialized) return;
    _sameNationalCallFlag = 0;
    _careerSeasonsCompleted = 0;
    for (int i = 0; i < 4; i++)
        _boardConfidenceHistory[i] = 0;
    _boardConfidenceScore = 0;
    _gamesPlayed = 0;
    _numWins = 0;
    _numDrawn = 0;
    _numLost = 0;
    _goalsScored = 0;
    _goalsTaken = 0;
    _careerReputationPoints = 0;
    _winCupBonusCount = 0;
    _minorTrophyBonusCount = 0;
    _majorTrophyBonusCount = 0;
    _playPromotedAnimation = 0;
    _playRelegatedAnimation = 0;
    _teamsLoaded = 0;
    _poolplyrLoaded = 0;
    _careerFileBuffer = 0;
    _previousBoardConfidence = 0xFFFF;
    _playWinCupAnimation = 0;
    _minorTrophyBonusActive = 0;
    _leagueTitleBonusActive = 0;
    _majorTrophyBonusActive = 0;
    _cupBonus1Active = 0;
    _cupBonus2Active = 0;
    _wonWorldCup = 0;
    _wonContinentalCup = 0;
    _qualifyingStage1PendingFlag0 = 0;
    _qualifyingStage1PendingFlag1 = 0;
    _qualifyingStage2PendingFlag0 = 0;
    _qualifyingStage2PendingFlag1 = 0;
    _wonInvitationTournament = 0;

    Career_InitTransferMarketState();
    InitCareerVariables(4);
    /* Expanded lifetime economy must not be reset on an employment change. */
    { extern void Expanded_StartCareer(void); Expanded_StartCareer(); }

    s_careerManagerInitialized = 1;
}

/* OG club-local season setup. Competition slots and financial counters are
 * still native and club-scoped; this MUST NOT reset worldwide cup records.
 * The existing midseason handoff replays the selected team's OG fixtures. */
__attribute__((used, externally_visible))
void InitCareerClub(uint16_t teamNo)
{
    uint32_t balance = CalculateStartingBalance(teamNo);
    Career_InitializeSelectedTeamAndStartingBalance(teamNo, (int32_t)balance);
    _careerFileBuffer = 0;
#ifdef SWOS_SDL_BUILD
    /* OG-only no longer has an LWE season roster to replay. Native
     * OpenSWOS already owns division memberships and tournament setup.
     * Use the old LWE mobility stage ONLY for historical snapshots with
     * fixture records; a new OG-only career never needs this bridge. */
    if (!EXPANDED_WORLD_OG_ONLY && ExpandedWorldLive_Fixtures()>0u &&
        !ExpandedWorldLive_StageNativeMobility())
        fprintf(stderr,"[world-native-roster] ERROR: OG membership NOT "
                       "synchronized; midseason join must not be accepted\n");
#endif
    _teamsLoaded = 0;
    _primaryCompetitionBuf.tourn.header.slotStateA = 3;
    _primaryCompetitionBuf.tourn.header.slotStateB = 2;
    _primaryCompetitionBuf.tourn.header.slotStateC = 8;
    _competitionContextBuf.tourn.header.slotStateA = 3;
    _competitionContextBuf.tourn.header.slotStateB = 2;
    _competitionContextBuf.tourn.header.slotStateC = 8;
    _tertiaryCompetitionContextBuf.tourn.header.slotStateA = 3;
    _tertiaryCompetitionContextBuf.tourn.header.slotStateB = 2;
    _tertiaryCompetitionContextBuf.tourn.header.slotStateC = 8;
    _euroCupsCompetitionContextBuf.tourn.header.slotStateA = 3;
    _euroCupsCompetitionContextBuf.tourn.header.slotStateB = 2;
    _euroCupsCompetitionContextBuf.tourn.header.slotStateC = 8;
#ifdef SWOS_SDL_BUILD
    fprintf(stderr,"[career-init-audit] CLUB_BIND_LOCAL_SETUP "
                   "team=%04x world=%s (NO SEASON INCREMENT; OG SLOT BIND STILL NEEDS PERSISTENT STATE)\n",
            (unsigned)teamNo,World_Tag());
#endif
    Expanded_InitializeClubSeasonState();
#ifdef SWOS_SDL_BUILD
    /* The legacy helper still allocates club-local menu/keycache metadata.
     * Replace its NEW league state with the already-running world league. */
    {
        ExpandedLiveCalendarState state;
        if(ExpandedWorldLive_CalendarState(&state)) {
            const uint16_t actual=(uint16_t)(state.today.year-
                                                  (state.today.month<7u?1u:0u));
            if(Expanded_OGWorld_BindClub(teamNo,(uint8_t)_currentDivisionIndex,
                                          actual)) {
                extern int Expanded_OGWorld_PositionClubCalendar(uint16_t,ExpandedWorldDate);
                if(!Expanded_OGWorld_PositionClubCalendar(actual,state.today))
                    fprintf(stderr,"[og-calendar] WARNING bound table but no "
                                   "valid future native career calendar event\n");
            } else
                fprintf(stderr,"[og-world-bind] NO EXISTING CONTEXT team=%04x "
                               "division=%u year=%u; legacy handoff required\n",
                        (unsigned)teamNo,(unsigned)_currentDivisionIndex,
                        (unsigned)actual);
        }
    }
#endif
    _careerLoadedFlag = 0;
}

/* Rehire can take place years after the last contract. Preserve the
 * manager's lifetime history and WORLD competition copies. Native season
 * counters are positioned so InitializeNewSeason's increment does not
 * accidentally advance a year merely because a club was attached. */
__attribute__((used, externally_visible))
void Career_AttachManagerToClub(uint16_t teamNo)
{
    const uint16_t season = _currentSeasonShowing;
    /* The original compatibility path decremented the year just so
     * InitializeNewSeason() could increment it again. A contract is NOT a
     * new-world-season event: retain both counters unchanged. */
    Career_InitTransferMarketState();
    /* Diagnostic guard: season and native competition ownership must not
     * silently change on an ordinary contract. The legacy club initializer
     * STILL rebuilds competition slots, so do not claim full state handoff. */
#ifdef SWOS_SDL_BUILD
    const uint16_t observedWorldSeason=season;
    fprintf(stderr,"[career-init-audit] ATTACH_BEGIN team=%04x world=%s "
                   "season=%u competition-init=%d world-init=%d manager-init=%d\n",
            (unsigned)teamNo,World_Tag(),(unsigned)observedWorldSeason,
            s_careerCompetitionsInitialized,s_careerWorldInitialized,
            s_careerManagerInitialized);
#endif
    InitCareerClub(teamNo);
#ifdef SWOS_SDL_BUILD
    fprintf(stderr,"[career-init-audit] ATTACH_END team=%04x world=%s "
                   "season=%u prior-season=%u (club competition slots rebuilt: YES)\n",
            (unsigned)teamNo,World_Tag(),(unsigned)_currentSeasonShowing,
            (unsigned)observedWorldSeason);
#endif
    fprintf(stderr,"[career-attach] attached team=%04x season=%u "
                   "without resetting manager/world\n",
            (unsigned)teamNo,(unsigned)_currentSeasonShowing);
}

/* Compatibility entry: a NEW career with an immediately selected club, or
 * a first contract after unemployment. START initializes world/manager
 * before waiting; the fallback also supports OG debug entry points. */
__attribute__((used, externally_visible))
void InitCareer(uint16_t teamNo)
{
    if(!s_careerWorldInitialized) InitCareerWorld();
    if(!s_careerCompetitionsInitialized) InitCareerCompetitions();
    if(!s_careerManagerInitialized) InitCareerManager();
    /* START set the initial world year in InitCareerVariables(4). The
     * first job attaches to that year without incrementing or rewinding. */
    InitCareerClub(teamNo);
}

/* ================================================================
 * ContinueCareerNextSeason  (SWOS2.s line 32355)
 *
 * If the manager has accepted a new job offer, show leaving message,
 * update competitions, init team with new funds, then init new season.
 * Otherwise, show chairman opinion, update competitions, recalculate
 * starting balance, then init new season.
 * ================================================================ */
__attribute__((used, externally_visible))
void ContinueCareerNextSeason(void)
{
    extern uint16_t _gameType;
    SWOS_AUTOSKIP_LOG("[bug1] ContinueCareerNextSeason ENTER: youreSacked=%u "
                      "teamNoOfferAccepted=0x%04X gameType=%u",
                      (unsigned)_youreSacked,
                      (unsigned)_teamNoOfferAccepted,
                      (unsigned)_gameType);
    if (_teamNoOfferAccepted != 0xFFFF) {
        if (_youreSacked == 0) {
            ShowLeavingMessage();
        }
        SWOS_AUTOSKIP_LOG("[bug1] ContinueCareerNextSeason: offer-accepted branch");
        Career_AdvanceToNextSeason_UpdateCompetitions();
        Career_ContinueToNextSeasonInitTeamAndFunds(_teamNoOfferAccepted, _teamFundsAccepted);
        InitializeNewSeason();
    } else {
        SWOS_AUTOSKIP_LOG("[bug1] ContinueCareerNextSeason: chairman branch (no offer accepted)");
        ShowChairmanOpinionAboutTheSeason();
        SWOS_AUTOSKIP_LOG("[bug1] back from ShowChairmanOpinion: youreSacked=%u", (unsigned)_youreSacked);
        Career_AdvanceToNextSeason_UpdateCompetitions();
        SWOS_AUTOSKIP_LOG("[bug1] back from AdvanceToNextSeason: youreSacked=%u", (unsigned)_youreSacked);
        InitializeStartingBalanceForNewSeason();
        SWOS_AUTOSKIP_LOG("[bug1] back from InitStartingBalance: youreSacked=%u", (unsigned)_youreSacked);
        InitializeNewSeason();
        SWOS_AUTOSKIP_LOG("[bug1] back from InitializeNewSeason: youreSacked=%u", (unsigned)_youreSacked);
    }
#ifdef SWOS_SDL_BUILD
    /* Keep the ONE living-world clock at the new season's July boundary.
     * This is after OG InitializeNewSeason, never inside a manual match.
     * The world now rolls old competitions before the next season starts. */
    if (_gameType==4 && _currentSeasonShowing!=0xffffu &&
        ExpandedWorldLive_Clubs()) {
        uint32_t year=(uint32_t)World_FirstYear()+_currentSeasonShowing;
        if (year<=2199u) {
            if (!ExpandedWorldLive_AdvanceToSeason((uint16_t)year))
                fprintf(stderr,"[living-world] WARN: unable to sync OG career "
                               "season %lu; native game left intact\n",
                        (unsigned long)year);
            else
                fprintf(stderr,"[living-world] OG career season %lu synced "
                               "window=%u\n",(unsigned long)year,
                        (unsigned)ExpandedWorldLive_WindowFirstYear());
        }
    }
#endif
    SWOS_AUTOSKIP_LOG("[bug1] ContinueCareerNextSeason EXIT");
}

/* ================================================================
 * Career_TeamInEuroCup
 *
 * Returns 1 if selTeamNumber is in cupCopy's static team list (the player's
 * club plays this euro cup), else 0. Shared by the season-init enrolment
 * (Career_SelectEuroCupSlotIfTeamParticipates) and the load-time slot-3
 * re-derivation (RebuildPointersAfterLoad) so both agree on participation —
 * see the load-fixup site for why a divergence panics 0x6006.
 * ================================================================ */
static int Career_TeamInEuroCup(uint8_t *cupCopy)
{
    /* team list = cupCopy + 7 + cupCopy[7], with the 64-bit text[2] widening
     * compensation (the static competition's unk1[] is packed BE (country,
     * ordinal) byte pairs; read via BE_READ_U16 to match selTeamNumber, which
     * is PACK_BE_U16(country, ordinal) everywhere). */
    uint8_t numEntries = cupCopy[7];
    uint16_t numEntriesAdj = (uint16_t)numEntries +
                             2u * (sizeof(char *) - sizeof(uint32_t));
    const uint8_t *teamList = (const uint8_t *)(cupCopy + 7 + numEntriesAdj);

    /* number of teams depends on competition type (byte at offset 1) */
    uint8_t compType = cupCopy[1];
    int16_t numTeams;
    if (compType == 1)      numTeams = (int8_t)cupCopy[0xA];
    else if (compType == 2) numTeams = (int8_t)cupCopy[0xF];
    else                    numTeams = (int8_t)cupCopy[0xD];

    uint16_t target = _selTeamNumber;
    for (int16_t i = (int16_t)(numTeams - 1); i >= 0; i--) {
        uint16_t entry = BE_READ_U16(teamList);
        teamList += 2;
        if (entry == target)
            return 1;   /* participating */
    }
    return 0;           /* not in this cup */
}

/* Career_TeamInLoadedEuroSlot — non-panicking membership probe for the LOADED
 * slot-3 buffer (a DIYCompetition), used by the load-time slot-3 re-derivation.
 *
 * It answers "is selTeamNumber genuinely a participant in this euro cup?" using
 * the SAME list `FindTeamInDIY` searches — the loaded buffer's team list
 * (teamIds for a type-2 tournament, cupTeamIds for a type-1 cup, each via the
 * buffer's own index table). This is the AUTHORITATIVE source for the loaded
 * season: the slot-3 buffer is saved (FA blob) so it holds the season's real
 * euro field, whereas `euroCupCopy` is reconstructed from its STATIC template on
 * load (save_format_json.c comp_copy_region) and holds DEFAULT teams — so
 * `Career_TeamInEuroCup(euroCupCopy)` gives a FALSE NEGATIVE for a club that's
 * in the saved season's cup but not the default template, leaving
 * quaternaryCompetitionPtr NULL and crashing the cached slot-3 game-list row in
 * `CareerGameListCommon` on `compInfo[5]`. Gating on the buffer keeps this
 * consistent with `FindTeamInDIY`: gate-pass => `FindTeamInDIY(team, buffer)`
 * also finds the team (no 0x6006); gate-fail => slot stays inactive (preserves
 * 1da09fa's protection — a true non-participant is in neither buffer nor
 * template). Mirrors `FindTeamInDIY`'s per-type search but returns found/
 * not-found instead of panicking. */
static int Career_TeamInLoadedEuroSlot(DIYCompetition *comp)
{
    uint16_t numTeams = (uint16_t)comp->numTeams;
    uint16_t target   = _selTeamNumber;

    if (comp->competitionType == 2) {
        /* DIY_TOURNAMENT (e.g. European Champions Cup) — teamIds[teamIndexOrder[i]]. */
        uint16_t *teamIds = ((DIYTournamentState *)comp)->teamIds;
        int8_t   *order   = (int8_t *)((DIYTournamentState *)comp)->teamIndexOrder;
        for (uint16_t i = 0; i < numTeams; i++) {
            int16_t idx = (int16_t)(int8_t)order[i];
            if (BE_READ_U16(&teamIds[idx]) == target)
                return 1;
        }
    } else if (comp->competitionType == 1) {
        /* DIY_CUP (Cup Winners'/UEFA) — cupTeamIds[teamGridPairings[i]]. */
        DIYCompetitionEx *ex      = (DIYCompetitionEx *)comp;
        uint16_t *cupTeamIds      = ex->cupTeamIds;
        int8_t   *pairings        = (int8_t *)ex->teamGridPairings;
        for (uint16_t i = 0; i < numTeams; i++) {
            int16_t idx = (int16_t)(int8_t)pairings[i];
            if (BE_READ_U16(&cupTeamIds[idx]) == target)
                return 1;
        }
    }
    return 0;           /* not in this loaded euro slot */
}

/* ================================================================
 * Career_SelectEuroCupSlotIfTeamParticipates  (SWOS2.s line 32087)
 *
 * Check if selTeamNumber is in the euro cup's team list.
 * If not found, return 1 (non-zero, "not participating").
 * If found, set up slot 3 (quaternary) for this euro cup and return 0.
 *
 * in:  a0 = euroCupCopy pointer, d7 = euroCupIndex (0/1/2)
 * out: d0 = 0 if participating, 1 if not
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t Career_SelectEuroCupSlotIfTeamParticipates(uint8_t *cupCopy, uint16_t euroCupIndex)
{
    /* Participation test (shared with the load-time slot-3 re-derivation in
     * RebuildPointersAfterLoad). */
    if (!Career_TeamInEuroCup(cupCopy))
        return 1;  /* not participating */
    {
#ifdef SWOS_DEBUG_AUTOSKIP
        SWOS_AUTOSKIP_LOG("[eurocup] team 0x%04X qualifies euroCupIndex=%u (cupCopy=%p) "
                          "-> activating slot 3",
                          (unsigned)_selTeamNumber, (unsigned)euroCupIndex, (void *)cupCopy);
#endif
        /* Save quaternary slot state */
        uint16_t savedA = _euroCupsCompetitionContextBuf.tourn.header.slotStateA;
        uint16_t savedB = _euroCupsCompetitionContextBuf.tourn.header.slotStateB;
        uint16_t savedC = _euroCupsCompetitionContextBuf.tourn.header.slotStateC;

        /* Set slot 3 for this euro cup (d0=3, d1=euroCupIndex, d2=0) */
        Career_SetCurrentCompetitionSlot(cupCopy, 3, 0, 0);

        /* quaternaryCompStringOffset == euroCupsCompetitionContext+0x28
         * namePtr on Amiga (slot 3). */
        ((SeasonInformations *)GetCurrentSeasonPointer())->strPtr3 =
            ((DIYCompetition *)_careerCompetitionSlotPtrTable[3])->namePtr;

        /* Restore quaternary slot state */
        _euroCupsCompetitionContextBuf.tourn.header.slotStateC = savedC;
        _euroCupsCompetitionContextBuf.tourn.header.slotStateB = savedB;
        _euroCupsCompetitionContextBuf.tourn.header.slotStateA = savedA;

        _euroCupSlotIndex = euroCupIndex;
        return 0;  /* participating */
    }
}

/* ================================================================
 * Career_SelectCopaLibertadoresSlotIfTeamParticipates
 *
 * CONMEBOL analog of Career_SelectEuroCupSlotIfTeamParticipates: if the player's
 * club is in this season's Copa Libertadores field (the 20 teams earned by the
 * 10 CONMEBOL leagues — already populated into Copa's *Copy seed list by the
 * qualifier in Career_AdvanceToNextSeason_UpdateCompetitions, which runs before
 * this), claim the quaternary slot (slot 3) for Copa so the player plays it
 * interactively (Career_UpdateCopaLibertadoresCompetitionSlot's
 * `== _quaternaryCompetitionPtr` branch).
 *
 * Tried only as the cascade fallback AFTER the three euro cups, so it can fire
 * only for a club in NONE of them — i.e. a South-American club. Copa reuses the
 * slot-3 context buffer (shared with the euro cups, mutually exclusive), so the
 * same slot-state bookkeeping applies. Reuses the static Career_TeamInEuroCup
 * field test (it is generic over a type-2 tournament *Copy). The Copa *Copy is
 * reached through Career_CopaWorkingCopy (src/career_copa.c).
 *
 * Copa claims slot 3 with _euroCupSlotIndex = 3 (the euro cups use 0/1/2). That
 * SAVED index drives BOTH (a) the load-time slot-3 re-derivation
 * (RebuildPointersAfterLoad below), so a saved CONMEBOL career re-loads with Copa
 * still interactive rather than auto-advancing, and (b) the continental-cup win
 * rewards in career_status.c, where index 3 is treated as euroCup-tier (major
 * trophy + euroCup financial bonus) — Copa Libertadores is South America's premier
 * club cup. The in-session play path branches on _quaternaryCompetitionPtr, not the
 * index.
 *
 * out: 0 if participating, 1 if not (or Copa not installed).
 * ================================================================ */
__attribute__((used, externally_visible))
uint16_t Career_SelectCopaLibertadoresSlotIfTeamParticipates(void)
{
    extern Competition *Career_CopaWorkingCopy(void);
    uint8_t *copaCopy = (uint8_t *)Career_CopaWorkingCopy();
    if (!copaCopy)
        return 1;  /* Copa not installed (e.g. SWOS_DATA_JSON disabled) */
    if (!Career_TeamInEuroCup(copaCopy))
        return 1;  /* player's club is not in the Copa field this season */

    /* Save quaternary slot state */
    uint16_t savedA = _euroCupsCompetitionContextBuf.tourn.header.slotStateA;
    uint16_t savedB = _euroCupsCompetitionContextBuf.tourn.header.slotStateB;
    uint16_t savedC = _euroCupsCompetitionContextBuf.tourn.header.slotStateC;

    /* Claim slot 3 for Copa */
    Career_SetCurrentCompetitionSlot(copaCopy, 3, 0, 0);

    ((SeasonInformations *)GetCurrentSeasonPointer())->strPtr3 =
        ((DIYCompetition *)_careerCompetitionSlotPtrTable[3])->namePtr;

    /* Restore quaternary slot state */
    _euroCupsCompetitionContextBuf.tourn.header.slotStateC = savedC;
    _euroCupsCompetitionContextBuf.tourn.header.slotStateB = savedB;
    _euroCupsCompetitionContextBuf.tourn.header.slotStateA = savedA;

    /* Copa sentinel for the slot-3 index: euro cups are 0/1/2, Copa is 3. Saved,
     * and consumed by the load gate (RebuildPointersAfterLoad) + the
     * continental-cup rewards (career_status.c). */
    _euroCupSlotIndex = 3;

    return 0;  /* participating */
}

/* ================================================================
 * DIY_SelectCompetitionSlot4FromCurrentDivision  (SWOS2.s line 30972)
 *
 * If the primary competition is a league (type 0), check if it has
 * a linked DIY competition and set up slot 4 for it.
 * ================================================================ */
/* England's leagues — standalone globals, consolidated from the old
 * EnglishLeaguesBlock (competition_data.c). CompetitionLeague / Leagues come from
 * game_types.h; competition_data.h is kept out of this TU, so extern them here. */
extern CompetitionLeague _englishLeague;
extern Leagues _englishLeagues1, _englishLeagues2, _englishLeagues3;

__attribute__((used, externally_visible))
void DIY_SelectCompetitionSlot4FromCurrentDivision(void)
{
    uint8_t *comp = (uint8_t *)_currentCompetitionPtr;
    if (((Competition *)comp)->type != 0)   /* type at offset 1; 0 == league */
        return;

    /* The original asm followed divisions[currentDivisionIndex].qualifierLinkOffset
     * (was unk1) as a SELF-RELATIVE byte offset (from that byte) to a {diyComp,
     * diyCompetitionTablePtr} pointer pair. That offset was baked for the Amiga
     * name[8] / 4-byte-pointer layout, where it pointed at the per-division
     * playoff link pair. The struct consolidation widened name[] to [10] and
     * 64-bit grew the pointers, so the byte walk now lands on name[] (e.g.
     * aDivisionTwo) -> garbage diyComp -> slot-4 type-check panic 0x4459.
     * Resolve via the typed fields instead — the SAME per-division playoff
     * links career_season.c uses. qualifierLinkOffset == 0 => no link (e.g.
     * Premier, div 0).
     *
     * Pairs verified against the Amiga 4-byte offsets + live pointers:
     *   div 1: _main.playoffLinkPtrs[0]/[1] = {&div1Playoffs, &_div1}
     *   div 2: _div1.playoffs/nextGroup     = {&div2Playoffs, &_div2}
     *   div 3: _div2.playoffs/nextGroup     = {&div3Playoffs, &_div3} */
    CompetitionLeague *league = (CompetitionLeague *)comp;
    if (league->divisions[(uint16_t)_currentDivisionIndex].qualifierLinkOffset == 0)
        return;
    if (comp_id(comp) != COMPID_englishLeague)   /* comp non-NULL: deref'd at type check above */
        return;  /* only the English leagues carry these per-division links */

    uint8_t *diyComp;
    void    *diyTable;
    switch ((uint16_t)_currentDivisionIndex) {
    case 1:
        diyComp  = (uint8_t *)_englishLeague.playoffLinkPtrs[0];
        diyTable = _englishLeague.playoffLinkPtrs[1];
        break;
    case 2:
        diyComp  = (uint8_t *)_englishLeagues1.playoffs;
        diyTable = _englishLeagues1.nextGroup;
        break;
    case 3:
        diyComp  = (uint8_t *)_englishLeagues2.playoffs;
        diyTable = _englishLeagues2.nextGroup;
        break;
    default:
        return;
    }

    _diyCompetitionTablePtr = (uint32_t *)diyTable;
    Career_SetCurrentCompetitionSlot(diyComp, 4, 0, 0);
}

/* ================================================================
 * InitializeNewSeason  (SWOS2.s line 31818)
 *
 * Large function that initializes a new career season: increments
 * season counter, clears board confidence, sets up competition
 * slots (primary/secondary/tertiary/quaternary/diy), determines
 * if national selector, assigns euro cups, builds team budget rank
 * table, copies manager name, fills market players, inits finances,
 * and shows chairman message.
 * ================================================================ */
__attribute__((used, externally_visible))
void InitializeNewSeason(void)
{
    SWOS_AUTOSKIP_LOG("[bug1] InitializeNewSeason ENTER: youreSacked=%u "
                      "teamNoOfferAccepted=0x%04X selTeamNumber=0x%04X",
                      (unsigned)_youreSacked,
                      (unsigned)_teamNoOfferAccepted,
                      (unsigned)_selTeamNumber);
    /* This function is called only when the native career actually moves
     * to a NEW season. NEVER call it from a contract/rehire. */
    _teamsLoaded = 0;
    _currentSeasonShowing++;
    if (_currentSeasonShowing == _careerParams.num_seasons)
        _currentSeasonShowing = 0;
    _seasonPlaying++;
    if (_seasonPlaying == 100)
        _seasonPlaying = 0;
#ifdef SWOS_SDL_BUILD
    DIY_OGStandings_SetSeason((uint16_t)(World_FirstYear()+_currentSeasonShowing));
    fprintf(stderr,"[og-season] ADVANCE world=%s index=%u native=%u (season transition, not hire)\n",
            World_Tag(),(unsigned)_currentSeasonShowing,(unsigned)_seasonPlaying);
#endif
    /* Native world mobility/playoffs change only on a real season event. */
    LeagueGroups_Apply((const int8_t *)_diyLeagueSeasonStateTable,
                       (int8_t *)_diyLeagueScratchBuffer);
    DIY_SaveLeagueScratchToTable();
    WorldPlayoff_Reset();
    Expanded_InitializeClubSeasonState();
}

/* Only the club-facing setup: consumes the existing world season. Until
 * the OG competition contexts are persistent per division, the native
 * club competition slot setup below is still a compatibility bridge.
 * Crucially it cannot advance _seasonPlaying or _currentSeasonShowing. */
__attribute__((used, externally_visible))
void Expanded_InitializeClubSeasonState(void)
{
    _teamsLoaded = 0;
    _selTeamSelectedPlayerPosition = 0xFFFF;

    /* Clear 4 entries of careerUnavailablePlayerList */
    /* careerUnavailablePlayerList holds 4 × { uint16 teamNumber,
     * uint16 playerIndex } entries at byte stride 4. Mark the first 4
     * teamNumber slots as empty (0xFFFF). */
    {
        uint16_t *upl = (uint16_t *)_careerUnavailablePlayerList;
        upl[0] = 0xFFFF;
        upl[2] = 0xFFFF;
        upl[4] = 0xFFFF;
        upl[6] = 0xFFFF;
    }

    _retired = 0;
    _youreSacked = 0;
    _competitionSlotUpdatedFlag = 0;
    _stayingInOldClub++;

    _boardConfidenceMod0 = 0;
    _boardConfidenceMod1 = 0;
    _boardConfidenceMod2 = 0;
    _boardConfidenceMod3 = 0;
    _boardConfidenceValue0 = 0;
    _boardConfidenceValue1 = 0;
    _boardConfidenceMod4 = 0;
    _boardConfidenceMod5 = 0;
    _boardConfidenceMod6 = 0;
    _boardConfidenceMod7 = 0;
    _boardConfidenceMod8 = 0;
    _boardConfidenceMod9 = 0;

    /* Get current season info pointer and fill it */
    SeasonInformations *si = (SeasonInformations *)GetCurrentSeasonPointer();
    si->clubNo            = _selTeamNumber;
    si->leagueStrPtr      = 0;
    si->strPtr1           = 0;
    si->strPtr2           = 0;
    si->strPtr3           = 0;
    si->comp1Result       = 0;
    si->comp2Result       = 0;
    si->comp3Result       = 0;
    si->comp4Result       = 0;
    si->bestScorerName[0] = 0;
    si->managerStatus     = 0;

    /* Copy team name to season info CLUBNAME */
    StringCopy((uint8_t *)_careerTeam.team.teamName, (uint8_t *)si->clubName);

    /* Verify team loaded and get current division index */
    TeamFileEx *teamFile = VerifyTeamLoaded(_selTeamNumber);
    uint16_t globalTeamNo = teamFile->team.globalTeamNumber;
    int8_t diyLeagueState = (int8_t)_diyLeagueSeasonStateTable[globalTeamNo];
    int16_t divIdx = (int16_t)diyLeagueState + (int16_t)_careerTeamLeague;
    _currentDivisionIndex = (uint16_t)divIdx;
    teamFile->team.league = (uint8_t)divIdx;

    _contractNegotiationActive = 0;
    _removedPlayerStatAccum = 0;
    Career_InitSeasonTransferMarketState();

    _teamSelectionKeyCacheEnd = 0;
    _isCareer = 0;
    _careerMatchSimulatedFlag = 0;
    _numNewJobOffers = 0;
    _numJobOffers = 0;
    _careerTeamLostMatch = 0;
    _currentMessage = 0;

    /* Fill careerLastMatchIndexByCompetitionSlot with 0xFF (one per compId, volatile to avoid memset) */
    {
        volatile uint8_t *dst = _careerLastMatchIndexByCompetitionSlot;
        for (int16_t i = 255; i >= 0; i--)
            *dst++ = 0xFF;
    }

    /* Initialize competition slot pointer table */
    _careerCompetitionSlotPtrTable[0] = (uint8_t *)&_primaryCompetitionBuf;
    _careerCompetitionSlotPtrTable[1] = (uint8_t *)&_competitionContextBuf;
    _careerCompetitionSlotPtrTable[2] = (uint8_t *)&_tertiaryCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[3] = (uint8_t *)&_euroCupsCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[4] = (uint8_t *)&_diyCompetitionContextBuf;

    /* Clear all competition pointers */
    _currentCompetitionPtr = 0;
    _secondaryCompetitionPtr = 0;
    _tertiaryCompetitionPtr = 0;
    _quaternaryCompetitionPtr = 0;
    _diyCompetitionPtr = 0;
    /* WorldPlayoff_Reset belongs to the new-season boundary, NOT a job. */

    _numSelectedTeams = 1;
    _teamSelectionKeyCacheCount = 0;
    _teamSelectionKeyCacheIndex = 0;
    _teamSelectionKeyCacheOffset = 0;
    _careerSlotProcessingInProgress = 0;
    _isNationalSelector = 0;

    /* Determine if this is a national team (country code 80..85 = CN_EUROPE..CN_OCEANIA) */
    uint16_t countryCode = _selTeamNumber >> 8;
    /* Walker over a country's competition pointer-array. Entries are
     * full-width pointers — `Competition *` — interleaved with -2/-1
     * sentinels. Iterating as `void **` makes the stride scale with
     * pointer width (4 B on Amiga/32-bit, 8 B on 64-bit) and lets the
     * sentinel comparison use the full pointer width via `(uintptr_t)-1`. */
    void **compTablePtr;

    if (countryCode >= 80 && countryCode <= 85) {
        /* National team selector */
        _isNationalSelector = 1;

        /* Check same nationality call */
        if (_selTeamNumber == _lastNationalityCall) {
            _sameNationalCallFlag = 1;
        }

        /* Look up team flags from region table. Walk as `void **` so
         * each slot is native pointer width (was `uint32_t *` which
         * read half a pointer on 64-bit and warned at the
         * (uint8_t *)<u32> cast). */
        void **regionTable = _careerTeamFlagsByRegionTable;
        uint16_t regionIdx = (uint8_t)(_selTeamNumber >> 8);  /* move.b — high byte = country */
        int16_t regionOffset = (int16_t)(int8_t)regionIdx - 80;
        uint8_t *flagsArray = (uint8_t *)regionTable[regionOffset];
        _someTeamFlags = flagsArray[_currentSeasonShowing % 20];  /* flags = fixed 20-entry period-4 euro-cup rotation; wraps for 35-season careers */

#ifdef SWOS_INJECT_WC
        /* DEBUG (SWOS_INJECT_WC, opt-in via `make SWOS_INJECT_WC=1`): force a
         * World Cup year for this national-team career, bypassing the natural
         * 4-year cycle, so the national-coach WC path runs every season — used
         * to reproduce the season-transition 0x8888 backup-clobber
         * (project_eurocup_national_backup_clobber). 0x80 selects the WC-year
         * branch in Career_SetCupPickList_ForSelectedRegion (called below);
         * qualifyingStage1PendingFlag0 = 0 picks the *invited/finals* pick list
         * (the actual WC tournament) rather than WC-qualifier friendlies — 0x80
         * alone would leave the team in friendlies with no WC. Flip the flag to
         * 1 if you want the qualifier path instead. National teams only (here). */
        someTeamFlags = 0x80;
        qualifyingStage1PendingFlag0 = 0;
        fprintf(stderr, "[SWOS_INJECT_WC] forcing World Cup for national team "
                "0x%04X (season %u)\n",
                (unsigned)selTeamNumber, (unsigned)currentSeasonShowing);
#endif

        /* Get nationality and team file */
        uint32_t natResult = GetNationalityAndTeamFile(_selTeamNumber);
        _nationalTeamNationality = (uint8_t)(natResult >> 16);  /* d0 */
        _nationalTeamFileId = (uint8_t)(natResult & 0xFF);       /* d1 */

        /* Look up season cycle flags. The A/B tables are 5 entries — one per
         * 4-year cycle across the fixed 20-season national scheduling period.
         * The original asm indexed seasonCycleNationalFlags[(season>>2)*2] with
         * NO wrap because careers were capped at 20 seasons (cycle 0..4, in
         * bounds). num_seasons now runs to 35, so cycle = season>>2 can reach 8
         * and must wrap into the 5 entries — otherwise it reads
         * seasonCycleNationalFlags[5..8] out of bounds and feeds garbage
         * World-Cup / Euro-Championship qualifier ids into the group fill.
         * cycle is <= 8 (CAREER_MAX_SEASONS caps season at 34), so a single
         * conditional subtract does the wrap — no `% 5` divide (slow on 68k). */
        uint16_t cycle = (_currentSeasonShowing >> 2) % 5; /* safe to season 66 */
        _worldCupQualifyingTeam = _seasonCycleNationalFlagsA[cycle];
        _euroChampionshipQualTeam = _seasonCycleNationalFlagsB[cycle];

        /* Set cup pick list for selected region (param unused, function writes to global) */
        Career_SetCupPickList_ForSelectedRegion(0);

        compTablePtr = (void **)&_careerCupPickList;
    } else {
        /* Club team */
        _nationalTeamFileCode = World_FileCountry((uint8_t)(_selTeamNumber >> 8));  /* move.b — high byte (a second team file: its country's, include/world.h) */

        uint16_t tableIdx = _selTeamNumber >> 8;
        compTablePtr = (void **)_competitionsTable[tableIdx];
    }

    Career_UpdateLastNationalityCall();

    /* Set up primary competition (slot 0) */
    {
        uint16_t savedA = _primaryCompetitionBuf.tourn.header.slotStateA;
        uint16_t savedB = _primaryCompetitionBuf.tourn.header.slotStateB;
        uint16_t savedC = _primaryCompetitionBuf.tourn.header.slotStateC;

        uint8_t *comp = (uint8_t *)*compTablePtr;
        compTablePtr++;
        Career_SetCurrentCompetitionSlot(comp, 0, _currentDivisionIndex, 0);

        /* SWOS2.s reads (primaryCompStringOffset), which on Amiga IS
         * competitionFileBuffer+0x28 = the comp namePtr that
         * Career_SetCurrentCompetitionSlot just wrote. Our standalone
         * primaryCompStringOffset global is never populated (only zeroed),
         * so read the slot-0 buffer's namePtr directly. */
        ((SeasonInformations *)GetCurrentSeasonPointer())->leagueStrPtr =
            ((DIYCompetition *)_careerCompetitionSlotPtrTable[0])->namePtr;

        _primaryCompetitionBuf.tourn.header.slotStateC = savedC;
        _primaryCompetitionBuf.tourn.header.slotStateB = savedB;
        _primaryCompetitionBuf.tourn.header.slotStateA = savedA;
    }

    /* Skip the per-primary divisionIndex slot (one pointer-sized entry,
     * usually -2 = "no extra division pointer"). */
    compTablePtr++;

    /* Set up secondary competition (slot 1) if present */
    if ((uintptr_t)*compTablePtr != (uintptr_t)-1) {
        uint16_t savedA = _competitionContextBuf.tourn.header.slotStateA;
        uint16_t savedB = _competitionContextBuf.tourn.header.slotStateB;
        uint16_t savedC = _competitionContextBuf.tourn.header.slotStateC;

        uint8_t *comp = (uint8_t *)*compTablePtr;
        compTablePtr++;
        /* a world's cup only if the club is in it (world_cups.c) */
        if (WorldCups_Plays(comp)) {
            Career_SetCurrentCompetitionSlot(comp, 1, 0, 0);

            /* secondaryCompStringOffset == competitionContextBuf+0x28 namePtr on
             * Amiga (slot 1); read the buffer namePtr directly. */
            ((SeasonInformations *)GetCurrentSeasonPointer())->strPtr1 =
                ((DIYCompetition *)_careerCompetitionSlotPtrTable[1])->namePtr;
        }

        _competitionContextBuf.tourn.header.slotStateC = savedC;
        _competitionContextBuf.tourn.header.slotStateB = savedB;
        _competitionContextBuf.tourn.header.slotStateA = savedA;

        /* Set up tertiary competition (slot 2) if present */
        if ((uintptr_t)*compTablePtr != (uintptr_t)-1) {
            uint16_t savedA2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateA;
            uint16_t savedB2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateB;
            uint16_t savedC2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateC;

            comp = (uint8_t *)*compTablePtr;
            if (WorldCups_Plays(comp)) {
                Career_SetCurrentCompetitionSlot(comp, 2, 0, 0);

                /* tertiaryCompStringOffset == tertiaryCompetitionContextBuf+0x28
                 * namePtr on Amiga (slot 2). */
                ((SeasonInformations *)GetCurrentSeasonPointer())->strPtr2 =
                    ((DIYCompetition *)_careerCompetitionSlotPtrTable[2])->namePtr;
            }

            _tertiaryCompetitionContextBuf.tourn.header.slotStateC = savedC2;
            _tertiaryCompetitionContextBuf.tourn.header.slotStateB = savedB2;
            _tertiaryCompetitionContextBuf.tourn.header.slotStateA = savedA2;
        }
    }

    /* A world's state championship (states.c): the career club's own state in slot 2 when its
     * country's table has nothing there (Brazil: its league cup's slot). */
    if (_tertiaryCompetitionPtr == 0) {
        uint8_t *st = States_SlotContest();
        if (st) {
            uint16_t savedA2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateA;
            uint16_t savedB2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateB;
            uint16_t savedC2 = _tertiaryCompetitionContextBuf.tourn.header.slotStateC;
            Career_SetCurrentCompetitionSlot(st, 2, 0, 0);
            ((SeasonInformations *)GetCurrentSeasonPointer())->strPtr2 =
                ((DIYCompetition *)_careerCompetitionSlotPtrTable[2])->namePtr;
            _tertiaryCompetitionContextBuf.tourn.header.slotStateC = savedC2;
            _tertiaryCompetitionContextBuf.tourn.header.slotStateB = savedB2;
            _tertiaryCompetitionContextBuf.tourn.header.slotStateA = savedA2;
        }
    }

    /* Try euro cups for slot 3 (quaternary); if the club is in none of them, try
     * Copa Libertadores (a South-American club). The quaternary slot holds the
     * player's ONE continental cup — euro and Copa are mutually exclusive, so this
     * cascade naturally routes a CONMEBOL club to Copa and everyone else to a euro
     * cup (or leaves slot 3 inactive). */
    uint16_t tieIndex = 0;
    uint8_t *tie = UefaQual_SeasonTie(&tieIndex);  /* (a world's: the career club's qualifying tie first) */
    if (!tie)
        tie = Conti_SeasonTie(&tieIndex);           /* (or its tie in another confederation's cup) */
    if (!tie || Career_SelectEuroCupSlotIfTeamParticipates(tie, tieIndex) != 0)
    if (Career_SelectEuroCupSlotIfTeamParticipates(_euroCupCopy, 0) != 0) {
        if (Career_SelectEuroCupSlotIfTeamParticipates(_cupWinnersCupCopy, 1) != 0) {
            if (Career_SelectEuroCupSlotIfTeamParticipates(_uefaCupCopy, 2) != 0) {
                Career_SelectCopaLibertadoresSlotIfTeamParticipates();
            }
        }
    }

    /* Set up DIY competition for slot 4 */
    DIY_SelectCompetitionSlot4FromCurrentDivision();

    InitializeNewSeason_BuildTeamBudgetRankTable();

    /* Determine leagueFirstRoundNumber */
    {
        uint16_t firstRound = 1;
        uint8_t *primaryComp = (uint8_t *)_currentCompetitionPtr;
        if (primaryComp[1] == 0) {
            firstRound = (uint16_t)primaryComp[9];
        }
        _leagueFirstRoundNumber = firstRound;
    }

    _seasonFinished = 0;
    _careerFinished = 0;

    /* Re-derive careerJobOfferPhaseBias from the freshly-built key cache.
     *
     * In SWOS2.s this byte OVERLAPS teamSelectionKeyCache[1] (adjacent-globals
     * BSS: `teamSelectionKeyCache: ds.b $1` immediately followed by
     * `careerJobOfferPhaseBias: ds.b $1`), so the cache builder writing entry-0's
     * sortKey there implicitly set the bias to the EARLIEST fixture's week. The
     * finance/job-offer code relies on that: it is the season-start week base for
     * lastFinancialWeekIndex and the `lastKey - bias` season span in
     * wageScalingDivisor. The SDL port promoted the byte to a standalone global
     * (career_key_cache.c) but never re-derived it, leaving it 0 — so
     * lastFinancialWeekIndex started at 0 and the FIRST weekly deduction
     * front-loaded the entire pre-season gap (both PLAYER WAGES BILL and GENERAL
     * RUNNING COSTS ~inflated on the first match, self-cancelling by season end
     * because wageScalingDivisor = lastKey grows to match). Restore the overlap
     * value explicitly so bias == entry-0 sortKey on every season init. */
    _careerJobOfferPhaseBias = ((TeamSelectionKeyCacheEntry *)_teamSelectionKeyCache)->sortKey;

    _viewRandomPhase = _careerJobOfferPhaseBias;

    /* Walk teamSelectionKeyCache to compute wageScalingDivisor */
    {
        uint8_t *cachePtr = _teamSelectionKeyCache;
        uint8_t lastKey = 0;
        int16_t count = _teamSelectionKeyCacheCount - 1;
        for (int16_t i = count; i >= 0; i--) {
            lastKey = ((TeamSelectionKeyCacheEntry *)cachePtr)->sortKey;
            uint16_t stride = ((TeamSelectionKeyCacheEntry *)cachePtr)->entryStride;
            cachePtr += stride;
        }
        uint16_t d0 = (uint16_t)(lastKey - _careerJobOfferPhaseBias) & 0xFF;
        if (d0 == 0)
            d0 = 1;
        _wageScalingDivisor = d0;
    }

    _lastFinancialWeekIndex = _careerJobOfferPhaseBias;

    InitTeams();

    /* Copy manager name into the team's coach-name slot */
    teamFile = VerifyTeamLoaded(_selTeamNumber);
    {
        uint8_t *namePos = (uint8_t *)teamFile->team.coachName;
        StringCopy(_managerNameBuffer, namePos);
        /* Find end of copied name and add space + surname */
        while (*namePos != 0)
            namePos++;
        *namePos++ = 0x20;  /* space */
        StringCopy(_managerSurnameBuffer, namePos);
    }

    FillForeignMarketPlayers();
    FillCareerTeamPlayers();
    Career_InitializeNewSeasonFinances();
    InitializeCareerSeasonFinancials();
    CupD_SeasonStart();     /* the Coppa Italia Serie D's rounds before the first match (world_cupd.c) */

    /* Show appropriate chairman message */
    if (_stayingInOldClub != 0) {
        if (_currentSeasonShowing == 0x13) {
            ShowEnjoyLastSeasonMessage();
        } else {
            ShowWishYouSuccessMessage();
        }
    } else {
        if (_currentSeasonShowing == 0x13) {
            ShowLastSeasonMessage();
        } else {
            ShowGreetingMessage();
        }
    }

    /* Post-message: trial players and international squad setup */
    if (_stayingInOldClub == 0) {
        TrainingMatch_AddTrialPlayersForNewSeason();
        if (_isNationalSelector != 0) {
            Career_InitInternationalSquadAndLookupTablesForNewSeason((TeamFileEx *)0);
        }
    }

}

/* ================================================================
 * InitNewSeason  (SWOS2.s line 32148)
 *
 * Simplified new season init used when continuing with a new team
 * (after job change). Sets up competition slots from the competition
 * table, counts non-computer squads, inits teams.
 *
 * in:  a0 = source comp record, d1 = divisionIndex
 * ================================================================ */
__attribute__((used, externally_visible))
void InitNewSeason(uint8_t *src, uint16_t divisionIndex)
{
    _retired = 0;
    uint8_t compTableIdx = (uint8_t)((Competition *)src)->teamNumber;
    _currentDivisionIndex = divisionIndex;

    /* Initialize competition slot pointer table */
    _careerCompetitionSlotPtrTable[0] = (uint8_t *)&_primaryCompetitionBuf;
    _careerCompetitionSlotPtrTable[1] = (uint8_t *)&_competitionContextBuf;
    _careerCompetitionSlotPtrTable[2] = (uint8_t *)&_tertiaryCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[3] = (uint8_t *)&_euroCupsCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[4] = (uint8_t *)&_diyCompetitionContextBuf;

    _currentCompetitionPtr = 0;
    _secondaryCompetitionPtr = 0;
    _tertiaryCompetitionPtr = 0;
    _quaternaryCompetitionPtr = 0;
    _diyCompetitionPtr = 0;
    WorldPlayoff_Reset();

    _teamSelectionKeyCacheCount = 0;
    _teamSelectionKeyCacheIndex = 0;
    _teamSelectionKeyCacheOffset = 0;
    _careerSlotProcessingInProgress = 0;

    /* Look up competitions table by index — entries are full-width
     * pointers (8 B on 64-bit, 4 B on Amiga). Iterate as `void **` so
     * stride/sentinel scale with pointer width — see equivalent walker
     * in `InitializeNewSeason`. */
    void **compTable = (void **)_competitionsTable[compTableIdx & 0xFF];

    /* Slot 0: primary competition */
    uint8_t *comp = (uint8_t *)*compTable;
    compTable++;
    Career_SetCurrentCompetitionSlot(comp, 0, _currentDivisionIndex, 1);

    /* Skip the per-primary divisionIndex slot (one pointer entry, -2 = "none"). */
    compTable++;

    /* Slot 1: secondary competition (if present) -- a world's cup only if the club is
     * in it (world_cups.c) */
    if ((uintptr_t)*compTable != (uintptr_t)-1) {
        comp = (uint8_t *)*compTable;
        compTable++;
        if (WorldCups_Plays(comp))
            Career_SetCurrentCompetitionSlot(comp, 1, 0, 0);

        /* Slot 2: tertiary competition (if present) */
        if ((uintptr_t)*compTable != (uintptr_t)-1) {
            comp = (uint8_t *)*compTable;
            if (WorldCups_Plays(comp))
                Career_SetCurrentCompetitionSlot(comp, 2, 0, 0);
        }
    }

    _seasonFinished = 0;
    _careerFinished = 0;

    /* Count non-computer-controlled squads */
    _seasonNumberOfSquads = 0;
    uint16_t *squadTeams = _seasonSquadTeamNumbers;
    TeamFileEx *teamPtr = _selectedTeams;
    int16_t numTeams = _numSelectedTeams - 1;
    for (int16_t i = numTeams; i >= 0; i--) {
        if (teamPtr->team.teamControls != 1) {
            _seasonNumberOfSquads++;
            *squadTeams++ = PACK_BE_U16(teamPtr->team.countryNumber,
                                        teamPtr->team.teamOrdinal);
        }
        teamPtr++;
    }

    if (_seasonNumberOfSquads == 0)
        _careerFinished = 1;

    _viewRandomPhase = _careerJobOfferPhaseBias;
    InitTeams();
}

/* ================================================================
 * RebuildPointersAfterLoad  (no asm equivalent — new code)
 *
 * Several fields in the careerFileBuffer overlay region (see SAVE_DATA.md)
 * are typed as absolute pointers — valid when the save was written but
 * meaningless in any subsequent binary build (or any other machine). The
 * JSON save format never stores these as raw addresses; it serializes a
 * stable IDENTITY instead (competition ID, buffer index, string-pool key).
 * After `SaveJson_LoadCareer` restores the typed fields, this function
 * re-derives every pointer field from its identity (current run's buffer
 * address, competition ID, etc.).
 *
 * This is now the ONLY pointer fixup. The legacy Amiga-absolute-address
 * translation LUT (`_TranslateAmigaAddressesAfterLoad` /
 * `save_pointer_lut.c` / `generated/amiga_address_map.c`) was deleted once
 * the raw-dump readers were retired — JSON identities make it unnecessary.
 *
 * Fields handled (see SAVE_DATA.md "Pointer-field summary for .CAR"):
 *
 *   1. careerCompetitionSlotPtrTable[5]  — five competition slot bases.
 *      Re-derived from `&competitionFileBuffer / &competitionContextBuf /
 *      &tertiaryCompetitionContextBuf / &euroCupsCompetitionContext /
 *      &diyCompetitionContextBuf` in the current build's BSS (slots 0/3
 *      still SLOT_ALIAS into their bufs; slots 1, 2 & 4 are the typed buffers).
 *
 *   2. diyCompetitionStart — runtime DIY working buffer (separate 4 KB
 *      BSS, NOT inside the careerFileBuffer overlay). Repopulated
 *      from the loaded slot-0 (primary competition) state.
 *
 *   3. Embedded `nameSource` pointers inside each cup *Copy record at
 *      offset (5 + nameOffset). Copied from the matching static cup
 *      template via the careerContests table.
 *
 *   4. Top-level competition pointers
 *      `currentCompetitionPtr / secondaryCompetitionPtr /
 *      tertiaryCompetitionPtr / diyCompetitionPtr /
 *      diyCompetitionTablePtr`. Re-derived from competitionsTable +
 *      saved selTeamNumber (or careerCupPickList for national
 *      selectors), mirroring `InitializeNewSeason` lines 994-1067
 *      but without re-initialising slot state.
 *
 * Not yet handled (will be layered in as we hit menus that read
 * them):
 *
 *   *CompStringOffset      — would re-derive from string-table
 *                              lookup by competition.
 *   quaternaryCompetitionPtr (slot-3 euro cup) — needs a saved-state
 *                              probe to disambiguate which cup. NULLed
 *                              for now; the slot is reachable via
 *                              careerCompetitionSlotPtrTable[3].
 *   tmdFileBuffer/careerTeam runtime pointers
 *                          — should be zeroed; rebuilt at match start.
 *
 * On the Amiga C port (post back-port) this function is identical —
 * neither port pins data at fixed addresses anymore, so both need the
 * same rebuild.
 * ================================================================ */
extern void Career_LoadCompetitionSlotStateFromBuffer(uint8_t *src);
/* declared at top of file (line 615) — uint16_t *teamList */

/* Step 3 helper: walk careerContests and re-derive each *Copy
 * record's embedded text[] pointer pair.
 *
 * Each cup record carries a signed `nameOffset` byte at offset 5. At
 * `record + 5 + nameOffset` lives an inline `text[2]` array — two
 * 4-byte pointers to static name strings (e.g. text[0] = "EUROPEAN
 * CHAMPIONS", text[1] = "EUROPEAN CUP"). The remaining bytes after
 * those 8 are the `unk1[]` lookup table (scalars, not pointers).
 *
 * The static template carries the canonical pointer values for the
 * current build; the saved *Copy carries stale values from the
 * saving build. Copy both pointers from src→dest. */
static void Rebuild_CupRecordNameSourcePointers(void)
{
    /* Iterate as typed CompetitionCareer[] — entry stride is 10 on
     * 32-bit, 18 on 64-bit. The text[] copy reads/writes two pointer-
     * width entries (4 B each → 8 B on 32-bit, 8 B each → 16 B on
     * 64-bit), so use sizeof(void *) and memcpy 2*sizeof(void *). */
    extern CompetitionCareer _careerContests[];
    for (CompetitionCareer *e = _careerContests;
         (uintptr_t)e->competition != 0xFFFFFFFFu; e++) {
        uint8_t *src  = (uint8_t *)e->competition;
        uint8_t *dest = (uint8_t *)e->competitionCopy;
        /* Copy the 2 name pointers (full + abbreviation) into the comp copy.
           dest is a byte copy of src, so its name table sits at the same place. */
        char **srcName = comp_name_table((Competition *)src);
        if (srcName) {
            memcpy((char **)comp_name_table((Competition *)dest),
                   srcName, 2 * sizeof(void *));
        }
    }
}

/* Step 4 helper: re-derive the top-level competition slot pointers.
 *
 * Mirrors the lookup in `InitializeNewSeason` (lines 994-1067) but
 * does NOT call `Career_SetCurrentCompetitionSlot` — that helper is
 * destructive (resets slot state from the static comp template,
 * losing the loaded season's progress). Here we only re-establish
 * the pointer values; the slot state itself is already correctly
 * loaded from the .CAR.
 *
 * National-selector careers populate these pointers via
 * `careerCupPickList` instead of competitionsTable. The pickList is
 * inside the saved overlay, so its entries are stale too — rebuild
 * via `Career_SetCupPickList_ForSelectedRegion(0)` first. */
static void Rebuild_CompetitionSlotPointers(void)
{
    /* Country / pickList tables are `Competition *[]` — 8 bytes per
     * entry on 64-bit. The Amiga path walked them as `uint32_t *`
     * which only read half a pointer per slot. Use `void **` so the
     * stride is correct on both targets. Sentinels stored as
     * `(Competition *)-1` / `-2` compare via (uintptr_t)X == -1u /
     * -2u — both halves of the pointer are all-ones on Amiga and
     * zero-extend identically on 64-bit. */
    void **compTablePtr;

    if (_isNationalSelector != 0) {
        Career_SetCupPickList_ForSelectedRegion(0);
        compTablePtr = (void **)&_careerCupPickList;
    } else {
        uint16_t tableIdx = (_selTeamNumber == 0xFFFF)
            ? SaveJson_GetLoadedPrimaryCountryCode()
            : (_selTeamNumber >> 8);
        compTablePtr = (void **)_competitionsTable[tableIdx];
    }
    if (!compTablePtr)
        return;

    /* Slot 0 (primary): always present. */
    _currentCompetitionPtr = (uint8_t *)*compTablePtr++;
    /* One filler slot after the primary entry (the `(Competition *)-2`
     * sentinel in country tables like albaniaTable). */
    compTablePtr++;

    /* Slot 1 (secondary): may be -1 sentinel. */
    if ((uintptr_t)*compTablePtr != (uintptr_t)-1) {
        _secondaryCompetitionPtr = (uint8_t *)*compTablePtr++;
        if ((uintptr_t)*compTablePtr != (uintptr_t)-1) {
            _tertiaryCompetitionPtr = (uint8_t *)*compTablePtr;
        } else {
            _tertiaryCompetitionPtr = 0;
        }
    } else {
        _secondaryCompetitionPtr = 0;
        _tertiaryCompetitionPtr = 0;
    }
    if (!WorldCups_Plays(_secondaryCompetitionPtr))     /* (as InitNewSeason: world_cups.c) */
        _secondaryCompetitionPtr = 0;
    if (!WorldCups_Plays(_tertiaryCompetitionPtr))
        _tertiaryCompetitionPtr = 0;
    if (!_tertiaryCompetitionPtr &&             /* (a world's state championship in slot 2: states.c) */
        (uint8_t)((DIYCompetition *)&_tertiaryCompetitionContextBuf)->field_2E == COMPID_stateContest)
        _tertiaryCompetitionPtr = States_RestoreContest();

    /* Slot 4 (DIY) — derive from primary comp's DIY link. Mirrors
     * `DIY_SelectCompetitionSlot4FromCurrentDivision`. The diyEntry
     * payload is two pointers (diy comp ptr + diy table ptr) — 8 B
     * stride on 64-bit. Use `void **` read instead of the old
     * `*(uint32_t *)` which truncated. */
    _diyCompetitionPtr = 0;
    _diyCompetitionTablePtr = (uint32_t *)0;
    if (_currentCompetitionPtr != 0) {
        uint8_t *primary = (uint8_t *)_currentCompetitionPtr;
        if (((Competition *)primary)->type == 0) {
            uint16_t off = _currentDivisionIndex * 6;
            uint8_t *entry = primary + off;
            uint8_t diyOffset = entry[0x12];
            if (diyOffset != 0) {
                void **diyEntry = (void **)(entry + 0x12 + diyOffset);
                _diyCompetitionPtr = (uint8_t *)diyEntry[0];
                _diyCompetitionTablePtr = (uint32_t *)diyEntry[1];
            }
        }
    }
    WorldPlayoff_RestoreSlot();     /* a world's play-off under way (world_playoff.c) */
    CupD_RestoreSlot();             /* a Coppa Italia Serie D tie under way (world_cupd.c) */

    /* Slot 3 (quaternary, euro cups) — re-derive from the SAVED euro-cup
     * slot CONTEXT, not from quaternaryCompetitionPtr.
     *
     * The JSON save format no longer stores quaternaryCompetitionPtr (it is
     * a pointer-to-disk field). The previous gate `quaternaryCompetitionPtr
     * != 0` therefore read the stale pre-load value — 0 on a fresh-start
     * load — and left an ACTIVE euro-cup slot inactive. The careerMenu then
     * draws the slot-3 schedule entry, CareerGameListCommon sets
     * compInfo = quaternaryCompetitionPtr = NULL, and `compInfo[5]` SIGSEGVs
     * (Playaveli's CLAUDE.CAR repro, in CareerGameListCommon menus.c:6687).
     *
     * Reliable saved signal: the slot-3 context (euroCupsCompetitionContext)
     * is persisted and restored on load. Its competitionType (+0x00) and
     * isActive (+0x02) are 2/1 for an active European Cup (type-2 group
     * tournament) and 1/1 for the Cup Winners'/UEFA cup (type-1); an
     * inactive slot reads back all-zero. euroCupSlotIndex (also saved)
     * selects which of the three cups; we then VALIDATE that the saved
     * context's type agrees with that cup, so a stale euroCupSlotIndex (it
     * is only written on qualification) or a stale context can't activate a
     * mismatched slot — the original FindTeamInDIY-panic concern. */
    {
        uint8_t *euroCup3 = 0;
        if (_euroCupSlotIndex == 0)      euroCup3 = _euroCupCopy;        /* type 2 */
        else if (_euroCupSlotIndex == 1) euroCup3 = _cupWinnersCupCopy;  /* type 1 */
        else if (_euroCupSlotIndex == 2) euroCup3 = _uefaCupCopy;        /* type 1 */
        else if (_euroCupSlotIndex == 3) {                              /* type 2 (Copa) */
            /* Copa Libertadores shares the quaternary slot (a CONMEBOL club is in
             * none of the euro cups). Re-derive its *Copy so a saved South-American
             * career re-loads with Copa still interactive. The same type-match +
             * Career_TeamInLoadedEuroSlot validation below applies (Copa is type 2,
             * like the European Cup). */
            extern Competition *Career_CopaWorkingCopy(void);
            euroCup3 = (uint8_t *)Career_CopaWorkingCopy();
        }
        else if (_euroCupSlotIndex >= 4 && _euroCupSlotIndex <= 6)    /* (a world's qualifying tie) */
            euroCup3 = UefaQual_RestoreTie(_euroCupSlotIndex);
        else if (_euroCupSlotIndex == 7 || _euroCupSlotIndex == 8)    /* (another confederation's cups) */
            euroCup3 = Conti_RestoreTie(_euroCupSlotIndex);
        else if (_euroCupSlotIndex == 9)                               /* (a world's group contest) */
            euroCup3 = WorldPlayoff_GroupsRecord();

        DIYCompetition *q3 = (DIYCompetition *)_careerCompetitionSlotPtrTable[3];
        /* Participation gate: euroCupSlotIndex is only written on qualification
         * (Career_SelectEuroCupSlotIfTeamParticipates) and is SAVED, so it can
         * be stale from a prior season when the club DID qualify. The euro cup
         * is contested every season by the qualifying clubs, so its slot-3
         * context can be active + type-matching while the player's club is NOT
         * in it. Without the participation check the fixup re-activates slot 3
         * for a non-participant, and ShowChairmanOpinionAboutTheSeason then
         * calls FindTeamInDIY(selTeamNumber, euroCupsCompetitionContext) which
         * hard-panics 0x6006 for a club coach (team not found). Mirror the
         * season-init enrolment: only activate if the club is actually in the
         * cup's team list. (gdb-confirmed 2026-06-10: club 5141 in SERIE A, euro
         * slot held EUROPEAN CHAMPIONS CUP whose Italian entrant is 5142, stale
         * euroCupSlotIndex=0.) */
        /* Gate on the LOADED BUFFER's team list (q3), NOT the static euroCupCopy
         * template. euroCupCopy is reconstructed from its static template on load
         * (default teams, NOT the saved season's qualifiers), so
         * Career_TeamInEuroCup(euroCup3) gives a FALSE NEGATIVE for a club that's
         * in the saved season's cup but not the default template — leaving
         * quaternaryCompetitionPtr NULL and crashing the cached slot-3 game-list
         * row in CareerGameListCommon on compInfo[5]. The saved slot-3 buffer is
         * the season's authoritative field; Career_TeamInLoadedEuroSlot checks it
         * with the same search FindTeamInDIY uses (gate-pass => no 0x6006). euroCup3
         * (the static record) is still the pointer TARGET — it carries the name
         * table the game list reads. */
        if (euroCup3 != 0 && q3->isActive != 0 &&
            q3->competitionType == ((Competition *)euroCup3)->type &&
            Career_TeamInLoadedEuroSlot(q3)) {
            _quaternaryCompetitionPtr = euroCup3;
        } else {
            _quaternaryCompetitionPtr = 0;  /* slot 3 inactive / club not in it this season */
        }
    }
}

/* Step 5 helper: zero the four *CompStringOffset fields on load.
 *
 * These are stale absolute pointers into the strings table (saved
 * verbatim from the saving binary's runtime address space). The only
 * consumer is `InitializeNewSeason` which copies them into the
 * SeasonInformations buffer at offsets 0x16/0x1A/0x1E/0x22 to display
 * the previous season's competition names on the season-end summary.
 * If we zero them, the summary may show empty strings for the prior
 * season's results, but no crash from dereferencing a stale pointer.
 *
 * Future improvement: re-derive each from its slot's compId by
 * looking up the comp record's `text[0]` field (same source as the
 * cup-record nameSource). Deferred until we have a save that exposes
 * a visible regression here. */
static void Rebuild_CompStringOffsets(void)
{
    _primaryCompStringOffset    = 0;
    _secondaryCompStringOffset  = 0;
    _tertiaryCompStringOffset   = 0;
    _quaternaryCompStringOffset = 0;
}

__attribute__((used, externally_visible))
void RebuildPointersAfterLoad(void)
{
    /* Step 1: refresh careerCompetitionSlotPtrTable[0..4]. Same five
     * lines as `InitNewSeason` lines 1165-1169 — but this time after
     * load, so the saved (now stale) pointer values get replaced with
     * the current run's competition-buffer addresses. */
    _careerCompetitionSlotPtrTable[0] = (uint8_t *)&_primaryCompetitionBuf;
    _careerCompetitionSlotPtrTable[1] = (uint8_t *)&_competitionContextBuf;
    _careerCompetitionSlotPtrTable[2] = (uint8_t *)&_tertiaryCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[3] = (uint8_t *)&_euroCupsCompetitionContextBuf;
    _careerCompetitionSlotPtrTable[4] = (uint8_t *)&_diyCompetitionContextBuf;

    /* Step 1b: re-derive each slot buffer's DIYCompetition.namePtr (+0x28).
     *
     * namePtr is a pointer-to-disk field: at save time it holds the
     * SAVING binary's absolute address of the division/cup name string
     * (e.g. divisionStringTable[d7], assigned in InitDIYLeagueFromRecord
     * diy_competition.c:8869 / InitDIYCupFromRecord:8924). The JSON save
     * stores it as raw bytes (primary_name_ptr), so after a cross-target
     * load it's a foreign address — on the Amiga a 64-bit-SDL value like
     * 0x1_004A70B8 truncates to a ~garbage 32-bit pointer. It then flows
     * into SeasonInformations.leagueStrPtr (line ~1067 below) and is
     * dereferenced by the season-recap renderer (statistics.c:2184,
     * StringCopy/RenderCompLine), which walks it to a NUL — off a wild
     * address that smashes the stack (observed: wild PC + "SWOS2" string
     * bytes on the stack walking past the comp buffer).
     *
     * Re-point it at the slot's OWN competitionName[36] buffer (+0x04),
     * which the same init code already filled with the full display name
     * ("GERMAN 1. LIGA"). That's valid, cross-target-stable memory in the
     * just-loaded buffer, so the recap renders and nothing derefs a
     * foreign address.
     *
     * LIMITATION (deferred): the original namePtr was the *bare* division
     * name ("1. LIGA") and is also tested by IDENTITY (== aFriendly,
     * diy_competition.c:2930/3223) and by ADDRESS RANGE (aEuropeanTour..
     * aOceaniaTour, career_status.c:3774) to classify friendlies/tours.
     * Pointing at competitionName loses those — fine for ordinary
     * league/cup (not a friendly, not a tour, and the recap shows the
     * country-prefixed name), but a friendly/tour career would
     * mis-classify. The full fix is a SYMBOLIC save (table-id + division
     * index, restored to the original static-string address) so identity
     * and range survive; see project_primary_comp_block_handoff. This
     * repoint unblocks the common case without that larger change. */
    for (int _slot = 0; _slot < 5; _slot++) {
        DIYCompetition *c = (DIYCompetition *)_careerCompetitionSlotPtrTable[_slot];
        if (c != NULL && c->namePtr != NULL)
            c->namePtr = (uint8_t *)c->competitionName;
        /* The slot's stage snapshot -- a whole diyCompetitionStart image, copied back
         * into the live competition at a stage change (and by Step 2 below) -- carries
         * a namePtr of its own: re-point it the same way, at the slot's name (stable,
         * unlike the snapshot's own bytes). It used to come back as the saving run's
         * address, so a loaded European-cup career put a stale pointer live. */
        if (c != NULL) {
            uint8_t *snap = ((CompetitionSlotBuffer *)c)->stageSnapshot + offsetof(DIYCompetition, namePtr);
            uint8_t *name;
            memcpy(&name, snap, sizeof name);
            if (name != NULL) {
                name = (uint8_t *)c->competitionName;
                memcpy(snap, &name, sizeof name);
            }
        }
    }

    /* Step 2: repopulate diyCompetitionStart (the runtime working
     * struct outside the careerFileBuffer overlay) from the primary
     * slot's saved bytes. Without this, the upcoming-match list
     * shows fixtures generated against whatever competition the
     * previous in-memory state held — typically the player team
     * "playing" every other team at home, with 00'00 kickoff times,
     * because the league fixture order/times live in
     * diyCompetitionStart and not in the saved overlay slot.
     *
     * VIEW SQUADS already worked without this step because that path
     * goes through `Career_OpenCompetitionSlotMenuAndApplyChanges`,
     * which calls Career_LoadCompetitionSlotStateFromBuffer itself.
     * The Play Match / Next Match menus skip that helper, so we
     * have to populate diyCompetitionStart up-front at load time. */
    Career_LoadCompetitionSlotStateFromBuffer((uint8_t *)&_primaryCompetitionBuf);

    /* Step 3: walk careerContests and rewrite each *Copy record's
     * embedded nameSource pointer at (record + 5 + nameOffset).
     * Without this, `InitDIYLeagueFromRecord` SIGSEGVs on a stale
     * string pointer when auto-advancing euroCupCopy /
     * cupWinnersCupCopy / uefaCupCopy after a cross-build load. */
    Rebuild_CupRecordNameSourcePointers();

    /* Step 4: re-derive top-level competition slot pointers
     * (`currentCompetitionPtr` / `secondaryCompetitionPtr` /
     * `tertiaryCompetitionPtr` / `quaternaryCompetitionPtr` /
     * `diyCompetitionPtr` / `diyCompetitionTablePtr`) from
     * competitionsTable + selTeamNumber + euroCupSlotIndex. Saved
     * values were stale BSS addresses from the saving build. */
    Rebuild_CompetitionSlotPointers();

    /* Step 5: zero the four *CompStringOffset fields. They held
     * stale absolute pointers into the strings table; downstream
     * consumers (InitializeNewSeason → SeasonInformations) will
     * tolerate NULL by displaying empty text. */
    Rebuild_CompStringOffsets();
}

/* ================================================================
 * Chunk 198: Career_ViewOrEditCompetitionSlot  (SWOS2.s line 32242)
 *
 * Views or edits a competition slot: maps the competition pointer
 * to its career copy via careerContests table, handles special
 * europeanChampionships/worldCup replacements, then determines which
 * slot owns the competition and shows appropriate menu.
 * If the competition isn't in any active slot, auto-advances it.
 *
 * in:  a0 = competition pointer, d0 = divisionIndex/selection
 * ================================================================ */
extern uint8_t _europeanChampionships[];
extern uint8_t _europeanChampionships2Copy[];
extern uint8_t _worldCup[];
extern uint8_t _worldCup2Copy[];
/* nationalTeamCompetitionsAndQualifications1+0x96 effectively points
 * 6 bytes before careerContests[0] — the loop skips 6 bytes then reads
 * 4 bytes per iteration, matching the 10-byte careerContests entries. */

extern void ShowDiyLeaguesTournamentMenu(uint16_t type, uint16_t viewOnly, uint16_t focusOverride);
extern void ShowDIYCupMenuWithParams(uint16_t arg0, uint16_t arg1, uint16_t arg2);
extern void DIY_EditCompetitionSlot(uint16_t paramA, uint16_t paramB, uint16_t paramC);
extern void SaveSelectedTeams(void);
extern void RestoreSelectedTeams(void);
extern void Career_LoadCompetitionSlotStateFromBuffer(uint8_t *src);
extern void Career_AutoAdvanceInternationalCompetitionFixtures(uint8_t *record, uint16_t index);
extern void Career_AutoAdvanceCupCompetitionFromRecord(uint8_t *record);
extern void Career_AutoAdvanceLeagueCompetitionFromRecord(uint8_t *record);

__attribute__((used, externally_visible))
void Career_ViewOrEditCompetitionSlot(uint8_t *comp, uint16_t sel)
{
    /* Step 1: Map competition pointer to its career copy via the
     * typed CompetitionCareer[] — entry stride differs by bitness
     * (10 B on 32-bit, 18 B on 64-bit), so iterate as the typed
     * array instead of advancing tbl by a literal byte count. */
    extern CompetitionCareer _careerContests[];
    for (CompetitionCareer *e = _careerContests;
         (uintptr_t)e->competition != 0xFFFFFFFFu; e++) {
        if ((uint8_t *)e->competition == comp) {
            comp = (uint8_t *)e->competitionCopy;
            break;
        }
    }

    /* Step 2: Special replacements for europeanChampionships and worldCup.
     * compId 22/4 appear only on these templates (the "2" copies are 21/3),
     * so by-id is exactly equivalent to the old template-address compare. */
    uint8_t cid = comp ? comp_id(comp) : 0xFFu;
    if (cid == COMPID_europeanChampionships)
        comp = _europeanChampionships2Copy;
    if (cid == COMPID_worldCup)
        comp = _worldCup2Copy;

    /* Step 3: Find which slot owns this competition */
    uint8_t *slotCtx = (uint8_t *)&_primaryCompetitionBuf;

    if (comp == (uint8_t *)_currentCompetitionPtr) {
        if (sel == _currentDivisionIndex)
            goto found_slot;
    }

    slotCtx = (uint8_t *)&_competitionContextBuf;
    if (comp == (uint8_t *)_secondaryCompetitionPtr)
        goto found_slot;

    slotCtx = (uint8_t *)&_tertiaryCompetitionContextBuf;
    if (comp == (uint8_t *)_tertiaryCompetitionPtr)
        goto found_slot;

    slotCtx = (uint8_t *)&_euroCupsCompetitionContextBuf;
    if (comp == (uint8_t *)_quaternaryCompetitionPtr)
        goto found_slot;

    /* Expanded VIEW WORLD is a pure observation path: it must NEVER run
     * the OG auto-advance callbacks when selecting an external competition.
     * Resident OG results are simulated by START/FORWARD TIME, not menus. */
#ifdef SWOS_SDL_BUILD
    {
        extern int Expanded_OGWorld_ViewLeague(const uint8_t *,uint16_t);
        (void)Expanded_OGWorld_ViewLeague(comp,sel);
    }
#endif
    return;

found_slot:
    Career_LoadCompetitionSlotStateFromBuffer(slotCtx);
    {
        uint8_t compType = slotCtx[1];
        if (compType == 1) {
            ShowDIYCupMenuWithParams(1, 1, 0);
        } else if (compType == 2) {
            DIY_EditCompetitionSlot(1, 1, 0);
        } else {
            ShowDiyLeaguesTournamentMenu(1, 1, 0);
        }
    }
}
