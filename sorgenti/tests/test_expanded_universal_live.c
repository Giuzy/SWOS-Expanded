/* Fase 1 — isolated C proof of the REAL WORLD2627 live cycle:
 *   CE fixture SCHEDULED -> claim -> OG match -> one result -> CE update
 *   -> durable CAR -> CONTINUE without replay,
 * for CPU (FORWARD, unemployed or employed) and manager (VIEW RESULT quick,
 * PLAY 50 Hz completed / cancelled before start / ESC abort / crash).
 *
 * REAL code under test: expanded_universal_live.c (FORWARD, ManagerNext,
 * manager actions, round CPU drain, ManagerBound), expanded_universal_career_
 * dispatch.c, expanded_universal_fixture_registry.h (RunDue), the universal
 * CAR codec expanded_universal_car.c (+ UE02 compact + archive), mjson,
 * Gregorian calendar and the one standings reader.
 * MOCKED: the OG match itself (QUICK/50 Hz score comes from a local RNG), the
 * catalog/rosters, LWE mirror registration, the CarWriter thread and the disk
 * (the "disk" is the last JSON image produced by the real CAR emitter).
 * It is NOT a Windows build, NOT gameplay and NOT the OG menus/board.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_live.h"
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_car.h"
#include "expanded_universal_native_match.h"
#include "expanded_universal_standings.h"
#include "expanded_world_live.h"
#include "platform_sdl.h"
#include "play_match_menu.h"

/* ---- game globals read by the live layer -------------------------------- */
uint16_t _quitToDos,_gameType,_selTeamNumber=0xffffu,_youreSacked;
TeamFileEx _careerTeam;

/* ---- platform / world / io stubs ---------------------------------------- */
static WorldId s_world=WORLD_2627;
WorldId World_Current(void){return s_world;}
uint16_t World_FirstYear(void){return 2026u;}
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}
Uint64 Platform_NowMs(void){return 0;}
void Platform_UniversalCpuProgress(uint32_t count){(void)count;}
int CarWriter_TakeFailure(void){return 0;}
static ExpandedWorldClockProvider s_provider;
void ExpandedWorldLive_SetClockProvider(ExpandedWorldClockProvider p){s_provider=p;}

/* ---- catalog: two leagues of 4 clubs, double round robin ---------------- */
#define CLUBS 8u
#define ROUNDS 6u
#define PER_ROUND 4u /* 2 matches per league per round */
#define NFIX (ROUNDS*PER_ROUND)
static TeamFileEx s_teams[CLUBS+1u];
static ExpandedUniversalCatalogFixture s_cat[NFIX+1u];
static uint32_t s_cat_count;
static int s_missing_roster; /* stable club whose roster "is missing" */
static int32_t date(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){
        (uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static int32_t s_round_day[ROUNDS];
int ExpandedUniversalCatalog_Audit(ExpandedUniversalCatalogAudit *a)
{
    memset(a,0,sizeof *a);
    a->sealed=1u;a->fixtures=s_cat_count;a->future_fixtures=s_cat_count;
    a->clubs=CLUBS;a->league_definitions=2u;a->source_complete=1u;
    return 1;
}
const ExpandedUniversalCatalogFixture *ExpandedUniversalCatalog_Fixture(uint32_t i)
{
    return i<s_cat_count?&s_cat[i]:NULL;
}
TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey key)
{
    if(key.world!=WORLD_2627 || key.stable_club<1u || key.stable_club>CLUBS ||
       (int)key.stable_club==s_missing_roster)return NULL;
    return &s_teams[key.stable_club];
}
static ExpandedClubKey club(uint32_t k){return (ExpandedClubKey){WORLD_2627,k};}
static void build_catalog(uint8_t first_type)
{
    /* rr pairs for 4 clubs (a,b,c,d) -> rounds 0..2, mirrored 3..5 */
    static const uint8_t pairs[3][2][2]={{{0,1},{2,3}},{{0,2},{3,1}},{{0,3},{1,2}}};
    unsigned rd,lg,m;
    s_cat_count=0u;
    for(rd=0;rd<ROUNDS;++rd)s_round_day[rd]=date(2026,8,2)+(int32_t)(7*rd);
    for(rd=0;rd<ROUNDS;++rd)for(lg=0;lg<2u;++lg)for(m=0;m<2u;++m) {
        ExpandedUniversalCatalogFixture *c=&s_cat[s_cat_count];
        ExpandedUniversalFixture *f=&c->fixture;
        const uint8_t *p=pairs[rd%3u][m];
        const uint32_t base=lg*4u+1u;
        const int mirror=rd>=3u;
        memset(c,0,sizeof *c);
        f->home=club(base+(mirror?p[1]:p[0]));
        f->away=club(base+(mirror?p[0]:p[1]));
        f->day_index=s_round_day[rd];
        /* League B (clubs 5-8) is ordered BEFORE league A on the same day:
         * FORWARD must run those CPU events, then stop at the manager's
         * round (OG board). */
        f->event_id=(uint64_t)f->day_index*100u+(lg==0u?50u:10u)+m;
        f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,1000u+lg};
        f->key.edition.start_day=s_round_day[0];
        f->key.edition.sequence=1u;
        f->key.stage=1u;f->key.tie=rd*2u+m+1u;
        f->country=16u;f->slot=(uint8_t)lg;
        f->competition_type=(s_cat_count==0u)?first_type:0u;
        c->date_provenance=EXP_CATALOG_DATE_CE_PLANNER;
        ++s_cat_count;
    }
}

/* ---- OG match mock ------------------------------------------------------ */
static uint64_t s_rng=0x9e3779b97f4a7c15ull;
static unsigned s_runs[NFIX],s_ok_runs[NFIX];
static unsigned s_50hz,s_quick,s_prompts,s_interactive_prepares;
static int s_cancel_next,s_abort_next,s_crash_next,s_result_prompt;
static int fix_index(uint64_t id)
{
    uint32_t i;
    for(i=0;i<s_cat_count;++i)if(s_cat[i].fixture.event_id==id)return (int)i;
    assert(0);return -1;
}
int ExpandedUniversal_NativeTeamKey(const TeamFileEx *t,ExpandedClubKey *out)
{
    if(!t || !out || t->team.countryNumber!=0x10u)return 0;
    *out=club(t->team.teamOrdinal);
    return 1;
}
void ExpandedUniversal_NativeSetResultPrompt(int on){s_result_prompt=on!=0;}
int ExpandedUniversal_NativePrepare(ExpandedUniversalNativeMatch *c,
                                    const ExpandedUniversalFixture *f)
{
    ExpandedClubKey h,a;
    c->lineup_ready=0u;c->abandoned=0u;
    assert(ExpandedUniversal_NativeTeamKey(c->home,&h) &&
           ExpandedClubKey_Equal(h,f->home));
    assert(ExpandedUniversal_NativeTeamKey(c->away,&a) &&
           ExpandedClubKey_Equal(a,f->away));
    if(c->interactive)++s_interactive_prepares;
    if(c->interactive || (s_result_prompt &&
       (c->home==&_careerTeam || c->away==&_careerTeam))) {
        if(!c->interactive)++s_prompts;
        if(s_cancel_next){s_cancel_next=0;return 0;} /* EXIT on team screen */
    }
    c->lineup_ready=1u;
    return 1;
}
int ExpandedUniversal_NativeRunOriginal(void *user,const ExpandedUniversalFixture *f,
                                        ExpandedUniversalMatchWitness *w)
{
    ExpandedUniversalNativeMatch *c=(ExpandedUniversalNativeMatch *)user;
    const int k=fix_index(f->event_id);
    memset(w,0,sizeof *w);
    assert(c->lineup_ready);
    c->lineup_ready=0u;
    ++s_runs[k];
    if(c->interactive) {
        ++s_50hz;
        if(s_abort_next){s_abort_next=0;c->abandoned=1u;return 0;}
        if(s_crash_next){s_crash_next=0;return 0;}
    } else ++s_quick;
    s_rng^=s_rng<<13;s_rng^=s_rng>>7;s_rng^=s_rng<<17;
    w->key=f->key;w->home=f->home;w->away=f->away;w->day_index=f->day_index;
    w->score=(ExpandedFixtureScore){(uint16_t)(s_rng%5u),(uint16_t)((s_rng>>9)%4u),0u,0u,0u};
    w->valid=1u;
    ++s_ok_runs[k];
    return 1;
}

/* ---- "disk": the real CAR emitter output -------------------------------- */
static char s_disk[1<<18];
static int s_disk_len;
static unsigned s_saves,s_claim_saves,s_complete_saves,s_release_saves,s_batch_saves;
static int s_fail_saves;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    struct mjson_fixedbuf dst={s_disk,(int)sizeof s_disk,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    static char backup[1<<18];
    int backup_len=s_disk_len;
    if(s_fail_saves)return 0; /* previous CAR untouched */
    assert(r);
    memcpy(backup,s_disk,(size_t)s_disk_len);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"{%Q:{%Q:%d}",
        "living_world","now",(int)r->scheduler.clock.day_index)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof s_disk-1);
    if(!ExpandedUniversal_CarValidate(s_disk,dst.len)) {
        memcpy(s_disk,backup,(size_t)backup_len);s_disk_len=backup_len;
        return 0;
    }
    s_disk_len=dst.len;
    ++s_saves;
    if(strstr(reason,"claimed"))++s_claim_saves;
    else if(strstr(reason,"complete"))++s_complete_saves;
    else if(strstr(reason,"abandoned"))++s_release_saves;
    else if(strstr(reason,"batch"))++s_batch_saves;
    return 1;
}
/* EXIT + CONTINUE: the process forgets RAM; only the last durable CAR. */
static int continue_from_disk(void)
{
    if(!ExpandedUniversal_CarValidate(s_disk,s_disk_len))return 0;
    return ExpandedUniversal_CarRestore(s_disk,s_disk_len);
}

/* ---- helpers ------------------------------------------------------------ */
static ExpandedUniversalRegistry *R(void){return ExpandedUniversal_CarRegistry();}
static int state_of(uint32_t k)
{
    ExpandedUniversalRegistry *r=R();
    const ExpandedUniversalFixture *f=ExpandedUniversal_Find(r,s_cat[k].fixture.event_id);
    const ExpandedFixtureLedgerEntry *e=f?ExpandedUniversal_EntryFor(r,f):NULL;
    assert(e);
    return (int)e->state;
}
static uint32_t count_state(int st)
{
    uint32_t k,n=0u;
    for(k=0;k<s_cat_count;++k)if(state_of(k)==st)++n;
    return n;
}
static ExpandedWorldDate D(int32_t day)
{
    ExpandedWorldDate d;
    assert(ExpandedWorld_IndexToDate(day,&d));
    return d;
}
static void employ(uint32_t k)
{
    memset(&_careerTeam,0,sizeof _careerTeam);
    _careerTeam.team.countryNumber=0x10u;
    _careerTeam.team.teamOrdinal=(uint8_t)k;
    _selTeamNumber=(uint16_t)(0x1000u|k);
    _youreSacked=0u;
    _gameType=GAME_TYPE_CAREER;
}
static void unemploy(void)
{
    _selTeamNumber=0xffffu;_youreSacked=0u;_gameType=GAME_TYPE_CAREER;
}
static void reset_counters(void)
{
    memset(s_runs,0,sizeof s_runs);memset(s_ok_runs,0,sizeof s_ok_runs);
    s_50hz=s_quick=s_prompts=s_interactive_prepares=0u;
    s_saves=s_claim_saves=s_complete_saves=s_release_saves=s_batch_saves=0u;
}
static void start_world(void)
{
    uint32_t k;
    for(k=1;k<=CLUBS;++k) {
        memset(&s_teams[k],0,sizeof s_teams[k]);
        s_teams[k].team.countryNumber=0x10u;
        s_teams[k].team.teamOrdinal=(uint8_t)k;
    }
    reset_counters();
    s_disk_len=0;
    assert(ExpandedUniversalLive_Start()==1);
    assert(ExpandedUniversalLive_Active());
    assert(R()->fixture_count==NFIX && count_state(EXP_FIXTURE_SCHEDULED)==NFIX);
    /* START never plays or claims anything, never marks a future fixture
     * completed (RG START / primo FORWARD). */
    assert(s_runs[0]==0u && s_saves==0u && R()->recovery_required==0u);
    assert(s_provider!=NULL);
}
static int manager_fixture_of_round(unsigned rd,uint32_t mclub)
{
    uint32_t k;
    for(k=0;k<s_cat_count;++k)
        if(s_cat[k].fixture.day_index==s_round_day[rd] &&
           (s_cat[k].fixture.home.stable_club==mclub ||
            s_cat[k].fixture.away.stable_club==mclub))return (int)k;
    return -1;
}
/* standings from the CE ledger == independent recomputation of the scores */
static void check_standings(uint32_t league)
{
    ExpandedUniversalStanding rows[8];
    uint32_t used=0u,i,k,pts[CLUBS+1u]={0},pl[CLUBS+1u]={0};
    ExpandedUniversalRegistry *r=R();
    ExpandedEditionKey ed=s_cat[league==0u?0:2].fixture.key.edition;
    assert(ed.competition.stable_competition==1000u+league);
    for(k=0;k<s_cat_count;++k) {
        const ExpandedUniversalFixture *f=ExpandedUniversal_Find(r,s_cat[k].fixture.event_id);
        const ExpandedFixtureLedgerEntry *e=ExpandedUniversal_EntryFor(r,f);
        if(!ExpandedEditionKey_Equal(f->key.edition,ed) ||
           e->state!=EXP_FIXTURE_PLAYED)continue;
        ++pl[f->home.stable_club];++pl[f->away.stable_club];
        if(e->score.home>e->score.away)pts[f->home.stable_club]+=3u;
        else if(e->score.home<e->score.away)pts[f->away.stable_club]+=3u;
        else {++pts[f->home.stable_club];++pts[f->away.stable_club];}
    }
    assert(ExpandedUniversal_Standings(r,ed,1u,3u,rows,8u,&used) && used==4u);
    for(i=0;i<used;++i) {
        assert(rows[i].points==pts[rows[i].club.stable_club]);
        assert(rows[i].played==pl[rows[i].club.stable_club]);
    }
}
static void snapshot_scores(ExpandedFixtureScore *out,uint8_t *st)
{
    uint32_t k;
    for(k=0;k<s_cat_count;++k) {
        const ExpandedUniversalFixture *f=ExpandedUniversal_Find(R(),s_cat[k].fixture.event_id);
        const ExpandedFixtureLedgerEntry *e=ExpandedUniversal_EntryFor(R(),f);
        out[k]=e->score;st[k]=(uint8_t)e->state;
    }
}
static ExpandedWorldDate END(void){return D(s_round_day[ROUNDS-1u]+3);}

int main(void)
{
    ExpandedWorldDate due;
    uint32_t k;
    int mk;
    ExpandedFixtureScore scoresA[NFIX],scoresB[NFIX];
    uint8_t stA[NFIX],stB[NFIX];

    /* ===== 1. START 2026/27 unemployed + first FORWARD (RG-T03/15) ===== */
    build_catalog(0u);
    unemploy();
    start_world();
    assert(ExpandedUniversalLive_ManagerNext(&due)==-1); /* no manager */
    assert(ExpandedUniversalLive_ManagerBound()==0);
    assert(ExpandedUniversalLive_Advance(D(s_round_day[0]))==1);
    assert(count_state(EXP_FIXTURE_PLAYED)==PER_ROUND);
    assert(ExpandedUniversalLive_Advance(END())==1);
    /* CPU only: OG QUICK, never an interactive match/prompt, each once. */
    assert(s_50hz==0u && s_interactive_prepares==0u && s_prompts==0u);
    assert(s_quick==NFIX && count_state(EXP_FIXTURE_PLAYED)==NFIX);
    for(k=0;k<NFIX;++k)assert(s_runs[k]==1u && s_ok_runs[k]==1u);
    /* FORWARD stop is not an autosave (v286 policy: batch below 4000). */
    assert(s_saves==0u);
    { int32_t today=-1; assert(ExpandedUniversalLive_TodayIndex(&today) &&
          today==s_round_day[ROUNDS-1u]+3); }
    check_standings(0u);check_standings(1u);
    /* A later FORWARD never replays results. */
    assert(ExpandedUniversalLive_Advance(D(s_round_day[ROUNDS-1u]+10))==1);
    for(k=0;k<NFIX;++k)assert(s_runs[k]==1u);
    puts("CASE1_START_UNEMPLOYED_FORWARD_CPU_QUICK_ONLY=PASS");

    /* ===== 2. START employed: FORWARD stops at the manager's round ===== */
    ExpandedUniversal_CarResetCurrent();
    build_catalog(0u);
    employ(1u);
    start_world();
    assert(ExpandedUniversalLive_ManagerBound()==1);
    assert(ExpandedUniversalLive_ManagerNext(&due)==1); /* FORWARD */
    assert(ExpandedUniversalLive_Advance(END())==1);
    /* League B (ordered first that day) ran as CPU; league A round waits. */
    {
        int32_t today=-1;
        assert(ExpandedUniversalLive_TodayIndex(&today) && today==s_round_day[0]);
    }
    assert(count_state(EXP_FIXTURE_PLAYED)==2u);
    mk=manager_fixture_of_round(0u,1u);
    assert(mk>=0 && state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED);
    assert(ExpandedUniversalLive_ManagerNext(&due)==2);
    assert(s_50hz==0u && s_interactive_prepares==0u);
    /* FORWARD again cannot skip the manager (no auto-play, no replay). */
    assert(ExpandedUniversalLive_Advance(END())==1);
    assert(state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED && s_runs[mk]==0u);
    puts("CASE2_START_EMPLOYED_FORWARD_STOPS_AT_MANAGER_ROUND=PASS");

    /* ===== 7. cancel before start: no result, no CAR (VIEW + PLAY) ===== */
    s_cancel_next=1;
    assert(ExpandedUniversalLive_ViewManagerResult()==1);
    assert(state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED && s_runs[mk]==0u);
    s_cancel_next=1;
    assert(ExpandedUniversalLive_PlayManager()==1);
    assert(state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED && s_runs[mk]==0u);
    assert(s_claim_saves==0u && s_complete_saves==0u);
    assert(R()->recovery_required==0u && ExpandedUniversalLive_ManagerNext(&due)==2);
    puts("CASE7_CANCEL_BEFORE_START_NO_RESULT_NO_CAR=PASS");

    /* ===== 5. manager VIEW RESULT = OG QUICK, one durable COMPLETED ===== */
    {
        const unsigned quick_before=s_quick;
        assert(ExpandedUniversalLive_ViewManagerResult()==2);
        assert(state_of((uint32_t)mk)==EXP_FIXTURE_PLAYED);
        assert(s_runs[mk]==1u && s_quick>quick_before && s_50hz==0u);
        /* league B ran before; the other league-A fixture of the round
         * comes AFTER the manager's in CE order: it stays for the OG
         * board (NEXT MATCH -> VIEW RESULT) or the next FORWARD. */
        assert(count_state(EXP_FIXTURE_PLAYED)==PER_ROUND-1u);
        assert(s_claim_saves==0u && s_complete_saves==1u);
        {
            const uint32_t other=(uint32_t)mk+1u; /* same round, league A */
            assert(s_cat[other].fixture.day_index==s_cat[mk].fixture.day_index);
            /* the manager's own fixture can't go through the CPU board path */
            assert(ExpandedUniversalLive_ViewRoundCpuResult(
                s_cat[mk].fixture.event_id)==1);
            assert(ExpandedUniversalLive_ViewRoundCpuResult(
                s_cat[other].fixture.event_id)==2);
            assert(state_of(other)==EXP_FIXTURE_PLAYED && s_runs[other]==1u);
            assert(ExpandedUniversalLive_ViewRoundCpuResult(
                s_cat[other].fixture.event_id)==1); /* already played */
            assert(s_runs[other]==1u);
            /* board CPU result is RAM batch: the durable CAR doesn't hold it
             * yet (next checkpoint); re-save to make the comparison exact */
            assert(Expanded_AutoCareer_SaveChecked("test-menu-exit"));
        }
        assert(count_state(EXP_FIXTURE_PLAYED)==PER_ROUND);
        /* durable: the CAR on disk already holds it as PLAYED */
        snapshot_scores(scoresA,stA);
        assert(continue_from_disk());
        snapshot_scores(scoresB,stB);
        assert(memcmp(stA,stB,sizeof stA)==0);
        for(k=0;k<NFIX;++k)assert(ExpandedFixtureScore_Equal(scoresA[k],scoresB[k]));
        assert(ExpandedUniversalLive_ViewManagerResult()==1); /* not due: no 2nd */
        assert(s_runs[mk]==1u);
    }
    check_standings(0u);
    puts("CASE5_MANAGER_VIEW_RESULT_QUICK_DURABLE_ONCE=PASS");

    /* ===== 6. manager PLAY = OG 50 Hz, CLAIMED before, COMPLETED after ===== */
    assert(ExpandedUniversalLive_Advance(END())==1);
    mk=manager_fixture_of_round(1u,1u);
    assert(ExpandedUniversalLive_ManagerNext(&due)==2);
    {
        const unsigned c0=s_claim_saves,p0=s_complete_saves;
        assert(ExpandedUniversalLive_PlayManager()==2);
        assert(state_of((uint32_t)mk)==EXP_FIXTURE_PLAYED && s_runs[mk]==1u);
        assert(s_claim_saves==c0+1u && s_complete_saves==p0+1u && s_50hz==1u);
    }
    check_standings(0u);
    puts("CASE6_MANAGER_PLAY_50HZ_CLAIMED_THEN_COMPLETED=PASS");

    /* ===== ESC abort in 50 Hz (v284 OG behaviour): back to SCHEDULED ===== */
    assert(ExpandedUniversalLive_Advance(END())==1);
    mk=manager_fixture_of_round(2u,1u);
    s_abort_next=1;
    assert(ExpandedUniversalLive_PlayManager()==1);
    assert(state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED && s_ok_runs[mk]==0u);
    assert(s_release_saves==1u && R()->recovery_required==0u);
    assert(continue_from_disk() && state_of((uint32_t)mk)==EXP_FIXTURE_SCHEDULED);
    assert(ExpandedUniversalLive_ManagerNext(&due)==2);
    assert(ExpandedUniversalLive_PlayManager()==2 && s_ok_runs[mk]==1u);
    puts("CASE6b_50HZ_ESC_ABORT_RELEASED_THEN_PLAYED_ONCE=PASS");

    /* ===== 11. CPU backlog after last durable CAR + crash + CONTINUE ===== */
    {
        const int32_t durable_day=R()->scheduler.clock.day_index;
        const unsigned ip0=s_interactive_prepares,pr0=s_prompts,hz0=s_50hz;
        uint32_t played_durable=count_state(EXP_FIXTURE_PLAYED);
        /* sack the manager: FORWARD runs the rest as CPU in RAM only */
        _youreSacked=1u; /* (case 8) old club is NOT the manager any more */
        assert(ExpandedUniversalLive_ManagerNext(&due)==-1);
        assert(ExpandedUniversalLive_ManagerBound()==0);
        assert(ExpandedUniversalLive_Advance(END())==1);
        assert(count_state(EXP_FIXTURE_PLAYED)==NFIX);
        assert(s_interactive_prepares==ip0 && s_prompts==pr0 && s_50hz==hz0);
        for(k=0;k<NFIX;++k)assert(s_ok_runs[k]==1u);
        /* crash: RAM lost, previous CAR restored, backlog is SCHEDULED
         * again (allowed to differ), durable results identical. */
        snapshot_scores(scoresA,stA);
        assert(continue_from_disk());
        assert(R()->scheduler.clock.day_index==durable_day);
        assert(count_state(EXP_FIXTURE_PLAYED)==played_durable);
        snapshot_scores(scoresB,stB);
        for(k=0;k<NFIX;++k)if(stB[k]==EXP_FIXTURE_PLAYED)
            assert(stA[k]==EXP_FIXTURE_PLAYED &&
                   ExpandedFixtureScore_Equal(scoresA[k],scoresB[k]));
        assert(ExpandedUniversalLive_Advance(END())==1);
        assert(count_state(EXP_FIXTURE_PLAYED)==NFIX);
        for(k=0;k<NFIX;++k)
            assert(stB[k]==EXP_FIXTURE_PLAYED?s_ok_runs[k]==1u:s_ok_runs[k]==2u);
    }
    check_standings(0u);check_standings(1u);
    puts("CASE8_SACKED_MANAGER_IS_UNEMPLOYED_FOR_FORWARD=PASS");
    puts("CASE11_CONTINUE_DURABLE_IDENTICAL_BACKLOG_RECOMPUTED_NO_DUP=PASS");

    /* ===== 8b. club change mid-season: manager rebinding only ===== */
    ExpandedUniversal_CarResetCurrent();
    build_catalog(0u);
    employ(1u);
    start_world();
    assert(ExpandedUniversalLive_Advance(END())==1);
    mk=manager_fixture_of_round(0u,1u);
    assert(ExpandedUniversalLive_ViewManagerResult()==2);
    {
        const uint32_t played=count_state(EXP_FIXTURE_PLAYED);
        const uint32_t fc=R()->fixture_count;
        snapshot_scores(scoresA,stA);
        employ(6u); /* new club in the OTHER league: no world rebuild */
        assert(ExpandedUniversalLive_ManagerBound()==1);
        assert(R()->fixture_count==fc && count_state(EXP_FIXTURE_PLAYED)==played);
        snapshot_scores(scoresB,stB);
        assert(memcmp(stA,stB,sizeof stA)==0);
        assert(ExpandedUniversalLive_Advance(END())==1);
        k=(uint32_t)manager_fixture_of_round(1u,6u);
        assert(state_of(k)==EXP_FIXTURE_SCHEDULED);
        assert(ExpandedUniversalLive_ManagerNext(&due)==2);
        /* the old club 1 plays as CPU now; club 6 waits for the manager */
        assert(ExpandedUniversalLive_ViewManagerResult()==2 && state_of(k)==EXP_FIXTURE_PLAYED);
        for(k=0;k<NFIX;++k)assert(s_ok_runs[k]<=1u);
    }
    puts("CASE8b_CLUB_CHANGE_REBINDS_ONLY_NO_RESET=PASS");

    /* ===== 10. CLAIMED in CAR (crash during 50 Hz): current behaviour ===== */
    assert(ExpandedUniversalLive_Advance(END())==1);
    assert(ExpandedUniversalLive_ManagerNext(&due)==2);
    s_crash_next=1;
    {
        const unsigned c0=s_claim_saves;
        assert(ExpandedUniversalLive_PlayManager()==1);
        assert(s_claim_saves==c0+1u && R()->recovery_required==1u);
    }
    /* negative: recovery blocks FORWARD/PLAY/saves, never replays */
    assert(ExpandedUniversalLive_Advance(END())==0);
    assert(ExpandedUniversalLive_ManagerNext(&due)==-1);
    assert(count_state(EXP_FIXTURE_CLAIMED)==1u);
    {
        /* the interrupted fixture: the only CLAIMED one */
        uint32_t ck=NFIX;
        for(k=0;k<NFIX;++k)if(state_of(k)==EXP_FIXTURE_CLAIMED)ck=k;
        assert(ck<NFIX && s_ok_runs[ck]==0u);
        /* EXIT + CONTINUE (user decision "come ESC"): the CAR holds CLAIMED,
         * restore puts it back to SCHEDULED, explicitly, without running it */
        assert(continue_from_disk());
        assert(R()->recovery_required==0u && count_state(EXP_FIXTURE_CLAIMED)==0u);
        assert(state_of(ck)==EXP_FIXTURE_SCHEDULED && s_ok_runs[ck]==0u);
        assert(ExpandedUniversalLive_ManagerNext(&due)==2);
        /* the manager decides again: VIEW RESULT this time, once */
        assert(ExpandedUniversalLive_ViewManagerResult()==2);
        assert(state_of(ck)==EXP_FIXTURE_PLAYED && s_ok_runs[ck]==1u);
        assert(continue_from_disk() && state_of(ck)==EXP_FIXTURE_PLAYED);
        assert(ExpandedUniversalLive_Advance(END())==1);
        for(k=0;k<NFIX;++k)assert(s_ok_runs[k]<=1u);
    }
    puts("CASE10_CLAIMED_IN_CAR_RELEASED_ON_CONTINUE_PLAYED_ONCE=PASS");

    /* ===== negatives: corrupted CAR / failed completion save ===== */
    {
        char saved[1<<10];
        memcpy(saved,s_disk+100,sizeof saved);
        memset(s_disk+100,'Z',64);
        assert(!continue_from_disk());
        memcpy(s_disk+100,saved,sizeof saved);
    }
    ExpandedUniversal_CarResetCurrent();
    build_catalog(0u);
    employ(1u);
    start_world();
    assert(ExpandedUniversalLive_Advance(END())==1);
    mk=manager_fixture_of_round(0u,1u);
    s_fail_saves=1;
    assert(ExpandedUniversalLive_ViewManagerResult()==1);
    s_fail_saves=0;
    assert(R()->recovery_required==1u); /* never COMPLETED without durable CAR */
    assert(state_of((uint32_t)mk)==EXP_FIXTURE_CLAIMED);
    puts("NEG_COMPLETION_SAVE_FAILED_NOT_COMPLETED=PASS");

    /* ===== KO guard: a type1 fixture is refused, never a winnerless draw ===== */
    ExpandedUniversal_CarResetCurrent();
    build_catalog(1u);
    unemploy();
    start_world();
    assert(ExpandedUniversalLive_Advance(END())==0);
    assert(s_runs[0]==0u && state_of(0u)==EXP_FIXTURE_SCHEDULED);
    puts("NEG_TYPE1_WITHOUT_KO_HANDLER_REFUSED_BEFORE_CLAIM=PASS");

    /* ===== missing roster: explicit fault, nothing played ===== */
    ExpandedUniversal_CarResetCurrent();
    build_catalog(0u);
    unemploy();
    start_world();
    s_missing_roster=(int)s_cat[0].fixture.home.stable_club;
    assert(ExpandedUniversalLive_Advance(END())==0);
    /* league B ran first that day; the broken fixture is NOT played and
     * nothing after it either (global recovery: documented limit). */
    assert(count_state(EXP_FIXTURE_PLAYED)==2u && s_runs[0]==0u);
    assert(state_of(0u)==EXP_FIXTURE_SCHEDULED && R()->recovery_required==1u);
    s_missing_roster=0;
    puts("NEG_MISSING_ROSTER_FAILS_CLOSED=PASS");

    /* ===== WORLD9697: CE inactive, nothing started ===== */
    ExpandedUniversal_CarResetCurrent();
    s_world=WORLD_9697;
    assert(ExpandedUniversalLive_Start()==1 && !ExpandedUniversalLive_Active());
    assert(ExpandedUniversalLive_ManagerBound()==0);
    s_world=WORLD_2627;
    puts("WORLD9697_CE_NOT_STARTED_OG_NATIVE_UNCHANGED=PASS (no 9697 CE data)");

    puts("FASE1_LIVE_CYCLE_ISOLATED=PASS");
    puts("OG_QUICK_50HZ_MENUS_BOARD_WINDOWS_GAMEPLAY=NOT_TESTED");
    return 0;
}
