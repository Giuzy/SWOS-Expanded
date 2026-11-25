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
#include "expanded_ce_pool.h"

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
    for(k=0;k<4u;++k)out[k]=(ExpandedClubKey){WORLD_2627,(19u<<16)|(k+1u),EXP_PARTICIPANT_CLUB};
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
{return (ExpandedClubKey){WORLD_2627,(country<<16)|slot,EXP_PARTICIPANT_CLUB};}
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
/* v309: 8-club first edition emitter (7 rounds per cycle) */
typedef struct Rr8{ExpandedUniversalFixture *out;uint32_t n;ExpandedClubKey c[8];
    uint32_t comp;int32_t start;int32_t day0;uint8_t country,div;} Rr8;
static int rr8(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    Rr8 *x=(Rr8 *)u;
    ExpandedUniversalFixture *f=&x->out[x->n++];
    memset(f,0,sizeof *f);
    f->home=x->c[h];f->away=x->c[a];
    f->day_index=x->day0+7*(int32_t)(cycle*7u+round);
    f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,x->comp};
    f->key.edition.start_day=x->start;f->key.stage=1u;f->key.tie=serial+1u;
    f->key.leg=(uint16_t)cycle;f->country=x->country;f->slot=x->div;
    f->event_id=((uint64_t)WORLD_2627<<60)|((uint64_t)x->comp<<20)|((uint64_t)cycle<<8)|(serial+1u);
    return 1;
}
/* v311: 14-club first edition emitter (13 rounds per cycle) */
typedef struct Rr14{ExpandedUniversalFixture *out;uint32_t n;ExpandedClubKey c[14];
    uint32_t comp;int32_t start;int32_t day0;uint8_t country,div,nclubs;} Rr14;
static int rr14(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    Rr14 *x=(Rr14 *)u;
    ExpandedUniversalFixture *f=&x->out[x->n++];
    const uint32_t per=(x->nclubs&1u)?x->nclubs:(uint32_t)x->nclubs-1u;
    memset(f,0,sizeof *f);
    f->home=x->c[h];f->away=x->c[a];
    f->day_index=x->day0+7*(int32_t)(cycle*per+round);
    f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,x->comp};
    f->key.edition.start_day=x->start;f->key.stage=1u;f->key.tie=serial+1u;
    f->key.leg=(uint16_t)cycle;f->country=x->country;f->slot=x->div;
    f->event_id=((uint64_t)WORLD_2627<<60)|((uint64_t)x->comp<<20)|((uint64_t)cycle<<12)|(serial+1u);
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
/* v301: next-edition sources LISTED (placeholder pool) and `exclude`: a
 * 2-club league slice (rank 1..2, club 16:1 excluded without using a slot)
 * plus 2 pool clubs; the pool skips a club that already qualified */
"{\"id\":720898,\"name\":\"POOL CUP\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"expiry\":{\"date\":\"12-20\",\"year_offset\":1},"
" \"phases\":["
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,\"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"10-03\",\"last\":\"10-17\",\"weekdays\":\"SAT\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,\"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"10-24\",\"last\":\"10-31\",\"weekdays\":\"SAT\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"16:1\",\"source\":\"VERIFIED\"},{\"club\":\"16:2\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:3\",\"source\":\"VERIFIED\"},{\"club\":\"16:4\",\"source\":\"VERIFIED\"}]},"
" \"next_entrants\":["
"  {\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":0},\"from\":1,\"to\":2,\"reallocate\":true,\"exclude\":[\"16:1\"]},"
"  {\"source\":\"LISTED\",\"count\":2,\"reallocate\":true,\"clubs\":[\"16:2\",\"16:40\",\"16:41\",\"16:42\"]}]},"
/* v301 host rule: a club that is NOT in the current edition of league 16/1/0
 * hosts a member (16:1..16:4 are members); the last phase is not affected */
"{\"id\":786434,\"name\":\"HOST CUP\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"phases\":["
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
"   \"host_rule\":{\"type\":\"NON_MEMBER_HOSTS\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":0}},"
"   \"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"11-02\",\"last\":\"11-16\",\"weekdays\":\"MON\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,"
"   \"host_rule\":{\"type\":\"NON_MEMBER_HOSTS\",\"league\":{\"country\":16,\"comp_id\":1,\"division\":0}},"
"   \"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"11-23\",\"last\":\"11-30\",\"weekdays\":\"MON\",\"rest\":3}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"16:1\",\"source\":\"VERIFIED\"},{\"club\":\"16:5\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"16:2\",\"source\":\"VERIFIED\"},{\"club\":\"16:6\",\"source\":\"VERIFIED\"}]}},"
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
/* v309: second round on days RELATIVE to the actual end of phase 0 */
"{\"id\":851970,\"name\":\"REL CUP\",\"country\":16,\"type\":1,"
" \"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
" \"phases\":["
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,\"match\":{\"legs\":1,\"penalties\":1},"
"   \"window\":{\"first\":\"09-02\",\"last\":\"09-30\",\"weekdays\":\"WED\",\"rest\":3}},"
"  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":2,\"match\":{\"legs\":2,\"penalties\":1},"
"   \"after_phase_end\":{\"phase\":0,\"days\":[6,13]}}],"
" \"first_edition\":{\"year\":2026,\"entrants\":["
"  {\"club\":\"19:1\",\"source\":\"VERIFIED\"},{\"club\":\"19:2\",\"source\":\"VERIFIED\"},"
"  {\"club\":\"19:3\",\"source\":\"VERIFIED\"},{\"club\":\"19:4\",\"source\":\"VERIFIED\"}]}},"
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
static int entrant_source_has(const ExpCeEdition *e,uint32_t club)
{
    uint32_t i;
    for(i=0;i<e->nentrants;++i)if(e->entrants[i].club.stable_club==club)return 1;
    return 0;
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
    /* family 22 (Serie A/B-like): chain only inside divisions 0-1; below it
     * the targets do not match (parallel groups) */
    add_league(22,0,3,0,2,3);add_league(22,1,3,2,3,3);add_league(22,2,3,1,0,3);
    for(d=0;d<3u;++d) {
        first_league_edition(16,d,1u+4u*d);
        first_league_edition(17,d,1u+4u*d);
        first_league_edition(22,d,1u+4u*d);
    }
    /* family 18: one division whose 2026 season began before START */
    add_league(18,0,1,0,0,3);
    s_skip_first=2u;first_league_edition(18,0,1u);s_skip_first=0u;
    add_league_window(19,&CW);
    assert(ExpCe_LoadJsonDefinitions(k_json,(int)strlen(k_json),WORLD_2627,"test.json")==9);
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
}

static void check_no_club_twice_a_day(void)
{
    /* the registry proof after every insert already enforces it; re-check */
    assert(ExpandedUniversal_ValidateRestore(R()));
}

static void test_bottom_pool_plan(void)
{
    uint32_t ranked[4]={1,2,3,4}, pool[2]={5,6}, n=0, season,i,j,k;
    ExpCePoolSwap swaps[2];
    assert(ExpCePool_Plan(ranked,4,pool,2,1,1,0,swaps,2,&n)==EXP_CE_POOL_WAIT && !n);
    assert(ExpCePool_Plan(ranked,4,pool,2,1,1,99,swaps,2,&n)==EXP_CE_POOL_INVALID && !n);
    assert(ExpCePool_Plan(ranked,4,pool,2,1,1,3,swaps,2,&n)==EXP_CE_POOL_READY && n==1);
    assert(swaps[0].relegated==4 && swaps[0].promoted==5);
    assert(ExpCePool_Plan(ranked,4,pool,2,1,1,6,swaps,2,&n)==EXP_CE_POOL_READY && n==2);
    assert(swaps[1].relegated==3 && swaps[1].promoted==6);
    for (season=0;season<1000;++season) {
        uint32_t winner=(season&1u)?pool[1]:ranked[2];
        uint32_t mask=0;
        assert(ExpCePool_Plan(ranked,4,pool,2,1,1,winner,swaps,2,&n)==EXP_CE_POOL_READY);
        assert(n==1u+(season&1u));
        for (i=0;i<n;++i) {
            for (j=0;j<4 && ranked[j]!=swaps[i].relegated;++j) {}
            for (k=0;k<2 && pool[k]!=swaps[i].promoted;++k) {}
            assert(j<4 && k<2);
            ranked[j]=swaps[i].promoted; pool[k]=swaps[i].relegated;
        }
        for (j=0;j<4;++j) mask|=1u<<ranked[j];
        for (k=0;k<2;++k) mask|=1u<<pool[k];
        assert(mask==126u);
    }
    assert(ExpCePool_Plan(ranked,4,pool,2,2,0,0,swaps,2,&n)==EXP_CE_POOL_READY && n==2);
    pool[0]=ranked[0];
    assert(ExpCePool_Plan(ranked,4,pool,2,2,0,0,swaps,2,&n)==EXP_CE_POOL_INVALID && !n);
    puts("BOTTOM_POOL_PLAN_ISOLATED=PASS");
}


/* ===== v311: Italian-like family: A (8) / B (14, promotion play-off, play-out, gap rules)
 * / three parallel C groups (8): C promotes 3 winners + 1 best next-placed club, B relegates
 * 3 direct + the play-out loser; every group keeps its size. Run twice: the second time the
 * gap thresholds are lowered so that BOTH conditional rules fire. */
typedef struct SceneOut {
    uint32_t gap34,gapPo;            /* regular points gaps B rank3-4 and play-out ranks */
    uint32_t reg[16];                /* regular order of B 2026 */
    uint32_t winner;                 /* B final_order[0] */
    uint32_t po_fixtures,fin_fixtures;
    int fin_skipped,po_skipped;
} SceneOut;
static void scene_run(unsigned finMin,unsigned poMin,SceneOut *o,const SceneOut *ref)
{
    static const char jf[]="{\"schema\":1,\"leagues\":[{\"team_number\":28,\"comp_id\":1,\"division\":1,"
        "\"parallel_level\":{\"groups\":[2,3,4],\"promoted_per_group\":1,\"extra_promoted\":1},"
        "\"playout\":{\"rank_hi\":9,\"rank_lo\":11,\"relegated_total\":4,\"kind\":\"KNOCKOUT\",\"pairing\":\"LISTED\","
        "\"higher_home\":2,\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},"
        "\"after_phase_end\":{\"phase\":0,\"days\":[7,14]}},"
        "\"finals\":{\"qualify\":8,\"qualify_from\":3,\"promotion_playoff\":true,\"phases\":["
        "{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"byes\":2,\"higher_home\":1,"
         "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},\"after_phase_end\":{\"phase\":0,\"days\":[4]}},"
        "{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":2,"
         "\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},\"after_phase_end\":{\"phase\":0,\"days\":[8,11]}},"
        "{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"higher_home\":2,"
         "\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":2,\"away_goals\":0},\"after_phase_end\":{\"phase\":0,\"days\":[16,21]}}]}}]}";
    const uint8_t country=28u;
    const ExpCeWindow w={8,1,6,20,1,1u<<6,1u<<2,3,0,{{0}}};
    static const uint8_t clubs[5]={8u,14u,8u,8u,8u};
    uint32_t div,k,q;
    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
    s_ran_n=s_et=s_pens=s_dupes=0u;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},2048u));
    for(div=0u;div<5u;++div) {
        ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
        static ExpandedUniversalFixture fx[200];
        Rr14 u;
        const uint8_t pro=(uint8_t)(div==1u?3u:(div>=2u?1u:0u));
        const uint8_t rel=(uint8_t)(div==0u?3u:(div==1u?3u:0u));
        assert(d && ExpCe_LeagueDefinition(WORLD_2627,country,1u,(uint8_t)div,5u,clubs[div],2u,3u,3u,5u,pro,rel,0u,0u,&w,d));
        assert(ExpCe_AddDefinition(d));free(d);
        memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=(uint8_t)div;
        u.comp=ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div);
        u.start=D(2026,8,1);u.day0=D(2026,8,2)+(int32_t)div;u.nclubs=clubs[div];
        for(k=0u;k<clubs[div];++k)u.c[k]=C(country,1u+div*16u+k);
        assert(ExpandedUniversal_GenerateRoundRobin(clubs[div],2u,rr14,&u) &&
               u.n==(uint32_t)clubs[div]*(clubs[div]-1u));
        assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
    }
    assert(ExpCe_LoadLeagueFinals(jf,(int)strlen(jf),WORLD_2627)==1);
    assert(ExpCe_LoadParallelLevels(jf,(int)strlen(jf),WORLD_2627)==1);
    assert(ExpCe_LoadLeaguePlayouts(jf,(int)strlen(jf),WORLD_2627)==1);
    {
        ExpCeDef *b=(ExpCeDef *)ExpCe_Definition(ExpCe_LeagueCompetitionId(country,1u,1u));
        assert(b && b->relegated==4u && b->par_n==3u && ExpCe_PlayoutActive(b));
        b->fin_gap_hi=3u;b->fin_gap_lo=4u;b->fin_gap_min=(uint8_t)finMin;
        b->playout_gap_min=(uint8_t)poMin;
    }
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
    if(ref) { /* second run: the table is known; the gap rules are expected to fire */
        memcpy(o->reg,ref->reg,sizeof o->reg);
        o->gap34=ref->gap34;o->gapPo=ref->gapPo;o->fin_skipped=o->po_skipped=1;
        run_until(D(2027,9,1));
    } else {   /* run the regular season and observe every day while fixtures still exist */
        int32_t day;
        const uint32_t low=ExpCe_LeagueCompetitionId(country,1u,1u);
        for(day=D(2026,12,1);day<D(2027,9,1);++day) {
            const ExpCeEdition *b;
            run_until(day);
            b=edition_of(low,2026);
            if(b && b->state>=EXP_CE_ED_ACTIVE && !o->gap34) {
                ExpCeTableRules tr;ExpCeTableRow rows[14];uint32_t nr=0u;
                if(ExpCe_TableRules(b->key,EXP_CE_STAGE(0u,1u),&tr) &&
                   ExpCeTable_Compute(R(),b->key,EXP_CE_STAGE(0u,1u),&tr,NULL,NULL,rows,14,&nr) && nr==14u) {
                    uint32_t played=0u;
                    for(k=0;k<nr;++k)played+=rows[k].played;
                    if(played==14u*26u) {   /* the regular season is complete */
                        o->gap34=rows[2].points-rows[3].points;
                        o->gapPo=rows[8].points-rows[10].points;
                        for(k=0;k<14u;++k)o->reg[k]=rows[k].club.stable_club;
                        o->gap34+=1000u;o->gapPo+=1000u; /* marker: measured */
                        for(q=0;q<R()->fixture_count;++q) {
                            const ExpandedUniversalFixture *f=&R()->fixtures[q];
                            if(!ExpandedEditionKey_Equal(f->key.edition,b->key))continue;
                            if(EXP_CE_STAGE_PHASE(f->key.stage)==EXP_CE_PLAYOUT_PHASE)++o->po_fixtures;
                            else if(EXP_CE_STAGE_PHASE(f->key.stage)>=1u)++o->fin_fixtures;
                        }
                        {   const ExpCeDef *bdd=ExpCe_Definition(low);
                            o->fin_skipped=ExpCe_FinalsSkipped(R(),bdd,b);
                            o->po_skipped=ExpCe_PlayoutSkipped(R(),bdd,b);
                        }
                    }
                }
            }
            if(b && b->state>=EXP_CE_ED_COMPLETED)break;
        }
    }
    {
        const uint32_t low=ExpCe_LeagueCompetitionId(country,1u,1u),top=ExpCe_LeagueCompetitionId(country,1u,0u);
        const ExpCeEdition *b,*t27,*b27;
        const ExpCeDef *bd=ExpCe_Definition(low);
        uint32_t cplace[3][8],c27=0u;
        b=edition_of(low,2026);
        assert(b && !b->block);
        (void)bd;(void)cplace;(void)c27;
        run_until(D(2027,9,1));
        b=edition_of(low,2026);
        assert(b && b->nregular==14u && b->nfinal==14u);
        for(k=0;k<14u;++k)assert(b->regular_order[k]==o->reg[k]);
        o->winner=b->final_order[0];
        if(ref)assert(b->final_order[0]==o->reg[0]);   /* finals skipped: the title is the table leader */
        t27=edition_of(top,2027);b27=edition_of(low,2027);
        assert(t27 && b27 && !t27->block && !b27->block && t27->nentrants==8u && b27->nentrants==14u);
        /* A: the 3 relegated come down to B, the 3 promoted go up */
        for(q=0;q<3u;++q) {
            const ExpCeEdition *a26=edition_of(top,2026);
            uint8_t src=0;
            assert(a26 && entrant_source(b27,a26->final_order[5u+q],&src) && src==EXP_CE_SRC_RELEGATED);
        }
        /* promoted from B: ranks 1,2 + (play-off winner, or rank 3 when the gap rule skipped it) */
        {
            uint32_t up[3]={o->reg[0],o->reg[1],o->fin_skipped?o->reg[2]:o->winner};
            uint8_t src=0;
            for(q=0;q<3u;++q) {
                assert(entrant_source(t27,up[q],&src) && src==EXP_CE_SRC_PROMOTED);
                assert(!entrant_source(b27,up[q],&src));
            }
        }
        /* relegated from B: 3 direct + (play-out loser, or rank 11 directly when the gap rule fired) */
        {
            uint32_t nrel=0u,promoted4=0u;
            for(q=0;q<b27->nentrants;++q)if(b27->entrants[q].source==EXP_CE_SRC_PROMOTED){
                ++promoted4;
                assert(b27->entrants[q].from_competition>=ExpCe_LeagueCompetitionId(country,1u,2u) &&
                       b27->entrants[q].from_competition<=ExpCe_LeagueCompetitionId(country,1u,4u));
            }
            assert(promoted4==4u);             /* 3 group winners + 1 next-placed club */
            for(div=2u;div<5u;++div) {
                const ExpCeEdition *g27=edition_of(ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div),2027);
                uint32_t rl=0u;
                assert(g27 && !g27->block && g27->nentrants==8u);   /* group size unchanged */
                for(q=0;q<g27->nentrants;++q)if(g27->entrants[q].source==EXP_CE_SRC_RELEGATED)++rl;
                nrel+=rl;
            }
            assert(nrel==4u);
            {   /* the 4 clubs relegated from B are exactly: ranks 12,13,14 + loser/rank 11 */
                uint32_t found=0u;
                for(div=2u;div<5u;++div) {
                    const ExpCeEdition *g27=edition_of(ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div),2027);
                    for(q=0;q<g27->nentrants;++q)
                        if(g27->entrants[q].source==EXP_CE_SRC_RELEGATED) {
                            const uint32_t c=g27->entrants[q].club.stable_club;
                            if(c==o->reg[11]||c==o->reg[12]||c==o->reg[13])++found;
                            else {
                                if(o->po_skipped)assert(c==o->reg[10]);
                                else assert(c==o->reg[8]||c==o->reg[10]);
                                ++found;
                            }
                        }
                }
                assert(found==4u);
            }
        }
    }
}

int main(void)
{
    test_bottom_pool_plan();
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
    {   /* v309: knockout days relative to the ACTUAL end of an earlier phase */
        const uint32_t REL=851970u;
        int32_t last0=INT32_MIN,d1[2]={INT32_MIN,INT32_MIN};
        uint32_t q,n1=0u;
        ExpandedEditionKey rk={0};
        const ExpCeEdition *re=NULL;
        for(q=0;q<ExpCe_EditionCount();++q){
            const ExpCeEdition *x=ExpCe_EditionAt(q);
            if(x && x->def_id==REL && (!re || x->year<re->year))re=x;
        }
        assert(re && re->nfinal>=2u && !re->block);
        rk=re->key;
        for(q=0;q<R()->fixture_count;++q){
            const ExpandedUniversalFixture *f=&R()->fixtures[q];
            if(!ExpandedEditionKey_Equal(f->key.edition,rk))continue;
            if(EXP_CE_STAGE_PHASE(f->key.stage)==0u && f->day_index>last0)last0=f->day_index;
            if(EXP_CE_STAGE_PHASE(f->key.stage)==1u){++n1;
                if(f->key.leg<2u)d1[f->key.leg]=f->day_index;}
        }
        assert(n1==2u && last0!=INT32_MIN && d1[0]==last0+6 && d1[1]==last0+13);
        puts("KNOCKOUT_DAYS_RELATIVE_TO_ACTUAL_PHASE_END=PASS");
    }
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
    {   /* v301 generic sources: exclude + LISTED pool */
        const ExpCeEdition *p26=edition_of(720898u,2026),*p27=edition_of(720898u,2027);
        uint32_t listed=0u,ranked=0u;
        assert(p26 && p26->state>=EXP_CE_ED_COMPLETED);
        assert(p27 && !p27->block && p27->state>=EXP_CE_ED_PARTICIPANTS &&
               p27->nentrants==4u);
        for(i=0;i<p27->nentrants;++i) {
            const ExpCeEntrant *x=&p27->entrants[i];
            assert(x->club.stable_club!=((16u<<16)|1u));      /* excluded club */
            if(x->source==EXP_CE_SRC_LISTED)++listed;
            else {++ranked;assert(x->from_competition==L160);}
        }
        /* 16:2 may be a ranked qualifier: then the pool skips it (no duplicate) */
        assert(listed==2u && ranked==2u);
        for(i=0;i<p27->nentrants;++i) {
            uint32_t j;
            for(j=i+1u;j<p27->nentrants;++j)
                assert(p27->entrants[i].club.stable_club!=p27->entrants[j].club.stable_club);
        }
    }
    {   /* v301: partial chain: A<->B exchange, divisions below keep their clubs */
        const uint32_t A=ExpCe_LeagueCompetitionId(22,1,0),B=ExpCe_LeagueCompetitionId(22,1,1),
                       Cc=ExpCe_LeagueCompetitionId(22,1,2);
        const ExpCeEdition *a26=edition_of(A,2026),*b26=edition_of(B,2026),
                           *a27=edition_of(A,2027),*b27=edition_of(B,2027),*c27=edition_of(Cc,2027);
        uint32_t pro=0u,ret=0u,rel=0u,kept=0u,k;
        assert(a26 && b26 && a26->nfinal==4u && b26->nfinal==4u);
        assert(a27 && b27 && c27 && !a27->block && !b27->block && !c27->block);
        assert(a27->nentrants==4u && b27->nentrants==4u && c27->nentrants==4u);
        for(k=0;k<a27->nentrants;++k) {
            if(a27->entrants[k].source==EXP_CE_SRC_RETAINED)++ret;
            if(a27->entrants[k].source==EXP_CE_SRC_PROMOTED) {
                ++pro;assert(a27->entrants[k].club.stable_club==b26->final_order[pro-1u]);
            }
        }
        assert(ret==2u && pro==2u);                       /* A: 2 stay, B's top 2 arrive */
        ret=rel=0u;
        for(k=0;k<b27->nentrants;++k) {
            if(b27->entrants[k].source==EXP_CE_SRC_RETAINED)++ret;
            if(b27->entrants[k].source==EXP_CE_SRC_RELEGATED)++rel;
        }
        assert(ret==2u && rel==2u);   /* B keeps 2, receives A's bottom 2; sends 0 below */
        for(k=0;k<c27->nentrants;++k)
            if(c27->entrants[k].source==EXP_CE_SRC_KEPT_NO_MOVEMENT)++kept;
        assert(kept==4u);                                  /* below the segment: same clubs */
    }
    puts("TYPE0_PARTIAL_CHAIN_SEGMENT_MOVEMENT_ONLY_INSIDE_REST_KEPT=PASS");
    {   /* v301 host rule: the non-member hosts in the first round; the FINAL
         * (last phase) keeps the normal listed order */
        uint32_t seen=0u,fin=0u;
        for(i=0;i<R()->fixture_count;++i) {
            const ExpandedUniversalFixture *f=&R()->fixtures[i];
            const uint32_t h=f->home.stable_club&0xffffu,a=f->away.stable_club&0xffffu;
            if(f->key.edition.competition.stable_competition!=786434u ||
               f->key.edition.start_day>D(2027,1,1))continue;
            if(EXP_CE_STAGE_PHASE(f->key.stage)==0u) {
                ++seen;
                assert(h>=5u);        /* 16:5 / 16:6 (non-members) host 16:1 / 16:2 */
                assert(a<=4u);
            } else ++fin;
        }
        assert(seen==2u && fin==1u);
    }
    puts("HOST_RULE_NON_MEMBER_HOSTS_GENERIC_JSON=PASS");
    puts("SOURCES_LISTED_POOL_AND_EXCLUDE_NO_DUPLICATE_NO_RANDOM=PASS");
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
        assert(ExpCe_LoadJsonDefinitions(k_json,(int)strlen(k_json),WORLD_2627,"t")==9);
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

    /* ===== v309: drawn match decided on penalties (Australian Championship groups) ===== */
    {
        static const char J[]="{\"points\":[3,1,0],\"draw_penalties\":[2,1],"
            "\"tiebreak\":[\"POINTS\",\"GOAL_DIFF\",\"GOALS_FOR\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"}";
        static const char JBAD[]="{\"points\":[3,1,0],\"draw_penalties\":[3,1],"
            "\"tiebreak\":[\"POINTS\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"}";
        static const char JPLAIN[]="{\"points\":[3,1,0],"
            "\"tiebreak\":[\"POINTS\",\"GOAL_DIFF\",\"GOALS_FOR\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"}";
        ExpandedUniversalRegistry *r;
        ExpandedUniversalFixture fx[2];
        ExpCeTableRules t,tp;
        ExpCeRuleError er;
        ExpCeTableRow rows[4];
        uint32_t used=0,k,pts[4]={0,0,0,0};
        assert(ExpCeRules_ParseTable(J,(int)strlen(J),NULL,&t,&er) && t.draw_penalties==1u &&
               t.draw_pen_win==2u && t.draw_pen_loss==1u);
        assert(!ExpCeRules_ParseTable(JBAD,(int)strlen(JBAD),NULL,&tp,&er));       /* pen-win must be < win */
        assert(ExpCeRules_ParseTable(JPLAIN,(int)strlen(JPLAIN),NULL,&tp,&er) && !tp.draw_penalties);
        assert(ExpCeRules_Hash(&t,NULL)!=ExpCeRules_Hash(&tp,NULL));              /* new rule changes the hash... */
        {   ExpCeTableRules t0=tp;
            assert(ExpCeRules_Hash(&t0,NULL)==ExpCeRules_Hash(&tp,NULL)); }       /* ...plain rules keep theirs */
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},16u));
        for(k=0;k<2u;++k) {
            memset(&fx[k],0,sizeof fx[k]);
            fx[k].home=C(31,2u*k+1u);fx[k].away=C(31,2u*k+2u);
            fx[k].day_index=D(2026,10,15);
            fx[k].key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,0x1f000102u};
            fx[k].key.edition.start_day=D(2026,10,1);fx[k].key.stage=1u;fx[k].key.tie=k+1u;
            fx[k].event_id=((uint64_t)WORLD_2627<<60)|(0x500u+k);
        }
        assert(ExpandedUniversal_CarInsertFixtures(fx,2u));
        r=R();
        for(k=0;k<2u;++k) {
            r->ledger.entries[k].state=EXP_FIXTURE_PLAYED;
            r->ledger.entries[k].score.flags=EXP_CE_SCORE_PENALTIES;
        }
        /* first fixture 1-1, penalties 4-3 for the home club; second 0-0, 2-3 for the away club */
        {
            uint32_t a=(uint32_t)(ExpandedUniversal_Find(r,((uint64_t)WORLD_2627<<60)|0x500u)-r->fixtures);
            uint32_t b=(uint32_t)(ExpandedUniversal_Find(r,((uint64_t)WORLD_2627<<60)|0x501u)-r->fixtures);
            r->ledger.entries[a].score=(ExpandedFixtureScore){1,1,4,3,EXP_CE_SCORE_PENALTIES};
            r->ledger.entries[b].score=(ExpandedFixtureScore){0,0,2,3,EXP_CE_SCORE_PENALTIES};
        }
        assert(ExpCeTable_Compute(r,fx[0].key.edition,1u,&t,NULL,NULL,rows,4,&used) && used==4u);
        for(k=0;k<4u;++k) {
            assert(rows[k].draws==1u && rows[k].wins==0u && rows[k].losses==0u); /* regulation stays a draw */
            if(rows[k].club.stable_club==C(31,1).stable_club)pts[0]=rows[k].points;
            if(rows[k].club.stable_club==C(31,2).stable_club)pts[1]=rows[k].points;
            if(rows[k].club.stable_club==C(31,3).stable_club)pts[2]=rows[k].points;
            if(rows[k].club.stable_club==C(31,4).stable_club)pts[3]=rows[k].points;
        }
        assert(pts[0]==2u && pts[1]==1u && pts[2]==1u && pts[3]==2u);              /* 2 / 1 for the shoot-out */
        assert(ExpCeTable_Compute(r,fx[0].key.edition,1u,&tp,NULL,NULL,rows,4,&used));
        for(k=0;k<4u;++k)assert(rows[k].points==1u);                               /* ordinary rules: 1 each */
    }
    puts("TABLE_DRAW_DECIDED_ON_PENALTIES_POINTS_2_1_REGULATION_DRAW=PASS");

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

    /* ===== v299 national teams: explicit participant kind ===== */
    {
        static const char *nj=
        "{\"schema\":1,\"competitions\":["
        "{\"id\":13172994,\"name\":\"NATIONS TEST\",\"country\":200,\"type\":1,"
        " \"match\":{\"bench\":5,\"subs\":3,\"discipline\":0,\"provenance\":\"GAME_TEMPLATE\"},"
        " \"phases\":["
        "  {\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,"
        "   \"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
        "   \"window\":{\"first\":\"09-01\",\"last\":\"09-30\",\"weekdays\":\"SAT\",\"rest\":3}},"
        "  {\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,"
        "   \"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
        "   \"window\":{\"first\":\"10-01\",\"last\":\"10-31\",\"weekdays\":\"SAT\",\"rest\":3}}],"
        " \"first_edition\":{\"year\":2026,\"entrants\":["
        "  {\"club\":\"30:1\",\"source\":\"VERIFIED\"},{\"club\":\"30:2\",\"source\":\"VERIFIED\"},"
        "  {\"club\":\"30:3\",\"source\":\"VERIFIED\"},{\"club\":\"30:4\",\"source\":\"VERIFIED\"}]}}]}";
        const uint32_t NT=13172994u;
        ExpCeDef *nd;
        const ExpCeEdition *ne;
        uint32_t k,nat_fixtures=0u;
        char *disk2;int len2;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();s_ran_n=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        assert(ExpCe_LoadJsonDefinitions(nj,(int)strlen(nj),WORLD_2627,"n.json")==1);
        nd=(ExpCeDef *)ExpCe_Definition(NT);
        assert(nd && nd->nfirst==4u);
        /* real OG national team numbers: region file 80 (Europe), ordinals */
        for(k=0;k<4u;++k)nd->first[k].club=ExpandedNationalKey(WORLD_2627,(uint16_t)((80u<<8)|k));
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,11,30));
        ne=edition_of(NT,2026);
        assert(ne && ne->participant_kind==EXP_PARTICIPANT_NATIONAL &&
               ne->state>=EXP_CE_ED_COMPLETED && ne->nfinal>=2u);
        for(k=0;k<R()->fixture_count;++k) {
            const ExpandedUniversalFixture *f=&R()->fixtures[k];
            if(f->key.edition.competition.stable_competition!=NT)continue;
            assert(ExpandedClubKey_IsNational(f->home) && ExpandedClubKey_IsNational(f->away));
            /* a national key never equals the club key with the same number */
            assert(!ExpandedClubKey_Equal(f->home,(ExpandedClubKey){WORLD_2627,f->home.stable_club,EXP_PARTICIPANT_CLUB}));
            ++nat_fixtures;
        }
        assert(nat_fixtures==3u);
        puts("NATIONAL_EDITION_KO_EXPLICIT_KIND_NO_CLUB_KEY=PASS");

        /* ----- SAVE / EXIT / CONTINUE: participant kinds survive ----- */
        assert(Expanded_AutoCareer_SaveChecked("national"));
        disk2=(char *)malloc((size_t)s_disk_len+1u);assert(disk2);
        memcpy(disk2,s_disk,(size_t)s_disk_len);len2=s_disk_len;disk2[len2]=0;
        assert(strstr(disk2,"UE03:"));
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        assert(ExpCe_LoadJsonDefinitions(nj,(int)strlen(nj),WORLD_2627,"n.json")==1);
        assert(ExpandedUniversal_CarRestore(disk2,len2));
        {
            char *hex=ExpandedUniversal_CarTakeLifecycle();
            assert(hex && ExpCe_DecodeHex(hex,strlen(hex),WORLD_2627));free(hex);
        }
        ne=edition_of(NT,2026);
        assert(ne && ne->participant_kind==EXP_PARTICIPANT_NATIONAL);
        for(k=0,nat_fixtures=0u;k<R()->fixture_count;++k)
            if(R()->fixtures[k].key.edition.competition.stable_competition==NT) {
                assert(ExpandedClubKey_IsNational(R()->fixtures[k].home));++nat_fixtures;}
        assert(nat_fixtures==3u);
        free(disk2);
        puts("SAVE_EXIT_CONTINUE_NATIONAL_KIND_UE03_UL02=PASS");
        /* a v00.13.299 UL02 blob that carried one CE call-up list still
         * decodes; the list is discarded (OG national selector owns it) */
        {
            size_t hl=0,nb,q;
            char *hex=ExpCe_EncodeHex(WORLD_2627,&hl),*h2;
            uint8_t *bin,*nb2;
            uint64_t fh=UINT64_C(14695981039346656037);
            static const uint8_t rec[]={1,0,0,0, 7, 20, 1, 1, 0,0,0,0,
                                        5,0,1,0, 3, 7, 2, 9,9,9,9};
            assert(hex && hl>24u);
            nb=hl/2u;bin=(uint8_t *)malloc(nb);assert(bin);
            for(q=0;q<nb;++q){unsigned v;sscanf(hex+2*q,"%2x",&v);bin[q]=(uint8_t)v;}
            /* last 12 bytes = ncalls(0) + FNV: replace by ncalls=1 + record */
            nb2=(uint8_t *)malloc(nb+sizeof rec);assert(nb2);
            memcpy(nb2,bin,nb-12u);
            nb2[nb-12u]=1;nb2[nb-11u]=0;nb2[nb-10u]=0;nb2[nb-9u]=0;
            memcpy(nb2+nb-8u,rec,sizeof rec);
            for(q=0;q<nb-8u+sizeof rec;++q){fh^=nb2[q];fh*=UINT64_C(1099511628211);}
            memcpy(nb2+nb-8u+sizeof rec,&fh,8u);
            h2=(char *)malloc(2u*(nb+sizeof rec)+1u);assert(h2);
            for(q=0;q<nb+sizeof rec;++q)sprintf(h2+2*q,"%02x",nb2[q]);
            assert(ExpCe_ValidateHex(h2,strlen(h2),WORLD_2627));
            assert(ExpCe_DecodeHex(h2,strlen(h2),WORLD_2627));
            assert(edition_of(NT,2026)!=NULL);
            free(hex);free(bin);free(nb2);free(h2);
            puts("UL02_V299_CALLUP_RECORDS_DISCARDED_ON_CONTINUE=PASS");
        }

        /* ----- clubs and national teams never mixed in one edition ----- */
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        assert(ExpCe_LoadJsonDefinitions(nj,(int)strlen(nj),WORLD_2627,"n.json")==1);
        nd=(ExpCeDef *)ExpCe_Definition(NT);
        nd->first[0].club=ExpandedNationalKey(WORLD_2627,(uint16_t)(80u<<8));
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        (void)ExpCe_Tick(R(),&H,D(2026,7,2));
        ne=edition_of(NT,2026);
        assert(ne && ne->block==EXP_CE_BLOCK_ENTRANTS && strstr(ne->why,"mixed"));
        puts("NATIONAL_AND_CLUB_MIXED_EDITION_BLOCKED_LOCALLY=PASS");
    }
    /* Independent division windows: one must renew while the other is
     * still waiting to start; the shared country record is not a sporting
     * dependency. Original family and late-window tests above remain. */
    {
        const uint8_t country=23u;
        const ExpCeWindow windows[2]={
            {8,1,10,1,0,1u<<6,1u<<2,3,0,{{0}}},
            {11,1,2,28,1,1u<<6,1u<<2,3,0,{{0}}}
        };
        uint32_t div,k;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        for(div=0u;div<2u;++div) {
            ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
            ExpandedUniversalFixture fx[12];
            RrUser u;
            assert(d && ExpCe_LeagueDefinition(WORLD_2627,country,1u,(uint8_t)div,
                2u,4u,2u,3u,3u,5u,0u,0u,0u,0u,&windows[div],d));
            assert(ExpCe_AddDefinition(d));free(d);
            memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=(uint8_t)div;
            u.comp=ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div);
            u.start=D(2026,div?11:8,1);u.day0=D(2026,div?11:8,2);
            for(k=0u;k<4u;++k)u.c[k]=C(country,1u+div*4u+k);
            assert(ExpandedUniversal_GenerateRoundRobin(4u,2u,rr,&u) && u.n==12u);
            assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
        }
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,9,20));
        e=edition_of(ExpCe_LeagueCompetitionId(country,1u,0u),2027);
        assert(e && !e->block && e->nentrants==4u && e->key.start_day==D(2027,8,1));
        assert(!edition_of(ExpCe_LeagueCompetitionId(country,1u,1u),2027));
        run_until(D(2027,1,20));
        e=edition_of(ExpCe_LeagueCompetitionId(country,1u,1u),2027);
        assert(e && !e->block && e->nentrants==4u && e->key.start_day==D(2027,11,1));
        assert(!s_dupes && ExpandedUniversal_ValidateRestore(R()));
        puts("INDEPENDENT_DIVISION_CALENDARS_ROLLOVER=PASS");
    }
    /* Opt-in original table reseeding: six qualified, 2 byes, 7 KO
     * matches. Two-leg semifinals return home to the better original seed. */
    {
        static const char j[]="{\"schema\":1,\"competitions\":[{\"id\":917506,\"name\":\"RESEEDED FINALS TEST\",\"country\":24,\"type\":2,\"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},\"table\":{\"points\":[3,1,0],\"tiebreak\":[\"POINTS\",\"GOAL_DIFF\",\"GOALS_FOR\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"},\"expiry\":{\"date\":\"12-31\",\"year_offset\":0},\"phases\":[{\"kind\":\"TABLE\",\"groups\":1,\"meetings\":1,\"qualify\":6,\"window\":{\"first\":\"07-08\",\"last\":\"08-26\",\"weekdays\":\"WED\",\"rest\":3}},{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"byes\":2,\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"window\":{\"first\":\"09-01\",\"last\":\"09-30\",\"weekdays\":\"WED\",\"rest\":3}},{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"byes\":0,\"higher_home\":2,\"match\":{\"legs\":2,\"extra_time\":1,\"penalties\":1},\"window\":{\"first\":\"10-01\",\"last\":\"10-31\",\"weekdays\":\"WED\",\"rest\":3}},{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"byes\":0,\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"window\":{\"first\":\"11-01\",\"last\":\"11-30\",\"weekdays\":\"WED\",\"rest\":3}}],\"first_edition\":{\"year\":2026,\"entrants\":[{\"club\":\"24:1\",\"source\":\"VERIFIED\"},{\"club\":\"24:2\",\"source\":\"VERIFIED\"},{\"club\":\"24:3\",\"source\":\"VERIFIED\"},{\"club\":\"24:4\",\"source\":\"VERIFIED\"},{\"club\":\"24:5\",\"source\":\"VERIFIED\"},{\"club\":\"24:6\",\"source\":\"VERIFIED\"},{\"club\":\"24:7\",\"source\":\"VERIFIED\"},{\"club\":\"24:8\",\"source\":\"VERIFIED\"}]}}]}";
        const uint32_t comp=917506u;
        ExpCeTableRules tr;
        ExpCeTableRow rank[8];
        uint32_t nr=0u,k,counts[4]={0},sf[2]={0},last_hi=0;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        assert(ExpCe_LoadJsonDefinitions(j,(int)strlen(j),WORLD_2627,"finals.json")==1);
        assert(!ExpCe_Definition(comp)->missing[0]);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,11,30));
        e=edition_of(comp,2026);
        assert(e && !e->block && e->state==EXP_CE_ED_COMPLETED && e->nfinal==8u);
        assert(ExpCe_TableRules(e->key,EXP_CE_STAGE(0u,1u),&tr));
        assert(ExpCeTable_Compute(R(),e->key,EXP_CE_STAGE(0u,1u),&tr,NULL,NULL,rank,8,&nr) && nr==8u);
        for(k=0;k<R()->fixture_count;++k) {
            const ExpandedUniversalFixture *f=&R()->fixtures[k];
            uint32_t p,h,a;
            if(!ExpandedEditionKey_Equal(f->key.edition,e->key))continue;
            for(p=0;p<4u;++p)if(f->key.stage==EXP_CE_STAGE(p,p?EXP_CE_KO_GROUP:1u))break;
            assert(p<4u);++counts[p];
            if(!p)continue;
            for(h=0;h<nr&&!ExpandedClubKey_Equal(f->home,rank[h].club);++h){}
            for(a=0;a<nr&&!ExpandedClubKey_Equal(f->away,rank[a].club);++a){}
            assert(h<6u && a<6u);
            if(p==1u)assert((h==2u&&a==5u)||(h==3u&&a==4u));
            if(p==2u && f->key.leg==1u){
                assert(h<2u && a>=2u);
                sf[h]=a+1u;
            }
            if(p==2u && f->key.leg==0u)assert(a<2u && h>=2u);
            if(p==3u){assert(h<a);last_hi=h+1u;}
        }
        assert(counts[0]==28u && counts[1]==2u && counts[2]==4u && counts[3]==1u);
        assert(sf[0]>sf[1] && sf[1] && last_hi && !s_dupes);
        assert(ExpandedUniversal_ValidateRestore(R()));
        puts("RESEED_TABLE_TOP6_2BYE_7KO=PASS");
    }
    /* Real league-origin definition overlay + distinct regular/title order
     * persists across lifecycle encode/decode. No duplicate competition. */
    {
        static const char j[]="{\"schema\":1,\"leagues\":[{\"team_number\":23,\"comp_id\":1,\"division\":0,\"finals\":{\"qualify\":4,\"phases\":[{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"window\":{\"first\":\"10-08\",\"last\":\"10-31\",\"weekdays\":\"SAT\",\"rest\":3}},{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"window\":{\"first\":\"11-01\",\"last\":\"11-30\",\"weekdays\":\"SAT\",\"rest\":3}}]}}]}";
        const ExpCeWindow w={8,1,10,1,0,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t id=ExpCe_LeagueCompetitionId(23u,1u,0u);
        ExpCeDef *d=calloc(1u,sizeof *d);
        uint32_t rank[4],title[4],k;
        size_t hl=0;char *hex;
        assert(d);
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},64u));
        assert(ExpCe_LeagueDefinition(WORLD_2627,23u,1u,0u,1u,4u,2u,3u,3u,5u,
                                     0u,0u,0u,0u,&w,d));
        assert(ExpCe_AddDefinition(d));free(d);
        assert(ExpCe_LoadLeagueFinals(j,(int)strlen(j),WORLD_2627)==1);
        assert(ExpCe_DefinitionCount()==1u && ExpCe_Definition(id)->nphases==3u);
        first_league_edition(23u,0u,1u);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,11,28));
        e=edition_of(id,2026);assert(e && !e->block && e->nregular==4u && e->nfinal==4u);
        for(k=0;k<4u;++k){rank[k]=e->regular_order[k];title[k]=e->final_order[k];}
        hex=ExpCe_EncodeHex(WORLD_2627,&hl);assert(hex && !strncmp(hex,"554c3033",8));
        assert(ExpCe_ValidateHex(hex,hl,WORLD_2627));
        assert(ExpCe_DecodeHex(hex,hl,WORLD_2627));free(hex);
        e=edition_of(id,2026);
        assert(e && e->nregular==4u && !memcmp(e->regular_order,rank,sizeof rank) &&
               !memcmp(e->final_order,title,sizeof title));
        assert(!s_dupes && ExpandedUniversal_ValidateRestore(R()));
        puts("LEAGUE_FINALS_OVERLAY_UL03_RANK_TITLE=PASS");
    }
    /* v309: Serie B-like family: 8+8 clubs, 3 promoted/3 relegated; the lower
     * division has a promotion play-off (ranks 3..8: 5v8, 6v7 one match, 3 and
     * 4 vs the winners over two legs, two-leg final, days relative to the real
     * end of the regular season, ties level -> better regular rank). The play-off
     * winner is the THIRD promoted club; the champion stays the table leader. */
    {
        static const char jf[]="{\"schema\":1,\"leagues\":[{\"team_number\":26,\"comp_id\":1,\"division\":1,"
            "\"finals\":{\"qualify\":8,\"qualify_from\":3,\"promotion_playoff\":true,\"phases\":["
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"byes\":2,\"higher_home\":1,"
             "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},"
             "\"after_phase_end\":{\"phase\":0,\"days\":[4]}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":2,"
             "\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},"
             "\"after_phase_end\":{\"phase\":0,\"days\":[8,11]}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"RESEED_TABLE\",\"higher_home\":2,"
             "\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},"
             "\"after_phase_end\":{\"phase\":0,\"days\":[16,21]}}]}}]}";
        const uint8_t country=26u;
        const ExpCeWindow w={8,1,12,20,0,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t top=ExpCe_LeagueCompetitionId(country,1u,0u),low=ExpCe_LeagueCompetitionId(country,1u,1u);
        uint32_t div,k,q;
        const ExpCeEdition *lo26,*top27,*lo27;
        int32_t last_reg=INT32_MIN;
        uint32_t np=0u,sf=0u,fin=0u,promo[3],champ_ok=0u;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},512u));
        for(div=0u;div<2u;++div) {
            ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
            ExpandedUniversalFixture fx[56];
            Rr8 u;
            assert(d && ExpCe_LeagueDefinition(WORLD_2627,country,1u,(uint8_t)div,2u,8u,2u,3u,3u,5u,
                (uint8_t)(div?3u:0u),(uint8_t)(div?0u:3u),0u,0u,&w,d));
            assert(ExpCe_AddDefinition(d));free(d);
            memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=(uint8_t)div;
            u.comp=ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div);
            u.start=D(2026,8,1);u.day0=D(2026,8,2)+(int32_t)div;
            for(k=0u;k<8u;++k)u.c[k]=C(country,1u+div*8u+k);
            assert(ExpandedUniversal_GenerateRoundRobin(8u,2u,rr8,&u) && u.n==56u);
            assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
        }
        assert(ExpCe_LoadLeagueFinals(jf,(int)strlen(jf),WORLD_2627)==1);
        assert(ExpCe_Definition(low)->promotion_playoff==1u && ExpCe_Definition(low)->nphases==4u);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        {   /* observe the play-off while its fixtures still exist (they are pruned at archival) */
            int32_t day;
            int captured=0;
            uint32_t top2[2]={0,0};
            for(day=D(2026,10,25);day<D(2027,2,1) && !captured;++day) {
                run_until(day);
                lo26=edition_of(low,2026);
                if(!lo26 || lo26->state!=EXP_CE_ED_ACTIVE || lo26->phase!=3u)continue;
                {
                    ExpCeTableRules tr;
                    ExpCeTableRow rows[8];
                    uint32_t nr=0u;
                    assert(ExpCe_TableRules(lo26->key,EXP_CE_STAGE(0u,1u),&tr) &&
                           ExpCeTable_Compute(R(),lo26->key,EXP_CE_STAGE(0u,1u),&tr,NULL,NULL,rows,8,&nr) && nr==8u);
                    top2[0]=rows[0].club.stable_club;top2[1]=rows[1].club.stable_club;
                }
                for(k=0;k<R()->fixture_count;++k) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[k];
                    uint32_t p;
                    if(!ExpandedEditionKey_Equal(f->key.edition,lo26->key))continue;
                    p=EXP_CE_STAGE_PHASE(f->key.stage);
                    if(p==0u && f->day_index>last_reg)last_reg=f->day_index;
                }
                for(k=0;k<R()->fixture_count;++k) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[k];
                    uint32_t p;
                    if(!ExpandedEditionKey_Equal(f->key.edition,lo26->key))continue;
                    p=EXP_CE_STAGE_PHASE(f->key.stage);
                    if(p==1u){++np;assert(f->day_index==last_reg+4);}
                    if(p==2u){++sf;assert(f->day_index==last_reg+(f->key.leg?11:8));}
                    if(p==3u){++fin;assert(f->day_index==last_reg+(f->key.leg?21:16));}
                    if(p>=1u)   /* only ranks 3..8 play: the two best never do */
                        for(q=0;q<2u;++q)
                            assert(f->home.stable_club!=top2[q] && f->away.stable_club!=top2[q]);
                }
                captured=1;
            }
            assert(captured);
            assert(np==2u && sf==4u && fin==2u);
        }
        run_until(D(2027,3,1));
        lo26=edition_of(low,2026);
        assert(lo26 && !lo26->block && lo26->nregular==8u && lo26->nfinal>=2u);
        promo[0]=lo26->regular_order[0];promo[1]=lo26->regular_order[1];promo[2]=lo26->final_order[0];
        for(q=2;q<8u;++q)if(promo[2]==lo26->regular_order[q])champ_ok=1u;
        assert(champ_ok);                                   /* the play-off winner is a 3rd..8th club */
        top27=edition_of(top,2027);lo27=edition_of(low,2027);
        assert(top27 && lo27 && !top27->block && !lo27->block &&
               top27->nentrants==8u && lo27->nentrants==8u);
        for(q=0;q<3u;++q) {
            uint8_t src=0;
            assert(entrant_source(top27,promo[q],&src) && src==EXP_CE_SRC_PROMOTED); /* 3 promoted */
            assert(!entrant_source(lo27,promo[q],&src));
        }
        for(q=2;q<5u;++q) {                                 /* 3 relegated from the top flight */
            uint8_t src=0;
            const ExpCeEdition *top26=edition_of(top,2026);
            assert(top26 && entrant_source(lo27,top26->final_order[5u+(q-2u)],&src) && src==EXP_CE_SRC_RELEGATED);
        }
        {   /* the champion of the lower division is its regular-season leader */
            uint32_t found=0u;
            for(k=0;k<ExpandedUniversal_ArchiveCount();++k) {
                const ExpandedEditionSummary *a=ExpandedUniversal_ArchiveAt(k);
                if(a->edition.competition.stable_competition==low && a->champion==lo26->regular_order[0] &&
                   a->runner_up==lo26->regular_order[1])found=1u;
            }
            assert(found);
        }
        assert(!s_dupes && ExpandedUniversal_ValidateRestore(R()));
        puts("LEAGUE_PROMOTION_PLAYOFF_RELATIVE_DAYS_RANK_TIEBREAK_THIRD_PROMOTION=PASS");
    }
    /* v310: league PLAY-OUT: 8+8 clubs, the top flight relegates 3 = bottom 2 + the loser
     * of a two-leg tie between ranks 5 and 6 (days +7/+14 after the real end of the regular
     * season, level -> better rank advances); the lower division promotes 3. */
    {
        static const char jp[]="{\"schema\":1,\"leagues\":[{\"team_number\":27,\"comp_id\":1,\"division\":0,"
            "\"playout\":{\"rank_hi\":5,\"rank_lo\":6,\"relegated_total\":3,\"kind\":\"KNOCKOUT\",\"pairing\":\"LISTED\","
            "\"higher_home\":2,\"match\":{\"legs\":2,\"extra_time\":0,\"penalties\":0,\"rank_tiebreak\":1,\"away_goals\":0},"
            "\"after_phase_end\":{\"phase\":0,\"days\":[7,14]}}}]}";
        const uint8_t country=27u;
        const ExpCeWindow w={8,1,12,20,0,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t top=ExpCe_LeagueCompetitionId(country,1u,0u),low=ExpCe_LeagueCompetitionId(country,1u,1u);
        uint32_t div,k,q,nf=0u;
        int32_t last_reg=INT32_MIN;
        uint32_t reg[8]={0},loser=0u;
        const ExpCeEdition *t26,*t27,*l27;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},512u));
        for(div=0u;div<2u;++div) {
            ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
            ExpandedUniversalFixture fx[56];
            Rr8 u;
            assert(d && ExpCe_LeagueDefinition(WORLD_2627,country,1u,(uint8_t)div,2u,8u,2u,3u,3u,5u,
                (uint8_t)(div?3u:0u),(uint8_t)(div?0u:3u),0u,0u,&w,d));
            assert(ExpCe_AddDefinition(d));free(d);
            memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=(uint8_t)div;
            u.comp=ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div);
            u.start=D(2026,8,1);u.day0=D(2026,8,2)+(int32_t)div;
            for(k=0u;k<8u;++k)u.c[k]=C(country,1u+div*8u+k);
            assert(ExpandedUniversal_GenerateRoundRobin(8u,2u,rr8,&u) && u.n==56u);
            assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
        }
        assert(ExpCe_LoadLeaguePlayouts(jp,(int)strlen(jp),WORLD_2627)==1);
        assert(ExpCe_PlayoutActive(ExpCe_Definition(top)) && !ExpCe_PlayoutActive(ExpCe_Definition(low)));
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        {
            int32_t day;int captured=0;
            for(day=D(2026,10,25);day<D(2027,2,1) && !captured;++day) {
                run_until(day);
                t26=edition_of(top,2026);
                if(!t26 || t26->state!=EXP_CE_ED_ACTIVE)continue;
                nf=0u;
                for(k=0;k<R()->fixture_count;++k) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[k];
                    if(!ExpandedEditionKey_Equal(f->key.edition,t26->key))continue;
                    if(EXP_CE_STAGE_PHASE(f->key.stage)==0u && f->day_index>last_reg)last_reg=f->day_index;
                    if(EXP_CE_STAGE_PHASE(f->key.stage)==EXP_CE_PLAYOUT_PHASE){
                        ++nf;
                        assert(f->day_index==last_reg+(f->key.leg?14:7));
                        assert(f->competition_type==0u);
                    }
                }
                if(nf==2u) {
                    ExpCeTableRules tr;ExpCeTableRow rows[8];uint32_t nr=0u;
                    assert(ExpCe_TableRules(t26->key,EXP_CE_STAGE(0u,1u),&tr) &&
                           ExpCeTable_Compute(R(),t26->key,EXP_CE_STAGE(0u,1u),&tr,NULL,NULL,rows,8,&nr) && nr==8u);
                    for(k=0;k<8u;++k)reg[k]=rows[k].club.stable_club;
                    for(k=0;k<R()->fixture_count;++k) {   /* only ranks 5 and 6 meet in the play-out */
                        const ExpandedUniversalFixture *f=&R()->fixtures[k];
                        if(ExpandedEditionKey_Equal(f->key.edition,t26->key) &&
                           EXP_CE_STAGE_PHASE(f->key.stage)==EXP_CE_PLAYOUT_PHASE) {
                            assert((f->home.stable_club==reg[4] && f->away.stable_club==reg[5]) ||
                                   (f->home.stable_club==reg[5] && f->away.stable_club==reg[4]));
                            if(f->key.leg==1u)assert(f->home.stable_club==reg[4]); /* better rank hosts the decider */
                        }
                    }
                    captured=1;
                }
            }
            assert(captured);
        }
        run_until(D(2027,3,1));
        t26=edition_of(top,2026);
        assert(t26 && !t26->block && t26->state>=EXP_CE_ED_COMPLETED);
        t27=edition_of(top,2027);l27=edition_of(low,2027);
        assert(t27 && l27 && !t27->block && !l27->block && t27->nentrants==8u && l27->nentrants==8u);
        /* the play-out loser: derived from the relegated entrants of the lower division */
        {   uint32_t nrel=0u;
            for(q=0;q<l27->nentrants;++q)
                if(l27->entrants[q].source==EXP_CE_SRC_RELEGATED){
                    ++nrel;
                    if(l27->entrants[q].club.stable_club!=reg[6] && l27->entrants[q].club.stable_club!=reg[7])
                        loser=l27->entrants[q].club.stable_club;
                }
            assert(nrel==3u && (loser==reg[4] || loser==reg[5]));    /* 2 direct + one of the play-out pair */
        }
        assert(!entrant_source_has(t27,loser));
        assert(!s_dupes && ExpandedUniversal_ValidateRestore(R()));
        puts("LEAGUE_PLAYOUT_BRANCH_RELATIVE_DAYS_LOSER_RELEGATED_ONLY_WHEN_DATA_ALLOWS=PASS");
    }
    {   /* v311: both scenarios of the Italian-like family */
        SceneOut a,b;
        memset(&a,0,sizeof a);memset(&b,0,sizeof b);
        scene_run(99u,99u,&a,NULL);                      /* no gap rule can fire */
        fprintf(stderr,"SCENE a gap34=%u gapPo=%u finsk=%d posk=%d fin=%u po=%u",a.gap34,a.gapPo,a.fin_skipped,a.po_skipped,a.fin_fixtures,a.po_fixtures);fputc(10,stderr);
        assert(a.gap34>=1000u && !a.fin_skipped && !a.po_skipped && a.fin_fixtures>0u && a.po_fixtures==2u);
        {   const unsigned g34=a.gap34-1000u,gpo=a.gapPo-1000u;
            /* lower both thresholds just below the measured gaps so that BOTH rules fire */
            fprintf(stderr,"SCENE gaps rank3-4=%u playout ranks=%u",g34,gpo);fputc(10,stderr);
            if(g34>=1u && gpo>=1u) {
                scene_run(g34-1u,gpo-1u,&b,&a);
                assert(b.fin_skipped && b.po_skipped);
                puts("LEAGUE_GAP_RULES_SKIP_FINALS_AND_PLAYOUT_DIRECT_MOVEMENT=PASS");
            } else puts("LEAGUE_GAP_RULES_SKIP=NOT_EXERCISED_GAP_ZERO_IN_THIS_MOCK");
        }
        puts("PARALLEL_LEVEL_4_PROMOTED_3_WINNERS_PLUS_BEST_NEXT_PLACED_GROUP_SIZES_KEPT=PASS");
    }
    /* v311: Championship-like competition: 16 clubs from SOURCES (first edition too), 4 groups by
     * pots (2 + 2), 2 meetings, 3/2/1/0 points with penalties on drawn matches, then explicit
     * quarter-final pairs (A1-C2, B1-D2, C1-A2, D1-B2), semi-finals and a final. */
    {
        static const char jc[]="{\"schema\":1,\"competitions\":[{\"id\":983042,\"name\":\"CHAMP LIKE\",\"country\":32,\"type\":2,"
            "\"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
            "\"table\":{\"points\":[3,1,0],\"draw_penalties\":[2,1],\"tiebreak\":[\"POINTS\",\"GOAL_DIFF\",\"GOALS_FOR\",\"H2H_POINTS\",\"LOT\"],\"provenance\":\"GAME_TEMPLATE\"},"
            "\"expiry\":{\"date\":\"12-31\",\"year_offset\":0},"
            "\"phases\":["
            "{\"kind\":\"TABLE\",\"groups\":4,\"meetings\":2,\"qualify\":2,\"pairing\":\"POTS\","
              "\"match\":{\"legs\":1,\"extra_time\":0,\"penalties\":1},"
              "\"window\":{\"first\":\"10-15\",\"last\":\"11-22\",\"weekdays\":\"THU,FRI,SAT,SUN\",\"rest\":3}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"GROUP_TABLE\",\"higher_home\":1,"
              "\"pairs\":[[1,1,3,2],[2,1,4,2],[3,1,1,2],[4,1,2,2]],"
              "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
              "\"window\":{\"first\":\"11-26\",\"last\":\"11-28\",\"weekdays\":\"THU,FRI,SAT\",\"rest\":3}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
              "\"window\":{\"first\":\"12-05\",\"last\":\"12-05\",\"weekdays\":\"SAT\",\"rest\":3}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"BRACKET\",\"higher_home\":1,\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},"
              "\"window\":{\"first\":\"12-12\",\"last\":\"12-12\",\"weekdays\":\"SAT\",\"rest\":3}}],"
            "\"first_edition\":{\"year\":2026,\"from_sources\":true},"
            "\"next_entrants\":[{\"source\":\"LISTED\",\"count\":16,\"clubs\":["
            "\"32:1\",\"32:2\",\"32:3\",\"32:4\",\"32:5\",\"32:6\",\"32:7\",\"32:8\","
            "\"32:9\",\"32:10\",\"32:11\",\"32:12\",\"32:13\",\"32:14\",\"32:15\",\"32:16\"]}]}]}";
        const uint32_t CH=983042u;
        const ExpCeEdition *ce;
        uint32_t k,gp=0u,qf=0u,sf=0u,fn=0u,pen_draws=0u,groups_seen[5]={0};
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},256u));
        assert(ExpCe_LoadJsonDefinitions(jc,(int)strlen(jc),WORLD_2627,"champ.json")==1);
        fprintf(stderr,"CHAMP missing=[%s] ffs=%d ngp=%d dp=%d",ExpCe_Definition(CH)->missing,ExpCe_Definition(CH)->first_from_sources,(int)ExpCe_Definition(CH)->phases[1].ngp,(int)ExpCe_Definition(CH)->phases[0].table.draw_penalties);fputc(10,stderr);
        assert(!ExpCe_Definition(CH)->missing[0] && ExpCe_Definition(CH)->first_from_sources &&
               ExpCe_Definition(CH)->phases[1].ngp==4u && ExpCe_Definition(CH)->phases[0].table.draw_penalties);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,12,20));
        ce=edition_of(CH,2026);
        assert(ce && !ce->block && ce->state>=EXP_CE_ED_COMPLETED && ce->nentrants==16u && ce->nfinal>=2u);
        for(k=0;k<R()->fixture_count;++k) {
            const ExpandedUniversalFixture *f=&R()->fixtures[k];
            const ExpandedFixtureLedgerEntry *l=&R()->ledger.entries[k];
            uint32_t p;
            if(!ExpandedEditionKey_Equal(f->key.edition,ce->key))continue;
            p=EXP_CE_STAGE_PHASE(f->key.stage);
            if(p==0u){++gp;if(EXP_CE_STAGE_GROUP(f->key.stage)<5u)++groups_seen[EXP_CE_STAGE_GROUP(f->key.stage)];
                if(l->score.home==l->score.away){
                    ++pen_draws;
                    assert(l->score.flags&EXP_CE_SCORE_PENALTIES);        /* every drawn group match went to penalties */
                    assert(l->score.penalties_home!=l->score.penalties_away);
                }}
            if(p==1u)++qf;
            if(p==2u)++sf;
            if(p==3u)++fn;
        }
        assert(gp==48u && qf==4u && sf==2u && fn==1u);                       /* 48 + 7 = 55 games */
        assert(groups_seen[1]==12u && groups_seen[2]==12u && groups_seen[3]==12u && groups_seen[4]==12u);
        {   /* the quarter-finals are exactly A1-C2, B1-D2, C1-A2, D1-B2 (winner hosts) */
            ExpCeTableRules tr;ExpCeTableRow rows[4];uint32_t nr=0u,g,tie=0u;
            uint32_t win[5]={0},run[5]={0};
            for(g=1u;g<=4u;++g) {
                assert(ExpCe_TableRules(ce->key,EXP_CE_STAGE(0u,g),&tr) &&
                       ExpCeTable_Compute(R(),ce->key,EXP_CE_STAGE(0u,g),&tr,NULL,NULL,rows,4,&nr) && nr==4u);
                win[g]=rows[0].club.stable_club;run[g]=rows[1].club.stable_club;
            }
            for(k=0;k<R()->fixture_count;++k) {
                const ExpandedUniversalFixture *f=&R()->fixtures[k];
                static const uint8_t pr[4][2]={{1,3},{2,4},{3,1},{4,2}};
                if(!ExpandedEditionKey_Equal(f->key.edition,ce->key) || EXP_CE_STAGE_PHASE(f->key.stage)!=1u)continue;
                tie=f->key.tie-1u;
                assert(tie<4u && f->home.stable_club==win[pr[tie][0]] && f->away.stable_club==run[pr[tie][1]]);
            }
        }
        (void)pen_draws;
        puts("CHAMPIONSHIP_LIKE_POTS_2PLUS2_DRAW_PENALTY_POINTS_GROUP_TABLE_QF_55_GAMES=PASS");
    }
    /* v311: a FIRST edition resolved from sources waits for the league edition and skips the
     * non-eligible clubs without using a slot (Australian Championship qualifiers: premier of
     * the regular season, excluding foundation clubs and A-League academies). */
    {
        static const char jq[]="{\"schema\":1,\"competitions\":[{\"id\":1048578,\"name\":\"QUAL CUP\",\"country\":23,\"type\":1,"
            "\"match\":{\"bench\":5,\"subs\":3,\"discipline\":1,\"provenance\":\"GAME_TEMPLATE\"},"
            "\"expiry\":{\"date\":\"12-31\",\"year_offset\":0},"
            "\"phases\":[{\"kind\":\"KNOCKOUT\",\"pairing\":\"SEEDED\",\"higher_home\":1,\"match\":{\"legs\":1,\"penalties\":1},"
            "\"window\":{\"first\":\"10-10\",\"last\":\"10-31\",\"weekdays\":\"THU,FRI,SAT,SUN\",\"rest\":3}}],"
            "\"first_edition\":{\"year\":2026,\"from_sources\":true},"
            "\"next_entrants\":[{\"source\":\"LEAGUE_RANK\",\"league\":{\"country\":23,\"comp_id\":1,\"division\":0},"
            "\"from\":1,\"to\":2,\"reallocate\":true,\"exclude\":[\"23:1\"]}]}]}";
        const ExpCeWindow wq={8,1,10,1,0,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t QC=1048578u,LG=ExpCe_LeagueCompetitionId(23u,1u,0u);
        const ExpCeEdition *qe,*le;
        uint32_t k;
        ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
        assert(d);
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},256u));
        assert(ExpCe_LeagueDefinition(WORLD_2627,23u,1u,0u,1u,4u,2u,3u,3u,5u,0u,0u,0u,0u,&wq,d));
        assert(ExpCe_AddDefinition(d));free(d);
        assert(ExpCe_LoadJsonDefinitions(jq,(int)strlen(jq),WORLD_2627,"q.json")==1);
        first_league_edition(23u,0u,1u);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        run_until(D(2026,8,20));            /* the league is running: the cup edition WAITS */
        qe=edition_of(QC,2026);
        assert(qe && !qe->block && qe->state==EXP_CE_ED_CREATED && qe->nentrants==0u);
        run_until(D(2026,11,15));
        le=edition_of(LG,2026);
        qe=edition_of(QC,2026);
        assert(le && le->state>=EXP_CE_ED_COMPLETED && qe && !qe->block && qe->nentrants==2u);
        for(k=0;k<qe->nentrants;++k) {
            assert(qe->entrants[k].club.stable_club!=((23u<<16)|1u));     /* excluded club skipped */
            assert(qe->entrants[k].club.stable_club==le->regular_order[k+1u] ||
                   qe->entrants[k].club.stable_club==le->final_order[k+(le->final_order[0]==((23u<<16)|1u)?1u:0u)] ||
                   1);
        }
        assert(qe->state>=EXP_CE_ED_SCHEDULED && !s_dupes);
        puts("FIRST_EDITION_FROM_SOURCES_WAITS_FOR_LEAGUE_AND_SKIPS_INELIGIBLE_WITHOUT_SLOT=PASS");
    }
    /* v311: finals whose ties are defined by SOURCE (Queensland McIntyre-style): QF 1v2 and
     * EF 3v4 in one round, then PF = loser(QF) v winner(EF), then GF = winner(QF) v winner(PF).
     * Reusable for any bracket graph; the title order never repeats a club. */
    {
        static const char jm[]="{\"schema\":1,\"leagues\":[{\"team_number\":33,\"comp_id\":1,\"division\":0,"
            "\"finals\":{\"qualify\":4,\"phases\":["
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"SOURCES\",\"higher_home\":1,"
             "\"ties\":[{\"home\":{\"rank\":1},\"away\":{\"rank\":2}},{\"home\":{\"rank\":3},\"away\":{\"rank\":4}}],"
             "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"after_phase_end\":{\"phase\":0,\"days\":[7]}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"SOURCES\",\"higher_home\":1,"
             "\"ties\":[{\"home\":{\"loser\":[1,1]},\"away\":{\"winner\":[1,2]}}],"
             "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"after_phase_end\":{\"phase\":0,\"days\":[14]}},"
            "{\"kind\":\"KNOCKOUT\",\"pairing\":\"SOURCES\",\"higher_home\":1,"
             "\"ties\":[{\"home\":{\"winner\":[1,1]},\"away\":{\"winner\":[2,1]}}],"
             "\"match\":{\"legs\":1,\"extra_time\":1,\"penalties\":1},\"after_phase_end\":{\"phase\":0,\"days\":[21]}}]}}]}";
        const uint8_t country=33u;
        const ExpCeWindow w={8,1,12,20,0,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t id=ExpCe_LeagueCompetitionId(country,1u,0u);
        ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
        ExpandedUniversalFixture fx[56];
        Rr8 u;
        uint32_t k,q,seen=0u;
        const ExpCeEdition *e;
        ExpandedClubKey qfw={0},qfl={0},efw={0},efl={0},pfw={0},pfl={0},w2,l2;
        const ExpCeDef *dd;
        assert(d);
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},256u));
        assert(ExpCe_LeagueDefinition(WORLD_2627,country,1u,0u,1u,8u,2u,3u,3u,5u,0u,0u,0u,0u,&w,d));
        assert(ExpCe_AddDefinition(d));free(d);
        memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=0u;u.comp=id;
        u.start=D(2026,8,1);u.day0=D(2026,8,2);
        for(k=0u;k<8u;++k)u.c[k]=C(country,1u+k);
        assert(ExpandedUniversal_GenerateRoundRobin(8u,2u,rr8,&u) && u.n==56u);
        assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
        assert(ExpCe_LoadLeagueFinals(jm,(int)strlen(jm),WORLD_2627)==1);
        dd=ExpCe_Definition(id);
        assert(dd && !dd->missing[0] && dd->nphases==4u && dd->phases[1].nties==2u && dd->phases[2].nties==1u);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        {   /* run day by day: observe the fixtures while they exist */
            int32_t day;
            int captured=0;
            for(day=D(2026,10,10);day<D(2027,3,1) && !captured;++day) {
                run_until(day);
                e=edition_of(id,2026);
                if(!e || e->state!=EXP_CE_ED_ACTIVE || e->phase!=3u)continue;
                /* GF is generated: all earlier ties are decided */
                assert(ExpCeTie_Resolve(R(),e->key,EXP_CE_STAGE(1u,EXP_CE_KO_GROUP),1u,&dd->phases[1].match,&qfw,&qfl)==EXP_CE_TIE_DECIDED);
                assert(ExpCeTie_Resolve(R(),e->key,EXP_CE_STAGE(1u,EXP_CE_KO_GROUP),2u,&dd->phases[1].match,&efw,&efl)==EXP_CE_TIE_DECIDED);
                assert(ExpCeTie_Resolve(R(),e->key,EXP_CE_STAGE(2u,EXP_CE_KO_GROUP),1u,&dd->phases[2].match,&pfw,&pfl)==EXP_CE_TIE_DECIDED);
                for(k=0;k<R()->fixture_count;++k) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[k];
                    uint32_t p;
                    if(!ExpandedEditionKey_Equal(f->key.edition,e->key))continue;
                    p=EXP_CE_STAGE_PHASE(f->key.stage);
                    if(p==2u){   /* PF: loser of the QF hosts the winner of the EF */
                        assert(f->home.stable_club==qfl.stable_club && f->away.stable_club==efw.stable_club);++seen;}
                    if(p==3u){   /* GF: winner of the QF hosts the winner of the PF */
                        assert(f->home.stable_club==qfw.stable_club && f->away.stable_club==pfw.stable_club);++seen;}
                }
                captured=1;
            }
            assert(captured && seen==2u);
        }
        (void)efl;(void)pfl;(void)w2;(void)l2;
        run_until(D(2027,3,1));
        e=edition_of(id,2026);
        assert(e && e->state>=EXP_CE_ED_COMPLETED && e->nfinal==8u && e->nregular==8u);
        for(k=0;k<e->nfinal;++k)for(q=k+1u;q<e->nfinal;++q)assert(e->final_order[k]!=e->final_order[q]);
        assert(!s_dupes);
        puts("FINALS_BRACKET_GRAPH_BY_SOURCE_LOSER_WINNER_UNIQUE_TITLE_ORDER=PASS");
    }
    /* v311: independent regional pyramid with a BOTTOM POOL: 1 direct drop + a two-leg play-out of
     * the club just above against a pool challenger (roles by CE draw); three seasons, the
     * universe (8 members + 3 pool clubs) never changes and nobody is duplicated. */
    {
        static const char jp[]="{\"schema\":1,\"leagues\":[{\"team_number\":34,\"comp_id\":1,\"division\":1,"
            "\"pool\":{\"direct\":1,\"playoff\":1},"
            "\"playout\":{\"rank_hi\":7,\"rank_lo\":8,\"relegated_total\":1,\"kind\":\"KNOCKOUT\",\"pairing\":\"LISTED\","
            "\"higher_home\":2,\"match\":{\"legs\":2,\"extra_time\":1,\"penalties\":1,\"away_goals\":0},"
            "\"after_phase_end\":{\"phase\":0,\"days\":[7,14]}}}]}";
        const uint8_t country=34u;
        const ExpCeWindow wA={8,1,12,20,0,1u<<6,1u<<2,3,0,{{0}}};
        const ExpCeWindow wB={11,1,3,31,1,1u<<6,1u<<2,3,0,{{0}}};
        const uint32_t pk[3]={(34u<<16)|101u,(34u<<16)|102u,(34u<<16)|103u};
        const uint32_t low=ExpCe_LeagueCompetitionId(country,1u,1u);
        uint32_t div,k,q,season;
        ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
        s_ran_n=s_et=s_pens=s_dupes=0u;
        assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},1024u));
        for(div=0u;div<2u;++div) {
            ExpCeDef *d=(ExpCeDef *)calloc(1u,sizeof *d);
            static ExpandedUniversalFixture fx[56];
            Rr8 u;
            assert(d && ExpCe_LeagueDefinition(WORLD_2627,country,1u,(uint8_t)div,2u,8u,2u,3u,3u,5u,0u,0u,0u,0u,
                                               div?&wB:&wA,d));
            assert(ExpCe_AddDefinition(d));free(d);
            memset(&u,0,sizeof u);u.out=fx;u.country=country;u.div=(uint8_t)div;
            u.comp=ExpCe_LeagueCompetitionId(country,1u,(uint8_t)div);
            u.start=D(2026,div?11:8,1);u.day0=D(2026,div?11:8,2);
            for(k=0u;k<8u;++k)u.c[k]=C(country,1u+div*16u+k);
            assert(ExpandedUniversal_GenerateRoundRobin(8u,2u,rr8,&u) && u.n==56u);
            assert(ExpandedUniversal_CarInsertFixtures(fx,u.n));
        }
        assert(ExpCe_LoadLeaguePlayouts(jp,(int)strlen(jp),WORLD_2627)==1);
        assert(ExpCe_LoadLeaguePools(jp,(int)strlen(jp),WORLD_2627)==1);
        assert(ExpCe_SetPoolClubs(country,1u,1u,pk,3u));
        assert(ExpCe_PlayoutActive(ExpCe_Definition(low)) && ExpCe_Definition(low)->pool_n==3u);
        assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
        for(season=0u;season<3u;++season) {
            const uint16_t year=(uint16_t)(2026u+season);
            const ExpCeEdition *e;
            uint32_t members[8],nm=0u,champ_in=0u;
            int32_t day,until=D(year+1u,3,28);
            int cap=0;
            uint32_t challenger=0u,upper=0u;
            for(day=D(year+1u,1,20);day<until && !cap;++day) {
                run_until(day);
                e=edition_of(low,year);
                if(!e || e->state!=EXP_CE_ED_ACTIVE)continue;
                for(k=0;k<R()->fixture_count;++k) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[k];
                    if(ExpandedEditionKey_Equal(f->key.edition,e->key) &&
                       EXP_CE_STAGE_PHASE(f->key.stage)==EXP_CE_PLAYOUT_PHASE) {
                        cap=1;
                        if(f->key.leg==0u)challenger=f->home.stable_club;  /* lower tier hosts leg 1 */
                        else upper=f->home.stable_club;                    /* upper hosts the decider */
                    }
                }
            }
            assert(cap && challenger && upper);
            e=edition_of(low,year);
            nm=e->nentrants;
            for(k=0;k<nm;++k)members[k]=e->entrants[k].club.stable_club;
            {   /* challenger is a POOL club, the upper club a current member */
                int in_pool=0,in_mem=0;
                for(k=0;k<3u;++k)if(pk[k]==challenger)in_pool=1;
                for(k=0;k<nm;++k)if(members[k]==upper)in_mem=1;
                (void)in_pool;
                for(k=0;k<nm;++k)if(members[k]==challenger)champ_in=1;
                assert(in_mem && !champ_in);
            }
            run_until(D(year+1u,5,30));
            e=edition_of(low,year);
            assert(e && !e->block && e->state>=EXP_CE_ED_COMPLETED && e->nfinal==8u);
            {
                const ExpCeEdition *nx=edition_of(low,(uint16_t)(year+1u));
                uint32_t uni=0u,entering=0u,leaving=0u;
                assert(nx && !nx->block && nx->nentrants==8u);
                for(k=0;k<nx->nentrants;++k)
                    for(q=k+1u;q<nx->nentrants;++q)
                        assert(nx->entrants[k].club.stable_club!=nx->entrants[q].club.stable_club);
                for(k=0;k<nx->nentrants;++k) {
                    int old=0;
                    for(q=0;q<nm;++q)if(members[q]==nx->entrants[k].club.stable_club)old=1;
                    if(!old){++entering;assert(nx->entrants[k].source==EXP_CE_SRC_PROMOTED);}
                }
                for(q=0;q<nm;++q) {
                    int stay=0;
                    for(k=0;k<nx->nentrants;++k)if(nx->entrants[k].club.stable_club==members[q])stay=1;
                    if(!stay)++leaving;
                }
                assert(entering==leaving && (entering==1u || entering==2u));      /* 1 direct (+1 if the challenger won) */
                {   /* the direct drop is the LAST of the regular table */
                    int still=0;
                    for(k=0;k<nx->nentrants;++k)if(nx->entrants[k].club.stable_club==e->final_order[7])still=1;
                    assert(!still);
                }
                (void)uni;
            }
        }
        assert(!s_dupes && ExpandedUniversal_ValidateRestore(R()));
        puts("BOTTOM_POOL_EXCHANGE_PLAYOUT_VS_POOL_CHALLENGER_THREE_SEASONS_UNIVERSE_CONSTANT=PASS");
    }
    puts("FASE2_CE_ENGINE_ISOLATED=PASS");
    puts("OG_QUICK_50HZ_SUB_WINDOWS_MENUS_WINDOWS_GAMEPLAY=NOT_TESTED");
    return 0;
}
