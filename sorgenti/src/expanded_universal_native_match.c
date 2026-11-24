/* SWOS Expanded universal career: ORIGINAL OG score functions only.
 * CPU/VIEW RESULT uses OG CalculateViewResult; actual PLAY uses OG 50 Hz.
 * No old TSKC, no DIY league/cup writer, no standings or CAR writes here.
 * A caller must provide persistent live rosters and durable fixture claim.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_native_match.h"
#include "game_init.h"
#include "match_sim.h" /* the original OG fast VIEW RESULT scorer */
#include "expanded_world_live.h" /* fixture date -> OG view-random phase */
#include "platform_sdl.h" /* real monotonic OG 50 Hz match duration */
#include "play_match_menu.h" /* GAME_TYPE_CAREER from existing OG header */
#include "world.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h> /* timer/main thread scope published atomically */

extern uint16_t _gameType, _exitGameFlag, _gameCanceled, _isCareer;
extern uint16_t _quitToDos; /* SDL close/crash/quit request, never clear here */
extern uint16_t _gamePitchTypeOrSeason, _gameSeason, _gamePitchType;
extern uint16_t _team1GoalsFirstLeg, _team2GoalsFirstLeg;
extern int16_t _extraTimeState, _penaltiesState;
extern uint16_t _secondLeg, _playing2ndGame, _isGameFriendly;
extern uint16_t _statsTeam1GoalsCopy, _statsTeam2GoalsCopy;
extern uint16_t _statsTeam1Goals, _statsTeam2Goals;
extern uint16_t _team1PenaltyGoals, _team2PenaltyGoals;
extern uint8_t _gameName[], _gameRound;
extern TeamFileEx _careerTeam;
extern uint8_t _viewRandomPhase, _currentMatchPlayers[];
extern uint16_t _inEditTactics; /* scorer writes this transient OG mode */
extern uint16_t _maxSubsPerTeam, _gameState, _gameStatePl;
extern uint16_t _team1PenaltyGoals, _team2PenaltyGoals;

/* ---- Fase 2: substitution-window adapter (CE 50 Hz only) ----
 * OG limits only the NUMBER of substitutions (_maxSubsPerTeam). Modern
 * rules also limit the number of interruptions ("windows"). A window is
 * one bench session in which at least one substitution was confirmed; a
 * half-time session is free when the rules say so; extra time may add
 * substitutions and windows. Inactive (OG behaviour) unless a CE match
 * with windows>0 is running: CLASSIC/DIY/SEASON/friendlies untouched. */
static struct {
    uint8_t active, windows, halftime_free, et_extra_windows, et_extra_subs;
    uint8_t used[2], session_counted[2], et_applied;
} s_subw;
static int subw_halftime(void)
{
    return _gameStatePl!=ST_GAME_IN_PROGRESS &&
        (_gameState==ST_GOING_TO_HALFTIME || _gameState==ST_PLAYERS_GOING_TO_SHOWER ||
         _gameState==ST_RESULT_ON_HALFTIME || _gameState==ST_FIRST_HALF_ENDED ||
         _gameState==ST_FIRST_EXTRA_ENDED);
}
static unsigned subw_limit(void)
{
    return (unsigned)s_subw.windows+(s_subw.et_applied?s_subw.et_extra_windows:0u);
}
int ExpandedUniversal_SubWindowAllows(int team_number)
{
    const int t=team_number==1?0:1;
    if(!s_subw.active || !s_subw.windows)return 1;
    if(s_subw.session_counted[t])return 1; /* same interruption */
    if(s_subw.halftime_free && subw_halftime())return 1;
    return s_subw.used[t]<subw_limit();
}
void ExpandedUniversal_SubWindowSessionBegin(int team_number)
{
    if(s_subw.active)s_subw.session_counted[team_number==1?0:1]=0u;
}
void ExpandedUniversal_SubWindowCommitted(int team_number)
{
    const int t=team_number==1?0:1;
    if(!s_subw.active || !s_subw.windows || s_subw.session_counted[t])return;
    if(s_subw.halftime_free && subw_halftime())return;
    s_subw.session_counted[t]=1u;
    ++s_subw.used[t];
    fprintf(stderr,"[universal-native] SUB_WINDOW team=%d used=%u limit=%u\n",
        team_number,(unsigned)s_subw.used[t],subw_limit());
}
void ExpandedUniversal_SubWindowExtraTime(void)
{
    if(!s_subw.active || s_subw.et_applied)return;
    s_subw.et_applied=1u;
    if(s_subw.et_extra_subs)_maxSubsPerTeam=(uint16_t)(_maxSubsPerTeam+s_subw.et_extra_subs);
}
extern uint8_t *StringCopy(const uint8_t *,uint8_t *);

/* Only the universal dispatcher sets this scope, around its original
 * OG 50 Hz game. Legacy match/replay/other worlds never use it. */
static SDL_atomic_t s_universal_cpu_match;
static SDL_atomic_t s_universal_match_active;
static SDL_atomic_t s_universal_match_completed;
void ExpandedUniversal_NativeMarkMatchCompleted(void)
{
    if(SDL_AtomicGet(&s_universal_match_active))
        SDL_AtomicSet(&s_universal_match_completed,1);
}
int ExpandedUniversal_NativeCpuSilent(void)
{
    return SDL_AtomicGet(&s_universal_cpu_match)!=0;
}
int ExpandedUniversal_NativeTeamKey(const TeamFileEx *team,
                                    ExpandedClubKey *club)
{
    uint8_t file,country,ordinal;
    uint16_t slot,packed;
    if(!team || !club || World_Current()>=WORLD_COUNT)return 0;
    file=team->team.countryNumber;
    ordinal=team->team.teamOrdinal;
    /* v299: OG national teams live in the region files 80..85 (CN_EUROPE..
     * CN_OCEANIA, career_init.c): their real team number is the identity,
     * with an explicit NATIONAL kind (never a club slot). */
    if(file>=80u && file<=85u) {
        *club=ExpandedNationalKey((uint16_t)World_Current(),
                                  (uint16_t)(((uint16_t)file<<8)|ordinal));
        return ExpandedClubKey_Valid(*club);
    }
    country=World_FileCountry(file);
    slot=World_Slot(file,ordinal);
    packed=(uint16_t)(((uint16_t)file<<8)|ordinal);
    if(slot>=WORLD_SLOTS || World_SlotTeam(country,slot)!=packed)
        return 0;
    club->world=(uint16_t)World_Current();
    club->stable_club=((uint32_t)country<<16)|((uint32_t)slot+1u);
    return ExpandedClubKey_Valid(*club);
}
static int exp_universal_native_match_valid(
    const ExpandedUniversalNativeMatch *c,
    const ExpandedUniversalFixture *f)
{
    ExpandedClubKey h={0},a={0};
    if(!c || !c->home || !c->away || c->home==c->away ||
       !ExpandedUniversal_FixtureValid(f) ||
       f->key.edition.competition.world!=(uint16_t)World_Current() ||
       !ExpandedUniversal_NativeTeamKey(c->home,&h) ||
       !ExpandedUniversal_NativeTeamKey(c->away,&a) ||
       !ExpandedClubKey_Equal(h,f->home) ||
       !ExpandedClubKey_Equal(a,f->away) ||
       _gameType!=GAME_TYPE_CAREER ||
       c->minimum_substitutes<0 ||
       c->maximum_substitutes<c->minimum_substitutes ||
       c->maximum_substitutes>16 ||
       c->interactive>1u)return 0;
    /* A type0 league's repeated meetings are independent matches,
     * NOT KO two-leg ties. Accept 1..4 JSON-declared cycles (leg 0..3);
     * the separate legacy bracket/aggregate remains forbidden. */
    if(f->competition_type!=0u || f->key.leg>3u ||
       f->key.replay!=0u)return 0;
    return 1;
}
static int s_result_prompt;
void ExpandedUniversal_NativeSetResultPrompt(int on)
{
    s_result_prompt=on!=0;
}
int ExpandedUniversal_NativePrepare(
    ExpandedUniversalNativeMatch *context,
    const ExpandedUniversalFixture *fixture)
{
    if(!context)return 0;
    context->lineup_ready=0u;context->abandoned=0u;
    if(!exp_universal_native_match_valid(context,fixture))return 0;
    if(!context->interactive && s_result_prompt &&
       (context->home==&_careerTeam || context->away==&_careerTeam)) {
        /* OG career RESULT on the manager's fixture: the original team
         * screen with VIEW RESULT (same SetupPlayers prompt as the OG DIY
         * RESULT path). EXIT there cancels BEFORE CAR CLAIMED. */
        extern void Expanded_CareerNativeOGResultPromptSet(int);
        const uint16_t savedHome=context->home->team.teamControls;
        const uint16_t savedAway=context->away->team.teamControls;
        if(context->home!=&_careerTeam)
            context->home->team.teamControls=TEAM_COMPUTER;
        if(context->away!=&_careerTeam)
            context->away->team.teamControls=TEAM_COMPUTER;
        _exitGameFlag=0;
        Expanded_CareerNativeOGResultPromptSet(1);
        SetupPlayers(context->home,context->away,
                     context->maximum_substitutes,context->match_parameters,
                     0,0);
        Expanded_CareerNativeOGResultPromptSet(0);
        if(context->home!=&_careerTeam)
            context->home->team.teamControls=savedHome;
        if(context->away!=&_careerTeam)
            context->away->team.teamControls=savedAway;
        if(_exitGameFlag)return 0;
    }
    if(!context->interactive) {
        /* Universal OG quick scoring is only certified for league 0/5
         * inputs. A malformed params/manager lineup is INVALID BEFORE
         * CAR CLAIMED, never an unreplayable poisoned claim. Runtime scorer
         * checks the same values again after the durable claim. */
        unsigned i;
        if(context->maximum_substitutes<1 ||
           context->maximum_substitutes>5 ||
           context->minimum_substitutes<0 ||
           context->minimum_substitutes>context->maximum_substitutes)
            return 0;
        if(context->home==&_careerTeam || context->away==&_careerTeam)
            for(i=0u;i<11u;++i)
                if(_currentMatchPlayers[i]>=32u)return 0;
    }
    if(context->interactive) {
        /* The JSON rosters default teamControls to 0 (human).  The original
         * OG SetupPlayers must see exactly ONE human side, otherwise it
         * offers a second controller/tactics screen for the CPU opponent.
         * Keep the manager's Player/Coach selection; this is temporary and
         * is restored even when the original OG UI is cancelled. */
        const uint16_t savedHome=context->home->team.teamControls;
        const uint16_t savedAway=context->away->team.teamControls;
        if(context->home!=&_careerTeam)
            context->home->team.teamControls=TEAM_COMPUTER;
        if(context->away!=&_careerTeam)
            context->away->team.teamControls=TEAM_COMPUTER;
        _exitGameFlag=0;
        SetupPlayers(context->home,context->away,
                     context->maximum_substitutes,context->match_parameters,
                     1,0);
        if(context->home!=&_careerTeam)
            context->home->team.teamControls=savedHome;
        if(context->away!=&_careerTeam)
            context->away->team.teamControls=savedAway;
        if(_exitGameFlag)return 0;
    }
    context->lineup_ready=1u;
    return 1;
}
/* OG's historical CPU auto-fixture mode never ran the match GameLoop.
 * Its original view-result scorer is the sole result producer here.
 * We pass a zeroed LEAGUE scoring context (no second OG table/fixture);
 * league placement-price adjustments requiring resident DIY standings
 * remain unavailable in JSON-first mode and must not be invented.
 * This call updates authentic OG scorer, injury/card and RNG fields on
 * the persistent team records; those effects are saved by the SAME CAR.
 * VIEW RESULT for an employed manager uses this same boundary, whereas
 * the separate PLAY callback still invokes the original 50 Hz engine. */
static uint8_t s_quick_flags;
static uint16_t s_quick_pen_home,s_quick_pen_away;
static int exp_universal_native_quick_result(
    const ExpandedUniversalNativeMatch *c,
    const ExpandedUniversalFixture *fixture,
    ExpandedUniversalMatchWitness *witness)
{
    /* Original CalculateViewResult holds ctx in a module-static pointer.
     * Give it a suitably aligned, persistent context backed by the FULL
     * DIYCompetitionEx size; a temporary packed local could leave a
     * dangling pointer or too-short standings tail for OG price logic. */
    static union {
        DIYCompetition base;
        unsigned char bytes[sizeof(DIYCompetitionEx)];
    } scoring;
    ExpandedWorldDate day={0};
    uint16_t home=0u,away=0u;
    const uint8_t previous_phase=_viewRandomPhase;
    const uint16_t previous_in_edit=_inEditTactics;
    s_quick_flags=0u;s_quick_pen_home=0u;s_quick_pen_away=0u;
    const Uint64 started=Platform_NowMs();
    /* The original scorer indexes candidate player tables by
     * (maximum_substitutes-1). A zero here underflows before the table.
     * Universal league config currently uses exactly 0/5. */
    if(!ExpandedWorld_IndexToDate(fixture->day_index,&day) ||
       _quitToDos || c->maximum_substitutes<1 ||
       c->maximum_substitutes>5 ||
       c->minimum_substitutes<0 ||
       c->minimum_substitutes>c->maximum_substitutes)
        return 0;
    /* The manager's original VIEW RESULT scorer reads the shared XI.
     * Do not index an unverified player slot in the 32-man career roster. */
    if(c->home==&_careerTeam || c->away==&_careerTeam) {
        unsigned i;
        for(i=0;i<11u;++i)
            if(_currentMatchPlayers[i]>=32u)return 0;
    }
    _exitGameFlag=0u;
    _gameCanceled=0u;
    memset(&scoring,0,sizeof scoring);
    scoring.base.competitionType=0;
    scoring.base.numTeams=2;
    scoring.base.substitutesCount=c->match_parameters;
    scoring.base.viewRandomPhase=day.month;
    _viewRandomPhase=day.month;
    CalculateViewResult(&scoring.base,(uint16_t)c->og_mode_flag,
        (uint16_t)c->minimum_substitutes,
        (uint16_t)c->maximum_substitutes,0u,
        c->home,c->away,&home,&away);
    {
        /* Fase 2: deciding match of a knockout tie, OG quick resolvers in
         * the ORIGINAL order of ProcessDIYKnockoutMatchResultAndCareerUpdates:
         * aggregate -> away goals (only if the rules enable it) -> extra
         * time (ExtraTimeSecondPhaseScoring) -> penalties
         * (PenaltyShootoutScorerPair). Same module state as the scorer. */
        if(c->decides_tie && home<=31u && away<=31u) {
            uint32_t ah=(uint32_t)home+c->first_leg_home,
                     aa=(uint32_t)away+c->first_leg_away;
            int level=ah==aa;
            if(level && c->second_leg && c->away_goals &&
               c->first_leg_home!=away)level=0; /* decided on away goals */
            if(level && c->extra_time) {
                uint16_t et1=0u,et2=0u;
                ExtraTimeSecondPhaseScoring(&et1,&et2);
                home=(uint16_t)(home+et1);away=(uint16_t)(away+et2);
                s_quick_flags|=0x01u; /* EXP_CE_SCORE_EXTRA_TIME */
                ah=(uint32_t)home+c->first_leg_home;
                aa=(uint32_t)away+c->first_leg_away;
                level=ah==aa;
            }
            if(level && c->penalties) {
                int16_t p1=0;uint16_t p2=0u;
                PenaltyShootoutScorerPair(&p1,&p2);
                if(p1<0 || (uint16_t)p1==p2) {
                    fprintf(stderr,"[universal-native] OG_QUICK_PENALTIES_LEVEL "
                        "event=%llu %d:%u no-result\n",
                        (unsigned long long)fixture->event_id,(int)p1,(unsigned)p2);
                    _viewRandomPhase=previous_phase;_inEditTactics=previous_in_edit;
                    return 0;
                }
                s_quick_pen_home=(uint16_t)p1;s_quick_pen_away=p2;
                s_quick_flags|=0x02u; /* EXP_CE_SCORE_PENALTIES */
            }
        }
    }
    _viewRandomPhase=previous_phase;
    _inEditTactics=previous_in_edit;
    if(_quitToDos || home>31u || away>31u) {
        fprintf(stderr,"[universal-native] OG_QUICK_REJECTED "
            "event=%llu score=%u:%u quit=%u CAR_CLAIMED_NO_REPLAY=1\n",
            (unsigned long long)fixture->event_id,(unsigned)home,
            (unsigned)away,(unsigned)_quitToDos);
        return 0;
    }
    witness->key=fixture->key;
    witness->home=fixture->home;
    witness->away=fixture->away;
    witness->day_index=fixture->day_index;
    witness->score=(ExpandedFixtureScore){home,away,s_quick_pen_home,
                                          s_quick_pen_away,s_quick_flags};
    witness->valid=1u;
    fprintf(stderr,"[universal-native] OG_QUICK_RESULT "
        "event=%llu score=%u:%u elapsed_ms=%llu "
        "original=CalculateViewResult game-loop=NO CAR_ONE_WITNESS=1\n",
        (unsigned long long)fixture->event_id,(unsigned)home,
        (unsigned)away,
        (unsigned long long)(Platform_NowMs()-started));
    return 1;
}
int ExpandedUniversal_NativeRunOriginal(
    void *user,const ExpandedUniversalFixture *fixture,
    ExpandedUniversalMatchWitness *witness)
{
    ExpandedUniversalNativeMatch *const c=(ExpandedUniversalNativeMatch *)user;
    uint16_t oldCareer;
    uint16_t homeControls,awayControls;
    Uint64 og_started_ms;
    int bad;
    if(witness)memset(witness,0,sizeof *witness);
    if(!witness || !c || !c->lineup_ready ||
       !exp_universal_native_match_valid(c,fixture))return 0;
    /* A prepared session is one-shot, even on cancel / match fault. */
    c->lineup_ready=0u;
    oldCareer=_isCareer;
    homeControls=c->home->team.teamControls;
    awayControls=c->away->team.teamControls;
    if(!c->interactive) {
        int quick_ok;
        /* CPU and manager VIEW RESULT use the ORIGINAL OG quick scorer.
         * Never open a field or invoke the old DIY competition writer. */
        if(c->home!=&_careerTeam)
            c->home->team.teamControls=TEAM_COMPUTER;
        if(c->away!=&_careerTeam)
            c->away->team.teamControls=TEAM_COMPUTER;
        _isCareer=(uint16_t)(c->home==&_careerTeam || c->away==&_careerTeam);
        quick_ok=exp_universal_native_quick_result(c,fixture,witness);
        c->home->team.teamControls=homeControls;
        c->away->team.teamControls=awayControls;
        _isCareer=oldCareer;
        return quick_ok;
    }
    /* Keep the CPU side computer-controlled THROUGH the original 50 Hz
     * GameLoop.  NativePrepare restored roster metadata after the menu;
     * here we restore it after the actual match (including abort). */
    if(c->home!=&_careerTeam)
        c->home->team.teamControls=TEAM_COMPUTER;
    if(c->away!=&_careerTeam)
        c->away->team.teamControls=TEAM_COMPUTER;
    _isCareer=(uint16_t)(c->home==&_careerTeam || c->away==&_careerTeam);
    if(c->competition_name && c->competition_name[0]) {
        uint8_t name[31];
        size_t n=0;
        while(n<sizeof name-1u && c->competition_name[n]) {
            name[n]=(uint8_t)c->competition_name[n];++n;
        }
        name[n]=0;
        StringCopy(name,_gameName);
    }
    _gameRound=0u;
    _gamePitchTypeOrSeason=c->pitch_type_or_season;
    _gameSeason=c->season;
    _gamePitchType=c->pitch_type;
    _team1GoalsFirstLeg=0u;_team2GoalsFirstLeg=0u;
    _extraTimeState=0;_penaltiesState=0;
    _secondLeg=0u;_playing2ndGame=0u;_isGameFriendly=0u;
    _exitGameFlag=0u;_gameCanceled=0u;
    /* Fase 2: knockout rules through the ORIGINAL OG globals, exactly as
     * PlayDIYCupFixtureAndCacheResult sets them (second leg + first-leg
     * goals, extra time / penalties only on the deciding match, away goals
     * only when the competition has that rule: _playing2ndGame==1). */
    if(c->decides_tie) {
        if(c->extra_time)_extraTimeState=1;
        if(c->penalties)_penaltiesState=1;
        if(c->second_leg) {
            _secondLeg=1u;
            _team1GoalsFirstLeg=c->first_leg_home;
            _team2GoalsFirstLeg=c->first_leg_away;
            _playing2ndGame=c->away_goals?1u:0u;
        }
    }
    memset(&s_subw,0,sizeof s_subw);
    s_subw.active=1u;
    s_subw.windows=c->sub_windows;s_subw.halftime_free=c->halftime_free;
    s_subw.et_extra_subs=c->et_extra_subs;s_subw.et_extra_windows=c->et_extra_windows;
    /* A new original match needs a NEW completion witness. Cached score
     * values left by a prior match are NOT proof of this game's completion. */
    SDL_AtomicSet(&s_universal_match_completed,0);
    SDL_AtomicSet(&s_universal_match_active,1);
    /* True OpenSWOS 50Hz/AI input and post-match player effects.
     * No old DIY competition progression callback is called. */
    og_started_ms=Platform_NowMs();
    InitializeInGameTeamsAndStartGame(0,c->minimum_substitutes,
        c->maximum_substitutes,c->match_parameters,c->match_postprocess,
        (TeamFile *)c->home,(TeamFile *)c->away);
    SDL_AtomicSet(&s_universal_match_active,0);
    s_subw.active=0u;
    /* Restore the caller's original controls even after an OG failure;
     * every claimed match still remains non-replayable on uncertain score. */
    c->home->team.teamControls=homeControls;
    c->away->team.teamControls=awayControls;
    _isCareer=oldCareer;
    bad=!SDL_AtomicGet(&s_universal_match_completed) || _quitToDos ||
        _gameCanceled || _exitGameFlag ||
        /* extra time / penalties are legal ONLY on a deciding KO match */
        (!c->decides_tie && (_extraTimeState<0 || _penaltiesState<0)) ||
        (_extraTimeState>=0 &&
         (_statsTeam1Goals!=_statsTeam1GoalsCopy ||
          _statsTeam2Goals!=_statsTeam2GoalsCopy)) ||
        (_penaltiesState<0 && _team1PenaltyGoals==_team2PenaltyGoals);
    fprintf(stderr,"[universal-perf] OG_NATIVE_TIMING "
        "event=%llu cpu=%u og_elapsed_ms=%llu natural_end=%u valid=%u\n",
        (unsigned long long)fixture->event_id,(unsigned)!c->interactive,
        (unsigned long long)(Platform_NowMs()-og_started_ms),
        (unsigned)SDL_AtomicGet(&s_universal_match_completed),
        (unsigned)!bad);
    /* Manual ESC abort (not a window close/crash): a clean "not played". */
    c->abandoned=(uint8_t)(bad && c->interactive && _gameCanceled &&
                           !_quitToDos);
    if(bad) {
        fprintf(stderr,"[universal-native] OG_RESULT_REJECTED "
            "cpu=%u completed=%u quit=%u canceled=%u exit=%u "
            "extra=%d penalty=%d live=%u:%u cached=%u:%u "
            "CAR_CLAIMED_NO_REPLAY=1\n",
            (unsigned)!c->interactive,
            (unsigned)SDL_AtomicGet(&s_universal_match_completed),
            (unsigned)_quitToDos,(unsigned)_gameCanceled,
            (unsigned)_exitGameFlag,(int)_extraTimeState,
            (int)_penaltiesState,(unsigned)_statsTeam1Goals,
            (unsigned)_statsTeam2Goals,(unsigned)_statsTeam1GoalsCopy,
            (unsigned)_statsTeam2GoalsCopy);
        if(c->abandoned) {
            fprintf(stderr,"[universal-native] MATCH_ABANDONED event=%llu "
                "OG=BACK_TO_PRE_MATCH_STATE replay-allowed=1\n",
                (unsigned long long)fixture->event_id);
            _gameCanceled=0u; /* do not leak the abort into the menu */
        }
        return 0;
    }
    witness->key=fixture->key;
    witness->home=fixture->home;
    witness->away=fixture->away;
    witness->day_index=fixture->day_index;
    /* total goals incl. extra time; shoot-out separately (OG fields) */
    witness->score=(ExpandedFixtureScore){
        _extraTimeState<0?_statsTeam1Goals:_statsTeam1GoalsCopy,
        _extraTimeState<0?_statsTeam2Goals:_statsTeam2GoalsCopy,
        _penaltiesState<0?_team1PenaltyGoals:0u,
        _penaltiesState<0?_team2PenaltyGoals:0u,
        (uint8_t)((_extraTimeState<0?0x01u:0u)|(_penaltiesState<0?0x02u:0u))};
    witness->valid=1u;
    return 1;
}
#endif
