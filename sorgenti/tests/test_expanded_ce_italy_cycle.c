/* v00.13.342 RG-T: multi-year cycle of the ITALIAN pyramid (Serie A, Serie B, Serie C x3, Serie D x9) through the SAME generic
 * Competition Engine: the REAL definitions of data_json/worlds/2627/competitions (leagues.json,
 * league_calendar.json, ce_competitions.json, bottom_pools.json, test_clubs.json, league_start_history)
 * built by the runtime's own ExpCe_BuildWorldDefinitions, real club sets of AUSTRALIA.json
 * (A-League 12, NPL NSW 16, Victoria 14, Queensland 12 + pools), real registry/RunDue + CAR codec.
 * MOCKED: the OG match (deterministic scores), the disk, the first editions (synthetic seasons that
 * start after 1 July 2026: the real 2026 history is not replayed). NOT gameplay, NOT Windows. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"
#include "expanded_ce_engine.h"
#include "expanded_ce_admin.h"
#include "expanded_ce_table.h"
#include "expanded_universal_pairings.h"
#include "expanded_universal_json_leagues.h"
#include "expanded_universal_world_catalog.h"
#include "expanded_universal_live.h"
#include "world.h"
#include "expanded_ce_shared_pool_data.h"

TeamFileEx *ExpandedUniversalCatalog_Roster(ExpandedClubKey k)
{
    static TeamFileEx t;
    memset(&t,0,sizeof t);
    snprintf((char *)t.team.teamName,sizeof t.team.teamName,"C%u-%u",
             (unsigned)(k.stable_club>>16),(unsigned)(k.stable_club&0xffffu));
    return &t;
}
int ExpandedUniversalLive_ClubNameCompare(void *u,ExpandedClubKey a,ExpandedClubKey b)
{(void)u;return a.stable_club<b.stable_club?-1:a.stable_club>b.stable_club;}
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
void *swos_io_alloc(uint32_t n){return malloc(n);}
void swos_io_free(void *p){free(p);}
WorldId World_Current(void){return WORLD_2627;}
uint16_t _teamsCountryNumbers[256]={[255]=2313u};
uint8_t World_FileCountry(uint8_t f){return f;}
uint16_t World_Slot(uint8_t f,uint8_t o){(void)f;return o;}
uint16_t World_SlotTeam(uint8_t c,uint16_t slot){return (uint16_t)(((uint16_t)c<<8)|slot);}
uint16_t World_FirstYear(void){return 2026u;}
const char *swos_data_path(const char *rel,char *buf,uint32_t sz)
{snprintf(buf,sz,"../%s",rel);return buf;}
unsigned long long Platform_NowMs(void){return 0;}
void ExpandedUniversalLive_ResetCpuBatch(void){}
void ExpandedUniversalLive_RegisterClock(void){}
int CarWriter_TakeFailure(void){return 0;}

static char s_disk[1<<23];
static int s_disk_len;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    struct mjson_fixedbuf dst={s_disk,(int)sizeof s_disk,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    (void)reason;
    assert(r);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"{%Q:{%Q:%d}","living_world",
        "now",(int)r->scheduler.clock.day_index)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof s_disk-1);
    assert(ExpandedUniversal_CarValidate(s_disk,dst.len));
    s_disk_len=dst.len;
    return 1;
}
static int h_ins(void *u,const ExpandedUniversalFixture *f,uint32_t n){(void)u;return ExpandedUniversal_CarInsertFixtures(f,n);}
static int h_prune(void *u,ExpandedEditionKey e){(void)u;return ExpandedUniversal_CarPruneEdition(e);}
static int h_hold(void *u,ExpandedEditionKey e,int h){(void)u;return ExpandedUniversal_CarHoldEdition(e,h);}
static int h_res(void *u,uint64_t id,int32_t d){(void)u;return ExpandedUniversal_CarRescheduleEvent(id,d);}
static int h_arch(void *u,ExpandedEditionKey e,uint32_t c,uint32_t r,int32_t d)
{(void)u;return ExpandedUniversal_ArchiveCloseEdition(e,c,r,d);}
static int h_club(void *u,ExpandedClubKey k){(void)u;return k.stable_club!=0u;}
static uint32_t h_div(void *u,uint8_t c,uint8_t d,ExpandedClubKey *o,uint32_t cap)
{(void)u;(void)c;(void)d;(void)o;(void)cap;return 0u;}
static const ExpCeHost H={h_ins,h_prune,h_hold,h_res,h_arch,h_club,NULL,NULL,h_div};
static int32_t D(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static ExpandedUniversalRegistry *R(void){return ExpandedUniversal_CarRegistry();}

/* ---- mock OG: deterministic, varied scores; level knockout ties go to extra time / penalties ---- */
static uint64_t s_ran[40000];
static uint32_t s_ran_n,s_dupes,s_admin;
static uint64_t mix(uint64_t x){x^=x>>33;x*=UINT64_C(0xff51afd7ed558ccd);x^=x>>33;return x;}
static int og_mock(void *user,const ExpandedUniversalFixture *f,ExpandedUniversalMatchWitness *w)
{
    ExpCeMatchRules mr;
    uint8_t decides=0;
    uint16_t h1=0,a1=0;
    const uint64_t v=mix(f->event_id);
    uint16_t home=(uint16_t)(v%4u),away=(uint16_t)((v>>8)%3u),ph=0,pa=0;
    uint8_t flags=0;
    ExpandedFixtureScore admin;
    const char *why=NULL;
    uint32_t i;
    (void)user;
    for(i=0;i<s_ran_n;++i)if(s_ran[i]==f->event_id)++s_dupes;
    if(s_ran_n<sizeof s_ran/sizeof *s_ran)s_ran[s_ran_n++]=f->event_id;
    memset(w,0,sizeof *w);
    w->key=f->key;w->home=f->home;w->away=f->away;w->day_index=f->day_index;
    if(ExpCeAdmin_Decide(f,&admin,&why)){++s_admin;w->score=admin;w->valid=1u;return 1;}
    assert(ExpCe_RulesForFixture(f,&mr,NULL));
    assert(ExpCeTie_MatchContext(R(),f,&mr,&decides,&h1,&a1));
    if(decides && (v>>16)%2u==0u) {
        if(a1>=h1){home=(uint16_t)(1u+a1-h1);away=1u;}
        else {home=1u;away=(uint16_t)(1u+h1-a1);}
    }
    if(decides) {
        int level=(uint32_t)home+h1==(uint32_t)away+a1;
        if(level && mr.legs==2u && mr.away_goals && h1!=away)level=0;
        if(level && mr.extra_time)flags|=EXP_CE_SCORE_EXTRA_TIME;
        if(level && mr.penalties) {
            flags|=EXP_CE_SCORE_PENALTIES;
            if((v>>24)&1u){ph=5u;pa=4u;}else{ph=3u;pa=5u;}
        }
    }
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

/* ---- first seasons of the 14 Italian divisions with the real club sets (ITALY.json: league = division) ---- */
static uint32_t s_sz[14]={20,20,20,20,20,17,17,17,18,17,18,17,17,18};
static uint32_t s_slot0[14]={1,21,41,61,81,101,118,135,152,170,187,205,222,239};
typedef struct Em{ExpandedUniversalFixture *out;uint32_t n,clubs;uint32_t slot0;uint8_t div;
    uint32_t comp;int32_t start,day0,step;} Em;
static int em_rr(void *u,uint32_t cycle,uint32_t round,uint32_t serial,uint32_t h,uint32_t a)
{
    Em *x=(Em *)u;
    ExpandedUniversalFixture *f=&x->out[x->n++];
    const uint32_t per=(x->clubs&1u)?x->clubs:x->clubs-1u;
    memset(f,0,sizeof *f);
    f->home=(ExpandedClubKey){WORLD_2627,(20u<<16)|(x->slot0+h),EXP_PARTICIPANT_CLUB};
    f->away=(ExpandedClubKey){WORLD_2627,(20u<<16)|(x->slot0+a),EXP_PARTICIPANT_CLUB};
    f->day_index=x->day0+x->step*(int32_t)(cycle*per+round);
    f->key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,x->comp};
    f->key.edition.start_day=x->start;f->key.stage=1u;f->key.tie=serial+1u;
    f->key.leg=(uint16_t)cycle;f->country=20u;f->slot=x->div;
    f->event_id=((uint64_t)WORLD_2627<<60)|((uint64_t)x->comp<<20)|((uint64_t)cycle<<12)|(serial+1u);
    return 1;
}
static void first_season(uint8_t div,int32_t start,int32_t day0)
{
    static ExpandedUniversalFixture fx[1024];
    Em e;
    memset(&e,0,sizeof e);
    e.out=fx;e.clubs=s_sz[div];e.slot0=s_slot0[div];e.div=div;
    e.comp=ExpCe_LeagueCompetitionId(20u,61u,div);e.start=start;e.day0=day0;e.step=7;
    assert(ExpandedUniversal_GenerateRoundRobin(e.clubs,2u,em_rr,&e));
    assert(ExpandedUniversal_CarInsertFixtures(fx,e.n));
}
static const ExpCeEdition *edition_of(uint32_t comp,uint16_t year)
{
    uint32_t i;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *e=ExpCe_EditionAt(i);
        if(e->key.competition.stable_competition==comp && e->year==year)return e;
    }
    return NULL;
}
static __attribute__((unused)) int has(const ExpCeEdition *e,uint32_t slot)
{
    uint32_t i;
    for(i=0;e && i<e->nentrants;++i)if(e->entrants[i].club.stable_club==((44u<<16)|slot))return 1;
    return 0;
}
static void setup(void)
{
    uint32_t div;
    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
    s_ran_n=s_dupes=s_admin=0u;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},65536u));
    assert(ExpandedUniversalJsonLeagues_Load());
    assert(ExpCe_BuildWorldDefinitions(WORLD_2627)>0);
    for(div=0u;div<14u;++div) {
        assert(ExpCe_Definition(ExpCe_LeagueCompetitionId(20u,61u,div)));
        if(div==0u)first_season(0u,D(2026,8,22),D(2026,8,23));      /* Serie A: real window from 22/08 */
        else first_season((uint8_t)div,D(2026,9,1),D(2026,9,6));    /* the other divisions from 01/09 (Sundays) */
    }
    assert(ExpCe_BeginWorld(R(),&H,WORLD_2627,2026));
}
#define SUPERCOPPA 1310978u
static uint32_t it_comp(uint32_t div){return ExpCe_LeagueCompetitionId(20u,61u,(uint8_t)div);}
static int is_it(uint32_t c)
{
    uint32_t d;
    if(c==SUPERCOPPA)return 1;
    for(d=0u;d<14u;++d)if(c==it_comp(d))return 1;
    return 0;
}
/* signature of the whole Italian state */
static uint64_t sig(void)
{
    uint64_t h=UINT64_C(14695981039346656037);
    uint32_t i,k;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *x=ExpCe_EditionAt(i);
        if(!is_it(x->key.competition.stable_competition))continue;
#define MIX(v) do{h^=(uint64_t)(v);h*=UINT64_C(1099511628211);}while(0)
        MIX(x->key.competition.stable_competition);MIX(x->year);MIX(x->state);MIX(x->block);
        MIX(x->nentrants);MIX(x->nfinal);MIX(x->nregular);MIX(x->phase);MIX(x->successor);MIX(x->expiry_day);
        for(k=0;k<x->nentrants;++k)MIX(x->entrants[k].club.stable_club);
        for(k=0;k<x->nfinal;++k)MIX(x->final_order[k]);
        for(k=0;k<x->nregular;++k)MIX(x->regular_order[k]);
#undef MIX
    }
    return h;
}
static void save_restore(void)
{
    static char disk[1<<23];
    int len;
    const uint64_t before=sig();
    const int32_t today=R()->scheduler.clock.day_index;
    assert(Expanded_AutoCareer_SaveChecked("it-cycle"));
    assert(s_disk_len<(int)sizeof disk-1);
    memcpy(disk,s_disk,(size_t)s_disk_len);len=s_disk_len;disk[len]=0;
    ExpandedUniversal_CarResetCurrent();ExpCe_Reset();
    assert(ExpandedUniversalJsonLeagues_Load());
    assert(ExpCe_BuildWorldDefinitions(WORLD_2627)>0);
    assert(ExpandedUniversal_CarValidate(disk,len));
    assert(ExpandedUniversal_CarRestore(disk,len));
    {   char *hex=ExpandedUniversal_CarTakeLifecycle();
        assert(hex && ExpCe_DecodeHex(hex,strlen(hex),WORLD_2627));free(hex);
    }
    assert(ExpCe_ValidateAfterRestore(R(),&H)>=0);
    assert(R()->scheduler.clock.day_index==today);
    assert(sig()==before);
}
static uint32_t rank_at(const ExpCeEdition *e,uint32_t i)
{return e->nregular?(i<e->nregular?e->regular_order[i]:0u):(i<e->nfinal?e->final_order[i]:0u);}
static const ExpCeEdition *latest_of(uint32_t comp)
{
    const ExpCeEdition *best=NULL;
    uint32_t i;
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *x=ExpCe_EditionAt(i);
        if(x->key.competition.stable_competition==comp && (!best || x->key.start_day>best->key.start_day))best=x;
    }
    return best;
}
static int has_key(const ExpCeEdition *e,uint32_t club)
{
    uint32_t i;
    for(i=0;e && i<e->nentrants;++i)if(e->entrants[i].club.stable_club==club)return 1;
    return 0;
}
static unsigned s_checks,s_restores,s_moves;
int main(void)
{
    uint32_t i,k,y,m,div;
    setup();
    /* the Supercoppa and the ceased historical competitions: nothing is regenerated */
    for(i=0;i<ExpCe_DefinitionCount();++i)assert(!strstr(ExpCe_DefinitionAt(i)->name,"NSL"));
    for(y=2026u;y<=2031u;++y)
      for(m=(y==2026u?8u:1u);m<=12u && !(y==2031u && m>7u);++m) {
        uint32_t seen=0u;
        run_until(D((int)y,(int)m,1));
        for(div=0u;div<14u;++div) {
            const ExpCeEdition *latest=latest_of(it_comp(div));
            if(latest && latest->block)
                printf("[cycle] BLOCKED div=%u year=%u block=%d why=%s at %u-%02u\n",(unsigned)div,(unsigned)latest->year,
                       (int)latest->block,latest->why,(unsigned)y,(unsigned)m);
            if(latest && latest->block) {
                uint32_t j,cnt=0u,da,db;
                for(da=0u;da<14u;++da)for(db=da+1u;db<14u;++db) {
                    const ExpCeEdition *xa=latest_of(it_comp(da)),*xb=latest_of(it_comp(db));
                    uint32_t ka;
                    for(ka=0u;xa && xb && ka<xa->nentrants;++ka)
                        if(has_key(xb,xa->entrants[ka].club.stable_club))
                            printf("[cycle] SHARED club=%u in div %u (year %u) and div %u (year %u)\n",
                                   (unsigned)(xa->entrants[ka].club.stable_club&0xffffu),(unsigned)da,(unsigned)xa->year,
                                   (unsigned)db,(unsigned)xb->year);
                }
                for(j=0u;j<R()->fixture_count;++j) {
                    const ExpandedUniversalFixture *f=&R()->fixtures[j];
                    if(f->key.edition.competition.stable_competition==SUPERCOPPA || f->key.edition.competition.stable_competition==it_comp(1u)) {
                        if(cnt<40u && f->day_index>=D(2030,6,1) && f->day_index<=D(2031,1,31)) {
                            printf("[cycle] other fixture comp=%u start=%d day=%d clubs=%u/%u stage=%u tie=%u\n",
                                   (unsigned)f->key.edition.competition.stable_competition,(int)f->key.edition.start_day,
                                   (int)f->day_index,(unsigned)f->home.stable_club,(unsigned)f->away.stable_club,(unsigned)f->key.stage,(unsigned)f->key.tie);
                            ++cnt;
                        }
                    }
                    if(0 && f->day_index>=D(2030,8,22) && f->day_index<=D(2030,12,31) && cnt<12u) {
                        printf("[cycle] registry fixture comp=%u start=%d day=%d clubs=%u/%u stage=%u\n",
                               (unsigned)f->key.edition.competition.stable_competition,(int)f->key.edition.start_day,
                               (int)f->day_index,(unsigned)f->home.stable_club,(unsigned)f->away.stable_club,(unsigned)f->key.stage);
                        ++cnt;
                    }
                }
            }
            assert(latest && !latest->block && latest->nentrants==s_sz[div]);
            for(k=0u;k<latest->nentrants;++k) {
                uint32_t q;
                const uint32_t c=latest->entrants[k].club.stable_club;
                for(q=0u;q<k;++q)assert(latest->entrants[q].club.stable_club!=c);
                assert((c>>16)==20u);
            }
            seen+=latest->nentrants;
        }
        assert(seen==256u);
        if(m==1u) {                                   /* settled date: nobody in two divisions */
            uint32_t a,b;
            for(a=0u;a<14u;++a)for(b=a+1u;b<14u;++b) {
                const ExpCeEdition *ea=latest_of(it_comp(a)),*eb=latest_of(it_comp(b));
                for(k=0u;k<ea->nentrants;++k)assert(!has_key(eb,ea->entrants[k].club.stable_club));
            }
        }
        assert(ExpandedUniversal_ValidateRestore(R()) && s_dupes==0u);
        ++s_checks;
        save_restore();++s_restores;
    }
    /* ===== Serie A <-> Serie B <-> Serie C movements, year by year ===== */
    for(y=2026u;y<=2029u;++y) {
        const ExpCeEdition *a=edition_of(it_comp(0u),(uint16_t)y),*an=edition_of(it_comp(0u),(uint16_t)(y+1u)),
                           *b=edition_of(it_comp(1u),(uint16_t)y),*bn=edition_of(it_comp(1u),(uint16_t)(y+1u));
        uint32_t relA[3],upB[3],relB[4],nrelB=0u,lo=0u,j;
        const ExpCeDef *bd=ExpCe_Definition(it_comp(1u));
        assert(a && an && b && bn && a->state>=EXP_CE_ED_COMPLETED && b->state>=EXP_CE_ED_COMPLETED);
        assert(an->nentrants==20u && bn->nentrants==20u);
        for(j=0u;j<3u;++j)relA[j]=rank_at(a,17u+j);                          /* Serie A: the bottom three go down */
        upB[0]=b->regular_order[0];upB[1]=b->regular_order[1];upB[2]=b->final_order[0]; /* B: 2 direct + play-off winner */
        assert(upB[2]!=upB[0] && upB[2]!=upB[1]);
        for(j=0u;j<3u;++j) {
            assert(!has_key(an,relA[j]) && has_key(bn,relA[j]));              /* relegated from A: in B next season */
            assert(has_key(an,upB[j]) && !has_key(bn,upB[j]));                /* promoted from B: in A, no longer in B */
        }
        /* Serie B relegates FOUR in total: the bottom three of the regular table + the play-out loser */
        for(j=0u;j<3u;++j)relB[nrelB++]=b->regular_order[b->nregular-3u+j];
        assert(ExpCe_PlayoutActive(bd));
        {   /* the 4th relegated is the only other club that left B (not promoted): the play-out loser, rank 16 or 17 */
            uint32_t q,found=0u;
            for(q=0u;q<b->nentrants;++q) {
                const uint32_t c=b->entrants[q].club.stable_club;
                if(has_key(bn,c) || c==upB[0] || c==upB[1] || c==upB[2] || c==relB[0] || c==relB[1] || c==relB[2])continue;
                lo=c;++found;
            }
            assert(found==1u && (lo==b->regular_order[15] || lo==b->regular_order[16]));
        }
        relB[nrelB++]=lo;
        for(j=0u;j<4u;++j)for(k=j+1u;k<4u;++k)assert(relB[j]!=relB[k]);
        for(j=0u;j<4u;++j) {
            int in_c=0;
            uint32_t g;
            assert(!has_key(bn,relB[j]));
            for(g=2u;g<5u;++g)if(has_key(edition_of(it_comp(g),(uint16_t)(y+1u)),relB[j]))in_c=1;
            assert(in_c);                                                    /* ... and in a Serie C group */
        }
        ++s_moves;
        printf("[cycle] year %u: A down %u/%u/%u, B up %u/%u/%u(play-off), B down(4) %u/%u/%u/%u(play-out)\n",(unsigned)y,
               (unsigned)(relA[0]&0xffffu),(unsigned)(relA[1]&0xffffu),(unsigned)(relA[2]&0xffffu),
               (unsigned)(upB[0]&0xffffu),(unsigned)(upB[1]&0xffffu),(unsigned)(upB[2]&0xffffu),
               (unsigned)(relB[0]&0xffffu),(unsigned)(relB[1]&0xffffu),(unsigned)(relB[2]&0xffffu),(unsigned)(relB[3]&0xffffu));
    }
    /* every ended edition: trophies in the archive equal the final order */
    for(i=0;i<ExpCe_EditionCount();++i) {
        const ExpCeEdition *x=ExpCe_EditionAt(i);
        if(!is_it(x->key.competition.stable_competition) || x->state<EXP_CE_ED_COMPLETED)continue;
        assert(!x->block && x->nfinal>=2u);
        if(x->state==EXP_CE_ED_ARCHIVED) {
            const ExpandedEditionSummary *sm=NULL;
            uint32_t q;
            for(q=0;q<ExpandedUniversal_ArchiveCount();++q) {
                const ExpandedEditionSummary *c=ExpandedUniversal_ArchiveAt(q);
                if(ExpandedEditionKey_Equal(c->edition,x->key))sm=c;
            }
            /* the title belongs to the REGULAR table winner (Serie B: the play-off winner is promoted, not champion) */
            const uint32_t t0=x->nregular?x->regular_order[0]:x->final_order[0],t1=x->nregular?x->regular_order[1]:x->final_order[1];
            assert(sm && sm->champion==t0 && sm->runner_up==t1);
        }
    }
    printf("[cycle] checks=%u restores=%u year-transitions=%u dupes=%u\n",s_checks,s_restores,s_moves,s_dupes);
    puts("ITALY_CYCLE_FIVE_YEARS_REAL_DEFINITIONS_A_B_C_MOVEMENTS_RESTORE_TROPHIES=PASS");
    return 0;
}
