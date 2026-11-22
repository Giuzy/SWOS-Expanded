/* JSON-first 2026/27 live path, experimental. Only the original OG 50Hz
 * match function produces scores. This scheduler never ticks OG contests.
 * Missing cup entrants, historical results and invalid dates stay OPEN.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_live.h"
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_career_dispatch.h"
#include "expanded_universal_car.h"
#include "expanded_universal_native_match.h"
#include "expanded_world_live.h"
#include "play_match_menu.h"
#include "world.h"
#include <stdio.h>
#include <string.h>
extern TeamFileEx _careerTeam;
extern uint16_t _gameType,_selTeamNumber;

static int manager_key(ExpandedClubKey *out)
{
    const uint16_t word=(uint16_t)(
        ((uint16_t)_careerTeam.team.countryNumber<<8) |
        (uint16_t)_careerTeam.team.teamOrdinal);
    return out && _gameType==GAME_TYPE_CAREER &&
        _selTeamNumber!=0xffffu && _selTeamNumber==word &&
        ExpandedUniversal_NativeTeamKey(&_careerTeam,out);
}
int ExpandedUniversalLive_Active(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    return World_Current()==WORLD_2627 && r &&
           r->world==WORLD_2627 && r->fixture_count>0u;
}
int ExpandedUniversalLive_CanRestoreRosters(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    uint32_t i;
    if(!ExpandedUniversalLive_Active())return 1;
    for(i=0;i<r->fixture_count;++i)
        if(!ExpandedUniversalCatalog_Roster(r->fixtures[i].home) ||
           !ExpandedUniversalCatalog_Roster(r->fixtures[i].away))
            return 0;
    return 1;
}
int ExpandedUniversalLive_Start(void)
{
    ExpandedUniversalCatalogAudit a={0};
    ExpandedUniversalRegistry *r;
    uint32_t i,registered=0u,skipped=0u;
    if(World_Current()!=WORLD_2627)return 1;
    if(!ExpandedUniversalCatalog_Audit(&a) || !a.sealed ||
       !a.future_fixtures) {
        fprintf(stderr,"[universal-live] NO_JSON_FIXTURES fail-closed=1\n");
        return 0;
    }
    ExpandedUniversal_CarResetCurrent(); /* in RAM only, not an old CAR file */
    if(!ExpandedUniversal_CarBegin(
        (ExpandedWorldDate){World_FirstYear(),7u,1u},
        a.future_fixtures))return 0;
    r=ExpandedUniversal_CarRegistry();
    if(!r)return 0;
    for(i=0;i<a.fixtures;++i) {
        const ExpandedUniversalCatalogFixture *f=
            ExpandedUniversalCatalog_Fixture(i);
        if(!f || f->past_at_start || f->date_collision ||
           ExpandedUniversal_Add(r,f->fixture)!=EXP_UNIVERSAL_OK)
            ++skipped;
        else ++registered;
    }
    if(!registered || !ExpandedUniversal_ValidateRestore(r)) {
        ExpandedUniversal_CarResetCurrent();
        fprintf(stderr,"[universal-live] REGISTRY_REJECTED no-CAR-write=1\n");
        return 0;
    }
    fprintf(stderr,"[universal-live] ACTIVE world=2627 "
        "owner=JSON_FIXTURE_REGISTRY OG=MATCH_50HZ_ONLY "
        "registered=%lu skipped=%lu source-complete=%u "
        "cups-missing=%lu historical-unresolved=%lu "
        "day-collisions=%lu roster-restore=NOT_PROVEN\n",
        (unsigned long)registered,(unsigned long)skipped,
        (unsigned)a.source_complete,(unsigned long)a.missing_cup_fixtures,
        (unsigned long)a.prestart_unresolved,
        (unsigned long)a.day_collisions);
    return 1;
}
static ExpandedUniversalResult run_match(
    const ExpandedUniversalFixture *f,int interactive)
{
    ExpandedUniversalNativeMatch ctx={0};
    ExpandedClubKey manager={0};
    const int human=manager_key(&manager);
    const uint16_t game_before=_gameType;
    ExpandedUniversalResult result;
    if(!f)return EXP_UNIVERSAL_INVALID;
    ctx.home=human && ExpandedClubKey_Equal(manager,f->home)?
        &_careerTeam:ExpandedUniversalCatalog_Roster(f->home);
    ctx.away=human && ExpandedClubKey_Equal(manager,f->away)?
        &_careerTeam:ExpandedUniversalCatalog_Roster(f->away);
    if(!ctx.home || !ctx.away)return EXP_UNIVERSAL_RECOVERY;
    ctx.competition_name="LEAGUE";
    ctx.minimum_substitutes=0;
    ctx.maximum_substitutes=5;
    ctx.match_parameters=3;
    ctx.match_postprocess=0;
    ctx.interactive=(uint8_t)(interactive!=0);
    _gameType=GAME_TYPE_CAREER; /* unemployed CPU still uses OG match */
    result=ExpandedUniversal_CareerExecuteNext(f->day_index,&ctx);
    _gameType=game_before;
    fprintf(stderr,"[universal-live] OG_MATCH event=%llu "
        "role=%s result=%d CAR_ONE_WITNESS=1\n",
        (unsigned long long)f->event_id,
        interactive?"MANAGER":"CPU",(int)result);
    return result;
}
int ExpandedUniversalLive_Advance(ExpandedWorldDate target)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedLiveCalendarState state={0};
    int32_t to=0,from=0;
    if(!ExpandedUniversalLive_Active() || !r || r->recovery_required ||
       !ExpandedWorldLive_CalendarState(&state) ||
       !ExpandedWorld_DateToIndex(state.today,&from) ||
       !ExpandedWorld_DateToIndex(target,&to) || to<from ||
       from!=r->scheduler.clock.day_index)return 0;
    for(;;) {
        const ExpandedUniversalFixture *f=NULL;
        ExpandedUniversalResult res=ExpandedUniversal_Peek(r,to,&f);
        ExpandedWorldDate day={0};
        ExpandedClubKey manager={0};
        if(res==EXP_UNIVERSAL_EMPTY) {
            if(!ExpandedWorldLive_AdvanceToDate(target))return 0;
            r->scheduler.clock.day_index=to;
            return 1;
        }
        if(res!=EXP_UNIVERSAL_DUE || !f ||
           f->day_index<r->scheduler.clock.day_index ||
           !ExpandedWorld_IndexToDate(f->day_index,&day)) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-live] ADVANCE_BLOCKED code=%d "
                "no-old-OG-tick=1 no-replay=1\n",(int)res);
            return 0;
        }
        if(!ExpandedWorldLive_AdvanceToDate(day)) {
            r->recovery_required=1u;return 0;
        }
        r->scheduler.clock.day_index=f->day_index;
        if(manager_key(&manager) &&
           (ExpandedClubKey_Equal(manager,f->home) ||
            ExpandedClubKey_Equal(manager,f->away))) {
            fprintf(stderr,"[universal-live] WAIT_MANAGER event=%llu "
                "date=%04u-%02u-%02u one-calendar=1\n",
                (unsigned long long)f->event_id,
                (unsigned)day.year,(unsigned)day.month,(unsigned)day.day);
            return 1;
        }
        if(run_match(f,0)!=EXP_UNIVERSAL_OK) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-live] CPU_FAULT "
                "event=%llu CAR_CLAIMED_NO_REPLAY=1\n",
                (unsigned long long)f->event_id);
            return 0;
        }
    }
}
int ExpandedUniversalLive_PlayManager(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedLiveCalendarState state={0};
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f=NULL;
    int32_t today=0;
    ExpandedUniversalResult res;
    if(!ExpandedUniversalLive_Active())return 0;
    if(!r || r->recovery_required ||
       !ExpandedWorldLive_CalendarState(&state) ||
       !ExpandedWorld_DateToIndex(state.today,&today) ||
       today!=r->scheduler.clock.day_index ||
       !manager_key(&manager)) {
        fprintf(stderr,"[universal-live] MANAGER_BLOCKED no-OG-TSKC=1\n");
        return 1;
    }
    res=ExpandedUniversal_Peek(r,today,&f);
    if(res!=EXP_UNIVERSAL_DUE || !f ||
       (!ExpandedClubKey_Equal(manager,f->home) &&
        !ExpandedClubKey_Equal(manager,f->away))) {
        fprintf(stderr,"[universal-live] MANAGER_NOT_DUE code=%d\n",(int)res);
        return 1;
    }
    res=run_match(f,1);
    if(res!=EXP_UNIVERSAL_OK && res!=EXP_UNIVERSAL_BLOCKED)
        r->recovery_required=1u;
    return 1; /* even cancel never routes to legacy fixture */
}
#endif
