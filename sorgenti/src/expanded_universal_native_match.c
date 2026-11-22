/* SWOS Expanded universal career: ONLY the original 50 Hz match engine.
 * No old TSKC, no DIY league/cup writer, no standings or CAR writes here.
 * A caller must provide persistent live rosters and durable fixture claim.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_native_match.h"
#include "game_init.h"
#include "play_match_menu.h" /* GAME_TYPE_CAREER from existing OG header */
#include "world.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h> /* timer/main thread scope published atomically */

extern uint16_t _gameType, _exitGameFlag, _gameCanceled, _isCareer;
extern uint16_t _gamePitchTypeOrSeason, _gameSeason, _gamePitchType;
extern uint16_t _team1GoalsFirstLeg, _team2GoalsFirstLeg;
extern int16_t _extraTimeState, _penaltiesState;
extern uint16_t _secondLeg, _playing2ndGame, _isGameFriendly;
extern uint16_t _statsTeam1GoalsCopy, _statsTeam2GoalsCopy;
extern uint16_t _statsTeam1Goals, _statsTeam2Goals;
extern uint16_t _team1PenaltyGoals, _team2PenaltyGoals;
extern uint8_t _gameName[], _gameRound;
extern TeamFileEx _careerTeam;
extern uint8_t *StringCopy(const uint8_t *,uint8_t *);

/* Only the universal dispatcher sets this scope, around its original
 * OG 50 Hz game. Legacy match/replay/other worlds never use it. */
static SDL_atomic_t s_universal_cpu_match;
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
int ExpandedUniversal_NativePrepare(
    ExpandedUniversalNativeMatch *context,
    const ExpandedUniversalFixture *fixture)
{
    if(!context)return 0;
    context->lineup_ready=0u;
    if(!exp_universal_native_match_valid(context,fixture))return 0;
    if(context->interactive) {
        _exitGameFlag=0;
        SetupPlayers(context->home,context->away,
                     context->maximum_substitutes,context->match_parameters,
                     1,0);
        if(_exitGameFlag)return 0;
    }
    context->lineup_ready=1u;
    return 1;
}
int ExpandedUniversal_NativeRunOriginal(
    void *user,const ExpandedUniversalFixture *fixture,
    ExpandedUniversalMatchWitness *witness)
{
    ExpandedUniversalNativeMatch *const c=(ExpandedUniversalNativeMatch *)user;
    uint16_t oldCareer;
    uint16_t homeControls,awayControls;
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
        /* Prevent game-input ownership by the user on CPU-only fixtures.
         * Restore the persistent roster controls after OG match effects. */
        c->home->team.teamControls=TEAM_COMPUTER;
        c->away->team.teamControls=TEAM_COMPUTER;
        SDL_AtomicSet(&s_universal_cpu_match,1);
    }
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
    /* True OpenSWOS 50Hz/AI input and post-match player effects.
     * No old DIY competition progression callback is called. */
    InitializeInGameTeamsAndStartGame(0,c->minimum_substitutes,
        c->maximum_substitutes,c->match_parameters,c->match_postprocess,
        (TeamFile *)c->home,(TeamFile *)c->away);
    /* Restore the caller's original controls even after an OG failure;
     * every claimed match still remains non-replayable on uncertain score. */
    SDL_AtomicSet(&s_universal_cpu_match,0);
    c->home->team.teamControls=homeControls;
    c->away->team.teamControls=awayControls;
    _isCareer=oldCareer;
    bad=_gameCanceled || _exitGameFlag || _extraTimeState<0 ||
        _penaltiesState<0 ||
        _statsTeam1Goals!=_statsTeam1GoalsCopy ||
        _statsTeam2Goals!=_statsTeam2GoalsCopy;
    if(bad) {
        fprintf(stderr,"[universal-native] OG_RESULT_REJECTED "
            "cpu=%u canceled=%u exit=%u extra=%d penalty=%d "
            "live=%u:%u cached=%u:%u CAR_CLAIMED_NO_REPLAY=1\n",
            (unsigned)!c->interactive,(unsigned)_gameCanceled,
            (unsigned)_exitGameFlag,(int)_extraTimeState,
            (int)_penaltiesState,(unsigned)_statsTeam1Goals,
            (unsigned)_statsTeam2Goals,(unsigned)_statsTeam1GoalsCopy,
            (unsigned)_statsTeam2GoalsCopy);
        return 0;
    }
    witness->key=fixture->key;
    witness->home=fixture->home;
    witness->away=fixture->away;
    witness->day_index=fixture->day_index;
    witness->score=(ExpandedFixtureScore){
        _statsTeam1GoalsCopy,_statsTeam2GoalsCopy,0u,0u,0u};
    witness->valid=1u;
    return 1;
}
#endif
