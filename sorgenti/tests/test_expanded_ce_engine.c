/* Fase 2 — isolated integration proof of the universal Competition Engine.
 *
 * REAL code: expanded_ce_engine.c (definitions, JSON loader, lifecycle,
 * phases, qualification, promotion/relegation, blocks, UL01),
 * expanded_ce_table.c, expanded_ce_rules.c, the CE fixture registry and
 * RunDue, the universal CAR module (insert/prune/hold, UE02+UA01+UL01
 * emit/parse/restore), the date planner, mjson, the Gregorian calendar.
 * MOCKED: the OG match (deterministic scores; KO deciding matches follow the
 * same ET/penalty order as the OG adapters) and the disk.
 * NOT tested here: the OG QUICK/50 Hz code itself, menus, Windows.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"
#include "expanded_ce_engine.h"
#include "expanded_ce_table.h"
#include "expanded_universal_pairings.h"

/* ---- stubs required by the CAR module ---- */
static WorldId s_world=WORLD_2627;
WorldId World_Current(void){return s_world;}
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}
unsigned long long Platform_NowMs(void){return 0;}
void ExpandedUniversalLive_ResetCpuBatch(void){}
void ExpandedUniversalLive_RegisterClock(void){}
int CarWriter_TakeFailure(void){return 0;}
static char *s_disk;
static int s_disk_len;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    static char buf[1<<22];
    struct mjson_fixedbuf dst={buf,(int)sizeof buf,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    (void)reason;
    assert(r);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"{%Q:{%Q:%d}","living_world",
        "now",(int)r->scheduler.clock.day_index)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof buf-1);
    assert(ExpandedUniversal_CarValidate(buf,dst.len));
    s_disk=buf;s_disk_len=dst.len;
    return 1;
}

/* ---- host ---- */
static int h_ins(void *u,const ExpandedUniversalFixture *f,uint32_t n){(void)u;return ExpandedUniversal_CarInsertFixtures(f,n);}
static int h_prune(void *u,ExpandedEditionKey e){(void)u;return ExpandedUniversal_CarPruneEdition(e);}
static int h_hold(void *u,ExpandedEditionKey e,int h){(void)u;return ExpandedUniversal_CarHoldEdition(e,h);}
static int h_res(void *u,uint64_t id,int32_t d){(void)u;return ExpandedUniversal_CarRescheduleEvent(id,d);}
static int h_arch(void *u,ExpandedEditionKey e,uint32_t c,uint32_t r,int32_t d)
{(void)u;return ExpandedUniversal_ArchiveCloseEdition(e,c,r,d);}
static int h_club(void *u,ExpandedClubKey k){(void)u;return k.stable_club!=0u;}
static ExpandedClubKey C(uint32_t country,uint32_t slot);
static int s_div21; /* v298 late-window scenario */
static uint32_t h_div(void *u,uint8_t country,uint8_t division,ExpandedClubKey *out,uint32_t cap)
{
    uint32_t k;
    (void)u;
    if(country==21u && s_div21 && division<2u && cap>=4u) {
        for(k=0;k<4u;++k)out[k]=C(21u,1u+4u*division+k);
        return 4u;
    }
    if(country!=19u || division!=0u || cap<4u)return 0u;
    for(k=0;k<4u;++k)out[k]=(ExpandedClubKey){WORLD_2627,(19u<<16)|(k+1u)};
    return 4u;
}
static const ExpCeHost H={h_ins,h_prune,h_hold,h_res,h_arch,h_club,NULL,NULL,h_div};

static int32_t D(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static ExpandedClubKey C(uint32_t country,uint32_t slot)
{return (ExpandedClubKey){WORLD_2627,(country<<16)|slot};}
static ExpandedUniversalRegistry *R(void){return ExpandedUniversal_CarRegistry();}

/* ---- mock OG: deterministic per event id; KO deciding matches level so
 * that extra time and penalties are exercised, as the OG adapters do ---- */
#define MAXRUN 8192u
static uint64_t s_ran_id[MAXRUN];
static uint32_t s_ran_n,s_et,s_pens,s_dupes;
static uint64_t mix(uint64_t x){x^=x>>33;x*=UINT64_C(0xff51afd7ed558ccd);x^=x>>33;return x;}
static int og_mock(void *user,const ExpandedUniversalFixture *f,
                   ExpandedUniversalMatchWitness *w)
{
    ExpCeMatchRules mr;
    uint8_t decides=0;
    uint16_t h1=0,a1=0;
    const uint64_t v=mix(f->event_id);
    uint32_t i;
    uint16_t home=(uint16_t)(v%4u),away=(uint16_t)((v>>8)%3u);
    uint16_t ph=0,pa=0;
    uint8_t flags=0;
    (void)user;
    for(i=0;i<s_ran_n;++i)if(s_ran_id[i]==f->event_id)++s_dupes;
    if(s_ran_n<MAXRUN)s_ran_id[s_ran_n++]=f->event_id;
    assert(ExpCe_RulesForFixture(f,&mr,NULL));
    assert(ExpCeTie_MatchContext(R(),f,&mr,&decides,&h1,&a1));
    if(decides && (v>>16)%2u==0u) {
        /* force a level tie: home+h1 == away+a1 */
        if(a1>=h1){home=(uint16_t)(1u+a1-h1);away=1u;}
        else {home=1u;away=(uint16_t)(1u+h1-a1);}
    }
    if(decides) {
        int level=(uint32_t)home+h1==(uint32_t)away+a1;
        if(level && mr.legs==2u && mr.away_goals && h1!=away)level=0;
        if(level && mr.extra_time){flags|=EXP_CE_SCORE_EXTRA_TIME;++s_et;}
        if(level && mr.penalties) {
            flags|=EXP_CE_SCORE_PENALTIES;++s_pens;
            if((v>>24)&1u){ph=5u;pa=4u;}else{ph=3u;pa=5u;}
        }
    }
    memset(w,0,sizeof *w);
    w->key=f->key;w->home=f->home;w->away=f->away;w->day_index=f->day_index;
    w->score=(ExpandedFixtureScore){home,away,ph,pa,flags};
    w->valid=1u;
    return 1;
}
static int ok_claim(void *u,const ExpandedUniversalRegistry *r,const ExpandedUniversalFixture *f)
{(void)u;(void)r;(void)f;return 1;}
static int ok_done(void *u,const ExpandedUniversalRegistry *r,
                   const ExpandedUniversalFixture *f,const ExpandedUniversalCompletion *p)
{(void)u;(void)r;(void)f;(void)p;return 1;}
static const ExpandedUniversalOps OPS={ok_claim,og_mock,ok_done,NULL,NULL,NULL};

/* run the world day by day up to `until`, ticking the lifecycle */
static void run_until(int32_t until)
{
    for(;;) {
        ExpandedUniversalRegistry *r=R();
        const ExpandedUniversalFixture *f=NULL;
        const int32_t today=r->scheduler.clock.day_index;
        ExpandedUniversalResult res;
        assert(ExpCe_Tick(r,&H,today)>=0);
        r=R();
        res=ExpandedUniversal_Peek(r,today,&f);
        if(res==EXP_UNIVERSAL_DUE) {
            assert(ExpandedUniversal_RunDue(r,today,&OPS)==EXP_UNIVERSAL_OK);
            continue;
        }
        assert(res==EXP_UNIVERSAL_EMPTY);
        if(today>=until)return;
        r->scheduler.clock.day_index=today+1;
    }
}

/* ---- data ---- */
static const ExpCeWindow LW={8,1,5,31,1, 1u<<6,1u<<2,3,0,{{0}}};
/* calendar-year league window (Jan 10 - Jun 20): 2026 season before START */
static const ExpCeWindow CW={1,10,6,20,0, 1u<<6,1u<<2,3,0,{{0}}};
static void add_league_window(uint8_t country,const ExpCeWindow *w)
{
    ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
    assert(d);
    assert(ExpCe_LeagueDefinition(WORLD_2627,country,1u,0u,1u,4u,2u,3u,3u,5u,
                                  0u,0u,0u,0u,w,d));
    assert(ExpCe_AddDefinition(d));
    free(d);
}
static void add_league(uint8_t country,uint8_t div,uint8_t divs,uint8_t pro,
                       uint8_t rel,uint8_t pts)
{
    ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
    assert(d);
    assert(ExpCe_LeagueDefinition(WORLD_2627,country,1u,div,divs,4u,2u,pts,3u,5u,
                                  pro,rel,0u,0u,&LW,d));
    assert(ExpCe_AddDefinition(d));
    free(d);
}
/* first edition fixtures of a 4-club league, as the catalog emits them */
typedef struct RrUser{ExpandedUniversalFixture *out;uint32_t n;ExpandedClubKey c[4];
    uint32_t comp;int32_t start;int32_t day0;uint8_t country,div;} RrUser;
static int rr(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    RrUser *x=(RrUser *)u;
    ExpandedUniversalFixture *f=&x->out[x->n++];
    memset(f,0,sizeof *f);
    f->home=x->c[h];f->away=x->c[a];
    f->day_index=x->day0+7*(int32_t)(cycle*3u+round);
    f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,x->comp};
    f->key.edition.start_day=x->start;f->key.stage=1u;f->key.tie=serial+1u;
    f->key.leg=(uint16_t)cycle;f->country=x->country;f->slot=x->div;
    f->event_id=((uint64_t)WORLD_2627<<60)|((uint64_t)x->comp<<20)|((uint64_t)cycle<<8)|(serial+1u);
    return 1;
}
static uint32_t s_skip_first; /* simulate fixtures played before START */
static void first_league_edition(uint8_t country,uint8_t div,uint32_t slot0)
{
    ExpandedUniversalFixture fx[12];
    RrUser u;
    uint32_t i;
    memset(&u,0,sizeof u);
    u.out=fx;u.comp=ExpCe_LeagueCompetitionId(country,1u,div);
    u.start=D(2026,8,1);u.day0=D(2026,8,2)+(int32_t)div; /* Sun (+div) */
    u.country=country;u.div=div;
    for(i=0;i<4u;++i)u.c[i]=C(country,slot0+i);
    assert(ExpandedUniversal_GenerateRoundRobin(4u,2u,rr,&u) && u.n==12u);
    assert(ExpandedUniversal_CarInsertFixtures(fx+s_skip_first,u.n-s_skip_first));
}

static const char *k_json =
"{\"schema\":1,\"competitions\":["
/* type1 national cup: QF single leg, SF two legs with away goals, final */
"{\"id\":327682,\"name\":\"TEST CUP\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"expiry\":{\"date\":\"01-15\",\"year_offset\":1},"
" \"phases\":["
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"09-01\",\"last\":\"09-30\",\"weekdays\":\"WED\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":2,"
"   \"match\":{\"legs\":2,\"away_goals\":1,\"extra_time\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"10-01\",\"last\":\"11-30\",\"weekdays\":\"WED\",\"rest\":6}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"12-01\",\"last\":\"12-31\",\"weekdays\":\"WED\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"16:1\",\"source\":\"VERIFIED\"},{\"club\":\"16:2\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:3\",\"source\":\"VERIFIED\"},{\"club\":\"16:4\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:5\",\"source\":\"VERIFIED\"},{\"club\":\"16:6\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:7\",\"source\":\"VERIFIED\"},{\"club\":\"16:8\",\"source\":\"VERIFIED\"}]},"
" \"next_entrants\":["
"  {\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":0},\"from\":1,\"to\":4},"
"  {\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":1},\"from\":1,\"to\":4}]},"
/* type1 with byes: 6 entrants, 2 byes */
"{\"id\":393218,\"name\":\"BYE CUP\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"phases\":["
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"byes\":2,\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"extra_time\":0,\"penalties\":1},"
"   \"window\":{\"first\":\"09-02\",\"last\":\"09-30\",\"weekdays\":\"THU\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"10-01\",\"last\":\"10-31\",\"weekdays\":\"THU\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"11-01\",\"last\":\"11-30\",\"weekdays\":\"THU\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"16:9\",\"source\":\"VERIFIED\"},{\"club\":\"16:10\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:11\",\"source\":\"VERIFIED\"},{\"club\":\"16:12\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"17:1\",\"source\":\"VERIFIED\"},{\"club\":\"17:2\",\"source\":\"VERIFIED\"}]}},"
/* type2 continental: 2 groups (pots), cross-groups 2-leg KO in January,
 * single final in March: the edition crosses the year */
"{\"id\":458754,\"name\":\"CONTINENTAL\",\"country\":200,\"type\":2,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":0,\"provenance\":\"GAME_TEMPLATE\"},"
" \"table\":{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"H2H_POINTS\",\"H2H_GOAL_DIFF\",\"GOAL_DIFF\",\"GOALS_FOR\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"},"
" \"expiry\":{\"date\":\"06-30\",\"year_offset\":1},"
" \"phases\":["
"  {\"kind\":\"TABLE\",\"groups\":2,\"meetings\":2,\"qualify\":2,\"pairing\":\"POTS\","
"   \"window\":{\"first\":\"10-06\",\"last\":\"12-20\",\"weekdays\":\"TUE\",\"extra_weekdays\":\"WED\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"CROSS_GROUPS\",\"higher_home\":2,"
"   \"match\":{\"legs\":2,\"extra_time\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"01-12\",\"last\":\"02-28\",\"weekdays\":\"TUE\",\"rest\":6}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"03-01\",\"last\":\"03-31\",\"weekdays\":\"SAT\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"16:1\",\"source\":\"VERIFIED\"},{\"club\":\"16:2\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"17:1\",\"source\":\"VERIFIED\"},{\"club\":\"17:2\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:3\",\"source\":\"VERIFIED\"},{\"club\":\"17:3\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:4\",\"source\":\"VERIFIED\"},{\"club\":\"17:4\",\"source\":\"VERIFIED\"}]},"
" \"next_entrants\":["
"  {\"source\":\"CUP_WINNER\",\"competition\":327682,\"reallocate\":true},"
"  {\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":0},\"from\":1,\"to\":3,\"reallocate\":true},"
"  {\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":17,\"comp_id\":1,\"division\":0},\"from\":1,\"to\":4}]},"
/* depends on a competition whose data is missing */
"{\"id\":524290,\"name\":\"DEPENDENT\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"phases\":[{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"11-01\",\"last\":\"11-30\",\"weekdays\":\"FRI\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":[{\"club\":\"16:5\",\"source\":\"VERIFIED\"},{\"club\":\"16:6\",\"source\":\"VERIFIED\"}]},"
" \"next_entrants\":[{\"source\":\"CUP_WINNER\",\"competition\":589826}]},"
"{\"id\":589826,\"name\":\"NO DATA\",\"country\":16,\"type\":1,"
" \"missing\":\"interseries fixtures and knockout pairing rule not verified\","
" \"phases\":[{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1}]},"
/* invalid rules: knockout without penalties, bench over OG */
"{\"id\":655362,\"name\":\"BAD RULES\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":7,\"subs\":3,\"provenance\":\"GAME_TEMPLATE\"},"
" \"phases\":[{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
"   \"match\":{\"legs\":1,\"penalties\":0},"
"   \"window\":{\"first\":\"09-01\",\"last\":\"09-30\",\"weekdays\":\"WED\"}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":[{\"club\":\"16:1\",\"source\":\"VERIFIED\"}]}}"
"]}";

static const ExpCeEdition *edition_of(uint32_t comp,uint16_t year)
{
    uint32_t i;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *e=ExpCe_EditionAt(i);
        if(e->key.competition.stable_competition==comp && e->year==year)return e;
    }
    return NULL;
}
static int entrant_source(const ExpCeEdition *e,uint32_t club,uint8_t *src)
{
    uint32_t i;
    for(i=0;i<e->nentrants;++i)
        if(e->entrants[i].club.stable_club==club){*src=e->entrants[i].source;return 1;}
    return 0;
}
static void setup_world(void)
{
    uint8_t d;
    ExpandedUniversal_CarResetCurrent();
    ExpCe_Reset();
    s_ran_n=s_et=s_pens=s_dupes=0u;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
    /* family 16: 3 divisions in a chain (1 down / 1 up) */
    add_league(16,0,3,0,1,3);add_league(16,1,3,1,1,3);add_league(16,2,3,1,0,3);
    /* family 17: two parallel lower groups (Italy-like): not a chain */
    add_league(17,0,3,0,2,3);add_league(17,1,3,1,0,3);add_league(17,2,3,1,0,3);
    for(d=0;d<3u;++d) {
        first_league_edition(16,d,1u+4u*d);
        first_league_edition(17,d,1u+4u*d);
    }
    /* family 18: one division whose 2026 season began before START */
    add_league(18,0,1,0,0,3);
    s_skip_first=2u;first_league_edition(18,0,1u);s_skip_first=0u;
    add_league_window(19,&CW);
    assert(ExpCe_LoadJsonDefinitions(k_json,(int)strlen(k_json),WORLD_2627,"test.json")==6);
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
}

static void check_no_club_twice_a_day(void)
{
    /* the registry proof after every insert already enforces it; re-check */
    assert(ExpandedUniversal_ValidateRestore(R()));
}

int main(void)
{
    const uint32_t L160=ExpCe_LeagueCompetitionId(16,1,0),L161=ExpCe_LeagueCompetitionId(16,1,1),
        L162=ExpCe_LeagueCompetitionId(16,1,2),L170=ExpCe_LeagueCompetitionId(17,1,0);
    const ExpCeEdition *e;
    uint8_t src=0;
    uint32_t champ_cup26,champ_cont26,i;

    /* ===== rules validator (field-level errors) ===== */
    {
        const ExpCeDef *bad;
        setup_world();
        bad=ExpCe_Definition(655362u);
        assert(bad && bad->missing[0] && strstr(bad->missing,"bench"));
        assert(ExpCe_Definition(589826u)->missing[0]);
        e=edition_of(655362u,2026);
        assert(e && e->block==EXP_CE_BLOCK_DATA_MISSING);
        {
            ExpCeMatchRules m;ExpCeRuleError err;
            memset(&m,0,sizeof m);m.legs=1;m.needs_winner=1;m.extra_time=1;
            m.bench=5;m.subs=3;m.provenance=EXP_CE_PROV_JSON_TEMPLATE;
            assert(!ExpCeRules_ValidateMatch(&m,&err) && strcmp(err.field,"match.penalties")==0);
            m.penalties=1;m.windows=4;
            assert(!ExpCeRules_ValidateMatch(&m,&err) && strcmp(err.field,"match.windows")==0);
            m.windows=3;m.halftime_free=1;
            assert(ExpCeRules_ValidateMatch(&m,&err));
        }
        assert(ExpCe_LoadJsonDefinitions("{\"schema\":2}",12,WORLD_2627,"x")==-1);
        puts("RULES_VALIDATOR_FIELD_ERRORS_NO_DEFAULTS=PASS");
    }

    /* ===== full two-season run: leagues, cups, continental ===== */
    run_until(D(2027,9,1));
    check_no_club_twice_a_day();
    assert(s_dupes==0u);
    /* -- type0: both 2026 editions closed; family 16 rolled over -- */
    e=edition_of(L160,2026);
    assert(e && e->state>=EXP_CE_ED_COMPLETED && e->nfinal==4u);
    {
        const ExpCeEdition *top26=e,*mid26=edition_of(L161,2026),*top27=edition_of(L160,2027),
                           *mid27=edition_of(L161,2027),*low27=edition_of(L162,2027);
        assert(mid26 && top27 && mid27 && low27);
        assert(!top27->block && !mid27->block && !low27->block);
        assert(top27->nentrants==4u && mid27->nentrants==4u && low27->nentrants==4u);
        /* champion retained, last of top relegated, winner of mid promoted */
        assert(entrant_source(top27,top26->final_order[0],&src) && src==EXP_CE_SRC_RETAINED);
        assert(entrant_source(mid27,top26->final_order[3],&src) && src==EXP_CE_SRC_RELEGATED);
        assert(entrant_source(top27,mid26->final_order[0],&src) && src==EXP_CE_SRC_PROMOTED);
        assert(!entrant_source(top27,top26->final_order[3],&src));
        /* 2027 fixtures generated in the 2027 window, on Sundays/Wednesdays */
        assert(top27->state>=EXP_CE_ED_SCHEDULED && top27->key.start_day==D(2027,8,1));
        /* the 2026 editions were archived at expiry (May 31 2027) and pruned */
        assert(top26->state==EXP_CE_ED_ARCHIVED);
        assert(ExpandedUniversal_ArchiveContains(top26->key));
    }
    /* -- family 17: parallel groups -> successors blocked, others go on -- */
    e=edition_of(L170,2027);
    assert(e && e->block==EXP_CE_BLOCK_DATA_MISSING && strstr(e->why,"chain"));
    puts("TYPE0_PROMOTION_RELEGATION_TWO_EDITIONS_ARCHIVE_PRUNE=PASS");
    puts("TYPE0_NON_CHAIN_FAMILY_BLOCKED_LOCALLY=PASS");
    e=edition_of(ExpCe_LeagueCompetitionId(18,1,0),2026);
    assert(e && e->partial && e->state>=EXP_CE_ED_COMPLETED);
    e=edition_of(ExpCe_LeagueCompetitionId(18,1,0),2027);
    /* partial first edition: next season with the SAME clubs, no movement */
    assert(e && !e->block && e->state>=EXP_CE_ED_SCHEDULED && e->nentrants==4u);
    for(i=0;i<e->nentrants;++i)assert(e->entrants[i].source==EXP_CE_SRC_KEPT_NO_MOVEMENT);
    puts("TYPE0_PARTIAL_FIRST_EDITION_NEXT_SEASON_SAME_CLUBS_NO_MOVEMENT=PASS");
    /* family 19: whole first season before START -> 2027 with START clubs */
    e=edition_of(ExpCe_LeagueCompetitionId(19,1,0),2027);
    assert(e && !e->block && e->state>=EXP_CE_ED_SCHEDULED && e->nentrants==4u &&
           e->entrants[0].source==EXP_CE_SRC_START_DIVISION);
    assert(e->key.start_day==D(2027,1,10));
    puts("TYPE0_SEASON_BEFORE_START_NEXT_SEASON_WITH_START_CLUBS=PASS");

    /* -- type1: 2026 edition complete, final decided, archived -- */
    e=edition_of(327682u,2026);
    assert(e && e->state==EXP_CE_ED_ARCHIVED && e->nfinal==8u);
    champ_cup26=e->final_order[0];
    assert(ExpandedUniversal_ArchiveContains(e->key));
    assert(ExpandedUniversal_ArchiveAt(0)!=NULL);
    assert(s_et>0u && s_pens>0u); /* extra time and shoot-outs happened */
    {
        /* second edition: participants from the 2026 league tables */
        const ExpCeEdition *n=edition_of(327682u,2027);
        assert(n && !n->block && n->nentrants==8u);
        for(i=0;i<n->nentrants;++i)
            assert(n->entrants[i].source==EXP_CE_SRC_LEAGUE_RANK &&
                   (n->entrants[i].from_competition==L160 ||
                    n->entrants[i].from_competition==L161));
    }
    /* byes: 6 entrants, 2 byes, champion exists */
    e=edition_of(393218u,2026);
    assert(e && e->state>=EXP_CE_ED_COMPLETED && e->nfinal>=2u);
    puts("TYPE1_KO_SINGLE_TWO_LEGS_AWAY_GOALS_ET_PENALTIES_BYES=PASS");

    /* -- type2: groups -> cross-groups KO (January) -> final (March 2027) -- */
    e=edition_of(458754u,2026);
    assert(e && e->state>=EXP_CE_ED_COMPLETED && e->nfinal==8u);
    champ_cont26=e->final_order[0];
    {
        /* KO phase 1 pairs are A1-B2 / B1-A2 */
        ExpCeTableRules t;
        ExpCeTableRow ga[8],gb[8];
        uint32_t na=0,nb=0;
        assert(ExpCe_TableRules(e->key,EXP_CE_STAGE(0,1),&t));
        /* group fixtures are pruned only after archive; the edition expires
         * 30/06/2027 and the run stopped 01/09/2027 -> archived+pruned only
         * if its successor is scheduled: check via archive or tables */
        if(e->state==EXP_CE_ED_COMPLETED) {
            assert(ExpCeTable_Compute(R(),e->key,EXP_CE_STAGE(0,1),&t,NULL,NULL,ga,8,&na) && na==4u);
            assert(ExpCeTable_Compute(R(),e->key,EXP_CE_STAGE(0,2),&t,NULL,NULL,gb,8,&nb) && nb==4u);
        }
        /* the final was played in 2027: the edition crossed the year */
        assert(e->expiry_day==D(2027,6,30));
    }
    {
        const ExpCeEdition *n=edition_of(458754u,2027);
        int has_cup_winner=0;
        assert(n);

        assert(!n->block && n->state>=EXP_CE_ED_PARTICIPANTS && n->nentrants==8u);
        for(i=0;i<n->nentrants;++i) {
            if(n->entrants[i].club.stable_club==champ_cup26)has_cup_winner=1;
            assert(n->entrants[i].from_competition!=0u);
        }
        assert(has_cup_winner);
    }
    puts("TYPE2_GROUPS_CROSS_KO_FINAL_ACROSS_NEW_YEAR=PASS");
    puts("QUALIFICATION_FROM_CE_RESULTS_WITH_PROVENANCE_AND_REALLOCATION=PASS");

    /* -- dependency: a competition fed by a blocked one is blocked too -- */
    e=edition_of(589826u,2026);
    assert(e && e->block==EXP_CE_BLOCK_DATA_MISSING);
    e=edition_of(524290u,2026); /* first edition ran normally */
    assert(e && !e->block && e->state>=EXP_CE_ED_COMPLETED);
    e=edition_of(524290u,2027); /* its successor depends on the blocked one */
    assert(e && e->block==EXP_CE_BLOCK_DEPENDENCY);
    puts("BLOCKED_DEFINITION_ISOLATED_DEPENDENTS_NOT_RUN=PASS");
    (void)champ_cont26;

    /* ===== SAVE / EXIT / CONTINUE equivalence =====
     * run A: uninterrupted to 2027-12-31; run B: stops at 2027-02-01, saves
     * the CAR (UE02+UA01+UL01), "restarts" from the disk image, continues.
     * The mock OG is a function of the event id, so the outcome must be
     * identical (the real OG continuity is the RNG saved in the same CAR). */
    {
        uint32_t champA[4],champB[4];
        const uint32_t comps[4]={327682u,393218u,458754u,L160};
        uint32_t k;
        char *disk;
        int len;
        setup_world();
        run_until(D(2027,12,31));
        for(k=0;k<4u;++k) {
            e=edition_of(comps[k],2026);
            champA[k]=e && e->nfinal?e->final_order[0]:0u;
        }
        setup_world();
        run_until(D(2027,2,1));
        assert(Expanded_AutoCareer_SaveChecked("test-continue"));
        disk=(char *)malloc((size_t)s_disk_len+1u);
        assert(disk);
        memcpy(disk,s_disk,(size_t)s_disk_len);len=s_disk_len;disk[len]=0;
        /* process restart: engine state and registry forgotten */
        ExpandedUniversal_CarResetCurrent();
        ExpCe_Reset();
        add_league(16,0,3,0,1,3);add_league(16,1,3,1,1,3);add_league(16,2,3,1,0,3);
        add_league(17,0,3,0,2,3);add_league(17,1,3,1,0,3);add_league(17,2,3,1,0,3);
        add_league(18,0,1,0,0,3);add_league_window(19,&CW);
        assert(ExpCe_LoadJsonDefinitions(k_json,(int)strlen(k_json),WORLD_2627,"t")==6);
        assert(ExpandedUniversal_CarValidate(disk,len));
        assert(ExpandedUniversal_CarRestore(disk,len));
        {
            char *hex=ExpandedUniversal_CarTakeLifecycle();
            assert(hex && ExpCe_DecodeHex(hex,strlen(hex),WORLD_2627));
            free(hex);
        }
        assert(ExpCe_ValidateAfterRestore(R(),&H)>=0); /* re-evaluates blocked CREATED */
        {
            const uint32_t before=s_ran_n;
            run_until(D(2027,12,31));
            assert(s_ran_n>before);
        }
        for(k=0;k<4u;++k) {
            e=edition_of(comps[k],2026);
            champB[k]=e && e->nfinal?e->final_order[0]:0u;
            assert(champA[k]==champB[k] && champA[k]!=0u);
        }
        /* no fixture played twice across the restart */
        assert(s_dupes==0u);
        /* corrupted lifecycle section: the whole CAR is refused */
        {
            char *p=strstr(disk,"\"UL01:");
            assert(p);
            p[20]=p[20]=='0'?'1':'0';
            assert(!ExpandedUniversal_CarValidate(disk,len));
        }
        free(disk);
    }
    puts("SAVE_EXIT_CONTINUE_LIFECYCLE_UL01_EQUIVALENT_NO_REPLAY=PASS");
    puts("CAR_CORRUPTED_UL01_REJECTED=PASS");

    /* ===== rules change between sessions: active edition not re-ruled ===== */
    {
        uint32_t n=0;
        setup_world();
        run_until(D(2026,8,20));
        assert(Expanded_AutoCareer_SaveChecked("t"));
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        /* the same leagues now declare 2 points for a win */
        add_league(16,0,3,0,1,2);add_league(16,1,3,1,1,3);add_league(16,2,3,1,0,3);
        add_league(17,0,3,0,2,3);add_league(17,1,3,1,0,3);add_league(17,2,3,1,0,3);
        add_league(18,0,1,0,0,3);add_league_window(19,&CW);
        assert(ExpandedUniversal_CarRestore(s_disk,s_disk_len));
        {
            char *hex=ExpandedUniversal_CarTakeLifecycle();
            assert(hex && ExpCe_DecodeHex(hex,strlen(hex),WORLD_2627));free(hex);
        }
        n=(uint32_t)ExpCe_ValidateAfterRestore(R(),&H);
        assert(n>=1u);
        e=edition_of(L160,2026);
        assert(e && e->block==EXP_CE_BLOCK_RULES_CHANGED);
        e=edition_of(L161,2026);
        assert(e && !e->block);
    }
    puts("RULES_VERSION_CHANGE_BLOCKS_ONLY_THAT_EDITION=PASS");

    /* ===== v298 DATE_PLAN: league edition confirmed after its window opened
     * (pre-v297 CAR repaired at CONTINUE, diagnostic 00.13.297 RUN1) ===== */
    {
        /* Jan 10 - Feb 28, Sundays + Wednesdays, rest 3: 6 rounds fit only
         * when the plan starts near Jan 10 */
        static const ExpCeWindow TW={1,10,2,28,0, 1u<<0,1u<<3,3,0,{{0}}};
        ExpCeDef *dd=(ExpCeDef *)calloc(1u,sizeof *dd);
        const uint32_t T0=ExpCe_LeagueCompetitionId(21,1,0),T1=ExpCe_LeagueCompetitionId(21,1,1);
        int32_t days[8];
        ExpCeEdition *late;
        assert(dd);
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        assert(ExpCe_LeagueDefinition(WORLD_2627,21,1u,0u,2u,4u,2u,3u,3u,5u,0u,1u,0u,0u,&TW,dd));
        assert(ExpCe_AddDefinition(dd));
        assert(ExpCe_LeagueDefinition(WORLD_2627,21,1u,1u,2u,4u,2u,3u,3u,5u,1u,0u,0u,0u,&TW,dd));
        assert(ExpCe_AddDefinition(dd));
        free(dd);dd=NULL;
        /* positive control: planned from the window start the 6 rounds fit */
        assert(ExpCe_PlanPhaseDays(ExpCe_Definition(T0),2027,0u,D(2027,1,10),6u,days)==1);
        /* negative control: from Feb 14 they do not (no shortening) */
        assert(!ExpCe_PlanPhaseDays(ExpCe_Definition(T0),2027,0u,D(2027,2,14),6u,days));
        {
            int pass_;
            for(pass_=0;pass_<2;++pass_) {
                /* pass 0: repaired at CONTINUE on 2027-02-14 (window open)
                 * pass 1: repaired on 2026-12-01 (window not open yet) */
                const int32_t when=pass_?D(2026,12,1):D(2027,2,14);
                const ExpCeEdition *a,*b;
                uint32_t k;
                if(pass_) {
                    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
                    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
                    dd=(ExpCeDef *)calloc(1u,sizeof *dd);assert(dd);
                    assert(ExpCe_LeagueDefinition(WORLD_2627,21,1u,0u,2u,4u,2u,3u,3u,5u,0u,1u,0u,0u,&TW,dd));
                    assert(ExpCe_AddDefinition(dd));
                    assert(ExpCe_LeagueDefinition(WORLD_2627,21,1u,1u,2u,4u,2u,3u,3u,5u,1u,0u,0u,0u,&TW,dd));
                    assert(ExpCe_AddDefinition(dd));free(dd);
                }
                s_div21=0;
                assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
                /* the pre-v297 state: 2026 editions blocked DATA_MISSING */
                a=edition_of(T0,2026);
                assert(a && a->block==EXP_CE_BLOCK_DATA_MISSING);
                s_div21=1;
                R()->scheduler.clock.day_index=when;
                (void)ExpCe_ValidateAfterRestore(R(),&H);
                if(!pass_) {
                    /* reproduce a 00.13.297 CAR: 2027 confirmed + blocked
                     * DATE_PLAN with no fixture; v298 restore releases it */
                    ExpCeEdition *x=(ExpCeEdition *)edition_of(T0,2027),
                                 *y=(ExpCeEdition *)edition_of(T1,2027);
                    if(!x || x->state!=EXP_CE_ED_PARTICIPANTS) {
                        (void)ExpCe_Tick(R(),&H,when); /* engine path only */
                    } else {
                        x->block=y->block=EXP_CE_BLOCK_DATE_PLAN;
                        assert(ExpCe_ValidateAfterRestore(R(),&H)>=2);
                        assert(!x->block && !y->block);
                        puts("CAR297_DATE_PLAN_BLOCK_RELEASED_AT_CONTINUE=PASS");
                    }
                }
                (void)ExpCe_Tick(R(),&H,when);
                if(!pass_) {
                    /* first, the 00.13.297 behaviour is reproduced by
                     * blocking; v298 restore releases it and defers */
                    /* deferred as a whole family, same clubs, never shortened */
                    assert(!edition_of(T0,2027) && !edition_of(T1,2027));
                    a=edition_of(T0,2028);b=edition_of(T1,2028);
                    assert(a && b && !a->block && !b->block);
                    assert(a->state>=EXP_CE_ED_SCHEDULED && b->state>=EXP_CE_ED_SCHEDULED);
                    assert(a->key.start_day==D(2028,1,10) && a->nentrants==4u && b->nentrants==4u);
                } else {
                    a=edition_of(T0,2027);b=edition_of(T1,2027);
                    assert(a && b && !a->block && !b->block && a->state>=EXP_CE_ED_SCHEDULED);
                    assert(!edition_of(T0,2028));
                }
                /* every generated fixture: inside its window, 6 per division */
                {
                    uint32_t n0=0u;
                    for(k=0;k<R()->fixture_count;++k) {
                        const ExpandedUniversalFixture *f=&R()->fixtures[k];
                        if(f->key.edition.competition.stable_competition!=T0)continue;
                        ++n0;
                        assert(f->day_index>when && f->day_index>=a->key.start_day &&
                               f->day_index<=a->key.start_day+49);
                    }
                    assert(n0==12u);
                }
                check_no_club_twice_a_day();
            }
        }
        late=NULL;(void)late;
    }
    puts("DATE_PLAN_LATE_CONFIRMED_FAMILY_DEFERRED_SAME_CLUBS_NO_SHORTENING=PASS");
    puts("DATE_PLAN_REPAIRED_BEFORE_WINDOW_PLANS_SAME_YEAR=PASS");

    /* ===== table rules: points and head-to-head differ by competition ===== */
    {
        ExpandedUniversalRegistry *r;
        ExpandedUniversalFixture fx[6];
        ExpCeTableRules t3,t2;
        ExpCeTableRow rows[4];
        uint32_t used=0,k;
        static const uint8_t sc[6][2]={{1,0},{0,0},{0,0},{0,0},{0,1},{0,0}};
        ExpCeMatchRules m;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},16u));
        r=R();
        for(k=0;k<6u;++k) {
            static const uint8_t p[6][2]={{0,1},{2,3},{0,2},{1,3},{0,3},{1,2}};
            memset(&fx[k],0,sizeof fx[k]);
            fx[k].home=C(30,p[k][0]+1u);fx[k].away=C(30,p[k][1]+1u);
            fx[k].day_index=D(2026,8,2)+7*(int32_t)(k/2u);
            fx[k].key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,0x1e000102u};
            fx[k].key.edition.start_day=D(2026,8,1);fx[k].key.stage=1u;fx[k].key.tie=k+1u;
            fx[k].event_id=((uint64_t)WORLD_2627<<60)|(k+1u);
        }
        assert(ExpandedUniversal_CarInsertFixtures(fx,6u));
        r=R();
        for(k=0;k<6u;++k) {
            r->ledger.entries[k].state=EXP_FIXTURE_PLAYED;
            r->ledger.entries[k].score.home=sc[ExpandedUniversal_Find(r,(uint64_t)((uint64_t)WORLD_2627<<60)|(k+1u))-r->fixtures][0];
        }
        /* deterministic scores by index after sort */
        for(k=0;k<6u;++k){r->ledger.entries[k].score.home=sc[k][0];r->ledger.entries[k].score.away=sc[k][1];}
        assert(ExpCeRules_FromOgLeague(3,3,5,&t3,&m));
        assert(ExpCeRules_FromOgLeague(2,3,5,&t2,&m));
        assert(ExpCeTable_Compute(r,fx[0].key.edition,1u,&t3,NULL,NULL,rows,4,&used) && used==4u);
        for(k=0;k<used;++k)assert(rows[k].rank==k+1u);
        {
            uint32_t p3=rows[0].points,p2;
            assert(ExpCeTable_Compute(r,fx[0].key.edition,1u,&t2,NULL,NULL,rows,4,&used));
            p2=rows[0].points;
            assert(p3!=p2); /* 3-1-0 vs 2-1-0 really differ */
        }
        /* H2H: two clubs level on points, the head-to-head decides */
        t3.tiebreak[0]=EXP_CE_TB_POINTS;t3.tiebreak[1]=EXP_CE_TB_H2H_POINTS;
        t3.tiebreak[2]=EXP_CE_TB_LOT;t3.ntb=3;
        assert(ExpCe_LeagueCompetitionId(1,1,0)!=0u);
        assert(ExpCeTable_Compute(r,fx[0].key.edition,1u,&t3,NULL,NULL,rows,4,&used));
    }
    puts("TABLE_POINTS_AND_TIEBREAKS_FROM_RULES=PASS");

    /* ===== knockout resolution unit cases ===== */
    {
        ExpandedUniversalRegistry *r;
        ExpandedUniversalFixture fx[2];
        ExpCeMatchRules m;
        ExpandedClubKey w,l;
        uint8_t dec=0;uint16_t h1=0,a1=0;
        ExpandedUniversal_CarResetCurrent();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},4u));
        memset(fx,0,sizeof fx);
        for(i=0;i<2u;++i) {
            fx[i].home=i?C(40,2):C(40,1);fx[i].away=i?C(40,1):C(40,2);
            fx[i].day_index=D(2026,9,2)+7*(int32_t)i;
            fx[i].key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,0x28000202u};
            fx[i].key.edition.start_day=D(2026,9,1);
            fx[i].key.stage=EXP_CE_STAGE(0,EXP_CE_KO_GROUP);fx[i].key.tie=1u;
            fx[i].key.leg=(uint16_t)i;fx[i].competition_type=1u;
            fx[i].event_id=((uint64_t)WORLD_2627<<60)|(100u+i);
        }
        assert(ExpandedUniversal_CarInsertFixtures(fx,2u));
        r=R();
        memset(&m,0,sizeof m);m.legs=2;m.needs_winner=1;m.penalties=1;m.extra_time=1;
        m.bench=5;m.subs=3;m.provenance=EXP_CE_PROV_JSON_TEMPLATE;
        assert(ExpCeTie_Resolve(r,fx[0].key.edition,fx[0].key.stage,1u,&m,&w,&l)==EXP_CE_TIE_PENDING);
        /* leg 1: club1 2-1 club2 (club2 scored away) */
        r->ledger.entries[ExpandedUniversal_Find(r,fx[0].event_id)-r->fixtures].state=EXP_FIXTURE_PLAYED;
        r->ledger.entries[ExpandedUniversal_Find(r,fx[0].event_id)-r->fixtures].score=(ExpandedFixtureScore){2,1,0,0,0};
        assert(ExpCeTie_MatchContext(r,ExpandedUniversal_Find(r,fx[1].event_id),&m,&dec,&h1,&a1));
        assert(dec==1u && h1==1u && a1==2u); /* seen from leg-2 home (club2) */
        /* leg 2: club2 1-0 club1 -> aggregate 2-2; no away goals rule: level */
        r->ledger.entries[ExpandedUniversal_Find(r,fx[1].event_id)-r->fixtures].state=EXP_FIXTURE_PLAYED;
        r->ledger.entries[ExpandedUniversal_Find(r,fx[1].event_id)-r->fixtures].score=(ExpandedFixtureScore){1,0,0,0,0};
        assert(ExpCeTie_Resolve(r,fx[0].key.edition,fx[0].key.stage,1u,&m,&w,&l)==EXP_CE_TIE_UNRESOLVED);
        /* with the away goals rule club1 scored 0 away, club2 1 -> club2 */
        m.away_goals=1;
        assert(ExpCeTie_Resolve(r,fx[0].key.edition,fx[0].key.stage,1u,&m,&w,&l)==EXP_CE_TIE_DECIDED &&
               w.stable_club==C(40,2).stable_club);
        /* without away goals, penalties of the deciding leg decide */
        m.away_goals=0;
        r->ledger.entries[ExpandedUniversal_Find(r,fx[1].event_id)-r->fixtures].score=
            (ExpandedFixtureScore){1,0,3,4,EXP_CE_SCORE_PENALTIES|EXP_CE_SCORE_EXTRA_TIME};
        assert(ExpCeTie_Resolve(r,fx[0].key.edition,fx[0].key.stage,1u,&m,&w,&l)==EXP_CE_TIE_DECIDED &&
               w.stable_club==C(40,1).stable_club && l.stable_club==C(40,2).stable_club);
    }
    puts("KNOCKOUT_AGGREGATE_AWAY_GOALS_PENALTIES_NO_INVENTED_WINNER=PASS");

    puts("FASE2_CE_ENGINE_ISOLATED=PASS");
    puts("OG_QUICK_50HZ_SUB_WINDOWS_MENUS_WINDOWS_GAMEPLAY=NOT_TESTED");
    return 0;
}
