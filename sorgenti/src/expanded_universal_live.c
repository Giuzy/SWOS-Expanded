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
#include "expanded_universal_json_leagues.h"
#include "expanded_ce_engine.h" /* Fase 2: rules, lifecycle, editions */
#include "expanded_fifa_ranking.h"
#include "expanded_ce_discipline.h"
#include "swos_io.h"
#include "car_writer.h"
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
extern uint16_t _youreSacked; /* sacked = unemployed for every OG reader */
/* RAM-only completed quick matches since last explicit batch checkpoint.
 * Accumulates across FORWARD calls; not a persisted journal field. */
static uint32_t s_cpu_batch_pending;
/* v292: FNV-1a/64 chain over (event id, score) of the RAM-only quick results
 * since the last durable checkpoint. Logged at each FORWARD end so a controlled
 * crash + CONTINUE + same FORWARD can be compared line by line: identical
 * digest at the same clock day = deterministic backlog recomputation. */
static uint64_t s_backlog_digest;
static void exp_backlog_note(const ExpandedUniversalRegistry *r,
                             const ExpandedUniversalFixture *g)
{
    const ExpandedFixtureLedgerEntry *e=
        ExpandedUniversal_EntryFor((ExpandedUniversalRegistry *)r,g);
    uint64_t h=s_backlog_digest?s_backlog_digest:UINT64_C(14695981039346656037);
    uint64_t v[2];
    unsigned i;
    v[0]=g->event_id;
    v[1]=e?((uint64_t)e->score.home<<48)|((uint64_t)e->score.away<<32)|
          ((uint64_t)e->score.penalties_home<<16)|e->score.penalties_away:0u;
    for(i=0u;i<16u;++i){h^=((const uint8_t *)v)[i];h*=UINT64_C(1099511628211);}
    s_backlog_digest=h;
}
/* RAM-only completed quick results that force a full CAR at the next
 * FORWARD (see exp_universal_live_advance_impl). */
#define EXP_CPU_BATCH_SAVE_THRESHOLD 4000u
/* A DIAGNOSTIC session (AVVIA_DIAGNOSTICA.bat sets SWOS_DIAGNOSTIC_SESSION)
 * checkpoints every 1000 quick results so the short scripted runs reach the
 * automatic snapshot activation AND real mode=ASYNC writes. Same code path,
 * same results; only the checkpoint cadence differs. */
static uint32_t exp_cpu_threshold(void)
{
    static int cached=-1;
    if(cached<0)cached=getenv("SWOS_DIAGNOSTIC_SESSION")!=NULL;
    return cached?1000u:EXP_CPU_BATCH_SAVE_THRESHOLD;
}
static int s_async_failed; /* last async CAR write failed: next flush is SYNC */

/* Fase 1: the ONLY test of "the manager owns this fixture". A sacked
 * manager keeps _selTeamNumber/_careerTeam of the old club until a new
 * contract (InitCareerClub clears _youreSacked), exactly like every other
 * OG reader treats `_selTeamNumber==0xffff || _youreSacked` as unemployed.
 * Without this check FORWARD after a dismissal stopped on the OLD club's
 * fixture (WAIT_MANAGER) and PLAY MATCH could offer it (RG FORWARD da
 * disoccupato / partita manager assente). */
static int manager_key(ExpandedClubKey *out)
{
    /* v299: a national team manager (OG _isNationalSelector, one job at a
     * time) is the NATIONAL participant of its real OG team number. Same
     * gates as a club manager; NativeTeamKey maps region files 80..85. */
    const uint16_t word=(uint16_t)(
        ((uint16_t)_careerTeam.team.countryNumber<<8) |
        (uint16_t)_careerTeam.team.teamOrdinal);
    return out && _gameType==GAME_TYPE_CAREER && !_youreSacked &&
        _selTeamNumber!=0xffffu && _selTeamNumber==word &&
        ExpandedUniversal_NativeTeamKey(&_careerTeam,out);
}
static const char *exp_live_fixture_state(ExpandedUniversalRegistry *r,
                                          const ExpandedUniversalFixture *f)
{
    const ExpandedFixtureLedgerEntry *e=r && f?ExpandedUniversal_EntryFor(r,f):NULL;
    if(!e)return "NO_LEDGER";
    return e->state==EXP_FIXTURE_SCHEDULED?"SCHEDULED":
           e->state==EXP_FIXTURE_CLAIMED?"CLAIMED":
           e->state==EXP_FIXTURE_PLAYED?"COMPLETED":"UNKNOWN";
}
/* Fase 1 diagnostics: every fault names world, country, competition,
 * edition, stage, date, fixture and clubs, the fixture state before the
 * attempt and after it, the result code and whether the cause is V/I. */
static void exp_live_fault(const char *tag,ExpandedUniversalRegistry *r,
                           const ExpandedUniversalFixture *f,
                           const char *before,int code,const char *cause)
{
    ExpandedWorldDate d={0};
    if(!f) {
        fprintf(stderr,"[universal-fault] %s world=%u fixture=NONE code=%d "
            "clock=%ld recovery=%u cause=%s\n",tag?tag:"FAULT",
            r?(unsigned)r->world:0u,code,
            r?(long)r->scheduler.clock.day_index:-1L,
            r?(unsigned)r->recovery_required:0u,cause?cause:"?");
        return;
    }
    (void)ExpandedWorld_IndexToDate(f->day_index,&d);
    fprintf(stderr,"[universal-fault] %s world=%u country=%u slot=%u "
        "type=%u competition=%lu edition=%ld/%u stage=%u tie=%u leg=%u "
        "date=%04u-%02u-%02u event=%llu home=%lu away=%lu "
        "state-before=%s state-after=%s code=%d recovery=%u cause=%s\n",
        tag?tag:"FAULT",(unsigned)f->key.edition.competition.world,
        (unsigned)f->country,(unsigned)f->slot,
        (unsigned)f->competition_type,
        (unsigned long)f->key.edition.competition.stable_competition,
        (long)f->key.edition.start_day,(unsigned)f->key.edition.sequence,
        (unsigned)f->key.stage,(unsigned)f->key.tie,(unsigned)f->key.leg,
        (unsigned)d.year,(unsigned)d.month,(unsigned)d.day,
        (unsigned long long)f->event_id,
        (unsigned long)f->home.stable_club,(unsigned long)f->away.stable_club,
        before?before:"?",exp_live_fixture_state(r,f),code,
        r?(unsigned)r->recovery_required:0u,cause?cause:"?");
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
/* Explicit registration (v283, replaces the v282 GCC constructor): called
 * from the two places that make a CE registry active, ExpandedUniversal_
 * CarBegin (new career) and ExpandedUniversal_CarRestore (CONTINUE). The
 * LWE mirror asks the CE for the day; the CE never calls LWE. Idempotent;
 * before any registry exists, and in WORLD9697, the provider answers 0. */
void ExpandedUniversalLive_RegisterClock(void)
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

/* ===================== Fase 2: Competition Engine glue ===================== */
int ExpandedUniversalLive_ClubNameCompare(void *user,ExpandedClubKey a,
                                          ExpandedClubKey b)
{
    /* OG native tie-break: alphabetical club name (DIY_SortLeagueTable) */
    const TeamFileEx *ta=ExpandedUniversalCatalog_Roster(a);
    const TeamFileEx *tb=ExpandedUniversalCatalog_Roster(b);
    (void)user;
    if(ta && tb) {
        const int c=strncmp((const char *)ta->team.teamName,
                            (const char *)tb->team.teamName,
                            sizeof ta->team.teamName);
        if(c)return c;
    }
    return a.stable_club<b.stable_club?-1:a.stable_club>b.stable_club?1:0;
}
static int ce_host_insert(void *u,const ExpandedUniversalFixture *f,uint32_t n)
{(void)u;return ExpandedUniversal_CarInsertFixtures(f,n);}
static int ce_host_prune(void *u,ExpandedEditionKey e)
{(void)u;return ExpandedUniversal_CarPruneEdition(e);}
static int ce_host_hold(void *u,ExpandedEditionKey e,int h)
{(void)u;return ExpandedUniversal_CarHoldEdition(e,h);}
static int ce_host_resched(void *u,uint64_t id,int32_t day)
{(void)u;return ExpandedUniversal_CarRescheduleEvent(id,day);}
static int ce_host_archive(void *u,ExpandedEditionKey e,uint32_t c,uint32_t r,
                           int32_t day)
{(void)u;return ExpandedUniversal_ArchiveCloseEdition(e,c,r,day);}
static int ce_host_club_ok(void *u,ExpandedClubKey k)
{(void)u;return ExpandedUniversalCatalog_Roster(k)!=NULL;}
static uint32_t ce_host_division_clubs(void *u,uint8_t country,uint8_t division,
                                       ExpandedClubKey *out,uint32_t cap)
{(void)u;return ExpandedUniversalCatalog_DivisionClubs(country,division,out,cap);}
static const ExpCeHost s_ce_host={ce_host_insert,ce_host_prune,ce_host_hold,
    ce_host_resched,ce_host_archive,ce_host_club_ok,
    ExpandedUniversalLive_ClubNameCompare,NULL,ce_host_division_clubs};
const ExpCeHost *ExpandedUniversalLive_EngineHost(void){return &s_ce_host;}

/* Definitions: shared builder (expanded_ce_definitions.c), same at START
 * and CONTINUE, also used by the data validator test. */
static int exp_live_engine_definitions(void)
{
    /* a malformed ce_competitions.json is logged by the builder and leaves
     * only the league definitions: leagues keep their lifecycle */
    return ExpCe_BuildWorldDefinitions((uint16_t)World_Current())!=-2;
}
/* FIFA ranking seed (2627 only): the user-approved 1 July 2026 projection. */
static int exp_live_fifa_seed(int32_t start_day)
{
    char path[1024],*buf=NULL;
    long len;
    int ok=0;
    FILE *f;
    ExpFifa_Reset();
    if(World_Current()!=WORLD_2627)return 1;
    f=fopen(swos_data_path("data_json/worlds/2627/rankings/fifa_nations.json",
                           path,(uint32_t)sizeof path),"rb");
    if(f) {
        if(fseek(f,0,SEEK_END)==0 && (len=ftell(f))>0 && len<0x7fffffffL &&
           fseek(f,0,SEEK_SET)==0 && (buf=(char *)malloc((size_t)len+1u))!=NULL &&
           fread(buf,1,(size_t)len,f)==(size_t)len) {
            buf[len]=0;ok=ExpFifa_LoadSeedJson(buf,(int)len,start_day);
        }
        free(buf);fclose(f);
    }
    fprintf(stderr,"[fifa-ranking] SEED %s nations=%lu week=%ld\n",ok?"LOADED":"MISSING",
        (unsigned long)ExpFifa_Count(),(long)ExpFifa_WeekId());
    return ok;
}
/* Old CAR without a ranking: seed + every PERSISTED eligible result, in
 * event order (no invented match, no silent seed for an advanced career). */
static uint32_t exp_live_fifa_rebuild(const ExpandedUniversalRegistry *r)
{
    uint32_t i,applied=0;
    for(i=0;i<r->scheduler.count;++i) {
        const ExpandedScheduledEvent *ev=&r->scheduler.events[i];
        uint32_t k;
        if(ev->state!=EXP_EVENT_COMPLETE)continue;
        for(k=0;k<r->fixture_count;++k)
            if(r->fixtures[k].event_id==ev->id) {
                const ExpandedFixtureLedgerEntry *e=&r->ledger.entries[k];
                if(e->state==EXP_FIXTURE_PLAYED &&
                   ExpandedUniversal_FifaRecord(&r->fixtures[k],&e->score,NULL)==1)++applied;
                break;
            }
    }
    return applied;
}
/* START: editions of the first season + JSON first editions. */
static int exp_live_engine_start(ExpandedUniversalRegistry *r)
{
    (void)exp_live_fifa_seed(r->scheduler.clock.day_index);
    if(!exp_live_engine_definitions() ||
       !ExpCe_BeginWorld(r,&s_ce_host,(uint16_t)World_Current(),World_FirstYear())) {
        fprintf(stderr,"[ce-engine] BEGIN_FAILED world=%s (leagues still "
            "play their first edition; lifecycle disabled)\n",World_Tag());
        return 0;
    }
    (void)ExpCe_Tick(r,&s_ce_host,r->scheduler.clock.day_index);
    return 1;
}
/* CONTINUE: called by the loader once rosters/catalog are back. */
int ExpandedUniversalLive_EngineRestore(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    char *hex;
    if(World_Current()!=WORLD_2627 || !r){ExpFifa_Reset();return 1;}
    if(!exp_live_engine_definitions())return 0;
    hex=ExpandedUniversal_CarTakeLifecycle();
    if(hex) {
        const int ok=ExpCe_DecodeHex(hex,strlen(hex),(uint16_t)World_Current());
        free(hex);
        if(!ok)return 0; /* already validated in the preflight */
    } else if(!ExpCe_BeginWorld(r,&s_ce_host,(uint16_t)World_Current(),
                                World_FirstYear()))
        return 0; /* pre-v296 CAR: rebuilt from the registry */
    fprintf(stderr,"[ce-engine] RESTORED editions=%lu source=%s changed=%d\n",
        (unsigned long)ExpCe_EditionCount(),hex?"UL01":"REGISTRY_REBUILD",
        ExpCe_ValidateAfterRestore(r,&s_ce_host));
    {
        char *fifa=ExpandedUniversal_CarTakeFifa();
        {   /* v00.13.355 discipline of real matches (validated in the preflight) */
            char *dt=ExpandedUniversal_CarTakeDiscipline();
            ExpCeDisc_Reset();
            if(dt && !ExpCeDisc_Decode(dt))fprintf(stderr,"[ce-discipline] RESTORE refused\n");
            free(dt);
        }
        if(!exp_live_fifa_seed(r->scheduler.clock.day_index)) {
            fprintf(stderr,"[fifa-ranking] RESTORE seed missing: ranking unavailable\n");
        } else if(fifa) {
            if(!ExpFifa_Decode(fifa)) {
                /* validated in the preflight: only an identity change of the
                 * seed (different 211 nations) can refuse it */
                fprintf(stderr,"[fifa-ranking] RESTORE refused (identity): ranking unavailable\n");
                ExpFifa_Reset();
            } else fprintf(stderr,"[fifa-ranking] RESTORED FR01 events=%lu snapshots=%lu week=%ld\n",
                (unsigned long)ExpFifa_EventCount(),(unsigned long)ExpFifa_SnapshotCount(),
                (long)ExpFifa_WeekId());
        } else {
            const uint32_t n=exp_live_fifa_rebuild(r);
            fprintf(stderr,"[fifa-ranking] OLD_CAR rebuilt from persisted results: "
                "applied=%lu (weekly references restart from the current week)\n",(unsigned long)n);
        }
        free(fifa);
    }
    return 1;
}
/* A refusal BEFORE any claim (rules/roster/tie context/lineup data): block
 * only that edition and its dependents; the rest of the world goes on. */
static int exp_live_local_fault(ExpandedUniversalRegistry *r,
                                const ExpandedUniversalFixture *f,
                                ExpandedUniversalResult res)
{
    const ExpandedFixtureLedgerEntry *e=ExpandedUniversal_EntryFor(r,f);
    if((res!=EXP_UNIVERSAL_INVALID && res!=EXP_UNIVERSAL_BLOCKED) || !e ||
       e->state!=EXP_FIXTURE_SCHEDULED || r->recovery_required)return 0;
    if(!ExpCe_BlockEdition(r,&s_ce_host,f->key.edition,
                           EXP_CE_BLOCK_MATCH_REFUSED,
                           "fixture refused before claim (rules/roster/tie)"))
        return 0;
    exp_live_fault("EDITION_BLOCKED_LOCAL",r,f,"SCHEDULED",(int)res,
        "V:pre-claim-refusal world-continues=1 no-score=1");
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
    {s_cpu_batch_pending=0u;s_backlog_digest=0u;} /* new career, never inherit another world */
    if(!ExpandedUniversalCatalog_Audit(&a) || !a.sealed ||
       !a.future_fixtures) {
        fprintf(stderr,"[universal-live] NO_JSON_FIXTURES fail-closed=1\n");
        return 0;
    }
    ExpandedUniversal_CarResetCurrent(); /* in RAM only, not an old CAR file */
    if(!ExpandedUniversal_CarBegin(
        (ExpandedWorldDate){World_FirstYear(),7u,1u},
        a.future_fixtures+a.historical_verified))return 0;
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
        if(f && f->seed_continuation && f->date_collision){
            free(ordered);ExpandedUniversal_CarResetCurrent();return 0;
        }
        if(!f || (f->past_at_start && !f->historical_verified) || f->date_collision) {
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
        claim->state=ordered[i]->historical_verified?EXP_FIXTURE_PLAYED:EXP_FIXTURE_SCHEDULED;
        if(ordered[i]->historical_verified)claim->score=ordered[i]->historical_score;
        *event=(ExpandedScheduledEvent){
            f->event_id,0u,f->day_index,10u,EXP_EVENT_FIXTURE,
            ordered[i]->historical_verified?EXP_EVENT_COMPLETE:EXP_EVENT_PENDING};
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
    if(!exp_live_engine_start(r)){ExpandedUniversal_CarResetCurrent();return 0;}
    return 1;
}
/* v311: rank_tiebreak==2 (Serie B final / play-out): when the two clubs finished the
 * REGULAR season level on points, the deciding match gets extra time and penalties.
 * Decided by the CE from the regular table BEFORE the match; OG only plays it. */
static int exp_live_equal_regular_points(const ExpandedUniversalFixture *f)
{
    ExpandedUniversalRegistry *const r=ExpandedUniversal_CarRegistry();
    ExpCeTableRules tr;
    ExpCeTableRow rows[EXP_CE_MAX_ENTRANTS];
    uint32_t n=0u,i;
    int h=-1,a=-1;
    if(!r || !ExpCe_TableRules(f->key.edition,EXP_CE_STAGE(0u,1u),&tr) ||
       !ExpCeTable_Compute(r,f->key.edition,EXP_CE_STAGE(0u,1u),&tr,
           ExpandedUniversalLive_ClubNameCompare,NULL,rows,EXP_CE_MAX_ENTRANTS,&n))return 0;
    for(i=0u;i<n;++i) {
        if(ExpandedClubKey_Equal(rows[i].club,f->home))h=(int)i;
        if(ExpandedClubKey_Equal(rows[i].club,f->away))a=(int)i;
    }
    return h>=0 && a>=0 && rows[h].points==rows[a].points;
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
    {
        /* Fase 2: the rules of THIS fixture (competition + phase) from the
         * CE engine: bench, substitutions, OG discipline mode, knockout
         * extra time / penalties / aggregate / away goals, windows. No
         * rules = refused BEFORE any claim (local block, never a guess). */
        ExpandedUniversalRegistry *const rr=ExpandedUniversal_CarRegistry();
        ExpCeMatchRules mr;
        ExpCeTableRules tr;
        uint8_t kind=0u,meetings=0u,decides=0u;
        uint16_t h1=0u,a1=0u;
        memset(&tr,0,sizeof tr);
        if(!ExpCe_RulesForFixture(f,&mr,&tr) ||
           !ExpCe_FixturePhase(f,&kind,&meetings)) {
            exp_live_fault("RULES_MISSING",rr,f,"SCHEDULED",
                (int)EXP_UNIVERSAL_INVALID,"V:no-valid-rules-for-competition-phase");
            return EXP_UNIVERSAL_INVALID;
        }
        if(!ExpCeTie_MatchContext(rr,f,&mr,&decides,&h1,&a1)) {
            exp_live_fault("TIE_CONTEXT_INVALID",rr,f,"SCHEDULED",
                (int)EXP_UNIVERSAL_INVALID,"V:first-leg-missing-or-not-played");
            return EXP_UNIVERSAL_INVALID;
        }
        /* v309: a TABLE phase whose drawn matches are decided on penalties
         * (table.draw_penalties): OG plays the shoot-out after a level
         * regulation score (same OG context as a single-leg knockout) */
        if(kind==EXP_CE_PHASE_TABLE && tr.draw_penalties && mr.penalties)
            decides=1u;
        ctx.minimum_substitutes=(int16_t)mr.subs;    /* OG minSubs = allowed */
        ctx.maximum_substitutes=(int16_t)mr.bench;   /* OG maxSubs = bench */
        ctx.match_parameters=(int16_t)mr.discipline; /* OG 0x3C / paramD3 */
        ctx.decides_tie=decides;ctx.extra_time=mr.extra_time;
        ctx.penalties=mr.penalties;ctx.away_goals=mr.away_goals;
        if(decides && mr.rank_tiebreak==2u && exp_live_equal_regular_points(f)) {
            ctx.extra_time=1;ctx.penalties=1;   /* level on regular points: played to the end */
        }
        ctx.second_leg=(uint8_t)(mr.legs==2u && f->key.leg==1u);
        ctx.first_leg_home=h1;ctx.first_leg_away=a1;
        ctx.sub_windows=mr.windows;ctx.halftime_free=mr.halftime_free;
        ctx.et_extra_subs=mr.et_extra_subs;ctx.et_extra_windows=mr.et_extra_windows;
        /* OG: a single-meeting league is scored without home advantage */
        ctx.og_mode_flag=(uint8_t)((kind==EXP_CE_PHASE_TABLE && meetings==1u)?0xffu:0u);
    }
    ctx.home=human && ExpandedClubKey_Equal(manager,f->home)?
        &_careerTeam:ExpandedUniversalCatalog_Roster(f->home);
    ctx.away=human && ExpandedClubKey_Equal(manager,f->away)?
        &_careerTeam:ExpandedUniversalCatalog_Roster(f->away);
    if(!ctx.home || !ctx.away) {
        exp_live_fault("ROSTER_MISSING",ExpandedUniversal_CarRegistry(),f,
            "SCHEDULED",(int)EXP_UNIVERSAL_INVALID,
            ctx.home?"V:away-roster-not-in-catalog":
                     "V:home-roster-not-in-catalog");
        return EXP_UNIVERSAL_INVALID; /* pre-claim: local block */
    }
    /* existing type0 presentation keeps the OG "LEAGUE" game name */
    ctx.competition_name=f->competition_type==0u?"LEAGUE":"CUP";
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
    {
        /* Normal flush: the CarWriter thread writes it while the game goes
         * on. After an async failure the next flush is synchronous. */
        const int async=!s_async_failed;
        if(!ExpandedUniversal_CarSaveCpuBatchEx(s_cpu_batch_pending,async)) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-live] CPU_BATCH_COMMIT_FAILED "
                "recovery=1 previous-CAR-preserved=1\n");
            return 0;
        }
        if(!async)s_async_failed=0; /* a synchronous save is durable again */
    }
    {s_cpu_batch_pending=0u;s_backlog_digest=0u;}
    return 1;
}
/* An ASYNC CAR write that failed left its results only in RAM: treat the
 * backlog as unsaved again and make the next flush synchronous. */
static void exp_live_check_async_failure(void)
{
    if(CarWriter_TakeFailure()) {
        s_async_failed=1;
        if(s_cpu_batch_pending<exp_cpu_threshold())
            s_cpu_batch_pending=exp_cpu_threshold();
        fprintf(stderr,"[universal-live] ASYNC_CAR_FAILED backlog-treated-as-"
            "unsaved next-flush=SYNC no-replay=1\n");
    }
}
/* A loaded CAR is already a durable snapshot: transient CPU count resets. */
void ExpandedUniversalLive_ResetCpuBatch(void)
{
    {s_cpu_batch_pending=0u;s_backlog_digest=0u;}
    s_async_failed=0;
}
/* OG CAREER board round: the manager's league fixtures of the same edition
 * on the same civil day. OG shows every one of them in its own tabellone
 * (CPU ones with VIEW RESULT), so ordinary FORWARD must stop BEFORE the
 * first of them, not only before the manager's own fixture. Cups keep the
 * previous manager-fixture-only stop (no native CE bracket yet). */
static int exp_live_is_league_round(const ExpandedUniversalFixture *f)
{
    uint8_t kind=0;
    return f && f->competition_type==0u &&
           ExpCe_FixturePhase(f,&kind,NULL) && kind==EXP_CE_PHASE_TABLE;
}
static int exp_live_same_round(const ExpandedUniversalFixture *f,
                               const ExpandedUniversalFixture *mf)
{
    return exp_live_is_league_round(f) && exp_live_is_league_round(mf) &&
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
        if(res!=EXP_UNIVERSAL_OK && exp_live_local_fault(r,g,res))continue;
        if(res!=EXP_UNIVERSAL_OK) {
            r->recovery_required=1u;
            fprintf(stderr,"[universal-live] ROUND_CPU_FAULT event=%llu "
                "CAR_CLAIMED_NO_REPLAY=1\n",(unsigned long long)g->event_id);
            exp_live_fault("ROUND_CPU_FAULT",r,g,"SCHEDULED",(int)res,
                "V:og-quick-or-dispatch-refused");
            return 1;
        }
        { if(s_cpu_batch_pending<UINT32_MAX)++s_cpu_batch_pending; exp_backlog_note(r,g); }
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
    /* v286 (user 23/11/2026): the weekly FORWARD no longer pays a full CAR
     * save. With ~800 CPU quick results a week the old 512 threshold fired on
     * EVERY forward (0.75 s of a 1.3 s call), although the manager's own
     * VIEW RESULT / PLAY already persists every pending result in one atomic
     * CAR. A save now happens at the manager's match, on quit/menu exit and
     * existing autosaves, and when this RAM-only backlog reaches the bound
     * below (about five weeks). A crash loses at most the backlog; CONTINUE
     * recomputes it from the previous complete CAR, never replaying a
     * durable result. */
    exp_live_check_async_failure();
    if(s_cpu_batch_pending>=exp_cpu_threshold() &&
       !exp_live_flush_cpu(r))return 0;
    /* The manager fixture is never executed by ordinary FORWARD, so its
     * round anchor cannot change inside this call. DEBUG keeps old path. */
    const ExpandedUniversalFixture *round_anchor=NULL;
    int32_t ticked_day=r->scheduler.clock.day_index;
    /* Fase 2 lifecycle (phases, closures, new editions) at each civil day
     * the FORWARD reaches; never plays a match. */
    (void)ExpCe_Tick(r,&s_ce_host,ticked_day);
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
        if(res==EXP_UNIVERSAL_DUE && f && f->day_index!=ticked_day) {
            /* new civil day: lifecycle first (may add fixtures), then peek
             * again so newly generated earlier events keep CE order */
            ticked_day=f->day_index;
            r->scheduler.clock.day_index=f->day_index;
            if(ExpCe_Tick(r,&s_ce_host,ticked_day)>0)continue;
        }
        if(res==EXP_UNIVERSAL_EMPTY) {
            r->scheduler.clock.day_index=to; /* CE clock only */
            if(ticked_day!=to) {
                ticked_day=to;
                if(ExpCe_Tick(r,&s_ce_host,to)>0)continue;
            }
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
            exp_live_fault("ADVANCE_BLOCKED",r,f,f?exp_live_fixture_state(r,f):
                "NONE",(int)res,res==EXP_UNIVERSAL_RECOVERY?
                "V:registry-recovery(claimed/past-pending/mismatch)":
                "V:peek-not-due");
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
            if(match_result!=EXP_UNIVERSAL_OK &&
               exp_live_local_fault(r,f,match_result))continue;
            if(match_result!=EXP_UNIVERSAL_OK) {
                /* Do not persist an uncertain partial block; preceding
                 * atomic CAR is still the last valid checkpoint. */
                r->recovery_required=1u;
                fprintf(stderr,"[universal-live] CPU_FAULT "
                    "event=%llu CAR_CLAIMED_NO_REPLAY=1\n",
                    (unsigned long long)f->event_id);
                exp_live_fault("CPU_FAULT",r,f,"SCHEDULED",
                    (int)match_result,"V:og-quick-or-dispatch-refused");
                return 0;
            }
            { if(s_cpu_batch_pending<UINT32_MAX)++s_cpu_batch_pending; exp_backlog_note(r,f); }
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
    if(s_cpu_batch_pending)
        fprintf(stderr,"[universal-live] BACKLOG_DIGEST pending=%lu "
            "digest=%016llx\n",(unsigned long)s_cpu_batch_pending,
            (unsigned long long)s_backlog_digest);
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

/* Fase 1: a contract/club change in WORLD2627 only rebinds the manager;
 * the CE world (fixtures, ledger, clock) is never rebuilt. 1 = the bound
 * club is a CE club with a catalog roster (replaces the OG resident
 * Expanded_OGWorld_ClubBound() test, which the CE never sets). */
int ExpandedUniversalLive_ManagerBound(void)
{
    ExpandedClubKey manager={0};
    /* v00.13.300: the OG national selector is bound to its NATIONAL key;
     * a national team has no club roster in the catalog. */
    return ExpandedUniversalLive_Active() && manager_key(&manager) &&
           (ExpandedClubKey_IsNational(manager) ||
            ExpandedUniversalCatalog_Roster(manager)!=NULL);
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
           f && f->event_id!=mf->event_id &&
           exp_live_drain_today(r,today,mf->event_id,0,manager)!=2) {
            /* Earlier same-day events did not all complete (CPU fault,
             * quit request): never jump over them to the manager. */
            exp_live_fault("MANAGER_PREDRAIN_STOPPED",r,mf,"SCHEDULED",
                r->recovery_required?(int)EXP_UNIVERSAL_RECOVERY:
                                     (int)EXP_UNIVERSAL_BLOCKED,
                r->recovery_required?"V:earlier-cpu-event-failed":
                                     "V:quit-requested");
            return 1;
        }
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
    if(res==EXP_UNIVERSAL_OK){s_cpu_batch_pending=0u;s_backlog_digest=0u;}
    fprintf(stderr,"[universal-live] MANAGER_ACTION mode=%s "
        "event=%llu result=%d CAR_ONE_FIXTURE=1\n",
        interactive?"PLAY_50HZ":"VIEW_RESULT_QUICK",
        (unsigned long long)f->event_id,(int)res);
    if(res!=EXP_UNIVERSAL_OK && res!=EXP_UNIVERSAL_BLOCKED) {
        r->recovery_required=1u;
        exp_live_fault("MANAGER_MATCH_FAULT",r,f,"SCHEDULED",(int)res,
            interactive?"V:50hz-ended-without-valid-witness(CLAIMED kept)":
                        "V:og-quick-witness-or-CAR-completion-rejected");
    }
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
/* 1 when the manager's fixture due today is NOT a league round (cup, super
 * cup, multi-phase): the OG league board has no projection for it, so the
 * menu uses the original OG team screen (VIEW RESULT = OG quick, MATCH =
 * OG 50 Hz) fed by the same CE fixture. Never auto-played by FORWARD. */
int ExpandedUniversalLive_ManagerDueIsNonLeague(void)
{
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    ExpandedClubKey manager={0};
    const ExpandedUniversalFixture *f=NULL;
    int32_t today=0;
    if(!r || r->recovery_required || !ExpandedUniversalLive_Active() ||
       !ExpandedUniversalLive_TodayIndex(&today) || !manager_key(&manager))
        return 0;
    f=exp_live_manager_fixture(r,manager);
    return f && f->day_index==today && !exp_live_is_league_round(f);
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
