/*
 * Amiga SWOS RE Project
 * SWOS Amiga C/68000 Reversed by Ben Baker (https://github.com/benbaker76)
 * SWOS DOS Reversed by Zlatko Karakaš (https://github.com/zlatkok)
 */
/* play_match_menu.c — Play Match Menu UI functions
 *
 * Chunk 98: PlayMatchMenuInit, PlayMatchMenuReinit, SetCurrentSelectedPlayerIndexPointer,
 * OnSwapPlayer, PlayMatchMenuSwapPlayers, HandleSquadSwapSelectionToggle,
 * PlayMatchChangeTactics, ExitPlayMatch, PlayMatchSelected, EditTacticsInPlayMatchMenu,
 * ViewOpponentsSelect, ReturnFromViewOpponent, PlayMatchOnSelect30, PlayMatch30AfterDraw,
 * UpdateTeamCheckmark, SetGameStartButtonText, ShowOrHidePlayMatchButton,
 * SetUpTacticsButtons, SetPlayerSurnamesOnSidePitch, ShowUseMatchTeamIfTraining,
 * SetVariableButtons, DrawPlayerEntries, RedrawPlayerNames, UpdateLittleCheckmark,
 * SavePositionsAndUpdateLittleCheckmark, UpdateCheckMarkSprite, UseMatchTeamSelect,
 * TogglePlayerCoach, GetPlayerPosition.
 */

#include "play_match_menu.h"
#include "endian_helpers.h"
#include "menu_system.h"
#include "game_types.h"
#ifdef SWOS_SDL_BUILD
#include "uniform.h"            /* the UNIFORM button (entry 68) */
#include "custom_team.h"        /* (CUSTOM) where the country would be named */
#include <stdio.h>
#include <stdlib.h>
#endif

/* Prevent GCC from converting byte loops to memcpy/memset (no libc) */
#pragma GCC optimize("no-tree-loop-distribute-patterns")

/* ---- Assembly externs (globals in SWOS2.s) ---- */
extern MenuEntry _menuEntries[];
extern uint16_t _fireResetFlag;
extern uint32_t _irqCookieObfuscated;
extern uint16_t _player1ClearFlag;
extern uint16_t _player2ClearFlag;
extern uint16_t _exitGameFlag;
extern uint16_t _isCareer;
extern uint16_t _longFireFlag;
extern uint8_t  _pl1Fire;
extern uint8_t  _pl2Fire;
extern uint16_t _numPlayers;
extern uint16_t _careerNumPlayers;
extern uint16_t _gameType;
extern uint16_t _trainingGame;
extern uint16_t _coachOrPlayer;
extern uint16_t _currentTeamNumber;
extern uint16_t _selTeamNumber;
extern uint16_t _panicReasonCode;
extern uint16_t _teamSkillCurrent;
extern uint16_t _teamSkillSnapshot;

extern TeamFileEx _currentTeam;
extern TeamFileEx _careerTeam;
extern uint8_t _playerPositions[];
extern uint8_t _currentMatchPlayers[];
extern uint8_t _playerPositionsArray[];
extern uint8_t _oldPlayerPositionsArray[];
extern void *_teamTacticsTable[];
extern void *_positionsTable[];
extern void *_countriesTable[];

/* Globals defined in play match menu data section of SWOS2.s */
extern uint16_t _showingCpuTeams;
extern uint16_t _squadChangesAllowed;
extern TeamFileEx *_playMatchTeam1Ptr;
extern TeamFileEx *_playMatchTeam2Ptr;
extern uint16_t _maxSubstitutesEntries;
extern uint16_t _pmMenuNumPlayers;
extern uint16_t _numPlayersToShowInMenu;
extern uint16_t _viewingOpponentTeam;
extern uint16_t _is_A_team;
extern int16_t  _selectedPlayerPosition;
extern int16_t  _selectedPlayerTacticsPosition;
extern uintptr_t _pmSelectedPlayerEntry;
extern uint16_t _playMatchControlFlags;
extern uint16_t _playerNumThatStarted;
extern int16_t  _playMatchExcludeFlags;
extern uint16_t _playMatchButtonVisibilityState;
extern uint16_t _careerTeamCoachOnly;
extern uint16_t _isFirstPlayerChoosingTeam;
extern uintptr_t _currentSelectedPlayerIndexPtr;
extern uint16_t _currentTeamSelectedPlayerPosition;
extern uint16_t _selTeamSelectedPlayerPosition;
extern uint16_t _team1SelectedPlayerIndex;
extern uint16_t _team2SelectedPlayerIndex;
extern uint32_t _allowShorterMenuItemsWithFrames;

#ifdef SWOS_SDL_BUILD
extern uint8_t _currentMenu[];
extern uintptr_t _currentMenuPtr;
extern uint8_t _playMatchMenu[];
extern uint16_t TrainingMatch_BSourceCareerSlot(uint16_t bFileIdx);
extern uint16_t TrainingMatch_SyncBForCareerSwap(uint16_t aSlot, uint16_t bSlot);
extern uint16_t TrainingMatch_KeepCrossSwapForMatch(uint16_t fileA,
                                                    uint16_t fileB, int swapShirtNumbers);
/* v332: the competition's REAL bench size (0 = use _maxSubstitutesEntries); set by the CE native prepare. */
uint16_t g_pmRealBench;
/* Original pre-match menu: extra true career players 16..31 (SDL only).
 * No synthetic TEAM B, no opponent editing, no second match controller. */
/* Reserve row footprint: name x=140+S width82, position x=223+S width14.
 * In OG 320px logical width S<=82 to leave a border; choose S=80. */
enum { kCareerExtraFirst=69, kCareerExtraCount=16, kCareerExtraShiftX=80 };
static int16_t s_extraReserveSlot=-1, s_extraDrawSlot=-1;
/* Original OG packed navigation and pitch visibility saved at every menu
 * unpack; never leak extra-list links into opponent/other match menus. */
static int8_t s_extraNativeRight[17];
static uint8_t s_extraNativePitchInvisible[12];
static uint8_t s_extraNativeDisabled[kCareerExtraCount];
static int16_t s_extraNativeX[kCareerExtraCount];
extern int Expanded_OGWorld_BoliviaModalActive(void);
extern int Expanded_OGWorld_BoliviaManagerInteractive(void);
extern int Expanded_OGWorld_BoliviaManagerAutoSim(void);
extern int Expanded_OGWorld_BoliviaModalManagerFixtureArmed(void);
extern int Expanded_OGWorld_IsSimulating(void);
/* Set only around a real user-facing OG league RESULT action-0 SetupPlayers.
 * Never a new UI simulator, scoring callback or second clock. */
static int s_nativeOGManagerResultPrompt;
void Expanded_CareerNativeOGResultPromptSet(int enabled)
{
    s_nativeOGManagerResultPrompt=enabled!=0;
}
static int16_t s_extraNativeCheckX;
static int s_extraNavReady, s_extraLastVisible=-1;

static int Expanded_ManagedCareerTeam(const TeamFileEx *team)
{
    if(!team || _gameType!=GAME_TYPE_CAREER)return 0;
    if(team==&_careerTeam)return 1;
    return !_trainingGame && _selTeamNumber!=0xFFFF &&
        PACK_BE_U16(team->team.countryNumber,
                    team->team.teamOrdinal)==_selTeamNumber;
}
/* Use the original native PLAY permission whenever available. In a live
 * original OG Bolivia modal, action=VIEW RESULT can also prompt the true
 * MANAGER fixture just BEFORE that fixture is scored. Unlike the previous
 * blanket career readonly guard, this narrowly lets the manager arrange
 * their OWN pending OG lineup, never CPU/auto-sim/processed results. The
 * modal's preflight already validated its date, fixture and manager mode.
 * Do not restore the removed career SIMULATE MATCH contest button. */
static int Expanded_CareerLineupEditsAllowed(void)
{
    if (_squadChangesAllowed) return 1;
    if (_gameType!=GAME_TYPE_CAREER || _trainingGame ||
        _showingCpuTeams || _viewingOpponentTeam ||
        !Expanded_ManagedCareerTeam(_playMatchTeam1Ptr))
        return 0;
    /* Resident OG ownership is NOT an auto-sim indicator. A verified,
     * armed manager fixture remains resident while its ORIGINAL action0
     * shows the pre-result lineup. The old !IsSimulating check was
     * contradictory and rejected every such legitimate Copa menu. */
    if (Expanded_OGWorld_BoliviaModalActive())
        return Expanded_OGWorld_BoliviaManagerInteractive() &&
            !Expanded_OGWorld_BoliviaManagerAutoSim() &&
            Expanded_OGWorld_BoliviaModalManagerFixtureArmed();
    /* A distinct original, user-entered local OG league RESULT prompt.
     * Its source path guarantees no season update / CPU auto-advance;
     * no stale modal fixture, no unbound team, no autonomous simulation. */
    return s_nativeOGManagerResultPrompt &&
        !Expanded_OGWorld_IsSimulating();
}
static int Expanded_CareerExtraVisible(void)
{
    /* Display is independent of permission: a readonly manager result/CPU
     * preview still has real native reserve records. Editing is guarded
     * separately by _squadChangesAllowed in OnSwapPlayer and menu focus. */
    if (_gameType!=GAME_TYPE_CAREER || _trainingGame ||
        _showingCpuTeams || _viewingOpponentTeam || !_playMatchTeam1Ptr ||
        _selTeamNumber==0xFFFF || _careerNumPlayers<=16 || _careerNumPlayers>32)
        return 0;
    return Expanded_ManagedCareerTeam(_playMatchTeam1Ptr);
}
static void Expanded_CareerExtraUpdate(void)
{
    const int visible=Expanded_CareerExtraVisible();
    const int canEdit=Expanded_CareerLineupEditsAllowed();
    const unsigned extras=visible?(unsigned)_careerNumPlayers-16u:0u;
    for(unsigned i=0;i<kCareerExtraCount;++i) {
        MenuEntry *reserve=&_menuEntries[kCareerExtraFirst+i];
        /* Native hitbox/focus moves together with OG row contents;
         * recompute from unpacked original x, never accumulate shifts. */
        if(s_extraNavReady)
            reserve->x=(int16_t)(s_extraNativeX[i]+(visible?kCareerExtraShiftX:0));
        reserve->isInvisible=!visible || i>=extras;
        /* A true CPU/result viewer remains visually available, not editable. */
        reserve->isDisabled=visible && !canEdit ?
            1 : s_extraNativeDisabled[i];
    }
    if(s_extraNavReady) {
        for(unsigned i=0;i<17u;++i) {
            /* RIGHT from any original player enters the reserve column.
             * LEFT from the reserve row is already linked in JSON. */
            _menuEntries[13u+i].rightEntry=(visible && canEdit)?
                (int8_t)(kCareerExtraFirst+
                         (i<extras?i:extras-1u)):
                s_extraNativeRight[i];
        }
        /* Native side-pitch art + surnames, not tactic buttons.
         * Their original visibility is restored for all other contexts. */
        for(unsigned i=0;i<12u;++i)
            _menuEntries[56u+i].isInvisible=visible?1:
                s_extraNativePitchInvisible[i];
    }
#ifndef NDEBUG
    if(s_extraLastVisible!=visible && _gameType==GAME_TYPE_CAREER &&
       _careerNumPlayers>16u) {
        const char *capture=getenv("SWOS_DIAGNOSTIC_CAPTURE");
        if(capture && capture[0]=='1')
            fprintf(stderr,
                "[career-prematch] extras-visible=%d roster=%u bench=%u "
                "squad-changes=%u modal-manager-edit=%u training=%u "
                "opponent=%u cpu-view=%u club=%04x shown=%04x controls=%u\n",
                visible,(unsigned)_careerNumPlayers,
                (unsigned)_maxSubstitutesEntries,
                (unsigned)_squadChangesAllowed,
                (unsigned)canEdit,(unsigned)_trainingGame,
                (unsigned)_viewingOpponentTeam,(unsigned)_showingCpuTeams,
                (unsigned)_selTeamNumber,
                _playMatchTeam1Ptr?
                    (unsigned)PACK_BE_U16(
                        _playMatchTeam1Ptr->team.countryNumber,
                        _playMatchTeam1Ptr->team.teamOrdinal):0xffffu,
                _playMatchTeam1Ptr?
                    (unsigned)_playMatchTeam1Ptr->team.teamControls:0u);
    }
#endif
    s_extraLastVisible=visible;
}
static int s_trainingCrossSwapRunning; /* suppress the automatic B rebuild for X */
static uint16_t s_trainingCrossSide; /* 0 none; 1 A; 2 B */
static uint16_t s_trainingCrossSlot;
int Expanded_TrainingCrossSwapAvailable(void)
{
    return _trainingGame && _gameType==GAME_TYPE_CAREER &&
           _squadChangesAllowed && !_viewingOpponentTeam &&
           !_showingCpuTeams &&
           _currentMenuPtr==(uintptr_t)_playMatchMenu &&
           (_playMatchTeam1Ptr==&_careerTeam ||
            _playMatchTeam1Ptr==&_currentTeam);
}
#endif

/* String constants from SWOS2.s */
extern uint8_t _aViewResult[];
extern uint8_t _aPlayCoachMatch[];
extern uint8_t _aCoachMatch[];
extern uint8_t _aViewMatch[];
extern uint8_t _aViewOppo[];
extern uint8_t _aViewTeam[];
extern uint8_t _aViewOtherTeam[];
extern uint8_t _aViewATeam[];
extern uint8_t _aViewBTeam[];
extern uint8_t _aCoach_1[];
extern uint8_t _aPlayer_1[];
extern uint8_t _fundsBuffer[];
extern void *_playerPositionsStringTable[];
extern uint8_t _teamAName[];
extern uint8_t _teamBName[];
extern void *_sidePitchSurnamePositionsTable[];
extern int16_t _playMatchRedrawList[];
extern int16_t _togglePlayerCoachRedrawItemList[];

/* ---- C-ported function externs ---- */
extern void CheckIsTeamNational(void);
extern void SetBenchPlayersNumbers(TeamFileEx *team);
extern int32_t SquadNumbers_On(void);           /* (real squad numbers: teams.c) */
extern void SquadNumbers_Free(uint16_t slot);
extern void DrawMenu(void);
extern void CallDrawMenuItem(MenuEntry *entry);
extern void SetCurrentEntry(uint16_t index);
extern void DrawMultipleItems(uint16_t maxItems, MenuEntry *firstEntry);
extern void CallDrawMultipleItems(uint16_t maxItems, MenuEntry *firstEntry);
extern void DrawMenuItemsWaitForVSync(const int16_t *list);
extern void SetExitMenuFlag(void);
extern MenuEntry *CalcMenuEntryAddress(uint16_t ordinal);
extern uint8_t *ExtractSurname(uint8_t *name, uint16_t maxLenAndFlags);
extern uint8_t *StringCopy(const uint8_t *src, uint8_t *dst);
extern uint16_t IsTeamNational(TeamFileEx *team);
extern void InitPlayerValueDisplayStruct(uint8_t *playerEntry, uint8_t *dst);
extern uint16_t IsPlayerInjured(PlayerFile *playerFile, int16_t mode);
/* Honest injury/suspension status for DISPLAY decisions — no beta override. */
extern uint16_t IsPlayerInjuredForDisplay(PlayerFile *playerFile, int16_t mode);
#ifdef SWOS_SDL_BUILD
extern uint16_t Career_IsSelectionUnavailable(const PlayerFile *player,int16_t flags);
#endif
extern uint16_t GetNumberOfSubstitutes(uint8_t *team, uint16_t numPl);
extern uint16_t GetNumberOfSubstitutesInSelectionList(uint8_t *team, uint16_t extraCount);
extern uint32_t GetNumberOfAvailablePlayers(uint8_t *team, int16_t excludeCardsFlag, uint16_t numPl);
extern uint32_t GetNumberOfAvailableSubstitutes(uint8_t *team, int16_t flag);
extern void RotatePlayersLeft(TeamFileEx *team, uint16_t maxSubs);
extern int16_t CheckIfTeamIsChanged(void);
extern uint8_t *GetPlayerInfoBreakPosition(TeamFileEx *teamPtr, uint16_t requestedSlot,
                                             uint16_t *outSlot, uint16_t *outPlayerIndex);
extern uint8_t *GetPlayerAtIndex(uint16_t teamNumber, uint16_t ordinal);
extern uint8_t *GetPlayer(uint16_t teamNumber, uint16_t playerFileIndex);
extern uint16_t _cGetPlayerAtIndexFileIdx;
extern uint16_t ComputeOldPlayerSkillForViewResult(TeamFileEx *team, uint16_t goalsCount);
extern void ShowStandardChooseTacticsMenu(TeamFileEx *team);
extern void TrainingMatch_RestoreLastMatchState(void);
extern void TrainingMatch_RestoreSavedTeamSelectionState(void);

extern void FormTopSkillsString(uint8_t *dst, PlayerFile *playerFile);
extern void FormatPlayerValueOrStatusText(uint8_t *base, uint8_t *dst);
extern void SetSquadMenuEntryStatusSprite(MenuEntry *entry, PlayerFile *playerFile, uint16_t flag);
extern void TrainingMatch_BuildCurrentTeamFromCareerTeam(void);
extern void TrainingMatch_ApplyUseMatchTeamAndSwapSubs(void);

#ifdef SWOS_SDL_BUILD
/* True OG selection from the club's ordered _currentMatchPlayers slots.
 * This native menu can nominate fewer substitutes than the competition
 * maximum; a legal XI with bench=0 must not become unplayable solely
 * because the original 16-player gate insists on >=1 substitute. */
static int Expanded_CareerPrematchLegalSelection(const TeamFileEx *team)
{
    if(!Expanded_ManagedCareerTeam(team) || _selTeamNumber==0xFFFF ||
       _trainingGame || !_squadChangesAllowed || _showingCpuTeams || _viewingOpponentTeam ||
       team->team.teamControls==TEAM_COMPUTER)return 0;
    const unsigned n=_careerNumPlayers;
    const unsigned bench=_maxSubstitutesEntries;
    if(n<11u || n>32u || bench>8u || 11u+bench>n)return 0;
    uint32_t seen=0;
    for(unsigned i=0;i<11u+bench;++i) {
        const unsigned id=_currentMatchPlayers[i];
        if(id>=32u || (seen&(1u<<id)))return 0;
        seen|=1u<<id;
        const PlayerFile *pf=(const PlayerFile *)
            (GetPlayer(_selTeamNumber,(uint16_t)id)+76);
        if(Career_IsSelectionUnavailable(pf,_playMatchExcludeFlags))return 0;
        const unsigned pos=((unsigned)pf->positionAndFace&PF_POSITION_MASK)
                            >>PF_POSITION_SHIFT;
        if((i==0u && pos!=0u) || (i>0u && i<11u && pos==0u))
            return 0;
    }
    return 1;
}
#endif

#ifdef SWOS_SDL_BUILD
/* v293: the ORIGINAL block for an incomplete squad. Stock SWOS hides the match
 * button while an injured/suspended player stands in the first 11 + bench
 * entries (GetNumberOfAvailablePlayers(11+maxSubs) disabled != 0, see
 * ShowOrHidePlayMatchButton). The stock counters read the OG 16-slot list of
 * the TeamFileEx they are given; the manager's club here is a selected-team
 * COPY whose real order lives in _currentMatchPlayers + the club roster
 * (GetPlayer), so the stock gate looked at another list and never blocked.
 * Same predicate, same slots (11+bench; the extra reserve row below the list
 * stays outside, exactly as before), evaluated on the real lineup. Returns 1
 * only when the manager can actually fix the lineup (edits allowed) and every
 * slot id is valid, so it can never soft-lock a read-only view. */
static int Expanded_CareerPrematchHasUnavailable(const TeamFileEx *team)
{
    if(!Expanded_ManagedCareerTeam(team) || team==&_careerTeam ||
       _selTeamNumber==0xFFFF || _trainingGame || _showingCpuTeams ||
       _viewingOpponentTeam || team->team.teamControls==TEAM_COMPUTER ||
       !Expanded_CareerLineupEditsAllowed())return 0;
    const unsigned n=_careerNumPlayers;
    const unsigned bench=_maxSubstitutesEntries;
    if(n<11u || n>32u || bench>8u || 11u+bench>n)return 0;
    uint32_t seen=0;
    int unavailable=0;
    for(unsigned i=0;i<11u+bench;++i) {
        const unsigned id=_currentMatchPlayers[i];
        if(id>=32u || (seen&(1u<<id)))return 0;
        seen|=1u<<id;
        const PlayerFile *pf=(const PlayerFile *)
            (GetPlayer(_selTeamNumber,(uint16_t)id)+76);
        if(Career_IsSelectionUnavailable(pf,_playMatchExcludeFlags))
            unavailable=1;
    }
    return unavailable;
}
#endif

/* ---- Helper: menu entry from byte offset ---- */

/* Helper: access bg as the high 16 bits (big-endian BACKANDFRAMECOLOR) */

/* ================================================================
 * GetPlayerPosition  (SWOS2.s line 79648)
 *
 * Reverse lookup: given team + player file index, return position slot.
 * ================================================================ */
__attribute__((used, externally_visible))
int16_t GetPlayerPosition(TeamFileEx *team, int16_t playerIndex) {
    int16_t total;
    if (team == &_careerTeam
#ifdef SWOS_SDL_BUILD
        || Expanded_ManagedCareerTeam(team)
#endif
       )
        total = (int16_t)_careerNumPlayers;
    else if (team == &_currentTeam)
        total = (int16_t)_numPlayers;
    else
        total = 16;

    int16_t i;
    for (i = total - 1; i >= 0; i--) {
        uint16_t slot, fIdx;
        GetPlayerInfoBreakPosition(team, (uint16_t)i, &slot, &fIdx);
        if ((int16_t)fIdx == playerIndex)
            return i;
    }
    _panicReasonCode = 0x5996;
    return -1;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void SetCurrentSelectedPlayerIndexPointer(void) {
    uint16_t *a0 = &_team1SelectedPlayerIndex;
    uint16_t *a1 = &_team2SelectedPlayerIndex;
    if (!_isFirstPlayerChoosingTeam) { uint16_t *t = a0; a0 = a1; a1 = t; }
    if (_viewingOpponentTeam)        { uint16_t *t = a0; a0 = a1; a1 = t; }
    _currentSelectedPlayerIndexPtr = (uintptr_t)a0;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void UpdateCheckMarkSprite(void) {
    MenuEntry *e = &_menuEntries[0];
    e->isInvisible = 1;
    if (_viewingOpponentTeam) return;
    e->stringFlags &= ~(1u << 13);
    if (_teamSkillCurrent == _teamSkillSnapshot) return;
    e->fgWord = (_teamSkillCurrent > _teamSkillSnapshot) ? SPR_CHECK_BIG : SPR_CROSS_BIG;
    e->stringFlags |= (1u << 13);
    e->isInvisible = 0;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void UpdateLittleCheckmark(void) {
    _teamSkillSnapshot = _teamSkillCurrent;
    int16_t i; for (i = 0; i < 32; i++) _oldPlayerPositionsArray[i] = _playerPositionsArray[i];
    UpdateCheckMarkSprite();
}

/* ================================================================ */
__attribute__((used, externally_visible))
void SavePositionsAndUpdateLittleCheckmark(void) {
    _teamSkillSnapshot = _teamSkillCurrent;
    int16_t i; for (i = 0; i < 32; i++) _oldPlayerPositionsArray[i] = _playerPositionsArray[i];
    TeamFileEx *team = _playMatchTeam1Ptr;
    team->team.playerValuesDirty = TEAM_VALUES_DIRTY;
    ComputeOldPlayerSkillForViewResult((TeamFileEx *)_playMatchTeam1Ptr, _maxSubstitutesEntries);
    UpdateCheckMarkSprite();
}

/* ================================================================ */
__attribute__((used, externally_visible))
#ifdef SWOS_SDL_BUILD
/* Set while the online lobby shows this screen (online_menus.c): the PLAY button's
 * label -- "CONFIRM", since there it confirms the side rather than starting a match. */
const char *g_playMatchStartLabel;
#endif

void SetGameStartButtonText(void) {
    MenuEntry *e = &_menuEntries[9];
    e->bgColor = (e->bgColor & 0xFFF0) | YELLOW_GREEN_BLACK_14;
    e->fg = (uintptr_t)_aViewResult;
    if (!_squadChangesAllowed) return;

    TeamFileEx *t1 = _playMatchTeam1Ptr;
    TeamFileEx *t2 = _playMatchTeam2Ptr;
    if (t1->team.teamControls == TEAM_COMPUTER &&
        t2->team.teamControls == TEAM_COMPUTER) {
        e->fg = (uintptr_t)_aViewMatch;
    } else {
        e->fg = (uintptr_t)_aPlayCoachMatch;
        if (t1->team.teamControls != TEAM_PLAYER_COACH)
            e->fg = (uintptr_t)_aCoachMatch;
    }
#ifdef SWOS_SDL_BUILD
    if (g_playMatchStartLabel)
        e->fg = (uintptr_t)g_playMatchStartLabel;
#endif
    e->bgColor = (e->bgColor & 0xFFF0) | RED_10;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void ShowUseMatchTeamIfTraining(void) {
    _menuEntries[5].isInvisible = 1;
    if (!_trainingGame) return;
    if (_is_A_team) return;
    if (!CheckIfTeamIsChanged()) return;
    _menuEntries[5].isInvisible = 0;
}

/* ================================================================
 * ShowOrHidePlayMatchButton  (line 81484)
 *
 * Very complex control flow. Translated label-by-label.
 * Labels mapped:
 *   loc_14855C  → notViewingOpponent
 *   loc_148590  → afterTrainingTextSet
 *   loc_1485D8  → careerPath
 *   loc_1485FE  → checkAvailSubs
 *   loc_148606  → checkMaxDisabled
 *   loc_14860E  → checkSubstitutes
 *   loc_148654  → checkAvailPlayersLimit
 *   loc_1486D0  → canPlay
 *   loc_1486D4  → nonCareerSubCheck
 *   loc_14872A  → nonCareerAvailPlayersLimit
 *   loc_14874A  → canPlay (shared)
 *   loc_148748  → cannotPlay
 *   loc_148760  → setVisState1
 *   loc_148768  → showPlayBtn (clr entry $1F8 invisible)
 *   loc_14876E  → hidePlayBtnPath
 *   loc_148780  → hidePlayBtn (set entry $1F8 invisible=1)
 * ================================================================ */
__attribute__((used, externally_visible))
void ShowOrHidePlayMatchButton(void) {
    _menuEntries[8].isInvisible = 1;

    if (_viewingOpponentTeam) {
        TeamFileEx *t2 = _playMatchTeam2Ptr;
        if (t2->team.teamControls != 1) goto hidePlayBtnPath;
        if (!_squadChangesAllowed) goto hidePlayBtnPath;
        _menuEntries[8].isInvisible = 0;
        if (!_playMatchButtonVisibilityState) goto hidePlayBtnPath;
        goto canPlay;
    }

    /* notViewingOpponent */
    _menuEntries[8].isInvisible = 0;
    {
        MenuEntry *vb = &_menuEntries[7];
        vb->stringFlags &= ~(1u << 13);
        if (_trainingGame) {
            vb->fg = (uintptr_t)_aViewATeam;
            if (!_is_A_team)
                vb->fg = (uintptr_t)_aViewBTeam;
        }
    }

    {
        TeamFileEx *team = _playMatchTeam1Ptr;
#ifdef SWOS_SDL_BUILD
        /* The stock gate expects one or more available subs even when the
         * legal nominated bench is zero. Check actual OG lineup identities
         * and availability, leaving all non-manager menus unchanged. */
        if(Expanded_CareerPrematchLegalSelection(team))goto canPlay;
        /* OG block: an injured/suspended player in 11+bench hides the button */
        if(Expanded_CareerPrematchHasUnavailable(team))goto showPlayBtnPath;
#endif
        uint32_t subsR = GetNumberOfAvailableSubstitutes((uint8_t *)team, _playMatchExcludeFlags);
        uint16_t d2 = (uint16_t)(subsR & 0xFFFF); /* d7→d2: available subs (goalkeepers) */

        uint32_t plR = GetNumberOfAvailablePlayers((uint8_t *)team, _playMatchExcludeFlags, 0);
        /* d7 = available, d6 = disabled (after bridge unpack) */
        /* But C returns packed: low=available, high=disabled */
        uint16_t d7_avail = (uint16_t)(plR & 0xFFFF);
        uint16_t d6_disabled = (uint16_t)(plR >> 16);

        if (_gameType == GAME_TYPE_CAREER) {
            /* careerPath */
            if (team == &_currentTeam) {
                uint32_t csR = GetNumberOfAvailableSubstitutes((uint8_t *)&_careerTeam, _playMatchExcludeFlags);
                uint16_t careerSubs = (uint16_t)(csR & 0xFFFF);
                if (careerSubs >= 2) goto checkMaxDisabled;
            }
            /* checkAvailSubs */
            if (d2 < 1) goto cannotPlay;
        checkMaxDisabled:
            if (d6_disabled > 4) goto cannotPlay;
        } else {
            if (d2 < 1) goto cannotPlay;
            if (d6_disabled > 2) goto cannotPlay;
        }

        /* checkSubstitutes */
        if (_gameType == GAME_TYPE_CAREER) {
            uint16_t ns = GetNumberOfSubstitutes((uint8_t *)team, 1);
            if (ns != 1) goto showPlayBtnPath;

            if (_maxSubstitutesEntries > 5) {
                uint16_t nss = GetNumberOfSubstitutesInSelectionList((uint8_t *)team, 1);
                if (nss != 1) goto showPlayBtnPath;
            }

            /* checkAvailPlayersLimit */
            uint32_t apR = GetNumberOfAvailablePlayers((uint8_t *)team, _playMatchExcludeFlags,
                                                         11 + _maxSubstitutesEntries);
            uint16_t disabled2 = (uint16_t)(apR >> 16);
            if (disabled2 != 0) goto showPlayBtnPath;

            if (_trainingGame && team == &_careerTeam) {
                uint16_t ns2 = GetNumberOfSubstitutes((uint8_t *)&_currentTeam, 1);
                if (ns2 != 1) {
                    /* Set blink on view button */
                    MenuEntry *vb = &_menuEntries[7];
                    vb->stringFlags |= (1u << 13);
                    uint32_t sr2 = GetNumberOfAvailableSubstitutes((uint8_t *)team, _playMatchExcludeFlags);
                    if ((uint16_t)(sr2 & 0xFFFF) < 2) goto cannotPlay;
                    goto showPlayBtnPath;
                }
            }
            goto canPlay;
        } else {
            /* nonCareerSubCheck */
            uint16_t ns = GetNumberOfSubstitutes((uint8_t *)team, 1);
            if (ns != 1) goto showPlayBtnPath;

            if (_maxSubstitutesEntries > 5) {
                uint32_t sr3 = GetNumberOfAvailableSubstitutes((uint8_t *)team, _playMatchExcludeFlags);
                if ((uint16_t)(sr3 & 0xFFFF) < 2) goto nonCareerAvailPlayersLimit;
                uint16_t nss2 = GetNumberOfSubstitutesInSelectionList((uint8_t *)team, 1);
                if (nss2 != 1) goto showPlayBtnPath;
            }

        nonCareerAvailPlayersLimit:
            {
                uint32_t apR2 = GetNumberOfAvailablePlayers((uint8_t *)team, _playMatchExcludeFlags, 11);
                uint16_t disabled3 = (uint16_t)(apR2 >> 16);
                if (disabled3 != 0) goto showPlayBtnPath;
                goto canPlay;
            }
        }
    }

cannotPlay:
    /* loc_148748: bra.s loc_148768 — jump directly to showPlayBtn,
     * bypassing canPlay's visibility-state check. When the player roster
     * is invalid (not enough subs/players), the button is always shown
     * so the user can still VIEW RESULT. */
    goto showPlayBtn;

canPlay:
    /* loc_14874A */
    if (_playMatchTeam1Ptr == &_currentTeam) {
        if (!_playMatchButtonVisibilityState) goto hidePlayBtn;
        /* else fall through to showPlayBtn */
    } else {
        _playMatchButtonVisibilityState = 1;
    }
showPlayBtn:
    /* loc_148768 */
    _menuEntries[9].isInvisible = 0;
    return;

showPlayBtnPath:
    /* loc_14876E */
hidePlayBtnPath:
    if (_playMatchTeam1Ptr != &_currentTeam)
        _playMatchButtonVisibilityState = 0;
hidePlayBtn:
    /* loc_148780 */
    _menuEntries[9].isInvisible = 1;
    return;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void UpdateTeamCheckmark(void) {
    _menuEntries[37].isInvisible = 1;
#ifdef SWOS_SDL_BUILD
    if (!Expanded_CareerLineupEditsAllowed()) return;
#else
    if (!_squadChangesAllowed) return;
#endif

    int16_t d1;
    TeamFileEx *team1 = _playMatchTeam1Ptr;
    int managed=team1==&_careerTeam;
#ifdef SWOS_SDL_BUILD
    managed=Expanded_ManagedCareerTeam(team1);
#endif

    if (_viewingOpponentTeam) {
        uint16_t *ip = (uint16_t *)_currentSelectedPlayerIndexPtr;
        d1 = (int16_t)*ip;
        if (d1 < 0) return;
        d1 = GetPlayerPosition(_playMatchTeam2Ptr, d1);
    } else {
        if (managed)
            d1 = (int16_t)_selTeamSelectedPlayerPosition;
        else if (team1 == &_currentTeam)
            d1 = (int16_t)_currentTeamSelectedPlayerPosition;
        else {
            uint16_t *ip = (uint16_t *)_currentSelectedPlayerIndexPtr;
            d1 = (int16_t)*ip;
        }
        if (d1 < 0) return;
        d1 = GetPlayerPosition(team1, d1);
        uint16_t allowed=_pmMenuNumPlayers;
#ifdef SWOS_SDL_BUILD
        if(managed && Expanded_CareerExtraVisible())
            allowed=_careerNumPlayers;
#endif
        if ((uint16_t)d1 >= allowed) {
            if (team1 == &_currentTeam) _currentTeamSelectedPlayerPosition = 0xFFFF;
            else if(managed)_selTeamSelectedPlayerPosition = 0xFFFF;
            return;
        }
    }

    {
        MenuEntry *ck = &_menuEntries[37];
        ck->stringFlags |= (1u << 13);
        MenuEntry *pe = CalcMenuEntryAddress(13 + d1);
#ifdef SWOS_SDL_BUILD
        if(managed && Expanded_CareerExtraVisible() && d1>=16) {
            pe=CalcMenuEntryAddress(kCareerExtraFirst+(uint16_t)(d1-16));
            ck->x=(int16_t)(pe->x-12);
        } else if(s_extraNavReady)
            ck->x=s_extraNativeCheckX;
#endif
        ck->y = pe->y;
        ck->isInvisible = 0;
    }
}

/* ================================================================ */
__attribute__((used, externally_visible))
void SetUpTacticsButtons(void) {
    TeamFileEx *team = _playMatchTeam1Ptr;
    if (_viewingOpponentTeam) team = _playMatchTeam2Ptr;
    uint8_t curTact = team->team.tactics;

    MenuEntry *e = &_menuEntries[38];
    int16_t i;
    for (i = 0; i < 18; i++, e++) {
        uint16_t c = e->bgColor & 0xFFF0;
        if (_viewingOpponentTeam || _showingCpuTeams) {
            c |= (i == curTact) ? PINK_TO_BROWN_1 : PINK_TO_BROWN_9;
        } else {
            c |= (i == curTact) ? ORANGE_TO_BROWN_5 : ORANGE_TO_BROWN_4;
        }
        e->bgColor = c;
        e->isDisabled = (_showingCpuTeams || _viewingOpponentTeam) ? 1 : 0;
    }
}

/* ================================================================ */
__attribute__((used, externally_visible))
void SetPlayerSurnamesOnSidePitch(void) {
    MenuEntry *entry = &_menuEntries[57];
    TeamFileEx *team = _playMatchTeam1Ptr;
    if (_viewingOpponentTeam) team = _playMatchTeam2Ptr;

    int8_t tIdx = (int8_t)team->team.tactics;
    TeamTactics *tac = (TeamTactics *)_teamTacticsTable[tIdx];
    int8_t fIdx = (int8_t)tac->ballOutOfPlayTactics;
    int16_t *pos = (int16_t *)_sidePitchSurnamePositionsTable[fIdx];

    uint16_t tNum = PACK_BE_U16(team->team.countryNumber, team->team.teamOrdinal);
    int16_t i;
    for (i = 0; i <= 10; i++, entry++) {
        entry->x = *pos++ + 196;
        entry->y = *pos++ + 40;
        entry->stringFlags = *pos++;
        uint8_t *pl = GetPlayerAtIndex(tNum, (uint16_t)i);
        PlayerFile *plPf = (PlayerFile *)(pl + 76);
        uint8_t *surname;
        if (entry->x == 0x106)
            surname = ExtractSurname((uint8_t *)plPf->playerName, 0x400F);
        else
            surname = ExtractSurname((uint8_t *)plPf->playerName, 0x400A);
        StringCopy(surname, (uint8_t *)entry->fg);
    }
}

/* ================================================================
 * UNIFORM (entry 68) -- OpenSWOS, SDL only: the kit each side wears for this match
 * (src/uniform_menu.c, docs/UNIFORM.md). It is there whenever this screen is about to
 * start a match somebody can still arrange: not while the other team is being looked
 * at, not before a training match, not when the screen only shows a result.
 *
 * The bottom row has no room for a fifth button, so UNIFORM shares EDIT TACTICS' place:
 * the two are stacked, each 10 px high, in the 23 the PLAY button beside them takes.
 * Where EDIT TACTICS is not shown (computer v computer) UNIFORM has the place alone.
 * Positions are taken from EXIT's (entry 8), which never moves, because this runs
 * again on every change of the screen without the menu being unpacked afresh.
 * ================================================================ */
enum { kUniformEntry = 68 };

#ifdef SWOS_SDL_BUILD
/* Set by a screen that shows this one for the line-up alone (the custom team editor,
 * whose kits are edited beside it): no UNIFORM button. */
int32_t g_playMatchNoUniform;
#endif

static void UpdateUniformButton(void) {
    MenuEntry *exitBtn = &_menuEntries[8];
    MenuEntry *tactics = &_menuEntries[10];
    MenuEntry *viewOpp = &_menuEntries[7];
    MenuEntry *uniform = &_menuEntries[kUniformEntry];
#ifdef SWOS_SDL_BUILD
    int16_t shown = _squadChangesAllowed && !_viewingOpponentTeam && !_trainingGame &&
                    !g_playMatchNoUniform;
#else
    int16_t shown = 0;          /* the screen behind it is a JSON menu, which the Amiga does not run */
#endif

    uniform->isInvisible = !shown;
    tactics->y = exitBtn->y;
    tactics->dwHeight = exitBtn->dwHeight;
    tactics->downEntry = -1;
    viewOpp->rightEntry = 10;
    if (!shown)
        return;
    uniform->x = tactics->x;
    uniform->dwWidth = tactics->dwWidth;
    uniform->leftEntry = 7;
    if (tactics->isInvisible) {
        uniform->y = exitBtn->y;
        uniform->dwHeight = exitBtn->dwHeight;
        uniform->upEntry = 55;                  /* the formation above, as EDIT TACTICS' */
        viewOpp->rightEntry = kUniformEntry;
        /* Two computer teams: the button beside this one is VIEW OTHER TEAM, 93 wide,
         * which reaches under it (ReportBot 0031). It gets the width and the words it
         * has when a player's team is shown. */
        if (viewOpp->x + viewOpp->dwWidth > uniform->x - 1) {
            viewOpp->dwWidth = uniform->x - 1 - viewOpp->x;
            if (viewOpp->fg == (uintptr_t)_aViewOtherTeam)
                viewOpp->fg = (uintptr_t)_aViewOppo;
        }
    } else {
        tactics->y = exitBtn->y - 2;
        tactics->dwHeight = 10;
        tactics->downEntry = kUniformEntry;
        uniform->y = exitBtn->y + 9;
        uniform->dwHeight = 10;
        uniform->upEntry = 10;
    }
}

/* UniformSelected -- the UNIFORM button. This screen is shown to each player in turn
 * with the teams the other way round, so "first team" is the one it shows only for the
 * first of them. */
__attribute__((used, externally_visible))
void UniformSelected(void) {
#ifdef SWOS_SDL_BUILD
    UniformMenu_Open(_playMatchTeam1Ptr, _playMatchTeam2Ptr, _isFirstPlayerChoosingTeam != 0);
#endif
}

/* ================================================================ */
__attribute__((used, externally_visible))
void SetVariableButtons(void) {
    SetGameStartButtonText();
    ShowUseMatchTeamIfTraining();

    MenuEntry *vb = &_menuEntries[7];
    vb->fg = (uintptr_t)_aViewOppo;
    if (_viewingOpponentTeam) vb->fg = (uintptr_t)_aViewTeam;
    if (_showingCpuTeams) { vb->fg = (uintptr_t)_aViewOtherTeam; vb->dwWidth = 93; }

    vb->stringFlags &= ~(1u << 13);

    if (_trainingGame) {
        vb->fg = (uintptr_t)_aViewATeam;
        if (!_is_A_team) {
            vb->fg = (uintptr_t)_aViewBTeam;
            uint16_t ns = GetNumberOfSubstitutes((uint8_t *)&_currentTeam, 1);
            if (ns != 1)
                vb->stringFlags |= (1u << 13);
        }
    }

    /* Team country name */
    {
        MenuEntry *top = &_menuEntries[2];
        uint8_t *nameOut = (uint8_t *)top->fg;
        if (!(_trainingGame && !_is_A_team)) {
            TeamFileEx *tm = _playMatchTeam1Ptr;
            if (_viewingOpponentTeam) tm = _playMatchTeam2Ptr;
            uint8_t *cname = (uint8_t *)_countriesTable[tm->team.countryNumber];
            cname++;
#ifdef SWOS_SDL_BUILD
            /* A custom team (src/custom_team.c) is a database team under another name,
             * so this would name the country of the team it was made from -- BORUSSIA
             * FULDA, made from BRAZIL, "(SOUTH AMERICA)". It says what the team is. */
            static uint8_t custom[] = "CUSTOM";
            if (CustomTeam_IsCustom(tm))
                cname = custom;
#endif
            *nameOut++ = '(';
            nameOut = StringCopy(cname, nameOut);
            *nameOut++ = ')';
        }
        *nameOut = 0;
    }

    UpdateCheckMarkSprite();

    TeamFileEx *curTeam = _playMatchTeam1Ptr;

    if (_viewingOpponentTeam) {
        /* Viewing opponent */
        _menuEntries[4].isInvisible = 1;   /* v324: coach name hidden (user, 26/11) */
        _menuEntries[3].isInvisible = 0;
        _menuEntries[3].y = 204;
        _menuEntries[4].y = 204;
        _menuEntries[6].isInvisible = 1;
        _menuEntries[1].bgColor = (_menuEntries[1].bgColor & 0xFFF0) | ORANGE_TO_BROWN_12;

        curTeam = _playMatchTeam2Ptr;
        _menuEntries[10].isInvisible = 1;

        TeamFileEx *t2 = _playMatchTeam2Ptr;
        if (t2->team.teamControls != TEAM_COMPUTER || !_squadChangesAllowed)
            _menuEntries[7].isInvisible = 1;

        _menuEntries[11].isInvisible = 0;
        _menuEntries[29].isInvisible = 1;
        _menuEntries[30].isInvisible = 1;
    } else {
        /* Viewing own team */
        _menuEntries[4].isInvisible = 1;   /* v324: coach name hidden (user, 26/11) */
        _menuEntries[3].isInvisible = 1;
        _menuEntries[3].y = 204;
        _menuEntries[4].y = 204;
        _menuEntries[6].isInvisible = 0;

        MenuEntry *cb = &_menuEntries[6];
        TeamFileEx *t1 = _playMatchTeam1Ptr;

        if (_showingCpuTeams) {
            _menuEntries[6].isInvisible = 1;
            _menuEntries[3].isInvisible = 0;
        }

        cb->fg = (uintptr_t)_aCoach_1;
        if (t1->team.teamControls == TEAM_PLAYER_COACH)
            cb->fg = (uintptr_t)_aPlayer_1;

        cb->bgColor = (cb->bgColor & 0xFFF0) | BRIGHT_BLUE_TO_BLUE_13;
        if (t1->team.teamControls == TEAM_PLAYER_COACH)
            cb->bgColor = (cb->bgColor & 0xFFF0) | BLUE_TO_PURPLE_11;

        if (!_squadChangesAllowed) {
            _menuEntries[3].isInvisible = 0;
            _menuEntries[6].isInvisible = 1;
        }

        cb->isDisabled = 0;
        if (_is_A_team) _menuEntries[3].isInvisible = 1;

        if (_trainingGame) {
            if (t1->team.teamControls != TEAM_PLAYER_COACH) {
                TeamFileEx *t2 = _playMatchTeam2Ptr;
                if (t2->team.teamControls == TEAM_PLAYER_COACH) {
                    if (t1->team.teamControls != TEAM_PLAYER_COACH)
                        cb->isDisabled = 1;
                }
            }
        }

        /* Edit tactics button */
        MenuEntry *eb = &_menuEntries[1];
        eb->bgColor = (eb->bgColor & 0xFFF0) | BLUE_TO_PURPLE_11;
        if (t1->team.teamControls != TEAM_PLAYER_COACH) {
            eb->bgColor = (eb->bgColor & 0xFFF0) | BRIGHT_BLUE_TO_BLUE_13;
            if (t1->team.teamControls != TEAM_COACH)
                eb->bgColor = (eb->bgColor & 0xFFF0) | ORANGE_TO_BROWN_12;
        }
        if (_trainingGame && t1->team.teamControls == TEAM_COMPUTER)
            eb->bgColor = (eb->bgColor & 0xFFF0) | BRIGHT_BLUE_TO_BLUE_13;

        _menuEntries[10].isInvisible = 0;
        if (_showingCpuTeams) _menuEntries[10].isInvisible = 1;
        if (_is_A_team) _menuEntries[10].isInvisible = 1;

        _menuEntries[7].isInvisible = 0;
        _menuEntries[11].isInvisible = 1;
        _menuEntries[29].isInvisible = 1;
        _menuEntries[30].isInvisible = 1;

        if (!_showingCpuTeams) {
            int16_t diff = (int16_t)_numPlayersToShowInMenu - (int16_t)_pmMenuNumPlayers;
            if (diff != 0) {
                _menuEntries[29].isInvisible = 0;
                if (diff != 1) _menuEntries[30].isInvisible = 0;
            }
        }
#ifdef SWOS_SDL_BUILD
        /* v342 (user 26/11/2026): with the right-hand column the substitutes are ONE list (left rows,
         * then the right column below them) and every excluded player sits at the bottom of the right
         * column. The original "first excluded" row 16 and its rotate arrow would only repeat the first
         * player of the right column: hidden while that column is shown. */
        if (Expanded_CareerExtraVisible()) {
            _menuEntries[29].isInvisible = 1;
            _menuEntries[30].isInvisible = 1;
        }
#endif
    }

    /* Set team name and coach name */
    {
        MenuEntry *nm = &_menuEntries[1];
        TeamFileEx *shownTeam = _viewingOpponentTeam ?
            _playMatchTeam2Ptr : _playMatchTeam1Ptr;
        nm->fg = (uintptr_t)shownTeam->team.teamName;
        /* In TRAINING, _is_A_team controls the VIEW button direction:
         * it starts at zero while the actual visible team is A. It
         * must NOT choose the name, or both screens can say "B". */
        if (_trainingGame) {
            if (shownTeam == &_careerTeam)
                nm->fg = (uintptr_t)_teamAName;
            else if (shownTeam == &_currentTeam)
                nm->fg = (uintptr_t)_teamBName;
        }
    }

    {
        TeamFileEx *tm = _playMatchTeam1Ptr;
        if (_viewingOpponentTeam) tm = _playMatchTeam2Ptr;
        _menuEntries[4].fg = (uintptr_t)tm->team.coachName;
    }

    UpdateUniformButton();
#ifdef SWOS_SDL_BUILD
    Expanded_CareerExtraUpdate();
#endif
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchMenuReinit(void) {
#ifdef SWOS_SDL_BUILD
    /* Native reserve x must be final BEFORE the checkmark reads row x.
     * Later SetVariableButtons repeats this idempotently for nav/pitch. */
    Expanded_CareerExtraUpdate();
#endif
    UpdateTeamCheckmark();
    _pmMenuNumPlayers = 16 - 5 + _maxSubstitutesEntries;

    uint16_t np = _numPlayers;
    if (_playMatchTeam1Ptr != &_currentTeam) {
        if (_selTeamNumber == 0xFFFF) {
            np = 16;
        } else {
            if (PACK_BE_U16(_playMatchTeam1Ptr->team.countryNumber, _playMatchTeam1Ptr->team.teamOrdinal) == _selTeamNumber)
                np = _careerNumPlayers;
            else
                np = 16;
        }
    }
    _numPlayersToShowInMenu = np;
    _pmSelectedPlayerEntry = 0;
    _selectedPlayerPosition = -1;

    SetVariableButtons();
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowOrHidePlayMatchButton();

    int16_t extra = 4 - (int16_t)_maxSubstitutesEntries;
    if (extra >= 0) {
        MenuEntry *sub = &_menuEntries[28];
        int16_t j;
        for (j = extra; j >= 0; j--) {
            sub->isInvisible = 1;
            sub--;
            _menuEntries[12].y -= 9;
            _menuEntries[30].y -= 9;
            _menuEntries[29].y -= 9;
        }
    }

    SavePositionsAndUpdateLittleCheckmark();
    UpdateLittleCheckmark();
}


#ifdef SWOS_SDL_BUILD
/* CAREER-only OG matchday squad repair: never changes injury, player DB,
 * fixture/result, tactics or the original competition's max substitutions.
 * With 16 players and 2 unavailable, 11 + 3 legal players can be nominated
 * even if the pre-match selection initially contains 11 + 5 slots.
 * The existing OG swap callback is the only way we move player positions. */
static void Expanded_RepairCareerMatchdaySquad(void)
{
    TeamFileEx *team=_playMatchTeam1Ptr;
    if(_gameType!=GAME_TYPE_CAREER || !team || !_squadChangesAllowed ||
       _trainingGame || team->team.teamControls==TEAM_COMPUTER)return;
    const uint16_t teamNum=PACK_BE_U16(team->team.countryNumber,
                                      team->team.teamOrdinal);
    /* The Copa menu can use a selected-team COPY of our same OG club. */
    if(_selTeamNumber==0xFFFF || teamNum!=_selTeamNumber)return;

    const unsigned n=_careerNumPlayers;
    const unsigned originalBench=_maxSubstitutesEntries;
    unsigned eligible=0,healthyGk=0,healthyOutfield=0,moves=0;
    uint32_t seen=0u;
#ifndef NDEBUG
    const char *capture=getenv("SWOS_DIAGNOSTIC_CAPTURE");
    const int trace=capture && capture[0]=='1';
#else
    const int trace=0;
#endif
    if(n<11u || n>32u || originalBench>8u) {
        if(trace)fprintf(stderr,"[career-lineup] UNSUPPORTED roster=%u bench=%u\n",
                         n,originalBench);
        return;
    }
    for(unsigned i=0;i<n;++i) {
        const unsigned id=_currentMatchPlayers[i];
        if(id>=32u || (seen&(1u<<id))) {
            if(trace)fprintf(stderr,
                "[career-lineup] INVALID_SLOT ordinal=%u index=%u\n",i,id);
            return;
        }
        seen|=1u<<id;
        const PlayerFile *pf=(const PlayerFile *)(GetPlayer(teamNum,(uint16_t)id)+76);
        if(Career_IsSelectionUnavailable(pf,_playMatchExcludeFlags))
            continue;
        ++eligible;
        const unsigned pos=((unsigned)pf->positionAndFace&PF_POSITION_MASK)
                            >>PF_POSITION_SHIFT;
        if(pos==0u)++healthyGk;
        else ++healthyOutfield;
    }
    if(eligible<11u || !healthyGk || healthyOutfield<10u) {
        if(trace)fprintf(stderr,
            "[career-lineup] NOT_REPAIRED roster=%u available=%u gk=%u outfield=%u reason=INSUFFICIENT_XI\n",
            n,eligible,healthyGk,healthyOutfield);
        return;
    }

    /* Preserve legal starters. Only a missing starter is replaced;
     * first look for the same position, then for another eligible player.
     * Goalkeeper slot zero must still contain a real eligible goalkeeper. */
    for(unsigned slot=0;slot<11u;++slot) {
        const unsigned id=_currentMatchPlayers[slot];
        const PlayerFile *pf=(const PlayerFile *)(GetPlayer(teamNum,(uint16_t)id)+76);
        const unsigned starterPos=((unsigned)pf->positionAndFace&PF_POSITION_MASK)
                                  >>PF_POSITION_SHIFT;
        /* A legal XI contains one GK at slot 0 and no GK in outfield. */
        if(!Career_IsSelectionUnavailable(pf,_playMatchExcludeFlags) &&
           ((slot==0u && starterPos==0u) ||
            (slot!=0u && starterPos!=0u)))
            continue;
        const unsigned wanted=((unsigned)pf->positionAndFace&PF_POSITION_MASK)
                               >>PF_POSITION_SHIFT;
        int chosen=-1;
        for(int pass=0;pass<2 && chosen<0;++pass) {
            for(unsigned j=11u;j<n;++j) {
                const PlayerFile *c=(const PlayerFile *)(GetPlayer(teamNum,(uint16_t)_currentMatchPlayers[j])+76);
                if(Career_IsSelectionUnavailable(c,_playMatchExcludeFlags))
                    continue;
                const unsigned pos=((unsigned)c->positionAndFace&PF_POSITION_MASK)
                                    >>PF_POSITION_SHIFT;
                if(slot==0u ? pos!=0u :
                   (pos==0u || (!pass && pos!=wanted)))continue;
                chosen=(int)j;
                break;
            }
        }
        if(chosen<0) {
            if(trace)fprintf(stderr,
               "[career-lineup] PARTIAL_REPAIR starter=%u reason=NO_ELIGIBLE_REPLACEMENT\n",
               slot);
            return;
        }
        PlayMatchMenuSwapPlayers(team,(int16_t)slot,(int16_t)chosen);
        ++moves;
    }

    /* The number actually NOMINATED on the bench can be lower than the
     * competition's maximum; never put injured players on that bench.
     * Remaining players stay selectable in the OG rotating extra row. */
    unsigned bench=eligible-11u;
    if(bench>originalBench)bench=originalBench;
    const unsigned selected=11u+bench;
    for(unsigned i=11u;i<selected;++i) {
        const unsigned id=_currentMatchPlayers[i];
        if(!Career_IsSelectionUnavailable((const PlayerFile *)(GetPlayer(teamNum,(uint16_t)id)+76),
                                           _playMatchExcludeFlags))continue;
        int chosen=-1;
        for(unsigned j=selected;j<n;++j) {
            const unsigned test=_currentMatchPlayers[j];
            if(!Career_IsSelectionUnavailable((const PlayerFile *)(GetPlayer(teamNum,(uint16_t)test)+76),
                                               _playMatchExcludeFlags)) {
                chosen=(int)j;break;
            }
        }
        if(chosen<0) {
            if(trace)fprintf(stderr,
               "[career-lineup] PARTIAL_REPAIR bench=%u reason=NO_ELIGIBLE_REPLACEMENT\n",i);
            return;
        }
        PlayMatchMenuSwapPlayers(team,(int16_t)i,(int16_t)chosen);
        ++moves;
    }
    _maxSubstitutesEntries=(uint16_t)bench;
    if(moves) {
        team->team.playerValuesDirty=TEAM_VALUES_DIRTY;
        SetBenchPlayersNumbers(team);
    }
    if(trace)fprintf(stderr,
       "[career-lineup] roster=%u available=%u unavailable=%u old-bench=%u active-bench=%u swaps=%u extra=%u status=REPAIRED\n",
       n,eligible,n-eligible,originalBench,bench,moves,n-selected);
}
#endif

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchMenuInit(void) {
#ifdef SWOS_SDL_BUILD
    s_trainingCrossSide=0; /* no cross-swap pending in a fresh menu */
    s_extraNavReady=0;
    s_extraLastVisible=-1;
    s_extraReserveSlot=-1;
    s_extraDrawSlot=-1;
    for(unsigned i=0;i<17u;++i)
        s_extraNativeRight[i]=_menuEntries[13u+i].rightEntry;
    for(unsigned i=0;i<12u;++i)
        s_extraNativePitchInvisible[i]=_menuEntries[56u+i].isInvisible;
    for(unsigned i=0;i<kCareerExtraCount;++i) {
        s_extraNativeDisabled[i]=_menuEntries[kCareerExtraFirst+i].isDisabled;
        s_extraNativeX[i]=_menuEntries[kCareerExtraFirst+i].x;
    }
    s_extraNativeCheckX=_menuEntries[37].x;
    s_extraNavReady=1;
#endif
    if (!_allowShorterMenuItemsWithFrames)
        _fireResetFlag = 1;

    uint32_t d0 = _irqCookieObfuscated ^ 0xCFD1F85F;
    if (d0 != 0x17B8) return;

    _player1ClearFlag = 0;
    _player2ClearFlag = 0;
    if (!(_playMatchControlFlags & 0x01)) _player1ClearFlag = 0xFF;
    if (!(_playMatchControlFlags & 0x02)) _player2ClearFlag = 0xFF;

    _showingCpuTeams = 0;
    {
        if (_playMatchTeam1Ptr->team.teamControls == TEAM_COMPUTER) {
            if (_playMatchTeam2Ptr->team.teamControls == TEAM_COMPUTER)
                _showingCpuTeams = 1;
        }
    }

    _viewingOpponentTeam = 0;
    SetCurrentSelectedPlayerIndexPointer();
    CheckIsTeamNational();
    SetBenchPlayersNumbers((TeamFileEx *)_playMatchTeam1Ptr);
    _is_A_team = 0;
#ifdef SWOS_SDL_BUILD
    Expanded_RepairCareerMatchdaySquad();
#endif
    PlayMatchMenuReinit();

    /* If entry 9 (PLAY/VIEW RESULT button) is invisible, default to entry 7 (VIEW OPPO).
     * Original asm: lea ($2A26).w,a0; tst.w 4(a0) — $2A26 = menuEntries[9] in original binary,
     * offset 4 = isInvisible (coincidentally same as TEAMFILE_TEAMCONTROLS EQU). */
    if (_menuEntries[9].isInvisible != 0)
        SetCurrentEntry(7);
}

/* ================================================================ */
__attribute__((used, externally_visible))
void OnSwapPlayer(MenuEntry *a5) {
    TeamFileEx *t1 = _playMatchTeam1Ptr;
#ifdef SWOS_SDL_BUILD
    /* The original manager's still-pending OG fixture may change their
     * lineup; an actual CPU/viewer/result after witness cannot. */
    if (_gameType == GAME_TYPE_CAREER && !Expanded_CareerLineupEditsAllowed())
        return;
#endif

    if (_viewingOpponentTeam) {
        t1 = _playMatchTeam2Ptr;
        goto cpuPath;
    }
    if (_showingCpuTeams) goto cpuPath;

    /* Normal player path */
    {
        int16_t d6 = a5->ordinal - _menuEntries[13].ordinal;
#ifdef SWOS_SDL_BUILD
        if(s_extraReserveSlot>=0) d6=s_extraReserveSlot;
        else
#endif
        if (d6 == 16) d6 = d6 - 5 + _maxSubstitutesEntries;

        uint16_t slot, fIdx;
        GetPlayerInfoBreakPosition((TeamFileEx *)_playMatchTeam1Ptr, (uint16_t)d6, &slot, &fIdx);
        d6 = (int16_t)slot;
        int16_t d7 = (int16_t)fIdx;

        if (_selectedPlayerPosition >= 0) {
            if (d7 == _selectedPlayerPosition) {
                d7 = -1;
                if (!HandleSquadSwapSelectionToggle(d6)) return;
            } else {
                TeamFileEx *team = _playMatchTeam1Ptr;
                PlayMatchMenuSwapPlayers(team, _selectedPlayerTacticsPosition, d6);
                SetBenchPlayersNumbers((TeamFileEx *)team);
                team->team.playerValuesDirty = TEAM_VALUES_DIRTY;
                SavePositionsAndUpdateLittleCheckmark();
                _selectedPlayerPosition = -1;
                SetPlayerSurnamesOnSidePitch();
                ShowOrHidePlayMatchButton();
                UpdateTeamCheckmark();
                ShowUseMatchTeamIfTraining();
                DrawMenu();
                return;
            }
        } else {
            if (_longFireFlag) return;
        }

        _selectedPlayerPosition = d7;
        _selectedPlayerTacticsPosition = d6;
        {
            MenuEntry *old = (MenuEntry *)_pmSelectedPlayerEntry;
            _pmSelectedPlayerEntry = (uintptr_t)a5;
            if (old) { CallDrawMenuItem(old); }
        }
        CallDrawMenuItem(a5);
        UpdateLittleCheckmark();
        RedrawPlayerNames();
        return;
    }

cpuPath:
    if (_longFireFlag) return;
    if (!_squadChangesAllowed) return;
    {
        int16_t d1 = a5->ordinal - _menuEntries[13].ordinal;
        uint16_t slot, fIdx;
        uint8_t *pl = GetPlayerInfoBreakPosition((TeamFileEx *)t1, (uint16_t)d1, &slot, &fIdx);
        PlayerFile *plPf = (PlayerFile *)(pl + 76);
        if (((plPf->positionAndFace & PF_POSITION_MASK) >> 5) == 0) return;

        uint16_t *ip = (uint16_t *)_currentSelectedPlayerIndexPtr;
        if (*ip == fIdx)
            *ip = 0xFFFF;
        else
            *ip = fIdx;
        UpdateTeamCheckmark();
        DrawMenu();
    }
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchMenuSwapPlayers(TeamFileEx *team, int16_t d3, int16_t d4) {
    /* Original asm only takes D3 (selected slot) and D4 (dest slot).
     * Primary swap runs when the two slots are in different groups
     * (one starting, one bench) OR either slot >= 16 (extended squad). */
    int skipPrimary = 0;
    if ((uint16_t)d3 < 16 && (uint16_t)d4 < 16) {
        /* Both < 16: skip primary if same group */
        if ((uint16_t)d3 < 11 && (uint16_t)d4 < 11)
            skipPrimary = 1;  /* both starting 11 */
        else if ((uint16_t)d3 >= 11 && (uint16_t)d4 >= 11)
            skipPrimary = 1;  /* both bench */
    }

    if (!skipPrimary) {
        /* Primary swap: playerPositionsArray + shirt numbers.
         * Asm calls GetPlayerAtIndex(teamNum, d3/d4) which translates
         * through _playerPositions/currentMatchPlayers/playerNumbers,
         * then uses the translated file indices to index playerPositionsArray. */
        uint16_t tNum = PACK_BE_U16(team->team.countryNumber, team->team.teamOrdinal);
        uint8_t *p1 = GetPlayerAtIndex(tNum, (uint16_t)d3);
        uint16_t fileIdx1 = _cGetPlayerAtIndexFileIdx;
        uint8_t *p2 = GetPlayerAtIndex(tNum, (uint16_t)d4);
        uint16_t fileIdx2 = _cGetPlayerAtIndexFileIdx;

        uint8_t tmp = _playerPositionsArray[fileIdx2];
        _playerPositionsArray[fileIdx2] = _playerPositionsArray[fileIdx1];
        _playerPositionsArray[fileIdx1] = tmp;

        PlayerFile *pf1 = (PlayerFile *)(p1 + 76);
        PlayerFile *pf2 = (PlayerFile *)(p2 + 76);
        int8_t ts = pf2->shirtNumber;
        if (!SquadNumbers_On()) {       /* (real squad numbers: not swapped, teams.c) */
            pf2->shirtNumber = pf1->shirtNumber;
            pf1->shirtNumber = ts;
        }
    }

    /* Secondary swap: swap in _playerPositions/currentMatchPlayers/playerNumbers
     * using d3/d4 (slot positions) */
    {
        uint8_t *pn;
        uint16_t tNum = PACK_BE_U16(team->team.countryNumber, team->team.teamOrdinal);
        if (team == &_currentTeam) {
            pn = (uint8_t *)_playerPositions;
        } else if (_gameType == GAME_TYPE_CAREER && (team == &_careerTeam || tNum == _selTeamNumber)) {
            /* The player's matchday lineup lives in currentMatchPlayers. In a CUP,
             * playMatchTeam1Ptr is a selTeams COPY (not &careerTeam) whose team
             * number still equals selTeamNumber. The asm's pointer-only test
             * (`==&careerTeam`) only works because the Amiga's playMatchTeam1Ptr IS
             * &careerTeam; on SDL the cup uses a copy, so that test fails and the
             * secondary swap edits team->playerNumbers instead — the lineup never
             * changes and a carded substitution silently fails. Key on
             * selTeamNumber to match the PRIMARY swap's GetPlayerAtIndex
             * (teams.c:924), so both halves of the swap agree. */
            pn = (uint8_t *)_currentMatchPlayers;
        } else {
            pn = team->team.playerNumbers;
        }
        uint8_t t = pn[(uint16_t)d4];
        pn[(uint16_t)d4] = pn[(uint16_t)d3];
        pn[(uint16_t)d3] = t;
    }

    if (_trainingGame && !_is_A_team
#ifdef SWOS_SDL_BUILD
        && !s_trainingCrossSwapRunning
#endif
    ) {
        uint16_t lim = 11 + _maxSubstitutesEntries;
        if ((d3 < (int16_t)lim && d4 >= (int16_t)lim) ||
            (d3 >= (int16_t)lim && d4 < (int16_t)lim))
            TrainingMatch_BuildCurrentTeamFromCareerTeam();
    }
}

/* ================================================================ */
__attribute__((used, externally_visible))
int16_t HandleSquadSwapSelectionToggle(int16_t d6) {
    uint16_t tNum = _currentTeamNumber;
    uint16_t *selPos = &_currentTeamSelectedPlayerPosition;

    if (_playMatchTeam1Ptr == &_currentTeam) {
        /* defaults already set */
    } else if (_playMatchTeam1Ptr == &_careerTeam
#ifdef SWOS_SDL_BUILD
               || Expanded_ManagedCareerTeam(_playMatchTeam1Ptr)
#endif
              ) {
        tNum = _selTeamNumber;
        selPos = &_selTeamSelectedPlayerPosition;
    } else {
        tNum = PACK_BE_U16(_playMatchTeam1Ptr->team.countryNumber, _playMatchTeam1Ptr->team.teamOrdinal);
        selPos = (uint16_t *)_currentSelectedPlayerIndexPtr;
    }

    if (!_longFireFlag) return 1;
    if (_longFireFlag != 1) return 0;
#ifdef SWOS_SDL_BUILD
    if (!Expanded_CareerLineupEditsAllowed()) return 0;
#else
    if (!_squadChangesAllowed) return 0;
#endif
    if ((uint16_t)d6 >= _pmMenuNumPlayers) {
#ifdef SWOS_SDL_BUILD
        if(!(s_extraReserveSlot>=0 && Expanded_CareerExtraVisible() &&
             (uint16_t)d6<_careerNumPlayers))return 0;
#else
        return 0;
#endif
    }

    int16_t curSel = (int16_t)_selectedPlayerPosition;
    if (curSel == (int16_t)*selPos) {
        curSel = -1;
    } else {
        /* Asm calls GetPlayer (not GetPlayerAtIndex) — selectedPlayerPosition
         * is already a file index, no translation needed. */
        uint8_t *pl = GetPlayer(tNum, (uint16_t)_selectedPlayerPosition);
        PlayerFile *plPf = (PlayerFile *)(pl + 76);
        if (((plPf->positionAndFace & PF_POSITION_MASK) >> 5) == 0) return 0;
    }

    *selPos = (uint16_t)curSel;
    _selectedPlayerPosition = -1;
    UpdateTeamCheckmark();
    ShowUseMatchTeamIfTraining();
    DrawMenu();
    return 0;
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchChangeTactics(MenuEntry *a5) {
    UpdateLittleCheckmark();
    RedrawPlayerNames();
    int16_t nt = a5->ordinal - _menuEntries[38].ordinal;
    TeamFileEx *t = _playMatchTeam1Ptr;
    if ((int8_t)t->team.tactics == (int8_t)nt) return;
    t->team.tactics = (uint8_t)nt;
    t->team.playerValuesDirty = TEAM_VALUES_DIRTY;
    SavePositionsAndUpdateLittleCheckmark();
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowUseMatchTeamIfTraining();
    UpdateTeamCheckmark();
    DrawMenu();
}

/* ================================================================ */
__attribute__((used, externally_visible))
void ExitPlayMatch(void) {
    _exitGameFlag = 1;
    _player1ClearFlag = 0;
    _player2ClearFlag = 0;
    _isCareer = 0;
    if (_playMatchTeam1Ptr == &_careerTeam ||
        _playMatchTeam2Ptr == &_careerTeam)
        _isCareer = 1;
    SetExitMenuFlag();
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchSelected(void) {
    if (_careerTeam.team.teamControls == TEAM_COMPUTER)
        _careerTeamCoachOnly = 1;
    if (_pl1Fire) {
        _playerNumThatStarted = 1;
        _player1ClearFlag = 0; _player2ClearFlag = 0;
        SetExitMenuFlag();
    } else if (_pl2Fire) {
        _playerNumThatStarted = 2;
        _player1ClearFlag = 0; _player2ClearFlag = 0;
        SetExitMenuFlag();
    }
}

/* ================================================================ */
__attribute__((used, externally_visible))
void EditTacticsInPlayMatchMenu(void) {
    UpdateLittleCheckmark();
    ShowStandardChooseTacticsMenu((TeamFileEx *)_playMatchTeam1Ptr);
}

/* ================================================================ */
__attribute__((used, externally_visible))
void ViewOpponentsSelect(void) {
    if (_viewingOpponentTeam) {
        _viewingOpponentTeam = 0;
        goto common;
    }
    if (_trainingGame) {
        _is_A_team ^= 1;
        TeamFileEx *tmp = _playMatchTeam1Ptr;
        _playMatchTeam1Ptr = _playMatchTeam2Ptr;
        _playMatchTeam2Ptr = tmp;
        _numPlayersToShowInMenu = _careerNumPlayers;
        if (_playMatchTeam1Ptr != &_careerTeam)
            _numPlayersToShowInMenu = _numPlayers;
        _selectedPlayerPosition = -1;
        SavePositionsAndUpdateLittleCheckmark();
        UpdateLittleCheckmark();
        SetVariableButtons();
        SetUpTacticsButtons();
        SetPlayerSurnamesOnSidePitch();
        ShowOrHidePlayMatchButton();
        UpdateTeamCheckmark();
        DrawMenu();
        return;
    }
    _viewingOpponentTeam = 1;
    {
        if (_playMatchTeam2Ptr->team.teamControls != TEAM_COMPUTER || !_squadChangesAllowed)
            SetCurrentEntry(11);
    }
common:
    SetCurrentSelectedPlayerIndexPointer();
    UpdateLittleCheckmark();
    SetVariableButtons();
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowOrHidePlayMatchButton();
    UpdateTeamCheckmark();
    DrawMenu();
}

#ifdef SWOS_SDL_BUILD
/* TRAINING Team B is regenerated from A's OG career slots >=13. Cross-team
 * X therefore exchanges canonical career lineup slots, never PlayerFile
 * copies (which would duplicate identities and vanish on B rebuild). */
void Expanded_TrainingCrossSwapX(void)
{
    if(!Expanded_TrainingCrossSwapAvailable()) {
        s_trainingCrossSide=0;
        return;
    }
    const Menu *menu=(const Menu *)_currentMenu;
    const MenuEntry *entry=menu->selectedEntry;
    if(!entry || entry->isInvisible || entry->isDisabled ||
       entry->ordinal<13u || entry->ordinal>29u) {
        fprintf(stderr,"[training-cross] IGNORE reason=NOT_PLAYER_ROW\n");
        return;
    }
    unsigned row=(unsigned)entry->ordinal-13u;
    if(row==16u) row=11u+(unsigned)_maxSubstitutesEntries;
    const unsigned side=_playMatchTeam1Ptr==&_careerTeam ? 1u : 2u;
    const unsigned visible=side==1u ? _careerNumPlayers : _numPlayers;
    if(row>=visible) {
        fprintf(stderr,"[training-cross] IGNORE reason=ROW_OUT_OF_RANGE row=%u visible=%u\n",
                row,visible);
        return;
    }
    uint16_t slot=0xffffu,fileIdx=0xffffu;
    GetPlayerInfoBreakPosition(_playMatchTeam1Ptr,(uint16_t)row,&slot,&fileIdx);
    const uint16_t careerSlot=side==1u ? slot :
        TrainingMatch_BSourceCareerSlot(fileIdx);
    if(careerSlot>=_careerNumPlayers || careerSlot>=32u) {
        fprintf(stderr,"[training-cross] IGNORE reason=UNKNOWN_CAREER_SLOT "
                       "side=%c row=%u file=%u slot=%u\n",
                side==1u?'A':'B',row,(unsigned)fileIdx,(unsigned)careerSlot);
        return;
    }
    if(!s_trainingCrossSide || s_trainingCrossSide==side) {
        s_trainingCrossSide=(uint16_t)side;
        s_trainingCrossSlot=careerSlot;
        fprintf(stderr,"[training-cross] SOURCE side=%c slot=%u "
                       "next=%c status=WAITING_TARGET\n",
                side==1u?'A':'B',(unsigned)careerSlot,side==1u?'B':'A');
        SetCurrentEntry(13);
        ViewOpponentsSelect();
        return;
    }
    const uint16_t aSlot=side==1u ? careerSlot : s_trainingCrossSlot;
    const uint16_t bSlot=side==2u ? careerSlot : s_trainingCrossSlot;
    s_trainingCrossSide=0;
    if(aSlot==bSlot) {
        fprintf(stderr,"[training-cross] CANCEL reason=SAME_PLAYER slot=%u\n",
                (unsigned)aSlot);
        return;
    }
    /* Prove that the two canonical OG player identities actually exchange.
     * Never report SWAPPED solely because the X callback was dispatched. */
    if (aSlot>=_careerNumPlayers || bSlot>=_careerNumPlayers ||
        aSlot>=32u || bSlot>=32u) {
        fprintf(stderr,"[training-cross] REJECT reason=INVALID_SWAP_SLOTS a=%u b=%u\n",
                (unsigned)aSlot,(unsigned)bSlot);
        return;
    }
    const uint8_t beforeA=_currentMatchPlayers[aSlot];
    const uint8_t beforeB=_currentMatchPlayers[bSlot];
    if (beforeA>=32u || beforeB>=32u || beforeA==beforeB) {
        fprintf(stderr,"[training-cross] REJECT reason=INVALID_PLAYER_IDS a=%u b=%u\n",
                (unsigned)beforeA,(unsigned)beforeB);
        return;
    }
    /* Same OG callback as a normal swap, minus its auto-B re-sort. */
    s_trainingCrossSwapRunning=1;
    PlayMatchMenuSwapPlayers(&_careerTeam,(int16_t)aSlot,(int16_t)bSlot);
    s_trainingCrossSwapRunning=0;
    const uint8_t afterA=_currentMatchPlayers[aSlot];
    const uint8_t afterB=_currentMatchPlayers[bSlot];
    if (afterA!=beforeB || afterB!=beforeA) {
        fprintf(stderr,"[training-cross] FAILED_INVARIANT a=%u->%u expected=%u "
                       "b=%u->%u expected=%u\n",
                (unsigned)beforeA,(unsigned)afterA,(unsigned)beforeB,
                (unsigned)beforeB,(unsigned)afterB,(unsigned)beforeA);
        return;
    }
    const unsigned bCopies=TrainingMatch_SyncBForCareerSwap(aSlot,bSlot);
    const unsigned kept=TrainingMatch_KeepCrossSwapForMatch(
        beforeA,beforeB,!SquadNumbers_On());
    _careerTeam.team.playerValuesDirty=TEAM_VALUES_DIRTY;
    _currentTeam.team.playerValuesDirty=TEAM_VALUES_DIRTY;
    SetBenchPlayersNumbers(&_careerTeam);
    _selectedPlayerPosition=-1;
    _pmSelectedPlayerEntry=0;
    /* PlayMatchMenuReinit decrements the Y coordinates of entries 12,29,30
     * on EVERY call. It must run only at menu setup, never on repeated X. */
    SavePositionsAndUpdateLittleCheckmark();
    SetVariableButtons();
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowOrHidePlayMatchButton();
    UpdateTeamCheckmark();
    SetCurrentEntry(13);
    DrawMenu();
    fprintf(stderr,"[training-cross] VERIFIED A-slot=%u file=%u->%u "
                   "B-source-slot=%u file=%u->%u B-copies=%u "
                   "official-match=%s B-order=RETAINED status=%s\n",
            (unsigned)aSlot,(unsigned)beforeA,(unsigned)afterA,
            (unsigned)bSlot,(unsigned)beforeB,(unsigned)afterB,
            bCopies,kept?"UPDATED":"NOT_IN_SAVED_LINEUP",
            bCopies?"OK":"B_COPY_NOT_FOUND");
}
#endif

/* ================================================================ */
__attribute__((used, externally_visible))
void ReturnFromViewOpponent(void) {
    _viewingOpponentTeam = 0;
    SetCurrentSelectedPlayerIndexPointer();
    SetVariableButtons();
    SetCurrentEntry(7);
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowOrHidePlayMatchButton();
    UpdateTeamCheckmark();
    DrawMenu();
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatchOnSelect30(void) {
    RotatePlayersLeft((TeamFileEx *)_playMatchTeam1Ptr, _maxSubstitutesEntries);
    /* Refresh eligibility when the extra row is rotated. */
    ShowOrHidePlayMatchButton();
    DrawMenuItemsWaitForVSync(_playMatchRedrawList);
}

/* ================================================================ */
__attribute__((used, externally_visible))
void PlayMatch30AfterDraw(void) {
    CallDrawMenuItem(&_menuEntries[29]);
}

/* ================================================================ */
__attribute__((used, externally_visible))
void RedrawPlayerNames(void) {
    CallDrawMultipleItems(11, &_menuEntries[13]);
    CallDrawMenuItem(&_menuEntries[0]);
}

/* ================================================================ */
__attribute__((used, externally_visible))
void UseMatchTeamSelect(void) {
    TrainingMatch_RestoreLastMatchState();
    uint8_t saved = _currentTeam.team.tactics;
    TrainingMatch_ApplyUseMatchTeamAndSwapSubs();
    _currentTeam.team.tactics = saved;
    TrainingMatch_RestoreSavedTeamSelectionState();
    SetBenchPlayersNumbers(&_careerTeam);
    TrainingMatch_BuildCurrentTeamFromCareerTeam();
    _selectedPlayerPosition = -1;
    SavePositionsAndUpdateLittleCheckmark();
    UpdateLittleCheckmark();
    SetVariableButtons();
    SetUpTacticsButtons();
    SetPlayerSurnamesOnSidePitch();
    ShowOrHidePlayMatchButton();
    UpdateTeamCheckmark();
    SetCurrentEntry(0x1D);
    DrawMenu();
}

/* ================================================================ */
__attribute__((used, externally_visible))
void TogglePlayerCoach(void) {
    if (_gameType == GAME_TYPE_CAREER) {
        if (_is_A_team) {
            uint8_t nc = (_currentTeam.team.teamControls == TEAM_PLAYER_COACH)
                         ? TEAM_COACH : TEAM_PLAYER_COACH;
            _currentTeam.team.teamControls = nc;
            _careerTeam.team.teamControls = 1;
            _coachOrPlayer = 0;
        } else {
            _coachOrPlayer ^= 1;
            uint8_t nc = _coachOrPlayer ? TEAM_PLAYER_COACH : TEAM_COACH;
            _careerTeam.team.teamControls = nc;
            _selectedTeams->team.teamControls = nc;
            if (_trainingGame)
                _currentTeam.team.teamControls = 1;
        }
    } else {
        TeamFileEx *t = _playMatchTeam1Ptr;
        t->team.teamControls = (t->team.teamControls == TEAM_PLAYER_COACH)
                          ? TEAM_COACH : TEAM_PLAYER_COACH;
    }
    SetVariableButtons();
    DrawMenuItemsWaitForVSync(_togglePlayerCoachRedrawItemList);
}

/* ================================================================
 * DrawPlayerEntries  (line 81983)
 *
 * Before-draw callback for player entries in play match menu.
 * Sets colors, status sprites, shirt numbers, names, positions,
 * skills, and value/status for each player row.
 * ================================================================ */
__attribute__((used, externally_visible))
void OnCareerReserveSelected(MenuEntry *e)
{
#ifdef SWOS_SDL_BUILD
    if(!Expanded_CareerLineupEditsAllowed() || !Expanded_CareerExtraVisible())return;
    const int row=(int)e->ordinal-kCareerExtraFirst;
    if(row<0 || row>=kCareerExtraCount ||
       16u+(unsigned)row>=(unsigned)_careerNumPlayers)return;
    s_extraReserveSlot=(int16_t)(16+row);
    OnSwapPlayer(e);
    s_extraReserveSlot=-1;
#else
    (void)e;
#endif
}

__attribute__((used, externally_visible))
void DrawPlayerEntries(MenuEntry *a5) {
    int16_t d7 = a5->ordinal - 13;
    int16_t d5 = 0;
#ifdef SWOS_SDL_BUILD
    if(s_extraDrawSlot>=0)d7=s_extraDrawSlot;
#endif

    MenuEntry *bgE = &_menuEntries[31];  /* entry 31 */
    MenuEntry *numE = &_menuEntries[33]; /* entry 33 */

    if (d7 == 16
#ifdef SWOS_SDL_BUILD
        && s_extraDrawSlot<0
#endif
       ) {
        d5 = 1;
        d7 = d7 - 5 + _maxSubstitutesEntries;
        bgE->bgColor = (bgE->bgColor & 0xFFF0) | BLUE_TO_BLACK_8;
        numE->bgColor = (numE->bgColor & 0xFFF0) | BLUE_TO_BLACK_8;
    } else if (d7 < 11) {
        if (!_showingCpuTeams && !_viewingOpponentTeam) {
            uint16_t c = (d7 == 0) ? YELLOW_GREEN_BLACK_14 : BRIGHT_BLUE_TO_BLUE_13;
            bgE->bgColor = (bgE->bgColor & 0xFFF0) | c;
            numE->bgColor = (numE->bgColor & 0xFFF0) | c;
        } else {
            bgE->bgColor = (bgE->bgColor & 0xFFF0) | RED_10;
            numE->bgColor = (numE->bgColor & 0xFFF0) | RED_10;
        }
    } else {
        /* >= 11: substitute */
        if (!_viewingOpponentTeam && !_showingCpuTeams) {
            if (d7 == 11 && _maxSubstitutesEntries > 5) {
                bgE->bgColor = (bgE->bgColor & 0xFFF0) | YELLOW_GREEN_BLACK_14;
                numE->bgColor = (numE->bgColor & 0xFFF0) | YELLOW_GREEN_BLACK_14;
            } else {
                /* v327 (user, 26/11): the bench colour only marks the seats the competition admits
                 * (first _maxSubstitutesEntries rows after the eleven); every other player, the extra
                 * squad of the right-hand column included, gets the EXCLUDED colour. */
                int16_t col = BLUE_TO_PURPLE_11;
#ifdef SWOS_SDL_BUILD
                /* the left rows are bench seats 1..5; the right-hand extra squad continues the SAME bench:
                 * with a bench of 7 the first two extras are bench seats 6 and 7 (bench colour), the rest
                 * of that column is excluded. */
                {
                    const int16_t bench = g_pmRealBench ? (int16_t)g_pmRealBench : (int16_t)_maxSubstitutesEntries;
                    if (s_extraDrawSlot >= 0) {
                        if ((int16_t)(5 + (s_extraDrawSlot - 16)) >= bench)
                            col = BLUE_TO_BLACK_8;
                    } else if ((d7 - 11) >= bench)
                        col = BLUE_TO_BLACK_8;
                }
#else
                if ((d7 - 11) >= (int16_t)_maxSubstitutesEntries)
                    col = BLUE_TO_BLACK_8;
#endif
                bgE->bgColor = (bgE->bgColor & 0xFFF0) | col;
                numE->bgColor = (numE->bgColor & 0xFFF0) | col;
            }
        } else {
            bgE->bgColor = (bgE->bgColor & 0xFFF0) | ORANGE_TO_BROWN_12;
            numE->bgColor = (numE->bgColor & 0xFFF0) | ORANGE_TO_BROWN_12;
        }
    }

    TeamFileEx *team = _playMatchTeam1Ptr;
    if (_viewingOpponentTeam) team = _playMatchTeam2Ptr;

    uint16_t slot, fIdx;
    uint8_t *player = GetPlayerInfoBreakPosition((TeamFileEx *)team, (uint16_t)d7, &slot, &fIdx);
    int16_t d4 = (int16_t)slot;
    int16_t d1 = (int16_t)fIdx;

    /* Injured check — DISPLAY decision, so it must use the honest status.
     * IsPlayerInjured carries the testers'-rule (test_rules.h) override
     * (returns 0 for everyone), which is a GAMEPLAY-availability override; using
     * it here made an injured/suspended player's row keep its normal colour
     * while SetSquadMenuEntryStatusSprite — which reads pf->cardsInjuries
     * directly and is unaffected by the flag — still drew the "3" indicator.
     * Same split already applied to the edit-tactics squad screen
     * (edit_tactics.c EditTacticsAfterDrawPlayerNames). */
    if (!_viewingOpponentTeam) {
        if (IsPlayerInjuredForDisplay((PlayerFile *)player, _playMatchExcludeFlags)) {
            bgE->bgColor = (bgE->bgColor & 0xFFF0) | RED_10;
            numE->bgColor = (numE->bgColor & 0xFFF0) | RED_10;
        }
    }

    /* Selected player highlight */
    if (!_viewingOpponentTeam && d1 == _selectedPlayerPosition) {
        bgE->bgColor = (bgE->bgColor & 0xFFF0) | ORANGE_TO_BROWN_6;
        numE->bgColor = (numE->bgColor & 0xFFF0) | ORANGE_TO_BROWN_6;
    }

    bgE->y = a5->y;

    /* Status sprite */
    SetSquadMenuEntryStatusSprite(bgE, (PlayerFile *)player, (uint16_t)_playMatchExcludeFlags);

    /* Position change arrows */
    if (!_viewingOpponentTeam && d4 < 11) {
        uint8_t cur = _playerPositionsArray[d1];
        uint8_t old = _oldPlayerPositionsArray[d1];
        if (cur < old) bgE->fgWord = SPR_CROSS_SMALL;       /* value fell → red cross */
        else if (cur > old) bgE->fgWord = SPR_CHECK_SMALL;  /* value rose → green tick */
    }

    /* Checkmark entry sync */
    {
        int16_t selIdx;
        if (_viewingOpponentTeam) {
            selIdx = (int16_t)*(uint16_t *)_currentSelectedPlayerIndexPtr;
        } else if (_playMatchTeam1Ptr == &_careerTeam
#ifdef SWOS_SDL_BUILD
                   || Expanded_ManagedCareerTeam(_playMatchTeam1Ptr)
#endif
                  ) {
            selIdx = (int16_t)_selTeamSelectedPlayerPosition;
        } else if (_playMatchTeam1Ptr == &_currentTeam) {
            selIdx = (int16_t)_currentTeamSelectedPlayerPosition;
        } else {
            selIdx = (int16_t)*(uint16_t *)_currentSelectedPlayerIndexPtr;
        }
        if (selIdx == d1) {
            MenuEntry *ck = &_menuEntries[37];
#ifdef SWOS_SDL_BUILD
            if(s_extraNavReady)
                ck->x=s_extraDrawSlot>=0?
                    (int16_t)(a5->x-12):s_extraNativeCheckX;
#endif
            ck->y = bgE->y;
            ck->bgColor = bgE->bgColor;
            ck->fgWord = bgE->fgWord;
        }
    }

    /* Shirt number entry (entry 32) */
    MenuEntry *shE = bgE + 1;
    PlayerFile *playerPf = (PlayerFile *)(player + 76);
    shE->isInvisible = 1;
    if (!d5) {
        shE->fgWord = (uint16_t)(uint8_t)playerPf->shirtNumber;
        shE->isInvisible = 0;
    }
    shE->y = a5->y;

    /* Surname entry (entry 33 = numE) */
    {
        uint8_t *surname = ExtractSurname((uint8_t *)playerPf->playerName, 0x800D);
        numE->fg = (uintptr_t)surname;
        numE->y = a5->y;
    }

    /* Position text (entry 34) */
    MenuEntry *posE = numE + 1;
    {
        uint16_t posField = (playerPf->positionAndFace & PF_POSITION_MASK) >> 3;
        posE->fg = (uintptr_t)_playerPositionsStringTable[posField >> 2];
        posE->y = a5->y;
    }

    /* Skills string (entry 35) */
    MenuEntry *skE = posE + 1;
    FormTopSkillsString((uint8_t *)skE->fg, (PlayerFile *)player);
    skE->y = a5->y;

    /* Value/status (entry 36) */
    MenuEntry *valE = skE + 1;
    valE->isInvisible = 1;
    valE->y = a5->y;

    {
        if (IsTeamNational((TeamFileEx *)_playMatchTeam1Ptr)) {
            InitPlayerValueDisplayStruct(player, (uint8_t *)valE);
        } else {
            FormatPlayerValueOrStatusText(player, _fundsBuffer);
            valE->fg = (uintptr_t)_fundsBuffer;
        }
    }

    {
        uint16_t cb = 0x0D;
        uint8_t sf = (uint8_t)playerPf->form;
        if (sf == 0x9B)      cb = 0x0D;
        else if (sf == 0x9A) cb = 0x01;
        else {
            cb = 0x0F;
            if (!(playerPf->positionAndFace & PF_LOAN)) cb = PF_LOAN;
        }
        valE->stringFlags = (valE->stringFlags & 0xFFF0) | cb;
        valE->isInvisible = 0;
    }

#ifdef SWOS_SDL_BUILD
    int16_t savedX[6];
    if(s_extraDrawSlot>=0)
        for(unsigned i=0;i<6;++i) {
            savedX[i]=_menuEntries[31+i].x;
            _menuEntries[31+i].x=(int16_t)(savedX[i]+93+kCareerExtraShiftX);
        }
#endif
    /* Extended reserves use real OG player/status/name/position only.
     * Drop solely their yellow skill column (entry35) and value (36):
     * never hide the shared scratch entries used by the starters. */
#ifdef SWOS_SDL_BUILD
    DrawMultipleItems(s_extraDrawSlot>=0?4:6, &_menuEntries[31]);
#else
    DrawMultipleItems(6, &_menuEntries[31]);
#endif
#ifdef SWOS_SDL_BUILD
    if(s_extraDrawSlot>=0)
        for(unsigned i=0;i<6;++i)_menuEntries[31+i].x=savedX[i];
#endif
}

/* Temporary second list; reuses the original OG row renderer and input. */
__attribute__((used, externally_visible))
void DrawCareerReserveEntries(MenuEntry *e)
{
#ifdef SWOS_SDL_BUILD
    if(!Expanded_CareerExtraVisible())return;
    const int row=(int)e->ordinal-kCareerExtraFirst;
    if(row<0 || row>=kCareerExtraCount ||
       16u+(unsigned)row>=(unsigned)_careerNumPlayers)return;
    s_extraDrawSlot=(int16_t)(16+row);
    DrawPlayerEntries(e);
    s_extraDrawSlot=-1;
#else
    (void)e;
#endif
}

/* ================================================================
 * Chunk 151: FormTopSkillsString
 *
 * Forms the 3-letter skill abbreviation string shown after a player's
 * name in the play match menu (e.g., "TSP", "HCF").
 *
 * in:  a1 = destination buffer (must hold 4 bytes)
 *      a6 = player base (team entry pointer, player data at +0x4C)
 *
 * The algorithm:
 * - Get position group (bits 7-5 of POSITIONANDFACE field)
 * - Select even/odd skill letter and bit-rotation tables based on
 *   XOR of price and first two name bytes (bit 0)
 * - Extract 7 skill values (3-bit each) from PASSING longword via
 *   rotating by amounts from the bit-rotation table
 * - Pick the top 3 skill values and output their corresponding letters
 * ================================================================ */
extern void *_topSkillLettersEven[];  /* 7 pointers to letter strings */
extern void *_topSkillLettersOdd[];
extern void *_topSkillBitRotEven[];   /* 7 pointers to byte arrays */
extern void *_topSkillBitRotOdd[];

__attribute__((used, externally_visible))
void FormTopSkillsString(uint8_t *dst, PlayerFile *playerFile)
{
    /* playerFile = GetPlayer-style pointer (teamBase + fileIdx*38) cast to PlayerFile *.
     * PlayerFile data is at playerFile[2] (2 * 38 = 76 bytes in). */
    PlayerFile *pf = &playerFile[2];

    uint16_t posGroup = (pf->positionAndFace & PF_POSITION_MASK) >> 5;
    if (posGroup == 0) {
        /* Goalkeeper: no skill string */
        *dst = 0;
        return;
    }
    posGroup -= 1;  /* 0-based index into tables (0-6) */

    /* Determine even/odd variant from XOR of price and name bytes */
    uint8_t price = (uint8_t)pf->price;
    uint8_t name0 = (uint8_t)pf->playerName[0];
    uint8_t name1 = (uint8_t)pf->playerName[1];
    uint8_t xorVal = price ^ name0 ^ name1;

    const uint8_t *letters;
    const uint8_t *bitRot;

    if (xorVal & 1) {
        letters = (const uint8_t *)_topSkillLettersOdd[posGroup];
        bitRot = (const uint8_t *)_topSkillBitRotOdd[posGroup];
    } else {
        letters = (const uint8_t *)_topSkillLettersEven[posGroup];
        bitRot = (const uint8_t *)_topSkillBitRotEven[posGroup];
    }

    /* Get PASSING longword (4 skills packed: passing, velocityHeading,
     * tacklingControl, speedFinishing) and shift left 4.
     *
     * The Amiga asm did `move.l (a0)+,d7` which on m68k BE reads the
     * bytes as (passing<<24)|(velocityHeading<<16)|(tacklingControl
     * <<8)|speedFinishing. A native `*(uint32_t *)` on our LE host
     * reverses the byte order, so the subsequent rot+mask extracts the
     * wrong skill nibbles — giving wrong top-skill letters ("VHF" in
     * place of "STP" etc.) and breaking the star count the same way.
     * Use BE_READ_U32 so both builds see the same packed layout. */
    uint32_t d7 = BE_READ_U32(&pf->passing);
    d7 <<= 4;

    /* Find top 3 skill values. Each skill is extracted as 3 bits after
     * rotating d7 left by the amount from bitRot[]. */
    int16_t best1 = -1, best2 = -1, best3 = -1;   /* skill values */
    uint8_t let1 = 0, let2 = 0, let3 = 0;          /* corresponding letters */

    for (int16_t i = 0; i < 7; i++) {
        uint8_t rot = bitRot[i];
        uint32_t rotated = (d7 << rot) | (d7 >> (32 - rot));
        uint8_t skillVal = (uint8_t)(rotated & 7);

        if ((int8_t)skillVal > (int8_t)best1) {
            best3 = best2; let3 = let2;
            best2 = best1; let2 = let1;
            best1 = skillVal;
            let1 = letters[i];
        } else if ((int8_t)skillVal > (int8_t)best2) {
            best3 = best2; let3 = let2;
            best2 = skillVal;
            let2 = letters[i];
        } else if ((int8_t)skillVal > (int8_t)best3) {
            best3 = skillVal;
            let3 = letters[i];
        }
    }

    *dst++ = let1;
    *dst++ = let2;
    *dst++ = let3;
    *dst = 0;
}

/* ================================================================
 * Chunk 212: ShowPlayMatchMenu (1 → play_match_menu.c)
 * ================================================================ */

extern uint16_t _inEditTactics;
extern uint8_t _playMatchMenu[];
extern void TrainingMatch_AdjustSelectedTeamRosterForTrialists(uint16_t teamNum, uint16_t baseCount, TeamFileEx *teamPtr);

/* ShowPlayMatchMenu (SWOS2.s line 65312)
 * Sets up play match globals from params, optionally adjusts trialists,
 * shows the play match menu, returns playerNumThatStarted. */
__attribute__((used, externally_visible))
int16_t ShowPlayMatchMenu(int16_t controlFlags, int16_t _maxSubstitutes,
                            int16_t excludeFlags, int16_t isFirstPlayer,
                            int16_t squadChanges, TeamFileEx *team1, TeamFileEx *team2)
{
    _isFirstPlayerChoosingTeam = (uint16_t)isFirstPlayer;
    _squadChangesAllowed = (uint16_t)squadChanges;
    _playMatchTeam1Ptr = team1;
    _playMatchTeam2Ptr = team2;
    _maxSubstitutesEntries = (uint16_t)_maxSubstitutes;
    _playMatchControlFlags = (uint16_t)controlFlags;
    _playMatchExcludeFlags = (int16_t)excludeFlags;
    _inEditTactics = (uint16_t)excludeFlags;

    if (_gameType == GAME_TYPE_CAREER) {
        TrainingMatch_AdjustSelectedTeamRosterForTrialists(
            (uint16_t)_playMatchExcludeFlags,
            _maxSubstitutesEntries,
            (TeamFileEx *)_playMatchTeam1Ptr);
    }

    ShowMenu(_playMatchMenu);
    return (int16_t)_playerNumThatStarted;
}
