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
#include "platform_sdl.h" /* CPU batch progress + monotonic diagnostics only */
#include "play_match_menu.h"
#include "world.h"
extern uint16_t _quitToDos; /* SDL close: do not durably claim another match */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
/* Source fixtures may be grouped by country/league. Install once with
 * indexed validation instead of O(N²) per-fixture Add/Schedule scans. */
static int exp_catalog_event_cmp(const void *aa,const void *bb)
{
    const ExpandedUniversalCatalogFixture *const a=
        *(const ExpandedUniversalCatalogFixture *const *)aa;
    const ExpandedUniversalCatalogFixture *const b=
        *(const ExpandedUniversalCatalogFixture *const *)bb;
    return a->fixture.event_id<b->fixture.event_id?-1:
           a->fixture.event_id>b->fixture.event_id?1:0;
}
static int exp_event_chrono_cmp(const void *aa,const void *bb)
{
    const ExpandedScheduledEvent *a=(const ExpandedScheduledEvent *)aa,
        *b=(const ExpandedScheduledEvent *)bb;
    return ExpandedEvent_Before(a,b)?-1:ExpandedEvent_Before(b,a)?1:0;
}
int ExpandedUniversalLive_Start(void)
{
    ExpandedUniversalCatalogAudit a={0};
    ExpandedUniversalRegistry *r;
    uint32_t i,registered=0u,skipped=0u;
    const ExpandedUniversalCatalogFixture **ordered=NULL;
    const Uint64 start_ms=Platform_NowMs();
    Uint64 validated_ms;
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
#if SIZE_MAX < UINT64_MAX
    if(a.fixtures && (size_t)a.fixtures>
       SIZE_MAX/sizeof *ordered) {
        ExpandedUniversal_CarResetCurrent();
        return 0;
    }
#endif
    ordered=(const ExpandedUniversalCatalogFixture **)malloc(
        (size_t)(a.fixtures?a.fixtures:1u)*sizeof *ordered);
    if(!ordered) {
        ExpandedUniversal_CarResetCurrent();
        return 0;
    }
    for(i=0;i<a.fixtures;++i) {
        const ExpandedUniversalCatalogFixture *f=
            ExpandedUniversalCatalog_Fixture(i);
        if(!f || f->past_at_start || f->date_collision) {
            ++skipped;
            continue;
        }
        if(registered>=r->fixture_capacity) {
            free(ordered);ExpandedUniversal_CarResetCurrent();return 0;
        }
        ordered[registered++]=f;
    }
    qsort(ordered,(size_t)registered,sizeof *ordered,exp_catalog_event_cmp);
    for(i=0;i<registered;++i) {
        const ExpandedUniversalFixture *f=&ordered[i]->fixture;
        ExpandedFixtureLedgerEntry *claim=&r->ledger.entries[i];
        ExpandedScheduledEvent *event=&r->scheduler.events[i];
        r->fixtures[i]=*f;
        memset(claim,0,sizeof *claim);
        claim->key=f->key;
        claim->state=EXP_FIXTURE_SCHEDULED;
        *event=(ExpandedScheduledEvent){
            f->event_id,0u,f->day_index,10u,EXP_EVENT_FIXTURE,
            EXP_EVENT_PENDING};
    }
    free(ordered);ordered=NULL;
    r->fixture_count=registered;
    r->ledger.count=registered;
    r->scheduler.count=registered;
    r->fixtures_sorted_by_event=1u;
    r->pending_hint=0u;
    qsort(r->scheduler.events,(size_t)registered,
          sizeof *r->scheduler.events,exp_event_chrono_cmp);
    if(!registered || !ExpandedUniversal_ValidateRestore(r)) {
        ExpandedUniversal_CarResetCurrent();
        fprintf(stderr,"[universal-live] REGISTRY_REJECTED no-CAR-write=1\n");
        return 0;
    }
    validated_ms=Platform_NowMs();
    fprintf(stderr,"[universal-perf] START_REGISTRY register=%lu "
        "sorted-validated-ms=%llu no-old-OG=1\n",
        (unsigned long)registered,
        (unsigned long long)(validated_ms-start_ms));
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
static int exp_universal_live_advance_impl(ExpandedWorldDate target)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedLiveCalendarState state={0};
    int32_t to=0,from=0;
    uint32_t cpu_batch_count=0u;
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
        /* A Windows close during a long CPU batch must not start/CLAIM
         * the NEXT fixture after the quit request was already observed. */
        if(_quitToDos) {
            fprintf(stderr,"[universal-live] CPU_BATCH_EXIT_REQUESTED "
                "before-next-claim=1 preserved-CAR=1 no-replay=1\n");
            return 0;
        }
        {
            const Uint64 started_ms=Platform_NowMs();
            ExpandedUniversalResult match_result;
            /* No partial civil advancement or new scorer: show OS-level
             * progress while this fixture's original OG 50 Hz runs. */
            Platform_UniversalCpuProgress(++cpu_batch_count);
            match_result=run_match(f,0);
            Platform_UniversalCpuProgress(0);
            fprintf(stderr,"[universal-live] CPU_MATCH_TIMING "
                "ordinal=%lu event=%llu elapsed_ms=%llu outcome=%d "
                "og-50hz=1 srp-background=OFF\n",
                (unsigned long)cpu_batch_count,
                (unsigned long long)f->event_id,
                (unsigned long long)(Platform_NowMs()-started_ms),
                (int)match_result);
            if(match_result!=EXP_UNIVERSAL_OK) {
                r->recovery_required=1u;
                fprintf(stderr,"[universal-live] CPU_FAULT "
                    "event=%llu CAR_CLAIMED_NO_REPLAY=1\n",
                    (unsigned long long)f->event_id);
                return 0;
            }
        }
    }
}
/* Measure the COMPLETE civil FORWARD call, including zero-event windows,
 * OG CPU batches, all durable CAR checkpoints, and fail-closed exits. No
 * new clock or result writer: the old return paths remain inside _impl. */
int ExpandedUniversalLive_Advance(ExpandedWorldDate target)
{
    const Uint64 started_ms=Platform_NowMs();
    const int ok=exp_universal_live_advance_impl(target);
    fprintf(stderr,"[universal-perf] FORWARD_TOTAL "
        "target=%04u-%02u-%02u elapsed_ms=%llu status=%s "
        "owner=JSON_ENGINE no-old-OG-tick=1\n",
        (unsigned)target.year,(unsigned)target.month,(unsigned)target.day,
        (unsigned long long)(Platform_NowMs()-started_ms),
        ok?"ADVANCED_OR_WAIT_MANAGER":"BLOCKED");
    return ok;
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
