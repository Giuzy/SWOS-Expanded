/* JSON-first 2026/27 live path, experimental. Only original OG
 * scoring produces scores: fast view result or actual 50Hz PLAY.
 * This scheduler never ticks OG contests.
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
/* RAM-only completed quick matches since last explicit batch checkpoint.
 * Accumulates across FORWARD calls; not a persisted journal field. */
static uint32_t s_cpu_batch_pending;

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
/* v282: THE world clock. ExpandedUniversalRegistry.scheduler.clock is the
 * single authority for the civil day in WORLD2627; every reader and the
 * legacy mirror (expanded_world_live.c) take the day from here. */
int ExpandedUniversalLive_TodayIndex(int32_t *out)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    if(!out || !r || !ExpandedUniversalLive_Active())return 0;
    *out=r->scheduler.clock.day_index;
    return 1;
}
int ExpandedUniversalLive_Today(ExpandedWorldDate *out)
{
    int32_t day=0;
    return out && ExpandedUniversalLive_TodayIndex(&day) &&
           ExpandedWorld_IndexToDate(day,out);
}
static int exp_universal_clock_provider(int32_t *day)
{
    return ExpandedUniversalLive_TodayIndex(day);
}
/* Registered before main(): the LWE mirror asks the CE for the day, the CE
 * never calls LWE. Harmless in WORLD9697 (provider answers 0 there). */
__attribute__((constructor)) static void exp_universal_register_clock(void)
{
    ExpandedWorldLive_SetClockProvider(exp_universal_clock_provider);
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
    s_cpu_batch_pending=0u; /* new career, never inherit another world */
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
        "owner=JSON_FIXTURE_REGISTRY OG=QUICK_CPU_AND_50HZ_PLAY "
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
    const ExpandedUniversalFixture *f,int interactive,int cpu_batch)
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
    /* The original quick scorer runs identically in both modes. Only
     * unattended CPU/debug matches may defer CAR until block commit. */
    result=cpu_batch?
        ExpandedUniversal_CareerExecuteNextCpuBatch(f->day_index,&ctx):
        ExpandedUniversal_CareerExecuteNext(f->day_index,&ctx);
    _gameType=game_before;
    fprintf(stderr,"[universal-live] OG_MATCH event=%llu "
        "role=%s result=%d CAR_ONE_WITNESS=1\n",
        (unsigned long long)f->event_id,
        interactive?"MANAGER":"CPU",(int)result);
    return result;
}
/* One full-world atomic CAR for a set of already-completed OG quick games.
 * Nothing in an unfinished block is durable; a process crash restarts the
 * block from the previous CAR, without duplicating a durable result. */
static int exp_live_flush_cpu(ExpandedUniversalRegistry *r)
{
    if(!r)return 0;
    if(!s_cpu_batch_pending)return 1;
    if(!ExpandedUniversal_CarSaveCpuBatch(s_cpu_batch_pending)) {
        r->recovery_required=1u;
        fprintf(stderr,"[universal-live] CPU_BATCH_COMMIT_FAILED "
            "recovery=1 previous-CAR-preserved=1\n");
        return 0;
    }
    s_cpu_batch_pending=0u;
    return 1;
}
/* A loaded CAR is already a durable snapshot: transient CPU count resets. */
void ExpandedUniversalLive_ResetCpuBatch(void)
{
    s_cpu_batch_pending=0u;
}
/* OG CAREER board round: the manager's league fixtures of the same edition
 * on the same civil day. OG shows every one of them in its own tabellone
 * (CPU ones with VIEW RESULT), so ordinary FORWARD must stop BEFORE the
 * first of them, not only before the manager's own fixture. Cups keep the
 * previous manager-fixture-only stop (no native CE bracket yet). */
static int exp_live_same_round(const ExpandedUniversalFixture *f,
                               const ExpandedUniversalFixture *mf)
{
    return f && mf && f->competition_type==0u && mf->competition_type==0u &&
           f->day_index==mf->day_index &&
           ExpandedEditionKey_Equal(f->key.edition,mf->key.edition);
}
static const ExpandedUniversalFixture *exp_live_manager_fixture(
    ExpandedUniversalRegistry *r,ExpandedClubKey manager)
{
    const ExpandedUniversalFixture *mf=NULL;
    if(!r || !r->scheduler.count)return NULL;
    if(ExpandedUniversal_NextForClub(r,manager,
           r->scheduler.events[r->scheduler.count-1u].day_index,&mf)!=
       EXP_UNIVERSAL_DUE)return NULL;
    return mf;
}
/* Runs, in the ONE CE chronological order, the pending events of TODAY
 * up to stop_event (included or not). Unattended fixtures only: original
 * OG quick scorer, same RAM CPU batch as FORWARD. Never runs a manager
 * fixture and never moves the civil clock. 2 reached, 1 blocked. */
static int exp_live_drain_today(ExpandedUniversalRegistry *r,int32_t today,
                                uint64_t stop_event,int inclusive,
                                ExpandedClubKey manager)
{
    for(;;) {
        const ExpandedUniversalFixture *g=NULL;
        ExpandedUniversalResult res=ExpandedUniversal_Peek(r,today,&g);
        if(res!=EXP_UNIVERSAL_DUE || !g || g->day_index!=today)return 1;
        if(g->event_id==stop_event && !inclusive)return 2;
        if(ExpandedClubKey_Equal(manager,g->home) ||
           ExpandedClubKey_Equal(manager,g->away))return 1;
        if(_quitToDos)return 1;
        res=run_match(g,0,1);
        if(res!=EXP_UNIVERSAL_OK) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-live] ROUND_CPU_FAULT event=%llu "
                "CAR_CLAIMED_NO_REPLAY=1\n",(unsigned long long)g->event_id);
            return 1;
        }
        if(s_cpu_batch_pending<UINT32_MAX)++s_cpu_batch_pending;
        if(g->event_id==stop_event)return 2;
    }
}
static int exp_universal_live_advance_impl(ExpandedWorldDate target,
                                           int debug_autosim_manager)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    int32_t to=0,from=0;
    uint32_t cpu_batch_count=0u;
    /* The CE clock is the only world clock (v282): no LWE state is read. */
    if(!ExpandedUniversalLive_Active() || !r || r->recovery_required ||
       !ExpandedUniversalLive_TodayIndex(&from) ||
       !ExpandedWorld_DateToIndex(target,&to) || to<from)return 0;
    /* Next FORWARD after at least 512 completed quick results saves the
     * whole prior RAM block, possibly larger than 512. This is NOT a
     * checkpoint at the end of the previous FORWARD. */
    if(s_cpu_batch_pending>=512u && !exp_live_flush_cpu(r))return 0;
    /* The manager fixture is never executed by ordinary FORWARD, so its
     * round anchor cannot change inside this call. DEBUG keeps old path. */
    const ExpandedUniversalFixture *round_anchor=NULL;
    {
        ExpandedClubKey round_manager={0};
        if(!debug_autosim_manager && manager_key(&round_manager))
            round_anchor=exp_live_manager_fixture(r,round_manager);
    }
    for(;;) {
        const ExpandedUniversalFixture *f=NULL;
        ExpandedUniversalResult res=ExpandedUniversal_Peek(r,to,&f);
        ExpandedWorldDate day={0};
        ExpandedClubKey manager={0};
        if(res==EXP_UNIVERSAL_EMPTY) {
            r->scheduler.clock.day_index=to; /* CE clock only */
            /* A FORWARD stop is not an autosave. Quick CPU scores below the
             * batch limit stay in RAM until a later durable checkpoint. */
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
        r->scheduler.clock.day_index=f->day_index; /* CE clock only */
        const int manager_due=manager_key(&manager) &&
            (ExpandedClubKey_Equal(manager,f->home) ||
             ExpandedClubKey_Equal(manager,f->away));
        if((manager_due || exp_live_same_round(f,round_anchor)) &&
           !debug_autosim_manager) {
            fprintf(stderr,"[universal-live] WAIT_MANAGER event=%llu "
                "date=%04u-%02u-%02u role=%s one-calendar=1\n",
                (unsigned long long)f->event_id,
                (unsigned)day.year,(unsigned)day.month,(unsigned)day.day,
                manager_due?"MANAGER_FIXTURE":"OG_BOARD_ROUND_CPU");
            return 1;
        }
        /* A Windows close during a long CPU batch must not start/CLAIM
         * the NEXT fixture after the quit request was already observed. */
        if(_quitToDos) {
            if(!exp_live_flush_cpu(r))return 0;
            fprintf(stderr,"[universal-live] CPU_BATCH_EXIT_REQUESTED "
                "before-next-claim=1 preserved-CAR=1 no-replay=1\n");
            return 0;
        }
        {
            const Uint64 started_ms=Platform_NowMs();
            ExpandedUniversalResult match_result;
            /* No partial civil advancement or new scorer: progress while
             * the original OG quick VIEW RESULT calculates this fixture. */
            Platform_UniversalCpuProgress(++cpu_batch_count);
            match_result=run_match(f,0,1); /* original OG quick, RAM batch */
            Platform_UniversalCpuProgress(0);
            fprintf(stderr,"[universal-live] CPU_MATCH_TIMING "
                "ordinal=%lu event=%llu role=%s elapsed_ms=%llu "
                "outcome=%d og-quick=1 srp-background=NOT_NEEDED\n",
                (unsigned long)cpu_batch_count,
                (unsigned long long)f->event_id,
                manager_due?"DEBUG_MANAGER":"CPU",
                (unsigned long long)(Platform_NowMs()-started_ms),
                (int)match_result);
            if(match_result!=EXP_UNIVERSAL_OK) {
                /* Do not persist an uncertain partial block; preceding
                 * atomic CAR is still the last valid checkpoint. */
                r->recovery_required=1u;
                fprintf(stderr,"[universal-live] CPU_FAULT "
                    "event=%llu CAR_CLAIMED_NO_REPLAY=1\n",
                    (unsigned long long)f->event_id);
                return 0;
            }
            if(s_cpu_batch_pending<UINT32_MAX)++s_cpu_batch_pending;
        }
    }
}
/* Measure the COMPLETE civil FORWARD call, including zero-event windows,
 * OG CPU batches, all durable CAR checkpoints, and fail-closed exits. No
 * new clock or result writer: the old return paths remain inside _impl. */
int ExpandedUniversalLive_Advance(ExpandedWorldDate target)
{
    const Uint64 started_ms=Platform_NowMs();
    const int ok=exp_universal_live_advance_impl(target,0);
    fprintf(stderr,"[universal-perf] FORWARD_TOTAL "
        "target=%04u-%02u-%02u elapsed_ms=%llu status=%s "
        "owner=JSON_ENGINE no-old-OG-tick=1\n",
        (unsigned)target.year,(unsigned)target.month,(unsigned)target.day,
        (unsigned long long)(Platform_NowMs()-started_ms),
        ok?"ADVANCED_OR_WAIT_MANAGER":"BLOCKED");
    return ok;
}
/* Next manager fixture is a readonly projection of the same CE ledger.
 * 2=ready, 1=advance (including same-day CPU events), 0=unknown, -1=error.
 * There is no independent calendar clock and no score writer here. */
int ExpandedUniversalLive_ManagerNext(ExpandedWorldDate *due)
{
    ExpandedUniversalRegistry *const r=ExpandedUniversal_CarRegistry();
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f=NULL,*world_due=NULL;
    int32_t today=0;
    ExpandedUniversalResult res;
    if(due)memset(due,0,sizeof *due);
    if(!due || !ExpandedUniversalLive_Active() || !r ||
       r->recovery_required || !manager_key(&manager) ||
       !ExpandedUniversalLive_TodayIndex(&today) ||
       !r->scheduler.count)return -1;
    /* Last chronological event bounds available source fixtures. */
    res=ExpandedUniversal_NextForClub(r,manager,
        r->scheduler.events[r->scheduler.count-1u].day_index,&f);
    if(res==EXP_UNIVERSAL_EMPTY)return 0;
    if(res!=EXP_UNIVERSAL_DUE || !f ||
       f->day_index<today ||
       !ExpandedWorld_IndexToDate(f->day_index,due))return -1;
    if(f->day_index>today)return 1;
    /* Earlier CPU events on the same day must complete before PLAY,
     * except the manager's own league round: OG shows those in its board. */
    res=ExpandedUniversal_Peek(r,today,&world_due);
    if(res!=EXP_UNIVERSAL_DUE || !world_due)return -1;
    return (world_due->event_id==f->event_id ||
            exp_live_same_round(world_due,f))?2:1;
}

static int exp_universal_live_manager_action(int interactive)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f=NULL;
    int32_t today=0;
    ExpandedUniversalResult res;
    if(!ExpandedUniversalLive_Active())return 0;
    if(!r || r->recovery_required ||
       !ExpandedUniversalLive_TodayIndex(&today) ||
       !manager_key(&manager)) {
        fprintf(stderr,"[universal-live] MANAGER_BLOCKED no-OG-TSKC=1\n");
        return 1;
    }
    {
        /* Same-day events ordered before the manager fixture (other
         * leagues, or round CPU fixtures not shown in the board) run first
         * in the one CE order, exactly like FORWARD would run them. */
        const ExpandedUniversalFixture *mf=exp_live_manager_fixture(r,manager);
        if(mf && mf->day_index==today &&
           ExpandedUniversal_Peek(r,today,&f)==EXP_UNIVERSAL_DUE &&
           f && f->event_id!=mf->event_id)
            (void)exp_live_drain_today(r,today,mf->event_id,0,manager);
        f=NULL;
    }
    res=ExpandedUniversal_Peek(r,today,&f);
    if(res!=EXP_UNIVERSAL_DUE || !f ||
       (!ExpandedClubKey_Equal(manager,f->home) &&
        !ExpandedClubKey_Equal(manager,f->away))) {
        fprintf(stderr,"[universal-live] MANAGER_NOT_DUE code=%d\n",(int)res);
        return 1;
    }
    res=run_match(f,interactive,0);
    /* This per-fixture durable completion also commits preceding CPU
     * results from RAM; avoid a redundant threshold save on next FORWARD. */
    if(res==EXP_UNIVERSAL_OK)s_cpu_batch_pending=0u;
    fprintf(stderr,"[universal-live] MANAGER_ACTION mode=%s "
        "event=%llu result=%d CAR_ONE_FIXTURE=1\n",
        interactive?"PLAY_50HZ":"VIEW_RESULT_QUICK",
        (unsigned long long)f->event_id,(int)res);
    if(res!=EXP_UNIVERSAL_OK && res!=EXP_UNIVERSAL_BLOCKED)
        r->recovery_required=1u;
    return res==EXP_UNIVERSAL_OK?2:1; /* no legacy fixture */
}
/* Only the explicit career DEBUG advance is permitted to quick-sim
 * a manager's fixture. Ordinary FORWARD always stops BEFORE that fixture. */
int ExpandedUniversalLive_DebugAdvance(ExpandedWorldDate target)
{
    const Uint64 started_ms=Platform_NowMs();
    const int ok=exp_universal_live_advance_impl(target,1);
    fprintf(stderr,"[universal-perf] DEBUG_ADVANCE_TOTAL "
        "target=%04u-%02u-%02u elapsed_ms=%llu status=%s "
        "manager=OG_QUICK no-legacy-writer=1\n",
        (unsigned)target.year,(unsigned)target.month,(unsigned)target.day,
        (unsigned long long)(Platform_NowMs()-started_ms),
        ok?"COMPLETED":"BLOCKED");
    return ok;
}
int ExpandedUniversalLive_PlayManager(void)
{
    return exp_universal_live_manager_action(1);
}
int ExpandedUniversalLive_ViewManagerResult(void)
{
    int rc;
    /* OG: RESULT opens the team screen; VIEW RESULT there = OG quick. */
    ExpandedUniversal_NativeSetResultPrompt(1);
    rc=exp_universal_live_manager_action(0);
    ExpandedUniversal_NativeSetResultPrompt(0);
    return rc;
}
/* OG board VIEW RESULT on a CPU fixture of the manager's round. */
int ExpandedUniversalLive_ViewRoundCpuResult(uint64_t event_id)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f;
    ExpandedFixtureLedgerEntry *entry;
    int32_t today=0;
    int rc;
    if(!ExpandedUniversalLive_Active())return 0;
    if(!r || r->recovery_required ||
       !ExpandedUniversalLive_TodayIndex(&today) ||
       !manager_key(&manager) ||
       !(f=ExpandedUniversal_Find(r,event_id)) || f->day_index!=today ||
       ExpandedClubKey_Equal(manager,f->home) ||
       ExpandedClubKey_Equal(manager,f->away) ||
       !(entry=ExpandedUniversal_EntryFor(r,f)) ||
       entry->state!=EXP_FIXTURE_SCHEDULED) {
        fprintf(stderr,"[universal-live] ROUND_CPU_BLOCKED event=%llu\n",
            (unsigned long long)event_id);
        return 1;
    }
    rc=exp_live_drain_today(r,today,event_id,1,manager);
    fprintf(stderr,"[universal-live] ROUND_CPU_VIEW_RESULT event=%llu "
        "result=%d og-quick=1 batch-pending=%lu\n",
        (unsigned long long)event_id,rc,(unsigned long)s_cpu_batch_pending);
    return rc;
}
#endif
